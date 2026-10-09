#include "lowering/abi/mangling.h"
#include "lowering/abi/local_entity_names.h"
#include "lowering/abi/mangling_owners.h"
#include "lowering/abi/mangling_vocabulary.h"

#include "abi/itanium/abi_mangle.h"
#include "lowering/ir/model.h"
#include "lowering/objects/polymorphism.h"
#include "lowering/support/errors.h"
#include "support/containers/flat_hash_set.h"

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cppgm
{
namespace lowering
{
namespace abi
{


namespace
{

using namespace semantic;


class AbiFactBuilder
{
	public:
	struct LocalContextHandle
	{
		std::size_t storage;
		std::size_t identity;
	};

	private:
	struct TypeArgumentCacheKey
	{
		TypeId type;
		BindingId function;
		FunctionTemplateAbiRecipeId recipe;

		TypeArgumentCacheKey(TypeId type_value, BindingId function_value,
			FunctionTemplateAbiRecipeId recipe_value)
			: type(type_value), function(function_value), recipe(recipe_value) {}

		bool operator==(const TypeArgumentCacheKey& other) const
		{
			return type == other.type && function == other.function &&
				recipe == other.recipe;
		}
	};

	struct TypeArgumentCacheEntry
	{
		TypeArgumentCacheKey key;
		std::size_t id;

		TypeArgumentCacheEntry(const TypeArgumentCacheKey& key_value,
			std::size_t id_value) : key(key_value), id(id_value) {}
	};

	const semantic::Program& program_;
	abi_mangle::AbiTypedCase& facts_;
	abi_mangle::AbiMangleContext* context_;
	abi_mangle::AbiMangleStats* stats_;
	LocalNames* names_;
	std::size_t next_argument_;
	std::vector<TypeArgumentCacheEntry> type_argument_cache_;
	std::vector<std::uint32_t> type_argument_cache_slots_;
	std::vector<semantic::NameId> semantic_path_scratch_;
	std::vector<std::size_t> resolved_path_scratch_;
	// One context per function within a case: every use of a local type of
	// the same function is then the same canonical type, one substitution
	// candidate (`_ZN8functionIZN1S1gEvE1GEC2ES1_`).
	std::vector<std::pair<std::uint64_t, std::size_t> > local_contexts_;
	std::vector<LocalContextHandle> local_context_handles_;

	TypeArgumentCacheKey TypeArgumentKey(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe) const
	{
		const BindingId function_id = function ?
			function->canonical : kNoBinding;
		FunctionTemplateAbiRecipeId recipe_id = kNoFunctionTemplateAbiRecipe;
		if (recipe)
		{
			if (program_.function_template_abi_recipes.empty())
				ThrowLoweringInternal("ABI type argument recipe has no owner");
			const FunctionTemplateAbiRecipe* begin =
				&program_.function_template_abi_recipes[0];
			const std::ptrdiff_t offset = recipe - begin;
			if (offset < 0 || static_cast<std::size_t>(offset) >=
				program_.function_template_abi_recipes.size())
				ThrowLoweringInternal("ABI type argument recipe is invalid");
			recipe_id = static_cast<FunctionTemplateAbiRecipeId>(offset);
		}
		return TypeArgumentCacheKey(type, function_id, recipe_id);
	}

	static std::size_t TypeArgumentCacheHash(
		const TypeArgumentCacheKey& key)
	{
		return MixHash(MixHash(key.type, key.function), key.recipe);
	}

	const std::size_t* FindTypeArgument(
		const TypeArgumentCacheKey& key) const
	{
		const std::size_t mask = type_argument_cache_slots_.size() - 1;
		std::size_t slot = TypeArgumentCacheHash(key) & mask;
		while (type_argument_cache_slots_[slot] != 0)
		{
			const TypeArgumentCacheEntry& entry = type_argument_cache_[
				type_argument_cache_slots_[slot] - 1];
			if (entry.key == key) return &entry.id;
			slot = (slot + 1) & mask;
		}
		return 0;
	}

	void RehashTypeArguments(std::size_t capacity)
	{
		std::vector<std::uint32_t> replacement(capacity, 0);
		const std::size_t mask = capacity - 1;
		for (std::size_t entry = 0; entry < type_argument_cache_.size(); ++entry)
		{
			std::size_t slot =
				TypeArgumentCacheHash(type_argument_cache_[entry].key) & mask;
			while (replacement[slot] != 0) slot = (slot + 1) & mask;
			replacement[slot] = static_cast<std::uint32_t>(entry + 1);
		}
		type_argument_cache_slots_.swap(replacement);
	}

	void CacheTypeArgument(const TypeArgumentCacheKey& key,
		std::size_t id)
	{
		if ((type_argument_cache_.size() + 1) * 10 >
			type_argument_cache_slots_.size() * 7)
			RehashTypeArguments(type_argument_cache_slots_.size() * 2);
		if (type_argument_cache_.size() >=
			std::numeric_limits<std::uint32_t>::max())
			ThrowLoweringResourceLimit("too many ABI type argument facts");
		const std::size_t mask = type_argument_cache_slots_.size() - 1;
		std::size_t slot = TypeArgumentCacheHash(key) & mask;
		while (type_argument_cache_slots_[slot] != 0)
			slot = (slot + 1) & mask;
		type_argument_cache_.push_back(TypeArgumentCacheEntry(key, id));
		type_argument_cache_slots_[slot] =
			static_cast<std::uint32_t>(type_argument_cache_.size());
	}

	std::size_t ResolvePath(semantic::ScopeId owner, semantic::NameId terminal)
	{
		program_.BuildEmissionPath(owner, terminal, &semantic_path_scratch_);
		return ResolvePath(semantic_path_scratch_, 0,
			semantic_path_scratch_.size());
	}

	public:
	LocalNames& Names()
	{
		if (!names_) ThrowLoweringInternal("ABI local names have no unit state");
		return *names_;
	}

	std::string AnonymousName(semantic::EntityId entity)
	{
		return "$_" + std::to_string(Names().AnonymousNumber(entity));
	}

	// A unit-local unnamed class or closure type of namespace scope is the
	// source name `$_<n>` in its namespace.
	std::size_t ResolveAnonymousPath(semantic::EntityId entity)
	{
		return ResolveGeneratedPath(program_.entities[entity].lambda_closure ?
			ClosureNamingScope(program_, entity) : program_.entities[entity].owner,
			AnonymousName(entity));
	}

	// A member of a namespace-scope class named `$_<n>` (an unnamed class or
	// a closure type without a mangling number).
	bool SetAnonymousMemberPath(abi_mangle::AbiFunctionTarget* target,
		semantic::EntityId owner, std::size_t terminal)
	{
		using namespace semantic;
		if (owner == kNoEntity || owner >= program_.entities.size()) return false;
		const EntityRecord& record = program_.entities[owner];
		if (record.lambda_closure ? !ClosureIsNamespaceScope(program_, owner) :
			(record.local_context != kNoBinding || !IsUnnamedClass(program_, owner) ||
			 record.enclosing_class != kNoEntity))
			return false;
		target->resolved_path = context_->resolve_path_component(
			ResolveAnonymousPath(owner), terminal);
		return true;
	}

	private:

	std::size_t ResolvePath(const std::vector<semantic::NameId>& path,
		std::size_t begin, std::size_t end)
	{
		if (begin >= end || end > path.size())
			ThrowLoweringInternal("invalid semantic ABI path range");
		resolved_path_scratch_.clear();
		resolved_path_scratch_.reserve(end - begin);
		for (std::size_t i = begin; i < end; ++i)
		{
			const semantic::NameId name = path[i];
			resolved_path_scratch_.push_back(context_->resolve_external_name(
				name, program_.names.Get(name)));
		}
		return context_->resolve_path(resolved_path_scratch_);
	}

	std::size_t ResolveName(semantic::NameId name)
	{
		return context_->resolve_external_name(name, program_.names.Get(name));
	}

	std::size_t ResolveScopePath(semantic::ScopeId owner)
	{
		program_.BuildEmissionPath(owner, 0, &semantic_path_scratch_);
		if (!semantic_path_scratch_.empty())
			semantic_path_scratch_.pop_back();
		if (semantic_path_scratch_.empty())
			return abi_mangle::ABI_NO_RESOLVED_REFERENCE;
		return ResolvePath(semantic_path_scratch_, 0,
			semantic_path_scratch_.size());
	}

	std::size_t ResolveGeneratedPath(semantic::ScopeId owner,
		const std::string& terminal)
	{
		program_.BuildEmissionPath(owner, 0, &semantic_path_scratch_);
		if (!semantic_path_scratch_.empty())
			semantic_path_scratch_.pop_back();
		resolved_path_scratch_.clear();
		resolved_path_scratch_.reserve(semantic_path_scratch_.size() + 1);
		for (std::size_t i = 0; i < semantic_path_scratch_.size(); ++i)
		{
			const semantic::NameId name = semantic_path_scratch_[i];
			resolved_path_scratch_.push_back(
				context_->resolve_external_name(
					name, program_.names.Get(name)));
		}
		resolved_path_scratch_.push_back(
			context_->resolve_generated_name(terminal));
		return context_->resolve_path(resolved_path_scratch_);
	}

	void AppendTypeAbiTags(abi_mangle::AbiType* destination,
		std::uint32_t begin, std::uint32_t count)
	{
		if (begin > program_.abi_tags.size() ||
			count > program_.abi_tags.size() - begin)
			ThrowLoweringInternal("invalid ABI tag fact range");
		for (std::size_t i = 0; i < count; ++i)
			destination->presentation_names.push_tag_resolved(
				ResolveName(program_.abi_tags[begin + i]));
	}

	LocalContextHandle StoreLocalContext(
		const abi_mangle::AbiLocalContext& context, std::size_t identity)
	{
		LocalContextHandle result;
		result.storage = context_->store_context(context);
		result.identity = identity;
		abi_mangle::AbiResolvedContextBinding binding;
		binding.identity = identity;
		binding.context = result.storage;
		facts_.contexts.push_back(binding);
		return result;
	}

	public:
	template<typename Fact>
	static void AssignLocalContext(Fact* fact,
		const LocalContextHandle& context)
	{
		fact->resolved_context = context.storage;
		fact->resolved_context_identity = context.identity;
	}

public:
	void SetPath(abi_mangle::AbiFunctionTarget* target,
		semantic::ScopeId owner, semantic::NameId terminal)
	{
		target->resolved_path = ResolvePath(owner, terminal);
	}

	void SetGeneratedPath(abi_mangle::AbiFunctionTarget* target,
		semantic::ScopeId owner, const std::string& terminal)
	{
		target->resolved_path = ResolveGeneratedPath(owner, terminal);
	}

	void SetNamespaceLambda(abi_mangle::AbiFunctionTarget* target,
		semantic::ScopeId owner, std::uint32_t ordinal)
	{
		target->resolved_path = ResolveScopePath(owner);
		target->resolved_context_identity = ordinal;
	}

	void SetNamespaceLambda(abi_mangle::AbiType* type,
		semantic::ScopeId owner, std::uint32_t ordinal)
	{
		const std::size_t path = ResolveScopePath(owner);
		type->index = path == abi_mangle::ABI_NO_RESOLVED_REFERENCE ?
			0 : path + 1;
		type->resolved_expression = ordinal;
	}

	void SetPath(abi_mangle::AbiFunctionTarget* target,
		const std::vector<semantic::NameId>& path)
	{
		target->resolved_path = ResolvePath(path, 0, path.size());
	}

	void SetSourceName(abi_mangle::AbiFunctionTarget* target,
		semantic::NameId name)
	{
		target->set_resolved_source_name(ResolveName(name));
	}

	void SetSourceName(abi_mangle::AbiFunctionRecord* target,
		semantic::NameId name)
	{
		target->set_resolved_source_name(ResolveName(name));
	}

	void SetGeneratedSourceName(abi_mangle::AbiFunctionRecord* target,
		const std::string& name)
	{
		target->set_resolved_source_name(context_->resolve_generated_name(name));
	}

	std::size_t ResolveSourceName(semantic::NameId name)
	{
		return ResolveName(name);
	}

	std::size_t ResolveGeneratedName(const std::string& name)
	{
		return context_->resolve_generated_name(name);
	}

	void SetLocalSourceName(abi_mangle::AbiFunctionRecord* target,
		semantic::NameId name)
	{
		target->type.index = ResolveName(name) + 1;
	}

	void SetGeneratedLocalSourceName(abi_mangle::AbiFunctionRecord* target,
		const std::string& name)
	{
		target->type.index = context_->resolve_generated_name(name) + 1;
	}

	void AppendNameComponent(abi_mangle::AbiFunctionRecord* target,
		semantic::NameId name, std::size_t* path)
	{
		const std::size_t resolved_name = ResolveName(name);
		*path = context_->resolve_path_component(*path, resolved_name);
		target->set_resolved_name_component(*path, resolved_name);
	}

	AbiFactBuilder(const semantic::Program& program,
		abi_mangle::AbiTypedCase& facts,
		abi_mangle::AbiMangleContext* context,
		abi_mangle::AbiMangleStats* stats, LocalNames* names)
		: program_(program), facts_(facts), context_(context), stats_(stats),
		  names_(names), next_argument_(0),
		  type_argument_cache_slots_(32, 0)
	{
		if (!context_)
			ThrowLoweringInternal("typed ABI builder has no graph context");
	}

	std::size_t AddTypeArgument(semantic::TypeId type,
		const semantic::BindingRecord* function = 0,
		const semantic::FunctionTemplateAbiRecipe* recipe = 0)
	{
		using namespace abi_mangle;
		const TypeArgumentCacheKey key =
			TypeArgumentKey(type, function, recipe);
		const std::size_t* cached = FindTypeArgument(key);
		if (cached) return *cached;
		++next_argument_;
		if (!context_)
			ThrowLoweringInternal("typed ABI argument has no graph context");
		AbiTemplateArgument argument;
		argument.kind = ABI_TEMPLATE_ARGUMENT_TYPE;
		argument.type = MakeType(type, function, recipe);
		const std::size_t id = context_->resolve_argument(argument);
		CacheTypeArgument(key, id);
		return id;
	}

	std::size_t AddEntity(semantic::BindingId source)
	{
		using namespace abi_mangle;
		using namespace semantic;
		if (source == kNoBinding || source >= program_.bindings.size())
			ThrowLoweringInternal("ABI template argument entity is invalid");
		source = program_.bindings[source].canonical;
		const BindingRecord& binding = program_.bindings[source];
		++next_argument_;
		AbiEntityFact entity;
		if (binding.kind == BIND_FUNCTION)
		{
			entity.kind = ABI_ENTITY_FACT_FUNCTION;
			entity.internal_linkage = binding.storage_class == STORAGE_CLASS_STATIC &&
				binding.member_owner == kNoEntity && !binding.lambda_invocation &&
				program_.KindOfScope(binding.owner) == SCOPE_NAMESPACE &&
				!program_.HasInternalLinkageScope(binding.owner);
			entity.function.kind = ABI_FUNCTION_TARGET_PATH;
			if (binding.name == 0)
				ThrowLoweringInternal(
					"function ABI entity has no semantic name");
			SetPath(&entity.function, binding.owner, binding.name);
			const TypeRecord& function = program_.types.Get(binding.type);
			const TypeId* parameters =
				program_.types.Parameters(binding.type);
			for (std::size_t i = 0; i < function.parameter_count; ++i)
				entity.function.signature_parameter_types.push_back(
					MakeType(parameters[i]));
		}
		else
		{
			entity.kind = ABI_ENTITY_FACT_VARIABLE;
			if (binding.name == 0)
				ThrowLoweringInternal(
					"variable ABI entity has no semantic name");
			SetPath(&entity.function, binding.owner, VariableAbiName(binding));
			entity.internal_linkage = VariableHasInternalMarker(program_, binding);
		}
		return context_->store_entity(entity);
	}

	std::size_t AddTemplateArgument(std::size_t argument,
		const semantic::BindingRecord* function = 0,
		const semantic::FunctionTemplateAbiRecipe* recipe = 0,
		std::size_t source_parameter = semantic::kNoTemplateParameter)
	{
		using namespace abi_mangle;
		using namespace semantic;
		if (argument >= program_.template_arguments.size())
			ThrowLoweringInternal("ABI template argument index is invalid");
		if (argument >= program_.canonical_template_arguments.size() ||
			program_.canonical_template_arguments[argument].kind ==
				TEMPLATE_ARGUMENT_TYPE)
			return source_parameter != kNoTemplateParameter ?
				AddTypeArgument(program_.template_arguments[argument]) :
				AddTypeArgument(
					program_.template_arguments[argument], function, recipe);
		const TemplateArgument& source =
			program_.canonical_template_arguments[argument];
		if (source.kind == TEMPLATE_ARGUMENT_TEMPLATE)
		{
			++next_argument_;
			AbiTemplateArgument target;
			if (source.dependent_parameter != kNoTemplateParameter)
			{
				target.kind =
					ABI_TEMPLATE_ARGUMENT_TEMPLATE_PARAMETER_TEMPLATE;
				target.index = source.dependent_parameter;
			}
			else
			{
				// The canonical marker type carries the template's indexed
				// qualified path. Its template-name argument and a later use as
				// a template prefix share that canonical substitution identity.
				target.kind = ABI_TEMPLATE_ARGUMENT_TYPE;
				target.type = MakeType(source.type, function, recipe);
			}
			return context_->resolve_argument(target);
		}
		++next_argument_;
		AbiTemplateArgument target;
		if (recipe && source_parameter < recipe->template_parameter_count &&
			recipe->template_parameter_type_begin <=
				program_.function_template_abi_template_parameter_types.size() &&
			source_parameter <
				program_.function_template_abi_template_parameter_types.size() -
					recipe->template_parameter_type_begin &&
			program_.function_template_abi_template_parameter_types[
				recipe->template_parameter_type_begin + source_parameter] !=
					kNoFunctionTemplateAbiType)
		{
			target.kind = ABI_TEMPLATE_ARGUMENT_DEPENDENT_VALUE;
			target.type = MakeFunctionTemplateAbiType(
				program_.function_template_abi_template_parameter_types[
					recipe->template_parameter_type_begin + source_parameter],
				*recipe);
			const TypeId value_type = source.source_value_type != kNoType ?
				source.source_value_type : source.type;
			target.value_type = MakeType(value_type);
			target.has_value_type = true;
			target.value = source.value;
		}
		else if (source.source_value_type != kNoType)
		{
			target.kind = ABI_TEMPLATE_ARGUMENT_VALUE;
			target.value_type = MakeType(source.source_value_type);
			target.has_value_type = true;
			target.value = source.value;
		}
		else if (source.value_binding != kNoBinding)
		{
			const BindingRecord& value = program_.bindings[
				program_.bindings[source.value_binding].canonical];
			const bool nonstatic_member = value.member_owner != kNoEntity &&
				((value.kind == BIND_FUNCTION &&
				  !value.static_member_function) ||
				 (value.kind != BIND_FUNCTION &&
				  value.storage_class != STORAGE_CLASS_STATIC));
			if (nonstatic_member)
			{
				if (value.member_owner >= program_.entities.size())
					ThrowLoweringInternal(
						"ABI member entity owner is invalid");
				target.kind = ABI_TEMPLATE_ARGUMENT_MEMBER_EXTERNAL_ENTITY;
				target.type = MakeType(
					program_.entities[value.member_owner].type);
				target.index = ResolveName(value.name) + 1;
				target.address_of = true;
				target.member_is_function = value.kind == BIND_FUNCTION;
				if (target.member_is_function)
				{
					const TypeRecord& member_type = program_.types.Get(value.type);
					if (member_type.kind != TYPE_FUNCTION)
						ThrowLoweringInternal(
							"ABI member function entity is not callable");
					target.member_function_const =
						(member_type.cv & CV_CONST) != 0;
					target.member_function_volatile =
						(member_type.cv & CV_VOLATILE) != 0;
					target.member_function_lvalue_ref =
						member_type.ref_qualifier == FUNCTION_REF_LVALUE;
					target.member_function_rvalue_ref =
						member_type.ref_qualifier == FUNCTION_REF_RVALUE;
					target.member_function_variadic = member_type.variadic;
				if (value.conversion_function)
				{
					target.member_function_terminal_kind =
						ABI_MEMBER_FUNCTION_TERMINAL_CONVERSION;
					target.member_function_conversion_type =
						MakeConversionFunctionType(value);
				}
				else if (value.operator_kind != OPERATOR_NONE)
				{
					target.member_function_terminal_kind =
						ABI_MEMBER_FUNCTION_TERMINAL_OPERATOR;
					target.member_function_terminal_code =
						value.operator_kind == OPERATOR_LITERAL ?
							ABI_TERMINAL_LITERAL :
						OperatorTerminal(value.operator_kind, true,
							member_type.parameter_count);
					if (value.operator_kind == OPERATOR_LITERAL)
						target.resolved_entity =
							ResolveName(value.operator_literal_suffix);
				}
				if (value.template_argument_count != 0 ||
					value.function_template_specialization)
				{
					if (value.function_template_abi_recipe ==
						kNoFunctionTemplateAbiRecipe ||
						value.function_template_abi_recipe >=
							program_.function_template_abi_recipes.size())
						ThrowLoweringInternal(
							"ABI member function template has no canonical recipe");
					const FunctionTemplateAbiRecipe& member_recipe =
						program_.function_template_abi_recipes[
							value.function_template_abi_recipe];
					ForEachFunctionTemplateArgument(value, member_recipe,
						[&](std::size_t argument) {
							target.argument_refs.push_resolved(argument);
						});
					target.resolved_expression = make_semantic_substitution(
						ABI_SEMANTIC_SUBSTITUTION_MEMBER_TEMPLATE,
						value.canonical);
					target.member_function_has_result_type =
						!value.conversion_function;
					if (target.member_function_has_result_type)
					{
						if (member_recipe.result_type !=
							kNoFunctionTemplateAbiType)
							target.member_function_result_type =
								MakeFunctionTemplateAbiType(
									member_recipe.result_type, member_recipe);
						else
						{
							const TypeRecord& recipe_type =
								program_.types.Get(member_recipe.function_type);
							target.member_function_result_type =
								MakeFunctionTemplateType(
									recipe_type.child, value, &member_recipe);
						}
					}
					const TypeRecord& recipe_type =
						program_.types.Get(member_recipe.function_type);
					const TypeId* recipe_parameters =
						program_.types.Parameters(member_recipe.function_type);
					for (std::size_t i = 0;
						i < recipe_type.parameter_count; ++i)
					{
						const FunctionTemplateAbiTypeId retained =
							FunctionTemplateParameterAbiType(member_recipe, i);
						AbiType encoded = retained != kNoFunctionTemplateAbiType ?
							MakeFunctionTemplateAbiType(retained, member_recipe) :
							MakeFunctionTemplateType(recipe_parameters[i], value, &member_recipe);
						if (member_recipe.function_parameter_pack &&
							i + 1 == recipe_type.parameter_count)
						{
							AbiTypeModifier expansion;
							expansion.kind = ABI_TYPE_PACK_EXPANSION;
							encoded.modifiers.insert(
								encoded.modifiers.begin(), expansion);
						}
						target.parameter_types.push_back(encoded);
					}
				}
				else
				{
					const TypeId* member_parameters =
						program_.types.Parameters(value.type);
					for (std::size_t i = 0;
						i < member_type.parameter_count; ++i)
						target.parameter_types.push_back(
							MakeType(member_parameters[i]));
				}
				}
			}
			else
			{
				target.kind = ABI_TEMPLATE_ARGUMENT_ENTITY;
				target.resolved_entity = AddEntity(source.value_binding);
				const TypeRecord& type = program_.types.Get(
					program_.types.RemoveTopCv(source.type));
				target.address_of = type.kind == TYPE_POINTER;
			}
		}
		else
		{
			target.kind = ABI_TEMPLATE_ARGUMENT_VALUE;
			target.value_type = MakeType(source.type, function, recipe);
			target.has_value_type = true;
			target.value = source.value;
		}
		return context_->resolve_argument(target);
	}

	// Whether a function template's parameter is a pack.
	bool IsPackParameter(const semantic::FunctionTemplateAbiRecipe& recipe,
		std::size_t parameter) const
	{
		using namespace semantic;
		if (recipe.argument_partition_begin == kNoTemplateParameter)
			return recipe.template_parameter_pack &&
				parameter + 1 == recipe.template_parameter_count;
		const std::size_t index = recipe.argument_partition_begin + parameter;
		return parameter < recipe.template_parameter_count &&
			index < program_.function_template_abi_argument_partitions.size() &&
			program_.function_template_abi_argument_partitions[index].pack;
	}

	// A dependent type that names a function template parameter pack is its
	// expansion where it appears (`Dp T0_` in `TT<Args...>` or `R(A...)`).
	bool PackExpansionParameter(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe, std::size_t* index) const
	{
		return recipe && FunctionTemplateParameter(type, function, recipe, index) &&
			IsPackParameter(*recipe, *index);
	}

	static abi_mangle::AbiType PackExpansion(abi_mangle::AbiType type)
	{
		abi_mangle::AbiTypeModifier expansion;
		expansion.kind = abi_mangle::ABI_TYPE_PACK_EXPANSION;
		type.modifiers.insert(type.modifiers.begin(), expansion);
		return type;
	}

	std::size_t AddTemplateArgumentPack(std::size_t first, std::size_t count,
		const semantic::BindingRecord* function = 0,
		const semantic::FunctionTemplateAbiRecipe* recipe = 0)
	{
		using namespace abi_mangle;
		if (first > program_.template_arguments.size() ||
			count > program_.template_arguments.size() - first)
			ThrowLoweringInternal("ABI template argument pack range is invalid");
		++next_argument_;
		AbiTemplateArgument argument_pack;
		argument_pack.kind = ABI_TEMPLATE_ARGUMENT_PACK;
		for (std::size_t argument = 0; argument < count; ++argument)
		{
			std::size_t parameter = 0;
			const bool type_argument = first + argument >=
				program_.canonical_template_arguments.size() ||
				program_.canonical_template_arguments[first + argument].kind ==
					semantic::TEMPLATE_ARGUMENT_TYPE;
			if (type_argument && PackExpansionParameter(
				program_.template_arguments[first + argument], function, recipe,
				&parameter))
			{
				AbiTemplateArgument expansion;
				expansion.kind = ABI_TEMPLATE_ARGUMENT_TYPE;
				expansion.type = PackExpansion(
					MakeFunctionTemplateParameterType(parameter));
				argument_pack.argument_refs.push_resolved(
					context_->resolve_argument(expansion));
			}
			else argument_pack.argument_refs.push_resolved(
				AddTemplateArgument(first + argument, function, recipe));
		}
		return context_->resolve_argument(argument_pack);
	}

	template<class Consumer>
	void ForEachFunctionTemplateArgument(
		const semantic::BindingRecord& binding,
		const semantic::FunctionTemplateAbiRecipe& recipe, Consumer consume)
	{
		using namespace semantic;
		const std::size_t count = binding.template_argument_count;
		const std::size_t first = count == 0 ? 0 : binding.template_argument_begin;
		if (first > program_.template_arguments.size() ||
			count > program_.template_arguments.size() - first)
			ThrowLoweringInternal("function template ABI arguments are invalid");
		if (recipe.argument_partition_begin != kNoTemplateParameter)
		{
			const std::size_t begin = recipe.argument_partition_begin;
			if (begin > program_.function_template_abi_argument_partitions.size() ||
				recipe.template_parameter_count >
					program_.function_template_abi_argument_partitions.size() - begin)
				ThrowLoweringInternal("function template ABI partitions are invalid");
			for (std::size_t p = 0; p < recipe.template_parameter_count; ++p)
			{
				const FunctionTemplateAbiArgumentPartition& partition =
					program_.function_template_abi_argument_partitions[begin + p];
				if (partition.begin > count || partition.count > count - partition.begin ||
					(!partition.pack && partition.count != 1))
					ThrowLoweringInternal("function template ABI partition range is invalid");
				consume(partition.pack ? AddTemplateArgumentPack(
					first + partition.begin, partition.count, &binding, &recipe) :
					AddTemplateArgument(first + partition.begin, &binding, &recipe, p));
			}
			return;
		}
		const std::size_t fixed = recipe.template_parameter_pack ?
			recipe.template_parameter_count - 1 : count;
		if (fixed > count)
			ThrowLoweringInternal("function template ABI pack range is invalid");
		for (std::size_t p = 0; p < fixed; ++p)
			consume(AddTemplateArgument(first + p, &binding, &recipe, p));
		if (recipe.template_parameter_pack)
			consume(AddTemplateArgumentPack(first + fixed, count - fixed, &binding, &recipe));
	}

	std::size_t AddTemplateParameterExpression(std::size_t parameter)
	{
		using namespace abi_mangle;
		++next_argument_;
		AbiDependentExpression expression;
		expression.kind = ABI_EXPRESSION_TEMPLATE_PARAMETER;
		expression.index = parameter;
		return context_->resolve_expression(expression);
	}

	abi_mangle::AbiType AddContextType(semantic::TypeId type)
	{
		++next_argument_;
		return MakeType(type);
	}

	static void AppendFunctionQualifiers(const semantic::TypeRecord& type,
		std::vector<abi_mangle::AbiFunctionQualifier>* qualifiers)
	{
		using namespace abi_mangle;
		if ((type.cv & semantic::CV_CONST) != 0)
			qualifiers->push_back(ABI_FUNCTION_QUALIFIER_CONST);
		if ((type.cv & semantic::CV_VOLATILE) != 0)
			qualifiers->push_back(ABI_FUNCTION_QUALIFIER_VOLATILE);
		if (type.ref_qualifier == semantic::FUNCTION_REF_LVALUE)
			qualifiers->push_back(ABI_FUNCTION_QUALIFIER_LVALUE_REFERENCE);
		else if (type.ref_qualifier == semantic::FUNCTION_REF_RVALUE)
			qualifiers->push_back(ABI_FUNCTION_QUALIFIER_RVALUE_REFERENCE);
	}

	// The context of a local class's member function:
	// Z <enclosing encoding> E N <local class> <member> E <parameters>.
	abi_mangle::AbiLocalContext MakeLocalMemberContext(
		abi_mangle::AbiLocalContext context,
		const semantic::BindingRecord& function)
	{
		using namespace abi_mangle;
		using namespace semantic;
		const EntityId owner_id = function.member_owner;
		const EntityRecord& owner = program_.entities[owner_id];
		context.target_signature_is_parameter_list = true;
		AbiFunctionTarget& target = context.function;
		target.kind = ABI_FUNCTION_TARGET_LOCAL;
		AssignLocalContext(&target, AddLocalContext(owner.local_context));
		target.local_presentation = ABI_LOCAL_PRESENTATION_NAME_ORDINAL;
		if (IsUnnamedClass(program_, owner_id) &&
			!UnnamedTypeIsVisible(program_, owner_id))
		{
			target.local_presentation = ABI_LOCAL_PRESENTATION_GENERATED_LAMBDA;
			target.resolved_path = Names().AnonymousNumber(owner_id);
		}
		else if (owner.unnamed_class)
			target.resolved_path = owner.local_name_ordinal;
		else
		{
			target.qualified_name = program_.names.Get(owner.identity_name);
			target.resolved_path = Names().ClassDiscriminator(owner_id);
		}
		target.owner_type = MakeType(owner.type);
		const TypeRecord& type = program_.types.Get(function.type);
		if (function.operator_kind != OPERATOR_NONE &&
			function.operator_kind != OPERATOR_LITERAL)
			target.terminal_code = OperatorTerminal(function.operator_kind,
				true, type.parameter_count);
		else target.source_name = program_.names.Get(function.name);
		const TypeId* parameters = program_.types.Parameters(function.type);
		for (std::size_t i = 0; i < type.parameter_count; ++i)
			target.signature_parameter_types.push_back(MakeType(parameters[i]));
		target.variadic = type.variadic;
		return context;
	}

	// A default member initializer's closure type,
	// `<class> <member> M Ul<sig>E<n>_` (`N8WithInit2fpMUlvE_E`).
	abi_mangle::AbiType MakeMemberLambdaType(semantic::EntityId closure,
		abi_mangle::AbiType result)
	{
		using namespace semantic;
		const EntityRecord& lambda = program_.entities[closure];
		const BindingRecord& member = program_.bindings[lambda.lambda_member_context];
		result.kind = abi_mangle::ABI_TYPE_MEMBER_LAMBDA;
		result.index = ResolveName(member.name) + 1;
		result.types.push_back(MakeType(program_.entities[member.member_owner].type));
		const TypeId call = program_.bindings[lambda.lambda_call_operator].type;
		const TypeId* parameters = program_.types.Parameters(call);
		for (std::size_t i = 0; i < program_.types.Get(call).parameter_count; ++i)
			result.types.push_back(MakeType(parameters[i]));
		result.local_presentation = abi_mangle::ABI_LOCAL_PRESENTATION_LAMBDA_DISCRIMINATOR;
		result.resolved_expression = lambda.lambda_abi_ordinal;
		return result;
	}

	// A closure's function call operator: `Ul<sig>E<n>_` in the closure's
	// local context where it has a mangling number, `$_<n>` in its function
	// or namespace otherwise.
	void SetClosureTarget(abi_mangle::AbiFunctionTarget* target,
		semantic::EntityId closure)
	{
		using namespace abi_mangle;
		using namespace semantic;
		const EntityRecord& lambda = program_.entities[closure];
		if (ClosureIsMemberLambda(program_, closure))
		{
			target->kind = ABI_FUNCTION_TARGET_MEMBER;
			target->owner_type = MakeType(lambda.type);
		}
		else if (ClosureHasManglingNumber(program_, closure))
		{
			target->kind = ABI_FUNCTION_TARGET_LAMBDA;
			AssignLocalContext(target, AddClosureContext(closure));
			target->local_presentation =
				ABI_LOCAL_PRESENTATION_LAMBDA_DISCRIMINATOR;
			target->resolved_path = lambda.lambda_abi_ordinal;
		}
		else if (!ClosureIsNamespaceScope(program_, closure))
		{
			target->kind = ABI_FUNCTION_TARGET_LOCAL;
			AssignLocalContext(target, AddLocalContext(lambda.local_context));
			target->local_presentation = ABI_LOCAL_PRESENTATION_GENERATED_LAMBDA;
			target->resolved_path = Names().AnonymousNumber(closure);
			target->owner_type = MakeType(lambda.type);
		}
		else
		{
			target->kind = ABI_FUNCTION_TARGET_NAMESPACE_LAMBDA;
			SetNamespaceLambda(target, ClosureNamingScope(program_, closure),
				Names().AnonymousNumber(closure));
		}
		target->terminal_code = ABI_TERMINAL_CALL;
	}

	// The context a numbered closure is named in: its function, or the
	// function and parameter of the default argument it belongs to.
	LocalContextHandle AddClosureContext(semantic::EntityId closure)
	{
		const semantic::EntityRecord& lambda = program_.entities[closure];
		return lambda.lambda_default_argument != 0 ?
			AddLocalContext(lambda.lambda_member_context,
				lambda.lambda_default_argument) :
			AddLocalContext(lambda.local_context);
	}

	LocalContextHandle AddLocalContext(semantic::BindingId binding,
		std::uint32_t default_argument = 0)
	{
		if (binding == semantic::kNoBinding || binding >= program_.bindings.size())
			ThrowLoweringInternal("local ABI type has no function context");
		binding = LocalContextFunction(program_, binding);
		const std::uint64_t key = (static_cast<std::uint64_t>(default_argument) << 32) |
			binding;
		for (std::size_t i = 0; i < local_contexts_.size(); ++i)
			if (local_contexts_[i].first == key)
				return local_context_handles_[local_contexts_[i].second];
		const LocalContextHandle handle = MakeLocalContext(binding, default_argument);
		local_contexts_.push_back(std::make_pair(key, local_context_handles_.size()));
		local_context_handles_.push_back(handle);
		return handle;
	}

	LocalContextHandle MakeLocalContext(semantic::BindingId binding,
		std::uint32_t default_argument)
	{
		using namespace abi_mangle;
		using namespace semantic;
		const BindingRecord& function = program_.bindings[binding];
		const TypeRecord& type = program_.types.Get(function.type);
		if (type.kind != TYPE_FUNCTION)
			ThrowLoweringInternal("local ABI context is not a function");
		const std::size_t identity = next_argument_++;
		AbiLocalContext context_fact;
		context_fact.default_argument = default_argument;
		if (function.member_owner != kNoEntity &&
			function.member_owner < program_.entities.size())
		{
			const EntityRecord& owner = program_.entities[function.member_owner];
			const BindingId call = owner.lambda_call_operator;
			if (owner.lambda_closure && call != kNoBinding &&
				call < program_.bindings.size() &&
				program_.bindings[call].canonical == function.canonical)
			{
				AbiLocalContext& context = context_fact;
				context.kind = ABI_CONTEXT_FUNCTION;
				context.target_signature_is_parameter_list = true;
				AbiFunctionTarget& target = context.function;
				SetClosureTarget(&target, function.member_owner);
				if ((type.cv & CV_CONST) != 0)
					context.qualifiers.push_back(
						ABI_FUNCTION_QUALIFIER_CONST);
				if ((type.cv & CV_VOLATILE) != 0)
					context.qualifiers.push_back(
						ABI_FUNCTION_QUALIFIER_VOLATILE);
				const TypeId* parameters =
					program_.types.Parameters(function.type);
				for (std::size_t i = 0; i < type.parameter_count; ++i)
					target.signature_parameter_types.push_back(
						MakeType(parameters[i]));
				target.variadic = type.variadic;
				return StoreLocalContext(context_fact, identity);
			}
		}
		if (function.owner == program_.GlobalScope() &&
			program_.names.Get(function.name) == "main")
		{
			if (stats_) ++stats_->typed_main_contexts;
			context_fact.kind = ABI_CONTEXT_MAIN;
			return StoreLocalContext(context_fact, identity);
		}
		context_fact.kind = ABI_CONTEXT_FUNCTION;
		AbiFunctionTarget& target = context_fact.function;
		target.kind = ABI_FUNCTION_TARGET_PATH;
		const EntityId member_owner =
			function.member_owner < program_.entities.size() ?
			function.member_owner : kNoEntity;
		// A member function's cv- and ref-qualifiers are part of the
		// encoding its local entities are named in (`_ZZNK1S1mEvE1x`).
		if (member_owner != kNoEntity && !function.static_member_function)
			AppendFunctionQualifiers(type, &context_fact.qualifiers);
		if (member_owner != kNoEntity &&
			program_.entities[member_owner].local_context != kNoBinding)
			return StoreLocalContext(
				MakeLocalMemberContext(context_fact, function), identity);
		if (member_owner != kNoEntity &&
			IsClassTemplateSpecialization(program_.entities[member_owner]))
		{
			const EntityRecord& owner = program_.entities[member_owner];
			context_fact.target_signature_is_parameter_list = true;
			target.kind = ABI_FUNCTION_TARGET_MEMBER;
			target.owner_type = AddContextType(owner.type);
			SetSourceName(&target, function.name);
		}
		if (function.name == 0)
			ThrowLoweringInternal(
				"function ABI local context has no semantic name");
		if (!SetAnonymousMemberPath(&target, member_owner, ResolveName(function.name)))
			SetPath(&target, function.owner, function.name);
		// Clang names a constructor's or destructor's local entities in its
		// complete entry (C1, D1).
		target.terminal_code = function.constructor ?
			ABI_TERMINAL_CONSTRUCTOR_COMPLETE : function.destructor ?
			ABI_TERMINAL_DESTRUCTOR_COMPLETE :
			function.operator_kind != OPERATOR_NONE &&
			function.operator_kind != OPERATOR_LITERAL ?
			OperatorTerminal(function.operator_kind, member_owner != kNoEntity &&
				!function.static_member_function, type.parameter_count) :
			ABI_TERMINAL_NONE;
		context_fact.internal_linkage =
			function.storage_class == STORAGE_CLASS_STATIC &&
			member_owner == kNoEntity &&
			program_.KindOfScope(function.owner) == SCOPE_NAMESPACE &&
			!program_.HasInternalLinkageScope(function.owner);
		std::size_t parameter_scope = 0;
		for (EntityId owner = function.member_owner; owner != kNoEntity;
			owner = program_.entities[owner].enclosing_class)
		{
			if (owner >= program_.entities.size() || parameter_scope >= program_.entities.size())
				ThrowLoweringInternal("local ABI context template owner is invalid");
			if (IsClassTemplateSpecialization(program_.entities[owner])) ++parameter_scope;
		}
		const FunctionTemplateAbiRecipe* recipe = 0;
		if (function.function_template_abi_recipe !=
			kNoFunctionTemplateAbiRecipe)
		{
			if (function.function_template_abi_recipe >=
				program_.function_template_abi_recipes.size())
				ThrowLoweringInternal(
					"local ABI context template recipe is invalid");
			recipe = &program_.function_template_abi_recipes[
				function.function_template_abi_recipe];
		}
		if (function.template_argument_count != 0 ||
			(function.function_template_specialization && recipe &&
			 (recipe->template_parameter_pack ||
			  recipe->argument_partition_begin != kNoTemplateParameter)))
		{
			if (recipe == 0)
				ThrowLoweringInternal(
					"function template specialization has no canonical recipe");
			ForEachFunctionTemplateArgument(function, *recipe,
				[&](std::size_t resolved) {
					AbiFunctionPathOperand argument;
					argument.kind = ABI_FUNCTION_PATH_TEMPLATE_ARGUMENT;
					argument.resolved_argument = resolved;
					target.path_operands.push_back(argument);
				});
			if (recipe->result_type != kNoFunctionTemplateAbiType)
				target.result_type = MakeFunctionTemplateAbiType(
					recipe->result_type, *recipe, parameter_scope);
			else
			{
				TypeId result = type.child;
				const TypeRecord& recipe_type =
					program_.types.Get(recipe->function_type);
				if (UsesFunctionTemplateParameter(
					recipe_type.child, &function, *recipe))
					result = recipe_type.child;
				target.result_type = result == type.child ? MakeType(result) :
					MakeType(result, &function, recipe);
			}
			target.has_result_type = true;
		}
		const TypeId* parameters = program_.types.Parameters(function.type);
		const TypeRecord* recipe_type = recipe ?
			&program_.types.Get(recipe->function_type) : 0;
		const TypeId* recipe_parameters = recipe ?
			program_.types.Parameters(recipe->function_type) : 0;
		const std::size_t parameter_count = recipe_type ?
			recipe_type->parameter_count : type.parameter_count;
		for (std::size_t i = 0; i < parameter_count; ++i)
		{
			TypeId parameter = recipe_type ? recipe_parameters[i] : parameters[i];
			const FunctionTemplateAbiRecipe* parameter_recipe = 0;
			std::size_t source = i;
			bool pack_expansion = false;
			if (recipe_type)
			{
				const std::size_t fixed = recipe->function_parameter_pack ?
					recipe_type->parameter_count - 1 :
					recipe_type->parameter_count;
				source = i < fixed ? i : fixed;
				if (source >= recipe_type->parameter_count)
					ThrowLoweringInternal(
						"local ABI context parameter recipe is invalid");
				pack_expansion = recipe->function_parameter_pack && i >= fixed;
				if (pack_expansion || UsesFunctionTemplateParameter(
					recipe_parameters[source], &function, *recipe))
				{
					parameter = recipe_parameters[source];
					parameter_recipe = recipe;
				}
			}
			const FunctionTemplateAbiTypeId retained = recipe ?
				FunctionTemplateParameterAbiType(*recipe, source) :
				kNoFunctionTemplateAbiType;
			AbiType encoded = retained != kNoFunctionTemplateAbiType ?
				MakeFunctionTemplateAbiType(retained, *recipe, parameter_scope) :
				parameter_scope != 0 && parameter_recipe ?
					MakeTypeCore(parameter, &function, parameter_recipe, parameter_scope) :
					MakeFunctionTemplateType(parameter, function, parameter_recipe);
			if (pack_expansion)
			{
				AbiTypeModifier expansion;
				expansion.kind = ABI_TYPE_PACK_EXPANSION;
				encoded.modifiers.insert(encoded.modifiers.begin(), expansion);
			}
			target.signature_parameter_types.push_back(encoded);
		}
		target.variadic = type.variadic;
		return StoreLocalContext(context_fact, identity);
	}

	bool FunctionTemplateParameter(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe,
		std::size_t* index) const
	{
		using namespace semantic;
		if (recipe)
		{
			const TypeRecord& shape = program_.types.Get(type);
			if (shape.kind != TYPE_NAMED) return false;
			const EntityRecord& entity = program_.entities[shape.entity];
			const std::size_t parameter = entity.template_parameter_ordinal;
			if (parameter >= recipe->template_parameter_count ||
				recipe->parameter_shape_begin >
					program_.function_template_parameter_shapes.size() ||
				parameter >= program_.function_template_parameter_shapes.size() -
					recipe->parameter_shape_begin ||
				program_.function_template_parameter_shapes[
					recipe->parameter_shape_begin + parameter] != type)
				return false;
			*index = parameter;
			return true;
		}
		if (!function || function->template_argument_count == 0) return false;
		const std::size_t first = function->template_argument_begin;
		if (first > program_.template_arguments.size() ||
			function->template_argument_count >
				program_.template_arguments.size() - first)
			ThrowLoweringInternal("function template ABI argument range is invalid");
		for (std::size_t i = 0; i < function->template_argument_count; ++i)
		{
			const std::size_t argument = first + i;
			if (argument < program_.canonical_template_arguments.size() &&
				program_.canonical_template_arguments[argument].kind !=
					TEMPLATE_ARGUMENT_TYPE)
				continue;
			if (program_.template_arguments[argument] == type)
			{
				*index = i;
				return true;
			}
		}
		return false;
	}

	abi_mangle::AbiType MakeConversionFunctionType(
		const semantic::BindingRecord& function)
	{
		using namespace semantic;
		if (function.function_template_abi_recipe ==
			kNoFunctionTemplateAbiRecipe)
			return MakeType(function.conversion_target);
		if (function.function_template_abi_recipe >=
			program_.function_template_abi_recipes.size())
			ThrowLoweringInternal("conversion function ABI recipe is invalid");
		const FunctionTemplateAbiRecipe& recipe =
			program_.function_template_abi_recipes[
				function.function_template_abi_recipe];
		if (recipe.result_type != kNoFunctionTemplateAbiType)
			return MakeFunctionTemplateAbiType(recipe.result_type, recipe);
		return MakeFunctionTemplateType(
			program_.types.Get(recipe.function_type).child, function, &recipe);
	}

	abi_mangle::AbiType MakeType(semantic::TypeId type)
	{
		return MakeType(type, 0, 0);
	}

	abi_mangle::AbiType MakeFunctionTemplateType(semantic::TypeId type,
		const semantic::BindingRecord& function,
		const semantic::FunctionTemplateAbiRecipe* recipe = 0)
	{
		// Only the retained pattern recipe can prove that a type occurrence is
		// dependent.  Inferring dependence by comparing an instantiated type
		// with the specialization's template arguments is ambiguous: for
		// `template<class T> int f(T)`, f<int>'s non-dependent result happens to
		// equal T's argument but must be encoded as `i`, not `T_`.
		return recipe ? MakeType(type, &function, recipe) : MakeType(type);
	}

	abi_mangle::AbiType MakeType(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe)
	{
		const TypeArgumentCacheKey key =
			TypeArgumentKey(type, function, recipe);
		std::size_t cached = 0;
		if (context_ && context_->find_resolved_type(
			key.type, key.function, key.recipe, &cached))
		{
			abi_mangle::AbiType direct;
			direct.kind = abi_mangle::ABI_TYPE_RESOLVED;
			direct.index = cached;
			return direct;
		}
		abi_mangle::AbiType result =
			MakeTypeCore(type, function, recipe);
		if (!CanResolveType(result)) return result;
		std::size_t resolved = context_->resolve_type(result);
		if (!context_->resolved_type_uses_case_facts(resolved))
			resolved = context_->cache_resolved_type(
				key.type, key.function, key.recipe, resolved);
		abi_mangle::AbiType direct;
		direct.kind = abi_mangle::ABI_TYPE_RESOLVED;
		direct.index = resolved;
		return direct;
	}

	semantic::FunctionTemplateAbiTypeId FunctionTemplateParameterAbiType(
		const semantic::FunctionTemplateAbiRecipe& recipe,
		std::size_t parameter) const
	{
		using namespace semantic;
		if (parameter >= program_.types.Get(recipe.function_type).parameter_count ||
			recipe.function_parameter_type_begin >
				program_.function_template_abi_function_parameter_types.size() ||
			parameter >=
				program_.function_template_abi_function_parameter_types.size() -
					recipe.function_parameter_type_begin)
			ThrowLoweringInternal(
				"function template ABI parameter recipe is invalid");
		return program_.function_template_abi_function_parameter_types[
			recipe.function_parameter_type_begin + parameter];
	}

	std::size_t AddFunctionTemplateAbiExpression(
		semantic::FunctionTemplateAbiExpressionId expression,
		const semantic::FunctionTemplateAbiRecipe& recipe,
		std::size_t parameter_scope = 0)
	{
		using namespace abi_mangle;
		using namespace semantic;
		if (expression == kNoFunctionTemplateAbiExpression ||
			expression >= program_.function_template_abi_expressions.size())
			ThrowLoweringInternal(
				"function template ABI expression recipe is invalid");
		const FunctionTemplateAbiExpression& source =
			program_.function_template_abi_expressions[expression];
		++next_argument_;
		AbiDependentExpression target;
		if (source.kind ==
			FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_PARAMETER)
		{
			target.kind = ABI_EXPRESSION_TEMPLATE_PARAMETER;
			target.index = source.parameter;
		}
		else if (source.kind ==
			FUNCTION_TEMPLATE_ABI_EXPRESSION_FUNCTION_PARAMETER)
		{
			target.kind = ABI_EXPRESSION_FUNCTION_PARAMETER;
			target.index = source.parameter;
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_TYPE_MEMBER)
		{
			target.kind = ABI_EXPRESSION_MEMBER;
			target.type = MakeFunctionTemplateAbiType(source.type, recipe, parameter_scope);
			target.type.suppress_template_prefix_substitution = true;
			target.index = ResolveName(source.name) + 1;
			const FunctionTemplateAbiTypeKind owner =
				program_.function_template_abi_types[source.type].kind;
			target.close_member_owner = owner == FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER ||
				owner == FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_SPECIALIZATION;
			if (source.argument_begin > program_.function_template_abi_arguments.size() ||
				source.argument_count > program_.function_template_abi_arguments.size() - source.argument_begin)
				ThrowLoweringInternal("retained dependent member argument range is invalid");
			for (std::size_t i = 0; i < source.argument_count; ++i)
				target.argument_refs.push_resolved(AddFunctionTemplateAbiArgument(
					program_.function_template_abi_arguments[source.argument_begin + i], recipe, parameter_scope));
		}
		else if (source.kind ==
			FUNCTION_TEMPLATE_ABI_EXPRESSION_OBJECT_MEMBER)
		{
			target.kind = ABI_EXPRESSION_OBJECT_MEMBER;
			target.operation = source.indirect_member ?
				ABI_EXPRESSION_OPERATION_INDIRECT_MEMBER :
				ABI_EXPRESSION_OPERATION_MEMBER;
			target.index = ResolveName(source.name) + 1;
			target.expression_refs.push_resolved(
				AddFunctionTemplateAbiExpression(source.left, recipe, parameter_scope));
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_STATIC_CAST)
		{
			target.kind = ABI_EXPRESSION_CAST;
			target.operation = ABI_EXPRESSION_OPERATION_STATIC_CAST;
			target.type = MakeFunctionTemplateAbiType(source.type, recipe, parameter_scope);
			target.expression_refs.push_resolved(AddFunctionTemplateAbiExpression(source.left, recipe, parameter_scope));
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_NAMESPACE_MEMBER)
		{
			target.kind = ABI_EXPRESSION_MEMBER;
			target.type.kind = ABI_TYPE_NAMED;
			target.type.index = ResolveScopePath(source.parameter) + 1;
			target.close_member_owner = true;
			target.index = ResolveName(source.name) + 1;
			for (std::size_t i = 0; i < source.argument_count; ++i)
				target.argument_refs.push_resolved(AddFunctionTemplateAbiArgument(
					program_.function_template_abi_arguments[source.argument_begin + i], recipe, parameter_scope));
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_ENTITY)
		{
			if (source.parameter >= program_.function_template_abi_recipes.size() ||
				source.left >= program_.function_template_abi_expressions.size())
				ThrowLoweringInternal("retained template entity has no primary recipe");
			const FunctionTemplateAbiExpression& name = program_.function_template_abi_expressions[source.left];
			const FunctionTemplateAbiRecipe& primary = program_.function_template_abi_recipes[source.parameter];
			AbiEntityFact entity;
			entity.kind = ABI_ENTITY_FACT_FUNCTION;
			SetPath(&entity.function, name.parameter, name.name);
			for (std::size_t i = 0; i < source.argument_count; ++i)
			{
				AbiFunctionPathOperand argument;
				argument.kind = ABI_FUNCTION_PATH_TEMPLATE_ARGUMENT;
				argument.resolved_argument = AddFunctionTemplateAbiArgument(
					program_.function_template_abi_arguments[source.argument_begin + i], recipe, parameter_scope);
				entity.function.path_operands.push_back(argument);
			}
			const TypeRecord& function = program_.types.Get(primary.function_type);
			entity.function.has_result_type = true;
			entity.function.result_type = primary.result_type != kNoFunctionTemplateAbiType ?
				MakeFunctionTemplateAbiType(primary.result_type, primary) : MakeType(function.child, 0, &primary);
			const TypeId* parameters = program_.types.Parameters(primary.function_type);
			for (std::size_t p = 0; p < function.parameter_count; ++p)
			{
				const FunctionTemplateAbiTypeId retained = FunctionTemplateParameterAbiType(primary, p);
				entity.function.signature_parameter_types.push_back(retained != kNoFunctionTemplateAbiType ?
					MakeFunctionTemplateAbiType(retained, primary) : MakeType(parameters[p], 0, &primary));
			}
			target.kind = ABI_EXPRESSION_ENTITY;
			target.resolved_entity = context_->store_entity(entity);
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_SOURCE_NAME)
		{
			target.kind = ABI_EXPRESSION_SOURCE_NAME;
			target.index = ResolveName(source.name) + 1;
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_CALL)
		{
			target.kind = ABI_EXPRESSION_CALL;
			target.expression_refs.push_resolved(
				AddFunctionTemplateAbiExpression(source.left, recipe, parameter_scope));
			if (source.argument_begin > program_.function_template_abi_arguments.size() ||
				source.argument_count > program_.function_template_abi_arguments.size() -
					source.argument_begin)
				ThrowLoweringInternal("retained dependent call argument range is invalid");
			for (std::size_t i = 0; i < source.argument_count; ++i)
			{
				const FunctionTemplateAbiArgument& argument =
					program_.function_template_abi_arguments[source.argument_begin + i];
				std::size_t expression = AddFunctionTemplateAbiExpression(argument.expression, recipe, parameter_scope);
				if (argument.pack_expansion)
				{
					AbiDependentExpression expansion;
					expansion.kind = ABI_EXPRESSION_PACK_EXPANSION;
					expansion.expression_refs.push_resolved(expression);
					expression = context_->resolve_expression(expansion);
				}
				target.expression_refs.push_resolved(expression);
			}
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_INTEGRAL)
		{
			if (source.argument_begin >= program_.canonical_template_arguments.size())
				ThrowLoweringInternal("retained dependent literal argument is invalid");
			const TemplateArgument& argument = program_.canonical_template_arguments[source.argument_begin];
			if (argument.kind != TEMPLATE_ARGUMENT_INTEGRAL)
				ThrowLoweringInternal("retained dependent literal argument is not integral");
			target.kind = ABI_EXPRESSION_INTEGRAL_VALUE;
			target.value_type = MakeType(argument.type);
			target.value = argument.value;
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_SIZEOF_PACK)
		{
			target.kind = ABI_EXPRESSION_UNARY;
			target.operation = ABI_EXPRESSION_OPERATION_SIZEOF_PACK;
			target.expression_refs.push_resolved(
				AddFunctionTemplateAbiExpression(source.left, recipe, parameter_scope));
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_UNARY)
		{
			target.kind = ABI_EXPRESSION_UNARY;
			if (source.operation != OPERATOR_STAR)
				ThrowLoweringInternal(
					"unsupported retained dependent unary operation");
			target.operation = ABI_EXPRESSION_OPERATION_DEREFERENCE;
			target.expression_refs.push_resolved(
				AddFunctionTemplateAbiExpression(source.left, recipe, parameter_scope));
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_BINARY)
		{
			target.kind = ABI_EXPRESSION_BINARY;
			if (source.operation == OPERATOR_PLUS)
				target.operation = ABI_EXPRESSION_OPERATION_ADD;
			else if (source.operation == OPERATOR_MINUS)
				target.operation = ABI_EXPRESSION_OPERATION_SUBTRACT;
			else if (source.operation == OPERATOR_EQUAL)
				target.operation = ABI_EXPRESSION_OPERATION_EQUAL;
			else if (source.operation == OPERATOR_LESS)
				target.operation = ABI_EXPRESSION_OPERATION_LESS;
			else ThrowLoweringInternal("unsupported retained dependent binary operation");
			target.expression_refs.push_resolved(
				AddFunctionTemplateAbiExpression(source.left, recipe, parameter_scope));
			target.expression_refs.push_resolved(
				AddFunctionTemplateAbiExpression(source.right, recipe, parameter_scope));
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_EXPRESSION_TEMPLATE_ID)
		{
			if (source.argument_begin >
					program_.function_template_abi_arguments.size() ||
				source.argument_count >
					program_.function_template_abi_arguments.size() -
						source.argument_begin)
				ThrowLoweringInternal(
					"retained dependent template-id is invalid");
			target.kind = ABI_EXPRESSION_TEMPLATE_ID;
			target.index = ResolveName(source.name) + 1;
			for (std::size_t i = 0; i < source.argument_count; ++i)
				target.argument_refs.push_resolved(AddFunctionTemplateAbiArgument(
					program_.function_template_abi_arguments[
						source.argument_begin + i], recipe, parameter_scope));
		}
		else ThrowLoweringInternal(
			"function template ABI expression node kind is invalid");
		return context_->resolve_expression(target);
	}

	std::size_t AddFunctionTemplateAbiArgument(
		const semantic::FunctionTemplateAbiArgument& source,
		const semantic::FunctionTemplateAbiRecipe& recipe,
		std::size_t parameter_scope = 0)
	{
		using namespace abi_mangle;
		using namespace semantic;
		++next_argument_;
		AbiTemplateArgument target;
		if (source.kind == FUNCTION_TEMPLATE_ABI_ARGUMENT_TYPE)
		{
			target.kind = ABI_TEMPLATE_ARGUMENT_TYPE;
			target.type = MakeFunctionTemplateAbiType(source.type, recipe, parameter_scope);
			if (source.pack_expansion)
			{
				AbiTypeModifier expansion;
				expansion.kind = ABI_TYPE_PACK_EXPANSION;
				target.type.modifiers.insert(target.type.modifiers.begin(), expansion);
			}
		}
		else if (!source.pack_expansion &&
			source.expression < program_.function_template_abi_expressions.size() &&
			program_.function_template_abi_expressions[source.expression].kind ==
				FUNCTION_TEMPLATE_ABI_EXPRESSION_INTEGRAL)
		{
			const FunctionTemplateAbiExpression& literal =
				program_.function_template_abi_expressions[source.expression];
			if (literal.argument_begin >= program_.canonical_template_arguments.size())
				ThrowLoweringInternal("retained template literal argument is invalid");
			const TemplateArgument& argument = program_.canonical_template_arguments[literal.argument_begin];
			target.kind = ABI_TEMPLATE_ARGUMENT_VALUE;
			target.value_type = MakeType(argument.type);
			target.value = argument.value;
		}
		else
		{
			target.kind = ABI_TEMPLATE_ARGUMENT_EXPRESSION;
			target.resolved_expression = AddFunctionTemplateAbiExpression(
				source.expression, recipe, parameter_scope);
			if (source.pack_expansion)
			{
				AbiDependentExpression expansion;
				expansion.kind = ABI_EXPRESSION_PACK_EXPANSION;
				expansion.expression_refs.push_resolved(target.resolved_expression);
				target.resolved_expression = context_->resolve_expression(expansion);
			}
		}
		return context_->resolve_argument(target);
	}

	abi_mangle::AbiType MakeFunctionTemplateParameterType(std::size_t index,
		std::size_t parameter_scope = 0)
	{
		abi_mangle::AbiType result;
		result.kind = abi_mangle::ABI_TYPE_TEMPLATE_PARAMETER;
		result.index = index;
		result.substitutable = true;
		if (parameter_scope != 0) result.resolved_expression = parameter_scope;
		return result;
	}

	abi_mangle::AbiType MakeFunctionTemplateAbiType(
		semantic::FunctionTemplateAbiTypeId type,
		const semantic::FunctionTemplateAbiRecipe& recipe,
		std::size_t parameter_scope = 0)
	{
		using namespace abi_mangle;
		using namespace semantic;
		if (type == kNoFunctionTemplateAbiType ||
			type >= program_.function_template_abi_types.size())
			ThrowLoweringInternal("function template ABI result recipe is invalid");
		const FunctionTemplateAbiType& source =
			program_.function_template_abi_types[type];
		AbiType result;
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_CONCRETE)
			return MakeType(source.concrete_type);
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_PARAMETER)
		{
			if (source.parameter >= recipe.template_parameter_count)
				ThrowLoweringInternal(
					"function template ABI result parameter is invalid");
			return MakeFunctionTemplateParameterType(source.parameter, parameter_scope);
		}
		if (source.kind ==
			FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_PARAMETER_SPECIALIZATION)
		{
			if (source.parameter >= recipe.template_parameter_count ||
				source.argument_begin >
					program_.function_template_abi_arguments.size() ||
				source.argument_count >
					program_.function_template_abi_arguments.size() -
						source.argument_begin)
				ThrowLoweringInternal(
					"function template ABI template-parameter specialization is invalid");
			result.kind = ABI_TYPE_TEMPLATE_PARAMETER_SPECIALIZATION;
			result.index = source.parameter;
			for (std::size_t i = 0; i < source.argument_count; ++i)
				result.argument_refs.push_resolved(AddFunctionTemplateAbiArgument(
					program_.function_template_abi_arguments[
						source.argument_begin + i], recipe, parameter_scope));
			return result;
		}
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_TEMPLATE_SPECIALIZATION ||
			source.kind == FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER)
		{
			if ((source.entity == kNoEntity && source.child == kNoFunctionTemplateAbiType) ||
				(source.entity != kNoEntity && source.entity >= program_.entities.size()) ||
				source.argument_begin >
					program_.function_template_abi_arguments.size() ||
				source.argument_count >
					program_.function_template_abi_arguments.size() -
						source.argument_begin)
				ThrowLoweringInternal(
					"function template ABI specialization is invalid");
			result.kind = source.kind == FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER &&
				source.argument_count == 0 ? ABI_TYPE_NAMED : source.child == kNoFunctionTemplateAbiType ?
				ABI_TYPE_TEMPLATE_SPECIALIZATION : ABI_TYPE_MEMBER_TEMPLATE_SPECIALIZATION;
			if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_WRITTEN_QUALIFIER)
				result.index = context_->resolve_path(
					std::vector<std::size_t>(1, ResolveName(source.name))) + 1;
			else if (source.child == kNoFunctionTemplateAbiType)
				result.index = ResolvePath(
					program_.entities[source.entity].owner, program_.entities[source.entity].identity_name) + 1;
			else
				result.index = ResolveName(source.entity == kNoEntity ? source.name :
					program_.entities[source.entity].identity_name) + 1;
			if (source.child != kNoFunctionTemplateAbiType)
				result.types.push_back(
					MakeFunctionTemplateAbiType(source.child, recipe, parameter_scope));
			const std::size_t fixed = source.parameter == kNoTemplateParameter ?
				source.argument_count : source.parameter;
			if (fixed > source.argument_count)
				ThrowLoweringInternal("function template ABI specialization pack is invalid");
			for (std::size_t i = 0; i < fixed; ++i)
				result.argument_refs.push_resolved(AddFunctionTemplateAbiArgument(
					program_.function_template_abi_arguments[
						source.argument_begin + i], recipe, parameter_scope));
			if (source.parameter != kNoTemplateParameter)
			{
				AbiTemplateArgument pack;
				pack.kind = ABI_TEMPLATE_ARGUMENT_PACK;
				for (std::size_t i = fixed; i < source.argument_count; ++i)
					pack.argument_refs.push_resolved(AddFunctionTemplateAbiArgument(
						program_.function_template_abi_arguments[
							source.argument_begin + i], recipe, parameter_scope));
				result.argument_refs.push_resolved(context_->resolve_argument(pack));
			}
			return result;
		}
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_FUNCTION || source.kind == FUNCTION_TEMPLATE_ABI_TYPE_MEMBER_POINTER)
		{
			if (source.argument_begin > program_.function_template_abi_arguments.size() ||
				source.argument_count > program_.function_template_abi_arguments.size() - source.argument_begin)
				ThrowLoweringInternal("retained function or member pointer argument range is invalid");
			result.kind = source.kind == FUNCTION_TEMPLATE_ABI_TYPE_FUNCTION ? ABI_TYPE_FUNCTION : ABI_TYPE_MEMBER_POINTER;
			if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_FUNCTION)
				result.types.push_back(MakeFunctionTemplateAbiType(source.child, recipe, parameter_scope));
			for (std::size_t i = 0; i < source.argument_count; ++i)
			{
				const FunctionTemplateAbiArgument& argument = program_.function_template_abi_arguments[source.argument_begin + i];
				AbiType type = MakeFunctionTemplateAbiType(argument.type, recipe, parameter_scope);
				if (argument.pack_expansion)
				{
					AbiTypeModifier expansion;
					expansion.kind = ABI_TYPE_PACK_EXPANSION;
					type.modifiers.insert(type.modifiers.begin(), expansion);
				}
				result.types.push_back(type);
			}
			if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_MEMBER_POINTER)
				result.types.push_back(MakeFunctionTemplateAbiType(source.child, recipe, parameter_scope));
			return result;
		}
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE ||
			source.kind == FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE_ID)
		{
			result.kind = source.kind == FUNCTION_TEMPLATE_ABI_TYPE_DECLTYPE_ID ?
				ABI_TYPE_DECLTYPE_ID_EXPRESSION : ABI_TYPE_DECLTYPE_EXPRESSION;
			result.resolved_expression = AddFunctionTemplateAbiExpression(
				source.expression, recipe, parameter_scope);
			return result;
		}
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_BUILTIN_TRANSFORM)
		{
			if (source.child == kNoFunctionTemplateAbiType || source.name == 0)
				ThrowLoweringInternal(
					"function template ABI builtin transform is invalid");
			result.kind = ABI_TYPE_BUILTIN_TRANSFORM;
			result.index = ResolveName(source.name) + 1;
			result.types.push_back(
				MakeFunctionTemplateAbiType(source.child, recipe, parameter_scope));
			return result;
		}
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_MEMBER)
		{
			if (source.child == kNoFunctionTemplateAbiType || source.name == 0)
				ThrowLoweringInternal(
					"function template ABI result member is invalid");
			result.kind = ABI_TYPE_MEMBER;
			result.index = ResolveName(source.name) + 1;
			result.types.push_back(
				MakeFunctionTemplateAbiType(source.child, recipe, parameter_scope));
			return result;
		}
		if (source.child == kNoFunctionTemplateAbiType)
			ThrowLoweringInternal("function template ABI result modifier is invalid");
		result = MakeFunctionTemplateAbiType(source.child, recipe, parameter_scope);
		AbiTypeModifier modifier;
		if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_QUALIFIED)
		{
			modifier.kind = ABI_TYPE_CV;
			modifier.is_const = (source.cv & CV_CONST) != 0;
			modifier.is_volatile = (source.cv & CV_VOLATILE) != 0;
		}
		else if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_POINTER)
			modifier.kind = ABI_TYPE_POINTER;
		else if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_LVALUE_REFERENCE)
			modifier.kind = ABI_TYPE_LVALUE_REFERENCE;
		else if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_RVALUE_REFERENCE)
			modifier.kind = ABI_TYPE_RVALUE_REFERENCE;
		else if (source.kind == FUNCTION_TEMPLATE_ABI_TYPE_ARRAY)
		{
			modifier.kind = ABI_TYPE_ARRAY;
			if (source.parameter != kNoTemplateParameter)
			{
				modifier.array_bound.kind = ABI_ARRAY_BOUND_EXPRESSION;
				modifier.array_bound.resolved_expression =
					AddTemplateParameterExpression(source.parameter);
			}
			else if (source.bound != 0)
			{
				modifier.array_bound.kind = ABI_ARRAY_BOUND_INTEGER;
				modifier.array_bound.resolved_expression = source.bound;
			}
		}
		else ThrowLoweringInternal(
			"function template ABI result node kind is invalid");
		result.modifiers.insert(result.modifiers.begin(), modifier);
		return result;
	}

	bool UsesFunctionTemplateParameter(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe& recipe) const
	{
		using namespace semantic;
		std::vector<TypeId> pending(1, type);
		detail::FlatHashSet<TypeId> visited;
		const std::size_t type_limit = program_.types.Size() + 1;
		while (!pending.empty())
		{
			const TypeId current = pending.back();
			pending.pop_back();
			if (current >= type_limit)
				ThrowLoweringInternal(
					"function template ABI source type is invalid");
			if (!visited.Insert(current)) continue;
			std::size_t parameter = 0;
			if (FunctionTemplateParameter(
				current, function, &recipe, &parameter)) return true;
			const TypeRecord& record = program_.types.Get(current);
			if (record.kind == TYPE_ARRAY &&
				record.dependent_bound_parameter != kNoTemplateParameter)
				return true;
			if (record.kind == TYPE_QUALIFIED || record.kind == TYPE_POINTER ||
				record.kind == TYPE_BLOCK_POINTER ||
				record.kind == TYPE_LVALUE_REFERENCE ||
				record.kind == TYPE_RVALUE_REFERENCE || record.kind == TYPE_ARRAY)
			{
				pending.push_back(record.child);
				continue;
			}
			if (record.kind == TYPE_FUNCTION)
			{
				pending.push_back(record.child);
				const TypeId* parameters = program_.types.Parameters(current);
				for (std::size_t i = 0; i < record.parameter_count; ++i)
					pending.push_back(parameters[i]);
				continue;
			}
			if (record.kind == TYPE_MEMBER_POINTER)
			{
				pending.push_back(record.child);
				pending.push_back(static_cast<TypeId>(record.bound));
				continue;
			}
			if (record.kind != TYPE_NAMED) continue;
			const EntityRecord& entity = program_.entities[record.entity];
			if (entity.enclosing_class != kNoEntity)
				pending.push_back(
					program_.entities[entity.enclosing_class].type);
			if (entity.template_argument_count == 0) continue;
			const std::size_t first = entity.template_argument_begin;
			if (first > program_.template_arguments.size() ||
				entity.template_argument_count >
					program_.template_arguments.size() - first)
				ThrowLoweringInternal(
					"function template ABI source argument range is invalid");
			for (std::size_t i = 0; i < entity.template_argument_count; ++i)
			{
				const std::size_t argument = first + i;
				if (argument >= program_.canonical_template_arguments.size() ||
					program_.canonical_template_arguments[argument].kind ==
						TEMPLATE_ARGUMENT_TYPE)
					pending.push_back(program_.template_arguments[argument]);
			}
		}
		return false;
	}

	bool MakeTemplateParameterSpecialization(
		const semantic::EntityRecord& entity,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe,
		abi_mangle::AbiType* result)
	{
		using namespace abi_mangle;
		using namespace semantic;
		if (!recipe || entity.template_parameter_ordinal >=
			recipe->template_parameter_count ||
			entity.template_argument_count == 0) return false;
		result->kind = ABI_TYPE_TEMPLATE_PARAMETER_SPECIALIZATION;
		result->index = entity.template_parameter_ordinal;
		const std::size_t first = entity.template_argument_begin;
		if (first > program_.template_arguments.size() ||
			entity.template_argument_count >
				program_.template_arguments.size() - first)
			ThrowLoweringInternal(
				"template-parameter specialization ABI arguments are invalid");
		const std::size_t pack = entity.template_argument_pack_begin;
		const std::size_t fixed = pack == kNoTemplateParameter ?
			entity.template_argument_count : pack;
		if (fixed > entity.template_argument_count)
			ThrowLoweringInternal(
				"template-parameter specialization ABI pack is invalid");
		for (std::size_t i = 0; i < fixed; ++i)
			result->argument_refs.push_resolved(AddTemplateArgument(
				first + i, function, recipe));
		if (pack != kNoTemplateParameter)
			result->argument_refs.push_resolved(AddTemplateArgumentPack(
				first + fixed, entity.template_argument_count - fixed,
				function, recipe));
		return true;
	}

	void AppendClassTemplateArguments(const semantic::EntityRecord& entity,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe,
		abi_mangle::AbiType* result)
	{
		using namespace semantic;
		const std::size_t first = entity.template_argument_begin;
		if (first > program_.template_arguments.size() ||
			entity.template_argument_count >
				program_.template_arguments.size() - first)
			ThrowLoweringInternal(
				"class template ABI argument range is invalid");
		const std::size_t pack = entity.template_argument_pack_begin;
		const std::size_t fixed = pack == kNoTemplateParameter ?
			entity.template_argument_count : pack;
		if (fixed > entity.template_argument_count)
			ThrowLoweringInternal("class template ABI pack offset is invalid");
		for (std::size_t i = 0; i < fixed; ++i)
			result->argument_refs.push_resolved(AddTemplateArgument(
				first + i, function, recipe));
		if (pack != kNoTemplateParameter)
			result->argument_refs.push_resolved(AddTemplateArgumentPack(
				first + fixed, entity.template_argument_count - fixed,
				function, recipe));
	}
	abi_mangle::AbiType MakeBlockPointerType(semantic::TypeId function_type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe,
		abi_mangle::AbiType result)
	{
		result.kind = abi_mangle::ABI_TYPE_VENDOR_QUALIFIED;
		result.vendor_qualifier =
			abi_mangle::ABI_VENDOR_QUALIFIER_BLOCK_POINTER;
		result.types.push_back(MakeType(function_type, function, recipe));
		return result;
	}

	bool CanResolveType(const abi_mangle::AbiType& type) const
	{
		using namespace abi_mangle;
		if (!context_) return false;
		if (type.kind == ABI_TYPE_RESOLVED) return true;
		if (type.kind == ABI_TYPE_NAME_OR_REFERENCE ||
			!type.expression_ref.empty() || !type.context_ref.empty() ||
			(!type.argument_refs.resolved() &&
			 !type.argument_refs.empty())) return false;
		if (type.array_bound.kind == ABI_ARRAY_BOUND_EXPRESSION &&
			!type.array_bound.value.empty()) return false;
		for (std::size_t i = 0; i < type.modifiers.size(); ++i)
			if (type.modifiers[i].array_bound.kind ==
					ABI_ARRAY_BOUND_EXPRESSION &&
				!type.modifiers[i].array_bound.value.empty())
				return false;
		for (std::size_t i = 0; i < type.types.size(); ++i)
			if (!CanResolveType(type.types[i])) return false;
		return true;
	}

	abi_mangle::AbiType MakeFunctionTypeCore(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe,
		abi_mangle::AbiType result)
	{
		using namespace abi_mangle;
		using namespace semantic;
		const TypeRecord* record = &program_.types.Get(type);
		result.kind = ABI_TYPE_FUNCTION;
		result.types.push_back(MakeType(record->child, function, recipe));
		const TypeId* parameters = program_.types.Parameters(type);
		// A dependent function type's trailing parameter pack is an
		// expansion, not a C variadic ellipsis (`PFT_DpT0_E`).
		std::size_t pack = 0;
		const bool trailing_pack = record->variadic &&
			record->parameter_count != 0 && PackExpansionParameter(
				parameters[record->parameter_count - 1], function, recipe, &pack);
		for (std::size_t i = 0; i < record->parameter_count; ++i)
			result.types.push_back(trailing_pack &&
				i + 1 == record->parameter_count ?
				PackExpansion(MakeFunctionTemplateParameterType(pack)) :
				MakeType(parameters[i], function, recipe));
		result.is_const = (record->cv & CV_CONST) != 0;
		result.is_volatile = (record->cv & CV_VOLATILE) != 0;
		result.lvalue_ref =
			record->ref_qualifier == FUNCTION_REF_LVALUE;
		result.rvalue_ref =
			record->ref_qualifier == FUNCTION_REF_RVALUE;
		result.variadic = record->variadic && !trailing_pack;
		return result;
	}

	abi_mangle::AbiType MakeTypeCore(semantic::TypeId type,
		const semantic::BindingRecord* function,
		const semantic::FunctionTemplateAbiRecipe* recipe, std::size_t parameter_scope = 0)
	{
		using namespace abi_mangle;
		using namespace semantic;
		std::vector<AbiTypeModifier> modifiers;
		const TypeRecord* record = &program_.types.Get(type);
		std::size_t template_parameter = 0;
		while (record->kind == TYPE_QUALIFIED || record->kind == TYPE_POINTER ||
			record->kind == TYPE_LVALUE_REFERENCE ||
			record->kind == TYPE_RVALUE_REFERENCE || record->kind == TYPE_ARRAY)
		{
			if (FunctionTemplateParameter(
				type, function, recipe, &template_parameter))
			{
				AbiType result = MakeFunctionTemplateParameterType(template_parameter, parameter_scope);
				result.modifiers.swap(modifiers);
				return result;
			}
			AbiTypeModifier modifier;
			if (record->kind == TYPE_QUALIFIED)
			{
				modifier.kind = ABI_TYPE_CV;
				modifier.is_const = (record->cv & CV_CONST) != 0;
				modifier.is_volatile = (record->cv & CV_VOLATILE) != 0;
			}
			else if (record->kind == TYPE_ARRAY)
			{
				modifier.kind = ABI_TYPE_ARRAY;
				if (record->dependent_bound_parameter !=
					kNoTemplateParameter)
				{
					modifier.array_bound.kind =
						ABI_ARRAY_BOUND_EXPRESSION;
					modifier.array_bound.resolved_expression =
						AddTemplateParameterExpression(
							record->dependent_bound_parameter);
				}
				else if (record->bound != 0 || record->zero_length_array)
				{
					modifier.array_bound.kind = ABI_ARRAY_BOUND_INTEGER;
					modifier.array_bound.resolved_expression = record->bound;
				}
			}
			else modifier.kind = record->kind == TYPE_POINTER ? ABI_TYPE_POINTER :
				record->kind == TYPE_LVALUE_REFERENCE ? ABI_TYPE_LVALUE_REFERENCE :
				ABI_TYPE_RVALUE_REFERENCE;
			modifiers.push_back(modifier);
			type = record->child;
			record = &program_.types.Get(type);
		}
		AbiType result;
		result.modifiers.swap(modifiers);
		if (FunctionTemplateParameter(type, function, recipe, &template_parameter))
		{
			AbiType parameter = MakeFunctionTemplateParameterType(template_parameter, parameter_scope);
			parameter.modifiers.swap(result.modifiers);
			return parameter;
		}
		if (record->kind == TYPE_BLOCK_POINTER) return MakeBlockPointerType(
			record->child, function, recipe, result);
		if (record->kind == TYPE_FUNCTION)
			return MakeFunctionTypeCore(type, function, recipe, result);
		if (record->kind == TYPE_MEMBER_POINTER)
		{
			result.kind = ABI_TYPE_MEMBER_POINTER;
			result.types.push_back(MakeType(
				static_cast<TypeId>(record->bound), function, recipe));
			result.types.push_back(MakeType(record->child, function, recipe));
			return result;
		}
		if (record->kind == TYPE_VECTOR)
		{
			result.kind = ABI_TYPE_VECTOR;
			result.array_bound.kind = ABI_ARRAY_BOUND_INTEGER;
			result.array_bound.resolved_expression =
				record->bound / program_.SizeOf(record->child);
			result.types.push_back(MakeType(record->child, function, recipe));
			return result;
		}
		if (record->kind == TYPE_NAMED)
		{
			const EntityRecord& entity = program_.entities[record->entity];
			if (MakeTemplateParameterSpecialization(entity, function, recipe, &result)) {}
			else if (entity.lambda_closure)
			{
				if (ClosureIsMemberLambda(program_, record->entity))
					result = MakeMemberLambdaType(record->entity, result);
				else if (!ClosureIsNamespaceScope(program_, record->entity) &&
					!ClosureHasManglingNumber(program_, record->entity))
				{
					result.kind = ABI_TYPE_LOCAL_TYPE;
					AssignLocalContext(&result,
						AddLocalContext(entity.local_context));
					result.local_presentation =
						ABI_LOCAL_PRESENTATION_GENERATED_LAMBDA;
					result.resolved_expression =
						Names().AnonymousNumber(record->entity);
				}
				else if (ClosureHasManglingNumber(program_, record->entity))
				{
					result.kind = ABI_TYPE_LAMBDA_CLOSURE;
					AssignLocalContext(&result, AddClosureContext(record->entity));
					result.local_presentation =
						ABI_LOCAL_PRESENTATION_LAMBDA_DISCRIMINATOR;
					result.resolved_expression = entity.lambda_abi_ordinal;
					if (entity.lambda_call_operator == kNoBinding ||
						entity.lambda_call_operator >= program_.bindings.size())
						ThrowLoweringInternal(
							"lambda ABI type has no call operator fact");
					const BindingRecord& call =
						program_.bindings[entity.lambda_call_operator];
					const TypeRecord& call_type =
						program_.types.Get(call.type);
					const TypeId* parameters =
						program_.types.Parameters(call.type);
					for (std::size_t i = 0;
						i < call_type.parameter_count; ++i)
						result.types.push_back(MakeType(parameters[i]));
				}
				else
				{
					result.kind = ABI_TYPE_NAMESPACE_LAMBDA;
					SetNamespaceLambda(&result,
						ClosureNamingScope(program_, record->entity),
						Names().AnonymousNumber(record->entity));
				}
			}
			else if (entity.local_context != kNoBinding)
			{
				result.kind = ABI_TYPE_LOCAL_TYPE;
				AssignLocalContext(&result,
					AddLocalContext(entity.local_context));
				if (IsUnnamedClass(program_, record->entity) &&
					!UnnamedTypeIsVisible(program_, record->entity))
				{
					result.local_presentation =
						ABI_LOCAL_PRESENTATION_GENERATED_LAMBDA;
					result.resolved_expression =
						Names().AnonymousNumber(record->entity);
				}
				else
				{
					if (!entity.unnamed_class)
						result.index = ResolveName(entity.identity_name) + 1;
					result.local_presentation =
						ABI_LOCAL_PRESENTATION_NAME_ORDINAL;
					result.resolved_expression = entity.unnamed_class ?
						entity.local_name_ordinal :
						Names().ClassDiscriminator(record->entity);
				}
			}
			else if (entity.enclosing_class != kNoEntity)
			{
				if (entity.enclosing_class >= program_.entities.size())
					ThrowLoweringInternal(
						"nested ABI type has no enclosing class");
				const bool specialization =
					IsClassTemplateSpecialization(entity);
				result.kind = specialization ?
					ABI_TYPE_MEMBER_TEMPLATE_SPECIALIZATION : ABI_TYPE_MEMBER;
				// An unnamed member class is `Ut<n>_` where other units name
				// its class and a unit-local `$_<n>` otherwise.
				if (!IsUnnamedClass(program_, record->entity))
					result.index = ResolveName(entity.identity_name) + 1;
				else if (!UnnamedTypeIsVisible(program_, record->entity))
					result.index = context_->resolve_generated_name(
						AnonymousName(record->entity)) + 1;
				else
				{
					result.local_presentation =
						ABI_LOCAL_PRESENTATION_NAME_ORDINAL;
					result.resolved_expression = entity.local_name_ordinal;
				}
				result.types.push_back(MakeType(
					program_.entities[entity.enclosing_class].type,
					function, recipe));
				// The specialization is the substitution a member function's
				// structured class owner enters for the same class.
				if (specialization &&
					(!recipe || !UsesFunctionTemplateParameter(type, function, *recipe)))
					result.resolved_expression = make_semantic_substitution(
						ABI_SEMANTIC_SUBSTITUTION_CLASS, record->entity);
				if (specialization)
					AppendClassTemplateArguments(
						entity, function, recipe, &result);
			}
			else if (!IsClassTemplateSpecialization(entity))
			{
				result.kind = ABI_TYPE_NAMED;
				result.index = (IsUnnamedClass(program_, record->entity) ?
					ResolveAnonymousPath(record->entity) :
					ResolvePath(entity.owner, entity.identity_name)) + 1;
			}
			else
			{
				const StandardTemplateSubstitution standard =
					StandardSubstitutionFor(program_, entity);
				result.kind = standard.code !=
					ABI_STANDARD_SUBSTITUTION_TEXT ?
					ABI_TYPE_STD_TEMPLATE_SPECIALIZATION :
					ABI_TYPE_TEMPLATE_SPECIALIZATION;
				if (standard.code != ABI_STANDARD_SUBSTITUTION_TEXT)
				{
					result.standard_substitution_code = standard.code;
					result.standard_substitution_includes_arguments =
						standard.includes_arguments;
				}
				else if (!recipe || !UsesFunctionTemplateParameter(type, function, *recipe))
					result.resolved_expression = make_semantic_substitution(
						ABI_SEMANTIC_SUBSTITUTION_CLASS, record->entity);
				result.index = ResolvePath(
					entity.owner, entity.identity_name) + 1;
				AppendClassTemplateArguments(entity, function, recipe, &result);
			}
			AppendTypeAbiTags(
				&result, entity.abi_tag_begin, entity.abi_tag_count);
			return result;
		}
		if (MakeBuiltinAbiType(program_, *record, &result)) return result;
		ThrowLoweringSource("unsupported ABI type in PA10");
	}
};

