#include "opt/scalar_fold.h"

#include <cmath>
#include <cstring>

#include "lowir/literals.h"

namespace opt {

using namespace lowir;

namespace {

typedef unsigned __int128 u128;
typedef __int128 s128;

u128 to_u128(Literal bits)
{
	return (static_cast<u128>(bits.high) << 64) | bits.low;
}

Literal from_u128(u128 value)
{
	Literal bits;
	bits.low = static_cast<uint64_t>(value);
	bits.high = static_cast<uint64_t>(value >> 64);
	return bits;
}

unsigned width_of(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I1:
		return 1;
	case TypeKind::I8:
	case TypeKind::U8:
		return 8;
	case TypeKind::I16:
	case TypeKind::U16:
		return 16;
	case TypeKind::I32:
	case TypeKind::U32:
		return 32;
	case TypeKind::I64:
	case TypeKind::Ptr:
		return 64;
	case TypeKind::I128:
		return 128;
	default:
		return 0;
	}
}

bool is_signed(Type type)
{
	switch (type.kind())
	{
	case TypeKind::I8:
	case TypeKind::I16:
	case TypeKind::I32:
	case TypeKind::I64:
	case TypeKind::I128:
	case TypeKind::Ptr:
		return true;
	default:
		return false;
	}
}

u128 mask_of(unsigned width)
{
	return width >= 128 ? ~static_cast<u128>(0) : (static_cast<u128>(1) << width) - 1;
}

// The value reduced to `width` bits and extended back by `sign`.
u128 extend(u128 value, unsigned width, bool sign)
{
	if (width >= 128)
		return value;
	value &= mask_of(width);
	if (sign && width > 0 && ((value >> (width - 1)) & 1) != 0)
		value |= ~mask_of(width);
	return value;
}

// The integer bits of a constant read as an integer of `width` bits.
bool raw_integer(const Constant& c, u128& out)
{
	if (c.kind != ValueKind::Integer)
		return false;
	out = to_u128(c.bits);
	return true;
}

long double float_of(const Constant& c, TypeKind kind)
{
	if (c.kind == ValueKind::Float)
		return float_value(c.float_kind, c.bits);
	// An integer literal in a floating context is its value.
	return static_cast<long double>(static_cast<s128>(to_u128(c.bits)));
}

Constant float_constant(TypeKind kind, long double value)
{
	Constant c;
	c.kind = ValueKind::Float;
	c.float_kind = kind;
	c.bits = float_literal(kind, value);
	return c;
}

// The value rounded to `kind` the way the back end stores it.
long double round_to(TypeKind kind, long double value)
{
	if (kind == TypeKind::F32)
		return static_cast<float>(value);
	if (kind == TypeKind::F64)
		return static_cast<double>(value);
	return value;
}

bool fold_float_binary(BinaryOp op, TypeKind kind, long double a, long double b, Constant& out)
{
	if (kind == TypeKind::F32)
	{
		const float x = static_cast<float>(a), y = static_cast<float>(b);
		volatile float r;
		switch (op)
		{
		case BinaryOp::Add:
			r = x + y;
			break;
		case BinaryOp::Sub:
			r = x - y;
			break;
		case BinaryOp::Mul:
			r = x * y;
			break;
		case BinaryOp::Div:
			r = x / y;
			break;
		default:
			return false;
		}
		out = float_constant(kind, r);
		return true;
	}
	// f64 is evaluated in x87 extended precision and rounded by the store.
	long double r;
	switch (op)
	{
	case BinaryOp::Add:
		r = a + b;
		break;
	case BinaryOp::Sub:
		r = a - b;
		break;
	case BinaryOp::Mul:
		r = a * b;
		break;
	case BinaryOp::Div:
		r = a / b;
		break;
	default:
		return false;
	}
	out = float_constant(kind, round_to(kind, r));
	return true;
}

u128 byte_swap(u128 value, unsigned width)
{
	u128 r = 0;
	for (unsigned i = 0; i < width / 8; ++i)
		r = (r << 8) | ((value >> (8 * i)) & 0xff);
	return r;
}

} // namespace

