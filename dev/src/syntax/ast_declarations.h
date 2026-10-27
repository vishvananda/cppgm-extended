#pragma once

#include "syntax/ast_statements.h"

// Syntax tree declarations and the translation unit (see syntax/ast.h).
namespace syntax {

enum class DeclKind : uint8_t
{
	Empty,
	Namespace,
	NamespaceAlias,
	UsingDirective,
	UsingDeclaration,
	Alias,
	ExplicitInstantiation,
	Template,
	Class,
	Enum,
	StaticAssert,
	SpecialMember,
	Function,
	Simple,
	Linkage,
	BitField,
	Access,
	Asm
};

struct Decl
{
	DeclKind kind;
	uint32_t location;
};

// A namespace-scope basic asm definition. Its data directives are lifted
// once by analysis; lowering consumes the resulting section bytes.
struct AsmDecl : Decl
{
	TokenSpelling text;
	// Non-null after analysis, including an empty definition.
	mutable const sema::AsmDataSection* sections = nullptr;
	mutable uint32_t section_count = 0;
};

struct NamespaceDecl : Decl
{
	const Identifier* name;
	bool is_inline;
	Slice<Decl*> declarations;
	// The kept attributes (GNU abi_tag) before or after the name.
	Attribute* attributes = nullptr;
};

struct NamespaceAliasDecl : Decl
{
	const Identifier* name;
	Name* target;
};

// using-directive and using-declaration.
struct UsingDecl : Decl
{
	Name* target;
	bool typename_keyword;
	// A using-declaration with Clang's using_if_exists attribute.
	bool if_exists;
};

struct AliasDecl : Decl
{
	const Identifier* name;
	TypeId* type;
	// The attributes after its identifier (GNU aligned among them).
	Attribute* attributes = nullptr;
	// The declaration semantic analysis made of it.
	mutable const sema::Binding* declared = nullptr;
};

struct ExplicitInstantiationDecl : Decl
{
	bool is_extern;
	Decl* declaration;
};

struct TemplateDecl : Decl
{
	TemplateParameterList* parameters;
	Decl* declaration;
};

// A class-specifier or elaborated class declaration standing alone.
struct ClassDecl : Decl
{
	DeclSpecifier specifier;
};

struct EnumDecl : Decl
{
	EnumSpecifier* enum_specifier;
};

struct StaticAssertDecl : Decl
{
	Expr* condition;
	bool has_message;
	// The message token is a user-defined string literal (2.14.8).
	bool user_defined_message;
	// A condition independent of its template arguments can share its
	// successful semantic check across instantiations. Failed or dependent
	// conditions retain the ordinary instantiation-time checks.
	mutable bool independent_condition = false;
	mutable bool condition_checked = false;
	TokenSpelling message;
};

struct MemInitializer
{
	Name* name;
	bool braced;
	bool pack;
	Slice<Expr*> arguments;
	BracedInitList* list;
	uint32_t location;
};

struct CtorInitializer
{
	Slice<MemInitializer> initializers;
	uint32_t location;
};

// compound-statement or function-try-block, with the ctor-initializer.
struct FunctionBody
{
	CtorInitializer* ctor_initializer;
	CompoundStmt* body;
	// A function-try-block's statement (15p1): `body` and the handlers;
	// null for an ordinary body.
	TryStmt* try_block;
	// The first call of a reserved `__builtin_` name no builtin registry
	// holds (hosted compilations; see Analyzer::define_function).
	const Identifier* reserved_call = nullptr;
};

struct InitDeclarator
{
	Declarator* declarator;
	Initializer* initializer;
	// The declaration semantic analysis made of it.
	mutable const sema::Binding* declared = nullptr;
	// The ordinals among the unit's tokens of the declarator's first token
	// and of the token after the init-declarator (a simple-declaration's;
	// 0 elsewhere).
	uint32_t first_token = 0;
	uint32_t end_token = 0;
};

struct SimpleDecl : Decl
{
	Slice<DeclSpecifier> specifiers;
	Slice<InitDeclarator> declarators;
	// The attributes before or among the decl-specifiers.
	Attribute* attributes = nullptr;
	// A structured binding declaration (C++17's 11.5, an extension here;
	// syntax/parser_declarations.cpp): the first declarator is the object
	// e's, `ref [a, b]` with the initializer (none in a range-based for,
	// whose for-range-declaration declares e), the others the names alone.
	bool decomposition = false;
};

struct FunctionDecl : Decl
{
	Slice<DeclSpecifier> specifiers;
	Declarator* declarator;
	FunctionBody body;
	Attribute* attributes = nullptr;
	// The declaration semantic analysis made of it.
	mutable const sema::Binding* declared = nullptr;
};

// A constructor, destructor or conversion function declared without a
// type (the grammar's special-member-declaration/-definition).
struct SpecialMemberDecl : Decl
{
	bool definition;
	// The member-function-specifiers (inline, virtual, explicit, constexpr,
	// friend, static) as keyword specifiers.
	Slice<DeclSpecifier> specifiers;
	Name* name;
	Declarator* declarator;
	Initializer* initializer;
	FunctionBody body;
	Attribute* attributes = nullptr;
	// The declaration semantic analysis made of it.
	mutable const sema::Binding* declared = nullptr;
};

struct LinkageDecl : Decl
{
	// The string-literal's contents without quotes ("C", "C++").
	TokenSpelling language;
	// `extern "C" { ... }` rather than a single declaration, which is
	// treated as if it had the extern specifier (7.5p7).
	bool braced;
	Slice<Decl*> declarations;
};

struct BitFieldDeclarator
{
	Declarator* declarator;
	Expr* width;
};

struct BitFieldDecl : Decl
{
	Slice<DeclSpecifier> specifiers;
	Slice<BitFieldDeclarator> declarators;
	Attribute* attributes = nullptr;
};

struct AccessDecl : Decl
{
	TokenKind access;
};

struct BaseSpecifier
{
	bool is_virtual;
	// `virtual` was written before the access-specifier.
	bool virtual_first;
	bool has_access;
	bool pack;
	TokenKind access;
	Name* name;
	uint32_t location;
	// Set by the analysis of a template definition: the base names a type
	// that depends on a template parameter (14.6.2p3: unqualified lookup
	// does not search it, in the template or in an instantiation, whose
	// copy of the specifier keeps the fact).
	mutable bool dependent = false;
};

struct ClassSpecifier
{
	TokenKind key;
	bool final;
	bool has_base_clause;
	Name* name;
	Slice<BaseSpecifier> bases;
	Slice<Decl*> members;
	uint32_t location;
	// Ordinals in the unit's token sequence of the class-key and of the
	// token after the closing brace (the course dump's unnamed-class names).
	uint64_t first_token;
	uint64_t end_token;
	// An anonymous union's unnamed object (9.5p5), in semantics mode.
	mutable const sema::Binding* storage = nullptr;
	// The attributes after the class-key (alignas), and the packing in
	// effect where the class is defined (#pragma pack; 0 for none).
	Attribute* attributes = nullptr;
	uint32_t pack = 0;
};

struct Enumerator
{
	const Identifier* name;
	Expr* value;
	uint32_t location;
};

struct EnumSpecifier
{
	// KW_CLASS or KW_STRUCT for a scoped enumeration.
	bool scoped;
	bool has_body;
	TokenKind key;
	// Null for an unnamed enumeration; qualified when it defines a member
	// enumeration out of its class (7.2p4).
	Name* name;
	TypeId* underlying;
	Slice<Enumerator> enumerators;
	// GNU abi_tag after the enum-key (pa28), the attributes kept there.
	Attribute* attributes;
	// The enumerator-list ends with a comma.
	bool trailing_comma;
	uint32_t location;
};

struct TranslationUnit
{
	Slice<Decl*> declarations;
};

} // namespace syntax
