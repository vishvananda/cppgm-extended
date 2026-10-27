#include "sema/constant_interpreter.h"

#include <cmath>
#include <cstring>

#include "sema/analyzer.h"
#include "sema/integral_arithmetic.h"

// GNU complex values in constant evaluation (GCC folds them; libstdc++'s
// std::complex constructors and accessors are constexpr): an Array value of
// the complex type holding its two parts, real then imaginary, built from a
// list of the parts or __builtin_complex, converted part by part (a real
// value's imaginary part zero), read and written through __real__ and
// __imag__, and combined as the lowering combines them
// (lower/complex_lowering.cpp). A quotient of two floating complex values
// is libgcc's __div?c3 at run time and no constant here. GNU vector values
// likewise: an Array value of the vector type holding its lanes (see
// LaneVector), built from a lane list, read by lane subscripts, and
// reinterpreted by a cast to another vector type of its size.
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool LaneIs(const Type* lane, FundamentalType which)
{
	return lane->kind == TypeKind::Fundamental && lane->fundamental == which;
}

bool IsComplexValue(const ConstValue& value)
{
	return value.kind == ValueKind::Array && value.count == 2 && value.type != nullptr && IsComplex(value.type);
}

bool SignedPart(const Type* part)
{
	IntegralKind kind;
	return IntegralKindOf(part, kind) && kind.is_signed;
}

} // namespace

bool ConstantInterpreter::complex_value(
	const Type* type, const ConstValue& real, const ConstValue& imaginary, ConstValue& out)
{
	// The parts in the evaluation's storage (a constexpr call's result is
	// made again in its caller's, see invoke).
	ConstValue* parts = arena_.allocate_array<ConstValue>(2);
	parts[0] = real;
	parts[1] = imaginary;
	parts[0].flags = 0;
	parts[1].flags = 0;
	out = ConstValue();
	out.kind = ValueKind::Array;
	out.type = type->unqualified;
	out.count = 2;
	out.elements = parts;
	return true;
}

bool ConstantInterpreter::part_list(
	const syntax::Slice<syntax::Expr*>& parts, const Type* type, ConstValue& out, uint32_t location)
{
	// `{re, im}` or __builtin_complex(re, im): each part converted.
	const Type* part = type->unqualified->element;
	ConstValue real, imaginary;
	if (parts.size != 2 || !operand(*parts[0], real) || !convert_scalar(real, part, location) ||
		!operand(*parts[1], imaginary) || !convert_scalar(imaginary, part, location))
		return parts.size == 2 ? false : fail(location, "a complex value of no two parts");
	return complex_value(type, real, imaginary, out);
}

bool ConstantInterpreter::vector_value(
	const Type* type, const ConstValue* lanes, uint32_t count, const ConstValue& fill, ConstValue& out)
{
	// A vector value of LaneVector type holding its lanes in the
	// evaluation's storage: `lanes`' first `count`, the others `fill`.
	const uint32_t bound = static_cast<uint32_t>(type->unqualified->bound);
	ConstValue* held = arena_.allocate_array<ConstValue>(bound);
	for (uint32_t i = 0; i < bound; ++i)
		held[i] = i < count ? lanes[i] : fill;
	out = ConstValue();
	out.kind = ValueKind::Array;
	out.type = type->unqualified;
	out.count = bound;
	out.elements = held;
	return true;
}

bool ConstantInterpreter::lane_list(
	const syntax::Slice<syntax::Expr*>& lanes, const Type* type, ConstValue& out, uint32_t location)
{
	// A GNU vector's list of lanes (V{...}, (V){...}, `= {...}`): each lane
	// a constant converted to the lane type, the lanes after them zero (GCC
	// and Clang fold such lists). A bool vector's value stays opaque: never
	// read by lane, converted (see convert_scalar) or placed as constant
	// data.
	const Type* lane = type->unqualified->element->unqualified;
	if (lanes.size > type->unqualified->bound)
		return fail(location, "more lanes than the vector has");
	SmallList<ConstValue, 16> values;
	for (const syntax::Expr* element : lanes)
	{
		ConstValue value;
		if (element->kind == syntax::ExprKind::BracedInit)
			return fail(location, "a braced vector lane");
		if (!operand(*element, value) || !convert_scalar(value, lane, location))
			return false;
		value.flags = 0;
		values.push_back(value);
	}
	if (!LaneVector(type))
	{
		out = ConstValue();
		out.type = type->unqualified;
		zero_into(out);
		out.flags = 0;
		return true;
	}
	ConstValue zero = ConstValue();
	zero.type = lane;
	zero_into(zero);
	return vector_value(type, values.empty() ? nullptr : &values[0], static_cast<uint32_t>(values.size()), zero, out);
}

