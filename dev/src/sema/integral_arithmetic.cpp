#include "sema/integral_arithmetic.h"

#include "sema/constant_values.h"
#include "sema/type_table.h"

namespace sema {

namespace {

bool FundamentalKind(FundamentalType type, IntegralKind& kind)
{
	switch (type)
	{
	case FundamentalType::Bool:
		kind = IntegralKind{ 1, false };
		return true;
	case FundamentalType::Char:
	case FundamentalType::SignedChar:
		kind = IntegralKind{ 8, true };
		return true;
	case FundamentalType::UnsignedChar:
		kind = IntegralKind{ 8, false };
		return true;
	case FundamentalType::ShortInt:
		kind = IntegralKind{ 16, true };
		return true;
	case FundamentalType::UnsignedShortInt:
	case FundamentalType::Char16T:
		kind = IntegralKind{ 16, false };
		return true;
	case FundamentalType::Int:
	case FundamentalType::WcharT:
		kind = IntegralKind{ 32, true };
		return true;
	case FundamentalType::UnsignedInt:
	case FundamentalType::Char32T:
		kind = IntegralKind{ 32, false };
		return true;
	case FundamentalType::LongInt:
	case FundamentalType::LongLongInt:
		kind = IntegralKind{ 64, true };
		return true;
	case FundamentalType::UnsignedLongInt:
	case FundamentalType::UnsignedLongLongInt:
		kind = IntegralKind{ 64, false };
		return true;
	case FundamentalType::Int128:
		kind = IntegralKind{ 128, true };
		return true;
	case FundamentalType::UnsignedInt128:
		kind = IntegralKind{ 128, false };
		return true;
	default:
		return false;
	}
}

// Integer conversion rank (4.13) of a promoted integral type.
unsigned Rank(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::Int:
	case FundamentalType::UnsignedInt:
		return 1;
	case FundamentalType::LongInt:
	case FundamentalType::UnsignedLongInt:
		return 2;
	case FundamentalType::Int128:
	case FundamentalType::UnsignedInt128:
		return 4;
	default:
		return 3;
	}
}

FundamentalType UnsignedOf(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::Int:
		return FundamentalType::UnsignedInt;
	case FundamentalType::LongInt:
		return FundamentalType::UnsignedLongInt;
	case FundamentalType::LongLongInt:
		return FundamentalType::UnsignedLongLongInt;
	case FundamentalType::Int128:
		return FundamentalType::UnsignedInt128;
	default:
		return type;
	}
}

} // namespace

bool IntegralKindOf(const Type* type, IntegralKind& kind)
{
	type = type->unqualified;
	if (type->kind == TypeKind::Enum)
	{
		const EnumEntity* entity = static_cast<const EnumEntity*>(type->entity);
		if (entity->underlying == nullptr)
			return false;
		type = entity->underlying;
	}
	if (type->kind != TypeKind::Fundamental || !FundamentalKind(type->fundamental, kind))
		return false;
	kind.is_signed = kind.is_signed && !type->plain_char_unsigned;
	return true;
}

bool IsIntegralType(const Type* type)
{
	IntegralKind kind;
	return type->unqualified->kind == TypeKind::Fundamental && FundamentalKind(type->unqualified->fundamental, kind);
}

bool IsWideIntegral(const Type* type)
{
	IntegralKind kind;
	return IntegralKindOf(type, kind) && kind.bits > 64;
}

bool HeldAsWideConstant(const Constant& value, IntegralKind to)
{
	if (to.bits <= 64)
		return true;
	const bool negative = IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0;
	return to.is_signed ? negative || (value.bits >> 63) == 0 : !negative;
}

bool IsScopedEnumeration(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Enum && static_cast<const EnumEntity*>(type->entity)->scoped;
}

bool IsSignedConstant(const Constant& value)
{
	IntegralKind kind;
	return IntegralKindOf(value.type, kind) && kind.is_signed;
}

