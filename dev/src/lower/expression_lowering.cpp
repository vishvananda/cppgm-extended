#include "lower/function_lowering.h"

#include <limits>
#include <string>

#include "lex/identifier_table.h"
#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "lowir/literals.h"
#include "sema/aggregates.h"
#include "sema/constant_values.h"
#include "sema/conversions.h"
#include "sema/integral_arithmetic.h"
#include "sema/string_literals.h"
#include "sema/template_records.h"

// Expressions (clause 5) from their recorded facts. A glvalue lowers to its
// storage; a prvalue to an operand and its LowIR type. The conversion an
// operand's context recorded is applied after the operand is evaluated,
// and the operands of a built-in operator are all evaluated before any is
// converted (the course's evaluation-order convention).
namespace lower {

namespace {

// A pointer that cannot be null: `this` (9.3.2) or an object's address
// (5.3.1p3); its conversion to a base needs no null test.
bool NonNullPointer(const syntax::Expr& expr)
{
	const syntax::Expr* e = &expr;
	while (e->kind == syntax::ExprKind::Paren)
		e = static_cast<const syntax::ParenExpr*>(e)->inner;
	if (e->kind == syntax::ExprKind::KeywordLiteral)
		return static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword == TokenKind::KW_THIS;
	return e->kind == syntax::ExprKind::Unary && static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_AMP &&
		(e->facts->flags & sema::kOperatorCall) == 0;
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool IsConditional(const syntax::Expr& expr)
{
	return WithoutParentheses(&expr)->kind == syntax::ExprKind::Conditional;
}

// A reference cast's operand that the result designates (or a base
// subobject of it, 8.5.3p5), not a temporary the cast's `T t(e);` makes.
bool BindsDirectly(const syntax::Expr* operand)
{
	const sema::Conversion* conversion = operand->facts->conversion;
	return conversion == nullptr ||
		(conversion->form == sema::ConversionForm::Reference && !conversion->temporary &&
			conversion->constructor == nullptr);
}

// The operand of a cast already classified by semantic analysis, including
// functional syntax parsed as a call before its type name was known.
const syntax::Expr* ReferenceCastOperand(const syntax::Expr* expr)
{
	if (expr->kind == syntax::ExprKind::Cast)
		return static_cast<const syntax::CastExpr*>(expr)->operand;
	if (expr->kind == syntax::ExprKind::FunctionalCast)
	{
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(expr);
		return cast->arguments.size == 1 ? cast->arguments[0] : nullptr;
	}
	if (expr->kind == syntax::ExprKind::Call)
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(expr);
		return call->callee->facts == nullptr && call->arguments.size == 1 ? call->arguments[0] : nullptr;
	}
	return nullptr;
}

// A cast to a reference type whose result is the temporary its operand's
// binding makes (5.2.9p4, 5.4p4: `T t(e);` through a converting
// constructor, a conversion function or a converted scalar, 8.5.3p5): the
// operand, else null.
const syntax::Expr* CastTemporaryOperand(const syntax::Expr* e)
{
	if ((e->facts->flags & (sema::kReferenceCast | sema::kRuntimeCheck)) != sema::kReferenceCast)
		return nullptr;
	const syntax::Expr* operand = ReferenceCastOperand(e);
	if (operand == nullptr)
		return nullptr;
	const sema::Conversion* binding = WithoutParentheses(operand)->facts->conversion;
	return binding != nullptr && binding->form == sema::ConversionForm::Reference &&
			(binding->constructor != nullptr || binding->temporary)
		? operand
		: nullptr;
}

// The operand of a built-in subscript that is an array (not a pointer):
// `E1[E2]` is `*((E1)+(E2))` (5.2.1p1), either may be; or null.
const syntax::Expr* ArrayOperand(const syntax::SubscriptExpr* expr)
{
	const syntax::Expr* const operands[2] = { expr->base, expr->index };
	for (const syntax::Expr* operand : operands)
	{
		const sema::ExprFacts* facts = WithoutParentheses(operand)->facts;
		if (facts != nullptr && facts->type != nullptr && facts->type->kind == sema::TypeKind::Array)
			return operand;
	}
	return nullptr;
}

bool IsPointerType(const sema::Type* type)
{
	return type->unqualified->kind == sema::TypeKind::Pointer;
}

bool IsNullptrType(const sema::Type* type)
{
	type = type->unqualified;
	return type->kind == sema::TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

bool IsBoolType(const sema::Type* type)
{
	type = type->unqualified;
	return type->kind == sema::TypeKind::Fundamental && type->fundamental == FundamentalType::Bool;
}

bool IsSigned(const sema::Type* type)
{
	sema::IntegralKind kind;
	return sema::IntegralKindOf(type, kind) && kind.is_signed && kind.bits > 1;
}

bool IsComparisonOperator(TokenKind op)
{
	return op == TokenKind::OP_EQ || op == TokenKind::OP_NE || op == TokenKind::OP_LT || op == TokenKind::OP_GT ||
		op == TokenKind::OP_LE || op == TokenKind::OP_GE;
}

// Operators whose left operand is read as a value: a left-nested chain of
// them is lowered iteratively.
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
		return IsComparisonOperator(op);
	}
}

// The type an operand's own evaluation yields as a prvalue: arrays and
// functions decay (4.2, 4.3), glvalues lose their cv-qualifiers (4.1).
const sema::Type* DecayedType(sema::TypeTable& types, const sema::Type* type)
{
	if (type->kind == sema::TypeKind::Array)
		return types.pointer(type->element);
	if (type->kind == sema::TypeKind::Function)
		return types.pointer(type);
	return type->unqualified;
}

// An integer, character or boolean literal as written (parenthesized or
// not), or a static data member's constant named (9.4.2p3, read as its
// value): the operands whose widening the course conventions fold.
bool IsLiteralExpression(const syntax::Expr& expr)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	if ((e->kind == syntax::ExprKind::Id || e->kind == syntax::ExprKind::Member) && e->facts != nullptr &&
		(e->facts->flags & sema::kStaticConstant) != 0)
		return e->facts->type != nullptr && e->facts->type->kind == sema::TypeKind::Fundamental &&
			!sema::IsFloating(e->facts->type);
	if (e->kind == syntax::ExprKind::KeywordLiteral)
	{
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword;
		return keyword == TokenKind::KW_TRUE || keyword == TokenKind::KW_FALSE;
	}
	return e->kind == syntax::ExprKind::Literal && e->facts != nullptr && e->facts->type != nullptr &&
		e->facts->type->kind == sema::TypeKind::Fundamental && !sema::IsFloating(e->facts->type);
}

// The comparison of two operands of the common type: its LowIR type, which
// an operand read in another type of the same width (a bit-field's unit)
// does not change.
// An operand's type after the conversion its context recorded (its
// decayed type when none was).
const sema::Type* ConvertedType(sema::TypeTable& types, const sema::ExprFacts* facts)
{
	const sema::Conversion* conversion = facts->conversion;
	if (conversion != nullptr && conversion->constructor == nullptr &&
		conversion->form != sema::ConversionForm::Reference && conversion->result != nullptr)
		return conversion->result->unqualified;
	return DecayedType(types, facts->type)->unqualified;
}

lowir::Type ComparisonType(lowir::Type operand, lowir::Type common)
{
	if (operand.is_integer() && common.is_integer() &&
		lowir::integer_bits(operand.kind()) == lowir::integer_bits(common.kind()))
		return common;
	return operand;
}

// sizeof or alignof as written (not of a class template specialization,
// see sema::kSpecializationQuery).
bool IsSizeQuery(const syntax::Expr& expr)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->facts != nullptr && (e->facts->flags & sema::kSpecializationQuery) != 0)
		return false;
	return e->kind == syntax::ExprKind::Sizeof || e->kind == syntax::ExprKind::SizeofPack ||
		(e->kind == syntax::ExprKind::TypeTrait &&
			static_cast<const syntax::TypeTraitExpr*>(e)->keyword == TokenKind::KW_ALIGNOF);
}

TokenKind CompoundOperator(TokenKind op)
{
	switch (op)
	{
	case TokenKind::OP_PLUSASS:
		return TokenKind::OP_PLUS;
	case TokenKind::OP_MINUSASS:
		return TokenKind::OP_MINUS;
	case TokenKind::OP_STARASS:
		return TokenKind::OP_STAR;
	case TokenKind::OP_DIVASS:
		return TokenKind::OP_DIV;
	case TokenKind::OP_MODASS:
		return TokenKind::OP_MOD;
	case TokenKind::OP_XORASS:
		return TokenKind::OP_XOR;
	case TokenKind::OP_BANDASS:
		return TokenKind::OP_AMP;
	case TokenKind::OP_BORASS:
		return TokenKind::OP_BOR;
	case TokenKind::OP_LSHIFTASS:
		return TokenKind::OP_LSHIFT;
	case TokenKind::OP_RSHIFTASS:
		return TokenKind::OP_RSHIFT;
	default:
		return op;
	}
}

} // namespace

// ------------------------------------------------------------- operands

// Whether computing a glvalue's address again (`read`: its value) repeats
// no side effect: names, `this`, literals, member accesses and subscripts
// of such, reading nothing volatile.
bool FunctionLowering::side_effect_free(const syntax::Expr* expr, bool read, uint32_t depth)
{
	const syntax::Expr* e = WithoutParentheses(expr);
	if (depth > 64 || e->facts == nullptr || e->facts->type == nullptr ||
		(read && (e->facts->type->cv & sema::kVolatile) != 0))
		return false;
	switch (e->kind)
	{
	case syntax::ExprKind::Id:
	case syntax::ExprKind::Literal:
	case syntax::ExprKind::KeywordLiteral:
		return true;
	case syntax::ExprKind::Member:
	{
		const syntax::MemberExpr* member = static_cast<const syntax::MemberExpr*>(e);
		return side_effect_free(member->base, member->op == TokenKind::OP_ARROW, depth + 1);
	}
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr* subscript = static_cast<const syntax::SubscriptExpr*>(e);
		const syntax::Expr* base = WithoutParentheses(subscript->base);
		const bool array = base->facts != nullptr && base->facts->type != nullptr &&
			base->facts->type->kind == sema::TypeKind::Array;
		return side_effect_free(base, !array, depth + 1) && side_effect_free(subscript->index, true, depth + 1);
	}
	default:
		return false;
	}
}

void FunctionLowering::member_object(const syntax::MemberExpr& expr)
{
	// 5.2.5p1: the object expression of a static member's or member
	// enumerator's access is evaluated for its effects (9.4p2), unless it
	// has none.
	if (!side_effect_free(expr.base, expr.op == TokenKind::OP_ARROW, 0))
		discard(*expr.base);
}

RValue FunctionLowering::rvalue(const syntax::Expr& expr)
{
	// The prvalue the expression's own evaluation yields: a glvalue read
	// (4.1) or decayed (4.2, 4.3), a prvalue computed.
	const syntax::Expr* e = WithoutParentheses(&expr);
	check_stack(e->location);
	const DebugScope debug(*this, e->location);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		throw LoweringError(e->location, "an expression without semantic facts");
	if ((facts->flags & sema::kUserConverted) != 0)
		return converted_operand(*e);
	if ((facts->flags & (sema::kFolded | sema::kStaticConstant)) != 0 && e->kind == syntax::ExprKind::Member)
		member_object(static_cast<const syntax::MemberExpr&>(*e));
	// A folded value-initialized pointer to member (5.2.3p2) is its null
	// value.
	if ((facts->flags & sema::kFolded) != 0)
		return facts->type->unqualified->kind == sema::TypeKind::MemberPointer ? zero_value(facts->type)
																			   : integer(facts->type, facts->value);
	if ((facts->flags & sema::kStaticConstant) != 0)
	{
		// 9.4.2p3: the member's constant value (a member access's object
		// expression names no object that needs evaluating).
		return integer(facts->type, facts->value);
	}
	// A constexpr 128-bit integer whose value no folded constant holds:
	// read as its evaluated value too (no odr-use, 3.2p2).
	const sema::VariableEntity* constant = facts->category != sema::ValueCategory::PRValue &&
			(e->kind == syntax::ExprKind::Id || e->kind == syntax::ExprKind::Member) && facts->entity != nullptr &&
			facts->entity->kind == sema::EntityKind::Variable
		? static_cast<const sema::VariableEntity*>(facts->entity)
		: nullptr;
	if (constant != nullptr && constant->is_constexpr && constant->constant == sema::ConstantState::Object &&
		sema::IsWideIntegral(facts->type) && (facts->type->cv & sema::kVolatile) == 0)
	{
		RValue value;
		const sema::ConstValue* evaluated = lowering_.evaluated_value(constant);
		if (evaluated != nullptr && constant_rvalue(*evaluated, facts->type, value, e->location))
		{
			if (e->kind == syntax::ExprKind::Member)
				member_object(static_cast<const syntax::MemberExpr&>(*e));
			return value;
		}
	}
	if ((facts->flags & sema::kOperatorCall) != 0 && facts->category == sema::ValueCategory::PRValue &&
		e->kind != syntax::ExprKind::Call)
		return operator_call(*e);
	if ((facts->flags & sema::kBitField) != 0 && facts->category != sema::ValueCategory::PRValue &&
		e->kind != syntax::ExprKind::Conditional)
		return bit_field_value(*e);
	if (e->kind == syntax::ExprKind::Subscript && facts->category == sema::ValueCategory::PRValue)
		return vector_lane_value(static_cast<const syntax::SubscriptExpr&>(*e));
	// A list of a vector's lanes or a complex value's parts (which stands
	// for its object in the analysis) is that value.
	if (e->kind == syntax::ExprKind::BracedInit &&
		sema::LaneList(facts->type, static_cast<const syntax::BracedInitList&>(*e)))
		return vector_list(facts->type, static_cast<const syntax::BracedInitList&>(*e));
	if (facts->category != sema::ValueCategory::PRValue)
	{
		const sema::Type* type = facts->type;
		if (type->kind == sema::TypeKind::Array || type->kind == sema::TypeKind::Function)
		{
			RValue pointer;
			pointer.value = address(*e);
			pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
			return pointer;
		}
		// A glvalue conditional read as a value selects the operands'
		// values, not their addresses.
		if (e->kind == syntax::ExprKind::Conditional)
			return conditional_value(static_cast<const syntax::ConditionalExpr&>(*e));
		// A constexpr pointer's value is its evaluated address constant or
		// null, and a constexpr static data member's floating value or
		// pointer to member its evaluated value: reading either is no
		// odr-use (3.2p2), so the
		// program need not define a member (as 9.4.2p3 reads an integral
		// one); a member access's object expression is still evaluated. A
		// volatile object's read stays an access (1.9p8).
		const sema::VariableEntity* named = (e->kind == syntax::ExprKind::Id || e->kind == syntax::ExprKind::Member) &&
				facts->entity != nullptr && facts->entity->kind == sema::EntityKind::Variable
			? static_cast<const sema::VariableEntity*>(facts->entity)
			: nullptr;
		const bool static_member = named != nullptr && !named->member && named->owner != nullptr &&
			named->owner->kind == sema::ScopeKind::Class;
		const sema::ConstValue* evaluated = named != nullptr && named->is_constexpr && !named->type->is_reference() &&
				(type->cv & sema::kVolatile) == 0 && (e->kind == syntax::ExprKind::Id || static_member)
			? lowering_.evaluated_value(named)
			: nullptr;
		if (evaluated == nullptr)
			return load_glvalue(*e);
		const lowir::Type storage = lowering_.storage_type(type);
		if (evaluated->kind == sema::ValueKind::Pointer &&
			(evaluated->root == sema::RootKind::Null || storage.is_pointer()))
		{
			RValue pointer;
			pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
			if (evaluated->root == sema::RootKind::Null || constant_address(*evaluated, pointer.value, e->location))
			{
				if (e->kind == syntax::ExprKind::Member)
					member_object(static_cast<const syntax::MemberExpr&>(*e));
				if (evaluated->root == sema::RootKind::Null)
					return integer(storage, 0);
				Place designated = Place();
				designated.storage = pointer.value;
				pointer.value = address_of(designated);
				return pointer;
			}
		}
		if (static_member && evaluated->kind == sema::ValueKind::Floating && storage.is_float())
		{
			if (e->kind == syntax::ExprKind::Member)
				member_object(static_cast<const syntax::MemberExpr&>(*e));
			return floating(storage, lowir::float_literal(storage.kind(), evaluated->floating));
		}
		// So is a constexpr static data member's pointer to member.
		if (static_member && evaluated->kind == sema::ValueKind::MemberPointer)
		{
			if (e->kind == syntax::ExprKind::Member)
				member_object(static_cast<const syntax::MemberExpr&>(*e));
			return member_pointer_constant(*evaluated, e->location);
		}
		return load_glvalue(*e);
	}
	switch (e->kind)
	{
	case syntax::ExprKind::Literal:
		return literal(static_cast<const syntax::LiteralExpr&>(*e));
	case syntax::ExprKind::KeywordLiteral:
		return keyword_literal(static_cast<const syntax::KeywordLiteralExpr&>(*e));
	case syntax::ExprKind::Binary:
		return binary_expression(static_cast<const syntax::BinaryExpr&>(*e));
	case syntax::ExprKind::Unary:
		return unary_expression(static_cast<const syntax::UnaryExpr&>(*e));
	case syntax::ExprKind::Postfix:
		return postfix_increment(static_cast<const syntax::UnaryExpr&>(*e));
	case syntax::ExprKind::Conditional:
		return conditional_value(static_cast<const syntax::ConditionalExpr&>(*e));
	case syntax::ExprKind::Throw:
		throw_expression(static_cast<const syntax::ThrowExpr&>(*e));
		return RValue();
	case syntax::ExprKind::Statement:
		return statement_expression(static_cast<const syntax::StatementExpr&>(*e), lowir::Value());
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr& call_expr = static_cast<const syntax::CallExpr&>(*e);
		if ((facts->flags & sema::kIntrinsic) != 0)
			return intrinsic(call_expr);
		if (call_expr.callee->facts == nullptr && call_expr.arguments.size == 1)
			return cast(*e, *call_expr.arguments[0]);
		return call(call_expr);
	}
	case syntax::ExprKind::Cast:
		return cast(*e, *static_cast<const syntax::CastExpr&>(*e).operand);
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& functional = static_cast<const syntax::FunctionalCastExpr&>(*e);
		if (functional.braced != nullptr && facts->type->kind == sema::TypeKind::Array)
		{
			// 4.2p1: an array prvalue converted to a pointer to its first
			// element, the temporary array's.
			RValue pointer;
			pointer.value = array_temporary(functional);
			pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
			return pointer;
		}
		if (functional.braced != nullptr)
		{
			if (sema::LaneList(facts->type, *functional.braced))
				return vector_list(facts->type, *functional.braced);
			if (functional.braced->elements.empty())
				return zero_value(facts->type);
			return fit(operand(*functional.braced->elements[0], ConversionContext::Cast), facts->type);
		}
		if (functional.arguments.size == 1)
			return cast(*e, *functional.arguments[0]);
		return zero_value(facts->type);
	}
	case syntax::ExprKind::BracedInit:
	{
		// A braced-init-list argument for a scalar (8.5.4p3): its element
		// converted, or zero.
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*e);
		if (sema::LaneList(facts->type, list))
			return vector_list(facts->type, list);
		if (list.elements.empty())
			return zero_value(facts->type);
		return fit(operand(*list.elements[0], ConversionContext::Initialization), facts->type);
	}
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
		return size_query(*e);
	case syntax::ExprKind::BuiltinTrait:
		return builtin_trait_value(static_cast<const syntax::BuiltinTraitExpr&>(*e));
	case syntax::ExprKind::Id:
	{
		// A pointer template parameter's name: its argument's
		// address constant (a null one is folded).
		const sema::EnumeratorEntity* constant = facts->entity != nullptr &&
				facts->entity->kind == sema::EntityKind::Enumerator
			? static_cast<const sema::EnumeratorEntity*>(facts->entity)
			: nullptr;
		// A pointer to member template parameter's.
		if (constant != nullptr && constant->argument != nullptr &&
			constant->argument->element->kind == sema::TypeKind::MemberPointer)
			return member_pointer_constant(sema::ArgumentPointer(constant->argument), e->location);
		RValue pointer;
		pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		if (constant == nullptr || constant->argument == nullptr ||
			!constant_address(sema::ArgumentPointer(constant->argument), pointer.value, e->location))
			throw LoweringError(e->location, "a name without an address constant");
		Place designated = Place();
		designated.storage = pointer.value;
		pointer.value = address_of(designated);
		return pointer;
	}
	case syntax::ExprKind::New:
		return allocation(static_cast<const syntax::NewExpr&>(*e));
	case syntax::ExprKind::Delete:
		return deletion(static_cast<const syntax::DeleteExpr&>(*e));
	default:
		throw LoweringError(e->location, "an expression outside the procedural lowering slice");
	}
}

