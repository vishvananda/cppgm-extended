#include "sema/lookup.h"

#include <algorithm>

#include "lex/identifier_table.h"
#include "sema/template_records.h"

namespace sema {

namespace {

// A 64-bit mixer (a multiplicative hash, its high half folded down): every
// key bit reaches the low bits the tables index by, so names declared in
// many scopes spread out.
uint64_t Mix64(uint64_t key)
{
	key *= 0x9e3779b97f4a7c15ull;
	return key ^ (key >> 32);
}

bool IsNamespaceBinding(const Binding* binding)
{
	return binding->kind == BindingKind::Namespace || binding->kind == BindingKind::NamespaceAlias;
}

// The virtual base (LookupResult::virtual_base) of two sets of subobjects.
uint32_t UnitedVirtualBase(uint32_t a, uint32_t b)
{
	return a == 0 ? b : b == 0 || a == b ? a : kSeveralVirtualBases;
}

Binding* Visible(Binding* binding)
{
	return binding != nullptr && !binding->invisible ? binding : nullptr;
}

// A redeclaration of an entity already in an entry is visible since its
// first visible declaration (see Binding::ordinal): a friend declaration
// before it made the name visible to no lookup but argument-dependent
// lookup (7.3.1.2p3).
void KeepOrdinal(Binding* binding, const Binding* earlier)
{
	if (earlier != nullptr && !earlier->invisible && earlier->entity == binding->entity &&
		earlier->ordinal < binding->ordinal)
		binding->ordinal = earlier->ordinal;
}

Scope* NearestNamespace(Scope* scope)
{
	while (scope != nullptr && scope->kind != ScopeKind::Namespace)
		scope = scope->parent;
	return scope;
}

// A namespace scope's depth.
uint32_t Depth(const Scope* scope)
{
	return static_cast<const NamespaceEntity*>(scope->entity)->depth;
}

bool IsCompleteClass(const Scope* scope)
{
	return scope->entity != nullptr && scope->entity->kind == EntityKind::Class &&
		static_cast<const ClassEntity*>(scope->entity)->complete;
}

const BaseClass* BasesOf(const Scope* scope)
{
	return static_cast<const ClassEntity*>(scope->entity)->bases;
}

// A class scope whose members' types may depend on template parameters:
// a templated class's or an instance's (a member of the current
// instantiation), or a local class's, or one nested in them.
bool DependentClassScope(const Scope* scope)
{
	for (; scope != nullptr && scope->kind == ScopeKind::Class; scope = scope->parent)
	{
		if (scope->entity == nullptr || static_cast<const ClassEntity*>(scope->entity)->templ != nullptr)
			return true;
	}
	return scope != nullptr && scope->kind != ScopeKind::Namespace;
}

// A typedef-name of the searched scope and a same-typed one a
// using-directive nominates into it denote one type (the course's
// convergence rule; two nominated typedef-names stay ambiguous).
bool Converges(const Binding* own, const Binding* nominated)
{
	return own->kind == BindingKind::TypeAlias && nominated->kind == BindingKind::TypeAlias &&
		own->type->canonical == nominated->type->canonical;
}

// The type a member type declaration designates (10.2p3: a typedef-name,
// a class or enumeration name, an injected-class-name), or null: a
// template's name designates none.
const Type* DesignatedType(const Binding* binding)
{
	const Entity* entity = binding->entity;
	const Type* type = nullptr;
	if (binding->kind == BindingKind::TypeAlias)
		type = entity != nullptr && entity->kind == EntityKind::AliasTemplate ? nullptr : binding->type;
	else if (binding->kind == BindingKind::Type && entity != nullptr &&
		(entity->kind == EntityKind::Enum ||
			(entity->kind == EntityKind::Class &&
				(static_cast<const ClassEntity*>(entity)->templ == nullptr ||
					static_cast<const ClassEntity*>(entity)->templ->role != TemplateRole::Template))))
		type = entity->type;
	return type != nullptr && type->kind != TypeKind::Template && type->kind != TypeKind::TemplateTemplateParameter
		? type->canonical
		: nullptr;
}

// Two type names designating one type (a typedef-name is a synonym, 7.1.3p1)
// or two namespace names one namespace (7.3.2): found in namespaces a
// lookup searches together, they denote one entity (7.3.4p6).
bool DesignateOne(const Binding* a, const Binding* b)
{
	const bool type_a = a->kind == BindingKind::Type || a->kind == BindingKind::TypeAlias;
	const bool type_b = b->kind == BindingKind::Type || b->kind == BindingKind::TypeAlias;
	if (type_a || type_b)
	{
		const Type* designated = type_a && type_b ? DesignatedType(a) : nullptr;
		return designated != nullptr && designated == DesignatedType(b);
	}
	if (!IsNamespaceBinding(a) || !IsNamespaceBinding(b))
		return false;
	const Entity* first = a->kind == BindingKind::NamespaceAlias
		? static_cast<const NamespaceAliasEntity*>(a->entity)->target
		: a->entity;
	const Entity* second = b->kind == BindingKind::NamespaceAlias
		? static_cast<const NamespaceAliasEntity*>(b->entity)->target
		: b->entity;
	return first == second;
}

// Two injected-class-names (9p2) of specializations of one class template
// (see LookupResult::injected_template).
bool SameTemplateInjectedNames(const Binding* a, const Binding* b)
{
	const Binding* names[2] = { a, b };
	const ClassEntity* templates[2] = { nullptr, nullptr };
	for (int i = 0; i < 2; ++i)
	{
		const Entity* entity = names[i]->entity;
		if (entity == nullptr || entity->kind != EntityKind::Class ||
			names[i]->declared_in != static_cast<const ClassEntity*>(entity)->scope)
			return false;
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(entity)->templ;
		if (facts == nullptr || facts->role != TemplateRole::Specialization)
			return false;
		templates[i] = facts->pattern;
	}
	return templates[0] == templates[1];
}

} // namespace

NameLookup::NameLookup(Arena& arena)
	: arena_(arena), slots_(1024), count_(0), visit_(0), lookups_(0), overloads_(256), overload_count_(0),
	  members_(256), member_count_(0), base_visit_(0), union_visit_(0)
{}

uint64_t NameLookup::key_of(const Scope* scope, const Identifier* name)
{
	// Never 0, the empty slot's key.
	return (static_cast<uint64_t>(scope->id) + 1) << 32 | name->id;
}

void NameLookup::grow()
{
	std::vector<Slot, ZeroedAllocator<Slot>> old(slots_.size() * 2);
	old.swap(slots_);
	const uint64_t mask = slots_.size() - 1;
	slot_mask_ = mask;
	for (const Slot& slot : old)
	{
		if (slot.key == 0)
			continue;
		uint64_t index = Mix64(slot.key) & mask;
		while (slots_[index].key != 0)
			index = (index + 1) & mask;
		slots_[index] = slot;
	}
}

void NameLookup::reserve(uint32_t names)
{
	// At most half full; the entries grow on their own.
	while (slots_.size() < static_cast<size_t>(names) * 2)
		grow();
	entries_.reserve(names);
}

NameEntry* NameLookup::entry(const Scope* scope, const Identifier* name)
{
	if ((count_ + 1) * 2 > slots_.size())
		grow();
	const uint64_t key = key_of(scope, name);
	const uint64_t mask = slot_mask_;
	uint64_t index = Mix64(key) & mask;
	while (slots_[index].key != 0)
	{
		if (slots_[index].key == key)
			return &entries_[slots_[index].entry];
		index = (index + 1) & mask;
	}
	++count_;
	slots_[index].key = key;
	slots_[index].entry = static_cast<uint32_t>(entries_.size());
	entries_.push_back(NameEntry());
	return &entries_.back();
}

const NameEntry* NameLookup::find(const Scope* scope, const Identifier* name) const
{
	const uint64_t key = key_of(scope, name);
	const uint64_t mask = slot_mask_;
	uint64_t index = Mix64(key) & mask;
	while (slots_[index].key != 0)
	{
		if (slots_[index].key == key)
			return &entries_[slots_[index].entry];
		index = (index + 1) & mask;
	}
	return nullptr;
}

void NameLookup::declare(const Scope* scope, Binding* binding)
{
	if (scope->kind == ScopeKind::Class)
	{
		const uint32_t id = binding->name->id;
		if (id >= member_names_.size())
			member_names_.resize(std::max<size_t>(id + 1, member_names_.size() * 2));
		member_names_[id] = true;
	}
	invalidate_members(scope, binding->name);
	NameEntry* entry = this->entry(scope, binding->name);
	// An invisible (friend) declaration never replaces a visible one.
	switch (binding->kind)
	{
	case BindingKind::Type:
	case BindingKind::TypeAlias:
	case BindingKind::Namespace:
	case BindingKind::NamespaceAlias:
		KeepOrdinal(binding, entry->type);
		KeepOrdinal(binding, entry->tag);
		if (!binding->invisible || entry->type == nullptr)
			entry->type = binding;
		if (binding->kind == BindingKind::Type && binding->entity != nullptr &&
			(binding->entity->kind == EntityKind::Class || binding->entity->kind == EntityKind::Enum) &&
			(!binding->invisible || entry->tag == nullptr))
			entry->tag = binding;
		return;
	case BindingKind::Function:
		break;
	default:
		KeepOrdinal(binding, entry->value);
		if (!binding->invisible || entry->value == nullptr)
			entry->value = binding;
		return;
	}
	if (!binding->invisible || entry->value == nullptr)
		entry->value = binding;
	// A redeclaration updates its function's node, found through the
	// overload index; a new function joins the set. A function template is
	// indexed by its signature and template head's shape in its own scope
	// (one a using-declaration introduces just joins).
	const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
	FunctionList** slot = nullptr;
	const bool indexed_template = function->templated && function->owner == scope;
	const TemplateOverloadKey key = TemplateOverloadKey{ scope, binding->name, function->signature,
		indexed_template ? HeadOf(function)->shape : nullptr };
	if (!function->templated)
	{
		OverloadSlot* found = overload_slot(scope, binding->name, function->signature, true);
		slot = &found->node;
	}
	else if (indexed_template)
		slot = template_overloads_.find(key);
	// A function only friend declarations declared has a node the overload
	// index finds but no set holds (argument-dependent lookup finds it
	// through its befriending classes, see befriend); its first visible
	// declaration puts it at the set's head, so a set's ordinals still grow
	// towards its head.
	if (slot != nullptr && *slot != nullptr && (*slot)->binding->entity == function)
	{
		FunctionList* node = *slot;
		if (binding->invisible)
			return;
		if (node->binding->invisible)
		{
			node->binding = binding;
			node->next = entry->functions;
			entry->functions = node;
			return;
		}
		KeepOrdinal(binding, node->binding);
		node->binding = binding;
		return;
	}
	FunctionList* node = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
	node->binding = binding;
	node->next = nullptr;
	if (!binding->invisible)
	{
		node->next = entry->functions;
		entry->functions = node;
	}
	if (slot != nullptr)
		*slot = node;
	else if (indexed_template)
		template_overloads_.insert(key, node);
}

void NameLookup::befriend(const Scope* befriending, Binding* binding)
{
	FunctionList* node = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
	const uint64_t key = key_of(befriending, binding->name);
	FunctionList** head = friends_.find(key);
	*node = FunctionList{ binding, head != nullptr ? *head : nullptr };
	if (head != nullptr)
		*head = node;
	else
		friends_.insert(key, node);
}

const FunctionList* NameLookup::friends(const Scope* befriending, const Identifier* name) const
{
	FunctionList* const* head = const_cast<NameLookup*>(this)->friends_.find(key_of(befriending, name));
	return head != nullptr ? *head : nullptr;
}

void NameLookup::remove_function(const Scope* scope, const Identifier* name, const Entity* function)
{
	invalidate_members(scope, name);
	NameEntry* entry = this->entry(scope, name);
	for (FunctionList** link = &entry->functions; *link != nullptr; link = &(*link)->next)
	{
		if ((*link)->binding->entity == function)
		{
			*link = (*link)->next;
			break;
		}
	}
	if (entry->value != nullptr && entry->value->entity == function)
		entry->value = entry->functions != nullptr ? entry->functions->binding : nullptr;
	const FunctionEntity* removed = static_cast<const FunctionEntity*>(function);
	if (OverloadSlot* slot = overload_slot(scope, name, removed->signature, false))
		slot->node = nullptr;
}

size_t NameLookup::TemplateOverloadHash::operator()(const TemplateOverloadKey& key) const
{
	return static_cast<size_t>(Mix64(key_of(key.scope, key.name) ^
		Mix64(reinterpret_cast<uintptr_t>(key.type) + 31 * reinterpret_cast<uintptr_t>(key.shape))));
}

NameLookup::OverloadSlot* NameLookup::overload_slot(
	const Scope* scope, const Identifier* name, const Type* signature, bool insert)
{
	if (insert && (overload_count_ + 1) * 2 > overloads_.size())
	{
		std::vector<OverloadSlot, ZeroedAllocator<OverloadSlot>> old(overloads_.size() * 2);
		old.swap(overloads_);
		overload_count_ = 0;
		for (const OverloadSlot& slot : old)
		{
			if (slot.scope != nullptr)
				*overload_slot(slot.scope, slot.name, slot.signature, true) = slot;
		}
	}
	const uint64_t mask = overloads_.size() - 1;
	uint64_t index = Mix64(key_of(scope, name) ^ Mix64(reinterpret_cast<uintptr_t>(signature))) & mask;
	while (overloads_[index].scope != nullptr)
	{
		const OverloadSlot& slot = overloads_[index];
		if (slot.scope == scope && slot.name == name && slot.signature == signature)
			return &overloads_[index];
		index = (index + 1) & mask;
	}
	if (!insert)
		return nullptr;
	++overload_count_;
	overloads_[index] = OverloadSlot{ scope, name, signature, nullptr };
	return &overloads_[index];
}

const FunctionList* NameLookup::overload(
	const Scope* scope, const Identifier* name, const Type* signature, const Type* shape) const
{
	if (shape != nullptr)
	{
		FunctionList* const* found = const_cast<NameLookup*>(this)->template_overloads_.find(
			TemplateOverloadKey{ scope, name, signature, shape });
		return found != nullptr ? *found : nullptr;
	}
	const uint64_t mask = overloads_.size() - 1;
	uint64_t index = Mix64(key_of(scope, name) ^ Mix64(reinterpret_cast<uintptr_t>(signature))) & mask;
	while (overloads_[index].scope != nullptr)
	{
		const OverloadSlot& slot = overloads_[index];
		if (slot.scope == scope && slot.name == name && slot.signature == signature)
			return slot.node;
		index = (index + 1) & mask;
	}
	return nullptr;
}

LookupResult NameLookup::own(const Scope* scope, const Identifier* name, LookupFilter filter) const
{
	LookupResult result = LookupResult();
	const NameEntry* entry = find(scope, name);
	if (entry == nullptr)
		return result;
	if (limit_ != kNoLimit && scope->kind == ScopeKind::Namespace)
		return limited(*entry, filter);
	Binding* type = Visible(entry->type);
	switch (filter)
	{
	case LookupFilter::Ordinary:
		if (Binding* value = Visible(entry->value))
		{
			result.binding = value;
			if (value->kind == BindingKind::Function)
				result.functions = entry->functions;
		}
		else
			result.binding = type;
		break;
	case LookupFilter::Elaborated:
		if (Binding* tag = Visible(entry->tag))
			result.binding = tag;
		else if (type != nullptr && !IsNamespaceBinding(type))
			result.binding = type;
		break;
	case LookupFilter::Prefix:
		result.binding = type;
		break;
	case LookupFilter::Namespace:
		if (type != nullptr && IsNamespaceBinding(type))
			result.binding = type;
		break;
	}
	return result;
}

LookupResult NameLookup::declared(const Scope* scope, const Identifier* name, LookupFilter filter)
{
	const uint32_t limit = limit_;
	limit_ = kNoLimit;
	const LookupResult result = own(scope, name, filter);
	limit_ = limit;
	return result;
}

FunctionList* NameLookup::functions(const NameEntry& entry) const
{
	return limit_ == kNoLimit || entry.functions == nullptr || entry.functions->binding->ordinal <= limit_
		? entry.functions
		: declared_by_limit(entry);
}

FunctionList* NameLookup::declared_by_limit(const NameEntry& entry) const
{
	// The suffix of the set declared by the limit (a set grows at its head,
	// its ordinals with it): a few functions declared since are skipped,
	// past them the set's checkpoints are searched, so repeated lookups do
	// not walk a long run of later declarations (a template instantiated
	// many times after many overloads of a name it uses).
	FunctionList* functions = entry.functions;
	for (int skipped = 0; skipped < 8; ++skipped)
	{
		if (functions == nullptr || functions->binding->ordinal <= limit_)
			return functions;
		functions = functions->next;
	}
	const uint32_t index = static_cast<uint32_t>(&entry - entries_.data());
	Checkpoints* checkpoints = checkpoints_.find(index);
	if (checkpoints == nullptr)
	{
		checkpoints_.insert(index, Checkpoints{ nullptr, 0, 0 });
		checkpoints = checkpoints_.find(index);
	}
	// The nodes declared since the last extension, newest first.
	const FunctionList* newest = checkpoints->count != 0 ? checkpoints->nodes[checkpoints->count - 1] : nullptr;
	newer_.clear();
	for (FunctionList* node = entry.functions; node != newest; node = node->next)
		newer_.push_back(node);
	if (checkpoints->count + newer_.size() > checkpoints->capacity)
	{
		const size_t capacity = std::max<size_t>(16, (checkpoints->count + newer_.size()) * 2);
		FunctionList** nodes = arena_.allocate_array<FunctionList*>(capacity);
		std::copy(checkpoints->nodes, checkpoints->nodes + checkpoints->count, nodes);
		checkpoints->nodes = nodes;
		checkpoints->capacity = static_cast<uint32_t>(capacity);
	}
	for (size_t i = newer_.size(); i-- > 0;)
		checkpoints->nodes[checkpoints->count++] = newer_[i];
	// The last node declared by the limit.
	FunctionList** const begin = checkpoints->nodes;
	FunctionList** const end = begin + checkpoints->count;
	FunctionList** const after = std::upper_bound(
		begin, end, limit_, [](uint32_t limit, const FunctionList* node) { return limit < node->binding->ordinal; });
	return after != begin ? *(after - 1) : nullptr;
}

LookupResult NameLookup::limited(const NameEntry& entry, LookupFilter filter) const
{
	// A namespace entry as it was at the limit: a declaration made later is
	// not there (14.6.4.2); an overload set is its functions declared by
	// then (a set grows at its head, so they are a suffix).
	++limited_lookups_;
	LookupResult result = LookupResult();
	Binding* type = Visible(entry.type);
	if (type != nullptr && type->ordinal > limit_)
		type = nullptr;
	switch (filter)
	{
	case LookupFilter::Ordinary:
	{
		Binding* value = Visible(entry.value);
		if (value != nullptr && value->kind == BindingKind::Function)
		{
			FunctionList* functions = this->functions(entry);
			if (functions != nullptr)
			{
				result.binding = functions->binding;
				result.functions = functions;
				return result;
			}
			value = nullptr;
		}
		result.binding = value != nullptr && value->ordinal <= limit_ ? value : type;
		break;
	}
	case LookupFilter::Elaborated:
	{
		Binding* tag = Visible(entry.tag);
		if (tag != nullptr && tag->ordinal <= limit_)
			result.binding = tag;
		else if (type != nullptr && !IsNamespaceBinding(type))
			result.binding = type;
		break;
	}
	case LookupFilter::Prefix:
		result.binding = type;
		break;
	case LookupFilter::Namespace:
		if (type != nullptr && IsNamespaceBinding(type))
			result.binding = type;
		break;
	}
	return result;
}

bool NameLookup::merge(LookupResult& result, const LookupResult& found)
{
	if (!found.found())
		return false;
	if (!result.found())
	{
		result = found;
		return true;
	}
	// 7.3.4p2, 3.4.3.2p2: the functions of the name in the namespaces
	// searched together form one overload set.
	if (result.binding->kind == BindingKind::Function && found.binding->kind == BindingKind::Function)
	{
		result.functions = function_union(result.functions, found.functions);
		return true;
	}
	if (result.binding->entity == found.binding->entity || DesignateOne(result.binding, found.binding))
		return true;
	result.ambiguous = true;
	result.injected_template = false;
	return true;
}

FunctionList* NameLookup::function_union(FunctionList* first, FunctionList* second)
{
	// The functions of the smaller set that the larger lacks, each in a
	// new node, ahead of the larger (shared: a namespace's list only grows
	// at its head, so a recorded set never changes); each function once. A
	// merging lookup allocates at most the smaller set and writes only its
	// functions' marks: the larger set's are read.
	if (first == nullptr || first == second)
		return second != nullptr ? second : first;
	// The list that ends first, walking both only that far.
	const FunctionList* a = first;
	const FunctionList* b = second;
	while (a != nullptr && b != nullptr)
	{
		a = a->next;
		b = b->next;
	}
	FunctionList* larger = a != nullptr ? first : second;
	FunctionList* smaller = larger == first ? second : first;
	const uint32_t pending = ++union_visit_;
	const uint32_t present = ++union_visit_;
	for (const FunctionList* node = smaller; node != nullptr; node = node->next)
		static_cast<FunctionEntity*>(node->binding->entity)->lookup_mark = pending;
	for (const FunctionList* node = larger; node != nullptr; node = node->next)
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
		if (function->lookup_mark == pending)
			function->lookup_mark = present;
	}
	FunctionList* head = larger;
	for (const FunctionList* node = smaller; node != nullptr; node = node->next)
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
		if (function->lookup_mark != pending)
			continue;
		function->lookup_mark = present;
		FunctionList* added = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
		*added = FunctionList{ node->binding, head };
		head = added;
	}
	return head;
}

