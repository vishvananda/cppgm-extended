#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "syntax/ast.h"
#include "syntax/gnu_names.h"
#include "syntax/name_conflicts.h"
#include "syntax/name_scopes.h"
#include "syntax/parse_token_stream.h"
#include "syntax/scratch_list.h"
#include "support/telemetry.h"

class IdentifierTable;

namespace syntax {

// Work counters of one translation unit's parse (spec §9 telemetry).
struct ParserCounters
{
	telemetry::Counter tokens;
	telemetry::Counter checkpoints;
	telemetry::Counter rewinds;
	telemetry::Counter deferred_bodies;
	telemetry::Counter lookups;
	// The names bound (also the semantic graph's name index's size hint,
	// so it is counted in every build).
	uint32_t bindings;
	telemetry::SmallCounter peak_window;
	telemetry::SmallCounter peak_nesting;
	telemetry::Counter arena_bytes;
	// Captured member function bodies: total tokens captured, and the
	// run arena's peak (runs are released once their class completes).
	telemetry::Counter deferred_tokens;
	telemetry::Counter peak_run_bytes;
	// Lambda bodies skipped while speculating and parsed after the commit.
	telemetry::Counter deferred_lambdas;
	// Kept member searches (NameScopes' memo).
	telemetry::SmallCounter kept_searches;
};

// The parser's state below the grammar: the token cursor and its helpers,
// checkpoints for speculation, the syntax arena and list scratch stacks,
// the name scopes, the deferred member bodies and the work counters. The
// grammar productions live in Parser (syntax/parser.h).
class ParserState
{
public:
	// `hosted`: a hosted compilation's contextual coroutine operators.
	ParserState(ParseTokenStream& tokens, IdentifierTable& identifiers, Arena& arena, bool hosted = false);

	// The furthest location where a parse failed (for the diagnostic).
	uint32_t error_location() const { return error_location_; }
	ParserCounters counters() const;

protected:
	~ParserState() = default;

	friend class Tentative;
	friend class NestingGuard;
	friend class SpeculationBoundary;

	struct Checkpoint
	{
		uint32_t position;
		Arena::Mark mark;
		Arena::Mark runs;
		uint32_t log;
		uint64_t environment;
		Scope* scope;
		size_t deferred;
		size_t lambdas;
		size_t classes;
		Scope* function_scope;
		bool angle;
		bool pending_template;
		bool definite_type;
		bool special_member_clause;
		uint32_t pack_mentions;
		uint32_t parameter_mentions;
		StatementConflict conflict;
	};

	struct DeferredBody
	{
		FunctionBody* target;
		TokenRun tokens;
		Scope* scope;
		bool allow_ctor_initializer;
	};

	// A lambda body skipped while speculating (see parse_deferred_lambdas):
	// its tokens [begin, end) in the current token source, its scope, the
	// name-change log position where it was written, and the flags it would
	// have been parsed with.
	struct DeferredLambda
	{
		LambdaExpr* lambda;
		uint32_t begin;
		uint32_t end;
		Scope* scope;
		uint32_t log;
		bool definite_type;
		bool special_member_clause;
	};

	// ------------------------------------------------------------ tokens
	ParseToken tok(uint32_t ahead = 0) { return tokens_.peek(ahead); }
	bool at(TokenKind kind, uint32_t ahead = 0) { return tokens_.peek(ahead).is(kind); }
	bool at_identifier(uint32_t ahead = 0) { return tokens_.peek(ahead).is_identifier(); }
	bool at_identifier(const Identifier* identifier) { return tok().is_identifier() && tok().identifier == identifier; }
	void next() { tokens_.advance(); }
	bool accept(TokenKind kind);
	bool expect(TokenKind kind);
	bool at_close_angle() { return at(TokenKind::OP_GT); }
	bool at_shift_right() { return at(TokenKind::OP_GT) && (tok().flags & kFirstShiftHalf) != 0; }
	uint32_t location() { return tok().location; }
	bool fail();
	bool skip_balanced();
	bool skip_attributes();
	bool at_attribute(uint32_t ahead = 0);
	// At `<`: whether a `>` follows at the same bracket depth before the
	// enclosing region ends (a template argument list needs one).
	bool closing_angle_ahead();

