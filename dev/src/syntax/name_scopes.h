#pragma once

#include <cstdint>
#include <vector>

#include "support/flat_map.h"
#include "support/memory/arena.h"
#include "support/pointer_map.h"
#include "support/telemetry.h"

struct Identifier;

namespace syntax {

// What a name denotes, as far as parsing needs to know (parsing.md's name
// categories): a value (object, function, enumerator, non-type template
// parameter), a type, a template whose template-id is a type or a value, or
// a namespace.
enum class NameCategory : uint8_t
{
	None,
	Value,
	Type,
	TypeTemplate,
	FunctionTemplate,
	Namespace
};

inline uint8_t CategoryBit(NameCategory category)
{
	return static_cast<uint8_t>(1u << static_cast<unsigned>(category));
}

enum class ScopeKind : uint8_t
{
	Namespace,
	Class,
	Enum,
	Template,
	Function,
	Block
};

struct Scope;

struct ScopeEdge
{
	Scope* target;
	// A using-directive's nominated names are found, by unqualified lookup,
	// as if declared in this enclosing namespace (7.3.4p2); null for edges
	// searched with their owner (inline and unnamed namespaces, bases).
	Scope* at;
	ScopeEdge* next;
	// A base depending on a template parameter, as its template's scope:
	// searched only by a lookup in a specialization's members (14.6.2p3
	// keeps it out of unqualified lookup in the template).
	bool dependent;
};

struct Scope
{
	ScopeKind kind;
	// A complete class, neither a template nor a specialization, whose bases
	// are all such classes: its members and bases change only through
	// declarations and checkpoint undo, so its member searches are kept.
	bool memo;
	// A class template's partial or explicit specialization, whose members
	// its primary template's scope shares (see add_specialization).
	bool specialization;
	// The parser's base-parameter record (index + 1), or zero. A
	// specialization's members include its arguments' members when its
	// template inherits a parameter (see Parser::parameter_base_lookup).
	uint32_t parameter_bases;
	uint32_t id;
	Scope* parent;
	const Identifier* name;
	// Namespace: nominated namespaces (using-directives, inline and unnamed
	// members); Class: base classes whose scopes are known.
	ScopeEdge* edges;
	// The function scope of an out-of-class member definition: the class
	// whose member it defines (3.4.1p8), searched before the lexical parents.
	Scope* member_of;
	// Lookup traversal marker (cycle protection through edges).
	uint32_t visit;
	// Enclosing scopes (0 for the global namespace).
	uint32_t depth;
};

struct NameBinding
{
	NameBinding()
		: category(NameCategory::None), pack(false), parameter(false), conflicted(false), alternatives(0), flags(0),
		  scope(nullptr)
	{}
	NameBinding(NameCategory category, bool pack, Scope* scope, bool parameter = false)
		: category(category), pack(pack), parameter(parameter), conflicted(false), alternatives(0), flags(0),
		  scope(scope)
	{}

	NameCategory category;
	// A template parameter pack (see ParserState::pack_mentions_).
	bool pack;
	// A template parameter (see ParserState::template_parameter_mentions_).
	bool parameter;
	// Found by a lookup in a template-id's members where the template's
	// specializations declare the name with different categories (which one
	// a template-id selects is semantic): `alternatives` holds the others, a
	// bit per NameCategory (see CategoryBit).
	bool conflicted;
	// Not conflicted, a typedef or alias of a class template specialization
	// holds its record instead (see specialization_record).
	uint8_t alternatives;
	// kApproximate: a typedef-name or alias whose `scope` is a guess: the
	// class of a member a template-id reaches that the template's
	// specializations declare too (which one a template-id selects is
	// semantic), so the categories of the names through it are guesses too
	// (see Parser::resolve_component). kDependent: a typedef-name whose
	// type depends on its template's parameters (not the current
	// instantiation): where unqualified lookup finds it (in its template),
	// the names through it are dependent (14.6.2.1p7, 14.6p2); `scope`
	// serves the lookups through a template-id's members.
	static const uint8_t kApproximate = 1;
	static const uint8_t kDependent = 2;
	uint8_t flags;
	bool approximate() const { return (flags & kApproximate) != 0; }
	bool dependent() const { return (flags & kDependent) != 0; }
	// A typedef's or alias's record of the specialization it names, of a
	// template whose bases name its parameters (from 1, Parser::
	// specialization_records_; 0: none): a lookup in its members searches
	// the dependent bases and the arguments' classes, as one through the
	// template-id does (Parser::parameter_base_lookup). (`alternatives`
	// holds it: the table's bindings get no field.)
	uint8_t specialization_record() const { return conflicted ? 0 : alternatives; }

	// The class, enumeration or namespace the name denotes, directly or
	// through a typedef or alias; null if unknown (e.g. a template
	// parameter). A class hidden by a same-scope object or function keeps
	// its scope for nested-name-specifier lookup (3.4.3p1).
	Scope* scope;
};

// Scope-structured name categories for one translation unit: bindings are
// keyed by (scope, interned identifier) in one open-addressed table, scopes
// are parent-linked, and using-directives and base classes are explicit
// edges. While a parser checkpoint is open every change is logged, so a
// failed alternative's declarations are undone with it.
class NameScopes
{
public:
	explicit NameScopes(Arena& arena);

