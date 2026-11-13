#include "backend/lowir_c_module.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include "lowir/builder.h"

namespace {

// The direct ABI writes the object representations of these trivially
// copyable records. Keep every field's offset and every record's span checked.
#define CHECK_FIELD(external, internal, field) \
	static_assert(offsetof(external, field) == offsetof(internal, field), "direct field offset")
#define CHECK_SIZE(external, internal) \
	static_assert(sizeof(external) == sizeof(internal) && std::is_trivially_copyable<internal>::value, "direct record")
CHECK_SIZE(CppgmDirectInstruction, lowir::Instruction);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, op);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, sub);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, flags);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, order);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, result);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, next);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, location);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, type);
static_assert(offsetof(CppgmDirectInstruction, operands) == offsetof(lowir::Instruction, ops), "direct operands");
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, extra_first);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, extra_count);
CHECK_FIELD(CppgmDirectInstruction, lowir::Instruction, immediate);
CHECK_SIZE(CppgmDirectValue, lowir::Value);
CHECK_FIELD(CppgmDirectValue, lowir::Value, kind);
CHECK_FIELD(CppgmDirectValue, lowir::Value, float_kind);
CHECK_FIELD(CppgmDirectValue, lowir::Value, index);
CHECK_SIZE(CppgmLiteral, lowir::Literal);
CHECK_SIZE(CppgmDirectSlot, lowir::Slot);
CHECK_FIELD(CppgmDirectSlot, lowir::Slot, name);
CHECK_FIELD(CppgmDirectSlot, lowir::Slot, kind);
CHECK_FIELD(CppgmDirectSlot, lowir::Slot, type);
#undef CHECK_SIZE
#undef CHECK_FIELD

template<class T>
void grow(std::vector<T>& values, uint32_t minimum)
{
	if (values.size() < minimum)
		values.resize(std::max(static_cast<uint64_t>(minimum), std::min<uint64_t>(UINT32_MAX, values.size() * 2)));
}

bool valid_value(lowir::Value v)
{
	return backend::c_api::valid_value(
		CppgmValue{ static_cast<uint32_t>(v.kind) | (static_cast<uint32_t>(v.float_kind) << 8), v.index });
}

bool valid_type(lowir::Type t)
{
	return backend::c_api::valid_type(t.kind_bits | (t.align_log2 << 8) | (t.bytes << 16));
}

} // namespace

namespace backend {
namespace c_api {

bool direct_layout()
{
	const lowir::Type sample = lowir::Type::object(0x12345, 3);
	uint64_t bits;
	std::memcpy(&bits, &sample, sizeof bits);
	return bits == (CPPGM_TYPE_OBJ | (uint64_t(3) << 8) | (uint64_t(0x12345) << 16));
}

} // namespace c_api
} // namespace backend

bool CppgmLowir::reserve_body(uint32_t symbol, const CppgmBodyCounts& minimum, CppgmBodyBuffers& buffers)
{
	if (!ready())
		return false;
	if (symbol >= program.symbols.size() || program.symbols[symbol].kind != lowir::SymbolKind::FunctionDefinition)
		return fail("direct body requires a defined function");
	direct_bodies.resize(program.symbols.size(), 0);
	lowir::FunctionBody& body = *program.functions[program.symbols[symbol].detail].body;
	if (direct_bodies[symbol] == 2 || (direct_bodies[symbol] == 0 && (!body.blocks.empty() || !body.temps.empty())))
		return fail("function body already supplied");
	direct_bodies[symbol] = 1;
	grow(body.instructions, minimum.instructions);
	grow(body.operands, minimum.operands);
	grow(body.literals, minimum.literals);
	grow(body.slots, minimum.slots);
	grow(body.blocks, minimum.blocks);
	grow(body.layout, static_cast<uint32_t>(body.blocks.size()));
	buffers.instructions = body.instructions.data();
	buffers.operands = body.operands.data();
	buffers.literals = body.literals.data();
	buffers.slots = body.slots.data();
	buffers.layout = body.layout.data();
	buffers.capacity = CppgmBodyCounts{ static_cast<uint32_t>(body.instructions.size()),
		static_cast<uint32_t>(body.operands.size()), static_cast<uint32_t>(body.literals.size()),
		static_cast<uint32_t>(body.slots.size()), static_cast<uint32_t>(body.blocks.size()) };
	return true;
}

bool CppgmLowir::commit_body(uint32_t symbol, const CppgmBodyCounts& counts, uint32_t temp_count)
{
	if (!ready())
		return false;
	if (symbol >= direct_bodies.size() || direct_bodies[symbol] != 1)
		return fail("no open direct body");
	lowir::FunctionBody& body = *program.functions[program.symbols[symbol].detail].body;
	if (counts.instructions > body.instructions.size() || counts.operands > body.operands.size() ||
		counts.literals > body.literals.size() || counts.slots > body.slots.size() ||
		counts.blocks > body.blocks.size())
		return fail("direct body exceeds reserved storage");
	body.instructions.resize(counts.instructions);
	body.operands.resize(counts.operands);
	body.literals.resize(counts.literals);
	body.slots.resize(counts.slots);
	body.blocks.resize(counts.blocks);
	body.layout.resize(counts.blocks);
	lowir::FunctionBuilder builder(program, symbol);
	if (temp_count < body.parameter_count)
		return fail("direct body omits parameters");
	body.temps.resize(temp_count, lowir::Temp{ 0, lowir::Type::scalar(lowir::TypeKind::Void), lowir::kNoId });
	for (const lowir::Slot& slot : body.slots)
		if (!valid_type(slot.type) || slot.type.is_void() || slot.name != 0 || slot.kind != lowir::SlotKind::Plain)
			return fail("invalid direct slot");
	for (lowir::Block& block : body.blocks)
		block = lowir::Block{ 0, lowir::kNoId, lowir::kNoId, lowir::BlockKind::Plain };
	for (lowir::Value operand : body.operands)
		if (!valid_value(operand))
			return fail("invalid direct operand");
	for (lowir::InstId id = 0; id < body.instructions.size(); ++id)
	{
		lowir::Instruction& inst = body.instructions[id];
		if (static_cast<unsigned>(inst.op) >= lowir::kOpcodeCount || !valid_type(inst.type) ||
			inst.location >= counts.blocks ||
			(inst.op == lowir::Opcode::Convert && !valid_type(lowir::convert_source_type(inst))))
			return fail("invalid direct instruction");
		lowir::Block& block = body.blocks[inst.location];
		inst.location = 0;
		inst.next = lowir::kNoId;
		if (block.last == lowir::kNoId)
			block.first = id;
		else
			body.instructions[block.last].next = id;
		block.last = id;
		for (lowir::Value operand : inst.ops)
			if (!valid_value(operand))
				return fail("invalid direct operand");
		if (!lowir::defines_value(inst.op) || (inst.op == lowir::Opcode::Call && inst.type.is_void()))
			continue;
		if (inst.result < body.parameter_count || inst.result >= temp_count ||
			body.temps[inst.result].def != lowir::kNoId)
			return fail("invalid direct result");
		builder.bind(inst.result, lowir::instruction_result_type(inst, program.pointer_type()), id);
	}
	direct_bodies[symbol] = 2;
	return true;
}
