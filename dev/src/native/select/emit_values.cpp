#include "native/select/function_selector.h"

#include <string>

#include "native/backend_error.h"
#include "support/internal_error.h"

// Instruction selection for value-producing LowIR instructions, loads and
// stores. Integer registers hold narrow values normalized to 64 bits (sign-
// extended for i8/i16/i32, zero-extended for u8/u16/u32, 0/1 for i1), so
// 64-bit ALU forms are used and results renormalized where an operation
// can leave stale upper bits. A memory right operand (a folded load or a
// frame home) is used directly where x86 accepts one.
namespace native {

using lowir::BinaryOp;
using lowir::ComparePredicate;
using lowir::ConvertOp;
using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::Type;
using lowir::TypeKind;
using lowir::UnaryOp;
using lowir::Value;
using lowir::ValueKind;

namespace {

// Whether a narrow result computed at 64 bits already is its canonical
// value: it fits the type non-negatively, an unsigned type below its
// width (fits_ stops there for a wrapped unsigned result).
bool fits_unnormalized_bits(unsigned fits, TypeKind kind)
{
	return fits <= canonical_bits(kind) - (unsigned_narrow(kind) ? 1u : 0u);
}

uint32_t mem_regs(const Operand& op)
{
	uint32_t mask = 0;
	if (op.kind == OperandKind::Mem || op.kind == OperandKind::Reg)
	{
		if (op.reg != kNoReg && op.reg < 32)
			mask |= 1u << op.reg;
		if (op.kind == OperandKind::Mem && op.index != kNoReg)
			mask |= 1u << op.index;
	}
	return mask;
}

} // namespace

// ---------------------------------------------------------------------------
// Results

Reg FunctionSelector::dying_reg(Value value) const
{
	value = resolve(value);
	if (value.kind != ValueKind::Temp)
		return kNoReg;
	const TempInfo& t = temps_[value.index];
	if (t.reg == kNoReg || !dies_here(value.index))
		return kNoReg;
	return t.reg;
}

Reg FunctionSelector::result_reg(const Instruction& inst, Reg prefer0, Reg prefer1, uint32_t exclude)
{
	const uint32_t temp = inst.result;
	TempInfo& t = temps_[temp];
	release_dying(position_);
	if (t.greg != kNoReg)
	{
		// O1+: bound at its definition (register_allocation.cpp keeps the
		// register free there).
		if (precise_globals_)
		{
			if (owner_[t.greg] != kNoId && owner_[t.greg] != temp)
				throw InternalError("function-level register of t" + std::to_string(temp) + " held by t" +
					std::to_string(owner_[t.greg]) + " at its definition in " +
					program_.spelling(program_.symbols[symbol_].name));
			bind(temp, t.greg);
		}
		++stats_.defined_in_plan;
		return t.greg;
	}
	const Reg reg = allocate(temp, exclude, prefer0, prefer1);
	if (reg == kNoReg)
	{
		if (!(t.flags & TF_GLOBAL) && t.last > t.def)
			++stats_.spills;
		else if (t.flags & TF_GLOBAL)
			++stats_.defined_frame;
		t.flags |= TF_HOMED;
		// A result without a register is computed in r10, or in r11 when an
		// operand the instruction still reads (`exclude`) occupies r10.
		if (t.cls == VClass::Xmm)
			return static_cast<Reg>(XMM0 + 14);
		return exclude & (1u << kSelectScratch) ? kEmitScratch : kSelectScratch;
	}
	if (t.flags & TF_GLOBAL)
		++stats_.defined_other_register;
	bind(temp, reg);
	return reg;
}

void FunctionSelector::finish_result(const Instruction& inst, Reg reg)
{
	const uint32_t temp = inst.result;
	TempInfo& t = temps_[temp];
	if (t.greg != kNoReg)
		return;
	if (t.flags & (TF_GLOBAL | TF_HOMED))
	{
		const Operand home = Operand::make_frame(home_of(temp));
		const TypeKind kind = body_->temps[temp].type.kind();
		if (is_xmm(reg))
			emit2(Op::FMov, kind, home, Operand::make_reg(reg));
		else
			emit2(Op::Store, kind, home, Operand::make_reg(reg));
	}
	if ((t.flags & TF_HOMED) || t.last <= t.def)
	{
		if (t.reg == reg && owner_[reg] == temp)
			release(reg);
		t.reg = kNoReg;
	}
}

void FunctionSelector::normalize(Reg reg, TypeKind type)
{
	// O1+: `mov reg, src` just emitted in this block and an extension of
	// reg become one extending move (`zext.u32 reg, src`: mov r32, r32).
	if (level_ >= 1 && type != TypeKind::I1 && narrow_integer(type) && out_->insts.size() > out_->blocks.back().first)
	{
		Inst& last = out_->insts.back();
		if (last.op == Op::Mov && last.nops == 2 && last.ops[0].kind == OperandKind::Reg && last.ops[0].reg == reg &&
			last.ops[1].kind == OperandKind::Reg && !is_xmm(last.ops[1].reg))
		{
			last.op = signed_integer(type) ? Op::Sext : Op::Zext;
			last.type = type;
			return;
		}
	}
	switch (type)
	{
	case TypeKind::I8:
	case TypeKind::I16:
	case TypeKind::I32:
		emit1(Op::Sext, type, Operand::make_reg(reg));
		break;
	case TypeKind::U8:
	case TypeKind::U16:
	case TypeKind::U32:
		emit1(Op::Zext, type, Operand::make_reg(reg));
		break;
	case TypeKind::I1:
		emit2(Op::And, TypeKind::I64, Operand::make_reg(reg), Operand::make_imm(1));
		break;
	default:
		break;
	}
}

Operand FunctionSelector::memory_result(const Instruction& inst)
{
	return Operand::make_frame(home_of(inst.result));
}

void FunctionSelector::store_result_memory(const Instruction& inst, const Operand& src, TypeKind type)
{
	const Operand home = memory_result(inst);
	if (type == TypeKind::F80)
		emit2(Op::FMov, TypeKind::F80, home, src);
	else if (type == TypeKind::I128)
	{
		Inst& mir = emit3(Op::Wide, TypeKind::I128, home, src, Operand());
		mir.sub = static_cast<uint8_t>(WideOp::Move);
		mir.nops = 2;
	}
	else
	{
		Inst& mir = emit2(Op::CopyBytes, TypeKind::Void, home, src);
		const Type t = body_->temps[inst.result].type;
		mir.aux = static_cast<uint32_t>(value_bytes(t));
		mir.aux2 = value_align(t);
	}
}

// ---------------------------------------------------------------------------
// Dispatch

void FunctionSelector::emit_inst(uint32_t id)
{
	const Instruction& inst = body_->instructions[id];
	position_ = pos_[id];
	location_ = inst.location;
	if (!live(id))
		return;
	if (inst.result != kNoId &&
		(temps_[inst.result].flags &
			(TF_ALIAS | TF_DEAD | TF_LAZY | TF_FOLDED | TF_FUSED | TF_TESTED | TF_ROTATED | TF_RMW)))
		return;
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Phi:
		break;
	case Opcode::Copy:
		emit_copy(inst);
		break;
	case Opcode::Addr:
		emit_addr(inst);
		break;
	case Opcode::Load:
	case Opcode::AtomicLoad:
		if (inst.op == Opcode::AtomicLoad && inst.type.kind() == TypeKind::I128)
			emit_atomic(inst);
		else
			emit_load(inst);
		break;
	case Opcode::Store:
		emit_store(inst);
		break;
	case Opcode::Index:
		emit_index(inst);
		break;
	case Opcode::Unary:
		emit_unary(inst);
		break;
	case Opcode::Binary:
		emit_binary(inst);
		break;
	case Opcode::Compare:
		emit_compare(inst);
		break;
	case Opcode::Select:
		emit_select(inst);
		break;
	case Opcode::Convert:
		emit_convert(inst);
		break;
	case Opcode::Call:
		emit_call(id);
		break;
	case Opcode::CopyObject:
	case Opcode::ZeroInit:
		emit_bulk(inst);
		break;
	case Opcode::AtomicStore:
	case Opcode::AtomicAddFetch:
	case Opcode::AtomicExchange:
	case Opcode::AtomicCompareExchange:
	case Opcode::AtomicThreadFence:
	case Opcode::AtomicSignalFence:
		emit_atomic(inst);
		break;
	case Opcode::VaStart:
		emit_va_start(inst);
		break;
	case Opcode::VaArg:
	{
		const Operand list = address_operand(inst.ops[0], kSelectScratch);
		if (temps_[inst.result].cls == VClass::Memory)
		{
			// A host object's binary16 or binary128 argument, its SSE class
			// (binary128's with SSEUP) stated: one XMM slot of the register
			// save area, or the overflow area, into its home; another
			// object, an i128 or a long double by its classes.
			const uint8_t classes = argument_classes(id, 0, nullptr);
			const uint32_t bytes = value_bytes(inst.type);
			TypeKind part = TypeKind::Void;
			if (classes == (lowir::OC_STATED | lowir::EB_SSE) && bytes <= 8)
				part = bytes <= 4 ? TypeKind::F32 : TypeKind::F64;
			else if (classes == (lowir::OC_STATED | lowir::EB_SSE | (lowir::EB_SSEUP << 2)) && bytes == 16)
				part = TypeKind::I128;
			if (part != TypeKind::Void)
			{
				emit2(Op::VaArg, part, memory_result(inst), list);
				break;
			}
			emit_va_arg_object(inst, classes, list);
			break;
		}
		const Reg d = result_reg(inst, kNoReg, kNoReg, mem_regs(list));
		emit2(Op::VaArg, inst.type.kind(), Operand::make_reg(d), list);
		finish_result(inst, d);
		break;
	}
	case Opcode::StackAlloc:
	{
		Operand size = rm_operand(inst.ops[0], Type::scalar(TypeKind::I64), kSelectScratch);
		if (size.kind == OperandKind::Imm && (size.imm < 0 || size.imm > 0x7ffffff0ll))
			throw BackendError("constant stack_alloc size exceeds the native stack adjustment range");
		if (size.kind != OperandKind::Imm && size.kind != OperandKind::Reg)
		{
			load_into(kSelectScratch, size, TypeKind::I64);
			size = Operand::make_reg(kSelectScratch);
		}
		// stack_alloc reads its size before writing the result: they may
		// share a scratch register.
		const Reg d = result_reg(inst, kNoReg, kNoReg, mem_regs(size) & kAllocatableGprs);
		emit2(Op::StackAlloc, TypeKind::Void, Operand::make_reg(d), size);
		finish_result(inst, d);
		break;
	}
	case Opcode::Exception:
	{
		if (temps_[inst.result].cls != VClass::Gpr)
			throw BackendError("exception payload of a non-integer type is not supported");
		const Reg d = result_reg(inst);
		// Host EH: the exception object its landing pad stored.
		if (eh_model_ == EhModel::Host)
			emit2(Op::Load, inst.type.kind(), Operand::make_reg(d), Operand::make_frame(out_->eh.slot));
		else
			emit1(Op::LoadException, inst.type.kind(), Operand::make_reg(d));
		finish_result(inst, d);
		break;
	}
	case Opcode::ExceptionSelector:
	{
		const Reg d = result_reg(inst);
		if (eh_model_ == EhModel::Host)
			emit2(Op::Load, inst.type.kind(), Operand::make_reg(d), Operand::make_frame(out_->eh.slot + 8));
		else
			emit1(Op::LoadSelector, inst.type.kind(), Operand::make_reg(d));
		finish_result(inst, d);
		break;
	}
	case Opcode::EhTry:
	case Opcode::EhCleanup:
	case Opcode::EhEnd:
	case Opcode::EhCatch:
	case Opcode::EhFilter:
	case Opcode::EhCatchAll:
		emit_eh(id);
		break;
	default:
		emit_terminator(id);
		break;
	}
	written_ |= clobber_[id] | early_[id];
	release_dying(position_);
	retire_dying();
}