	// ------------------------------------------------------- checkpoints
	Checkpoint save();
	void restore(const Checkpoint& checkpoint);
	void commit(const Checkpoint& checkpoint);
	// Parses the lambda bodies queued from `first` on, once the speculation
	// that skipped them has committed (Parser implements the grammar).
	virtual void parse_deferred_lambdas(size_t first) = 0;
	void close(const Checkpoint& checkpoint);

	// ------------------------------------------------------- allocation
	template<typename T>
	T* make()
	{
		return arena_.allocate_array<T>(1);
	}
	template<typename T, typename K>
	T* make(K kind, uint32_t location)
	{
		T* node = arena_.allocate_array<T>(1);
		node->kind = kind;
		node->location = location;
		return node;
	}
	Name* make_simple_name(const Identifier* identifier, uint32_t location);
	// Where a declaration's name is bound: the current scope, outside any
	// template parameter scopes (a template's name belongs to the scope
	// enclosing its template-declaration).
	Scope* declaration_scope() const;
	// Whether a template-head's parameters are in scope.
	bool in_template_definition();
	// A class scope nothing has reached yet (no definition, no
	// specialization): a prefix naming it is as unknown as a class
	// without a scope.
	bool scope_pending(Scope* scope) const;
	// The class template whose definition encloses the name, or whose
	// member an out-of-class definition defines (14.6.2.1p1, 3.4.1p8).
	bool current_instantiation(const Scope* templ) const;
	// A primary class template's base naming one of its template
	// parameters (see parameter_bases_).
	void record_parameter_base(Scope* class_scope, const Identifier* name, bool pack);
	// A member of a specialization of `templ` (its arguments `arguments`)
	// found in a base naming a template parameter: in the argument's class
	// (argument_class: a lone class name's, looked up again).
	NameBinding parameter_base_lookup(const Scope* templ, const Slice<TemplateArgument>& arguments,
		const Identifier* identifier, bool prefix_only, uint32_t depth);
	Scope* argument_class(const TemplateArgument& argument);
	// The specialization record (NameBinding::specialization_record) of a
	// named specialization of a template whose bases name its parameters: a
	// template-id's, made (0 when the records are exhausted), or an
	// unqualified typedef name's own (0: none).
	uint8_t specialization_record(const Name& name);
	// A typedef's or alias's record of the type its specifiers name (see
	// specialization_record): a specialization of a template whose bases
	// name its parameters, by a template-id whose arguments name no
	// template parameter (`mentions`) or by another typedef of one.
	uint8_t typedef_record(const Name* name, Scope* scope, NameCategory category, bool mentions);
	// Whether a statement here is read once per category of a name it
	// meets that has several: where a template's specializations disagree,
	// and outside template definitions where the parser only guesses one
	// (see Parser::guessed_component).
	bool reads_alternatives() { return scopes_.has_conflicts() || (scopes_.uncertain() && !in_template_definition()); }

	ParseTokenStream& tokens_;
	IdentifierTable& identifiers_;
	Arena& arena_;
	NameScopes scopes_;

	ScratchStack<Decl*> decls_;
	ScratchStack<Stmt*> stmts_;
	ScratchStack<Expr*> exprs_;
	ScratchStack<DeclSpecifier> specs_;
	ScratchStack<NameComponent> components_;
	ScratchStack<TemplateArgument> template_args_;
	ScratchStack<PtrOperator> pointers_;
	ScratchStack<TokenKind> cvs_;
	ScratchStack<DeclaratorSuffix> suffixes_;
	ScratchStack<FunctionQualifier> qualifiers_;
	ScratchStack<ParameterDeclaration*> params_;
	ScratchStack<TypeId*> type_ids_;
	ScratchStack<InitDeclarator> init_declarators_;
	ScratchStack<MemInitializer> mem_inits_;
	ScratchStack<Handler> handlers_;
	ScratchStack<BaseSpecifier> bases_;
	ScratchStack<Enumerator> enumerators_;
	ScratchStack<BitFieldDeclarator> bit_fields_;
	ScratchStack<TemplateParameter*> template_params_;
	ScratchStack<LambdaCapture> captures_;