RValue FunctionLowering::operand(const syntax::Expr& expr, ConversionContext context)
{
	const sema::Conversion* conversion = WithoutParentheses(&expr)->facts->conversion;
	if (conversion_function(conversion))
		return converted_value(expr, conversion, context);
	RValue member;
	if (member_conversion_operand(expr, conversion, member))
		return member;
	return apply_conversion(expr, rvalue(expr), context);
}

RValue FunctionLowering::apply_conversion(const syntax::Expr& expr, const RValue& value, ConversionContext context)
{
	return apply_converted(expr, WithoutParentheses(&expr)->facts->conversion, value, context);
}

RValue FunctionLowering::apply_converted(
	const syntax::Expr& expr, const sema::Conversion* conversion, const RValue& value, ConversionContext context)
{
	// The value conversion the context recorded (clause 4), from the
	// operand's decayed type to the conversion's result type.
	const sema::ExprFacts* facts = WithoutParentheses(&expr)->facts;
	// An operand a converting constructor takes (13.3.3.1.2) initializes its
	// parameter by the user-defined conversion's first standard conversion.
	if (conversion != nullptr && conversion->constructor != nullptr)
		conversion = conversion->argument;
	if (conversion == nullptr || conversion->form == sema::ConversionForm::Reference || conversion->result == nullptr)
		return value;
	const sema::Type* from = DecayedType(lowering_.graph().types, facts->type);
	if (conversion->second == sema::ValueConversion::NullPointer &&
		conversion->result->unqualified->kind == sema::TypeKind::MemberPointer)
	{
		// 4.11p1: a null pointer constant (or nullptr) is the null member
		// pointer value.
		const lowir::Type storage = lowering_.storage_type(conversion->result);
		return integer(
			storage, storage.kind() == lowir::TypeKind::I128 ? 0 : static_cast<uint64_t>(lowering_.null_data_member()));
	}
	if (conversion->second == sema::ValueConversion::NullPointer && !IsNullptrType(from))
	{
		// 4.10p1: a null pointer constant becomes the null pointer value.
		uint64_t bits = 0;
		if (is_integer_literal(value, bits))
			return integer(lowir::Type::scalar(lowir::TypeKind::Ptr), 0);
	}
	if (conversion->second == sema::ValueConversion::MemberToDerived)
		return member_pointer_converted(value, member_adjustment(from, conversion->result, expr.location));
	if (conversion->second == sema::ValueConversion::DerivedToBase && IsPointerType(from) &&
		IsPointerType(conversion->result) && context != ConversionContext::Cast)
		return pointer_to_base(value, sema::ClassOf(from->element),
			sema::ClassOf(conversion->result->unqualified->element), expr.location, NonNullPointer(expr),
			!IsConditional(expr));
	return convert(value, from, conversion->result, context, IsLiteralExpression(expr));
}

// -------------------------------------------------------------- glvalues

Place FunctionLowering::glvalue(const syntax::Expr& expr)
{
	const syntax::Expr* e = WithoutParentheses(&expr);
	check_stack(e->location);
	const DebugScope debug(*this, e->location);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr)
		throw LoweringError(e->location, "an expression without semantic facts");
	Place place = Place();
	if ((facts->flags & sema::kUserConverted) != 0)
	{
		// A conversion function's reference result designates its referent.
		place.storage = call_conversion(
			*e, facts->original, static_cast<const sema::FunctionEntity*>(facts->entity), lowir::Value())
							.value;
		qualify(place, facts->type);
		return place;
	}
	if ((facts->flags & sema::kOperatorCall) != 0 && e->kind != syntax::ExprKind::Call)
	{
		// 5.2.2p10: an operator function returning a reference designates
		// its referent.
		place.storage = operator_call(*e).value;
		qualify(place, facts->type);
		return place;
	}
	switch (e->kind)
	{
	case syntax::ExprKind::Id:
		if ((facts->flags & sema::kImplicitThis) != 0 && facts->entity != nullptr &&
			facts->entity->kind == sema::EntityKind::Variable &&
			static_cast<const sema::VariableEntity*>(facts->entity)->member)
		{
			// 9.3.1p3: a non-static member named in a member function is
			// `(*this).m`; in a lambda's body the captured `this` converted
			// to its class first (the course's form: indexed at offset zero).
			lowir::Value object = object_pointer(e->location);
			if (closure_this_)
				object = index(lowir::Type::scalar(lowir::TypeKind::I8), 0, object,
					integer(lowir::Type::scalar(lowir::TypeKind::I64), 0))
							 .value;
			// Through a qualified name's naming class (11.2p6), which selects
			// a repeated base's subobject.
			const sema::ClassEntity* from = object_class();
			if (const sema::ClassEntity* naming = lowering_.member_naming_class(*e))
			{
				object = to_base(object, from, naming, e->location);
				from = naming;
			}
			return member_place(
				object, from, static_cast<const sema::VariableEntity*>(facts->entity), facts->type, e->location);
		}
		return variable_place(facts->entity, facts->type, e->location);
	case syntax::ExprKind::KeywordLiteral:
		break;
	case syntax::ExprKind::Literal:
		// A user-defined literal's literal operator returning a reference
		// designates its referent (5.2.2p10); a string literal its array.
		if ((facts->flags & sema::kUserLiteral) != 0)
		{
			place.storage = literal(static_cast<const syntax::LiteralExpr&>(*e)).value;
			qualify(place, facts->type);
			return place;
		}
		place.storage = lowir::Value::symbol(lowering_.string_literal(static_cast<const syntax::LiteralExpr&>(*e)));
		return place;
	case syntax::ExprKind::Unary:
	{
		const syntax::UnaryExpr& unary = static_cast<const syntax::UnaryExpr&>(*e);
		if (unary.op == TokenKind::OP_INC || unary.op == TokenKind::OP_DEC)
			return prefix_increment(unary);
		if (unary.op != TokenKind::OP_STAR)
			break;
		// 5.3.1p1: the object or function the pointer points to.
		const sema::ExprFacts* operand_facts = WithoutParentheses(unary.operand)->facts;
		if (operand_facts->type->kind == sema::TypeKind::Function)
			return glvalue(*unary.operand);
		place.storage = operand(*unary.operand, ConversionContext::Operand).value;
		qualify(place, facts->type);
		return place;
	}
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr& assign = static_cast<const syntax::BinaryExpr&>(*e);
		return assign.op == TokenKind::OP_ASS ? assignment(assign) : compound_assignment(assign);
	}
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr& binary = static_cast<const syntax::BinaryExpr&>(*e);
		if (binary.op == TokenKind::OP_DOTSTAR || binary.op == TokenKind::OP_ARROWSTAR)
			return member_pointer_place(binary);
		if (binary.op != TokenKind::OP_COMMA)
			break;
		// 5.18p1: the right operand is the result, the left discarded.
		discard(*binary.left);
		return glvalue(*binary.right);
	}
	case syntax::ExprKind::Subscript:
		return subscript(static_cast<const syntax::SubscriptExpr&>(*e));
	case syntax::ExprKind::Member:
		return member(static_cast<const syntax::MemberExpr&>(*e));
	case syntax::ExprKind::Conditional:
		return conditional_place(static_cast<const syntax::ConditionalExpr&>(*e));
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr& call_expr = static_cast<const syntax::CallExpr&>(*e);
		if (call_expr.callee->facts == nullptr && call_expr.arguments.size == 1)
			return reference_cast(*e, *call_expr.arguments[0]);
		// 5.2.2p10: a call returning a reference designates its referent.
		place.storage = call(call_expr).value;
		qualify(place, facts->type);
		return place;
	}
	case syntax::ExprKind::TypeTrait:
		if (static_cast<const syntax::TypeTraitExpr&>(*e).keyword != TokenKind::KW_TYPEID)
			break;
		return typeid_place(static_cast<const syntax::TypeTraitExpr&>(*e));
	case syntax::ExprKind::Cast:
	{
		if ((facts->flags & sema::kRuntimeCheck) != 0)
			return dynamic_cast_reference(static_cast<const syntax::CastExpr&>(*e));
		return reference_cast(*e, *static_cast<const syntax::CastExpr*>(e)->operand);
	}
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& functional = static_cast<const syntax::FunctionalCastExpr&>(*e);
		if (functional.arguments.size == 1)
			return reference_cast(*e, *functional.arguments[0]);
		// An array prvalue: the temporary array (12.2p1).
		if (functional.braced != nullptr && facts->type->kind == sema::TypeKind::Array)
		{
			place.storage = array_temporary(functional);
			return place;
		}
		break;
	}
	case syntax::ExprKind::BuiltinTrait:
	{
		// GNU `__real__ x` of a real lvalue designates x; of a complex
		// lvalue, its real part, as `__imag__ x` its imaginary part.
		const syntax::BuiltinTraitExpr* trait = static_cast<const syntax::BuiltinTraitExpr*>(e);
		const syntax::Expr* operand = WithoutParentheses(trait->operand);
		if ((trait->trait == GnuTrait::RealPart || trait->trait == GnuTrait::ImagPart) &&
			operand->facts->type->unqualified->kind == sema::TypeKind::Complex)
			return complex_part_place(*operand, trait->trait == GnuTrait::ImagPart);
		if (trait->trait == GnuTrait::RealPart)
			return glvalue(*trait->operand);
		break;
	}
	default:
		break;
	}
	throw LoweringError(e->location, "a glvalue outside the procedural lowering slice");
}

Place FunctionLowering::reference_cast(const syntax::Expr& expr, const syntax::Expr& operand_expr)
{
	const sema::ExprFacts* facts = expr.facts;
	Place place = Place();
	// 5.2.9p2, 5.2.11, 5.4: a cast to a reference type designates the
	// operand's object; 5.2.9p4: or the temporary its binding makes.
	const sema::Conversion* binding = WithoutParentheses(&operand_expr)->facts->conversion;
	if ((facts->flags & sema::kReferenceCast) != 0 && binding != nullptr &&
		binding->form == sema::ConversionForm::Reference &&
		(binding->constructor != nullptr || binding->temporary ||
			binding->second == sema::ValueConversion::DerivedToBase))
	{
		place.storage = bind_reference(operand_expr, facts->target, ConversionContext::Cast);
		return place;
	}
	// A class prvalue operand: the temporary it initializes (12.2p1).
	const sema::ExprFacts* operand_facts = WithoutParentheses(&operand_expr)->facts;
	if (operand_facts->category == sema::ValueCategory::PRValue && sema::ClassOf(operand_facts->type) != nullptr)
		place.storage = temporary_object(operand_expr);
	else
		place = glvalue(operand_expr);
	// 5.2.9p2: a base class subobject (indexed even at offset zero, the
	// course's form of a reference upcast), or the derived object
	// holding it (adjusted past the table pointer the derived class put
	// before its base).
	if ((facts->flags & (sema::kBaseCast | sema::kDerivedCast)) != 0)
	{
		const sema::ClassEntity* from = sema::ClassOf(operand_facts->type);
		const sema::ClassEntity* to = sema::ClassOf(facts->type);
		const bool upcast = (facts->flags & sema::kBaseCast) != 0;
		if (upcast || lowering_.base_offset(to, from, expr.location) != 0)
		{
			const lowir::Value object = address_of(place);
			const bool is_volatile = place.is_volatile;
			place = Place();
			place.is_volatile = is_volatile;
			// (A complete object's virtual base at its offset there.)
			place.storage = upcast
				? to_base(object, from, to, expr.location,
					  VirtualBaseCount(from) != 0 ? member_base_access(operand_expr, false) : BaseAccess::Table)
				: to_derived(object, from, to, expr.location);
		}
	}
	return place;
}

