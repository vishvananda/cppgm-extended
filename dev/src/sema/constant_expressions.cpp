#include "sema/constant_interpreter.h"

#include <cmath>
#include <limits>
#include <cstring>
#include <string>

#include "lex/number_literal.h"
#include "sema/analyzer.h"
#include "sema/integral_arithmetic.h"
#include "sema/pretty_function.h"
#include "sema/string_literals.h"

// The expressions of constant evaluation over their recorded facts: the
// prvalue an expression yields, the object a glvalue designates, and the
// conversions the analysis recorded where an operand is used, as the
// lowering reads the same facts (lower/expression_lowering.cpp).
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The conversion the analysis recorded where an operand is used; none for
// an expression it left untyped (a hosted wrapper's deferred statement, see
// Analyzer::define_function), whose evaluation then fails.
const Conversion* RecordedConversion(const syntax::Expr* expr)
{
	const ExprFacts* facts = WithoutParentheses(expr)->facts;
	return facts != nullptr ? facts->conversion : nullptr;
}

bool IsPointer(const Type* type)
{
	return type->unqualified->kind == TypeKind::Pointer;
}

bool IsNullptr(const Type* type)
{
	return type->unqualified->kind == TypeKind::Fundamental &&
		type->unqualified->fundamental == FundamentalType::NullptrT;
}

bool IsBool(const Type* type)
{
	return type->unqualified->kind == TypeKind::Fundamental && type->unqualified->fundamental == FundamentalType::Bool;
}

bool IsComparison(TokenKind op)
{
	return op == TokenKind::OP_EQ || op == TokenKind::OP_NE || op == TokenKind::OP_LT || op == TokenKind::OP_GT ||
		op == TokenKind::OP_LE || op == TokenKind::OP_GE;
}

bool IsChainable(TokenKind op)
{
	switch (op)
	{
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
	case TokenKind::OP_STAR:
	case TokenKind::OP_DIV:
	case TokenKind::OP_MOD:
	case TokenKind::OP_AMP:
	case TokenKind::OP_BOR:
	case TokenKind::OP_XOR:
	case TokenKind::OP_LSHIFT:
	case TokenKind::OP_RSHIFT:
		return true;
	default:
		return IsComparison(op);
	}
}

IntegralKind KindOf(const Type* type)
{
	IntegralKind kind = IntegralKind{ 32, true };
	IntegralKindOf(type, kind);
	return kind;
}

// A value rounded to bfloat16 (8 significant bits; float's exponent range,
// its subnormals' quantum 2^-133), to nearest, ties to even; beyond its
// range an infinity.
long double BFloatRounded(long double value)
{
	if (value == 0 || std::isnan(value) || std::isinf(value))
		return value;
	int exponent = 0;
	std::frexp(value, &exponent);
	const int quantum = (exponent > -125 ? exponent : -125) - 8;
	const long double rounded = std::ldexp(std::nearbyint(std::ldexp(value, -quantum)), quantum);
	if (std::fabs(rounded) >= std::ldexp(1.0L, 128))
		return value < 0 ? -std::numeric_limits<long double>::infinity() : std::numeric_limits<long double>::infinity();
	return rounded;
}

// A floating value in the precision of its floating type (4.8, 4.9: the
// target's long double is the host's x87 format).
long double InPrecision(long double value, const Type* type)
{
	switch (FloatingRepresentation(type->unqualified->fundamental))
	{
	case FundamentalType::Float:
		return static_cast<float>(value);
	case FundamentalType::Double:
		return static_cast<double>(value);
	case FundamentalType::BFloat16:
		return BFloatRounded(value);
	default:
		return value;
	}
}

// An integer value as 128 bits: a 128-bit type's whole value, a narrower
// type's extended by its signedness (its bits are, Constant's invariant).
WideBits WideInteger(const ConstValue& value)
{
	if (IsWideIntegral(value.type))
		return WideOf(value);
	return KindOf(value.type).is_signed ? static_cast<WideBits>(static_cast<__int128>(static_cast<int64_t>(value.bits)))
										: static_cast<WideBits>(value.bits);
}

// A 128-bit value converted to the floating type `target` (4.9p2), through
// a long double rounded to nearest for a long double target and to odd (a
// sticky low bit) for a narrower one, which its conversion then rounds
// correctly.
long double WideFloating(WideBits bits, bool is_signed, const Type* target)
{
	const bool negative = is_signed && static_cast<__int128>(bits) < 0;
	const WideBits magnitude = negative ? ~bits + 1 : bits;
	int length = 0;
	for (WideBits rest = magnitude; rest != 0; rest >>= 1)
		++length;
	if (length <= 64)
	{
		const long double value = static_cast<long double>(static_cast<uint64_t>(magnitude));
		return negative ? -value : value;
	}
	int shift = length - 64;
	uint64_t top = static_cast<uint64_t>(magnitude >> shift);
	const WideBits rest = magnitude & ((static_cast<WideBits>(1) << shift) - 1);
	if (FloatingRepresentation(target->unqualified->fundamental) == FundamentalType::LongDouble)
	{
		const WideBits half = static_cast<WideBits>(1) << (shift - 1);
		if ((rest > half || (rest == half && (top & 1) != 0)) && ++top == 0)
		{
			top = uint64_t(1) << 63;
			++shift;
		}
	}
	else if (rest != 0)
		top |= 1;
	const long double value = std::ldexp(static_cast<long double>(top), shift);
	return negative ? -value : value;
}

// A floating type whose values a long double holds exactly (no binary16
// or binary128: their constants are not evaluated; bfloat16's are, as
// Clang evaluates them).
bool ExactlyHeld(const Type* type)
{
	const FundamentalType kind = FloatingRepresentation(type->unqualified->fundamental);
	return kind == FundamentalType::Float || kind == FundamentalType::Double || kind == FundamentalType::LongDouble ||
		kind == FundamentalType::BFloat16;
}

template<typename T>
bool FloatingStep(TokenKind op, long double left, long double right, long double& result)
{
	const T a = static_cast<T>(left), b = static_cast<T>(right);
	T value = 0;
	switch (op)
	{
	case TokenKind::OP_PLUS:
		value = a + b;
		break;
	case TokenKind::OP_MINUS:
		value = a - b;
		break;
	case TokenKind::OP_STAR:
		value = a * b;
		break;
	case TokenKind::OP_DIV:
		if (b == 0)
			return false;
		value = a / b;
		break;
	default:
		return false;
	}
	result = value;
	return !std::isnan(value) && !std::isinf(value);
}

// The decayed type an operand of a built-in operator has (4.1-4.3).
const Type* Decayed(TypeTable& types, const Type* type)
{
	if (type->kind == TypeKind::Array)
		return types.pointer(type->element);
	if (type->kind == TypeKind::Function)
		return types.pointer(type);
	return type->unqualified;
}

bool ConversionFunction(const Conversion* conversion)
{
	return conversion != nullptr && conversion->constructor != nullptr && !conversion->constructor->constructor;
}

} // namespace

// ------------------------------------------------------------- prvalues

bool ConstantInterpreter::rvalue(const syntax::Expr& expr, ConstValue& out)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return fail(e->location, "an expression outside constant evaluation");
	if (!step(e->location))
		return false;
	TypeTable& types = analyzer_.types_;
	if ((facts->flags & kUserConverted) != 0)
	{
		Result result = Result();
		if (!conversion_call(*e, facts->original, static_cast<const FunctionEntity*>(facts->entity), result))
			return false;
		if (facts->category == ValueCategory::PRValue)
		{
			out = result.value;
			return true;
		}
		return read(result.place, out, e->location);
	}
	if ((facts->flags & (kFolded | kStaticConstant)) != 0 && e->kind == syntax::ExprKind::Member &&
		!member_object(static_cast<const syntax::MemberExpr&>(*e)))
		return false;
	if ((facts->flags & (kFolded | kStaticConstant)) != 0)
	{
		// A folded query, enumerator or value-initialized scalar, or a
		// static data member's constant (9.4.2p3).
		const Type* type = facts->type->unqualified;
		if (IsFloating(type))
			out = FloatingValue(type, 0);
		else if (type->kind == TypeKind::MemberPointer)
			out = NullMemberPointer(type);
		else if (IsPointer(type) || IsNullptr(type))
		{
			out = ConstValue();
			out.kind = ValueKind::Pointer;
			out.root = RootKind::Null;
			out.type = type;
		}
		else
			out = IntegerValue(type, facts->value);
		return true;
	}
	if ((facts->flags & kOperatorCall) != 0 && e->kind != syntax::ExprKind::Call)
	{
		if (facts->category != ValueCategory::PRValue)
		{
			ConstPlace place;
			return glvalue(*e, place) && read(place, out, e->location);
		}
		return call_value(*e, out);
	}
	if (e->kind == syntax::ExprKind::BracedInit && ClassOf(facts->type) == nullptr &&
		facts->type->kind != TypeKind::Array)
	{
		// A scalar's braced list: its element converted, or zero (8.5.4p3);
		// a complex value's list of its two parts.
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*e);
		if (IsComplex(facts->type) && list.elements.size == 2)
			return part_list(list.elements, facts->type, out, e->location);
		if (facts->type->unqualified->kind == TypeKind::Vector)
			return lane_list(list.elements, facts->type, out, e->location);
		if (list.elements.empty())
		{
			out = ConstValue();
			out.type = facts->type->unqualified;
			zero_into(out);
			out.flags = 0;
			return true;
		}
		return operand(*list.elements[0], out) && convert_scalar(out, facts->type, e->location);
	}
	if (facts->category != ValueCategory::PRValue)
	{
		const Type* type = facts->type;
		if (type->kind == TypeKind::Array || type->kind == TypeKind::Function)
		{
			// 4.2, 4.3: the address of the array's first element, or of the
			// function.
			ConstPlace place;
			if (!glvalue(*e, place))
				return false;
			if (type->kind == TypeKind::Array && place.past_end)
				return fail(e->location, "an array one past the end of an array");
			if (type->kind == TypeKind::Array)
				place.type = type->element;
			out = PointerTo(place, Decayed(types, type));
			return true;
		}
		if (e->kind == syntax::ExprKind::Conditional)
			return conditional_value(static_cast<const syntax::ConditionalExpr&>(*e), out);
		ConstPlace place;
		return glvalue(*e, place) && read(place, out, e->location);
	}
	if (ClassOf(facts->type) != nullptr || facts->type->kind == TypeKind::Array)
	{
		// A class prvalue's value: the temporary it initializes.
		ConstPlace place;
		ConstValue* value = nullptr;
		if (!temporary(*e, place) || !locate(place, false, value, e->location))
			return false;
		out = *value;
		return true;
	}
	switch (e->kind)
	{
	case syntax::ExprKind::Subscript:
		// A lane of a GNU vector prvalue (an operator call's was above).
		return vector_lane(static_cast<const syntax::SubscriptExpr&>(*e), nullptr, &out);
	case syntax::ExprKind::Literal:
		return literal(static_cast<const syntax::LiteralExpr&>(*e), out);
	case syntax::ExprKind::KeywordLiteral:
	{
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr&>(*e).keyword;
		if (keyword == TokenKind::KW_TRUE || keyword == TokenKind::KW_FALSE)
		{
			out = IntegerValue(types.fundamental(FundamentalType::Bool), keyword == TokenKind::KW_TRUE ? 1 : 0);
			return true;
		}
		if (keyword == TokenKind::KW_NULLPTR)
		{
			out = ConstValue();
			out.kind = ValueKind::Pointer;
			out.root = RootKind::Null;
			out.type = facts->type->unqualified;
			return true;
		}
		ConstPlace self;
		if (!this_place(self, e->location))
			return false;
		out = PointerTo(self, facts->type->unqualified);
		return true;
	}
	case syntax::ExprKind::Binary:
		return binary(static_cast<const syntax::BinaryExpr&>(*e), out);
	case syntax::ExprKind::Unary:
		return unary(static_cast<const syntax::UnaryExpr&>(*e), out);
	case syntax::ExprKind::Postfix:
	{
		ConstPlace place;
		return increment(static_cast<const syntax::UnaryExpr&>(*e), true, place, out);
	}
	case syntax::ExprKind::Conditional:
		return conditional_value(static_cast<const syntax::ConditionalExpr&>(*e), out);
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr& call_expr = static_cast<const syntax::CallExpr&>(*e);
		if ((facts->flags & kIntrinsic) != 0)
			return intrinsic_value(call_expr, out);
		if (call_expr.callee->facts == nullptr && call_expr.arguments.size == 1)
			return cast(*e, *call_expr.arguments[0], out);
		return call_value(*e, out);
	}
	case syntax::ExprKind::Cast:
		return cast(*e, *static_cast<const syntax::CastExpr&>(*e).operand, out);
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& functional = static_cast<const syntax::FunctionalCastExpr&>(*e);
		const syntax::Slice<syntax::Expr*>& operands = functional.braced != nullptr ? functional.braced->elements
																					: functional.arguments;
		if (functional.braced != nullptr && operands.size == 2 && IsComplex(facts->type))
			return part_list(operands, facts->type, out, e->location);
		if (functional.braced != nullptr && facts->type->unqualified->kind == TypeKind::Vector)
			return lane_list(operands, facts->type, out, e->location);
		if (operands.size == 1)
		{
			if (functional.braced != nullptr)
				return operand(*operands[0], out) && convert_scalar(out, facts->type, e->location);
			return cast(*e, *operands[0], out);
		}
		// T() and T{}: value-initialized (8.5p7), zero; a GNU vector's
		// lanes (no lane model here: the value is opaque, never converted
		// to another type, see convert_scalar, nor placed as constant data).
		out = ConstValue();
		out.type = facts->type->unqualified;
		zero_into(out);
		out.flags = 0;
		return true;
	}
	case syntax::ExprKind::BuiltinTrait:
	{
		// A trait's bool or std::size_t value; GNU __real__ of a real operand
		// its value, __imag__ zero (the operand still a constant). Reading a
		// variadic argument, a complex part, a vector conversion and a bit
		// cast are never constants here.
		const syntax::BuiltinTraitExpr& trait = static_cast<const syntax::BuiltinTraitExpr&>(*e);
		const bool any_part = trait.trait == GnuTrait::RealPart || trait.trait == GnuTrait::ImagPart;
		const bool part = any_part && !IsComplex(WithoutParentheses(trait.operand)->facts->type);
		if (any_part && !part)
			return complex_part(trait, out);
		if (part)
		{
			if (!operand(*trait.operand, out))
				return false;
			if (trait.trait == GnuTrait::RealPart)
				return true;
			out = ConstValue();
			out.type = facts->type->unqualified;
			zero_into(out);
			out.flags = 0;
			return true;
		}
		if (trait.trait >= GnuTrait::UnderlyingType)
			return fail(e->location, "a builtin's value outside constant evaluation");
		out = IntegerValue(facts->type->unqualified, facts->value);
		return true;
	}
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
		out = IntegerValue(facts->type->unqualified, facts->value);
		return true;
	case syntax::ExprKind::Id:
		// A pointer template parameter's name: its argument.
		if (facts->entity != nullptr && facts->entity->kind == EntityKind::Enumerator &&
			static_cast<const EnumeratorEntity*>(facts->entity)->argument != nullptr)
		{
			out = ArgumentPointer(static_cast<const EnumeratorEntity*>(facts->entity)->argument);
			return true;
		}
		return fail(e->location, "an expression that is not a constant expression");
	default:
		return fail(e->location, "an expression that is not a constant expression");
	}
}

