#include "native/select/function_selector.h"

#include "native/backend_error.h"
#include "support/internal_error.h"

// Atomic operations, bulk object memory and i128 arithmetic. Atomics use
// the x86 forms directly (loads and non-seq_cst stores are ordinary moves
// under TSO; seq_cst stores exchange). copy_bytes/zero_bytes name their
// address registers or frame locations directly; i128 values live in
// frame storage and their operations are emitter pseudo instructions.
namespace native {

using lowir::BinaryOp;
using lowir::Instruction;
using lowir::kNoId;
using lowir::MemoryOrder;
using lowir::Opcode;
using lowir::Type;
using lowir::TypeKind;
using lowir::UnaryOp;
using lowir::Value;
using lowir::ValueKind;

namespace {

uint32_t operand_regs(const Operand& op)
{
	uint32_t mask = 0;
	if (op.kind == OperandKind::Reg && op.reg < 32)
		mask |= 1u << op.reg;
	if (op.kind == OperandKind::Mem)
	{
		if (op.reg < 32)
			mask |= 1u << op.reg;
		if (op.index != kNoReg)
			mask |= 1u << op.index;
	}
	return mask;
}

// A copy_bytes/zero_bytes address: a bare register dereference names the
// register holding the address.
Operand bulk_address(const Operand& mem)
{
	if (mem.kind == OperandKind::Mem && mem.index == kNoReg && mem.disp == 0 && mem.symbol == kNoId && mem.reg < 32)
		return Operand::make_reg(mem.reg);
	return mem;
}

} // namespace

void FunctionSelector::emit_atomic(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	switch (inst.op)
	{
	case Opcode::AtomicThreadFence:
		if (inst.order == MemoryOrder::SeqCst)
			emit(Op::Mfence);
		return;
	case Opcode::AtomicSignalFence:
		return;
	case Opcode::AtomicLoad:
	{
		// i128 (the others are ordinary loads).
		needs_scratch_area_ = true;
		const Operand mem = address_operand(inst.ops[0], kSelectScratch);
		Inst& mir = emit2(Op::Wide, TypeKind::I128, memory_result(inst), mem);
		mir.sub = static_cast<uint8_t>(WideOp::Move);
		mir.flags |= MF_VOLATILE;
		return;
	}
	case Opcode::AtomicStore:
	{
		if (kind == TypeKind::I128)
			throw BackendError("atomic i128 store is not supported");
		const Operand mem = address_operand(inst.ops[1], kSelectScratch);
		Operand value = rm_operand(inst.ops[0], inst.type, kEmitScratch);
		if (inst.order == MemoryOrder::SeqCst || value.kind == OperandKind::Mem || value.kind == OperandKind::Symbol)
		{
			load_into(kEmitScratch, value, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
			value = Operand::make_reg(kEmitScratch);
		}
		emit2(inst.order == MemoryOrder::SeqCst ? Op::Xchg : Op::Store, kind, mem, value);
		return;
	}
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
	{
		const Operand mem = address_operand(inst.ops[0], kSelectScratch);
		Operand value = rm_operand(inst.ops[1], inst.type, kEmitScratch);
		if (value.kind == OperandKind::Mem || value.kind == OperandKind::Symbol)
		{
			load_into(kEmitScratch, value, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
			value = Operand::make_reg(kEmitScratch);
		}
		const Reg d = result_reg(inst, kNoReg, kNoReg, operand_regs(mem) | operand_regs(value));
		load_into(d, value, TypeKind::I64);
		const Operand dst = Operand::make_reg(d);
		if (inst.op == Opcode::AtomicExchange)
		{
			emit2(Op::Xchg, kind, mem, dst);
			normalize(d, kind);
		}
		else
		{
			emit2(Op::LockXadd, kind, mem, dst);
			normalize(d, kind);
			Operand delta = value;
			if (value.kind == OperandKind::Reg && value.reg == d)
			{
				// A spilled result took the delta's scratch copy (the address
				// holding the other scratch): the address is dead after the
				// exchange, so the delta forms again in its scratch.
				const Reg s = d == kEmitScratch ? kSelectScratch : kEmitScratch;
				delta = rm_operand(inst.ops[1], inst.type, s);
				if (delta.kind == OperandKind::Mem || delta.kind == OperandKind::Symbol)
				{
					load_into(s, delta, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
					delta = Operand::make_reg(s);
				}
			}
			emit2(Op::Add, TypeKind::I64, dst, delta);
			normalize(d, kind);
		}
		finish_result(inst, d);
		return;
	}
	case Opcode::AtomicCompareExchange:
	{
		const Operand target = address_operand(inst.ops[0], kSelectScratch);
		const Operand expected = address_operand(inst.ops[1], kEmitScratch);
		if (kind == TypeKind::I128)
		{
			needs_scratch_area_ = true;
			const Operand desired = rm_operand(inst.ops[2], inst.type, kSelectScratch);
			const Reg d = result_reg(inst);
			Inst& mir = emit3(Op::LockCmpxchg16b, kind, target, expected, desired);
			mir.aux = d;
			finish_result(inst, d);
			return;
		}
		const Value desired_value = resolve(inst.ops[2]);
		const bool slot_address = desired_value.kind == ValueKind::Slot;
		Reg borrowed = kNoReg;
		int32_t borrowed_home = 0;
		Operand desired = slot_address ? Operand::make_frame(slot_offset_[desired_value.index])
									   : rm_operand(inst.ops[2], inst.type, kNoReg);
		if (desired.kind != OperandKind::Reg)
		{
			// Neither scratch is free: borrow an unoccupied register.
			const uint32_t busy = occupied_ | pinned_ | operand_regs(target) | operand_regs(expected) | (1u << RAX) |
				reserved_gprs(*out_);
			uint32_t free = kAllocatableGprs & ~busy;
			// When every allocatable register holds a value, a reserved
			// scratch register the two addresses leave free stages it.
			if (free == 0)
				free = ((1u << kSelectScratch) | (1u << kEmitScratch)) & ~busy;
			// When the two addresses hold both scratch registers too, a
			// register they do not use is saved in a frame word around the
			// exchange.
			if (free == 0)
			{
				const uint32_t spare = kAllocatableGprs &
					~(operand_regs(target) | operand_regs(expected) | (1u << RAX) | reserved_gprs(*out_));
				if (spare == 0)
					throw InternalError("no register for atomic compare-exchange");
				borrowed = static_cast<Reg>(__builtin_ctz(spare));
				borrowed_home = frame_alloc(8, 8);
				emit2(Op::Store, TypeKind::I64, Operand::make_frame(borrowed_home), Operand::make_reg(borrowed));
				free = 1u << borrowed;
			}
			const Reg r = static_cast<Reg>(__builtin_ctz(free));
			if (kCalleeSavedGprs & (1u << r))
				used_callee_saved_ |= 1u << r;
			written_ |= 1u << r;
			if (slot_address)
				emit2(Op::Lea, TypeKind::Void, Operand::make_reg(r), desired);
			else if (desired.kind == OperandKind::Mem && desired_value.kind == ValueKind::Temp)
			{
				// A home holds the value at its own width (an elided
				// extension stands for a narrower value).
				load_into(r, desired, body_->temps[desired_value.index].type.kind());
			}
			else
				load_into(r, desired, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
			desired = Operand::make_reg(r);
		}
		emit2(Op::Load, kind, Operand::make_reg(RAX), expected);
		emit2(Op::LockCmpxchg, kind, target, desired);
		emit2(Op::Store, kind, expected, Operand::make_reg(RAX));
		if (borrowed != kNoReg)
			emit2(Op::Load, TypeKind::I64, Operand::make_reg(borrowed), Operand::make_frame(borrowed_home));
		const Reg d = result_reg(inst);
		Inst& set = emit1(Op::Setcc, TypeKind::Void, Operand::make_reg(d));
		set.sub = CC_E;
		emit2(Op::Movzx, TypeKind::Void, Operand::make_reg(d), Operand::make_reg(d));
		finish_result(inst, d);
		return;
	}
	default:
		throw InternalError("unsupported atomic operation");
	}
}

void FunctionSelector::emit_bulk(const Instruction& inst)
{
	const uint32_t bytes = static_cast<uint32_t>(inst.immediate);
	const uint32_t align = 1u << inst.sub;
	if (inst.op == Opcode::ZeroInit)
	{
		const Operand dst = bulk_address(address_operand(inst.ops[0], kSelectScratch));
		Inst& mir = emit1(Op::ZeroBytes, TypeKind::Void, dst);
		mir.aux = bytes;
		mir.aux2 = align;
		return;
	}
	const Value src_value = resolve(inst.ops[0]);
	// The call already stored its result here (emit_calls.cpp).
	if (src_value.kind == ValueKind::Temp && (temps_[src_value.index].flags & TF_STORED))
		return;
	Operand src;
	if (src_value.kind == ValueKind::Temp && temps_[src_value.index].cls == VClass::Memory)
		src = temp_location(src_value.index);
	else
		src = bulk_address(address_operand(inst.ops[0], kEmitScratch));
	const Operand dst = bulk_address(address_operand(inst.ops[1], kSelectScratch));
	// An object parameter whose home is this slot (layout_slots).
	if (src.kind == OperandKind::Mem && dst.kind == OperandKind::Mem && src.reg == kFrame && dst.reg == kFrame &&
		src.index == kNoReg && dst.index == kNoReg && src.disp == dst.disp)
		return;
	Inst& mir = emit2(Op::CopyBytes, TypeKind::Void, dst, src);
	mir.aux = bytes;
	mir.aux2 = align;
}

void FunctionSelector::emit_wide(const Instruction& inst)
{
	WideOp op = WideOp::Move;
	if (inst.op == Opcode::Unary)
	{
		const UnaryOp u = static_cast<UnaryOp>(inst.sub);
		if (u == UnaryOp::Neg)
			op = WideOp::Neg;
		else if (u == UnaryOp::BitNot)
			op = WideOp::Not;
		else
			throw BackendError("unsupported i128 unary operation");
	}
	else
	{
		switch (static_cast<BinaryOp>(inst.sub))
		{
		case BinaryOp::Add:
			op = WideOp::Add;
			break;
		case BinaryOp::Sub:
			op = WideOp::Sub;
			break;
		case BinaryOp::Mul:
			op = WideOp::Mul;
			break;
		case BinaryOp::And:
			op = WideOp::And;
			break;
		case BinaryOp::Or:
			op = WideOp::Or;
			break;
		case BinaryOp::Xor:
			op = WideOp::Xor;
			break;
		case BinaryOp::Shl:
			op = WideOp::Shl;
			break;
		case BinaryOp::Shr:
			op = WideOp::Sar;
			break;
		case BinaryOp::UShr:
			op = WideOp::Shr;
			break;
		case BinaryOp::Div:
			op = WideOp::Div;
			break;
		case BinaryOp::UDiv:
			op = WideOp::UDiv;
			break;
		case BinaryOp::Mod:
			op = WideOp::Mod;
			break;
		case BinaryOp::UMod:
			op = WideOp::UMod;
			break;
		default:
			throw InternalError("unsupported i128 binary operation");
		}
	}
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	Operand b;
	if (inst.op == Opcode::Binary)
	{
		if (op == WideOp::Shl || op == WideOp::Shr || op == WideOp::Sar)
		{
			const Value count = resolve(inst.ops[1]);
			if (count.kind == ValueKind::Integer)
				b = Operand::make_imm(static_cast<int64_t>(literal(count).low & 127));
			else
			{
				const Type ct = type_of(inst.ops[1]);
				const Operand c = rm_operand(inst.ops[1], ct, kEmitScratch);
				load_into(RCX, c, ct.kind() == TypeKind::I128 ? TypeKind::I64 : ct.kind());
				b = Operand::make_reg(RCX);
			}
		}
		else
			b = rm_operand(inst.ops[1], inst.type, kEmitScratch);
	}
	Inst& mir = emit3(Op::Wide, TypeKind::I128, memory_result(inst), a, b);
	mir.sub = static_cast<uint8_t>(op);
	if (inst.op == Opcode::Unary)
		mir.nops = 2;
}

} // namespace native