bool NameLookup::derives_from(const Scope* derived, const Scope* base)
{
	// The class of scope `derived` has the class of scope `base` as a
	// (direct or indirect) base. Each class of the base graph is visited
	// once (a lattice of shared bases has exponentially many paths), with
	// an explicit stack for long chains.
	if (derived == nullptr || base == nullptr)
		return false;
	const uint32_t visit = ++base_visit_;
	base_stack_.assign(1, derived);
	while (!base_stack_.empty())
	{
		const Scope* scope = base_stack_.back();
		base_stack_.pop_back();
		if (scope->entity == nullptr || scope->entity->kind != EntityKind::Class)
			continue;
		for (const BaseClass* candidate = BasesOf(scope); candidate != nullptr; candidate = candidate->next)
		{
			Scope* candidate_scope = static_cast<const ClassEntity*>(candidate->base)->scope;
			if (candidate_scope == base)
				return true;
			if (candidate_scope->base_visit == visit)
				continue;
			candidate_scope->base_visit = visit;
			base_stack_.push_back(candidate_scope);
		}
	}
	return false;
}

bool NameLookup::within_virtual_bases(const LookupResult& found, const Scope* cls)
{
	// Whether every subobject a base's result was found in is a base class
	// subobject of each subobject of class `cls` (10.2p6): within a virtual
	// base of the class searched that is cls itself or a virtual base of
	// cls, which every subobject of cls shares (10.1p4). The classes below
	// cls are visited once, as derives_from visits them. Found in several
	// virtual bases, a result's subobjects are taken to be shared.
	if (!found.shared)
		return false;
	if (found.virtual_base == kSeveralVirtualBases || cls->id + 1 == found.virtual_base)
		return true;
	const uint32_t visit = ++base_visit_;
	base_stack_.assign(1, cls);
	while (!base_stack_.empty())
	{
		const Scope* scope = base_stack_.back();
		base_stack_.pop_back();
		if (scope->entity == nullptr || scope->entity->kind != EntityKind::Class)
			continue;
		for (const BaseClass* candidate = BasesOf(scope); candidate != nullptr; candidate = candidate->next)
		{
			Scope* candidate_scope = static_cast<const ClassEntity*>(candidate->base)->scope;
			if (candidate->is_virtual && candidate_scope->id + 1 == found.virtual_base)
				return true;
			if (candidate_scope->base_visit == visit)
				continue;
			candidate_scope->base_visit = visit;
			base_stack_.push_back(candidate_scope);
		}
	}
	return false;
}