bool ConstantInterpreter::literal(const syntax::LiteralExpr& expr, ConstValue& out)
{
	const ExprFacts* facts = expr.facts;
	if ((facts->flags & kUserLiteral) != 0)
		return call_value(expr, out);
	const Type* type = facts->type->unqualified;
	if (!IsFloating(type))
	{
		out = IntegerValue(type, facts->value);
		return true;
	}
	const NumberLiteral number = ClassifyNumber(expr.spelling.text, expr.spelling.length);
	unsigned char bytes[16];
	std::string scratch;
	if (!ExactlyHeld(type))
		return fail(expr.location, "a binary16 or binary128 constant is not supported");
	const FundamentalType representation = FloatingRepresentation(number.type);
	if (number.kind != NumberLiteralKind::Floating ||
		!ScanFloatingValue(expr.spelling.text, number.suffix_begin, representation, scratch, bytes))
		return fail(expr.location, "a floating literal outside constant evaluation");
	long double value = 0;
	if (representation == FundamentalType::Float)
	{
		float scanned;
		std::memcpy(&scanned, bytes, sizeof scanned);
		value = scanned;
	}
	else if (representation == FundamentalType::Double)
	{
		double scanned;
		std::memcpy(&scanned, bytes, sizeof scanned);
		value = scanned;
	}
	else
		std::memcpy(&value, bytes, sizeof value);
	out = FloatingValue(type, value);
	return true;
}

// ----------------------------------------------------------- conversions

bool ConstantInterpreter::operand(const syntax::Expr& expr, ConstValue& out)
{
	return converted(expr, RecordedConversion(&expr), out);
}

bool ConstantInterpreter::converted(const syntax::Expr& expr, const Conversion* conversion, ConstValue& out)
{
	// The operand's value as the context uses it: a class operand by its
	// conversion function (12.3.2, the result then converted by the second
	// standard conversion), else its prvalue by the standard conversion.
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->facts == nullptr)
		return fail(e->location, "an expression outside constant evaluation");
	if (ConversionFunction(conversion))
	{
		Result result = Result();
		const FunctionEntity* function = conversion->constructor;
		if (!conversion_call(*e, e->facts, function, result))
			return false;
		const Type* returned = function->type->canonical->element;
		out = result.value;
		if (returned->is_reference() && !read(result.place, out, e->location))
			return false;
		// The second standard conversion as recorded (4.10p3's subobject,
		// 4.11p2's offset).
		return apply_standard(*e, conversion->after, out);
	}
	if (conversion != nullptr && conversion->constructor != nullptr)
		return fail(e->location, "a class object where a scalar is needed");
	return rvalue(*e, out) && apply_standard(*e, conversion, out);
}

bool ConstantInterpreter::apply_standard(const syntax::Expr& expr, const Conversion* conversion, ConstValue& value)
{
	// The value conversion the context recorded (clause 4), from the
	// operand's decayed type to the conversion's result type.
	if (conversion == nullptr || conversion->form == ConversionForm::Reference || conversion->result == nullptr)
		return true;
	const Type* result = conversion->result->unqualified;
	switch (conversion->second)
	{
	case ValueConversion::NullPointer:
		if (result->kind == TypeKind::MemberPointer)
		{
			value = NullMemberPointer(result);
			return true;
		}
		value = ConstValue();
		value.kind = ValueKind::Pointer;
		value.root = RootKind::Null;
		value.type = result;
		return true;
	case ValueConversion::DerivedToBase:
		if (value.kind == ValueKind::Pointer && IsPointer(result))
		{
			// 4.10p3: the base class subobject; the null pointer stays null.
			if (value.root != RootKind::Null)
			{
				ConstPlace place = PlaceOf(value, value.type->element);
				if (!to_base(place, ClassOf(result->element), expr.location))
					return false;
				value = PointerTo(place, result);
			}
			value.type = result;
			return true;
		}
		return convert_scalar(value, result, expr.location);
	case ValueConversion::MemberToDerived:
		return member_adjusted(value, result, false, expr.location);
	default:
		return convert_scalar(value, result, expr.location);
	}
}

bool ConstantInterpreter::convert_scalar(ConstValue& value, const Type* to, uint32_t location)
{
	// A scalar value converted to the scalar type `to` (4.5-4.12, the
	// explicit conversions of 5.2.9 and 5.4 between scalars).
	to = to->unqualified;
	if (to->is_reference())
		to = to->element->unqualified;
	if (value.type == to || IsVoid(to))
		return true;
	if (value.kind == ValueKind::Indeterminate)
		return fail(location, "a conversion of an indeterminate value");
	// A GNU vector cast: the bytes of a vector or an integer as another
	// vector's lanes or an integer (GCC folds them all).
	if ((value.type != nullptr && value.type->unqualified->kind == TypeKind::Vector) || to->kind == TypeKind::Vector)
		return vector_reinterpretation(value, to, location);
	if (IsComplex(to))
		return complex_convert(value, to, location);
	if (IsBool(to))
	{
		bool result = false;
		if (!truth_of(value, result, location))
			return false;
		value = IntegerValue(to, result ? 1 : 0);
		return true;
	}
	if (IsFloating(to))
	{
		if (!ExactlyHeld(to))
			return fail(location, "a binary16 or binary128 constant is not supported");
		long double source = 0;
		if (value.kind == ValueKind::Integer && IsWideIntegral(value.type))
			source = WideFloating(WideOf(value), KindOf(value.type).is_signed, to);
		else if (value.kind == ValueKind::Integer)
			source = KindOf(value.type).is_signed ? static_cast<long double>(static_cast<int64_t>(value.bits))
												  : static_cast<long double>(value.bits);
		else if (value.kind == ValueKind::Floating)
			source = value.floating;
		else
			return fail(location, "a pointer converted to a floating type");
		const long double rounded = InPrecision(source, to);
		// 4.8p1: a value outside the target's range is undefined.
		if (std::isinf(rounded) && !std::isinf(source))
			return fail(location, "a floating conversion out of range");
		value = FloatingValue(to, rounded);
		return true;
	}
	IntegralKind kind;
	if (IntegralKindOf(to, kind))
	{
		if (value.kind == ValueKind::Integer)
		{
			// 4.7: modulo 2^N (a 128-bit target's from the source's value
			// extended to 128 bits).
			value = kind.bits > 64 ? WideValue(to, WideInteger(value))
								   : IntegerValue(to, ConvertIntegral(value.bits, kind));
			return true;
		}
		if (value.kind == ValueKind::Floating)
		{
			// 4.9p1: truncated; undefined unless the result fits.
			const long double truncated = std::trunc(value.floating);
			const long double low = kind.is_signed ? -std::ldexp(1.0L, static_cast<int>(kind.bits) - 1) : 0;
			const long double high = kind.is_signed ? std::ldexp(1.0L, static_cast<int>(kind.bits) - 1)
													: std::ldexp(1.0L, static_cast<int>(kind.bits));
			if (std::isnan(truncated) || truncated < low || truncated >= high)
				return fail(location, "a floating value out of the integral type's range");
			if (kind.bits > 64)
			{
				// The magnitude's halves, exact in long double.
				const long double magnitude = std::fabs(truncated);
				const long double upper = std::floor(std::ldexp(magnitude, -64));
				const long double lower = magnitude - std::ldexp(upper, 64);
				WideBits bits = (static_cast<WideBits>(static_cast<uint64_t>(upper)) << 64) |
					static_cast<uint64_t>(lower);
				value = WideValue(to, truncated < 0 ? ~bits + 1 : bits);
				return true;
			}
			const uint64_t bits = kind.is_signed ? static_cast<uint64_t>(static_cast<int64_t>(truncated))
												 : static_cast<uint64_t>(truncated);
			value = IntegerValue(to, ConvertIntegral(bits, kind));
			return true;
		}
		return fail(location, "a pointer converted to an integer in a constant expression");
	}
	if (to->kind == TypeKind::MemberPointer)
	{
		// 4.4 keeps a pointer to member's value; 4.11p1 makes a null pointer
		// constant the null value.
		if (value.kind == ValueKind::MemberPointer)
		{
			value.type = to;
			return true;
		}
		if ((value.kind == ValueKind::Pointer && value.root == RootKind::Null) ||
			(value.kind == ValueKind::Integer && value.bits == 0))
		{
			value = NullMemberPointer(to);
			return true;
		}
		return fail(location, "a conversion to a pointer to member outside constant evaluation");
	}
	if (to->kind == TypeKind::Pointer || IsNullptr(to))
	{
		if (value.kind == ValueKind::Pointer)
		{
			value.type = to;
			return true;
		}
		if (value.kind == ValueKind::Integer && value.bits == 0)
		{
			// 4.10p1: a null pointer constant.
			value = ConstValue();
			value.kind = ValueKind::Pointer;
			value.root = RootKind::Null;
			value.type = to;
			return true;
		}
		return fail(location, "an integer converted to a pointer in a constant expression");
	}
	return fail(location, "a conversion outside constant evaluation");
}

