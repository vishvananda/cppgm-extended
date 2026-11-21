#include "machine/constant_division.h"
#include "machine/exact_division.h"
#include "aarch64/selector.h"
#include "aarch64/logical_immediate.h"

#include "machine/switch_plan.h"
#include "opt/scalar_fold.h"
#include <algorithm>
#include <climits>

namespace aarch64 {

using namespace lowir;

unsigned Selector::operand_register(Value operand, Type type, unsigned scratch)
{
	if (operand.is(ValueKind::Temp) && registers_[operand.index] != kNoId)
		return registers_[operand.index];
	value(scratch, operand, type);
	return scratch;
}

unsigned Selector::condition_register(Value operand, unsigned scratch)
{
	const Type type = type_of(operand, Type::scalar(TypeKind::I64));
	if (type.kind() != TypeKind::I128)
		return operand_register(operand, type, scratch);
	pair(operand, scratch, 15);
	out_.emit(Op::Orr, scratch, scratch, 15);
	return scratch;
}

// A 32-bit condition is tested in its W register (narrow.cpp).
bool Selector::word_condition(Value operand) const
{
	return integer_bits(type_of(operand, Type::scalar(TypeKind::I64)).kind()) == 32;
}

// Insert the surviving low bits into their shifted position and clear the rest
// of the result. A W bitfield operation also clears the top half of the X home.
void Selector::narrow_left(Value src, Type type, TempId result, unsigned width, unsigned count)
{
	if (count >= width)
	{
		save(31, result, true);
		return;
	}
	const unsigned dest = registers_[result] == kNoId ? 9 : registers_[result];
	out_.emit(Op::UbfmW, dest, operand_register(src, type, 10), 0, ((-count & 31) << 16) | ((width - count - 1) << 10));
	save(dest, result, true);
}

// A known multiple needs no rounding. Remove the divisor's powers of two,
// then multiply by its odd part's modular inverse, as on x86-64.
bool Selector::constant_quotient(
	const lowir::Instruction& inst, uint64_t divisor, unsigned bits, unsigned lhs, unsigned dst)
{
	const auto op = static_cast<BinaryOp>(inst.sub);
	const bool sign = op == BinaryOp::Div;
	if ((!sign && op != BinaryOp::UDiv) || (bits != 32 && bits != 64))
		return false;
	const bool w = bits == 32;
	if (w)
		divisor = static_cast<uint32_t>(divisor);
	if (!divisor)
		return false;
	const bool negative = sign && (divisor >> (bits - 1));
	const uint64_t magnitude = negative ? (w ? static_cast<uint32_t>(0 - divisor) : 0 - divisor) : divisor;
	const unsigned shift = __builtin_ctzll(magnitude);
	const uint64_t odd = magnitude >> shift;
	const bool exact = inst.flags & IF_EXACT;
	if (odd != 1 && !exact)
		return false;
	unsigned from = lhs;
	if (shift)
	{
		if (sign && !exact)
		{
			out_.emit(w ? Op::SbfmW : Op::Sbfm, 9, lhs, 0, ((bits - 1) << 16) | ((bits - 1) << 10));
			out_.emit(w ? Op::AddW : Op::Add, 9, lhs, 9, (1u << 22) | ((bits - shift) << 10));
			from = 9;
		}
		out_.emit(sign ? (w ? Op::SbfmW : Op::Sbfm) : (w ? Op::UbfmW : Op::Ubfm), dst, from, 0,
			(shift << 16) | ((bits - 1) << 10));
		from = dst;
	}
	if (odd != 1)
	{
		uint64_t inverse = machine::odd_inverse(odd);
		if (negative)
			inverse = 0 - inverse;
		constant(11, w ? static_cast<uint32_t>(inverse) : inverse);
		out_.emit(w ? Op::MulW : Op::Mul, dst, from, 11);
	}
	else if (negative)
		out_.emit(w ? Op::SubW : Op::Sub, dst, 31, from);
	else if (from != dst)
		out_.emit(w ? Op::OrrW : Op::Orr, dst, 31, from);
	save(dst, inst.result, true);
	return true;
}

namespace {
// The multiply that replaces a division or remainder by a 32-bit or 64-bit
// literal other than 0, 1 and the powers of two (a 32-bit one at most
// INT32_MAX, a 64-bit signed one not -1), as on x86-64:
// - 32 bits: the shared word plan in a widening multiply, its multiplier
//   below 2^32; a signed multiplier above INT32_MAX is a negative word, so
//   the dividend times 2^32 is added to the product;
// - 64 bits: the high half of the product (SMULH, UMULH) by Hacker's
//   Delight's multiplier, with the dividend's correction when signed, and
//   with the 65-bit fix-up or the even divisor's zeros shifted out of the
//   dividend first when unsigned.
// A signed quotient adds its sign bit (truncation toward zero); a
// remainder is the dividend minus the quotient times the divisor.
struct DivisionPlan
{
	uint64_t multiplier;
	unsigned shift, zeros;
	bool sign, remainder, add;
};

bool division_plan(BinaryOp op, uint64_t raw, unsigned bits, DivisionPlan& plan)
{
	plan.sign = op == BinaryOp::Div || op == BinaryOp::Mod;
	plan.remainder = op == BinaryOp::Mod || op == BinaryOp::UMod;
	plan.zeros = 0;
	plan.add = false;
	if (!plan.sign && op != BinaryOp::UDiv && op != BinaryOp::UMod)
		return false;
	if (bits == 32)
	{
		const auto divisor = static_cast<uint32_t>(raw);
		if (divisor <= 1 || divisor > INT32_MAX || (divisor & (divisor - 1)) == 0)
			return false;
		const auto magic = machine::word_magic(divisor, plan.sign ? 31 : 32);
		plan.multiplier = magic.multiplier;
		plan.shift = magic.shift;
		plan.add = plan.sign && magic.multiplier > INT32_MAX;
		return magic.multiplier <= UINT32_MAX;
	}
	if (bits != 64)
		return false;
	const uint64_t magnitude = plan.sign && static_cast<int64_t>(raw) < 0 ? 0 - raw : raw;
	if (magnitude <= 2 || (magnitude & (magnitude - 1)) == 0)
		return false;
	if (plan.sign)
	{
		const auto magic = machine::signed_magic(static_cast<int64_t>(raw));
		plan.multiplier = static_cast<uint64_t>(magic.multiplier);
		plan.shift = magic.shift;
		return true;
	}
	auto magic = machine::unsigned_magic(raw);
	if (magic.add && !(raw & 1))
	{
		plan.zeros = __builtin_ctzll(raw);
		magic = machine::narrow_unsigned_magic(raw >> plan.zeros, 64 - plan.zeros);
	}
	plan.multiplier = magic.multiplier;
	plan.shift = magic.shift;
	plan.add = magic.add;
	return true;
}
}

bool division_constants(
	const lowir::Instruction& inst, const FunctionBody& body, uint64_t& multiplier, uint64_t& divisor)
{
	const auto op = static_cast<BinaryOp>(inst.sub);
	DivisionPlan plan;
	if (inst.op != Opcode::Binary || !inst.type.is_integer() || !inst.ops[1].is(ValueKind::Integer) ||
		((op == BinaryOp::Div || op == BinaryOp::UDiv) && (inst.flags & IF_EXACT)))
		return false;
	const unsigned bits = integer_bits(inst.type.kind());
	const uint64_t raw = body.literals[inst.ops[1].index].low;
	if (!division_plan(op, raw, bits, plan))
		return false;
	multiplier = plan.multiplier;
	divisor = plan.remainder ? (bits == 32 ? static_cast<uint32_t>(raw) : raw) : 0;
	return true;
}

// A constant held through the loop (hold_division_constants) is read from
// its register.
bool Selector::constant_division(
	const lowir::Instruction& inst, uint64_t raw, unsigned bits, unsigned lhs, unsigned dst)
{
	DivisionPlan plan;
	if (!division_plan(static_cast<BinaryOp>(inst.sub), raw, bits, plan))
		return false;
	// (A combination can select a rewritten copy of the instruction.)
	const bool listed = &inst >= body_.instructions.data() &&
		&inst < body_.instructions.data() + body_.instructions.size();
	const auto id = listed ? static_cast<InstId>(&inst - body_.instructions.data()) : kNoId;
	const auto held = std::lower_bound(held_divisions_.begin(), held_divisions_.end(), std::make_pair(id, 0u));
	const uint32_t regs = held != held_divisions_.end() && held->first == id ? held->second : ~0u;
	const unsigned multiplier = regs & 255, divisor_reg = (regs >> 8) & 255;
	const unsigned quotient = plan.remainder ? 9 : dst, factor = multiplier == 255 ? 11 : multiplier;
	if (multiplier == 255)
		constant(11, plan.multiplier);
	if (bits == 32)
	{
		out_.emit(plan.sign ? Op::Smull : Op::Umull, 9, lhs, factor);
		if (plan.add)
			out_.emit(Op::Add, 9, 9, lhs, 32u << 10);
		out_.emit(plan.sign ? Op::Sbfm : Op::Ubfm, plan.sign ? 9 : quotient, 9, 0, (plan.shift << 16) | (63u << 10));
		if (plan.sign)
			out_.emit(Op::AddW, quotient, 9, lhs, (1u << 22) | (31u << 10));
	}
	else if (plan.sign)
	{
		out_.emit(Op::Smulh, 9, lhs, factor);
		const bool positive = static_cast<int64_t>(raw) > 0;
		if (positive != (static_cast<int64_t>(plan.multiplier) > 0))
			out_.emit(positive ? Op::Add : Op::Sub, 9, 9, lhs);
		if (plan.shift)
			out_.emit(Op::Sbfm, 9, 9, 0, (plan.shift << 16) | (63u << 10));
		out_.emit(Op::Add, quotient, 9, 9, (1u << 22) | (63u << 10));
	}
	else
	{
		if (plan.zeros)
			out_.emit(Op::Ubfm, 9, lhs, 0, (plan.zeros << 16) | (63u << 10));
		out_.emit(Op::Umulh, 9, plan.zeros ? 9 : lhs, factor);
		unsigned shift = plan.shift;
		if (plan.add)
		{
			// ((x - t) >> 1) + t, then the remaining shift.
			out_.emit(Op::Sub, 11, lhs, 9);
			out_.emit(Op::Add, 9, 9, 11, (1u << 22) | (1u << 10));
			--shift;
		}
		if (shift)
			out_.emit(Op::Ubfm, quotient, 9, 0, (shift << 16) | (63u << 10));
		else if (quotient != 9)
			out_.emit(Op::Orr, quotient, 31, 9);
	}
	if (plan.remainder)
	{
		if (divisor_reg == 255)
			constant(11, bits == 32 ? static_cast<uint32_t>(raw) : raw);
		out_.emit(bits == 32 ? Op::MsubW : Op::Msub, dst, quotient, divisor_reg == 255 ? 11 : divisor_reg, lhs << 10);
	}
	save(dst, inst.result, true);
	return true;
}

// The constants held through the loops that `block` enters, built at its start.
void Selector::build_held_constants(BlockId block)
{
	for (const auto& held : held_constants_)
		if (held.block == block)
			constant(held.reg, held.value);
}

bool Selector::simple_binary(const lowir::Instruction& inst)
{
	const auto op = static_cast<BinaryOp>(inst.sub);
	const unsigned bits = inst.type.is_pointer() ? 64 : integer_bits(inst.type.kind());
	const bool logical = op == BinaryOp::And || op == BinaryOp::Or || op == BinaryOp::Xor;
	const bool small = bits == 1 || bits == 8 || bits == 16;
	if (small && (op == BinaryOp::Shl || op == BinaryOp::UShr) && inst.ops[1].is(ValueKind::Integer))
	{
		const auto count = body_.literals[inst.ops[1].index].low;
		if (count >= bits)
			return false;
		if (op == BinaryOp::Shl)
			narrow_left(inst.ops[0], inst.type, inst.result, bits, count);
		else
		{
			const unsigned dest = registers_[inst.result] == kNoId ? 9 : registers_[inst.result];
			out_.emit(
				Op::UbfmW, dest, operand_register(inst.ops[0], inst.type, 10), 0, (count << 16) | ((bits - 1) << 10));
			save(dest, inst.result, true);
		}
		return true;
	}
	if (bits != 32 && bits != 64 && !(small && (logical || op == BinaryOp::Add || op == BinaryOp::Sub)))
		return false;
	Value lhs = inst.ops[0], rhs = inst.ops[1];
	if ((op == BinaryOp::Add || op == BinaryOp::Mul || op == BinaryOp::And || op == BinaryOp::Or ||
			op == BinaryOp::Xor) &&
		lhs.is(ValueKind::Integer))
		std::swap(lhs, rhs);
	const unsigned left = operand_register(lhs, inst.type, 10);
	const unsigned dest = registers_[inst.result] == kNoId ? 9 : registers_[inst.result];
	const bool w = bits <= 32;
	// For AND, OR and XOR, canonical inputs guarantee canonical high bits.
	const auto canonical = [&](Value v) {
		return v.is(ValueKind::Integer) ||
			(v.is(ValueKind::Temp) && body_.temps[v.index].type == inst.type && !loose(v.index));
	};
	const bool normalized = bits >= 32 || (logical && canonical(lhs) && canonical(rhs));
	if ((op == BinaryOp::Add || op == BinaryOp::Sub) && rhs.is(ValueKind::Integer))
	{
		const uint64_t raw = body_.literals[rhs.index].low;
		const uint64_t sign = uint64_t(1) << (bits - 1);
		const uint64_t value = small ? ((raw & (2 * sign - 1)) ^ sign) - sign : raw;
		const int64_t signed_value = w ? static_cast<int32_t>(value) : static_cast<int64_t>(value);
		const uint64_t magnitude = signed_value < 0 ? 0 - uint64_t(signed_value) : uint64_t(signed_value);
		const unsigned shift = (magnitude & 4095) == 0 ? 12 : 0;
		if ((magnitude >> shift) <= 4095)
		{
			const bool sub = (op == BinaryOp::Sub) != (signed_value < 0);
			out_.emit(sub ? (w ? Op::SubIW : Op::SubI) : (w ? Op::AddIW : Op::AddI), dest, left, 0,
				(shift ? 1u << 22 : 0) | uint32_t(magnitude >> shift) << 10);
			save(dest, inst.result, normalized);
			return true;
		}
	}
	if (rhs.is(ValueKind::Integer))
	{
		const uint64_t raw = body_.literals[rhs.index].low;
		// A coefficient of 1 + 2^k needs just ADD with a shifted register.
		const uint64_t factor = w ? static_cast<uint32_t>(raw) : raw;
		if ((op == BinaryOp::Div || op == BinaryOp::UDiv) && constant_quotient(inst, raw, bits, left, dest))
			return true;
		if (level_ >= 3 && !inst.type.is_pointer() &&
			(op == BinaryOp::Div || op == BinaryOp::Mod || op == BinaryOp::UDiv || op == BinaryOp::UMod) &&
			constant_division(inst, raw, bits, left, dest))
			return true;
		if (op == BinaryOp::Mul && factor > 2 && ((factor - 1) & (factor - 2)) == 0)
		{
			out_.emit(w ? Op::AddW : Op::Add, dest, left, left, __builtin_ctzll(factor - 1) << 10);
			save(dest, inst.result, true);
			return true;
		}
		const bool scale = op == BinaryOp::Mul && raw && !(raw & (raw - 1)) && (!w || raw <= UINT32_MAX);
		if (scale || op == BinaryOp::Shl || op == BinaryOp::Shr || op == BinaryOp::UShr)
		{
			const unsigned shift = scale ? __builtin_ctzll(raw) : raw & (bits - 1);
			const unsigned r = (scale || op == BinaryOp::Shl) ? (-shift & (bits - 1)) : shift;
			const unsigned end = (scale || op == BinaryOp::Shl) ? bits - 1 - shift : bits - 1;
			out_.emit(op == BinaryOp::Shr ? (w ? Op::SbfmW : Op::Sbfm) : (w ? Op::UbfmW : Op::Ubfm), dest, left, 0,
				(r << 16) | (end << 10));
			save(dest, inst.result, true);
			return true;
		}
		uint32_t field = 0;
		const uint64_t mask = bits < 32 ? raw & ((1u << bits) - 1) : raw;
		if (logical && logical_immediate(mask, w ? 32 : 64, field))
		{
			const Op selected = op == BinaryOp::And ? (w ? Op::AndImmW : Op::AndImm)
				: op == BinaryOp::Or                ? (w ? Op::OrrImmW : Op::OrrImm)
													: (w ? Op::EorImmW : Op::EorImm);
			out_.emit(selected, dest, left, 0, field);
			save(dest, inst.result, normalized);
			return true;
		}
	}
	const unsigned right = operand_register(rhs, inst.type, 11);
	static const Op narrow[] = { Op::AddW, Op::SubW, Op::MulW, Op::SDivW, Op::SDivW, Op::UDivW, Op::UDivW, Op::AndW,
		Op::OrrW, Op::EorW, Op::LslW, Op::AsrW, Op::LsrW };
	static const Op wide[] = { Op::Add, Op::Sub, Op::Mul, Op::SDiv, Op::SDiv, Op::UDiv, Op::UDiv, Op::And, Op::Orr,
		Op::Eor, Op::Lsl, Op::Asr, Op::Lsr };
	const bool remainder = op == BinaryOp::Mod || op == BinaryOp::UMod;
	out_.emit(w ? narrow[inst.sub] : wide[inst.sub], remainder ? 9 : dest, left, right);
	if (remainder)
		out_.emit(w ? Op::MsubW : Op::Msub, dest, 9, right, left << 10);
	save(dest, inst.result, normalized);
	return true;
}

namespace {
const unsigned conditions[] = { 0, 1, 11, 13, 12, 10, 3, 9, 8, 2 };
}

unsigned integer_condition(unsigned predicate)
{
	return conditions[predicate];
}

void Selector::compare_flags(const lowir::Instruction& inst)
{
	const unsigned bits = integer_bits(inst.type.kind());
	if (level_ && (bits == 8 || bits == 16 || bits == 32 || bits == 64 || inst.type.is_pointer()))
	{
		const bool w = bits && bits <= 32;
		const auto pred = static_cast<ComparePredicate>(inst.sub);
		const bool sign = w && bits > 1 && bits < 32 && pred >= ComparePredicate::Lt && pred <= ComparePredicate::Ge;
		unsigned lhs = operand_register(inst.ops[0], inst.type, 10);
		if (sign)
		{
			out_.emit(Op::SbfmW, 10, lhs, 0, (bits - 1) << 10);
			lhs = 10;
		}
		if (inst.ops[1].is(ValueKind::Integer))
		{
			uint64_t raw = body_.literals[inst.ops[1].index].low;
			if (w && bits < 32)
			{
				const uint64_t top = uint64_t(1) << (bits - 1);
				raw &= 2 * top - 1;
				if (sign)
					raw = (raw ^ top) - top;
			}
			const int64_t value = w ? static_cast<int32_t>(raw) : static_cast<int64_t>(raw);
			const uint64_t magnitude = value < 0 ? 0 - uint64_t(value) : uint64_t(value);
			const unsigned shift = (magnitude & 4095) == 0 ? 12 : 0;
			if ((magnitude >> shift) <= 4095)
			{
				out_.emit(value < 0 ? (w ? Op::AddsIW : Op::AddsIX) : (w ? Op::SubsIW : Op::SubsIX), 31, lhs, 0,
					(shift ? 1u << 22 : 0) | uint32_t(magnitude >> shift) << 10);
				return;
			}
		}
		const unsigned rhs = operand_register(inst.ops[1], inst.type, 11);
		if (sign)
			out_.emit(Op::SubsW, 31, lhs, rhs, (1u << 21) | ((bits == 16 ? 5u : 4u) << 13));
		else
			out_.emit(w ? Op::SubsW : Op::Subs, 31, lhs, rhs);
		return;
	}
	const auto pred = static_cast<ComparePredicate>(inst.sub);
	const bool sign = pred >= ComparePredicate::Lt && pred <= ComparePredicate::Ge;
	value(10, inst.ops[0], inst.type);
	value(11, inst.ops[1], inst.type);
	extend(10, bits, sign);
	extend(11, bits, sign);
	out_.emit(Op::Subs, 31, 10, 11);
}

void Selector::compare(const lowir::Instruction& inst, bool flags_ready)
{
	if (!flags_ready)
		compare_flags(inst);
	set_condition(inst.result, conditions[inst.sub]);
}

void Selector::set_condition(TempId result, unsigned condition)
{
	const unsigned dest = registers_[result] == kNoId ? 9 : registers_[result];
	out_.emit(Op::CsincW, dest, 31, 31, (condition ^ 1) << 12);
	save(dest, result, true);
}

// An increment of a word or doubleword read only by a selection between it
// and the value it increments is not computed where it is defined: the
// selection is CSINC of the value with itself (choose).
void Selector::find_chosen_increments()
{
	increments_.clear();
	if (!level_)
		return;
	for (const auto& block : body_.blocks)
		for (InstId i = block.first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& select = body_.instructions[i];
			const unsigned bits = integer_bits(select.type.kind());
			if (select.op != Opcode::Select || (bits != 32 && bits != 64))
				continue;
			for (unsigned k = 1; k <= 2; ++k)
			{
				const Value up = select.ops[k], same = select.ops[3 - k];
				if (!up.is(ValueKind::Temp) || !same.is(ValueKind::Temp) || up == same || uses_[up.index] != 1 ||
					rematerialized(up.index) || body_.temps[up.index].def == kNoId)
					continue;
				const auto& add = body_.instructions[body_.temps[up.index].def];
				if (add.op != Opcode::Binary || add.sub != static_cast<unsigned>(BinaryOp::Add) ||
					add.type != select.type)
					continue;
				const unsigned j = add.ops[0] == same ? 1 : add.ops[1] == same ? 0 : 2;
				if (j == 2 || !add.ops[j].is(ValueKind::Integer) || body_.literals[add.ops[j].index].low != 1)
					continue;
				if (increments_.empty())
					increments_.assign(body_.temps.size(), 0);
				increments_[up.index] = 1;
				break;
			}
		}
}

void Selector::choose(const lowir::Instruction& inst, unsigned condition)
{
	if (inst.type.is_float())
	{
		float_choice(inst, condition);
		return;
	}
	Value yes = inst.ops[1], no = inst.ops[2];
	const unsigned dest = registers_[inst.result] == kNoId ? 9 : registers_[inst.result];
	const unsigned bits = integer_bits(inst.type.kind());
	const bool up_yes = chosen_increment(yes);
	if (up_yes || chosen_increment(no))
	{
		const unsigned source = zero_or_register(up_yes ? no : yes, inst.type, 10);
		out_.emit(bits == 32 ? Op::CsincW : Op::Csinc, dest, source, source, (condition ^ up_yes) << 12);
		save(dest, inst.result, true);
		return;
	}
	auto one = [&](Value v) { return bits && v.is(ValueKind::Integer) && body_.literals[v.index].low == 1; };
	if (one(yes))
	{
		std::swap(yes, no);
		condition ^= 1;
	}
	// Both CSEL and CSINC read register 31 as zero. For a one-valued
	// alternative, CSINC constructs the one only on the selected arm.
	const unsigned a = zero_or_register(yes, inst.type, 10);
	if (one(no))
		out_.emit(bits <= 32 ? Op::CsincW : Op::Csinc, dest, a, 31, condition << 12);
	else
		out_.emit(bits == 32 ? Op::CselW : Op::Csel, dest, a, zero_or_register(no, inst.type, 11), condition << 12);
	auto normalized = [&](Value v) {
		return v.is(ValueKind::Integer) || (v.is(ValueKind::Temp) && body_.temps[v.index].type == inst.type);
	};
	save(dest, inst.result, !bits || bits >= 32 || (normalized(yes) && normalized(no)));
}

// Record the two-instruction form until block placement establishes whether
// the bit-test branch can reach its destination.
void Selector::conditional_bit_edges(
	BlockId block, const lowir::Instruction& branch, Op test, unsigned source, unsigned bit, unsigned width)
{
	uint32_t field = 0;
	logical_immediate(uint64_t(1) << bit, width <= 32 ? 32 : 64, field);
	const uint32_t pc = out_.code.size();
	out_.emit(width <= 32 ? Op::AndImmW : Op::AndImm, 9, source, 0, field);
	conditional_edges(block, branch, test, 9);
	bit_tests_.push_back({ pc, bit });
}

InstId Selector::bit_branch(const lowir::Instruction& inst, BlockId block, const lowir::Instruction* shift)
{
	const unsigned width = integer_bits(inst.type.kind());
	if (inst.next == kNoId || uses_[inst.result] != 1 || !width || width > 64)
		return kNoId;
	InstId edge = inst.next;
	Value condition = Value::temp(inst.result);
	Op test_zero = Op::Cbz;
	const auto& test = body_.instructions[edge];
	if (test.op == Opcode::Compare && test.type == inst.type && test.ops[0] == condition &&
		test.ops[1].is(ValueKind::Integer) && body_.literals[test.ops[1].index].low == 0 &&
		(test.sub == static_cast<unsigned>(ComparePredicate::Ne) ||
			test.sub == static_cast<unsigned>(ComparePredicate::Eq)) &&
		uses_[test.result] == 1 && test.next != kNoId)
	{
		test_zero = test.sub == static_cast<unsigned>(ComparePredicate::Eq) ? Op::Cbnz : Op::Cbz;
		condition = Value::temp(test.result);
		edge = test.next;
	}
	const auto& branch = body_.instructions[edge];
	if (branch.op != Opcode::Branch || !(branch.ops[0] == condition))
		return kNoId;
	Value operand = inst.ops[0], mask = inst.ops[1];
	unsigned src, bit;
	if (shift)
	{
		// Test bit zero of word >> index. W and X shifts mask the count
		// identically to the original 1 << index at their respective widths.
		if (operand == Value::temp(shift->result))
			std::swap(operand, mask);
		if (!(mask == Value::temp(shift->result)) || operand == mask || !operand.is(ValueKind::Temp))
			return kNoId;
		const unsigned word = operand_register(operand, inst.type, 10);
		const unsigned index = operand_register(shift->ops[1], shift->type, 11);
		out_.emit(width == 32 ? Op::LsrW : Op::Lsr, 9, word, index);
		src = 9;
		bit = 0;
	}
	else
	{
		if (operand.is(ValueKind::Integer))
			std::swap(operand, mask);
		if (!mask.is(ValueKind::Integer))
			return kNoId;
		const uint64_t value = body_.literals[mask.index].low & (~uint64_t(0) >> (64 - width));
		if (!value || (value & (value - 1)))
			return kNoId;
		bit = __builtin_ctzll(value);
		src = operand_register(operand, inst.type, 10);
	}
	conditional_bit_edges(block, branch, test_zero, src, bit, width);
	return edge;
}

// Test a narrow modular interval using one W subtraction and a compare.
// Underflow produces a large unsigned value in either width when the
// comparison threshold is strictly below the smallest wrapped result.
InstId Selector::narrow_range_branch(const lowir::Instruction& sub, BlockId block)
{
	const unsigned width = integer_bits(sub.type.kind());
	if ((width != 8 && width != 16) || sub.sub != static_cast<unsigned>(BinaryOp::Sub) || sub.next == kNoId ||
		uses_[sub.result] != 1 || !sub.ops[0].is(ValueKind::Temp) || body_.temps[sub.ops[0].index].type != sub.type ||
		!sub.ops[1].is(ValueKind::Integer) || loose(sub.ops[0].index))
		return kNoId;
	const auto& cmp = body_.instructions[sub.next];
	if (cmp.op != Opcode::Compare || cmp.next == kNoId || cmp.type != sub.type || uses_[cmp.result] != 1 ||
		!(cmp.ops[0] == Value::temp(sub.result)) || !cmp.ops[1].is(ValueKind::Integer))
		return kNoId;
	const auto pred = static_cast<ComparePredicate>(cmp.sub);
	const auto& branch = body_.instructions[cmp.next];
	if ((pred >= ComparePredicate::Lt && pred <= ComparePredicate::Ge) || branch.op != Opcode::Branch ||
		!(branch.ops[0] == Value::temp(cmp.result)))
		return kNoId;
	const unsigned limit = 1u << width;
	const uint32_t base = body_.literals[sub.ops[1].index].low & (limit - 1);
	const uint32_t threshold = body_.literals[cmp.ops[1].index].low & (limit - 1);
	if (threshold >= limit - base || base > 4095 || threshold > 4095)
		return kNoId;
	const auto src = operand_register(sub.ops[0], sub.type, 10);
	out_.emit(Op::SubIW, 9, src, 0, base << 10);
	out_.emit(Op::SubsIW, 31, 9, 0, threshold << 10);
	conditional_edges(block, branch, Op::BCond, conditions[cmp.sub] ^ 1);
	return cmp.next;
}

// The frontend may promote comparison results to int before a bitwise AND/OR.
// A truncated comparison is still exactly zero or one.
namespace {
InstId predicate_join(const FunctionBody& body, const std::vector<uint32_t>& uses, const lowir::Instruction& first,
	const lowir::Instruction& second)
{
	Value a = Value::temp(first.result), b = Value::temp(second.result);
	InstId id = second.next;
	for (unsigned n = 0; n < 2 && id != kNoId; ++n)
	{
		const auto& cast = body.instructions[id];
		if (cast.op != Opcode::Convert || cast.sub != static_cast<unsigned>(ConvertOp::Trunc) ||
			!cast.type.is_integer() || uses[cast.result] != 1)
			break;
		if (cast.ops[0] == a)
			a = Value::temp(cast.result);
		else if (cast.ops[0] == b)
			b = Value::temp(cast.result);
		else
			return kNoId;
		id = cast.next;
	}
	if (id == kNoId)
		return kNoId;
	const auto& join = body.instructions[id];
	if (join.op != Opcode::Binary ||
		(join.sub != static_cast<unsigned>(BinaryOp::Or) && join.sub != static_cast<unsigned>(BinaryOp::And)) ||
		!((join.ops[0] == a && join.ops[1] == b) || (join.ops[1] == a && join.ops[0] == b)))
		return kNoId;
	return id;
}
}

// CCMP keeps both predicates in flags. If the first decides the combined
// test, the fallback NZCV makes the second condition report that result.
InstId Selector::compare_pair(const lowir::Instruction& first, BlockId block)
{
	if (first.next == kNoId || uses_[first.result] != 1 || !(first.type.is_integer() || first.type.is_pointer()) ||
		integer_bits(first.type.kind()) > 64 || first.ops[0].is(ValueKind::Symbol) ||
		first.ops[1].is(ValueKind::Symbol))
		return kNoId;
	const auto& second = body_.instructions[first.next];
	if (second.op != Opcode::Compare || second.next == kNoId || uses_[second.result] != 1 ||
		!second.ops[0].is(ValueKind::Temp) ||
		!(second.ops[1].is(ValueKind::Temp) || second.ops[1].is(ValueKind::Integer)))
		return kNoId;
	const auto width = integer_bits(second.type.kind());
	if ((!width && !second.type.is_pointer()) || width > 64)
		return kNoId;
	const auto pred = static_cast<ComparePredicate>(second.sub);
	const bool narrow = width && width < 32;
	if (narrow &&
		((pred >= ComparePredicate::Lt && pred <= ComparePredicate::Ge) ||
			body_.temps[second.ops[0].index].type != second.type ||
			(second.ops[1].is(ValueKind::Temp) && body_.temps[second.ops[1].index].type != second.type)))
		return kNoId;
	const InstId join_id = predicate_join(body_, uses_, first, second);
	if (join_id == kNoId)
		return kNoId;
	const bool either = body_.instructions[join_id].sub == static_cast<unsigned>(BinaryOp::Or);
	compare_flags(first);
	const bool w = width && width <= 32;
	const unsigned lhs = operand_register(second.ops[0], second.type, 10);
	unsigned rhs = 0;
	Op selected = w ? Op::CcmpW : Op::Ccmp;
	if (second.ops[1].is(ValueKind::Integer))
	{
		uint64_t raw = body_.literals[second.ops[1].index].low;
		if (width && width < 64)
			raw &= (uint64_t(1) << width) - 1;
		if (raw < 32)
		{
			selected = w ? Op::CcmpIW : Op::CcmpI;
			rhs = raw;
		}
		else
		{
			constant(11, raw);
			rhs = 11;
		}
	}
	else
		rhs = operand_register(second.ops[1], second.type, 11);
	const unsigned cond = conditions[second.sub];
	out_.emit(selected, nzcv_satisfying(cond ^ !either), lhs, rhs, (conditions[first.sub] ^ either) << 12);
	return finish_condition(join_id, cond, block);
}

// Delay materializing the flags through adjacent uses that preserve a boolean.
InstId Selector::finish_condition(InstId id, unsigned condition, BlockId block)
{
	while (body_.instructions[id].next != kNoId && uses_[body_.instructions[id].result] == 1)
	{
		const auto next_id = body_.instructions[id].next;
		const auto& next = body_.instructions[next_id];
		if (!(next.ops[0] == Value::temp(body_.instructions[id].result)))
			break;
		if (next.op == Opcode::Branch)
		{
			conditional_edges(block, next, Op::BCond, condition ^ 1);
			return next_id;
		}
		if (next.op == Opcode::Compare && next.ops[1].is(ValueKind::Integer) && next.type.is_integer() &&
			integer_bits(next.type.kind()) <= 64 && body_.literals[next.ops[1].index].low == 0 &&
			(next.sub == static_cast<unsigned>(ComparePredicate::Eq) ||
				next.sub == static_cast<unsigned>(ComparePredicate::Ne)))
			condition ^= next.sub == static_cast<unsigned>(ComparePredicate::Eq);
		else if (next.op != Opcode::Convert || next.sub != static_cast<unsigned>(ConvertOp::Trunc) ||
			!next.type.is_integer() || integer_bits(next.type.kind()) > 64)
			break;
		id = next_id;
	}
	set_condition(body_.instructions[id].result, condition);
	return id;
}

namespace {

bool direct_select(const lowir::Instruction& select, TempId test)
{
	return select.op == Opcode::Select && select.ops[0] == Value::temp(test) && !memory_value(select.type) &&
		!select.ops[1].is(ValueKind::Symbol) && !select.ops[2].is(ValueKind::Symbol);
}

} // namespace

// LowIR gives each select its own comparison so it can be emitted beside
// its use. Here a run of selects of comparisons of the same operands uses
// the same flags. Each word, doubleword or pointer predicate reads the
// flags of one comparison; a narrower one also selects its extension.
InstId Selector::compare_select_pair(const lowir::Instruction& inst)
{
	if (inst.next == kNoId || uses_[inst.result] != 1 ||
		!(inst.type.is_pointer() || (inst.type.is_integer() && integer_bits(inst.type.kind()) <= 64)))
		return kNoId;
	const auto& first = body_.instructions[inst.next];
	if (!direct_select(first, inst.result) || first.next == kNoId)
		return kNoId;
	const unsigned bits = integer_bits(inst.type.kind());
	const bool any_predicate = inst.type.is_pointer() || bits == 32 || bits == 64;
	const auto same = [&](const lowir::Instruction& other) {
		const auto same_operand = [&](unsigned k) {
			return inst.ops[k] == other.ops[k] || opt::same_literal(body_, inst.type, inst.ops[k], other.ops[k]);
		};
		return other.op == Opcode::Compare && other.next != kNoId && uses_[other.result] == 1 &&
			other.type == inst.type && (other.sub == inst.sub || any_predicate) && same_operand(0) && same_operand(1) &&
			direct_select(body_.instructions[other.next], other.result);
	};
	if (!same(body_.instructions[first.next]))
		return kNoId;
	compare_flags(inst);
	choose(first, conditions[inst.sub]);
	InstId last = inst.next;
	while (body_.instructions[last].next != kNoId && same(body_.instructions[body_.instructions[last].next]))
	{
		const auto& other = body_.instructions[body_.instructions[last].next];
		choose(body_.instructions[other.next], conditions[other.sub]);
		last = other.next;
	}
	return last;
}

bool Selector::compare_consumer(const lowir::Instruction& inst, BlockId block, bool flags_ready)
{
	if (inst.next == kNoId || uses_[inst.result] != 1 ||
		!(inst.type.is_pointer() || (inst.type.is_integer() && integer_bits(inst.type.kind()) <= 64)))
		return false;
	const auto& next = body_.instructions[inst.next];
	if (direct_select(next, inst.result))
	{
		if (!flags_ready)
			compare_flags(inst);
		choose(next, conditions[inst.sub]);
		return true;
	}
	if (next.op != Opcode::Branch || !(next.ops[0] == Value::temp(inst.result)))
		return false;
	const unsigned narrow = integer_bits(inst.type.kind());
	const auto pred = static_cast<ComparePredicate>(inst.sub);
	if (!flags_ready && (narrow == 8 || narrow == 16) &&
		(pred == ComparePredicate::Lt || pred == ComparePredicate::Ge) && inst.ops[1].is(ValueKind::Integer) &&
		body_.literals[inst.ops[1].index].low == 0)
	{
		const unsigned source = operand_register(inst.ops[0], inst.type, 10);
		conditional_bit_edges(
			block, next, pred == ComparePredicate::Lt ? Op::CbzW : Op::CbnzW, source, narrow - 1, narrow);
		return true;
	}
	const bool eq = inst.sub == static_cast<unsigned>(ComparePredicate::Eq);
	if (!flags_ready && (eq || inst.sub == static_cast<unsigned>(ComparePredicate::Ne)))
	{
		Value a = inst.ops[0], b = inst.ops[1];
		if (a.is(ValueKind::Integer))
			std::swap(a, b);
		const unsigned bits = integer_bits(inst.type.kind());
		const bool canonical = bits >= 32 || inst.type.is_pointer() ||
			(a.is(ValueKind::Temp) && body_.temps[a.index].type == inst.type);
		if (canonical && b.is(ValueKind::Integer) && body_.literals[b.index].low == 0)
		{
			const bool w = bits && bits <= 32;
			conditional_edges(block, next, eq ? (w ? Op::CbnzW : Op::Cbnz) : (w ? Op::CbzW : Op::Cbz),
				operand_register(a, inst.type, 10));
			return true;
		}
	}
	if (!flags_ready)
		compare_flags(inst);
	conditional_edges(block, next, Op::BCond, conditions[inst.sub] ^ 1);
	return true;
}

// An integer increment or decrement, or a pointer index, preserves NZCV for
// the branch or select after it. Keep the comparison's original operand
// reads, which may precede a register being reused by the step.
InstId Selector::compare_before_increment(const lowir::Instruction& inst, BlockId block)
{
	const bool fp = inst.type.kind() == TypeKind::F32 || inst.type.kind() == TypeKind::F64;
	if (inst.next == kNoId || uses_[inst.result] != 1 ||
		!(fp || inst.type.is_pointer() || (inst.type.is_integer() && integer_bits(inst.type.kind()) <= 64)))
		return kNoId;
	const auto& step = body_.instructions[inst.next];
	const bool increment = step.op == Opcode::Binary &&
		(step.sub == static_cast<unsigned>(BinaryOp::Add) || step.sub == static_cast<unsigned>(BinaryOp::Sub)) &&
		step.type.is_integer() && integer_bits(step.type.kind()) <= 64;
	if ((!increment && step.op != Opcode::Index) || step.next == kNoId)
		return kNoId;
	const auto& branch = body_.instructions[step.next];
	const bool chooses = direct_select(branch, inst.result);
	if ((!chooses && branch.op != Opcode::Branch) || !(branch.ops[0] == Value::temp(inst.result)))
		return kNoId;
	unsigned condition = 0;
	if (fp)
		condition = float_flags(inst);
	else
	{
		compare_flags(inst);
		condition = conditions[inst.sub];
	}
	// Selecting between a value and its sole increment by one is CSINC.
	const unsigned bits = integer_bits(step.type.kind());
	if (chooses && increment && step.sub == static_cast<unsigned>(BinaryOp::Add) && (bits == 32 || bits == 64) &&
		branch.type == step.type && uses_[step.result] == 1 && step.ops[0].is(ValueKind::Temp) &&
		step.ops[1].is(ValueKind::Integer) && body_.literals[step.ops[1].index].low == 1)
	{
		const Value next = Value::temp(step.result);
		const bool on_true = branch.ops[1] == next && branch.ops[2] == step.ops[0];
		if (on_true || (branch.ops[2] == next && branch.ops[1] == step.ops[0]))
		{
			const unsigned source = operand_register(step.ops[0], step.type, 10);
			const unsigned dest = registers_[branch.result] == kNoId ? 9 : registers_[branch.result];
			out_.emit(bits == 32 ? Op::CsincW : Op::Csinc, dest, source, source, (condition ^ on_true) << 12);
			save(dest, branch.result, true);
			return step.next;
		}
	}
	if (increment || address_sequence(inst.next) == kNoId)
		select(step, block);
	if (chooses)
		choose(branch, condition);
	else
		conditional_edges(block, branch, Op::BCond, condition ^ 1);
	return step.next;
}

// A comparison result added to a word or doubleword, through truncations
// and zero extensions of the boolean, is CINC (CSINC of the other addend
// with itself on the inverted condition), as is a selection between a value
// and its increment that follows the comparison and the increment. The
// instructions are adjacent and each consumed result has no other reader.
InstId Selector::added_condition(const lowir::Instruction& cmp)
{
	if (cmp.next == kNoId || uses_[cmp.result] != 1 ||
		!(cmp.type.is_pointer() || (cmp.type.is_integer() && integer_bits(cmp.type.kind()) <= 64)))
		return kNoId;
	const auto& step = body_.instructions[cmp.next];
	if (step.next != kNoId && increment_choice(cmp, step, body_.instructions[step.next]))
		return step.next;
	Value flag = Value::temp(cmp.result);
	InstId id = cmp.next;
	for (unsigned n = 0; n < 2; ++n)
	{
		const auto& cast = body_.instructions[id];
		if (cast.op != Opcode::Convert || !(cast.ops[0] == flag) ||
			(cast.sub != static_cast<unsigned>(ConvertOp::Trunc) &&
				cast.sub != static_cast<unsigned>(ConvertOp::ZExt)) ||
			!cast.type.is_integer() || integer_bits(cast.type.kind()) > 64 || uses_[cast.result] != 1 ||
			cast.next == kNoId)
			break;
		flag = Value::temp(cast.result);
		id = cast.next;
	}
	const auto& add = body_.instructions[id];
	const unsigned bits = integer_bits(add.type.kind());
	if (add.op != Opcode::Binary || add.sub != static_cast<unsigned>(BinaryOp::Add) || (bits != 32 && bits != 64) ||
		(add.ops[0] == flag) == (add.ops[1] == flag))
		return kNoId;
	const Value other = add.ops[add.ops[0] == flag ? 1 : 0];
	if (other.is(ValueKind::Symbol))
		return kNoId;
	compare_flags(cmp);
	const unsigned source = zero_or_register(other, add.type, 10);
	const unsigned dest = registers_[add.result] == kNoId ? 9 : registers_[add.result];
	out_.emit(bits == 32 ? Op::CsincW : Op::Csinc, dest, source, source, (conditions[cmp.sub] ^ 1) << 12);
	save(dest, add.result, true);
	return id;
}

void Selector::integer_unary(const lowir::Instruction& inst)
{
	const unsigned bits = integer_bits(inst.type.kind());
	const auto op = static_cast<UnaryOp>(inst.sub);
	unsigned src = operand_register(inst.ops[0], inst.type, 10);
	const unsigned dst = registers_[inst.result] == kNoId ? 9 : registers_[inst.result];
	switch (op)
	{
	case UnaryOp::Neg:
		out_.emit(bits <= 32 ? Op::SubW : Op::Sub, dst, 31, src);
		break;
	case UnaryOp::BitNot:
		out_.emit(bits <= 32 ? Op::OrnW : Op::Orn, dst, 31, src);
		break;
	case UnaryOp::Not:
		out_.emit(Op::Subs, 31, src, 31);
		out_.emit(Op::CsincW, dst, 31, 31, 1u << 12);
		break;
	case UnaryOp::ByteSwap:
		out_.emit(bits == 64 ? Op::Rev64 : bits == 32 ? Op::Rev32 : Op::Rev16, dst, src);
		break;
	case UnaryOp::CountTrailingZeros:
	case UnaryOp::CountLeadingZeros:
		if (op == UnaryOp::CountTrailingZeros)
		{
			out_.emit(Op::Rbit, 10, src, 0, bits == 64 ? 1u << 31 : 0);
			src = 10;
		}
		out_.emit(Op::Clz, dst, src, 0, bits == 64 ? 1u << 31 : 0);
		break;
	case UnaryOp::CountOnes:
		move(10, src);
		extend(10, bits, false);
		out_.emit(Op::FmovDX, 16, 10);
		out_.emit(Op::Cnt8B, 16, 16);
		out_.emit(Op::Addv8B, 16, 16);
		out_.emit(Op::UmovB, dst, 16);
		break;
	case UnaryOp::Sqrt:
		break; // Handled by float_instruction.
	}
	save(dst, inst.result, bits == 32 || bits == 64);
}

// selector - amount (a compare into the zero register), an immediate when
// its magnitude encodes and x10 otherwise.
void Selector::compare_key(unsigned dest, unsigned input, uint64_t amount, bool narrow)
{
	const bool negative = static_cast<int64_t>(amount) < 0;
	const uint64_t magnitude = negative ? 0 - amount : amount;
	const unsigned shift = (magnitude & 4095) == 0 ? 12 : 0;
	if ((magnitude >> shift) <= 4095)
		out_.emit(negative ? (narrow ? Op::AddsIW : Op::AddsIX) : (narrow ? Op::SubsIW : Op::SubsIX), dest, input, 0,
			(shift ? 1u << 22 : 0) | (uint32_t(magnitude >> shift) << 10));
	else
	{
		constant(10, amount);
		out_.emit(narrow ? Op::SubsW : Op::Subs, dest, input, 10);
	}
}

// A switch's runs of cases, searched by halves: the runs not taking the
// fallback (which a miss reaches anyway), each run's label, and the stubs
// holding the phi transfers of the nonempty edges after the tests.
struct Selector::SwitchSearch
{
	std::vector<machine::CaseCluster> runs;
	std::vector<uint32_t> labels;
	std::vector<std::pair<BlockId, uint32_t>> stubs;
	BlockId fallback = kNoId;
	uint32_t other = kNoId;
	unsigned source = 31;
	bool narrow = false, sign = false;
};

// machine::kNativeSearchRuns runs or more with gaps between them (values
// the fallback takes; a run of consecutive cases, an enumeration's, keeps
// its order) search by halves, as on x86-64: a missed value runs about
// log2 of the runs' tests instead of all of them. The keys order as
// switch_edges reads them: a 64-bit or signed 32-bit selector compares
// signed, a narrower or unsigned one unsigned.
bool Selector::search_switch(SwitchSearch& search, BlockId from, Type type)
{
	auto& runs = search.runs;
	runs.erase(std::remove_if(runs.begin(), runs.end(),
				   [&](const machine::CaseCluster& c) { return c.target == search.fallback; }),
		runs.end());
	uint32_t gaps = 0;
	for (size_t i = 1; i < runs.size(); ++i)
		gaps += runs[i].lo > runs[i - 1].hi + 1 ? 1 : 0;
	if (runs.size() < machine::kNativeSearchRuns || gaps < machine::kNativeSearchGaps)
		return false;
	auto label = [&](BlockId target) {
		if (empty_edge(from, target))
			return target;
		for (const auto& stub : search.stubs)
			if (stub.first == target)
				return stub.second;
		const auto next = static_cast<uint32_t>(out_.labels.size());
		out_.labels.push_back(kNoId);
		search.stubs.push_back({ target, next });
		return next;
	};
	search.other = label(search.fallback);
	for (const auto& c : runs)
		search.labels.push_back(label(c.target));
	const unsigned bits = type.is_pointer() ? 64 : integer_bits(type.kind());
	search.sign = bits == 64 || type.kind() == TypeKind::I32;
	const int64_t lo = bits == 64 ? INT64_MIN : search.sign ? INT32_MIN : 0;
	const int64_t hi = bits == 64 ? INT64_MAX
		: search.sign             ? INT32_MAX
								  : static_cast<int64_t>((uint64_t(1) << bits) - 1);
	search_cases(search, 0, static_cast<uint32_t>(runs.size()), lo, hi, false);
	for (const auto& stub : search.stubs)
	{
		out_.bind(stub.second);
		edge(from, stub.first, &stub == &search.stubs.back());
	}
	return true;
}

// Runs [first, past) of a search, the selector known within [lo, hi]: four
// or more test whether it is below the middle run (the lower half after
// the upper one); fewer test each run, one compare when a known bound
// meets it. A failed test at a known bound moves that bound. (The upper
// half's first run, at the value the search just compared, reads its
// flags.)
void Selector::search_cases(
	const SwitchSearch& search, uint32_t first, uint32_t past, int64_t lo, int64_t hi, bool compared)
{
	const auto& runs = search.runs;
	const bool narrow = search.narrow;
	const unsigned source = search.source;
	enum : unsigned
	{
		Eq = 0,
		Hs = 2,
		Lo = 3,
		Ls = 9,
		Ge = 10,
		Lt = 11,
		Le = 13
	};
	if (past - first >= 4)
	{
		const uint32_t middle = (first + past) / 2;
		const int64_t pivot = runs[middle].lo;
		const auto lower = static_cast<uint32_t>(out_.labels.size());
		out_.labels.push_back(kNoId);
		compare_key(31, source, static_cast<uint64_t>(pivot), narrow);
		out_.reference(Op::BCond, Fixup::Block, lower, search.sign ? Lt : Lo);
		search_cases(search, middle, past, pivot, hi, true);
		out_.bind(lower);
		search_cases(search, first, middle, lo, pivot - 1, false);
		return;
	}
	bool flags = compared;
	for (uint32_t i = first; i < past; ++i)
	{
		const auto& c = runs[i];
		const uint32_t to = search.labels[i];
		if (c.lo <= lo && c.hi >= hi)
		{
			out_.reference(Op::B, Fixup::Block, to);
			return;
		}
		if (c.lo == c.hi && !c.lo && !flags)
			out_.reference(narrow ? Op::CbzW : Op::Cbz, Fixup::Block, to, source);
		else if (c.lo == c.hi)
		{
			if (!flags)
				compare_key(31, source, static_cast<uint64_t>(c.lo), narrow);
			out_.reference(Op::BCond, Fixup::Block, to, Eq);
		}
		else if (c.lo <= lo)
		{
			compare_key(31, source, static_cast<uint64_t>(c.hi), narrow);
			out_.reference(Op::BCond, Fixup::Block, to, search.sign ? Le : Ls);
		}
		else if (c.hi >= hi)
		{
			compare_key(31, source, static_cast<uint64_t>(c.lo), narrow);
			out_.reference(Op::BCond, Fixup::Block, to, search.sign ? Ge : Hs);
		}
		else
		{
			compare_key(9, source, static_cast<uint64_t>(c.lo), narrow);
			compare_key(31, 9, static_cast<uint64_t>(c.hi) - static_cast<uint64_t>(c.lo), narrow);
			out_.reference(Op::BCond, Fixup::Block, to, Ls);
		}
		flags = false;
		if (c.lo <= lo)
			lo = c.hi + 1;
		else if (c.hi >= hi)
			hi = c.lo - 1;
	}
	out_.reference(Op::B, Fixup::Block, search.other);
}

void Selector::switch_edges(const lowir::Instruction& inst, BlockId from)
{
	const Type type = type_of(inst.ops[0], Type::scalar(TypeKind::I64));
	const unsigned bits = type.is_pointer() ? 64 : integer_bits(type.kind());
	const uint64_t mask = bits < 64 ? (uint64_t(1) << bits) - 1 : ~uint64_t(0);
	std::vector<std::pair<int64_t, uint32_t>> cases;
	for (unsigned i = 0; i < inst.extra_count; i += 2)
	{
		const Value operand = body_.operands[inst.extra_first + i];
		const uint64_t raw = body_.literals[operand.index].low & mask;
		// A W subtraction wraps at 32 bits. Keeping signed cases in source
		// order lets one bounds check cover a table that crosses zero.
		const int64_t key = type.kind() == TypeKind::I32 ? static_cast<int32_t>(raw) : static_cast<int64_t>(raw);
		cases.push_back({ key, body_.operands[inst.extra_first + i + 1].index });
	}
	std::vector<machine::CaseCluster> clusters;
	std::vector<uint32_t> targets;
	machine::cluster_switch_cases(cases, clusters, targets, machine::kNativeSwitchTargets);
	const unsigned source = operand_register(inst.ops[0], type, 15);
	const bool narrow = bits <= 32;
	auto subtract = [&](unsigned dest, unsigned input, uint64_t amount) { compare_key(dest, input, amount, narrow); };
	const BlockId fallback = inst.ops[1].index;
	const uint64_t first = cases.empty() ? 0 : static_cast<uint64_t>(cases.front().first);
	const uint64_t span = cases.empty() ? 0 : static_cast<uint64_t>(cases.back().first) - first;
	const bool masks = !cases.empty() && span < 64 && !targets.empty() && targets.size() <= 2 &&
		clusters.size() >= 2 * targets.size() + 1;
	// A64 table entries are executable branch instructions. A sparse table
	// adds an indirect branch as well as the selected entry's direct branch.
	if (level_ && (masks || (span < 4 * cases.size() && machine::native_table(cases, clusters.size(), targets.size()))))
	{
		// Several entries may share the same phi transfer. Give each
		// nonempty edge one stub, including the out-of-range default.
		std::vector<std::pair<BlockId, uint32_t>> stubs;
		auto label = [&](BlockId target) {
			if (empty_edge(from, target))
				return target;
			for (const auto& stub : stubs)
				if (stub.first == target)
					return stub.second;
			const auto next = static_cast<uint32_t>(out_.labels.size());
			out_.labels.push_back(kNoId);
			stubs.push_back({ target, next });
			return next;
		};
		const auto other = label(fallback);
		const unsigned index = first ? 9 : source;
		if (first)
			subtract(index, source, first);
		subtract(31, index, span);
		out_.reference(Op::BCond, Fixup::Block, other, 8);
		if (masks)
		{
			for (BlockId target : targets)
			{
				if (target == fallback)
					continue;
				uint64_t members = 0;
				for (const auto& c : clusters)
					if (c.target == target)
					{
						const auto lo = static_cast<uint64_t>(c.lo) - first;
						const auto hi = static_cast<uint64_t>(c.hi) - first;
						members |= (UINT64_MAX >> (63 - hi + lo)) << lo;
					}
				constant(10, members);
				out_.emit(Op::Lsr, 11, 10, index);
				uint32_t field = 0;
				logical_immediate(1, 64, field);
				const uint32_t pc = out_.code.size();
				out_.emit(Op::AndImm, 11, 11, 0, field);
				out_.reference(Op::Cbnz, Fixup::Block, label(target), 11);
				if (!out_.exceptions)
					bit_tests_.push_back({ pc, 0 });
			}
			out_.reference(Op::B, Fixup::Block, other);
		}
		else
		{
			const auto table = static_cast<uint32_t>(out_.labels.size());
			out_.labels.push_back(kNoId);
			out_.reference(Op::Adr, Fixup::Block, table, 16);
			out_.emit(Op::Add, 16, 16, index, (2u << 10) | (narrow ? (1u << 21) | (2u << 13) : 0u));
			out_.emit(Op::Br, 0, 16);
			out_.bind(table);
			size_t cluster = 0;
			uint32_t destination = label(clusters[0].target);
			for (uint64_t i = 0; i <= span; ++i)
			{
				while (uint64_t(clusters[cluster].hi) - first < i)
					destination = label(clusters[++cluster].target);
				out_.reference(Op::B, Fixup::Block, i < uint64_t(clusters[cluster].lo) - first ? other : destination);
			}
		}
		for (const auto& stub : stubs)
		{
			out_.bind(stub.second);
			edge(from, stub.first, &stub == &stubs.back());
		}
		return;
	}
	SwitchSearch search;
	search.runs = clusters;
	search.fallback = fallback;
	search.source = source;
	search.narrow = narrow;
	if (level_ && search_switch(search, from, type))
		return;
	for (const auto& c : clusters)
	{
		const bool single = c.lo == c.hi;
		const bool zero = single && !c.lo;
		if (!zero)
		{
			if (single)
				subtract(31, source, c.lo);
			else
			{
				if (c.lo)
					subtract(9, source, c.lo);
				subtract(31, c.lo ? 9 : source, uint64_t(c.hi) - uint64_t(c.lo));
			}
		}
		// The last successful case can fall into its own block. Branch on a
		// miss directly to the default when that edge needs no phi copies.
		const bool last_falls = &c == &clusters.back() && c.target == fallthrough_ && empty_edge(from, fallback);
		const bool direct = last_falls || empty_edge(from, c.target);
		const uint32_t label = last_falls ? fallback : direct ? c.target : static_cast<uint32_t>(out_.labels.size());
		const bool on_match = direct && !last_falls;
		if (!direct)
			out_.labels.push_back(kNoId);
		if (zero)
			out_.reference(on_match ? (narrow ? Op::CbzW : Op::Cbz) : (narrow ? Op::CbnzW : Op::Cbnz), Fixup::Block,
				label, source);
		else
			out_.reference(Op::BCond, Fixup::Block, label, (single ? 0u : 9u) ^ (on_match ? 0u : 1u));
		if (last_falls)
		{
			edge(from, c.target, true);
			return;
		}
		if (!direct)
		{
			edge(from, c.target);
			out_.bind(label);
		}
	}
	edge(from, inst.ops[1].index, true);
}

} // namespace aarch64