void NameLookup::merge_base(LookupResult& result, const LookupResult& found, const Scope* base, bool via_virtual)
{
	// 10.2p6: a base's lookup set (found through a virtual base-specifier
	// or not) merged into the class's. One member reached through several
	// paths is found once (redeclared when through declarations in
	// different classes, 10.2p3). A declaration in a class derived from
	// another's class dominates it (10.2p10) only where the other's
	// subobjects are base class subobjects of its: shared, within virtual
	// bases it has (within_virtual_bases); a member of a non-virtual base's
	// subobject reached through another path is not hidden (10.2p10's
	// note), and different declarations are ambiguous.
	if (!found.found())
		return;
	const bool shared = found.shared || via_virtual;
	// Through a virtual base-specifier the subobjects within no virtual
	// base are within `base`'s.
	const uint32_t virtual_base = via_virtual && !found.shared ? UnitedVirtualBase(found.virtual_base, base->id + 1)
															   : found.virtual_base;
	if (!result.found())
	{
		result = found;
		result.shared = shared;
		result.virtual_base = virtual_base;
		return;
	}
	if (found.binding->entity == result.binding->entity)
	{
		if (found.ambiguous)
			result.injected_template = (!result.ambiguous || result.injected_template) && found.injected_template;
		result.ambiguous = result.ambiguous || found.ambiguous;
		result.shared = result.shared && shared;
		result.virtual_base = UnitedVirtualBase(result.virtual_base, virtual_base);
		result.redeclared = result.redeclared || found.redeclared || found.binding != result.binding;
		return;
	}
	const Scope* found_class = found.binding->entity->owner;
	const Scope* result_class = result.binding->entity->owner;
	LookupResult merged = found;
	merged.shared = shared;
	merged.virtual_base = virtual_base;
	if (result.shared && derives_from(found_class, result_class) && within_virtual_bases(result, found_class))
		result = merged;
	else if (!shared || !derives_from(result_class, found_class) || !within_virtual_bases(merged, result_class))
	{
		// 10.2p3: declarations of types stand for the types designated, so
		// two naming one type make equal sets (p6), the first kept as the
		// representative.
		const Type* designated = DesignatedType(result.binding);
		if (designated != nullptr && designated == DesignatedType(found.binding) && !result.ambiguous &&
			!found.ambiguous)
		{
			result.shared = result.shared && shared;
			result.virtual_base = UnitedVirtualBase(result.virtual_base, virtual_base);
			return;
		}
		result.injected_template = (!result.ambiguous || result.injected_template) &&
			SameTemplateInjectedNames(result.binding, found.binding);
		result.ambiguous = true;
		result.shared = result.shared && shared;
		result.virtual_base = UnitedVirtualBase(result.virtual_base, virtual_base);
	}
}