bool ConstantInterpreter::truth_of(const ConstValue& value, bool& out, uint32_t location)
{
	// 4.12: zero, a null pointer and false are false.
	switch (value.kind)
	{
	case ValueKind::Integer:
		out = (value.bits | value.high) != 0;
		return true;
	case ValueKind::Floating:
		out = value.floating != 0;
		return true;
	case ValueKind::Pointer:
		out = value.root != RootKind::Null;
		return true;
	case ValueKind::MemberPointer:
		out = value.address.entity != nullptr;
		return true;
	case ValueKind::Array:
		if (value.count == 2 && value.type != nullptr && IsComplex(value.type))
			return complex_truth(value, out, location);
		return fail(location, "a truth value of an object that is no scalar");
	default:
		return fail(location, "a truth value of an object that is no scalar");
	}
}

bool ConstantInterpreter::truth(const syntax::Expr& expr, bool& out)
{
	// A condition or logical operand: contextually converted to bool (4p3)
	// as the analysis recorded.
	ConstValue value;
	return operand(expr, value) && truth_of(value, out, expr.location);
}

// ----------------------------------------------------------------- glvalues

bool ConstantInterpreter::glvalue(const syntax::Expr& expr, ConstPlace& out)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return fail(e->location, "an expression outside constant evaluation");
	if (!step(e->location))
		return false;
	if ((facts->flags & kUserConverted) != 0)
	{
		// A conversion function's reference result designates its referent.
		Result result = Result();
		if (!conversion_call(*e, facts->original, static_cast<const FunctionEntity*>(facts->entity), result))
			return false;
		out = result.place;
		return true;
	}
	if ((facts->flags & kOperatorCall) != 0 && e->kind != syntax::ExprKind::Call)
		return call_place(*e, out);
	switch (e->kind)
	{
	case syntax::ExprKind::Id:
		if ((facts->flags & kImplicitThis) != 0 && facts->entity != nullptr &&
			facts->entity->kind == EntityKind::Variable && static_cast<const VariableEntity*>(facts->entity)->member)
		{
			// 9.3.1p3: `(*this).m`.
			ConstPlace self;
			return this_place(self, e->location) &&
				through_naming(self, analyzer_.member_naming_class(*e), e->location) &&
				member_place(self, static_cast<const VariableEntity*>(facts->entity), out, e->location);
		}
		return entity_place(facts->entity, facts->type, out, e->location);
	case syntax::ExprKind::Literal:
		if (facts->type->kind != TypeKind::Array || (facts->flags & kUserLiteral) != 0)
			break;
		out = ConstPlace();
		out.root = RootKind::Literal;
		out.entity = e;
		out.type = facts->type;
		return true;
	case syntax::ExprKind::Unary:
	{
		const syntax::UnaryExpr& unary_expr = static_cast<const syntax::UnaryExpr&>(*e);
		if (unary_expr.op == TokenKind::OP_INC || unary_expr.op == TokenKind::OP_DEC)
		{
			ConstValue old;
			return increment(unary_expr, false, out, old);
		}
		if (unary_expr.op != TokenKind::OP_STAR)
			break;
		// 5.3.1p1: the object or function the pointer points to.
		if (WithoutParentheses(unary_expr.operand)->facts->type->kind == TypeKind::Function)
			return glvalue(*unary_expr.operand, out);
		ConstValue pointer;
		if (!operand(*unary_expr.operand, pointer))
			return false;
		if (pointer.kind != ValueKind::Pointer || pointer.root == RootKind::Null)
			return fail(e->location, "an indirection through a null pointer");
		out = PlaceOf(pointer, facts->type);
		return true;
	}
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr& assign = static_cast<const syntax::BinaryExpr&>(*e);
		return assign.op == TokenKind::OP_ASS ? assignment(assign, out) : compound_assignment(assign, out);
	}
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr& binary_expr = static_cast<const syntax::BinaryExpr&>(*e);
		if (binary_expr.op == TokenKind::OP_DOTSTAR || binary_expr.op == TokenKind::OP_ARROWSTAR)
		{
			// 5.5: the data member a pointer to member designates.
			ConstValue pointer;
			ConstPlace object;
			return member_pointer_object(binary_expr, pointer, object) &&
				member_place(object, static_cast<const VariableEntity*>(pointer.address.entity), out, e->location);
		}
		if (binary_expr.op != TokenKind::OP_COMMA)
			break;
		return discard(*binary_expr.left) && glvalue(*binary_expr.right, out);
	}
	case syntax::ExprKind::Subscript:
		return subscript(static_cast<const syntax::SubscriptExpr&>(*e), out);
	case syntax::ExprKind::Member:
		return member(static_cast<const syntax::MemberExpr&>(*e), out);
	case syntax::ExprKind::BuiltinTrait:
	{
		// GNU __real__ and __imag__ of a complex glvalue.
		const syntax::BuiltinTraitExpr& trait = static_cast<const syntax::BuiltinTraitExpr&>(*e);
		if (trait.trait != GnuTrait::RealPart && trait.trait != GnuTrait::ImagPart)
			break;
		return complex_part_place(trait, out);
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
		bool selected = false;
		if (!truth(*choice.condition, selected))
			return false;
		return glvalue(selected ? *choice.then_expr : *choice.else_expr, out);
	}
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr& call_expr = static_cast<const syntax::CallExpr&>(*e);
		if (call_expr.callee->facts == nullptr && call_expr.arguments.size == 1)
			return glvalue(*call_expr.arguments[0], out);
		return call_place(*e, out);
	}
	case syntax::ExprKind::Cast:
		return reference_cast(*e, *static_cast<const syntax::CastExpr&>(*e).operand, out);
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& functional = static_cast<const syntax::FunctionalCastExpr&>(*e);
		if (functional.arguments.size == 1)
			return glvalue(*functional.arguments[0], out);
		break;
	}
	case syntax::ExprKind::TypeTrait:
		// 5.2.8p3-p4: typeid of a type or of an operand that is not a
		// glvalue of polymorphic class type designates its static type's
		// object, without evaluating the operand (5.19p2 excludes the
		// other); libc++'s std::function puts `&typeid(void)` in a
		// constexpr object.
		if (static_cast<const syntax::TypeTraitExpr&>(*e).keyword != TokenKind::KW_TYPEID ||
			(facts->flags & kDynamicType) != 0 || facts->target == nullptr)
			break;
		out = ConstPlace();
		out.root = RootKind::TypeInfo;
		out.entity = facts->target->canonical->unqualified;
		out.type = facts->type;
		return true;
	default:
		break;
	}
	// A class prvalue used as an object: its temporary (12.2p1).
	if (facts->category == ValueCategory::PRValue && ClassOf(facts->type) != nullptr)
		return temporary(*e, out);
	return fail(e->location, "a glvalue outside constant evaluation");
}

bool ConstantInterpreter::object_place(const syntax::Expr& expr, ConstPlace& out)
{
	// A member access's or call's object: a glvalue, or a class prvalue
	// materialized in a temporary (12.2p1).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->facts != nullptr && e->facts->category == ValueCategory::PRValue && (e->facts->flags & kUserConverted) == 0)
		return temporary(*e, out);
	return glvalue(*e, out);
}

bool ConstantInterpreter::entity_place(const Entity* entity, const Type* type, ConstPlace& out, uint32_t location)
{
	if (entity == nullptr)
		return fail(location, "a name outside constant evaluation");
	if (local(entity, out))
		return true;
	switch (entity->kind)
	{
	case EntityKind::Function:
		out = ConstPlace();
		out.root = RootKind::Function;
		out.entity = entity;
		out.type = entity->type;
		return true;
	case EntityKind::Parameter:
		return fail(location, "a parameter is not usable in a constant expression");
	case EntityKind::Variable:
	{
		const VariableEntity* variable = static_cast<const VariableEntity*>(entity);
		if (variable->member)
		{
			// A namespace- or block-scope anonymous union's member named
			// alone: in the union's unnamed object (9.5p5).
			const ClassEntity* home = MemberClassOf(variable);
			if (!home->anonymous || home->storage == nullptr)
				return fail(location, "a non-static data member named without an object");
			ConstPlace object;
			return entity_place(home->storage->entity, home->type, object, location) &&
				member_place(object, variable, out, location);
		}
		if (variable->predefined_name)
		{
			// A function's predefined __func__ (8.4.1p8), a static const
			// array of the characters the analysis gave it: as the string
			// literal of them (the lowering shares one object).
			const syntax::LiteralExpr* literal = analyzer_.text_literal(
				static_cast<const PredefinedNameEntity*>(variable)->text);
			if (literal == nullptr)
				return fail(location, "a predefined name's characters");
			out = ConstPlace();
			out.root = RootKind::Literal;
			out.entity = literal;
			out.type = literal->facts->type;
			return true;
		}
		if (const BindingEntity* alias = BindingAlias(variable))
		{
			// A structured binding: its element or member of the object e
			// (C++17's 11.5p2, p4).
			const Decomposition* decomposition = alias->decomposition;
			ConstPlace object;
			if (decomposition == nullptr || decomposition->form == DecompositionForm::Tuple ||
				!entity_place(decomposition->object, decomposition->object->type, object, location))
				return fail(location, "a structured binding outside constant evaluation");
			object.type = decomposition->type;
			if (decomposition->form == DecompositionForm::Members)
				return alias->member != nullptr && member_place(object, alias->member, out, location);
			ConstValue pointer = PointerTo(object, analyzer_.types_.pointer(alias->type));
			if (!advance(pointer, static_cast<int64_t>(alias->index), location))
				return false;
			out = PlaceOf(pointer, alias->type);
			return true;
		}
		if (variable->type->is_reference())
		{
			// A reference initialized with a constant expression designates
			// its referent (5.19p2).
			const ConstValue* bound = analyzer_.variable_value(variable);
			if (bound == nullptr || bound->kind != ValueKind::Pointer)
				return fail(location, "a reference is not usable in a constant expression");
			out = PlaceOf(*bound, variable->type->element);
			return true;
		}
		out = ConstPlace();
		out.root = RootKind::Variable;
		out.entity = variable;
		out.type = variable->type;
		return true;
	}
	default:
		(void)type;
		return fail(location, "a name that designates no object");
	}
}

bool ConstantInterpreter::this_place(ConstPlace& out, uint32_t location)
{
	// 9.3.2p1: the object of the member function call being evaluated.
	if (frames_.size() == first_frame_ || !frames_.back().has_this)
		return fail(location, "`this` outside a constexpr member function call");
	out = frames_.back().self;
	return true;
}

