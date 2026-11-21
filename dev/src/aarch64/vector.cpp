#include "aarch64/selector.h"
#include "aarch64/error.h"

namespace aarch64 {

using namespace lowir;

void Selector::vector_move(unsigned to, unsigned from)
{
	if (to != from)
		out_.emit(Op::VOrr, to, from, from);
}

InstId Selector::vector_combination(const lowir::Instruction& inst)
{
	switch (static_cast<VectorOp>(inst.sub))
	{
	case VectorOp::Fromi32:
		return scalar_splat(inst);
	case VectorOp::Tof32:
	case VectorOp::Tof64:
		return scalar_lane_binary(inst);
	case VectorOp::Fromf32:
	case VectorOp::Fromf64:
	{
		const InstId splat = scalar_splat(inst);
		return splat == kNoId ? combine_float_pair(inst) : splat;
	}
	case VectorOp::Pmullw:
	case VectorOp::Pmulld:
		return packed_multiply_accumulate(inst);
	case VectorOp::Minps:
	case VectorOp::Minpd:
	case VectorOp::Maxps:
	case VectorOp::Maxpd:
		return lane_bound(inst);
	case VectorOp::Punpcklbw:
	case VectorOp::Punpcklwd:
		return vector_extension(inst);
	case VectorOp::Toi64:
		return vector_store(inst);
	case VectorOp::Paddw:
	case VectorOp::Paddd:
		return halving_add(inst);
	case VectorOp::Psrlwi:
	case VectorOp::Psrldi:
	case VectorOp::Psrlqi:
	case VectorOp::Psrawi:
	case VectorOp::Psradi:
		return shift_accumulate(inst);
	case VectorOp::Pshufd:
	{
		const InstId pair = horizontal_double_sum(inst);
		if (pair != kNoId)
			return pair;
		const InstId sum = horizontal_word_sum(inst);
		return sum == kNoId ? packed_lane_multiply(inst) : sum;
	}
	default:
		return kNoId;
	}
}

// Keep the lane order of the scalar addition, including NaN propagation.
InstId Selector::horizontal_double_sum(const lowir::Instruction& inst)
{
	if (!inst.ops[2].is(ValueKind::Integer) || body_.literals[inst.ops[2].index].low != 0x4e || inst.next == kNoId ||
		uses_[inst.result] != 1)
		return kNoId;
	const auto& added = body_.instructions[inst.next];
	if (added.op != Opcode::Vector || added.sub != static_cast<uint8_t>(VectorOp::Addpd) ||
		!(added.ops[0] == inst.ops[0]) || !(added.ops[1] == Value::temp(inst.result)) || added.next == kNoId ||
		uses_[added.result] != 1)
		return kNoId;
	const auto& result = body_.instructions[added.next];
	if (result.op != Opcode::Vector || result.sub != static_cast<uint8_t>(VectorOp::Tof64) ||
		!(result.ops[0] == Value::temp(added.result)))
		return kNoId;
	const unsigned source = vector_value(inst.ops[0], 16);
	const unsigned dest = vectors_[result.result] == kNoId ? 16 : vectors_[result.result];
	out_.emit(Op::VFaddpD, dest, source);
	save_fp(dest, result.result);
	return added.next;
}

// A shuffle/add tree that extracts its low word is an all-lane sum.
InstId Selector::horizontal_word_sum(const lowir::Instruction& first)
{
	const auto matches = [](const lowir::Instruction& inst, VectorOp op) {
		return inst.op == Opcode::Vector && inst.sub == static_cast<unsigned>(op);
	};
	const auto mask_is = [&](const lowir::Instruction& inst, unsigned mask) {
		return inst.ops[2].is(ValueKind::Integer) && body_.literals[inst.ops[2].index].low == mask;
	};
	const auto added = [](const lowir::Instruction& inst, Value a, Value b) {
		return (inst.ops[0] == a && inst.ops[1] == b) || (inst.ops[0] == b && inst.ops[1] == a);
	};
	if (!mask_is(first, 0x4e) || first.next == kNoId || uses_[first.result] != 1)
		return kNoId;
	const auto& pair = body_.instructions[first.next];
	if (!matches(pair, VectorOp::Paddd) || !added(pair, first.ops[0], Value::temp(first.result)) ||
		pair.next == kNoId || uses_[pair.result] != 2)
		return kNoId;
	const auto& swap = body_.instructions[pair.next];
	if (!matches(swap, VectorOp::Pshufd) || !(swap.ops[0] == Value::temp(pair.result)) || !mask_is(swap, 0xb1) ||
		swap.next == kNoId || uses_[swap.result] != 1)
		return kNoId;
	const auto& sum = body_.instructions[swap.next];
	if (!matches(sum, VectorOp::Paddd) || !added(sum, Value::temp(pair.result), Value::temp(swap.result)) ||
		sum.next == kNoId || uses_[sum.result] != 1)
		return kNoId;
	const auto& word = body_.instructions[sum.next];
	if (!matches(word, VectorOp::Toi32) || !(word.ops[0] == Value::temp(sum.result)))
		return kNoId;
	const unsigned source = vector_value(first.ops[0], 16);
	out_.emit(Op::VAddvS, 16, source);
	out_.emit(Op::FmovWS, 9, 16);
	save(9, word.result);
	return sum.next;
}

// MLA overwrites the addend. Fuse the pair when allocation already placed
// the addend and the result in the same register.
InstId Selector::packed_multiply_accumulate(
	const lowir::Instruction& inst, const lowir::Instruction* splat, unsigned lane_field)
{
	if (inst.next == kNoId || uses_[inst.result] != 1)
		return kNoId;
	const auto& next = body_.instructions[inst.next];
	const bool words = static_cast<VectorOp>(inst.sub) == VectorOp::Pmullw;
	if (next.op != Opcode::Vector || static_cast<VectorOp>(next.sub) != (words ? VectorOp::Paddw : VectorOp::Paddd))
		return kNoId;
	const Value product = Value::temp(inst.result);
	const bool right = next.ops[1] == product;
	if (!right && !(next.ops[0] == product))
		return kNoId;
	const Value acc = next.ops[right ? 0 : 1];
	const unsigned dest = vectors_[next.result];
	if (!acc.is(ValueKind::Temp) || dest == kNoId || vectors_[acc.index] != dest)
		return kNoId;
	const bool left_splat = splat && inst.ops[0] == Value::temp(splat->result);
	const unsigned left = vector_value(inst.ops[left_splat ? 1 : 0], 16);
	const unsigned factor = vector_value(splat ? splat->ops[0] : inst.ops[1], 17);
	out_.emit(splat ? Op::VMlaLaneS : Op::VMla, dest, left, factor, splat ? lane_field : (words ? 1u : 2u) << 22);
	return inst.next;
}

// Multiplying by a repeated i32 lane can read that lane from the original
// vector. This also applies when the product immediately feeds an accumulation.
InstId Selector::packed_lane_multiply(const lowir::Instruction& inst)
{
	if (inst.next == kNoId || uses_[inst.result] != 1 || !inst.ops[2].is(ValueKind::Integer))
		return kNoId;
	const uint64_t mask = body_.literals[inst.ops[2].index].low;
	if (mask > 255 || mask % 0x55)
		return kNoId;
	const auto& mul = body_.instructions[inst.next];
	const Value splat = Value::temp(inst.result);
	const bool left = mul.ops[0] == splat;
	if (mul.op != Opcode::Vector || mul.sub != static_cast<unsigned>(VectorOp::Pmulld) ||
		(left == (mul.ops[1] == splat)))
		return kNoId;
	const unsigned lane = static_cast<unsigned>(mask) / 0x55;
	const unsigned field = ((lane & 1) << 21) | ((lane & 2) << 10);
	const InstId combined = packed_multiply_accumulate(mul, &inst, field);
	if (combined != kNoId)
		return combined;
	const unsigned dest = vectors_[mul.result] == kNoId ? 16 : vectors_[mul.result];
	const unsigned input = vector_value(mul.ops[left ? 1 : 0], 16), factor = vector_value(inst.ops[0], 17);
	out_.emit(Op::VMulLaneS, dest, input, factor, field);
	save_vector(dest, mul.result);
	return inst.next;
}

unsigned Selector::vector_value(Value src, unsigned scratch)
{
	if (src.is(ValueKind::Temp) && vectors_[src.index] != kNoId)
		return vectors_[src.index];
	if (src.is(ValueKind::Integer))
	{
		const auto bits = body_.literals[src.index];
		constant(16, bits.low);
		out_.emit(Op::FmovDX, scratch, 16);
		if (bits.high)
		{
			constant(16, bits.high);
			out_.emit(Op::VInsHi, scratch, 16);
		}
	}
	else if (src.is(ValueKind::Temp) && memory_value(body_.temps[src.index].type))
		vector_home(false, scratch, src.index);
	else
	{
		address(16, src);
		out_.emit(Op::LdrQ, scratch, 16);
	}
	return scratch;
}

void Selector::save_vector(unsigned reg, TempId id)
{
	if (vectors_[id] != kNoId)
		vector_move(vectors_[id], reg);
	else
		vector_home(true, reg, id);
}

void Selector::vector_home(bool store, unsigned reg, TempId id)
{
	const Op op = store ? Op::StrQ : Op::LdrQ;
	const uint32_t offset = homes_[id];
	if ((call_result_slots_.empty() || call_result_slots_[id] == kNoId) && offset % 16 == 0 && offset / 16 < 4096)
		out_.emit(op, reg, frame_base(), 0, (offset / 16) << 10);
	else
	{
		home_address(16, id);
		out_.emit(op, reg, 16);
	}
}

void Selector::vector_shift(VectorOp op, unsigned dest, unsigned source, uint64_t count)
{
	const bool left = op < VectorOp::Psrlwi;
	const bool arithmetic = op >= VectorOp::Psrawi;
	const auto first = left ? VectorOp::Psllwi : arithmetic ? VectorOp::Psrawi : VectorOp::Psrlwi;
	const unsigned width = 16u << (unsigned(op) - unsigned(first));
	if (count == 0)
		vector_move(dest, source);
	else if (count >= width && !arithmetic)
		out_.emit(Op::VEor, dest, dest, dest);
	else
	{
		const unsigned shift = left ? width + count : 2 * width - std::min<uint64_t>(count, width);
		out_.emit(left ? Op::VShl : arithmetic ? Op::VSshr : Op::VUshr, dest, source, 0, shift << 16);
	}
}

// Broadcast a scalar directly. MOVI can also encode a single nonzero byte
// at any byte position in each i32 lane. A floating-point broadcast whose
// only reader is the next instruction, a multiply, is FMUL by element.
InstId Selector::scalar_splat(const lowir::Instruction& inst)
{
	if (inst.next == kNoId || uses_[inst.result] != 1)
		return kNoId;
	const auto op = static_cast<VectorOp>(inst.sub);
	const bool wide = op == VectorOp::Fromf64;
	const auto& shuffle = body_.instructions[inst.next];
	if (shuffle.op != Opcode::Vector || shuffle.sub != static_cast<unsigned>(VectorOp::Pshufd) ||
		!(shuffle.ops[0] == Value::temp(inst.result)) || !shuffle.ops[2].is(ValueKind::Integer) ||
		body_.literals[shuffle.ops[2].index].low != (wide ? 0x44u : 0u))
		return kNoId;
	const auto dest = vectors_[shuffle.result] == kNoId ? 16 : vectors_[shuffle.result];
	if (op != VectorOp::Fromi32)
	{
		const unsigned source = fp_operand(inst.ops[0], Type::scalar(wide ? TypeKind::F64 : TypeKind::F32), 16);
		const Value splat = Value::temp(shuffle.result);
		const auto& mul = body_.instructions[shuffle.next == kNoId ? inst.next : shuffle.next];
		if (shuffle.next != kNoId && uses_[shuffle.result] == 1 && mul.op == Opcode::Vector &&
			mul.sub == static_cast<unsigned>(wide ? VectorOp::Mulpd : VectorOp::Mulps) &&
			(mul.ops[0] == splat) != (mul.ops[1] == splat))
		{
			const unsigned input = vector_value(mul.ops[mul.ops[0] == splat ? 1 : 0], 17);
			const unsigned product = vectors_[mul.result] == kNoId ? 16 : vectors_[mul.result];
			out_.emit(Op::VFmulElem, product, input, source, wide ? 1u << 22 : 0);
			save_vector(product, mul.result);
			return shuffle.next;
		}
		out_.emit(wide ? Op::VDupD : Op::VDupS, dest, source);
		save_vector(dest, shuffle.result);
		return inst.next;
	}
	if (inst.ops[0].is(ValueKind::Integer))
	{
		const auto bits = static_cast<uint32_t>(body_.literals[inst.ops[0].index].low);
		for (unsigned byte = 0; byte != 4; ++byte)
			if ((bits & ~(0xffu << (8 * byte))) == 0)
			{
				const unsigned imm = bits >> (8 * byte);
				out_.emit(Op::VMovIS, dest, 0, 0, (1u << 30) | ((imm & 0xe0) << 11) | ((imm & 31) << 5) | (byte << 13));
				save_vector(dest, shuffle.result);
				return inst.next;
			}
	}
	const unsigned reg = operand_register(inst.ops[0], Type::scalar(TypeKind::I32), 9);
	out_.emit(Op::VDupWS, dest, reg);
	save_vector(dest, shuffle.result);
	return inst.next;
}

void Selector::vector_shuffle(const lowir::Instruction& inst, unsigned source)
{
	const unsigned mask = body_.literals[inst.ops[2].index].low;
	// The common splat is one DUP. The remaining shuffles construct each
	// destination lane while leaving the source intact, including when it is v16.
	const auto op = static_cast<VectorOp>(inst.sub);
	if (op == VectorOp::Pshuflw || op == VectorOp::Pshufhw)
	{
		const unsigned first = op == VectorOp::Pshufhw ? 4 : 0;
		vector_move(17, source);
		for (unsigned i = 0; i < 4; ++i)
		{
			const unsigned lane = (mask >> (2 * i)) & 3;
			if (i != lane)
				out_.emit(Op::VInsH, 17, source, 0, ((first + i) << 18) | ((first + lane) << 12));
		}
	}
	else if (((mask & 3) * 0x55) == mask || mask == 0x44 || mask == 0xee)
	{
		const auto dest = vectors_[inst.result] == kNoId ? 17 : vectors_[inst.result];
		if (mask == 0x44 || mask == 0xee)
			out_.emit(Op::VDupD, dest, source, 0, (mask == 0xee ? 1u : 0u) << 20);
		else
			out_.emit(Op::VDupS, dest, source, 0, (mask & 3) << 19);
		save_vector(dest, inst.result);
		return;
	}
	else if (mask == 0x08 || mask == 0x88)
	{
		// Keep the low word of each doubleword. The 0x08 form repeats
		// word zero in the last lane, which is still the same after UZP1.
		const unsigned dest = vectors_[inst.result] == kNoId ? 17 : vectors_[inst.result];
		out_.emit(Op::VUzp1, dest, source, source, 2u << 22);
		if (mask == 0x08)
			out_.emit(Op::VInsS, dest, dest, 0, 3u << 19);
		save_vector(dest, inst.result);
		return;
	}
	else
		for (unsigned i = 0; i < 4; ++i)
			out_.emit(Op::VInsS, 17, source, 0, (i << 19) | (((mask >> (2 * i)) & 3) << 13));
	save_vector(17, inst.result);
}

namespace {
struct Pattern
{
	VectorOp first, last;
	Op op;
	unsigned first_size;
};
const Pattern patterns[] = {
	{ VectorOp::Punpcklbw, VectorOp::Punpcklqdq, Op::VZip1, 0 },
	{ VectorOp::Punpckhbw, VectorOp::Punpckhqdq, Op::VZip2, 0 },
	{ VectorOp::Unpcklps, VectorOp::Unpcklpd, Op::VZip1, 2 },
	{ VectorOp::Unpckhps, VectorOp::Unpckhpd, Op::VZip2, 2 },
	{ VectorOp::Movlhps, VectorOp::Movlhps, Op::VZip1, 3 },
	{ VectorOp::Addps, VectorOp::Addpd, Op::VFadd, 0 },
	{ VectorOp::Subps, VectorOp::Subpd, Op::VFsub, 0 },
	{ VectorOp::Mulps, VectorOp::Mulpd, Op::VFmul, 0 },
	{ VectorOp::Divps, VectorOp::Divpd, Op::VFdiv, 0 },
	{ VectorOp::Sqrtps, VectorOp::Sqrtpd, Op::VFsqrt, 0 },
	{ VectorOp::Paddb, VectorOp::Paddq, Op::VAdd, 0 },
	{ VectorOp::Psubb, VectorOp::Psubq, Op::VSub, 0 },
	{ VectorOp::Pmullw, VectorOp::Pmulld, Op::VMul, 1 },
	{ VectorOp::Pcmpeqb, VectorOp::Pcmpeqq, Op::VCmeq, 0 },
	{ VectorOp::Pcmpgtb, VectorOp::Pcmpgtq, Op::VCmgt, 0 },
	{ VectorOp::Pmaxsb, VectorOp::Pmaxsd, Op::VSmax, 0 },
	{ VectorOp::Pmaxub, VectorOp::Pmaxud, Op::VUmax, 0 },
	{ VectorOp::Pminsb, VectorOp::Pminsd, Op::VSmin, 0 },
	{ VectorOp::Pminub, VectorOp::Pminud, Op::VUmin, 0 },
	{ VectorOp::Paddsb, VectorOp::Paddsw, Op::VSqadd, 0 },
	{ VectorOp::Paddusb, VectorOp::Paddusw, Op::VUqadd, 0 },
	{ VectorOp::Psubsb, VectorOp::Psubsw, Op::VSqsub, 0 },
	{ VectorOp::Psubusb, VectorOp::Psubusw, Op::VUqsub, 0 },
	{ VectorOp::Pavgb, VectorOp::Pavgw, Op::VUrhadd, 0 },
};
}

namespace {
// A literal that is neither NaN nor a zero of either sign. A bound returns
// its second operand when the first is NaN or equal to it, as FMINNM and
// FMAXNM do when that operand is such a number; they differ only for a
// signaling NaN, which C++ code is not assumed to hold (as in GCC and Clang).
bool number_bound(const FunctionBody& body, Value v, bool wide)
{
	if (!v.is(ValueKind::Float))
		return false;
	const uint64_t bits = body.literals[v.index].low;
	const uint64_t magnitude = bits & (wide ? ~(uint64_t(1) << 63) : 0x7fffffffu);
	const uint64_t infinity = wide ? uint64_t(0x7ff) << 52 : 0x7f800000u;
	return magnitude && magnitude <= infinity;
}
}

// Two scalar values can feed a packed doubleword pair or a scalar bound
// directly from their registers. A bound may take them in either order.
InstId Selector::combine_float_pair(const lowir::Instruction& first)
{
	const auto from = static_cast<VectorOp>(first.sub);
	const bool wide = from == VectorOp::Fromf64;
	if ((from != VectorOp::Fromf32 && !wide) || first.next == kNoId || uses_[first.result] != 1)
		return kNoId;
	const auto& second = body_.instructions[first.next];
	if (second.op != Opcode::Vector || second.sub != first.sub || second.next == kNoId || uses_[second.result] != 1)
		return kNoId;
	const auto& merge = body_.instructions[second.next];
	const bool reversed = merge.ops[0] == Value::temp(second.result);
	if (merge.op != Opcode::Vector || !(merge.ops[reversed ? 1 : 0] == Value::temp(first.result)) ||
		!(merge.ops[reversed ? 0 : 1] == Value::temp(second.result)))
		return kNoId;
	const auto op = static_cast<VectorOp>(merge.sub);
	if (wide && op == VectorOp::Unpcklpd && !reversed)
	{
		const Type type = Type::scalar(TypeKind::F64);
		const unsigned lhs = fp_operand(first.ops[0], type, 16), rhs = fp_operand(second.ops[0], type, 17);
		const unsigned dest = vectors_[merge.result] == kNoId ? 16 : vectors_[merge.result];
		out_.emit(Op::VZip1, dest, lhs, rhs, 3u << 22);
		save_vector(dest, merge.result);
		return second.next;
	}
	if (merge.next == kNoId || uses_[merge.result] != 1)
		return kNoId;
	const bool maximum = op == (wide ? VectorOp::Maxpd : VectorOp::Maxps);
	if (!maximum && op != (wide ? VectorOp::Minpd : VectorOp::Minps))
		return kNoId;
	const auto& extract = body_.instructions[merge.next];
	if (extract.op != Opcode::Vector ||
		static_cast<VectorOp>(extract.sub) != (wide ? VectorOp::Tof64 : VectorOp::Tof32) ||
		!(extract.ops[0] == Value::temp(merge.result)))
		return kNoId;
	const Type type = extract.type;
	const auto& left = reversed ? second : first;
	const auto& right = reversed ? first : second;
	const unsigned lhs = fp_operand(left.ops[0], type, 16), rhs = fp_operand(right.ops[0], type, 17);
	const unsigned dest = vectors_[extract.result] == kNoId ? 16 : vectors_[extract.result];
	scalar_bound(maximum, wide, dest, lhs, rhs, number_bound(body_, right.ops[0], wide));
	save_fp(dest, extract.result);
	return merge.next;
}

// A bound is FCMP and FCSEL, which select the second operand when the
// first is NaN or equal to it, or FMINNM or FMAXNM for a second operand
// number_bound accepts.
void Selector::scalar_bound(bool maximum, bool wide, unsigned dest, unsigned lhs, unsigned rhs, bool number)
{
	const unsigned precision = wide ? 1u << 22 : 0;
	if (number)
		out_.emit(maximum ? Op::Fmaxnm : Op::Fminnm, dest, lhs, rhs, precision);
	else
	{
		out_.emit(Op::Fcmp, 0, lhs, rhs, precision);
		out_.emit(Op::Fcsel, dest, lhs, rhs, precision | ((maximum ? 12u : 4u) << 12));
	}
}

// Whether the only reader of a new vector, among the next four instructions
// of its block, is a bound that lane_bound selects, which reads only the
// low lane.
bool Selector::low_lane_only(const lowir::Instruction& made) const
{
	if (uses_[made.result] != 1)
		return false;
	const Value v = Value::temp(made.result);
	InstId id = made.next;
	for (unsigned n = 0; n < 4 && id != kNoId; ++n, id = body_.instructions[id].next)
	{
		const auto& inst = body_.instructions[id];
		if (!(inst.ops[0] == v) && !(inst.ops[1] == v) && !(inst.ops[2] == v))
			continue;
		const auto op = static_cast<VectorOp>(inst.sub);
		const bool wide = op == VectorOp::Minpd || op == VectorOp::Maxpd;
		if (inst.op != Opcode::Vector || (!wide && op != VectorOp::Minps && op != VectorOp::Maxps) ||
			inst.next == kNoId || uses_[inst.result] != 1)
			return false;
		const auto& extract = body_.instructions[inst.next];
		return extract.op == Opcode::Vector &&
			static_cast<VectorOp>(extract.sub) == (wide ? VectorOp::Tof64 : VectorOp::Tof32) &&
			extract.ops[0] == Value::temp(inst.result);
	}
	return false;
}

// The low lane of a minimum or maximum read only by the next extraction of
// that lane is the scalar bound of the operands' low lanes.
InstId Selector::lane_bound(const lowir::Instruction& merge)
{
	const auto op = static_cast<VectorOp>(merge.sub);
	const bool wide = op == VectorOp::Minpd || op == VectorOp::Maxpd;
	if (merge.next == kNoId || uses_[merge.result] != 1)
		return kNoId;
	const auto& extract = body_.instructions[merge.next];
	if (extract.op != Opcode::Vector ||
		static_cast<VectorOp>(extract.sub) != (wide ? VectorOp::Tof64 : VectorOp::Tof32) ||
		!(extract.ops[0] == Value::temp(merge.result)))
		return kNoId;
	const Value second = merge.ops[1];
	const InstId made = second.is(ValueKind::Temp) ? body_.temps[second.index].def : kNoId;
	const auto from = wide ? VectorOp::Fromf64 : VectorOp::Fromf32;
	const bool number = made != kNoId && body_.instructions[made].op == Opcode::Vector &&
		static_cast<VectorOp>(body_.instructions[made].sub) == from &&
		number_bound(body_, body_.instructions[made].ops[0], wide);
	// A vector that only lane bounds read shares its scalar's home (scalar_aliases).
	const auto lane = [&](Value v, unsigned scratch) {
		return v.is(ValueKind::Temp) && !aliases_.empty() && aliases_[v.index] != v.index
			? fp_operand(Value::temp(aliases_[v.index]), extract.type, scratch)
			: vector_value(v, scratch);
	};
	const unsigned lhs = lane(merge.ops[0], 16), rhs = lane(second, 17);
	const unsigned dest = vectors_[extract.result] == kNoId ? 16 : vectors_[extract.result];
	scalar_bound(op == VectorOp::Maxps || op == VectorOp::Maxpd, wide, dest, lhs, rhs, number);
	save_fp(dest, extract.result);
	return merge.next;
}

// Ordered comparison selects the left operand only when strictly better.
// Ties, signed zero and unordered inputs keep the second operand's bits.
void Selector::vector_minmax(VectorOp op, unsigned dest, unsigned left, unsigned right)
{
	const bool maximum = op == VectorOp::Maxps || op == VectorOp::Maxpd;
	const bool wide = op == VectorOp::Minpd || op == VectorOp::Maxpd;
	unsigned mask = dest;
	const auto available = [&](unsigned reg) { return reg != left && reg != right; };
	if (!available(mask))
		mask = available(16) ? 16 : available(17) ? 17 : 0;
	// With two spilled operands and a spilled destination, borrow v0.
	const bool borrowed = !available(dest) && mask == 0;
	if (borrowed)
	{
		out_.emit(Op::FmovXD, 10, 0);
		out_.emit(Op::VUmovDHi, 11, 0);
	}
	out_.emit(Op::VFcmgt, mask, maximum ? left : right, maximum ? right : left, unsigned(wide) << 22);
	if (dest == right)
		out_.emit(Op::VBit, dest, left, mask);
	else if (dest == left)
		out_.emit(Op::VBif, dest, right, mask);
	else
		out_.emit(Op::VBsl, dest, left, right);
	if (borrowed)
	{
		out_.emit(Op::FmovDX, 0, 10);
		out_.emit(Op::VInsHi, 0, 11);
	}
}

void Selector::vector_instruction(const lowir::Instruction& inst)
{
	const auto op = static_cast<VectorOp>(inst.sub);
	const auto shape = shape_of(op);
	const unsigned dest = vectors_[inst.result] == kNoId ? 16 : vectors_[inst.result];
	if (shape == VectorShape::Zero)
		out_.emit(Op::VEor, dest, dest, dest);
	else if (shape == VectorShape::FromScalar || shape == VectorShape::ToScalar)
	{
		const auto type = Type::scalar(vector_scalar_kind(op));
		const bool narrow = storage_bytes(type) == 4;
		if (shape == VectorShape::ToScalar)
		{
			const unsigned source = vector_value(inst.ops[0], 16);
			if (type.is_float())
				save_fp(source, inst.result);
			else
			{
				out_.emit(narrow ? Op::FmovWS : Op::FmovXD, 9, source);
				save(9, inst.result);
			}
			return;
		}
		if (type.is_float())
		{
			// The scalar write clears the other lanes of the new vector, as
			// does each write of a literal or a reload into the destination.
			// A vector read only by a lane bound needs no clear lanes.
			const bool held = inst.ops[0].is(ValueKind::Temp) && vectors_[inst.ops[0].index] != kNoId;
			const unsigned source = fp_operand(inst.ops[0], type, dest);
			if (source != dest || (held && !low_lane_only(inst)))
				out_.emit(Op::FmovFP, dest, source, 0, narrow ? 0 : 1u << 22);
		}
		else
		{
			value(9, inst.ops[0], type);
			out_.emit(narrow ? Op::FmovSW : Op::FmovDX, dest, 9);
		}
	}
	else
	{
		const unsigned left = vector_value(inst.ops[0], 16);
		if (op >= VectorOp::Psllwi && op <= VectorOp::Psradi)
		{
			vector_shift(op, dest, left, body_.literals[inst.ops[2].index].low);
			save_vector(dest, inst.result);
			return;
		}
		if (op == VectorOp::Pshufd || op == VectorOp::Pshuflw || op == VectorOp::Pshufhw)
		{
			vector_shuffle(inst, left);
			return;
		}
		const unsigned right = shape == VectorShape::Binary ? vector_value(inst.ops[1], 17) : 0;
		if (op >= VectorOp::Packsswb && op <= VectorOp::Packusdw)
		{
			const auto narrow = op < VectorOp::Packuswb ? Op::VSqxtn : Op::VSqxtun;
			const unsigned size = op == VectorOp::Packssdw || op == VectorOp::Packusdw ? 1u << 22 : 0;
			// Keep both inputs live until the upper half has been inserted.
			// A vector packed with itself whose only reader takes its low
			// eight bytes needs only the low half.
			const bool low_half = inst.ops[0] == inst.ops[1] && inst.next != kNoId && uses_[inst.result] == 1 &&
				body_.instructions[inst.next].op == Opcode::Vector &&
				body_.instructions[inst.next].sub == static_cast<unsigned>(VectorOp::Toi64) &&
				body_.instructions[inst.next].ops[0] == Value::temp(inst.result);
			const unsigned packed = dest == right && !low_half ? 16 : dest;
			out_.emit(narrow, packed, left, 0, size);
			if (!low_half)
				out_.emit(narrow, packed, right, 0, size | (1u << 30));
			save_vector(packed, inst.result);
			return;
		}
		for (const auto& p : patterns)
			if (op >= p.first && op <= p.last)
			{
				out_.emit(p.op, dest, left, right,
					(p.first_size + static_cast<unsigned>(op) - static_cast<unsigned>(p.first)) << 22);
				save_vector(dest, inst.result);
				return;
			}
		switch (op)
		{
		case VectorOp::Minps:
		case VectorOp::Minpd:
		case VectorOp::Maxps:
		case VectorOp::Maxpd:
			vector_minmax(op, dest, left, right);
			break;
		case VectorOp::Pmaddwd:
		{
			// Sign-extend both halves, then add adjacent pairs modulo 2^32.
			const bool dest_is_input = dest == left || dest == right;
			const unsigned low = dest_is_input ? dest : (dest == 16 ? 17 : 16);
			unsigned high = dest;
			if (dest_is_input)
				high = left != 17 && right != 17 ? 17 : left != 16 && right != 16 ? 16 : 0;
			if (high == 0 && dest_is_input)
			{
				out_.emit(Op::FmovXD, 10, 0);
				out_.emit(Op::VUmovDHi, 11, 0);
			}
			out_.emit(Op::VSmullH, high, left, right, 1u << 30);
			out_.emit(Op::VSmullH, low, left, right);
			out_.emit(Op::VAddpS, dest, low, high);
			if (high == 0 && dest_is_input)
			{
				out_.emit(Op::FmovDX, 0, 10);
				out_.emit(Op::VInsHi, 0, 11);
			}
			break;
		}
		case VectorOp::Cvtdq2pd:
			out_.emit(Op::VSxtl2d, dest, left);
			out_.emit(Op::VScvtf, dest, dest, 0, 1u << 22);
			break;
		case VectorOp::Cvtdq2ps:
			out_.emit(Op::VScvtf, dest, left);
			break;
		case VectorOp::Cvtps2pd:
		case VectorOp::Cvtpd2ps:
			out_.emit(op == VectorOp::Cvtps2pd ? Op::VFcvtl : Op::VFcvtn, dest, left);
			break;
		case VectorOp::Movhlps:
			out_.emit(Op::VZip2, dest, right, left, 3u << 22);
			break;
		case VectorOp::Pand:
		case VectorOp::Andps:
		case VectorOp::Andpd:
			out_.emit(Op::VAnd, dest, left, right);
			break;
		case VectorOp::Pandn:
		case VectorOp::Andnps:
		case VectorOp::Andnpd:
			out_.emit(Op::VBic, dest, right, left);
			break;
		case VectorOp::Por:
		case VectorOp::Orps:
		case VectorOp::Orpd:
			out_.emit(Op::VOrr, dest, left, right);
			break;
		case VectorOp::Pxor:
		case VectorOp::Xorps:
		case VectorOp::Xorpd:
			out_.emit(Op::VEor, dest, left, right);
			break;
		default:
			throw AArch64Error(std::string("unsupported AArch64 vector operation: ") + spelling(op));
		}
	}
	save_vector(dest, inst.result);
}

} // namespace aarch64