// ---------------------------------------------------------------------------
// Copies, addresses, loads and stores

void FunctionSelector::emit_copy(const Instruction& inst)
{
	const TempInfo& t = temps_[inst.result];
	const TypeKind kind = inst.type.kind();
	if (t.cls == VClass::Memory)
	{
		const Value src = resolve(inst.ops[0]);
		const Operand from = memory_value_operand(src, inst.type);
		if (inst.type.is_object() && from.kind != OperandKind::Mem && from.kind != OperandKind::Reg)
			throw BackendError("object copy requires a storage operand");
		store_result_memory(inst, from, kind);
		return;
	}
	const Operand src = rm_operand(inst.ops[0], type_of(inst.ops[0]), kSelectScratch);
	if (t.cls == VClass::Xmm)
	{
		if (src.kind == OperandKind::Symbol)
			throw BackendError("floating copy requires a numeric value or storage operand");
		const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
		if (src.kind == OperandKind::Imm)
		{
			// An integer carrier is copied by representation, not converted.
			load_into(kSelectScratch, src, TypeKind::I64);
			emit2(Op::FMov, kind, Operand::make_reg(d), Operand::make_reg(kSelectScratch));
		}
		else
			load_into(d, src, kind);
		finish_result(inst, d);
		return;
	}
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
	if (src.kind == OperandKind::Mem)
	{
		// A typed load of the low bytes yields the normalized value.
		emit2(Op::Load, kind == TypeKind::Ptr ? TypeKind::Ptr : kind, Operand::make_reg(d), src);
	}
	else
	{
		load_into(d, src, TypeKind::I64);
		normalize(d, kind);
	}
	finish_result(inst, d);
}

