#ifndef CPPGM_CPPAST_PARSER_H
#define CPPGM_CPPAST_PARSER_H

#include <cstddef>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cppast_ast.h"
#include "recog_token_cursor.h"
#include "template_angle_lookup.h"

struct CppAstParser : RecogTokenCursor
{
  typedef template_angle_lookup::NameSet NameSet;

  enum TemplateArgumentFragmentMode
  {
    TAF_PARSE_BOTH,
    TAF_PARSE_TYPE_ONLY,
    TAF_PARSE_EXPRESSION_ONLY,
    TAF_PARSE_TYPE_THEN_EXPRESSION
  };

  explicit CppAstParser(const std::vector<RecogToken> & tokens);
  explicit CppAstParser(IRecogTokenSequence & tokens);

  bool parse_translation_unit(CppAstNode & out);
  bool parse_compound_statement_fragment(CppAstNode & out);
  bool parse_class_specifier_fragment(CppAstNode & out);
  const std::string & error() const { return error_msg; }
  void seed_known_type_names(const NameSet & names);
  void seed_known_template_names(const NameSet & names);
  void seed_known_value_names(const NameSet & names);
  void seed_known_template_value_names(const NameSet & names);
  void set_external_name_lookup(const template_angle::NameLookup * lookup);
  std::shared_ptr<const CppAstNameLookupSnapshot> snapshot_name_lookup_state(
      const NameSet * used_names = nullptr) const;
  void restore_name_lookup_state_from(const CppAstNameLookupSnapshot & snapshot);
  bool parse_template_argument_fragment_node(std::size_t start,
                                             std::size_t end,
                                             CppAstNode & out,
                                             bool & is_type_id,
                                             bool * pack_expansion = nullptr);
  bool parse_template_argument_fragment_syntax(
      std::size_t start,
      std::size_t end,
      cpp_decl::TemplateArgumentSyntax & out,
      TemplateArgumentFragmentMode mode = TAF_PARSE_BOTH,
      bool suppress_nested_template_argument_syntax = false);
  bool is_template_type_parameter_name(const RecogToken & token) const;

protected:
  bool parse_conversion_operator_type_id(std::size_t name_start,
                                         std::size_t name_end,
                                         CppAstNode & out);
  bool parse_declaration(CppAstNode & out);
  bool parse_empty_declaration(CppAstNode & out);
  bool parse_namespace_declaration(CppAstNode & out);
  bool parse_explicit_instantiation(CppAstNode & out);
  bool parse_explicit_instantiation_target(CppAstNode & out);
  bool parse_linkage_specification(CppAstNode & out);
  bool parse_using_or_alias_declaration(CppAstNode & out);
  bool parse_template_declaration(CppAstNode & out);
  bool parse_class_specifier(CppAstNode & out);
  bool parse_class_declaration(CppAstNode & out);
  bool parse_enum_specifier(CppAstNode & out);
  bool parse_enum_declaration(CppAstNode & out);
  bool parse_static_assert_declaration(CppAstNode & out);
  bool parse_qualified_special_member_declaration(CppAstNode & out);
  bool parse_qualified_special_member_definition(CppAstNode & out);
  bool parse_template_special_member_declaration(CppAstNode & out);
  bool parse_special_member_declaration(CppAstNode & out);
  bool parse_deduction_guide_declaration(CppAstNode & out);
  bool parse_bit_field_declaration(CppAstNode & out);
  bool parse_decl_specifier_leading_declaration(CppAstNode & out);
  bool parse_function_definition(CppAstNode & out);
  bool parse_simple_declaration(CppAstNode & out);
  bool parse_simple_declaration_after_specifiers(CppAstNode & out,
                                                 CppAstNode & specifiers,
                                                 std::size_t start,
                                                 CppAstNode * first_declarator = nullptr,
                                                 std::size_t first_declarator_start = 0);
  bool parse_structured_binding_declaration_after_specifiers(
      CppAstNode & out,
      CppAstNode & specifiers,
      std::size_t start);
  bool parse_init_declarator_after_declarator(CppAstNode & out,
                                              CppAstNode & declarator,
                                              std::size_t start);
  bool parse_decl_specifier_seq(CppAstNode & out);
  bool parse_type_id(CppAstNode & out);
  bool parse_new_type_id(CppAstNode & out);
  bool parse_type_specifier_seq(CppAstNode & out);
  bool parse_init_declarator_list(CppAstNode & out);
  bool parse_init_declarator(CppAstNode & out);
  bool parse_initializer(CppAstNode & out);
  bool parse_declarator(CppAstNode & out, bool require_parameters = false);
  bool parse_abstract_declarator(CppAstNode & out, bool require_parameters = false);
  bool parse_new_abstract_declarator(CppAstNode & out);
  bool parse_function_suffixes(CppAstNode & out);
  bool parse_template_parameter_clause(CppAstNode & out);
  bool parse_template_parameter_list(CppAstNode & out);
  bool parse_template_parameter(CppAstNode & out);
  bool parse_type_parameter(CppAstNode & out);
  bool parse_non_type_template_parameter(CppAstNode & out);
  bool parse_non_type_template_default_argument(CppAstNode & out);
  bool parse_default_template_argument(CppAstNode & out);
  bool parse_base_clause(CppAstNode & out);
  bool parse_base_specifier(CppAstNode & out);
  bool parse_parameter_clause(CppAstNode & out);
  bool parse_parameter_declaration(CppAstNode & out);
  bool parse_ctor_initializer(CppAstNode & out);
  bool parse_mem_initializer(CppAstNode & out);
  bool parse_mem_initializer_id(CppAstNode & out);
  bool parse_paren_argument_list(CppAstNode & out);
  bool parse_condition(CppAstNode & out, ETokenType terminator);
  bool parse_for_init_statement(CppAstNode & out);
  bool parse_range_declaration(CppAstNode & out);
  bool parse_range_initializer(CppAstNode & out);
  bool parse_structured_binding_declarator(CppAstNode & out);
  bool parse_structured_binding_identifier_list(CppAstNode & out);
  bool parse_compound_statement(CppAstNode & out);
  bool parse_function_body(CppAstNode & out);
  bool parse_lazy_header_compound_function_body(CppAstNode & out);
  bool parse_function_try_body(CppAstNode & out, CppAstNode * ctor_initializer = nullptr);
  bool parse_block_item(CppAstNode & out);
  bool parse_class_member(CppAstNode & out);
  bool parse_statement(CppAstNode & out);
  bool parse_labeled_statement(CppAstNode & out);
  bool parse_if_statement(CppAstNode & out);
  bool parse_switch_statement(CppAstNode & out);
  bool parse_while_statement(CppAstNode & out);
  bool parse_do_statement(CppAstNode & out);
  bool parse_for_statement(CppAstNode & out);
  bool parse_jump_statement(CppAstNode & out);
  bool parse_try_statement(CppAstNode & out);
  bool parse_asm_statement(CppAstNode & out);
  bool parse_exception_declaration(CppAstNode & out);
  bool parse_return_statement(CppAstNode & out);
  bool parse_expression_statement(CppAstNode & out);
  bool parse_assignment_expression(CppAstNode & out);
  bool parse_conditional_expression(CppAstNode & out);
  bool parse_logical_or_expression(CppAstNode & out);
  bool parse_logical_and_expression(CppAstNode & out);
  bool parse_inclusive_or_expression(CppAstNode & out);
  bool parse_exclusive_or_expression(CppAstNode & out);
  bool parse_and_expression(CppAstNode & out);
  bool parse_equality_expression(CppAstNode & out);
  bool parse_relational_expression(CppAstNode & out);
  bool parse_shift_expression(CppAstNode & out);
  bool parse_pm_expression(CppAstNode & out);
  bool parse_expression(CppAstNode & out);
  bool parse_additive_expression(CppAstNode & out);
  bool parse_multiplicative_expression(CppAstNode & out);
  bool parse_unary_expression(CppAstNode & out);
  bool parse_postfix_expression(CppAstNode & out);
  bool parse_postfix_suffixes(CppAstNode & out, std::size_t start);
  bool parse_builtin_va_arg_argument_list(CppAstNode & out,
                                          std::size_t start);
  bool parse_fold_expression(CppAstNode & out);
  bool parse_primary_expression(CppAstNode & out);
  bool parse_lambda_expression(CppAstNode & out);
  bool parse_lambda_declarator(CppAstNode & out);
  bool parse_keyword_cast_expression(CppAstNode & out);
  bool parse_new_expression(CppAstNode & out);
  bool parse_delete_expression(CppAstNode & out);
  bool parse_type_trait_expression(CppAstNode & out);
  bool parse_braced_init_list(CppAstNode & out);
  bool parse_designated_initializer(CppAstNode & out);
  bool parse_designator(CppAstNode & out);
  bool parse_id_expression(CppAstNode & out);
  bool parse_unqualified_name_text(std::string & out,
                                   bool allow_template_id = true,
                                   bool allow_destructor = true,
                                   bool allow_operator = true);
  bool can_start_decl_specifier_seq() const;
  bool can_start_type_id() const;
  bool qualified_name_span_prefers_expression(std::size_t begin,
                                              std::size_t end) const;
  bool qualified_name_span_names_known_type(std::size_t begin,
                                            std::size_t end) const;
  bool qualified_template_id_span_has_head_expression_lookup(
      std::size_t begin,
      std::size_t end) const;
  bool parenthesized_type_id_prefers_expression(
      const CppAstNode & type_id) const;
  bool can_start_named_decl_specifier_seq() const;
  bool scan_named_decl_specifier_seq_end(std::size_t & end) const;
  bool can_start_id_expression() const;
  bool can_start_primary_expression() const;
  bool can_start_block_declaration();
  bool can_start_range_declaration();
  bool can_start_special_member_candidate() const;
  bool find_template_parameter_clause_end(std::size_t template_pos,
                                          std::size_t & end) const;
  bool can_start_unqualified_implicit_type_function_candidate_at(
      std::size_t cursor) const;
  bool can_start_unqualified_implicit_type_function_candidate() const;
  bool can_start_template_special_member_candidate() const;
  bool can_start_qualified_implicit_type_function_candidate() const;
  bool can_start_deduction_guide_declaration() const;
  bool is_current_class_name_identifier(const RecogToken & token) const;
  bool can_start_statement_as_label() const;
  bool can_start_attributed_decl_specifier_seq();
  bool can_start_structured_binding_declarator() const;
  bool declarator_has_parameter_clause(const CppAstNode & node) const;
  bool type_id_has_function_style_abstract_declarator(
      const CppAstNode & type_id) const;
  bool parse_condition_declaration_candidate(CppAstNode & out);
  bool parse_parenthesized_type_id_or_expression(CppAstNode & out,
                                                 bool & is_type_id,
                                                 bool allow_expression);
  bool parse_decltype_or_typeof_operand_node(std::size_t specifier_start,
                                             std::size_t specifier_end,
                                             bool is_typeof,
                                             CppAstNode & out);
  bool parse_type_name_component_text(std::string & out);
  bool parse_decltype_specifier_text(std::string & out);
  bool parse_name_component_text(std::string & out);
  bool parse_ptr_operator_node(CppAstNode & out);
  bool parse_template_argument_text(std::string & out);
  bool parse_template_id_suffix_text(std::string & out);
  bool parse_function_style_simple_type_text(std::string & out);
  bool parse_type_name_text(std::string & out);
  bool parse_qualified_name_text(std::string & out,
                                 cpp_decl::QualifiedName * syntax = nullptr,
                                 cpp_decl::TemplateIdSyntax * template_id_syntax = nullptr,
                                 std::vector<cpp_decl::TemplateIdSyntax> *
                                     qualifier_template_id_syntaxes = nullptr,
                                 std::vector<CppAstNode> *
                                     qualifier_type_syntaxes = nullptr,
                                 bool prefer_unknown_template_ids = false,
                                 bool suppress_unforced_template_id_crossing_logical_operator =
                                     false,
                                 bool allow_conversion_operator_type_without_call = false);
  void attach_qualified_name_syntax_from_span(CppAstNode & node,
                                              std::size_t start,
                                              std::size_t end);
  bool parse_angle_clause_text(std::string & out);
  bool can_open_nested_template_angle_at(std::size_t boundary) const;
  bool looks_like_unknown_nested_template_id_at(std::size_t boundary) const;
  std::string token_span_text_spaced(std::size_t start, std::size_t end) const;
  bool token_text_needs_separator(const RecogToken & lhs,
                                  const RecogToken & rhs) const;
  bool skip_gnu_attribute_specifier_seq(CppAstNode * annotated = nullptr);
  bool skip_standard_attribute_specifier(CppAstNode * annotated = nullptr);
  bool skip_attribute_specifier_seq(CppAstNode * annotated = nullptr);
  void note_attribute_specifier(CppAstNode * annotated, std::size_t start, std::size_t end);
  bool skip_trailing_declarator_extensions(CppAstNode * annotated = nullptr);
  bool is_known_template_name_identifier(const RecogToken & token) const;
  bool is_known_type_name_identifier(const RecogToken & token) const;
  bool is_known_value_template_parameter_identifier(const RecogToken & token) const;
  bool is_known_value_name_identifier(const RecogToken & token) const;
  template_angle_lookup::ScopedNameLookup make_template_angle_lookup(
      bool prefer_unknown_template_ids = false) const;
  struct SeededClassNameScopes
  {
    bool template_names = false;
    bool type_names = false;
    bool value_names = false;
  };
  struct ClassMemberNameScopes
  {
    NameSet template_names;
    NameSet type_names;
    NameSet value_names;
  };
  std::string normalized_name_without_template_args(const std::string & text) const;
  std::string class_scope_definition_key(const std::string & name) const;
  std::string owner_class_scope_key(const std::string & qualified_name) const;
  std::string first_identifier_text(const CppAstNode & node) const;
  bool resolve_declarator_owner_class_scope_key(const CppAstNode & declarator,
                                                std::string & out) const;
  SeededClassNameScopes push_class_member_name_hints(const std::string & class_key);
  void pop_class_member_name_hints(const SeededClassNameScopes & seeded);
  bool decl_specifier_seq_has_typedef(const CppAstNode & node) const;
  void collect_signature_type_hint_names(const CppAstNode & node,
                                         NameSet & out) const;
  void collect_declarator_identifiers(const CppAstNode & node,
                                      NameSet & out) const;
  bool collect_outer_parameter_value_names(const CppAstNode & node,
                                           NameSet & out) const;
  void collect_declared_type_names(const CppAstNode & node,
                                   NameSet & out) const;
  void collect_declared_template_names(const CppAstNode & node,
                                       NameSet & out) const;
  void collect_declared_value_names(const CppAstNode & node,
                                    NameSet & out) const;
  void note_declared_type_names(const CppAstNode & node);
  void note_declared_template_names(const CppAstNode & node);
  void note_declared_value_names(const CppAstNode & node);
  void note_namespace_alias_definition(const CppAstNode & node);
  void note_using_imports(const CppAstNode & node);
  void note_visible_names_after_declaration(const CppAstNode & node);
  void note_name_lookup_mutation();
  struct NamedDeclSpecifierSeqCacheKey
  {
    std::size_t pos = 0;
    std::size_t template_type_parameter_depth = 0;
    std::size_t template_value_parameter_depth = 0;
    std::size_t template_name_depth = 0;
    std::size_t type_name_depth = 0;
    std::size_t value_name_depth = 0;