bool ConstantInterpreter::local(const Entity* entity, ConstPlace& out) const
{
	const size_t first = frames_.size() == first_frame_ ? first_local_ : frames_.back().first_local;
	for (size_t i = locals_.size(); i-- > first;)
	{
		if (locals_[i].entity == entity)
		{
			out = locals_[i].place;
			return true;
		}
	}
	return false;
}

bool ConstantInterpreter::discard(const syntax::Expr& expr)
{
	// A discarded-value expression (clause 5p10): evaluated for its
	// effects; a glvalue is not read.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return fail(e->location, "an expression outside constant evaluation");
	if (IsVoid(facts->type) && e->kind == syntax::ExprKind::Call)
	{
		Result result = Result();
		return call(static_cast<const syntax::CallExpr&>(*e), result);
	}
	if (IsVoid(facts->type) && e->kind == syntax::ExprKind::Cast)
		return discard(*static_cast<const syntax::CastExpr*>(e)->operand);
	if (IsVoid(facts->type) && e->kind == syntax::ExprKind::Conditional)
	{
		const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
		bool selected = false;
		return truth(*choice.condition, selected) && discard(selected ? *choice.then_expr : *choice.else_expr);
	}
	if (e->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA &&
		(facts->flags & kOperatorCall) == 0)
		return discard(*static_cast<const syntax::BinaryExpr*>(e)->left) &&
			discard(*static_cast<const syntax::BinaryExpr*>(e)->right);
	if (IsVoid(facts->type))
		return fail(e->location, "a void expression outside constant evaluation");
	if (facts->category != ValueCategory::PRValue && facts->type->kind != TypeKind::Function)
	{
		ConstPlace place;
		return glvalue(*e, place);
	}
	ConstValue value;
	return rvalue(*e, value);
}

// ----------------------------------------------------------------- operators

bool ConstantInterpreter::binary(const syntax::BinaryExpr& expr, ConstValue& out)
{
	if ((expr.facts->flags & kOperatorCall) != 0)
		return call_value(expr, out);
	if (expr.op == TokenKind::OP_COMMA)
		return discard(*expr.left) && rvalue(*expr.right, out);
	if (expr.op == TokenKind::OP_LAND || expr.op == TokenKind::OP_LOR)
		return logical(expr, out);
	if (expr.op == TokenKind::OP_DOTSTAR || expr.op == TokenKind::OP_ARROWSTAR)
		return fail(expr.location, "a pointer to member in a constant expression");
	// A left-nested chain of arithmetic and relational operators is walked
	// iteratively: its leftmost operand first, then each operator upward.
	SmallList<const syntax::BinaryExpr*, 16> spine;
	const syntax::BinaryExpr* node = &expr;
	for (;;)
	{
		spine.push_back(node);
		const syntax::Expr* left = WithoutParentheses(node->left);
		if (left->kind != syntax::ExprKind::Binary || !IsChainable(static_cast<const syntax::BinaryExpr*>(left)->op) ||
			(left->facts->flags & kOperatorCall) != 0)
			break;
		node = static_cast<const syntax::BinaryExpr*>(left);
	}
	ConstValue value = ConstValue();
	bool known = false;
	for (size_t i = spine.size(); i-- > 0;)
	{
		const syntax::BinaryExpr& current = *spine[i];
		const bool additive = current.op == TokenKind::OP_PLUS || current.op == TokenKind::OP_MINUS;
		const bool pointer_operand = additive &&
			(IsPointer(Decayed(analyzer_.types_, WithoutParentheses(current.left)->facts->type)) ||
				IsPointer(Decayed(analyzer_.types_, WithoutParentheses(current.right)->facts->type)));
		ConstValue result;
		if (pointer_operand)
		{
			if (!pointer_arithmetic(current, known ? &value : nullptr, result))
				return false;
		}
		else
		{
			if (!known && !rvalue(*current.left, value))
				return false;
			if (!(IsComparison(current.op) ? comparison(current, value, result) : arithmetic(current, value, result)))
				return false;
		}
		value = result;
		known = true;
	}
	out = value;
	return true;
}

bool ConstantInterpreter::floating_operation(
	TokenKind op, const Type* type, long double left, long double right, long double& out, uint32_t location)
{
	// 5.6, 5.7 in the operation's own precision, as the native back end
	// evaluates it: a double operation in x87 extended precision, then
	// rounded to double (pa25/plan.md D1, [expr]p11); a zero divisor or a
	// result outside the type's range is undefined (5p4, 5.6p4).
	bool defined = false;
	switch (FloatingRepresentation(type->unqualified->fundamental))
	{
	case FundamentalType::Float:
		defined = FloatingStep<float>(op, left, right, out);
		break;
	case FundamentalType::Double:
		defined = FloatingStep<long double>(op, left, right, out);
		out = static_cast<double>(out);
		defined = defined && !std::isinf(out);
		break;
	case FundamentalType::BFloat16:
		// Each operation rounded (Clang's constant evaluation; in float,
		// then rounded: 24 >= 2 * 8 + 2 bits make that one rounding).
		defined = FloatingStep<float>(op, left, right, out);
		out = BFloatRounded(out);
		defined = defined && !std::isinf(out);
		break;
	default:
		defined = FloatingStep<long double>(op, left, right, out);
		break;
	}
	return defined || fail(location, "a floating operation with an undefined result");
}

bool ConstantInterpreter::arithmetic(const syntax::BinaryExpr& expr, const ConstValue& left_value, ConstValue& out)
{
	// 5.6-5.8, 5.11-5.13 on the operands converted as recorded (the usual
	// arithmetic conversions; a shift's promotions, each its own).
	const ExprFacts* right_facts = WithoutParentheses(expr.right)->facts;
	const bool pointer_right = (expr.op == TokenKind::OP_PLUS || expr.op == TokenKind::OP_MINUS) &&
		IsPointer(Decayed(analyzer_.types_, right_facts->type));
	if (left_value.kind == ValueKind::Pointer || pointer_right)
		return fail(expr.location, "pointer arithmetic outside constant evaluation");
	ConstValue left = left_value, right;
	if (!apply_standard(*expr.left, WithoutParentheses(expr.left)->facts->conversion, left) ||
		!operand(*expr.right, right))
		return false;
	const Type* type = expr.facts->type->unqualified;
	if (IsComplex(type))
		return complex_operation(expr.op, type, left, right, out, expr.location);
	if (IsFloating(type))
	{
		if (!convert_scalar(left, type, expr.location) || !convert_scalar(right, type, expr.location))
			return false;
		long double result = 0;
		if (!floating_operation(expr.op, type, left.floating, right.floating, result, expr.location))
			return false;
		out = FloatingValue(type, result);
		return true;
	}
	if (left.kind != ValueKind::Integer || right.kind != ValueKind::Integer)
		return fail(expr.location, "an arithmetic operand that is not an integer");
	if (IsWideIntegral(type))
	{
		// A 128-bit type's operation (the shift count of its own type).
		WideBits result = 0;
		const bool shift = expr.op == TokenKind::OP_LSHIFT || expr.op == TokenKind::OP_RSHIFT;
		const bool defined = shift
			? WideShift(expr.op, KindOf(type).is_signed, WideInteger(left), KindOf(right.type), right.bits, result)
			: WideOperation(expr.op, KindOf(type).is_signed, WideInteger(left), WideInteger(right), result);
		if (shift && IsWideIntegral(right.type) && right.high != 0 && defined)
			return fail(expr.location, "a shift outside its defined range");
		if (!defined)
			return fail(expr.location,
				shift ? "a shift outside its defined range" : "an arithmetic operation with an undefined result");
		out = WideValue(type, result);
		return true;
	}
	out = IntegerValue(type, 0);
	if (expr.op == TokenKind::OP_LSHIFT || expr.op == TokenKind::OP_RSHIFT)
	{
		if (!IntegralShift(expr.op, KindOf(type), left.bits, KindOf(right.type), right.bits, out.bits))
			return fail(expr.location, "a shift outside its defined range");
		return true;
	}
	if (!IntegralOperation(expr.op, KindOf(type), left.bits, right.bits, out.bits))
		return fail(expr.location, "an arithmetic operation with an undefined result");
	return true;
}

bool ConstantInterpreter::pointer_arithmetic(
	const syntax::BinaryExpr& expr, const ConstValue* left_value, ConstValue& out)
{
	// 5.7p5: a pointer moved by an integer's elements; 5.7p6: the
	// difference of two pointers into one array, in elements.
	TypeTable& types = analyzer_.types_;
	const Type* left_type = Decayed(types, WithoutParentheses(expr.left)->facts->type);
	const Type* right_type = Decayed(types, WithoutParentheses(expr.right)->facts->type);
	ConstValue left, right;
	if (left_value != nullptr)
		left = *left_value;
	else if (!rvalue(*expr.left, left))
		return false;
	if (!rvalue(*expr.right, right))
		return false;
	if (IsPointer(left_type) && IsPointer(right_type))
	{
		// 5.7p6: elements of one array (or one past its end).
		if (left.root != right.root || left.count != right.count || left.extra != right.extra ||
			(left.address.entity != right.address.entity && !same_literal(left, right)))
			return fail(expr.location, "a difference of pointers into different objects");
		const int64_t size = static_cast<int64_t>(size_of(left_type->element, expr.location));
		if (size == 0)
			return fail(expr.location, "a difference of pointers to an incomplete type");
		int64_t left_first = 0, left_bound = 0, right_first = 0, right_bound = 0;
		if (left.root == RootKind::Null)
			left_first = right_first = 0;
		else if (!array_of(PlaceOf(left, left_type->element), size, left_first, left_bound, expr.location) ||
			!array_of(PlaceOf(right, right_type->element), size, right_first, right_bound, expr.location))
			return false;
		if (left_first != right_first || left_bound != right_bound)
			return fail(expr.location, "a difference of pointers into different arrays");
		out = IntegerValue(
			expr.facts->type->unqualified, static_cast<uint64_t>((left.address.offset - right.address.offset) / size));
		return true;
	}
	ConstValue pointer = IsPointer(left_type) ? left : right;
	const ConstValue& count = IsPointer(left_type) ? right : left;
	if (pointer.kind != ValueKind::Pointer || count.kind != ValueKind::Integer)
		return fail(expr.location, "pointer arithmetic outside constant evaluation");
	int64_t steps = KindOf(count.type).is_signed ? static_cast<int64_t>(count.bits) : static_cast<int64_t>(count.bits);
	if (expr.op == TokenKind::OP_MINUS)
		steps = -steps;
	pointer.type = expr.facts->type->unqualified;
	if (!advance(pointer, steps, expr.location))
		return false;
	out = pointer;
	return true;
}

