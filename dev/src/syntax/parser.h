#pragma once

#include <cstdint>
#include <vector>

#include "syntax/parser_regions.h"

namespace syntax {

// The PA5 parser: recursive descent over the shared source grammar
// (pa5.gram), building the syntax tree directly while it tracks the name
// categories that choose between syntactic alternatives (parsing.md).
//
// Speculation is explicit and bounded: a Tentative region records the
// token position, the arena mark, the name-change log and the deferred-body
// queue, and a failed alternative restores all of them. Parse functions
// return null/false for a failed parse; outside a tentative region that is
// a syntax error of the translation unit. Member function bodies of a class
// are captured as token runs and parsed once, when the outermost enclosing
// class is complete (9.2p2).
class Parser : public ParserState
{
public:
	Parser(ParseTokenStream& tokens, IdentifierTable& identifiers, Arena& arena, bool hosted = false);

	// The translation unit, or null after a syntax error.
	TranslationUnit* parse_translation_unit();

private:
	enum class DeclContext : uint8_t
	{
		Namespace,
		Class,
		Block
	};

	enum class DeclaratorMode : uint8_t
	{
		// A declarator-id is required (declarations, conditions).
		Named,
		// A declarator-id is optional (parameters, exception declarations).
		Parameter,
		// No declarator-id (type-ids).
		Abstract,
		// new-type-id: ptr-operators and array bounds only.
		New,
		// conversion-type-id: ptr-operators only.
		Conversion
	};

	enum class NameMode : uint8_t
	{
		// An id-expression.
		Expression,
		// A declarator-id: operators, conversions and destructors allowed.
		Declarator,
		// After `.` or `->`: template-ids need `template` (14.2p4).
		Member,
		// A type name in a specifier position (no operator names).
		Type
	};

	// Where the prefix of a name being parsed was resolved.
	enum class PrefixState : uint8_t
	{
		Unqualified,
		Known,
		Dependent,
		Unknown
	};

	// What a parsed name denotes.
	struct NameInfo
	{
		NameCategory category;
		PrefixState prefix;
		// A known prefix that is a template-id whose arguments name no
		// template parameter: a specialization, whose members include its
		// dependent bases' (see ScopeEdge::dependent).
		bool prefix_specialization;
		// A known prefix that is a template-id whose arguments name one:
		// its members are dependent names (14.6.2.1) unless it is the
		// current instantiation.
		bool prefix_dependent;
		// The last component is a member a template-id prefix reaches
		// (`scope` the primary template's member's class).
		bool template_member;
		// prefix_approximate: a known prefix whose class is a guess (a
		// member a template-id reaches that a specialization declares too,
		// or a name through one, NameBinding::approximate): the category of
		// a member found in it is a guess too. prefix_uncertain: a dependent
		// prefix outside every template definition, a type whose class the
		// parser does not know (a typedef of a template parameter reached
		// through a template-id, an alias template of a member, a
		// decltype), so a member's category is unknown rather than a value
		// (14.6p2 is for dependent names). approximate, dependent: the
		// last component's binding's (NameBinding::approximate, dependent;
		// the latter where unqualified lookup found it).
		bool prefix_approximate, prefix_uncertain, approximate, dependent;
		// A known specialization prefix of a template whose bases name its
		// parameters (Scope::parameter_bases): its template arguments (a
		// template-id's or a typedef's record), for parameter_base_lookup;
		// unset otherwise. (Ordered to share the flags' padding.)
		uint32_t prefix_argument_count;
		Scope* scope;
		Scope* prefix_scope;
		const TemplateArgument* prefix_arguments;
	};