	std::vector<DeferredBody> deferred_;
	// Lambda bodies skipped by open speculation, in source order.
	std::vector<DeferredLambda> lambdas_;
	telemetry::Counter deferred_lambdas_;
	// The lambda bodies parsed for the readings of a statement read several
	// ways (Parser::read_statement), by their first token: each is parsed
	// once and shared by the readings (a body is read as written whatever
	// the statement's reading, see ConflictScope). Null elsewhere.
	std::vector<std::pair<uint32_t, CompoundStmt*>>* shared_bodies_ = nullptr;
	// The captured token runs of deferred bodies, released when the
	// outermost class that queued them is complete and they are parsed.
	Arena runs_;
	telemetry::Counter deferred_tokens_;
	telemetry::Counter peak_run_bytes_;
	// A speculative template argument list: its `<` (absolute token
	// position) and what its parse depends on besides the tokens, the names
	// visible and the scope they are looked up from.
	struct TemplateListKey
	{
		uint64_t position;
		uint64_t environment;
		uint32_t scope;

		bool operator<(const TemplateListKey& other) const
		{
			if (position != other.position)
				return position < other.position;
			if (environment != other.environment)
				return environment < other.environment;
			return scope < other.scope;
		}
	};
	TemplateListKey template_list_key() const
	{
		return TemplateListKey{ tokens_.absolute_position(), scopes_.environment(), scopes_.current()->id };
	}
	// Speculative template argument lists that failed (sorted): each is
	// tried once, so nested undeclared names cannot make parsing
	// exponential.
	std::vector<TemplateListKey> failed_template_lists_;
	// skip_balanced's closer stack.
	std::vector<TokenKind> closers_;
	// Token runs, by absolute position, that a depth-0 scan crossed before
	// finding a closing `>` or the end of its region (sorted, disjoint): a
	// later `<` in a run shares the answer, so chains and nests of `<` are
	// scanned once.
	struct AngleFence
	{
		uint64_t begin;
		uint64_t end;
		bool found;
	};
	std::vector<AngleFence> angle_fences_;
	// The classes being defined, innermost last: their names (constructor
	// detection) and whether the outermost one is waiting for bodies.
	std::vector<const Identifier*> class_names_;
	// The function scope of the last function declarator whose parameter
	// clause directly followed a declarator-id, and of the last clause.
	Scope* function_scope_;
	Scope* parameter_scope_;
	// The class or namespace the last named declarator's qualified
	// declarator-id declares a member of, when known: the names after the
	// declarator-id (array bounds, the initializer) are looked up in its
	// scope (3.4.3p3, 9.4.2p2).
	Scope* declarator_member_of_;
	// The class or other scope named by the last type-id's lone type name,
	// and its specialization record (see typedef_record).
	Scope* type_id_scope_;
	uint8_t type_id_record_ = 0;
	// The innermost template-parameter-clause whose declaration is being
	// parsed (null outside one).
	const TemplateParameterList* template_head_ = nullptr;
	// The primary class templates with bases naming their template
	// parameters (Scope::parameter_bases): each such base's parameter index
	// (kPackBase marks a parameter pack's expansion, every argument from
	// the index on).
	static const uint32_t kPackBase = 0x80000000u;
	std::vector<std::pair<const Scope*, std::vector<uint32_t>>> parameter_bases_;
	// The specializations of templates whose bases name their parameters
	// that typedefs and aliases name (NameBinding::specialization_record -
	// 1): the template-id's arguments.
	std::vector<Slice<TemplateArgument>> specialization_records_;
	// Inside a template argument list: `>` ends the list (14.2p3).
	bool angle_;
	// Parsing the type specifiers of a definite type-id.
	bool definite_type_;
	// Parsing a special member's parameter clause (no initializer can
	// follow its name).
	bool special_member_clause_;
	// Parsing a friend declaration's declarators: a namespace member's
	// qualified declarator-id leaves the names after it to the befriending
	// class's lookup (see Analyzer::declarator).
	bool friend_declarator_;
	// Parsing a class-head's or enum-head's name (see
	// Parser::advance_prefix's `declared`).
	bool naming_declared_;
	// Inside a statement read once per category (see reads_alternatives).
	bool alternatives_active_ = false;
	// in_template_definition's last scope, its depth then and the answer.
	uint32_t template_query_id_ = ~0u;
	uint32_t template_query_depth_ = 0;
	bool template_query_result_ = false;
	// Names of template parameter packs parsed so far, less those a pack
	// expansion consumed, and the count where the current parameter
	// declaration began: a parameter's `...` before `)` is its own pack
	// expansion only if its type names an unexpanded pack, otherwise the
	// clause's ellipsis (8.3.5p14).
	uint32_t pack_mentions_;
	uint32_t parameter_mentions_;
	// Names of template parameters parsed so far: a base-specifier naming
	// one is a dependent base.
	uint32_t template_parameter_mentions_;
	// Names whose category specializations disagree on (see NameConflicts).
	NameConflicts conflicts_;
	// The declaration being parsed follows a template-parameter-clause, so
	// the entity it declares is a template.
	bool pending_template_;
	// The attributes of the declaration being parsed (before or among its
	// decl-specifiers), attached to its node.
	Attribute* pending_attributes_;
	Attribute* leading_attributes_;
	uint32_t tentative_depth_;
	// Checkpoints opened since the innermost speculation boundary (a class
	// body, a deferred lambda body): lambda bodies are skipped while it is
	// nonzero.
	uint32_t speculation_;
	// The speculative template argument lists being parsed, outermost
	// first; those above open_lists_base_ belong to the
	// innermost speculation boundary.
	std::vector<TemplateListKey> open_lists_;
	size_t open_lists_base_;
	// A nested speculative list failed (see Parser::abandon_template_lists):
	// the open lists fail with it, and the parse returns failure up to the
	// outermost one, which reads its `<` as a value. The farthest failure
	// is reported as it was when the lists were abandoned.
	bool abandoning_lists_;
	uint64_t abandoned_error_position_;
	uint32_t abandoned_error_location_;
	// Open nested constructs and the stack they may use (see NestingGuard).
	uint32_t nesting_;
	telemetry::SmallCounter peak_nesting_;
	uintptr_t stack_base_;
	uintptr_t stack_budget_;
	// The farthest failure in token order (locations are not ordered
	// across inclusions) and its token's location.
	uint64_t error_position_;
	uint32_t error_location_;
	telemetry::Counter checkpoints_;
	telemetry::Counter rewinds_;
	telemetry::Counter deferred_count_;
	const bool hosted_;

