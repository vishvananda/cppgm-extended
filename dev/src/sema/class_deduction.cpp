#include "sema/analyzer.h"
#include "sema/instantiation_context.h"

#include <algorithm>

namespace sema {

const Type* Analyzer::deduce_class_declaration(
	const Specifiers& specifiers, const syntax::InitDeclarator& init, Scope* scope, uint32_t location)
{
	for (const syntax::Declarator* declarator = init.declarator; declarator != nullptr; declarator = declarator->nested)
	{
		if (!declarator->pointers.empty() || !declarator->suffixes.empty())
			return substitution_error(location, "a deduced class placeholder requires a plain object declarator"),
				   nullptr;
	}
	if (declaration_scope(scope)->kind == ScopeKind::Class && !specifiers.is_static)
		return substitution_error(location, "a deduced class placeholder cannot declare a non-static member"), nullptr;
	syntax::Initializer empty = syntax::Initializer();
	empty.kind = syntax::InitializerKind::Parenthesized;
	const syntax::Initializer& initializer = init.initializer != nullptr ? *init.initializer : empty;
	syntax::Expr* single = initializer.expression;
	syntax::Slice<syntax::Expr*> arguments = initializer.kind == syntax::InitializerKind::Parenthesized
		? initializer.arguments
		: initializer.expression != nullptr && initializer.expression->kind == syntax::ExprKind::BracedInit
		? static_cast<const syntax::BracedInitList*>(initializer.expression)->elements
		: syntax::Slice<syntax::Expr*>{ &single, 1 };
	const Type* type = nullptr;
	if (template_depth_ != 0)
	{
		syntax::FunctionalCastExpr* cast = static_cast<syntax::FunctionalCastExpr*>(
			arena_.allocate(sizeof(syntax::FunctionalCastExpr), alignof(syntax::FunctionalCastExpr)));
		*cast = syntax::FunctionalCastExpr();
		cast->kind = syntax::ExprKind::FunctionalCast;
		cast->location = location;
		cast->type =
			syntax::Slice<syntax::DeclSpecifier>{ const_cast<syntax::DeclSpecifier*>(specifiers.type_specifier), 1 };
		cast->arguments = syntax::Slice<syntax::Expr*>{ arena_.copy_array(arguments.data, arguments.size),
			arguments.size };
		type = dependent_decltype(*cast, scope);
	}
	else
		type = deduce_class_type(specifiers.deduction, arguments, scope, location,
			initializer.kind == syntax::InitializerKind::Assign,
			initializer.expression != nullptr && initializer.expression->kind == syntax::ExprKind::BracedInit);
	return type != nullptr ? types_.qualified(type, specifiers.cv) : nullptr;
}

ClassEntity* Analyzer::deduction_template(const syntax::Slice<syntax::DeclSpecifier>& type, Scope* scope)
{
	if (type.size != 1 || type[0].kind != syntax::DeclSpecKind::Name)
		return nullptr;
	const syntax::Name& name = *type[0].name;
	if (name.last().has_arguments)
		return nullptr;
	const LookupResult found = lookup_name(name, scope, LookupFilter::Ordinary);
	if (!found.found() || found.ambiguous || found.binding->kind != BindingKind::Type ||
		found.binding->entity == nullptr || found.binding->entity->kind != EntityKind::Class)
		return nullptr;
	ClassEntity* cls = static_cast<ClassEntity*>(found.binding->entity);
	return cls->templ != nullptr && cls->templ->role == TemplateRole::Template &&
			found.binding->declared_in != cls->scope
		? cls
		: nullptr;
}

const Type* Analyzer::deduce_class_type(ClassEntity* templ, const syntax::Slice<syntax::Expr*>& arguments, Scope* scope,
	uint32_t location, bool copy_initialization, bool list_initialization)
{
	// [over.match.class.deduct]: constructors supply deduction candidates;
	// selecting a specialization and initializing its object are separate
	// operations. Preserve the original expressions for the latter.
	for (const syntax::Expr* argument : arguments)
	{
		if (argument->kind == syntax::ExprKind::BracedInit)
			braced_argument(static_cast<const syntax::BracedInitList&>(*argument), scope);
		else
			typed(*argument, scope);
	}
	if (failed())
		return nullptr;
	std::vector<FunctionEntity*> candidates;
	class_deduction_candidates(templ, candidates);
	// Explicit notional constructors do not participate in ordinary copy
	// initialization. Copy-list initialization considers them but rejects
	// an explicit winner, just as ordinary constructor overload resolution.
	if (copy_initialization && !list_initialization)
		candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
							 [](const FunctionEntity* guide) { return guide->is_explicit; }),
			candidates.end());
	Candidate chosen = Candidate();
	const Resolution resolution = resolve(arguments, candidates, nullptr, chosen);
	if (resolution != Resolution::Selected)
	{
		substitution_error(location, "class template argument deduction has no unique viable candidate");
		return nullptr;
	}
	if (copy_initialization && list_initialization && chosen.function->is_explicit)
		return substitution_error(location, "an explicit deduction guide selected in copy-list initialization"),
			   nullptr;
	return chosen.function->type->element;
}

} // namespace sema