Place FunctionLowering::variable_place(const sema::Entity* entity, const sema::Type* type, uint32_t location)
{
	Place place = Place();
	if (type != nullptr)
		qualify(place, type);
	if (entity == nullptr)
		throw LoweringError(location, "a name without an entity");
	if (entity->kind == sema::EntityKind::Function)
	{
		// A static member function is an ordinary function (9.4.1).
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(entity);
		if (function->member && !function->is_static)
			throw LoweringError(location, "member functions are outside the procedural lowering slice");
		lowering_.demand(function);
		place.storage = lowir::Value::symbol(lowering_.function_symbol(function));
		return place;
	}
	if (entity->kind == sema::EntityKind::Variable && static_cast<const sema::VariableEntity*>(entity)->member)
	{
		// A member of a namespace- or block-scope anonymous union named
		// alone: in the union's unnamed object (9.5p5).
		const sema::VariableEntity* member = static_cast<const sema::VariableEntity*>(entity);
		uint64_t offset = 0;
		const sema::ClassEntity* home = lowering_.member_home(member, offset, location);
		if (home->anonymous && home->storage != nullptr)
		{
			const sema::VariableEntity* object = static_cast<const sema::VariableEntity*>(home->storage->entity);
			const Place outer = variable_place(object, object->type, location);
			place.storage = this->index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD,
									address_of(outer),
									integer(lowir::Type::scalar(lowir::TypeKind::I64),
										offset + lowering_.member_offset(member, location)))
								.value;
			if (union_member(member))
				mark_untyped(place.storage);
			return place;
		}
	}
	const uint32_t index = local_index_.find(entity);
	bool reference = false;
	if (index != PointerMap::kMissing && locals_[index].captured)
	{
		// A lambda's capture: the closure's member is its copy or holds its
		// address.
		place.storage = this->index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD,
								this_pointer(location),
								integer(lowir::Type::scalar(lowir::TypeKind::I64), locals_[index].slot))
							.value;
		reference = locals_[index].reference;
	}
	else if (index != PointerMap::kMissing)
	{
		const Local& local = locals_[index];
		place.storage = local.address.kind != lowir::ValueKind::None ? local.address : lowir::Value::slot(local.slot);
		reference = local.reference;
	}
	else if (const sema::BindingEntity* alias = sema::BindingAlias(entity))
	{
		// A structured binding of an array or a class (C++17's 11.5p2,
		// p4): its element or non-static data member of the object e.
		const sema::Decomposition* decomposition = alias->decomposition;
		const lowir::Value object = address_of(variable_place(decomposition->object, decomposition->type, location));
		if (decomposition->form == sema::DecompositionForm::Members)
			return member_place(object, sema::ClassOf(decomposition->type), alias->member, type, location);
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		const sema::Type* element = alias->type;
		if (element->unqualified->kind == sema::TypeKind::Class || element->kind == sema::TypeKind::Array)
			place.storage = this->index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, object,
									integer(i64, alias->index * lowering_.size_of(element, location)))
								.value;
		else
			place.storage = this->index(lowering_.storage_type(element), ir_model::IPK_ARRAY_ELEMENT, object,
									integer(i64, alias->index))
								.value;
		return place;
	}
	else if (entity->kind == sema::EntityKind::Variable &&
		static_cast<const sema::VariableEntity*>(entity)->is_static && entity->owner != nullptr &&
		(entity->owner->kind == sema::ScopeKind::Block || entity->owner->kind == sema::ScopeKind::Function))
	{
		// An enclosing function's block-scope static named in a lambda's or
		// a local class's member's body: that function's global (6.7p4),
		// made where its lowering met the declaration, before it demanded
		// the body naming it.
		const lowir::SymbolId global = lowering_.local_static_symbol(static_cast<const sema::VariableEntity*>(entity));
		if (global == lowir::kNoId)
			throw LoweringError(location, "a static local variable of a function not lowered first");
		place.storage = lowir::Value::symbol(global);
		reference = entity->type->is_reference();
	}
	else
	{
		if (entity->kind != sema::EntityKind::Variable)
			throw LoweringError(location, "a name outside the procedural lowering slice");
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(entity);
		const bool block_extern = variable->is_extern && variable->owner != nullptr &&
			variable->owner->kind == sema::ScopeKind::Block;
		// 9.4.2: a static data member is an object of the program.
		const bool static_member = !variable->member && variable->owner != nullptr &&
			variable->owner->kind == sema::ScopeKind::Class;
		// A function's predefined name variable (8.4.1p8, GCC's
		// __FUNCTION__ and __PRETTY_FUNCTION__): the characters the analysis
		// gave it.
		if (variable->predefined_name)
		{
			place.storage = lowir::Value::symbol(lowering_.predefined_variable(variable));
			return place;
		}
		if (variable->member || variable->owner == nullptr ||
			(variable->owner->kind != sema::ScopeKind::Namespace && !block_extern && !static_member))
			throw LoweringError(location, "a variable outside the procedural lowering slice");
		// A constexpr reference designates its referent (5.19p2: its
		// evaluated address constant is reused, not loaded).
		lowir::Value referent;
		const sema::ConstValue* evaluated = variable->is_constexpr && variable->type->is_reference() &&
				!lowering_.read_through_storage(variable)
			? lowering_.evaluated_value(variable)
			: nullptr;
		if (evaluated != nullptr && constant_address(*evaluated, referent, location))
		{
			place.storage = referent;
			return place;
		}
		place.storage = lowir::Value::symbol(lowering_.variable_symbol(variable));
		reference = variable->type->is_reference();
		// A thread-local object is reached through its address, computed
		// for the executing thread (storage=thread_local); one initialized
		// per thread through its wrapper.
		if (variable->is_thread_local)
		{
			// A use before the unit's definition goes through the wrapper the
			// definition will give a body (its initialization unknown yet).
			uint32_t wrapper = lowering_.thread_local_wrapper(variable);
			if (wrapper == PointerMap::kMissing && variable->defined &&
				lowering_.program().symbols[place.storage.index].kind == lowir::SymbolKind::Unresolved)
				wrapper = lowering_.thread_local_wrapper_symbol(place.storage.index, variable);
			// Object mode: one another unit defines, through a wrapper running
			// that unit's initialization for the thread first.
			else if (wrapper == PointerMap::kMissing && lowering_.object_mode() && !variable->defined &&
				!lowering_.unit_local(variable))
				wrapper = lowering_.external_thread_local_wrapper(place.storage.index, variable);
			if (wrapper != PointerMap::kMissing)
				place.storage = emit_call(lowir::Type::scalar(lowir::TypeKind::Ptr), lowir::Value::symbol(wrapper),
					arguments_.size(), lowir::kNoId, false);
			else
				place.storage = address_of(place);
		}
	}
	if (reference)
	{
		// The slot or global holds the address the reference is bound to.
		Place holder = Place();
		holder.storage = place.storage;
		place.storage = load(lowir::Type::scalar(lowir::TypeKind::Ptr), holder).value;
	}
	return place;
}

bool FunctionLowering::constant_rvalue(
	const sema::ConstValue& value, const sema::Type* type, RValue& out, uint32_t location)
{
	// An evaluated scalar as an operand of its type: a constant, or an
	// address constant's symbol plus offset.
	const lowir::Type storage = lowering_.storage_type(type);
	switch (value.kind)
	{
	case sema::ValueKind::Integer:
		if (!storage.is_integer())
			return false;
		// A 128-bit type's whole value.
		out = sema::IsWideIntegral(type) ? integer(storage, lowir::Literal{ value.bits, value.high })
										 : integer(type, value.bits);
		return true;
	case sema::ValueKind::Floating:
		if (SoftFloatOf(type) == SoftFloat::BFloat)
		{
			// A bfloat16 value, which the evaluation holds exactly.
			StaticValue floating = StaticValue(), encoded = StaticValue();
			floating.kind = StaticValue::Kind::Floating;
			floating.floating = value.floating;
			if (!SoftStaticValue(floating, nullptr, SoftFloat::BFloat, encoded))
				return false;
			out = soft_constant(SoftFloat::BFloat, encoded.bits, 0);
			return true;
		}
		if (!storage.is_float())
			return false;
		out = floating(storage, lowir::float_literal(storage.kind(), value.floating));
		return true;
	case sema::ValueKind::Pointer:
		if (!storage.is_pointer())
			return false;
		out.type = storage;
		if (value.root == sema::RootKind::Null)
		{
			out = integer(storage, 0);
			return true;
		}
		if (!constant_address(value, out.value, location))
			return false;
		if (value.address.offset == 0)
		{
			Place designated = Place();
			designated.storage = out.value;
			out.value = address_of(designated);
		}
		return true;
	default:
		return false;
	}
}

bool FunctionLowering::constant_address(const sema::ConstValue& value, lowir::Value& out, uint32_t location)
{
	// An evaluated address constant (3.6.2p2's) as the storage of the
	// object or function it designates: its symbol, or the symbol's
	// address plus the offset; false for one no symbol names.
	if (value.kind != sema::ValueKind::Pointer)
		return false;
	Place place = Place();
	switch (value.root)
	{
	case sema::RootKind::Variable:
	{
		const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(value.address.entity);
		if (variable->owner == nullptr ||
			(variable->owner->kind != sema::ScopeKind::Namespace && variable->owner->kind != sema::ScopeKind::Class) ||
			variable->is_thread_local)
			return false;
		place.storage = lowir::Value::symbol(lowering_.variable_symbol(variable));
		break;
	}
	case sema::RootKind::Literal:
		place.storage = lowir::Value::symbol(
			lowering_.string_literal(*static_cast<const syntax::LiteralExpr*>(value.address.entity)));
		break;
	case sema::RootKind::Temporary:
	{
		const lowir::SymbolId temporary = lowering_.constant_temporary(
			static_cast<const sema::ConstValue*>(value.address.entity), location);
		if (temporary == lowir::kNoId)
			return false;
		place.storage = lowir::Value::symbol(temporary);
		break;
	}
	case sema::RootKind::Function:
	{
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(value.address.entity);
		lowering_.demand(function);
		place.storage = lowir::Value::symbol(lowering_.function_symbol(function));
		break;
	}
	case sema::RootKind::TypeInfo:
		place.storage = lowir::Value::symbol(
			lowering_.type_information(static_cast<const sema::Type*>(value.address.entity)));
		break;
	default:
		return false;
	}
	// The symbol itself designates the object at offset zero.
	out = place.storage;
	if (value.address.offset != 0)
		out = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, address_of(place),
			integer(lowir::Type::scalar(lowir::TypeKind::I64), static_cast<uint64_t>(value.address.offset)))
				  .value;
	(void)location;
	return true;
}

lowir::Value FunctionLowering::address(const syntax::Expr& expr)
{
	return address_of(glvalue(expr));
}

lowir::Value FunctionLowering::this_pointer(uint32_t location)
{
	if (this_slot_ == lowir::kNoId)
		throw LoweringError(location, "`this` outside a non-static member function");
	Place holder = Place();
	holder.storage = lowir::Value::slot(this_slot_);
	const lowir::Value value = load(lowir::Type::scalar(lowir::TypeKind::Ptr), holder).value;
	if (tables_pending_ || vtt_param_.kind != lowir::ValueKind::None)
		this_values_.push_back(value);
	return value;
}

lowir::Value FunctionLowering::object_pointer(uint32_t location)
{
	if (!closure_this_)
		return this_pointer(location);
	Place captured = Place();
	captured.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, this_pointer(location),
		integer(lowir::Type::scalar(lowir::TypeKind::I64), closure_this_offset_))
						   .value;
	return load(lowir::Type::scalar(lowir::TypeKind::Ptr), captured).value;
}

const sema::ClassEntity* FunctionLowering::object_class() const
{
	return closure_this_ ? closure_this_class_ : sema::ClassOf(function_->owner->entity->type);
}

lowir::Value FunctionLowering::function_address(const sema::FunctionEntity* function, uint32_t location)
{
	return address_of(variable_place(function, function->type, location));
}

lowir::Value FunctionLowering::to_base(lowir::Value object, const sema::ClassEntity* from, const sema::ClassEntity* to,
	uint32_t location, BaseAccess access)
{
	// The base subobject at its offset, one adjustment; through a virtual
	// base, that base's address from the object's table first (in a
	// complete object, its offset there).
	if (from == to)
		return object;
	const ProgramLowering::BaseRoute route = lowering_.base_route(from, to, location);
	uint64_t offset = route.offset;
	if (route.via != nullptr && access == BaseAccess::Nullable)
	{
		RValue pointer;
		pointer.value = object;
		pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		return virtual_base_pointer(pointer, from, to, false, location).value;
	}
	if (route.via != nullptr && access == BaseAccess::Complete)
		offset += lowering_.virtual_base_offset(from, route.via);
	else if (route.via != nullptr)
		object = virtual_base_address(object, from, route.via);
	return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, object,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), offset))
		.value;
}

FunctionLowering::BaseAccess FunctionLowering::member_base_access(const syntax::Expr& base, bool arrow)
{
	const syntax::Expr* e = WithoutParentheses(&base);
	const sema::ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->type == nullptr)
		return BaseAccess::Table;
	if (arrow)
		return facts->type->unqualified->kind == sema::TypeKind::Pointer && !NonNullPointer(*e) ? BaseAccess::Nullable
																								: BaseAccess::Table;
	if (facts->type->unqualified->kind != sema::TypeKind::Class)
		return BaseAccess::Table;
	if (e->kind != syntax::ExprKind::Id)
		return facts->category == sema::ValueCategory::PRValue ? BaseAccess::Complete : BaseAccess::Table;
	// A named variable or by-value parameter of the class is a complete
	// object of it.
	const sema::Entity* entity = facts->entity;
	return (facts->flags & sema::kImplicitThis) == 0 && entity != nullptr &&
			(entity->kind == sema::EntityKind::Variable || entity->kind == sema::EntityKind::Parameter) &&
			entity->type != nullptr && entity->type->unqualified->kind == sema::TypeKind::Class
		? BaseAccess::Complete
		: BaseAccess::Table;
}

lowir::Value FunctionLowering::to_derived(
	lowir::Value object, const sema::ClassEntity* from, const sema::ClassEntity* to, uint32_t location)
{
	// 5.2.9p2, p11: the derived object containing the base subobject.
	const uint64_t offset = lowering_.base_offset(to, from, location);
	if (offset == 0)
		return object;
	return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, object,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), 0 - offset))
		.value;
}

RValue FunctionLowering::pointer_to_base(const RValue& value, const sema::ClassEntity* from,
	const sema::ClassEntity* to, uint32_t location, bool nonnull, bool reserve)
{
	// 4.10p3: a null pointer converts to the null pointer; another to its
	// base class subobject, which lies past the table pointer when the
	// derived class introduced it (a nonzero offset, tested first).
	if (from == to)
		return value;
	const ProgramLowering::BaseRoute route = lowering_.base_route(from, to, location);
	if (route.via != nullptr)
	{
		// The course reserves a null-tested conversion's result slot
		// before it finds the route; through a virtual base the projection
		// holds the result in a slot of its own, the reserved one unused
		// (a conditional operand's result slot is that reservation).
		if (!nonnull && reserve)
			generated_slot(lowir::SlotKind::Basecast, lowir::Type::scalar(lowir::TypeKind::Ptr));
		return virtual_base_pointer(value, from, to, nonnull, location);
	}
	return adjusted_pointer(value, route.offset, route.offset != 0 && !nonnull);
}

RValue FunctionLowering::pointer_to_derived(
	const RValue& value, const sema::ClassEntity* from, const sema::ClassEntity* to, uint32_t location, bool nonnull)
{
	// 5.2.9p11: the null pointer stays null; another points to the derived
	// object containing the base subobject.
	const uint64_t offset = lowering_.base_offset(to, from, location);
	if (offset == 0)
		return value;
	return adjusted_pointer(value, 0 - offset, !nonnull);
}

RValue FunctionLowering::adjusted_pointer(const RValue& value, uint64_t delta, bool null_test)
{
	// The pointer moved by `delta` bytes (two's complement), a null one
	// kept when tested.
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue adjusted;
	adjusted.type = value.type;
	if (!null_test)
	{
		adjusted.value = index(i8, ir_model::IPK_NONE, value.value, integer(i64, delta)).value;
		return adjusted;
	}
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Basecast, pointer));
	const lowir::BlockId null_block = new_block(lowir::BlockKind::BasecastNull);
	const lowir::BlockId adjust = new_block(lowir::BlockKind::BasecastAdjust);
	const lowir::BlockId end = new_block(lowir::BlockKind::BasecastEnd);
	branch(compare(lowir::ComparePredicate::Eq, pointer, value, integer(pointer, 0)), null_block, adjust);
	place(null_block);
	store(pointer, integer(pointer, 0), result);
	jump(end);
	place(adjust);
	adjusted.value = index(i8, ir_model::IPK_NONE, value.value, integer(i64, delta)).value;
	store(pointer, adjusted, result);
	jump(end);
	place(end);
	return load(pointer, result);
}

Place FunctionLowering::member_place(lowir::Value object, const sema::ClassEntity* from,
	const sema::VariableEntity* member, const sema::Type* type, uint32_t location, BaseAccess access)
{
	// 5.2.5: the member at its offset in its class's subobject.
	if (member->bit_field)
		throw LoweringError(location, "a bit-field glvalue outside the supported forms");
	uint64_t anonymous = 0;
	const sema::ClassEntity* holder = lowering_.member_holder(member, from, anonymous, location);
	object = anonymous_object(object, from, holder, location);
	object = to_base(object, from, holder, location, access);
	Place place = Place();
	place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, object,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), anonymous + lowering_.member_offset(member, location)))
						.value;
	// Named in a union, or in a union object (lowir.md "Instruction Facts").
	if (union_member(member) || (holder != nullptr && holder->key == TokenKind::KW_UNION) ||
		(from != nullptr && from->key == TokenKind::KW_UNION))
		mark_untyped(place.storage);
	if (member->type->is_reference())
	{
		// 9.2: a reference member holds the address it is bound to.
		place.storage = load(lowir::Type::scalar(lowir::TypeKind::Ptr), place).value;
	}
	qualify(place, type);
	return place;
}

RValue FunctionLowering::load_glvalue(const syntax::Expr& expr)
{
	const sema::ExprFacts* facts = WithoutParentheses(&expr)->facts;
	Place place = glvalue(expr);
	// An atomic object's assignment or read-modify-write has the value it
	// stored (C11 6.5.16p3, 6.5.16.2p3), not a later read of the object.
	if (place.has_known && (!place.is_volatile || place.is_atomic))
		return place.known;
	return load(lowering_.storage_type(facts->type), place);
}

