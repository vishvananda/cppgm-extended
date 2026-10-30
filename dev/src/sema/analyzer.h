#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "sema/analysis_records.h"
#include "sema/analyzer_keys.h"
#include "sema/constant_frames.h"
#include "sema/constant_values.h"
#include "sema/conversions.h"
#include "sema/declaration_records.h"
#include "sema/expression_facts.h"
#include "sema/integral_arithmetic.h"
#include "sema/semantic_unit.h"
#include "sema/string_literals.h"
#include "sema/template_records.h"
#include "support/flat_map.h"
#include "support/pointer_map.h"
#include "support/small_list.h"
#include "support/stack_budget.h"
#include "syntax/ast.h"
#include "syntax/ast_clone.h"

class SourcePositions;

class IdentifierTable;
struct NumberLiteral;
struct CharacterLiteralValue;

namespace sema {

struct FixedFacts;

// What the analysis establishes: PA6's declarations with expression names
// checked (--emit-types), or also the typed expressions, conversions,
// calls and statement rules of PA7 (--emit-semantics and later phases).
enum class AnalysisMode : uint8_t
{
	Types,
	Semantics
};

// Builds the semantic graph of one translation unit from its syntax tree,
// in source order: scopes and bindings as declarations are met, types from
// specifiers and declarators, integral constants where the language needs
// them. Member function bodies wait for their outermost class. Violations
// of the PA6 rules throw SemanticError; in a substitution whose failure is
// a result, those it can fail by are recorded instead (soft_substitution_).
class Analyzer
{
public:
	Analyzer(SemanticGraph& graph, IdentifierTable& identifiers, AnalysisMode mode = AnalysisMode::Types);
	~Analyzer();

	void analyze(const syntax::TranslationUnit& unit);
	// Hosted compilations take GNU's zero-length and flexible array
	// members (see array_bound and member_size).
	void set_hosted(bool on) { hosted_ = on; }
	bool hosted() const { return hosted_; }
	// -std=c++14 or later (hosted -c): the C++11 rules its headers' mode
	// gates contradict are dropped -- a constexpr non-static member function
	// is not implicitly const (7.1.5p8 in C++11) and may return void (N3652).
	void set_cxx14(bool on) { cxx14_ = on; }
	// -fno-exceptions (hosted -c): allocation and deallocation functions'
	// redeclarations are not compared (check_redeclared_exceptions).
	void set_exceptions(bool on) { exceptions_ = on; }
	// Hosted -Wno-narrowing suppresses the diagnostic outside SFINAE.
	void set_ignore_narrowing(bool on) { ignore_narrowing_ = on; }
	// Where the source-location builtins find the presumed file and line
	// of a location (none: an empty name and line 0).
	void set_source_positions(const SourcePositions* positions) { positions_ = positions; }
	const SourcePositions* source_positions() const { return positions_; }
	uint64_t source_line(uint32_t location) const;
	std::string source_file(uint32_t location) const;
	// A string literal of the analysis's own holding `text` (a source
	// location builtin's value in a constant evaluation); one per text.
	const syntax::LiteralExpr* text_literal(const std::string& text);
	// Itanium 2.2's nearly empty classes and 2.4 II.3's primary base of a
	// class (`is_virtual`: a nearly empty virtual one): layouts are built
	// from them and tables lay out their rows by them
	// (sema/primary_bases.cpp). Each is computed once per
	// class, from its bases alone (a class being defined has its primary
	// base once its base-clause is known).
	bool itanium_nearly_empty(const ClassEntity* cls);
	const ClassEntity* itanium_primary(const ClassEntity* cls, bool& is_virtual);
	// Per slot of a class's table, the virtual primary base whose subobject
	// the slot's function belongs to (the deepest on the class's primary
	// chain at or above the class declaring the function nearest it), null
	// for none; null for a chain without a virtual primary base.
	const ClassEntity* const* slot_levels(const ClassEntity* cls);
	// The unit's classes with a table group (VirtualFacts::group), in
	// completion order: the thunks their tables need for a virtual function
	// are defined with its definition.
	const std::vector<const ClassEntity*>& grouped_classes() const { return grouped_classes_; }

	// Facts later phases demand of an analyzed unit. The size and alignment
	// of a complete object type, classes laid out once on first demand
	// (false for an incomplete type), and a data member's byte offset.
	bool object_layout(const Type* type, uint64_t& size, uint64_t& alignment);
	bool member_offset(const VariableEntity* member, uint64_t& offset);
	// Semantics mode: a complete class's implicitly declared copy and move
	// members (ClassEntity::copy_constructor and the others), declared on
	// first demand; every reader of them demands them first.
	void implicit_copy_members(const ClassEntity* entity);
	// A polymorphic class's virtual table: each slot's final overrider
	// (10.3p2), built once on first demand (null for none).
	const VirtualSlot* virtual_slots(const ClassEntity* entity);
	// The value of an integral constant expression analyzed in `scope`
	// (5.19); false when it is not one.
	bool integral_constant(const syntax::Expr& expr, Scope* scope, Constant& value);
	// As integral_constant, a 128-bit type's value beyond the 64-bit
	// constants included: `high` is its upper half (of another type's
	// value, its lower half's extension by the type's signedness).
	bool integral_constant(const syntax::Expr& expr, Scope* scope, Constant& value, uint64_t& high);
	// The value of a variable's initialization when it is a constant
	// expression (3.6.2p2: constant initialization), published in the
	// semantic arena; null when it is not one.
	const ConstValue* constant_initialization(const VariableEntity* variable, const syntax::Initializer* initializer);
	// Whether a function's exception-specification is non-throwing
	// (15.4p12), a noexcept-specification's expression evaluated on first
	// need.
	bool function_nothrow(const FunctionEntity* function);
	// Whether a call's callee designates its function through a pointer,
	// reference or pointer to member declared non-throwing (15.4p2, p12:
	// see note_nothrow_target): such a call does not throw (5.3.7p3).
	bool callee_nothrow(const syntax::Expr& callee) { return nothrow_target(callee); }
	// The value constant evaluation gave a variable's initialization (PA16:
	// a constexpr object's complete value, a reference's referent, a const
	// integral object's value, published on first request), in the
	// semantic arena; null when it has none.
	const ConstValue* evaluated_value(const VariableEntity* variable);
	// A constexpr reference whose initializer selects its referent by a
	// conditional expression (5.16): the course reads it through its
	// storage and defines the functions its initializer odr-uses (see
	// record_constant_initializer).
	bool read_through_storage(const VariableEntity* variable) const;
	// The course's integral_constant convention: whether a condition
	// converted to bool by a conversion function has a value known without
	// evaluating it (its value then); asked by the lowering, when a
	// specialization's member body has been instantiated.
	bool constant_conversion(const syntax::Expr& condition, bool& value) const;
	// The functions template arguments designate (3.2p2) in the definition
	// of `context` (null: outside any function body), which no lowered code
	// names: a list from argument_demand_head (0 ends it) through `previous`.
	struct ArgumentDemand
	{
		const FunctionEntity* function;
		uint32_t previous;
	};
	uint32_t argument_demand_head(const FunctionEntity* context) const;
	const ArgumentDemand& argument_demand(uint32_t index) const { return argument_demands_[index - 1]; }
	// Templates. A class's definition, instantiated first if it is
	// a specialization or member instance not yet defined (14.7.1p1);
	// whether the class is complete.
	bool require_complete(const ClassEntity* entity)
	{
		return entity != nullptr && (entity->complete || complete_class(entity));
	}
	bool complete_class(const ClassEntity* entity);
	// An instantiated function's definition (14.7.1p2-p3, on the
	// lowering's demand): its template's body copied and analyzed once;
	// false when the template has no definition.
	bool instantiate_definition(const FunctionEntity* function);
	FunctionEntity* inherited_specialization(
		ClassEntity* entity, FunctionEntity* specialization, uint32_t arguments, uint32_t location);
	void planned_inheriting_constructor(FunctionEntity* inheriting, ClassEntity* entity, uint32_t location);
	bool plan_nothrow(const FunctionEntity* constructor);
	void check_inheriting_access(const FunctionEntity* constructor, const ClassEntity* entity, uint32_t location);
	static uint32_t definition_context(const FunctionEntity* definition);
	static uint32_t class_definition_context(const Scope* scope);
	// An instance's static data member's definition (14.7.1p1), on demand:
	// false when its pattern has none.
	bool instantiate_variable(const VariableEntity* variable);
	// A function's default argument, instantiated first if it is a
	// template's or a class template instance member's (14.7.1p12).
	const syntax::Expr* default_argument(const FunctionEntity* function, uint32_t index);
	// Instances' static data members named where evaluated (in the order
	// named), which the lowering defines.
	const std::vector<const VariableEntity*>& demanded_variables() const { return demanded_variables_; }
	// A qualified call through `(*this)` whose naming class (11.2p5) is
	// neither the object's class nor the called member's: that class (the
	// lowering converts the object through it); null otherwise.
	const ClassEntity* call_naming_class(const syntax::CallExpr& call) const
	{
		const uint32_t index = naming_index_.find(&call);
		return index != PointerMap::kMissing ? namings_[index] : nullptr;
	}
	// A non-static member access (explicit, or through `(*this)`) whose
	// naming class is neither the object's class nor the member's: that
	// class, through which the object converts (11.2p6, a repeated base's
	// subobject chosen by it); null otherwise.
	const ClassEntity* member_naming_class(const syntax::Expr& access) const
	{
		const uint32_t index = naming_index_.find(&access);
		return index != PointerMap::kMissing ? namings_[index] : nullptr;
	}
	// `cls`'s non-virtual base-specifier naming `base`, or null. A
	// well-formed conversion to a direct base goes through it (any other
	// path would reach a second subobject, 10.2), so the base walks take
	// it before searching the other bases.
	const BaseClass* direct_base(const ClassEntity* cls, const ClassEntity* base);
	// `cls`'s base-specifier naming `base` (virtual or not), or null.
	const BaseClass* base_specifier(const ClassEntity* cls, const ClassEntity* base);
	// An alias stand-in's specialization (CWG 1558, see
	// ClassTemplateFacts::alias_stand_in): the type it aliases, which the
	// ABI names; null for another type.
	const Type* aliased_of(const Type* type) const;
	// Whether a type depends on a template parameter (14.6.2.1p7), as the
	// ABI asks of a function template's non-type parameter's type.
	bool dependent(const Type* type) const { return dependent_type(type); }
	// How many of a dependent specialization's arguments an unresolved
	// name's qualifier spells (see class_templates.cpp).
	uint32_t qualifier_arguments(const TemplateHead* head, const Type* const* arguments, uint32_t count);
	// The member functions explicit instantiation definitions name (14.7.2),
	// which the lowering emits as object roots.
	const std::vector<const FunctionEntity*>& explicit_instantiations() const { return explicit_instantiations_; }
	// The class template specializations (and their member classes)
	// explicit instantiation definitions name: their units hold their
	// table groups, VTTs and type information (Itanium 5.2.3).
	const std::vector<const ClassEntity*>& explicit_class_instantiations() const
	{
		return explicit_class_instantiations_;
	}
	// Whether an instantiated function's implicit instantiation is
	// suppressed by an explicit instantiation declaration (14.7.2p10): the
	// unit refers to it without defining it.
	static bool InstantiationSuppressed(const FunctionEntity* function);
	static bool Excluded(const FunctionEntity* function);
	static bool InstantiationSuppressed(const VariableEntity* variable);

private:
	friend class ConstantEvaluator;
	friend class ConstantInterpreter;

	// ------------------------------------------------ declarations
	void declarations(const syntax::Slice<syntax::Decl*>& list, Scope* scope);
	void declaration(const syntax::Decl& decl, Scope* scope);
	void asm_declaration(const syntax::AsmDecl& decl, Scope* scope);
	void namespace_definition(const syntax::NamespaceDecl& decl, Scope* scope);
	NamespaceEntity* namespace_target(const syntax::Name& name, Scope* scope);
	void namespace_alias(const syntax::NamespaceAliasDecl& decl, Scope* scope);
	void using_directive(const syntax::UsingDecl& decl, Scope* scope);
	void using_declaration(const syntax::UsingDecl& decl, Scope* scope);
	void introduce(Binding* target, Scope* scope, uint32_t location);
	void introduce_functions(const std::vector<Binding*>& functions, Scope* scope, uint32_t location);
	void check_template_parameter_name(Scope* scope, const Identifier* name, uint32_t location);
	void alias_declaration(const syntax::AliasDecl& decl, Scope* scope);
	void template_declaration(const syntax::TemplateDecl& decl, Scope* scope);
	void explicit_instantiation(const syntax::ExplicitInstantiationDecl& decl, Scope* scope);
	static bool FriendDeclaration(const syntax::Decl& decl);
	void template_parameters(const syntax::TemplateParameterList& list, Scope* scope, bool declare,
		std::vector<TemplateParameterEntity*>* out = nullptr, int32_t base_depth = -1);
	const Type* non_type_parameter_type(const syntax::TemplateParameter& parameter, Scope* scope);
	static bool TemplateParameterPack(const syntax::TemplateParameter& parameter);
	void static_assertion(const syntax::StaticAssertDecl& decl, Scope* scope);

	// ------------- explicit specializations (template_specializations.cpp)
	void explicit_specialization(const syntax::TemplateDecl& decl, Scope* scope);
	ClassEntity* specialized_class(const syntax::Name& name, TokenKind key, Scope* scope);
	Entity* class_explicit_specialization(const syntax::ClassSpecifier& specifier, Scope* scope, Specifiers& out);
	Binding* explicit_function_specialization(const Specifiers& specifiers, const DeclaratorInfo& info, Scope* scope,
		bool definition, uint32_t location, bool deleted);
	FunctionEntity* deduce_for_declaration(
		FunctionEntity* primary, const TypeList& explicit_arguments, const Type* type);
	static uint32_t SpecializedEnclosing(const Scope* qualifier);
	const Type* declaration_pattern(const Type* pattern, const Type* declared, uint32_t depth);
	Binding* member_template_specialization(
		const DeclaratorInfo& info, Scope* scope, bool definition, uint32_t location);
	const Identifier* specialization_key(const syntax::Name& name);
	bool explicit_variable_specialization(VariableEntity* variable, bool has_initializer, uint32_t location);
	FunctionEntity* declared_specialization(const DeclaratorInfo& info, Scope* scope, uint32_t location);
	void explicit_function_instantiation(const syntax::ExplicitInstantiationDecl& decl, Scope* scope);
	void explicit_variable_instantiation(const syntax::ExplicitInstantiationDecl& decl, const DeclaratorInfo& info,
		const Specifiers& specifiers, const Binding* found, Scope* scope);
	FunctionEntity* instantiated_function(const DeclaratorInfo& info, Scope* scope, uint32_t location);
	ClassEntity* partial_specialization(const syntax::Name& name, TokenKind key, Scope* scope);
	void check_partial_arguments(const TemplateHead* primary, const TemplateHead* head, const Type* const* arguments,
		uint32_t count, uint32_t location);
	void deduced_parameters(const Type* const* types, uint32_t count, bool template_arguments, uint32_t depth,
		SmallList<uint8_t, 8>& deduced);
	void deduced_parameters(const Type* type, uint32_t depth, SmallList<uint8_t, 8>& deduced);
	bool partial_index_key(const TemplateHead* head, const Type* const* arguments, uint32_t count, const Type** key);
	Entity* index_partial(Entity* templ, Entity* partial, const Type* const* key, Entity* first, uint64_t& positions);
	void indexed_partials(const Entity* templ, uint64_t positions, const Type* const* arguments, uint32_t count,
		SmallList<Entity*, 4>& lists);
	ClassEntity* partial_of_arguments(ClassEntity* templ, const Type* const* arguments, uint32_t count);
	Entity* class_partial_specialization(const syntax::ClassSpecifier& specifier, Scope* scope, Specifiers& out);
	ClassEntity* member_partial_pattern(const syntax::ClassSpecifier& specifier);
	ClassEntity* matching_partial(ClassEntity* templ, const Type* const* arguments, uint32_t count, TypeList& deduced,
		const ClassEntity* instance = nullptr);
	bool deduce_partial(const ClassEntity* partial, const Type* const* arguments, uint32_t count, TypeList& deduced,
		const ClassEntity* instance = nullptr);
	bool deduce_partial_arguments(const TemplateHead* head, const Type* const* pattern, uint32_t pattern_count,
		const Type* const* arguments, uint32_t count, TypeList& deduced);
	bool partial_at_least_as_specialized(const ClassEntity* first, const ClassEntity* second);
	bool partial_ordered(const Entity* first, const TemplateHead* head, const Type* const* pattern, uint32_t count,
		const Entity* second, const TemplateHead* other, const Type* const* other_pattern, uint32_t other_count);
	bool arguments_at_least_as_specialized(const TemplateHead* head, const Type* const* pattern, uint32_t count,
		const TemplateHead* other, const Type* const* other_pattern, uint32_t other_count);
	bool same_ordering_type(const Type* first, const Type* second);

	// ----------------------------- variable templates (variable_templates.cpp)
	static VariableEntity* variable_template_of(const Binding* binding);
	void declare_variable_template(VariableEntity* variable, const syntax::Initializer* initializer, Scope* scope,
		Scope* lookup, uint32_t location);
	VariableEntity* named_variable_template(const syntax::Name& name, Scope* scope);
	Binding* variable_template_binding(const Binding* found, const syntax::NameComponent& component, Scope* scope);
	VariableEntity* variable_specialization(
		VariableEntity* templ, const TypeList& given, uint32_t at, bool declared = false);
	const VariableEntity* matched_partial(const VariableEntity* templ, const Type* const* arguments, uint32_t count,
		TypeList& deduced, uint32_t location);
	const Type* auto_variable_type(const VariableEntity* variable, const Specifiers& specifiers, const Type* declared,
		const syntax::Initializer* initializer, Scope* scope, uint32_t location);
	void variable_partial_specialization(const Specifiers& specifiers, const DeclaratorInfo& info,
		const syntax::Initializer* initializer, Scope* scope, uint32_t location);
	void variable_explicit_specialization(const Specifiers& specifiers, const DeclaratorInfo& info,
		const syntax::Initializer* initializer, Scope* scope, uint32_t location);
	bool instantiate_variable_specialization(VariableEntity* variable);
	void initialize_specialization(
		VariableEntity* variable, const syntax::Initializer* initializer, Scope* scope, uint32_t location);
	bool variable_partial_ordered(const VariableEntity* first, const VariableEntity* second);

