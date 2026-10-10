#include "sema/analyzer.h"

#include "lex/identifier_table.h"

// Run-time type identification: typeid (5.2.8), the course's
// comparison of type information objects, and dynamic_cast's run-time check
// (5.2.7p8). The lowering reads the facts: a typeid designates the type
// information of `target` or, evaluated, of its operand's dynamic type; a
// checked dynamic_cast calls the runtime's search.
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool IsTypeid(const syntax::Expr* expr)
{
	const syntax::Expr* e = WithoutParentheses(expr);
	return e->kind == syntax::ExprKind::TypeTrait &&
		static_cast<const syntax::TypeTraitExpr*>(e)->keyword == TokenKind::KW_TYPEID;
}

// 5.2.8p5's cv-unqualified type: an array of cv T is itself so qualified
// (3.9.3p5), its cv-unqualified type the array of T.
const Type* Unqualified(TypeTable& types, const Type* type)
{
	if (type->kind != TypeKind::Array)
		return type->unqualified;
	const Type* element = Unqualified(types, type->element);
	return element == type->element ? type : types.array(element, type->bound, type->owner);
}

} // namespace

const ClassEntity* Analyzer::type_info_class(uint32_t location)
{
	// 5.2.8p1, p6: typeid's result is an lvalue of type const std::type_info,
	// which the program must have declared (18.7.1; the course has no
	// <typeinfo>); the class may stay incomplete.
	if (type_info_ != nullptr)
		return type_info_;
	const LookupResult space = names_.qualified(graph_.global, std_key_, LookupFilter::Namespace);
	const LookupResult found = space.found() && space.binding->entity != nullptr &&
			space.binding->entity->kind == EntityKind::Namespace
		? names_.qualified(static_cast<NamespaceEntity*>(space.binding->entity)->scope,
			  identifiers_.intern("type_info", 9), LookupFilter::Elaborated)
		: LookupResult();
	const ClassEntity* cls = found.found() && !found.ambiguous && found.binding->kind == BindingKind::Type &&
			found.binding->type != nullptr
		? ClassOf(found.binding->type)
		: nullptr;
	if (cls == nullptr)
		error(location, "typeid without a declaration of std::type_info (5.2.8p6)");
	type_info_ = cls;
	return cls;
}

ExprFacts* Analyzer::typeid_expression(const syntax::TypeTraitExpr& expr, Scope* scope)
{
	const ClassEntity* type_info = type_info_class(expr.location);
	const Type* queried = nullptr;
	bool dynamic = false;
	if (expr.type != nullptr)
	{
		// p4: a reference's referenced type; a class type complete.
		queried = type_id(*expr.type, scope);
		if (queried == nullptr)
			return nullptr;
		if (queried->is_reference())
			queried = queried->element;
		const ClassEntity* cls = ClassOf(queried);
		if (cls != nullptr && !require_complete(cls))
		{
			substitution_error(expr.location, "typeid of an incomplete class type", true);
			return nullptr;
		}
	}
	else
	{
		const ExprFacts* operand = typeid_operand(*expr.operand, scope, dynamic);
		if (operand == nullptr)
			return nullptr;
		if (resolve_lone_function(*expr.operand))
			operand = expr.operand->facts;
		// A member access naming one static member function is an lvalue of
		// its function type (5.2.5p4), a non-static one's is no expression.
		const bool static_member = (operand->flags & kOverloadSet) == 0 && operand->entity != nullptr &&
			operand->entity->kind == EntityKind::Function &&
			static_cast<const FunctionEntity*>(operand->entity)->is_static;
		if ((operand->flags & kOverloadSet) != 0 || ((operand->flags & kBoundMember) != 0 && !static_member))
		{
			substitution_error(expr.location, "typeid of a function name that is not an object");
			return nullptr;
		}
		queried = operand->type;
	}
	// p5: the cv-unqualified type.
	ExprFacts* facts = new_facts(expr, types_.qualified(type_info->type, kConst), ValueCategory::LValue);
	facts->target = Unqualified(types_, queried);
	if (dynamic)
		facts->flags |= kDynamicType;
	return facts;
}

