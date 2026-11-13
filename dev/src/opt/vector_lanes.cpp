#include "opt/vector_lanes.h"

namespace opt {

using namespace lowir;

unsigned lane_bytes(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::U8:
		return 1;
	case TypeKind::I16:
	case TypeKind::U16:
		return 2;
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::F32:
		return 4;
	case TypeKind::I64:
	case TypeKind::F64:
		return 8;
	default:
		return 0;
	}
}

// The operation on each lane of a binary operator, or false.
bool lane_operation(BinaryOp op, Type type, VectorOp& out)
{
	const unsigned bytes = lane_bytes(type);
	if (type.is_float())
	{
		const bool wide_lanes = bytes == 8;
		switch (op)
		{
		case BinaryOp::Add:
			out = wide_lanes ? VectorOp::Addpd : VectorOp::Addps;
			return true;
		case BinaryOp::Sub:
			out = wide_lanes ? VectorOp::Subpd : VectorOp::Subps;
			return true;
		case BinaryOp::Mul:
			out = wide_lanes ? VectorOp::Mulpd : VectorOp::Mulps;
			return true;
		case BinaryOp::Div:
			out = wide_lanes ? VectorOp::Divpd : VectorOp::Divps;
			return true;
		default:
			return false;
		}
	}
	switch (op)
	{
	case BinaryOp::Add:
		out = bytes == 1 ? VectorOp::Paddb
			: bytes == 2 ? VectorOp::Paddw
			: bytes == 4 ? VectorOp::Paddd
						 : VectorOp::Paddq;
		return true;
	case BinaryOp::Sub:
		out = bytes == 1 ? VectorOp::Psubb
			: bytes == 2 ? VectorOp::Psubw
			: bytes == 4 ? VectorOp::Psubd
						 : VectorOp::Psubq;
		return true;
	case BinaryOp::And:
		out = VectorOp::Pand;
		return true;
	case BinaryOp::Or:
		out = VectorOp::Por;
		return true;
	case BinaryOp::Xor:
		out = VectorOp::Pxor;
		return true;
	case BinaryOp::Mul:
		out = VectorOp::Pmullw;
		return bytes == 2;
	default:
		return false;
	}
}

} // namespace opt