void FunctionSelector::emit_addr(const Instruction& inst)
{
	const TempInfo& t = temps_[inst.result];
	AddrExpr expr;
	if (t.addr != kNoId)
		expr = addrs_[t.addr];
	else if (!address_expr(body_->temps[inst.result].def, expr))
		throw InternalError("unsupported address");
	const Reg d = result_reg(inst);
	materialize_address(d, expr, true);
	finish_result(inst, d);
}

void FunctionSelector::emit_index(const Instruction& inst)
{
	const TempInfo& t = temps_[inst.result];
	if (t.addr != kNoId)
	{
		const AddrExpr expr = addrs_[t.addr];
		const Reg d = result_reg(inst, dying_reg(inst.ops[0]), dying_reg(inst.ops[1]));
		materialize_address(d, expr, true);
		finish_result(inst, d);
		return;
	}
	// base + offset * size with a size outside {1,2,4,8} or a symbol base
	// and register offset.
	const uint32_t size = value_bytes(inst.type);
	const Operand base = rm_operand(inst.ops[0], Type::scalar(TypeKind::Ptr), kSelectScratch);
	const Operand off = rm_operand(inst.ops[1], type_of(inst.ops[1]), kEmitScratch);
	const Reg d = result_reg(inst, dying_reg(inst.ops[1]), kNoReg, mem_regs(base));
	// A homed offset is read at its own type, extending.
	const Value offset = resolve(inst.ops[1]);
	load_into(d, off,
		off.kind == OperandKind::Mem && offset.kind == ValueKind::Temp ? body_->temps[offset.index].type.kind()
																	   : TypeKind::I64);
	// value_bytes bounds element sizes below 2^31: an imm32 multiplier.
	if (size != 1)
		emit2(Op::Imul, TypeKind::I64, Operand::make_reg(d), Operand::make_imm(size));
	if (base.kind == OperandKind::Symbol)
	{
		// The symbol's address forms in the scratch register the result
		// (spilled to r10) does not occupy.
		const Reg s = d == kSelectScratch ? kEmitScratch : kSelectScratch;
		emit2(Op::Mov, TypeKind::Void, Operand::make_reg(s), base);
		emit2(Op::Add, TypeKind::I64, Operand::make_reg(d), Operand::make_reg(s));
	}
	else
		emit2(Op::Add, TypeKind::I64, Operand::make_reg(d), base);
	finish_result(inst, d);
}

