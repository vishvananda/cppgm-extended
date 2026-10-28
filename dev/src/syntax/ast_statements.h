#pragma once

#include "syntax/ast_expressions.h"

// Syntax tree statements (see syntax/ast.h).
namespace syntax {

enum class StmtKind : uint8_t
{
	Compound,
	Expression,
	Declaration,
	If,
	Switch,
	While,
	Do,
	For,
	RangeFor,
	Case,
	Default,
	Labeled,
	Break,
	Continue,
	Goto,
	Return,
	Throw,
	Try,
	Ambiguous,
	Asm
};

struct Stmt
{
	StmtKind kind;
	// A compound statement the parser made of a C++17 selection statement's
	// init-statement and the statement (`{ init-statement if (...) ... }`,
	// see Parser::with_selection_init): no block of the source, its names
	// are the selection statement's (see Analyzer::compound).
	bool selection_init = false;
	uint32_t location;
};

// A compound's statement view and closing delimiter share one 16-byte
// record: the delimiter occupies the padding after the ordinary slice's
// count. Assignment of a new view keeps that immutable parsed location.
struct CompoundItems
{
	Stmt** data;
	uint32_t size;
	uint32_t closing_location = 0; // zero for a synthetic block
	Stmt** begin() const { return data; }
	Stmt** end() const { return data + size; }
	Stmt*& operator[](uint32_t index) const { return data[index]; }
	bool empty() const { return size == 0; }
	operator Slice<Stmt*>() const { return Slice<Stmt*>{ data, size }; }
	CompoundItems& operator=(Slice<Stmt*> view)
	{
		data = view.data;
		size = view.size;
		return *this;
	}
};

struct CompoundStmt : Stmt
{
	CompoundItems items;
};
static_assert(sizeof(CompoundItems) == sizeof(Slice<Stmt*>), "compound locations reuse slice padding");

struct ExpressionStmt : Stmt
{
	Expr* expression;
};

struct DeclarationStmt : Stmt
{
	Decl* declaration;
};

struct ConditionDeclaration
{
	Slice<DeclSpecifier> specifiers;
	Declarator* declarator;
	Initializer* initializer;
	// The declaration semantic analysis made of it, and the conversion of
	// the declared variable's value the condition applies (to bool, or the
	// promotion of a switch condition).
	mutable const sema::Binding* declared = nullptr;
	mutable const sema::Conversion* conversion = nullptr;
};

// An expression or a condition declaration (6.4).
struct Condition
{
	Expr* expression;
	ConditionDeclaration* declaration;
};

struct IfStmt : Stmt
{
	Condition condition;
	Stmt* then_branch;
	Stmt* else_branch;
	// `if constexpr` (C++17 6.4.1p2, a GNU extension in C++11 that hosted
	// headers use).
	bool is_constexpr;
	// Set by semantic analysis of an `if constexpr` outside a template
	// definition: its condition's value (1 false, 2 true). The branch it
	// does not select is discarded: neither analyzed (instantiated) nor
	// lowered.
	mutable uint8_t constexpr_value = 0;
};

// The branches of an if statement that analysis and lowering visit (null
// for none): an `if constexpr`'s discarded one is left out.
inline const Stmt* LiveThen(const IfStmt& stmt)
{
	return stmt.constexpr_value == 1 ? nullptr : stmt.then_branch;
}
inline const Stmt* LiveElse(const IfStmt& stmt)
{
	return stmt.constexpr_value == 2 ? nullptr : stmt.else_branch;
}

// switch and while.
struct ConditionalLoopStmt : Stmt
{
	Condition condition;
	Stmt* body;
};

struct DoStmt : Stmt
{
	Stmt* body;
	Expr* condition;
};

struct ForStmt : Stmt
{
	// for-init-statement: a simple-declaration or an optional expression.
	Decl* init_declaration;
	Expr* init_expression;
	Condition* condition;
	Expr* iteration;
	Stmt* body;
};

struct RangeForStmt : Stmt
{
	Slice<DeclSpecifier> specifiers;
	Declarator* declarator;
	// An expression or braced-init-list.
	Expr* range;
	Stmt* body;
	// Semantics mode: the statement's equivalent (6.5.4p1).
	mutable const sema::RangeForFacts* facts = nullptr;
};

struct CaseStmt : Stmt
{
	Expr* value;
	Stmt* body;
	// Semantics mode: the label's value converted to the switch's promoted
	// condition type (6.4.2p2).
	mutable uint64_t converted = 0;
};

// default: and identifier: labels.
struct LabeledStmt : Stmt
{
	const Identifier* label;
	Stmt* body;
};

// goto.
struct GotoStmt : Stmt
{
	const Identifier* label;
};

// A GNU asm statement's operand: `[ name ]`? constraint ( expression ).
struct AsmOperand
{
	const Identifier* name;
	TokenSpelling constraint;
	Expr* expression;
	uint32_t location;
};

// A GNU (extended) asm statement, `asm volatile? ( string-literal ( :
// outputs ( : inputs ( : clobbers )? )? )? ) ;` -- C++11's asm-definition
// (7.4) its simplest form: the template, operands and clobbers as written.
struct AsmStmt : Stmt
{
	TokenSpelling text;
	bool is_volatile;
	Slice<AsmOperand> outputs;
	Slice<AsmOperand> inputs;
	Slice<TokenSpelling> clobbers;
	// The template lifted to its operations (sema/asm_statements.h).
	mutable const sema::AsmProgram* program = nullptr;
};

// return and throw statements.
struct ValueStmt : Stmt
{
	Expr* value;
};

struct ExceptionDeclaration
{
	bool ellipsis;
	Slice<DeclSpecifier> specifiers;
	Declarator* declarator;
	// Semantics mode: the type the handler catches (15.3p2's adjustments
	// made) and the variable it declares (an unnamed class object's too;
	// null for other unnamed ones).
	mutable const sema::Type* type = nullptr;
	mutable const sema::Binding* declared = nullptr;
};

struct Handler
{
	ExceptionDeclaration exception;
	CompoundStmt* body;
};

struct TryStmt : Stmt
{
	CompoundStmt* body;
	Slice<Handler> handlers;
};

// A statement whose reading turns on the category of a name in a
// template-id's members that the template's specializations declare
// differently (the parser shares their members, see
// NameScopes::add_specialization), or of a name the parser can only guess
// (Parser::guessed_component): the readings that parse, the first
// taking the name with the category the parser found and each other one
// with another of its categories; the name (in the first) and its
// component; and the reading semantic analysis chose by looking the name up
// in the specialization the template-id selects, or, where the prefix
// depends on template parameters, the one taking it for a value (14.6p2),
// in the template's instantiations too (`dependent`).
enum class ReadingCategory : uint8_t
{
	Value,
	Type,
	Template,
	// The first reading of a name the parser could not classify (found in
	// no class it knows), taken as its context suggests.
	Unknown
};

struct AmbiguousStmt : Stmt
{
	// One per category a member name can have (Value, Type, TypeTemplate,
	// FunctionTemplate).
	static const uint32_t kMaxReadings = 4;
	Stmt* readings[kMaxReadings];
	ReadingCategory categories[kMaxReadings];
	uint8_t count;
	const Name* name;
	uint32_t component;
	mutable bool dependent = false;
	mutable uint8_t chosen = 0;

	const Stmt& reading() const { return *readings[chosen]; }
};

// A statement as semantic analysis read it.
inline const Stmt& ChosenReading(const Stmt& stmt)
{
	return stmt.kind == StmtKind::Ambiguous ? static_cast<const AmbiguousStmt&>(stmt).reading() : stmt;
}

// A GNU statement expression's last statement past its labels (as GCC
// reads `({ ... done: e; })`): the expression statement whose value it
// has, else null.
inline const ExpressionStmt* StatementExprValue(const StatementExpr& expr)
{
	const Slice<Stmt*>& items = expr.body->items;
	const Stmt* last = items.size != 0 ? &ChosenReading(*items[items.size - 1]) : nullptr;
	while (last != nullptr && last->kind == StmtKind::Labeled)
		last = &ChosenReading(*static_cast<const LabeledStmt*>(last)->body);
	return last != nullptr && last->kind == StmtKind::Expression ? static_cast<const ExpressionStmt*>(last) : nullptr;
}

// Break and Continue have no operands beyond the Stmt header.

} // namespace syntax