// -------------------------------------------------------------- literals

RValue FunctionLowering::literal(const syntax::LiteralExpr& expr, lowir::Value destination)
{
	if ((expr.facts->flags & sema::kUserLiteral) != 0)
	{
		// A class result initializes `destination` (as call's).
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(expr.facts->entity);
		const size_t base = arguments_.size();
		const bool indirect = lowering_.indirect_result(function);
		if (indirect)
			arguments_.push_back(destination);
		if (expr.facts->value == sema::kLiteralCallString)
		{
			// 2.14.8p5: the literal operator called with the literal's array
			// (its first code unit's address) and its length (an int constant
			// converted to std::size_t, the course's form).
			sema::StringLiteralFacts string;
			std::vector<uint64_t> units;
			if (!sema::StringLiteralUnits(expr.spelling, string, units))
				throw LoweringError(expr.location, "an invalid string literal");
			const sema::Type* unit = lowering_.graph().types.fundamental(string.element);
			Place array = Place();
			array.storage = lowir::Value::symbol(lowering_.string_literal(expr, unit));
			arguments_.push_back(address_of(array));
			arguments_.push_back(convert_op(lowir::ConvertOp::SExt, lowir::Type::scalar(lowir::TypeKind::I64),
				integer(lowir::Type::scalar(lowir::TypeKind::I32), string.units - 1))
					.value);
		}
		else if (expr.facts->value == sema::kLiteralCallRaw)
		{
			// 2.14.8p3-p4: the raw literal operator called with the
			// number's characters (without the ud-suffix), a string
			// literal's array.
			Place array = Place();
			array.storage = lowir::Value::symbol(lowering_.string_literal(expr));
			arguments_.push_back(address_of(array));
		}
		else if (expr.facts->value == sema::kLiteralCallCharacter)
		{
			// 2.14.8p6: the character's value converted to the parameter of
			// the literal operator the call selected.
			const CharacterLiteralValue character = EvaluateCharacterLiteral(expr.spelling.text, expr.spelling.length);
			const sema::Type* type = lowering_.graph().types.fundamental(character.type);
			arguments_.push_back(convert(integer(lowering_.storage_type(type), character.code_point), type,
				function->type->canonical->params[0], ConversionContext::Initialization, false)
					.value);
		}
		else if (expr.facts->value == sema::kLiteralCallCooked)
		{
			// 2.14.8p3-p4: the number's value (as its literal without the
			// ud-suffix) converted to the parameter's type (a floating one's
			// scanned in the long double parameter's precision).
			const NumberLiteral number = ClassifyNumber(expr.spelling.text, expr.spelling.length);
			const sema::Type* parameter = function->type->canonical->params[0];
			const NumberLiteral value = ClassifyNumber(expr.spelling.text, number.suffix_begin);
			uint64_t digits = 0;
			if (value.kind == NumberLiteralKind::Integer)
			{
				const sema::Type* type = lowering_.graph().types.fundamental(value.type);
				arguments_.push_back(convert(integer(lowering_.storage_type(type), value.integer), type, parameter,
					ConversionContext::Initialization, false)
						.value);
			}
			else if (number.kind == NumberLiteralKind::UserDefinedInteger)
			{
				// Digits no literal type holds: `n ULL` (the analysis checked
				// unsigned long long does).
				if (!UserIntegerValue(expr.spelling.text, number.suffix_begin, digits))
					throw LoweringError(expr.location, "a user-defined integer literal beyond unsigned long long");
				arguments_.push_back(integer(lowering_.storage_type(parameter), digits).value);
			}
			else
			{
				const lowir::Type storage = lowering_.storage_type(parameter);
				lowir::TypeKind kind = storage.kind();
				lowir::Literal bits;
				std::string scratch;
				if (!storage.is_float() ||
					!lowir::scan_float_literal(
						expr.spelling.text, number.suffix_begin, false, storage.kind(), kind, bits, scratch))
					throw LoweringError(expr.location, "a floating literal outside its type's range");
				arguments_.push_back(floating(storage, bits).value);
			}
		}
		lowering_.demand(function);
		RValue value;
		value.type = result_type(function);
		value.value = emit_call(value.type, lowir::Value::symbol(lowering_.function_symbol(function)), base,
			lowir::kNoId, call_may_throw(function));
		if (value.type.kind() == lowir::TypeKind::Obj)
			copy_result(value, expr.facts->type, destination, expr.location);
		return value;
	}
	const sema::Type* type = expr.facts->type;
	const lowir::Type storage = lowering_.storage_type(type);
	if (storage.is_object() && SoftFloatOf(type) != SoftFloat::None)
		return soft_literal(SoftFloatOf(type), expr.spelling.text,
			ClassifyNumber(expr.spelling.text, expr.spelling.length).suffix_begin, expr.location);
	if (!storage.is_float())
		return integer(storage, expr.facts->value);
	// 2.14.4: the value in the literal's own type (its digits without the
	// suffix: an extended type's is several characters).
	lowir::TypeKind kind = storage.kind();
	lowir::Literal bits;
	std::string scratch;
	const uint32_t digits = ClassifyNumber(expr.spelling.text, expr.spelling.length).suffix_begin;
	if (!lowir::scan_float_literal(expr.spelling.text, digits, false, storage.kind(), kind, bits, scratch))
		throw LoweringError(expr.location, "a floating literal outside its type's range");
	return floating(storage, bits);
}

RValue FunctionLowering::builtin_trait_value(const syntax::BuiltinTraitExpr& trait)
{
	// The traits the analysis leaves unfolded in an operand: GNU's parts of
	// a complex or real value, Clang's vector conversion, the bit cast, and
	// __builtin_va_arg (LowIR's va_arg of the list's __va_list_tag).
	const sema::ExprFacts* facts = trait.facts;
	if ((trait.trait == GnuTrait::RealPart || trait.trait == GnuTrait::ImagPart) &&
		WithoutParentheses(trait.operand)->facts->type->unqualified->kind == sema::TypeKind::Complex)
		return complex_part(*WithoutParentheses(trait.operand), trait.trait == GnuTrait::ImagPart);
	if (trait.trait == GnuTrait::RealPart)
		return rvalue(*trait.operand);
	if (trait.trait == GnuTrait::ImagPart)
	{
		// Zero of the operand's type, the operand evaluated first.
		discard(*trait.operand);
		const lowir::Type type = lowering_.storage_type(facts->type);
		if (SoftFloatOf(facts->type) != SoftFloat::None)
			return soft_zero(SoftFloatOf(facts->type));
		return type.is_float() ? floating(type, lowir::float_literal(type.kind(), 0)) : integer(type, 0);
	}
	if (trait.trait == GnuTrait::ConvertVector)
		return convert_vector(trait);
	if (trait.trait == GnuTrait::BitCast)
		return bit_cast(trait);
	if (trait.trait != GnuTrait::VaArg)
		throw LoweringError(trait.location, "an unfolded type trait");
	const lowir::Type type = lowering_.storage_type(facts->type);
	const sema::ClassEntity* entity = sema::ClassOf(facts->type);
	if (entity != nullptr && !(lowering_.object_mode() && lowering_.passed_directly(entity)))
	{
		// A class object a caller passes to `...` through the address of
		// its copy (the course's form; see arguments): that address, then
		// the object's bytes there.
		lowir::Instruction inst = lowir::make_instruction(
			lowir::Opcode::VaArg, lowir::Type::scalar(lowir::TypeKind::Ptr));
		inst.ops[0] = va_list_tag(*trait.operand);
		Place copy = Place();
		copy.storage = emit_value(inst, lowir::Type::scalar(lowir::TypeKind::Ptr));
		return load(type, copy);
	}
	// An i128 is read as the object of its bytes its caller passes (see
	// variadic_wide_object; object mode).
	const bool wide = type.kind() == lowir::TypeKind::I128 && lowering_.object_mode();
	const lowir::Type read = wide ? variadic_wide_type(facts->type) : type;
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::VaArg, read);
	inst.ops[0] = va_list_tag(*trait.operand);
	RValue value;
	value.value = emit_value(inst, read);
	value.type = read;
	// The classes of a parameter of its type (object mode, psABI 3.2.3): a
	// class object's as its bytes, a member function pointer's, a binary16
	// or binary128 value's.
	const uint8_t classes = lowering_.object_classes(facts->type);
	if (classes != 0)
		builder_.body().argument_classes.push_back(
			lowir::ArgumentClasses{ builder_.last_instruction(builder_.insertion_block()), 0, classes });
	if (!wide)
		return value;
	Place bytes = Place();
	bytes.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Arg, read));
	store(read, value, bytes);
	Place view = Place();
	view.storage = address_of(bytes);
	return load(type, view);
}

lowir::Type FunctionLowering::variadic_wide_type(const sema::Type* type)
{
	// psABI 3.2.3, Itanium 2.3: a member function pointer is a pair of
	// eightbytes (8-aligned), an __int128 16-aligned.
	const sema::Type* object = type->unqualified;
	const bool pair = object->kind == sema::TypeKind::MemberPointer &&
		object->element->kind == sema::TypeKind::Function;
	return lowir::Type::object(16, pair ? 3 : 4);
}

lowir::Value FunctionLowering::variadic_wide_object(const RValue& value, const sema::Type* type)
{
	const lowir::Type bytes = variadic_wide_type(type);
	Place object = Place();
	object.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Arg, bytes));
	Place view = Place();
	view.storage = address_of(object);
	store(value.type, value, view);
	return object.storage;
}

RValue FunctionLowering::keyword_literal(const syntax::KeywordLiteralExpr& expr)
{
	switch (expr.keyword)
	{
	case TokenKind::KW_TRUE:
		return integer(lowir::Type::scalar(lowir::TypeKind::U8), 1);
	case TokenKind::KW_FALSE:
		return integer(lowir::Type::scalar(lowir::TypeKind::U8), 0);
	case TokenKind::KW_NULLPTR:
		return integer(lowir::Type::scalar(lowir::TypeKind::Ptr), 0);
	default:
	{
		// 9.3.2p1: `this` is a prvalue, the member function's object (in
		// an anonymous class's member's default member initializer, that
		// class's object).
		RValue pointer;
		const sema::Type* type = expr.facts != nullptr ? expr.facts->type : nullptr;
		pointer.value = type != nullptr && type->kind == sema::TypeKind::Pointer
			? object_pointer_as(sema::ClassOf(type->element), expr.location)
			: object_pointer(expr.location);
		pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		return pointer;
	}
	}
}

RValue FunctionLowering::size_query(const syntax::Expr& expr)
{
	// 5.3.3: sizeof (and alignof, 5.3.6) is its folded size_t value,
	// materialized as a constant.
	return constant(lowering_.storage_type(expr.facts->type), expr.facts->value);
}

// ------------------------------------------------------------- operators

RValue FunctionLowering::binary_expression(const syntax::BinaryExpr& expr)
{
	if ((expr.facts->flags & sema::kTypeIdentity) != 0)
		return type_identity(expr);
	if ((expr.facts->flags & sema::kOperatorCall) != 0)
		return operator_call(expr);
	if (expr.op == TokenKind::OP_COMMA)
	{
		// 5.18: the left operand discarded (its own left-nested commas
		// walked by discard), the right one the value.
		discard(*expr.left);
		return rvalue(*expr.right);
	}
	if (expr.op == TokenKind::OP_LAND || expr.op == TokenKind::OP_LOR)
		return logical_value(expr);
	// 5.5p6: `.*` on a class prvalue designates a member of its temporary
	// and is a prvalue: the member's value.
	if (expr.op == TokenKind::OP_DOTSTAR || expr.op == TokenKind::OP_ARROWSTAR)
		return load(lowering_.storage_type(expr.facts->type), member_pointer_place(expr));
	if (bfloat_operation(expr))
		return bfloat_arithmetic(expr);
	// A left-nested chain of arithmetic and relational operators, as long
	// as the source makes it (the analysis accepts any length), is walked
	// iteratively: the left spine first, then each operator upward with the
	// value below it as its left operand.
	const size_t base = spine_.size();
	const syntax::BinaryExpr* node = &expr;
	for (;;)
	{
		spine_.push_back(node);
		// An operand that calls an operator function, compares type
		// information objects (type_identity) or is bfloat16 arithmetic (in
		// float, rounded once: bfloat_arithmetic) is a leaf of the chain.
		const syntax::Expr* left = WithoutParentheses(node->left);
		if (left->kind != syntax::ExprKind::Binary || !IsChainable(static_cast<const syntax::BinaryExpr*>(left)->op) ||
			(left->facts->flags & (sema::kOperatorCall | sema::kTypeIdentity)) != 0 || bfloat_operation(*left))
			break;
		node = static_cast<const syntax::BinaryExpr*>(left);
	}
	RValue value = RValue();
	bool known = false;
	for (size_t i = spine_.size(); i-- > base;)
	{
		const syntax::BinaryExpr& current = *spine_[i];
		const RValue* left = known ? &value : nullptr;
		value = IsComparisonOperator(current.op) ? comparison(current, left) : arithmetic(current, left);
		known = true;
	}
	spine_.resize(base);
	return value;
}

RValue FunctionLowering::arithmetic(const syntax::BinaryExpr& expr, const RValue* left_value)
{
	const sema::Type* type = expr.facts->type;
	const sema::ExprFacts* left_facts = WithoutParentheses(expr.left)->facts;
	const sema::ExprFacts* right_facts = WithoutParentheses(expr.right)->facts;
	sema::TypeTable& types = lowering_.graph().types;
	const sema::Type* left_type = DecayedType(types, left_facts->type);
	const sema::Type* right_type = DecayedType(types, right_facts->type);
	if (expr.op == TokenKind::OP_PLUS || expr.op == TokenKind::OP_MINUS)
	{
		const bool left_pointer = IsPointerType(left_type);
		const bool right_pointer = IsPointerType(right_type);
		if (left_pointer && right_pointer)
		{
			// 5.7p6: the difference in elements.
			RValue left = left_value != nullptr ? *left_value : rvalue(*expr.left);
			RValue right = rvalue(*expr.right);
			return pointer_difference(left, right, left_type, expr.location);
		}
		if (left_pointer || right_pointer)
		{
			// 5.7p5: the pointer advanced by the integer's elements.
			RValue left = left_value != nullptr ? *left_value : rvalue(*expr.left);
			RValue right = rvalue(*expr.right);
			if (left_pointer)
				return pointer_offset(left, right, right_type, left_type, expr.op == TokenKind::OP_MINUS,
					IsLiteralExpression(*expr.right), expr.location);
			return pointer_offset(
				right, left, left_type, right_type, false, IsLiteralExpression(*expr.left), expr.location);
		}
	}
	const bool additive = expr.op == TokenKind::OP_PLUS || expr.op == TokenKind::OP_MINUS;
	const ConversionContext context = additive ? ConversionContext::Additive : ConversionContext::Operand;
	RValue left = left_value != nullptr ? *left_value : rvalue(*expr.left);
	RValue right = rvalue(*expr.right);
	// A literal added to a size query's constant folds its widening (the
	// course's immediates convention for sizeof and alignof arithmetic).
	left = apply_conversion(
		*expr.left, left, additive && IsSizeQuery(*expr.right) ? ConversionContext::BesideSize : context);
	right = apply_conversion(
		*expr.right, right, additive && IsSizeQuery(*expr.left) ? ConversionContext::BesideSize : context);
	if (type->unqualified->kind == sema::TypeKind::Complex)
		return complex_arithmetic(expr.op, type, left,
			ConvertedType(types, left_facts)->kind == sema::TypeKind::Complex, right,
			ConvertedType(types, right_facts)->kind == sema::TypeKind::Complex);
	if (type->unqualified->kind == sema::TypeKind::Vector)
		return vector_arithmetic(expr, left, right);
	if (expr.op == TokenKind::OP_LSHIFT || expr.op == TokenKind::OP_RSHIFT)
	{
		// 5.8: the promoted count takes the promoted left operand's type,
		// so the operation has one operand type.
		const sema::Type* count = right_facts->conversion != nullptr && right_facts->conversion->result != nullptr
			? right_facts->conversion->result
			: right_type;
		right = convert_integer(right, count, lowering_.storage_type(type), ConversionContext::Operand, IsSigned(type),
			IsLiteralExpression(*expr.right));
	}
	return arithmetic_op(static_cast<uint32_t>(expr.op), type, left, right);
}

// `nowrap` (lowir.md "Instruction Facts") on a signed add, sub or mul of
// int or a wider type: its overflow is undefined (5p4), so its exact result
// is representable. Narrower operands were promoted to int first. A left
// shift stays unmarked: GCC and clang define a signed shift that overflows.
static uint8_t signed_no_wrap(lowir::BinaryOp code, lowir::Type storage, bool is_unsigned)
{
	const lowir::TypeKind k = storage.kind();
	const bool arithmetic = code == lowir::BinaryOp::Add || code == lowir::BinaryOp::Sub ||
		code == lowir::BinaryOp::Mul;
	return !is_unsigned && arithmetic && (k == lowir::TypeKind::I32 || k == lowir::TypeKind::I64) ? lowir::IF_NO_WRAP
																								  : 0;
}