void FunctionSelector::emit_load(const Instruction& inst)
{
	const TempInfo& t = temps_[inst.result];
	const TypeKind kind = inst.type.kind();
	const Operand mem = address_operand(inst.ops[0], kSelectScratch);
	if (t.cls == VClass::Memory)
	{
		store_result_memory(inst, mem, kind);
		return;
	}
	// The result takes its hinted register, else a dying address register.
	Reg prefer = kNoReg;
	if (t.hint == kNoReg && mem.reg != kNoReg && mem.reg < 16 && owner_[mem.reg] != kNoId && dies_here(owner_[mem.reg]))
		prefer = mem.reg;
	const Reg d = result_reg(inst, prefer);
	Inst& mir = emit2(t.cls == VClass::Xmm ? Op::FMov : Op::Load, kind, Operand::make_reg(d), mem);
	if (inst.flags & lowir::IF_VOLATILE)
		mir.flags |= MF_VOLATILE;
	finish_result(inst, d);
}

void FunctionSelector::emit_store(const Instruction& inst)
{
	const Type type = inst.type;
	const TypeKind kind = type.kind();
	const Value value = resolve(inst.ops[0]);
	if (value.kind == ValueKind::Temp && type.is_float() && body_->temps[value.index].type.is_float() &&
		body_->temps[value.index].type.kind() != kind)
	{
		// A floating store of another floating width converts implicitly.
		const TypeKind from = body_->temps[value.index].type.kind();
		const Operand src = temp_location(value.index);
		const Operand mem = address_operand(inst.ops[1], kSelectScratch);
		const ConvKind conv = lowir::float_rank(kind) > lowir::float_rank(from) ? ConvKind::FPExt : ConvKind::FPTrunc;
		needs_scratch_area_ = true;
		if (kind == TypeKind::F80)
		{
			Inst& mir = emit2(Op::Convert, from, mem, src);
			mir.type2 = kind;
			mir.sub = static_cast<uint8_t>(conv);
			return;
		}
		const Reg x = static_cast<Reg>(XMM0 + 14);
		Inst& mir = emit2(Op::Convert, from, Operand::make_reg(x), src);
		mir.type2 = kind;
		mir.sub = static_cast<uint8_t>(conv);
		emit2(Op::FMov, kind, mem, Operand::make_reg(x));
		return;
	}
	if (kind == TypeKind::F80 || kind == TypeKind::I128 || type.is_object())
	{
		const Operand src = memory_value_operand(value, type);
		if (type.is_object() && src.kind != OperandKind::Mem && src.kind != OperandKind::Reg)
			throw BackendError("object store requires a storage operand");
		const Operand mem = address_operand(inst.ops[1], kSelectScratch);
		if (kind == TypeKind::F80)
			emit2(Op::FMov, kind, mem, src);
		else if (kind == TypeKind::I128)
		{
			Inst& mir = emit2(Op::Wide, kind, mem, src);
			mir.sub = static_cast<uint8_t>(WideOp::Move);
		}
		else
		{
			Inst& mir = emit2(Op::CopyBytes, TypeKind::Void, mem, src);
			mir.aux = static_cast<uint32_t>(value_bytes(type));
			mir.aux2 = value_align(type);
		}
		return;
	}
	if (value.kind == ValueKind::Temp && (temps_[value.index].flags & TF_RMW))
	{
		// The operation on its folded load (find_rmw), in memory: through a
		// truncation, at the store's width.
		const Instruction* def = &body_->instructions[body_->temps[value.index].def];
		if (def->op == Opcode::Convert)
			def = &body_->instructions[body_->temps[resolve(def->ops[0]).index].def];
		const Instruction& op = *def;
		const Operand mem = address_operand(inst.ops[1], kSelectScratch);
		Op mop = Op::Add;
		Operand x;
		if (op.op == Opcode::Index)
			x = Operand::make_imm(static_cast<int64_t>(literal(resolve(op.ops[1])).low * value_bytes(op.type)));
		else
		{
			static const Op ops[] = { Op::Add, Op::Sub, Op::Imul, Op::Imul, Op::Imul, Op::Imul, Op::Imul, Op::And,
				Op::Or, Op::Xor };
			mop = ops[op.sub];
			const Value a = resolve(op.ops[0]);
			const Value other = a.kind == ValueKind::Temp && (temps_[a.index].flags & (TF_FOLDED | TF_RMW))
				? resolve(op.ops[1])
				: a;
			x = rm_operand(other, type, kEmitScratch);
			if (x.kind == OperandKind::Mem || x.kind == OperandKind::Symbol)
			{
				load_into(kEmitScratch, x, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
				x = Operand::make_reg(kEmitScratch);
			}
		}
		emit2(mop, kind, mem, x);
		return;
	}
	Operand src = rm_operand(inst.ops[0], type, kEmitScratch);
	const Operand mem = address_operand(inst.ops[1], kSelectScratch);
	if (type.is_float())
	{
		Inst& mir = emit2(Op::FMov, kind, mem, src);
		if (inst.flags & lowir::IF_VOLATILE)
			mir.flags |= MF_VOLATILE;
		return;
	}
	if (src.kind == OperandKind::Mem || src.kind == OperandKind::Symbol)
	{
		load_into(kEmitScratch, src, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
		src = Operand::make_reg(kEmitScratch);
	}
	Inst& mir = emit2(Op::Store, kind, mem, src);
	if (inst.flags & lowir::IF_VOLATILE)
		mir.flags |= MF_VOLATILE;
}

// ---------------------------------------------------------------------------
// Arithmetic

void FunctionSelector::emit_unary(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	const UnaryOp op = static_cast<UnaryOp>(inst.sub);
	if (kind == TypeKind::I128)
	{
		emit_wide(inst);
		return;
	}
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	if (inst.type.is_float())
	{
		if (op != UnaryOp::Neg)
			throw BackendError("unsupported floating unary operation");
		if (kind == TypeKind::F80)
		{
			Inst& mir = emit2(Op::FNeg, kind, memory_result(inst), a);
			(void)mir;
			return;
		}
		const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
		emit2(Op::FNeg, kind, Operand::make_reg(d), a);
		finish_result(inst, d);
		return;
	}
	if (op == UnaryOp::Not)
	{
		Operand lhs = a;
		if (lhs.kind == OperandKind::Imm)
		{
			load_into(kSelectScratch, lhs, TypeKind::I64);
			lhs = Operand::make_reg(kSelectScratch);
		}
		const Reg d = result_reg(inst);
		emit2(Op::Cmp, kind, lhs, Operand::make_imm(0));
		Inst& set = emit1(Op::Setcc, TypeKind::Void, Operand::make_reg(d));
		set.sub = CC_E;
		emit2(Op::Movzx, TypeKind::Void, Operand::make_reg(d), Operand::make_reg(d));
		finish_result(inst, d);
		return;
	}
	if (op == UnaryOp::CountTrailingZeros || op == UnaryOp::CountLeadingZeros)
	{
		// bsf, or bsr and 31 - index (63 - index) as index ^ 31 (^ 63):
		// either is small and non-negative, normalized at the result's width.
		const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
		Operand src = a;
		if (src.kind == OperandKind::Imm)
		{
			load_into(kSelectScratch, src, kind);
			src = Operand::make_reg(kSelectScratch);
		}
		emit2(op == UnaryOp::CountTrailingZeros ? Op::Bsf : Op::Bsr, kind, Operand::make_reg(d), src);
		if (op == UnaryOp::CountLeadingZeros)
			emit2(Op::Xor, TypeKind::I64, Operand::make_reg(d), Operand::make_imm(register_bytes(kind) == 8 ? 63 : 31));
		finish_result(inst, d);
		return;
	}
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
	load_into(d, a, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
	if (op == UnaryOp::Neg)
	{
		emit1(Op::Neg, TypeKind::Void, Operand::make_reg(d));
		normalize(d, kind);
	}
	else if (op == UnaryOp::BitNot)
	{
		emit1(Op::Not, TypeKind::Void, Operand::make_reg(d));
		if (!signed_integer(kind))
			normalize(d, kind);
	}
	else
	{
		emit1(Op::Bswap, kind, Operand::make_reg(d));
		normalize(d, kind);
	}
	finish_result(inst, d);
}

void FunctionSelector::emit_binary(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	if (inst.type.is_float())
	{
		emit_float_binary(inst);
		return;
	}
	if (kind == TypeKind::I128)
	{
		emit_wide(inst);
		return;
	}
	if (op == BinaryOp::Div || op == BinaryOp::Mod || op == BinaryOp::UDiv || op == BinaryOp::UMod)
	{
		emit_divide(inst);
		return;
	}
	if (op == BinaryOp::Shl || op == BinaryOp::Shr || op == BinaryOp::UShr)
	{
		emit_shift(inst);
		return;
	}
	if (level_ >= 1 && rotate_[inst.result] != 0)
	{
		// A rotation (find_rmw): x, the value both shifts read, into the
		// result's register, then rol by the left shift's count, or ror by
		// the right one's when it is the smaller.
		const unsigned width = static_cast<unsigned>(value_bytes(inst.type) * 8);
		const unsigned left = rotate_[inst.result];
		const Value x = resolve(body_->instructions[body_->temps[resolve(inst.ops[0]).index].def].ops[0]);
		const Operand a = rm_operand(x, inst.type, kSelectScratch);
		const Reg d = result_reg(inst, dying_reg(x));
		if (a.kind == OperandKind::Mem)
			emit2(Op::Load, kind, Operand::make_reg(d), a);
		else
			load_into(d, a, TypeKind::I64);
		const bool rol = left * 2 <= width;
		emit2(rol ? Op::Rol : Op::Ror, kind == TypeKind::U32 ? TypeKind::U32 : TypeKind::I64, Operand::make_reg(d),
			Operand::make_imm(rol ? left : width - left));
		finish_result(inst, d);
		return;
	}
	const bool commutative = op != BinaryOp::Sub;
	const bool wide_mem = kind == TypeKind::I64 || kind == TypeKind::Ptr || kind == TypeKind::I32 ||
		kind == TypeKind::U32;
	Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	Operand b = rm_operand(inst.ops[1], inst.type, kEmitScratch);
	// A scaled byte offset holds x * K / scale (find_scaled_offsets).
	if (op == BinaryOp::Mul && level_ >= 1 && scale_of_[inst.result] != 0)
	{
		if (a.kind == OperandKind::Imm)
			std::swap(a, b);
		b = Operand::make_imm(b.imm / scale_of_[inst.result]);
	}
	if (b.kind == OperandKind::Symbol)
	{
		load_into(kEmitScratch, b, TypeKind::Ptr);
		b = Operand::make_reg(kEmitScratch);
	}
	if (a.kind == OperandKind::Symbol)
	{
		load_into(kSelectScratch, a, TypeKind::Ptr);
		a = Operand::make_reg(kSelectScratch);
	}
	// A narrow memory right operand and a multiplier beyond imm32 go to a
	// register: r11, or r10 when the left operand (after a swap, the former
	// right operand) already occupies r11.
	auto right_to_register = [&]() {
		// O1+: a 32-bit multiply reads its multiplier's low 32 bits only, so
		// one beyond imm32 as an i64 (a u32 hash constant above 2^31) is its
		// sign-extended low half, an imm32 (imul r32, r/m32, imm32).
		if (level_ >= 1 && op == BinaryOp::Mul && b.kind == OperandKind::Imm && !fits_imm32(b) &&
			(kind == TypeKind::U32 || kind == TypeKind::I32))
			b = Operand::make_imm(static_cast<int32_t>(static_cast<uint32_t>(b.imm)));
		const bool load = (b.kind == OperandKind::Mem && !wide_mem) ||
			(op == BinaryOp::Mul && b.kind == OperandKind::Imm && !fits_imm32(b));
		if (!load)
			return;
		const Reg s = a.uses_reg(kEmitScratch) ? kSelectScratch : kEmitScratch;
		load_into(s, b, b.kind == OperandKind::Mem ? kind : TypeKind::I64);
		b = Operand::make_reg(s);
	};
	right_to_register();
	// A commutative operation with only its left operand in memory or
	// immediate swaps so the register operand is the destination source.
	if (commutative && a.kind != OperandKind::Reg && b.kind == OperandKind::Reg)
	{
		std::swap(a, b);
		right_to_register();
	}
	// A dying operand's register takes the result, except an integer
	// parameter's in a function with vector parameters.
	auto reusable = [&](const Operand& op) {
		return op.kind == OperandKind::Reg && op.reg < 16 && owner_[op.reg] != kNoId && dies_here(owner_[op.reg]) &&
			!(vector_params_ && owner_[op.reg] < body_->parameter_count);
	};
	const Reg dying_a = reusable(a) ? a.reg : kNoReg;
	const Reg dying_b = commutative && reusable(b) ? b.reg : kNoReg;
	uint32_t exclude = mem_regs(b);
	if (dying_b != kNoReg)
		exclude &= ~(1u << dying_b);
	if (vector_params_)
	{
		// Those parameters' registers become free only after this use.
		const Operand ops2[2] = { a, b };
		for (unsigned k = 0; k < 2; ++k)
			if (ops2[k].kind == OperandKind::Reg && ops2[k].reg < 16 && owner_[ops2[k].reg] < body_->parameter_count)
				exclude |= 1u << ops2[k].reg;
	}
	// A result without a register goes to r11 when the right operand (the
	// former left one, after a swap) reads r10.
	const Reg d = result_reg(inst, dying_a, dying_b, exclude);
	static const Op ops[] = { Op::Add, Op::Sub, Op::Imul, Op::Imul, Op::Imul, Op::Imul, Op::Imul, Op::And, Op::Or,
		Op::Xor };
	const Op mop = ops[static_cast<unsigned>(op)];
	const Operand dst = Operand::make_reg(d);
	const bool bitwise = op == BinaryOp::And || op == BinaryOp::Or || op == BinaryOp::Xor;
	// O1+: u32 arithmetic in its 32-bit form, which zero-extends (no
	// renormalization).
	const TypeKind width = b.kind == OperandKind::Mem || (level_ >= 1 && kind == TypeKind::U32 && !bitwise)
		? kind
		: TypeKind::I64;
	if (a.kind == OperandKind::Reg && a.reg == d)
		emit2(mop, width, dst, b);
	else if (commutative && b.kind == OperandKind::Reg && b.reg == d)
		emit2(mop, width, dst, a);
	else
	{
		if (a.kind == OperandKind::Mem)
			emit2(Op::Load, kind == TypeKind::Ptr ? TypeKind::Ptr : kind, dst, a);
		else
			load_into(d, a, TypeKind::I64);
		emit2(mop, width, dst, b);
	}
	// A result known to fit its type non-negatively (fits_; an unsigned one
	// below its width, which a wrapped result would claim), or one computed
	// at 64 bits from canonical operands that never wraps its type
	// (`nowrap`: signed source arithmetic, a bounded loop counter), is
	// already in its canonical form; a 32-bit form zero-extends instead.
	const bool no_wrap = level_ >= 1 && (inst.flags & lowir::IF_NO_WRAP) != 0 && width == TypeKind::I64;
	if (narrow_integer(kind) && !fits_unnormalized_bits(fits_[inst.result], kind) && !no_wrap)
	{
		if (width != TypeKind::I64)
		{
			if (kind == TypeKind::I32)
				normalize(d, kind);
		}
		else if (!bitwise)
			normalize(d, kind);
	}
	finish_result(inst, d);
}

void FunctionSelector::emit_float_binary(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	static const Op ops[] = { Op::FAdd, Op::FSub, Op::FMul, Op::FDiv };
	const unsigned sub = inst.sub;
	if (sub > static_cast<unsigned>(BinaryOp::Div))
		throw BackendError("unsupported floating binary operation");
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	const Operand b = rm_operand(inst.ops[1], inst.type, kEmitScratch);
	if (kind == TypeKind::F80)
	{
		emit3(ops[sub], kind, memory_result(inst), a, b);
		return;
	}
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]), sub == 0 || sub == 2 ? dying_reg(inst.ops[1]) : kNoReg);
	emit3(ops[sub], kind, Operand::make_reg(d), a, b);
	finish_result(inst, d);
}