bool AppendClassTemplateOwner(const semantic::Program& program,
	const semantic::BindingRecord& binding, AbiFactBuilder* builder,
	abi_mangle::AbiTypedCase* facts, bool retain_complete_substitution)
{
	using namespace abi_mangle;
	using namespace semantic;
	if (binding.member_owner == kNoEntity) return false;
	const EntityRecord& entity = program.entities[binding.member_owner];
	if (!IsClassTemplateSpecialization(entity)) return false;
	const std::size_t first = entity.template_argument_begin;
	if (first > program.template_arguments.size() ||
		entity.template_argument_count > program.template_arguments.size() - first)
		ThrowLoweringInternal("class template ABI owner arguments are invalid");
	std::vector<NameId> path;
	program.BuildEmissionPath(entity.owner, entity.identity_name, &path);
	const StandardTemplateSubstitution standard =
		StandardSubstitutionFor(program, entity);
	// Standard template substitutions include their `std::` owner. Do not emit
	// that namespace again when the substitution is a nested-name prefix.
	const std::size_t path_begin = standard.code !=
		ABI_STANDARD_SUBSTITUTION_TEXT && path.size() == 2 ? 1 : 0;
	std::size_t resolved_prefix = ABI_NO_RESOLVED_REFERENCE;
	for (std::size_t i = path_begin; i < path.size(); ++i)
	{
		AbiFactRecord component;
		component.set_kind(ABI_FACT_RECORD_FUNCTION);
		builder->AppendNameComponent(
			&component.function, path[i], &resolved_prefix);
		if (i + 1 == path.size())
		{
			component.function.kind = ABI_FUNCTION_RECORD_NAME_TEMPLATE;
			if (retain_complete_substitution)
				component.function.type.resolved_expression =
					make_semantic_substitution(
						ABI_SEMANTIC_SUBSTITUTION_CLASS,
						binding.member_owner);
			component.function.standard_substitution_code = standard.code;
			component.function.standard_substitution_includes_arguments =
				standard.includes_arguments;
			const std::size_t pack = entity.template_argument_pack_begin;
			const std::size_t fixed = pack == kNoTemplateParameter ?
				entity.template_argument_count : pack;
			if (fixed > entity.template_argument_count)
				ThrowLoweringInternal("class template ABI pack offset is invalid");
			for (std::size_t argument = 0; argument < fixed; ++argument)
				component.function.argument_refs.push_resolved(
					builder->AddTemplateArgument(first + argument));
			if (pack != kNoTemplateParameter)
				component.function.argument_refs.push_resolved(
					builder->AddTemplateArgumentPack(first + fixed,
						entity.template_argument_count - fixed));
		}
		else if (i == 0 && program.IsInStandardNamespace(entity.owner))
			component.function.kind = ABI_FUNCTION_RECORD_NAME_STD;
		else component.function.kind = ABI_FUNCTION_RECORD_NAME_SOURCE;
		AppendTypedFact(facts, &component);
	}
	return true;
}

}

