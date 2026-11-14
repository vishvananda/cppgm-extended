#include "opt/target_cost.h"

namespace opt {

using namespace lowir;

namespace {

uint32_t vector_cost(VectorOp op)
{
	switch (op)
	{
	case VectorOp::Pclmulqdq:
	case VectorOp::Phminposuw:
		return UINT32_MAX;
	case VectorOp::Rcpps:
	case VectorOp::Rcpss:
	case VectorOp::Rsqrtps:
	case VectorOp::Rsqrtss:
		return 32;
	case VectorOp::Dpps:
	case VectorOp::Dppd:
		return 16;
	case VectorOp::Cvtps2dq:
	case VectorOp::Cvtpd2dq:
	case VectorOp::Cvttps2dq:
	case VectorOp::Cvttpd2dq:
		return 12;
	default:
		break;
	}
	// Lane-preserving scalar float operations need extraction/replacement;
	// shuffles and horizontal/reduction operations may need several SIMD ops.
	if (op >= VectorOp::Addss && op <= VectorOp::Cvtsd2ss)
		return 6;
	switch (shape_of(op))
	{
	case VectorShape::Zero:
	case VectorShape::FromScalar:
	case VectorShape::ToScalar:
	case VectorShape::Mask:
		return 2;
	case VectorShape::Unary:
	case VectorShape::Binary:
		return 3;
	case VectorShape::UnaryImmediate:
	case VectorShape::BinaryImmediate:
	case VectorShape::Blend:
	case VectorShape::Test:
		return 8;
	}
	return UINT32_MAX;
}

uint32_t wide_cost(const Instruction& inst)
{
	if (inst.type.kind() == TypeKind::F80 ||
		(inst.op == Opcode::Convert && convert_source_type(inst).kind() == TypeKind::F80))
	{
		if (inst.op == Opcode::Const || inst.op == Opcode::Load || inst.op == Opcode::Store || inst.op == Opcode::Copy)
			return 4;
		// Binary128 arithmetic/conversion uses the supplied helper ABI and
		// explicit result storage. It is not the native x87 instruction.
		return inst.op == Opcode::Select ? 8 : 16;
	}
	if (inst.op == Opcode::Const || inst.op == Opcode::Load || inst.op == Opcode::Store)
		return 4;
	if (inst.op == Opcode::Binary)
	{
		switch (static_cast<BinaryOp>(inst.sub))
		{
		case BinaryOp::And:
		case BinaryOp::Or:
		case BinaryOp::Xor:
			return 2;
		case BinaryOp::Add:
		case BinaryOp::Sub:
			return 6;
		case BinaryOp::Shl:
		case BinaryOp::Shr:
		case BinaryOp::UShr:
			return 16;
		default:
			return 24;
		}
	}
	return inst.op == Opcode::Compare ? 8 : 6;
}

} // namespace

uint32_t wasm_inline_instruction_cost(const Instruction& inst)
{
	switch (inst.op)
	{
	case Opcode::Return:
	case Opcode::Unreachable:
	case Opcode::Copy:
		return 0;
	case Opcode::Call:
		return 6 + inst.extra_count;
	case Opcode::Index:
		return 2; // explicit scale multiply and pointer addition
	case Opcode::Switch:
		return 2 + inst.extra_count;
	case Opcode::Vector:
		return vector_cost(static_cast<VectorOp>(inst.sub));
	case Opcode::CopyObject:
	case Opcode::ZeroInit:
		return inst.immediate < 0 ? UINT32_MAX : 4; // memory.copy / memory.fill
	case Opcode::EhTry:
		return 12; // try_table, context storage and the personality call
	case Opcode::EhCleanup:
	case Opcode::EhCatch:
	case Opcode::EhFilter:
	case Opcode::EhCatchAll:
	case Opcode::EhEnd:
	case Opcode::Exception:
	case Opcode::ExceptionSelector:
	case Opcode::Resume:
	case Opcode::Throw:
		return 4;
	case Opcode::Jump:
	case Opcode::Branch:
	case Opcode::Phi:
		return 1;
	default:
		break;
	}
	const Type source = inst.op == Opcode::Convert ? convert_source_type(inst) : inst.type;
	if (inst.type.is_object())
		return inst.op == Opcode::Load || inst.op == Opcode::Store ? 4 : UINT32_MAX;
	if (inst.type.kind() == TypeKind::F80 || source.kind() == TypeKind::F80 || inst.type.kind() == TypeKind::I128 ||
		source.kind() == TypeKind::I128)
		return wide_cost(inst);
	switch (inst.op)
	{
	case Opcode::Const:
	case Opcode::Addr:
	case Opcode::Load:
	case Opcode::Store:
	case Opcode::Compare:
	case Opcode::Select:
		return 1;
	case Opcode::Unary:
		return static_cast<UnaryOp>(inst.sub) == UnaryOp::ByteSwap ? 4 * storage_bytes(inst.type) : 1;
	case Opcode::Binary:
		return storage_bytes(inst.type) < 4 ? 3 : 1;
	case Opcode::Convert:
		return 1;
	default:
		return UINT32_MAX;
	}
}

} // namespace opt
