#pragma once

#include <cstdint>
#include <vector>

#include "sema/semantic_graph.h"
#include "support/flat_map.h"
#include "support/memory/arena.h"
#include "support/memory/zeroed_allocator.h"
#include "support/telemetry.h"

namespace sema {

// Which declarations a lookup considers.
enum class LookupFilter : uint8_t
{
	// Every name; in one scope a variable, function or enumerator hides a
	// class or enumeration name (3.3.10p2).
	Ordinary,
	// Class and enumeration names, and typedef-names so the caller can
	// reject them; other names are ignored (3.4.4p2).
	Elaborated,
	// A name before `::`: namespaces and types (3.4.3p1).
	Prefix,
	// Namespace names only (3.4.6).
	Namespace
};

const uint32_t kSeveralVirtualBases = 0xffffffffu;

struct LookupResult
{
	Binding* binding;
	// The visible overload set when `binding` is a function.
	FunctionList* functions;
	// Namespaces searched together contributed different entities
	// (7.3.4p6, 3.4.3.2p2).
	bool ambiguous;
	// The ambiguity is only between injected-class-names of
	// specializations of one class template found in several bases: used
	// as a template-name, the name is that template (14.6.1p4).
	bool injected_template;
	// A member lookup's: every subobject the declarations were found in is
	// (within) a virtual base subobject of the class searched, so it may be
	// a base class subobject of another base's subobject (10.2p6); one
	// reached through non-virtual bases only is a distinct subobject.
	bool shared;
	// A member lookup's: declarations of one entity in different classes
	// (a using-declaration's and another, 7.3.3p1) were merged, whose
	// accesses may differ (see Analyzer::accessible_declarations).
	bool redeclared;
	// A member lookup's: the virtual base subobject of the class searched
	// that the subobjects found within one are in (the last virtual base
	// on their paths), its scope's id + 1; 0 if none is within one,
	// kSeveralVirtualBases if they are in different ones.
	uint32_t virtual_base;

	bool found() const { return binding != nullptr; }
};

// The names of one translation unit: a flat open-addressed table from
// (scope, identifier) to the NameEntry of that name in that scope, and the
// lookup rules over it. Unqualified lookup walks the scope parents; the
// namespaces nominated by using-directives met on the way are searched at
// the nearest namespace enclosing both the directive and the nominee
// (7.3.4p2), transitively (7.3.4p4). Namespace and block lookups are not
// cached, so they see exactly the declarations and directives made before
// them; member lookups in complete classes are (see MemberSlot).
class NameLookup
{
public:
	explicit NameLookup(Arena& arena);

	// Sizes the name table for about `names` (scope, name) pairs.
	void reserve(uint32_t names);

	// The entry of `name` in `scope`, created empty if absent. The pointer
	// is valid until the next entry() call.
	NameEntry* entry(const Scope* scope, const Identifier* name);
	const NameEntry* find(const Scope* scope, const Identifier* name) const;
	// Registers a binding in the entry of its name in `scope`. A function
	// only a friend declaration declared (invisible, 7.3.1.2p3) joins the
	// entry's overload set at its first visible declaration.
	void declare(const Scope* scope, Binding* binding);
	// Records an invisible friend function's declaration in the class of
	// scope `befriending`, through which argument-dependent lookup finds
	// it (3.4.2p4), and the functions so recorded for a class and name,
	// newest first.
	void befriend(const Scope* befriending, Binding* binding);
	const FunctionList* friends(const Scope* befriending, const Identifier* name) const;
	// Drops a function from the overload set of `name` in `scope` (a base
	// member hidden by a member of the same signature, 7.3.3p15).
	void remove_function(const Scope* scope, const Identifier* name, const Entity* function);
	// The function of `name` declared or introduced in `scope` with this
	// signature (FunctionEntity::signature); with a template head's shape
	// (TemplateHead::shape), the function template declared there with this
	// canonical function type and an equivalent template parameter list
	// (14.5.6.1p4: an equivalent declaration redeclares it; templates and
	// non-templates are indexed apart).
	const FunctionList* overload(
		const Scope* scope, const Identifier* name, const Type* signature, const Type* shape = nullptr) const;