namespace
{

std::string MangleProductionCase(const abi_mangle::AbiTypedCase& fact_case,
	abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context)
{
	if (stats)
	{
		++stats->production_mangles;
		stats->production_fact_bytes +=
			abi_mangle::abi_typed_case_storage_bytes(fact_case);
		for (const abi_mangle::AbiDefinitionRecord& definition :
			fact_case.definitions)
		{
			switch (definition.kind)
			{
			case abi_mangle::ABI_DEFINITION_TYPE:
				++stats->production_type_definitions; break;
			case abi_mangle::ABI_DEFINITION_TEMPLATE_ARGUMENT:
				++stats->production_argument_definitions; break;
			case abi_mangle::ABI_DEFINITION_EXPRESSION:
				++stats->production_expression_definitions; break;
			case abi_mangle::ABI_DEFINITION_CONTEXT:
				++stats->production_context_definitions; break;
			case abi_mangle::ABI_DEFINITION_ENTITY:
				++stats->production_entity_definitions; break;
			}
		}
	}
	if (context) return context->mangle_case(fact_case);
	abi_mangle::AbiMangleContext local_context(stats);
	return local_context.mangle_case(fact_case);
}

}

std::string MangleType(const semantic::Program& program, semantic::TypeId type,
	abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context, LocalNames* names)
{
	using namespace abi_mangle;
	std::unique_ptr<AbiMangleContext> local_context;
	if (!context)
	{
		local_context.reset(new AbiMangleContext(stats));
		context = local_context.get();
	}
	AbiTypedCase fact_case;
	AbiFactBuilder facts(program, fact_case, context, stats, names);
	AbiFactRecord target;
	target.set_kind(ABI_FACT_RECORD_TARGET);
	target.target.kind = ABI_TARGET_FACT_TYPE;
	target.target.type = facts.MakeType(type);
	AppendTypedFact(&fact_case, &target);
	return MangleProductionCase(fact_case, stats, context);
}


