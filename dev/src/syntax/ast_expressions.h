#pragma once

#include <utility>

#include "lex/gnu_builtins.h"
#include "syntax/ast_types.h"

// Syntax tree expressions (see syntax/ast.h).
namespace syntax {

enum class ExprKind : uint8_t
{
	Id,
	Literal,
	KeywordLiteral,
	Paren,
	Binary,
	Assign,
	Conditional,
	Unary,
	Postfix,
	Call,
	FunctionalCast,
	Subscript,
	Member,
	Cast,
	Sizeof,
	SizeofPack,
	TypeTrait,
	New,
	Delete,
	Lambda,
	BracedInit,
	PackExpansion,
	Throw,
	// GNU `({ ... })`.
	Statement,
	// A GNU type trait `__is_class ( type-id )` ... (lex/gnu_builtins.h).
	BuiltinTrait,
	// C++17's fold-expression (a hosted-header extension in C++11 mode).
	Fold
};

struct Expr
{
	ExprKind kind;
	uint32_t location;
	// Set once by semantic analysis (in semantics mode): what the
	// expression means. The tree is otherwise immutable (but see
	// MemberExpr::base).
	mutable const sema::ExprFacts* facts = nullptr;
};

struct IdExpr : Expr
{
	Name* name;
};

struct LiteralExpr : Expr
{
	// The literal's source; for a concatenated string literal the pieces
	// separated by single spaces.
	TokenSpelling spelling;
	// Dumped with its token kind (`literal TT_LITERAL:0`), as the course
	// reference prints the lone-literal default of an unnamed `int`
	// template parameter.
	bool token_form;
};

// true, false, nullptr, this.
struct KeywordLiteralExpr : Expr
{
	TokenKind keyword;
};

struct ParenExpr : Expr
{
	Expr* inner;
};

// Binary operators including `,`, `.*` and `->*`; Assign uses it too.
struct BinaryExpr : Expr
{
	TokenKind op;
	// Spelled as an alternative token (`and` stays `and` in the dump).
	bool alternative;
	Expr* left;
	Expr* right;
};

struct ConditionalExpr : Expr
{
	Expr* condition;
	Expr* then_expr;
	Expr* else_expr;
};

// Prefix operators; Postfix (`++`, `--`) uses it too.
struct UnaryExpr : Expr
{
	TokenKind op;
	bool alternative;
	Expr* operand;
};

struct CallExpr : Expr
{
	Expr* callee;
	Slice<Expr*> arguments;
};

// A function-style cast whose type the parser knows (5.2.3): simple type
// keywords (one in C++11; the course also accepts sequences such as
// `unsigned long(e)`), a named type or a decltype-specifier. `braced` holds
// T{...}.
struct FunctionalCastExpr : Expr
{
	Slice<DeclSpecifier> type;
	Slice<Expr*> arguments;
	BracedInitList* braced;
	// Semantics mode: a parenthesized expression-list that
	// aggregate-initializes the class (the course's direct aggregate
	// construction, as C++20's P0960), as the braced list the analysis made
	// of it; and the list the object is initialized from.
	mutable const BracedInitList* aggregate = nullptr;
	const BracedInitList* list() const { return braced != nullptr ? braced : aggregate; }
};

struct SubscriptExpr : Expr
{
	Expr* base;
	// An expression or braced-init-list.
	Expr* index;
};

struct MemberExpr : Expr
{
	// OP_DOT or OP_ARROW.
	TokenKind op;
	// For E1->E2 with E1 of class type, semantic analysis replaces E1 by
	// the call E1.operator->() it builds (13.5.6).
	Expr* base;
	Name* member;
};

struct CastExpr : Expr
{
	// KW_STATIC_CAST, KW_DYNAMIC_CAST, KW_REINTERPET_CAST, KW_CONST_CAST, or
	// OP_LPAREN for a C-style cast.
	TokenKind cast;
	TypeId* type;
	Expr* operand;
};

struct SizeofExpr : Expr
{
	TypeId* type;
	Expr* operand;
	// The operand was parenthesized: `sizeof(expr)`.
	bool parenthesized;
};

struct SizeofPackExpr : Expr
{
	const Identifier* pack;
};

// typeid, alignof and the noexcept operator.
struct TypeTraitExpr : Expr
{
	TokenKind keyword;
	TypeId* type;
	Expr* operand;
};

struct NewExpr : Expr
{
	bool global;
	bool has_placement;
	bool parenthesized_type;
	Slice<Expr*> placement;
	TypeId* type;
	Initializer* initializer;
	// Set by semantic analysis: the constructor that initializes the
	// allocated class object (each element of an array), if any (the
	// facts' entity is the allocation function), and an array's element
	// count, the first bound (null for a single object).
	mutable const sema::FunctionEntity* constructor = nullptr;
	mutable const Expr* count = nullptr;
	// The deallocation function freeing the storage when an element's
	// construction throws (5.3.4p19-p20), for an array of class objects.
	mutable const sema::FunctionEntity* deallocation = nullptr;
};

struct DeleteExpr : Expr
{
	bool global;
	bool array;
	Expr* operand;
};

struct LambdaCapture
{
	// TT identifier capture (by copy), OP_AMP (by reference) or KW_THIS.
	TokenKind kind;
	bool by_reference;
	bool pack;
	const Identifier* name;
};

struct LambdaExpr : Expr
{
	// OP_AMP or OP_ASS when has_default.
	TokenKind capture_default;
	bool has_default;
	bool has_declarator;
	bool is_mutable;
	bool has_noexcept;
	// A dynamic-exception-specification, its type-ids in throw_types.
	bool has_throw;
	Slice<LambdaCapture> captures;
	ParameterClause* parameters;
	Expr* noexcept_operand;
	Slice<TypeId*> throw_types;
	TypeId* trailing;
	CompoundStmt* body;
	// GNU attributes of the lambda-declarator (noinline, used, pure, const:
	// the call operator's).
	Attribute* attributes = nullptr;
	// The unit token ordinals of the parameter clause's `(` (the body's
	// when there is none) and of the body's `{`: the closure's LowIR name
	// (ProgramLowering::append_closure_label).
	uint32_t clause_token = 0;
	uint32_t body_token = 0;
	// C++20's template-parameter-list after the lambda-introducer (hosted
	// compilations): the call operator is a member function template.
	TemplateParameterList* template_parameters = nullptr;
	// An instantiation's copy whose parameter clause expanded function
	// parameter packs: the clause it copied, and each pack's name and
	// number of elements (see Analyzer::function_pack_scope).
	const ParameterClause* pattern_parameters = nullptr;
	Slice<const std::pair<const Identifier*, uint32_t>> parameter_packs =
		Slice<const std::pair<const Identifier*, uint32_t>>();
};

struct BracedInitList : Expr
{
	Slice<Expr*> elements;
	bool trailing_comma;
	// Designated initializers (hosted compilations): each element's
	// designator `.member`, null for one without; null when none has one.
	// The analysis rewrites a class's designated list into the equivalent
	// list in member order (see Analyzer::designated_list), recording a
	// union's designated member.
	const Identifier** designators = nullptr;
	mutable const sema::DataMember* union_member = nullptr;
};

struct PackExpansionExpr : Expr
{
	Expr* pattern;
};

// `( pattern op ... )`, `( ... op pattern )`, `( pattern op ... op init )`
// or `( init op ... op pattern )` (C++17's 5.1.6): an instantiation expands
// it into the operator's applications over the pack's elements (see
// syntax/ast_clone.cpp), so analysis meets it only in a template's
// definition.
struct FoldExpr : Expr
{
	TokenKind op;
	Expr* pattern;
	// The binary fold's other operand, else null.
	Expr* init;
	// The pattern is the left operand (a right fold: E1 op (... op En)).
	bool right;
};

struct ThrowExpr : Expr
{
	Expr* operand;
};

// A GNU type trait or transform: its type-ids (an instantiation's copy has
// its pack expansions expanded). The analysis also makes Declval operands
// of a semantic `type` (no type-ids) when it checks a trait.
struct BuiltinTraitExpr : Expr
{
	GnuTrait trait;
	Slice<TypeId*> types;
	const sema::Type* type;
	// IntegerPack: N.
	Expr* operand;
};

// A GNU statement expression `( compound-statement )` inside a function:
// the block's statements in order; when the last is an expression
// statement, its value (lvalue-to-rvalue, array and function decay; a
// class object copy-initialized) is the expression's prvalue, else it is
// void.
struct StatementExpr : Expr
{
	CompoundStmt* body;
};

} // namespace syntax
