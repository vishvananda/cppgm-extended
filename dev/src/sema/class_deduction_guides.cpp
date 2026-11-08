#include "sema/analyzer.h"

namespace sema {

TemplateHead* Analyzer::class_deduction_head(const TemplateHead* outer, const TemplateHead* inner, TypeList& remap)
{
	if (inner == nullptr)
		return const_cast<TemplateHead*>(outer);
	TemplateHead* head = static_cast<TemplateHead*>(arena_.allocate(sizeof(TemplateHead), alignof(TemplateHead)));
	*head = TemplateHead();
	head->count = outer->count + inner->count;
	head->ordinal = inner->ordinal;
	head->parameters = arena_.allocate_array<TemplateParameterEntity*>(head->count);
	head->defaults = arena_.allocate_array<const syntax::TypeId*>(head->count);
	head->default_values = arena_.allocate_array<const syntax::Expr*>(head->count);
	head->default_scopes = arena_.allocate_array<Scope*>(head->count);
	head->argument_defaults = inner->argument_defaults;
	const uint32_t depth = outer->count != 0 ? outer->parameters[0]->depth : 0;
	for (uint32_t i = 0; i < head->count; ++i)
	{
		const TemplateHead* source = i < outer->count ? outer : inner;
		const uint32_t position = i < outer->count ? i : i - outer->count;
		TemplateParameterEntity* parameter = source->parameters[position];
		if (i >= outer->count)
		{
			TemplateParameterEntity* renamed = new_entity<TemplateParameterEntity>(
				EntityKind::TemplateParameter, parameter->name, parameter->owner, nullptr);
			*renamed = *parameter;
			renamed->depth = static_cast<uint16_t>(depth);
			renamed->index = static_cast<uint16_t>(i);
			renamed->canonical = parameter->template_template
				? positional_template_parameter(depth, i, parameter->pack, parameter->template_head)
				: positional_parameter(depth, i, parameter->non_type, parameter->pack);
			renamed->type = types_.named(
				parameter->template_template ? TypeKind::TemplateTemplateParameter : TypeKind::TemplateParameter,
				renamed);
			parameter = renamed;
			const Type* argument = parameter->pack ? types_.expansion(parameter->type) : parameter->type;
			remap.push_back(parameter->pack ? types_.pack(&argument, 1) : argument);
		}
		head->parameters[i] = parameter;
		head->defaults[i] = source->defaults[position];
		head->default_values[i] = source->default_values[position];
		head->default_scopes[i] = source->default_scopes[position];
	}
	if (inner->count != 0)
	{
		const Substitution renamed = substitution_of(inner, remap.data());
		for (uint32_t i = outer->count; i < head->count; ++i)
		{
			TemplateParameterEntity* parameter = head->parameters[i];
			if (parameter->value_type != nullptr)
				parameter->value_type = substitute(parameter->value_type, renamed);
		}
	}
	head->trailing_pack = head->count != 0 && head->parameters[head->count - 1]->pack ? head->count - 1 : head->count;
	head->shape = template_shape(head->parameters, head->count);
	DeductionHeadOrigins* origins = static_cast<DeductionHeadOrigins*>(
		arena_.allocate(sizeof(DeductionHeadOrigins), alignof(DeductionHeadOrigins)));
	*origins = DeductionHeadOrigins{ outer, inner };
	head->deduction_origins = origins;
	return head;
}

FunctionEntity* Analyzer::class_deduction_guide(ClassEntity* cls, const FunctionEntity* constructor, bool copy)
{
	const TemplateHead* outer = cls->templ->head;
	const TemplateHead* inner = constructor != nullptr && constructor->templated ? HeadOf(constructor) : nullptr;
	TypeList remap;
	TemplateHead* head = class_deduction_head(outer, inner, remap);
	TypeList parameters;
	bool variadic = false;
	if (copy)
		parameters.push_back(cls->type);
	else if (constructor != nullptr)
	{
		const Type* type = constructor->type->canonical;
		if (inner != nullptr && inner->count != 0)
			substitute_list(type->params, type->param_count, substitution_of(inner, remap.data()), parameters);
		else
			for (uint32_t i = 0; i < type->param_count; ++i)
				parameters.push_back(type->params[i]);
		variadic = type->variadic;
	}
	FunctionEntity* guide = new_entity<FunctionEntity>(EntityKind::Function, cls->name, cls->owner,
		types_.function(
			cls->type, parameters.data(), static_cast<uint32_t>(parameters.size()), variadic, 0, RefQualifier::None));
	guide->signature = guide->type;
	guide->templated = true;
	guide->is_explicit = constructor != nullptr && constructor->is_explicit;
	guide->default_arguments = constructor != nullptr ? constructor->default_arguments : nullptr;
	guide->parameters = constructor != nullptr ? constructor->parameters : nullptr;
	FunctionTemplateFacts* facts = function_template_facts(guide);
	facts->head = head;
	facts->class_deduction_parameters = outer->count;
	facts->deduction_guide = copy ? DeductionGuideKind::Copy
		: inner != nullptr        ? DeductionGuideKind::TemplateConstructor
								  : DeductionGuideKind::Constructor;
	return guide;
}

Analyzer::ClassDeductionSet& Analyzer::class_deduction_set(ClassEntity* cls)
{
	uint32_t index = class_deduction_index_.find(cls);
	if (index == PointerMap::kMissing)
	{
		index = static_cast<uint32_t>(class_deduction_sets_.size());
		class_deduction_sets_.push_back(ClassDeductionSet());
		class_deduction_index_.set(cls, index);
	}
	return class_deduction_sets_[index];
}

void Analyzer::class_deduction_candidates(ClassEntity* cls, std::vector<FunctionEntity*>& out)
{
	ClassDeductionSet& set = class_deduction_set(cls);
	if (set.implicit.empty())
	{
		std::vector<FunctionEntity*>& guides = set.implicit;
		const NameEntry* entry = names_.find(cls->scope, constructor_key_);
		for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr; node != nullptr;
			node = node->next)
			guides.push_back(
				class_deduction_guide(cls, static_cast<const FunctionEntity*>(node->binding->entity), false));
		if (guides.empty())
			guides.push_back(class_deduction_guide(cls, nullptr, false));
		guides.push_back(class_deduction_guide(cls, nullptr, true));
	}
	out = set.implicit;
	out.insert(out.end(), set.declared.begin(), set.declared.end());
}

} // namespace sema