bool HasWeakLinkage(
	const semantic::Program& program, semantic::BindingId binding, bool function)
{
	const semantic::BindingRecord& record = program.bindings[binding];
	const semantic::BindingRecord& canonical = program.bindings[record.canonical];
	const bool class_template_member = record.member_owner != semantic::kNoEntity &&
		IsClassTemplateSpecialization(program.entities[record.member_owner]);
	const bool primary_template_member = class_template_member &&
		!program.entities[record.member_owner].explicit_template_specialization;
	const bool preempted = record.explicit_instantiation_suppressed ||
		canonical.explicit_instantiation_suppressed;
	return canonical.weak_symbol || (!preempted &&
		(record.weak_odr || primary_template_member ||
		 (function && (record.template_argument_count != 0 ||
		   record.function_template_specialization) &&
		  !record.explicit_function_specialization)));
}


void AppendFunctionTemplateArgumentsAndResult(const semantic::Program& program,
	const semantic::BindingRecord& binding,
	const semantic::TypeRecord& function_type,
	const semantic::FunctionTemplateAbiRecipe* recipe,
	AbiFactBuilder* facts, abi_mangle::AbiTypedCase* output)
{
	using namespace abi_mangle;
	using namespace semantic;
	// An empty pack is still a template argument list: "IJEE".
	if (binding.template_argument_count == 0 &&
		!(binding.function_template_specialization && recipe &&
		  (recipe->template_parameter_pack ||
		   recipe->argument_partition_begin != kNoTemplateParameter))) return;
	const auto append_argument = [&](std::size_t resolved) {
		AbiFactRecord argument;
		argument.set_kind(ABI_FACT_RECORD_FUNCTION);
		argument.function.kind = ABI_FUNCTION_RECORD_FUNCTION_TEMPLATE_ARGUMENT;
		argument.function.argument_refs.push_resolved(resolved);
		AppendTypedFact(output, &argument);
	};
	if (recipe) facts->ForEachFunctionTemplateArgument(binding, *recipe, append_argument);
	else
	{
		const std::size_t first = binding.template_argument_begin;
		const std::size_t count = binding.template_argument_count;
		if (first > program.template_arguments.size() ||
			count > program.template_arguments.size() - first)
			ThrowLoweringInternal("function template argument range is invalid during mangling");
		for (std::size_t i = 0; i < count; ++i)
			append_argument(facts->AddTemplateArgument(first + i, &binding, 0, i));
	}
	// Itanium constructor, destructor, and conversion-function encodings do
	// not carry a result type, including when the callable is a template.
	if (binding.constructor || binding.destructor ||
		binding.conversion_function) return;
	AbiFactRecord result;
	result.set_kind(ABI_FACT_RECORD_FUNCTION);
	result.function.kind = ABI_FUNCTION_RECORD_RESULT;
	if (recipe && recipe->result_type != kNoFunctionTemplateAbiType)
	{
		result.function.type = facts->MakeFunctionTemplateAbiType(
			recipe->result_type, *recipe);
	}
	else
	{
		TypeId result_type = function_type.child;
		if (recipe)
		{
			const TypeId source = program.types.Get(recipe->function_type).child;
			if (facts->UsesFunctionTemplateParameter(source, &binding, *recipe))
				result_type = source;
		}
		result.function.type = facts->MakeFunctionTemplateType(result_type,
			binding, result_type == function_type.child ? 0 : recipe);
	}
	AppendTypedFact(output, &result);
}