	// ------------------------------------------ simple declarations
	void simple_declaration(const syntax::SimpleDecl& decl, Scope* scope);
	void declare_declarator(
		const Specifiers& specifiers, const syntax::InitDeclarator& init, Scope* scope, uint32_t location);
	void request_alignment(VariableEntity* variable, uint64_t alignment, const syntax::Attribute* specifier);
	void overlapping_attribute(
		VariableEntity* variable, const syntax::Attribute* leading, const syntax::Declarator* declarator);
	bool holds_auto(const Type* type) const;
	const Type* deduced_type(const Specifiers& specifiers, const Type* declared, const syntax::Initializer* initializer,
		Scope* scope, uint32_t location);
	const Type* replaced_auto(const Type* declared, const Type* type, uint8_t cv);
	Binding* declare_typedef(const DeclaredName& name, const Type* type, Scope* scope, uint32_t location);
	// `existing` is the predeclared name of one of GCC's extended floating
	// types that the host compiler may not predeclare (Clang has neither
	// _Float32 ... _Float64x nor _Float128 in C++), whose host library then
	// declares it: glibc's bits/floatn.h, `typedef __float128 _Float128;`.
	bool predeclared_extended_float(const Binding* existing, const Scope* where) const;
	Binding* declare_variable(const Specifiers& specifiers, const DeclaratorInfo& info,
		const syntax::Initializer* initializer, Scope* scope, uint32_t location, bool structured_binding = false);
	void check_variable_specifiers(const Specifiers& specifiers, const Type* type, const Scope* where, bool qualified,
		bool member, bool definition, bool initialized, uint32_t location);
	VariableEntity* earlier_variable(const Specifiers& specifiers, const DeclaredName& name, Scope* where,
		bool qualified, bool initialized, bool& definition, uint32_t location);
	VariableEntity* new_variable(const Specifiers& specifiers, const char* asm_label, const DeclaredName& name,
		Scope* where, const Type* type, bool is_extern, bool member, bool initialized, uint32_t location,
		bool structured_binding = false);
	void add_data_member(ClassEntity* entity, Entity* variable, uint64_t width, bool bit_field);
	void note_user_provided(const FunctionEntity* function, const syntax::Initializer* initializer);
	Binding* declare_function(const Specifiers& specifiers, const DeclaratorInfo& info, Scope* scope, bool definition,
		uint32_t location, bool deleted = false);
	Binding* qualified_friend(
		const Specifiers& specifiers, const DeclaratorInfo& info, Scope* scope, uint32_t location);
	FunctionEntity* earlier_function(const DeclaredName& name, Scope* where, const Type* signature, bool templated,
		bool& invisible, uint32_t location);
	void redeclare_function(FunctionEntity* function, const Specifiers& specifiers, const DeclaratorInfo& info,
		Scope* scope, const Scope* where, bool definition, bool deleted, bool exceptions, uint32_t location);
	FunctionEntity* new_function(const Specifiers& specifiers, const DeclaratorInfo& info, const DeclaredName& name,
		Scope* scope, Scope* where, const Type* signature, bool templated, uint32_t location);
	const Type* signature_of(const Type* function);
	Binding* friend_specialization(const DeclaratorInfo& info, Scope* scope, uint32_t location);
	void check_member_overload(
		Scope* scope, const Identifier* key, const Type* type, bool is_static, uint32_t location);
	void braced_argument(const syntax::BracedInitList& list, Scope* scope);
	bool list_sequence(const syntax::BracedInitList& list, const Type* parameter, Conversion& out);
	bool aggregate_accepts(const syntax::BracedInitList& list, const ClassEntity* cls);
	bool designated_list(
		const syntax::BracedInitList& list, const ClassEntity* cls, syntax::BracedInitList& out, Scope* scope);
	bool members_accept(const syntax::BracedInitList& list, uint32_t& next, const ClassEntity* cls);
	bool elements_accept(const syntax::BracedInitList& list, uint32_t& next, const Type* array);
	bool element_accepts(const syntax::BracedInitList& list, uint32_t& next, const Type* type);
	const FunctionEntity* list_constructor(const syntax::BracedInitList& list, ClassEntity* cls);
	void list_argument(const syntax::BracedInitList& list, const Type* parameter, Scope* scope, uint32_t location);
	ExprFacts* member_pointer_call(
		const syntax::CallExpr& expr, const ExprFacts* callee, const syntax::BinaryExpr& access);
	void symbol_attributes(Entity* entity, const syntax::Attribute* leading, const syntax::Declarator* declarator);
	void collect_abi_tags(const syntax::Attribute* list, bool on_class, std::vector<std::string>& tags,
		const Identifier* implied = nullptr);
	void declare_abi_tags(const void* owner, std::vector<std::string>& tags, bool redeclaration, uint32_t location);
	void class_abi_tags(
		const Entity* entity, const syntax::Attribute* list, bool on_class, bool redeclaration, uint32_t location);
	void note_block_linkage(const Entity* entity, Scope* block, bool c_linkage);
	void agree_block_linkage(const Entity* entity, bool c_linkage, bool internal, uint32_t location);
	void agree_language_linkage(bool c_linkage, uint32_t location);
	void effect_attributes(
		FunctionEntity* function, const syntax::Attribute* leading, const syntax::Declarator* declarator);
	void reject_noreturn(const syntax::Attribute* list, const char* message) const;
	void function_definition(const syntax::FunctionDecl& decl, Scope* scope);
	void special_member(const syntax::SpecialMemberDecl& decl, Scope* scope);
	void default_definition(FunctionEntity* function, bool first, uint32_t location);
	void bit_field_declaration(const syntax::BitFieldDecl& decl, Scope* scope);
	void require_definition_types(const Type* type, uint32_t location);
	void define_function(FunctionEntity* function, const Type* type, const syntax::ParameterClause* parameters,
		const syntax::FunctionBody& body, Scope* print, Scope* parent);
	bool reserved_wrapper(const syntax::FunctionBody& body, Scope* parent);
	void wrapper_body(FunctionEntity* function, const Type* type, const syntax::ParameterClause* parameters,
		const syntax::FunctionBody& body, Scope* print, Scope* parent, bool reserved);
	DeclaredName declared_name(const syntax::Name& name, Scope* target);
	Scope* declarator_target(const syntax::Name& name, Scope* qualifier, Scope* scope, uint32_t location);
	void check_parameter_redeclaration(const Identifier* name, Scope* scope, uint32_t location);
	void check_type_name_conflict(Scope* target, const Identifier* name, const Entity* entity, uint32_t location);
	void record_constant_initializer(VariableEntity* variable, const syntax::Initializer* initializer, Scope* scope);
	const Type* complete_array_from_initializer(const Type* type, const syntax::Initializer* initializer, bool typed);

	// ---------------------------------------------- specifiers, types
	void specifiers(
		const syntax::Slice<syntax::DeclSpecifier>& list, Scope* scope, Specifiers& out, bool lone_declaration);
	const Type* keyword_type(const syntax::Slice<syntax::DeclSpecifier>& list, uint32_t location);
	const Type* int128_type(const syntax::Slice<syntax::DeclSpecifier>& list, uint32_t location);
	enum class NameTypeUse : uint8_t
	{
		Type,
		TemplateArgument
	};
	const Type* named_type(const syntax::Name& name, Scope* scope, NameTypeUse use = NameTypeUse::Type);
	bool check_template_name_type(const Binding* binding, uint32_t location);
	const Type* binding_type(const Binding* binding, uint32_t location);
	DeclaratorInfo declarator(const syntax::Declarator* declarator, const Type* type, Scope* scope, uint32_t location,
		const Specifiers* specifiers = nullptr);
	const Type* ptr_operator(const syntax::PtrOperator& pointer, const Type* type, Scope* scope, uint32_t location);
	const Type* apply_suffix(const syntax::DeclaratorSuffix& suffix, const Type* type, Scope* scope, uint32_t location,
		const ClassEntity* member = nullptr);
	Scope* prototype_scope(
		const syntax::ParameterClause& clause, const Type* const* types, uint32_t count, Scope* scope);
	// A parameter's object type: an array's or function's adjusted to a
	// pointer (8.3.5p5).
	const Type* parameter_object_type(const Type* type);
	void prototype_dependence(Scope* scope, const Type* pattern, const syntax::ParameterClause& clause,
		const std::pair<const Identifier*, uint32_t>* packs, size_t count);
	void declare_parameter(Scope* prototype, const syntax::ParameterClause& clause, uint32_t index, const Type* type);
	Scope* qualified_lookup_scope(Scope* qualifier, Scope* scope);
	bool member_hides_parameter(Scope* qualifier, const Identifier* name);
	const Type* parameter_type(const syntax::ParameterDeclaration& parameter, Scope* scope);
	const Type* type_id(const syntax::TypeId& type, Scope* scope);
	const Type* decltype_type(const syntax::Expr& operand, Scope* scope);
	uint64_t array_bound(const syntax::Expr& bound, Scope* scope);
	const Type* checked_pointer(const Type* pointee, uint32_t location);
	const Type* checked_array(const Type* element, uint64_t bound, uint32_t location);

	// ------------------------------------------------------- names
	// The binding a name denotes, by unqualified or qualified lookup;
	// errors if it is not found or ambiguous.
	LookupResult lookup_name(const syntax::Name& name, Scope* scope, LookupFilter filter);
	// Unqualified lookup, with the predefined __func__ of function bodies.
	LookupResult unqualified(Scope* scope, const Identifier* name, LookupFilter filter);
	// What all but the last component of a qualified name denote.
	// `declared`: the qualifier of a declarator-id or class-head, whose
	// classes enclose the declared member and grant it access (11p6, 11.7).
	NamePrefix name_prefix(const syntax::Name& name, Scope* scope, bool declared = false);
	NamePrefix scope_of_binding(const Binding* binding, uint32_t location);
	LookupResult lookup_in(const NamePrefix& prefix, const Identifier* name, LookupFilter filter);
	const Identifier* special_key(const char* spelling);

	// -------------------------------------------- classes and enums
	ClassEntity* new_class(TokenKind key, const Identifier* name, Scope* owner);
	const char* anonymous_class_label(const syntax::ClassSpecifier& specifier, bool after_semicolon);
	Entity* class_specifier(const syntax::ClassSpecifier& specifier, Scope* scope, Specifiers& out);
	void explicit_member_class(ClassEntity* entity, const Scope* owner, const Scope* scope, uint32_t location);
	bool alias_template_id(const syntax::Name& name, Scope* scope);
	void class_body(ClassEntity* entity, const syntax::ClassSpecifier& specifier, Scope* parent, Scope* print);
	uint64_t attribute_alignment(
		const syntax::Attribute* list, Scope* scope, bool& packed, bool& no_inline, bool on_class = false);
	void base_classes(ClassEntity* entity, const syntax::ClassSpecifier& specifier, Scope* scope);
	void virtual_base_list(ClassEntity* entity);
	void distinct_direct_bases(const ClassEntity* entity, uint32_t location);
	void anonymous_union(ClassEntity* entity, const Specifiers& specifiers, Scope* scope, uint32_t location);
	void name_unnamed_type(Specifiers& specifiers, const Identifier* name, Scope* scope);
	LookupResult earlier_class(Scope* scope, const Identifier* name);
	Entity* elaborated_class(
		const syntax::DeclSpecifier& specifier, Scope* scope, bool lone_declaration, bool is_friend, Binding*& printed);
	EnumEntity* redeclared_enum(const syntax::EnumSpecifier& specifier, Scope* target, const Type* underlying);
	Entity* enum_specifier(
		const syntax::EnumSpecifier& specifier, Scope* scope, bool lone_declaration, Binding*& printed);
	Entity* qualified_enum_definition(
		const syntax::EnumSpecifier& specifier, const Type* underlying, Scope* scope, Binding*& printed);
	void enumerators(EnumEntity* entity, const syntax::EnumSpecifier& specifier, const Binding* declaration,
		Scope* print, Scope* lookup, Scope* values = nullptr);
	void declare_member_enum_instance(EnumEntity* entity, ClassEntity* owner);
	void instantiate_member_enum(EnumEntity* entity);
	Constant next_enumerator_value(
		const EnumEntity* entity, const Constant& previous, IntegralKind fixed_kind, uint32_t location);
	const Type* enumeration_underlying(size_t base);
	void finish_class_nesting(size_t first_deferred);
	void define_implicit_default_constructor(ClassEntity* entity);

	// ----------------------------------------------------- bodies
	bool function_body(FunctionEntity* function, const Type* type, const syntax::ParameterClause* parameters,
		const syntax::FunctionBody& body, Scope* print, Scope* parent);
	void call_names(const syntax::CallExpr& call, Scope* scope, bool unevaluated);
	void compound(const syntax::CompoundStmt& body, Scope* scope, bool guarded, bool handles = false);
	void statement(const syntax::Stmt& stmt, Scope* scope);
	void scoped_statement(const syntax::Stmt& stmt, Scope* scope, BodyContext* body);
	void deferrable_statement(const syntax::Stmt& stmt, Scope* scope, BodyContext* body);
	bool deferrable_plan(FunctionEntity* constructor, ClassEntity* entity, const syntax::CtorInitializer& list,
		Scope* scope, uint32_t location);
	void substatement(const syntax::Stmt& stmt, Scope* scope);
	bool deferrable_expression(const syntax::Expr* expr, Scope* scope);
	bool condition(const syntax::Condition& condition, Scope* scope);
	void handlers(const syntax::Slice<syntax::Handler>& list, Scope* scope);
	void expression(const syntax::Expr* expr, Scope* scope);
	bool typed_initializer(VariableEntity* variable, const Type* type, const syntax::Initializer* initializer,
		Scope* scope, uint32_t location);
	void expression_walk(const syntax::Expr* expr, Scope* scope, bool unevaluated = false);
	// A template definition's expressions whose types are fixed by their
	// form (literals, sizeof), checked where they appear (see
	// sema/template_fixed_expressions.cpp).
	void validate_fixed_expression(const syntax::Expr& expr, Scope* scope);
	void fixed_expression(const syntax::Expr& expr, Scope* scope, uint32_t depth, FixedFacts& out);
	void fixed_operator(const syntax::Expr& expr, const FixedFacts& left, const FixedFacts* right, FixedFacts& out);
	void fixed_call(const syntax::CallExpr& call, Scope* scope, uint32_t depth, FixedFacts& out);
	const Type* fixed_decayed(const FixedFacts& facts);
	void expression_names(const syntax::Expr& expr, Scope* scope, bool unevaluated);
	bool dependent_object(const syntax::Expr& object, bool arrow, Scope* scope);
	void new_expression(const syntax::NewExpr& expr, Scope* scope, bool unevaluated);
	void initializer_names(const syntax::Initializer* initializer, Scope* scope, bool constructs_class);
	const Binding* check_id_expression(const syntax::Name& name, Scope* scope, bool allow_undeclared, bool allow_type);
	void template_argument_names(const syntax::NameComponent& component, Scope* scope);
	void default_argument_names(FunctionEntity* function, const syntax::ParameterClause* clause, Scope* scope);
	void default_template_argument_names(const syntax::TypeId& type, Scope* scope);
	const Type* known_argument_type(const syntax::Expr& expr, Scope* scope, uint32_t depth = 0);
	void check_unbound_callee(const syntax::CallExpr& call, const syntax::Name& name, Scope* scope);
	void value_initialized_class(const Type* type);

