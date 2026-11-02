#include "native/select/function_selector.h"

#include "support/internal_error.h"

// Conversions. Integer extensions and truncations act on the normalized
// 64-bit register form: a sign extension of a signed source and a zero
// extension of an unsigned one are aliases (resolved in analysis), the
// others re-extend the source's low bits and renormalize to the
// destination type. Integer/floating conversions go through the 64-bit
// signed SSE forms whenever the normalized source fits; full-width
// unsigned forms and every f80 form are emitter pseudo instructions; an
// i128 one calls the runtime's routine (libgcc's __floattidf, __fixdfti,
// ... names; the result in a register or, for i128 and f80, memory).
namespace native {

using lowir::ConvertOp;
using lowir::Instruction;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::ValueKind;

namespace {

TypeKind flip_sign(TypeKind kind)
{
	switch (kind)
	{
	case TypeKind::I8:
		return TypeKind::U8;
	case TypeKind::I16:
		return TypeKind::U16;
	case TypeKind::I32:
		return TypeKind::U32;
	case TypeKind::U8:
		return TypeKind::I8;
	case TypeKind::U16:
		return TypeKind::I16;
	case TypeKind::U32:
		return TypeKind::I32;
	default:
		return kind;
	}
}

unsigned bits_of(TypeKind kind)
{
	return kind == TypeKind::Ptr ? 64 : lowir::integer_bits(kind);
}

} // namespace

void FunctionSelector::emit_int_convert(const Instruction& inst)
{
	const ConvertOp op = static_cast<ConvertOp>(inst.sub);
	const TypeKind st = lowir::convert_source_type(inst).kind();
	const TypeKind dt = inst.type.kind();
	const Operand src = rm_operand(inst.ops[0], lowir::convert_source_type(inst), kSelectScratch);
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
	if (src.kind == OperandKind::Mem && op == ConvertOp::Trunc)
	{
		// A typed load reads the low bytes; i1 keeps only their low bit.
		emit2(Op::Load, dt, Operand::make_reg(d), src);
		if (dt == TypeKind::I1)
			normalize(d, dt);
		finish_result(inst, d);
		return;
	}
	if (src.kind == OperandKind::Mem)
		emit2(Op::Load, st, Operand::make_reg(d), src);
	else
		load_into(d, src, TypeKind::I64);
	// A register source no reader reads above its width holds its value in
	// the low bits alone.
	const Value v = resolve(inst.ops[0]);
	const bool raw = src.kind == OperandKind::Reg && v.kind == ValueKind::Temp &&
		(low_read(v.index) || (inst.ops[0].kind == ValueKind::Temp && low_read(inst.ops[0].index)));
	switch (op)
	{
	case ConvertOp::SExt:
		if (raw && signed_narrow(st))
			normalize(d, st);
		if (st == TypeKind::I1)
			emit1(Op::Neg, TypeKind::Void, Operand::make_reg(d));
		else if (unsigned_narrow(st))
			normalize(d, flip_sign(st));
		if (unsigned_narrow(dt) || dt == TypeKind::I1)
			normalize(d, dt);
		break;
	case ConvertOp::ZExt:
		if (signed_narrow(st))
			normalize(d, flip_sign(st));
		else if (raw && unsigned_narrow(st))
			normalize(d, st);
		if (bits_of(dt) <= bits_of(st))
			normalize(d, dt);
		break;
	default:
		normalize(d, dt);
		break;
	}
	finish_result(inst, d);
}

// The high half of a 64-bit product (find_high_products): a in rax, one
// mul (or imul) of b, the half in rdx. A factor is the value an extension
// the product alone reads extends, a literal, or the low half of an
// extension's home.
void FunctionSelector::emit_high_product(const Instruction& inst)
{
	const lowir::FunctionBody& body = *body_;
	auto def_of = [&](Value v) -> const Instruction& { return body.instructions[body.temps[resolve(v).index].def]; };
	const Instruction& product = def_of(def_of(inst.ops[0]).ops[0]);
	const Type i64 = Type::scalar(TypeKind::I64);
	auto factor = [&](Value v, Reg scratch) {
		v = resolve(v);
		if (v.kind == ValueKind::Temp && (temps_[v.index].flags & TF_ROTATED))
			return rm_operand(def_of(v).ops[0], i64, scratch);
		if (v.kind == ValueKind::Integer)
			return Operand::make_imm(static_cast<int64_t>(literal(v).low));
		return rm_operand(v, i64, scratch);
	};
	const Operand a = factor(product.ops[0], kSelectScratch);
	Operand b = factor(product.ops[1], kEmitScratch);
	if (b.kind != OperandKind::Reg && b.kind != OperandKind::Mem)
	{
		load_into(kEmitScratch, b, TypeKind::I64);
		b = Operand::make_reg(kEmitScratch);
	}
	if (a.kind == OperandKind::Mem)
		emit2(Op::Load, TypeKind::I64, Operand::make_reg(RAX), a);
	else
		load_into(RAX, a, TypeKind::I64);
	emit1(high_product_[inst.result] == 1 ? Op::MulWide : Op::ImulWide, TypeKind::I64, b);
	const Reg d = result_reg(inst, RDX);
	load_into(d, Operand::make_reg(RDX), TypeKind::I64);
	finish_result(inst, d);
}

void FunctionSelector::emit_convert(const Instruction& inst)
{
	if (level_ >= 1 && inst.result != lowir::kNoId && high_product_[inst.result] != 0)
	{
		emit_high_product(inst);
		return;
	}
	const ConvertOp op = static_cast<ConvertOp>(inst.sub);
	const Type source = lowir::convert_source_type(inst);
	const TypeKind st = source.kind();
	const TypeKind dt = inst.type.kind();
	if (op == ConvertOp::SExt || op == ConvertOp::ZExt || op == ConvertOp::Trunc)
	{
		if (dt == TypeKind::I128)
		{
			Operand src = reg_operand(inst.ops[0], source, kEmitScratch);
			if ((op == ConvertOp::ZExt && signed_narrow(st)) || (op == ConvertOp::SExt && unsigned_narrow(st)))
			{
				load_into(kEmitScratch, src, TypeKind::I64);
				normalize(kEmitScratch, flip_sign(st));
				src = Operand::make_reg(kEmitScratch);
			}
			Inst& mir = emit2(Op::Wide, TypeKind::I128, memory_result(inst), src);
			mir.sub = static_cast<uint8_t>(op == ConvertOp::ZExt || st == TypeKind::I1 ? WideOp::ZExt : WideOp::SExt);
			return;
		}
		if (st == TypeKind::I128)
		{
			const Operand src = rm_operand(inst.ops[0], source, kSelectScratch);
			const Reg d = result_reg(inst);
			if (src.kind == OperandKind::Mem)
			{
				emit2(Op::Load, dt, Operand::make_reg(d), src);
				if (dt == TypeKind::I1)
					normalize(d, dt);
			}
			else
			{
				load_into(d, Operand::make_imm(src.imm), TypeKind::I64);
				normalize(d, dt);
			}
			finish_result(inst, d);
			return;
		}
		emit_int_convert(inst);
		return;
	}
	ConvKind kind = ConvKind::SIToFP;
	if (st == TypeKind::F80 || dt == TypeKind::F80)
		needs_scratch_area_ = true;
	switch (op)
	{
	case ConvertOp::SIToFP:
	case ConvertOp::UIToFP:
	{
		if (st == TypeKind::I128)
		{
			const Operand wide = rm_operand(inst.ops[0], source, kSelectScratch);
			kind = op == ConvertOp::UIToFP ? ConvKind::UIToFP : ConvKind::SIToFP;
			if (dt == TypeKind::F80)
			{
				Inst& mir = emit2(Op::Convert, st, memory_result(inst), wide);
				mir.type2 = dt;
				mir.sub = static_cast<uint8_t>(kind);
				return;
			}
			const Reg d = result_reg(inst);
			Inst& mir = emit2(Op::Convert, st, Operand::make_reg(d), wide);
			mir.type2 = dt;
			mir.sub = static_cast<uint8_t>(kind);
			finish_result(inst, d);
			return;
		}
		Operand src = rm_operand(inst.ops[0], source, kEmitScratch);
		const bool adjust = (op == ConvertOp::SIToFP && unsigned_narrow(st)) ||
			(op == ConvertOp::UIToFP && signed_narrow(st));
		if (src.kind != OperandKind::Reg || adjust)
		{
			if (src.kind == OperandKind::Mem)
				emit2(Op::Load, st == TypeKind::Ptr ? TypeKind::Ptr : st, Operand::make_reg(kEmitScratch), src);
			else
				load_into(kEmitScratch, src, TypeKind::I64);
			if (adjust)
				normalize(kEmitScratch, flip_sign(st));
			src = Operand::make_reg(kEmitScratch);
		}
		kind = op == ConvertOp::UIToFP && bits_of(st) == 64 ? ConvKind::UIToFP : ConvKind::SIToFP;
		if (dt == TypeKind::F80)
		{
			Inst& mir = emit2(Op::Convert, st, memory_result(inst), src);
			mir.type2 = dt;
			mir.sub = static_cast<uint8_t>(kind);
			return;
		}
		const Reg d = result_reg(inst);
		Inst& mir = emit2(Op::Convert, st, Operand::make_reg(d), src);
		mir.type2 = dt;
		mir.sub = static_cast<uint8_t>(kind);
		finish_result(inst, d);
		return;
	}
	case ConvertOp::FPToSI:
	case ConvertOp::FPToUI:
	{
		const Operand src = rm_operand(inst.ops[0], source, kSelectScratch);
		if (dt == TypeKind::I128)
		{
			Inst& mir = emit2(Op::Convert, st, memory_result(inst), src);
			mir.type2 = dt;
			mir.sub = static_cast<uint8_t>(op == ConvertOp::FPToUI ? ConvKind::FPToUI : ConvKind::FPToSI);
			return;
		}
		kind = op == ConvertOp::FPToUI && bits_of(dt) == 64 ? ConvKind::FPToUI : ConvKind::FPToSI;
		const Reg d = result_reg(inst);
		Inst& mir = emit2(Op::Convert, st, Operand::make_reg(d), src);
		mir.type2 = dt;
		mir.sub = static_cast<uint8_t>(kind);
		normalize(d, dt);
		finish_result(inst, d);
		return;
	}
	case ConvertOp::FPExt:
	case ConvertOp::FPTrunc:
	{
		const Operand src = rm_operand(inst.ops[0], source, kSelectScratch);
		kind = op == ConvertOp::FPExt ? ConvKind::FPExt : ConvKind::FPTrunc;
		if (dt == TypeKind::F80)
		{
			Inst& mir = emit2(Op::Convert, st, memory_result(inst), src);
			mir.type2 = dt;
			mir.sub = static_cast<uint8_t>(kind);
			return;
		}
		const Reg d = result_reg(inst, st == TypeKind::F80 ? kNoReg : dying_reg(inst.ops[0]));
		Inst& mir = emit2(Op::Convert, st, Operand::make_reg(d), src);
		mir.type2 = dt;
		mir.sub = static_cast<uint8_t>(kind);
		finish_result(inst, d);
		return;
	}
	default:
		throw InternalError("unsupported conversion");
	}
}

} // namespace native