bool ConstantInterpreter::comparison(const syntax::BinaryExpr& expr, const ConstValue& left_value, ConstValue& out)
{
	// 5.9, 5.10 on the operands converted to their common type; pointers
	// compare by the objects they designate.
	ConstValue left = left_value, right;
	if (!apply_standard(*expr.left, WithoutParentheses(expr.left)->facts->conversion, left) ||
		!operand(*expr.right, right))
		return false;
	const Type* boolean = analyzer_.types_.fundamental(FundamentalType::Bool);
	if (left.kind == ValueKind::Array || right.kind == ValueKind::Array)
	{
		// GNU complex values (of the common complex type).
		bool result = false;
		if (!complex_equal(expr.op == TokenKind::OP_EQ, left, right, result, expr.location))
			return false;
		out = IntegerValue(boolean, result ? 1 : 0);
		return true;
	}
	if (left.kind == ValueKind::MemberPointer || right.kind == ValueKind::MemberPointer)
	{
		// 5.10p2: null pointers to members are equal, others when they
		// designate the same member of the same subobject (unspecified for
		// a virtual member function's).
		const Type* common = left.kind == ValueKind::MemberPointer ? left.type : right.type;
		if (!convert_scalar(left, common, expr.location) || !convert_scalar(right, common, expr.location))
			return false;
		const Entity* a = static_cast<const Entity*>(left.address.entity);
		const Entity* b = static_cast<const Entity*>(right.address.entity);
		const bool virtual_member = (a != nullptr && a->kind == EntityKind::Function &&
										static_cast<const FunctionEntity*>(a)->is_virtual) ||
			(b != nullptr && b->kind == EntityKind::Function && static_cast<const FunctionEntity*>(b)->is_virtual);
		if (virtual_member && a != nullptr && b != nullptr)
			return fail(expr.location, "an unspecified comparison of pointers to virtual member functions");
		const bool equal = a == b && (a == nullptr || left.address.offset == right.address.offset);
		out = IntegerValue(boolean, (expr.op == TokenKind::OP_EQ) == equal ? 1 : 0);
		return true;
	}
	if (left.kind == ValueKind::Pointer || right.kind == ValueKind::Pointer)
	{
		if (left.kind == ValueKind::Integer && left.bits == 0)
			convert_scalar(left, right.type, expr.location);
		if (right.kind == ValueKind::Integer && right.bits == 0)
			convert_scalar(right, left.type, expr.location);
		if (left.kind != ValueKind::Pointer || right.kind != ValueKind::Pointer)
			return fail(expr.location, "a comparison of a pointer with a non-pointer");
		const bool same_object = left.root == right.root && left.count == right.count && left.extra == right.extra &&
			(left.address.entity == right.address.entity || same_literal(left, right));
		if (expr.op == TokenKind::OP_EQ || expr.op == TokenKind::OP_NE)
		{
			const bool equal = same_object && left.address.offset == right.address.offset;
			out = IntegerValue(boolean, (expr.op == TokenKind::OP_EQ) == equal ? 1 : 0);
			return true;
		}
		// 5.9p2: an order only within one complete object, between elements
		// of one array or members of one access control.
		const Type* root = nullptr;
		if (!same_object || left.root == RootKind::Null || left.root == RootKind::Function ||
			!root_type(PlaceOf(left, left.type->element), root, expr.location) ||
			!ordered(root, left.address.offset, right.address.offset, 0))
			return fail(expr.location, "an unspecified pointer comparison");
		out = IntegerValue(boolean,
			IntegralCompare(
				expr.op, true, static_cast<uint64_t>(left.address.offset), static_cast<uint64_t>(right.address.offset))
				? 1
				: 0);
		return true;
	}
	if (left.kind == ValueKind::Floating || right.kind == ValueKind::Floating)
	{
		const Type* common = left.kind == ValueKind::Floating ? left.type : right.type;
		if (!convert_scalar(left, common, expr.location) || !convert_scalar(right, common, expr.location))
			return false;
		const long double a = left.floating, b = right.floating;
		bool result = false;
		switch (expr.op)
		{
		case TokenKind::OP_EQ:
			result = a == b;
			break;
		case TokenKind::OP_NE:
			result = a != b;
			break;
		case TokenKind::OP_LT:
			result = a < b;
			break;
		case TokenKind::OP_GT:
			result = a > b;
			break;
		case TokenKind::OP_LE:
			result = a <= b;
			break;
		default:
			result = a >= b;
			break;
		}
		out = IntegerValue(boolean, result ? 1 : 0);
		return true;
	}
	if (left.kind != ValueKind::Integer || right.kind != ValueKind::Integer)
		return fail(expr.location, "a comparison of values that are not scalars");
	const bool is_signed = KindOf(left.type).is_signed;
	if (IsWideIntegral(left.type) || IsWideIntegral(right.type))
	{
		out = IntegerValue(boolean, WideCompare(expr.op, is_signed, WideInteger(left), WideInteger(right)) ? 1 : 0);
		return true;
	}
	out = IntegerValue(boolean, IntegralCompare(expr.op, is_signed, left.bits, right.bits) ? 1 : 0);
	return true;
}

bool ConstantInterpreter::logical(const syntax::BinaryExpr& expr, ConstValue& out)
{
	// 5.14, 5.15: the right operand is not evaluated when the left decides.
	bool left = false;
	if (!truth(*expr.left, left))
		return false;
	const bool decided = expr.op == TokenKind::OP_LOR ? left : !left;
	bool result = left;
	if (!decided && !truth(*expr.right, result))
		return false;
	out = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Bool), result ? 1 : 0);
	return true;
}

bool ConstantInterpreter::assignment(const syntax::BinaryExpr& expr, ConstPlace& out)
{
	// 5.17p2: the right operand's value converted to the left's type
	// replaces the object's (C++14's assignment in constant evaluation).
	if ((expr.facts->flags & kOperatorCall) != 0)
		return call_place(expr, out);
	const Type* type = WithoutParentheses(expr.left)->facts->type;
	ConstValue value;
	if (expr.right->kind == syntax::ExprKind::BracedInit)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*expr.right);
		if (list.elements.empty())
		{
			value = ConstValue();
			value.type = type->unqualified;
			zero_into(value);
			value.flags = 0;
		}
		else if (!operand(*list.elements[0], value))
			return false;
	}
	else if (!operand(*expr.right, value))
		return false;
	if (!convert_scalar(value, type, expr.location) || !glvalue(*expr.left, out))
		return false;
	value.flags &= ~kValueZeroed;
	return write(out, value, expr.location);
}

bool ConstantInterpreter::compound_assignment(const syntax::BinaryExpr& expr, ConstPlace& out)
{
	// 5.17p7: E1 op= E2 as E1 = E1 op E2, E1 evaluated once; the operation
	// in the type the analysis recorded.
	if ((expr.facts->flags & kOperatorCall) != 0)
		return call_place(expr, out);
	TokenKind op = TokenKind::OP_PLUS;
	switch (expr.op)
	{
	case TokenKind::OP_PLUSASS:
		op = TokenKind::OP_PLUS;
		break;
	case TokenKind::OP_MINUSASS:
		op = TokenKind::OP_MINUS;
		break;
	case TokenKind::OP_STARASS:
		op = TokenKind::OP_STAR;
		break;
	case TokenKind::OP_DIVASS:
		op = TokenKind::OP_DIV;
		break;
	case TokenKind::OP_MODASS:
		op = TokenKind::OP_MOD;
		break;
	case TokenKind::OP_BANDASS:
		op = TokenKind::OP_AMP;
		break;
	case TokenKind::OP_BORASS:
		op = TokenKind::OP_BOR;
		break;
	case TokenKind::OP_XORASS:
		op = TokenKind::OP_XOR;
		break;
	case TokenKind::OP_LSHIFTASS:
		op = TokenKind::OP_LSHIFT;
		break;
	case TokenKind::OP_RSHIFTASS:
		op = TokenKind::OP_RSHIFT;
		break;
	default:
		return fail(expr.location, "an assignment outside constant evaluation");
	}
	const Type* left_type = WithoutParentheses(expr.left)->facts->type->unqualified;
	ConstValue old, right;
	if (!glvalue(*expr.left, out) || !read(out, old, expr.location))
		return false;
	if (IsPointer(left_type))
	{
		if (!rvalue(*expr.right, right) || right.kind != ValueKind::Integer)
			return false;
		const int64_t steps = static_cast<int64_t>(right.bits);
		if (!advance(old, op == TokenKind::OP_MINUS ? -steps : steps, expr.location))
			return false;
		return write(out, old, expr.location);
	}
	const Type* operation = expr.facts->target != nullptr ? expr.facts->target->unqualified : left_type;
	if (!operand(*expr.right, right) || !convert_scalar(old, operation, expr.location))
		return false;
	ConstValue result;
	if (IsComplex(operation))
	{
		if (!complex_operation(op, operation, old, right, result, expr.location))
			return false;
	}
	else if (IsFloating(operation))
	{
		long double value = 0;
		if (!convert_scalar(right, operation, expr.location) ||
			!floating_operation(op, operation, old.floating, right.floating, value, expr.location))
			return false;
		result = FloatingValue(operation, value);
	}
	else
	{
		if (old.kind != ValueKind::Integer || right.kind != ValueKind::Integer)
			return fail(expr.location, "a compound assignment of values that are not integers");
		result = IntegerValue(operation, 0);
		const bool shift = op == TokenKind::OP_LSHIFT || op == TokenKind::OP_RSHIFT;
		bool defined = false;
		if (IsWideIntegral(operation))
		{
			WideBits wide = 0;
			const bool is_signed = KindOf(operation).is_signed;
			if (!shift && !convert_scalar(right, operation, expr.location))
				return false;
			defined = shift ? WideShift(op, is_signed, WideInteger(old), KindOf(right.type), right.bits, wide) &&
					!(IsWideIntegral(right.type) && right.high != 0)
							: WideOperation(op, is_signed, WideInteger(old), WideInteger(right), wide);
			result = WideValue(operation, wide);
		}
		else
			defined = shift
				? IntegralShift(op, KindOf(operation), old.bits, KindOf(right.type), right.bits, result.bits)
				: IntegralOperation(op, KindOf(operation), old.bits, right.bits, result.bits);
		if (!defined)
			return fail(expr.location, "an arithmetic operation with an undefined result");
	}
	return convert_scalar(result, left_type, expr.location) && write(out, result, expr.location);
}

bool ConstantInterpreter::unary(const syntax::UnaryExpr& expr, ConstValue& out)
{
	if ((expr.facts->flags & kOperatorCall) != 0)
		return call_value(expr, out);
	switch (expr.op)
	{
	case TokenKind::OP_AMP:
	{
		// 5.3.1p3: the address of the lvalue or function; `&C::m`, the
		// member at no adjustment.
		if (expr.facts->type->unqualified->kind == TypeKind::MemberPointer)
		{
			const Entity* member = WithoutParentheses(expr.operand)->facts->entity;
			if (member == nullptr)
				return fail(expr.location, "a pointer to member of no member");
			out = NullMemberPointer(expr.facts->type->unqualified);
			out.address.entity = member;
			return true;
		}
		ConstPlace place;
		if (!glvalue(*expr.operand, place))
			return false;
		if (place.bit_field != nullptr)
			return fail(expr.location, "the address of a bit-field");
		out = PointerTo(place, expr.facts->type->unqualified);
		return true;
	}
	case TokenKind::OP_LNOT:
	{
		bool value = false;
		if (!truth(*expr.operand, value))
			return false;
		out = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Bool), value ? 0 : 1);
		return true;
	}
	case TokenKind::OP_PLUS:
		return operand(*expr.operand, out);
	case TokenKind::OP_MINUS:
	case TokenKind::OP_COMPL:
	{
		ConstValue value;
		if (!operand(*expr.operand, value))
			return false;
		const Type* type = expr.facts->type->unqualified;
		if (IsComplex(type))
			return complex_unary(expr.op, type, value, out, expr.location);
		if (value.kind == ValueKind::Floating && expr.op == TokenKind::OP_MINUS)
		{
			out = FloatingValue(type, -value.floating);
			return true;
		}
		if (value.kind != ValueKind::Integer)
			return fail(expr.location, "a unary operand that is not an integer");
		if (IsWideIntegral(type))
		{
			WideBits result = ~WideInteger(value);
			if (expr.op == TokenKind::OP_MINUS &&
				!WideOperation(TokenKind::OP_MINUS, KindOf(type).is_signed, 0, WideInteger(value), result))
				return fail(expr.location, "a negation that overflows");
			out = WideValue(type, result);
			return true;
		}
		out = IntegerValue(type, 0);
		if (expr.op == TokenKind::OP_COMPL)
		{
			out.bits = ConvertIntegral(~value.bits, KindOf(type));
			return true;
		}
		if (!IntegralOperation(TokenKind::OP_MINUS, KindOf(type), 0, value.bits, out.bits))
			return fail(expr.location, "a negation that overflows");
		return true;
	}
	default:
	{
		ConstPlace place;
		return glvalue(expr, place) && read(place, out, expr.location);
	}
	}
}

