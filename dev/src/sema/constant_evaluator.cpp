#include "sema/constant_evaluator.h"

#include <cmath>
#include <cstring>
#include <string>

#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "sema/analyzer.h"
#include "sema/constant_interpreter.h"
#include "sema/integral_arithmetic.h"
#include "sema/string_literals.h"

namespace sema {

namespace {

// A class member access's object expression is evaluated (5.2.5p1): an
// enumerator reached through one is a constant expression only when that
// evaluation designates an object without effects (names, `this`, member
// accesses and subscripts of such, 5.19p2).
bool DesignatesWithoutEffects(const syntax::Expr* expr, uint32_t depth)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	if (depth > 64)
		return false;
	switch (expr->kind)
	{
	case syntax::ExprKind::Id:
	case syntax::ExprKind::Literal:
	case syntax::ExprKind::KeywordLiteral:
		return true;
	case syntax::ExprKind::Member:
		return DesignatesWithoutEffects(static_cast<const syntax::MemberExpr*>(expr)->base, depth + 1);
	case syntax::ExprKind::Subscript:
		return DesignatesWithoutEffects(static_cast<const syntax::SubscriptExpr*>(expr)->base, depth + 1) &&
			DesignatesWithoutEffects(static_cast<const syntax::SubscriptExpr*>(expr)->index, depth + 1);
	default:
		return false;
	}
}

IntegralKind KindOf(const Type* type)
{
	IntegralKind kind = IntegralKind{ 32, true };
	IntegralKindOf(type, kind);
	return kind;
}

int64_t Minimum(unsigned bits)
{
	return bits == 64 ? INT64_MIN : -(int64_t(1) << (bits - 1));
}

bool IsComparison(TokenKind op)
{
	return op == TokenKind::OP_EQ || op == TokenKind::OP_NE || op == TokenKind::OP_LT || op == TokenKind::OP_GT ||
		op == TokenKind::OP_LE || op == TokenKind::OP_GE;
}

// The single expression of a copy-, direct- or list-initializer.
const syntax::Expr* SingleInitializer(const syntax::Initializer* initializer)
{
	if (initializer == nullptr)
		return nullptr;
	switch (initializer->kind)
	{
	case syntax::InitializerKind::Assign:
		if (initializer->expression->kind == syntax::ExprKind::BracedInit)
		{
			const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(initializer->expression);
			return list->elements.size == 1 ? list->elements[0] : nullptr;
		}
		return initializer->expression;
	case syntax::InitializerKind::Braced:
	{
		const syntax::BracedInitList* list = static_cast<const syntax::BracedInitList*>(initializer->expression);
		return list->elements.size == 1 ? list->elements[0] : nullptr;
	}
	case syntax::InitializerKind::Parenthesized:
		return initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr;
	default:
		return nullptr;
	}
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

} // namespace

ConstantEvaluator::ConstantEvaluator(Analyzer& analyzer, Scope* scope)
	: analyzer_(analyzer), scope_(scope), skipping_(0), failure_location_(0), failure_(nullptr)
{}

bool ConstantEvaluator::fail(uint32_t location, const char* why)
{
	if (failure_ == nullptr)
	{
		failure_location_ = location;
		failure_ = why;
	}
	return false;
}

Constant ConstantEvaluator::boolean(bool value) const
{
	return Constant{ value ? 1u : 0u, analyzer_.types_.fundamental(FundamentalType::Bool) };
}

bool ConstantEvaluator::evaluate(const syntax::Expr& expr, Constant& value)
{
	return eval(expr, value);
}

bool ConstantEvaluator::type_of(const syntax::Expr& expr, const Type*& type)
{
	++skipping_;
	Constant value;
	const bool typed = eval(expr, value);
	--skipping_;
	if (typed)
		type = value.type;
	return typed;
}

void ConstantEvaluator::check_integral(const Constant& value, uint32_t location)
{
	if (IsScopedEnumeration(value.type))
		analyzer_.error(location, "a scoped enumeration value does not convert implicitly to an integer");
}

bool ConstantEvaluator::to_bool(const Constant& value, uint32_t location, bool& result)
{
	// Contextual conversion to bool (4p3): not from a scoped enumeration.
	check_integral(value, location);
	result = value.bits != 0;
	return true;
}

bool ConstantEvaluator::eval(const syntax::Expr& expr, Constant& value)
{
	analyzer_.check_stack(expr.location);
	// Semantics mode typed the expression first: the values its typing
	// computed (folded names and queries, integer and character literals,
	// sizeof and alignof) are read from its facts, not derived again.
	const ExprFacts* facts = expr.facts;
	if (facts != nullptr && facts->type != nullptr && IsWideIntegral(facts->type) && (facts->flags & kFolded) == 0)
		return fail(expr.location, "a 128-bit integer value (not evaluated)");
	if (expr.kind == syntax::ExprKind::Member &&
		!DesignatesWithoutEffects(static_cast<const syntax::MemberExpr&>(expr).base, 0))
		return fail(expr.location, "a member access whose object expression has effects");
	if (facts != nullptr && facts->type != nullptr &&
		((facts->flags & kFolded) != 0 || expr.kind == syntax::ExprKind::Sizeof ||
			expr.kind == syntax::ExprKind::SizeofPack || expr.kind == syntax::ExprKind::TypeTrait ||
			(expr.kind == syntax::ExprKind::Literal && IsIntegralType(facts->type) &&
				(facts->flags & kUserLiteral) == 0)))
	{
		value = facts->constant();
		return true;
	}
	switch (expr.kind)
	{
	case syntax::ExprKind::Literal:
		// A user-defined literal calls its literal operator (2.14.8p5).
		if (facts != nullptr && (facts->flags & kUserLiteral) != 0)
			return fail(expr.location, "a user-defined literal is not a constant expression here");
		return literal(static_cast<const syntax::LiteralExpr&>(expr), value);
	case syntax::ExprKind::KeywordLiteral:
	{
		const TokenKind keyword = static_cast<const syntax::KeywordLiteralExpr&>(expr).keyword;
		if (keyword == TokenKind::KW_NULLPTR && skipping_ != 0)
		{
			// Typed only (decltype(nullptr)): a null pointer constant.
			value = Constant{ 0, analyzer_.types_.fundamental(FundamentalType::NullptrT) };
			return true;
		}
		if (keyword != TokenKind::KW_TRUE && keyword != TokenKind::KW_FALSE)
			return fail(expr.location, "not an integral constant expression");
		value = boolean(keyword == TokenKind::KW_TRUE);
		return true;
	}
	case syntax::ExprKind::Paren:
		return eval(*static_cast<const syntax::ParenExpr&>(expr).inner, value);
	case syntax::ExprKind::Id:
		return identifier(static_cast<const syntax::IdExpr&>(expr), value);
	case syntax::ExprKind::Binary:
		return chain(static_cast<const syntax::BinaryExpr&>(expr), value);
	case syntax::ExprKind::Unary:
		return unary(static_cast<const syntax::UnaryExpr&>(expr), value);
	case syntax::ExprKind::Conditional:
		return conditional(static_cast<const syntax::ConditionalExpr&>(expr), value);
	case syntax::ExprKind::Cast:
	{
		const syntax::CastExpr& cast_expr = static_cast<const syntax::CastExpr&>(expr);
		if (cast_expr.cast != TokenKind::KW_STATIC_CAST && cast_expr.cast != TokenKind::OP_LPAREN)
			return fail(expr.location, "cast is not allowed in a constant expression");
		return cast(cast_target(expr, cast_expr.type), cast_expr.operand, expr.location, value);
	}
	case syntax::ExprKind::FunctionalCast:
		return functional_cast(static_cast<const syntax::FunctionalCastExpr&>(expr), value);
	case syntax::ExprKind::Subscript:
		return string_element(static_cast<const syntax::SubscriptExpr&>(expr), value);
	case syntax::ExprKind::SizeofPack:
	{
		// A template definition's pack has no length yet: value-dependent.
		uint64_t count = 0;
		if (!analyzer_.pack_size(static_cast<const syntax::SizeofPackExpr&>(expr).pack, scope_, count))
			return fail(expr.location, "a value-dependent sizeof...");
		value = Constant{ count, analyzer_.types_.fundamental(FundamentalType::UnsignedLongInt) };
		return true;
	}
	case syntax::ExprKind::Sizeof:
		return size_of(static_cast<const syntax::SizeofExpr&>(expr), value);
	case syntax::ExprKind::BuiltinTrait:
		if (!analyzer_.trait_constant(static_cast<const syntax::BuiltinTraitExpr&>(expr), scope_, value))
			return fail(expr.location, "a type trait of a dependent type");
		return true;
	case syntax::ExprKind::TypeTrait:
	{
		// alignof of a type-id, or GNU's of an expression's type.
		const syntax::TypeTraitExpr& trait = static_cast<const syntax::TypeTraitExpr&>(expr);
		if (trait.keyword != TokenKind::KW_ALIGNOF)
			return fail(expr.location, "not an integral constant expression");
		const Type* type = nullptr;
		if (trait.type != nullptr)
			type = analyzer_.type_id(*trait.type, scope_);
		else if (!operand_type(*trait.operand, type))
			return false;
		if (type->is_reference())
			type = type->element;
		if (analyzer_.template_depth_ != 0 && analyzer_.dependent_type(type))
			return fail(expr.location, "a value-dependent alignof");
		uint64_t size = 0, alignment = 0;
		if (!analyzer_.type_size(type, size, alignment, expr.location))
			return fail(expr.location, "alignment of a dependent type");
		value = Constant{ alignment, analyzer_.types_.fundamental(FundamentalType::UnsignedLongInt) };
		return true;
	}
	default:
		return fail(expr.location, "not an integral constant expression");
	}
}

bool ConstantEvaluator::string_element(const syntax::SubscriptExpr& expr, Constant& value)
{
	// A string literal's element (2.14.5p8: an array of const code units
	// initialized with its constant characters, 5.19p2), at an index
	// within the array.
	const syntax::Expr* base = WithoutParentheses(expr.base);
	if (base->kind != syntax::ExprKind::Literal)
		return fail(expr.location, "not an integral constant expression");
	const TokenSpelling& spelling = static_cast<const syntax::LiteralExpr*>(base)->spelling;
	StringLiteralFacts facts;
	std::vector<uint64_t> units;
	TextLiteralParts parts;
	if (!SplitTextLiteral(spelling.text, spelling.length, parts) || parts.quote != '"' ||
		!StringLiteralUnits(spelling, facts, units) || facts.suffix_length != 0)
		return fail(expr.location, "not an integral constant expression");
	Constant index;
	if (!eval(*expr.index, index))
		return false;
	check_integral(index, expr.index->location);
	if ((IsSignedConstant(index) && static_cast<int64_t>(index.bits) < 0) || index.bits >= units.size())
		return fail(expr.location, "a string literal subscript outside its array");
	const Type* type = analyzer_.types_.fundamental(facts.element);
	value = Constant{ ConvertIntegral(units[index.bits], KindOf(type)), type };
	return true;
}

bool ConstantEvaluator::literal(const syntax::LiteralExpr& expr, Constant& value)
{
	const TokenSpelling& spelling = expr.spelling;
	TextLiteralParts parts;
	if (SplitTextLiteral(spelling.text, spelling.length, parts))
	{
		if (parts.quote != '\'')
			return fail(expr.location, "a string literal is not an integral constant");
		const CharacterLiteralValue character = EvaluateCharacterLiteral(spelling.text, spelling.length);
		if (!character.valid || character.suffix_begin != spelling.length)
			return fail(expr.location, "not an integral constant expression");
		const Type* type = analyzer_.types_.fundamental(character.type);
		value = Constant{ ConvertIntegral(character.code_point, KindOf(type)), type };
		return true;
	}
	const NumberLiteral number = ClassifyNumber(spelling.text, spelling.length);
	if (number.kind == NumberLiteralKind::Floating)
		return fail(expr.location, "floating-point constant evaluation is not supported here");
	if (number.kind != NumberLiteralKind::Integer)
		return fail(expr.location, "not an integral constant expression");
	value = Constant{ number.integer, analyzer_.types_.fundamental(number.type) };
	return true;
}

bool ConstantEvaluator::identifier(const syntax::IdExpr& expr, Constant& value)
{
	// The entity the typing recorded (semantics mode), else the one lookup
	// finds.
	if (expr.facts != nullptr && expr.facts->entity != nullptr)
		return entity_value(*expr.facts->entity, expr.location, value);
	const LookupResult result = analyzer_.lookup_name(*expr.name, scope_, LookupFilter::Ordinary);
	const Binding* binding = result.binding;
	// A substitution's failed lookup (14.8.2p8) has no binding.
	if (binding == nullptr)
		return fail(expr.location, "a name whose lookup failed");
	// A dependent member (`T::size` in a template) is value-dependent: its
	// value is known in instantiations.
	if (IsDependentMember(binding) && analyzer_.template_depth_ != 0)
		return fail(expr.location, "a value-dependent constant");
	switch (binding->kind)
	{
	case BindingKind::Type:
	case BindingKind::TypeAlias:
	case BindingKind::Namespace:
	case BindingKind::NamespaceAlias:
		analyzer_.error(expr.location, "name does not denote a value");
	case BindingKind::Variable:
	{
		// 5.1.1p13: a non-static data member named alone outside the member
		// functions of its class (9.3.1p3) and the unnamed object of an
		// anonymous union holding it (9.5p5) names no object: only in an
		// unevaluated operand (sizeof(S::m); an unselected operand is only
		// typed too). Semantics mode knows the body analyzed.
		const VariableEntity* variable = static_cast<const VariableEntity*>(binding->entity);
		if (variable->member && skipping_ == 0 && analyzer_.unevaluated_ == 0 && analyzer_.semantic())
		{
			const ClassEntity* home = MemberClassOf(variable);
			if (!(home->anonymous && home->storage != nullptr) && !analyzer_.implicit_object(home))
				analyzer_.error(expr.location, "non-static data member used without an object");
		}
		return entity_value(*variable, expr.location, value);
	}
	default:
		return entity_value(*binding->entity, expr.location, value);
	}
}

bool ConstantEvaluator::entity_value(const Entity& entity, uint32_t location, Constant& value)
{
	switch (entity.kind)
	{
	case EntityKind::Enumerator:
		// A pointer or reference template argument is no integral value.
		if (static_cast<const EnumeratorEntity&>(entity).argument != nullptr)
			return fail(location, "not an integral constant expression");
		value = static_cast<const EnumeratorEntity&>(entity).value;
		return true;
	case EntityKind::Variable:
	case EntityKind::Parameter:
	{
		const bool variable = entity.kind == EntityKind::Variable;
		if (variable && analyzer_.variable_constant(static_cast<const VariableEntity*>(&entity), value))
			return true;
		// Not a constant, but an unevaluated operand (the unselected arm
		// of ?:) is only typed.
		const Type* type = entity.type->is_reference() ? entity.type->element : entity.type;
		if (skipping_ != 0 && (IsIntegralType(type) || type->unqualified->kind == TypeKind::Enum))
		{
			value = Constant{ 0, type->unqualified };
			return true;
		}
		return fail(location,
			variable ? "variable is not usable in a constant expression"
					 : "a parameter is not usable in a constant expression");
	}
	default:
		return fail(location, "not an integral constant expression");
	}
}

const Type* ConstantEvaluator::cast_target(const syntax::Expr& expr, const syntax::TypeId* type)
{
	const ExprFacts* facts = expr.facts;
	if (facts == nullptr)
		return analyzer_.type_id(*type, scope_);
	return (facts->flags & kReferenceCast) != 0 ? facts->target : facts->type;
}

bool ConstantEvaluator::chain(const syntax::BinaryExpr& expr, Constant& value)
{
	// Operators the parser builds in a loop nest on the left without bound,
	// so the left spine is walked iteratively.
	const size_t base = spine_.size();
	const syntax::Expr* node = &expr;
	while (node->kind == syntax::ExprKind::Binary)
	{
		spine_.push_back(static_cast<const syntax::BinaryExpr*>(node));
		node = spine_.back()->left;
	}
	Constant current;
	bool ok = eval(*node, current);
	for (size_t i = spine_.size(); ok && i-- > base;)
		ok = combine(*spine_[i], current, current);
	spine_.resize(base);
	if (ok)
		value = current;
	return ok;
}

bool ConstantEvaluator::logical(const syntax::BinaryExpr& node, const Constant& left, Constant& value)
{
	bool left_value = false;
	to_bool(left, node.left->location, left_value);
	const bool decided = node.op == TokenKind::OP_LOR ? left_value : !left_value;
	Constant right;
	if (decided)
	{
		// The right operand is not evaluated (5.14p1, 5.15p1): not even
		// its constancy matters. In a template definition one that cannot
		// be typed and names a dependent entity or type is value-dependent,
		// and so is the whole expression (14.6.2.3p1: `false && sizeof(X)
		// == 4` in a member template of an instance); its instantiations
		// evaluate it. A call of a function that is no constexpr one is not.
		++skipping_;
		const char* failure = failure_;
		const bool typed = eval(*node.right, right);
		--skipping_;
		if (!typed && analyzer_.template_depth_ != 0 && analyzer_.instantiation_dependent(*node.right, scope_))
			return false;
		if (typed)
			check_integral(right, node.right->location);
		failure_ = failure;
		value = boolean(left_value);
		return true;
	}
	if (!eval(*node.right, right))
		return false;
	bool right_value = false;
	to_bool(right, node.right->location, right_value);
	value = boolean(right_value);
	return true;
}

bool ConstantEvaluator::combine(const syntax::BinaryExpr& node, const Constant& left, Constant& value)
{
	if (node.op == TokenKind::OP_LAND || node.op == TokenKind::OP_LOR)
		return logical(node, left, value);
	Constant right;
	if (!eval(*node.right, right))
		return false;
	if (node.op == TokenKind::OP_COMMA)
	{
		value = right;
		return true;
	}
	return arithmetic(node.op, left, right, node.location, value);
}

bool ConstantEvaluator::compare(
	TokenKind op, const Constant& left, const Constant& right, uint32_t location, Constant& value)
{
	bool is_signed = false;
	uint64_t a = left.bits, b = right.bits;
	if (IsScopedEnumeration(left.type) || IsScopedEnumeration(right.type))
	{
		// Only values of the same scoped enumeration compare (5.9p2, 5.10p1).
		if (left.type->unqualified != right.type->unqualified)
			analyzer_.error(location, "comparison between a scoped enumeration and another type");
		is_signed = IsSignedConstant(left);
	}
	else
	{
		TypeTable& types = analyzer_.types_;
		const Type* common = CommonType(types, PromotedType(types, left.type), PromotedType(types, right.type));
		const IntegralKind kind = KindOf(common);
		is_signed = kind.is_signed;
		a = ConvertIntegral(a, kind);
		b = ConvertIntegral(b, kind);
	}
	value = boolean(IntegralCompare(op, is_signed, a, b));
	return true;
}

bool ConstantEvaluator::shift(
	TokenKind op, const Constant& left, const Constant& right, uint32_t location, Constant& value)
{
	TypeTable& types = analyzer_.types_;
	const Type* type = PromotedType(types, left.type);
	value.type = type;
	if (!IntegralShift(op, KindOf(type), left.bits, KindOf(PromotedType(types, right.type)), right.bits, value.bits))
		return skipping_ != 0 || fail(location, "a shift outside its defined range");
	return true;
}

bool ConstantEvaluator::arithmetic(
	TokenKind op, const Constant& left, const Constant& right, uint32_t location, Constant& value)
{
	if (IsComparison(op))
		return compare(op, left, right, location, value);
	check_integral(left, location);
	check_integral(right, location);
	if (op == TokenKind::OP_LSHIFT || op == TokenKind::OP_RSHIFT)
		return shift(op, left, right, location, value);
	if (op != TokenKind::OP_PLUS && op != TokenKind::OP_MINUS && op != TokenKind::OP_STAR && op != TokenKind::OP_DIV &&
		op != TokenKind::OP_MOD && op != TokenKind::OP_AMP && op != TokenKind::OP_BOR && op != TokenKind::OP_XOR)
		return fail(location, "operator is not allowed in a constant expression");
	TypeTable& types = analyzer_.types_;
	const Type* type = CommonType(types, PromotedType(types, left.type), PromotedType(types, right.type));
	value.type = type;
	// 5p4, 5.6p4: an undefined result is not a constant.
	if (!IntegralOperation(op, KindOf(type), left.bits, right.bits, value.bits))
		return skipping_ != 0 || fail(location, "an arithmetic operation with an undefined result");
	return true;
}

bool ConstantEvaluator::unary(const syntax::UnaryExpr& expr, Constant& value)
{
	// 5.3.1p3: `&` yields an address (`&C::m` of a non-static member a
	// pointer to member): its operand is no value read, and the result no
	// integral constant (a template argument's is address_argument's).
	if (expr.op == TokenKind::OP_AMP)
		return fail(expr.location, "operator is not allowed in a constant expression");
	Constant operand;
	if (!eval(*expr.operand, operand))
		return false;
	if (expr.op == TokenKind::OP_LNOT)
	{
		bool truth = false;
		to_bool(operand, expr.operand->location, truth);
		value = boolean(!truth);
		return true;
	}
	if (expr.op != TokenKind::OP_PLUS && expr.op != TokenKind::OP_MINUS && expr.op != TokenKind::OP_COMPL)
		return fail(expr.location, "operator is not allowed in a constant expression");
	check_integral(operand, expr.location);
	const Type* type = PromotedType(analyzer_.types_, operand.type);
	const IntegralKind kind = KindOf(type);
	const uint64_t bits = ConvertIntegral(operand.bits, kind);
	value.type = type;
	if (expr.op == TokenKind::OP_PLUS)
		value.bits = bits;
	else if (expr.op == TokenKind::OP_COMPL)
		value.bits = ConvertIntegral(~bits, kind);
	else
	{
		if (kind.is_signed && static_cast<int64_t>(bits) == Minimum(kind.bits))
			return skipping_ != 0 || fail(expr.location, "signed overflow in a constant expression");
		value.bits = ConvertIntegral(0 - bits, kind);
	}
	return true;
}

bool ConstantEvaluator::conditional(const syntax::ConditionalExpr& expr, Constant& value)
{
	Constant condition;
	if (!eval(*expr.condition, condition))
		return false;
	bool truth = false;
	to_bool(condition, expr.condition->location, truth);
	const syntax::Expr* selected = truth ? expr.then_expr : expr.else_expr;
	const syntax::Expr* other = truth ? expr.else_expr : expr.then_expr;
	Constant chosen, unchosen;
	if (!eval(*selected, chosen))
		return false;
	++skipping_;
	const bool typed = eval(*other, unchosen);
	--skipping_;
	if (!typed)
		return false;
	// 5.16p6: both arms convert to their common type.
	if (chosen.type->unqualified == unchosen.type->unqualified)
	{
		value = chosen;
		return true;
	}
	check_integral(chosen, selected->location);
	check_integral(unchosen, other->location);
	TypeTable& types = analyzer_.types_;
	const Type* type = CommonType(types, PromotedType(types, chosen.type), PromotedType(types, unchosen.type));
	value = Constant{ ConvertIntegral(chosen.bits, KindOf(type)), type };
	return true;
}

bool ConstantEvaluator::cast(const Type* type, const syntax::Expr* operand, uint32_t location, Constant& value)
{
	const Type* target = type->unqualified;
	IntegralKind kind;
	if (!IntegralKindOf(target, kind))
		return fail(location, "cast to a type that is not integral");
	Constant source = Constant{ 0, target };
	if (operand != nullptr && !eval(*operand, source))
		return false;
	IntegralKind source_kind;
	if (!IntegralKindOf(source.type, source_kind))
		return fail(location, "not an integral constant expression");
	value = Constant{ ConvertIntegral(source.bits, kind), target };
	return true;
}

bool ConstantEvaluator::functional_cast(const syntax::FunctionalCastExpr& expr, Constant& value)
{
	const Type* target = expr.facts != nullptr ? cast_target(expr, nullptr) : nullptr;
	if (target == nullptr)
	{
		Specifiers specifiers;
		analyzer_.specifiers(expr.type, scope_, specifiers, false);
		if (specifiers.type == nullptr || specifiers.type == analyzer_.auto_placeholder_)
			return fail(expr.location, "not an integral constant expression");
		target = specifiers.type;
	}
	if (expr.braced != nullptr)
	{
		if (expr.braced->elements.size > 1)
			return fail(expr.location, "not an integral constant expression");
		return cast(target, expr.braced->elements.empty() ? nullptr : expr.braced->elements[0], expr.location, value);
	}
	if (expr.arguments.size > 1)
		return fail(expr.location, "not an integral constant expression");
	return cast(target, expr.arguments.empty() ? nullptr : expr.arguments[0], expr.location, value);
}

bool ConstantEvaluator::operand_type(const syntax::Expr& expr, const Type*& type)
{
	const syntax::Expr* inner = WithoutParentheses(&expr);
	if (inner->kind == syntax::ExprKind::Id)
	{
		const LookupResult result = analyzer_.lookup_name(
			*static_cast<const syntax::IdExpr*>(inner)->name, scope_, LookupFilter::Ordinary);
		const Binding* binding = result.binding;
		if (binding == nullptr)
			return fail(inner->location, "a name whose lookup failed");
		switch (binding->kind)
		{
		case BindingKind::Type:
		case BindingKind::TypeAlias:
			// sizeof(T) where lookup finds a type is sizeof of the type.
			type = analyzer_.binding_type(binding, inner->location);
			return true;
		case BindingKind::Variable:
			type = binding->entity->type;
			return true;
		case BindingKind::Parameter:
		case BindingKind::Function:
			type = binding->type;
			return true;
		default:
			break;
		}
	}
	return type_of(expr, type);
}

bool ConstantEvaluator::size_of(const syntax::SizeofExpr& expr, Constant& value)
{
	// 5.3.3: the operand is not evaluated; a reference type's size is its
	// referent's.
	const Type* type = nullptr;
	if (expr.type != nullptr)
		type = analyzer_.type_id(*expr.type, scope_);
	else if (!operand_type(*expr.operand, type))
		return false;
	if (type->is_reference())
		type = type->element;
	// 14.6.2.3p2: in a template definition, sizeof of a dependent type (or
	// of an expression of one) is value-dependent, whatever its size
	// (sizeof(T*)): the mangled signature keeps it (Itanium 5.1.6).
	if (analyzer_.template_depth_ != 0 && analyzer_.dependent_type(type))
		return fail(expr.location, "a value-dependent sizeof");
	uint64_t size = 0, alignment = 0;
	if (!analyzer_.type_size(type, size, alignment, expr.location))
		return fail(expr.location, "size of a dependent type");
	value = Constant{ size, analyzer_.types_.fundamental(FundamentalType::UnsignedLongInt) };
	return true;
}

// ------------------------------------------------------ analyzer entries

void Analyzer::constant_operand(const syntax::Expr& expr, Scope* scope, bool resolved)
{
	// A constant expression outside a function body, typed as one would be
	// in a body (no object is available to it). Not `resolved`: a set of
	// overloaded functions (or its address) is left for its context's
	// target type to resolve (13.4).
	BodyContext* saved = body_;
	BodyContext context = BodyContext();
	if (body_ == nullptr)
		body_ = &context;
	if (resolved)
		typed_value(expr, scope);
	else
		typed(expr, scope);
	body_ = saved;
}

bool Analyzer::constant_value(const syntax::Expr& expr, Scope* scope, Constant& value, bool required)
{
	// An operand whose typing a hosted wrapper's reserved builtin call
	// deferred (see deferrable_statement) has no value yet.
	if (deferred_reserved_call())
		return false;
	// Semantics mode evaluates typed expressions over their facts (the
	// constant interpreter); an expression analysis has not typed (types
	// mode, a template definition's) by the integral evaluator.
	if (semantic() && WithoutParentheses(&expr)->facts != nullptr)
	{
		ConstantInterpreter interpreter(*this);
		ConstValue result;
		if (interpreter.evaluate(expr, false, result) && result.kind == ValueKind::Integer && HeldAsConstant(result))
		{
			value = Constant{ result.bits, result.type };
			return true;
		}
		if (required)
			error(interpreter.failure() != nullptr ? interpreter.failure_location() : expr.location,
				interpreter.failure() != nullptr        ? interpreter.failure()
					: result.kind == ValueKind::Integer ? "a 128-bit constant beyond the 64-bit integral constants"
														: "not an integral constant expression");
		return false;
	}
	++graph_.constant_evaluations;
	ConstantEvaluator evaluator(*this, scope);
	if (evaluator.evaluate(expr, value))
		return true;
	if (required)
		error(evaluator.failure_location(), evaluator.failure());
	return false;
}

bool Analyzer::converted_constant(const syntax::Expr& expr, Constant& value, bool required)
{
	if (deferred_reserved_call())
		return false;
	// A converted constant expression (5.19p3) or a condition: the value
	// after the conversion its context recorded (a class operand's
	// constexpr conversion function, a contextual conversion to bool).
	ConstantInterpreter interpreter(*this);
	ConstValue result;
	if (interpreter.evaluate(expr, true, result) && result.kind == ValueKind::Integer && HeldAsConstant(result))
	{
		value = Constant{ result.bits, result.type };
		return true;
	}
	if (required)
		error(interpreter.failure() != nullptr ? interpreter.failure_location() : expr.location,
			interpreter.failure() != nullptr ? interpreter.failure() : "not an integral constant expression");
	return false;
}

bool Analyzer::integral_constant(const syntax::Expr& expr, Scope* scope, Constant& value)
{
	// After analysis the operands' facts are recorded, so the evaluation
	// reads them; an ill-formed operand was diagnosed then.
	IntegralKind kind;
	return constant_value(expr, scope, value, false) && IntegralKindOf(value.type, kind);
}

bool Analyzer::integral_constant(const syntax::Expr& expr, Scope* scope, Constant& value, uint64_t& high)
{
	// The interpreter's integer keeps a 128-bit type's whole value, which
	// a Constant holds only when its high half extends its low one.
	IntegralKind kind;
	if (semantic() && WithoutParentheses(&expr)->facts != nullptr)
	{
		ConstantInterpreter interpreter(*this);
		ConstValue result;
		if (!interpreter.evaluate(expr, false, result) || result.kind != ValueKind::Integer ||
			!IntegralKindOf(result.type, kind))
			return false;
		value = Constant{ result.bits, result.type };
		high = kind.bits > 64                                         ? result.high
			: kind.is_signed && static_cast<int64_t>(result.bits) < 0 ? ~uint64_t(0)
																	  : 0;
		return true;
	}
	if (!integral_constant(expr, scope, value))
		return false;
	IntegralKindOf(value.type, kind);
	high = kind.is_signed && static_cast<int64_t>(value.bits) < 0 ? ~uint64_t(0) : 0;
	return true;
}

bool Analyzer::floating_constant(const syntax::Expr& expr, Scope* scope, long double& value)
{
	// 5.19p2 over the typed expression, in its type's precision (an
	// integral constant converted, 4.9p2).
	(void)scope;
	if (!semantic() || WithoutParentheses(&expr)->facts == nullptr)
		return false;
	ConstantInterpreter interpreter(*this);
	ConstValue result;
	if (!interpreter.evaluate(expr, false, result))
		return false;
	if (result.kind == ValueKind::Floating)
	{
		value = result.floating;
		return true;
	}
	IntegralKind kind;
	if (result.kind != ValueKind::Integer || !IntegralKindOf(result.type, kind) || IsScopedEnumeration(result.type))
		return false;
	value = kind.is_signed ? static_cast<long double>(static_cast<int64_t>(result.bits))
						   : static_cast<long double>(result.bits);
	return true;
}

bool Analyzer::variable_constant(const VariableEntity* variable, Constant& value)
{
	if (variable->constant == ConstantState::Unknown && variable->templ != nullptr &&
		((variable->type->cv & kConst) != 0 || variable->is_constexpr))
		instantiate_variable(variable);
	if (variable->constant != ConstantState::Value)
		return false;
	value = variable->value;
	return true;
}

const ConstValue* Analyzer::variable_value(const VariableEntity* variable)
{
	// A variable usable in constant expressions (5.19p2): a const integral
	// object or a constexpr object initialized with a constant expression,
	// or a reference so initialized (its referent); an instance's static
	// data member's initializer instantiated first.
	if (variable->constant == ConstantState::Unknown && variable->templ != nullptr &&
		((variable->type->cv & kConst) != 0 || variable->is_constexpr || variable->type->is_reference()))
		instantiate_variable(variable);
	if (variable->constant != ConstantState::Value && variable->constant != ConstantState::Object)
		return nullptr;
	return evaluated_value(variable);
}

const ConstValue* Analyzer::evaluated_value(const VariableEntity* variable)
{
	const uint32_t index = evaluated_index_.find(variable);
	if (index != PointerMap::kMissing)
		return evaluated_[index];
	if (variable->constant != ConstantState::Value || variable->type->is_reference())
		return nullptr;
	// An integral constant (its Constant: the integral evaluation's, or an
	// evaluated integer's), published on first use.
	ConstValue* value = static_cast<ConstValue*>(arena_.allocate(sizeof(ConstValue), alignof(ConstValue)));
	*value = IntegerValue(variable->value.type, variable->value.bits);
	set_evaluated(variable, value);
	return value;
}

bool Analyzer::read_through_storage(const VariableEntity* variable) const
{
	return stored_references_.find(variable) != PointerMap::kMissing;
}

void Analyzer::set_evaluated(const VariableEntity* variable, const ConstValue* value)
{
	const uint32_t index = evaluated_index_.find(variable);
	if (index != PointerMap::kMissing)
	{
		evaluated_[index] = value;
		return;
	}
	evaluated_index_.set(variable, static_cast<uint32_t>(evaluated_.size()));
	evaluated_.push_back(value);
}

bool Analyzer::constexpr_definition(const FunctionEntity* function)
{
	// An in-class member body is a complete-class context (9.2p2), but
	// a static data member's initializer is not. It cannot evaluate that
	// body before the owning class has completed.
	if (!function->instantiated && function->owner != nullptr && function->owner->kind == ScopeKind::Class &&
		!static_cast<const ClassEntity*>(function->owner->entity)->complete)
		return false;
	// The definition a constant evaluation runs: instantiated on first
	// demand (14.7.1p3), or a deferred member body analyzed once its owning
	// class is complete.
	if (function->implicit || ((function->defaulted || function->synthesized) && function->inits != nullptr))
		return true;
	if (function->instantiated)
	{
		// A member of an instance whose definition is being instantiated (or
		// the member template whose specialization this is) learns its
		// pattern now (the rest of the class links later).
		const FunctionEntity* member = function->primary != nullptr ? function->primary : function;
		const ClassEntity* owner = member->owner != nullptr && member->owner->kind == ScopeKind::Class
			? static_cast<const ClassEntity*>(member->owner->entity)
			: nullptr;
		if (PatternOf(member) == nullptr && member->primary == nullptr && owner != nullptr && owner->templ != nullptr)
		{
			// The members analyzed since the last such link (each member
			// once per instantiation).
			for (size_t i = class_instantiations_.size(); i-- > 0;)
			{
				ClassInstantiation& record = class_instantiations_[i];
				if (record.entity != owner)
					continue;
				link_member_patterns(*record.pattern, *record.copy, record.linked, record.declared);
				record.linked = std::max(record.linked, record.declared);
				break;
			}
		}
		instantiate_definition(function);
		return function->scope != nullptr && function->body != nullptr;
	}
	// A body is analyzed once its function scope exists (a member's waits
	// for its class's completion, 9.2p2).
	if (function->scope != nullptr && function->body != nullptr)
		return true;
	const uint32_t i = deferred_constexpr_.find(function);
	if (i == PointerMap::kMissing || i >= deferred_.size())
		return false;
	const DeferredBody body = deferred_[i];
	if (body.function != function || body.body == nullptr || body.member != nullptr || function->templated)
		return false;
	// Analyzed now, out of the queue (its entry left to be skipped).
	deferred_[i].body = nullptr;
	deferred_[i].parameters = nullptr;
	deferred_[i].function = nullptr;
	deferred_[i].member = nullptr;
	deferred_[i].type = nullptr;
	const uint32_t saved_template = template_depth_;
	template_depth_ = 0;
	// A hosted wrapper's body defers as at its class's completion (see
	// finish_class_nesting); the evaluation fails if it reaches such a
	// deferred full-expression.
	const bool reserved = reserved_wrapper(*body.body, body.parent);
	if (reserved)
		body.function->unanalyzed_body = true;
	wrapper_body(body.function, body.type, body.parameters, *body.body, body.print, body.parent, reserved);
	template_depth_ = saved_template;
	return function->body != nullptr;
}

const ConstValue* Analyzer::constant_initialization(
	const VariableEntity* variable, const syntax::Initializer* initializer)
{
	// Without an initializer: default-initialization by a constructor (a
	// constexpr one, 12.1p6, gives a constant).
	ConstantInterpreter interpreter(*this);
	const ConstValue* value = nullptr;
	return interpreter.initialize(variable, initializer, value, false, variable->is_constinit) ? value : nullptr;
}

void Analyzer::record_constant_initializer(
	VariableEntity* variable, const syntax::Initializer* initializer, Scope* scope)
{
	// 5.19p2: a const, non-volatile object of integral or enumeration type
	// initialized with a constant expression, a constexpr object (7.1.5p9:
	// which must be so initialized) and a reference so initialized are
	// usable in constant expressions. Evaluated at the declaration, where
	// the initializer's names are looked up.
	//
	// A non-static data member is no such object: its brace-or-equal-
	// initializer initializes each object (12.6.2p8), typed in the
	// complete class where its names may be members of `*this` (9.2p2,
	// 5.1.1p13); named alone, the member is `(*this).m` (9.3.1p3), not
	// constant (5.19p2), or names no object.
	if (variable->member)
		return;
	const ConstValue* initialized = nullptr;
	if (variable->is_constinit && semantic())
	{
		initialized = constant_initialization(variable, initializer);
		if (initialized == nullptr)
			error(initializer != nullptr ? initializer->location : 0,
				"a __constinit variable is not constant-initialized");
		// Initialization bytes are available to lowering, while the mutable
		// variable remains unusable as a constant expression.
		set_evaluated(variable, initialized);
	}
	const Type* type = variable->type;
	const bool reference = type->is_reference();
	const Type* object = reference ? type->element : type;
	const syntax::Expr* expr = SingleInitializer(initializer);
	IntegralKind kind;
	const bool const_integral = !reference && (object->cv & kConst) != 0 && (object->cv & kVolatile) == 0 &&
		IntegralKindOf(object, kind);
	if (!variable->is_constexpr && !const_integral && !reference)
		return;
	if (!semantic())
	{
		// Types mode: the integral evaluation of an unanalyzed initializer;
		// a reference bound to an object designates that object.
		if (!IntegralKindOf(object, kind) || (object->cv & kConst) == 0 || (object->cv & kVolatile) != 0 ||
			expr == nullptr)
			return;
		variable->constant = ConstantState::Evaluating;
		Constant value;
		const syntax::Expr* inner = WithoutParentheses(expr);
		bool constant = false;
		if (reference && inner->kind == syntax::ExprKind::Id)
		{
			const Binding* found =
				lookup_name(*static_cast<const syntax::IdExpr*>(inner)->name, scope, LookupFilter::Ordinary).binding;
			const Entity* designated = found != nullptr ? found->entity : nullptr;
			constant = designated != nullptr && designated->kind == EntityKind::Variable &&
				variable_constant(static_cast<const VariableEntity*>(designated), value);
		}
		else
			constant = constant_value(*expr, scope, value, false);
		variable->constant = ConstantState::None;
		if (!constant)
			return;
		if ((IsScopedEnumeration(object) || IsScopedEnumeration(value.type)) &&
			value.type->unqualified != object->unqualified)
			error(expr->location, "a scoped enumeration value does not convert implicitly");
		if (object->unqualified->kind == TypeKind::Enum && value.type->unqualified != object->unqualified)
			error(expr->location, "an integer does not convert implicitly to an enumeration");
		variable->value = Constant{ ConvertIntegral(value.bits, kind), object->unqualified };
		variable->constant = ConstantState::Value;
		return;
	}
	variable->constant = ConstantState::Evaluating;
	ConstantInterpreter interpreter(*this);
	const ConstValue* value = nullptr;
	const bool constant = initialized != nullptr || interpreter.initialize(variable, initializer, value, true);
	if (initialized != nullptr)
		value = initialized;
	variable->constant = ConstantState::None;
	// The course reads a constexpr reference whose referent a conditional
	// selects through its storage, its initializer's functions defined as
	// for a dynamic initialization (a presentation; its value stays usable
	// in constant expressions).
	if (reference && variable->is_constexpr && expr != nullptr &&
		WithoutParentheses(expr)->kind == syntax::ExprKind::Conditional)
		stored_references_.set(variable, 1);
	// A static data member's in-class initializer converting a pointer to
	// member to a class being defined (4.11p2: the base subobject's offset
	// its layout gives) is evaluated again once the outermost class is
	// complete (finish_class_nesting) when its value may hold one (an
	// array's or a class's element may); until then its value is unknown.
	const Type* held = object;
	while (held->kind == TypeKind::Array)
		held = held->element;
	if (interpreter.met_incomplete_class() && variable->is_constexpr && class_depth_ != 0 &&
		variable->owner->kind == ScopeKind::Class &&
		(held->unqualified->kind == TypeKind::MemberPointer || held->unqualified->kind == TypeKind::Class))
	{
		variable->constant_deferred = true;
		deferred_.push_back(DeferredBody{ nullptr, nullptr, nullptr, nullptr, scope, scope, nullptr, variable });
		return;
	}
	if (!constant)
	{
		// 7.1.5p9: a constexpr variable's initialization is a constant
		// expression.
		if (variable->is_constexpr)
			error(interpreter.failure() != nullptr ? interpreter.failure_location()
					: initializer != nullptr       ? initializer->location
												   : 0,
				interpreter.failure() != nullptr ? interpreter.failure()
												 : "a constexpr variable is not constant-initialized");
		return;
	}
	// An integer is its Constant (published on first use, variable_value);
	// a value-initialization's zero keeps its published value, a 128-bit
	// one no Constant holds its whole value (an object's).
	const bool whole = value->kind == ValueKind::Integer && HeldAsConstant(*value);
	if (reference || !whole || (value->flags & kValueZeroed) != 0)
		set_evaluated(variable, value);
	variable->constant = ConstantState::Object;
	if (!reference && whole)
	{
		variable->value = Constant{ value->bits, value->type };
		variable->constant = ConstantState::Value;
	}
}

const Type* Analyzer::decltype_type(const syntax::Expr& operand, Scope* scope)
{
	// 7.1.6.2p4: an unparenthesized id-expression or class member access
	// gives the declared type of the entity it names; otherwise an lvalue
	// of type T gives T&, an xvalue T&&, a prvalue T.
	const syntax::Expr* expr = &operand;
	bool parenthesized = false;
	while (expr->kind == syntax::ExprKind::Paren)
	{
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
		parenthesized = true;
	}
	// A template definition's decltype of an expression is dependent until
	// substituted (14.6.2.2; typed then, see Analyzer::substitute).
	if (template_depth_ != 0 && expr->kind != syntax::ExprKind::Id)
		return dependent_expression(operand, scope, false, nullptr, semantic());
	if (expr->kind == syntax::ExprKind::Id)
	{
		const LookupResult result = lookup_name(
			*static_cast<const syntax::IdExpr*>(expr)->name, scope, LookupFilter::Ordinary);
		if (failed())
			return nullptr;
		const Binding* binding = result.binding;
		if (template_depth_ != 0 &&
			(last_prefix_.dependent != nullptr ||
				(binding->entity != nullptr && binding->entity->kind == EntityKind::TemplateParameter)))
			return dependent_decltype(operand, scope);
		const Type* type = nullptr;
		switch (binding->kind)
		{
		case BindingKind::Variable:
			type = binding->entity->type;
			// A structured binding gives its referenced type (C++17's
			// 7.1.6.2p4), and parenthesized, its lvalue's (11.5p1).
			if (static_cast<const VariableEntity*>(binding->entity)->structured_binding)
			{
				type = static_cast<const BindingEntity*>(binding->entity)->referenced;
				if (parenthesized && type->is_reference())
					type = type->element;
			}
			break;
		case BindingKind::Parameter:
			type = binding->type;
			break;
		case BindingKind::Function:
		{
			// A template-id names the specializations its template arguments
			// identify (14.8.1p3); a set of overloaded functions is
			// ill-formed, and so is a function template's name, whose
			// specialization only a target type selects (13.4p1).
			const syntax::NameComponent& last = static_cast<const syntax::IdExpr*>(expr)->name->last();
			const FunctionList* set = last.has_arguments ? explicit_specializations(last, result.functions, scope)
														 : result.functions;
			if (failed())
				return nullptr;
			const Entity* function = set != nullptr ? set->binding->entity : binding->entity;
			if ((set != nullptr && set->next != nullptr) || static_cast<const FunctionEntity*>(function)->templated)
				error(expr->location, "decltype of an overloaded function");
			// A function's type has its parameters adjusted (8.3.5p5).
			type = function->type->canonical;
			break;
		}
		case BindingKind::Enumerator:
			return static_cast<const EnumeratorEntity*>(binding->entity)->value.type;
		default:
			error(expr->location, "decltype operand is not an expression");
		}
		// An entity of dependent type named in a template definition: the
		// decltype-specifier is dependent (14.6.2.2p3), a placeholder, not its
		// declared type, so that deduction takes it as a non-deduced context
		// (14.8.2.5p5) and its mangling spells the operand (Dt).
		if (template_depth_ != 0 &&
			(binding->kind == BindingKind::Variable || binding->kind == BindingKind::Parameter) && dependent_type(type))
			return dependent_decltype(operand, scope);
		// 5.1.2p18: in a lambda's body, the member a capture by copy makes.
		if (parenthesized && body_ != nullptr && body_->lambda != nullptr &&
			(binding->kind == BindingKind::Variable || binding->kind == BindingKind::Parameter))
			type = captured_type(binding->entity, type);
		return parenthesized ? types_.reference(type, false) : type;
	}
	if (semantic() && template_depth_ == 0)
	{
		// Semantics mode types the (unevaluated) operand: an lvalue of type
		// T gives T&, an xvalue T&&, a prvalue T.
		// 5.2.2p11: a function call that is the operand (or the right operand
		// of a comma that is the operand) introduces no temporary: its class
		// type may be incomplete and is not instantiated (see new_facts). A
		// type transform or typeof names its type and makes no object either
		// (`__remove_cv(pair<K, V>)` in std::vector<pair<K, V>> as a member
		// of the incomplete V).
		BodyContext* saved = body_;
		BodyContext context = BodyContext();
		if (body_ == nullptr)
			body_ = &context;
		const syntax::Expr* call = expr;
		while (call->kind == syntax::ExprKind::Binary &&
			static_cast<const syntax::BinaryExpr*>(call)->op == TokenKind::OP_COMMA)
			call = WithoutParentheses(static_cast<const syntax::BinaryExpr*>(call)->right);
		const syntax::Expr* saved_call = decltype_call_;
		decltype_call_ = call->kind == syntax::ExprKind::Call || call->kind == syntax::ExprKind::BuiltinTrait ? call
																											  : nullptr;
		++unevaluated_;
		const ExprFacts* facts = typed_value(operand, scope);
		--unevaluated_;
		decltype_call_ = saved_call;
		body_ = saved;
		if (facts == nullptr)
			return nullptr;
		// Any other class prvalue is a temporary, unevaluated or not (12.2p1);
		// a type transform's or typeof's result names a type and makes no
		// object.
		if (call->kind != syntax::ExprKind::Call && call->kind != syntax::ExprKind::BuiltinTrait)
			check_temporary_destruction(facts, operand.location);
		// `obj.m` or `p->m` naming a data member: m's declared type, a
		// reference member's reference type, without the object's
		// cv-qualifiers (7.1.6.2p4 example: decltype(a->x) is double).
		if (!parenthesized && expr->kind == syntax::ExprKind::Member && facts->entity != nullptr &&
			facts->entity->kind == EntityKind::Variable)
			return facts->entity->type;
		if (facts->category == ValueCategory::LValue)
			return types_.reference(facts->type, false);
		if (facts->category == ValueCategory::XValue)
			return types_.reference(facts->type, true);
		return facts->type;
	}
	ConstantEvaluator evaluator(*this, scope);
	const Type* type = nullptr;
	if (!evaluator.type_of(operand, type))
		error(operand.location, "unsupported decltype operand");
	return type;
}

} // namespace sema