void FunctionSelector::emit_shift(const Instruction& inst)
{
	const TypeKind kind = inst.type.kind();
	const BinaryOp op = static_cast<BinaryOp>(inst.sub);
	const Operand a = rm_operand(inst.ops[0], inst.type, kSelectScratch);
	const Value cv = resolve(inst.ops[1]);
	Operand count;
	if (cv.kind == ValueKind::Integer)
	{
		count = Operand::make_imm(static_cast<int64_t>(literal(cv).low & 63));
		// A scaled byte offset holds x << (k - log2 scale) (find_scaled_offsets).
		if (op == BinaryOp::Shl && level_ >= 1 && scale_of_[inst.result] != 0)
			count.imm -= log2_of(scale_of_[inst.result]);
	}
	else
	{
		const Operand c = rm_operand(inst.ops[1], type_of(inst.ops[1]), kEmitScratch);
		load_into(RCX, c, TypeKind::I64);
		count = Operand::make_reg(RCX);
	}
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]), kNoReg, 1u << RCX);
	if (a.kind == OperandKind::Mem)
		emit2(Op::Load, kind == TypeKind::Ptr ? TypeKind::Ptr : kind, Operand::make_reg(d), a);
	else
		load_into(d, a, TypeKind::I64);
	const Operand dst = Operand::make_reg(d);
	// O1+: a u32 shift left or arithmetic right by an immediate below 32
	// in its 32-bit form, which zero-extends.
	const bool short_u32 = level_ >= 1 && kind == TypeKind::U32 && count.kind == OperandKind::Imm && count.imm < 32;
	if (short_u32 && op != BinaryOp::UShr)
		emit2(op == BinaryOp::Shl ? Op::Shl : Op::Sar, TypeKind::U32, dst, count);
	else if (op == BinaryOp::Shl)
	{
		emit2(Op::Shl, TypeKind::I64, dst, count);
		if (!fits_unnormalized_bits(fits_[inst.result], kind))
			normalize(d, kind);
	}
	else if (op == BinaryOp::UShr)
	{
		if (signed_integer(kind) && kind != TypeKind::I64)
			normalize(d, unsigned_of(kind));
		emit2(Op::Shr, TypeKind::I64, dst, count);
		if (signed_integer(kind) && kind != TypeKind::I64)
			normalize(d, kind);
	}
	else
	{
		const bool unsigned_kind = narrow_integer(kind) && !signed_integer(kind);
		if (unsigned_kind)
			normalize(d, signed_of(kind));
		emit2(Op::Sar, TypeKind::I64, dst, count);
		if (unsigned_kind)
			normalize(d, kind);
	}
	finish_result(inst, d);
}