std::string MangleLambdaCallOperator(const semantic::Program& program,
	const semantic::BindingRecord& binding,
	const semantic::EntityRecord& lambda, abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context, LocalNames* names)
{
	using namespace abi_mangle;
	using namespace semantic;
	if (!context)
		ThrowLoweringInternal("lambda ABI mangling has no graph context");
	if (binding.operator_kind != OPERATOR_CALL)
		ThrowLoweringInternal("invalid lambda call-operator ABI identity");
	AbiTypedCase fact_case;
	AbiFactBuilder facts(program, fact_case, context, stats, names);
	const FunctionTemplateAbiRecipe* recipe = 0;
	if (binding.function_template_abi_recipe != kNoFunctionTemplateAbiRecipe)
	{
		if (binding.function_template_abi_recipe >=
			program.function_template_abi_recipes.size())
			ThrowLoweringInternal("invalid generic lambda ABI recipe");
		recipe = &program.function_template_abi_recipes[
			binding.function_template_abi_recipe];
	}
	AbiFactRecord lambda_target;
	lambda_target.set_kind(ABI_FACT_RECORD_TARGET);
	lambda_target.target.kind = ABI_TARGET_FACT_FUNCTION;
	AbiFunctionTarget& function = lambda_target.target.function;
	facts.SetClosureTarget(&function, binding.member_owner);
	if (function.kind == ABI_FUNCTION_TARGET_MEMBER)
	{
		AbiFactRecord terminal;
		terminal.set_kind(ABI_FACT_RECORD_FUNCTION);
		terminal.function.kind = ABI_FUNCTION_RECORD_OPERATOR_TERMINAL;
		terminal.function.terminal_code = ABI_TERMINAL_CALL;
		AppendTypedFact(&fact_case, &terminal);
	}
	(void)lambda;
	const TypeRecord& lambda_type = program.types.Get(binding.type);
	AbiFactRecord qualifier;
	qualifier.set_kind(ABI_FACT_RECORD_FUNCTION);
	qualifier.function.kind = ABI_FUNCTION_RECORD_QUALIFIER;
	if ((lambda_type.cv & CV_CONST) != 0)
		qualifier.function.qualifiers.push_back(
			ABI_FUNCTION_QUALIFIER_CONST);
	if ((lambda_type.cv & CV_VOLATILE) != 0)
		qualifier.function.qualifiers.push_back(
			ABI_FUNCTION_QUALIFIER_VOLATILE);
	if (!qualifier.function.qualifiers.empty())
		AppendTypedFact(&fact_case, &qualifier);
	const TypeId signature_type = recipe ? recipe->function_type : binding.type;
	const TypeRecord& signature = program.types.Get(signature_type);
	const TypeId* signature_parameters = program.types.Parameters(signature_type);
	if (function.kind == ABI_FUNCTION_TARGET_LAMBDA)
		for (std::size_t i = 0; i < signature.parameter_count; ++i)
			function.signature_parameter_types.push_back(recipe ?
				facts.MakeFunctionTemplateType(
					signature_parameters[i], binding, recipe) :
				facts.MakeType(signature_parameters[i]));
	AppendFunctionTemplateArgumentsAndResult(program, binding, lambda_type,
		recipe, &facts, &fact_case);
	const TypeId* lambda_parameters = program.types.Parameters(binding.type);
	for (std::size_t i = 0; i < lambda_type.parameter_count; ++i)
	{
		AbiFactRecord parameter;
		parameter.set_kind(ABI_FACT_RECORD_FUNCTION);
		parameter.function.kind = ABI_FUNCTION_RECORD_PARAMETER;
		parameter.function.type = facts.MakeType(lambda_parameters[i]);
		AppendTypedFact(&fact_case, &parameter);
	}
	AppendTypedFact(&fact_case, &lambda_target);
	return MangleProductionCase(fact_case, stats, context);
}