	LookupResult unqualified(Scope* scope, const Identifier* name, LookupFilter filter);
	// Lookup in a namespace, class or scoped enumeration named by a
	// nested-name-specifier.
	LookupResult qualified(Scope* scope, const Identifier* name, LookupFilter filter);
	// The scope's own declarations only; `declared` whatever the limit (a
	// redeclaration's search for the entity it declares again, whose
	// earlier declarations an instantiation's copy repeats).
	LookupResult own(const Scope* scope, const Identifier* name, LookupFilter filter) const;
	LookupResult declared(const Scope* scope, const Identifier* name, LookupFilter filter);
	// Type aliases may be distinct declarations designating one canonical
	// type, or namespace aliases one namespace. Template names stay distinct.
	static bool designates_one(const Binding* a, const Binding* b);
	// A class scope as unqualified lookup searches it: its members and those
	// of its bases that do not depend on a template parameter (14.6.2p3).
	LookupResult members(Scope* scope, const Identifier* name, LookupFilter filter);

	// The nearest namespace that encloses both (7.3.4p2).
	static Scope* common_namespace(Scope* a, Scope* b);

	// The declaration ordinal after which namespace-scope declarations and
	// using-directives are not found by unqualified and qualified lookups
	// (14.6.4.2: a template definition's context, while an instantiation
	// analyzes it); kNoLimit otherwise. Argument-dependent lookup reads
	// entries through `functions` (limited for a call bound in the
	// definition, 14.6p10).
	static const uint32_t kNoLimit = 0xffffffffu;
	uint32_t limit() const { return limit_; }
	void set_limit(uint32_t limit) { limit_ = limit; }
	// The overload set of a namespace entry as the limit sees it (all of it
	// without one).
	FunctionList* functions(const NameEntry& entry) const;
	// A count of the unqualified lookups that found a template parameter, a
	// member of a templated or local class, or a dependent binding (see
	// Binding::dependent), and of other such uses the analysis notes (a
	// `this`): an expression whose analysis in an instantiation leaves it
	// unchanged names nothing whose type depends on the template's
	// parameters, so it is not type-dependent in its template (14.6.2.2;
	// conservatively: any other is taken as dependent). An instantiation
	// restores it when it ends, so the work it nests does not count.
	uint64_t dependent_uses() const { return dependent_uses_; }
	void set_dependent_uses(uint64_t uses) { dependent_uses_ = uses; }
	void note_dependent_use() { ++dependent_uses_; }
	// A count of the unqualified lookups that found a non-type template
	// parameter an instantiation frame binds (an enumerator there, see
	// dependent_uses): a template argument naming one makes its
	// template-id a dependent type in its template (14.6.2.1p7).
	uint64_t value_parameter_uses() const { return value_parameter_uses_; }

	telemetry::Counter lookups() const { return lookups_; }
	telemetry::Counter limited_lookups() const { return limited_lookups_; }
	uint32_t names() const { return count_; }
	uint32_t cached_member_lookups() const { return member_count_; }

private:
	LookupResult limited(const NameEntry& entry, LookupFilter filter) const;
	FunctionList* declared_by_limit(const NameEntry& entry) const;
	const NamespaceList* directives(const Scope* scope) const;
	// Open-addressed slots index the dense entry array, so growth moves
	// only 16-byte slots.
	struct Slot
	{
		uint64_t key;
		uint32_t entry;
	};

	// A nominated namespace waiting for the enclosing namespace where its
	// names appear (at that namespace's depth: all lie on the lookup's way
	// out, one per depth), in a max-heap by depth.
	struct Pending
	{
		Scope* nominated;
		uint32_t depth;
	};

	// The overload index: (scope, name, signature) to the function's node;
	// function templates apart, by (scope, name, signature with the return
	// type, template head shape).
	struct OverloadSlot
	{
		const Scope* scope;
		const Identifier* name;
		const Type* signature;
		FunctionList* node;
	};
	struct TemplateOverloadKey
	{
		const Scope* scope;
		const Identifier* name;
		const Type* type;
		const Type* shape;
		bool operator==(const TemplateOverloadKey& other) const
		{
			return scope == other.scope && name == other.name && type == other.type && shape == other.shape;
		}
	};
	struct TemplateOverloadHash
	{
		size_t operator()(const TemplateOverloadKey& key) const;
	};

	// Member lookup in a complete class, bases included (10.2): a complete
	// class's bases no longer change, so each (class, name, filter) result
	// is kept for the unit, up to four per declared name. Its members still
	// can: an out-of-class
	// definition redeclares one, and an unscoped member enumeration defined
	// outside adds enumerators (7.2p4). A declaration in a complete class
	// scope advances its name's epoch, and a result is valid only for the
	// epoch it was computed in.
	struct MemberSlot
	{
		const Scope* scope;
		const Identifier* name;
		LookupFilter filter;
		uint32_t epoch;
		LookupResult result;
	};

