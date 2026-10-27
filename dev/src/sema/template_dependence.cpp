#include "sema/analyzer.h"

// Type-dependence of a template definition's expressions (14.6.2.2):
// whether an expression's type may differ between instantiations, decided
// from its names, types and operands where it appears. The answer is
// conservative: a form the walk does not decide is taken as dependent, so
// only an expression proven non-dependent is analyzed in the definition
// (14.6p8, p10).
namespace sema {

bool Analyzer::TemplatedClassScope(const Scope* scope)
{
	// A class scope of a templated class (a class template, its partial
	// specialization, or a member class of one) or a scope in one: its
	// `this` and its members' names are type-dependent (14.6.2.2p2-p3).
	for (; scope != nullptr; scope = scope->parent)
	{
		if (scope->kind == ScopeKind::Class && scope->entity != nullptr && templated_class(scope))
			return true;
	}
	return false;
}

bool Analyzer::type_dependent(const syntax::Expr* expr, Scope* scope)
{
	if (expr == nullptr)
		return false;
	// Operands nested beyond the stack's budget are an error (Annex B), as
	// the expression's own analysis finds them.
	check_stack(expr->location);
	switch (expr->kind)
	{
	case syntax::ExprKind::Id:
		return dependent_name_expression(*static_cast<const syntax::IdExpr*>(expr)->name, scope);
	case syntax::ExprKind::Literal:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::Delete:
	case syntax::ExprKind::Throw:
		// Of type std::size_t, bool, const std::type_info or void (5.3.3,
		// 5.3.6, 5.3.7, 5.2.8, 5.3.5, 15.1).
		return false;
	case syntax::ExprKind::BuiltinTrait:
	{
		// A trait is a bool; a type transform's operand is of the type it
		// names, typeof's of its expression's or type-id's.
		const syntax::BuiltinTraitExpr& trait = *static_cast<const syntax::BuiltinTraitExpr*>(expr);
		// _BitInt has a width expression rather than a type operand. Its
		// resulting type depends on that expression's value.
		if (trait.trait == GnuTrait::BitInt)
			return trait.operand == nullptr || instantiation_dependent(*trait.operand, scope);
		if ((trait.trait == GnuTrait::TypeofOperand || trait.trait == GnuTrait::RealPart ||
				trait.trait == GnuTrait::ImagPart) &&
			trait.operand != nullptr)
			return type_dependent(trait.operand, scope);
		if (!IsGnuTypeTransform(trait.trait) && trait.trait != GnuTrait::TypeofOperand &&
			trait.trait != GnuTrait::VaArg && trait.trait != GnuTrait::ConvertVector &&
			trait.trait != GnuTrait::BitCast)
			return false;
		const Type* type = type_id(*trait.types[0], scope);
		return type == nullptr || dependent_type(type);
	}
	case syntax::ExprKind::KeywordLiteral:
		return static_cast<const syntax::KeywordLiteralExpr*>(expr)->keyword == TokenKind::KW_THIS &&
			TemplatedClassScope(scope);
	case syntax::ExprKind::Paren:
		return type_dependent(static_cast<const syntax::ParenExpr*>(expr)->inner, scope);
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(expr);
		return type_dependent(binary->left, scope) || type_dependent(binary->right, scope);
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* conditional = static_cast<const syntax::ConditionalExpr*>(expr);
		return type_dependent(conditional->condition, scope) || type_dependent(conditional->then_expr, scope) ||
			type_dependent(conditional->else_expr, scope);
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		return type_dependent(static_cast<const syntax::UnaryExpr*>(expr)->operand, scope);
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(expr);
		if (type_dependent(call->callee, scope))
			return true;
		for (const syntax::Expr* argument : call->arguments)
		{
			if (type_dependent(argument, scope))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr* subscript = static_cast<const syntax::SubscriptExpr*>(expr);
		return type_dependent(subscript->base, scope) || type_dependent(subscript->index, scope);
	}
	case syntax::ExprKind::Member:
	{
		// A member of a non-dependent object: its template arguments decide.
		const syntax::MemberExpr* member = static_cast<const syntax::MemberExpr*>(expr);
		if (type_dependent(member->base, scope))
			return true;
		for (const syntax::NameComponent& component : member->member->components)
		{
			if (component.kind != syntax::NameComponentKind::Identifier || component.has_arguments)
				return true;
		}
		return false;
	}
	case syntax::ExprKind::Cast:
	{
		const syntax::CastExpr* cast = static_cast<const syntax::CastExpr*>(expr);
		const Type* type = type_id(*cast->type, scope);
		return type == nullptr || dependent_type(type) || type_dependent(cast->operand, scope);
	}
	case syntax::ExprKind::BracedInit:
		for (const syntax::Expr* element : static_cast<const syntax::BracedInitList*>(expr)->elements)
		{
			if (type_dependent(element, scope))
				return true;
		}
		return false;
	case syntax::ExprKind::FunctionalCast:
	{
		// (check_fixed_operands' reading: of a non-dependent type, its
		// operands not type-dependent.)
		if (!precise_dependence_)
			return true;
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(expr);
		Specifiers specifiers;
		this->specifiers(cast->type, scope, specifiers, false);
		if (specifiers.type == nullptr || dependent_type(specifiers.type))
			return true;
		if (cast->braced != nullptr)
			return type_dependent(cast->braced, scope);
		for (const syntax::Expr* argument : cast->arguments)
		{
			if (type_dependent(argument, scope))
				return true;
		}
		return false;
	}
	default:
		// Functional casts, new-expressions, lambdas and pack expansions.
		return true;
	}
}

bool Analyzer::fixed_expression(const syntax::Expr& operand, Scope* scope)
{
	// An expression of a template definition that names no dependent
	// entity or type (its placeholder key carries no dependent mark, see
	// placeholder_words) and is not type-dependent: the same in every
	// instantiation (14.6.2.2, 14.6.2.3).
	const size_t base = decltype_words_.size();
	uint64_t* const enclosing = named_parameters_;
	uint64_t named = 0;
	named_parameters_ = &named;
	const bool sizeof_pack = keyed_sizeof_pack_;
	const bool callee_lookup = keyed_callee_lookup_;
	const bool callee = keyed_callee_;
	const bool keyed = placeholder_words(operand, scope, scope, false, nullptr);
	const bool fixed = keyed && (decltype_words_[base] & 1) == 0;
	decltype_words_.resize(base);
	named_parameters_ = enclosing;
	keyed_sizeof_pack_ = sizeof_pack;
	keyed_callee_lookup_ = callee_lookup;
	keyed_callee_ = callee;
	return fixed && !type_dependent(&operand, scope);
}

bool Analyzer::dependent_name_expression(const syntax::Name& name, Scope* scope)
{
	// 14.6.2.2p3: an id-expression naming a template parameter, a
	// declaration of dependent type, a member of a templated class, or a
	// dependent qualified or template-id name; one that names nothing (an
	// argument-dependent call's callee) is decided by its call. Only an
	// unqualified identifier, or one qualified by a namespace or by a class
	// nested only in classes and namespaces that is not templated (no
	// member of it depends on a template parameter), is decided.
	// (check_fixed_operands' reading: a function template-id whose
	// arguments are non-dependent types names specializations whose types
	// its call's arguments decide, 14.6.2.2p3.)
	bool fixed_template_id = false;
	for (const syntax::NameComponent& component : name.components)
	{
		if (component.kind != syntax::NameComponentKind::Identifier)
			return true;
		if (!component.has_arguments)
			continue;
		if (!precise_dependence_ || &component != &name.last() || !fixed_template_arguments(component, scope))
			return true;
		fixed_template_id = true;
	}
	LookupResult found = LookupResult();
	if (!name.qualified())
		found = names_.unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
	else
	{
		const NamePrefix prefix = name_prefix(name, scope);
		if (failed() || prefix.dependent != nullptr || prefix.scope == nullptr)
			return true;
		for (const Scope* enclosing = prefix.scope; enclosing->kind != ScopeKind::Namespace;
			enclosing = enclosing->parent)
		{
			if (enclosing->kind != ScopeKind::Class || enclosing->entity == nullptr || templated_class(enclosing) ||
				enclosing->parent == nullptr)
				return true;
		}
		found = names_.qualified(prefix.scope, name.last().identifier, LookupFilter::Ordinary);
	}
	if (!found.found() || found.ambiguous || found.binding->entity == nullptr)
		return !found.found() && !name.qualified();
	const Binding* binding = found.binding;
	const Entity* entity = binding->entity;
	if (entity->owner != nullptr && TemplatedClassScope(entity->owner))
		return true;
	switch (binding->kind)
	{
	case BindingKind::Variable:
		return dependent_type(entity->type);
	case BindingKind::Parameter:
		return dependent_type(binding->type);
	case BindingKind::Enumerator:
		return false;
	case BindingKind::Function:
		if (fixed_template_id)
			return false;
		for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
		{
			if (dependent_type(node->binding->entity->type))
				return true;
		}
		return found.functions == nullptr && dependent_type(entity->type);
	default:
		return true;
	}
}

bool Analyzer::fixed_template_arguments(const syntax::NameComponent& component, Scope* scope)
{
	// A template argument list of types alone, none dependent.
	for (const syntax::TemplateArgument& argument : component.arguments)
	{
		if (argument.type == nullptr || argument.pack)
			return false;
		const Type* type = type_id(*argument.type, scope);
		if (failed() || type == nullptr || dependent_type(type))
			return false;
	}
	return true;
}

void Analyzer::check_fixed_operands(const syntax::Expr& operand, Scope* scope, uint32_t level)
{
	// A member template's declaration that an instantiation of its class
	// declares (14.7.1p1, 14.5.5.3p2) has its enclosing templates'
	// parameters replaced: a call or member access in a decltype operand
	// that now depends on none of its own parameters is not dependent, and
	// one that is invalid makes the instance ill-formed, outside any
	// deduction (14.8.2p8). Each such operand is typed for its errors; the
	// decltype stays dependent on the rest.
	if (level > 64)
		return;
	const syntax::Expr* expr = &operand;
	switch (expr->kind)
	{
	case syntax::ExprKind::Call:
	case syntax::ExprKind::Member:
	{
		precise_dependence_ = true;
		const bool fixed = !type_dependent(expr, scope) && !failed() && fixed_expression(*expr, scope);
		precise_dependence_ = false;
		if (failed())
			return;
		if (fixed)
		{
			const uint32_t depth = template_depth_;
			template_depth_ = 0;
			decltype_type(*expr, scope);
			template_depth_ = depth;
			return;
		}
		if (expr->kind == syntax::ExprKind::Member)
			return check_fixed_operands(*static_cast<const syntax::MemberExpr*>(expr)->base, scope, level + 1);
		// A member function's name is typed only with its call: its
		// object expression is.
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(expr);
		const syntax::Expr* callee = call->callee;
		while (callee->kind == syntax::ExprKind::Paren)
			callee = static_cast<const syntax::ParenExpr*>(callee)->inner;
		if (callee->kind == syntax::ExprKind::Member)
			check_fixed_operands(*static_cast<const syntax::MemberExpr*>(callee)->base, scope, level + 1);
		for (const syntax::Expr* argument : call->arguments)
			check_fixed_operands(*argument, scope, level + 1);
		return;
	}
	case syntax::ExprKind::Paren:
		return check_fixed_operands(*static_cast<const syntax::ParenExpr*>(expr)->inner, scope, level + 1);
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr* binary = static_cast<const syntax::BinaryExpr*>(expr);
		check_fixed_operands(*binary->left, scope, level + 1);
		return check_fixed_operands(*binary->right, scope, level + 1);
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* conditional = static_cast<const syntax::ConditionalExpr*>(expr);
		check_fixed_operands(*conditional->condition, scope, level + 1);
		check_fixed_operands(*conditional->then_expr, scope, level + 1);
		return check_fixed_operands(*conditional->else_expr, scope, level + 1);
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		return check_fixed_operands(*static_cast<const syntax::UnaryExpr*>(expr)->operand, scope, level + 1);
	case syntax::ExprKind::FunctionalCast:
		for (const syntax::Expr* argument : static_cast<const syntax::FunctionalCastExpr*>(expr)->arguments)
			check_fixed_operands(*argument, scope, level + 1);
		return;
	case syntax::ExprKind::Cast:
		return check_fixed_operands(*static_cast<const syntax::CastExpr*>(expr)->operand, scope, level + 1);
	default:
		return;
	}
}

} // namespace sema