void FunctionSelector::emit_compare(const Instruction& inst)
{
	const Type type = inst.type;
	const TypeKind kind = type.kind();
	ComparePredicate pred = static_cast<ComparePredicate>(inst.sub);
	Operand a = rm_operand(inst.ops[0], type, kSelectScratch);
	Operand b = rm_operand(inst.ops[1], type, kEmitScratch);
	if (kind == TypeKind::I128)
	{
		const Reg d = result_reg(inst);
		Inst& mir = emit3(Op::WideSetcc, kind, Operand::make_reg(d), a, b);
		mir.sub = static_cast<uint8_t>(pred);
		finish_result(inst, d);
		return;
	}
	if (type.is_float())
	{
		const Reg d = result_reg(inst);
		Inst& mir = emit3(Op::FSetcc, kind, Operand::make_reg(d), a, b);
		mir.sub = static_cast<uint8_t>(pred);
		finish_result(inst, d);
		return;
	}
	if (b.kind == OperandKind::Symbol)
	{
		load_into(kEmitScratch, b, TypeKind::Ptr);
		b = Operand::make_reg(kEmitScratch);
	}
	if (a.kind == OperandKind::Symbol)
	{
		load_into(kSelectScratch, a, TypeKind::Ptr);
		a = Operand::make_reg(kSelectScratch);
	}
	if (a.kind == OperandKind::Imm && b.kind != OperandKind::Imm)
	{
		std::swap(a, b);
		pred = swapped(pred);
	}
	if (a.kind == OperandKind::Imm || (a.kind == OperandKind::Mem && b.kind == OperandKind::Mem))
	{
		load_into(kSelectScratch, a, kind == TypeKind::Ptr ? TypeKind::Ptr : kind);
		a = Operand::make_reg(kSelectScratch);
	}
	if (b.kind == OperandKind::Imm && compare_scratch(inst))
	{
		load_into(kCompareScratch, b, TypeKind::I64);
		b = Operand::make_reg(kCompareScratch);
	}
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]), dying_reg(inst.ops[1]));
	emit2(Op::Cmp, compare_width(inst), a, b);
	Inst& set = emit1(Op::Setcc, TypeKind::Void, Operand::make_reg(d));
	set.sub = int_cond(pred);
	// O1+: a result whose one reader stores its byte needs no extension.
	const TempInfo& t = temps_[inst.result];
	const Instruction* user = t.uses == 1 && t.user != kNoId ? &body_->instructions[t.user] : nullptr;
	const bool byte_stored = level_ >= 1 && user != nullptr && user->op == Opcode::Store &&
		register_bytes(user->type.kind()) == 1 && resolve(user->ops[0]) == Value::temp(inst.result) &&
		!(resolve(user->ops[1]) == Value::temp(inst.result)) && !(t.flags & (TF_GLOBAL | TF_HOMED)) &&
		d != kSelectScratch && d != kEmitScratch;
	if (!byte_stored && !(level_ >= 1 && byte_only_[inst.result]))
		emit2(Op::Movzx, TypeKind::Void, Operand::make_reg(d), Operand::make_reg(d));
	finish_result(inst, d);
}