uint64_t ConvertIntegral(uint64_t bits, IntegralKind to)
{
	if (to.bits == 1)
		return bits != 0;
	if (to.bits >= 64)
		return bits;
	const uint64_t mask = (uint64_t(1) << to.bits) - 1;
	uint64_t value = bits & mask;
	if (to.is_signed && (value >> (to.bits - 1)) != 0)
		value |= ~mask;
	return value;
}

bool Representable(const Constant& value, IntegralKind to)
{
	const bool from_signed = IsSignedConstant(value);
	const bool negative = from_signed && static_cast<int64_t>(value.bits) < 0;
	if (to.bits == 1)
		return value.bits <= 1;
	if (!to.is_signed)
	{
		if (negative)
			return false;
		return to.bits >= 64 || value.bits <= ((uint64_t(1) << to.bits) - 1);
	}
	// A Constant holds at most 64 bits (a 128-bit type's, extended).
	if (to.bits > 64)
		return true;
	if (!negative && !from_signed && (value.bits >> 63) != 0)
		return false;
	if (to.bits == 64)
		return true;
	const int64_t low = -(int64_t(1) << (to.bits - 1));
	const int64_t high = (int64_t(1) << (to.bits - 1)) - 1;
	const int64_t signed_value = static_cast<int64_t>(value.bits);
	return signed_value >= low && signed_value <= high;
}

namespace {

int64_t Minimum(unsigned bits)
{
	return bits == 64 ? INT64_MIN : -(int64_t(1) << (bits - 1));
}

int64_t Maximum(unsigned bits)
{
	return bits == 64 ? INT64_MAX : (int64_t(1) << (bits - 1)) - 1;
}

} // namespace

bool IntegralOperation(TokenKind op, IntegralKind kind, uint64_t a, uint64_t b, uint64_t& out)
{
	a = ConvertIntegral(a, kind);
	b = ConvertIntegral(b, kind);
	int64_t result = 0;
	bool overflow = false;
	switch (op)
	{
	case TokenKind::OP_PLUS:
		overflow = __builtin_add_overflow(static_cast<int64_t>(a), static_cast<int64_t>(b), &result);
		out = a + b;
		break;
	case TokenKind::OP_MINUS:
		overflow = __builtin_sub_overflow(static_cast<int64_t>(a), static_cast<int64_t>(b), &result);
		out = a - b;
		break;
	case TokenKind::OP_STAR:
		overflow = __builtin_mul_overflow(static_cast<int64_t>(a), static_cast<int64_t>(b), &result);
		out = a * b;
		break;
	case TokenKind::OP_DIV:
	case TokenKind::OP_MOD:
		out = 0;
		if (b == 0)
			return false;
		if (kind.is_signed)
		{
			const int64_t sa = static_cast<int64_t>(a), sb = static_cast<int64_t>(b);
			if (sa == Minimum(kind.bits) && sb == -1)
				return false;
			out = static_cast<uint64_t>(op == TokenKind::OP_DIV ? sa / sb : sa % sb);
		}
		else
			out = op == TokenKind::OP_DIV ? a / b : a % b;
		break;
	case TokenKind::OP_AMP:
		out = a & b;
		break;
	case TokenKind::OP_BOR:
		out = a | b;
		break;
	case TokenKind::OP_XOR:
		out = a ^ b;
		break;
	default:
		out = 0;
		return false;
	}
	const bool additive = op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS || op == TokenKind::OP_STAR;
	if (kind.is_signed && additive && (overflow || result < Minimum(kind.bits) || result > Maximum(kind.bits)))
	{
		out = ConvertIntegral(out, kind);
		return false;
	}
	out = ConvertIntegral(out, kind);
	return true;
}