Literal normalize_integer(Type type, Literal bits)
{
	const unsigned width = width_of(type);
	if (width == 0)
		return bits;
	if (width <= 64)
	{
		// extend() in the low half: the high half is the sign's.
		const uint64_t mask = width == 64 ? ~0ull : (1ull << width) - 1;
		const bool negative = is_signed(type) && ((bits.low >> (width - 1)) & 1) != 0;
		Literal out;
		out.low = negative ? bits.low | ~mask : bits.low & mask;
		out.high = negative ? ~0ull : 0;
		return out;
	}
	return from_u128(extend(to_u128(bits), width, is_signed(type)));
}

bool integer_of(Type type, const Constant& c, Literal& out)
{
	if (c.kind != ValueKind::Integer)
		return false;
	out = normalize_integer(type, c.bits);
	return true;
}

Constant integer_constant(Type type, Literal bits)
{
	Constant c;
	c.kind = ValueKind::Integer;
	c.float_kind = TypeKind::Void;
	c.bits = normalize_integer(type, bits);
	return c;
}

Constant truth_constant(bool value)
{
	Literal bits;
	bits.low = value ? 1 : 0;
	bits.high = 0;
	return integer_constant(Type::scalar(TypeKind::I64), bits);
}

bool fold_unary(UnaryOp op, Type type, const Constant& a, Constant& out)
{
	if (type.is_float())
	{
		if (op != UnaryOp::Neg || a.kind != ValueKind::Float || a.float_kind != type.kind())
			return false;
		// Negation flips the sign bit only (the back end's xor): a NaN keeps
		// its payload, a signaling one stays signaling.
		out = a;
		if (type.kind() == TypeKind::F32)
			out.bits.low ^= 0x80000000ull;
		else if (type.kind() == TypeKind::F64)
			out.bits.low ^= 0x8000000000000000ull;
		else
			out.bits.high ^= 0x8000ull;
		return true;
	}
	const unsigned width = width_of(type);
	u128 x;
	if (width == 0 || !raw_integer(a, x))
		return false;
	x = extend(x, width, is_signed(type));
	u128 r;
	switch (op)
	{
	case UnaryOp::Neg:
		r = 0 - x;
		break;
	case UnaryOp::Not:
		r = (x & mask_of(width)) == 0 ? 1 : 0;
		break;
	case UnaryOp::BitNot:
		r = ~x;
		break;
	case UnaryOp::ByteSwap:
		if (width != 16 && width != 32 && width != 64)
			return false;
		r = byte_swap(x, width);
		break;
	case UnaryOp::CountTrailingZeros:
	case UnaryOp::CountLeadingZeros:
	{
		// Undefined for 0: left unfolded.
		const u128 bits = x & mask_of(width);
		if ((width != 32 && width != 64) || bits == 0)
			return false;
		unsigned n = 0;
		if (op == UnaryOp::CountTrailingZeros)
			while (((bits >> n) & 1) == 0)
				++n;
		else
			while (((bits >> (width - 1 - n)) & 1) == 0)
				++n;
		r = n;
		break;
	}
	default:
		return false;
	}
	out = integer_constant(type, from_u128(r));
	return true;
}

