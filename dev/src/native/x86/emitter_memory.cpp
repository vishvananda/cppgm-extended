#include "native/x86/emitter.h"

#include <stdexcept>

#include "native/mir/type_kinds.h"
#include "native/runtime/eh_record.h"
#include "native/select/bulk_policy.h"

// Memory pseudo instructions: bulk copies and zeroing, i128 arithmetic and
// comparisons, the 16-byte compare-exchange, va_arg, and the handler-stack
// exception operations.
namespace native {

using lowir::ComparePredicate;
using lowir::TypeKind;

namespace {

const Reg kX15 = static_cast<Reg>(XMM0 + 15);

// The bytes of a ModRM memory operand beyond the ModRM byte itself.
unsigned rm_extra(const RM& rm)
{
	if (rm.is_reg)
		return 0;
	if (rm.symbol != 0xffffffffu || rm.label != 0xffffffffu || rm.base == kNoReg)
		return rm.base == kNoReg && rm.symbol == 0xffffffffu && rm.label == 0xffffffffu ? 5 : 4;
	unsigned extra = (rm.index != kNoReg || (rm.base & 7) == 4) ? 1 : 0;
	if (rm.disp == 0 && (rm.base & 7) != 5)
		return extra;
	return extra + (fits_i8(rm.disp) ? 1 : 4);
}

bool needs_rex(const RM& rm)
{
	return !rm.is_reg && ((rm.base != kNoReg && (rm.base & 8)) || (rm.index != kNoReg && (rm.index & 8)));
}

// Encoded bytes of `mov [rm], 0` at width `w`.
unsigned zero_store_bytes(const RM& rm, unsigned w)
{
	const unsigned imm = w == 8 ? 4 : w;
	return (w == 2 ? 1 : 0) + ((w == 8 || needs_rex(rm)) ? 1 : 0) + 2 + rm_extra(rm) + imm;
}

Cond strict_high(ComparePredicate pred)
{
	switch (pred)
	{
	case ComparePredicate::Lt:
	case ComparePredicate::Le:
		return CC_L;
	case ComparePredicate::Gt:
	case ComparePredicate::Ge:
		return CC_G;
	case ComparePredicate::Ult:
	case ComparePredicate::Ule:
		return CC_B;
	default:
		return CC_A;
	}
}

Cond low_cond(ComparePredicate pred)
{
	switch (pred)
	{
	case ComparePredicate::Lt:
	case ComparePredicate::Ult:
		return CC_B;
	case ComparePredicate::Le:
	case ComparePredicate::Ule:
		return CC_BE;
	case ComparePredicate::Gt:
	case ComparePredicate::Ugt:
		return CC_A;
	default:
		return CC_AE;
	}
}

} // namespace

// ---------------------------------------------------------------------------
// Bulk memory

// The widest chunk a direct copy may load from `src` without reading bytes
// that narrower stores of the same block just wrote: a load spanning
// several stores, or wider than one, cannot take its data from the store
// buffer and waits for them to commit (a store-forwarding stall, ~12+
// cycles). A call ends the window; chunks stay at least 4 bytes wide.
unsigned FunctionEmitter::copy_chunk_limit(const Operand& src, uint32_t bytes) const
{
	const uint32_t kWindow = 16;
	const Reg base = src.kind == OperandKind::Reg ? src.reg : (src.kind == OperandKind::Mem ? src.reg : kNoReg);
	if (base == kNoReg || (src.kind == OperandKind::Mem && (src.symbol != lowir::kNoId || src.index != kNoReg)))
		return 16;
	const int32_t disp = src.kind == OperandKind::Mem ? src.disp : 0;
	unsigned limit = 16;
	const uint32_t stop = inst_index_ > block_first_ + kWindow ? inst_index_ - kWindow : block_first_;
	for (uint32_t k = inst_index_; k-- > stop;)
	{
		const Inst& prior = function_->insts[k];
		if (prior.op == Op::Call)
			break;
		// A prior copy or zeroing writes 8- or 16-byte chunks: 8-byte loads
		// forward from either.
		const bool bulk = (prior.op == Op::CopyBytes || prior.op == Op::ZeroBytes) && !(prior.flags & MF_COPY_DYNAMIC);
		if (!bulk && ((prior.op != Op::Store && prior.op != Op::FMov) || prior.ops[0].kind != OperandKind::Mem))
			continue;
		const Operand& at = prior.ops[0];
		if (at.kind == OperandKind::Reg ? at.reg != base || disp != 0
										: (at.reg != base || at.index != kNoReg || at.symbol != lowir::kNoId))
			continue;
		const unsigned width = bulk                                      ? (prior.aux >= 8 ? 8 : 4)
			: prior.type == TypeKind::Ptr || prior.type == TypeKind::F64 ? 8
																		 : register_bytes(prior.type);
		const int32_t extent = bulk ? static_cast<int32_t>(prior.aux) : static_cast<int32_t>(width);
		if (width < limit && at.disp < disp + static_cast<int32_t>(bytes) && disp < at.disp + extent)
			limit = width < 4 ? 4 : width;
		continue;
	}
	return limit;
}

void FunctionEmitter::copy_bytes(const Inst& inst)
{
	if (inst.flags & MF_COPY_DYNAMIC)
	{
		asm_.mov_rr(RCX, RDX);
		asm_.byte(0xf3); // rep movsb
		asm_.byte(0xa4);
		return;
	}
	const uint32_t bytes = inst.aux;
	const uint32_t align = inst.aux2;
	const Operand& dst = inst.ops[0];
	const Operand& src = inst.ops[1];
	auto location = [&](const Operand& op) { return op.kind == OperandKind::Reg ? RM::m(op.reg, 0) : rm(op); };
	if (direct_copy(bytes, align, function_->wide_bulk))
	{
		RM to = location(dst);
		RM from = location(src);
		uint32_t offset = 0;
		const unsigned limit = function_->wide_bulk ? copy_chunk_limit(src, bytes) : 16;
		while (limit >= 16 && bytes - offset >= 16)
		{
			asm_.sse(0xf3, 0x6f, kX15, from.displaced(static_cast<int32_t>(offset)));
			asm_.sse_store(0xf3, 0x7f, to.displaced(static_cast<int32_t>(offset)), kX15);
			offset += 16;
		}
		if (offset == bytes)
			return;
		Reg s = kNoReg;
		const Reg candidates[2] = { R11, R10 };
		for (unsigned c = 0; c < 2 && s == kNoReg; ++c)
			if (!to.uses(candidates[c]) && !from.uses(candidates[c]))
				s = candidates[c];
		// With both scratch registers in the addresses, a register neither
		// address uses (two addresses name at most two others) is borrowed
		// on the stack; an rsp-based address (an outgoing argument) moves
		// with the push.
		const bool saved = s == kNoReg;
		if (saved)
		{
			const Reg spares[3] = { RAX, RCX, RDX };
			for (unsigned c = 0; c < 3 && s == kNoReg; ++c)
				if (!to.uses(spares[c]) && !from.uses(spares[c]))
					s = spares[c];
			asm_.push(s);
			if (to.base == RSP)
				to.disp += 8;
			if (from.base == RSP)
				from.disp += 8;
		}
		const unsigned widths[4] = { 8, 4, 2, 1 };
		for (unsigned k = 0; k < 4; ++k)
		{
			while (widths[k] <= limit && bytes - offset >= widths[k])
			{
				asm_.mov_load(s, from.displaced(static_cast<int32_t>(offset)), widths[k], false);
				asm_.mov_store(to.displaced(static_cast<int32_t>(offset)), s, widths[k]);
				offset += widths[k];
			}
		}
		if (saved)
			asm_.pop(s);
		return;
	}
	// rep movsb: rdi <- destination, rsi <- source, without losing either.
	const bool src_uses_rdi = src.uses_reg(RDI);
	const bool dst_uses_rsi = dst.uses_reg(RSI);
	if (!src_uses_rdi)
	{
		address_into(RDI, dst);
		address_into(RSI, src);
	}
	else if (!dst_uses_rsi)
	{
		address_into(RSI, src);
		address_into(RDI, dst);
	}
	else
	{
		address_into(RCX, dst);
		address_into(RSI, src);
		asm_.mov_rr(RDI, RCX);
	}
	asm_.mov_ri(RCX, bytes);
	asm_.byte(0xf3);
	asm_.byte(0xa4);
}

void FunctionEmitter::zero_bytes(const Inst& inst)
{
	const uint32_t bytes = inst.aux;
	const Operand& dst = inst.ops[0];
	const RM to = dst.kind == OperandKind::Reg ? RM::m(dst.reg, 0) : rm(dst);
	if (bytes == 16 || (function_->wide_bulk && bytes > 16 && bytes <= 128))
	{
		// 16-byte chunks of a cleared xmm15, then scalar stores.
		asm_.op(0x66, false, 0x0fef, 2, kX15 & 15, RM::r(kX15)); // pxor xmm15, xmm15
		uint32_t offset = 0;
		for (; bytes - offset >= 16; offset += 16)
			asm_.sse_store(0xf3, 0x7f, to.displaced(static_cast<int32_t>(offset)), kX15);
		const unsigned widths[4] = { 8, 4, 2, 1 };
		for (unsigned k = 0; k < 4; ++k)
			for (; bytes - offset >= widths[k]; offset += widths[k])
				asm_.mov_store_imm(to.displaced(static_cast<int32_t>(offset)), 0, widths[k]);
		return;
	}
	unsigned direct = 0;
	{
		uint32_t offset = 0;
		const unsigned widths[4] = { 8, 4, 2, 1 };
		for (unsigned k = 0; k < 4; ++k)
			while (bytes - offset >= widths[k])
			{
				direct += zero_store_bytes(to.displaced(static_cast<int32_t>(offset)), widths[k]);
				offset += widths[k];
			}
	}
	const unsigned setup = (dst.kind == OperandKind::Reg ? (dst.reg == RDI ? 0 : 3) : 3 + rm_extra(to) + 1) + 2 +
		(bytes <= 0xffffffffu ? 5 : 10) + 2;
	if (always_direct_zero(bytes, function_->wide_bulk) || direct <= setup)
	{
		uint32_t offset = 0;
		const unsigned widths[4] = { 8, 4, 2, 1 };
		for (unsigned k = 0; k < 4; ++k)
			while (bytes - offset >= widths[k])
			{
				asm_.mov_store_imm(to.displaced(static_cast<int32_t>(offset)), 0, widths[k]);
				offset += widths[k];
			}
		return;
	}
	address_into(RDI, dst);
	asm_.op(0, false, 0x33, 1, RAX, RM::r(RAX)); // xor eax, eax
	asm_.mov_ri(RCX, bytes);
	asm_.byte(0xf3);
	asm_.byte(0xaa);
}

// ---------------------------------------------------------------------------
// i128

void FunctionEmitter::wide_half(Reg dst, const Operand& op, unsigned half)
{
	if (op.kind == OperandKind::Imm)
		move_imm(dst, half ? op.imm_high : op.imm, true);
	else if (op.kind == OperandKind::Reg)
		asm_.mov_rr(dst, op.reg);
	else
		asm_.mov_load(dst, rm(op).displaced(8 * static_cast<int32_t>(half)), 8, false);
}

void FunctionEmitter::wide_alu(unsigned digit, Reg dst, const Operand& op, unsigned half)
{
	if (op.kind == OperandKind::Imm)
	{
		const int64_t value = half ? op.imm_high : op.imm;
		if (fits_i32(value))
			asm_.alu_ri(digit, RM::r(dst), value, 8);
		else
		{
			const Reg other = dst == R10 ? R11 : R10;
			move_imm(other, value, true);
			asm_.alu_rr(digit, dst, RM::r(other), 8);
		}
		return;
	}
	asm_.alu_rr(digit, dst, rm(op).displaced(8 * static_cast<int32_t>(half)), 8);
}

void FunctionEmitter::wide(const Inst& inst)
{
	const WideOp op = static_cast<WideOp>(inst.sub);
	const Operand& d = inst.ops[0];
	const Operand& a = inst.ops[1];
	const Operand& b = inst.ops[2];
	const RM to = rm(d);
	switch (op)
	{
	case WideOp::Move:
		if (inst.flags & MF_VOLATILE)
		{
			// An atomic 16-byte load: compare-exchange with zero.
			asm_.lea(R11, rm(a));
			asm_.mov_store(scratch_rm(), RBX, 8);
			asm_.op(0, false, 0x33, 1, RAX, RM::r(RAX));
			asm_.op(0, false, 0x33, 1, RDX, RM::r(RDX));
			asm_.op(0, false, 0x33, 1, RCX, RM::r(RCX));
			asm_.op(0, false, 0x33, 1, RBX, RM::r(RBX));
			asm_.byte(0xf0);
			asm_.op(0, true, 0x0fc7, 2, 1, RM::m(R11, 0));
			asm_.mov_load(RBX, scratch_rm(), 8, false);
			asm_.mov_store(to, RAX, 8);
			asm_.mov_store(to.displaced(8), RDX, 8);
			return;
		}
		for (unsigned half = 0; half < 2; ++half)
		{
			wide_half(R11, a, half);
			asm_.mov_store(to.displaced(8 * static_cast<int32_t>(half)), R11, 8);
		}
		return;
	case WideOp::Add:
	case WideOp::Sub:
	case WideOp::And:
	case WideOp::Or:
	case WideOp::Xor:
	{
		static const unsigned low_digit[] = { 0, 0, 5, 0, 4, 1, 6 };
		static const unsigned high_digit[] = { 0, 2, 3, 0, 4, 1, 6 };
		const unsigned index = static_cast<unsigned>(op);
		wide_half(R11, a, 0);
		wide_alu(low_digit[index], R11, b, 0);
		asm_.mov_store(to, R11, 8);
		wide_half(R11, a, 1);
		wide_alu(high_digit[index], R11, b, 1);
		asm_.mov_store(to.displaced(8), R11, 8);
		return;
	}
	case WideOp::Div:
	case WideOp::UDiv:
	case WideOp::Mod:
	case WideOp::UMod:
		// A call: the selector counts every caller-saved register clobbered.
		wide_half(RDI, a, 0);
		wide_half(RSI, a, 1);
		wide_half(RDX, b, 0);
		wide_half(RCX, b, 1);
		asm_.call_symbol(runtime_.wide_division[static_cast<unsigned>(op) - static_cast<unsigned>(WideOp::Div)]);
		asm_.mov_store(to, RAX, 8);
		asm_.mov_store(to.displaced(8), RDX, 8);
		return;
	case WideOp::Mul:
		wide_half(RAX, a, 0);
		if (b.kind == OperandKind::Imm)
		{
			move_imm(R11, b.imm, true);
			asm_.unary(4, RM::r(R11), 8);
		}
		else
			asm_.unary(4, rm(b), 8);
		wide_half(R11, a, 0);
		if (b.kind == OperandKind::Imm)
		{
			move_imm(R10, b.imm_high, true);
			asm_.imul_rr(R11, RM::r(R10), 8);
		}
		else
			asm_.imul_rr(R11, rm(b).displaced(8), 8);
		asm_.alu_rr(0, RDX, RM::r(R11), 8);
		wide_half(R11, a, 1);
		if (b.kind == OperandKind::Imm)
		{
			move_imm(R10, b.imm, true);
			asm_.imul_rr(R11, RM::r(R10), 8);
		}
		else
			asm_.imul_rr(R11, rm(b), 8);
		asm_.alu_rr(0, RDX, RM::r(R11), 8);
		asm_.mov_store(to, RAX, 8);
		asm_.mov_store(to.displaced(8), RDX, 8);
		return;
	case WideOp::Shl:
	case WideOp::Shr:
	case WideOp::Sar:
	{
		wide_half(R10, a, 0);
		wide_half(R11, a, 1);
		const bool left = op == WideOp::Shl;
		if (b.kind == OperandKind::Imm)
		{
			const unsigned k = static_cast<unsigned>(b.imm & 127);
			if (k >= 64)
			{
				if (left)
				{
					asm_.mov_rr(R11, R10);
					if (k > 64)
						asm_.shift_ri(4, RM::r(R11), k - 64, 8);
					asm_.op(0, false, 0x33, 1, R10, RM::r(R10));
				}
				else
				{
					asm_.mov_rr(R10, R11);
					if (k > 64)
						asm_.shift_ri(op == WideOp::Sar ? 7 : 5, RM::r(R10), k - 64, 8);
					if (op == WideOp::Sar)
						asm_.shift_ri(7, RM::r(R11), 63, 8);
					else
						asm_.op(0, false, 0x33, 1, R11, RM::r(R11));
				}
			}
			else if (k != 0)
			{
				if (left)
				{
					asm_.op(0, true, 0x0fa4, 2, R10, RM::r(R11), false, 1); // shld r11, r10, k
					asm_.imm(k, 1);
					asm_.shift_ri(4, RM::r(R10), k, 8);
				}
				else
				{
					asm_.op(0, true, 0x0fac, 2, R11, RM::r(R10), false, 1); // shrd r10, r11, k
					asm_.imm(k, 1);
					asm_.shift_ri(op == WideOp::Sar ? 7 : 5, RM::r(R11), k, 8);
				}
			}
		}
		else
		{
			const uint32_t done = asm_.new_label();
			if (left)
			{
				asm_.op(0, true, 0x0fa5, 2, R10, RM::r(R11)); // shld r11, r10, cl
				asm_.shift_cl(4, RM::r(R10), 8);
			}
			else
			{
				asm_.op(0, true, 0x0fad, 2, R11, RM::r(R10)); // shrd r10, r11, cl
				asm_.shift_cl(op == WideOp::Sar ? 7 : 5, RM::r(R11), 8);
			}
			asm_.op(0, false, 0xf6, 1, 0, RM::r(RCX), true, 1);
			asm_.imm(64, 1); // test cl, 64
			asm_.jcc_short(CC_E, done);
			if (left)
			{
				asm_.mov_rr(R11, R10);
				asm_.op(0, false, 0x33, 1, R10, RM::r(R10));
			}
			else
			{
				asm_.mov_rr(R10, R11);
				if (op == WideOp::Sar)
					asm_.shift_ri(7, RM::r(R11), 63, 8);
				else
					asm_.op(0, false, 0x33, 1, R11, RM::r(R11));
			}
			asm_.bind(done);
		}
		asm_.mov_store(to, R10, 8);
		asm_.mov_store(to.displaced(8), R11, 8);
		return;
	}
	case WideOp::Neg:
	case WideOp::Not:
		wide_half(R10, a, 0);
		wide_half(R11, a, 1);
		if (op == WideOp::Neg)
		{
			asm_.unary(3, RM::r(R10), 8);
			asm_.alu_ri(2, RM::r(R11), 0, 8);
			asm_.unary(3, RM::r(R11), 8);
		}
		else
		{
			asm_.unary(2, RM::r(R10), 8);
			asm_.unary(2, RM::r(R11), 8);
		}
		asm_.mov_store(to, R10, 8);
		asm_.mov_store(to.displaced(8), R11, 8);
		return;
	case WideOp::SExt:
		asm_.mov_rr(R11, a.reg);
		asm_.mov_store(to, R11, 8);
		asm_.shift_ri(7, RM::r(R11), 63, 8);
		asm_.mov_store(to.displaced(8), R11, 8);
		return;
	case WideOp::ZExt:
		asm_.mov_store(to, a.reg, 8);
		asm_.mov_store_imm(to.displaced(8), 0, 8);
		return;
	}
}

void FunctionEmitter::wide_compare(const Inst& inst)
{
	const bool branching = inst.op == Op::WideCmpBranch;
	const Operand& a = branching ? inst.ops[0] : inst.ops[1];
	const Operand& b = branching ? inst.ops[1] : inst.ops[2];
	const ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
	const Reg d = branching ? kNoReg : inst.ops[0].reg;
	const uint32_t on_true = branching ? labels_[inst.ops[2].symbol] : 0;
	const uint32_t on_false = branching ? labels_[inst.aux] : 0;
	if (pred == ComparePredicate::Eq || pred == ComparePredicate::Ne)
	{
		// Low words first; equal low words leave the high-word flags.
		const Cond cond = pred == ComparePredicate::Eq ? CC_E : CC_NE;
		wide_half(R11, a, 0);
		wide_alu(7, R11, b, 0);
		if (branching)
		{
			branch(CC_NE, true, pred == ComparePredicate::Eq ? on_false : on_true);
			wide_half(R11, a, 1);
			wide_alu(7, R11, b, 1);
			branch(cond, true, on_true);
			branch(CC_O, false, on_false);
			return;
		}
		const uint32_t done = asm_.new_label();
		asm_.jcc_short(CC_NE, done);
		wide_half(R11, a, 1);
		wide_alu(7, R11, b, 1);
		asm_.bind(done);
		asm_.setcc(cond, d);
		asm_.movzx_rr8(d, d);
		return;
	}
	wide_half(R11, a, 1);
	wide_alu(7, R11, b, 1);
	if (branching)
	{
		branch(strict_high(pred), true, on_true);
		branch(CC_NE, true, on_false);
		wide_half(R11, a, 0);
		wide_alu(7, R11, b, 0);
		branch(low_cond(pred), true, on_true);
		branch(CC_O, false, on_false);
		return;
	}
	const uint32_t high = asm_.new_label();
	const uint32_t done = asm_.new_label();
	asm_.jcc_short(CC_NE, high);
	wide_half(R11, a, 0);
	wide_alu(7, R11, b, 0);
	asm_.setcc(low_cond(pred), d);
	asm_.jmp_short(done);
	asm_.bind(high);
	asm_.setcc(strict_high(pred), d);
	asm_.bind(done);
	asm_.movzx_rr8(d, d);
}

void FunctionEmitter::cmpxchg16b(const Inst& inst)
{
	const Reg d = static_cast<Reg>(inst.aux);
	const Operand& desired = inst.ops[2];
	address_into(RCX, inst.ops[1]);
	address_into(R11, inst.ops[0]);
	asm_.mov_rr(R10, RCX);
	asm_.mov_store(scratch_rm(), RBX, 8);
	asm_.mov_load(RAX, RM::m(R10, 0), 8, false);
	asm_.mov_load(RDX, RM::m(R10, 8), 8, false);
	wide_half(RBX, desired, 0);
	wide_half(RCX, desired, 1);
	asm_.byte(0xf0);
	asm_.op(0, true, 0x0fc7, 2, 1, RM::m(R11, 0));
	asm_.mov_store(RM::m(R10, 0), RAX, 8);
	asm_.mov_store(RM::m(R10, 8), RDX, 8);
	asm_.mov_load(RBX, scratch_rm(), 8, false);
	asm_.setcc(CC_E, d);
	asm_.movzx_rr8(d, d);
}

// ---------------------------------------------------------------------------
// va_arg and exceptions

void FunctionEmitter::variadic_argument(const Inst& inst)
{
	if (inst.type == TypeKind::Obj)
	{
		va_arg_object(inst);
		return;
	}
	// A binary16 or binary128 value (f32 or i128 here) arrives in its
	// memory destination through the scratch XMM register.
	const bool to_memory = inst.ops[0].kind != OperandKind::Reg;
	const Reg d = to_memory ? static_cast<Reg>(XMM0 + 15) : inst.ops[0].reg;
	const bool wide = inst.type == TypeKind::I128;
	const bool floating = inst.type == TypeKind::F32 || inst.type == TypeKind::F64 || wide;
	// An integer result without a register arrives in r10 or r11, which
	// the sequence uses: rax, saved around it, forms the address instead.
	const bool scratch_result = !floating && (d == R10 || d == R11);
	const Reg address = floating ? R10 : (scratch_result ? RAX : d);
	asm_.lea(R11, rm(inst.ops[1]));
	const Reg offset_reg = floating ? RAX : R10;
	if (floating || scratch_result)
		asm_.push(RAX);
	const int32_t field = floating ? 4 : 0;
	const int32_t limit = floating ? 176 : 48;
	const int32_t step = floating ? 16 : 8;
	const uint32_t stack = asm_.new_label();
	const uint32_t load = asm_.new_label();
	asm_.mov_load(offset_reg, RM::m(R11, field), 4, false);
	asm_.alu_ri(7, RM::r(offset_reg), limit, 4);
	asm_.jcc_short(CC_AE, stack);
	asm_.mov_load(address, RM::m(R11, 16), 8, false);
	asm_.alu_rr(0, address, RM::r(offset_reg), 8);
	asm_.alu_ri(0, RM::r(offset_reg), step, 4);
	asm_.mov_store(RM::m(R11, field), offset_reg, 4);
	asm_.jmp_short(load);
	asm_.bind(stack);
	asm_.mov_load(address, RM::m(R11, 8), 8, false);
	if (wide)
	{
		// A 16-aligned argument's overflow slot is aligned (psABI 3.5.7).
		asm_.alu_ri(0, RM::r(address), 15, 8);
		asm_.alu_ri(4, RM::r(address), -16, 8);
	}
	asm_.lea(offset_reg, RM::m(address, wide ? 16 : 8));
	asm_.mov_store(RM::m(R11, 8), offset_reg, 8);
	asm_.bind(load);
	if (floating)
	{
		asm_.pop(RAX);
		const uint8_t prefix = inst.type == TypeKind::F32 ? 0xf3 : wide ? 0 : 0xf2;
		asm_.sse(prefix, 0x10, d, RM::m(address, 0));
		if (to_memory)
			asm_.sse_store(prefix, 0x11, rm(inst.ops[0]), d);
		return;
	}
	const TypeKind kind = inst.type;
	const bool sign = kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32 || kind == TypeKind::I64 ||
		kind == TypeKind::Ptr;
	const unsigned width = kind == TypeKind::I8 || kind == TypeKind::U8 || kind == TypeKind::I1
		? 1
		: (kind == TypeKind::I16 || kind == TypeKind::U16 ? 2
														  : (kind == TypeKind::I32 || kind == TypeKind::U32 ? 4 : 8));
	asm_.mov_load(d, RM::m(address, 0), width, sign);
	if (scratch_result)
		asm_.pop(RAX);
}

void FunctionEmitter::va_copy_bytes(const RM& home, int32_t source, uint32_t offset, uint32_t bytes)
{
	// `bytes` from [r10 + source] to the home at `offset` on, through
	// xmm15 (16 and 8 bytes at a time) and, for a last part under eight
	// bytes, r10 itself: the variadic areas pad each argument to eight
	// bytes, so its whole last eightbyte may be read.
	const Reg x = static_cast<Reg>(XMM0 + 15);
	uint32_t done = 0;
	for (; bytes - done >= 16; done += 16)
	{
		asm_.sse(0, 0x10, x, RM::m(R10, source + static_cast<int32_t>(done)));
		asm_.sse_store(0, 0x11, home.displaced(static_cast<int32_t>(offset + done)), x);
	}
	if (bytes - done >= 8)
	{
		asm_.sse(0xf2, 0x10, x, RM::m(R10, source + static_cast<int32_t>(done)));
		asm_.sse_store(0xf2, 0x11, home.displaced(static_cast<int32_t>(offset + done)), x);
		done += 8;
	}
	if (done == bytes)
		return;
	asm_.mov_load(R10, RM::m(R10, source + static_cast<int32_t>(done)), 8, false);
	for (unsigned width = 4; width != 0; width /= 2)
	{
		if ((bytes - done) < width)
			continue;
		asm_.mov_store(home.displaced(static_cast<int32_t>(offset + done)), R10, width);
		done += width;
		if (done != bytes)
			asm_.shift_ri(5, RM::r(R10), width * 8, 8); // shr
	}
}

void FunctionEmitter::va_arg_object(const Inst& inst)
{
	// An argument of `aux` bytes into its memory home (see
	// FunctionSelector::emit_va_arg_object): each eightbyte of its classes
	// `sub` from the register save area at the offset its class's field
	// gives (gp_offset: 8 apiece up to 48, fp_offset: 16 apiece up to 176;
	// an SSEUP eightbyte in its SSE eightbyte's slot), if all fit, else
	// the whole object from the overflow area aligned to `aux2`, which
	// then moves past it rounded to eight bytes. r11 holds the list, r10
	// each address.
	const RM home = rm(inst.ops[0]);
	const uint8_t classes = inst.sub;
	const uint32_t bytes = inst.aux;
	const uint32_t align = inst.aux2;
	asm_.lea(R11, rm(inst.ops[1]));
	const uint32_t stack = asm_.new_label();
	const uint32_t end = asm_.new_label();
	if ((classes & lowir::OC_MEMORY) == 0)
	{
		const uint32_t eightbytes = (bytes + 7) / 8;
		int32_t gprs = 0, xmms = 0;
		for (uint32_t k = 0; k < eightbytes; ++k)
		{
			gprs += lowir::eightbyte_class(classes, k) == lowir::EB_INTEGER ? 1 : 0;
			xmms += lowir::eightbyte_class(classes, k) == lowir::EB_SSE ? 1 : 0;
		}
		if (gprs != 0)
		{
			asm_.mov_load(R10, RM::m(R11, 0), 4, false);
			asm_.alu_ri(7, RM::r(R10), 48 - 8 * gprs, 4);
			asm_.jcc(CC_A, stack);
		}
		if (xmms != 0)
		{
			asm_.mov_load(R10, RM::m(R11, 4), 4, false);
			asm_.alu_ri(7, RM::r(R10), 176 - 16 * xmms, 4);
			asm_.jcc(CC_A, stack);
		}
		for (uint32_t k = 0; k < eightbytes; ++k)
		{
			const lowir::EightbyteClass cls = lowir::eightbyte_class(classes, k);
			if (cls == lowir::EB_NONE || cls == lowir::EB_SSEUP)
				continue;
			const bool sse = cls == lowir::EB_SSE;
			const int32_t field = sse ? 4 : 0;
			uint32_t part = bytes - 8 * k < 8 ? bytes - 8 * k : 8;
			if (sse && k + 1 < eightbytes && lowir::eightbyte_class(classes, k + 1) == lowir::EB_SSEUP)
				part = bytes - 8 * k;
			asm_.mov_load(R10, RM::m(R11, field), 4, false);
			asm_.alu_rr(0, R10, RM::m(R11, 16), 8);
			asm_.alu_ri(0, RM::m(R11, field), sse ? 16 : 8, 4);
			va_copy_bytes(home, 0, 8 * k, part);
		}
		asm_.jmp(end);
	}
	asm_.bind(stack);
	const int32_t rounded = static_cast<int32_t>((bytes + 7) & ~7u);
	asm_.mov_load(R10, RM::m(R11, 8), 8, false);
	if (align > 8)
	{
		asm_.alu_ri(0, RM::r(R10), static_cast<int32_t>(align) - 1, 8);
		asm_.alu_ri(4, RM::r(R10), -static_cast<int32_t>(align), 8);
	}
	asm_.alu_ri(0, RM::r(R10), rounded, 8);
	asm_.mov_store(RM::m(R11, 8), R10, 8);
	va_copy_bytes(home, -rounded, 0, bytes);
	asm_.bind(end);
}

void FunctionEmitter::eh(const Inst& inst)
{
	switch (inst.op)
	{
	// The record below the frame (RSP lowered past it) or in its frame
	// slot; its saved RSP is RSP after the push.
	case Op::EhPush:
	{
		const RM record = RM::m(R11, 0);
		if (function_->eh_records_pushed)
		{
			asm_.alu_ri(5, RM::r(RSP), kEhRecordBytes, 8); // sub rsp, record
			asm_.mov_rr(R11, RSP);
		}
		else
			asm_.lea(R11, RM::m(RBP, static_cast<int32_t>(inst.aux2)));
		asm_.mov_load(R10, RM::rip(runtime_.eh_top, 0), 8, false);
		asm_.mov_store(record.displaced(kEhPrev), R10, 8);
		RM handler;
		handler.is_reg = false;
		handler.label = labels_[inst.ops[0].symbol];
		asm_.lea(R10, handler);
		asm_.mov_store(record.displaced(kEhTarget), R10, 8);
		if (inst.aux == kEhTry)
			asm_.mov_store(record.displaced(kEhRbp), RBP, 8);
		else
		{
			asm_.lea(R10, RM::m(RBP, static_cast<int32_t>(inst.aux)));
			asm_.mov_store(record.displaced(kEhRbp), R10, 8);
		}
		asm_.mov_store(record.displaced(kEhRsp), RSP, 8);
		const Reg saved[5] = { RBX, R12, R13, R14, R15 };
		for (unsigned k = 0; k < 5; ++k)
			asm_.mov_store(record.displaced(kEhSaved + 8 * static_cast<int32_t>(k)), saved[k], 8);
		asm_.mov_store(RM::rip(runtime_.eh_top, 0), R11, 8);
		return;
	}
	// The top record: on the normal path the region's own, in a landing
	// the landing's own (active) record until a catch landing's
	// `exception` read has taken it. A record below the frame is released
	// by RSP set past it from the record's address (lea rsp, [record +
	// 80]), which writes RSP without reading it: on the O1 workloads `add
	// rsp, 80` ran up to 20% slower.
	case Op::EhPop:
	{
		const Reg prev = function_->eh_records_pushed ? R10 : R11;
		asm_.mov_load(R11, RM::rip(runtime_.eh_top, 0), 8, false);
		asm_.mov_load(prev, RM::m(R11, kEhPrev), 8, false);
		asm_.mov_store(RM::rip(runtime_.eh_top, 0), prev, 8);
		if (function_->eh_records_pushed)
			asm_.lea(RSP, RM::m(R11, kEhRecordBytes));
		return;
	}
	case Op::Throw:
	{
		const Operand& v = inst.ops[0];
		if (v.kind == OperandKind::Reg)
			asm_.mov_store(RM::rip(runtime_.eh_payload, 0), v.reg, 8);
		else if (v.kind == OperandKind::Imm && fits_i32(v.imm))
			asm_.mov_store_imm(RM::rip(runtime_.eh_payload, 0), v.imm, 8);
		else
		{
			if (v.kind == OperandKind::Imm)
				asm_.mov_ri(R11, v.imm);
			else
				asm_.mov_load(R11, rm(v), 8, false);
			asm_.mov_store(RM::rip(runtime_.eh_payload, 0), R11, 8);
		}
		asm_.jmp_symbol(runtime_.eh_dispatch);
		return;
	}
	case Op::Resume:
		if (eh_model_ == EhModel::Host)
		{
			terminal_used_ = true;
			branch(CC_O, false, terminal_);
			return;
		}
		asm_.jmp_symbol(runtime_.eh_dispatch);
		return;
	// A host landing pad: the personality's exception object and selector
	// or filter (rax, rdx) into the frame slots.
	case Op::Landing:
		asm_.mov_store(rm(inst.ops[0]), RAX, 8);
		asm_.mov_store(rm(inst.ops[1]), RDX, 8);
		if (inst.nops == 3)
			asm_.mov_store(rm(inst.ops[2]), RDX, 8);
		return;
	// A host landing's selector: that of the first (filter, selector) pair
	// with the delivered filter, else 0.
	case Op::EhSelect:
	{
		const EhLanding& landing = function_->eh.landings[inst.aux];
		const uint32_t done = asm_.new_label();
		asm_.mov_load(R10, rm(inst.ops[1]), 8, false);
		for (uint32_t k = 0; k < landing.map_count; ++k)
		{
			const uint32_t next = asm_.new_label();
			asm_.alu_ri(7, RM::r(R10), function_->eh.map[landing.map_first + 2 * k], 4);
			asm_.jcc(CC_NE, next);
			asm_.mov_store_imm(rm(inst.ops[0]), function_->eh.map[landing.map_first + 2 * k + 1], 8);
			asm_.jmp(done);
			asm_.bind(next);
		}
		asm_.mov_store_imm(rm(inst.ops[0]), 0, 8);
		asm_.bind(done);
		return;
	}
	// A host mid-landing catch-all: its selector when none is set.
	case Op::CatchAll:
	{
		const uint32_t skip = asm_.new_label();
		asm_.alu_ri(7, rm(inst.ops[0]), 0, 8);
		asm_.jcc_short(CC_NE, skip);
		asm_.mov_store_imm(rm(inst.ops[0]), inst.aux, 8);
		asm_.bind(skip);
		return;
	}
	case Op::EhMark:
	{
		// The top record, if it is this frame's active try record, becomes
		// a cleanup record.
		const uint32_t skip = asm_.new_label();
		asm_.mov_load(R11, RM::rip(runtime_.eh_top, 0), 8, false);
		asm_.op(0, true, 0x85, 1, R11, RM::r(R11)); // test r11, r11
		asm_.jcc_short(CC_E, skip);
		asm_.lea(R10, RM::m(RBP, kEhTry | kEhActive));
		asm_.alu_mr(7, RM::m(R11, kEhRbp), R10, 8); // cmp [r11+saved rbp], rbp|active
		asm_.jcc_short(CC_NE, skip);
		asm_.alu_ri(1, RM::m(R11, kEhRbp), kEhCleanup, 1); // or byte [r11+saved rbp], cleanup
		asm_.bind(skip);
		return;
	}
	case Op::EhCatch:
		// The runtime's clause routine (type information in rdi, null for
		// `...`; the selector in esi) clobbers what a call clobbers.
		if (inst.nops == 1)
			asm_.lea(RDI, RM::rip(inst.ops[0].symbol, inst.ops[0].disp));
		else
			asm_.op(0, false, 0x33, 1, RDI, RM::r(RDI)); // xor edi, edi
		asm_.op_plus(0, false, 0xb8, RSI);               // mov esi, selector
		asm_.imm(inst.aux, 4);
		asm_.call_symbol(runtime_.eh_catch);
		return;
	case Op::LoadSelector:
		asm_.mov_load(inst.ops[0].reg, RM::rip(runtime_.eh_selector, 0), 4, false);
		return;
	default:
	{
		// A catch landing reading its exception takes it: an active try
		// record of this frame on top is popped (and released when below
		// the frame), so the landing's later eh_end pops an enclosing
		// region.
		const uint32_t taken = asm_.new_label();
		asm_.mov_load(R11, RM::rip(runtime_.eh_top, 0), 8, false);
		asm_.op(0, true, 0x85, 1, R11, RM::r(R11)); // test r11, r11
		asm_.jcc_short(CC_E, taken);
		asm_.lea(R10, RM::m(RBP, kEhTry | kEhActive));
		asm_.alu_mr(7, RM::m(R11, kEhRbp), R10, 8); // cmp [r11+saved rbp], rbp|active
		asm_.jcc_short(CC_NE, taken);
		const Reg prev = function_->eh_records_pushed ? R10 : R11;
		asm_.mov_load(prev, RM::m(R11, kEhPrev), 8, false);
		asm_.mov_store(RM::rip(runtime_.eh_top, 0), prev, 8);
		if (function_->eh_records_pushed)
			asm_.lea(RSP, RM::m(R11, kEhRecordBytes));
		asm_.bind(taken);
		const TypeKind kind = inst.type;
		const bool sign = kind == TypeKind::I8 || kind == TypeKind::I16 || kind == TypeKind::I32;
		const unsigned width = kind == TypeKind::I8 || kind == TypeKind::U8 || kind == TypeKind::I1
			? 1
			: (kind == TypeKind::I16 || kind == TypeKind::U16
					  ? 2
					  : (kind == TypeKind::I32 || kind == TypeKind::U32 ? 4 : 8));
		asm_.mov_load(inst.ops[0].reg, RM::rip(runtime_.eh_payload, 0), width, sign);
		return;
	}
	}
}

} // namespace native