RValue FunctionLowering::arithmetic_op(uint32_t token, const sema::Type* type, const RValue& left, const RValue& right)
{
	// 5.6-5.12 on the converted operands; the unsigned forms for unsigned
	// types (the LowIR type does not carry signedness).
	const TokenKind op = static_cast<TokenKind>(token);
	const lowir::Type storage = lowering_.storage_type(type);
	const bool soft = storage.is_object() && SoftFloatOf(type) != SoftFloat::None;
	const bool is_unsigned = !storage.is_float() && !soft && !IsSigned(type);
	lowir::BinaryOp code = lowir::BinaryOp::Add;
	switch (op)
	{
	case TokenKind::OP_PLUS:
		code = lowir::BinaryOp::Add;
		break;
	case TokenKind::OP_MINUS:
		code = lowir::BinaryOp::Sub;
		break;
	case TokenKind::OP_STAR:
		code = lowir::BinaryOp::Mul;
		break;
	case TokenKind::OP_DIV:
		code = is_unsigned ? lowir::BinaryOp::UDiv : lowir::BinaryOp::Div;
		break;
	case TokenKind::OP_MOD:
		code = is_unsigned ? lowir::BinaryOp::UMod : lowir::BinaryOp::Mod;
		break;
	case TokenKind::OP_AMP:
		code = lowir::BinaryOp::And;
		break;
	case TokenKind::OP_BOR:
		code = lowir::BinaryOp::Or;
		break;
	case TokenKind::OP_XOR:
		code = lowir::BinaryOp::Xor;
		break;
	case TokenKind::OP_LSHIFT:
		code = lowir::BinaryOp::Shl;
		break;
	case TokenKind::OP_RSHIFT:
		code = is_unsigned ? lowir::BinaryOp::UShr : lowir::BinaryOp::Shr;
		break;
	default:
		throw LoweringError(0, "an operator outside the procedural lowering slice");
	}
	if (soft)
		return soft_arithmetic(code, SoftFloatOf(type), left, right);
	return binary(code, storage, left, right, signed_no_wrap(code, storage, is_unsigned));
}

RValue FunctionLowering::pointer_offset(const RValue& pointer, const RValue& offset, const sema::Type* index_type,
	const sema::Type* pointer_type, bool subtract, bool literal, uint32_t location)
{
	// The integer widened to i64 and scaled by the element size (a byte
	// element needs no scaling), negated for subtraction.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue count = convert_integer(offset, index_type, i64, ConversionContext::Additive, true, literal);
	const uint64_t size = lowering_.size_of(pointer_type->unqualified->element, location);
	if (size != 1)
		count = binary(lowir::BinaryOp::Mul, i64, count, integer(i64, size));
	if (subtract)
	{
		uint64_t bits = 0;
		if (size == 1 && is_integer_literal(count, bits))
			count = integer(i64, 0 - bits);
		else
			count = binary(lowir::BinaryOp::Sub, i64, integer(i64, 0), count);
	}
	return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, pointer.value, count);
}

RValue FunctionLowering::pointer_difference(
	const RValue& left, const RValue& right, const sema::Type* pointer_type, uint32_t location)
{
	// The byte difference is an exact multiple of the element size: a
	// power of two divides by an arithmetic shift, other sizes by a
	// signed division (README: pointer differences) marked exact for the
	// backend (IF_EXACT).
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue bytes = binary(lowir::BinaryOp::Sub, lowir::Type::scalar(lowir::TypeKind::Ptr), left, right);
	bytes.type = i64;
	const uint64_t size = lowering_.size_of(pointer_type->unqualified->element, location);
	if (size == 1)
		return bytes;
	if ((size & (size - 1)) == 0)
	{
		unsigned shift = 0;
		while ((uint64_t(1) << shift) < size)
			++shift;
		return binary(lowir::BinaryOp::Shr, i64, bytes, integer(i64, shift), lowir::IF_EXACT);
	}
	lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Binary, i64);
	inst.sub = static_cast<uint8_t>(lowir::BinaryOp::Div);
	inst.flags = lowir::IF_EXACT;
	inst.ops[0] = bytes.value;
	inst.ops[1] = integer(i64, size).value;
	RValue quotient;
	quotient.value = emit_value(inst, i64);
	quotient.type = i64;
	return quotient;
}

RValue FunctionLowering::comparison(const syntax::BinaryExpr& expr, const RValue* left_value)
{
	// 5.9, 5.10: the operands converted to their common type; pointers
	// compare as unsigned addresses.
	// A pointer converted to its base is adjusted (null kept, 4.10p3)
	// before the right operand is evaluated (the course's order).
	const sema::ExprFacts* left_facts = WithoutParentheses(expr.left)->facts;
	const bool base_pointer = left_facts->conversion != nullptr &&
		left_facts->conversion->second == sema::ValueConversion::DerivedToBase &&
		left_facts->conversion->form != sema::ConversionForm::Reference && left_facts->conversion->result != nullptr &&
		IsPointerType(left_facts->conversion->result);
	RValue left = left_value != nullptr ? *left_value : rvalue(*expr.left);
	if (base_pointer)
		left = apply_conversion(*expr.left, left, ConversionContext::Comparison);
	RValue right = rvalue(*expr.right);
	if (!base_pointer)
		left = apply_conversion(*expr.left, left, ConversionContext::Comparison);
	right = apply_conversion(*expr.right, right, ConversionContext::Comparison);
	const sema::Type* type = left_facts->conversion != nullptr && left_facts->conversion->result != nullptr
		? left_facts->conversion->result
		: DecayedType(lowering_.graph().types, left_facts->type);
	if (expr.facts->type->unqualified->kind == sema::TypeKind::Vector)
		return vector_compare(expr, left, right);
	if (type->unqualified->kind == sema::TypeKind::Complex)
		return complex_equal(expr.op == TokenKind::OP_EQ, type, left, right);
	const bool pointer = IsPointerType(type) || IsNullptrType(type);
	const bool is_unsigned = pointer || (!sema::IsFloating(type) && !IsSigned(type));
	lowir::ComparePredicate predicate = lowir::ComparePredicate::Eq;
	switch (expr.op)
	{
	case TokenKind::OP_EQ:
		predicate = lowir::ComparePredicate::Eq;
		break;
	case TokenKind::OP_NE:
		predicate = lowir::ComparePredicate::Ne;
		break;
	case TokenKind::OP_LT:
		predicate = is_unsigned ? lowir::ComparePredicate::Ult : lowir::ComparePredicate::Lt;
		break;
	case TokenKind::OP_LE:
		predicate = is_unsigned ? lowir::ComparePredicate::Ule : lowir::ComparePredicate::Le;
		break;
	case TokenKind::OP_GT:
		predicate = is_unsigned ? lowir::ComparePredicate::Ugt : lowir::ComparePredicate::Gt;
		break;
	default:
		predicate = is_unsigned ? lowir::ComparePredicate::Uge : lowir::ComparePredicate::Ge;
		break;
	}
	if (lowering_.host_objects() && left.type.kind() == lowir::TypeKind::I128 &&
		type->unqualified->kind == sema::TypeKind::MemberPointer)
		return member_pointer_equal(left, right, predicate == lowir::ComparePredicate::Eq);
	if (left.type.is_object() && SoftFloatOf(type) != SoftFloat::None)
		return soft_compare(predicate, SoftFloatOf(type), left, right);
	return compare(
		predicate, pointer ? left.type : ComparisonType(left.type, lowering_.storage_type(type)), left, right);
}

Place FunctionLowering::assignment(const syntax::BinaryExpr& expr)
{
	// 5.17p1: the right operand is evaluated before the left's address.
	const sema::ExprFacts* left_facts = WithoutParentheses(expr.left)->facts;
	const sema::Type* type = left_facts->type;
	if (type->unqualified->kind == sema::TypeKind::Class)
		throw LoweringError(expr.location, "class assignment is outside the procedural lowering slice");
	RValue value;
	if (expr.right->kind == syntax::ExprKind::BracedInit)
	{
		// 5.17p9: `x = {v}` is `x = T{v}`, `x = {}` is `x = T{}`.
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*expr.right);
		value = sema::LaneList(type, list) ? vector_list(type, list)
			: list.elements.empty()        ? zero_value(type)
										   : operand(*list.elements[0], ConversionContext::Assignment);
	}
	else
	{
		// A union member's assignment, and a class template
		// specialization's own non-static data member's, fold a literal's
		// widening as an initialization does (the course's immediates
		// convention, as the reference presents them).
		const sema::Entity* member = left_facts->entity;
		const sema::ClassEntity* home = member != nullptr && member->kind == sema::EntityKind::Variable &&
				static_cast<const sema::VariableEntity*>(member)->member &&
				member->owner->kind == sema::ScopeKind::Class
			? static_cast<const sema::ClassEntity*>(member->owner->entity)
			: nullptr;
		const bool variant = home != nullptr && home->key == TokenKind::KW_UNION;
		const bool instance = home != nullptr && home->templ != nullptr &&
			home->templ->role == sema::TemplateRole::Specialization;
		value = operand(
			*expr.right, variant || instance ? ConversionContext::Initialization : ConversionContext::Assignment);
	}
	value = fit(value, type);
	Place place = glvalue(*expr.left);
	store(lowering_.storage_type(type), value, place);
	place.has_known = !place.is_volatile || place.is_atomic;
	place.known = value;
	return place;
}

Place FunctionLowering::compound_assignment(const syntax::BinaryExpr& expr)
{
	// 5.17p7: E1 op= E2 is E1 = E1 op E2 with E1 evaluated once: its
	// RHS evaluated and converted before the LHS address and read (the
	// course order), and the result converted back to E1's type.
	const sema::ExprFacts* left_facts = WithoutParentheses(expr.left)->facts;
	const sema::ExprFacts* right_facts = WithoutParentheses(expr.right)->facts;
	const sema::Type* left_type = left_facts->type->unqualified;
	const lowir::Type storage = lowering_.storage_type(left_type);
	const TokenKind op = CompoundOperator(expr.op);
	// E2's value (a pointer offset's count, a bool's pointer, an operand
	// in the operation's type), then E1's combined with it.
	const bool pointer = IsPointerType(left_type);
	const bool bool_pointer = !pointer && IsPointerType(expr.facts->target);
	const sema::Type* operation = expr.facts->target;
	const bool bfloat = operation != nullptr && SoftFloatOf(operation) == SoftFloat::BFloat &&
		(op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS || op == TokenKind::OP_STAR || op == TokenKind::OP_DIV);
	RValue right;
	const auto evaluate_right = [&]() {
		if (bfloat)
		{
			right = bfloat_promoted(*expr.right);
			return;
		}
		if (pointer)
		{
			right = rvalue(*expr.right);
			return;
		}
		right = rvalue(*expr.right);
		if (bool_pointer)
		{
			right = apply_conversion(*expr.right, right, ConversionContext::CompoundAssignment);
			return;
		}
		const sema::Type* common = right_facts->conversion != nullptr && right_facts->conversion->result != nullptr
			? right_facts->conversion->result
			: DecayedType(lowering_.graph().types, right_facts->type);
		right = apply_conversion(*expr.right, right, ConversionContext::CompoundAssignment);
		// (A vector's count is already its lane type or vector, as the
		// analysis converted it.)
		if ((op == TokenKind::OP_LSHIFT || op == TokenKind::OP_RSHIFT) &&
			operation->unqualified->kind != sema::TypeKind::Vector)
			right = convert_integer(right, common, lowering_.storage_type(operation),
				ConversionContext::CompoundAssignment, IsSigned(operation), IsLiteralExpression(*expr.right));
	};
	evaluate_right();
	Place place = glvalue(*expr.left);
	// An atomic integer's += -= &= |= ^= in its own type: one atomic
	// read-modify-write (C11 6.5.16.2p3).
	if (place.is_atomic && storage.is_integer() && !IsBoolType(left_type) && expr.facts->target == left_type &&
		(op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS || op == TokenKind::OP_AMP || op == TokenKind::OP_BOR ||
			op == TokenKind::OP_XOR))
	{
		const sema::Intrinsic update = op == TokenKind::OP_PLUS ? sema::Intrinsic::AtomicAddFetch
			: op == TokenKind::OP_MINUS                         ? sema::Intrinsic::AtomicSubFetch
			: op == TokenKind::OP_AMP                           ? sema::Intrinsic::AtomicAndFetch
			: op == TokenKind::OP_BOR                           ? sema::Intrinsic::AtomicOrFetch
																: sema::Intrinsic::AtomicXorFetch;
		place.known = atomic_update(update, storage, place.storage, right, lowir::MemoryOrder::SeqCst);
		place.has_known = true;
		return place;
	}
	const auto combined = [&](const RValue& old) {
		if (pointer)
			return pointer_offset(old, right, DecayedType(lowering_.graph().types, right_facts->type), left_type,
				op == TokenKind::OP_MINUS, IsLiteralExpression(*expr.right), expr.location);
		if (bool_pointer)
		{
			// A bool plus a pointer (5.17p7): the sum's pointer converted
			// back to bool (4.12).
			const RValue sum = pointer_offset(right, old, left_type, operation, false, false, expr.location);
			return convert(sum, operation, left_type, ConversionContext::CompoundAssignment);
		}
		// The operation's type as the analysis recorded it (5.17p7).
		const RValue left = convert(old, left_type, operation, ConversionContext::CompoundAssignment);
		const RValue result = operation->unqualified->kind == sema::TypeKind::Complex
			? complex_arithmetic(op, operation, left, true, right,
				  ConvertedType(lowering_.graph().types, right_facts)->kind == sema::TypeKind::Complex)
			: operation->unqualified->kind == sema::TypeKind::Vector
			? vector_operation(op, operation, left, true, right,
				  ConvertedType(lowering_.graph().types, right_facts)->kind == sema::TypeKind::Vector)
			: arithmetic_op(static_cast<uint32_t>(op), operation, left, right);
		return fit(convert(result, operation, left_type, ConversionContext::CompoundAssignment), left_type);
	};
	// An atomic object's other updates: E2 first, then one read-modify-
	// write (C11 6.5.16.2p3).
	AtomicLoop loop;
	if (place.is_atomic && atomic_bits(storage, loop.bits))
	{
		place.known = combined(begin_atomic_loop(place, storage, loop));
		end_atomic_loop(place, storage, loop, place.known);
		place.has_known = true;
		return place;
	}
	const RValue old = load(storage, place);
	const RValue result = bfloat ? bfloat_compound(expr, op, left_type, old, right) : combined(old);
	store(storage, result, place);
	place.has_known = !place.is_volatile && !place.is_atomic;
	place.known = result;
	return place;
}

RValue FunctionLowering::unary_expression(const syntax::UnaryExpr& expr)
{
	switch (expr.op)
	{
	case TokenKind::OP_AMP:
	{
		if (expr.facts->type != nullptr && expr.facts->type->unqualified->kind == sema::TypeKind::MemberPointer)
			return member_pointer_value(expr);
		// 5.3.1p3: the address of the lvalue (or function).
		RValue pointer;
		pointer.value = address(*expr.operand);
		pointer.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		return pointer;
	}
	case TokenKind::OP_LNOT:
	{
		// 5.3.1p8: `!x` is `x == 0` on the operand's value.
		const RValue value = truth_operand(*expr.operand);
		if (WithoutParentheses(expr.operand)->facts->type->unqualified->kind == sema::TypeKind::MemberPointer)
			return member_pointer_null(value, true);
		const RValue zero = value.type.is_float() ? floating(value.type, lowir::float_literal(value.type.kind(), 0.0L))
												  : integer(value.type, 0);
		return compare(lowir::ComparePredicate::Eq, value.type, value, zero);
	}
	case TokenKind::OP_PLUS:
		if (bfloat_operation(expr))
			return bfloat_arithmetic(expr);
		return operand(*expr.operand, ConversionContext::Unary);
	case TokenKind::OP_MINUS:
	{
		if (bfloat_operation(expr))
			return bfloat_arithmetic(expr);
		const RValue value = operand(*expr.operand, ConversionContext::Unary);
		if (expr.facts->type->unqualified->kind == sema::TypeKind::Complex)
			return complex_unary(expr.op, expr.facts->type, value);
		if (expr.facts->type->unqualified->kind == sema::TypeKind::Vector)
			return vector_unary(expr.op, expr.facts->type, value);
		if (value.type.is_object() && SoftFloatOf(expr.facts->type) != SoftFloat::None)
			return soft_negate(SoftFloatOf(expr.facts->type), value);
		return unary(lowir::UnaryOp::Neg, value);
	}
	case TokenKind::OP_COMPL:
		if (expr.facts->type->unqualified->kind == sema::TypeKind::Complex)
			return complex_unary(expr.op, expr.facts->type, operand(*expr.operand, ConversionContext::Unary));
		if (expr.facts->type->unqualified->kind == sema::TypeKind::Vector)
			return vector_unary(expr.op, expr.facts->type, operand(*expr.operand, ConversionContext::Unary));
		return unary(lowir::UnaryOp::BitNot, operand(*expr.operand, ConversionContext::Unary));
	default:
		throw LoweringError(expr.location, "a unary operator outside the procedural lowering slice");
	}
}

