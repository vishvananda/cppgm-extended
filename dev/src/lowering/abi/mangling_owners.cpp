#include "lowering/abi/mangling_owners.h"

#include "lowering/support/errors.h"

#include <utility>

namespace cppgm
{
namespace lowering
{
namespace abi
{

using namespace semantic;

void AppendTypedFact(abi_mangle::AbiTypedCase* facts,
	abi_mangle::AbiFactRecord* record)
{
	using namespace abi_mangle;
	switch (record->kind)
	{
	case ABI_FACT_RECORD_DEFINITION:
		facts->definitions.push_back(std::move(record->definition));
		return;
	case ABI_FACT_RECORD_FUNCTION:
		facts->functions.push_back(std::move(record->function));
		return;
	case ABI_FACT_RECORD_TARGET:
		if (facts->has_target)
			ThrowLoweringInternal("production ABI case has two targets");
		facts->target = std::move(record->target);
		facts->has_target = true;
		return;
	}
	ThrowLoweringInternal("invalid production ABI fact record");
}

void AppendFunctionAbiTagFacts(const semantic::Program& program,
	const BindingRecord& binding, abi_mangle::AbiMangleContext* context,
	abi_mangle::AbiTypedCase* facts)
{
	using namespace abi_mangle;
	if (binding.abi_tag_count == 0) return;
	if (binding.abi_tag_begin > program.abi_tags.size() ||
		binding.abi_tag_count > program.abi_tags.size() - binding.abi_tag_begin)
		ThrowLoweringInternal("invalid function ABI tag fact range");
	for (std::size_t i = 0; i < binding.abi_tag_count; ++i)
	{
		AbiFactRecord tag;
		tag.set_kind(ABI_FACT_RECORD_FUNCTION);
		tag.function.kind = ABI_FUNCTION_RECORD_ABI_TAG;
		const NameId name = program.abi_tags[binding.abi_tag_begin + i];
		tag.function.set_resolved_source_name(
			context->resolve_external_name(name, program.names.Get(name)));
		AppendTypedFact(facts, &tag);
	}
}

void AppendComponentAbiTagFacts(const semantic::Program& program,
	const EntityRecord& entity, abi_mangle::AbiMangleContext* context,
	abi_mangle::AbiTypedCase* facts)
{
	using namespace abi_mangle;
	if (entity.abi_tag_begin > program.abi_tags.size() ||
		entity.abi_tag_count > program.abi_tags.size() - entity.abi_tag_begin)
		ThrowLoweringInternal("invalid component ABI tag fact range");
	for (std::size_t i = 0; i < entity.abi_tag_count; ++i)
	{
		AbiFactRecord tag;
		tag.set_kind(ABI_FACT_RECORD_FUNCTION);
		tag.function.kind = ABI_FUNCTION_RECORD_COMPONENT_ABI_TAG;
		const NameId name = program.abi_tags[entity.abi_tag_begin + i];
		tag.function.set_resolved_source_name(
			context->resolve_external_name(name, program.names.Get(name)));
		AppendTypedFact(facts, &tag);
	}
}

bool IsClassTemplateSpecialization(const EntityRecord& entity)
{
	return entity.template_argument_begin != kNoBinding;
}

const semantic::BindingRecord* InheritedConstructorAbiSource(
	const semantic::Program& program, const semantic::BindingRecord& binding,
	const semantic::DumpNode& node)
{
	using namespace semantic;
	if (!binding.constructor || node.selected_binding == kNoBinding) return 0;
	if (node.selected_binding >= program.bindings.size())
		ThrowLoweringInternal("inherited constructor ABI source is invalid");
	const BindingRecord& source = program.bindings[node.selected_binding];
	if (!source.constructor || source.member_owner == kNoEntity ||
		source.member_owner >= program.entities.size())
		ThrowLoweringInternal("inherited constructor ABI source has no base class");
	return &source;
}

const semantic::FunctionTemplateAbiRecipe* FunctionAbiRecipe(
	const semantic::Program& program, const semantic::BindingRecord& binding)
{
	using namespace semantic;
	if (binding.function_template_abi_recipe == kNoFunctionTemplateAbiRecipe) return 0;
	if (binding.function_template_abi_recipe >= program.function_template_abi_recipes.size())
		ThrowLoweringInternal("invalid function template ABI recipe");
	const FunctionTemplateAbiRecipe& recipe =
		program.function_template_abi_recipes[binding.function_template_abi_recipe];
	if (!program.types.IsFunction(recipe.function_type))
		ThrowLoweringInternal("function template ABI recipe is not callable");
	return &recipe;
}

bool ClassOwnerHasAbiTags(const semantic::Program& program, EntityId entity)
{
	for (std::size_t depth = 0;
		entity != kNoEntity && entity < program.entities.size() &&
			depth < program.entities.size(); ++depth)
	{
		const EntityRecord& record = program.entities[entity];
		if (record.abi_tag_count != 0) return true;
		entity = record.enclosing_class;
	}
	return false;
}

bool ClassOwnerHasSpecializedAncestor(const semantic::Program& program, EntityId entity)
{
	bool first = true;
	for (std::size_t depth = 0;
		entity != kNoEntity && entity < program.entities.size() &&
			depth < program.entities.size(); ++depth)
	{
		const EntityRecord& record = program.entities[entity];
		if (!first && IsClassTemplateSpecialization(record)) return true;
		first = false;
		entity = record.enclosing_class;
	}
	return false;
}

bool VariableHasInternalMarker(const semantic::Program& program,
	const BindingRecord& binding)
{
	return binding.storage_class == STORAGE_CLASS_STATIC &&
		binding.member_owner == kNoEntity && !binding.anonymous_union_storage &&
		program.KindOfScope(binding.owner) == SCOPE_NAMESPACE &&
		!program.HasInternalLinkageScope(binding.owner);
}

NameId VariableAbiName(const BindingRecord& binding)
{
	return binding.anonymous_union_storage &&
		binding.anonymous_union_object_name != 0 ?
		binding.anonymous_union_object_name : binding.name;
}

bool ClassOwnerIsNested(const semantic::Program& program, EntityId entity)
{
	return entity != kNoEntity && entity < program.entities.size() &&
		program.entities[entity].enclosing_class != kNoEntity;
}

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