// cmov<cc> d, t over d = f, on the condition's flags: a compare fused into
// the select or a test of the condition (a Boolean's byte,
// find_byte_booleans), set first, as d may take a dying operand's
// register. The moves past the flags keep them (the emitter zeroes by mov
// there). t goes to a register (r11 unless in its own); d holding t's
// dying register instead takes f under the inverted condition.
void FunctionSelector::emit_select(const Instruction& inst)
{
	const Value cond = resolve(inst.ops[0]);
	Value t = resolve(inst.ops[1]), f = resolve(inst.ops[2]);
	const TypeKind kind = inst.type.is_pointer() ? TypeKind::Ptr : inst.type.kind();
	Cond cc = CC_NE;
	bool known = cond.kind == ValueKind::Integer, taken = known && (literal(cond).low | literal(cond).high) != 0;
	if (cond.kind == ValueKind::Temp && (temps_[cond.index].flags & TF_FUSED))
		cc = compare_flags(body_->instructions[body_->temps[cond.index].def]);
	else if (!known)
	{
		const Type type = type_of(cond);
		if (!(type.is_integer() || type.is_pointer()) || type.kind() == TypeKind::I128)
			throw BackendError("unsupported select condition");
		const Operand c = rm_operand(cond, type, kSelectScratch);
		known = c.kind != OperandKind::Reg && c.kind != OperandKind::Mem;
		taken = c.kind == OperandKind::Symbol || (c.kind == OperandKind::Imm && (c.imm | c.imm_high) != 0);
		const TypeKind width = cond.kind == ValueKind::Temp && known_boolean(cond.index) ? TypeKind::U8
			: type.is_pointer()                                                          ? TypeKind::Ptr
																						 : type.kind();
		if (c.kind == OperandKind::Reg)
			emit2(Op::Test, width, c, c);
		else if (c.kind == OperandKind::Mem)
			emit2(Op::Cmp, width, c, Operand::make_imm(0));
	}
	// A known condition copies its operand.
	if (known)
		t = f = taken ? t : f;
	// Into a register for cmov's source: its own, or r11.
	auto source = [&](Value v) {
		const Operand from = rm_operand(v, inst.type, kEmitScratch);
		if (from.kind == OperandKind::Reg)
			return from;
		if (from.kind == OperandKind::Mem)
			emit2(Op::Load, kind, Operand::make_reg(kEmitScratch), from);
		else
			load_into(kEmitScratch, from, TypeKind::I64);
		return Operand::make_reg(kEmitScratch);
	};
	const Operand ts = t == f ? Operand::make_imm(0) : source(t);
	const Reg own_t = ts.kind == OperandKind::Reg && ts.reg != kEmitScratch ? ts.reg : kNoReg;
	uint32_t exclude = ts.kind == OperandKind::Reg && ts.reg == kEmitScratch ? 1u << kEmitScratch : 0;
	if (vector_params_)
	{
		// Those parameters' registers become free only after this use.
		const Value ops2[2] = { t, f };
		for (const Value& v : ops2)
			if (v.kind == ValueKind::Temp && v.index < body_->parameter_count && temps_[v.index].reg < 16)
				exclude |= 1u << temps_[v.index].reg;
	}
	const Reg d = result_reg(inst, dying_reg(f), dying_reg(t), exclude);
	if (!(t == f) && d == own_t)
	{
		Inst& mov = emit2(Op::Cmov, TypeKind::Void, Operand::make_reg(d), source(f));
		mov.sub = invert(cc);
		finish_result(inst, d);
		return;
	}
	const Operand from = rm_operand(f, inst.type, d);
	if (from.kind == OperandKind::Mem)
		emit2(Op::Load, kind, Operand::make_reg(d), from);
	else
		load_into(d, from, TypeKind::I64);
	if (!(t == f))
	{
		Inst& mov = emit2(Op::Cmov, TypeKind::Void, Operand::make_reg(d), ts);
		mov.sub = cc;
	}
	finish_result(inst, d);
}

} // namespace native
