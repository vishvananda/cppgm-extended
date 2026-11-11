#include "sema/constant_floating.h"

#include "sema/floating/binary128.h"
#include "sema/integral_arithmetic.h"

namespace sema {

namespace {

binary128::Bits bits(const ConstValue& value)
{
	return binary128::Bits{ value.bits, value.high };
}

} // namespace

ConstValue Binary128FromHost(const Type* type, long double value)
{
	binary128::Bits bits;
	if (!binary128::from_host(value, bits))
		return ConstValue();
	return Binary128Value(type, bits.low, bits.high);
}

bool Binary128Zero(const ConstValue& value)
{
	return binary128::zero(bits(value));
}

bool ConvertBinary128(ConstValue& value, const Type* target)
{
	if (Binary128Type(target))
	{
		binary128::Bits result;
		if (value.kind == ValueKind::Binary128)
			result = bits(value);
		else if (value.kind == ValueKind::Floating)
		{
			if (!binary128::from_host(value.floating, result))
				return false;
		}
		else if (value.kind == ValueKind::Integer)
		{
			IntegralKind kind;
			if (!IntegralKindOf(value.type, kind))
				return false;
			WideBits integer = kind.bits > 64 ? WideOf(value)
				: kind.is_signed ? static_cast<WideBits>(static_cast<__int128>(static_cast<int64_t>(value.bits)))
								 : value.bits;
			const bool sign = kind.is_signed && (integer >> 127) != 0;
			if (sign)
				integer = ~integer + 1;
			if (!binary128::from_integer(sign,
					binary128::Bits{ static_cast<uint64_t>(integer), static_cast<uint64_t>(integer >> 64) }, result))
				return false;
		}
		else
			return false;
		value = Binary128Value(target, result.low, result.high);
		return true;
	}
	if (value.kind != ValueKind::Binary128)
		return false;
	IntegralKind integral;
	if (IntegralKindOf(target, integral))
	{
		binary128::Bits result;
		if (!binary128::to_integer(bits(value), integral.bits, integral.is_signed, result))
			return false;
		value = integral.bits > 64 ? WideValue(target, (WideBits(result.high) << 64) | result.low)
								   : IntegerValue(target, ConvertIntegral(result.low, integral));
		return true;
	}
	if (target->kind != TypeKind::Fundamental)
		return false;
	const auto representation = FloatingRepresentation(target->fundamental, target->target);
	unsigned precision = 0;
	int maximum = 0;
	switch (representation)
	{
	case FundamentalType::Float16:
		precision = 11;
		maximum = 15;
		break;
	case FundamentalType::Float:
		precision = 24;
		maximum = 127;
		break;
	case FundamentalType::Double:
		precision = 53;
		maximum = 1023;
		break;
	case FundamentalType::LongDouble:
		precision = 64;
		maximum = 16383;
		break;
	case FundamentalType::BFloat16:
		precision = 8;
		maximum = 127;
		break;
	default:
		return false;
	}
	long double result = 0;
	if (!binary128::to_host(bits(value), precision, maximum, result))
		return false;
	value = FloatingValue(target, result);
	return true;
}

bool Binary128Fits(const ConstValue& value, const Type* target)
{
	if (value.kind != ValueKind::Binary128)
		return false;
	const auto representation = FloatingRepresentation(target->fundamental, target->target);
	if (representation == FundamentalType::Float16 || representation == FundamentalType::BFloat16)
	{
		long double rounded = 0;
		return binary128::to_host(bits(value), representation == FundamentalType::Float16 ? 11 : 8,
			representation == FundamentalType::Float16 ? 15 : 127, rounded);
	}
	ConstValue converted = value;
	return ConvertBinary128(converted, target);
}

bool Binary128Operation(
	TokenKind operation, const Type* type, const ConstValue& left, const ConstValue& right, ConstValue& result)
{
	binary128::Operation op;
	switch (operation)
	{
	case TokenKind::OP_PLUS:
		op = binary128::Operation::Add;
		break;
	case TokenKind::OP_MINUS:
		op = binary128::Operation::Subtract;
		break;
	case TokenKind::OP_STAR:
		op = binary128::Operation::Multiply;
		break;
	case TokenKind::OP_DIV:
		op = binary128::Operation::Divide;
		break;
	default:
		return false;
	}
	binary128::Bits value;
	if (!binary128::calculate(op, bits(left), bits(right), value))
		return false;
	result = Binary128Value(type, value.low, value.high);
	return true;
}

bool Binary128Comparison(TokenKind operation, const ConstValue& left, const ConstValue& right)
{
	const int order = binary128::compare(bits(left), bits(right));
	switch (operation)
	{
	case TokenKind::OP_EQ:
		return order == 0;
	case TokenKind::OP_NE:
		return order != 0;
	case TokenKind::OP_LT:
		return order == -1;
	case TokenKind::OP_GT:
		return order == 1;
	case TokenKind::OP_LE:
		return order == -1 || order == 0;
	default:
		return order == 0 || order == 1;
	}
}

} // namespace sema