LookupResult NameLookup::class_members(Scope* scope, const Identifier* name, LookupFilter filter, bool unqualified)
{
	LookupResult result = own(scope, name, filter);
	if (result.found() || scope->entity == nullptr || scope->entity->kind != EntityKind::Class || !member_name(name))
		return result;
	// 14.6.2p3: unqualified lookup from a class does not search its bases
	// that depend on a template parameter (nor its instantiations' copies
	// of them); the search without them is not the cached one.
	bool skips = false;
	for (const BaseClass* base = BasesOf(scope); base != nullptr && unqualified; base = base->next)
		skips = skips || base->dependent;
	if (skips)
		return class_members_without_dependent_bases(scope, name, filter);
	if (const MemberSlot* cached = cached_members(scope, name, filter))
		return cached->result;
	// The bases, depth-first with an explicit stack: each class whose own
	// declarations lack the name contributes its bases' results, which are
	// kept for complete classes (so each (class, name) is searched once).
	const size_t base_frame = frames_.size();
	frames_.push_back(BaseFrame{ scope, BasesOf(scope), LookupResult(), false });
	for (;;)
	{
		BaseFrame& frame = frames_.back();
		if (frame.next == nullptr)
		{
			const BaseFrame done = frame;
			frames_.pop_back();
			MemberSlot* slot = IsCompleteClass(done.scope) && member_count_ < member_limit()
				? member_slot(done.scope, name, filter, true)
				: nullptr;
			if (slot != nullptr)
			{
				slot->epoch = member_epoch(name);
				slot->result = done.found;
				if (name->id >= cached_names_.size())
					cached_names_.resize(std::max<size_t>(name->id + 1, cached_names_.size() * 2));
				cached_names_[name->id] = true;
			}
			if (frames_.size() == base_frame)
				return done.found;
			merge_base(frames_.back().found, done.found, done.scope, done.via_virtual);
			continue;
		}
		const Scope* base = static_cast<const ClassEntity*>(frame.next->base)->scope;
		const bool via_virtual = frame.next->is_virtual;
		frame.next = frame.next->next;
		if (base == nullptr)
			continue;
		const LookupResult found = own(base, name, filter);
		if (found.found())
		{
			merge_base(frame.found, found, base, via_virtual);
			continue;
		}
		if (const MemberSlot* cached = cached_members(base, name, filter))
		{
			merge_base(frames_.back().found, cached->result, base, via_virtual);
			continue;
		}
		frames_.push_back(BaseFrame{ base, BasesOf(base), LookupResult(), via_virtual });
	}
}