namespace {

// Whether a long double NaN is signaling: its x87 significand's bit 62
// (the quiet bit) clear.
bool SignalingNan(long double value)
{
	uint64_t significand = 0;
	std::memcpy(&significand, &value, sizeof significand);
	return std::isnan(value) && (significand >> 62 & 1) == 0;
}

// One scalar's bytes (little-endian) at `bytes`: an integer's (a 128-bit
// one's high half too), a float's or double's encoding; false for another
// value and for a signaling NaN, whose quiet bit converting the long double
// to the part type would set: the cast is then the run time's. (A quiet
// NaN converts as the run time's would: its payload's top bits kept.)
bool ScalarBytes(const ConstValue& value, const Type* type, uint64_t size, uint8_t* bytes)
{
	uint64_t low = 0, high = 0;
	if (value.kind == ValueKind::Floating && SignalingNan(value.floating))
		return false;
	if (value.kind == ValueKind::Integer)
	{
		low = value.bits;
		high = value.high;
	}
	else if (value.kind == ValueKind::Floating && size == 4 && LaneIs(type, FundamentalType::Float))
	{
		const float f = static_cast<float>(value.floating);
		uint32_t word = 0;
		std::memcpy(&word, &f, 4);
		low = word;
	}
	else if (value.kind == ValueKind::Floating && size == 8 && LaneIs(type, FundamentalType::Double))
	{
		const double d = static_cast<double>(value.floating);
		std::memcpy(&low, &d, 8);
	}
	else
		return false;
	for (uint64_t b = 0; b < size; ++b)
		bytes[b] = static_cast<uint8_t>(b < 8 ? low >> (8 * b) : high >> (8 * (b - 8)));
	return true;
}

// The scalar of `type` whose encoding is the `size` bytes at `bytes`: an
// integer, a float or a double (a quiet NaN's payload a long double keeps,
// a signaling NaN's it does not: none); false for another type.
bool BytesScalar(const uint8_t* bytes, const Type* type, uint64_t size, ConstValue& out)
{
	uint64_t low = 0, high = 0;
	for (uint64_t b = 0; b < size; ++b)
	{
		if (b < 8)
			low |= static_cast<uint64_t>(bytes[b]) << (8 * b);
		else
			high |= static_cast<uint64_t>(bytes[b]) << (8 * (b - 8));
	}
	IntegralKind kind;
	if (IntegralKindOf(type, kind))
	{
		if (size > 8)
			out = WideValue(type, (static_cast<WideBits>(high) << 64) | low);
		else
			out = IntegerValue(type, ConvertIntegral(low, kind));
		return true;
	}
	if (size == 4 && LaneIs(type, FundamentalType::Float))
	{
		float f = 0;
		const uint32_t word = static_cast<uint32_t>(low);
		std::memcpy(&f, &word, 4);
		out = FloatingValue(type, f);
		return !std::isnan(f) || (word & 0x00400000u) != 0;
	}
	if (size == 8 && LaneIs(type, FundamentalType::Double))
	{
		double d = 0;
		std::memcpy(&d, &low, 8);
		out = FloatingValue(type, d);
		return !std::isnan(d) || (low & 0x0008000000000000u) != 0;
	}
	return false;
}

} // namespace

bool ConstantInterpreter::bit_cast(const syntax::BuiltinTraitExpr& expr, ConstValue& out)
{
	// Reuse the target's scalar encodings, never a numeric conversion.
	// Pointer representations and class subobjects are not scalar constants.
	if (!operand(*expr.operand, out))
		return false;
	const Type* to = expr.facts->type->unqualified;
	const uint64_t size = size_of(to, expr.location);
	const Type* representation = to->kind == TypeKind::Enum ? static_cast<const EnumEntity*>(to->entity)->underlying
															: to;
	uint8_t bytes[16];
	if (size == 0 || size > sizeof bytes || size != size_of(out.type, expr.location) ||
		!ScalarBytes(out, out.type->unqualified, size, bytes) ||
		(LaneIs(representation, FundamentalType::Bool) && bytes[0] > 1) || !BytesScalar(bytes, to, size, out))
		return fail(expr.location, "a bit cast outside supported constant representations");
	return true;
}