RValue FunctionLowering::stepped(const RValue& value, const sema::Type* type, bool decrement, uint32_t location)
{
	// 5.2.6, 5.3.2: one more or less, in the operand's own type; a pointer
	// by one element.
	type = type->unqualified;
	if (IsPointerType(type))
		return pointer_offset(value, integer(lowir::Type::scalar(lowir::TypeKind::I64), 1),
			lowering_.graph().types.fundamental(FundamentalType::LongInt), type, decrement, true, location);
	if (IsBoolType(type))
		return integer(value.type, 1);
	if (type->kind == sema::TypeKind::Complex)
	{
		// GNU: the real part stepped.
		RValue real, imaginary;
		complex_parts(value, type, real, imaginary);
		return complex_value(type, stepped(real, type->element, decrement, location), imaginary);
	}
	const SoftFloat soft = value.type.is_object() ? SoftFloatOf(type) : SoftFloat::None;
	if (soft != SoftFloat::None)
		return soft_arithmetic(decrement ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add, soft, value,
			soft_constant(soft,
				soft == SoftFloat::Half         ? 0x3c00
					: soft == SoftFloat::BFloat ? 0x3f80
												: 0,
				soft == SoftFloat::Quad ? 0x3fff000000000000ull : 0));
	const RValue one = value.type.is_float() ? floating(value.type, lowir::float_literal(value.type.kind(), 1.0L))
											 : integer(value.type, 1);
	const lowir::BinaryOp code = decrement ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add;
	return binary(
		code, value.type, value, one, value.type.is_float() ? 0 : signed_no_wrap(code, value.type, !IsSigned(type)));
}

Place FunctionLowering::prefix_increment(const syntax::UnaryExpr& expr)
{
	const sema::Type* type = WithoutParentheses(expr.operand)->facts->type;
	const lowir::Type storage = lowering_.storage_type(type);
	Place place = glvalue(*expr.operand);
	if (atomic_step(place, type, storage))
	{
		place.known = atomic_update(
			expr.op == TokenKind::OP_DEC ? sema::Intrinsic::AtomicSubFetch : sema::Intrinsic::AtomicAddFetch, storage,
			place.storage, integer(storage, 1), lowir::MemoryOrder::SeqCst);
		place.has_known = true;
		return place;
	}
	AtomicLoop loop;
	if (place.is_atomic && atomic_bits(storage, loop.bits))
	{
		const RValue old = begin_atomic_loop(place, storage, loop);
		place.known = stepped(old, type, expr.op == TokenKind::OP_DEC, expr.location);
		end_atomic_loop(place, storage, loop, place.known);
		place.has_known = true;
		return place;
	}
	const RValue old = load(storage, place);
	const RValue result = stepped(old, type, expr.op == TokenKind::OP_DEC, expr.location);
	store(storage, result, place);
	place.has_known = !place.is_volatile && !place.is_atomic;
	place.known = result;
	return place;
}

RValue FunctionLowering::postfix_increment(const syntax::UnaryExpr& expr)
{
	const sema::ExprFacts* operand = WithoutParentheses(expr.operand)->facts;
	if (operand != nullptr && (operand->flags & sema::kBitField) != 0)
		return bit_field_increment(expr, false);
	const sema::Type* type = WithoutParentheses(expr.operand)->facts->type;
	const lowir::Type storage = lowering_.storage_type(type);
	const Place place = glvalue(*expr.operand);
	if (atomic_step(place, type, storage))
		return atomic_update(
			expr.op == TokenKind::OP_DEC ? sema::Intrinsic::AtomicFetchSub : sema::Intrinsic::AtomicFetchAdd, storage,
			place.storage, integer(storage, 1), lowir::MemoryOrder::SeqCst);
	AtomicLoop loop;
	if (place.is_atomic && atomic_bits(storage, loop.bits))
	{
		const RValue old = begin_atomic_loop(place, storage, loop);
		end_atomic_loop(place, storage, loop, stepped(old, type, expr.op == TokenKind::OP_DEC, expr.location));
		return old;
	}
	const RValue old = load(storage, place);
	store(storage, stepped(old, type, expr.op == TokenKind::OP_DEC, expr.location), place);
	return old;
}

Place FunctionLowering::subscript(const syntax::SubscriptExpr& expr)
{
	// 5.2.1: *(E1 + E2): the pointer operand first, the index as written.
	if (WithoutParentheses(expr.base)->facts->type->unqualified->kind == sema::TypeKind::Vector)
		return vector_lane_place(expr);
	const bool commuted = (expr.facts->flags & sema::kCommuted) != 0;
	const syntax::Expr& base = commuted ? *expr.index : *expr.base;
	const syntax::Expr& position = commuted ? *expr.base : *expr.index;
	const RValue pointer = rvalue(base);
	const RValue offset = rvalue(position);
	Place place = Place();
	if (expr.facts->type->unqualified->kind == sema::TypeKind::Class || expr.facts->type->kind == sema::TypeKind::Array)
	{
		// A class or array element: the index widened and scaled to a byte
		// offset.
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		const sema::Type* index_type = DecayedType(lowering_.graph().types, WithoutParentheses(&position)->facts->type);
		RValue scaled = convert_integer(
			offset, index_type, i64, ConversionContext::Additive, IsSigned(index_type), IsLiteralExpression(position));
		scaled = binary(
			lowir::BinaryOp::Mul, i64, scaled, integer(i64, lowering_.size_of(expr.facts->type, expr.location)));
		place.storage =
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, pointer.value, scaled).value;
		qualify(place, expr.facts->type);
		return place;
	}
	place.storage =
		index(lowering_.storage_type(expr.facts->type), ir_model::IPK_ARRAY_ELEMENT, pointer.value, offset).value;
	qualify(place, expr.facts->type);
	return place;
}

Place FunctionLowering::member(const syntax::MemberExpr& expr)
{
	// 5.2.5: the member at its offset in the object (E1->E2 is (*E1).E2).
	const sema::Entity* entity = expr.facts->entity;
	// A static member function named through an object (5.2.5p4): the
	// object evaluated, the function designated.
	if (entity != nullptr && entity->kind == sema::EntityKind::Function &&
		static_cast<const sema::FunctionEntity*>(entity)->is_static)
	{
		member_object(expr);
		return variable_place(entity, expr.facts->type, expr.location);
	}
	if (entity == nullptr || entity->kind != sema::EntityKind::Variable)
		throw LoweringError(expr.location, "member functions are outside the procedural lowering slice");
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(entity);
	if (!variable->member)
	{
		member_object(expr);
		return variable_place(variable, expr.facts->type, expr.location);
	}
	const bool arrow = expr.op == TokenKind::OP_ARROW;
	lowir::Value object = arrow ? operand(*expr.base, ConversionContext::Operand).value : object_address(*expr.base);
	const sema::Type* object_type = WithoutParentheses(expr.base)->facts->type;
	if (arrow)
		object_type = object_type->kind == sema::TypeKind::Array ? object_type->element
																 : object_type->unqualified->element;
	const sema::ClassEntity* from = sema::ClassOf(object_type);
	BaseAccess access = from != nullptr && VirtualBaseCount(from) != 0 ? member_base_access(*expr.base, arrow)
																	   : BaseAccess::Table;
	if (const sema::ClassEntity* naming = lowering_.member_naming_class(expr))
	{
		object = to_base(object, from, naming, expr.location, access);
		from = naming;
		access = BaseAccess::Table;
	}
	return member_place(object, from, variable, expr.facts->type, expr.location, access);
}

RValue FunctionLowering::cast(const syntax::Expr& expr, const syntax::Expr& operand_expr)
{
	// 5.2.9, 5.2.10, 5.2.11, 5.4: the operand's value converted to the
	// cast's type (the recorded conversion, or the explicit one's).
	if ((expr.facts->flags & sema::kRuntimeCheck) != 0)
		return dynamic_cast_pointer(static_cast<const syntax::CastExpr&>(expr));
	const sema::Type* type = expr.facts->type;
	if (sema::IsVoid(type))
	{
		discard_to_void(operand_expr);
		return integer(lowir::Type::scalar(lowir::TypeKind::Void), 0);
	}
	const sema::ExprFacts* facts = WithoutParentheses(&operand_expr)->facts;
	const sema::Conversion* conversion = facts->conversion;
	// 5.2.9p4: a class operand's conversion function.
	if (conversion_function(conversion))
		return fit(converted_value(operand_expr, conversion, ConversionContext::Cast), type);
	if (type->unqualified->kind == sema::TypeKind::MemberPointer && conversion != nullptr &&
		(conversion->second == sema::ValueConversion::MemberToDerived || (expr.facts->flags & sema::kBaseCast) != 0))
		return member_pointer_cast(expr, operand_expr);
	RValue value = rvalue(operand_expr);
	if (conversion != nullptr && conversion->second == sema::ValueConversion::NullPointer)
		return apply_conversion(operand_expr, value, ConversionContext::Cast);
	if (conversion != nullptr && conversion->second == sema::ValueConversion::DerivedToBase && IsPointerType(type))
	{
		// 5.2.9p2 (static_cast's standard conversion): the base subobject.
		return pointer_to_base(value, sema::ClassOf(DecayedType(lowering_.graph().types, facts->type)->element),
			sema::ClassOf(type->unqualified->element), expr.location, NonNullPointer(operand_expr),
			!IsConditional(operand_expr));
	}
	if ((expr.facts->flags & sema::kDerivedCast) != 0)
		return pointer_to_derived(value, sema::ClassOf(DecayedType(lowering_.graph().types, facts->type)->element),
			sema::ClassOf(type->unqualified->element), expr.location, NonNullPointer(operand_expr));
	if ((expr.facts->flags & sema::kBaseCast) != 0)
	{
		const sema::ClassEntity* from = sema::ClassOf(DecayedType(lowering_.graph().types, facts->type)->element);
		const sema::ClassEntity* to = sema::ClassOf(type->unqualified->element);
		const ProgramLowering::BaseRoute route = lowering_.base_route(from, to, expr.location);
		if (route.via != nullptr || route.offset != 0)
			return pointer_to_base(value, from, to, expr.location, NonNullPointer(operand_expr));
	}
	// A GNU vector's bytes as another vector or an integer, or the reverse.
	const sema::Type* source = DecayedType(lowering_.graph().types, facts->type)->unqualified;
	if (source != type->unqualified &&
		(source->kind == sema::TypeKind::Vector || type->unqualified->kind == sema::TypeKind::Vector))
		return reinterpret_bytes(fit(value, source), type);
	return fit(convert(value, DecayedType(lowering_.graph().types, facts->type), type, ConversionContext::Cast,
				   IsLiteralExpression(operand_expr)),
		type);
}

// ------------------------------------------------------------------ calls

RValue FunctionLowering::call(const syntax::CallExpr& expr, lowir::Value destination)
{
	// 5.2.2: the arguments in order, each converted to its parameter; an
	// indirect callee after them, with its function type as the boundary.
	// The arguments are pushed on a stack shared by the calls nested in
	// them, which pop their own before an outer argument is pushed. A
	// class result initializes `destination`: through it as the first
	// argument (an indirect result), or copied from the call's value.
	const sema::ExprFacts* facts = expr.facts;
	if ((facts->flags & sema::kPseudoDestructor) != 0)
	{
		// 5.2.4: only the object expression is evaluated.
		const syntax::MemberExpr& access = static_cast<const syntax::MemberExpr&>(*WithoutParentheses(expr.callee));
		if (access.op == TokenKind::OP_ARROW)
			operand(*access.base, ConversionContext::Operand);
		else
			glvalue(*access.base);
		RValue none;
		none.type = lowir::Type::scalar(lowir::TypeKind::Void);
		none.value = lowir::Value::none();
		return none;
	}
	const size_t base = arguments_.size();
	lowir::Value callee;
	lowir::Type result = lowir::Type::scalar(lowir::TypeKind::Void);
	lowir::SignatureId signature = lowir::kNoId;
	const bool throws = (facts->flags & sema::kDirectCall) == 0 ||
		call_may_throw(static_cast<const sema::FunctionEntity*>(facts->entity));
	if ((facts->flags & sema::kDirectCall) != 0 &&
		static_cast<const sema::FunctionEntity*>(facts->entity)->builtin == sema::BuiltinFunction::Expect &&
		expr.arguments.size == 2)
	{
		// GCC's __builtin_expect: its first argument's value (the second,
		// a branch hint, evaluated for its effects).
		const RValue value = fit(operand(*expr.arguments[0], ConversionContext::Argument),
			static_cast<const sema::FunctionEntity*>(facts->entity)->type->canonical->params[0]);
		discard(*expr.arguments[1]);
		return value;
	}
	if ((facts->flags & sema::kDirectCall) != 0 &&
		(static_cast<const sema::FunctionEntity*>(facts->entity)->builtin == sema::BuiltinFunction::Unreachable ||
			static_cast<const sema::FunctionEntity*>(facts->entity)->builtin == sema::BuiltinFunction::Trap))
	{
		// GCC's __builtin_unreachable and __builtin_trap: the block ends in
		// `unreachable` (a trap instruction natively), a trap's after a call
		// of the runtime's trap, which executes however the program is
		// optimized.
		if (static_cast<const sema::FunctionEntity*>(facts->entity)->builtin == sema::BuiltinFunction::Trap)
			emit_call(lowir::Type::scalar(lowir::TypeKind::Void), lowir::Value::symbol(lowering_.trap_function()),
				arguments_.size(), lowir::kNoId, false);
		terminate(lowir::make_instruction(lowir::Opcode::Unreachable, lowir::Type::scalar(lowir::TypeKind::Void)));
		RValue none;
		none.type = lowir::Type::scalar(lowir::TypeKind::Void);
		none.value = lowir::Value::none();
		return none;
	}
	if ((facts->flags & sema::kDirectCall) != 0)
	{
		const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(facts->entity);
		if (function->destructor && function->trivial)
		{
			// 12.4p13: a trivial destructor's explicit call has no effect
			// beyond evaluating its object expression (as a pseudo-
			// destructor call's, 5.2.4).
			const syntax::Expr* callee_expr = WithoutParentheses(expr.callee);
			if (callee_expr->kind == syntax::ExprKind::Member)
			{
				const syntax::MemberExpr& access = static_cast<const syntax::MemberExpr&>(*callee_expr);
				if (access.op == TokenKind::OP_ARROW)
					operand(*access.base, ConversionContext::Operand);
				else
					object_address(*access.base);
			}
			RValue none;
			none.type = lowir::Type::scalar(lowir::TypeKind::Void);
			none.value = lowir::Value::none();
			return none;
		}
		const sema::ClosureFacts* converting = function->conversion
			? lowering_.graph().closure_facts(sema::ClassOf(function->owner->entity->type))
			: nullptr;
		if (converting != nullptr && converting->conversion == function)
		{
			// 5.1.2p6: a captureless closure's conversion function called
			// explicitly yields the invoker's address as an implicit
			// conversion does (call_conversion, the course's form: no call);
			// its object expression is evaluated for its effects.
			const syntax::Expr* callee_expr = WithoutParentheses(expr.callee);
			if (callee_expr->kind == syntax::ExprKind::Member)
			{
				const syntax::MemberExpr& access = static_cast<const syntax::MemberExpr&>(*callee_expr);
				if (access.base->kind != syntax::ExprKind::Lambda && !side_effect_free(access.base, false, 0))
				{
					if (access.op == TokenKind::OP_ARROW)
						operand(*access.base, ConversionContext::Operand);
					else
						object_address(*access.base);
				}
			}
			RValue address;
			address.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
			address.value = function_address(converting->invoker, expr.location);
			return address;
		}
		// 5.2.2p4: a non-static member function's object first; a static
		// one's object expression is evaluated too (9.4p2).
		note_call_arguments(function, expr.arguments);
		begin_call(throws);
		const syntax::Expr* callee_expr = WithoutParentheses(expr.callee);
		const sema::Type* returned = function->type->canonical->element;
		if (lowering_.indirect_result(function))
			arguments_.push_back(destination);
		lowir::Value object;
		const sema::ClosureFacts* closure = function->member && !function->is_static
			? lowering_.graph().closure_facts(sema::ClassOf(function->owner->entity->type))
			: nullptr;
		const bool invoked = closure != nullptr && closure->call == function && closure->invoker != nullptr &&
			callee_expr->kind == syntax::ExprKind::Lambda;
		if (invoked)
		{
			// A captureless lambda-expression called: through the address
			// of the function with its operator's effect (5.1.2p6, the
			// course's form), the arguments first; missing ones are the
			// operator's default arguments (the invoker declares none).
			const sema::FunctionEntity* invoker = closure->invoker;
			arguments(expr.arguments, invoker->type->canonical, function, expr.location);
			callee = function_address(invoker, expr.location);
			result = result_type(invoker);
			signature = lowering_.call_signature(invoker->type, lowering_.function_nothrow(function));
		}
		else if (function->member && !function->is_static)
		{
			object = call_object(expr, function);
			arguments_.push_back(object);
		}
		else if (callee_expr->kind == syntax::ExprKind::Member)
			member_object(static_cast<const syntax::MemberExpr&>(*callee_expr));
		else if (function->member && (facts->flags & sema::kOperatorCall) != 0 &&
			!side_effect_free(callee_expr, false, 0))
		{
			// A static operator() (C++23, hosted): its object is evaluated
			// for its effects, as a static member's access's is (9.4p2).
			discard(*callee_expr);
		}
		if (!invoked)
		{
			arguments(expr.arguments, function->type->canonical, function, expr.location);
			callee = member_callee(function, object, (facts->flags & sema::kVirtualCall) != 0, signature);
			result = result_type(function);
			// The course defines the table of a class with virtual bases a
			// virtual call names its object by (a host object only the
			// tables its code refers to).
			if ((facts->flags & sema::kVirtualCall) != 0 && call_object_class_ != nullptr &&
				VirtualBaseCount(call_object_class_) != 0 && !lowering_.host_objects())
				lowering_.class_table(call_object_class_);
		}
		(void)returned;
	}
	else if ((WithoutParentheses(expr.callee)->facts->flags & sema::kBoundMember) != 0)
	{
		// 5.5p6: through a pointer to member function: the object, the
		// arguments, then the function's address from the pointer.
		const syntax::BinaryExpr& access = static_cast<const syntax::BinaryExpr&>(*WithoutParentheses(expr.callee));
		const sema::Type* member_pointer = WithoutParentheses(access.right)->facts->type;
		const sema::Type* function = member_pointer->unqualified->element->canonical;
		const sema::Type* returned = function->element;
		begin_call(throws);
		if (lowering_.indirect_result(returned, function))
			arguments_.push_back(destination);
		const size_t object_argument = arguments_.size();
		arguments_.push_back(member_pointer_object(access));
		arguments(expr.arguments, function, nullptr, expr.location);
		callee = member_pointer_callee(access, object_argument);
		result = result_type(returned, function);
		if (callee.kind != lowir::ValueKind::Symbol)
			signature = lowering_.member_call_signature(member_pointer, lowering_.callee_nothrow(*expr.callee));
	}
	else if (WithoutParentheses(expr.callee)->facts->type->unqualified->kind == sema::TypeKind::BlockPointer)
		callee = block_call(expr, destination, throws, result, signature);
	else
	{
		const sema::ExprFacts* callee_facts = WithoutParentheses(expr.callee)->facts;
		const sema::Type* function = callee_facts->type->kind == sema::TypeKind::Function
			? callee_facts->type
			: callee_facts->type->unqualified->element;
		const sema::Type* returned = function->canonical->element;
		begin_call(throws);
		if (lowering_.indirect_result(returned, function))
			arguments_.push_back(destination);
		arguments(expr.arguments, function, nullptr, expr.location);
		if (callee_facts->type->kind == sema::TypeKind::Function)
			callee = address(*expr.callee);
		else
			callee = operand(*expr.callee, ConversionContext::Operand).value;
		uint64_t null_bits = 0;
		RValue literal_callee;
		literal_callee.value = callee;
		if (is_integer_literal(literal_callee, null_bits))
		{
			// A constant pointer callee (a null template argument on a path
			// that cannot execute) is called through a temporary: LowIR
			// callees are functions, globals or temporaries.
			callee = constant(lowir::Type::scalar(lowir::TypeKind::Ptr), null_bits).value;
		}
		result = result_type(returned, function);
		signature = lowering_.call_signature(function, lowering_.callee_nothrow(*expr.callee));
	}
	RValue value;
	value.type = result;
	value.value = emit_call(result, callee, base, signature, throws);
	end_call(throws);
	if (result.kind() == lowir::TypeKind::Obj)
		copy_result(value, facts->type, destination, expr.location);
	return value;
}