LookupResult NameLookup::class_members_without_dependent_bases(
	Scope* scope, const Identifier* name, LookupFilter filter)
{
	// The class's own members are not declared here (the caller looked);
	// its other direct bases are searched as qualified lookup searches
	// them, their results merged as 10.2 merges base results.
	LookupResult result = LookupResult();
	for (const BaseClass* base = BasesOf(scope); base != nullptr; base = base->next)
	{
		Scope* base_scope = static_cast<const ClassEntity*>(base->base)->scope;
		if (base->dependent || base_scope == nullptr)
			continue;
		merge_base(result, class_members(base_scope, name, filter, false), base_scope, base->is_virtual);
	}
	return result;
}

NameLookup::MemberSlot* NameLookup::member_slot(
	const Scope* scope, const Identifier* name, LookupFilter filter, bool insert)
{
	if (insert && (member_count_ + 1) * 2 > members_.size() && member_count_ < member_limit())
	{
		std::vector<MemberSlot, ZeroedAllocator<MemberSlot>> old(members_.size() * 2);
		old.swap(members_);
		member_count_ = 0;
		for (const MemberSlot& slot : old)
		{
			if (slot.scope != nullptr)
				*member_slot(slot.scope, slot.name, slot.filter, true) = slot;
		}
	}
	const uint64_t mask = members_.size() - 1;
	uint64_t index = Mix64(key_of(scope, name) * 4 + static_cast<unsigned>(filter)) & mask;
	while (members_[index].scope != nullptr)
	{
		const MemberSlot& slot = members_[index];
		if (slot.scope == scope && slot.name == name && slot.filter == filter)
			return &members_[index];
		index = (index + 1) & mask;
	}
	// The cache holds at most a few results per declared name: a search of
	// deep bases for many different names would otherwise keep one per
	// class per name. Past that, searches are repeated instead.
	if (!insert || member_count_ >= member_limit())
		return nullptr;
	++member_count_;
	members_[index] = MemberSlot{ scope, name, filter, 0, LookupResult() };
	return &members_[index];
}

