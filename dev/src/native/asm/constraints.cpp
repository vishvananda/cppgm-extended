#include "native/asm/constraints.h"

#include <algorithm>

#include "native/backend_error.h"

namespace native {
namespace {

using namespace asm_model;
using namespace mir_model;

unsigned width(lowir::Type type, RegisterBank bank)
{
	if (bank == RegisterBank::Vector)
		return 128;
	return type.is_vector()                                        ? 128
		: type.kind() == lowir::TypeKind::F32                      ? 32
		: type.kind() == lowir::TypeKind::F64 || type.is_pointer() ? 64
																   : std::max(8u, lowir::integer_bits(type.kind()));
}

uint32_t register_domain(
	const lowir::AssemblyModule& module, const lowir::AssemblyOperand& operand, const Alternative& alt)
{
	if (operand.type.is_object() || operand.type.is_void() || operand.type.kind() == lowir::TypeKind::I128 ||
		operand.type.kind() == lowir::TypeKind::F80)
		return 0;
	if (alt.matching != asm_model::kNoId)
		return kAllXmms | (operand.type.is_vector() ? 0 : 0xffcfu);
	if ((alt.locations & AL_REGISTER) == 0)
		return 0;
	uint32_t mask = 0;
	for (uint32_t i = 0; i < alt.group_count; ++i)
	{
		const RegisterGroup& group = module.groups[alt.first_group + i];
		if (group.register_count != 1)
			continue;
		const Register& reg = module.registers[group.first_register];
		if (reg.offset != 0 || reg.number >= 16 ||
			(reg.bank != RegisterBank::Gpr && reg.bank != RegisterBank::Vector) ||
			(reg.bank == RegisterBank::Gpr && operand.type.is_vector()) ||
			(reg.bits != 0 && reg.bits != width(operand.type, reg.bank)))
			continue;
		const Reg r = static_cast<Reg>(reg.number + (reg.bank == RegisterBank::Vector ? 16 : 0));
		if (r != RSP && r != RBP)
			mask |= reg_bit(r);
	}
	return mask;
}

bool less_equal(const lowir::Literal& a, const Bits& b, bool is_signed)
{
	const uint64_t sign = is_signed ? uint64_t(1) << 63 : 0;
	return a.high == b.high ? a.low <= b.low : (a.high ^ sign) < (b.high ^ sign);
}

bool immediate_domain(const lowir::AssemblyModule& module, const Alternative& alt, const lowir::FunctionBody& body,
	lowir::Value value, lowir::Literal& bits)
{
	if ((alt.locations & AL_IMMEDIATE) == 0 || !value.is(lowir::ValueKind::Integer))
		return false;
	bits = body.literals[value.index];
	if (alt.immediate_count == 0)
		return true;
	for (uint32_t i = 0; i < alt.immediate_count; ++i)
	{
		const Immediate& domain = module.immediates[alt.first_immediate + i];
		const lowir::Literal low{ domain.low.low, domain.low.high };
		if (domain.kind == ImmediateKind::IntegerRange &&
			less_equal(low, Bits{ bits.low, bits.high }, domain.is_signed) &&
			less_equal(bits, domain.high, domain.is_signed))
			return true;
	}
	return false;
}

bool symbol_value(const lowir::FunctionBody& body, lowir::Value value, SymbolReference& result, unsigned depth = 0)
{
	if (value.is(lowir::ValueKind::Symbol))
	{
		result.symbol = value.index;
		return true;
	}
	if (!value.is(lowir::ValueKind::Temp) || depth == 64)
		return false;
	const lowir::InstId def = body.temps[value.index].def;
	if (def == lowir::kNoId)
		return false;
	const lowir::Instruction& inst = body.instructions[def];
	if (inst.op == lowir::Opcode::Addr || inst.op == lowir::Opcode::Copy)
		return symbol_value(body, inst.ops[0], result, depth + 1);
	if (inst.op != lowir::Opcode::Index || !inst.ops[1].is(lowir::ValueKind::Integer) ||
		!symbol_value(body, inst.ops[0], result, depth + 1))
		return false;
	const lowir::Literal& bits = body.literals[inst.ops[1].index];
	const int64_t index = static_cast<int64_t>(bits.low);
	const uint64_t scale = (inst.type.is_object() ? inst.type.bytes : lowir::data_alignment(inst.type));
	if (scale == 0 || scale > INT32_MAX || bits.high != (index < 0 ? ~uint64_t(0) : 0) ||
		index < INT32_MIN / static_cast<int64_t>(scale) || index > INT32_MAX / static_cast<int64_t>(scale))
		return false;
	result.addend += index * static_cast<int64_t>(scale);
	return result.addend >= INT32_MIN && result.addend <= INT32_MAX;
}

struct Group
{
	uint32_t registers = ~uint32_t(0);
	uint32_t addresses = 0;
	uint8_t access = 0;
	bool early = false;
	bool immediate = false;
	bool symbol = false;
	bool label = false;
	uint8_t location = 0;
	Reg assigned = kNoReg;
};

uint8_t access(const Group& group)
{
	return group.location == AL_MEMORY ? uint8_t(AO_READ) : group.access;
}

bool conflict(const Group& a, const Group& b)
{
	const uint8_t aa = access(a), ba = access(b);
	return ((aa & ba & (AO_READ | AO_WRITE)) != 0) || (a.location == AL_REGISTER && a.early && (ba & AO_READ) != 0) ||
		(b.location == AL_REGISTER && b.early && (aa & AO_READ) != 0);
}

uint32_t available(const std::vector<Group>& groups, uint32_t index, uint32_t mask)
{
	for (uint32_t j = 0; j < groups.size(); ++j)
		if (groups[j].assigned != kNoReg && conflict(groups[index], groups[j]))
			mask &= ~reg_bit(groups[j].assigned);
	return mask;
}

const Reg kPreference[] = { R10, R11, XMM14, XMM15, RAX, RCX, RDX, RSI, RDI, R8, R9, XMM0, XMM1, XMM2, XMM3, XMM4, XMM5,
	XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, RBX, R12, R13, R14, R15 };

// Location and physical register are one joint choice. Memory operands retain
// their address registers for the effect; an immediate uses no register unit.
bool search(std::vector<Group>& groups, uint32_t left, uint32_t& work)
{
	if (left == 0)
		return true;
	if (++work > 100000)
		throw BackendError("assembly constraint search exceeded its work limit");
	uint32_t selected = 0;
	unsigned count = 65;
	for (uint32_t i = 0; i < groups.size(); ++i)
	{
		Group& group = groups[i];
		if (group.location != 0)
			continue;
		group.location = AL_REGISTER;
		unsigned n = static_cast<unsigned>(__builtin_popcount(available(groups, i, group.registers)));
		group.location = AL_MEMORY;
		n += static_cast<unsigned>(__builtin_popcount(available(groups, i, group.addresses))) + group.immediate +
			group.symbol + group.label;
		group.location = 0;
		if (n == 0)
			return false;
		if (n < count)
		{
			selected = i;
			count = n;
		}
	}
	Group& group = groups[selected];
	if (group.immediate || group.symbol || group.label)
	{
		group.location = group.label ? AL_LABEL : group.symbol ? AL_SYMBOL : AL_IMMEDIATE;
		if (search(groups, left - 1, work))
			return true;
	}
	for (uint8_t location : { uint8_t(AL_REGISTER), uint8_t(AL_MEMORY) })
	{
		group.location = location;
		const uint32_t mask = available(groups, selected, location == AL_REGISTER ? group.registers : group.addresses);
		for (Reg r : kPreference)
			if ((mask & reg_bit(r)) != 0)
			{
				group.assigned = r;
				if (search(groups, left - 1, work))
					return true;
				group.assigned = kNoReg;
			}
	}
	group.location = 0;
	return false;
}

bool column(const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition, uint32_t column,
	uint32_t clobbers, const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments,
	AssemblyBinding& result, const std::vector<uint32_t>& sources, uint32_t& work)
{
	std::vector<uint32_t> roots(definition.operand_count), vertices(definition.operand_count, asm_model::kNoId);
	std::vector<uint8_t> matched(definition.operand_count, 0);
	result.constants.assign(definition.operand_count, lowir::Literal{});
	result.symbols.assign(definition.operand_count, SymbolReference{});
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& op = module.operands[definition.first_operand + i];
		roots[i] = (op.flags & (AO_FLAG | AO_EXIT_TAG)) != 0
			? i
			: module.alternatives[op.first_alternative + column].matching;
		if (roots[i] == asm_model::kNoId)
			roots[i] = i;
		else if (roots[i] != i)
			matched[roots[i]] = 1;
	}
	std::vector<Group> groups;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& op = module.operands[definition.first_operand + i];
		if ((op.flags & (AO_FLAG | AO_EXIT_TAG)) != 0)
			continue;
		const Alternative& alt = module.alternatives[op.first_alternative + column];
		const uint32_t root = roots[i];
		uint32_t& vertex = vertices[root];
		if (vertex == asm_model::kNoId)
		{
			vertex = static_cast<uint32_t>(groups.size());
			groups.push_back(Group());
		}
		Group& group = groups[vertex];
		if (alt.locations == AL_LABEL)
		{
			const lowir::Value value = arguments[op.input];
			if (!value.is(lowir::ValueKind::Integer) || body.literals[value.index].high != 0 ||
				body.literals[value.index].low == 0 || body.literals[value.index].low > INT32_MAX)
				throw BackendError("assembly label requires a positive i32 exit tag");
			result.constants[i] = body.literals[value.index];
			group.registers = 0;
			group.label = true;
			continue;
		}
		group.registers &= register_domain(module, op, alt) & ~clobbers;
		group.access |= op.flags & (AO_READ | AO_WRITE);
		group.early |= alt.early_write;
		if (!matched[root])
		{
			const lowir::AssemblyOperand& source = module.operands[definition.first_operand + sources[i]];
			const bool shared_storage = op.output == asm_model::kNoId ||
				(op.input != asm_model::kNoId && arguments[source.input] == arguments[op.output]);
			const bool address = (source.input_storage && source.input != asm_model::kNoId && shared_storage) ||
				((op.flags & AO_READ) == 0 && op.output != asm_model::kNoId);
			if ((alt.locations & AL_MEMORY) != 0 && (op.flags & AO_NO_MEMORY) == 0 && address)
				group.addresses = 0xffcfu & ~clobbers;
			if ((op.flags & (AO_READ | AO_WRITE)) == AO_READ && !source.input_storage)
			{
				group.immediate = immediate_domain(module, alt, body, arguments[source.input], result.constants[i]);
				group.symbol = (alt.locations & AL_SYMBOL) != 0 &&
					symbol_value(body, arguments[source.input], result.symbols[i]);
			}
		}
	}
	if (!search(groups, static_cast<uint32_t>(groups.size()), work))
		return false;
	result.registers.assign(definition.operand_count, kNoReg);
	result.locations.assign(definition.operand_count, 0);
	for (uint32_t i = 0; i < definition.operand_count; ++i)
		if (vertices[roots[i]] != asm_model::kNoId)
		{
			const Group& group = groups[vertices[roots[i]]];
			result.registers[i] = group.assigned;
			result.locations[i] = group.location;
		}
	result.sources = sources;
	return true;
}

} // namespace