    bool operator==(const NamedDeclSpecifierSeqCacheKey & rhs) const
    {
      return pos == rhs.pos &&
             template_type_parameter_depth == rhs.template_type_parameter_depth &&
             template_value_parameter_depth == rhs.template_value_parameter_depth &&
             template_name_depth == rhs.template_name_depth &&
             type_name_depth == rhs.type_name_depth &&
             value_name_depth == rhs.value_name_depth;
    }
  };
  struct NamedDeclSpecifierSeqCacheKeyHash
  {
    std::size_t operator()(const NamedDeclSpecifierSeqCacheKey & key) const;
  };
  NamedDeclSpecifierSeqCacheKey make_named_decl_specifier_seq_cache_key() const;
  struct DeclarationStartProbeCacheKey
  {
    std::size_t pos = 0;
    std::size_t template_type_parameter_depth = 0;
    std::size_t template_value_parameter_depth = 0;
    std::size_t template_name_depth = 0;
    std::size_t type_name_depth = 0;
    std::size_t value_name_depth = 0;
    std::size_t class_name_depth = 0;

    bool operator==(const DeclarationStartProbeCacheKey & rhs) const
    {
      return pos == rhs.pos &&
             template_type_parameter_depth == rhs.template_type_parameter_depth &&
             template_value_parameter_depth == rhs.template_value_parameter_depth &&
             template_name_depth == rhs.template_name_depth &&
             type_name_depth == rhs.type_name_depth &&
             value_name_depth == rhs.value_name_depth &&
             class_name_depth == rhs.class_name_depth;
    }
  };
  struct DeclarationStartProbeCacheKeyHash
  {
    std::size_t operator()(const DeclarationStartProbeCacheKey & key) const;
  };
  DeclarationStartProbeCacheKey make_declaration_start_probe_cache_key(
      std::size_t probe_pos) const;
  void collect_template_parameter_names(const CppAstNode & node,
                                        NameSet & out) const;
  void collect_template_parameter_value_names(const CppAstNode & node,
                                              NameSet & out) const;
  void collect_template_parameter_template_names(const CppAstNode & node,
                                                 NameSet & out) const;
  std::string current_namespace_path_key(const std::string & next = std::string()) const;
  std::string normalized_lookup_name(const std::string & text) const;
  bool namespace_scope_exists(const std::string & key) const;
  std::string resolve_namespace_alias_chain(const std::string & key) const;
  std::string resolve_visible_namespace_scope_key(const std::string & text) const;
  void push_namespace_name_scopes(const std::string & name, bool is_inline = false);
  void pop_namespace_name_scopes(bool commit);
  void set_span(CppAstNode & node, std::size_t start) const;
  void set_error(const std::string & error);
  void inherit_name_lookup_state_from(const CppAstParser & parent);

