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
	const syntax::BracedInitList* list = initializer.expression != nullptr &&
			initializer.expression->kind == syntax::ExprKind::BracedInit
		? static_cast<const syntax::BracedInitList*>(initializer.expression)
		: nullptr;
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
		if (list != nullptr)
			cast->braced = const_cast<syntax::BracedInitList*>(list);
		else
			cast->arguments = syntax::Slice<syntax::Expr*>{ arena_.copy_array(arguments.data, arguments.size),
				arguments.size };
		type = dependent_decltype(*cast, scope);
	}
	else
		type = deduce_class_type(specifiers.deduction, arguments, scope, location,
			initializer.kind == syntax::InitializerKind::Assign, list);
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

bool Analyzer::class_deduction_copy_source(ClassEntity* templ, ClassEntity* source)
{
	if (source->templ != nullptr && source->templ->pattern == templ)
		return true;
	if (!require_complete(source))
		return false;
	for (const BaseClass* base = source->bases; base != nullptr; base = base->next)
	{
		ClassEntity* parent = const_cast<ClassEntity*>(ClassOf(base->base->type));
		if (parent != nullptr && class_deduction_copy_source(templ, parent))
			return true;
	}
	return false;
}

const Type* Analyzer::deduce_class_type(ClassEntity* templ, const syntax::Slice<syntax::Expr*>& arguments, Scope* scope,
	uint32_t location, bool copy_initialization, const syntax::BracedInitList* list)
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
	if (copy_initialization && list == nullptr)
		candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
							 [](const FunctionEntity* guide) { return guide->is_explicit; }),
			candidates.end());
	Candidate chosen = Candidate();
	Resolution resolution = Resolution::NoViable;
	bool skip_list = false;
	if (list != nullptr && arguments.size == 1)
	{
		ClassEntity* source = const_cast<ClassEntity*>(ClassOf(arguments[0]->facts->type));
		skip_list = source != nullptr && class_deduction_copy_source(templ, source);
	}
	if (list != nullptr && !skip_list)
	{
		std::vector<FunctionEntity*> first_phase;
		bool default_constructor = false;
		for (FunctionEntity* guide : candidates)
		{
			if (initializer_list_constructor(guide))
				first_phase.push_back(guide);
			if (guide->templ->deduction_guide != DeductionGuideKind::Constructor &&
				guide->templ->deduction_guide != DeductionGuideKind::TemplateConstructor)
				continue;
			const Type* type = guide->type->canonical;
			bool defaults = true;
			for (uint32_t i = 0; i < type->param_count; ++i)
				defaults = defaults && guide->default_arguments != nullptr && guide->default_arguments[i] != nullptr;
			default_constructor = default_constructor || defaults;
		}
		if ((!arguments.empty() || !default_constructor) && !first_phase.empty())
		{
			syntax::Expr* argument = const_cast<syntax::BracedInitList*>(list);
			resolution = resolve(syntax::Slice<syntax::Expr*>{ &argument, 1 }, first_phase, nullptr, chosen);
		}
	}
	if (resolution == Resolution::NoViable)
		resolution = resolve(arguments, candidates, nullptr, chosen);
	if (resolution != Resolution::Selected)
	{
		substitution_error(location, "class template argument deduction has no unique viable candidate");
		return nullptr;
	}
	if (copy_initialization && list != nullptr && chosen.function->is_explicit)
		return substitution_error(location, "an explicit deduction guide selected in copy-list initialization"),
			   nullptr;
	return chosen.function->type->element;
}

} // namespace sema