AssemblyBinding assign_assembly_operands(const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition,
	uint32_t clobbers, const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments)
{
	uint32_t columns = 1;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if ((operand.flags & (AO_FLAG | AO_EXIT_TAG)) == 0)
			columns = operand.alternative_count;
	}
	uint32_t work = 0;
	AssemblyBinding result;
	std::vector<uint32_t> sources(definition.operand_count), pairs;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		sources[i] = i;
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if ((operand.flags & AO_COMMUTATIVE) != 0)
		{
			const lowir::AssemblyOperand& other = module.operands[definition.first_operand + i + 1];
			if (operand.type == other.type)
				pairs.push_back(i);
		}
	}
	std::vector<bool> swapped(pairs.size(), false);
	for (;;)
	{
		for (uint32_t c = 0; c < columns; ++c)
		{
			if (++work > 100000)
				throw BackendError("assembly constraint search exceeded its work limit");
			if (column(module, definition, c, clobbers, body, arguments, result, sources, work))
				return result;
		}
		size_t pair = 0;
		for (; pair < pairs.size(); ++pair)
		{
			std::swap(sources[pairs[pair]], sources[pairs[pair] + 1]);
			swapped[pair] = !swapped[pair];
			if (swapped[pair])
				break;
		}
		if (pair == pairs.size())
			break;
	}
	throw BackendError("assembly constraints are unsupported or have no valid joint assignment");
}

} // namespace native