size_t NameLookup::member_limit() const
{
	return std::max<size_t>(1u << 16, static_cast<size_t>(count_) * 4);
}

const NameLookup::MemberSlot* NameLookup::cached_members(
	const Scope* scope, const Identifier* name, LookupFilter filter)
{
	if (!IsCompleteClass(scope) || name->id >= cached_names_.size() || !cached_names_[name->id])
		return nullptr;
	const MemberSlot* slot = member_slot(scope, name, filter, false);
	return slot != nullptr && slot->epoch == member_epoch(name) ? slot : nullptr;
}

bool NameLookup::member_name(const Identifier* name) const
{
	return name->id < member_names_.size() && member_names_[name->id];
}

uint32_t NameLookup::member_epoch(const Identifier* name) const
{
	return name->id < member_epochs_.size() ? member_epochs_[name->id] : 0;
}

void NameLookup::invalidate_members(const Scope* scope, const Identifier* name)
{
	// Only a complete class's members are cached, as are the results of
	// the classes derived from it: all are keyed by the name.
	if (!IsCompleteClass(scope))
		return;
	if (name->id >= member_epochs_.size())
		member_epochs_.resize(std::max<size_t>(name->id + 1, member_epochs_.size() * 2));
	++member_epochs_[name->id];
}

Scope* NameLookup::common_namespace(Scope* a, Scope* b)
{
	// Namespace parents are namespaces: level the two, then climb together.
	a = NearestNamespace(a);
	b = NearestNamespace(b);
	uint32_t depth_a = Depth(a);
	uint32_t depth_b = Depth(b);
	for (; depth_a > depth_b; --depth_a)
		a = a->parent;
	for (; depth_b > depth_a; --depth_b)
		b = b->parent;
	while (a != b)
	{
		a = a->parent;
		b = b->parent;
	}
	return a;
}