	// ------------------------------------ expression semantics
	bool semantic() const { return mode_ == AnalysisMode::Semantics; }
	ExprFacts* new_facts(const syntax::Expr& expr, const Type* type, ValueCategory category);
	static ExprFacts* facts_of(const syntax::Expr& expr) { return const_cast<ExprFacts*>(expr.facts); }
	static Operand operand_of(const ExprFacts* facts)
	{
		return Operand{ facts->type, facts->category, static_cast<uint16_t>(facts->flags) };
	}
	// Types an expression and its operands, attaching facts; returns them.
	ExprFacts* typed(const syntax::Expr& expr, Scope* scope);
	ExprFacts* typed_form(const syntax::Expr& expr, Scope* scope);
	ExprFacts* typed_value(const syntax::Expr& expr, Scope* scope);
	ExprFacts* id_expression(const syntax::IdExpr& expr, Scope* scope);
	ExprFacts* binding_expression(
		const syntax::Expr& expr, const syntax::Name* name, const LookupResult& result, Scope* scope);
	bool implicit_object(const ClassEntity* home);
	ExprFacts* literal(const syntax::LiteralExpr& expr);
	ExprFacts* user_string_literal(
		const syntax::LiteralExpr& expr, const Type* array, const StringLiteralFacts& string);
	ExprFacts* user_numeric_literal(const syntax::LiteralExpr& expr, const NumberLiteral& number);
	ExprFacts* user_character_literal(const syntax::LiteralExpr& expr, const CharacterLiteralValue& character);
	ExprFacts* literal_call(const syntax::LiteralExpr& expr, FunctionEntity* function, uint64_t form);
	ExprFacts* keyword_literal(const syntax::KeywordLiteralExpr& expr, Scope* scope);
	ExprFacts* binary(const syntax::BinaryExpr& expr, Scope* scope);
	ExprFacts* binary_chain(const syntax::BinaryExpr& expr, Scope* scope);
	ExprFacts* combine(const syntax::BinaryExpr& expr, Scope* scope);
	ExprFacts* arithmetic(const syntax::BinaryExpr& expr, ExprFacts* left, ExprFacts* right);
	ExprFacts* complex_arithmetic(const syntax::BinaryExpr& expr, const Type* lt, const Type* rt);
	ExprFacts* vector_arithmetic(const syntax::BinaryExpr& expr, const Type* lt, const Type* rt, bool integral_only);
	ExprFacts* comparison(const syntax::BinaryExpr& expr, ExprFacts* left, ExprFacts* right);
	bool scalar_splats(const syntax::Expr& scalar, const Type* type, const Type* lane);
	ExprFacts* member_pointer_access(const syntax::BinaryExpr& expr, ExprFacts* left, ExprFacts* right);
	ExprFacts* assignment(const syntax::BinaryExpr& expr, Scope* scope);
	ExprFacts* conditional(const syntax::ConditionalExpr& expr, Scope* scope);
	bool folding_condition(const syntax::Expr& expr) const;
	ExprFacts* converted_conditional(const syntax::ConditionalExpr& expr, Scope* scope);
	ExprFacts* builtin_conditional(const syntax::ConditionalExpr& expr);
	uint8_t conditional_match(const syntax::Expr& operand, const ExprFacts* other, Conversion& out);
	ExprFacts* unary(const syntax::UnaryExpr& expr, Scope* scope);
	ExprFacts* address_of(const syntax::UnaryExpr& expr, Scope* scope);
	ExprFacts* increment(const syntax::UnaryExpr& expr, Scope* scope);
	ExprFacts* subscript(const syntax::SubscriptExpr& expr, Scope* scope);
	ExprFacts* member_access(const syntax::MemberExpr& expr, Scope* scope);
	ExprFacts* member_of_object(const syntax::Expr& expr, ExprFacts* object, const Binding* member,
		const FunctionList* functions, bool implicit_this);
	ExprFacts* arrow_operator(const syntax::MemberExpr& expr, ExprFacts* object, Scope* scope);
	FunctionList* member_templates(const LookupResult& result, uint32_t location);
	ExprFacts* conversion_member(
		const syntax::MemberExpr& expr, ExprFacts* object, const ClassEntity* cls, const Type* type, Scope* scope);
	ExprFacts* conversion_name(const syntax::IdExpr& expr, Scope* scope);
	const ClassEntity* member_qualifier(const syntax::Name& name, const ClassEntity* cls, Scope* scope);
	ExprFacts* destructor_access(const syntax::MemberExpr& expr, const Type* object, Scope* scope);
	ExprFacts* cast_expression(const syntax::CastExpr& expr, Scope* scope);
	ExprFacts* functional_cast(const syntax::FunctionalCastExpr& expr, Scope* scope);
	ExprFacts* explicit_conversion(
		const syntax::Expr& expr, const Type* target, const syntax::Expr* operand, TokenKind cast, Scope* scope);
	ExprFacts* member_pointer_cast(
		const syntax::Expr& expr, ExprFacts* source, const Type* from, const Type* goal, bool c_style);
	bool static_member_pointer_cast(const ExprFacts* source, const Type* from, const Type* goal);
	bool member_subobject(const syntax::Expr& expr, const Entity* member, const ClassEntity* cls,
		const ClassEntity* naming, uint32_t location);
	bool downcast_base(const ClassEntity* base, const ClassEntity* derived, bool access, uint32_t location);
	bool upcast_base(const ClassEntity* derived, const ClassEntity* base, bool access, uint32_t location);
	// base_path_facts of a base in a derived class: its subobjects in an
	// object of that class, up to 2 (kBasePathCount: one per path through
	// no virtual base-specifier, and per such path from each virtual base,
	// 10.1p4), whether a path passes through a virtual base-specifier
	// (kVirtualBasePath: a subobject is a virtual base or within one) and
	// whether one passes through none (kNonVirtualBasePath).
	static const uint32_t kBasePathCount = 3;
	static const uint32_t kVirtualBasePath = 4;
	static const uint32_t kNonVirtualBasePath = 8;
	uint32_t base_paths(
		const ClassEntity* base, const ClassEntity* cls, PointerMap& memo, uint32_t& shared, uint32_t location);
	uint32_t base_path_facts(const ClassEntity* base, const ClassEntity* derived, uint32_t location);
	const ClassEntity* chain_bottom(const ClassEntity* cls);
	int32_t base_table_fact(const ClassEntity* cls, const ClassEntity* base, uint32_t location);
	bool build_base_table(const ClassEntity* cls, uint32_t location);
	bool simple_hierarchy(const ClassEntity* cls);
	ExprFacts* size_of(const syntax::SizeofExpr& expr, Scope* scope);
	bool empty_functional_cast(const syntax::Expr& expr, Scope* scope);
	ExprFacts* braced_list(const syntax::BracedInitList& list, const Type* target, Scope* scope);
	ExprFacts* this_expression(uint32_t location);
	void static_constant(ExprFacts* facts, const VariableEntity* variable);
	const ClassEntity* naming_class(const ClassEntity* home, const Scope* scope);
	// A prvalue operand of a built-in operator: array-to-pointer,
	// function-to-pointer and lvalue-to-rvalue conversions (5p9), recorded.
	const Type* decayed(ExprFacts* facts);
	// The integral promotions (4.5) of an arithmetic or unscoped
	// enumeration operand; the usual arithmetic conversions (5p10).
	const Type* promoted(const Type* type);
	const Type* field_promoted(const ExprFacts* facts, const Type* type);
	const Type* usual_arithmetic(const Type* a, const Type* b);
	// 5.9p2: the composite pointer type of two pointer or null operands
	// (of pointers to members, 5.10p2's common type when `comparison`,
	// else 5.16p6's).
	const Type* composite_pointer(const ExprFacts* a, const ExprFacts* b, bool comparison = false);
	const Type* cv_combined(const Type* a, const Type* b);
	bool modifiable_lvalue(const ExprFacts* facts) const;
	// Converts an analyzed expression to `target` (copy- or direct-
	// initialization, 8.5), recording the conversion; errors if none (in a
	// substitution its failure: null, 14.8.2p8).
	const Conversion* convert(const syntax::Expr& expr, const Type* target, bool direct, bool operand = false);
	const Conversion* convert_operand(const syntax::Expr& expr, const Type* target)
	{
		return convert(expr, target, false, true);
	}
	const Conversion* record_conversion(ExprFacts* facts, const Conversion& conversion);
	// `access` false: a C-style cast's, which waives access but not
	// ambiguity (5.4p4).
	void check_conversion_access(
		const ExprFacts* from, const Conversion& conversion, uint32_t location, bool access = true);
	const Conversion* intern_conversion(const Conversion& conversion);
	// 4p3: contextual conversion to bool.
	void convert_to_bool(const syntax::Expr& expr);
	// 13.4: selects the function of an overloaded name (or its address)
	// whose type matches the target type; null if none or ambiguous.
	// `explicit_cast`: a static_cast's target (5.2.9p12 also takes a
	// derived class's member to a base's pointer).
	FunctionEntity* select_overloaded(const ExprFacts* facts, const Type* target, bool explicit_cast = false);
	void resolve_overloaded(const syntax::Expr& expr, FunctionEntity* function);
	bool check_resolved(const syntax::Expr& expr);
	bool resolve_lone_function(const syntax::Expr& expr);
	void odr_use(FunctionEntity* function, bool call = false);
	// 8.4.3p2: naming a deleted function other than to declare it.
	bool check_not_deleted(const FunctionEntity* function, uint32_t location);

	// ----------------------------- class templates (class_templates.cpp)
	class InstantiationContext;
	friend class InstantiationContext;
	class SoftSubstitution;
	friend class SoftSubstitution;
	// The analysis scratch of instantiations, one set per nesting level,
	// kept so later instantiations reuse its capacity (owned; released by
	// the destructor).
	struct InstantiationScratch;
	std::vector<InstantiationScratch*> scratch_pool_;
	uint32_t scratch_level_ = 0;
	static const uint32_t kInstantiationDepthLimit = 2048;
	uint32_t enter_instantiation(uint32_t depth, uint32_t location) const;
	TemplateParameterEntity* positional_parameter(uint32_t depth, uint32_t index, bool value, bool pack);
	ClassTemplateFacts* class_template_facts(ClassEntity* entity, TemplateRole role);
	TemplateHead* template_head(const syntax::TemplateParameterList& list, Scope* scope,
		const std::vector<TemplateParameterEntity*>& parameters);
	void merge_template_head(
		TemplateHead* head, const syntax::TemplateParameterList& list, Scope* scope, uint32_t location);
	void default_ordinal(const TemplateHead* head, uint32_t index, uint32_t ordinal);
	void check_template_defaults(const TemplateHead* head, uint32_t location);
	void declare_class_template(ClassEntity* entity, bool existed, uint32_t location);
	void define_pattern(ClassEntity* entity, const syntax::ClassSpecifier& specifier, Scope* parameters);
	static bool templated_class(const Scope* scope);
	static bool instance_class(const Scope* scope);
	static bool WithinInstance(const Scope* scope);
	static bool InstanceMemberTemplate(const Scope* scope);
	static bool has_dependent_bases(const Scope* scope);
	static ClassEntity* class_template_of(const Entity* entity);
	bool template_arguments(
		const syntax::NameComponent& component, Scope* scope, TypeList& out, const TemplateHead* head = nullptr);
	const Type* template_argument(const syntax::TemplateArgument& argument, Scope* scope, uint32_t location,
		const Type* target = nullptr, bool unknown = false);
	const Type* template_value(
		const syntax::Expr& expr, Scope* scope, const Type* target = nullptr, bool unknown = false);
	const Type* argument_target(const TemplateHead* head, uint32_t index, const Type* const* earlier);
	static bool NamesLinkedObject(const syntax::Expr& expr);
	static bool value_argument(const Type* argument);
	bool match_template_arguments(const TemplateHead* head, const Type* const* given, uint32_t count, TypeList& out,
		bool fatal, uint32_t location, bool partial = false);
	// `element`: an expanded pack's element type, converted to in place of
	// the parameter's declared type.
	const Type* matched_element(const TemplateParameterEntity* parameter, const Type* argument, const TypeList& earlier,
		bool fatal, uint32_t location, const Type* element = nullptr);
	const Type* converted_value(const TemplateParameterEntity* parameter, const Type* argument, const TypeList& earlier,
		bool fatal, uint32_t location, const Type* element = nullptr);
	static const TemplateHead* SoleTemplateHead(const FunctionList* functions);
	// Where a definition being made is, in declaration order: the context
	// limit of the instantiation making it, else the unit's point so far.
	uint32_t definition_point() const
	{
		return names_.limit() != NameLookup::kNoLimit ? names_.limit() : graph_.bindings;
	}
	static bool TemplatedClassScope(const Scope* scope);
	bool type_dependent(const syntax::Expr* expr, Scope* scope);
	bool fixed_expression(const syntax::Expr& operand, Scope* scope);
	bool dependent_name_expression(const syntax::Name& name, Scope* scope);
	bool fixed_template_arguments(const syntax::NameComponent& component, Scope* scope);
	void check_fixed_operands(const syntax::Expr& operand, Scope* scope, uint32_t level);
	void bound_expansion(const Type* expansion, TypeList& out);
	// type_dependent's reading for check_fixed_operands: a template-id whose
	// arguments are non-dependent types, and a functional cast, decided.
	bool precise_dependence_ = false;
	const Type* decltype_base_type(const syntax::Expr& operand, Scope* scope);
	const Type* address_argument(const syntax::Expr& expr, const Type* target);
	void note_argument_demand(const Entity* entity);
	const Type* converted_address(const Type* target, const Type* argument, bool fatal, uint32_t location);
	ExprFacts* argument_name(const syntax::Expr& expr, const EnumeratorEntity* constant);
	Binding* value_binding(
		const TemplateParameterEntity* parameter, const Type* argument, Scope* frame, const Identifier* name = nullptr);
	const Type* template_default(const TemplateHead* head, const Type* const* arguments, uint32_t index,
		bool arguments_dependent, bool substitution = false, Scope* named_in = nullptr);
	void default_template_arguments(
		const TemplateHead* head, TypeList& arguments, uint32_t location, Scope* named_in = nullptr);
	bool own_arguments(const TemplateHead* head, const Type* const* canonical, uint32_t count);
	ClassEntity* class_specialization(
		ClassEntity* templ, const TypeList& arguments, uint32_t location, bool fatal = true);
	Binding* template_id_binding(const Binding* found, const syntax::NameComponent& component, Scope* scope);
	bool current_instantiation(const ClassEntity* templ, const Scope* scope) const;
	ClassEntity* qualifying_template(const syntax::Decl& decl, Scope* scope, Scope** named = nullptr);
	Scope* member_template_home(const syntax::TemplateDecl& decl, Scope* scope);
	ClassEntity* named_partial(ClassEntity* templ, const syntax::NameComponent& component, Scope* scope);
	ClassEntity* named_specialization(ClassEntity* templ, const syntax::NameComponent& component, Scope* scope);
	bool dependent_type(const Type* type) const;
	bool leaf_dependent(const Type* type) const;
	Binding* dependent_member_binding(const Type* qualifier, const Identifier* name);
	const Type* dependent_member(const Type* qualifier, const Identifier* name);
	const Type* dependent_decltype(const syntax::Expr& operand, Scope* scope);
	const Type* dependent_expression(const syntax::Expr& operand, Scope* scope, bool value,
		const Type* target = nullptr, bool fixed = false, const Type* evaluation_target = nullptr,
		bool unknown = false);
	const Type* fixed_template_value(
		const syntax::TemplateArgument& argument, Scope* scope, const Type* target, bool unknown);
	bool decltype_words(const syntax::Expr* expr, Scope* scope);
	bool trait_words(const syntax::BuiltinTraitExpr& trait, Scope* scope);
	bool size_words(const syntax::Expr& expr, Scope* scope);
	bool name_words(const syntax::Name& name, Scope* scope, bool resolve, Constant* constant = nullptr,
		const syntax::CallExpr* call = nullptr);
	bool dependent_arguments(const syntax::CallExpr& call, Scope* scope);
	uint64_t parameter_word(const Binding* binding, const Scope* scope) const;
	void note_named_parameter(uint64_t word);
	bool unnamed_parameter(const TemplateParameterEntity* operand, const Binding* binding);
	bool resolution_words(const syntax::Name& name, Scope* scope, size_t last_arguments, bool dependent_arguments,
		size_t name_start, size_t qualifier_arguments);
	void qualified_name_words(const syntax::Name& name, const Type* qualifier, size_t start, size_t arguments);
	TemplateParameterEntity* qualified_placeholder(const DecltypeKey& raw, size_t first, DecltypeKey& alias);
	static FunctionEntity* SoleFunction(const LookupResult& found);
	const FunctionEntity* resolved_specialization(
		FunctionEntity* function, const uint64_t* arguments, uint32_t count, uint32_t location);
	Scope* substituted_scope(
		Scope* scope, const Substitution& substitution, const TemplateParameterEntity* operand = nullptr);
	const Type* substitute_decltype(const Type* type, const Substitution& substitution);
	const Type* substitute_value(const Type* type, const Substitution& substitution);
	const Type* failed_substitution(const TemplateParameterEntity* placeholder);
	static bool OwnParameter(const Type* argument, uint32_t depth, uint32_t index);
	const Type* substituted_value(const TemplateParameterEntity* keyed, const TemplateParameterEntity* placeholder,
		const struct PendingSubstitution* earlier, uint32_t depth, const Type* const* canonical, uint32_t count,
		bool dependent, const int32_t* elements = nullptr);
	bool element_renames(const Scope* scope, uint32_t depth, const int32_t* elements, uint32_t count,
		const Type* nested, std::vector<std::pair<const Identifier*, const Identifier*>>& out);
	static uint8_t HeadDepth(const Scope* scope);
	bool unsubstituted_depth(
		const TemplateParameterEntity* placeholder, const PendingSubstitution* earlier, uint32_t depth);
	static bool DeclaresParameter(const Binding* binding);
	static bool DeclaresDepth(const Scope* scope, uint32_t depth);
	bool placeholder_words(
		const syntax::Expr& operand, Scope* scope, const Scope* home, bool value, const Type* target);
	bool instantiation_dependent(const syntax::Expr& expr, Scope* scope, bool contextual_access = false);
	const Type* pending_value(const TemplateParameterEntity* placeholder, uint32_t depth, const Type* const* canonical,
		uint32_t count, const PendingSubstitution* earlier);
	const Type* substitute_pending(const Type* type, const Substitution& substitution);
	Scope* enclosing_frame(Scope* scope, const Substitution& substitution);
	void bind_argument(Scope* frame, Binding* binding, const Type* argument, int32_t element = -1);
	const Type* substitute_bound(const Type* array, const Substitution& substitution);
	const Type* substitute_vector_size(const Type* vector, const Substitution& substitution);
	Scope* instantiation_frame(Scope* parameters, const Type* const* arguments, uint32_t count, Scope* parent,
		const Scope* bridge = nullptr, const int32_t* elements = nullptr);
	void instantiate_class(ClassEntity* entity);
	bool declare_member_instance(const syntax::ClassSpecifier* specifier, const syntax::Name& name, TokenKind key,
		Scope* where, Specifiers& out);
	void explicitly_instantiate_members(ClassEntity* cls);
	void link_member_patterns(const syntax::ClassSpecifier& pattern, const syntax::ClassSpecifier& instance,
		uint32_t first = 0, uint32_t end = ~0u);
	const ClassEntity* original_member_template(const ClassEntity* copy);
	const FunctionEntity* outside_pattern_member(const FunctionEntity* member);
	const VariableEntity* outside_pattern_variable(const VariableEntity* member);
	void link_member(const Binding* pattern, const Binding* instance);
	VariableTemplateFacts* variable_template_facts(VariableEntity* variable);
	EnumTemplateFacts* enum_template_facts(EnumEntity* entity);
	FunctionTemplateFacts* function_template_facts(FunctionEntity* function);

	// ------------------ template names and alias templates (template_names.cpp)
	TemplateParameterEntity* positional_template_parameter(
		uint32_t depth, uint32_t index, bool pack, const TemplateHead* head);
	const Type* template_shape(TemplateParameterEntity* const* parameters, uint32_t count);
	const Type* template_name_type(const Binding* binding, bool injected);
	const Type* template_name_argument(const syntax::Name& name, Scope* scope, bool injected);
	static const syntax::Name* PlainTemplateName(const syntax::TemplateArgument& argument, bool* expansion = nullptr);
	static const TemplateHead* TemplateHeadOf(const Type* templ);
	bool template_heads_match(const TemplateHead* parameter, const TemplateHead* argument);
	bool template_parameters_match(const TemplateParameterEntity* p, const TemplateParameterEntity* a);
	const Type* dependent_template_name(const Type* qualifier, const Identifier* name);
	ClassEntity* stand_in_template(const TemplateParameterEntity* name);
	static bool IsStandIn(const ClassEntity* entity)
	{
		return entity != nullptr && entity->templ != nullptr && entity->templ->template_name != nullptr;
	}
	ClassEntity* alias_stand_in(const AliasTemplateEntity* alias);
	const AliasTemplateEntity* stand_in_alias(const ClassEntity* templ) const;
	// (A type that is no class is itself, without a call.)
	const Type* aliased_type(const Type* type)
	{
		return type->unqualified->kind != TypeKind::Class ? type : aliased_class_type(type);
	}
	const Type* aliased_class_type(const Type* type);
	const Type* declared_type(const Type* type);
	void equivalent_representative(TemplateParameterEntity* entity, const syntax::Expr& operand, Scope* scope,
		bool value, const Type* target, bool lookup);
	const Type* without_stand_ins(const Type* type);
	void named_parameters(const Type* type, uint32_t depth, uint64_t& named, bool& opaque, uint32_t level);
	static uint32_t EnclosingTemplateLists(const Scope* qualifier);
	static bool MemberTemplateLists(const Scope* qualifier, const Scope* scope);
	Scope* member_template_frame(const FunctionEntity* function, const FunctionEntity* definition);
	Scope* outside_frames(Scope* own, const Type* const* arguments, uint32_t count, const ClassEntity* cls,
		Scope* scope, const Scope* bridge);
	ClassEntity* outside_member_template(const ClassEntity* templ);
	ClassEntity* outside_member_class(const ClassEntity* member);
	typedef SmallList<const ClassTemplateFacts*, 4> EnclosingSpecializations;
	static void enclosing_specializations(const ClassEntity* cls, EnclosingSpecializations& out);
	const Type* const* enclosing_arguments(const ClassTemplateFacts* facts, uint32_t& count) const;
	ClassEntity* pattern_member_template(const ClassEntity* templ);
	void enclosing_substitution(const Type* const* pattern, uint32_t count, const ClassEntity* instance, TypeList& out);
	const Type* specialize_template(const Type* templ, TypeList& arguments, uint32_t location);
	Binding* specialization_binding(const Type* type, const Entity* templ);
	const Type* substitute_template_name(const Type* type, const Substitution& substitution);
	const Type* substitute_stand_in(const Type* type, const Substitution& substitution);
	bool deduce_stand_in(const ClassTemplateFacts* pattern, const Type* a, uint32_t depth, TypeList& arguments);
	const Type* synthesized_template(uint32_t index);
	const Type* synthesized_argument(const TemplateParameterEntity* parameter, uint32_t index);
	void declare_alias_template(const syntax::AliasDecl& decl, Scope* scope);
	bool explicit_value(const syntax::Expr& condition, Scope* scope, bool converted = false);
	struct PendingExplicit
	{
		const syntax::Expr* condition;
		Scope* scope;
	};
	bool specialized_explicit(const PendingExplicit& pending, const TypeList& arguments);
	const Type* alias_specialization(const AliasTemplateEntity* alias, TypeList& arguments, uint32_t location);