bool ConstantInterpreter::vector_reinterpretation(ConstValue& value, const Type* to, uint32_t location)
{
	// GCC's vector casts (see Analyzer::vector_reinterpretation): the bytes
	// of a vector's lanes, or of an integer, read as another vector's lanes
	// or as an integer of their size (little-endian; integral, float and
	// double lanes). GCC folds every form, Clang the casts to a vector type.
	const Type* from = value.type->unqualified;
	to = to->unqualified;
	const uint64_t size = size_of(from, location);
	if (size != size_of(to, location) || size == 0 || size > 256)
		return fail(location, "a vector reinterpretation outside constant evaluation");
	uint8_t bytes[256];
	if (LaneVector(from) && value.kind == ValueKind::Array)
	{
		const Type* lane = from->element->unqualified;
		const uint64_t lane_size = size_of(lane, location);
		if (lane_size == 0 || value.count * lane_size != size)
			return fail(location, "a vector reinterpretation outside constant evaluation");
		for (uint32_t i = 0; i < value.count; ++i)
		{
			if (!ScalarBytes(value.elements[i], lane, lane_size, bytes + i * lane_size))
				return fail(location, "a vector reinterpretation outside constant evaluation");
		}
	}
	else if (from->kind == TypeKind::Vector || size > 16 || !ScalarBytes(value, from, size, bytes))
		return fail(location, "a vector reinterpretation outside constant evaluation");
	if (!LaneVector(to))
	{
		if (to->kind == TypeKind::Vector || size > 16 || !BytesScalar(bytes, to, size, value))
			return fail(location, "a vector reinterpretation outside constant evaluation");
		return true;
	}
	const Type* lane = to->element->unqualified;
	const uint64_t lane_size = size_of(lane, location);
	const uint32_t lanes = static_cast<uint32_t>(to->bound);
	if (lane_size == 0 || lanes * lane_size != size)
		return fail(location, "a vector reinterpretation outside constant evaluation");
	SmallList<ConstValue, 16> values;
	for (uint32_t i = 0; i < lanes; ++i)
	{
		ConstValue lane_value;
		if (!BytesScalar(bytes + i * lane_size, lane, lane_size, lane_value))
			return fail(location, "a vector reinterpretation making a lane outside constant evaluation");
		values.push_back(lane_value);
	}
	return vector_value(to, &values[0], lanes, values[0], value);
}

bool ConstantInterpreter::complex_convert(ConstValue& value, const Type* to, uint32_t location)
{
	// To a complex type: part by part, a real value's imaginary part zero.
	const Type* part = to->unqualified->element;
	ConstValue real = value, imaginary = ConstValue();
	if (IsComplexValue(value))
	{
		real = value.elements[0];
		imaginary = value.elements[1];
	}
	else
	{
		imaginary.type = part->unqualified;
		zero_into(imaginary);
		imaginary.flags = 0;
	}
	if (!convert_scalar(real, part, location) || !convert_scalar(imaginary, part, location))
		return false;
	return complex_value(to, real, imaginary, value);
}

bool ConstantInterpreter::complex_truth(const ConstValue& value, bool& out, uint32_t location)
{
	// C99 6.3.1.2: false only when both parts are zero.
	bool real = false, imaginary = false;
	if (!truth_of(value.elements[0], real, location) || !truth_of(value.elements[1], imaginary, location))
		return false;
	out = real || imaginary;
	return true;
}

bool ConstantInterpreter::part_operation(
	TokenKind op, const Type* part, ConstValue a, ConstValue b, ConstValue& out, uint32_t location)
{
	// One part's + - * / in the part type, as the native code computes it.
	if (!convert_scalar(a, part, location) || !convert_scalar(b, part, location))
		return false;
	if (IsFloating(part))
	{
		long double result = 0;
		if (!floating_operation(op, part, a.floating, b.floating, result, location))
			return false;
		out = FloatingValue(part->unqualified, result);
		return true;
	}
	IntegralKind kind;
	if (a.kind != ValueKind::Integer || b.kind != ValueKind::Integer || IsWideIntegral(part) ||
		!IntegralKindOf(part, kind))
		return fail(location, "a complex part outside constant evaluation");
	out = IntegerValue(part->unqualified, 0);
	return IntegralOperation(op, kind, a.bits, b.bits, out.bits) ||
		fail(location, "an arithmetic operation with an undefined result");
}