// The local class or closure type that owns a member function: a closure
// with a mangling number is `Ul<sig>E<n>_`, a unit-local unnamed class or
// closure `$_<n>`, an unnamed class other units name `Ut<n>_`, and a named
// class its name and discriminator.
void AppendLocalFunctionOwner(const semantic::Program& program,
	semantic::EntityId owner_id, AbiFactBuilder* facts,
	abi_mangle::AbiMangleContext* context,
	abi_mangle::AbiTypedCase* fact_case)
{
	using namespace abi_mangle;
	using namespace semantic;
	const EntityRecord& owner = program.entities[owner_id];
	AbiFactRecord local;
	local.set_kind(ABI_FACT_RECORD_FUNCTION);
	local.function.kind = ABI_FUNCTION_RECORD_LOCAL_CONTEXT;
	AbiFactBuilder::AssignLocalContext(&local.function, owner.lambda_closure ?
		facts->AddClosureContext(owner_id) :
		facts->AddLocalContext(owner.local_context));
	local.function.local_presentation = ABI_LOCAL_PRESENTATION_NAME_ORDINAL;
	local.function.type.resolved_expression = 0;
	if (owner.lambda_closure && ClosureHasManglingNumber(program, owner_id))
	{
		local.function.kind = ABI_FUNCTION_RECORD_LAMBDA_CONTEXT;
		local.function.type.resolved_expression = owner.lambda_abi_ordinal;
		const TypeId call = program.bindings[owner.lambda_call_operator].type;
		const TypeId* parameters = program.types.Parameters(call);
		for (std::size_t i = 0; i < program.types.Get(call).parameter_count; ++i)
			local.function.types.push_back(facts->MakeType(parameters[i]));
	}
	else if (owner.lambda_closure || (IsUnnamedClass(program, owner_id) &&
		!UnnamedTypeIsVisible(program, owner_id)))
		facts->SetGeneratedLocalSourceName(&local.function,
			facts->AnonymousName(owner_id));
	else if (owner.unnamed_class)
		local.function.type.resolved_expression = owner.local_name_ordinal;
	else
	{
		facts->SetLocalSourceName(&local.function, owner.identity_name);
		local.function.type.resolved_expression =
			facts->Names().ClassDiscriminator(owner_id);
	}
	if (local.function.kind == ABI_FUNCTION_RECORD_LOCAL_CONTEXT)
		local.function.types.push_back(facts->MakeType(owner.type));
	AppendTypedFact(fact_case, &local);
	AppendComponentAbiTagFacts(program, owner, context, fact_case);
}