  std::string error_msg;
  std::size_t template_declaration_depth = 0;
  std::vector<NameSet> template_type_parameter_scopes;
  std::vector<NameSet> template_value_parameter_scopes;
  std::vector<NameSet> template_name_scopes;
  std::vector<NameSet> type_name_scopes;
  std::vector<NameSet> value_name_scopes;
  std::vector<NameSet> materialized_inherited_template_type_parameter_scopes;
  std::vector<NameSet> materialized_inherited_template_value_parameter_scopes;
  std::vector<NameSet> materialized_inherited_template_name_scopes;
  std::vector<NameSet> materialized_inherited_type_name_scopes;
  std::vector<NameSet> materialized_inherited_value_name_scopes;
  const std::vector<NameSet> * inherited_template_type_parameter_scopes = nullptr;
  const std::vector<NameSet> * inherited_template_value_parameter_scopes = nullptr;
  const std::vector<NameSet> * inherited_template_name_scopes = nullptr;
  const std::vector<NameSet> * inherited_type_name_scopes = nullptr;
  const std::vector<NameSet> * inherited_value_name_scopes = nullptr;
  const CppAstParser * borrowed_template_parameter_lookup = nullptr;
  const template_angle::NameLookup * external_name_lookup = nullptr;
  bool suppress_template_argument_fragment_syntax = false;
  std::vector<std::string> class_name_stack;
  std::vector<std::string> namespace_path_stack;
  std::vector<bool> namespace_inline_stack;
  std::unordered_map<std::string, ClassMemberNameScopes> class_member_name_scopes;
  std::unordered_map<std::string, NameSet> namespace_template_name_scopes;
  std::unordered_map<std::string, NameSet> namespace_type_name_scopes;
  std::unordered_map<std::string, NameSet> namespace_value_name_scopes;
  std::unordered_map<std::string, std::string> namespace_alias_targets;
  mutable std::unordered_map<NamedDeclSpecifierSeqCacheKey,
                             bool,
                             NamedDeclSpecifierSeqCacheKeyHash>
      named_decl_specifier_seq_cache;
  mutable std::unordered_map<DeclarationStartProbeCacheKey,
                             bool,
                             DeclarationStartProbeCacheKeyHash>
      declaration_start_probe_cache;
};

#endif