bool ConstantInterpreter::part_negation(const Type* part, ConstValue value, ConstValue& out, uint32_t location)
{
	if (!convert_scalar(value, part, location))
		return false;
	if (value.kind == ValueKind::Floating)
	{
		out = FloatingValue(part->unqualified, -value.floating);
		return true;
	}
	ConstValue zero = IntegerValue(part->unqualified, 0);
	return part_operation(TokenKind::OP_MINUS, part, zero, value, out, location);
}

bool ConstantInterpreter::complex_operation(
	TokenKind op, const Type* type, const ConstValue& left, const ConstValue& right, ConstValue& out, uint32_t location)
{
	// (a + bi) op (c + di) as lower/complex_lowering.cpp computes it: a
	// real operand's absent imaginary part takes no part (C99 G.5.1), a
	// product (ac - bd) + (ad + bc)i, an integral quotient by Smith's
	// formula.
	const Type* part = type->unqualified->element;
	const bool left_complex = IsComplexValue(left), right_complex = IsComplexValue(right);
	const ConstValue a = left_complex ? left.elements[0] : left;
	const ConstValue c = right_complex ? right.elements[0] : right;
	ConstValue b = left_complex ? left.elements[1] : ConstValue();
	ConstValue d = right_complex ? right.elements[1] : ConstValue();
	ConstValue real, imaginary;
	switch (op)
	{
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
		if (!part_operation(op, part, a, c, real, location))
			return false;
		imaginary = right_complex ? d : b;
		if (right_complex && left_complex)
		{
			if (!part_operation(op, part, b, d, imaginary, location))
				return false;
		}
		else if (right_complex && op == TokenKind::OP_MINUS && !part_negation(part, d, imaginary, location))
			return false;
		break;
	case TokenKind::OP_STAR:
		if (left_complex && right_complex)
		{
			ConstValue ac, bd, ad, bc;
			if (!part_operation(op, part, a, c, ac, location) || !part_operation(op, part, b, d, bd, location) ||
				!part_operation(op, part, a, d, ad, location) || !part_operation(op, part, b, c, bc, location) ||
				!part_operation(TokenKind::OP_MINUS, part, ac, bd, real, location) ||
				!part_operation(TokenKind::OP_PLUS, part, ad, bc, imaginary, location))
				return false;
			break;
		}
		if (!part_operation(op, part, a, c, real, location) ||
			!part_operation(op, part, left_complex ? b : a, left_complex ? c : d, imaginary, location))
			return false;
		break;
	default:
		if (!right_complex)
		{
			if (!part_operation(op, part, a, c, real, location) || !part_operation(op, part, b, c, imaginary, location))
				return false;
			break;
		}
		if (IsFloating(part))
			return fail(location, "a quotient of complex values outside constant evaluation");
		if (!left_complex)
			b = IntegerValue(part->unqualified, 0);
		{
			// If |c| < |d|, r = c/d: ((ar + b) + (br - a)i) / (cr + d); else
			// r = d/c: ((br + a) + (b - ar)i) / (dr + c).
			ConstValue cc = c, dd = d;
			if (!convert_scalar(cc, part, location) || !convert_scalar(dd, part, location))
				return false;
			const bool is_signed = SignedPart(part);
			const uint64_t magnitude_c = is_signed && static_cast<int64_t>(cc.bits) < 0 ? 0 - cc.bits : cc.bits;
			const uint64_t magnitude_d = is_signed && static_cast<int64_t>(dd.bits) < 0 ? 0 - dd.bits : dd.bits;
			const bool by_d = magnitude_c < magnitude_d;
			const ConstValue& small = by_d ? cc : dd;
			const ConstValue& large = by_d ? dd : cc;
			ConstValue ratio, scaled, divisor, t, u, real_sum, imaginary_sum;
			if (!part_operation(TokenKind::OP_DIV, part, small, large, ratio, location) ||
				!part_operation(TokenKind::OP_STAR, part, small, ratio, scaled, location) ||
				!part_operation(TokenKind::OP_PLUS, part, scaled, large, divisor, location) ||
				!part_operation(TokenKind::OP_STAR, part, by_d ? a : b, ratio, t, location) ||
				!part_operation(TokenKind::OP_PLUS, part, t, by_d ? b : a, real_sum, location) ||
				!part_operation(TokenKind::OP_STAR, part, by_d ? b : a, ratio, u, location) ||
				!(by_d ? part_operation(TokenKind::OP_MINUS, part, u, a, imaginary_sum, location)
					   : part_operation(TokenKind::OP_MINUS, part, b, u, imaginary_sum, location)) ||
				!part_operation(TokenKind::OP_DIV, part, real_sum, divisor, real, location) ||
				!part_operation(TokenKind::OP_DIV, part, imaginary_sum, divisor, imaginary, location))
				return false;
		}
		break;
	}
	return complex_value(type, real, imaginary, out);
}

