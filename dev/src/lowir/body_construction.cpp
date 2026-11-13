#include "lowir/builder.h"

#include <algorithm>
#include <cstring>

namespace {
template<class T>
void grow(std::vector<T>& values, uint32_t minimum)
{
	if (values.size() < minimum)
		values.resize(std::max(static_cast<uint64_t>(minimum), std::min<uint64_t>(UINT32_MAX, values.size() * 2)));
}

bool valid_type(lowir::Type t)
{
	if (t.kind_bits >= lowir::kTypeKindCount)
		return false;
	if (t.is_object())
		return t.bytes != 0 && t.align_log2 <= 31;
	if (t.is_pointer())
		return t.align_log2 == 0 && (t.bytes == 0 || t.bytes == 4 || t.bytes == 8);
	return t.align_log2 == 0 && t.bytes == 0;
}

// The producer's halfword after float_kind (C's `reserved`), read in place:
// C++'s Value leaves those bytes as padding.
uint16_t reserved_halfword(const lowir::Value& v)
{
	uint16_t bits;
	std::memcpy(&bits, reinterpret_cast<const unsigned char*>(&v) + 2, sizeof bits);
	return bits;
}

bool valid_value(const lowir::Value& v)
{
	return static_cast<unsigned>(v.kind) < lowir::kValueKindCount && reserved_halfword(v) == 0 &&
		(v.kind == lowir::ValueKind::Float
				? v.float_kind >= lowir::TypeKind::F32 && v.float_kind <= lowir::TypeKind::F80
				: v.float_kind == lowir::TypeKind::Void);
}
} // namespace

namespace lowir {

bool FunctionBuilder::construction_fail(const char* message)
{
	construction_error_ = message;
	return false;
}

bool FunctionBuilder::reserve(const BodyCounts& minimum, BodyBuffers& buffers)
{
	if (construction_error_[0] != 0)
		return false;
	if (body_.construction_state == 2)
		return construction_fail("body already committed");
	if (body_.construction_state == 0 && (!body_.instructions.empty() || !body_.blocks.empty()))
		return construction_fail("body already constructed");
	body_.construction_state = 1;
	grow(body_.instructions, minimum.instructions);
	grow(body_.operands, minimum.operands);
	grow(body_.literals, minimum.literals);
	grow(body_.slots, minimum.slots);
	grow(body_.blocks, minimum.blocks);
	grow(body_.layout, static_cast<uint32_t>(body_.blocks.size()));
	buffers.instructions = body_.instructions.data();
	buffers.operands = body_.operands.data();
	buffers.literals = body_.literals.data();
	buffers.slots = body_.slots.data();
	buffers.layout = body_.layout.data();
	buffers.capacity = BodyCounts{ static_cast<uint32_t>(body_.instructions.size()),
		static_cast<uint32_t>(body_.operands.size()), static_cast<uint32_t>(body_.literals.size()),
		static_cast<uint32_t>(body_.slots.size()), static_cast<uint32_t>(body_.blocks.size()) };
	return true;
}

bool FunctionBuilder::commit(const BodyCounts& counts, uint32_t temp_count)
{
	if (construction_error_[0] != 0)
		return false;
	if (body_.construction_state != 1)
		return construction_fail("no open body");
	body_.construction_state = 2;
	if (counts.instructions > body_.instructions.size() || counts.operands > body_.operands.size() ||
		counts.literals > body_.literals.size() || counts.slots > body_.slots.size() ||
		counts.blocks > body_.blocks.size())
		return construction_fail("direct body exceeds reserved storage");
	body_.instructions.resize(counts.instructions);
	body_.operands.resize(counts.operands);
	body_.literals.resize(counts.literals);
	body_.slots.resize(counts.slots);
	body_.blocks.resize(counts.blocks);
	body_.layout.resize(counts.blocks);
	if (temp_count < body_.parameter_count)
		return construction_fail("direct body omits parameters");
	body_.temps.resize(temp_count, lowir::Temp{ 0, lowir::Type::scalar(lowir::TypeKind::Void), lowir::kNoId });
	for (const lowir::Slot& slot : body_.slots)
		if (!valid_type(slot.type) || slot.type.is_void() || slot.name != 0 || slot.kind != lowir::SlotKind::Plain ||
			slot.reserved != 0 || slot.index != 0)
			return construction_fail("invalid direct slot");
	for (lowir::Block& block : body_.blocks)
		block = lowir::Block{ 0, lowir::kNoId, lowir::kNoId, lowir::BlockKind::Plain };
	for (const lowir::Value& operand : body_.operands)
		if (!valid_value(operand))
			return construction_fail("invalid direct operand");
	for (lowir::InstId id = 0; id < body_.instructions.size(); ++id)
	{
		lowir::Instruction& inst = body_.instructions[id];
		if (static_cast<unsigned>(inst.op) >= lowir::kOpcodeCount || !valid_type(inst.type) ||
			inst.location >= counts.blocks ||
			(inst.op == lowir::Opcode::Convert && !valid_type(lowir::convert_source_type(inst))))
			return construction_fail("invalid direct instruction");
		lowir::Block& block = body_.blocks[inst.location];
		inst.location = 0;
		inst.next = lowir::kNoId;
		if (block.last == lowir::kNoId)
			block.first = id;
		else
			body_.instructions[block.last].next = id;
		block.last = id;
		for (const lowir::Value& operand : inst.ops)
			if (!valid_value(operand))
				return construction_fail("invalid direct operand");
		if (!lowir::defines_value(inst.op) || (inst.op == lowir::Opcode::Call && inst.type.is_void()))
			continue;
		if (inst.result < body_.parameter_count || inst.result >= temp_count ||
			body_.temps[inst.result].def != lowir::kNoId)
			return construction_fail("invalid direct result");
		bind(inst.result, lowir::instruction_result_type(inst, program_.pointer_type()), id);
	}
	return true;
}

} // namespace lowir