	Scope* global() const { return global_; }
	Scope* current() const { return current_; }

	// A new scope below the current one, made current.
	Scope* push(ScopeKind kind, const Identifier* name);
	// A class scope made at the class's first declaration, placed below the
	// current scope and made current as push does: its definition's
	// template parameters and enclosing scopes (nothing is below it yet).
	void reopen(Scope* scope);
	// Makes `scope` current; returns the previous current scope.
	Scope* enter(Scope* scope);
	void set_current(Scope* scope) { current_ = scope; }

	void declare(Scope* scope, const Identifier* name, NameBinding binding);
	void add_edge(Scope* from, Scope* to);
	// An edge to a scope just made, which no edge reaches yet (no scan for
	// a repeat).
	void add_new_edge(Scope* from, Scope* to);
	// A base clause's edge (a dependent base's repeats any edge to its
	// scope; another repeats only one searched as it is), the repeat found
	// in `reached`, which the caller keeps over the clause: the targets of
	// the class scope's edges (taken once they are several), 2 when one is
	// searched as it is, else 1. So a class of many bases costs each once.
	void add_base_edge(Scope* from, Scope* to, bool dependent, PointerMap& reached);
	void add_using_directive(Scope* from, Scope* nominated);
	// A specialization's scope just made, linked from its primary
	// template's: the categories its members take are recorded by (template
	// name, member name), so a lookup in a template-id's members knows when
	// the specializations disagree (NameBinding::conflicted).
	void add_specialization(Scope* primary, Scope* specialization);
	// Whether some template of this name (any template) has a member its
	// specializations may give different categories (declared so, or
	// through a specialization's base or a late primary).
	bool member_conflicts(const Identifier* name) const;
	bool any_member_conflicts() const { return any_member_conflicts_; }
	// A specialization's base whose scope is known (see lookup_in).
	void note_specialization_base(Scope* specialization);
	// A primary template defined after some specialization (see lookup_in).
	void note_late_primary(const Identifier* name);
	// Some name has several categories there, or a category the parser can
	// only guess (see note_uncertain): a unit without either reads each
	// statement once (see Parser::reads_alternatives).
	bool has_category_conflicts() const { return conflicts_ != 0 || uncertain_; }
	bool has_conflicts() const { return conflicts_ != 0; }
	bool uncertain() const { return uncertain_; }
	// Some declaration names a type whose class the parser does not know
	// (a typedef of a template parameter, an alias template of a member a
	// template-id reaches) or only guesses (NameBinding::approximate): the
	// names through it may be read with another category.
	void note_uncertain() { uncertain_ = true; }
	// Whether a specialization of the primary template declares the member
	// (a template-id's member's class is then a guess).
	bool member_specialized(Scope* primary, const Identifier* name);
	// Whether the primary template's specializations (and itself) declare
	// the member with several categories.
	bool member_categories_differ(Scope* primary, const Identifier* name);
	// The class's closing brace has been parsed.
	void complete_class(Scope* scope, bool templated);
	// Whether some declaration so far, in any scope, made the name a
	// template (a class, alias, function or variable template, or a
	// template template-parameter). A member name the parser cannot look
	// up (after `.` or `->`) that was never one is no template-id's name.
	bool ever_template(const Identifier* name) const;

	// Unqualified lookup from the current scope. With `prefix_only`, only
	// types, templates and namespaces are found (a name before `::`).
	NameBinding lookup(const Identifier* name, bool prefix_only);
	// Qualified lookup in a class, enumeration or namespace scope; with
	// `specialization`, in a class template specialization's (a
	// template-id's), whose dependent bases are searched too.
	NameBinding lookup_in(Scope* scope, const Identifier* name, bool prefix_only, bool specialization = false);
	// A binding of the scope itself, without edges.
	NameBinding own(Scope* scope, const Identifier* name) const;

private:
	struct LogEntry
	{
		// A binding's previous contents (by key: slots move when the table
		// grows), or an edge list's previous head.
		uint64_t key;
		NameBinding previous;
		Scope* edge_scope;
		ScopeEdge* previous_edges;
	};

public:
	// A logged change with the value it set, detached from the log.
	struct Change
	{
		LogEntry entry;
		NameBinding binding;
		ScopeEdge* edges;
	};

	// Change logging for checkpoints.
	uint32_t log_mark() const { return static_cast<uint32_t>(log_.size()); }
	void undo_to(uint32_t mark);
	// Undoes the changes logged after `mark`, appending them to `changes` in
	// their original order; reattach() then redoes and relogs a prefix of
	// them. Between the two, lookups see the names as they were at `mark`.
	void detach(uint32_t mark, std::vector<Change>& changes);
	void reattach(const std::vector<Change>& changes, size_t begin, size_t end);
	void begin_logging() { ++logging_; }
	// The log is dropped when the outermost checkpoint closes.
	void end_logging()
	{
		if (--logging_ == 0)
			log_.clear();
	}

