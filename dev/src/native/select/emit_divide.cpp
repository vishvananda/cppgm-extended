#include "native/select/function_selector.h"

#include "native/mir/type_kinds.h"

// Integer division and remainder. A divisor that is not a constant uses
// the hardware divide (cqo/idiv or a zeroed rdx and div: rax and rdx). At
// O1 and above that is 32 bits wide for a 32-bit type: the 64-bit divide
// costs far more on x86 cores before Ice Lake and Zen 3, and a 32-bit one
// reads only the low halves, so neither operand needs a conversion. (A
// constant divisor keeps the 64-bit divide, as does O0, where -1 is not yet
// a constant: x / -1 of INT_MIN does not trap there, as GCC's negation does
// not.) An unsigned power of two is a shift or a mask at every level. At O1 and
// above the other constants avoid the divide (14-18 cycles on current x86
// against 3-4 for a multiply; T. Granlund and P. Montgomery, "Division by
// Invariant Integers using Multiplication", PLDI 1994; H. Warren, Hacker's
// Delight, 2nd ed., ch. 10):
//
// - signed by +-2^k: the dividend plus 2^k - 1 when negative, shifted
//   arithmetically (the remainder: the dividend minus that sum's multiple
//   of 2^k), negated for a negative divisor;
// - an exact division (IF_EXACT, a pointer difference: 5.7p6 makes the
//   byte difference a multiple of the element size): an arithmetic shift
//   by the divisor's power of two, then a multiply by the inverse of its
//   odd part modulo 2^64;
// - otherwise the high half of a widening multiply by a magic number,
//   shifted (unsigned: with Hacker's Delight's add fix-up when the magic
//   needs 65 bits; signed: with the dividend's correction and the sign
//   bit added), and the remainder as the dividend minus quotient times
//   divisor.
//
// Narrow operands are their registers' canonical 64-bit forms at the
// operation's signedness (a converted dividend is extended in r10 first),
// so the 64-bit sequences give the narrow quotient and remainder, which
// are in range for the result type except after such a conversion (then
// normalized). Divisors 0 and +-1 and INT64_MIN keep the hardware divide.
namespace native {

using lowir::BinaryOp;
using lowir::Instruction;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

namespace {

struct UnsignedMagic
{
	uint64_t multiplier;
	unsigned shift;
	bool add;
};

struct SignedMagic
{
	int64_t multiplier;
	unsigned shift;
};

// Hacker's Delight 10-10 (magicu2) at 64 bits: the multiplier and shift
// for an unsigned divisor d >= 2 that is not a power of two.
UnsignedMagic unsigned_magic(uint64_t d)
{
	const uint64_t two63 = uint64_t(1) << 63;
	UnsignedMagic magic = { 0, 0, false };
	const uint64_t nc = ~uint64_t(0) - (0 - d) % d;
	unsigned p = 63;
	uint64_t q1 = two63 / nc, r1 = two63 - q1 * nc;
	uint64_t q2 = (two63 - 1) / d, r2 = (two63 - 1) - q2 * d;
	uint64_t delta = 0;
	do
	{
		++p;
		if (r1 >= nc - r1)
		{
			q1 = 2 * q1 + 1;
			r1 = 2 * r1 - nc;
		}
		else
		{
			q1 = 2 * q1;
			r1 = 2 * r1;
		}
		if (r2 + 1 >= d - r2)
		{
			if (q2 >= two63 - 1)
				magic.add = true;
			q2 = 2 * q2 + 1;
			r2 = 2 * r2 + 1 - d;
		}
		else
		{
			if (q2 >= two63)
				magic.add = true;
			q2 = 2 * q2;
			r2 = 2 * r2 + 1;
		}
		delta = d - 1 - r2;
	} while (p < 128 && (q1 < delta || (q1 == delta && r1 == 0)));
	magic.multiplier = q2 + 1;
	magic.shift = p - 64;
	return magic;
}

// Hacker's Delight 10-1 (magic) at 64 bits: for a signed divisor with
// |d| >= 2 that is not a power of two.
SignedMagic signed_magic(int64_t d)
{
	const uint64_t two63 = uint64_t(1) << 63;
	const uint64_t ad = d < 0 ? 0 - static_cast<uint64_t>(d) : static_cast<uint64_t>(d);
	const uint64_t t = two63 + (static_cast<uint64_t>(d) >> 63);
	const uint64_t anc = t - 1 - t % ad;
	unsigned p = 63;
	uint64_t q1 = two63 / anc, r1 = two63 - q1 * anc;
	uint64_t q2 = two63 / ad, r2 = two63 - q2 * ad;
	uint64_t delta = 0;
	do
	{
		++p;
		q1 = 2 * q1;
		r1 = 2 * r1;
		if (r1 >= anc)
		{
			++q1;
			r1 -= anc;
		}
		q2 = 2 * q2;
		r2 = 2 * r2;
		if (r2 >= ad)
		{
			++q2;
			r2 -= ad;
		}
		delta = ad - r2;
	} while (q1 < delta || (q1 == delta && r1 == 0));
	SignedMagic magic;
	magic.multiplier = static_cast<int64_t>(q2 + 1);
	if (d < 0)
		magic.multiplier = static_cast<int64_t>(0 - static_cast<uint64_t>(magic.multiplier));
	magic.shift = p - 64;
	return magic;
}

// The inverse of an odd number modulo 2^64 (Newton's iteration doubles
// the correct low bits: 3 -> 6 -> 12 -> 24 -> 48 -> 96).
uint64_t odd_inverse(uint64_t odd)
{
	uint64_t x = odd;
	for (unsigned k = 0; k < 5; ++k)
		x *= 2 - odd * x;
	return x;
}

unsigned integer_width(TypeKind kind)
{
	switch (kind)
	{
	case TypeKind::I8:
	case TypeKind::U8:
		return 8;
	case TypeKind::I16:
	case TypeKind::U16:
		return 16;
	case TypeKind::I32:
	case TypeKind::U32:
		return 32;
	default:
		return 64;
	}
}

unsigned trailing_zeros(uint64_t v)
{
	return static_cast<unsigned>(__builtin_ctzll(v));
}

bool is_unsigned_op(BinaryOp op)
{
	return op == BinaryOp::UDiv || op == BinaryOp::UMod;
}

bool is_remainder(BinaryOp op)
{
	return op == BinaryOp::Mod || op == BinaryOp::UMod;
}

Operand reg(Reg r)
{
	return Operand::make_reg(r);
}

Operand imm(int64_t v)
{
	return Operand::make_imm(v, v < 0 ? -1 : 0);
}

} // namespace

DivideForm FunctionSelector::divide_form(const Instruction& inst, int64_t& divisor) const
{
	const TypeKind kind = inst.type.kind();
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const bool unsigned_op = is_unsigned_op(op);
	const Value bv = resolve(inst.ops[1]);
	divisor = 0;
	if (bv.kind != ValueKind::Integer)
		return DivideForm::Hardware;
	const uint64_t low = literal(bv).low;
	if (unsigned_op && power_of_two(low) && static_cast<int64_t>(low) > 0)
	{
		divisor = static_cast<int64_t>(low);
		return DivideForm::UnsignedShift;
	}
	if (level_ < 1 || kind == TypeKind::I1)
		return DivideForm::Hardware;
	// The divisor at the type's width, read at the operation's signedness.
	const unsigned width = integer_width(kind);
	uint64_t bits = low;
	if (width < 64)
	{
		const uint64_t mask = (uint64_t(1) << width) - 1;
		bits &= mask;
		if (!unsigned_op && ((bits >> (width - 1)) & 1))
			bits |= ~mask;
	}
	divisor = static_cast<int64_t>(bits);
	if (unsigned_op)
	{
		if (bits <= 1)
			return DivideForm::Hardware;
		return power_of_two(bits) ? DivideForm::UnsignedShift : DivideForm::UnsignedMagic;
	}
	if (divisor == 0 || divisor == 1 || divisor == -1 || divisor == INT64_MIN)
		return DivideForm::Hardware;
	if ((inst.flags & lowir::IF_EXACT) && op == BinaryOp::Div)
		return DivideForm::Exact;
	const uint64_t magnitude = divisor < 0 ? 0 - bits : bits;
	return power_of_two(magnitude) ? DivideForm::SignedShift : DivideForm::SignedMagic;
}

void FunctionSelector::emit_divide(const Instruction& inst)
{
	int64_t divisor = 0;
	const DivideForm form = divide_form(inst, divisor);
	switch (form)
	{
	case DivideForm::Hardware:
		emit_divide_hardware(inst);
		return;
	case DivideForm::UnsignedShift:
		emit_divide_unsigned_shift(inst, static_cast<uint64_t>(divisor));
		return;
	default:
		emit_divide_constant(inst, form, divisor);
		return;
	}
}

void FunctionSelector::emit_divide_unsigned_shift(const Instruction& inst, uint64_t divisor)
{
	const TypeKind kind = inst.type.kind();
	const bool remainder = is_remainder(static_cast<BinaryOp>(inst.sub));
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
	load_into(d, a, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
	if (signed_integer(kind) && kind != TypeKind::I64)
		normalize(d, unsigned_of(kind));
	if (remainder)
		emit2(Op::And, TypeKind::I64, reg(d), imm(static_cast<int64_t>(divisor - 1)));
	else if (divisor != 1)
		emit2(Op::Shr, TypeKind::I64, reg(d), imm(log2_of(divisor)));
	if (signed_integer(kind) && kind != TypeKind::I64)
		normalize(d, kind);
	finish_result(inst, d);
}

void FunctionSelector::emit_divide_hardware(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const bool unsigned_op = is_unsigned_op(op);
	const bool remainder = is_remainder(op);
	const bool word = level_ >= 1 && (kind == TypeKind::I32 || kind == TypeKind::U32) &&
		resolve(inst.ops[1]).kind != ValueKind::Integer;
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	Operand b = rm_operand(inst.ops[1], inst.type, kEmitScratch);
	const bool convert_a = !word && narrow_integer(kind) && (unsigned_op == signed_integer(kind));
	if (b.kind == OperandKind::Imm || b.kind == OperandKind::Symbol ||
		(b.kind == OperandKind::Mem && !word && (narrow_integer(kind) || convert_a)) ||
		(convert_a && b.kind == OperandKind::Reg))
	{
		load_into(kEmitScratch, b, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
		b = reg(kEmitScratch);
		if (convert_a)
			normalize(kEmitScratch, unsigned_op ? unsigned_of(kind) : signed_of(kind));
	}
	const Operand rax = reg(RAX);
	if (a.kind == OperandKind::Mem)
		emit2(Op::Load, kind == TypeKind::Ptr ? TypeKind::Ptr : kind, rax, a);
	else
		load_into(RAX, a, TypeKind::I64);
	if (convert_a)
		normalize(RAX, unsigned_op ? unsigned_of(kind) : signed_of(kind));
	const TypeKind width = word ? TypeKind::I32 : TypeKind::I64;
	if (unsigned_op)
	{
		emit2(Op::Mov, TypeKind::Void, reg(RDX), Operand::make_imm(0));
		emit1(Op::Div, width, b);
	}
	else
	{
		emit(Op::Cqo, width);
		emit1(Op::Idiv, width, b);
	}
	const Reg from = remainder ? RDX : RAX;
	const Reg d = result_reg(inst, from);
	load_into(d, reg(from), TypeKind::I64);
	// A 32-bit divide zero-extends both results.
	if (word ? kind == TypeKind::I32 : narrow_integer(kind) && (convert_a || !remainder))
		normalize(d, kind);
	finish_result(inst, d);
}

// The dividend's register at the operation's signedness: its own register
// when no conversion is needed, else r10.
Reg FunctionSelector::divide_dividend(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	const bool unsigned_op = is_unsigned_op(static_cast<BinaryOp>(inst.sub));
	const bool convert = narrow_integer(kind) && (unsigned_op == signed_integer(kind));
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	if (a.kind == OperandKind::Reg && !convert)
		return a.reg;
	load_into(kSelectScratch, a, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
	if (convert)
		normalize(kSelectScratch, unsigned_op ? unsigned_of(kind) : signed_of(kind));
	return kSelectScratch;
}

void FunctionSelector::emit_divide_constant(const Instruction& inst, DivideForm form, int64_t divisor)
{
	const TypeKind kind = inst.type.kind();
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const bool convert = narrow_integer(kind) && (is_unsigned_op(op) == signed_integer(kind));
	const Reg x = divide_dividend(inst);
	Reg d = kNoReg;
	if (form == DivideForm::SignedShift || form == DivideForm::Exact)
	{
		d = result_reg(inst, dying_reg(inst.ops[0]), kNoReg, x == kSelectScratch ? 1u << kSelectScratch : 0);
		if (form == DivideForm::SignedShift)
			emit_signed_shift_divide(x, d, divisor, is_remainder(op));
		else
			emit_exact_divide(x, d, divisor);
	}
	else
	{
		const Reg from = form == DivideForm::UnsignedMagic ? emit_unsigned_magic_divide(x, divisor, is_remainder(op))
														   : emit_signed_magic_divide(x, divisor, is_remainder(op));
		d = result_reg(inst, from);
		load_into(d, reg(from), TypeKind::I64);
	}
	if (convert)
		normalize(d, kind);
	finish_result(inst, d);
}

// x / +-2^k or x % +-2^k into d, with x preserved until read last. The
// bias (2^k - 1 for a negative x) forms in d when d is not x, else in a
// scratch register x and d do not use.
void FunctionSelector::emit_signed_shift_divide(Reg x, Reg d, int64_t divisor, bool remainder)
{
	const uint64_t magnitude = divisor < 0 ? 0 - static_cast<uint64_t>(divisor) : static_cast<uint64_t>(divisor);
	const unsigned k = log2_of(magnitude);
	const Reg t = d != x ? d : (x == kEmitScratch ? kSelectScratch : kEmitScratch);
	emit2(Op::Mov, TypeKind::Void, reg(t), reg(x));
	if (k == 1)
		emit2(Op::Shr, TypeKind::I64, reg(t), imm(63));
	else
	{
		emit2(Op::Sar, TypeKind::I64, reg(t), imm(63));
		emit2(Op::Shr, TypeKind::I64, reg(t), imm(64 - k));
	}
	if (!remainder)
	{
		emit2(Op::Add, TypeKind::I64, reg(d), reg(t == d ? x : t));
		emit2(Op::Sar, TypeKind::I64, reg(d), imm(k));
		if (divisor < 0)
			emit1(Op::Neg, TypeKind::I64, reg(d));
		return;
	}
	// The remainder: x - ((x + bias) & -2^k).
	emit2(Op::Add, TypeKind::I64, reg(t), reg(x));
	if (k <= 31)
		emit2(Op::And, TypeKind::I64, reg(t), imm(-static_cast<int64_t>(magnitude)));
	else
	{
		emit2(Op::Sar, TypeKind::I64, reg(t), imm(k));
		emit2(Op::Shl, TypeKind::I64, reg(t), imm(k));
	}
	if (t == d)
	{
		emit1(Op::Neg, TypeKind::I64, reg(d));
		emit2(Op::Add, TypeKind::I64, reg(d), reg(x));
	}
	else
		emit2(Op::Sub, TypeKind::I64, reg(d), reg(t));
}

// An exact x / divisor into d: (x >> k) * inverse(odd), negated for a
// negative divisor. x is dead after the first move, so a scratch x uses
// may hold the inverse.
void FunctionSelector::emit_exact_divide(Reg x, Reg d, int64_t divisor)
{
	const uint64_t magnitude = divisor < 0 ? 0 - static_cast<uint64_t>(divisor) : static_cast<uint64_t>(divisor);
	const unsigned k = trailing_zeros(magnitude);
	const uint64_t odd = magnitude >> k;
	if (d != x)
		emit2(Op::Mov, TypeKind::Void, reg(d), reg(x));
	if (k != 0)
		emit2(Op::Sar, TypeKind::I64, reg(d), imm(k));
	if (odd != 1)
	{
		const int64_t inverse = static_cast<int64_t>(odd_inverse(odd));
		if (inverse >= INT32_MIN && inverse <= INT32_MAX)
			emit2(Op::Imul, TypeKind::I64, reg(d), imm(inverse));
		else
		{
			const Reg t = d == kEmitScratch ? kSelectScratch : kEmitScratch;
			emit2(Op::Mov, TypeKind::Void, reg(t), imm(inverse));
			emit2(Op::Imul, TypeKind::I64, reg(d), reg(t));
		}
	}
	if (divisor < 0)
		emit1(Op::Neg, TypeKind::I64, reg(d));
}

// x - q * divisor with q in `q` and the other of rax/rdx free: the
// remainder's register.
Reg FunctionSelector::emit_magic_remainder(Reg x, Reg q, int64_t divisor)
{
	if (divisor >= INT32_MIN && divisor <= INT32_MAX)
		emit2(Op::Imul, TypeKind::I64, reg(q), imm(divisor));
	else
	{
		emit2(Op::Mov, TypeKind::Void, reg(kEmitScratch), imm(divisor));
		emit2(Op::Imul, TypeKind::I64, reg(q), reg(kEmitScratch));
	}
	const Reg r = q == RDX ? RAX : RDX;
	emit2(Op::Mov, TypeKind::Void, reg(r), reg(x));
	emit2(Op::Sub, TypeKind::I64, reg(r), reg(q));
	return r;
}

// Unsigned x / d (or x % d) by the magic multiplier: the quotient's (or
// remainder's) register, rax or rdx. x is in neither.
Reg FunctionSelector::emit_unsigned_magic_divide(Reg x, int64_t divisor, bool remainder)
{
	const UnsignedMagic magic = unsigned_magic(static_cast<uint64_t>(divisor));
	emit2(Op::Mov, TypeKind::Void, reg(RAX), reg(x));
	emit2(Op::Mov, TypeKind::Void, reg(kEmitScratch), imm(static_cast<int64_t>(magic.multiplier)));
	emit1(Op::MulWide, TypeKind::I64, reg(kEmitScratch));
	Reg q = RDX;
	if (!magic.add)
	{
		if (magic.shift != 0)
			emit2(Op::Shr, TypeKind::I64, reg(RDX), imm(magic.shift));
	}
	else
	{
		// ((x - t) >> 1) + t, then the remaining shift (65-bit magic).
		emit2(Op::Mov, TypeKind::Void, reg(RAX), reg(x));
		emit2(Op::Sub, TypeKind::I64, reg(RAX), reg(RDX));
		emit2(Op::Shr, TypeKind::I64, reg(RAX), imm(1));
		emit2(Op::Add, TypeKind::I64, reg(RAX), reg(RDX));
		if (magic.shift > 1)
			emit2(Op::Shr, TypeKind::I64, reg(RAX), imm(magic.shift - 1));
		q = RAX;
	}
	return remainder ? emit_magic_remainder(x, q, divisor) : q;
}

// Signed x / d (or x % d) by the magic multiplier: rax or rdx.
Reg FunctionSelector::emit_signed_magic_divide(Reg x, int64_t divisor, bool remainder)
{
	const SignedMagic magic = signed_magic(divisor);
	emit2(Op::Mov, TypeKind::Void, reg(RAX), reg(x));
	emit2(Op::Mov, TypeKind::Void, reg(kEmitScratch), imm(magic.multiplier));
	emit1(Op::ImulWide, TypeKind::I64, reg(kEmitScratch));
	if (divisor > 0 && magic.multiplier < 0)
		emit2(Op::Add, TypeKind::I64, reg(RDX), reg(x));
	else if (divisor < 0 && magic.multiplier > 0)
		emit2(Op::Sub, TypeKind::I64, reg(RDX), reg(x));
	if (magic.shift != 0)
		emit2(Op::Sar, TypeKind::I64, reg(RDX), imm(magic.shift));
	// Add one to a negative quotient (truncation toward zero).
	emit2(Op::Mov, TypeKind::Void, reg(RAX), reg(RDX));
	emit2(Op::Shr, TypeKind::I64, reg(RAX), imm(63));
	emit2(Op::Add, TypeKind::I64, reg(RDX), reg(RAX));
	return remainder ? emit_magic_remainder(x, RDX, divisor) : RDX;
}

} // namespace native