lowir::Value FunctionLowering::block_call(const syntax::CallExpr& expr, lowir::Value destination, bool throws,
	lowir::Type& result, lowir::SignatureId& signature)
{
	// A call of a block (Clang's blocks ABI): the block literal's invoke
	// function, at offset 16 of the literal (after its isa pointer, flags
	// and reserved words), called with the literal's address before the
	// arguments.
	const sema::Type* block = WithoutParentheses(expr.callee)->facts->type->unqualified;
	const sema::Type* function = block->element->canonical;
	sema::TypeTable& types = lowering_.graph().types;
	std::vector<const sema::Type*> parameters(1, types.pointer(types.fundamental(FundamentalType::Void)));
	parameters.insert(parameters.end(), function->params, function->params + function->param_count);
	const sema::Type* invoke = types.function(function->element, parameters.data(),
		static_cast<uint32_t>(parameters.size()), function->variadic, 0, sema::RefQualifier::None);
	begin_call(throws);
	const RValue literal = operand(*expr.callee, ConversionContext::Operand);
	if (lowering_.indirect_result(function->element, invoke))
		arguments_.push_back(destination);
	arguments_.push_back(literal.value);
	arguments(expr.arguments, function, nullptr, expr.location);
	Place slot = Place();
	slot.storage = element_address(literal.value, 2, 8);
	result = result_type(function->element, invoke);
	signature = lowering_.call_signature(invoke);
	return load(lowir::Type::scalar(lowir::TypeKind::Ptr), slot).value;
}

lowir::Type FunctionLowering::result_type(const sema::FunctionEntity* function)
{
	// A direct call's result, as its callee's convention decides.
	const sema::Type* returned = function->type->canonical->element;
	if (!returned->is_reference() && lowering_.indirect_result(function))
		return lowir::Type::scalar(lowir::TypeKind::Void);
	return result_type(returned, function->type);
}

lowir::Type FunctionLowering::result_type(const sema::Type* returned, const sema::Type* function)
{
	// A call's LowIR result: a reference's address, a class returned as its
	// bytes (none for an indirect result), a scalar's value.
	if (returned->is_reference())
		return lowir::Type::scalar(lowir::TypeKind::Ptr);
	if (lowering_.indirect_result(returned, function))
		return lowir::Type::scalar(lowir::TypeKind::Void);
	return lowering_.storage_type(returned);
}

void FunctionLowering::copy_result(
	const RValue& value, const sema::Type* type, lowir::Value destination, uint32_t location)
{
	// A class returned as its bytes: copied into its destination (an empty
	// class has none). A binary16 or binary128 result is a value.
	if (destination.kind == lowir::ValueKind::None && ObjectValueType(type))
		return;
	if (destination.kind == lowir::ValueKind::None)
		throw LoweringError(location, "a class result without a destination");
	const sema::ClassEntity* entity = sema::ClassOf(type);
	if (entity != nullptr && entity->empty)
		return;
	copy_object(type, value.value, destination, location);
}

lowir::Value FunctionLowering::call_object(const syntax::CallExpr& expr, const sema::FunctionEntity* function)
{
	// The object is `*this` for an implicit member call (9.3.1p3), else the
	// member access's operand: E1.f() names E1, E1->f() is (*E1).f().
	const syntax::Expr* callee = WithoutParentheses(expr.callee);
	const sema::ClassEntity* target = sema::ClassOf(function->owner->entity->type);
	call_object_class_ = nullptr;
	if ((expr.facts->flags & sema::kOperatorCall) != 0)
	{
		// 13.3.1.1.2: the class object called is operator()'s object.
		return to_base(object_address(*callee), sema::ClassOf(callee->facts->type), target, expr.location);
	}
	if ((callee->facts->flags & sema::kImplicitThis) != 0)
	{
		const sema::ClassEntity* current = object_class();
		call_object_class_ = current;
		lowir::Value object = object_pointer(expr.location);
		if (current->anonymous)
		{
			// From an anonymous class's own constructor (a member's default
			// member initializer): through the named class holding its object.
			uint64_t anonymous = 0;
			object = anonymous_object(
				object, current, lowering_.anonymous_home(current, anonymous, expr.location), expr.location);
		}
		if (const sema::ClassEntity* naming = lowering_.call_naming_class(expr))
		{
			object = to_base(object, current, naming, expr.location);
			current = naming;
		}
		return to_base(object, current, target, expr.location);
	}
	if (callee->kind != syntax::ExprKind::Member)
		throw LoweringError(expr.location, "a member function call without an object");
	const syntax::MemberExpr& access = static_cast<const syntax::MemberExpr&>(*callee);
	const sema::Type* object_type = WithoutParentheses(access.base)->facts->type;
	lowir::Value object;
	if (access.op == TokenKind::OP_ARROW)
	{
		object = operand(*access.base, ConversionContext::Operand).value;
		object_type = object_type->kind == sema::TypeKind::Array ? object_type->element
																 : object_type->unqualified->element;
	}
	else
		object = object_address(*access.base);
	const sema::ClassEntity* from = sema::ClassOf(object_type);
	call_object_class_ = from;
	if (const sema::ClassEntity* naming = lowering_.member_naming_class(access))
	{
		object = to_base(object, from, naming, expr.location);
		from = naming;
	}
	return to_base(object, from, target, expr.location);
}

lowir::Value FunctionLowering::object_address(const syntax::Expr& expr)
{
	// A member call's object: a glvalue's address, or a class prvalue
	// materialized in a temporary of the frame (12.2p1).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->facts->category != sema::ValueCategory::PRValue)
		return address(*e);
	return temporary_object(*e);
}

RValue FunctionLowering::operator_call(const syntax::Expr& expr, lowir::Value destination)
{
	// 13.3.1.2: the operator function the analysis selected, called with
	// the operands in order: a member's object (its address) first, then
	// the arguments initializing the parameters; a postfix operator's int
	// parameter takes zero (13.5.7).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::FunctionEntity* function = static_cast<const sema::FunctionEntity*>(e->facts->entity);
	const syntax::Expr* operands[2] = { nullptr, nullptr };
	uint32_t count = 0;
	bool postfix = false;
	switch (e->kind)
	{
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
		operands[0] = static_cast<const syntax::BinaryExpr*>(e)->left;
		operands[1] = static_cast<const syntax::BinaryExpr*>(e)->right;
		count = 2;
		break;
	case syntax::ExprKind::Subscript:
		operands[0] = static_cast<const syntax::SubscriptExpr*>(e)->base;
		operands[1] = static_cast<const syntax::SubscriptExpr*>(e)->index;
		count = 2;
		break;
	case syntax::ExprKind::Postfix:
		postfix = true;
		// fall through
	case syntax::ExprKind::Unary:
		operands[0] = static_cast<const syntax::UnaryExpr*>(e)->operand;
		count = 1;
		break;
	default:
		throw LoweringError(e->location, "an operator call outside the supported forms");
	}
	const sema::Type* type = function->type->canonical;
	const bool member = function->member && !function->is_static;
	const size_t base = arguments_.size();
	const bool throws = call_may_throw(function);
	begin_call(throws);
	if (lowering_.indirect_result(function))
		arguments_.push_back(destination);
	uint32_t first = 0;
	lowir::Value object_value;
	if (member)
	{
		const syntax::Expr* object = WithoutParentheses(operands[0]);
		object_value = to_base(object_address(*object), sema::ClassOf(object->facts->type),
			sema::ClassOf(function->owner->entity->type), e->location);
		arguments_.push_back(object_value);
		first = 1;
	}
	for (uint32_t i = first; i < count; ++i)
		push_argument(*operands[i], type->params[i - first]);
	if (postfix)
		arguments_.push_back(integer(lowir::Type::scalar(lowir::TypeKind::I32), 0).value);
	lowir::SignatureId signature = lowir::kNoId;
	const lowir::Value callee = member_callee(
		function, object_value, (e->facts->flags & sema::kVirtualCall) != 0, signature);
	const lowir::Type result = result_type(function);
	RValue value;
	value.type = result;
	value.value = emit_call(result, callee, base, signature, throws);
	end_call(throws);
	if (result.kind() == lowir::TypeKind::Obj)
		copy_result(value, e->facts->type, destination, e->location);
	return value;
}

void FunctionLowering::arguments(const syntax::Slice<syntax::Expr*>& written, const sema::Type* function,
	const sema::FunctionEntity* callee, uint32_t location)
{
	// Each argument initializes its parameter (5.2.2p4): a reference binds
	// to it; `...` arguments are promoted (5.2.2p7). Missing trailing
	// arguments are the callee's default arguments (8.3.6).
	const uint32_t given = written.size;
	const uint32_t count = function->param_count > given ? function->param_count : given;
	for (uint32_t i = 0; i < count; ++i)
	{
		const syntax::Expr* argument = i < given ? written[i] : nullptr;
		if (argument == nullptr)
		{
			if (callee == nullptr || callee->default_arguments == nullptr || callee->default_arguments[i] == nullptr)
				throw LoweringError(location, "a call without an argument for a parameter");
			push_default_argument(callee, i, function->params[i], location);
		}
		else
			push_argument(*argument, i < function->param_count ? function->params[i] : nullptr);
	}
}

void FunctionLowering::push_default_argument(
	const sema::FunctionEntity* callee, uint32_t index, const sema::Type* parameter, uint32_t location)
{
	// (An implicitly defined constructor's work has no position: 0.)
	const uint32_t saved = source_site_;
	if (source_site_ == kNoSourceSite && location != 0)
		source_site_ = location;
	push_argument(*lowering_.default_argument(callee, index), parameter);
	source_site_ = saved;
}

void FunctionLowering::push_argument(const syntax::Expr& argument, const sema::Type* parameter)
{
	push_converted(argument, parameter, WithoutParentheses(&argument)->facts->conversion);
}

void FunctionLowering::push_converted(
	const syntax::Expr& argument, const sema::Type* parameter, const sema::Conversion* conversion)
{
	// One argument initializing its parameter (null for `...`) as
	// `conversion` says.
	if (parameter != nullptr && parameter->is_reference())
		arguments_.push_back(bind_converted(argument, conversion, parameter, ConversionContext::Argument, false));
	else if (parameter != nullptr && parameter->unqualified->kind == sema::TypeKind::Class)
	{
		// 5.2.2p4: a class parameter is copy-initialized in a temporary of
		// the caller, passed as its bytes (the slot itself) or its address.
		// One whose destructor runs is passed by address and destroyed by
		// the caller (Itanium 3.1.2.3): a temporary of the call's
		// full-expression (12.2p3), whose region creates_temporaries has
		// opened (a static initializer has none: an exception there
		// terminates), or of the implicit call's own (a declaration's or
		// member's default construction, 1.9p10), as a default argument's
		// temporaries are (an array element's end with the element, see
		// begin_element_temporaries).
		const bool caller = caller_destroyed(parameter);
		if (caller)
			temporary_region();
		const lowir::SlotKind kind = caller || !lowering_.passed_directly(sema::ClassOf(parameter))
			? lowir::SlotKind::Arg
			: lowir::SlotKind::Argobj;
		const lowir::SlotId slot = caller ? argument_slot(lowering_.storage_type(parameter), kind)
										  : generated_slot(kind, lowering_.storage_type(parameter));
		Place temporary = Place();
		temporary.storage = lowir::Value::slot(slot);
		const lowir::Value object = address_of(temporary);
		if (caller)
			region_event();
		const sema::ClassEntity* target = sema::ClassOf(parameter);
		// An empty prvalue of the parameter's class is not zeroed (a derived
		// one sliced into it is; so is one a nested expression makes, such
		// as a call's object).
		const bool saved = passing_empty_;
		const syntax::Expr* source = WithoutParentheses(&argument);
		passing_empty_ = source->kind == syntax::ExprKind::FunctionalCast && source->facts->type != nullptr &&
			sema::ClassOf(source->facts->type) == target;
		initialize_from(argument, conversion, object, argument.location);
		passing_empty_ = saved;
		// The caller destroys it with its full-expression's temporaries (a
		// sharing array element's by its slot, addressed again: its handler
		// serves the next elements, see begin_element_temporaries).
		if (caller)
			temporary_alive(element_sharing_ && element_scopes_ == 1 ? temporary.storage : object, parameter);
		arguments_.push_back(caller || !lowering_.passed_directly(target) ? object : lowir::Value::slot(slot));
	}
	else if (parameter == nullptr && WithoutParentheses(&argument)->facts->type != nullptr &&
		WithoutParentheses(&argument)->facts->type->unqualified->kind == sema::TypeKind::Class &&
		!conversion_function(conversion))
	{
		// A class object passed to `...` (5.2.2p7): a temporary copy made
		// by the caller, passed by its address (the course's form). In
		// object mode one a parameter takes as its bytes crosses as its
		// bytes, classified as such a parameter (psABI 3.2.3, as host
		// compilers pass it).
		const sema::Type* type = WithoutParentheses(&argument)->facts->type->unqualified;
		Place temporary = Place();
		temporary.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Arg, lowering_.storage_type(type)));
		const lowir::Value object = address_of(temporary);
		initialize_from(argument, conversion, object, argument.location);
		if (lowering_.object_mode() && lowering_.passed_directly(sema::ClassOf(type)))
		{
			while (!variadic_classes_.empty() && variadic_classes_.back().first >= arguments_.size())
				variadic_classes_.pop_back();
			variadic_classes_.push_back(std::make_pair(arguments_.size(), lowering_.object_classes(type)));
			arguments_.push_back(temporary.storage);
		}
		else
			arguments_.push_back(object);
	}
	else
	{
		RValue value = conversion_function(conversion)
			? converted_value(argument, conversion, ConversionContext::Argument)
			: apply_converted(argument, conversion, rvalue(argument), ConversionContext::Argument);
		if (parameter != nullptr)
			value = fit(value, parameter);
		// A binary16, binary128, complex or vector value or a member
		// function pointer passed to `...` (5.2.2p7 promotes none of them):
		// the classes of a parameter of its type (object mode, psABI 3.2.3;
		// the course's arguments to `...` state none).
		const uint8_t classes = parameter == nullptr && lowering_.object_mode() &&
				(value.type.is_object() || value.type.kind() == lowir::TypeKind::I128)
			? lowering_.object_classes(conversion != nullptr && conversion->target != nullptr
					  ? conversion->target
					  : WithoutParentheses(&argument)->facts->type)
			: 0;
		if (classes != 0)
		{
			while (!variadic_classes_.empty() && variadic_classes_.back().first >= arguments_.size())
				variadic_classes_.pop_back();
			variadic_classes_.push_back(std::make_pair(arguments_.size(), classes));
		}
		// An i128 (a member function pointer, an __int128) passed to `...`
		// crosses as an object of its bytes, as such a parameter does (a
		// literal argument there being an i64; object mode).
		if (parameter == nullptr && value.type.kind() == lowir::TypeKind::I128 && lowering_.object_mode())
			value.value = variadic_wide_object(value, WithoutParentheses(&argument)->facts->type);
		arguments_.push_back(value.value);
	}
}