bool NameLookup::shallower(const Pending& a, const Pending& b)
{
	return a.depth < b.depth;
}

const NamespaceList* NameLookup::directives(const Scope* scope) const
{
	// A namespace's using-directives written by the limit (newest first).
	const NamespaceList* directive = scope->directives;
	if (limit_ != kNoLimit && scope->kind == ScopeKind::Namespace)
	{
		while (directive != nullptr && directive->ordinal > limit_)
			directive = directive->next;
	}
	return directive;
}

void NameLookup::nominate(Scope* origin, Scope* nominated)
{
	// The nominee's own directives act as if written at the origin
	// (7.3.4p4); each namespace is taken once per lookup.
	frontier_.clear();
	frontier_.push_back(nominated);
	while (!frontier_.empty())
	{
		Scope* scope = frontier_.back();
		frontier_.pop_back();
		if (scope->visit == visit_)
			continue;
		scope->visit = visit_;
		pending_.push_back(Pending{ scope, Depth(common_namespace(origin, scope)) });
		std::push_heap(pending_.begin(), pending_.end(), shallower);
		for (const NamespaceList* directive = directives(scope); directive != nullptr; directive = directive->next)
			frontier_.push_back(directive->scope);
	}
}

LookupResult NameLookup::unqualified(Scope* scope, const Identifier* name, LookupFilter filter)
{
	++lookups_;
	++visit_;
	pending_.clear();
	// Past an out-of-class definition's template parameters (Scope::bridge)
	// into its class, and then the class's enclosing scopes: no template
	// parameter scope is searched there.
	bool bridged = false;
	bool outside = false;
	bool concrete_members = false;
	for (Scope* current = scope; current != nullptr; current = current->parent)
	{
		if (current->kind == ScopeKind::TemplateParameters)
		{
			concrete_members = concrete_members || current->concrete_members;
			if (outside)
				continue;
			bridged = bridged || current->bridge;
		}
		else
			outside = bridged;
		LookupResult result = current->kind == ScopeKind::Class
			? class_members(current, name, filter, !concrete_members)
			: own(current, name, filter);
		const Binding* declared_here = result.binding;
		for (const NamespaceList* directive = directives(current); directive != nullptr; directive = directive->next)
			nominate(current, directive->scope);
		if (current->kind == ScopeKind::Namespace)
		{
			// The nominees whose names appear here: this is the one
			// namespace of its depth on the way out.
			const uint32_t depth = Depth(current);
			while (!pending_.empty() && pending_.front().depth == depth)
			{
				std::pop_heap(pending_.begin(), pending_.end(), shallower);
				const Scope* nominated = pending_.back().nominated;
				pending_.pop_back();
				const LookupResult found = own(nominated, name, filter);
				if (declared_here != nullptr && found.found() && Converges(declared_here, found.binding))
					continue;
				merge(result, found);
			}
		}
		if (result.found())
		{
			// A dependent use (see dependent_uses): a template parameter
			// (an instantiation frame binds a non-type one's name as an
			// enumerator, dependent if its declared type is), a dependent
			// binding, a member of a templated or local class.
			if ((current->kind == ScopeKind::TemplateParameters && result.binding->kind != BindingKind::Enumerator) ||
				result.binding->dependent || (current->kind == ScopeKind::Class && DependentClassScope(current)))
				++dependent_uses_;
			else if (current->kind == ScopeKind::TemplateParameters)
				++value_parameter_uses_;
			return result;
		}
	}
	return LookupResult();
}

