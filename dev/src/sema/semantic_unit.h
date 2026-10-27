#pragma once

#include <cstdint>
#include <vector>

#include "sema/lookup.h"
#include "sema/semantic_graph.h"
#include "sema/type_table.h"
#include "support/memory/arena.h"
#include "support/pointer_map.h"
#include "support/telemetry.h"

namespace sema {

// The semantic graph of one translation unit and the storage it lives in:
// records in the arena, types interned in `types`, names indexed by
// `names`. Everything is released with the graph.
struct SemanticGraph
{
	SemanticGraph();

	Arena arena;
	TypeTable types;
	NameLookup names;
	Scope* global;
	// Semantics mode: the functions namespace-scope code odr-uses (the
	// roots of the demand graph), and the unnamed classes named so far.
	DemandList* root_demands;
	DemandList* last_root_demand;
	uint32_t local_types;
	uint32_t scopes;
	uint32_t bindings;
	telemetry::SmallCounter entities;
	// Constant evaluations (PA16: the interpreter's evaluations and
	// initializations, the integral evaluator's expressions) and the
	// interpreter's steps (its budget's unit: expressions, statements and
	// value elements).
	telemetry::Counter constant_evaluations;
	telemetry::Counter constant_steps;
	telemetry::Counter constant_call_builds = 0;
	telemetry::Counter constant_call_reuses = 0;
	telemetry::Counter function_order_builds = 0;
	telemetry::Counter function_order_reuses = 0;
	// Semantics-mode work: expressions typed, conversions recorded,
	// overload resolutions and the candidates they considered, function
	// template specializations declared, demand edges recorded.
	telemetry::Counter expressions;
	telemetry::Counter conversions;
	telemetry::Counter resolutions;
	telemetry::Counter candidates;
	telemetry::Counter specializations;
	telemetry::Counter demands;
	// Virtual functions: the base classes overriding searches probed, the
	// virtual-table slots built.
	telemetry::Counter override_steps;
	telemetry::Counter virtual_slots;
	// Templates: class definitions, function definitions, default arguments
	// and static data member definitions instantiated (specializations
	// declared are counted above).
	telemetry::Counter instantiated_classes;
	telemetry::Counter instantiated_functions;
	telemetry::Counter instantiated_arguments;
	telemetry::Counter instantiated_variables;
	// Metaprogramming: value-dependent template arguments of
	// substituted types evaluated (instantiated bodies evaluate theirs in
	// the frame), and partial specializations tried for a specialization
	// (after the index's filter).
	telemetry::Counter value_substitutions;
	telemetry::Counter partial_candidates;
	// Deduction and SFINAE: substitutions whose failure was their
	// result (a candidate dropped, 14.8.2p8). NameLookup counts the
	// lookups an instantiation's definition context limited (14.6.4.2).
	telemetry::Counter substitution_failures;
	// Multiple inheritance: the classes its base walks stepped
	// through (base path tables built, base path counts, access path
	// choices, the interpreter's base offset searches).
	telemetry::Counter base_steps;
	// Classes whose destructor is not trivial: none, and no expression
	// makes a temporary that needs destroying.
	uint32_t nontrivial_destructors;
	// Closure types (5.1.2p3): each one's facts and function call operator
	// (null for another class), kept beside the classes, whose records
	// carry no closure facts; a unit without lambdas looks up nothing.
	const ClosureFacts* closure_facts(const ClassEntity* entity) const
	{
		if (closures.empty())
			return nullptr;
		const uint32_t index = closure_index.find(entity);
		return index == PointerMap::kMissing ? nullptr : closures[index];
	}
	const FunctionEntity* closure_call(const ClassEntity* entity) const
	{
		const ClosureFacts* facts = closure_facts(entity);
		return facts != nullptr ? facts->call : nullptr;
	}
	PointerMap closure_index;
	std::vector<ClosureFacts*> closures;
	// The constructors (not trivial ones) a namespace-scope or static data
	// member initializer names, which the variable's constant initialization
	// may run uncalled: the lowering emits them with the variable (the
	// course's form). By initializer, a chain from the latest; a repeat of
	// the latest is not noted again.
	struct InitializerConstructor
	{
		const FunctionEntity* constructor;
		uint32_t next;
	};
	void note_initializer_constructor(const syntax::Initializer* initializer, const FunctionEntity* constructor)
	{
		const uint32_t head = initializer_constructor_heads.find(initializer);
		if (head != PointerMap::kMissing && initializer_constructors[head].constructor == constructor)
			return;
		initializer_constructor_heads.set(initializer, static_cast<uint32_t>(initializer_constructors.size()));
		initializer_constructors.push_back(InitializerConstructor{ constructor, head });
	}
	PointerMap initializer_constructor_heads;
	std::vector<InitializerConstructor> initializer_constructors;
	// GNU asm labels: the symbol name replacing a function's or variable's
	// object name, kept beside the entities, whose records carry none (a
	// unit without asm labels looks up nothing).
	const char* asm_label(const Entity* entity) const
	{
		if (asm_labels.empty())
			return nullptr;
		const uint32_t index = asm_label_index.find(entity);
		return index == PointerMap::kMissing ? nullptr : asm_labels[index];
	}
	void set_asm_label(const Entity* entity, const char* label)
	{
		asm_label_index.set(entity, static_cast<uint32_t>(asm_labels.size()));
		asm_labels.push_back(label);
	}
	PointerMap asm_label_index;
	std::vector<const char*> asm_labels;
	// Structured binding declarations (C++17's 11.5): each one's record,
	// kept beside its unnamed object e as asm labels are (e's Itanium name
	// lists the names, 5.1.4).
	const Decomposition* decomposition(const Entity* object) const
	{
		if (decompositions.empty())
			return nullptr;
		const uint32_t index = decomposition_index.find(object);
		return index == PointerMap::kMissing ? nullptr : decompositions[index];
	}
	void set_decomposition(const Entity* object, const Decomposition* record)
	{
		decomposition_index.set(object, static_cast<uint32_t>(decompositions.size()));
		decompositions.push_back(record);
	}
	PointerMap decomposition_index;
	std::vector<const Decomposition*> decompositions;
	// GNU weak and section attributes of functions and variables (pa27),
	// kept beside the entities as asm labels are.
	struct SymbolAttributes
	{
		const char* section; // an object's ELF section, or null
		bool weak;
	};
	const SymbolAttributes* symbol_attributes(const Entity* entity) const
	{
		if (symbol_attribute_list.empty())
			return nullptr;
		const uint32_t index = symbol_attribute_index.find(entity);
		return index == PointerMap::kMissing ? nullptr : &symbol_attribute_list[index];
	}
	SymbolAttributes& own_symbol_attributes(const Entity* entity)
	{
		const uint32_t index = symbol_attribute_index.find(entity);
		if (index != PointerMap::kMissing)
			return symbol_attribute_list[index];
		symbol_attribute_index.set(entity, static_cast<uint32_t>(symbol_attribute_list.size()));
		symbol_attribute_list.push_back(SymbolAttributes{ nullptr, false });
		return symbol_attribute_list.back();
	}
	PointerMap symbol_attribute_index;
	std::vector<SymbolAttributes> symbol_attribute_list;
	// GNU abi_tag (pa28): the tags, sorted and unique, of a class, function
	// or variable, or of an inline namespace (keyed by its scope), kept
	// beside them as asm labels are. A unit without tags (has_abi_tags()
	// false) looks up nothing and derives no implicit tags.
	struct AbiTags
	{
		const char* const* tags;
		uint32_t count;
	};
	const AbiTags* abi_tags(const void* owner) const
	{
		if (abi_tag_list.empty())
			return nullptr;
		const uint32_t index = abi_tag_index.find(owner);
		return index == PointerMap::kMissing ? nullptr : &abi_tag_list[index];
	}
	void set_abi_tags(const void* owner, AbiTags tags)
	{
		const uint32_t index = abi_tag_index.find(owner);
		if (index != PointerMap::kMissing)
		{
			abi_tag_list[index] = tags;
			return;
		}
		abi_tag_index.set(owner, static_cast<uint32_t>(abi_tag_list.size()));
		abi_tag_list.push_back(tags);
	}
	bool has_abi_tags() const { return !abi_tag_list.empty(); }
	PointerMap abi_tag_index;
	std::vector<AbiTags> abi_tag_list;
	// A function's dynamic-exception-specification (15.4, `throw()`
	// included): its types as 15.4p2 adjusts them (a reference's kept),
	// kept beside the functions for the host objects' regions that call
	// the unexpected handler (15.5.2; pa28).
	struct ThrowSpecification
	{
		const Type* const* types;
		uint32_t count;
	};
	const ThrowSpecification* throw_specification(const Entity* function) const
	{
		if (throw_specification_list.empty())
			return nullptr;
		const uint32_t index = throw_specification_index.find(function);
		return index == PointerMap::kMissing ? nullptr : &throw_specification_list[index];
	}
	void set_throw_specification(const Entity* function, ThrowSpecification specification)
	{
		throw_specification_index.set(function, static_cast<uint32_t>(throw_specification_list.size()));
		throw_specification_list.push_back(specification);
	}
	PointerMap throw_specification_index;
	std::vector<ThrowSpecification> throw_specification_list;
	// An inheriting constructor's root (12.9): the constructor at the end
	// of its bases' inheriting constructors (one a class declared, or a
	// constructor template's specialization), recorded when it is
	// declared for one inheriting another inheriting constructor (else its
	// `inherited` is the root); null for another function.
	const FunctionEntity* inherited_root(const FunctionEntity* function) const
	{
		if (function->inherited == nullptr || inherited_root_list.empty())
			return function->inherited;
		const uint32_t index = inherited_root_index.find(function);
		return index == PointerMap::kMissing ? function->inherited : inherited_root_list[index];
	}
	void set_inherited_root(const FunctionEntity* function, const FunctionEntity* root)
	{
		inherited_root_index.set(function, static_cast<uint32_t>(inherited_root_list.size()));
		inherited_root_list.push_back(root);
	}
	PointerMap inherited_root_index;
	std::vector<const FunctionEntity*> inherited_root_list;
	// A block-scope static or thread_local variable's discriminator
	// (Itanium 5.1.7): the number of its function's earlier ones of its
	// name, kept beside the entities for those that have earlier ones.
	uint32_t local_discriminator(const Entity* entity) const
	{
		const uint32_t found = local_discriminators.find(entity);
		return found == PointerMap::kMissing ? 0 : found;
	}
	PointerMap local_discriminators;
	// An unnamed class or enumeration defined in a function body (a
	// closure type excepted): its position among the function's in
	// definition order, those a typedef-name later names included (the
	// Itanium <unnamed-type-name> number as GCC and Clang count it).
	uint32_t unnamed_local_ordinal(const Entity* entity) const { return unnamed_local_ordinals.find(entity); }
	PointerMap unnamed_local_ordinals;
	// The classes defined in function bodies other than a template
	// specialization's (closure types included), in the order their
	// definitions began; whether an unnamed class (or a member class of
	// one) defines a member function in its member-specification.
	std::vector<const ClassEntity*> local_classes;
	bool unnamed_member_definitions = false;
};

} // namespace sema