bool ConstantInterpreter::increment(const syntax::UnaryExpr& expr, bool postfix, ConstPlace& place, ConstValue& old)
{
	// 5.2.6, 5.3.2: one more or less (a pointer by one element), the result
	// converted back to the operand's type from its promoted type.
	if ((expr.facts->flags & kOperatorCall) != 0)
	{
		if (postfix)
			return call_value(expr, old);
		return call_place(expr, place);
	}
	if (!glvalue(*expr.operand, place) || !read(place, old, expr.location))
		return false;
	const bool decrement = expr.op == TokenKind::OP_DEC;
	const Type* type = WithoutParentheses(expr.operand)->facts->type->unqualified;
	ConstValue stepped = old;
	if (IsComplex(type))
	{
		// The old value's parts kept apart from the object's, which the
		// write replaces.
		if (!complex_value(type, old.elements[0], old.elements[1], old) ||
			!complex_step(type, old, decrement, stepped, expr.location))
			return false;
	}
	else if (old.kind == ValueKind::Pointer)
	{
		if (!advance(stepped, decrement ? -1 : 1, expr.location))
			return false;
	}
	else if (old.kind == ValueKind::Floating)
	{
		// In the operand's precision, rounded once (x += 1, 5.2.6p1).
		long double result = 0;
		if (!floating_operation(
				decrement ? TokenKind::OP_MINUS : TokenKind::OP_PLUS, type, old.floating, 1.0L, result, expr.location))
			return false;
		stepped = FloatingValue(type, result);
	}
	else if (IsBool(type))
		stepped = IntegerValue(type, 1);
	else if (old.kind == ValueKind::Integer && IsWideIntegral(type))
	{
		WideBits result = 0;
		if (!WideOperation(decrement ? TokenKind::OP_MINUS : TokenKind::OP_PLUS, KindOf(type).is_signed,
				WideInteger(old), 1, result))
			return fail(expr.location, "an increment that overflows");
		stepped = WideValue(type, result);
	}
	else if (old.kind == ValueKind::Integer)
	{
		const Type* promoted = PromotedType(analyzer_.types_, type);
		uint64_t bits = 0;
		if (!IntegralOperation(
				decrement ? TokenKind::OP_MINUS : TokenKind::OP_PLUS, KindOf(promoted), old.bits, 1, bits))
			return fail(expr.location, "an increment that overflows");
		stepped = IntegerValue(type, ConvertIntegral(bits, KindOf(type)));
	}
	else
		return fail(expr.location, "an increment outside constant evaluation");
	return write(place, stepped, expr.location);
}

bool ConstantInterpreter::intrinsic_value(const syntax::CallExpr& call_expr, ConstValue& out)
{
	// A builtin the analysis expanded (sema::Intrinsic) with constant
	// operands.
	const ExprFacts* facts = call_expr.facts;
	const TypeTable& types = analyzer_.types_;
	const Intrinsic which = static_cast<Intrinsic>(facts->value & 0xff);
	// A signaling NaN of float, double or long double (binary16's and
	// binary128's are no constants here): its bits as the lowering states
	// them (std::numeric_limits<T>::signaling_NaN() is constexpr).
	if (which == Intrinsic::NanSignaling)
	{
		const FundamentalType fundamental = facts->type->unqualified->fundamental;
		const FundamentalType representation = FloatingRepresentation(fundamental);
		if (representation != FundamentalType::Float && representation != FundamentalType::Double &&
			representation != FundamentalType::LongDouble)
			return fail(call_expr.location, "a signaling NaN of a binary16 or binary128 type");
		out = FloatingValue(facts->type, BuiltinSignalingNan(representation, *call_expr.arguments[0]));
		return true;
	}
	if (which == Intrinsic::MakeComplex)
		return part_list(call_expr.arguments, facts->type, out, call_expr.location);
	if (which == Intrinsic::Nan || which == Intrinsic::Inf)
	{
		out = FloatingValue(facts->type,
			which == Intrinsic::Nan ? BuiltinNan(facts->type->unqualified->fundamental, *call_expr.arguments[0])
									: std::numeric_limits<long double>::infinity());
		return true;
	}
	// __builtin_addressof (std::addressof): the lvalue's address, never
	// an operator& (5.3.1p3).
	if (which == Intrinsic::AddressOf)
	{
		ConstPlace place;
		if (!glvalue(*call_expr.arguments[0], place))
			return false;
		if (place.bit_field != nullptr)
			return fail(call_expr.location, "the address of a bit-field");
		out = PointerTo(place, facts->type->unqualified);
		return true;
	}
	// A constant evaluation is one (C++20's 20.15.11).
	if (which == Intrinsic::IsConstantEvaluated)
	{
		out = IntegerValue(types.fundamental(FundamentalType::Bool), 1);
		return true;
	}
	if ((which >= Intrinsic::ByteSwap && which <= Intrinsic::PopCount) ||
		(which >= Intrinsic::GenericCountLeadingZeros && which <= Intrinsic::GenericPopCount))
		return bit_builtin(call_expr, out);
	if (which >= Intrinsic::AddOverflow && which <= Intrinsic::MulOverflow)
		return overflow_builtin(call_expr, out);
	if (which >= Intrinsic::SourceFile && which <= Intrinsic::SourceColumn)
		return source_location(call_expr, out);
	const syntax::Slice<syntax::Expr*>& arguments = call_expr.arguments;
	if (which >= Intrinsic::IsGreater && which <= Intrinsic::IsUnordered)
	{
		// The quiet comparisons of constant operands.
		ConstValue x, y;
		if (!operand(*arguments[0], x) || !operand(*arguments[1], y) || x.kind != ValueKind::Floating ||
			y.kind != ValueKind::Floating)
			return false;
		const long double a = x.floating, b = y.floating;
		const bool unordered = std::isnan(a) || std::isnan(b);
		bool result = false;
		switch (which)
		{
		case Intrinsic::IsGreater:
			result = !unordered && a > b;
			break;
		case Intrinsic::IsGreaterEqual:
			result = !unordered && a >= b;
			break;
		case Intrinsic::IsLess:
			result = !unordered && a < b;
			break;
		case Intrinsic::IsLessEqual:
			result = !unordered && a <= b;
			break;
		case Intrinsic::IsLessGreater:
			result = !unordered && a != b;
			break;
		default:
			result = unordered;
			break;
		}
		out = IntegerValue(types.fundamental(FundamentalType::Int), result ? 1 : 0);
		return true;
	}
	// GCC's floating classifications of a constant operand (a
	// signaling NaN, whose bits an evaluation in long double would
	// quiet, is no constant here).
	ConstValue x;
	if (which == Intrinsic::NanSignaling || !operand(*arguments[arguments.size - 1], x) ||
		x.kind != ValueKind::Floating)
		return false;
	const long double v = x.floating;
	const FundamentalType representation = FloatingRepresentation(x.type->fundamental);
	// The least normal value of the operand's format, exactly: 2^(1 -
	// bias) (a decimal approximation in long double is not the double's).
	const long double least = std::ldexp(1.0L,
		representation == FundamentalType::Float || representation == FundamentalType::BFloat16 ? -126
			: representation == FundamentalType::Double                                         ? -1022
			: representation == FundamentalType::Float16                                        ? -14
																								: -16382);
	const bool nan = std::isnan(v), infinite = std::isinf(v);
	const bool normal = !nan && !infinite && (v >= least || v <= -least);
	uint64_t result = 0;
	switch (which)
	{
	case Intrinsic::SignBit:
		result = std::signbit(v) ? 1 : 0;
		break;
	case Intrinsic::IsNan:
		result = nan;
		break;
	case Intrinsic::IsInf:
		result = infinite;
		break;
	case Intrinsic::IsSignaling:
	{
		// The x87 value's quiet bit (significand bit 62), which a
		// signaling NaN constant keeps (BuiltinSignalingNan).
		unsigned char bytes[sizeof(long double)] = {};
		std::memcpy(bytes, &v, sizeof v);
		uint64_t significand = 0;
		std::memcpy(&significand, bytes, 8);
		result = nan && ((significand >> 62) & 1) == 0;
		break;
	}
	case Intrinsic::IsInfSign:
		result = !infinite    ? 0
			: std::signbit(v) ? ConvertIntegral(~uint64_t(0), KindOf(types.fundamental(FundamentalType::Int)))
							  : 1;
		break;
	case Intrinsic::IsFinite:
		result = !nan && !infinite;
		break;
	case Intrinsic::IsNormal:
		result = normal;
		break;
	case Intrinsic::FpClassify:
	{
		const uint32_t category = nan ? 0 : infinite ? 1 : normal ? 2 : v != 0 ? 3 : 4;
		ConstValue code;
		if (!operand(*arguments[category], code) || code.kind != ValueKind::Integer)
			return false;
		result = code.bits;
		break;
	}
	default:
		return false;
	}
	out = IntegerValue(types.fundamental(FundamentalType::Int), result);
	return true;
}

bool ConstantInterpreter::bit_builtin(const syntax::CallExpr& expr, ConstValue& out)
{
	// The operand converted to the builtin's unsigned type (16, 32 or 64
	// bits); a byte swap is of that type, a count an int. GCC leaves clz
	// and ctz of 0 undefined: no constant.
	Intrinsic which = static_cast<Intrinsic>(expr.facts->value & 0xff);
	ConstValue x;
	if (!operand(*expr.arguments[0], x) || x.kind != ValueKind::Integer)
		return false;
	const unsigned width = KindOf(x.type).bits;
	if (width == 128)
	{
		// A generic count of an unsigned __int128: its halves'.
		const WideBits wide = WideOf(x);
		const uint64_t high = static_cast<uint64_t>(wide >> 64), low = static_cast<uint64_t>(wide);
		uint64_t result = 0;
		if (wide == 0 && which != Intrinsic::GenericPopCount)
		{
			ConstValue zero;
			if (expr.arguments.size != 2)
				return fail(expr.location, "__builtin_clzg or __builtin_ctzg of zero");
			if (!operand(*expr.arguments[1], zero) || zero.kind != ValueKind::Integer)
				return false;
			result = zero.bits;
		}
		else if (which == Intrinsic::GenericPopCount)
			result = static_cast<uint64_t>(__builtin_popcountll(high) + __builtin_popcountll(low));
		else if (which == Intrinsic::GenericCountLeadingZeros)
			result = static_cast<uint64_t>(high != 0 ? __builtin_clzll(high) : 64 + __builtin_clzll(low));
		else
			result = static_cast<uint64_t>(low != 0 ? __builtin_ctzll(low) : 64 + __builtin_ctzll(high));
		out = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Int), result);
		return true;
	}
	const uint64_t v = width >= 64 ? x.bits : x.bits & ((uint64_t(1) << width) - 1);
	uint64_t result = 0;
	// The generic forms count in the operand's own type; clzg and ctzg of
	// 0 give their second operand.
	if (which >= Intrinsic::GenericCountLeadingZeros)
	{
		which = static_cast<Intrinsic>(static_cast<uint8_t>(which) -
			static_cast<uint8_t>(Intrinsic::GenericCountLeadingZeros) +
			static_cast<uint8_t>(Intrinsic::CountLeadingZeros));
		if (v == 0 && which != Intrinsic::PopCount && expr.arguments.size == 2)
		{
			ConstValue zero;
			if (!operand(*expr.arguments[1], zero) || zero.kind != ValueKind::Integer)
				return false;
			out = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Int), zero.bits);
			return true;
		}
	}
	if (which == Intrinsic::ByteSwap)
	{
		for (unsigned i = 0; i < width; i += 8)
			result |= ((v >> i) & 0xff) << (width - 8 - i);
		out = IntegerValue(expr.facts->type->unqualified, result);
		return true;
	}
	if (which != Intrinsic::PopCount && v == 0)
		return fail(expr.location, "__builtin_clz or __builtin_ctz of zero");
	if (which == Intrinsic::PopCount)
	{
		for (unsigned i = 0; i < width; ++i)
			result += (v >> i) & 1;
	}
	else if (which == Intrinsic::CountLeadingZeros)
	{
		while (((v >> (width - 1 - result)) & 1) == 0)
			++result;
	}
	else
	{
		while (((v >> result) & 1) == 0)
			++result;
	}
	out = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Int), result);
	return true;
}

