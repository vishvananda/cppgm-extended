#pragma once

#include <cstdint>

// A value in a preprocessing controlling expression. Every signed integer
// type acts as intmax_t and every unsigned one as uintmax_t ([cpp.cond]
// 16.1p4), both 64 bits on x86-64, so a value is its two's complement bit
// pattern and whether it is unsigned.
struct PPValue
{
	uint64_t bits;
	bool is_unsigned;
};

// The operators of the controlling-expression grammar, apart from `?:` and
// grouping.
enum class PPOperator : uint8_t
{
	// unary-expression ([expr.unary.op] 5.3.1)
	Plus,
	Minus,
	LogicalNot,
	Complement,
	// multiplicative-expression through logical-or-expression
	Multiply,
	Divide,
	Modulo,
	Add,
	Subtract,
	ShiftLeft,
	ShiftRight,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Equal,
	NotEqual,
	BitAnd,
	BitXor,
	BitOr,
	LogicalAnd,
	LogicalOr
};

inline bool IsPPUnaryOperator(PPOperator op)
{
	return op <= PPOperator::Complement;
}

// How tightly the operator binds in the clause 5 grammar: the unary operators
// highest, then multiplicative (10) down to logical-or (1).
unsigned PPOperatorPrecedence(PPOperator op);

// Apply an operator after the conversions of clause 5: the result of a
// relational, equality or logical operator is a bool and so acts as intmax_t;
// a shift has the type of its left operand; every other binary operator
// takes the usual arithmetic conversions (5p10), which give uintmax_t if
// either operand is unsigned.
//
// The result's type is always set. The functions return false when the
// operation has no value: a signed result out of range ([expr] 5p4, so not a
// constant expression, [expr.const] 5.19p2), a zero divisor or an
// unrepresentable quotient ([expr.mul] 5.6p4), or a shift count that is
// negative or at least 64 ([expr.shift] 5.8p1; course-defined as an error).
// `result.bits` is then unspecified. Otherwise signed values are two's
// complement as the course defines: a left shift discards the bits shifted
// out and a right shift of a negative value copies the sign bit.
bool ApplyPPUnary(PPOperator op, PPValue operand, PPValue& result);
bool ApplyPPBinary(PPOperator op, PPValue left, PPValue right, PPValue& result);
