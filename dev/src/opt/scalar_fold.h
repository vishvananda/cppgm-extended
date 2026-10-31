#pragma once

#include "lowir/program.h"

// Compile-time evaluation of LowIR scalar operations, exactly as the
// native back end computes them (opt/optimizer.h): integers wrap to their
// type's width (i* signed, u* and i1 unsigned, ptr as i64); division,
// remainder and shifts fold only where they cannot trap or depend on an
// out-of-range count; f32 and f64 arithmetic round in their IEEE single
// and double formats, and f80 is x87 extended, matching native evaluation.
namespace opt {

// The constant an operand denotes when it is a literal.
inline bool literal_constant(const lowir::FunctionBody& body, lowir::Value value, lowir::Constant& out)
{
	if (!value.is_literal())
		return false;
	out.kind = value.kind;
	out.float_kind = value.float_kind;
	out.bits = body.literals[value.index];
	return true;
}

// An integer literal's bits reduced to `type`'s width and extended back
// to 128 bits by the type's signedness (the canonical form).
lowir::Literal normalize_integer(lowir::Type type, lowir::Literal bits);

// The integer value of a constant read as `type` (an integer constant
// reduced to its width); false for a floating constant.
bool integer_of(lowir::Type type, const lowir::Constant& c, lowir::Literal& out);

lowir::Constant integer_constant(lowir::Type type, lowir::Literal bits);
// i64 0 or 1, the result of cmp.
lowir::Constant truth_constant(bool value);

bool fold_unary(lowir::UnaryOp op, lowir::Type type, const lowir::Constant& a, lowir::Constant& out);
bool fold_binary(
	lowir::BinaryOp op, lowir::Type type, const lowir::Constant& a, const lowir::Constant& b, lowir::Constant& out);
bool fold_compare(lowir::ComparePredicate pred, lowir::Type type, const lowir::Constant& a, const lowir::Constant& b,
	lowir::Constant& out);
bool fold_convert(
	lowir::ConvertOp op, lowir::Type to, lowir::Type from, const lowir::Constant& a, lowir::Constant& out);
// A copy retypes the bits (same width) or keeps a literal's value.
bool fold_copy(lowir::Type type, const lowir::Constant& a, lowir::Constant& out);

// Whether two constants are the same value of `type`.
bool same_constant(lowir::Type type, const lowir::Constant& a, const lowir::Constant& b);
// Whether a constant is the integer `value` when read as `type`.
bool is_integer(lowir::Type type, const lowir::Constant& c, int64_t value);
// The bits is_integer compares (c as an integer of `type`, normalized);
// false when it would answer false for every value.
bool integer_bits(lowir::Type type, const lowir::Constant& c, lowir::Literal& bits);
// Whether the integer constant is all ones at `type`'s width.
bool is_all_ones(lowir::Type type, const lowir::Constant& c);

} // namespace opt