	// A long overload set's nodes in declaration order (oldest first), for
	// finding the suffix a limit sees by binary search: the set grows only
	// at its head, and its functions' ordinals grow with it (see
	// NameLookup::declare), so the ones declared by a limit are the
	// suffix from the last node at or before it. Extended from the head
	// when a lookup finds the set longer.
	struct Checkpoints
	{
		FunctionList** nodes;
		uint32_t count;
		uint32_t capacity;
	};
	struct KeyHash
	{
		size_t operator()(uint64_t key) const { return static_cast<size_t>(key); }
	};

	// One class of the iterative base-class search, entered through a
	// virtual base-specifier or not.
	struct BaseFrame
	{
		const Scope* scope;
		const BaseClass* next;
		LookupResult found;
		bool via_virtual;
	};

	static uint64_t key_of(const Scope* scope, const Identifier* name);
	void grow();
	OverloadSlot* overload_slot(const Scope* scope, const Identifier* name, const Type* signature, bool insert);
	bool member_name(const Identifier* name) const;
	uint32_t member_epoch(const Identifier* name) const;
	void invalidate_members(const Scope* scope, const Identifier* name);
	size_t member_limit() const;
	MemberSlot* member_slot(const Scope* scope, const Identifier* name, LookupFilter filter, bool insert);
	const MemberSlot* cached_members(const Scope* scope, const Identifier* name, LookupFilter filter);
	bool derives_from(const Scope* derived, const Scope* base);
	bool within_virtual_bases(const LookupResult& found, const Scope* cls);
	void merge_base(LookupResult& result, const LookupResult& found, const Scope* base, bool via_virtual);
	LookupResult class_members(Scope* scope, const Identifier* name, LookupFilter filter, bool unqualified);
	LookupResult class_members_without_dependent_bases(Scope* scope, const Identifier* name, LookupFilter filter);
	static bool shallower(const Pending& a, const Pending& b);
	void nominate(Scope* origin, Scope* nominated);
	bool merge(LookupResult& result, const LookupResult& found);
	FunctionList* function_union(FunctionList* first, FunctionList* second);
	LookupResult namespace_members(Scope* scope, const Identifier* name, LookupFilter filter);
	void search_inline_set(Scope* scope, const Identifier* name, LookupFilter filter, LookupResult& result);

	Arena& arena_;
	std::vector<Slot, ZeroedAllocator<Slot>> slots_;
	// slots_.size() - 1, the probes' index mask.
	uint64_t slot_mask_ = 1023;
	std::vector<NameEntry> entries_;
	uint32_t count_;
	uint32_t visit_;
	telemetry::Counter lookups_;
	mutable telemetry::Counter limited_lookups_ = 0;
	uint64_t dependent_uses_ = 0;
	uint64_t value_parameter_uses_ = 0;
	uint32_t limit_ = kNoLimit;
	// By entry index, the checkpoints of the sets limited lookups found
	// long (see Checkpoints); by (class scope, name), the invisible friend
	// functions a class declared.
	mutable FlatMap<uint32_t, Checkpoints, KeyHash> checkpoints_;
	mutable std::vector<FunctionList*> newer_;
	FlatMap<uint64_t, FunctionList*, KeyHash> friends_;
	std::vector<Pending> pending_;
	std::vector<Scope*> frontier_;
	// The nominated namespaces of qualified lookup's current and next level.
	std::vector<Scope*> level_;
	std::vector<Scope*> next_level_;
	std::vector<OverloadSlot, ZeroedAllocator<OverloadSlot>> overloads_;
	uint32_t overload_count_;
	FlatMap<TemplateOverloadKey, FunctionList*, TemplateOverloadHash> template_overloads_;
	std::vector<MemberSlot, ZeroedAllocator<MemberSlot>> members_;
	uint32_t member_count_;
	// Indexed by Identifier::id (and possibly shorter): the names declared
	// in some class scope, the only ones a search of base classes can find,
	// and each name's epoch (0 until declared in a complete class).
	std::vector<bool> member_names_;
	std::vector<uint32_t> member_epochs_;
	// The names the cache has held a result for.
	std::vector<bool> cached_names_;
	std::vector<BaseFrame> frames_;
	std::vector<const Scope*> base_stack_;
	uint32_t base_visit_;
	// The mark of the function set being united (see function_union).
	uint32_t union_visit_;
};

} // namespace sema