namespace {

// An integer as a sign and a 256-bit magnitude: any 128-bit operand, and
// the exact sum, difference or product of two.
struct ExactInteger
{
	bool negative;
	WideBits low;
	WideBits high;
};

ExactInteger ExactOf(const ConstValue& value, IntegralKind kind)
{
	// A narrower type's bits are its value sign-extended (or not) to 64.
	const WideBits bits = kind.bits > 64 ? WideOf(value)
		: kind.is_signed ? static_cast<WideBits>(static_cast<__int128>(static_cast<int64_t>(value.bits)))
						 : value.bits;
	const bool negative = kind.is_signed && (bits >> 127) != 0;
	return ExactInteger{ negative, negative ? 0 - bits : bits, 0 };
}

bool MagnitudeLess(const ExactInteger& a, const ExactInteger& b)
{
	return a.high != b.high ? a.high < b.high : a.low < b.low;
}

// The exact a + b (b's sign flipped for a difference).
ExactInteger ExactSum(const ExactInteger& a, const ExactInteger& b)
{
	if (a.negative == b.negative)
	{
		const WideBits low = a.low + b.low;
		return ExactInteger{ a.negative, low, a.high + b.high + (low < a.low ? 1 : 0) };
	}
	const bool swap = MagnitudeLess(a, b);
	const ExactInteger& larger = swap ? b : a;
	const ExactInteger& smaller = swap ? a : b;
	return ExactInteger{ larger.negative, larger.low - smaller.low,
		larger.high - smaller.high - (larger.low < smaller.low ? 1 : 0) };
}

// The exact product of two 128-bit magnitudes, by 64-bit limbs.
ExactInteger ExactProduct(const ExactInteger& a, const ExactInteger& b)
{
	const uint64_t x[2] = { static_cast<uint64_t>(a.low), static_cast<uint64_t>(a.low >> 64) };
	const uint64_t y[2] = { static_cast<uint64_t>(b.low), static_cast<uint64_t>(b.low >> 64) };
	uint64_t limbs[4] = { 0, 0, 0, 0 };
	for (unsigned i = 0; i < 2; ++i)
	{
		WideBits carry = 0;
		for (unsigned j = 0; j < 2; ++j)
		{
			const WideBits t = static_cast<WideBits>(x[i]) * y[j] + limbs[i + j] + carry;
			limbs[i + j] = static_cast<uint64_t>(t);
			carry = t >> 64;
		}
		limbs[i + 2] = static_cast<uint64_t>(carry);
	}
	return ExactInteger{ a.negative != b.negative, (static_cast<WideBits>(limbs[1]) << 64) | limbs[0],
		(static_cast<WideBits>(limbs[3]) << 64) | limbs[2] };
}

} // namespace

bool ConstantInterpreter::overflow_builtin(const syntax::CallExpr& expr, ConstValue& out)
{
	// The infinitely precise a op b (of the promoted operands' types,
	// either signedness, up to 128 bits), stored through r modulo 2^N of
	// r's type (4.7p2 as GCC converts); the result whether that changed
	// it.
	const Intrinsic which = static_cast<Intrinsic>(expr.facts->value & 0xff);
	ConstValue a, b, pointer;
	if (!operand(*expr.arguments[0], a) || !operand(*expr.arguments[1], b) || !operand(*expr.arguments[2], pointer))
		return false;
	if (a.kind != ValueKind::Integer || b.kind != ValueKind::Integer)
		return false;
	if (pointer.kind != ValueKind::Pointer || pointer.root == RootKind::Null)
		return fail(expr.location, "an overflow builtin's result through a null pointer");
	const Type* result_type = pointer.type->unqualified->element;
	IntegralKind left_kind, right_kind, result_kind;
	if (!IntegralKindOf(a.type, left_kind) || !IntegralKindOf(b.type, right_kind) ||
		!IntegralKindOf(result_type, result_kind))
		return false;
	ExactInteger x = ExactOf(a, left_kind), y = ExactOf(b, right_kind);
	if (which == Intrinsic::SubOverflow)
		y.negative = !y.negative;
	const ExactInteger exact = which == Intrinsic::MulOverflow ? ExactProduct(x, y) : ExactSum(x, y);
	// Representable: a magnitude below 2^(N-1) (2^N unsigned), or 2^(N-1)
	// itself when negative and signed; no negative value unsigned.
	const unsigned bits = result_kind.bits - (result_kind.is_signed ? 1 : 0);
	const WideBits limit = bits >= 128 ? 0 : static_cast<WideBits>(1) << bits;
	const bool zero = exact.low == 0 && exact.high == 0;
	const bool fits = exact.high == 0 && (!exact.negative || result_kind.is_signed || zero) &&
		(limit == 0 || exact.low < limit || (exact.negative && result_kind.is_signed && exact.low == limit));
	const WideBits wrapped = exact.negative ? 0 - exact.low : exact.low;
	const ConstValue stored = result_kind.bits > 64
		? WideValue(result_type, wrapped)
		: IntegerValue(result_type, ConvertIntegral(static_cast<uint64_t>(wrapped), result_kind));
	if (!write(PlaceOf(pointer, result_type), stored, expr.location))
		return false;
	out = IntegerValue(analyzer_.types_.fundamental(FundamentalType::Bool), fits ? 0 : 1);
	return true;
}

bool ConstantInterpreter::source_location(const syntax::CallExpr& expr, ConstValue& out)
{
	// The outermost default argument's (or member initializer's) use, or
	// the call's own position; the name `__func__` holds in the function
	// evaluating it (the frame's, else the body being analyzed; none at
	// namespace scope), a string literal's array.
	const Intrinsic which = static_cast<Intrinsic>(expr.facts->value & 0xff);
	const uint32_t location = source_site_ != kNoSourceSite ? source_site_ : expr.location;
	if (which == Intrinsic::SourceLine || which == Intrinsic::SourceColumn)
	{
		out = IntegerValue(expr.facts->type, which == Intrinsic::SourceLine ? analyzer_.source_line(location) : 0);
		return true;
	}
	std::string text;
	if (which == Intrinsic::SourceFile)
		text = analyzer_.source_file(location);
	else
	{
		const FunctionEntity* function = frames_.size() > first_frame_ ? frames_.back().function
			: analyzer_.body_ != nullptr                               ? analyzer_.body_->function
																	   : nullptr;
		if (function != nullptr)
			text = PredefinedFunctionName(function);
	}
	const syntax::LiteralExpr* literal = analyzer_.text_literal(text);
	if (literal == nullptr)
		return fail(expr.location, "a source location builtin's string");
	ConstPlace place = ConstPlace();
	place.root = RootKind::Literal;
	place.entity = literal;
	place.type = literal->facts->type->element;
	out = PointerTo(place, expr.facts->type);
	return true;
}

bool ConstantInterpreter::conditional_value(const syntax::ConditionalExpr& expr, ConstValue& out)
{
	// 5.16: the selected operand only, converted to the common type as
	// recorded on it.
	bool selected = false;
	if (!truth(*expr.condition, selected))
		return false;
	const syntax::Expr& chosen = selected ? *expr.then_expr : *expr.else_expr;
	if (IsVoid(expr.facts->type))
	{
		out = ConstValue();
		out.type = expr.facts->type;
		return discard(chosen);
	}
	return operand(chosen, out) && convert_scalar(out, expr.facts->type, expr.location);
}

bool ConstantInterpreter::cast(const syntax::Expr& expr, const syntax::Expr& operand_expr, ConstValue& out)
{
	// 5.2.9, 5.2.11, 5.4 between scalars (reinterpret_cast is no constant
	// expression, 5.19p2).
	const Type* type = expr.facts->type;
	if (expr.kind == syntax::ExprKind::Cast)
	{
		const TokenKind form = static_cast<const syntax::CastExpr&>(expr).cast;
		if (form == TokenKind::KW_REINTERPET_CAST || form == TokenKind::KW_DYNAMIC_CAST)
			return fail(expr.location, "a reinterpret_cast or dynamic_cast in a constant expression");
	}
	if (IsVoid(type))
	{
		out = ConstValue();
		out.type = type;
		return discard(operand_expr);
	}
	const ExprFacts* facts = WithoutParentheses(&operand_expr)->facts;
	const Conversion* conversion = facts->conversion;
	if (ConversionFunction(conversion))
		return converted(operand_expr, conversion, out) && convert_scalar(out, type, expr.location);
	ConstValue value;
	if (!rvalue(operand_expr, value))
		return false;
	if ((expr.facts->flags & kBaseCast) != 0 && value.kind == ValueKind::MemberPointer)
	{
		// 5.2.9p12: to a pointer to member of a base class.
		if (!member_adjusted(value, type, true, expr.location))
			return false;
		out = value;
		return true;
	}
	if (conversion != nullptr &&
		(conversion->second == ValueConversion::NullPointer || conversion->second == ValueConversion::DerivedToBase ||
			conversion->second == ValueConversion::MemberToDerived))
	{
		if (!apply_standard(operand_expr, conversion, value) || !convert_scalar(value, type, expr.location))
			return false;
		out = value;
		return true;
	}
	if ((expr.facts->flags & (kDerivedCast | kBaseCast)) != 0 && value.kind == ValueKind::Pointer && IsPointer(type))
	{
		if (value.root != RootKind::Null)
		{
			ConstPlace place = PlaceOf(value, value.type->element);
			const ClassEntity* to = ClassOf(type->unqualified->element);
			const bool derived = (expr.facts->flags & kDerivedCast) != 0;
			if (!(derived ? to_derived(place, to, type->unqualified->element, expr.location)
						  : to_base(place, to, expr.location)))
				return false;
			value = PointerTo(place, type->unqualified);
		}
		value.type = type->unqualified;
		out = value;
		return true;
	}
	// 5.19p2: a conversion from cv void* to a pointer to an object type,
	// even of a null pointer.
	if (value.kind == ValueKind::Pointer && IsPointer(type) && IsVoid(value.type->element) &&
		!IsVoid(type->unqualified->element) && type->unqualified->element->kind != TypeKind::Function)
		return fail(expr.location, "a cast from void* in a constant expression");
	if (value.kind == ValueKind::Pointer && IsPointer(type) && value.root != RootKind::Null &&
		!IsVoid(type->unqualified->element) &&
		analyzer_.conversions_.without_cv(type->unqualified->element) !=
			analyzer_.conversions_.without_cv(value.type->element))
		return fail(expr.location, "a pointer cast between unrelated types in a constant expression");
	if (!convert_scalar(value, type, expr.location))
		return false;
	out = value;
	return true;
}