ExprFacts* Analyzer::typeid_operand(const syntax::Expr& operand, Scope* scope, bool& dynamic)
{
	// p2-p3: an operand that is a glvalue of polymorphic class type is
	// evaluated, any other one is not; which it is shows once it is typed.
	// So it is typed as an unevaluated operand whose odr-uses and captures
	// are kept aside (defer_use), then made if it is evaluated.
	const size_t first = deferred_uses_.size();
	const uint32_t saved = deferring_;
	++unevaluated_;
	deferring_ = unevaluated_;
	ExprFacts* facts = typed(operand, scope);
	deferring_ = saved;
	--unevaluated_;
	// p3: an operand of class type names a completely-defined class (an
	// instance's defined, 14.7.1p1), polymorphic or not.
	const ClassEntity* cls = facts != nullptr && facts->type != nullptr ? ClassOf(facts->type) : nullptr;
	if (cls != nullptr && !require_complete(cls))
	{
		deferred_uses_.resize(first);
		substitution_error(operand.location, "typeid of an expression of incomplete class type", true);
		return nullptr;
	}
	dynamic = cls != nullptr && facts->category != ValueCategory::PRValue && cls->polymorphic;
	if (dynamic)
		replay_uses(first);
	deferred_uses_.resize(first);
	return facts;
}

void Analyzer::defer_use(const DeferredUse& use)
{
	deferred_uses_.push_back(use);
}

void Analyzer::replay_uses(size_t first)
{
	// In order, as the evaluated operand would have made them; an enclosing
	// typeid operand being typed keeps them aside in turn.
	const size_t count = deferred_uses_.size();
	for (size_t i = first; i < count; ++i)
	{
		const DeferredUse use = deferred_uses_[i];
		switch (use.kind)
		{
		case DeferredUse::Kind::Function:
			odr_use(const_cast<FunctionEntity*>(static_cast<const FunctionEntity*>(use.entity)), use.call);
			break;
		case DeferredUse::Kind::Local:
			local_name(*use.expr, use.facts, use.entity, use.scope);
			break;
		case DeferredUse::Kind::This:
			capture_this(use.location);
			break;
		case DeferredUse::Kind::Variable:
			demand_variable(static_cast<const VariableEntity*>(use.entity));
			break;
		case DeferredUse::Kind::Error:
			error(use.location, use.message);
		}
	}
}

void Analyzer::demand_variable(const VariableEntity* variable)
{
	// An instance's static data member named where evaluated: its
	// definition is emitted (14.7.1p2).
	if (unevaluated_ != 0)
	{
		if (deferring())
			defer_use(
				DeferredUse{ DeferredUse::Kind::Variable, false, nullptr, nullptr, variable, nullptr, nullptr, 0 });
		return;
	}
	if (variable->templ == nullptr || variable->templ->pattern == nullptr || variable->templ->demanded)
		return;
	const_cast<VariableTemplateFacts*>(variable->templ)->demanded = true;
	demanded_variables_.push_back(variable);
}

void Analyzer::evaluated_error(uint32_t location, const char* message)
{
	// A rule of evaluated operands (5.1.1p13's objects): deferred while a
	// typeid operand may yet be unevaluated.
	if (unevaluated_ == 0)
		error(location, message);
	if (deferring())
		defer_use(
			DeferredUse{ DeferredUse::Kind::Error, false, nullptr, nullptr, nullptr, nullptr, message, location });
}

ExprFacts* Analyzer::type_identity(const syntax::Expr& expr, TokenKind op, const syntax::Slice<syntax::Expr*>& operands,
	const FunctionEntity* selected)
{
	// The course's std::type_info: `typeid(a) == typeid(b)` (and `!=`) is
	// true when both designate the same object, each type's being unique
	// (the Itanium ABI's merged type information), whatever the member
	// operator declared for it does; it is neither called nor odr-used.
	// Null for any other operator call.
	if ((op != TokenKind::OP_EQ && op != TokenKind::OP_NE) || operands.size != 2 || !IsTypeid(operands[0]) ||
		!IsTypeid(operands[1]) || type_info_ == nullptr || !selected->member || selected->owner == nullptr ||
		selected->owner->entity != type_info_)
		return nullptr;
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
	facts->flags |= kTypeIdentity;
	return facts;
}

