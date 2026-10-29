#include "syntax/name_scopes.h"

#include "lex/identifier_table.h"

#include <algorithm>

namespace syntax {

namespace {

const uint64_t kEmptyKey = ~0ull;

// Orders NameScopes::pending_ as a max-heap by the depth of `at`.
bool Shallower(const ScopeEdge* a, const ScopeEdge* b)
{
	return a->at->depth < b->at->depth;
}

const uint32_t kInitialSlots = 1024;

// NameScopes::marks_ bits.
const uint8_t kDeclared = 1;
const uint8_t kMember = 2;
// The memo has held a search of the name.
const uint8_t kMemo = 4;
// Some declaration made the name a template (see ever_template).
const uint8_t kTemplate = 8;
// A template of this name has a specialization with a base (see
// lookup_in); has members whose categories may differ between its
// specializations (see member_conflicts).
const uint8_t kBasedSpecialization = 16;
const uint8_t kMemberConflicts = 32;
// A template's lookups through a template-id compare its members'
// categories without a recorded conflict (a based specialization, a
// primary defined after a specialization; see lookup_in).
const uint8_t kCompareMembers = 64;
// A namespace has declared a value template of this name. Unqualified
// namespace lookup may combine it with an ordinary function overload.
const uint8_t kNamespaceTemplateValue = 128;

bool Found(const NameBinding& binding, bool prefix_only)
{
	if (binding.category == NameCategory::None)
		return false;
	if (!prefix_only)
		return true;
	return binding.category == NameCategory::Type || binding.category == NameCategory::TypeTemplate ||
		binding.category == NameCategory::Namespace || binding.scope != nullptr;
}

bool IsTypeCategory(NameCategory category)
{
	return category == NameCategory::Type || category == NameCategory::TypeTemplate;
}

// A redeclaration in the same scope: a function template and ordinary
// functions share a name (the name stays a template-name), and an object
// or function hides a class or enumeration of the same name (9.1p2) while
// the class stays reachable before `::`.
NameBinding Merge(const NameBinding& previous, const NameBinding& binding)
{
	if (previous.category == NameCategory::None)
		return binding;
	if (previous.category == NameCategory::FunctionTemplate && binding.category == NameCategory::Value)
		return previous;
	if (binding.category == NameCategory::Value && IsTypeCategory(previous.category))
		return NameBinding{ NameCategory::Value, false, previous.scope };
	if (previous.category == NameCategory::Value && IsTypeCategory(binding.category) && binding.scope != nullptr)
		return NameBinding{ NameCategory::Value, false, binding.scope };
	if (binding.category == previous.category && binding.scope == nullptr)
		return previous;
	return binding;
}

} // namespace

NameScopes::NameScopes(Arena& arena)
	: arena_(arena), slots_(kInitialSlots, Slot{ kEmptyKey, NameBinding{ NameCategory::None, false, nullptr } }),
	  slot_mask_(kInitialSlots - 1), count_(0), next_scope_id_(0), next_visit_(0), logging_(0), global_(nullptr),
	  current_(nullptr), lookups_(0), environment_(0), epoch_(0), memo_count_(0), edge_epoch_(0)
{
	global_ = push(ScopeKind::Namespace, nullptr);
}

uint64_t NameScopes::key_of(const Scope* scope, const Identifier* name)
{
	return (static_cast<uint64_t>(scope->id) << 32) | name->id;
}

uint32_t NameScopes::find_slot(uint64_t key) const
{
	// Fibonacci hashing of the combined key.
	const uint32_t mask = slot_mask_;
	uint32_t index = static_cast<uint32_t>((key * 0x9E3779B97F4A7C15ull) >> 40) & mask;
	while (slots_[index].key != key && slots_[index].key != kEmptyKey)
		index = (index + 1) & mask;
	return index;
}

void NameScopes::grow()
{
	std::vector<Slot> old(slots_.size() * 2, Slot{ kEmptyKey, NameBinding{ NameCategory::None, false, nullptr } });
	old.swap(slots_);
	slot_mask_ = static_cast<uint32_t>(slots_.size()) - 1;
	for (const Slot& slot : old)
	{
		if (slot.key != kEmptyKey)
			slots_[find_slot(slot.key)] = slot;
	}
}

Scope* NameScopes::push(ScopeKind kind, const Identifier* name)
{
	Scope* scope = arena_.allocate_array<Scope>(1);
	scope->kind = kind;
	scope->memo = false;
	scope->specialization = false;
	scope->parameter_bases = 0;
	scope->id = next_scope_id_++;
	scope->parent = current_;
	scope->depth = current_ != nullptr ? current_->depth + 1 : 0;
	scope->name = name;
	scope->edges = nullptr;
	scope->member_of = nullptr;
	scope->visit = 0;
	current_ = scope;
	return scope;
}

void NameScopes::reopen(Scope* scope)
{
	scope->parent = current_;
	scope->depth = current_->depth + 1;
	current_ = scope;
}

Scope* NameScopes::enter(Scope* scope)
{
	Scope* previous = current_;
	current_ = scope;
	return previous;
}

uint8_t NameScopes::marks(const Identifier* name) const
{
	return name->id < marks_.size() ? marks_[name->id] : 0;
}

bool NameScopes::ever_template(const Identifier* name) const
{
	return (marks(name) & kTemplate) != 0;
}

void NameScopes::declare(Scope* scope, const Identifier* name, NameBinding binding)
{
	if (name->id >= marks_.size())
		marks_.resize(std::max<size_t>(name->id + 1, marks_.size() * 2));
	const bool member = scope->kind == ScopeKind::Class || scope->kind == ScopeKind::Enum;
	marks_[name->id] |= member ? kDeclared | kMember : kDeclared;
	// 3.4.5p1: after `.` or `->` a template-id names a member template of
	// the object's class or a class template of the context, never a
	// namespace-scope function template (`std::count`, `std::min`).
	if (binding.category == NameCategory::TypeTemplate ||
		(binding.category == NameCategory::FunctionTemplate && member))
		marks_[name->id] |= kTemplate;
	if (binding.category == NameCategory::FunctionTemplate && scope->kind == ScopeKind::Namespace)
		marks_[name->id] |= kNamespaceTemplateValue;
	if (member)
		touch_name(name->id);
	const uint64_t key = key_of(scope, name);
	uint32_t index = find_slot(key);
	if (slots_[index].key == kEmptyKey)
	{
		if (2 * (count_ + 1) > slots_.size())
		{
			grow();
			index = find_slot(key);
		}
		slots_[index].key = key;
		slots_[index].binding = NameBinding{ NameCategory::None, false, nullptr };
		++count_;
	}
	Slot& slot = slots_[index];
	if (logging_ != 0)
		log_.push_back(LogEntry{ key, slot.binding, nullptr, nullptr });
	slot.binding = Merge(slot.binding, binding);
	environment_ = ++epoch_;
	if (scope->specialization && binding.category != NameCategory::None && scope->name != nullptr)
	{
		// The category a specialization gives the name, beside its
		// primary's and other specializations' (an undone declaration keeps
		// its bit: a lookup then only reports an ambiguity semantic
		// analysis resolves).
		const CategoryKey key = category_key(scope, name);
		const uint8_t bit = CategoryBit(binding.category);
		uint8_t* bits = categories_.find(key);
		uint8_t before = 0;
		if (bits == nullptr)
		{
			before = primary_member(scope, name);
			categories_.insert(key, static_cast<uint8_t>(before | bit));
		}
		else
		{
			before = *bits;
			*bits = static_cast<uint8_t>(*bits | bit);
		}
		const uint8_t after = static_cast<uint8_t>(before | bit);
		if ((before & (before - 1)) == 0 && (after & (after - 1)) != 0)
		{
			++conflicts_;
			mark_member_conflicts(scope->name);
		}
	}
}

const Scope* NameScopes::template_home(const Scope* scope)
{
	// The scope declaring a class template or its specialization (outside
	// their template parameters).
	const Scope* where = scope->parent;
	while (where != nullptr && where->kind == ScopeKind::Template)
		where = where->parent;
	return where;
}

NameScopes::CategoryKey NameScopes::category_key(const Scope* scope, const Identifier* name)
{
	// A primary template's or specialization's member: where the template
	// is declared, its name and the member's (same-named templates of other
	// scopes have their own records).
	const Scope* where = template_home(scope);
	return CategoryKey{ where != nullptr ? where->id : 0, scope->name->id, name->id };
}

uint8_t NameScopes::primary_member(const Scope* specialization, const Identifier* name) const
{
	// The primary template's own member: its scope is the template name's
	// where the specialization is declared.
	Scope* where = const_cast<Scope*>(template_home(specialization));
	const NameBinding templ = where != nullptr ? own(where, specialization->name) : NameBinding();
	const NameBinding member = templ.scope != nullptr ? own(templ.scope, name) : NameBinding();
	return member.category == NameCategory::None ? 0 : CategoryBit(member.category);
}

bool NameScopes::member_categories_differ(Scope* primary, const Identifier* name)
{
	if (primary->name == nullptr)
		return false;
	const uint8_t* bits = categories_.find(category_key(primary, name));
	if (bits == nullptr)
		return false;
	uint8_t all = *bits;
	const NameBinding own_member = own(primary, name);
	if (own_member.category != NameCategory::None)
		all = static_cast<uint8_t>(all | CategoryBit(own_member.category));
	all = static_cast<uint8_t>(all & ~CategoryBit(NameCategory::None));
	return (all & (all - 1)) != 0;
}

bool NameScopes::member_specialized(Scope* primary, const Identifier* name)
{
	return primary->name != nullptr && categories_.find(category_key(primary, name)) != nullptr;
}

void NameScopes::add_specialization(Scope* primary, Scope* specialization)
{
	add_edge_at(primary, specialization, nullptr, true);
	specialization->specialization = true;
}

void NameScopes::mark_member_conflicts(const Identifier* name)
{
	if (name == nullptr)
		return;
	any_member_conflicts_ = true;
	if (name->id >= marks_.size())
		marks_.resize(std::max<size_t>(name->id + 1, marks_.size() * 2));
	marks_[name->id] |= kMemberConflicts;
}

bool NameScopes::member_conflicts(const Identifier* name) const
{
	return (marks(name) & kMemberConflicts) != 0;
}

void NameScopes::note_late_primary(const Identifier* name)
{
	// Its members' categories, which its specializations' declarations did
	// not compare with (primary_member found none), join theirs where a
	// lookup compares them (lookup_in): statements naming members of the
	// template are read each way, outside template definitions as names
	// the parser can only guess are (note_uncertain).
	if (name == nullptr)
		return;
	if (name->id >= marks_.size())
		marks_.resize(std::max<size_t>(name->id + 1, marks_.size() * 2));
	marks_[name->id] |= kCompareMembers;
	mark_member_conflicts(name);
	uncertain_ = true;
}

void NameScopes::note_specialization_base(Scope* specialization)
{
	// Its members inherited from the base may take another category than
	// the primary's or other specializations' (lookup_in compares them):
	// statements naming members of the template are read each way,
	// outside template definitions (as note_late_primary).
	if (specialization->name == nullptr)
		return;
	if (specialization->name->id >= marks_.size())
		marks_.resize(std::max<size_t>(specialization->name->id + 1, marks_.size() * 2));
	marks_[specialization->name->id] |= kBasedSpecialization | kCompareMembers;
	mark_member_conflicts(specialization->name);
	uncertain_ = true;
}

void NameScopes::add_edge(Scope* from, Scope* to)
{
	add_edge_at(from, to, nullptr);
}

void NameScopes::add_new_edge(Scope* from, Scope* to)
{
	add_edge_at(from, to, nullptr, true);
}

void NameScopes::add_using_directive(Scope* from, Scope* nominated)
{
	// The nearest enclosing namespace of the directive that also encloses
	// the nominated namespace: level the two scopes, climb together to
	// their nearest common scope, then to a namespace.
	Scope* at = from;
	Scope* other = nominated;
	while (at->depth > other->depth)
		at = at->parent;
	while (other->depth > at->depth)
		other = other->parent;
	while (at != other)
	{
		at = at->parent;
		other = other->parent;
	}
	while (at->kind != ScopeKind::Namespace)
		at = at->parent;
	add_edge_at(from, nominated, at);
}

void NameScopes::add_base_edge(Scope* from, Scope* to, bool dependent, PointerMap& reached)
{
	// add_edge_at's repeat test: its scan while the class scope has few
	// edges (no map is made for most base clauses), then by the targets'
	// marks, taken when the edges reach that count.
	static const size_t kScannedEdges = 8;
	if (reached.size() == 0)
	{
		size_t edges = 0;
		for (const ScopeEdge* edge = from->edges; edge != nullptr && edges < kScannedEdges; edge = edge->next)
			++edges;
		if (edges < kScannedEdges)
		{
			add_edge_at(from, to, nullptr, false, dependent);
			return;
		}
		for (const ScopeEdge* edge = from->edges; edge != nullptr; edge = edge->next)
		{
			if (!edge->dependent || reached.find(edge->target) == PointerMap::kMissing)
				reached.set(edge->target, edge->dependent ? 1 : 2);
		}
	}
	const uint32_t known = reached.find(to);
	if (known != PointerMap::kMissing && (dependent || known == 2))
		return;
	add_edge_at(from, to, nullptr, true, dependent);
	reached.set(to, dependent ? 1 : 2);
}

void NameScopes::add_edge_at(Scope* from, Scope* to, Scope* at, bool fresh, bool dependent)
{
	// A dependent base repeats any edge to its scope; another edge repeats
	// only one that is searched as it is.
	for (ScopeEdge* edge = fresh ? nullptr : from->edges; edge != nullptr; edge = edge->next)
	{
		if (edge->target == to && (dependent || !edge->dependent))
			return;
	}
	if (logging_ != 0)
		log_.push_back(LogEntry{ 0, NameBinding{ NameCategory::None, false, nullptr }, from, from->edges });
	if (from->memo)
		++edge_epoch_;
	ScopeEdge* edge = arena_.allocate_array<ScopeEdge>(1);
	edge->target = to;
	edge->at = at;
	edge->dependent = dependent;
	edge->next = from->edges;
	from->edges = edge;
	environment_ = ++epoch_;
}

void NameScopes::complete_class(Scope* scope, bool templated)
{
	// A kept search is the same for every query: none through a dependent
	// base.
	scope->memo = !templated;
	for (const ScopeEdge* edge = scope->edges; edge != nullptr && scope->memo; edge = edge->next)
		scope->memo = edge->target->kind == ScopeKind::Class && edge->target->memo && !edge->dependent;
}

void NameScopes::touch_name(uint32_t id)
{
	// Only kept searches read an epoch, and one kept later records the
	// epoch of its time: names never kept need none.
	if (id >= marks_.size() || (marks_[id] & kMemo) == 0)
		return;
	if (id >= name_epochs_.size())
		name_epochs_.resize(std::max<size_t>(id + 1, name_epochs_.size() * 2));
	++name_epochs_[id];
}

uint32_t NameScopes::name_epoch(const Identifier* name) const
{
	return name->id < name_epochs_.size() ? name_epochs_[name->id] : 0;
}

void NameScopes::undone(const LogEntry& entry)
{
	// An undone or redone change: kept searches through it are stale.
	if (entry.edge_scope != nullptr)
	{
		if (entry.edge_scope->kind == ScopeKind::Class)
			++edge_epoch_;
	}
	else
		touch_name(static_cast<uint32_t>(entry.key));
}

uint64_t NameScopes::memo_key(const Scope* scope, const Query& query)
{
	return key_of(scope, query.name) | (query.prefix_only ? 1ull << 31 : 0);
}

size_t NameScopes::memo_limit() const
{
	return std::max<size_t>(1u << 16, static_cast<size_t>(count_) * 4);
}

NameScopes::MemoSlot* NameScopes::memo_slot(uint64_t key, bool insert)
{
	const size_t limit = memo_limit();
	if (insert && (memo_count_ + 1) * 2 > memo_.size() && memo_count_ < limit)
	{
		std::vector<MemoSlot> old(std::max<size_t>(1024, memo_.size() * 2), MemoSlot{ kEmptyKey, NameBinding(), 0, 0 });
		old.swap(memo_);
		memo_count_ = 0;
		for (const MemoSlot& slot : old)
		{
			if (slot.key != kEmptyKey)
				*memo_slot(slot.key, true) = slot;
		}
	}
	if (memo_.empty())
		return nullptr;
	const uint32_t mask = static_cast<uint32_t>(memo_.size()) - 1;
	uint32_t index = static_cast<uint32_t>((key * 0x9E3779B97F4A7C15ull) >> 40) & mask;
	while (memo_[index].key != key && memo_[index].key != kEmptyKey)
		index = (index + 1) & mask;
	if (memo_[index].key == key)
		return &memo_[index];
	if (!insert || memo_count_ >= limit)
		return nullptr;
	++memo_count_;
	memo_[index].key = key;
	return &memo_[index];
}

void NameScopes::undo_to(uint32_t mark)
{
	while (log_.size() > mark)
	{
		const LogEntry& entry = log_.back();
		undone(entry);
		if (entry.edge_scope != nullptr)
			entry.edge_scope->edges = entry.previous_edges;
		else
			slots_[find_slot(entry.key)].binding = entry.previous;
		log_.pop_back();
	}
}

void NameScopes::detach(uint32_t mark, std::vector<Change>& changes)
{
	const size_t first = changes.size();
	while (log_.size() > mark)
	{
		const LogEntry& entry = log_.back();
		undone(entry);
		Change change{ entry, NameBinding{ NameCategory::None, false, nullptr }, nullptr };
		if (entry.edge_scope != nullptr)
		{
			change.edges = entry.edge_scope->edges;
			entry.edge_scope->edges = entry.previous_edges;
		}
		else
		{
			Slot& slot = slots_[find_slot(entry.key)];
			change.binding = slot.binding;
			slot.binding = entry.previous;
		}
		changes.push_back(change);
		log_.pop_back();
	}
	std::reverse(changes.begin() + first, changes.end());
	environment_ = ++epoch_;
}

void NameScopes::reattach(const std::vector<Change>& changes, size_t begin, size_t end)
{
	for (size_t i = begin; i < end; ++i)
	{
		const Change& change = changes[i];
		undone(change.entry);
		if (change.entry.edge_scope != nullptr)
			change.entry.edge_scope->edges = change.edges;
		else
			slots_[find_slot(change.entry.key)].binding = change.binding;
		if (logging_ != 0)
			log_.push_back(change.entry);
	}
	environment_ = ++epoch_;
}

NameBinding NameScopes::own(Scope* scope, const Identifier* name) const
{
	const Slot& slot = slots_[find_slot(key_of(scope, name))];
	if (slot.key == kEmptyKey)
		return NameBinding{ NameCategory::None, false, nullptr };
	return slot.binding;
}

NameScopes::Query NameScopes::query(const Identifier* name, bool prefix_only)
{
	const uint8_t bits = marks(name);
	return Query{ name, prefix_only, ++next_visit_, (bits & kMember) != 0, (bits & kMemo) != 0, false };
}

NameBinding NameScopes::search(Scope* scope, Query& query)
{
	if (scope->visit == query.visit)
		return NameBinding{ NameCategory::None, false, nullptr };
	scope->visit = query.visit;
	const NameBinding binding = own(scope, query.name);
	if (Found(binding, query.prefix_only))
		return binding;
	if (scope->kind == ScopeKind::Class && !query.member)
		return NameBinding{ NameCategory::None, false, nullptr };
	// A kept search of a memo class. Scopes this lookup visited before hold
	// no binding of the name (the lookup would have ended), so a search's
	// result does not depend on which were.
	bool kept = false;
	if (scope->memo && query.memoized)
	{
		const MemoSlot* slot = memo_slot(memo_key(scope, query), false);
		if (slot != nullptr && slot->name_epoch == name_epoch(query.name) && slot->edge_epoch == edge_epoch_)
			return slot->result;
		kept = slot != nullptr;
	}
	NameBinding result{ NameCategory::None, false, nullptr };
	for (ScopeEdge* edge = scope->edges; edge != nullptr && result.category == NameCategory::None; edge = edge->next)
	{
		if (!edge->dependent || query.specialization)
			result = search(edge->target, query);
	}
	// A full memo only renews the searches it keeps.
	if (scope->memo && (kept || memo_count_ < memo_limit()))
	{
		const uint64_t key = memo_key(scope, query);
		if (MemoSlot* slot = memo_slot(key, true))
		{
			*slot = MemoSlot{ key, result, name_epoch(query.name), edge_epoch_ };
			marks_[query.name->id] |= kMemo;
			query.memoized = true;
		}
	}
	return result;
}

NameBinding NameScopes::search_class_context(Scope* scope, Query& query, Scope*& home)
{
	// The class of an out-of-class member definition and its enclosing
	// classes, up to the first namespace, which becomes `home`. A class
	// defined out of its enclosing class (`struct S::L { ... };`) is
	// enclosed by that class (9.7p3, 3.4.1p8), not by the scope its
	// definition appears in. The classes' definitions' template parameters
	// are not in scope: the out-of-class definition's own (lexically
	// enclosing it) are (14.5.1p3, 14.6.1p7).
	Scope* s = scope;
	while (s != nullptr && s->kind != ScopeKind::Namespace)
	{
		if (s->kind != ScopeKind::Template)
		{
			const NameBinding binding = search(s, query);
			if (binding.category != NameCategory::None)
				return binding;
		}
		s = s->kind == ScopeKind::Class && s->member_of != nullptr ? s->member_of : s->parent;
	}
	home = s;
	return NameBinding{ NameCategory::None, false, nullptr };
}

inline NameBinding NameScopes::search_level(Scope* scope, Query& query)
{
	// One scope on an unqualified lookup's way out: its own names and owned
	// edges, then the using-directives whose names appear at its depth.
	NameBinding binding{ NameCategory::None, false, nullptr };
	if (scope->kind == ScopeKind::Class)
	{
		// Its members and bases (all its edges).
		binding = search(scope, query);
		if (binding.category != NameCategory::None)
			return binding;
	}
	else if (scope->visit != query.visit)
	{
		scope->visit = query.visit;
		binding = own(scope, query.name);
		if (Found(binding, query.prefix_only))
			return binding;
		binding = NameBinding{ NameCategory::None, false, nullptr };
		for (ScopeEdge* edge = scope->edges; edge != nullptr && binding.category == NameCategory::None;
			edge = edge->next)
		{
			if (edge->at == nullptr)
				binding = search(edge->target, query);
			else
			{
				pending_.push_back(edge);
				std::push_heap(pending_.begin(), pending_.end(), Shallower);
			}
		}
		if (binding.category != NameCategory::None)
			return binding;
	}
	while (!pending_.empty() && pending_.front()->at->depth == scope->depth)
	{
		std::pop_heap(pending_.begin(), pending_.end(), Shallower);
		const ScopeEdge* edge = pending_.back();
		pending_.pop_back();
		binding = search(edge->target, query);
		if (binding.category != NameCategory::None)
			return binding;
	}
	return NameBinding{ NameCategory::None, false, nullptr };
}

NameBinding NameScopes::lookup(const Identifier* name, bool prefix_only)
{
	// Each scope's own names and owned edges, then the using-directives
	// whose names appear at this scope, then an out-of-class member's class.
	++lookups_;
	if ((marks(name) & kDeclared) == 0)
		return NameBinding{ NameCategory::None, false, nullptr };
	Query query = this->query(name, prefix_only);
	pending_.clear();
	// The namespace of an out-of-class member's class (or a namespace
	// member's namespace) once its class context is searched: it and the
	// namespaces enclosing it are searched before the definition's own
	// namespace, which encloses them (3.4.1p6, p8, p13).
	Scope* home = nullptr;
	for (Scope* scope = current_; scope != nullptr; scope = scope->parent)
	{
		NameBinding binding;
		if (home != nullptr && scope->kind == ScopeKind::Namespace)
		{
			Scope* inner = home;
			home = nullptr;
			Scope* outer = inner;
			while (outer != nullptr && outer->depth > scope->depth)
				outer = outer->parent;
			for (; outer == scope && inner != scope; inner = inner->parent)
			{
				binding = search_level(inner, query);
				if (binding.category != NameCategory::None)
					return binding;
			}
		}
		binding = search_level(scope, query);
		if (binding.category != NameCategory::None)
			return binding;
		if (scope->member_of != nullptr)
		{
			binding = search_class_context(scope->member_of, query, home);
			if (binding.category != NameCategory::None)
				return binding;
		}
	}
	return NameBinding{ NameCategory::None, false, nullptr };
}

NameBinding NameScopes::lookup_in(Scope* scope, const Identifier* name, bool prefix_only, bool specialization)
{
	++lookups_;
	if ((marks(name) & kDeclared) == 0)
		return NameBinding{ NameCategory::None, false, nullptr };
	Query query = this->query(name, prefix_only);
	query.specialization = specialization;
	NameBinding result = search(scope, query);
	if (specialization && result.category != NameCategory::None && scope->name != nullptr &&
		(conflicts_ != 0 || (marks(scope->name) & kCompareMembers) != 0))
	{
		// Specializations (and the primary) declaring the name with another
		// category make the found one a guess; so do specializations whose
		// bases give it another (a name some specialization or the primary
		// declares itself).
		const uint8_t* bits = categories_.find(category_key(scope, name));
		const bool based = (marks(scope->name) & kBasedSpecialization) != 0;
		const NameBinding primary = bits != nullptr || based ? own(scope, name) : NameBinding();
		if (bits != nullptr || primary.category != NameCategory::None)
		{
			uint8_t all = bits != nullptr ? *bits : 0;
			if (primary.category != NameCategory::None)
				all = static_cast<uint8_t>(all | CategoryBit(primary.category));
			for (ScopeEdge* edge = based ? scope->edges : nullptr; edge != nullptr; edge = edge->next)
			{
				if (!edge->target->specialization || edge->target->edges == nullptr)
					continue;
				Query inherited = this->query(name, prefix_only);
				inherited.specialization = true;
				const NameBinding found = search(edge->target, inherited);
				if (found.category != NameCategory::None)
					all = static_cast<uint8_t>(all | CategoryBit(found.category));
			}
			all = static_cast<uint8_t>(all & ~CategoryBit(result.category) & ~CategoryBit(NameCategory::None));
			if (all != 0)
			{
				result.conflicted = true;
				result.alternatives = all;
			}
		}
	}
	return result;
}

[[gnu::cold]] NameBinding NameScopes::namespace_overloads(Scope* scope, Query& query, uint32_t depth)
{
	// Using-directives are transitive for unqualified lookup (7.3.4p4).
	// Preserve a function-template category in the combined overload set;
	// semantic lookup diagnoses any invalid mixture with a non-function.
	if (scope->visit == query.visit)
		return NameBinding();
	scope->visit = query.visit;
	NameBinding binding = own(scope, query.name);
	for (ScopeEdge* edge = scope->edges;
		edge != nullptr && (binding.category == NameCategory::None || binding.category == NameCategory::Value);
		edge = edge->next)
	{
		if (edge->at != nullptr && edge->at->depth < depth)
		{
			pending_.push_back(edge);
			std::push_heap(pending_.begin(), pending_.end(), Shallower);
			continue;
		}
		const NameBinding found = namespace_overloads(edge->target, query, depth);
		if (binding.category == NameCategory::None || found.category == NameCategory::FunctionTemplate)
			binding = found;
	}
	return binding;
}

[[gnu::cold]] NameBinding NameScopes::search_template_level(Scope* scope, Query& query)
{
	// One scope on an unqualified lookup's way out: its own names and owned
	// edges, then the using-directives whose names appear at its depth.
	NameBinding binding{ NameCategory::None, false, nullptr };
	const bool overloads = scope->kind == ScopeKind::Namespace;
	if (scope->kind == ScopeKind::Class)
	{
		// Its members and bases (all its edges).
		binding = search(scope, query);
		if (binding.category != NameCategory::None)
			return binding;
	}
	else if (scope->visit != query.visit)
	{
		scope->visit = query.visit;
		binding = own(scope, query.name);
		if (Found(binding, query.prefix_only) && !(overloads && binding.category == NameCategory::Value))
			return binding;
		if (!Found(binding, query.prefix_only))
			binding = NameBinding{ NameCategory::None, false, nullptr };
		for (ScopeEdge* edge = scope->edges; edge != nullptr &&
			(binding.category == NameCategory::None || (overloads && binding.category == NameCategory::Value));
			edge = edge->next)
		{
			if (edge->at == nullptr)
			{
				const NameBinding found = overloads ? namespace_overloads(edge->target, query, scope->depth)
													: search(edge->target, query);
				if (binding.category == NameCategory::None || found.category == NameCategory::FunctionTemplate)
					binding = found;
			}
			else
			{
				pending_.push_back(edge);
				std::push_heap(pending_.begin(), pending_.end(), Shallower);
			}
		}
		if (binding.category != NameCategory::None && !(overloads && binding.category == NameCategory::Value))
			return binding;
	}
	while (!pending_.empty() && pending_.front()->at->depth == scope->depth)
	{
		std::pop_heap(pending_.begin(), pending_.end(), Shallower);
		const ScopeEdge* edge = pending_.back();
		pending_.pop_back();
		const NameBinding found = overloads ? namespace_overloads(edge->target, query, scope->depth)
											: search(edge->target, query);
		if (binding.category == NameCategory::None || found.category == NameCategory::FunctionTemplate)
			binding = found;
		if (binding.category != NameCategory::None && !(overloads && binding.category == NameCategory::Value))
			return binding;
	}
	return binding;
}

[[gnu::cold]] NameBinding NameScopes::lookup_template(const Identifier* name)
{
	// Each scope's own names and owned edges, then the using-directives
	// whose names appear at this scope, then an out-of-class member's class.
	++lookups_;
	if ((marks(name) & kDeclared) == 0)
		return NameBinding{ NameCategory::None, false, nullptr };
	Query query = this->query(name, false);
	pending_.clear();
	// The namespace of an out-of-class member's class (or a namespace
	// member's namespace) once its class context is searched: it and the
	// namespaces enclosing it are searched before the definition's own
	// namespace, which encloses them (3.4.1p6, p8, p13).
	Scope* home = nullptr;
	for (Scope* scope = current_; scope != nullptr; scope = scope->parent)
	{
		NameBinding binding;
		if (home != nullptr && scope->kind == ScopeKind::Namespace)
		{
			Scope* inner = home;
			home = nullptr;
			Scope* outer = inner;
			while (outer != nullptr && outer->depth > scope->depth)
				outer = outer->parent;
			for (; outer == scope && inner != scope; inner = inner->parent)
			{
				binding = search_template_level(inner, query);
				if (binding.category != NameCategory::None)
					return binding;
			}
		}
		binding = search_template_level(scope, query);
		if (binding.category != NameCategory::None)
			return binding;
		if (scope->member_of != nullptr)
		{
			binding = search_class_context(scope->member_of, query, home);
			if (binding.category != NameCategory::None)
				return binding;
		}
	}
	return NameBinding{ NameCategory::None, false, nullptr };
}

bool NameScopes::unqualified_template(const Identifier* name)
{
	return (marks(name) & kNamespaceTemplateValue) != 0 &&
		lookup_template(name).category == NameCategory::FunctionTemplate;
}

} // namespace syntax
