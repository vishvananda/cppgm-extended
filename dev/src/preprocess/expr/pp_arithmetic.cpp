#include "preprocess/expr/pp_arithmetic.h"

// Signed values are handled through their bit patterns, so no host signed
// arithmetic can overflow and no implementation-defined conversion is needed.

namespace {

const uint64_t kSignBit = 1ull << 63;

bool IsNegative(PPValue value)
{
	return !value.is_unsigned && (value.bits & kSignBit) != 0;
}

// The absolute value of a signed value; 2^63 for the minimum.
uint64_t Magnitude(uint64_t bits)
{
	return (bits & kSignBit) != 0 ? 0 - bits : bits;
}

PPValue Boolean(bool value)
{
	return PPValue{ value ? 1u : 0u, false };
}

// Whether left < right after the usual arithmetic conversions. Flipping the
// sign bits orders signed values as unsigned ones.
bool IsLess(uint64_t left, uint64_t right, bool is_unsigned)
{
	return is_unsigned ? left < right : (left ^ kSignBit) < (right ^ kSignBit);
}

bool SignedProductFits(uint64_t left, uint64_t right)
{
	const uint64_t a = Magnitude(left);
	const uint64_t b = Magnitude(right);
	// A negative product may reach -2^63.
	const uint64_t limit = ((left ^ right) & kSignBit) != 0 ? kSignBit : kSignBit - 1;
	return a == 0 || b <= limit / a;
}

// `/` and `%` ([expr.mul] 5.6p4): the quotient truncates toward zero and the
// remainder takes the sign of the dividend.
bool Divide(PPOperator op, uint64_t left, uint64_t right, bool is_unsigned, uint64_t& result)
{
	result = 0;
	if (right == 0)
		return false;
	if (is_unsigned)
	{
		result = op == PPOperator::Divide ? left / right : left % right;
		return true;
	}
	// -2^63 / -1 is 2^63, which intmax_t cannot represent; the remainder is
	// then undefined as well.
	if (left == kSignBit && right == ~0ull)
		return false;
	const uint64_t a = Magnitude(left);
	const uint64_t b = Magnitude(right);
	if (op == PPOperator::Divide)
	{
		const uint64_t quotient = a / b;
		result = ((left ^ right) & kSignBit) != 0 ? 0 - quotient : quotient;
	}
	else
	{
		const uint64_t remainder = a % b;
		result = (left & kSignBit) != 0 ? 0 - remainder : remainder;
	}
	return true;
}

bool Shift(PPOperator op, PPValue left, PPValue right, PPValue& result)
{
	result = PPValue{ 0, left.is_unsigned };
	if (IsNegative(right) || right.bits >= 64)
		return false;
	const unsigned count = static_cast<unsigned>(right.bits);
	if (op == PPOperator::ShiftLeft)
		result.bits = left.bits << count;
	else if (IsNegative(left))
		result.bits = ~(~left.bits >> count);
	else
		result.bits = left.bits >> count;
	return true;
}

} // namespace

unsigned PPOperatorPrecedence(PPOperator op)
{
	switch (op)
	{
	case PPOperator::Plus:
	case PPOperator::Minus:
	case PPOperator::LogicalNot:
	case PPOperator::Complement:
		return 11;
	case PPOperator::Multiply:
	case PPOperator::Divide:
	case PPOperator::Modulo:
		return 10;
	case PPOperator::Add:
	case PPOperator::Subtract:
		return 9;
	case PPOperator::ShiftLeft:
	case PPOperator::ShiftRight:
		return 8;
	case PPOperator::Less:
	case PPOperator::Greater:
	case PPOperator::LessEqual:
	case PPOperator::GreaterEqual:
		return 7;
	case PPOperator::Equal:
	case PPOperator::NotEqual:
		return 6;
	case PPOperator::BitAnd:
		return 5;
	case PPOperator::BitXor:
		return 4;
	case PPOperator::BitOr:
		return 3;
	case PPOperator::LogicalAnd:
		return 2;
	case PPOperator::LogicalOr:
		return 1;
	}
	return 0;
}

bool ApplyPPUnary(PPOperator op, PPValue operand, PPValue& result)
{
	result = operand;
	switch (op)
	{
	case PPOperator::Minus:
		result.bits = 0 - operand.bits;
		return operand.is_unsigned || operand.bits != kSignBit;
	case PPOperator::LogicalNot:
		result = Boolean(operand.bits == 0);
		return true;
	case PPOperator::Complement:
		result.bits = ~operand.bits;
		return true;
	case PPOperator::Plus:
		return true;
	default:
		// Not a unary operator.
		return false;
	}
}

bool ApplyPPBinary(PPOperator op, PPValue left, PPValue right, PPValue& result)
{
	const uint64_t l = left.bits;
	const uint64_t r = right.bits;
	const bool is_unsigned = left.is_unsigned || right.is_unsigned;
	result = PPValue{ 0, is_unsigned };
	switch (op)
	{
	case PPOperator::Multiply:
		result.bits = l * r;
		return is_unsigned || SignedProductFits(l, r);
	case PPOperator::Divide:
	case PPOperator::Modulo:
		return Divide(op, l, r, is_unsigned, result.bits);
	case PPOperator::Add:
		result.bits = l + r;
		// Signed overflow: both operands have the sign the sum lacks.
		return is_unsigned || ((l ^ result.bits) & (r ^ result.bits) & kSignBit) == 0;
	case PPOperator::Subtract:
		result.bits = l - r;
		// Signed overflow: the operands' signs differ and the difference
		// lacks the minuend's.
		return is_unsigned || ((l ^ r) & (l ^ result.bits) & kSignBit) == 0;
	case PPOperator::ShiftLeft:
	case PPOperator::ShiftRight:
		return Shift(op, left, right, result);
	case PPOperator::Less:
		result = Boolean(IsLess(l, r, is_unsigned));
		return true;
	case PPOperator::Greater:
		result = Boolean(IsLess(r, l, is_unsigned));
		return true;
	case PPOperator::LessEqual:
		result = Boolean(!IsLess(r, l, is_unsigned));
		return true;
	case PPOperator::GreaterEqual:
		result = Boolean(!IsLess(l, r, is_unsigned));
		return true;
	case PPOperator::Equal:
		result = Boolean(l == r);
		return true;
	case PPOperator::NotEqual:
		result = Boolean(l != r);
		return true;
	case PPOperator::BitAnd:
		result.bits = l & r;
		return true;
	case PPOperator::BitXor:
		result.bits = l ^ r;
		return true;
	case PPOperator::BitOr:
		result.bits = l | r;
		return true;
	case PPOperator::LogicalAnd:
		result = Boolean(l != 0 && r != 0);
		return true;
	case PPOperator::LogicalOr:
		result = Boolean(l != 0 || r != 0);
		return true;
	default:
		// Not a binary operator.
		return false;
	}
}