void NameLookup::search_inline_set(Scope* scope, const Identifier* name, LookupFilter filter, LookupResult& result)
{
	frontier_.clear();
	frontier_.push_back(scope);
	while (!frontier_.empty())
	{
		Scope* current = frontier_.back();
		frontier_.pop_back();
		if (current->visit == visit_)
			continue;
		current->visit = visit_;
		merge(result, own(current, name, filter));
		const NamespaceEntity* entity = static_cast<const NamespaceEntity*>(current->entity);
		for (NamespaceList* member = entity->inline_members; member != nullptr; member = member->next)
			frontier_.push_back(member->scope);
	}
}

LookupResult NameLookup::namespace_members(Scope* scope, const Identifier* name, LookupFilter filter)
{
	// 3.4.3.2p2: the namespace and its inline namespace set; where that
	// finds nothing, the namespaces its using-directives nominate, level by
	// level, each branch stopping where it finds the name.
	++visit_;
	LookupResult result = LookupResult();
	std::vector<Scope*>& level = level_;
	std::vector<Scope*>& next_level = next_level_;
	level.assign(1, scope);
	while (!level.empty() && !result.found())
	{
		next_level.clear();
		for (Scope* nominated : level)
		{
			if (nominated->visit == visit_)
				continue;
			LookupResult found = LookupResult();
			search_inline_set(nominated, name, filter, found);
			if (found.found())
			{
				merge(result, found);
				continue;
			}
			// The directives of the namespace and of its inline set.
			frontier_.push_back(nominated);
			for (size_t i = 0; i < frontier_.size(); ++i)
			{
				Scope* member = frontier_[i];
				for (const NamespaceList* directive = directives(member); directive != nullptr;
					directive = directive->next)
				{
					if (directive->scope->visit != visit_)
						next_level.push_back(directive->scope);
				}
				const NamespaceEntity* entity = static_cast<const NamespaceEntity*>(member->entity);
				for (NamespaceList* inline_member = entity->inline_members; inline_member != nullptr;
					inline_member = inline_member->next)
					frontier_.push_back(inline_member->scope);
			}
			frontier_.clear();
		}
		level.swap(next_level);
	}
	return result;
}

LookupResult NameLookup::qualified(Scope* scope, const Identifier* name, LookupFilter filter)
{
	++lookups_;
	switch (scope->kind)
	{
	case ScopeKind::Class:
		++visit_;
		return class_members(scope, name, filter, false);
	case ScopeKind::Namespace:
		return namespace_members(scope, name, filter);
	default:
		return own(scope, name, filter);
	}
}

LookupResult NameLookup::members(Scope* scope, const Identifier* name, LookupFilter filter)
{
	++lookups_;
	++visit_;
	return class_members(scope, name, filter, true);
}

} // namespace sema
