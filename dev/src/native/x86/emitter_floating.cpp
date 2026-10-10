#include "native/x86/emitter.h"

#include <cstring>

#include "support/internal_error.h"

// Floating-point emission: f32/f64 on SSE scalar forms (memory and
// literal operands read from the read-only constant pool) except f64
// arithmetic, which, like f80, runs on the x87 stack through memory; and
// the integer/floating conversions.
namespace native {

using lowir::ComparePredicate;
using lowir::TypeKind;

namespace {

// movss, movsd; movups for a whole XMM register (an i128 move: binary128
// values at the call boundary).
uint8_t sse_prefix(TypeKind kind)
{
	return kind == TypeKind::F32 ? 0xf3 : kind == TypeKind::I128 ? 0 : 0xf2;
}

const Reg kX14 = static_cast<Reg>(XMM0 + 14);
const Reg kX15 = static_cast<Reg>(XMM0 + 15);

bool is_xmm_reg(const Operand& op)
{
	return op.kind == OperandKind::Reg && is_xmm(op.reg);
}

} // namespace

// st0 <- an f80 (or f32/f64 with `type`) memory operand or literal.
void FunctionEmitter::x87_load(const Operand& op, TypeKind type)
{
	const RM src = op.kind == OperandKind::FImm ? constant_rm(op) : rm(op);
	if (type == TypeKind::F80)
		asm_.x87(0xdb, 5, src);
	else if (type == TypeKind::F64)
		asm_.x87(0xdd, 0, src);
	else
		asm_.x87(0xd9, 0, src);
}

// A call's f80 result in st0 stored to memory, or discarded.
void FunctionEmitter::x87_result(const Inst& inst)
{
	if (inst.nops != 0)
	{
		asm_.x87(0xdb, 7, rm(inst.ops[0]));
		return;
	}
	asm_.byte(0xdd);
	asm_.byte(0xd8); // fstp st0
}

void FunctionEmitter::float_move(const Inst& inst)
{
	const Operand& dst = inst.ops[0];
	const Operand& src = inst.ops[1];
	const TypeKind kind = inst.type;
	if (kind == TypeKind::F80)
	{
		// A bit-exact 10-byte copy through the integer scratch.
		const Reg s = scratch(inst);
		const RM to = rm(dst);
		if (src.kind == OperandKind::FImm)
		{
			asm_.mov_ri(s, src.imm);
			asm_.mov_store(to, s, 8);
			asm_.mov_store_imm(to.displaced(8), src.imm_high & 0xffff, 2);
			return;
		}
		const RM from = rm(src);
		asm_.mov_load(s, from, 8, false);
		asm_.mov_store(to, s, 8);
		asm_.mov_load(s, from.displaced(8), 2, false);
		asm_.mov_store(to.displaced(8), s, 2);
		return;
	}
	const uint8_t prefix = sse_prefix(kind);
	if (is_xmm_reg(dst))
	{
		if (is_xmm_reg(src))
		{
			if (src.reg != dst.reg)
				asm_.sse(0, 0x28, dst.reg, RM::r(src.reg));
		}
		else if (src.kind == OperandKind::FImm)
		{
			if (src.imm == 0 && (kind != TypeKind::I128 || src.imm_high == 0))
				asm_.sse(0, 0x57, dst.reg, RM::r(dst.reg)); // xorps
			else if (kind == TypeKind::I128)
				throw InternalError("a 128-bit floating constant");
			else
				asm_.sse(prefix, 0x10, dst.reg, constant_rm(src));
		}
		else if (src.kind == OperandKind::Reg)
			throw InternalError("floating move from a general register");
		else
			asm_.sse(prefix, 0x10, dst.reg, rm(src));
		return;
	}
	const RM to = rm(dst);
	if (is_xmm_reg(src))
	{
		asm_.sse_store(prefix, 0x11, to, src.reg);
		return;
	}
	if (kind == TypeKind::I128)
		throw InternalError("a 128-bit floating move without an XMM register");
	const unsigned w = kind == TypeKind::F32 ? 4 : 8;
	if (src.kind == OperandKind::FImm)
	{
		if (w == 4 || fits_i32(src.imm))
			asm_.mov_store_imm(to, src.imm, w);
		else
		{
			const Reg s = scratch(inst);
			asm_.mov_ri(s, src.imm);
			asm_.mov_store(to, s, 8);
		}
		return;
	}
	const Reg s = scratch(inst);
	asm_.mov_load(s, rm(src), w, false);
	asm_.mov_store(to, s, w);
}

void FunctionEmitter::float_arith(const Inst& inst)
{
	const TypeKind kind = inst.type;
	const Operand& d = inst.ops[0];
	const Operand& a = inst.ops[1];
	const Operand& b = inst.ops[2];
	if (kind == TypeKind::F80)
	{
		x87_load(a, kind);
		if (inst.op == Op::FNeg)
		{
			asm_.byte(0xd9);
			asm_.byte(0xe0); // fchs
		}
		else
		{
			x87_load(b, kind);
			const uint8_t second = inst.op == Op::FAdd
				? 0xc1
				: (inst.op == Op::FSub ? 0xe9 : (inst.op == Op::FMul ? 0xc9 : 0xf9));
			asm_.byte(0xde);
			asm_.byte(second);
		}
		asm_.x87(0xdb, 7, rm(d));
		return;
	}
	const uint8_t prefix = sse_prefix(kind);
	auto source = [&](const Operand& op) { return op.kind == OperandKind::FImm ? constant_rm(op) : rm(op); };
	if (kind == TypeKind::F64 && inst.op != Op::FNeg)
	{
		// f64 arithmetic runs in x87 extended precision and is rounded to
		// double by the store, as the reference backend evaluates it (a
		// greater precision [expr]p11 allows; the PA25 fixtures pin it): a
		// in st0, b a memory operand, registers through the red zone.
		auto memory = [&](const Operand& op, int32_t below) {
			if (!is_xmm_reg(op))
				return source(op);
			asm_.sse_store(prefix, 0x11, RM::m(RSP, below), op.reg);
			return RM::m(RSP, below);
		};
		asm_.x87(0xdd, 0, memory(a, -8)); // fld qword
		const unsigned digit = inst.op == Op::FAdd ? 0 : (inst.op == Op::FMul ? 1 : (inst.op == Op::FSub ? 4 : 6));
		asm_.x87(0xdc, digit, memory(b, -16)); // fadd/fmul/fsub/fdiv qword
		if (is_xmm_reg(d))
		{
			asm_.x87(0xdd, 3, RM::m(RSP, -8)); // fstp qword
			asm_.sse(prefix, 0x10, d.reg, RM::m(RSP, -8));
		}
		else
			asm_.x87(0xdd, 3, rm(d));
		return;
	}
	if (inst.op == Op::FNeg)
	{
		uint8_t mask[16];
		std::memset(mask, 0, sizeof mask);
		if (kind == TypeKind::F32)
			mask[3] = 0x80;
		else
			mask[7] = 0x80;
		if (!(is_xmm_reg(a) && a.reg == d.reg))
		{
			if (is_xmm_reg(a))
				asm_.sse(0, 0x28, d.reg, RM::r(a.reg));
			else
				asm_.sse(prefix, 0x10, d.reg, source(a));
		}
		asm_.sse(0, 0x57, d.reg, RM::rip(image_.constant(mask, 16, 16), 0));
		return;
	}
	const uint8_t opcode = inst.op == Op::FAdd ? 0x58
											   : (inst.op == Op::FSub ? 0x5c : (inst.op == Op::FMul ? 0x59 : 0x5e));
	const bool commutative = inst.op == Op::FAdd || inst.op == Op::FMul;
	if (is_xmm_reg(a) && a.reg == d.reg)
	{
		asm_.sse(prefix, opcode, d.reg, source(b));
		return;
	}
	if (is_xmm_reg(b) && b.reg == d.reg)
	{
		if (commutative)
		{
			asm_.sse(prefix, opcode, d.reg, source(a));
			return;
		}
		// d = a - d: compute in scratch.
		if (is_xmm_reg(a))
			asm_.sse(0, 0x28, kX15, RM::r(a.reg));
		else
			asm_.sse(prefix, 0x10, kX15, source(a));
		asm_.sse(prefix, opcode, kX15, RM::r(b.reg));
		asm_.sse(0, 0x28, d.reg, RM::r(kX15));
		return;
	}
	if (is_xmm_reg(a))
		asm_.sse(0, 0x28, d.reg, RM::r(a.reg));
	else
		asm_.sse(prefix, 0x10, d.reg, source(a));
	asm_.sse(prefix, opcode, d.reg, source(b));
}

// Flags of `ucomis a, b` (a in a register).
void FunctionEmitter::float_compare(const Operand& a, const Operand& b, TypeKind kind)
{
	if (kind == TypeKind::F80)
	{
		x87_load(b, kind);
		x87_load(a, kind);
		asm_.byte(0xdf);
		asm_.byte(0xe9); // fucomip st0, st1
		asm_.byte(0xdd);
		asm_.byte(0xd8); // fstp st0
		return;
	}
	const uint8_t prefix = sse_prefix(kind);
	Reg left = kX15;
	if (is_xmm_reg(a))
		left = a.reg;
	else
		asm_.sse(prefix, 0x10, kX15, a.kind == OperandKind::FImm ? constant_rm(a) : rm(a));
	const RM right = b.kind == OperandKind::FImm ? constant_rm(b) : rm(b);
	asm_.sse(kind == TypeKind::F32 ? 0 : 0x66, 0x2e, left, right);
}

void FunctionEmitter::float_setcc(const Inst& inst)
{
	const ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
	const Reg d = inst.ops[0].reg;
	const bool swap = pred == ComparePredicate::Lt || pred == ComparePredicate::Le || pred == ComparePredicate::Ugt ||
		pred == ComparePredicate::Uge;
	float_compare(swap ? inst.ops[2] : inst.ops[1], swap ? inst.ops[1] : inst.ops[2], inst.type);
	const Reg s = d == R11 ? R10 : R11;
	switch (pred)
	{
	case ComparePredicate::Eq:
		asm_.setcc(CC_E, d);
		asm_.setcc(CC_NP, s);
		asm_.alu_rr(4, d, RM::r(s), 1);
		break;
	case ComparePredicate::Ne:
		asm_.setcc(CC_NE, d);
		asm_.setcc(CC_P, s);
		asm_.alu_rr(1, d, RM::r(s), 1);
		break;
	case ComparePredicate::Gt:
	case ComparePredicate::Lt:
		asm_.setcc(CC_A, d);
		break;
	case ComparePredicate::Ge:
	case ComparePredicate::Le:
		asm_.setcc(CC_AE, d);
		break;
	case ComparePredicate::Ult:
	case ComparePredicate::Ugt:
		asm_.setcc(CC_B, d);
		break;
	default:
		asm_.setcc(CC_BE, d);
		break;
	}
	asm_.movzx_rr8(d, d);
}

// An i128/floating conversion: a call of the runtime's routine (libgcc's
// convention: the i128 in rdi:rsi or rax:rdx, an f32/f64 in xmm0, an f80
// result in st0 and an f80 argument in 16 bytes at the stack pointer); the
// selector counts every caller-saved register clobbered.
void FunctionEmitter::wide_float_convert(const Inst& inst)
{
	const ConvKind kind = static_cast<ConvKind>(inst.sub);
	const TypeKind from = inst.type;
	const TypeKind to = inst.type2;
	const Operand& d = inst.ops[0];
	const Operand& s = inst.ops[1];
	const bool to_float = from == TypeKind::I128;
	const TypeKind floating = to_float ? to : from;
	const unsigned routine = (to_float ? 0u : 6u) + (kind == ConvKind::UIToFP || kind == ConvKind::FPToUI ? 3u : 0u) +
		(floating == TypeKind::F32          ? 0u
				: floating == TypeKind::F64 ? 1u
											: 2u);
	if (to_float)
	{
		wide_half(RDI, s, 0);
		wide_half(RSI, s, 1);
		asm_.call_symbol(runtime_.float_conversion[routine]);
		if (to == TypeKind::F80)
			asm_.x87(0xdb, 7, rm(d)); // fstp tbyte
		else if (d.reg != XMM0)
			asm_.sse(0, 0x28, d.reg, RM::r(XMM0));
		return;
	}
	if (from == TypeKind::F80)
	{
		x87_load(s, from);
		asm_.alu_ri(5, RM::r(RSP), 16, 8);
		asm_.x87(0xdb, 7, RM::m(RSP, 0));
		asm_.call_symbol(runtime_.float_conversion[routine]);
		asm_.alu_ri(0, RM::r(RSP), 16, 8);
	}
	else
	{
		if (!is_xmm_reg(s) || s.reg != XMM0)
			asm_.sse(sse_prefix(from), 0x10, XMM0, s.kind == OperandKind::FImm ? constant_rm(s) : rm(s));
		asm_.call_symbol(runtime_.float_conversion[routine]);
	}
	asm_.mov_store(rm(d), RAX, 8);
	asm_.mov_store(rm(d).displaced(8), RDX, 8);
}

void FunctionEmitter::convert(const Inst& inst)
{
	const ConvKind kind = static_cast<ConvKind>(inst.sub);
	const TypeKind from = inst.type;
	const TypeKind to = inst.type2;
	const Operand& d = inst.ops[0];
	const Operand& s = inst.ops[1];
	auto source = [&](const Operand& op) { return op.kind == OperandKind::FImm ? constant_rm(op) : rm(op); };
	if (from == TypeKind::I128 || to == TypeKind::I128)
	{
		wide_float_convert(inst);
		return;
	}
	switch (kind)
	{
	case ConvKind::SIToFP:
		if (to == TypeKind::F80)
		{
			asm_.mov_store(scratch_rm(), s.reg, 8);
			asm_.x87(0xdf, 5, scratch_rm());
			asm_.x87(0xdb, 7, rm(d));
			return;
		}
		asm_.sse(0, 0x57, d.reg, RM::r(d.reg));
		asm_.sse(sse_prefix(to), 0x2a, d.reg, RM::r(s.reg), true);
		return;
	case ConvKind::UIToFP:
	{
		if (to == TypeKind::F80)
		{
			asm_.mov_store(scratch_rm(), s.reg, 8);
			asm_.x87(0xdf, 5, scratch_rm());
			asm_.op(0, true, 0x85, 1, s.reg, RM::r(s.reg));
			const uint32_t done = asm_.new_label();
			asm_.jcc_short(CC_NS, done);
			const float two64 = 18446744073709551616.0f;
			asm_.x87(0xd8, 0, RM::rip(image_.constant(&two64, 4, 4), 0));
			asm_.bind(done);
			asm_.x87(0xdb, 7, rm(d));
			return;
		}
		const uint8_t prefix = sse_prefix(to);
		const uint32_t negative = asm_.new_label();
		const uint32_t done = asm_.new_label();
		asm_.op(0, true, 0x85, 1, s.reg, RM::r(s.reg));
		asm_.jcc_short(CC_S, negative);
		asm_.sse(0, 0x57, d.reg, RM::r(d.reg));
		asm_.sse(prefix, 0x2a, d.reg, RM::r(s.reg), true);
		asm_.jmp_short(done);
		asm_.bind(negative);
		Reg t1 = R11;
		Reg t2 = R10;
		if (s.reg == R11)
		{
			t1 = R10;
			t2 = R11;
		}
		else if (s.reg == R10)
		{
			t1 = R11;
			t2 = R10;
		}
		asm_.mov_rr(t1, s.reg);
		asm_.shift_ri(5, RM::r(t1), 1, 8);
		if (t2 != s.reg)
			asm_.mov_rr(t2, s.reg);
		asm_.alu_ri(4, RM::r(t2), 1, 4);
		asm_.alu_rr(1, t1, RM::r(t2), 8);
		asm_.sse(0, 0x57, d.reg, RM::r(d.reg));
		asm_.sse(prefix, 0x2a, d.reg, RM::r(t1), true);
		asm_.sse(prefix, 0x58, d.reg, RM::r(d.reg));
		asm_.bind(done);
		return;
	}
	case ConvKind::FPToSI:
		if (from == TypeKind::F80)
		{
			x87_load(s, from);
			asm_.x87(0xdd, 1, scratch_rm()); // fisttp qword
			asm_.mov_load(d.reg, scratch_rm(), 8, false);
			return;
		}
		asm_.sse(sse_prefix(from), 0x2c, d.reg, source(s), true);
		return;
	case ConvKind::FPToUI:
	{
		const uint32_t small = asm_.new_label();
		const uint32_t done = asm_.new_label();
		if (from == TypeKind::F80)
		{
			const float two63 = 9223372036854775808.0f;
			const RM limit = RM::rip(image_.constant(&two63, 4, 4), 0);
			x87_load(s, from);
			asm_.x87(0xd9, 0, limit);
			asm_.byte(0xdf);
			asm_.byte(0xe9); // fucomip: 2^63 vs x, pops 2^63
			asm_.jcc_short(CC_A, small);
			asm_.x87(0xd8, 4, limit); // fsub 2^63
			asm_.x87(0xdd, 1, scratch_rm());
			asm_.mov_load(d.reg, scratch_rm(), 8, false);
			asm_.op(0, true, 0x0fba, 2, 7, RM::r(d.reg), false, 1);
			asm_.imm(63, 1); // btc d, 63
			asm_.jmp_short(done);
			asm_.bind(small);
			asm_.x87(0xdd, 1, scratch_rm());
			asm_.mov_load(d.reg, scratch_rm(), 8, false);
			asm_.bind(done);
			return;
		}
		const uint8_t prefix = sse_prefix(from);
		Reg x = kX15;
		if (is_xmm_reg(s))
			x = s.reg;
		else
			asm_.sse(prefix, 0x10, kX15, source(s));
		uint8_t limit_bytes[8];
		if (from == TypeKind::F32)
		{
			const float two63 = 9223372036854775808.0f;
			std::memcpy(limit_bytes, &two63, 4);
		}
		else
		{
			const double two63 = 9223372036854775808.0;
			std::memcpy(limit_bytes, &two63, 8);
		}
		const uint32_t size = from == TypeKind::F32 ? 4 : 8;
		asm_.sse(prefix, 0x10, kX14, RM::rip(image_.constant(limit_bytes, size, size), 0));
		asm_.sse(from == TypeKind::F32 ? 0 : 0x66, 0x2e, x, RM::r(kX14));
		asm_.jcc_short(CC_B, small);
		if (x != kX15)
			asm_.sse(0, 0x28, kX15, RM::r(x));
		asm_.sse(prefix, 0x5c, kX15, RM::r(kX14));
		asm_.sse(prefix, 0x2c, d.reg, RM::r(kX15), true);
		asm_.op(0, true, 0x0fba, 2, 7, RM::r(d.reg), false, 1);
		asm_.imm(63, 1);
		asm_.jmp_short(done);
		asm_.bind(small);
		asm_.sse(prefix, 0x2c, d.reg, RM::r(x), true);
		asm_.bind(done);
		return;
	}
	case ConvKind::FPExt:
		if (to == TypeKind::F80)
		{
			if (is_xmm_reg(s))
			{
				asm_.sse_store(sse_prefix(from), 0x11, scratch_rm(), s.reg);
				x87_load(Operand::make_frame(function_->scratch_offset), from);
			}
			else
				x87_load(s, from);
			asm_.x87(0xdb, 7, rm(d));
			return;
		}
		asm_.sse(0xf3, 0x5a, d.reg, source(s));
		return;
	case ConvKind::FPTrunc:
		if (from == TypeKind::F80)
		{
			x87_load(s, from);
			asm_.x87(to == TypeKind::F64 ? 0xdd : 0xd9, 3, scratch_rm());
			asm_.sse(sse_prefix(to), 0x10, d.reg, scratch_rm());
			return;
		}
		asm_.sse(0xf2, 0x5a, d.reg, source(s));
		return;
	}
}

} // namespace native