void AppendMemberFunctionQualifiers(const semantic::Program& program,
	const semantic::BindingRecord& binding, bool member,
	abi_mangle::AbiTypedCase* fact_case)
{
	using namespace abi_mangle;
	using namespace semantic;
	if (!member) return;
	const TypeRecord& declared_type = program.types.Get(binding.type);
	AbiFactRecord qualifier;
	qualifier.set_kind(ABI_FACT_RECORD_FUNCTION);
	qualifier.function.kind = ABI_FUNCTION_RECORD_QUALIFIER;
	if ((declared_type.cv & CV_CONST) != 0)
		qualifier.function.qualifiers.push_back(
			ABI_FUNCTION_QUALIFIER_CONST);
	if ((declared_type.cv & CV_VOLATILE) != 0)
		qualifier.function.qualifiers.push_back(
			ABI_FUNCTION_QUALIFIER_VOLATILE);
	if (declared_type.ref_qualifier == FUNCTION_REF_LVALUE)
		qualifier.function.qualifiers.push_back(
			ABI_FUNCTION_QUALIFIER_LVALUE_REFERENCE);
	else if (declared_type.ref_qualifier == FUNCTION_REF_RVALUE)
		qualifier.function.qualifiers.push_back(
			ABI_FUNCTION_QUALIFIER_RVALUE_REFERENCE);
	if (!qualifier.function.qualifiers.empty())
		AppendTypedFact(fact_case, &qualifier);
}

// An operator's, conversion function's, constructor's or destructor's
// terminal record.
void AppendSpecialTerminal(const semantic::Program& program,
	const semantic::BindingRecord& binding,
	const semantic::BindingRecord* inherited,
	abi_mangle::AbiTerminalKind operator_terminal,
	bool force_lifecycle_base_entry, AbiFactBuilder* facts,
	abi_mangle::AbiTypedCase* fact_case)
{
	using namespace abi_mangle;
	using namespace semantic;
	if (binding.operator_kind == OPERATOR_LITERAL ||
		operator_terminal != ABI_TERMINAL_NONE)
	{
		AbiFactRecord terminal;
		terminal.set_kind(ABI_FACT_RECORD_FUNCTION);
		terminal.function.kind = ABI_FUNCTION_RECORD_OPERATOR_TERMINAL;
		if (binding.operator_kind == OPERATOR_LITERAL)
		{
			terminal.function.terminal_code = ABI_TERMINAL_LITERAL;
			facts->SetSourceName(
				&terminal.function, binding.operator_literal_suffix);
		}
		else terminal.function.terminal_code = operator_terminal;
		AppendTypedFact(fact_case, &terminal);
	}
	else if (binding.conversion_function)
	{
		AbiFactRecord terminal;
		terminal.set_kind(ABI_FACT_RECORD_FUNCTION);
		terminal.function.kind = ABI_FUNCTION_RECORD_CONVERSION_TERMINAL;
		terminal.function.type = facts->MakeConversionFunctionType(binding);
		AppendTypedFact(fact_case, &terminal);
	}
	else if (binding.constructor)
	{
		AbiFactRecord terminal;
		terminal.set_kind(ABI_FACT_RECORD_FUNCTION);
		terminal.function.kind = ABI_FUNCTION_RECORD_TERMINAL;
		const bool base_entry = binding.constructor_base_entry || force_lifecycle_base_entry;
		terminal.function.terminal_code = inherited ?
			(base_entry ? ABI_TERMINAL_INHERITED_CONSTRUCTOR_BASE : ABI_TERMINAL_INHERITED_CONSTRUCTOR_COMPLETE) :
			(base_entry ? ABI_TERMINAL_CONSTRUCTOR_BASE : ABI_TERMINAL_CONSTRUCTOR_COMPLETE);
		if (inherited) terminal.function.type = facts->MakeType(program.entities[inherited->member_owner].type);
		AppendTypedFact(fact_case, &terminal);
	}
	else if (binding.destructor)
	{
		AbiFactRecord terminal;
		terminal.set_kind(ABI_FACT_RECORD_FUNCTION);
		terminal.function.kind = ABI_FUNCTION_RECORD_TERMINAL;
		terminal.function.terminal_code = binding.destructor_base_entry ||
			force_lifecycle_base_entry ?
			ABI_TERMINAL_DESTRUCTOR_BASE :
			ABI_TERMINAL_DESTRUCTOR_COMPLETE;
		AppendTypedFact(fact_case, &terminal);
	}
}