bool fold_binary(BinaryOp op, Type type, const Constant& a, const Constant& b, Constant& out)
{
	if (type.is_float())
		return fold_float_binary(op, type.kind(), float_of(a, type.kind()), float_of(b, type.kind()), out);
	const unsigned width = width_of(type);
	u128 x, y;
	if (width == 0 || width == 1 || !raw_integer(a, x) || !raw_integer(b, y))
		return false;
	const bool sign = is_signed(type);
	const u128 sx = extend(x, width, true), sy = extend(y, width, true);
	const u128 ux = x & mask_of(width), uy = y & mask_of(width);
	const u128 min = static_cast<u128>(1) << (width - 1);
	u128 r;
	switch (op)
	{
	case BinaryOp::Add:
		r = x + y;
		break;
	case BinaryOp::Sub:
		r = x - y;
		break;
	case BinaryOp::Mul:
		r = x * y;
		break;
	case BinaryOp::And:
		r = x & y;
		break;
	case BinaryOp::Or:
		r = x | y;
		break;
	case BinaryOp::Xor:
		r = x ^ y;
		break;
	case BinaryOp::Div:
	case BinaryOp::Mod:
		if (uy == 0 || (ux == min && uy == mask_of(width)))
			return false;
		r = op == BinaryOp::Div ? static_cast<u128>(static_cast<s128>(sx) / static_cast<s128>(sy))
								: static_cast<u128>(static_cast<s128>(sx) % static_cast<s128>(sy));
		break;
	case BinaryOp::UDiv:
	case BinaryOp::UMod:
		if (uy == 0)
			return false;
		r = op == BinaryOp::UDiv ? ux / uy : ux % uy;
		break;
	case BinaryOp::Shl:
	case BinaryOp::Shr:
	case BinaryOp::UShr:
		if (uy >= width)
			return false;
		r = op == BinaryOp::Shl    ? x << static_cast<unsigned>(uy)
			: op == BinaryOp::UShr ? ux >> static_cast<unsigned>(uy)
								   : static_cast<u128>(static_cast<s128>(sx) >> static_cast<unsigned>(uy));
		break;
	default:
		return false;
	}
	(void)sign;
	out = integer_constant(type, from_u128(r));
	return true;
}

bool fold_compare(ComparePredicate pred, Type type, const Constant& a, const Constant& b, Constant& out)
{
	if (type.is_float())
	{
		const long double x = float_of(a, type.kind()), y = float_of(b, type.kind());
		bool r;
		switch (pred)
		{
		case ComparePredicate::Eq:
			r = x == y;
			break;
		case ComparePredicate::Ne:
			r = x != y;
			break;
		case ComparePredicate::Lt:
			r = x < y;
			break;
		case ComparePredicate::Le:
			r = x <= y;
			break;
		case ComparePredicate::Gt:
			r = x > y;
			break;
		case ComparePredicate::Ge:
			r = x >= y;
			break;
		default:
			return false;
		}
		out = truth_constant(r);
		return true;
	}
	const unsigned width = width_of(type);
	u128 x, y;
	if (width == 0 || !raw_integer(a, x) || !raw_integer(b, y))
		return false;
	// i1 is unsigned (0 or 1) for the signed predicates too, as the back
	// end compares it.
	const bool sign = width > 1;
	const s128 sx = static_cast<s128>(extend(x, width, sign)), sy = static_cast<s128>(extend(y, width, sign));
	const u128 ux = x & mask_of(width), uy = y & mask_of(width);
	bool r;
	switch (pred)
	{
	case ComparePredicate::Eq:
		r = ux == uy;
		break;
	case ComparePredicate::Ne:
		r = ux != uy;
		break;
	case ComparePredicate::Lt:
		r = sx < sy;
		break;
	case ComparePredicate::Le:
		r = sx <= sy;
		break;
	case ComparePredicate::Gt:
		r = sx > sy;
		break;
	case ComparePredicate::Ge:
		r = sx >= sy;
		break;
	case ComparePredicate::Ult:
		r = ux < uy;
		break;
	case ComparePredicate::Ule:
		r = ux <= uy;
		break;
	case ComparePredicate::Ugt:
		r = ux > uy;
		break;
	case ComparePredicate::Uge:
		r = ux >= uy;
		break;
	default:
		return false;
	}
	out = truth_constant(r);
	return true;
}

