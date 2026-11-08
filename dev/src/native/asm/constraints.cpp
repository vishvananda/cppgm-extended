#include "native/asm/constraints.h"

#include <algorithm>

#include "native/backend_error.h"

namespace native {
namespace {

using namespace asm_model;
using namespace mir_model;

unsigned width(lowir::Type type)
{
	return type.is_vector() || type.is_float() ? 128
		: type.is_pointer()                    ? 64
											   : std::max(8u, lowir::integer_bits(type.kind()));
}

uint32_t domain(const lowir::AssemblyModule& module, const lowir::AssemblyOperand& operand, uint32_t column)
{
	if (operand.input_storage || operand.type.is_object() || operand.type.is_void() ||
		operand.type.kind() == lowir::TypeKind::I128 || operand.type.kind() == lowir::TypeKind::F80)
		return 0;
	const Alternative& alt = module.alternatives[operand.first_alternative + column];
	if (alt.matching != asm_model::kNoId)
		return (operand.type.is_vector() || operand.type.is_float()) ? kAllXmms : 0xffffu;
	if ((alt.locations & AL_REGISTER) == 0)
		return 0;
	uint32_t mask = 0;
	for (uint32_t i = 0; i < alt.group_count; ++i)
	{
		const RegisterGroup& group = module.groups[alt.first_group + i];
		if (group.register_count != 1)
			continue;
		const Register& reg = module.registers[group.first_register];
		const bool vector = operand.type.is_vector() || operand.type.is_float();
		if (reg.offset != 0 || reg.number >= 16 || (reg.bits != 0 && reg.bits != width(operand.type)) ||
			reg.bank != (vector ? RegisterBank::Vector : RegisterBank::Gpr))
			continue;
		const Reg r = static_cast<Reg>(reg.number + (vector ? 16 : 0));
		if (r != RSP && r != RBP)
			mask |= reg_bit(r);
	}
	return mask;
}

struct Group
{
	uint32_t domain = 0;
	uint8_t access = 0;
	bool early = false;
	Reg assigned = kNoReg;
};

bool conflict(const Group& a, const Group& b)
{
	return ((a.access & b.access & (AO_READ | AO_WRITE)) != 0) || (a.early && (b.access & AO_READ) != 0) ||
		(b.early && (a.access & AO_READ) != 0);
}

const Reg kPreference[] = { R10, R11, XMM14, XMM15, RAX, RCX, RDX, RSI, RDI, R8, R9, XMM0, XMM1, XMM2, XMM3, XMM4, XMM5,
	XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, RBX, R12, R13, R14, R15 };

// Select the most constrained remaining vertex, propagate conflicts, and
// backtrack jointly. A late output can share a read-only input; early writes,
// independent inputs and independent outputs cannot. Matching operands are
// one vertex, rather than a coincidental equal physical-register choice.
bool search(std::vector<Group>& groups, uint32_t left, uint32_t& work)
{
	if (left == 0)
		return true;
	if (++work > 100000)
		throw BackendError("assembly register-constraint search exceeded its work limit");
	uint32_t selected = 0, available = 0;
	unsigned count = 33;
	for (uint32_t i = 0; i < groups.size(); ++i)
	{
		if (groups[i].assigned != kNoReg)
			continue;
		uint32_t mask = groups[i].domain;
		for (uint32_t j = 0; j < groups.size(); ++j)
			if (groups[j].assigned != kNoReg && conflict(groups[i], groups[j]))
				mask &= ~reg_bit(groups[j].assigned);
		const unsigned n = static_cast<unsigned>(__builtin_popcount(mask));
		if (n == 0)
			return false;
		if (n < count)
		{
			selected = i;
			available = mask;
			count = n;
		}
	}
	for (Reg r : kPreference)
		if ((available & reg_bit(r)) != 0)
		{
			groups[selected].assigned = r;
			if (search(groups, left - 1, work))
				return true;
			groups[selected].assigned = kNoReg;
		}
	return false;
}

bool column(const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition, uint32_t column,
	uint32_t clobbers, std::vector<Reg>& result, uint32_t& work)
{
	std::vector<uint32_t> roots(definition.operand_count), vertices(definition.operand_count, asm_model::kNoId);
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& op = module.operands[definition.first_operand + i];
		roots[i] = (op.flags & AO_FLAG) != 0 ? i : module.alternatives[op.first_alternative + column].matching;
		if (roots[i] == asm_model::kNoId)
			roots[i] = i;
	}
	std::vector<Group> groups;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& op = module.operands[definition.first_operand + i];
		if ((op.flags & AO_FLAG) != 0)
			continue;
		const uint32_t root = roots[i];
		uint32_t& vertex = vertices[root];
		const uint32_t mask = domain(module, op, column) & ~clobbers;
		if (mask == 0)
			return false;
		if (vertex == asm_model::kNoId)
		{
			vertex = static_cast<uint32_t>(groups.size());
			Group group;
			group.domain = mask;
			groups.push_back(group);
		}
		Group& group = groups[vertex];
		group.domain &= mask;
		group.access |= op.flags & (AO_READ | AO_WRITE);
		group.early |= module.alternatives[op.first_alternative + column].early_write;
	}
	if (!search(groups, static_cast<uint32_t>(groups.size()), work))
		return false;
	result.assign(definition.operand_count, kNoReg);
	for (uint32_t i = 0; i < definition.operand_count; ++i)
		if (vertices[roots[i]] != asm_model::kNoId)
			result[i] = groups[vertices[roots[i]]].assigned;
	return true;
}

} // namespace

std::vector<Reg> assign_assembly_registers(
	const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition, uint32_t clobbers)
{
	uint32_t columns = 1;
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const lowir::AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if ((operand.flags & AO_FLAG) == 0)
			columns = operand.alternative_count;
	}
	uint32_t work = 0;
	std::vector<Reg> result;
	for (uint32_t c = 0; c < columns; ++c)
		if (column(module, definition, c, clobbers, result, work))
			return result;
	throw BackendError("assembly register constraints are unsupported or have no valid assignment");
}

} // namespace native