std::string MangleFunction(const semantic::Program& program,
	const semantic::DumpNode& node,
	bool force_lifecycle_base_entry, abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context, LocalNames* names)
{
	using namespace abi_mangle;
	using namespace semantic;
	const BindingRecord& binding = program.bindings[node.binding];
	if (binding.assembly_name != 0)
	{
		if (stats) ++stats->external_assembly_names;
		return program.names.Get(binding.assembly_name);
	}
	if (binding.owner == program.GlobalScope() &&
		program.names.Get(binding.name) == "main") return std::string();
	if (binding.builtin_function != BUILTIN_FUNCTION_NONE)
	{
		if (stats) ++stats->external_builtin_runtime_names;
		if (binding.builtin_function == BUILTIN_FUNCTION_OPERATOR_NEW)
			return "cppgm_builtin_operator_new";
		if (binding.builtin_function == BUILTIN_FUNCTION_OPERATOR_DELETE)
			return "cppgm_builtin_operator_delete";
		if (binding.builtin_function == BUILTIN_FUNCTION_OPERATOR_NEW_ARRAY)
			return "cppgm_builtin_operator_new_array";
		if (binding.builtin_function == BUILTIN_FUNCTION_OPERATOR_DELETE_ARRAY)
			return "cppgm_builtin_operator_delete_array";
		if (binding.builtin_function == BUILTIN_FUNCTION_ABORT)
			return "abort";
		return "cppgm_builtin_" + program.names.Get(binding.name).substr(10);
	}
	if (binding.language_linkage == LANGUAGE_LINKAGE_C)
	{
		if (stats) ++stats->external_c_function_names;
		return program.names.Get(binding.name);
	}
	std::unique_ptr<AbiMangleContext> local_context;
	if (!context)
	{
		local_context.reset(new AbiMangleContext(stats));
		context = local_context.get();
	}
	const EntityRecord* lambda = binding.member_owner == kNoEntity ? 0 :
		&program.entities[binding.member_owner];
	if (lambda && lambda->lambda_closure &&
		binding.operator_kind == OPERATOR_CALL)
		return MangleLambdaCallOperator(
			program, binding, *lambda, stats, context, names);
	AbiTypedCase fact_case;
	AbiFactBuilder facts(program, fact_case, context, stats, names);
	const BindingRecord* inherited = InheritedConstructorAbiSource(program, binding, node);
	const FunctionTemplateAbiRecipe* recipe = FunctionAbiRecipe(program,
		inherited ? *inherited : binding);
	const BindingRecord& signature_binding = inherited ? *inherited : binding;
	// A captureless closure's static invoker is its static member function
	// `__invoke`, as clang names it (GCC's is `_FUN`).
	const EntityId owner_id = binding.lambda_invocation ?
		binding.lambda_invocation_owner : binding.member_owner;
	// A data member initializer's numbered closure is its class's member.
	const bool member_lambda_owner = owner_id != kNoEntity &&
		program.entities[owner_id].lambda_closure &&
		ClosureIsMemberLambda(program, owner_id);
	// An in-class default argument's closure is named in its function even
	// where a namespace-scope initializer's call analyzed it.
	const bool structured_local_owner = owner_id != kNoEntity &&
		!member_lambda_owner &&
		(program.entities[owner_id].local_context != kNoBinding ||
		 program.entities[owner_id].lambda_default_argument != 0) &&
		!ClosureIsNamespaceScope(program, owner_id);
	const bool tagged_class_owner = binding.member_owner != kNoEntity &&
		ClassOwnerHasAbiTags(program, binding.member_owner);
	const bool nested_specialized_class_owner =
		binding.member_owner != kNoEntity &&
		ClassOwnerHasSpecializedAncestor(program, binding.member_owner);
	const bool structured_class_owner = binding.member_owner != kNoEntity &&
		!structured_local_owner && !tagged_class_owner &&
		!nested_specialized_class_owner &&
		IsClassTemplateSpecialization(program.entities[binding.member_owner]);
	const bool structured_owner =
		structured_local_owner || structured_class_owner;
	const bool typed_class_owner = member_lambda_owner ||
		(binding.member_owner != kNoEntity && !structured_local_owner &&
		 (tagged_class_owner || nested_specialized_class_owner ||
		  ClassOwnerIsNested(program, binding.member_owner)));
	AbiFactRecord target;
	target.set_kind(ABI_FACT_RECORD_TARGET);
	target.target.kind = ABI_TARGET_FACT_FUNCTION;
	target.target.internal_linkage =
		binding.storage_class == STORAGE_CLASS_STATIC && binding.member_owner == kNoEntity &&
		!binding.lambda_invocation && program.KindOfScope(binding.owner) == SCOPE_NAMESPACE &&
		!program.HasInternalLinkageScope(binding.owner);
	target.target.function.kind = structured_owner ?
		ABI_FUNCTION_TARGET_ENCODING : typed_class_owner ?
			ABI_FUNCTION_TARGET_MEMBER : ABI_FUNCTION_TARGET_PATH;
	if (!structured_owner && !facts.SetAnonymousMemberPath(
		&target.target.function, owner_id, binding.lambda_invocation ?
			facts.ResolveGeneratedName("__invoke") :
			facts.ResolveSourceName(binding.name)))
		facts.SetPath(&target.target.function, binding.owner, binding.name);
	if (typed_class_owner)
	{
		const EntityRecord& owner = program.entities[owner_id];
		target.target.function.owner_type = facts.MakeType(owner.type);
		if (binding.lambda_invocation)
			target.target.function.set_resolved_source_name(
				facts.ResolveGeneratedName("__invoke"));
		else facts.SetSourceName(&target.target.function, binding.name);
	}
	AppendTypedFact(&fact_case, &target);
	AppendFunctionAbiTagFacts(program, binding, context, &fact_case);
	if (structured_local_owner)
		AppendLocalFunctionOwner(
			program, owner_id, &facts, context, &fact_case);
	if (structured_class_owner &&
		!AppendClassTemplateOwner(program, binding, &facts, &fact_case, true))
		ThrowLoweringInternal("class template ABI owner was lost");
	const bool member = binding.member_owner != kNoEntity &&
		!binding.static_member_function;
	// Most member dump boundaries prepend the implicit object and the ABI path
	// skips it. Out-of-class defaulted assignment definitions can retain only
	// the declared parameter; use the binding type for that shortened boundary.
	bool use_declared_assignment_parameters = false;
	if (member && binding.operator_kind == OPERATOR_ASSIGN)
	{
		const TypeRecord& node_function_type = program.types.Get(node.type);
		const TypeRecord& binding_function_type = program.types.Get(binding.type);
		use_declared_assignment_parameters =
			node_function_type.parameter_count !=
				binding_function_type.parameter_count + 1;
	}
	const TypeId encoded_function_type = use_declared_assignment_parameters ?
		binding.type : node.type;
	const TypeRecord& function_type =
		program.types.Get(encoded_function_type);
	const TypeId* parameters =
		program.types.Parameters(encoded_function_type);
	AppendFunctionTemplateArgumentsAndResult(program, signature_binding, function_type,
		recipe, &facts, &fact_case);
	AppendMemberFunctionQualifiers(program, binding, member, &fact_case);
	const AbiTerminalKind operator_terminal =
		OperatorTerminal(binding.operator_kind, member,
			program.types.Get(binding.type).parameter_count);
	if (structured_owner && binding.operator_kind == OPERATOR_NONE &&
		!binding.conversion_function && !binding.constructor && !binding.destructor)
	{
		AbiFactRecord terminal;
		terminal.set_kind(ABI_FACT_RECORD_FUNCTION);
		terminal.function.kind = structured_local_owner ?
			ABI_FUNCTION_RECORD_TERMINAL_SOURCE :
			ABI_FUNCTION_RECORD_NAME_SOURCE;
		if (binding.lambda_invocation)
			facts.SetGeneratedSourceName(&terminal.function, "__invoke");
		else facts.SetSourceName(&terminal.function, binding.name);
		AppendTypedFact(&fact_case, &terminal);
	}
	AppendSpecialTerminal(program, binding, inherited, operator_terminal,
		force_lifecycle_base_entry, &facts, &fact_case);
	const std::size_t first_parameter =
		member && !use_declared_assignment_parameters ? 1 : 0;
	const TypeRecord* recipe_function = recipe ?
		&program.types.Get(recipe->function_type) : 0;
	const TypeId* recipe_parameters = recipe_function &&
		recipe_function->parameter_count != 0 ?
		program.types.Parameters(recipe->function_type) : 0;
	const auto append_parameter = [&](TypeId parameter_type,
		const FunctionTemplateAbiRecipe* parameter_recipe,
		FunctionTemplateAbiTypeId retained_type,
		bool pack_expansion)
	{
		AbiFactRecord parameter;
		parameter.set_kind(ABI_FACT_RECORD_FUNCTION);
		parameter.function.kind = ABI_FUNCTION_RECORD_PARAMETER;
		parameter.function.type = retained_type != kNoFunctionTemplateAbiType ?
			facts.MakeFunctionTemplateAbiType(retained_type, *parameter_recipe) :
			facts.MakeFunctionTemplateType(
				parameter_type, signature_binding, parameter_recipe);
		if (pack_expansion)
		{
			AbiTypeModifier expansion;
			expansion.kind = ABI_TYPE_PACK_EXPANSION;
			parameter.function.type.modifiers.insert(
				parameter.function.type.modifiers.begin(), expansion);
		}
		AppendTypedFact(&fact_case, &parameter);
	};
	if (recipe_function)
	{
		for (std::size_t i = 0; i < recipe_function->parameter_count; ++i)
			append_parameter(recipe_parameters[i], recipe,
				facts.FunctionTemplateParameterAbiType(*recipe, i),
				recipe->function_parameter_pack &&
				i + 1 == recipe_function->parameter_count);
	}
	else
	{
		for (std::size_t i = first_parameter;
			i < function_type.parameter_count; ++i)
			append_parameter(parameters[i], 0,
				kNoFunctionTemplateAbiType, false);
	}
	if (function_type.variadic)
	{
		AbiFactRecord variadic;
		variadic.set_kind(ABI_FACT_RECORD_FUNCTION);
		variadic.function.kind = ABI_FUNCTION_RECORD_VARIADIC;
		AppendTypedFact(&fact_case, &variadic);
	}
	return MangleProductionCase(fact_case, stats, context);
}

std::string MangleVariable(const semantic::Program& program,
	const semantic::DumpNode& node,
	abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context, LocalNames* names)
{
	using namespace abi_mangle;
	using namespace semantic;
	const BindingRecord& binding = program.bindings[node.binding];
	if (binding.language_linkage == LANGUAGE_LINKAGE_C &&
		binding.storage_class != STORAGE_CLASS_STATIC)
	{
		if (stats) ++stats->external_c_variable_names;
		return program.names.Get(binding.name);
	}
	// Linux global-namespace TLS objects retain their source spelling; wrapper
	// functions still use the corresponding Itanium TLS special name.
	if (binding.owner == program.GlobalScope() &&
		binding.member_owner == kNoEntity &&
		binding.thread_local_storage &&
		binding.storage_class != STORAGE_CLASS_STATIC &&
		!binding.unnamed_namespace_linkage &&
		!binding.variable_template_specialization &&
		binding.template_argument_count == 0)
	{
		if (stats) ++stats->external_global_tls_names;
		return program.names.Get(binding.name);
	}
	std::unique_ptr<AbiMangleContext> local_context;
	if (!context)
	{
		local_context.reset(new AbiMangleContext(stats));
		context = local_context.get();
	}
	AbiTypedCase fact_case;
	AbiFactBuilder facts(program, fact_case, context, stats, names);
	const bool tagged_class_owner = binding.member_owner != kNoEntity &&
		ClassOwnerHasAbiTags(program, binding.member_owner);
	const bool nested_specialized_class_owner =
		binding.member_owner != kNoEntity &&
		ClassOwnerHasSpecializedAncestor(program, binding.member_owner);
	const bool structured_class_owner = binding.member_owner != kNoEntity &&
		!tagged_class_owner && !nested_specialized_class_owner &&
		IsClassTemplateSpecialization(program.entities[binding.member_owner]);
	const bool typed_class_owner = binding.member_owner != kNoEntity &&
		(tagged_class_owner || nested_specialized_class_owner ||
		 ClassOwnerIsNested(program, binding.member_owner));
	const bool member_variable_template =
		(structured_class_owner || typed_class_owner) &&
		binding.variable_template_specialization;
	AbiFactRecord target;
	target.set_kind(ABI_FACT_RECORD_TARGET);
	target.target.kind = ABI_TARGET_FACT_VARIABLE;
	target.target.function.kind = structured_class_owner ?
		ABI_FUNCTION_TARGET_ENCODING : typed_class_owner ?
		ABI_FUNCTION_TARGET_MEMBER : ABI_FUNCTION_TARGET_PATH;
	target.target.internal_linkage = VariableHasInternalMarker(program, binding);
	target.target.internal_unmarked = binding.anonymous_union_storage &&
		binding.member_owner == kNoEntity;
	const bool needs_path =
		target.target.function.kind == ABI_FUNCTION_TARGET_PATH ||
		(target.target.function.kind == ABI_FUNCTION_TARGET_MEMBER &&
		 (binding.template_argument_count != 0 ||
		  binding.function_template_specialization));
	if (needs_path)
	{
		if (binding.name == 0)
			ThrowLoweringInternal("variable ABI target has no semantic name");
		facts.SetPath(&target.target.function, binding.owner,
			VariableAbiName(binding));
	}
	if (typed_class_owner)
	{
		const EntityRecord& owner = program.entities[binding.member_owner];
		target.target.function.owner_type = facts.MakeType(owner.type);
		facts.SetSourceName(&target.target.function, binding.name);
	}
	AppendTypedFact(&fact_case, &target);
	if (structured_class_owner)
	{
		if (!AppendClassTemplateOwner(
			program, binding, &facts, &fact_case, false))
			ThrowLoweringInternal("class template ABI variable owner was lost");
		AbiFactRecord member;
		member.set_kind(ABI_FACT_RECORD_FUNCTION);
		member.function.kind = ABI_FUNCTION_RECORD_NAME_SOURCE;
		facts.SetSourceName(&member.function, binding.name);
		AppendTypedFact(&fact_case, &member);
	}
	// A variable template specialization carries its template arguments
	// (`_Z2vtIiE`, `_ZN2ns2nvIiEE`), at namespace scope as in a class.
	if ((member_variable_template || (binding.variable_template_specialization &&
		binding.member_owner == kNoEntity)) && binding.template_argument_count != 0)
	{
		const std::size_t first = binding.template_argument_begin;
		const std::size_t count = binding.template_argument_count;
		if (first > program.template_arguments.size() ||
			count > program.template_arguments.size() - first)
			ThrowLoweringInternal(
				"variable template argument range is invalid during mangling");
		for (std::size_t i = 0; i < count; ++i)
		{
			const std::size_t argument_id = facts.AddTemplateArgument(first + i);
			AbiFactRecord argument;
			argument.set_kind(ABI_FACT_RECORD_FUNCTION);
			argument.function.kind =
				ABI_FUNCTION_RECORD_FUNCTION_TEMPLATE_ARGUMENT;
			argument.function.argument_refs.push_resolved(argument_id);
			AppendTypedFact(&fact_case, &argument);
		}
	}
	return MangleProductionCase(fact_case, stats, context);
}

std::string MangleThreadLocalWrapper(const semantic::Program& program,
	semantic::BindingId binding_id, semantic::NameId fallback_name,
	abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context, LocalNames* names)
{
	using namespace abi_mangle;
	using namespace semantic;
	if (binding_id == kNoBinding || binding_id >= program.bindings.size())
		ThrowLoweringInternal("invalid thread-local wrapper binding");
	const BindingRecord& binding = program.bindings[binding_id];
	std::unique_ptr<AbiMangleContext> local_context;
	if (!context)
	{
		local_context.reset(new AbiMangleContext(stats));
		context = local_context.get();
	}
	AbiTypedCase fact_case;
	AbiFactBuilder facts(program, fact_case, context, stats, names);
	AbiFactRecord target;
	target.set_kind(ABI_FACT_RECORD_TARGET);
	target.target.kind = ABI_TARGET_FACT_THREAD_LOCAL_WRAPPER;
	target.target.internal_linkage = VariableHasInternalMarker(program, binding);
	const NameId terminal = binding.name != 0 ? binding.name : fallback_name;
	if (terminal != 0 && !program.names.Get(terminal).empty())
		facts.SetPath(&target.target.function, binding.owner, terminal);
	if (target.target.function.resolved_path == ABI_NO_RESOLVED_REFERENCE)
		ThrowLoweringInternal("thread-local wrapper has no semantic name (binding " +
			std::to_string(binding_id) + ", owner " +
			std::to_string(binding.owner) + ", terminal " +
			std::to_string(binding.name) + ", fallback " +
			std::to_string(fallback_name) + ")");
	AppendTypedFact(&fact_case, &target);
	return MangleProductionCase(fact_case, stats, context);
}

std::string MangleLocalStatic(const semantic::Program& program,
	semantic::BindingId object, semantic::BindingId function,
	std::uint32_t discriminator, bool guard, abi_mangle::AbiMangleStats* stats,
	abi_mangle::AbiMangleContext* context, LocalNames* names)
{
	using namespace abi_mangle;
	using namespace semantic;
	std::unique_ptr<AbiMangleContext> local_context;
	if (!context)
	{
		local_context.reset(new AbiMangleContext(stats));
		context = local_context.get();
	}
	AbiTypedCase fact_case;
	AbiFactBuilder facts(program, fact_case, context, stats, names);
	AbiFactRecord target;
	target.set_kind(ABI_FACT_RECORD_TARGET);
	target.target.kind = guard ? ABI_TARGET_FACT_GUARD_VARIABLE :
		ABI_TARGET_FACT_VARIABLE;
	AbiFunctionTarget& local = target.target.function;
	local.kind = ABI_FUNCTION_TARGET_LOCAL;
	AbiFactBuilder::AssignLocalContext(&local, facts.AddLocalContext(function));
	local.source_name = program.names.Get(VariableAbiName(program.bindings[object]));
	local.resolved_path = discriminator;
	AppendTypedFact(&fact_case, &target);
	return MangleProductionCase(fact_case, stats, context);
}

}  // namespace abi
}  // namespace lowering
}  // namespace cppgm