bool IntegralShift(TokenKind op, IntegralKind kind, uint64_t a, IntegralKind count_kind, uint64_t count, uint64_t& out)
{
	const uint64_t bits = ConvertIntegral(a, kind);
	count = ConvertIntegral(count, count_kind);
	out = 0;
	if ((count_kind.is_signed && static_cast<int64_t>(count) < 0) || count >= kind.bits)
		return false;
	if (op == TokenKind::OP_RSHIFT)
	{
		out = kind.is_signed ? static_cast<uint64_t>(static_cast<int64_t>(bits) >> count) : bits >> count;
		return true;
	}
	out = ConvertIntegral(bits << count, kind);
	if (kind.is_signed)
	{
		// E1 non-negative and E1 * 2^E2 representable in the corresponding
		// unsigned type; the result is that value converted to E1's type.
		const uint64_t unsigned_maximum = kind.bits == 64 ? UINT64_MAX : (uint64_t(1) << kind.bits) - 1;
		if (static_cast<int64_t>(bits) < 0 || (count != 0 && bits > (unsigned_maximum >> count)))
			return false;
	}
	return true;
}

bool IntegralCompare(TokenKind op, bool is_signed, uint64_t a, uint64_t b)
{
	const bool less = is_signed ? static_cast<int64_t>(a) < static_cast<int64_t>(b) : a < b;
	switch (op)
	{
	case TokenKind::OP_EQ:
		return a == b;
	case TokenKind::OP_NE:
		return a != b;
	case TokenKind::OP_LT:
		return less;
	case TokenKind::OP_GT:
		return !less && a != b;
	case TokenKind::OP_LE:
		return less || a == b;
	default:
		return !less;
	}
}

namespace {

typedef unsigned __int128 Wide;
typedef __int128 SignedWide;

} // namespace

bool WideOperation(TokenKind op, bool is_signed, Wide a, Wide b, Wide& out)
{
	SignedWide result = 0;
	bool overflow = false;
	switch (op)
	{
	case TokenKind::OP_PLUS:
		overflow = __builtin_add_overflow(static_cast<SignedWide>(a), static_cast<SignedWide>(b), &result);
		out = a + b;
		break;
	case TokenKind::OP_MINUS:
		overflow = __builtin_sub_overflow(static_cast<SignedWide>(a), static_cast<SignedWide>(b), &result);
		out = a - b;
		break;
	case TokenKind::OP_STAR:
		overflow = __builtin_mul_overflow(static_cast<SignedWide>(a), static_cast<SignedWide>(b), &result);
		out = a * b;
		break;
	case TokenKind::OP_DIV:
	case TokenKind::OP_MOD:
		out = 0;
		if (b == 0)
			return false;
		if (is_signed)
		{
			const SignedWide sa = static_cast<SignedWide>(a), sb = static_cast<SignedWide>(b);
			if (sa == static_cast<SignedWide>(static_cast<Wide>(1) << 127) && sb == -1)
				return false;
			out = static_cast<Wide>(op == TokenKind::OP_DIV ? sa / sb : sa % sb);
		}
		else
			out = op == TokenKind::OP_DIV ? a / b : a % b;
		return true;
	case TokenKind::OP_AMP:
		out = a & b;
		return true;
	case TokenKind::OP_BOR:
		out = a | b;
		return true;
	case TokenKind::OP_XOR:
		out = a ^ b;
		return true;
	default:
		out = 0;
		return false;
	}
	return !(is_signed && overflow);
}

bool WideShift(TokenKind op, bool is_signed, Wide a, IntegralKind count_kind, uint64_t count, Wide& out)
{
	count = ConvertIntegral(count, count_kind);
	out = 0;
	if ((count_kind.is_signed && static_cast<int64_t>(count) < 0) || count >= 128)
		return false;
	if (op == TokenKind::OP_RSHIFT)
	{
		out = is_signed ? static_cast<Wide>(static_cast<SignedWide>(a) >> count) : a >> count;
		return true;
	}
	out = a << count;
	// As IntegralShift: E1 non-negative and E1 * 2^E2 representable in the
	// corresponding unsigned type.
	return !is_signed || (static_cast<SignedWide>(a) >= 0 && (count == 0 || a <= (~static_cast<Wide>(0) >> count)));
}