ExprFacts* Analyzer::dynamic_cast_expression(const syntax::CastExpr& expr, const Type* target, Scope* scope)
{
	// 5.2.7: to a pointer or reference to a complete class, or to a pointer
	// to void; to the operand's own class or an accessible unambiguous base
	// (p3, p5) it is the static
	// conversion, any other checks the object's dynamic type at run time
	// (p8), its operand then pointing to or designating an object of
	// polymorphic class type (p6), without casting away constness (p2).
	ExprFacts* source = typed(*expr.operand, scope);
	if (source == nullptr)
		return nullptr;
	const bool pointer = target->kind == TypeKind::Pointer;
	const Type* to = pointer || target->is_reference() ? target->element : nullptr;
	const ClassEntity* goal = to != nullptr ? ClassOf(to) : nullptr;
	const Type* from = nullptr;
	if (pointer)
	{
		// p2: a prvalue pointer, so an array operand decays (5p9, 4.2).
		const Type* value = decayed(source);
		from = value->kind == TypeKind::Pointer ? value->element : nullptr;
	}
	else if (target->is_reference())
		from = source->type;
	const ClassEntity* origin = from != nullptr ? ClassOf(from) : nullptr;
	// p1, p2: both classes complete (instances defined, 14.7.1p1).
	if ((goal != nullptr && !require_complete(goal)) || (origin != nullptr && !require_complete(origin)))
	{
		substitution_error(expr.location, "a dynamic_cast to or from an incomplete class", true);
		return nullptr;
	}
	// p7: to a pointer to cv void, the most derived object a pointer to a
	// polymorphic class points to (p6), not casting away constness (p2).
	if (pointer && IsVoid(to))
	{
		if (origin == nullptr || !origin->polymorphic)
		{
			substitution_error(expr.location, "a dynamic_cast to void* of no pointer to a polymorphic class");
			return nullptr;
		}
		if (ConversionRules::casts_away_constness(types_.pointer(from), target->unqualified))
		{
			substitution_error(expr.location, "a dynamic_cast casting away constness");
			return nullptr;
		}
		decayed(source);
		ExprFacts* facts = new_facts(expr, target->unqualified, ValueCategory::PRValue);
		facts->flags |= kRuntimeCheck;
		return facts;
	}
	if (goal == nullptr || origin == nullptr || origin == goal || conversions_.is_base_of(goal, origin))
	{
		// The static forms, and what they reject.
		pretyped_[0] = expr.operand;
		ExprFacts* facts = explicit_conversion(expr, target, expr.operand, expr.cast, scope);
		pretyped_[0] = nullptr;
		if (facts != nullptr && (origin == nullptr || goal == nullptr))
		{
			substitution_error(expr.location, "a dynamic_cast not to or from a pointer or reference to a class");
			return nullptr;
		}
		return facts;
	}
	if (!origin->polymorphic)
	{
		substitution_error(expr.location, "a dynamic_cast of an operand of no polymorphic class");
		return nullptr;
	}
	if (ConversionRules::casts_away_constness(types_.pointer(from), types_.pointer(to)))
	{
		substitution_error(expr.location, "a dynamic_cast casting away constness");
		return nullptr;
	}
	if (!pointer && target->kind == TypeKind::LValueReference && source->category != ValueCategory::LValue)
	{
		substitution_error(expr.location, "a dynamic_cast of an rvalue to an lvalue reference");
		return nullptr;
	}
	if (pointer)
	{
		decayed(source);
		ExprFacts* facts = new_facts(expr, target->unqualified, ValueCategory::PRValue);
		facts->flags |= kRuntimeCheck;
		return facts;
	}
	const ValueCategory category = target->kind == TypeKind::LValueReference ? ValueCategory::LValue
																			 : ValueCategory::XValue;
	ExprFacts* facts = new_facts(expr, to, category);
	facts->flags |= kRuntimeCheck | kReferenceCast;
	facts->target = target;
	return facts;
}

} // namespace sema