	// ------------------------------ parameter packs (template_packs.cpp)
	class FrameExpander;
	friend class FrameExpander;
	const Identifier* pack_element_name(const Identifier* name, uint32_t index);
	void unexpanded_packs(
		const Type* type, uint32_t depth, PackList& out, TypeList* bound = nullptr, bool placeholders = true);
	static int32_t BoundLength(const TypeList& bound);
	const Type* expression_packs(const syntax::Expr& expr, Scope* scope, const Type** nested = nullptr);
	const Type* canonical_packs(const std::vector<const Identifier*>& names, Scope* scope);
	void substitute_list(const Type* const* types, uint32_t count, const Substitution& substitution, TypeList& out);
	const Type* const* effective_arguments(const Substitution& substitution, TypeList& scratch);
	void flattened_arguments(const Type* const* arguments, uint32_t count, TypeList& out);
	void expanded_argument(const syntax::TemplateArgument& argument, Scope* scope, uint32_t location, TypeList& out);
	const Type* pack_expansion_type(const Type* pattern, uint32_t location);
	template<typename Deduce>
	bool deduce_elements(
		const Type* pattern, uint32_t count, bool counted, uint32_t depth, TypeList& arguments, Deduce deduce);
	bool deduce_pack(const Type* p, const Type* a, uint32_t depth, TypeList& arguments);
	bool deduce_list(const Type* const* p, uint32_t count, const Type* const* a, uint32_t a_count, uint32_t depth,
		TypeList& arguments);
	bool deduce_pack_types(
		const Type* pattern, const Type* const* types, uint32_t count, uint32_t depth, TypeList& arguments);
	bool pack_size(const Identifier* pack, Scope* scope, uint64_t& count);
	Scope* function_pack_scope(Scope* parent, const syntax::ParameterClause& clause,
		const std::pair<const Identifier*, uint32_t>* packs, size_t count, const Type* type);
	bool deduce_call_pack(const Type* pattern, const syntax::Slice<syntax::Expr*>& call_arguments, uint32_t first,
		uint32_t depth, TypeList& arguments);
	void bind_pack(Scope* frame, const TemplateParameterEntity* parameter, const Type* argument, int32_t element = -1);

	// ------------------------------------------- function templates
	static Substitution substitution_of(const TemplateHead* head, const Type* const* arguments);
	const Type* substitute(const Type* type, const Substitution& substitution);
	const Type* substitute_class(const Type* type, const Substitution& substitution);
	static bool AnyNull(const TypeList& types);
	const Type* substitute_member(const Type* type, const Substitution& substitution);
	const Type* substitute_function(const Type* type, const Substitution& substitution, bool leading);
	const Type* without_cv(const Type* type, uint8_t cv);
	bool deduce_type(const Type* p, const Type* a, uint32_t depth, TypeList& arguments, bool extra_cv);
	bool deduce_array_bound(const Type* array, uint64_t count, uint32_t depth, TypeList& arguments);
	bool deduce_class(const Type* p, const Type* a, uint32_t depth, TypeList& arguments);
	bool deduce_from_base(const Type* p, const Type* a, uint32_t depth, TypeList& arguments);
	bool complete_deduction(FunctionEntity* primary, TypeList& arguments);
	bool deduce_argument(const Type* p, const Type* a, ValueCategory category, uint32_t depth, TypeList& arguments);
	void deduce_overloaded(const Type* p, const FunctionList* functions, uint32_t depth, TypeList& arguments);
	FunctionEntity* deduce_for_call(FunctionEntity* primary, const syntax::Slice<syntax::Expr*>& arguments,
		const TypeList* explicit_arguments = nullptr);
	FunctionEntity* deduce_for_target(FunctionEntity* primary, const Type* function, const Type* given = nullptr);
	FunctionEntity* deduce_for_conversion(FunctionEntity* primary, const Type* required, bool named = false);
	const Type* without_level_cv(const Type* type);
	FunctionEntity* deduce_for_operand(FunctionEntity* primary, const ExprFacts* from);
	FunctionEntity* specialize(FunctionEntity* primary, const TypeList& arguments);
	FunctionEntity* deduced_specialization(
		FunctionEntity* primary, TypeList& arguments, const syntax::Slice<syntax::Expr*>* call_arguments = nullptr);
	bool fixed_call_arguments(
		const FunctionEntity* primary, const TypeList& arguments, const syntax::Slice<syntax::Expr*>& call_arguments);
	bool deduced_values_typed(const FunctionEntity* primary, const TypeList& arguments);
	const Type* synthesized_type(uint32_t index);
	bool at_least_as_specialized(
		const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form = Ordering::Call);
	int function_template_order(
		const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form = Ordering::Call);
	bool compute_function_order(
		const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form);
	bool function_type_at_least_as_specialized(
		const FunctionEntity* first, const FunctionEntity* second, const Substitution& substitution);
	const Type* ordering_parameter(const FunctionEntity* function, uint32_t index, bool inserted);
	const Type* ordering_type(const Type* type);
	bool result_at_least_as_specialized(const FunctionEntity* first, const FunctionEntity* second);
	int conversion_template_order(const FunctionEntity* first, const FunctionEntity* second);
	static bool trailing_pack_unused(const FunctionEntity* primary, const FunctionEntity* other, uint32_t arguments);
	int reference_ordering(
		const FunctionEntity* first, const FunctionEntity* second, uint32_t arguments, Ordering form = Ordering::Call);
	static bool has_trailing_pack(const FunctionEntity* primary);
	const FunctionList* explicit_specializations(
		const syntax::NameComponent& component, const FunctionList* functions, Scope* scope);

	// ---------------------------------- lambdas (analyzer_lambdas.cpp)
	ExprFacts* lambda_expression(const syntax::LambdaExpr& expr, Scope* scope);
	FunctionEntity* closure_call(ClassEntity* closure, const syntax::LambdaExpr& expr, Scope* scope);
	FunctionEntity* templated_closure_call(
		ClassEntity* closure, const syntax::LambdaExpr& expr, Scope* scope, LambdaFrame& frame);
	void scan_capture(const Binding* binding, uint32_t location);
	void scan_nested_lambda(const syntax::LambdaExpr& lambda, Scope* scope);
	ExprFacts* templated_lambda(
		ClassEntity* closure, ClosureFacts* facts, const syntax::LambdaExpr& expr, Scope* scope);
	bool sealed_frame(const ClassEntity* closure, LambdaFrame& frame);
	void number_closure(ClassEntity* closure, ClosureFacts* facts);
	void explicit_captures(LambdaFrame& frame, Scope* scope);
	void add_capture(LambdaFrame& frame, const Entity* entity, bool written, bool by_copy, uint32_t location);
	const FunctionEntity* capture_constructor(ClassEntity* closure, const Type* type, uint32_t location);
	void local_name(const syntax::Expr& expr, ExprFacts* facts, const Entity* entity, Scope* scope);
	bool capture_variable(const Entity* entity, uint32_t location);
	// An operand converted by `binding` (null: none) odr-uses the constants
	// it designates: only a glvalue a reference binds directly, or none, in
	// an evaluated operand (checked here, per conversion: the common case
	// returns at once).
	void odr_use_constants(const syntax::Expr& expr, const Conversion* binding)
	{
		if ((binding == nullptr ||
				(binding->form == ConversionForm::Reference && !binding->temporary &&
					binding->constructor == nullptr)) &&
			unevaluated_ == 0)
			odr_use_constant_operand(expr);
	}
	void odr_use_constant_operand(const syntax::Expr& expr);
	const Type* captured_type(const Entity* entity, const Type* type);
	void capture_this(uint32_t location);
	void finish_captures(LambdaFrame& frame);
	void closure_conversion(const ClassEntity* closure, bool conversion_function = true);
	const Type* deduced_result(ExprFacts* facts, uint32_t location);
	const Type* return_deduction(ExprFacts* facts, uint32_t location);
	void deduce_function_result(FunctionEntity* function, const Type* result);
	bool require_deduced(FunctionEntity* function, uint32_t location);

	// ------------------------------------- run-time types (analyzer_rtti.cpp)
	const ClassEntity* type_info_class(uint32_t location);
	ExprFacts* typeid_expression(const syntax::TypeTraitExpr& expr, Scope* scope);
	const Type* typeid_operand(const syntax::Expr& operand, Scope* scope, bool& dynamic);
	ExprFacts* type_identity(const syntax::Expr& expr, TokenKind op, const syntax::Slice<syntax::Expr*>& operands,
		const FunctionEntity* selected);
	ExprFacts* dynamic_cast_expression(const syntax::CastExpr& expr, const Type* target, Scope* scope);
	// What an evaluated operand does beyond its typing (odr-uses, captures,
	// an instance member's definition demanded, 5.1.1p13's error), kept
	// aside while a typeid operand is typed before 5.2.8p2-p3 tell whether
	// it is evaluated (at the innermost unevaluated level, `deferring_`).
	struct DeferredUse
	{
		enum class Kind : uint8_t
		{
			Function,
			Local,
			This,
			Variable,
			Error
		};
		Kind kind;
		bool call;
		const syntax::Expr* expr;
		ExprFacts* facts;
		const Entity* entity;
		Scope* scope;
		const char* message;
		uint32_t location;
	};
	bool deferring() const { return deferring_ != 0 && deferring_ == unevaluated_; }
	void defer_use(const DeferredUse& use);
	void replay_uses(size_t first);
	void demand_variable(const VariableEntity* variable);
	void evaluated_error(uint32_t location, const char* message);

	// ------------ std::initializer_list (analyzer_initializer_lists.cpp)
	bool is_initializer_list_template(const ClassEntity* templ) const;
	bool in_std(const ClassEntity* entity) const;
	const ClassEntity* implementation_template(const char* name, uint32_t parameters);
	const Type* initializer_list_element(const Type* type) const;
	ClassEntity* declared_initializer_list(uint32_t location);
	const Type* initializer_list_of(const Type* element, uint32_t location);
	void builtin_initializer_list(ClassEntity* entity);
	bool builtin_list_class(const ClassEntity* entity) const;
	const Identifier* builtin_list_key(unsigned member);
	bool initializer_list_sequence(
		const syntax::BracedInitList& list, const Type* parameter, const Type* element, Conversion& out);
	ExprFacts* initializer_list_object(const syntax::BracedInitList& list, const Type* type, Scope* scope);
	const Type* initializer_list_array(const syntax::BracedInitList& list, const Type* type, Scope* scope);
	FunctionEntity* select_list_constructor(ClassEntity* entity, ExprFacts* facts, const syntax::BracedInitList& list,
		ConstructionKind kind, Scope* scope, uint32_t location);
	bool initializer_list_constructor(const FunctionEntity* constructor) const;
	FunctionEntity* list_constructor_phase(
		ClassEntity* entity, const syntax::BracedInitList& list, std::vector<FunctionEntity*>& set, Candidate& chosen);
	const Type* list_deduced_type(const Specifiers& specifiers, const Type* declared,
		const syntax::BracedInitList& list, Scope* scope, uint32_t location);
	bool deduce_list_elements(const Type* p, const syntax::BracedInitList& list, uint32_t depth, TypeList& arguments);

	// ------------------- throw and handlers (analyzer_handlers.cpp)
	const Type* throw_operand(const syntax::Expr& operand, Scope* scope, uint32_t location);
	ExprFacts* throw_expression(const syntax::ThrowExpr& expr, Scope* scope);
	ExprFacts* statement_expression(const syntax::StatementExpr& expr, Scope* scope);
	void statement_block(const syntax::StatementExpr& expr, Scope* scope);
	void try_statement(const syntax::TryStmt& stmt, Scope* scope);
	void exception_declaration(
		const syntax::ExceptionDeclaration& exception, const DeclaratorInfo& info, Scope* block, uint32_t location);

	// -------------------------- range-based for (analyzer_range_for.cpp)
	uint8_t ambiguous_reading(const syntax::AmbiguousStmt& stmt, Scope* scope);
	void range_for(const syntax::RangeForStmt& loop, Scope* scope);
	void range_for_names(const syntax::RangeForStmt& loop, Scope* block);
	const Type* known_range_element(
		const syntax::RangeForStmt& loop, Scope* block, bool names_dependent, ValueCategory& category);
	void range_for_equivalent(const syntax::RangeForStmt& loop, Scope* block);
	template<typename T>
	T* implicit_node(uint32_t location);
	const Identifier* range_for_identifier(unsigned which);
	syntax::Name* implicit_name(const Identifier* identifier, uint32_t location);
	syntax::InitDeclarator* implicit_variable(
		unsigned key, const Type* declared, syntax::Expr* init, Scope* block, uint32_t location);
	syntax::Expr* implicit_use(const syntax::InitDeclarator& variable, uint32_t location, bool dependent);
	syntax::Expr* implicit_copy(const syntax::Expr& named);
	syntax::Expr* implicit_constant(const Type* type, uint64_t value, uint32_t location);
	ExprFacts* typed_already(const syntax::Expr& expr);
	ExprFacts* argument_dependent_call(syntax::CallExpr& call, const Identifier* name, bool bound, Scope* scope);