	// The virt-specifiers; C++20's coroutine operators, identifiers in C++11
	// (see Parser::at_coroutine_keyword).
	const Identifier *final_, *override_;
	const Identifier* coroutine_keywords_[3];
	const Identifier* unnamed_namespace_;
	// GCC's 128-bit and extended floating type names, which may follow a
	// complex specifier; the first, `__int128`, combines with `signed` and
	// `unsigned`.
	const Identifier* extended_types_[10];
	GnuNames gnu_names_;

	// The trait whose name is the current token, followed by `(`; GNU
	// typeof; an identifier that starts a name (no trait followed by `(`,
	// no typeof or attribute).
	bool at_trait(GnuTrait& trait), at_typeof();
	bool at_name_identifier(uint32_t ahead = 0)
	{
		const uint8_t name = tok(ahead).is_identifier() ? gnu_names_.kind(tok(ahead).identifier) : GnuNames::kTypeof;
		return name == 0 || name == GnuNames::kComplex ||
			(name < GnuNames::kFirstSpecial && !at(TokenKind::OP_LPAREN, ahead + 1));
	}
	// GNU `__typeof__ (` or `__underlying_type (`, which denote types as a
	// decltype-specifier does.
	bool at_gnu_decltype(uint32_t ahead = 0);
};

} // namespace syntax
