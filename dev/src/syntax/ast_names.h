#pragma once

#include <cstdint>

#include "lex/token_kind.h"
#include "lex/token_spelling.h"

struct Identifier;

// Semantic records the analysis attaches to nodes (sema/expression_facts.h,
// sema/semantic_graph.h); the tree itself does not use them.
namespace sema {
struct AsmProgram;
struct Binding;
struct Conversion;
struct DataMember;
struct ExprFacts;
struct FunctionEntity;
struct RangeForFacts;
struct Type;
}

// Syntax tree names: slices, forward declarations and names as written
// (see syntax/ast.h).
namespace syntax {

template<typename T>
struct Slice
{
	T* data;
	uint32_t size;

	T* begin() const { return data; }
	T* end() const { return data + size; }
	T& operator[](uint32_t index) const { return data[index]; }
	bool empty() const { return size == 0; }
};

struct Expr;
struct TypeId;
struct Declarator;
struct Decl;
struct Stmt;
struct CompoundStmt;
struct ClassSpecifier;
struct EnumSpecifier;
struct BracedInitList;
struct ParameterDeclaration;
struct TemplateParameterList;

// A template-argument: a type-id or an assignment-expression (14.3p2
// resolves the ambiguity to the type-id), optionally a pack expansion.
struct TemplateArgument
{
	TypeId* type;
	Expr* expression;
	bool pack;
};

enum class NameComponentKind : uint8_t
{
	// An identifier, or a template-name with its argument list.
	Identifier,
	// operator-function-id (13.5), optionally with template arguments.
	Operator,
	// conversion-function-id (12.3.2).
	Conversion,
	// literal-operator-id (13.5.8): the ud-suffix identifier.
	LiteralOperator,
	// ~class-name (12.4) or ~decltype-specifier.
	Destructor,
	// A decltype-specifier as the root of a nested-name-specifier.
	Decltype
};

struct NameComponent
{
	NameComponentKind kind;
	// Preceded by the `template` keyword (14.2p4).
	bool template_keyword;
	// A template-id: followed by `<` template-argument-list? `>`.
	bool has_arguments;
	// Operator: the array forms `new[]` and `delete[]`.
	bool array_form;
	// Operator: the operator's token; OP_LPAREN for `()`, OP_LSQUARE for
	// `[]`, OP_RSHIFT for `>>`, KW_NEW/KW_DELETE for allocation functions.
	TokenKind op;
	// Operator: spelled as an alternative token (`and` for `&&`).
	bool alternative;
	// Destructor: of a decltype-specifier rather than a class name.
	bool destructor_decltype;
	// An instantiation's copy of a template-id whose argument list had a
	// pack expansion, now its elements (see SyntaxCloner::expanded_arguments).
	bool expanded;
	uint32_t location;
	union
	{
		// Identifier, LiteralOperator (the suffix), Destructor (the class
		// name).
		const Identifier* identifier;
		// Conversion: the conversion-type-id.
		TypeId* conversion;
		// Decltype, and Destructor of a decltype-specifier.
		Expr* decltype_operand;
	};
	Slice<TemplateArgument> arguments;
};

// An id-expression, nested-name-specifier-qualified type name, or other name
// as written: an optional leading `::` and components separated by `::`.
struct Name
{
	bool global;
	// Written as a typename-specifier (14.6p2).
	bool typename_keyword;
	Slice<NameComponent> components;
	uint32_t location;
	// Set by the analysis of a template definition: an id-expression whose
	// nested-name-specifier depends on a template parameter, assumed to
	// name no type (14.6p2); an instantiation where it names one is
	// ill-formed (14.6p6). A copy of the name keeps the fact.
	mutable bool dependent_qualifier = false;

	const NameComponent& last() const { return components[components.size - 1]; }
	bool qualified() const { return global || components.size > 1; }
};

} // namespace syntax