	// ------------------------------------------------------ calls
	ExprFacts* call(const syntax::CallExpr& expr, Scope* scope);
	bool member_template_arguments(
		const syntax::MemberExpr& access, const ExprFacts* callee, Scope* scope, TypeList& explicit_arguments);
	ExprFacts* template_callee(const syntax::IdExpr& named, const syntax::Expr* callee, Scope* scope,
		TypeList& explicit_arguments, bool& undeclared);
	ExprFacts* template_id_call(
		const syntax::CallExpr& expr, ExprFacts* callee, const TypeList& explicit_arguments, const ClassEntity* naming);
	// C++17's structured bindings (analyzer_bindings.cpp).
	friend class Decomposer;
	void structured_binding_declaration(const syntax::SimpleDecl& decl, const Specifiers& specifiers, Scope* scope);
	// GNU asm statements (analyzer_asm.cpp).
	void asm_statement(const syntax::AsmStmt& stmt, Scope* scope);
	// ----------------------- builtin functions (analyzer_builtins.cpp)
	// The registry's builtin `name` (lex/gnu_builtins.h), declared in the
	// global namespace on first need; null for none.
	FunctionEntity* builtin_function(const Identifier* name);
	// A lookup of `name` that found nothing, or the builtin of that name
	// (its binding in the global namespace).
	LookupResult builtin_lookup(const Identifier* name, const LookupResult& none);
	const Type* builtin_signature(const char* signature, char floating);
	// A call of a builtin of an intrinsic or query form.
	ExprFacts* builtin_operation(const syntax::CallExpr& expr, const FunctionEntity* builtin, Scope* scope);
	ExprFacts* constant_p_call(const syntax::CallExpr& expr, Scope* scope);
	ExprFacts* object_size_call(const syntax::CallExpr& expr, Scope* scope);
	ExprFacts* floating_builtin(const syntax::CallExpr& expr, GnuBuiltin builtin, uint8_t variant, Scope* scope);
	ExprFacts* gnu_intrinsic(const syntax::CallExpr& expr, GnuBuiltin builtin, uint8_t variant, Scope* scope);
	ExprFacts* vector_lane_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	ExprFacts* vector_shift_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	ExprFacts* vector_shuffle_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	ExprFacts* vector_permute_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	ExprFacts* vector_index_shuffle_builtin(const syntax::CallExpr& expr, Scope* scope);
	ExprFacts* overflow_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	ExprFacts* allocation_builtin(const syntax::CallExpr& expr, bool allocate, Scope* scope);
	// The C library's nan, nanf or nanl (by `variant`) called as
	// __builtin_nan[f|l] of an operand that is no string literal is.
	ExprFacts* library_nan_call(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	ExprFacts* source_location_builtin(const syntax::CallExpr& expr, uint8_t variant, Scope* scope);
	// An __atomic, __sync or __c11_atomic builtin's call as `form` reads it.
	struct AtomicForm
	{
		Intrinsic kind;
		const char* roles;
		char result;
		// The memory order an operand does not give (its encoding + 1), or 0.
		uint8_t implied;
		// A compare-exchange's weakness no operand gives (0, 1), or 0xff.
		uint8_t weak;
		// __sync: trailing operands allowed (and ignored).
		bool trailing;
		// __c11_atomic: an object of any type of 1, 2, 4 or 8 bytes but for
		// the arithmetic.
		bool any_object;
	};
	ExprFacts* atomic_builtin(const syntax::CallExpr& expr, const AtomicForm& form, Scope* scope);
	ExprFacts* builtin_invoke(const syntax::CallExpr& expr, Scope* scope);
	ExprFacts* invoke_call(
		const syntax::CallExpr& rewritten, const syntax::Expr* callable, const syntax::Expr* object, Scope* scope);
	ExprFacts* indirect_call(const syntax::CallExpr& expr, ExprFacts* callee);
	ExprFacts* resolved_call(const syntax::CallExpr& expr, const FunctionList* functions, ExprFacts* object,
		bool implicit_this, const ClassEntity* naming, bool operator_object = false);
	ExprFacts* call_operator(const syntax::CallExpr& expr, ExprFacts* callee);
	bool surrogate_call(const syntax::CallExpr& expr, const ClassEntity* cls);
	void argument_dependent(const syntax::Slice<syntax::Expr*>& arguments, const Identifier* name, bool bound = false);
	void associated(const Type* type, uint32_t depth);
	void associate(Scope* scope);
	void associate_enclosing(const Entity* entity);
	void renew_association();
	Candidate best_viable(const syntax::Slice<syntax::Expr*>& arguments, uint32_t location,
		std::vector<FunctionEntity*>& set, ExprFacts* object, bool substitution = false,
		const ClassEntity* constructed = nullptr);
	void note_added_default_arguments(const FunctionEntity* function, const syntax::ParameterClause& clause);
	bool default_argument_visible(const FunctionEntity* function, uint32_t index) const;
	Resolution resolve(const syntax::Slice<syntax::Expr*>& arguments, std::vector<FunctionEntity*>& set,
		ExprFacts* object, Candidate& chosen, const ClassEntity* constructed = nullptr);
	static bool PerfectSequence(const Conversion& conversion, const ExprFacts* argument);
	bool implicit_sequence(const syntax::Expr& argument, const Type* parameter, Conversion& out);
	bool object_sequence(const ExprFacts* object, const FunctionEntity* function, bool ranking, Conversion& out);
	int compare_candidates(const Candidate& a, const Candidate& b, uint32_t count, bool object);
	// Records the arguments' conversions: the chosen candidate's sequences
	// (the implicit object's, if any, first, then `offset` on), or for an
	// indirect call the conversions to the function type's parameters.
	void convert_arguments(
		const syntax::Slice<syntax::Expr*>& arguments, const Type* function, const Candidate* chosen, uint32_t offset);
	// A function type with a member's implicit object parameter (the
	// --emit-semantics spelling of member functions).
	const Type* result_category(const Type* result, ValueCategory& category);

	// ---------------------------- operators (analyzer_operators.cpp)
	const Identifier* operator_key(TokenKind op);
	const Identifier* name_key(const syntax::NameComponent& component);
	LookupResult operator_function_lookup(const syntax::Name& name, Scope* scope);
	void operator_candidates(
		const Identifier* key, const syntax::Slice<syntax::Expr*>& operands, bool members_only, Scope* scope);
	uint32_t builtin_operator(TokenKind op, const syntax::Slice<syntax::Expr*>& operands, bool postfix,
		bool same_enumeration_list, const Type** parameters);
	// 13.3.1.2: the call of the operator function an operator expression
	// selects, or null for the built-in operator.
	ExprFacts* overloaded_operator(const syntax::Expr& expr, TokenKind op, const syntax::Slice<syntax::Expr*>& operands,
		bool postfix, Scope* scope);
	void operator_conversions(
		const syntax::Slice<syntax::Expr*>& operands, FunctionEntity* selected, const Candidate& chosen, Scope* scope);
	ExprFacts* binary_operator(const syntax::BinaryExpr& expr, Scope* scope);
	void class_builtin_candidates(TokenKind op, const syntax::Slice<syntax::Expr*>& operands, bool postfix);
	ExprFacts* unary_operator(const syntax::UnaryExpr& expr, bool postfix, Scope* scope);
	void check_operator_declaration(const FunctionEntity* function, uint32_t location);

	// ----------------------------------------- statements
	void condition_to_bool(const syntax::Condition& condition);
	const Type* switch_condition(const syntax::Condition& condition);
	void return_statement(const syntax::ValueStmt& stmt, Scope* scope);
	bool names_automatic_object(const syntax::Expr& expr) const;
	bool moves_from(ClassEntity* entity, const syntax::Expr& source);
	bool move_return_conversion(const syntax::Expr& source, const Type* result);
	const Conversion* finish_conversion(const syntax::Expr& expr, Conversion& conversion, bool operand);
	void label_statement(const syntax::LabeledStmt& stmt, Scope* scope);
	void check_gotos(const BodyContext& body);
	bool bypasses_initialization(const PendingGoto& jump, const PendingLabel& label);
	void case_label(const syntax::CaseStmt& stmt, Scope* scope);
	void initialize_variable(VariableEntity* variable, const Type* type, const syntax::Initializer* initializer,
		Scope* scope, uint32_t location);
	void default_initialize(VariableEntity* variable, const Type* type, uint32_t location);
	void check_const_default(ClassEntity* entity, const FunctionEntity* constructor, uint32_t location);
	bool nontrivial_destruction(const Type* type);
	void list_initialize(const syntax::BracedInitList& list, const Type* type, Scope* scope);
	void aggregate_initialize(const syntax::BracedInitList& list, ClassEntity* entity, Scope* scope);
	void aggregate_members(const syntax::BracedInitList& list, uint32_t& next, ClassEntity* entity, Scope* scope);
	void aggregate_element(const syntax::BracedInitList& list, uint32_t& next, const Type* type, Scope* scope);
	void initialize_element(const syntax::Expr& element, const Type* type, Scope* scope);
	FunctionEntity* implicit_constructor(ClassEntity* entity);
	FunctionEntity* queried_default_constructor(ClassEntity* entity);
	void record_default_arguments(FunctionEntity* function, const syntax::ParameterClause* clause, Scope* scope);
	void record_template_default_arguments(
		FunctionEntity* function, const syntax::ParameterClause& clause, Scope* scope);
	void instantiate_default_argument(FunctionEntity* function, uint32_t index);
	void type_default_arguments(FunctionEntity* function, const syntax::ParameterClause* clause, Scope* scope);
	void name_local_type(ClassEntity* entity);

	// ---------------------- construction (analyzer_construction.cpp)
	void declare_destructor(ClassEntity* entity, uint32_t location);
	bool implicit_destructor_nothrow(const ClassEntity* entity);
	bool destructor_deleted(const ClassEntity* entity);
	void declare_inherited_constructors(ClassEntity* entity);
	void record_inherited_constructor(FunctionEntity* inheriting);
	bool excluded_inherited_constructor(const FunctionEntity* function, const ClassEntity* constructed);
	void implicit_default_facts(ClassEntity* entity, FunctionEntity* constructor);
	FunctionEntity* default_constructor(ClassEntity* entity, uint32_t location);
	// 5.3.4 (a single object): the allocation function's call and the
	// object's initialization.
	static bool TypeIdHoldsAuto(const syntax::TypeId& type);
	const Type* new_auto_type(const syntax::NewExpr& expr, const syntax::TypeId& type_syntax, bool array, Scope* scope);
	ExprFacts* allocation(const syntax::NewExpr& expr, Scope* scope);
	ExprFacts* deletion(const syntax::DeleteExpr& expr, Scope* scope);
	bool array_new_list(const syntax::NewExpr& expr, const syntax::BracedInitList& list, const Type* element,
		bool& trailing, Scope* scope);
	void new_deallocation(const syntax::NewExpr& expr, const ClassEntity* entity, const Identifier* key, Scope* scope,
		const FunctionEntity* allocation);
	const FunctionEntity* usual_deallocation(const FunctionList* functions);
	LookupResult global_allocation_functions(const Identifier* key);
	// Whether the class's usual operator delete[] takes the size
	// (kSizedArrayDelete).
	bool sized_array_delete(const ClassEntity* entity);
	bool check_narrowing(const syntax::Expr& element, const Conversion* conversion, const Type* type, Scope* scope);
	bool narrowing_constant(
		const syntax::Expr& element, const Conversion* before, Scope* scope, Constant* integer, long double* floating);
	FunctionEntity* select_constructor(ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments,
		ConstructionKind kind, uint32_t location, bool* elide = nullptr);
	FunctionEntity* value_initialize(ClassEntity* entity, uint32_t location);
	// 8.5p16: initializes an object (or reference) of `type`; returns the
	// constructor a class object runs.
	FunctionEntity* initialize_class(
		ClassEntity* entity, const Type* type, const syntax::Initializer& init, Scope* scope, uint32_t location);
	FunctionEntity* direct_initialization(
		ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments, uint32_t location);
	ExprFacts* class_prvalue(const syntax::Expr& expr, const Type* type, const syntax::Initializer& view, Scope* scope);
	bool parenthesized_aggregate(ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments, Scope* scope);
	bool same_class_source(const ClassEntity* entity, const syntax::Expr& source);
	FunctionEntity* copy_initialization(ClassEntity* entity, const syntax::Expr& source, ConstructionKind kind,
		uint32_t location, bool in_place = false);
	const Conversion* record_copy(
		ClassEntity* entity, const Type* target, const syntax::Expr& source, uint32_t location, bool in_place = false);
	FunctionEntity* initialize_object(
		const Type* type, const syntax::Initializer& init, Scope* scope, uint32_t location);
	void constructor_plan(FunctionEntity* constructor, ClassEntity* entity, const syntax::CtorInitializer* list,
		Scope* scope, uint32_t location);
	void member_plans(const ClassEntity* entity, SubobjectInit* out);
	static const syntax::MemInitializer* plan_designated(const SubobjectInit& init);
	void member_plan(SubobjectInit& init, bool variant, bool designated, bool user, Scope* scope, uint32_t location);
	void inner_plan_facts(const SubobjectInit& init, bool& trivial, bool& nothrow, bool& initialized);
	void anonymous_default_facts(const SubobjectInit& init, bool& nontrivial, bool& deleted);
	void delegate_construction(
		FunctionEntity* constructor, ClassEntity* entity, const syntax::MemInitializer& initializer, Scope* scope);
	SubobjectInit* mem_initializer_target(
		FunctionEntity* constructor, ClassEntity* entity, const syntax::MemInitializer& initializer, Scope* scope);
	void member_default_initializer(VariableEntity* member, Scope* scope);
	void typed_inheriting_plans(const VariableEntity* member);
	void instantiate_member_initializer(VariableEntity* member);
	bool subobject_destructor_usable(
		const FunctionEntity* destructor, const ClassEntity* cls, const ClassEntity* entity, bool member);
	bool subobjects_destructible(const ClassEntity* entity);
	bool members_destructible(const ClassEntity* holder, const ClassEntity* entity);
	void check_subobject_destruction(const ClassEntity* entity, const Scope* scope, uint32_t location);
	void check_destruction(const Type* type, const Scope* scope, uint32_t location);
	void check_temporary_destruction(const ExprFacts* facts, uint32_t location);
	void check_binding_temporary(const ExprFacts* from, const Conversion& conversion, uint32_t location);
	void check_character_array(const Type* type, const syntax::Expr& single, const ExprFacts* facts, uint32_t location);
	// 13.3.3.1.2: a user-defined conversion to a class type (or a
	// reference to one) by a converting constructor; false if none or
	// ambiguous.
	bool user_conversion(const ExprFacts* from, const Type* target, Conversion& out, bool direct = false);
	// The selected user-defined conversion applied where it is used: its
	// function checked (deleted, access) and odr-used, with the class
	// objects its steps initialize by constructors (operand_constructor).
	void apply_user_conversion(const ExprFacts* from, Conversion& conversion, uint32_t location);
	FunctionEntity* operand_constructor(ClassEntity* entity, const Type* type, ValueCategory category,
		ConstructionKind kind, uint32_t location, Conversion& binding);
	void check_final_copy(ClassEntity* entity, uint32_t location);

	// ------------------------ user conversions (analyzer_conversions.cpp)
	void conversion_functions(const ClassEntity* entity, std::vector<FunctionEntity*>& out, bool templates = false);
	struct ConversionSet;
	ConversionSet conversion_set(const ClassEntity* entity);
	void visible_conversions(const ClassEntity* entity);
	void count_conversion_hiders(const FunctionList* own, uint32_t delta);
	void dominated_conversions(const ClassEntity* cls);
	bool conversion_dominated(const ClassEntity* cls, const Binding* binding);
	const Binding* conversion_binding(const ClassEntity* source, const FunctionEntity* function);
	bool conversion_subobject(const ClassEntity* source, const FunctionEntity* function, uint32_t location);
	FunctionList* conversion_lookup(const ClassEntity* cls, const Type* target, uint32_t location);
	bool direct_reference_conversion(
		const ExprFacts* from, const Type* target, const ClassEntity* source, bool direct, Conversion& out);
	int user_candidate_order(const FunctionEntity* a, const Conversion& a_first, const Conversion& a_second,
		const FunctionEntity* b, const Conversion& b_first, const Conversion& b_second);
	ExprFacts* convert_class_operand(const syntax::Expr& expr, const FunctionEntity* function);
	bool convert_to_scalar(const syntax::Expr& expr, const Type* target, bool direct);
	const FunctionEntity* unique_conversion(const ClassEntity* entity, bool (*accepts)(const Type*));

	// ------------------------------ copy and move (analyzer_copy.cpp)
	CopyKind copy_kind(const FunctionEntity* function, const ClassEntity* entity) const;
	void classify_copy_members(ClassEntity* entity);
	void declare_implicit_copy_members(ClassEntity* entity);
	void implicit_members_for(const Scope* scope, const Identifier* name);
	FunctionEntity* implicit_copy_member(ClassEntity* entity, CopyKind kind, bool deleted);
	bool const_copy_source(const ClassEntity* entity, bool assignment);
	bool accepts_const_source(const ClassEntity* entity, bool assignment);
	void defaulted_copy_facts(FunctionEntity* function, ClassEntity* entity, CopyKind kind);
	void copy_parameter(
		SubobjectInit& init, ClassEntity* cls, CopyKind kind, uint8_t cv, const ClassEntity* from, bool& deleted);
	FunctionEntity* subobject_copy(ClassEntity* cls, CopyKind kind, uint8_t cv, uint8_t object_cv,
		const ClassEntity* from, bool& deleted, ConstructionKind construction = ConstructionKind::Direct,
		bool member = false);
	void constructor_candidates(ClassEntity* entity, const syntax::Slice<syntax::Expr*>& arguments,
		ConstructionKind kind, std::vector<FunctionEntity*>& set);
	void assignment_candidates(ClassEntity* entity, std::vector<FunctionEntity*>& set);

	// ---------------------------- virtual functions (analyzer_virtual.cpp)
	void virtual_declaration(FunctionEntity* function, const Specifiers& specifiers, const DeclaratorInfo& info,
		bool first, uint32_t location);
	void override_facts(FunctionEntity* function, ClassEntity* cls, uint32_t location);
	bool result_needs_adjustment(const FunctionEntity* function, const FunctionEntity* overridden, uint32_t location);
	void check_override_result(
		const FunctionEntity* function, const FunctionEntity* overridden, const ClassEntity* cls, uint32_t location);
	void pure_specifier(
		FunctionEntity* function, const syntax::Initializer* initializer, bool member, uint32_t location);
	void push_override_bases(const ClassEntity* entity, const BaseClass* primary, const ClassEntity* virtual_primary);
	void overrides(
		FunctionEntity* function, const FunctionEntity* found, bool primary, ClassEntity* cls, uint32_t location);
	// The class's virtual facts at completion (its table built on demand).
	void virtual_facts(ClassEntity* entity, uint32_t location);
	void share_virtual_primaries(ClassEntity* entity, uint32_t location);
	FunctionEntity* chain_overrider(const ClassEntity* head, const FunctionEntity* function, const ClassEntity* vbase);
	void place_virtual(FunctionEntity* function, VirtualSlot* slots);
	// The table group of a class with secondary tables (VtableGroup), at
	// completion; a class's own function overriding `function`, or null.
	void vtable_group(ClassEntity* entity, uint32_t location);
	void group_slots(const ClassEntity* top, const ClassEntity* cls, uint32_t segment);
	void group_below(const ClassEntity* top, const ClassEntity* cls, uint32_t segment, uint32_t within);
	void group_splice(const ClassEntity* top, const ClassEntity* cls, uint32_t within, bool chained);
	void group_outside(uint32_t location);
	bool group_dominates(const VtableEntry& over, const VtableEntry& under);
	uint32_t group_chain_depth(const void* key);
	FunctionEntity* declared_overrider(const ClassEntity* cls, const FunctionEntity* function);
	void virtual_definition(FunctionEntity* function, uint32_t location);
	const FunctionEntity* virtual_deallocation(ClassEntity* entity, bool fatal, uint32_t location);
	// 10.4p2-p3: errors for an object (or array element, parameter or
	// result) of an abstract class type.
	bool check_abstract_object(const Type* type, uint32_t location);
	bool complete_pointee(const Type* pointer, uint32_t location);

	// ------------------------------ access control (analyzer_access.cpp)
	Access member_access() const;
	Access redeclared_access(const Scope* scope, const Binding* binding) const;
	void befriend_class(ClassEntity* friend_class, Scope* scope);
	void befriend_function(FunctionEntity* function, ClassEntity* befriending);
	void record_friend(const ClassEntity* befriending, const Entity* befriended);
	bool befriends(const ClassEntity* befriending, const Entity* befriended);
	const Scope* access_point(const Scope* scope) const;
	Scope* member_declaration_class(const syntax::Declarator* declarator, Scope* scope);
	bool member_or_friend_of(const ClassEntity* entity, const Scope* scope);
	bool access_as_member(const ClassEntity* naming, const ClassEntity* declaring, Access declared, Access& out);
	bool access_walk(
		const ClassEntity* naming, const ClassEntity* declaring, Access declared, Access& out, PointerMap& paths);
	uint32_t access_chain(const ClassEntity* top, const ClassEntity* declaring, Access declared);
	const ClassEntity* derived_context(const ClassEntity* naming, const ClassEntity* declaring, Access declared,
		const Scope* scope, const ClassEntity* object, const ClassEntity** via, bool public_path, bool& object_context);
	bool access_object(const ClassEntity* privilege, const ClassEntity* object, const ClassEntity** via);
	bool base_accessible(const ClassEntity* derived, const BaseClass* base, const Scope* scope,
		const ClassEntity* object = nullptr, const ClassEntity** via = nullptr);
	// 11.2p5 for a member of `declaring` with access `declared` named in
	// `naming`; `location` is the use's (a hierarchy too deep to search
	// is an error there).
	bool accessible(const ClassEntity* naming, const ClassEntity* declaring, Access declared, const Scope* scope,
		const ClassEntity** via, uint32_t location, const ClassEntity* object = nullptr);
	bool accessible_below(const ClassEntity* naming, const ClassEntity* declaring, Access declared, const Scope* scope,
		const ClassEntity** via, const ClassEntity* object);
	bool accessible_declarations(const Binding* binding, const ClassEntity* naming, const Scope* scope,
		const ClassEntity** via, uint32_t location, const ClassEntity* object);
	const Binding* member_declaration(const ClassEntity* cls, const Binding* binding) const;
	bool access_blocked(const ClassEntity* cls, const ClassEntity* declaring) const;
	// Errors unless the member is accessible at `scope` (11.2, 11.4).
	void check_access(const Binding* binding, const ClassEntity* naming, const ClassEntity* object, const Scope* scope,
		uint32_t location);
	void check_unqualified_access(const Binding* binding, const Scope* scope, uint32_t location);
	void check_function_access(const FunctionEntity* function, const FunctionList* functions, const ClassEntity* naming,
		const ClassEntity* object, const Scope* scope, uint32_t location);
	void check_special_access(const FunctionEntity* function, const ClassEntity* entity, const Scope* scope,
		uint32_t location, const ClassEntity* object = nullptr);
	void check_base_access(const ClassEntity* derived, const ClassEntity* base, const Scope* scope, uint32_t location);
	bool leads_to_base(const ClassEntity* cls, const ClassEntity* base);
	uint32_t leads_chain(const ClassEntity* top, const ClassEntity* base);
	// 11p6-p7 for a namespace-scope declaration whose declarator-id is not
	// qualified: its failed checks wait (from begin_declaration_access's
	// mark; the caller restores access_deferral_) until the declared entity
	// is known; settling checks those from `first` in it, then keeps those
	// before `keep` (the decl-specifiers', shared by later declarators).
	size_t begin_declaration_access(Scope* scope);
	void settle_declaration_access(size_t first, size_t keep, const Binding* declared);
	bool defer_access(const Binding* binding, const ClassEntity* naming, const ClassEntity* object, const Scope* scope,
		uint32_t location);

	// ---------------------------- exception facts (analyzer_exceptions.cpp)
	void pending_noexcept(FunctionEntity* function, const syntax::DeclaratorSuffix* suffix,
		const syntax::ParameterClause* parameters, Scope* scope);
	bool noexcept_value(const FunctionEntity* function, const syntax::Expr* operand, Scope* scope);
	static bool NonThrowingSpecification(const syntax::DeclaratorSuffix* suffix);
	static bool HasExceptionSpecification(const syntax::DeclaratorSuffix* suffix);
	void record_throw_specification(FunctionEntity* function, const syntax::DeclaratorSuffix* suffix, Scope* scope);
	void resolve_throw_specification(
		FunctionEntity* function, const syntax::FunctionQualifier& qualifier, Scope* scope);
	void check_complete_throw_type(const Type* type, uint32_t location);
	void finish_class_exceptions(size_t first_throw, size_t first_override);
	void note_exception_override(const FunctionEntity* function, const FunctionEntity* found, uint32_t location);
	void check_exception_overrides(size_t first, bool final);
	bool allowed_exceptions(const FunctionEntity* function, TypeList& out);
	bool public_base(const ClassEntity* base, const ClassEntity* derived);
	bool handler_allows(const Type* handler, const Type* thrown);
	void substitute_throw_specification(FunctionEntity* function);
	static const Type* AdjustedThrowType(TypeTable& types, const Type* type);
	static bool DeclaratorSpecifiesExceptions(const syntax::Declarator* declarator);
	void note_nothrow_target(
		const Entity* entity, const Type* type, const syntax::Declarator* declarator, Scope* scope);
	void check_nothrow_initializer(const syntax::Initializer& initializer);
	bool nothrow_target(const syntax::Expr& callee);
	void check_redeclared_exceptions(
		const FunctionEntity* function, const DeclaratorInfo& info, Scope* scope, uint32_t location);
	void check_pending_exceptions();
	void check_fixed_template_noexcept(size_t index);
	ExprFacts* noexcept_expression(const syntax::TypeTraitExpr& expr, Scope* scope);
	bool function_may_throw(const FunctionEntity* function);
	bool conversion_may_throw(const Conversion* conversion, uint32_t depth);
	bool temporary_may_throw(const ExprFacts* facts);
	bool may_throw(const syntax::Expr& expr, uint32_t depth);

	// -------------------------------- constexpr rules (analyzer_constexpr.cpp)
	bool literal_type(const Type* type);
	// ------------------------------ GNU type traits (analyzer_traits.cpp)
	ExprFacts* builtin_trait(const syntax::BuiltinTraitExpr& expr, Scope* scope);
	bool trait_types(const syntax::BuiltinTraitExpr& expr, Scope* scope, SmallList<const Type*, 4>& out);
	bool trait_constant(const syntax::BuiltinTraitExpr& expr, Scope* scope, Constant& value);
	const Type* trait_type(GnuTrait trait, const Type* type, uint32_t location);
	const Type* transformed_type(GnuTrait trait, const Type* type, uint32_t location);
	bool trait_value(
		GnuTrait trait, const Type* const* types, uint32_t count, uint32_t location, Scope* scope, uint64_t& value);
	const Type* trait_result_type(GnuTrait trait);
	bool shape_trait(GnuTrait trait, const Type* type);
	bool offsetof_value(const syntax::BuiltinTraitExpr& expr, Scope* scope, uint64_t& offset);
	bool member_offset(ClassEntity* cls, const Entity* member, uint64_t& bytes, uint32_t location);
	bool trait_destructible(const Type* type, uint32_t location, Scope* scope, bool& nothrow, bool& trivial);
	bool trait_convertible(const Type* from, const Type* to, uint32_t location, Scope* scope, bool& nothrow);
	bool trait_temporary_binding(
		GnuTrait trait, const Type* target, const Type* source, uint32_t location, Scope* scope);
	bool trait_invocable(const Type* const* types, uint32_t count, uint32_t location, Scope* scope, bool& nothrow);
	bool trivially_copyable(const Type* type);
	bool unique_object_representations(const Type* type);
	bool representation_bits(const Type* type, uint64_t& bits);
	bool class_representation_bits(ClassEntity* entity, uint64_t& bits);
	bool subobjects_copy_trivially(ClassEntity* entity);
	bool subobjects_default_trivially(ClassEntity* entity);
	bool gnu_trivial_default_constructor(ClassEntity* entity);
	bool default_constructor_declarations_trivial(ClassEntity* entity);
	bool default_constructor_shape_trivial(ClassEntity* entity);
	void copy_members(ClassEntity* entity, bool assignment, SmallList<const FunctionEntity*, 8>& out);
	bool copy_member_trivial(const FunctionEntity* function, ClassEntity* entity);
	const FunctionEntity* selected_copy_member(ClassEntity* entity, CopyKind kind);
	bool subobjects_member(ClassEntity* entity, CopyKind kind, bool nothrow);
	bool subobject_members_select(ClassEntity* entity, CopyKind kind, bool nothrow);
	bool copy_selection(ClassEntity* entity, CopyKind kind, SmallList<const FunctionEntity*, 8>& best);
	bool legacy_trait(GnuTrait trait, const Type* type);
	bool trait_literal_type(const Type* type);
	bool trivially_default_constructible(const Type* type);
	bool standard_layout(const Type* type);
	bool standard_layout_class(ClassEntity* entity, uint32_t depth);
	static bool has_data_members(const ClassEntity* entity);
	// std::declval<T>()'s value, or with `prvalue` a prvalue of a
	// non-reference T (the VAL<T> of C++23's 20.15.5.4).
	syntax::Expr* declval_operand(const Type* type, uint32_t location, bool prvalue = false);
	bool trait_constructible(const Type* target, const Type* const* arguments, uint32_t count, uint32_t location,
		Scope* scope, bool& nothrow, bool& trivial);
	bool trait_assignable(
		const Type* target, const Type* source, uint32_t location, Scope* scope, bool& nothrow, bool& trivial);
	bool integer_pack_argument(const syntax::TemplateArgument& argument, Scope* scope, TypeList& out);
	bool constexpr_constructor(const FunctionEntity* constructor, uint32_t depth);
	bool constexpr_default(const FunctionEntity* constructor, uint32_t depth);
	bool constexpr_default_plan(const FunctionEntity* constructor, uint32_t depth);
	bool constexpr_default_subobject(const SubobjectInit& init, bool variant, uint32_t& initialized, uint32_t depth);
	bool constexpr_inheriting(const FunctionEntity* constructor, uint32_t depth);
	bool constexpr_inheriting_plan(const FunctionEntity* constructor, uint32_t depth);
	bool constant_member_initializers(const SubobjectInit* inits, uint32_t count, uint32_t depth);
	bool constexpr_copy(const FunctionEntity* constructor, uint32_t depth);
	bool constexpr_copy_plan(const FunctionEntity* constructor, uint32_t depth);
	bool constexpr_assignment(const FunctionEntity* assignment, uint32_t depth);
	bool constexpr_assignment_plan(const FunctionEntity* assignment, uint32_t depth);
	bool remember_constexpr(const FunctionEntity* function, bool result);
	bool settled_constructor_inits(const SubobjectInit* inits, uint32_t count, bool initializers);
	void check_defaulted_constexpr(FunctionEntity* function, uint32_t location);
	void check_defaulted_constexprs();
	static const ClassEntity* ObjectClassOf(const Type* type);
	void check_constexpr_function(const FunctionEntity* function, const Type* type, uint32_t location);
	void check_constexpr_constructor(const FunctionEntity* constructor, uint32_t location);
	void check_constexpr_subobject(const SubobjectInit& init, bool variant, uint32_t location);
	void check_constexpr_member_initializer(const SubobjectInit& init, uint32_t location);
	void check_constexpr_variable(const VariableEntity* variable, uint32_t location);
	void integral_constant_operand(const syntax::Expr& expr, Scope* scope, const Type* target);

	// ------------------------------------------------ constants
	bool constant_value(const syntax::Expr& expr, Scope* scope, Constant& value, bool required);
	bool converted_constant(const syntax::Expr& expr, Constant& value, bool required);
	void constant_operand(const syntax::Expr& expr, Scope* scope, bool resolved = true);
	// 5.19 for an expression of floating type the analysis typed: its value
	// in its type's precision; false when it is not a constant expression.
	bool floating_constant(const syntax::Expr& expr, Scope* scope, long double& value);
	bool variable_constant(const VariableEntity* variable, Constant& value);
	// The constant value of a variable usable in constant expressions
	// (sema/constant_values.h), or null.
	const ConstValue* variable_value(const VariableEntity* variable);
	// A function's definition analyzed for a constant evaluation calling it
	// (instantiated, or a member's body ahead of its class's completion).
	bool constexpr_definition(const FunctionEntity* function);
	void set_evaluated(const VariableEntity* variable, const ConstValue* value);
	bool type_size(const Type* type, uint64_t& size, uint64_t& alignment, uint32_t location);
	bool member_size(const Type* type, uint64_t& size, uint64_t& alignment, uint32_t location);
	void record_alias_alignment(const Specifiers& specifiers, const Binding* declared, uint64_t own);
	uint32_t alias_alignment_of(const syntax::Expr& expr, bool pointee) const;
	uint64_t declared_alignment(const void* declared, uint64_t natural) const;
	uint64_t declared_alignment(const DataMember* member, uint64_t natural) const;
	void filter_mode_alias(Specifiers& specifiers);
	const Type* apply_mode(const Type* type, FundamentalType target, uint32_t location, bool complex_mode = false);
	const Type* declaration_mode_type(
		const Type* type, const Specifiers* specifiers, const syntax::Attribute* attributes);
	const Type* mode_type(const Type* type, const Identifier& mode, uint32_t location);
	const Type* vector_type(const Type* type, const syntax::Expr& size, bool bytes, Scope* scope, uint32_t location);
	const Type* checked_vector(const Type* lane, uint64_t size, bool bytes, uint32_t location);
	bool vector_reinterpretation(const Type* from, const Type* to);
	bool lay_out(ClassEntity* root, uint32_t location);
	bool class_layout(ClassEntity* entity, uint32_t location);
	// An object's size in bits fits a signed 64-bit offset (Clang's limit,
	// below GCC's PTRDIFF_MAX bytes): the layout counts bits in 64 bits
	// (`bytes` may be several sizes or'ed).
	void check_class_size(uint64_t bytes, uint32_t location) const
	{
		if ((bytes >> 60) != 0)
			error(location, "a class too large (its size reaches 2^60 bytes)");
	}
	void place_overlapping_member(
		DataMember* member, uint64_t member_alignment, uint64_t natural, uint64_t& data_bits, uint32_t location);
	void record_member(const Type* type, uint64_t offset, uint64_t tail, uint32_t location);
	bool empty_subobjects(
		const ClassEntity* root, uint64_t offset, EmptyWalk walk, uint32_t location, bool complete = false);
	void allocate_virtual_bases(
		ClassEntity* entity, uint64_t& data_bits, uint64_t& size, uint64_t& alignment, uint32_t location);
	void start_layout(const ClassEntity* entity, uint64_t empty_base_limit, uint32_t location);
	// The bit-field runs of the class being laid out (from `first` in
	// bit_runs_) and their storage units (see assign_bit_units).
	struct BitRunField
	{
		DataMember* member;
		uint64_t bit, width, storage, unit_begin, unit_bytes, split;
		bool packed, shared;
	};
	struct BitRun
	{
		size_t first, last;
		uint64_t lo, hi;
		bool open;
	};
	void begin_bit_run(size_t first, uint64_t first_byte);
	void end_bit_run(size_t first, uint64_t limit);
	void assign_bit_units(size_t first, uint64_t open_limit, uint32_t location);
	void assign_bit_segment(
		const BitRun& run, size_t first, size_t last, uint64_t begin, uint64_t bytes, uint32_t location);
	void assign_bit_fields(size_t first, size_t last, uint64_t lo, uint64_t hi, uint32_t location);
	void place_table_pointer(const ClassEntity* entity, const BaseClass* primary, uint64_t& data_bits, uint64_t& size,
		uint64_t& alignment, uint32_t location);
	// The base sharing a class's table pointer (VirtualFacts::primary_base;
	// see sema/primary_bases.cpp), a layout's indirect primary bases and
	// their claimants' offsets.
	const BaseClass* layout_primary(const ClassEntity* entity);
	const ClassEntity* itanium_virtual_primary(const ClassEntity* cls);
	bool indirect_primaries(const ClassEntity* entity);
	bool has_indirect_primaries(const ClassEntity* entity);
	void claim_primaries(const ClassEntity* entity, uint32_t location);
	void plan_claims(const ClassEntity* entity, const BaseClass* primary);
	bool claimed_conflict(const BaseClass* resolver, uint64_t offset, uint32_t location);
	void record_claimed(const BaseClass* resolver, uint64_t offset, EmptyWalk walk, uint32_t location);
	bool base_conflict(const BaseClass* base, const ClassEntity* base_class, uint64_t offset, uint32_t location);
	bool member_conflict(const Type* type, uint64_t offset, uint32_t location);
	void spend_layout_work(uint32_t location);
	bool place_empty_subobject(const ClassEntity* entity, uint64_t offset, bool insert);

	// ------------------------------------------- graph construction
	void predeclare_allocation(TokenKind op, bool array, const Type* type, AllocationRole role, Scope* scope);
	Scope* new_scope(ScopeKind kind, Scope* parent, Scope* print_parent, Entity* entity, const char* label);
	void attach_scope(Scope* scope, Scope* print_parent);
	Binding* new_binding(BindingKind kind, const Identifier* name, Entity* entity, const Type* type);
	void print_in(Scope* scope, Binding* binding)
	{
		(scope->last_binding == nullptr ? scope->bindings : scope->last_binding->next) = binding;
		scope->last_binding = binding;
	}
	void declare_in(Scope* scope, Binding* binding);
	template<typename T>
	T* new_entity(EntityKind kind, const Identifier* name, Scope* owner, const Type* type);
	const char* copy_label(const char* text, size_t length);
	const char* asm_label(const TokenSpelling& spelling);
	[[noreturn]] void error(uint32_t location, const char* message) const;
	// An invalid type or expression: in a soft substitution its recorded
	// failure (false; the caller returns its own), else an error.
	bool substitution_error(uint32_t location, const char* message, bool transient = false);
	bool failed() const { return failure_.message != nullptr; }
	// A hosted wrapper's call of a reserved builtin this compiler lacks
	// (see define_function) is recorded as a failure (kDeferredCall): each
	// analysis returns null up to where the call's full-expression began
	// (deferrable_statement and the like), which consumes it.
	void defer_reserved_call(uint32_t location);
	bool deferred_reserved_call() const { return failure_.message == kDeferredCall; }
	void consume_deferred_call() { failure_ = SubstitutionFailure(); }
	static const char kDeferredCall[];
	void check_stack(uint32_t location) const
	{
		char marker;
		if (stack_bytes_used(stack_base_, &marker) > stack_budget_)
			error(location, "constructs nested too deeply");
	}
	bool stack_half_used() const;
	static Scope* declaration_scope(Scope* scope);
	static Scope* enclosing_namespace(Scope* scope);
	bool hidden_below_namespace(const Scope* block, const Identifier* name) const;
	static bool encloses(const Scope* outer, const Scope* inner);

	SemanticGraph& graph_;
	AnalysisMode mode_;
	bool hosted_ = false;
	bool cxx14_ = false;
	bool exceptions_ = true;
	bool ignore_narrowing_ = false;
	const SourcePositions* positions_ = nullptr;
	TypeTable& types_;
	NameLookup& names_;
	IdentifierTable& identifiers_;
	Arena& arena_;
	std::vector<DeferredBody> deferred_;
	// A deferred constexpr member function body's index in deferred_ (an
	// entry is checked against its function: the queue is cut back as each
	// outermost class completes), for constexpr_definition.
	PointerMap deferred_constexpr_;
	// constexpr_default/copy/assignment's answers (1 or 0) within one
	// outermost query (constexpr_queries_ deep, see ConstexprQuery).
	PointerMap constexpr_members_;
	// Positive default/inheriting-constructor facts whose complete class's
	// plan and queried subconstructors are settled. Other queries retain
	// the per-outer-query memo above while declarations are still changing.
	PointerMap constexpr_constructor_facts_;
	uint32_t constexpr_queries_ = 0;
	friend struct ConstexprQuery;
	uint32_t class_depth_;
	// How many of the classes being defined are unnamed.
	uint32_t unnamed_class_depth_ = 0;
	// Inside a template declaration: dependent names are not resolved.
	uint32_t template_depth_;
	// The template declaration whose declaration is being analyzed: its
	// parameter list and scope (the templated entity is declared with
	// that scope).
	const syntax::TemplateParameterList* template_list_ = nullptr;
	Scope* template_scope_ = nullptr;
	// The class template an out-of-class member declaration under analysis
	// belongs to (its declarator-id's X<T...>::, see qualifying_template).
	ClassEntity* template_owner_ = nullptr;
	// The decl-specifiers of such an out-of-class member declaration are
	// being analyzed (they precede its declarator-id).
	bool owner_specifiers_ = false;
	// The class or namespace whose member an out-of-class declaration
	// redeclares, while its decl-specifiers or a member template's own
	// template-head (which precede its declarator-id) are analyzed: their
	// placeholders are keyed there (see dependent_expression).
	Scope* placeholder_home_ = nullptr;
	// A non-templated class whose friend template is analyzed (14.5.4);
	// where a placeholder substitution checks its copy's (access_scope_'s)
	// own names (see substituted_value, access_point).
	const Scope* namespace_friend_class_ = nullptr;
	const Scope* access_point_ = nullptr;
	const Scope* access_scope_ = nullptr;
	// Inside `template<>`: the declaration's first class head or declarator
	// declares an explicit specialization (14.7.3), and takes the flag.
	bool explicit_specialization_ = false;
	// The lookup limit is a non-template declaration's own point (its
	// deferred noexcept-specification's, see function_nothrow), not an
	// instantiation's definition context (see global_allocation_functions).
	bool lexical_limit_ = false;
	// The number of nested `template<>` the declaration is under (read
	// while explicit_specialization_ holds, see explicit_function_specialization).
	uint32_t specialization_levels_ = 0;
	std::vector<TemplateParameterEntity*> template_entities_;
	struct FixedTemplateValue
	{
		const Type* value;
		const Type* target;
		bool unknown;
	};
	// Fixed body-local argument facts live for this analysis and remain valid
	// in copies of their syntax. The conversion context is part of the key.
	std::vector<FixedTemplateValue> fixed_template_values_;
	// The nesting of the instantiation being analyzed (0 outside any).
	uint32_t instantiation_depth_ = 0;
	// Positional template parameters by depth, kind and index (see
	// TemplateParameterEntity), created on first use.
	std::vector<std::vector<TemplateParameterEntity*>> positional_;
	// Dependent member placeholders by (qualifier, name) (see
	// dependent_member).
	FlatMap<PlaceholderKey, Binding*, PlaceholderHash> placeholders_;
	// Decltype placeholders by their operand's structural key (see
	// dependent_decltype), the key's words built in the scratch; and each
	// placeholder's type by (placeholder, depth, canonical arguments) once
	// substituted (see substitute_decltype).
	FlatMap<DecltypeKey, TemplateParameterEntity*, DecltypeHash> decltype_placeholders_;
	// Keying the equivalence of placeholders as redeclarations compare them
	// (a dependent callee by its spelling, see equivalent_representative);
	// whether a placeholder's own key named one by its lookup.
	bool equivalence_keying_ = false;
	bool keyed_callee_lookup_ = false;
	// Whether it names a dependent callee at all (found or not).
	bool keyed_callee_ = false;
	// Placeholders by that equivalence key, and the representative of each
	// placeholder that has an earlier equivalent (declared_type's).
	FlatMap<DecltypeKey, TemplateParameterEntity*, DecltypeHash> decltype_equivalents_;
	// The class scopes a using-declaration introduced a base's member
	// template into (see introduce).
	PointerMap introduced_templates_;
	// The members of complete copies of member-specifications (an
	// instance's member class templates) paired with the class template's
	// member template's members of the same position, a class at a time
	// (see outside_pattern_member): member -> index in outside_members_,
	// and the classes paired.
	PointerMap outside_member_index_;
	std::vector<const FunctionEntity*> outside_members_;
	PointerMap outside_paired_classes_;
	// The specializations instantiated from a partial specialization: the
	// arguments deduced for its parameters (14.5.5.1p2), which the frames of
	// its members' definitions outside it bind (14.5.5.3p1; see
	// enclosing_arguments): facts -> index in partial_deduced_.
	// Member partial definitions and their copies share a typed origin;
	// definitions -> index in member_partial_patterns_. The selected copy
	// keeps the origin in ClassTemplateFacts::partial, without rematching.
	PointerMap member_partial_definitions_;
	std::vector<ClassEntity*> member_partial_patterns_;
	PointerMap partial_deduced_index_;
	std::vector<const Type* const*> partial_deduced_;
	// Prototype scopes' parameters' positions (see parameter_word).
	mutable PointerMap parameter_positions_;
	PointerMap placeholder_representatives_;
	std::vector<const TemplateParameterEntity*> representatives_;
	std::vector<uint64_t> decltype_words_;
	std::vector<QualifiedNameWords> qualified_names_;
	// The declarator parameters the operand whose key is being built names
	// (TemplateParameterEntity::named_parameters' form; bit 63: some are
	// unknown), null outside one.
	uint64_t* named_parameters_ = nullptr;
	DecltypeKey kept_key(const DecltypeKey& key);
	// The key being built names a dependent entity or type (placeholder_words)
	// or applies sizeof... (TemplateParameterEntity::sizeof_packs).
	bool keyed_dependent_ = false;
	bool keyed_sizeof_pack_ = false;
	const Type* equivalent_placeholder(const TemplateParameterEntity* placeholder, uint32_t depth,
		const Type* const* canonical, uint32_t count, DecltypeKey& key, bool& shared);
	bool names_placeholder(const Type* type, uint32_t level) const;
	FlatMap<SubstitutionKey, const Type*, SubstitutionHash> decltype_substitutions_;
	uint32_t anonymous_enums_;
	// Inside a class object's initializer (see initializer_names).
	bool suppress_implicit_;
	// The access of the member declarations being analyzed (a class body's
	// access-specifiers), and the class scope whose member-specification
	// they are in.
	TokenKind current_access_;
	Scope* member_scope_;
	// The scope of the expression being typed (the point access control
	// and conversions to bases are checked at), and the class a member
	// function name was last looked up in (the naming class of the call
	// that resolves it, 11.2p5).
	Scope* expression_scope_;
	const ClassEntity* naming_class_;
	// What the last qualified lookup_name's nested-name-specifier named,
	// and the class scope of the member a namespace-scope declaration is
	// declaring (its access context, 11p6).
	NamePrefix last_prefix_;
	Scope* declaration_class_;
	// The function or variable declaration declare_function or
	// declare_variable last returned from redeclared an earlier one (its
	// ABI tags may only repeat the first one's); the attributes of the
	// lone elaborated-type-specifier being declared (`class-key
	// __attribute__((abi_tag(...))) identifier ;`).
	bool redeclared_;
	const syntax::Attribute* lone_class_attributes_;
	// The namespace scope of the declaration whose failed access checks
	// wait for its entity, and those checks (see settle_declaration_access).
	Scope* access_deferral_;
	std::vector<PendingAccess> pending_access_;
	// Each friend declaration as (befriending class, friend): a class, a
	// function, or a function template, whose every specialization it
	// befriends (14.5.4p1); whether a point is in a friend is one probe per
	// class or function enclosing it (see member_or_friend_of).
	FlatMap<EntityPair, bool, EntityPairHash> friendships_;
	// The entities each closure captures (by closure and entity), for the
	// lambdas being analyzed (lambda_bodies_ of them): dropped when the
	// outermost one's body is done.
	FlatMap<EntityPair, bool, EntityPairHash> captured_;
	uint32_t lambda_bodies_ = 0;
	// While a constructor plan initializes a base subobject: the planned
	// class, whose object the base's constructor acts on (11.4).
	const ClassEntity* base_object_;
	// Inside a linkage-specification for C (7.5), directly in one that is
	// not braced (its declaration is treated as extern, 7.5p7), and inside
	// any linkage-specification (an explicit language linkage, 7.5p5).
	bool c_language_;
	bool linkage_extern_;
	bool linkage_specified_;
	// The block-scope declarations of functions and variables with linkage
	// that denote no earlier declaration (3.5p6): members of their
	// innermost enclosing namespace (`space`) whose names are not declared
	// there, which a later declaration there redeclares. By name: 1 + the
	// newest record's index, each record 1 + its predecessor's (0 ends).
	struct BlockLinkage
	{
		const Scope* space;
		const Entity* entity;
		bool c_linkage;
		uint32_t next;
	};
	std::vector<BlockLinkage> block_linkages_;
	PointerMap block_linkage_index_;
	uintptr_t stack_base_;
	uintptr_t stack_budget_;
	const Identifier* constructor_key_;
	const Identifier* destructor_key_;
	const Identifier* conversion_key_;
	const Identifier* nullptr_t_;
	const Identifier* func_;
	const Identifier* function_name_;
	const Identifier* pretty_function_;
	// GNU's complex type specifiers (`_Complex`, `__complex__`,
	// `__complex`) and `__int128`, type names here (see type_specifiers).
	const Identifier* complex_keys_[3];
	const Identifier* int128_key_;
	// Registry builtin names already looked up, by identifier id: 1 none,
	// 2 a builtin (see builtin_function).
	std::vector<uint8_t> builtin_names_;
	const Identifier* assign_key_;
	// `operator new[]` and `operator delete[]` (as name_key keys them).
	const Identifier* new_array_key_;
	const Identifier* delete_array_key_;
	// The type of an `auto` decl-specifier until deduction.
	const Type* auto_placeholder_;
	// The prototype scope of the innermost trailing return type, until
	// declarator() attaches it to the dump.
	PendingPrototype pending_prototype_;
	std::vector<const Type*> parameter_scratch_;
	std::vector<EnumeratorEntity*> enumerator_scratch_;
	std::vector<PendingExpr> expression_work_;
	// The class being laid out: the empty class subobjects placed so far
	// (an open-addressed set of type and byte offset), the largest such
	// offset, the gap EmptyWalk::RecordReachable skips, the work left (see
	// class_layout) and the last conflict found (class, subobject met).
	std::vector<EmptySubobject> empty_subobjects_;
	std::vector<EmptySubobject> empty_work_;
	std::vector<LayoutFrame> layout_frames_;
	std::vector<BitRun> bit_runs_;
	std::vector<BitRunField> bit_run_fields_;
	uint32_t empty_count_;
	uint64_t max_empty_offset_;
	uint64_t empty_record_limit_;
	uint64_t empty_record_tail_;
	uint64_t layout_budget_;
	const ClassEntity* conflict_root_;
	EmptySubobject conflict_hint_;
	// Semantics mode.
	ConversionRules conversions_;
	// Completes class template instances for the conversion rules'
	// derivation queries.
	struct InstanceCompleter : ClassCompleter
	{
		Analyzer* analyzer;
		bool complete(const ClassEntity* entity) override { return analyzer->require_complete(entity); }
	};
	InstanceCompleter completer_;
	BodyContext* body_;
	// Where odr-uses are recorded: the definition being analyzed, or the
	// unit's roots; and the mark that keeps each list free of repeats.
	FunctionEntity* demanding_;
	uint32_t demand_epoch_;
	uint32_t root_epoch_;
	uint32_t current_epoch_;
	// The static variable's initializer being analyzed (see
	// SemanticGraph::note_initializer_constructor).
	const syntax::Initializer* initializing_ = nullptr;
	// The braced list of an object of static or thread storage duration's
	// own initializer and the object's class: the one list whose elements
	// may give GNU's flexible array member elements (see aggregate_members,
	// kFlexibleElements); whether its analysis gave them.
	const syntax::BracedInitList* flexible_list_ = nullptr;
	const ClassEntity* flexible_class_ = nullptr;
	bool flexible_given_ = false;
	// Nonzero inside unevaluated operands (5p8): nothing is odr-used.
	uint32_t unevaluated_;
	// Analyzing the body of a hosted inline wrapper of a reserved builtin
	// this compiler lacks (see define_function): such a call is deferred
	// (defer_reserved_call), and its callee is no undeclared name.
	uint32_t deferring_reserved_ = 0;
	// The typeid operands' deferred uses (see DeferredUse), innermost last,
	// and the unevaluated level whose uses are deferred (0: none); the
	// program's std::type_info once a typeid found it.
	std::vector<DeferredUse> deferred_uses_;
	uint32_t deferring_ = 0;
	const ClassEntity* type_info_ = nullptr;
	// `std` and `initializer_list` (see is_initializer_list_template),
	// `bad_alloc` (see check_redeclared_exceptions).
	const Identifier* std_key_ = nullptr;
	const Identifier* initializer_list_key_ = nullptr;
	const Identifier* bad_alloc_key_ = nullptr;
	// The hosted builtin declarations' __type_pack_element (see
	// alias_specialization) and its name.
	const Identifier* type_pack_element_key_ = nullptr;
	const AliasTemplateEntity* type_pack_element_ = nullptr;
	const Identifier* builtin_list_keys_[2] = { nullptr, nullptr };
	// Handlers' variables (12.8p31: no move from them).
	PointerMap catch_parameters_;
	// __is_literal_type's answers (GCC's rule, trait_literal_type) for the
	// complete classes 3.9p10 alone does not make literal.
	PointerMap trait_literal_classes_;
	// GCC's trivial default constructor of a complete class, by its
	// declarations alone and as planned (default_constructor_shape_trivial,
	// gnu_trivial_default_constructor).
	PointerMap trivial_shape_classes_;
	PointerMap trivial_default_classes_;
	// subobjects_member's answers per class: two bits per copy kind and
	// question (trivial, non-throwing).
	PointerMap copy_subobject_answers_;
	// Template arguments being typed (their constant expressions demand no
	// instance static member's definition, see static_constant).
	uint32_t template_arguments_ = 0;
	// A call's callee name is being analyzed (its function is checked and
	// odr-used once the call's resolution selects it).
	bool callee_name_ = false;
	// Nonzero while a substitution whose failure is a result runs (14.8.2p8:
	// a function template's deduced specialization, a partial
	// specialization's argument list): an invalid type or expression in it
	// records the failure (substitution_error) and each analysis returns
	// null up to the substitution, which consumes it (SoftSubstitution).
	uint32_t soft_substitution_ = 0;
	// Placeholders made so far (a clause's later parameters chain a scope
	// after each parameter that made one, see apply_suffix).
	uint32_t placeholders_made_ = 0;
	// The element of a pack expansion's pattern a deduction is matching,
	// which each bound pack (TypeKind::BoundPack) the pattern names stands
	// for (-1 outside one; see deduce_elements).
	int32_t deduced_element_ = -1;
	// The template whose deduction's substitution runs (a function
	// template, or a partial specialization being matched): the context in
	// which it checks the access of the member types it substitutes
	// (14.8.2p8, 11p4); null for none (matching a declaration to the
	// specialization it names, as explicit instantiations' names are not
	// access-checked, 14.7.2p12).
	const Entity* substitution_context_ = nullptr;
	// The lambda function call operator whose body is about to be analyzed
	// when its one return statement deduces its result (5.1.2p4).
	const FunctionEntity* deducing_call_ = nullptr;
	// The lambda-expression whose function call operator's body is about to
	// be analyzed (function_body takes it into its context).
	LambdaFrame* pending_lambda_ = nullptr;
	// The templated lambda whose call operator's definition is being walked
	// for the captures its body names (see templated_closure_call).
	LambdaFrame* capture_scan_ = nullptr;
	// While a lambda's or a function declaration's default arguments are
	// typed where they appear, the scope they are typed in: names used
	// directly in them there name no local variable (8.3.6p7).
	const Scope* default_argument_scope_ = nullptr;
	// The function call a decltype-specifier's operand is (or ends with
	// after commas), whose class type is not instantiated (5.2.2p11).
	const syntax::Expr* decltype_call_ = nullptr;
	// The failure recorded (no message: none; a diagnostic's structure,
	// rendered only if it ends the analysis as an error); `transient` when it observed an incomplete class (one being defined,
	// or not yet), so that a later substitution of the same arguments may
	// succeed: not memoized.
	struct SubstitutionFailure
	{
		const char* message;
		uint32_t location;
		bool transient;
	};
	SubstitutionFailure failure_ = SubstitutionFailure();
	// The alias template-id whose aliased type is being substituted: where
	// an error found in types, with no syntax at hand, is reported (see
	// error).
	uint32_t substitution_location_ = 0;
	// How many times require_complete has found a class incomplete: a
	// substitution that saw it happen is not memoized (see
	// substitute_decltype).
	uint32_t incomplete_classes_seen_ = 0;
	// Calls of functions (and constructors) that may throw analyzed so far
	// (a member initializer's count decides the implicit constructor's
	// exception-specification, 15.4p14).
	uint32_t potentially_throwing_;
	// An implicit or defaulted default constructor's plan found a member it
	// cannot initialize (see implicit_default_facts).
	bool plan_deleted_ = false;
	// A variable's declarator may apply operators to auto (7.1.6.4p6).
	bool auto_declarator_ = false;
	// A function declaration's own function declarator, whose return type
	// may hold auto (see apply_suffix); null elsewhere.
	const syntax::DeclaratorSuffix* placeholder_suffix_ = nullptr;
	// The type that replaced auto in the declaration's earlier declarators
	// (7.1.6.4p7; null before the first).
	const Type* auto_deduced_ = nullptr;
	// The operands __builtin_invoke typed to choose its expression's form,
	// which typed() then returns as they are (see builtin_invoke).
	const syntax::Expr* pretyped_[2] = { nullptr, nullptr };
	const Identifier* this_key_;
	// The identifiers of range-based for statements' equivalents (see
	// analyzer_range_for.cpp), interned on first use.
	static const unsigned kRangeForIdentifiers = 10;
	const Identifier* range_for_identifiers_[kRangeForIdentifiers] = {};
	// A captureless closure's invoker's key (`__invoke`), interned on first use.
	const Identifier* invoker_key_ = nullptr;
	// The names decltype and value placeholders print with.
	const Identifier* decltype_key_;
	const Identifier* value_key_;
	// The virt-specifier identifiers (9.2p9).
	const Identifier* final_key_;
	const Identifier* override_key_;
	// The base classes an overriding search visits (see override_facts),
	// each on the primary path or not; the names and signatures of the
	// unit's virtual functions (the search's filter).
	struct OverrideStep
	{
		const ClassEntity* entity;
		bool primary;
	};
	std::vector<OverrideStep> override_stack_;
	// Overriders of non-throwing functions whose noexcept-specification
	// waits for their complete class (15.4p5, checked at the end of the
	// unit).
	struct PendingOverride
	{
		const FunctionEntity* function;
		uint32_t location;
	};
	std::vector<PendingOverride> pending_overrides_;
	// Member dynamic-exception-specifications whose types wait for the
	// outermost class, and overriders whose allowed sets are compared
	// with them (see analyzer_exception_sets.cpp).
	struct PendingThrow
	{
		FunctionEntity* function;
		const syntax::FunctionQualifier* qualifier;
		Scope* scope;
	};
	std::vector<PendingThrow> pending_throws_;
	struct ExceptionOverride
	{
		const FunctionEntity* function;
		const FunctionEntity* found;
		uint32_t location;
	};
	std::vector<ExceptionOverride> exception_overrides_;
	// Variables, parameters and members whose declarator gives the function
	// type they point or refer to a non-throwing exception-specification
	// (see note_nothrow_target).
	PointerMap nothrow_targets_;
	// base_path_facts' memo of base_paths, cleared per query, and the
	// complete classes simple_hierarchy established (1 simple, 2 not).
	PointerMap base_path_memo_;
	PointerMap simple_hierarchies_;
	// base_path_facts' tables: by (class, base below it) the facts of the
	// base (kBasePathCount, kVirtualBasePath, kNonVirtualBasePath), for
	// complete classes with several bases (or a virtual one)
	// whose hierarchy is not simple, built at a class's first query within
	// a budget of entries (base_tables_: 1 built, 2 refused); by complete
	// class, its chain_bottom (an index of chain_bottom_list_); the scratch
	// of a table's build.
	FlatMap<EntityPair, uint8_t, EntityPairHash> base_table_;
	PointerMap base_tables_;
	PointerMap chain_bottoms_;
	std::vector<const ClassEntity*> chain_bottom_list_;
	std::vector<const ClassEntity*> table_order_;
	std::vector<std::pair<const ClassEntity*, const BaseClass*>> table_stack_;
	std::vector<uint8_t> table_facts_;
	// base_specifier's index for complete classes of many bases (their bases
	// are distinct, 10.1p3, and fixed): by (class, base) its specifier,
	// and the classes indexed.
	FlatMap<EntityPair, const BaseClass*, EntityPairHash> direct_bases_;
	PointerMap direct_base_classes_;
	// access_as_member's accesses of the bases below a class of several
	// (by base, kNoAccess for none), per query; leads_to_base's classes (1
	// leading to base_leads_target_, 0 not), cleared per check_base_access
	// and accessible query and when another base is sought.
	static const uint32_t kNoAccess = 0xff;
	PointerMap access_paths_;
	// One accessible query's: the accesses of its member as a member of
	// the bases below a class of several (as access_paths_), the classes
	// below such a class that bullet 4 found no access through (1), and
	// the use's location.
	PointerMap member_paths_;
	PointerMap access_unreached_;
	uint32_t access_location_ = 0;
	// The names (by identifier id) a using-declaration introduced a class
	// member of, whose access checks see if lookup merged declarations;
	// accessible_declarations' query: the classes searched from the naming
	// class (1 declaring the member, 0 not), the declarations found with
	// their classes, its search stack, and the class whose declaration's
	// paths are being tried (null otherwise).
	std::vector<bool> using_names_;
	PointerMap access_frontier_;
	std::vector<std::pair<const ClassEntity*, const Binding*>> access_declarations_;
	std::vector<const ClassEntity*> access_classes_;
	const ClassEntity* access_target_ = nullptr;
	// access_walk's and leads_to_base's stack: a class whose fact is
	// sought, the class of several bases its chain of single bases ends
	// at, that class's next base, and for access_walk the chain's access
	// and the most access through the bases before.
	struct BaseWalkFrame
	{
		const ClassEntity* top;
		const ClassEntity* bottom;
		const BaseClass* next;
		Access path;
		Access best;
		bool found;
	};
	std::vector<BaseWalkFrame> base_frames_;
	PointerMap base_leads_;
	const ClassEntity* base_leads_target_ = nullptr;
	// The lists of argument_demand_head, by context, and the (context,
	// function) pairs they hold.
	std::vector<ArgumentDemand> argument_demands_;
	PointerMap argument_demand_heads_;
	FlatMap<EntityPair, bool, EntityPairHash> argument_demanded_;
	// `&C::f` of a set of overloaded member functions: its naming class and
	// scope, for the access check of the function its target selects, and
	// for `&C::f<A>` the lookup's functions (the set holds specializations).
	struct MemberAddress
	{
		const ClassEntity* naming;
		const Scope* scope;
		const FunctionList* functions;
	};
	PointerMap member_address_index_;
	std::vector<MemberAddress> member_addresses_;
	PointerMap override_visits_;
	PointerMap virtual_names_;
	PointerMap virtual_signatures_;
	// Scratch of virtual_slots: the primary chain being built and the
	// table in progress.
	std::vector<const ClassEntity*> slot_chain_;
	std::vector<VirtualSlot> slot_work_;
	// itanium_primary's classes (class -> index into itanium_primaries_
	// (+1, 0 for none) << 1 | its virtual bit), itanium_nearly_empty's
	// (class -> 1 or 2), their walks' scratch; a layout's indirect
	// primary bases (marked 1) and its base subobject tree (Clang's
	// BaseSubobjectInfo: each subobject's class, base-specifier and parent
	// node, a virtual base's one node), each virtual primary base's
	// claimant node (kMissing: the class's own primary) and the nodes'
	// offsets once resolved.
	PointerMap itanium_primary_index_;
	std::vector<const ClassEntity*> itanium_primaries_;
	PointerMap itanium_nearly_empty_;
	PointerMap itanium_seen_;
	PointerMap itanium_expanded_;
	std::vector<const ClassEntity*> itanium_stack_;
	std::vector<const BaseClass*> itanium_walk_;
	PointerMap indirect_primary_set_;
	// has_indirect_primaries' answers (class -> 1 none, 2 some) and stack.
	PointerMap has_indirect_;
	std::vector<const ClassEntity*> has_indirect_stack_;
	struct ClaimNode
	{
		const ClassEntity* cls;
		const BaseClass* edge;
		uint32_t parent;
	};
	struct ClaimFrame
	{
		uint32_t node;
		const BaseClass* next;
		const ClassEntity* claim_after;
	};
	// A layout's claims (plan_claims): whether the class has indirect
	// primary bases; its virtual bases' positions by class; each resolver's
	// (a base-specifier) list of the indirect primary bases it places, each
	// the class's entry for it and its offset from the resolver.
	struct ClaimEntry
	{
		BaseClass* vbase;
		uint64_t relative;
		uint32_t next;
	};
	bool layout_claims_ = false;
	PointerMap claim_vbases_;
	PointerMap claim_heads_;
	std::vector<ClaimEntry> claim_entries_;
	std::vector<ClaimNode> claim_nodes_;
	std::vector<ClaimFrame> claim_frames_;
	PointerMap claim_virtual_nodes_;
	PointerMap claimants_;
	// Scratch of share_virtual_primaries: the group's segments' offsets,
	// the virtual bases' segments, the segments by their class's virtual
	// primary base (heads, next) and the complete class's chain's virtual
	// primary bases; slot_levels' arrays (class -> 1 + index, 0 for none)
	// and its chain.
	std::vector<uint64_t> share_offsets_;
	PointerMap share_levels_;
	PointerMap share_heads_;
	std::vector<uint32_t> share_next_;
	PointerMap share_chain_;
	// chain_overrider's answers (by class, function and virtual base) and
	// its walk.
	FlatMap<ChainKey, FunctionEntity*, ChainHash> chain_overriders_;
	std::vector<const ClassEntity*> chain_walk_;
	// has_virtual_base's index: a class's virtual bases, entered once per
	// class on its first query (`virtual_base_indexed_`).
	bool has_virtual_base(const ClassEntity* cls, const ClassEntity* base);
	FlatMap<VirtualBaseKey, bool, VirtualBaseHash> virtual_base_index_;
	PointerMap virtual_base_indexed_;
	PointerMap slot_level_index_;
	std::vector<const ClassEntity*> slot_level_store_;
	std::vector<const ClassEntity*> slot_level_chain_;
	std::vector<const ClassEntity*> grouped_classes_;
	// Scratch of virtual_base_list.
	std::vector<VirtualBase> virtual_base_scratch_;
	PointerMap virtual_base_marks_;
	// Scratch of vtable_group, per call: the segments and entries; a
	// spliced base's chain classes (by scope) and their base-specifiers
	// with their segments (group_below); the virtual bases with segments
	// numbered, the subobjects with virtual bases (each with its segment
	// and the virtual base's it is in), each such base's list of them, and
	// a slot's overriders from outside a virtual base (group_outside); the
	// depths of the primary chain's classes and their base-specifiers
	// (group_chain_depth).
	std::vector<VtableSegment> group_segments_;
	std::vector<VtableEntry> group_entries_;
	PointerMap group_chain_;
	PointerMap group_shared_;
	PointerMap group_depths_;
	struct GroupSubobject
	{
		const ClassEntity* cls;
		uint32_t segment;
		uint32_t within;
	};
	std::vector<GroupSubobject> group_subobjects_;
	std::vector<uint32_t> group_starts_;
	std::vector<uint32_t> group_members_;
	std::vector<VtableEntry> group_candidates_;
	std::vector<const syntax::BinaryExpr*> binary_spine_;
	// The labels and goto statements of the bodies being analyzed,
	// innermost last (see BodyContext).
	std::vector<PendingLabel> labels_;
	std::vector<PendingGoto> gotos_;
	// How many local classes of a name, and closure types of a list of
	// parameter types (an interned Pack), each function declared so far:
	// the next one's Itanium discriminator (5.1.6, 5.1.8).
	FlatMap<LocalNumberKey, uint32_t, LocalNumberHash> local_numbers_;
	uint16_t local_number(const void* key);
	// The closures of each member-specification context (BodyContext::
	// closure_context and its parameter) and lambda-sig so far.
	FlatMap<ClosureContextKey, uint32_t, ClosureContextHash> closure_context_numbers_;
	void number_unnamed_local_type(const Entity* entity);
	// Scope chains of a goto check (see bypasses_initialization).
	std::vector<const Scope*> goto_chain_;
	std::vector<const Scope*> label_chain_;
	// The case values of the switch statements being analyzed, innermost
	// last (see SwitchContext).
	std::vector<uint64_t> case_values_;
	std::vector<Conversion> sequences_;
	// The conversions convert_arguments records, out of sequences_.
	std::vector<Conversion> argument_scratch_;
	// The class a single argument direct-initializes during constructor
	// overload resolution (explicit conversion functions, CWG 899).
	const ClassEntity* direct_class_ = nullptr;
	// Arguments convert by standard conversions only (13.3.3.1p4): 13.3.1.4
	// candidates and the second step of a class copy-initialization.
	bool standard_only_ = false;
	// A template-id call's explicit template arguments, taken by the
	// resolution of its candidates.
	const TypeList* explicit_arguments_ = nullptr;
	// The next resolution's constructor templates are copy-initialization's
	// (13.3.1.4p1: converting constructors only): a specialization its
	// explicit(condition) makes explicit is no candidate. Set by
	// constructor_candidates, taken by that resolution.
	bool converting_only_ = false;
	std::vector<const VariableEntity*> demanded_variables_;
	std::vector<const FunctionEntity*> explicit_instantiations_;
	std::vector<const ClassEntity*> explicit_class_instantiations_;
	// The functions explicit instantiation definitions name, with their
	// locations: each one's definition is in the unit (14.7.2p5).
	struct NamedInstantiation
	{
		const FunctionEntity* function;
		uint32_t location;
	};
	std::vector<NamedInstantiation> named_instantiations_;
	// Odr-used instantiated functions whose definitions the unit's end
	// instantiates (FunctionEntity::definition_demanded), in the order met.
	std::vector<const FunctionEntity*> demanded_definitions_;
	// The template specializations a target type selects from a set of
	// overloaded functions (select_overloaded's scratch).
	std::vector<FunctionEntity*> selected_specializations_;
	// Classes' inheriting constructors made for specializations of the
	// constructor templates they inherit (inherited_specialization): by
	// (class, specialization), a chain of its forms' constructors.
	struct InheritedSpecialization
	{
		FunctionEntity* inheriting;
		uint32_t next;
	};
	std::vector<InheritedSpecialization> inherited_specializations_;
	FlatMap<EntityPair, uint32_t, EntityPairHash> inherited_heads_;
	// Instances' member enumerations defined out of their class template,
	// instantiated once the instance class they belong to is complete.
	std::vector<EnumEntity*> pending_member_enums_;
	// Rare PA16 facts kept out of the entity records, by entity: variables'
	// evaluated values (evaluated_value) and pending noexcept-specifications'
	// expressions with the scope their names are looked up in (the
	// declarator's parameters in scope, 3.3.4p1) and the parameter clause
	// (a function template specialization's are its template's, see
	// function_nothrow).
	struct PendingNoexcept
	{
		const syntax::Expr* operand;
		Scope* scope;
		const syntax::ParameterClause* parameters;
		// Its function, and whether the end of the unit evaluates it
		// (check_pending_exceptions).
		FunctionEntity* function;
		bool unit;
		// Its declaration's context (14.6.4.2 for a template's or an
		// instance member's): the declaration ordinal where it is.
		uint32_t ordinal;
	};
	PointerMap evaluated_index_;
	std::vector<const ConstValue*> evaluated_;
	// The references read_through_storage holds.
	PointerMap stored_references_;
	// String literals' arrays as constant evaluation reads them (2.14.5p8),
	// by literal, made on first read.
	PointerMap literal_index_;
	std::vector<const ConstValue*> literal_values_;
	// library_nan_call's functions (double, float, long double).
	FunctionEntity* library_nans_[3] = {};
	// text_literal's literals, by the text interned as an identifier.
	PointerMap text_literal_index_;
	std::vector<const syntax::LiteralExpr*> text_literals_;
	// The constant interpreters' scratch values and stacks: each evaluation
	// rewinds and pops what it took (evaluations nest), so their storage is
	// reused.
	Arena constant_scratch_;
	ConstantStacks constant_stacks_;
	PointerMap noexcept_index_;
	// Class template instances' members whose brace-or-equal-initializers
	// wait for a constructor (see instantiate_member_initializer): the
	// index of each one's complete-class scope and its class
	// instantiation's lookup limit (its definition context, 14.6.4.2).
	struct PendingMemberInitializer
	{
		Scope* scope;
		uint32_t limit;
	};
	PointerMap pending_member_initializers_;
	std::vector<PendingMemberInitializer> pending_member_scopes_;
	std::vector<PendingNoexcept> noexcepts_;
	// Special member functions defaulted and declared constexpr on their
	// first declaration, checked at the end of the unit (8.4.2p2).
	struct DefaultedConstexpr
	{
		FunctionEntity* function;
		uint32_t location;
	};
	std::vector<DefaultedConstexpr> defaulted_constexprs_;
	// The class definitions being instantiated, innermost last: the
	// pattern's definition and the copy being analyzed, so a member function
	// a constant expression in the class calls learns its pattern before the
	// class completes (see constexpr_definition).
	struct ClassInstantiation
	{
		const ClassEntity* entity;
		const syntax::ClassSpecifier* pattern;
		const syntax::ClassSpecifier* copy;
		// The copy's members analyzed so far, and those linked to their
		// patterns ahead of the class's completion (constexpr_definition).
		uint32_t declared;
		uint32_t linked;
	};
	std::vector<ClassInstantiation> class_instantiations_;
	// Partial ordering's unique types, by index (see synthesized_type).
	std::vector<const Type*> synthesized_;
	std::vector<Candidate> candidates_;
	std::vector<FunctionEntity*> overload_set_;
	// The classes whose implicit copy and move members are being declared.
	std::vector<CopyFrame> copy_frames_;
	// Scratch of the conversion functions collected for a user-defined
	// conversion or a class operand, and of a user-defined conversion's
	// candidates (its function, the operand's conversion and a conversion
	// function's second standard conversion): each use appends from the
	// current size and truncates back to it.
	std::vector<FunctionEntity*> conversion_scratch_;
	// A complete class's visible conversion functions and templates (see
	// visible_conversions), computed once: the declarations that make each
	// visible, at its index in the list. The scratch of their computation:
	// the set being made, the walk's path (each class with its own
	// conversion functions and the next base to walk), the hiding counts
	// of the path's functions and templates by return type, and the walks
	// and functions seen.
	struct ConversionSet
	{
		Binding* const* bindings;
		uint32_t count;
	};
	struct ConversionStep
	{
		const ClassEntity* cls;
		const FunctionList* own;
		const BaseClass* base;
	};
	PointerMap conversion_sets_;
	std::vector<ConversionSet> conversion_set_list_;
	std::vector<Binding*> conversion_set_scratch_;
	std::vector<ConversionStep> conversion_path_;
	PointerMap conversion_hiding_[2];
	PointerMap conversion_seen_;
	struct UserCandidate
	{
		const FunctionEntity* function;
		Conversion first;
		Conversion second;
	};
	std::vector<UserCandidate> user_candidates_;
	// The types class operands of a built-in operator stand for (see
	// class_builtin_candidates), used as the scratch above.
	std::vector<const Type*> operand_options_;
	// Ref-qualified member functions declared so far (13.1p2's check probes
	// for them only when there are some).
	uint32_t ref_qualified_members_ = 0;
	// Operator functions by operator (keys interned once), and the
	// candidate set of the operator being resolved.
	std::vector<const Identifier*> operator_keys_;
	std::vector<FunctionEntity*> operator_set_;
	std::vector<const FunctionEntity*> converter_scratch_;
	uint32_t candidate_epoch_;
	// Argument-dependent lookup (3.4.2): the functions it found for the
	// call being resolved, the associated namespaces, the classes the
	// argument types are members of, the base walk's stack, the associated
	// classes (whose friends it finds), and the mark of the current
	// lookup's associated classes and namespaces.
	std::vector<FunctionEntity*> adl_functions_;
	std::vector<Scope*> adl_namespaces_;
	std::vector<ClassEntity*> adl_enclosing_;
	std::vector<ClassEntity*> adl_pending_;
	std::vector<ClassEntity*> adl_classes_;
	uint32_t association_epoch_;
	// The functions whose later declarations added default arguments
	// (8.3.6p4): by function, the first of its parameters' declaration
	// ordinals in `added_default_ordinals_` (0 for a default of the first
	// declaration), which a lookup limit compares (see
	// default_argument_visible).
	PointerMap added_defaults_;
	std::vector<uint32_t> added_default_ordinals_;
	// The template heads with a redeclaration's default template
	// arguments: by head, the first of its parameters' default ordinals in
	// `default_ordinals_` (see default_ordinal).
	PointerMap default_ordinal_index_;
	std::vector<uint32_t> default_ordinals_;
	// The aggregate initialization being analyzed: the mark of the member
	// classes it value-initialized (see aggregate_element).
	uint32_t empty_list_epoch_;
	// The list a direct aggregate construction made of its arguments, whose
	// own elements may narrow (see class_prvalue); null elsewhere.
	const syntax::BracedInitList* aggregate_arguments_ = nullptr;
	// The unit's distinct conversion records (see intern_conversion).
	std::vector<const Conversion*> conversion_slots_;
	size_t conversion_count_;
	// The class and function template specializations declared so far,
	// by template and (canonical) arguments; owned by the unit's analysis.
	FlatMap<SpecializationKey, Entity*, SpecializationHash> specializations_;
	// The function template specializations whose types are being
	// substituted, innermost last, each with the depth of class template
	// instantiations (class_instantiations_) it began in (see specialize).
	struct ActiveSpecialization
	{
		SpecializationKey key;
		size_t classes;
	};
	std::vector<ActiveSpecialization> specializing_;
	// A function template's default template arguments as deductions
	// completed them (see complete_deduction), by template and the
	// (canonical) arguments of the parameters before the default's.
	FlatMap<SpecializationKey, const Type*, SpecializationHash> deduced_defaults_;
	// Class and variable template partial specializations by (template,
	// position tag, key; see partial_index_key): by their first argument
	// that depends on no parameter (the canonical argument), else by their
	// first that is a class template's specialization (that template's
	// type), the first of a list linked through next_partial (see
	// ClassTemplateFacts::partial_positions); by (template) alone the last
	// of its unindexed ones, once it has two. Class partial specializations
	// by (template, canonical argument list), and partial ordering's result
	// by ordered pair of partial specializations once compared (14.5.5.2).
	FlatMap<SpecializationKey, Entity*, SpecializationHash> partial_index_;
	FlatMap<SpecializationKey, ClassEntity*, SpecializationHash> partial_arguments_;
	FlatMap<EntityPair, bool, EntityPairHash> partial_orderings_;
	// Function-template ordering is structural. Declaration/type identities,
	// compared arity and call/address form distinguish its complete input.
	FlatMap<FunctionOrderKey, bool, FunctionOrderHash> function_orderings_;
	// Template template-parameters' positional entities by (depth, index,
	// pack, the shape of their own parameter lists) (see
	// positional_template_parameter); dependent member template-names by
	// (qualifier, name) (see dependent_template_name); the bindings of alias
	// template specializations by (alias template, canonical arguments),
	// and by (null, the type) the hidden alias binding a template-id
	// naming a type other than a class specialization returns; partial
	// ordering's unique templates, by index.
	FlatMap<DecltypeKey, TemplateParameterEntity*, DecltypeHash> template_template_parameters_;
	FlatMap<PlaceholderKey, TemplateParameterEntity*, PlaceholderHash> template_name_placeholders_;
	FlatMap<SpecializationKey, Binding*, SpecializationHash> alias_specializations_;
	std::vector<const Type*> synthesized_templates_;
	std::vector<const Type*> synthesized_values_;
	// Function templates' explicit(condition) specifiers (see
	// specialized_explicit), by template.
	PointerMap explicit_index_;
	// GNU aligned on a typedef or alias-declaration (hosted, PA29): the
	// alignment it gives, which may be weaker than its type's, keyed by the
	// typedef's binding (Binding::aligned_alias), and in aligned_variables_
	// by each variable or data member declared with it (its type the
	// typedef's), or the pointee's alignment keyed by the variable's
	// address + 1 for one declared a pointer to it. The types themselves are
	// unchanged (__is_same(alias, int)). named_binding_: the type-name
	// binding named_type last resolved; type_id_alignment_: the last
	// type_id's typedef alignment.
	PointerMap alias_alignments_;
	PointerMap aligned_variables_;
	// Complete classes' representation payloads, computed on first trait
	// demand. UINT64_MAX denotes non-unique values. Layout/completion is
	// required before insertion, so these facts never need invalidation.
	PointerMap representation_index_;
	std::vector<uint64_t> representation_bits_;
	// Default type arguments analyzed where declared, in their own parameter
	// scopes. Uses substitute these facts instead of rebuilding a scope of
	// every preceding argument (see default_template_argument_names).
	FlatMap<DefaultKey, const Type*, DefaultKeyHash> unbound_defaults_;
	// The default type-ids that bear no facts (no expression), which a
	// deduction analyzes without a copy (see template_default).
	PointerMap fact_free_defaults_;
	const Binding* named_binding_ = nullptr;
	uint32_t type_id_alignment_ = 0;
	// The alias stand-ins' facts (see ClassTemplateFacts::alias_stand_in):
	// a stand-in template's AliasTemplateEntity, a stand-in
	// specialization's aliased Type, by class.
	PointerMap alias_index_;
	std::vector<const void*> alias_facts_;
	std::vector<PendingExplicit> explicits_;
	// Qualified implicit-object calls' naming classes (see
	// call_naming_class), by call.
	PointerMap naming_index_;
	std::vector<const ClassEntity*> namings_;
};

} // namespace sema