bool WideCompare(TokenKind op, bool is_signed, Wide a, Wide b)
{
	const bool less = is_signed ? static_cast<SignedWide>(a) < static_cast<SignedWide>(b) : a < b;
	switch (op)
	{
	case TokenKind::OP_EQ:
		return a == b;
	case TokenKind::OP_NE:
		return a != b;
	case TokenKind::OP_LT:
		return less;
	case TokenKind::OP_GT:
		return !less && a != b;
	case TokenKind::OP_LE:
		return less || a == b;
	default:
		return !less;
	}
}

uint64_t WideHigh(const Type* type, uint64_t bits)
{
	IntegralKind kind;
	if (type == nullptr || !IntegralKindOf(type, kind) || kind.bits <= 64)
		return 0;
	return kind.is_signed && static_cast<int64_t>(bits) < 0 ? ~uint64_t(0) : 0;
}

const Type* PromotedType(TypeTable& types, const Type* type)
{
	type = type->unqualified;
	if (type->kind == TypeKind::Enum)
	{
		const EnumEntity* entity = static_cast<const EnumEntity*>(type->entity);
		if (entity->scoped || entity->underlying == nullptr)
			return type;
		type = entity->underlying;
	}
	if (type->kind != TypeKind::Fundamental)
		return type;
	switch (type->fundamental)
	{
	case FundamentalType::Bool:
	case FundamentalType::Char:
	case FundamentalType::SignedChar:
	case FundamentalType::UnsignedChar:
	case FundamentalType::ShortInt:
	case FundamentalType::UnsignedShortInt:
	case FundamentalType::Char16T:
	case FundamentalType::WcharT:
		return types.fundamental(FundamentalType::Int);
	case FundamentalType::Char32T:
		return types.fundamental(FundamentalType::UnsignedInt);
	default:
		return type;
	}
}

const Type* CommonType(TypeTable& types, const Type* a, const Type* b)
{
	if (a == b)
		return a;
	IntegralKind ka = IntegralKind{ 32, true };
	IntegralKind kb = ka;
	FundamentalKind(a->fundamental, ka);
	FundamentalKind(b->fundamental, kb);
	if (ka.is_signed == kb.is_signed)
		return Rank(a->fundamental) >= Rank(b->fundamental) ? a : b;
	const Type* unsigned_type = ka.is_signed ? b : a;
	const Type* signed_type = ka.is_signed ? a : b;
	const IntegralKind unsigned_kind = ka.is_signed ? kb : ka;
	const IntegralKind signed_kind = ka.is_signed ? ka : kb;
	if (Rank(unsigned_type->fundamental) >= Rank(signed_type->fundamental))
		return unsigned_type;
	if (signed_kind.bits > unsigned_kind.bits)
		return signed_type;
	return types.fundamental(UnsignedOf(signed_type->fundamental));
}

const Type* SmallestTypeFor(TypeTable& types, uint64_t bits, bool is_signed)
{
	static const FundamentalType kCandidates[] = { FundamentalType::Int, FundamentalType::UnsignedInt,
		FundamentalType::LongInt, FundamentalType::UnsignedLongInt, FundamentalType::LongLongInt,
		FundamentalType::UnsignedLongLongInt };
	const Constant value = Constant{ bits,
		is_signed ? types.fundamental(FundamentalType::LongLongInt)
				  : types.fundamental(FundamentalType::UnsignedLongLongInt) };
	for (FundamentalType candidate : kCandidates)
	{
		IntegralKind kind;
		FundamentalKind(candidate, kind);
		if (Representable(value, kind))
			return types.fundamental(candidate);
	}
	return types.fundamental(FundamentalType::UnsignedLongLongInt);
}

} // namespace sema