	// Identifies the current set of bindings and edges: every change makes a
	// new id, and ids are never reused, so equal ids mean equal names (a
	// checkpoint restores its id after undo_to).
	uint64_t environment() const { return environment_; }
	void set_environment(uint64_t environment) { environment_ = environment; }

	telemetry::Counter lookups() const { return lookups_; }
	uint32_t binding_count() const { return count_; }
	uint32_t kept_searches() const { return memo_count_; }

private:
	struct Slot
	{
		uint64_t key;
		NameBinding binding;
	};

	// A kept member search of a `memo` class: (scope, name, prefix_only) to
	// its result, valid while the name's epoch (advanced by each change of
	// the name in a class or enumeration scope) and the class-edge epoch
	// (advanced by each undone or redone change of a class's edges) are
	// those it was found in.
	struct MemoSlot
	{
		uint64_t key;
		NameBinding result;
		uint32_t name_epoch;
		uint32_t edge_epoch;
	};

	static uint64_t key_of(const Scope* scope, const Identifier* name);
	uint8_t marks(const Identifier* name) const;
	void touch_name(uint32_t id);
	uint32_t name_epoch(const Identifier* name) const;
	size_t memo_limit() const;
	MemoSlot* memo_slot(uint64_t key, bool insert);
	void undone(const LogEntry& entry);
	uint32_t find_slot(uint64_t key) const;
	void grow();
	// One lookup's name and what is known of it, for the searches.
	struct Query
	{
		const Identifier* name;
		bool prefix_only;
		uint32_t visit;
		// Declared in some class or enumeration scope.
		bool member;
		// Some kept search is of this name.
		bool memoized;
		// Dependent bases are searched (see ScopeEdge::dependent).
		bool specialization;
	};

	Query query(const Identifier* name, bool prefix_only);
	static uint64_t memo_key(const Scope* scope, const Query& query);
	NameBinding search(Scope* scope, Query& query);
	void add_edge_at(Scope* from, Scope* to, Scope* at, bool fresh = false, bool dependent = false);
	NameBinding search_class_context(Scope* scope, Query& query, Scope*& home);
	NameBinding search_level(Scope* scope, Query& query);

	Arena& arena_;
	std::vector<Slot> slots_;
	// slots_.size() - 1, the probes' index mask.
	uint32_t slot_mask_;
	std::vector<LogEntry> log_;
	uint32_t count_;
	uint32_t next_scope_id_;
	uint32_t next_visit_;
	uint32_t logging_;
	Scope* global_;
	Scope* current_;
	telemetry::Counter lookups_;
	uint64_t environment_;
	uint64_t epoch_;
	// Using-directives met on the current unqualified lookup's way out,
	// waiting for their `at` namespace: a max-heap by its depth (each `at`
	// is the one scope of its depth on the way out; reused storage).
	std::vector<const ScopeEdge*> pending_;
	// Indexed by Identifier::id (and possibly shorter): whether the name was
	// ever declared (a lookup of a name never declared finds nothing without
	// a search), ever in a class or enumeration scope (a class scope's
	// edges lead only to class scopes, so they are not searched for other
	// names), and ever in a kept search. An undone declaration leaves its
	// marks, which only costs a search.
	std::vector<uint8_t> marks_;
	// Kept member searches (open-addressed, at most four per binding), the
	// per-name epochs (indexed by Identifier::id, possibly shorter) and the
	// class-edge epoch.
	std::vector<MemoSlot> memo_;
	uint32_t memo_count_;
	std::vector<uint32_t> name_epochs_;
	uint32_t edge_epoch_;
	// The categories of the members specializations declare, with the
	// primary's own member when first recorded (a bit per NameCategory), by
	// (the scope declaring the template, its name, the member's name). How
	// many records have several categories.
	struct CategoryKey
	{
		uint32_t where;
		uint32_t templ;
		uint32_t name;
		bool operator==(const CategoryKey& other) const
		{
			return where == other.where && templ == other.templ && name == other.name;
		}
	};
	struct CategoryKeyHash
	{
		size_t operator()(const CategoryKey& key) const
		{
			const uint64_t mixed = (static_cast<uint64_t>(key.templ) << 32 | key.name) ^
				(key.where * 0xff51afd7ed558ccdull);
			return static_cast<size_t>(mixed * 0x9e3779b97f4a7c15ull >> 17);
		}
	};
	static const Scope* template_home(const Scope* scope);
	static CategoryKey category_key(const Scope* scope, const Identifier* name);
	void mark_member_conflicts(const Identifier* name);
	uint8_t primary_member(const Scope* specialization, const Identifier* name) const;
	FlatMap<CategoryKey, uint8_t, CategoryKeyHash> categories_;
	uint32_t conflicts_ = 0;
	bool any_member_conflicts_ = false;
	bool uncertain_ = false;
};

} // namespace syntax
