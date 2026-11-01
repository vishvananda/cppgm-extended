#include "native/select/function_selector.h"

// The selection of `vector`. The operands of an operation are XMM registers: a
// value in its home reloads into the vector scratch, xmm15 and, for a second
// operand, xmm14. The destructive first operand of a two-operand form may stay
// in memory, since the emitter copies it into the destination.
namespace native {

using lowir::Instruction;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::VectorOp;
using lowir::VectorShape;

namespace {

const Reg kX14 = static_cast<Reg>(XMM0 + 14);
const Reg kX15 = static_cast<Reg>(XMM0 + 15);

Cond test_cond(VectorOp op)
{
	return op == VectorOp::Ptestz ? CC_E : op == VectorOp::Ptestc ? CC_B : CC_A;
}

} // namespace

Operand FunctionSelector::vector_register(Value value, Reg scratch)
{
	const Type type = Type::scalar(TypeKind::V128);
	const Operand op = rm_operand(value, type, scratch);
	if (op.kind == OperandKind::Reg)
		return op;
	load_into(scratch, op, TypeKind::V128);
	return Operand::make_reg(scratch);
}

void FunctionSelector::emit_vector_two_operand(const Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const bool blend = lowir::shape_of(op) == VectorShape::Blend;
	if (blend)
		load_into(XMM0, rm_operand(inst.ops[2], Type::scalar(TypeKind::V128), kX15), TypeKind::V128);
	const Operand b = vector_register(inst.ops[1], kX15);
	Operand a = rm_operand(inst.ops[0], Type::scalar(TypeKind::V128), kX14);
	if (a.kind != OperandKind::Reg && a.kind != OperandKind::Mem)
		a = vector_register(inst.ops[0], kX14);
	const uint32_t exclude = (lowir::vector_commutes(op) ? 0 : 1u << b.reg) | (blend ? 1u << XMM0 : 0);
	const Reg d = result_reg(
		inst, dying_reg(inst.ops[0]), lowir::vector_commutes(op) ? dying_reg(inst.ops[1]) : kNoReg, exclude);
	// A blend's result may be planned in xmm0, which holds its mask.
	const Reg work = blend && d == XMM0 ? kX14 : d;
	Inst& mir = emit3(Op::Vector, TypeKind::V128, Operand::make_reg(work), a, b);
	mir.sub = inst.sub;
	if (lowir::shape_of(op) == VectorShape::BinaryImmediate)
		mir.aux = static_cast<uint32_t>(literal(resolve(inst.ops[2])).low);
	if (work != d)
		emit2(Op::FMov, TypeKind::V128, Operand::make_reg(d), Operand::make_reg(work));
	finish_result(inst, d);
}

void FunctionSelector::emit_vector_from_scalar(const Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const TypeKind kind = lowir::vector_scalar_kind(op);
	const Operand src = Type::scalar(kind).is_float() ? rm_operand(inst.ops[0], Type::scalar(kind), kSelectScratch)
													  : reg_operand(inst.ops[0], Type::scalar(kind), kSelectScratch);
	const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
	// A load or a literal already clears the other lanes.
	if (src.kind == OperandKind::Reg)
		emit2(Op::Vector, TypeKind::V128, Operand::make_reg(d), src).sub = inst.sub;
	else
		load_into(d, src, kind);
	finish_result(inst, d);
}

void FunctionSelector::emit_vector_to_scalar(const Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const TypeKind kind = lowir::vector_scalar_kind(op);
	if (Type::scalar(kind).is_float())
	{
		const Operand src = vector_register(inst.ops[0], kX15);
		const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
		load_into(d, src, TypeKind::V128);
		finish_result(inst, d);
		return;
	}
	const Operand src = vector_register(inst.ops[0], kX15);
	const Reg d = result_reg(inst);
	emit2(Op::Vector, TypeKind::V128, Operand::make_reg(d), src).sub = inst.sub;
	normalize(d, kind);
	finish_result(inst, d);
}

void FunctionSelector::emit_vector(const Instruction& inst)
{
	const VectorOp op = static_cast<VectorOp>(inst.sub);
	const VectorShape shape = lowir::shape_of(op);
	switch (shape)
	{
	case VectorShape::Binary:
	case VectorShape::BinaryImmediate:
	case VectorShape::Blend:
		emit_vector_two_operand(inst);
		break;
	case VectorShape::FromScalar:
		emit_vector_from_scalar(inst);
		break;
	case VectorShape::ToScalar:
		emit_vector_to_scalar(inst);
		break;
	case VectorShape::Zero:
	{
		const Reg d = result_reg(inst);
		emit1(Op::Vector, TypeKind::V128, Operand::make_reg(d)).sub = inst.sub;
		finish_result(inst, d);
		break;
	}
	case VectorShape::Unary:
	case VectorShape::UnaryImmediate:
	case VectorShape::Mask:
	{
		const Operand a = vector_register(inst.ops[0], kX15);
		const bool immediate = shape == VectorShape::UnaryImmediate;
		const Reg d = shape == VectorShape::Mask ? result_reg(inst) : result_reg(inst, dying_reg(inst.ops[0]));
		Inst& mir = emit2(Op::Vector, TypeKind::V128, Operand::make_reg(d), a);
		mir.sub = inst.sub;
		if (immediate)
			mir.aux = static_cast<uint32_t>(literal(resolve(inst.ops[2])).low);
		finish_result(inst, d);
		break;
	}
	case VectorShape::Test:
	{
		const Operand a = vector_register(inst.ops[0], kX14);
		const Operand b = vector_register(inst.ops[1], kX15);
		const Reg d = result_reg(inst);
		emit2(Op::Vector, TypeKind::V128, a, b).sub = inst.sub;
		Inst& set = emit1(Op::Setcc, TypeKind::Void, Operand::make_reg(d));
		set.sub = test_cond(op);
		emit2(Op::Movzx, TypeKind::Void, Operand::make_reg(d), Operand::make_reg(d));
		finish_result(inst, d);
		break;
	}
	}
}

} // namespace native