	// What a decl-specifier-seq contained.
	struct SpecifierInfo
	{
		bool has_type;
		bool has_typedef;
		bool has_friend;
		// A lone class or enum specifier (a class/enum declaration when no
		// declarator follows).
		bool defines_type;
		// The class, enumeration or namespace-like scope of the named type
		// (for typedefs of classes), if known; whether the type is a member a
		// template-id reaches (NameInfo::template_member).
		Scope* type_scope;
		bool type_template_member;
		// The type's name, when one named it (not a keyword or class
		// specifier), what its parse found and whether its template
		// arguments name a template parameter (see note_type_name).
		const Name* type_name;
		NameInfo type_name_info;
		bool type_mentions;
		// The specifiers are exactly the keyword `int` (see the literal
		// token form in ast.h).
		bool only_int;
		// The type specifiers so far are `signed` or `unsigned` alone, which
		// GCC's `__int128` (a type name here) may follow; a complex
		// specifier alone, which GCC's extended type names may follow
		// (`_Complex _Float16`).
		bool sign_only, complex_only;
	};

	// ----------------------------------------------------- declarations
	bool parse_declaration_seq(ListFrame<Decl*>& list, bool until_brace, DeclContext context);
	// `leading`: attributes a declaration statement began with.
	Decl* parse_declaration(DeclContext context, Attribute* leading = nullptr);
	Decl* parse_namespace();
	Decl* parse_using();
	Decl* parse_static_assert();
	Decl* parse_template_declaration(DeclContext context);
	Decl* parse_explicit_instantiation(DeclContext context);
	Decl* parse_linkage_specification(DeclContext context);
	Decl* parse_simple_or_function(DeclContext context);
	// False if the declaration is not a special member (nothing consumed);
	// otherwise `decl` is the declaration, or null after a syntax error.
	bool try_special_member(DeclContext context, Decl*& decl);
	Decl* parse_special_member(
		DeclContext context, Slice<DeclSpecifier> specifiers, Name* name, uint32_t start, Tentative& tentative);
	bool special_member_ahead(DeclContext context, Name*& name);
	Decl* finish_simple_declaration(
		DeclContext context, Slice<DeclSpecifier> specifiers, const SpecifierInfo& info, uint32_t start);
	Decl* parse_bit_field_rest(Slice<DeclSpecifier> specifiers, Declarator* first, uint32_t start);
	bool parse_decl_specifier_seq(ListFrame<DeclSpecifier>& list, SpecifierInfo& info, bool definite = false);
	bool parse_decl_specifiers(ListFrame<DeclSpecifier>& list, SpecifierInfo& info);
	void vector_specifiers(Attribute*& attributes, ListFrame<DeclSpecifier>& list);
	void leading_vector_specifiers(const Attribute* attributes, ListFrame<DeclSpecifier>& list);
	// An attribute-specifier-seq, alignas and GNU __attribute__, keeping
	// the attributes later stages use on `list` (the rest are dropped).
	bool gnu_attribute(const char* spelling, Attribute& attribute, bool& kept);
	bool collect_attributes(Attribute*& list) { return !at_attribute() || collect_attribute_specifiers(list); }
	bool collect_attribute_specifiers(Attribute*& list);
	bool parse_alignment_operand(Attribute& attribute);
	// GNU `__typeof__ ( type-id )` or `__typeof__ unary-expression` as a
	// decltype-specifier of a TypeofOperand (parse_typeof_operand).
	bool parse_typeof_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info);
	bool parse_type_specifier_seq(ListFrame<DeclSpecifier>& list, SpecifierInfo& info);
	bool parse_named_type_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info);
	bool parse_class_or_enum_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info);
	ClassSpecifier* parse_class_body(TokenKind key, Name* name, bool final, uint32_t start, Scope* member_of);
	void link_specialization(const Identifier* template_name, Scope* scope);
	void declare_class(const Identifier* name, bool is_template, Scope* scope);
	// A class declared before its definition gets its scope at once: names
	// bound to the class meanwhile (typedefs, alias and using-declarations)
	// keep that scope, which the definition reopens (see defined_later).
	Scope* class_scope_ahead(const Identifier* name);
	// The binding's scope is such a scope, its class not defined yet.
	bool defined_later(const NameBinding& binding, const Identifier* name) const;
	bool parse_base_clause(ListFrame<BaseSpecifier>& bases, Scope* class_scope, bool primary_template);
	EnumSpecifier* parse_enum_specifier();
	Initializer* parse_initializer(bool allow_parenthesized);
	void declare_declarator(const Declarator* declarator, const SpecifierInfo& info);
	bool parse_function_body(FunctionBody& body, bool allow_ctor_initializer);
	bool defer_function_body(FunctionBody& body, bool allow_ctor_initializer, Scope* scope);
	bool skip_function_body();
	bool parse_deferred_bodies(size_t first);
	void parse_deferred_lambdas(size_t first) override;
	CtorInitializer* parse_ctor_initializer();
	bool at_function_body_start(bool allow_ctor_initializer);
	bool parse_handlers(ListFrame<Handler>& handlers);

	// ------------------------------------------------------- declarators
	Declarator* parse_declarator(DeclaratorMode mode);
	bool parse_declarator_into(DeclaratorMode mode, Declarator* declarator);
	bool parse_ptr_operator(ListFrame<PtrOperator>& pointers);
	bool parse_declarator_suffixes(DeclaratorMode mode, Declarator* declarator, bool named, Scope* member_of);
	ParameterClause* parse_parameter_clause(Scope* member_of, bool definite);
	// A scope below the current one whose lookups search `member_of`'s
	// scope before the enclosing ones (for the names after a qualified
	// declarator-id); the current scope stays.
	Scope* member_lookup_scope(Scope* member_of);
	ParameterDeclaration* parse_parameter_declaration(bool definite);
	bool parse_function_qualifiers(ListFrame<FunctionQualifier>& qualifiers, Attribute** attributes);
	bool parse_throw_types(Slice<TypeId*>& out);
	// In a definite type context (an alias, a cast's target, a trailing
	// return type ...) an undeclared name is a type; where a type-id competes
	// with an expression (template arguments, sizeof, `(T)x`) it is not.
	// `specified`: what its type specifiers named (SpecifierInfo).
	TypeId* parse_type_id(
		DeclaratorMode mode = DeclaratorMode::Abstract, bool definite = true, SpecifierInfo* specified = nullptr);
	// A type-id whose leading type name was already parsed (see
	// name_starts_here); the name is not parsed twice when the
	// alternative is an expression.
	TypeId* parse_type_id_from_name(Name* name, const NameInfo& name_info, uint32_t start);
	TypeId* finish_type_id(TypeId* type, const SpecifierInfo& info, DeclaratorMode mode);
	bool continue_type_specifier_seq(ListFrame<DeclSpecifier>& list, SpecifierInfo& info);
	bool nested_declarator_ahead(DeclaratorMode mode);
	bool ptr_operator_ahead(uint32_t ahead);
	bool member_pointer_ahead(uint32_t ahead);
	static bool declarator_is_empty(const Declarator* declarator);
	static const Name* declarator_name(const Declarator* declarator);
	static bool is_function_declarator(const Declarator* declarator);

	// ------------------------------------------------------------- names
	Name* parse_name(NameMode mode, NameInfo& info);
	bool parse_name_component(NameMode mode, ListFrame<NameComponent>& components, NameInfo& info, bool& more);
	// `member_of`: the class a qualified conversion-function-id's type is
	// looked up in first.
	bool parse_operator_name(NameComponent& component, Scope* member_of = nullptr);
	bool parse_template_arguments(Slice<TemplateArgument>& arguments);
	bool parse_template_argument(ListFrame<TemplateArgument>& arguments);
	enum class TemplateIdChoice : uint8_t
	{
		No,
		Yes,
		Maybe,
		// A member name after `.`/`->`: a template-id if the arguments parse
		// and a call or `::` follows.
		MaybeCall
	};
	TemplateIdChoice template_id_follows(
		const Identifier* identifier, const NameInfo& info, NameMode mode, bool template_keyword);
	// `declared`: the name declares or defines its last component (a
	// declarator-id, a class-head), whose prefix names the class or
	// namespace it is a member of, whatever its template arguments.
	void advance_prefix(
		const NameComponent& component, NameInfo& info, bool dependent_arguments = true, bool declared = false);
	uint8_t prefix_guesses(
		const NameComponent& component, const NameInfo& info, const NameBinding& binding, bool declared);
	NameBinding resolve_component(const Identifier* identifier, const NameInfo& info, bool prefix_only);
	NameBinding guessed_component(NameBinding binding);
	void note_name_conflict(uint32_t conflicts, uint32_t component);
	// For a typedef-name's or alias's binding of a type specifier's name
	// (one of a class whose scope is known): whether the class it records
	// is a guess and whether the type depends on template parameters
	// (NameBinding::approximate and dependent); notes a name whose class is
	// unknown or a guess (NameScopes::note_uncertain).
	void note_type_name(const SpecifierInfo& info, NameBinding& binding, bool alias_template);
	Name* parse_name_components(NameMode mode, NameInfo& info);
	bool is_type(const Name* name, const NameInfo& info);
	bool names_type_hint(const Identifier* identifier) const;
	bool at_type_specifier_start(uint32_t ahead = 0);
	Expr* parse_decltype_operand();
	// An identifier or `::identifier` starts a name here that parse_name
	// accepts; ambiguous contexts parse it once and continue as a type-id or
	// an expression.
	bool name_starts_here() { return at_name_identifier() || (at(TokenKind::OP_COLON2) && at_identifier(1)); }
	bool template_list_failed_before(const TemplateListKey& key) const;
	// An undeclared name's `<` tried as a template argument list (with
	// `call_follows`, only if a call or `::` follows it) where no enclosing
	// speculative list is open; nested ones abandon all open lists on
	// failure.
	bool try_outermost_template_arguments(
		Slice<TemplateArgument>& arguments, bool call_follows, const TemplateListKey& key);
	// Whether a member name's tentative arguments can be a member
	// template's: none is a bare logical-or or logical-and expression.
	static bool member_template_arguments_plausible(const Slice<TemplateArgument>& arguments);
	// Fails every open speculative list (abandoning_lists_); the caller
	// returns failure.
	void abandon_template_lists();
	void remember_failed_template_lists(const TemplateListKey* keys, size_t count);

	// --------------------------------------------------------- templates
	TemplateParameterList* parse_template_parameter_clause();
	TemplateParameter* parse_template_parameter();

	// -------------------------------------------------------- statements
	Stmt* parse_statement();
	CompoundStmt* parse_compound_statement(bool new_scope);
	Stmt* parse_declaration_or_expression_statement(Attribute* attributes);
	typedef Stmt* (Parser::*StatementReader)(Attribute* attributes);
	Stmt* read_statement(StatementReader reader, Attribute* attributes);
	Stmt* parse_value_statement(Attribute* attributes);
	// A selection or iteration statement (read whole, see read_statement).
	Stmt* parse_asm();
	bool at_structured_binding();
	Declarator* binding_declarator(const Identifier* name, TokenKind ref, uint32_t location);
	bool parse_binding_names(TokenKind& ref, const Identifier*& hidden, std::vector<Declarator*>& names);
	Slice<InitDeclarator> binding_declarators(
		const std::vector<Declarator*>& names, const SpecifierInfo& info, const InitDeclarator& object);
	Decl* parse_structured_binding(Slice<DeclSpecifier> specifiers, const SpecifierInfo& info, uint32_t start);
	bool at_deduction_guide();
	Decl* parse_deduction_guide(uint32_t start);
	bool at_fold_operator();
	Expr* parse_fold_rest(uint32_t start, Expr* left);
	Expr* parse_left_fold(uint32_t start);
	Stmt* parse_expression_statement();
	bool statement_starts_declaration(bool& tentative);
	Stmt* parse_statement_reading(Attribute* attributes);
	Stmt* parse_control_reading(Attribute* attributes);
	bool parse_condition(Condition& condition);
	bool parse_selection_head(Stmt*& init, Condition& condition);
	Stmt* with_selection_init(Stmt* init, Stmt* stmt, uint32_t start);
	Stmt* parse_if();
	bool at_coroutine_keyword(const Identifier* keyword)
	{
		return hosted_ && at_identifier(keyword) && coroutine_operator(keyword);
	}
	bool coroutine_operator(const Identifier* keyword);
	Expr* coroutine_operation(const Identifier* keyword, Expr* operand, uint32_t location);
	Stmt* parse_loop_or_switch(StmtKind kind);
	Stmt* parse_do();
	Stmt* parse_for();
	Stmt* parse_try();
	bool parse_handler(Handler& handler);

	// ------------------------------------------------------- expressions
	Expr* parse_expression();
	Expr* parse_assignment_expression();
	Expr* parse_initializer_clause();
	Expr* parse_binary(int min_precedence);
	Expr* parse_binary_rest(Expr* left, int min_precedence);
	Expr* finish_assignment_expression(Expr* left);
	Expr* finish_expression(Expr* left);
	// Continuations of an expression whose primary expression is parsed.
	Expr* continue_assignment_expression(Expr* primary);
	Expr* continue_expression(Expr* primary);
	Expr* primary_from_name(Name* name, NameInfo& info, uint32_t start);
	Expr* parenthesized_from_name(uint32_t start);
	Expr* parse_cast_expression();
	Expr* parse_unary_expression();
	Expr* parse_postfix_expression();
	Expr* parse_postfix_suffixes(Expr* expr);
	Expr* parse_primary_expression();
	Expr* parse_parenthesized_or_cast();
	bool at_compound_literal(const TypeId* type);
	Expr* compound_literal(const TypeId* type, uint32_t start);
	Expr* parse_id_expression_or_cast();
	Expr* parse_keyword_functional_cast();
	Expr* parse_sizeof();
	// sizeof's operand after the keyword: `( type-id )` (type) or a
	// unary-expression (operand, without the parentheses of a
	// parenthesized one); `function_types`: `( T() )` is the function type
	// (8.2p2), as __typeof__ reads it, not sizeof's functional cast.
	bool parse_sizeof_operand(TypeId*& type, Expr*& operand, bool& parenthesized, bool function_types = false);
	// At GNU `__typeof__`: its operand, read as sizeof's (as GCC does), in
	// a TypeofOperand node (the type-id's type, or a prvalue of the
	// expression's type).
	Expr* parse_typeof_operand();
	// At a GNU decltype-like specifier (at_gnu_decltype): its operand node.
	Expr* parse_gnu_decltype_operand();
	Expr* parse_type_trait(TokenKind keyword);
	// A GNU type trait `__is_class ( type-id )` ...
	Expr* parse_builtin_trait(GnuTrait trait);
	// `__underlying_type ( type-id )` as a decltype-specifier.
	bool parse_transform_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info, GnuTrait trait);
	Expr* parse_keyword_cast();
	Expr* parse_new();
	Expr* parse_delete();
	Expr* parse_lambda();
	BracedInitList* parse_braced_init_list();
	bool parse_expression_list(ListFrame<Expr*>& list, TokenKind close);
	bool at_cast_operand_start();
	int binary_precedence(TokenKind& op, uint32_t& width);

	// The function body being parsed: its first call of a reserved builtin
	// name no registry holds (FunctionBody::reserved_call).
	const Identifier* reserved_call_ = nullptr;
	// Absolute token positions (sorted) where a selection statement's init-statement
	// tried as a declaration failed (see parse_selection_head): an
	// enclosing attempt's rewind parses it again without the attempt, so
	// nested heads do not double the work per level.
	std::vector<uint64_t> failed_selection_inits_;
};

} // namespace syntax
