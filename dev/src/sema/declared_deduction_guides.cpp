#include "sema/analyzer.h"

namespace sema {

void Analyzer::deduction_guide_declaration(const syntax::DeductionGuideDecl& decl, Scope* scope)
{
	// Hosted C++11 headers have historically accepted guides as declarations
	// with no effect. Only C++17 performs class template argument deduction.
	if (!cxx17_)
		return;
	const LookupResult found = names_.unqualified(scope, decl.name, LookupFilter::Ordinary);
	ClassEntity* cls = found.found() && !found.ambiguous ? class_template_of(found.binding->entity) : nullptr;
	if (cls == nullptr || found.binding->kind != BindingKind::Type || cls->owner != declaration_scope(scope))
		error(decl.location, "a deduction guide must be declared in its class template's scope");
	if (cls->owner->kind == ScopeKind::Class)
	{
		const Access access = current_access_ == TokenKind::KW_PRIVATE ? Access::Private
			: current_access_ == TokenKind::KW_PROTECTED               ? Access::Protected
																	   : Access::Public;
		if (found.binding->access != access)
			error(decl.location, "a deduction guide must have its class template's access");
	}
	// [temp.deduct.guide]: the result is a simple-template-id naming this
	// same class template, not a pointer, alias or unrelated specialization.
	const syntax::TypeId& result = *decl.result;
	if (result.declarator != nullptr || result.specifiers.size != 1 ||
		result.specifiers[0].kind != syntax::DeclSpecKind::Name)
		error(decl.location, "a deduction guide requires a class template specialization result");
	const syntax::Name& name = *result.specifiers[0].name;
	if (name.components.size != 1 || name.last().identifier != decl.name || !name.last().has_arguments)
		error(decl.location, "a deduction guide result must name its class template");
	syntax::FunctionQualifier trailing = syntax::FunctionQualifier();
	trailing.kind = syntax::FunctionQualifierKind::TrailingReturn;
	trailing.trailing = decl.result;
	syntax::DeclaratorSuffix suffix = syntax::DeclaratorSuffix();
	suffix.kind = syntax::DeclaratorSuffixKind::Function;
	suffix.parameters = decl.parameters;
	suffix.qualifiers = syntax::Slice<syntax::FunctionQualifier>{ &trailing, 1 };
	const PendingPrototype saved = pending_prototype_;
	const Type* type = apply_suffix(suffix, auto_placeholder_, scope, decl.location, nullptr);
	pending_prototype_ = saved;
	if (class_template_of(ClassOf(type->element)) != cls)
		error(decl.location, "a deduction guide result must specialize its class template");
	FunctionEntity* guide = new_entity<FunctionEntity>(EntityKind::Function, decl.name, cls->owner, type);
	guide->signature = signature_of(type);
	guide->parameters = decl.parameters;
	guide->is_explicit = decl.is_explicit;
	guide->templated = template_list_ != nullptr && scope == template_scope_;
	if (guide->templated)
	{
		FunctionTemplateFacts* facts = function_template_facts(guide);
		facts->head = template_head(*template_list_, template_scope_, template_entities_);
		facts->head->shape = template_shape(facts->head->parameters, facts->head->count);
		facts->trailing_result = true;
		facts->deduction_guide = DeductionGuideKind::Declared;
	}
	record_default_arguments(guide, decl.parameters, scope);
	ClassDeductionSet& set = class_deduction_set(cls);
	for (const FunctionEntity* previous : set.declared)
	{
		if (previous->signature == guide->signature && previous->templated == guide->templated &&
			(!guide->templated || HeadOf(previous)->shape == HeadOf(guide)->shape))
			error(decl.location, "equivalent deduction guides in one translation unit");
	}
	// Guides are kept with the class deduction facts, never entered into
	// the ordinary function/name table or emitted by lowering.
	set.declared.push_back(guide);
}

} // namespace sema