bool ConstantInterpreter::complex_equal(
	bool equal, const ConstValue& left, const ConstValue& right, bool& out, uint32_t location)
{
	// C99 6.5.9p4: equal when both parts are.
	if (!IsComplexValue(left) || !IsComplexValue(right))
		return fail(location, "a complex comparison outside constant evaluation");
	bool same = true;
	for (uint32_t i = 0; i < 2; ++i)
	{
		const ConstValue& a = left.elements[i];
		const ConstValue& b = right.elements[i];
		if (a.kind == ValueKind::Floating && b.kind == ValueKind::Floating)
			same = same && a.floating == b.floating;
		else if (a.kind == ValueKind::Integer && b.kind == ValueKind::Integer)
			same = same && a.bits == b.bits && a.high == b.high;
		else
			return fail(location, "a complex comparison outside constant evaluation");
	}
	out = equal == same;
	return true;
}

bool ConstantInterpreter::complex_unary(
	TokenKind op, const Type* type, const ConstValue& value, ConstValue& out, uint32_t location)
{
	// -z negates both parts, GNU ~z (the conjugate) the imaginary one.
	const Type* part = type->unqualified->element;
	if (!IsComplexValue(value))
		return fail(location, "a complex operand outside constant evaluation");
	ConstValue real = value.elements[0], imaginary;
	if ((op == TokenKind::OP_MINUS && !part_negation(part, real, real, location)) ||
		!part_negation(part, value.elements[1], imaginary, location))
		return false;
	return complex_value(type, real, imaginary, out);
}

bool ConstantInterpreter::complex_step(
	const Type* type, const ConstValue& old, bool decrement, ConstValue& out, uint32_t location)
{
	// GNU ++z and --z: the real part stepped.
	const Type* part = type->unqualified->element;
	if (!IsComplexValue(old))
		return fail(location, "a complex operand outside constant evaluation");
	ConstValue one = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Int), 1);
	ConstValue real;
	return part_operation(
			   decrement ? TokenKind::OP_MINUS : TokenKind::OP_PLUS, part, old.elements[0], one, real, location) &&
		complex_value(type, real, old.elements[1], out);
}

bool ConstantInterpreter::complex_part(const syntax::BuiltinTraitExpr& trait, ConstValue& out)
{
	// __real__ or __imag__ of a complex prvalue: that part.
	ConstValue value;
	if (!rvalue(*trait.operand, value))
		return false;
	if (!IsComplexValue(value))
		return fail(trait.location, "a complex part outside constant evaluation");
	out = value.elements[trait.trait == GnuTrait::RealPart ? 0 : 1];
	return true;
}

bool ConstantInterpreter::complex_part_place(const syntax::BuiltinTraitExpr& trait, ConstPlace& out)
{
	// Of a complex glvalue: the part's subobject (the imaginary one after
	// the real one).
	const syntax::Expr* operand = WithoutParentheses(trait.operand);
	const Type* type = operand->facts->type;
	if (!IsComplex(type) || !glvalue(*operand, out))
		return IsComplex(type) ? false : fail(trait.location, "a part of a real glvalue in a constant expression");
	const Type* part = type->unqualified->element;
	if (trait.trait == GnuTrait::ImagPart)
		out.offset += static_cast<int64_t>(size_of(part, trait.location));
	out.type = analyzer_.types_.qualified(part->unqualified, type->cv);
	return true;
}

} // namespace sema