bool fold_convert(ConvertOp op, Type to, Type from, const Constant& a, Constant& out)
{
	switch (op)
	{
	case ConvertOp::SExt:
	case ConvertOp::ZExt:
	case ConvertOp::Trunc:
	{
		const unsigned width = width_of(from);
		u128 x;
		if (width == 0 || width_of(to) == 0 || !raw_integer(a, x))
			return false;
		x = extend(x, width, op == ConvertOp::SExt);
		if (op == ConvertOp::ZExt)
			x &= mask_of(width);
		out = integer_constant(to, from_u128(x));
		return true;
	}
	case ConvertOp::SIToFP:
	case ConvertOp::UIToFP:
	{
		const unsigned width = width_of(from);
		u128 x;
		if (width == 0 || width > 64 || !to.is_float() || !raw_integer(a, x))
			return false;
		const long double v = op == ConvertOp::SIToFP
			? static_cast<long double>(static_cast<int64_t>(static_cast<uint64_t>(extend(x, width, true))))
			: static_cast<long double>(static_cast<uint64_t>(x & mask_of(width)));
		out = float_constant(to.kind(), round_to(to.kind(), v));
		return true;
	}
	case ConvertOp::FPToSI:
	case ConvertOp::FPToUI:
	{
		const unsigned width = width_of(to);
		if (width == 0 || width > 64 || a.kind != ValueKind::Float || !from.is_float())
			return false;
		const long double v = std::trunc(float_of(a, from.kind()));
		if (std::isnan(v))
			return false;
		const long double limit = std::ldexp(1.0L, static_cast<int>(width) - (op == ConvertOp::FPToSI ? 1 : 0));
		const long double low = op == ConvertOp::FPToSI ? -limit : 0.0L;
		if (!(v >= low && v < limit))
			return false;
		const u128 r = op == ConvertOp::FPToSI ? static_cast<u128>(static_cast<s128>(static_cast<int64_t>(v)))
											   : static_cast<u128>(static_cast<uint64_t>(v));
		out = integer_constant(to, from_u128(r));
		return true;
	}
	case ConvertOp::FPExt:
	case ConvertOp::FPTrunc:
		if (!to.is_float() || !from.is_float())
			return false;
		out = float_constant(to.kind(), round_to(to.kind(), float_of(a, from.kind())));
		return true;
	}
	return false;
}

bool fold_copy(Type type, const Constant& a, Constant& out)
{
	if (type.is_float())
	{
		if (a.kind != ValueKind::Float || a.float_kind != type.kind())
			return false;
		out = a;
		return true;
	}
	if (width_of(type) == 0 || a.kind != ValueKind::Integer)
		return false;
	out = integer_constant(type, a.bits);
	return true;
}

bool same_constant(Type type, const Constant& a, const Constant& b)
{
	if (type.is_float())
	{
		if (a.kind != ValueKind::Float || b.kind != ValueKind::Float || a.float_kind != b.float_kind)
			return false;
		return a.bits.low == b.bits.low && a.bits.high == b.bits.high;
	}
	Literal x, y;
	if (!integer_of(type, a, x) || !integer_of(type, b, y))
		return false;
	return x.low == y.low && x.high == y.high;
}

bool integer_bits(Type type, const Constant& c, Literal& bits)
{
	return !type.is_float() && width_of(type) != 0 && integer_of(type, c, bits);
}

bool is_integer(Type type, const Constant& c, int64_t value)
{
	Literal bits;
	if (!integer_bits(type, c, bits))
		return false;
	Literal want;
	want.low = static_cast<uint64_t>(value);
	want.high = value < 0 ? ~0ull : 0;
	want = normalize_integer(type, want);
	return bits.low == want.low && bits.high == want.high;
}

bool is_all_ones(Type type, const Constant& c)
{
	const unsigned width = width_of(type);
	u128 x;
	if (width == 0 || !raw_integer(c, x))
		return false;
	return (x & mask_of(width)) == mask_of(width);
}

} // namespace opt