lowir::Value FunctionLowering::temporary_storage(const sema::Type* type, bool persistent, lowir::SlotKind kind)
{
	// A temporary's storage: a frame slot, or (persistent: bound to a
	// reference of an object with static storage duration, 12.2p5) an
	// internal global of its own.
	if (persistent)
		return lowir::Value::symbol(lowering_.static_temporary(type));
	return lowir::Value::slot(generated_slot(kind, lowering_.storage_type(type)));
}

void FunctionLowering::extension_path(const syntax::Expr* expr)
{
	// 12.2p5: the class prvalue (or array prvalue: `A{...}`, or a braced
	// list initializing a reference to an array's temporary, 8.5.4p3) a
	// reference binds, or whose subobject it binds through member access
	// (`.` or `.*` of a data member), an array member's subscript, a cast
	// to a reference type designating it or a base (5.2.9p2-p3, 5.4p4),
	// either glvalue operand of a conditional, or a comma's right operand.
	// A reference bound to a temporary its initialization makes (a
	// converting constructor's or conversion function's result, or a
	// converted scalar, 8.5.3p5) binds none of the expression's own.
	const sema::Conversion* binding = WithoutParentheses(expr)->facts->conversion;
	if (binding != nullptr &&
		(binding->constructor != nullptr || (binding->form == sema::ConversionForm::Reference && binding->temporary)))
		return;
	for (;;)
	{
		const syntax::Expr* e = WithoutParentheses(expr);
		const sema::ExprFacts* facts = e->facts;
		if (facts->category == sema::ValueCategory::PRValue && !facts->type->is_reference() &&
			(sema::ClassOf(facts->type) != nullptr ||
				(facts->type->kind == sema::TypeKind::Array &&
					(e->kind == syntax::ExprKind::FunctionalCast || e->kind == syntax::ExprKind::BracedInit))))
		{
			persistent_path_.push_back(e);
			return;
		}
		if (e->kind == syntax::ExprKind::Member && static_cast<const syntax::MemberExpr*>(e)->op == TokenKind::OP_DOT &&
			(facts->flags & sema::kOperatorCall) == 0)
			expr = static_cast<const syntax::MemberExpr*>(e)->base;
		else if (e->kind == syntax::ExprKind::Binary &&
			static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_DOTSTAR &&
			(facts->flags & (sema::kOperatorCall | sema::kBoundMember)) == 0)
			expr = static_cast<const syntax::BinaryExpr*>(e)->left;
		else if (e->kind == syntax::ExprKind::Subscript && (facts->flags & sema::kOperatorCall) == 0 &&
			ArrayOperand(static_cast<const syntax::SubscriptExpr*>(e)) != nullptr)
			expr = ArrayOperand(static_cast<const syntax::SubscriptExpr*>(e));
		else if ((facts->flags & sema::kReferenceCast) != 0 && facts->target != nullptr &&
			facts->target->is_reference() && ReferenceCastOperand(e) != nullptr &&
			BindsDirectly(WithoutParentheses(ReferenceCastOperand(e))))
			expr = ReferenceCastOperand(e);
		else if (e->kind == syntax::ExprKind::Conditional && facts->category != sema::ValueCategory::PRValue)
		{
			extension_path(static_cast<const syntax::ConditionalExpr*>(e)->then_expr);
			expr = static_cast<const syntax::ConditionalExpr*>(e)->else_expr;
		}
		else if (e->kind == syntax::ExprKind::Binary &&
			static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA &&
			(facts->flags & sema::kOperatorCall) == 0)
			expr = static_cast<const syntax::BinaryExpr*>(e)->right;
		else
			return;
	}
}

void FunctionLowering::persistent_alive(lowir::Value storage, const sema::Type* type)
{
	// A persistent class temporary is destroyed at program end if it was
	// constructed (3.6.3p1): its flag set now. A local static's temporary
	// (12.2p5), constructed on the first pass through its declaration,
	// has its destruction registered then, as the object's own is; so has
	// every one in object mode (see ProgramLowering::set_object_mode).
	if (!storage.is(lowir::ValueKind::Symbol) || !needs_destruction(type))
		return;
	// A temporary g++ may also define (begin_owned_temporaries) has its
	// destruction registered once among the units, by GCC's guard.
	const lowir::SymbolId registered = static_local_ ? lowir::kNoId
													 : lowering_.temporary_registration_guard(storage.index);
	if (registered != lowir::kNoId)
	{
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		Place flag = Place();
		flag.storage = lowir::Value::symbol(registered);
		const lowir::BlockId run = new_block(lowir::BlockKind::LocalStaticCtorRun);
		const lowir::BlockId done = new_block(lowir::BlockKind::LocalStaticCtorDone);
		branch(compare(lowir::ComparePredicate::Ne, i64, load(i64, flag), integer(i64, 0)), done, run);
		place(run);
		store(i64, integer(i64, 1), flag);
		register_destruction(storage.index, type, 0);
		jump(done);
		place(done);
		return;
	}
	if (static_temporary(storage.index, type, 0))
		return;
	if (static_local_ || lowering_.object_mode())
		return register_destruction(storage.index, type, 0);
	const lowir::SymbolId flag = lowering_.static_temporary_constructed(storage.index, type, 0);
	Place place = Place();
	place.storage = lowir::Value::symbol(flag);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	store(i8, integer(i8, 1), place);
}

lowir::Value FunctionLowering::scalar_temporary(const syntax::Expr& expr, lowir::Value chosen, const sema::Type* source,
	lowir::Type storage, const sema::Type* type, bool persistent, lowir::SlotKind kind)
{
	// A reference's scalar temporary: a conditional's own result slot when
	// its value is stored unconverted (the course's form), else storage of
	// its own.
	if (!persistent && chosen.is(lowir::ValueKind::Slot) &&
		WithoutParentheses(&expr)->kind == syntax::ExprKind::Conditional && lowering_.storage_type(source) == storage)
		return chosen;
	return temporary_storage(type, persistent, kind);
}

lowir::Value FunctionLowering::bind_reference(
	const syntax::Expr& expr, const sema::Type* reference, ConversionContext context, bool persistent)
{
	return bind_converted(expr, WithoutParentheses(&expr)->facts->conversion, reference, context, persistent);
}

const sema::Type* FunctionLowering::binding_temporary(const syntax::Expr& expr)
{
	// The class of the temporary a reference's binding makes itself (a
	// converting constructor's, a conversion function's class result,
	// 8.5.3p5) when it is destroyed, else null.
	const sema::Conversion* binding = WithoutParentheses(&expr)->facts->conversion;
	if (conversion_function(binding))
	{
		const sema::Type* returned = binding->constructor->type->canonical->element;
		return !returned->is_reference() && destroyed_temporary(returned) ? returned : nullptr;
	}
	if (binding != nullptr && binding->constructor != nullptr && destroyed_temporary(binding->result))
		return binding->result;
	// (A reference cast's operand's, which the cast designates.)
	const syntax::Expr* operand = binding == nullptr || (binding->constructor == nullptr && !binding->temporary)
		? CastTemporaryOperand(WithoutParentheses(&expr))
		: nullptr;
	return operand != nullptr ? binding_temporary(*operand) : nullptr;
}

lowir::Value FunctionLowering::bind_extended(
	const syntax::Expr& expr, const sema::Type* reference, const sema::Entity* owner)
{
	// 12.2p5: the temporary a local reference (or a local aggregate's
	// reference member) is bound to, or whose subobject it is bound to,
	// lives until `owner`'s scope ends: its cleanup is the owner's. Other
	// temporaries end with the full-expression.
	const size_t mark = persistent_path_.size();
	const sema::Type* own = binding_temporary(expr);
	if (own == nullptr)
		extension_path(&expr);
	const sema::Entity* saved = scope_extension_;
	const syntax::Expr* saved_root = extension_root_;
	scope_extension_ = owner;
	extension_root_ = &expr;
	const lowir::Value address = bind_reference(expr, reference, ConversionContext::Initialization);
	scope_extension_ = saved;
	extension_root_ = saved_root;
	persistent_path_.resize(mark);
	// The binding's own temporary is the last one it made.
	if (own != nullptr)
		extend_temporary(owner);
	return address;
}

void FunctionLowering::extend_temporary(const sema::Entity* owner)
{
	cleanups_.back().temporary = false;
	cleanups_.back().variable = owner;
}

bool FunctionLowering::extends_temporary(const syntax::Expr* expr)
{
	// Whether a reference initialized from the expression extends a
	// temporary that is destroyed (bind_extended's).
	if (binding_temporary(*expr) != nullptr)
		return true;
	const size_t mark = persistent_path_.size();
	extension_path(expr);
	bool found = false;
	for (size_t i = mark; i < persistent_path_.size() && !found; ++i)
		found = destroyed_temporary(persistent_path_[i]->facts->type) || destroyed_list_array(*persistent_path_[i]);
	persistent_path_.resize(mark);
	return found;
}

bool FunctionLowering::aggregate_extends(const syntax::Expr* list)
{
	// Whether an aggregate's braced initializer may bind a reference member
	// to a temporary that is destroyed: an element (of any nested list)
	// bound as a reference that extends one, or a std::initializer_list
	// object (the list itself, or a member's) whose array is destroyed
	// (8.5.4p6). A work list.
	std::vector<const syntax::Expr*>& work = throw_scan_;
	const size_t base = work.size();
	work.push_back(list);
	bool found = false;
	while (!found && work.size() > base)
	{
		const syntax::Expr* e = WithoutParentheses(work.back());
		work.pop_back();
		if (e->kind == syntax::ExprKind::BracedInit && list_object(*e) != nullptr)
		{
			found = destroyed_list_array(*e);
			continue;
		}
		if (e->kind == syntax::ExprKind::BracedInit)
		{
			for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(e)->elements)
				work.push_back(element);
			continue;
		}
		const sema::Conversion* binding = e->facts != nullptr ? e->facts->conversion : nullptr;
		found = binding != nullptr && binding->form == sema::ConversionForm::Reference && extends_temporary(e);
	}
	work.resize(base);
	return found;
}

lowir::Value FunctionLowering::bind_converted(const syntax::Expr& expr, const sema::Conversion* conversion,
	const sema::Type* reference, ConversionContext context, bool persistent)
{
	// 8.5.3: a reference binds to the glvalue itself, or to a temporary
	// holding the converted value (12.2p5). A scalar temporary's slot is
	// named for what binds it (the course's spellings).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	const lowir::SlotKind reference_kind = context == ConversionContext::Argument ? lowir::SlotKind::Refarg
		: context == ConversionContext::Return                                    ? lowir::SlotKind::Retref
																				  : lowir::SlotKind::Tmpref;
	if (conversion_function(conversion))
		return converted_reference(expr, conversion, reference, persistent);
	// 8.5.3p5: an array prvalue binds a reference to an array directly,
	// materialized (12.2p1).
	if (facts->category == sema::ValueCategory::PRValue && facts->type->kind == sema::TypeKind::Array &&
		reference->element->kind == sema::TypeKind::Array && e->kind == syntax::ExprKind::FunctionalCast &&
		static_cast<const syntax::FunctionalCastExpr*>(e)->braced != nullptr)
		return array_temporary(*static_cast<const syntax::FunctionalCastExpr*>(e), persistent,
			context == ConversionContext::Argument ? lowir::SlotKind::Argarr : lowir::SlotKind::Arraytmp);
	if (facts->type != nullptr && facts->type->kind == sema::TypeKind::Array &&
		e->kind == syntax::ExprKind::BracedInit && reference->element->kind == sema::TypeKind::Array)
		return array_temporary(*e, *static_cast<const syntax::BracedInitList*>(e), persistent,
			context == ConversionContext::Argument ? lowir::SlotKind::Argarr : lowir::SlotKind::Arraytmp);
	if (conversion != nullptr && conversion->constructor != nullptr)
	{
		// 8.5.3p5, 13.3.3.1.2: a temporary of the referenced class made by
		// the converting constructor.
		const bool destroyed = !persistent && destroyed_temporary(conversion->result);
		if (destroyed)
			temporary_region();
		Place temporary = Place();
		temporary.storage = temporary_storage(conversion->result, persistent,
			context == ConversionContext::Argument ? lowir::SlotKind::Arg : lowir::SlotKind::Tmpobj);
		const lowir::Value object = address_of(temporary);
		open_deferred_region();
		call_constructor(conversion->constructor, object, expr, conversion->argument, expr.location);
		if (destroyed)
			temporary_alive(object, conversion->result);
		else if (persistent)
			persistent_alive(temporary.storage, conversion->result);
		return object;
	}
	if (conversion != nullptr && conversion->form == sema::ConversionForm::Reference && conversion->temporary)
	{
		const sema::Type* object = conversion->result != nullptr ? conversion->result : reference->element;
		conditional_slot_ = lowir::Value::none();
		RValue value = rvalue(expr);
		const lowir::Value chosen = conditional_slot_;
		value = fit(
			convert_implicit(value, DecayedType(lowering_.graph().types, facts->type), object, context, expr.location),
			object);
		const lowir::Type storage = lowering_.storage_type(object);
		Place temporary = Place();
		temporary.storage = scalar_temporary(expr, chosen, facts->type, storage, object, persistent, reference_kind);
		store(storage, value, temporary);
		return address_of(temporary);
	}
	if (facts->category == sema::ValueCategory::PRValue && facts->type->unqualified->kind == sema::TypeKind::Class)
	{
		// 8.5.3p5: a class prvalue binds directly, materialized (12.2p1).
		const lowir::Value object = temporary_object(
			expr, persistent, context == ConversionContext::Argument ? lowir::SlotKind::Arg : lowir::SlotKind::Tmpobj);
		if (conversion != nullptr && conversion->second == sema::ValueConversion::DerivedToBase)
			return to_base(object, sema::ClassOf(facts->type), sema::ClassOf(reference->element), expr.location,
				BaseAccess::Complete);
		return object;
	}
	if (facts->category == sema::ValueCategory::PRValue && facts->type->kind != sema::TypeKind::Function)
	{
		conditional_slot_ = lowir::Value::none();
		RValue value = rvalue(expr);
		const lowir::Value chosen = conditional_slot_;
		const lowir::Type storage = lowering_.storage_type(reference->element);
		value = fit(value, reference->element);
		Place temporary = Place();
		temporary.storage = scalar_temporary(
			expr, chosen, facts->type, storage, reference->element, persistent, reference_kind);
		store(storage, value, temporary);
		return address_of(temporary);
	}
	// 8.5.3p5: a reference to a base class binds to the base subobject. A
	// reference cast's temporary (its operand's binding's) persists as one
	// the reference bound itself would (12.2p5; as g++ and clang).
	const syntax::Expr* made = CastTemporaryOperand(e);
	const lowir::Value bound = made != nullptr
		? bind_reference(*made, facts->target, ConversionContext::Cast, persistent)
		: address(expr);
	// A complete object's virtual base is at its offset there (a named
	// object's, a temporary's).
	if (conversion != nullptr && conversion->second == sema::ValueConversion::DerivedToBase)
		return to_base(bound, sema::ClassOf(facts->type), sema::ClassOf(reference->element), expr.location,
			made == nullptr && VirtualBaseCount(sema::ClassOf(facts->type)) != 0 ? member_base_access(expr, false)
																				 : BaseAccess::Table);
	return bound;
}

} // namespace lower