bool ConstantInterpreter::subscript(const syntax::SubscriptExpr& expr, ConstPlace& out)
{
	// 5.2.1: *(E1 + E2).
	if ((expr.facts->flags & kOperatorCall) != 0)
		return call_place(expr, out);
	if (WithoutParentheses(expr.base)->facts->type->unqualified->kind == TypeKind::Vector)
		return vector_lane(expr, &out, nullptr);
	const bool commuted = (expr.facts->flags & kCommuted) != 0;
	const syntax::Expr& base = commuted ? *expr.index : *expr.base;
	const syntax::Expr& index = commuted ? *expr.base : *expr.index;
	ConstValue pointer, position;
	const syntax::Expr* array = WithoutParentheses(&base);
	if (array->facts->category == ValueCategory::PRValue && array->facts->type->kind == TypeKind::Array)
	{
		// An array prvalue's element (`A{1, 2}[1]`): of its temporary
		// (12.2p1, CWG 1213).
		ConstPlace place;
		if (!temporary(*array, place))
			return false;
		place.type = array->facts->type->element;
		pointer = PointerTo(place, Decayed(analyzer_.types_, array->facts->type));
	}
	else if (!rvalue(base, pointer))
		return false;
	if (!operand(index, position))
		return false;
	if (pointer.kind != ValueKind::Pointer || position.kind != ValueKind::Integer)
		return fail(expr.location, "a subscript outside constant evaluation");
	const int64_t steps = KindOf(position.type).is_signed ? static_cast<int64_t>(position.bits)
														  : static_cast<int64_t>(position.bits);
	if (pointer.root == RootKind::Null)
		return fail(expr.location, "a subscript of a null pointer");
	if (!advance(pointer, steps, expr.location))
		return false;
	out = PlaceOf(pointer, expr.facts->type);
	return true;
}

bool ConstantInterpreter::vector_lane(const syntax::SubscriptExpr& expr, ConstPlace* place, ConstValue* value)
{
	// A GNU vector's lane (GCC's subscript of a vector): of a vector
	// lvalue its place, of a prvalue its value; a lane outside the vector
	// is no constant expression.
	const syntax::Expr& base = *expr.base;
	const Type* vector = WithoutParentheses(&base)->facts->type;
	ConstValue position;
	ConstPlace object;
	ConstValue whole;
	if (!LaneVector(vector))
		return fail(expr.location, "a lane of a vector of packed lanes in a constant expression");
	if (place != nullptr ? !glvalue(base, object) : !rvalue(base, whole))
		return false;
	if (!operand(*expr.index, position))
		return false;
	const uint64_t bound = vector->unqualified->bound;
	const bool negative = position.kind == ValueKind::Integer && KindOf(position.type).is_signed &&
		static_cast<int64_t>(position.bits) < 0;
	if (position.kind != ValueKind::Integer || negative || position.bits >= bound)
		return fail(expr.location, "a vector lane outside the vector");
	if (place != nullptr)
	{
		object.offset += static_cast<int64_t>(position.bits * size_of(vector->unqualified->element, expr.location));
		object.type = expr.facts->type;
		*place = object;
		return true;
	}
	if (whole.kind != ValueKind::Array || whole.count != bound)
		return fail(expr.location, "a vector lane outside constant evaluation");
	*value = whole.elements[position.bits];
	return true;
}

bool ConstantInterpreter::member_object(const syntax::MemberExpr& expr)
{
	// 5.2.5p1: the object expression is evaluated even when the member is
	// static or an enumerator (9.4p2).
	const syntax::Expr* base = WithoutParentheses(expr.base);
	if (base->facts == nullptr)
		return true;
	if (expr.op == TokenKind::OP_ARROW)
	{
		ConstValue pointer;
		return rvalue(*base, pointer);
	}
	ConstPlace place;
	return object_place(*base, place);
}

bool ConstantInterpreter::member(const syntax::MemberExpr& expr, ConstPlace& out)
{
	// 5.2.5: the member of the object (E1->E2 is (*E1).E2); a static member
	// or member function is designated after the object's evaluation.
	const Entity* entity = expr.facts->entity;
	if (entity == nullptr)
		return fail(expr.location, "a member access outside constant evaluation");
	const bool data_member = entity->kind == EntityKind::Variable && static_cast<const VariableEntity*>(entity)->member;
	if (!data_member)
		return member_object(expr) && entity_place(entity, expr.facts->type, out, expr.location);
	ConstPlace object;
	if (expr.op == TokenKind::OP_ARROW)
	{
		ConstValue pointer;
		if (!rvalue(*expr.base, pointer))
			return false;
		if (pointer.kind != ValueKind::Pointer || pointer.root == RootKind::Null)
			return fail(expr.location, "a member access through a null pointer");
		object = PlaceOf(pointer, pointer.type->element);
	}
	else if (!object_place(*expr.base, object))
		return false;
	return through_naming(object, analyzer_.member_naming_class(expr), expr.location) &&
		member_place(object, static_cast<const VariableEntity*>(entity), out, expr.location);
}

bool ConstantInterpreter::through_naming(ConstPlace& object, const ClassEntity* naming, uint32_t location)
{
	// 11.2p6: a qualified name's naming class (recorded when it is neither
	// the object's class nor the member's) selects a repeated base's
	// subobject: the object converts to it first.
	return naming == nullptr || ClassOf(object.type) == naming || to_base(object, naming, location);
}

bool ConstantInterpreter::reference_cast(const syntax::Expr& expr, const syntax::Expr& operand_expr, ConstPlace& out)
{
	// 5.2.9p2, 5.2.11, 5.4: a cast to a reference type designates the
	// operand's object (a base subobject, or the derived object holding
	// it), or a temporary its binding makes.
	const ExprFacts* facts = expr.facts;
	if (static_cast<const syntax::CastExpr&>(expr).cast == TokenKind::KW_REINTERPET_CAST ||
		static_cast<const syntax::CastExpr&>(expr).cast == TokenKind::KW_DYNAMIC_CAST)
		return fail(expr.location, "a reinterpret_cast or dynamic_cast in a constant expression");
	const ExprFacts* operand_facts = WithoutParentheses(&operand_expr)->facts;
	const Conversion* binding = operand_facts->conversion;
	if ((facts->flags & kReferenceCast) != 0 && binding != nullptr && binding->form == ConversionForm::Reference &&
		(binding->constructor != nullptr || binding->temporary || binding->second == ValueConversion::DerivedToBase))
		return bind(operand_expr, binding, facts->target, out);
	if (!object_place(operand_expr, out))
		return false;
	if ((facts->flags & kBaseCast) != 0)
	{
		if (!to_base(out, ClassOf(facts->type), expr.location))
			return false;
		out.type = facts->type;
		return true;
	}
	if ((facts->flags & kDerivedCast) != 0)
		return to_derived(out, ClassOf(facts->type), facts->type, expr.location);
	out.type = facts->type;
	return true;
}

bool ConstantInterpreter::binds_temporary(const syntax::Expr& expr, const Conversion* conversion)
{
	// Whether the object a reference is bound to is (a subobject of) a
	// temporary of the initializer (8.5.3p5, 12.2p5): a conversion
	// function's or converting constructor's class result, a converted
	// value, a prvalue, or a member or element of a class or array prvalue.
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (ConversionFunction(conversion))
		return !conversion->constructor->type->canonical->element->is_reference();
	if (conversion != nullptr &&
		(conversion->constructor != nullptr ||
			(conversion->form == ConversionForm::Reference && conversion->temporary)))
		return true;
	for (uint32_t depth = 0; e->facts != nullptr && depth < 256; ++depth)
	{
		const ExprFacts* facts = e->facts;
		if (facts->category == ValueCategory::PRValue)
			return facts->type->kind != TypeKind::Function && (facts->flags & kUserConverted) == 0;
		if (e->kind == syntax::ExprKind::Member && static_cast<const syntax::MemberExpr*>(e)->op == TokenKind::OP_DOT &&
			facts->entity != nullptr && facts->entity->kind == EntityKind::Variable &&
			static_cast<const VariableEntity*>(facts->entity)->member && !facts->entity->type->is_reference())
			e = WithoutParentheses(static_cast<const syntax::MemberExpr*>(e)->base);
		else if (e->kind == syntax::ExprKind::Subscript && (facts->flags & (kOperatorCall | kCommuted)) == 0 &&
			WithoutParentheses(static_cast<const syntax::SubscriptExpr*>(e)->base)->facts != nullptr &&
			WithoutParentheses(static_cast<const syntax::SubscriptExpr*>(e)->base)->facts->type->kind ==
				TypeKind::Array)
			e = WithoutParentheses(static_cast<const syntax::SubscriptExpr*>(e)->base);
		else
			return false;
	}
	return false;
}

bool ConstantInterpreter::bind(
	const syntax::Expr& expr, const Conversion* conversion, const Type* reference, ConstPlace& out)
{
	// 8.5.3: a reference binds to the glvalue itself (its base class
	// subobject), or to a temporary holding the converted value (a class
	// prvalue's own).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return fail(e->location, "an expression outside constant evaluation");
	const Type* referenced = reference->is_reference() ? reference->element : reference;
	if (ConversionFunction(conversion))
	{
		// 13.3.1.6: a conversion function's reference result, else a
		// temporary its result initializes.
		const FunctionEntity* function = conversion->constructor;
		const Type* returned = function->type->canonical->element;
		Result result = Result();
		if (returned->is_reference())
		{
			if (!conversion_call(*e, facts, function, result))
				return false;
			out = result.place;
			if (conversion->after != nullptr && conversion->after->second == ValueConversion::DerivedToBase)
				return to_base(out, ClassOf(referenced), e->location);
			return true;
		}
		if (!new_object(ClassOf(returned) != nullptr ? returned : referenced, out, e->location))
			return false;
		if (ClassOf(returned) != nullptr)
		{
			result.target = &out;
			if (!conversion_call(*e, facts, function, result))
				return false;
			if (conversion->after != nullptr && conversion->after->second == ValueConversion::DerivedToBase)
				return to_base(out, ClassOf(referenced), e->location);
			return true;
		}
		if (!conversion_call(*e, facts, function, result) || !convert_scalar(result.value, referenced, e->location))
			return false;
		return write(out, result.value, e->location);
	}
	if (conversion != nullptr && conversion->constructor != nullptr)
	{
		// A temporary of the referenced class made by the converting
		// constructor (8.5.3p5, 13.3.3.1.2).
		if (!new_object(conversion->result, out, e->location))
			return false;
		Argument argument = Argument();
		argument.expr = &expr;
		argument.conversion = conversion->argument;
		return run_constructor(conversion->constructor, out, &argument, 1, e->location);
	}
	if (conversion != nullptr && conversion->form == ConversionForm::Reference && conversion->temporary)
	{
		const Type* object = conversion->result != nullptr ? conversion->result : referenced;
		ConstValue value;
		if (!rvalue(*e, value) || !convert_scalar(value, object, e->location) || !new_object(object, out, e->location))
			return false;
		return write(out, value, e->location);
	}
	if (facts->category == ValueCategory::PRValue && ClassOf(facts->type) != nullptr)
	{
		// 8.5.3p5: a class prvalue binds directly, materialized (12.2p1).
		if (!temporary(*e, out))
			return false;
		if (conversion != nullptr && conversion->second == ValueConversion::DerivedToBase)
			return to_base(out, ClassOf(referenced), e->location);
		return true;
	}
	if (facts->category == ValueCategory::PRValue && facts->type->kind != TypeKind::Function)
	{
		ConstValue value;
		if (!rvalue(*e, value) || !convert_scalar(value, referenced, e->location) ||
			!new_object(referenced, out, e->location))
			return false;
		return write(out, value, e->location);
	}
	if (!glvalue(*e, out))
		return false;
	if (conversion != nullptr && conversion->second == ValueConversion::DerivedToBase)
		return to_base(out, ClassOf(referenced), e->location);
	return true;
}

} // namespace sema
