#pragma once

#include <cstddef>
#include <iosfwd>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"
#include "semantic_source_use.h"
#include "template_api.h"
#include "template_model.h"
#include "witness_api.h"

class SemanticContext;

namespace callsemantic {

witness::SourceSelectionKind source_selection_kind_for_match_kind(
    template_api::MatchKind kind);
witness::TemplateWitnessSourceAnchor class_use_selected_decl_anchor(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl * class_template,
    const template_api::ClassSpecializationSelection & selection);
bool scope_is_inside_source_template_context(semantic_model::Scope & scope);
std::string canonicalize_template_parameter_redeclaration_text(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::string & text);
bool template_parameter_redeclarations_compatible(
    const std::vector<template_model::TemplateParameterInfo> & existing_parameters,
    const std::vector<template_model::TemplateParameterInfo> & incoming_parameters,
    std::size_t index);
bool merge_template_parameter_redeclarations(
    std::vector<template_model::TemplateParameterInfo> & target,
    const std::vector<template_model::TemplateParameterInfo> & incoming);
void prefer_incoming_template_parameter_spellings(
    std::vector<template_model::TemplateParameterInfo> & target,
    const std::vector<template_model::TemplateParameterInfo> & incoming);
void record_definition_parameter_aliases(
    semantic_model::FunctionBinding & binding,
    const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params);
void record_definition_parameter_aliases(
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params);
void invalidate_out_of_class_definition_caches(
    semantic_model::ClassTemplateDecl & decl);
semantic_model::PartialClassTemplateSpecializationDecl *
find_partial_specialization_decl(semantic_model::ClassTemplateDecl & decl,
                                 const semantic_model::ClassInfo * owner);
const CppAstNode * find_function_body_node(const CppAstNode & node);
const CppAstNode * find_descendant_kind(const CppAstNode & node,
                                        CppAstKind kind);
const cpp_decl::TemplateIdSyntax * first_template_id_syntax_in_subtree(
    const CppAstNode & node);
bool qualified_template_id_syntax_in_subtree(const CppAstNode & node);
std::string qualified_name_syntax_text(const cpp_decl::QualifiedName & name);
std::string repair_compacted_template_argument_expression_spacing(
    const std::string & text);
std::string template_argument_syntax_text_preserving_spacing(
    const cpp_decl::TemplateArgumentSyntax & argument);
std::string template_argument_syntax_witness_source_text(
    const cpp_decl::TemplateArgumentSyntax & argument);
std::string template_id_syntax_text_preserving_spacing(
    const cpp_decl::TemplateIdSyntax & syntax);
std::vector<std::string> template_id_argument_texts_preserving_spacing(
    const cpp_decl::TemplateIdSyntax & syntax);
std::vector<std::string> template_id_argument_witness_source_texts(
    const cpp_decl::TemplateIdSyntax & syntax);
std::vector<const CppAstNode *> parameter_declarations_from_clause(
    const CppAstNode & parameter_clause);
std::vector<cpp_decl::TemplateArgumentSyntax> normalized_template_argument_syntaxes(
    const cpp_decl::TemplateIdSyntax & source_syntax,
    const std::vector<template_model::TemplateParameterInfo> & primary_parameters,
    const std::vector<std::string> & normalized_arg_texts);
bool should_prefer_named_key_for_instantiation_identity(
    const cpp_decl::TypePtr & type);

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const std::string & location);
  ~ScopedTemplateUseLocation();
  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;

private:
  bool active_;
};

struct ExactTemplateTypeLookupAnchor
{
  std::string location;
  std::string template_text;
  std::string identifier;
  std::string compact_key;
  std::vector<std::string> arg_texts;
  std::vector<cpp_decl::TemplateArgumentSyntax> arg_syntaxes;
  bool has_argument_list = false;
};

struct ScopedExactTemplateTypeLookupAnchor
{
  explicit ScopedExactTemplateTypeLookupAnchor(
      const ExactTemplateTypeLookupAnchor & anchor);
  ~ScopedExactTemplateTypeLookupAnchor();
  ScopedExactTemplateTypeLookupAnchor(
      const ScopedExactTemplateTypeLookupAnchor &) = delete;
  ScopedExactTemplateTypeLookupAnchor & operator=(
      const ScopedExactTemplateTypeLookupAnchor &) = delete;

private:
  bool active_ = false;
};

struct ScopedSuppressedTemplateUseLocation
{
  explicit ScopedSuppressedTemplateUseLocation(bool active = true);
  ~ScopedSuppressedTemplateUseLocation();
  ScopedSuppressedTemplateUseLocation(
      const ScopedSuppressedTemplateUseLocation &) = delete;
  ScopedSuppressedTemplateUseLocation & operator=(
      const ScopedSuppressedTemplateUseLocation &) = delete;

private:
  bool active_;
};

std::string compact_lookup_text(const std::string & text);
std::string template_lookup_fragment_text(const std::string & lookup_name);
std::string template_lookup_fragment_identifier(
    const std::string & template_fragment);
bool node_has_template_id_qualifier_syntax(const CppAstNode & node);
// Build a type-lookup anchor carrying the structured argument syntaxes of the
// owner template-id (the final qualifier) of a qualified-id node. Returns an
// empty (has_argument_list == false) anchor when the node has no template-id
// owner qualifier with a full structured argument list. Shared by the
// declaration collector and the expression analyzers so out-of-class owner
// resolution recovers structured arguments instead of re-parsing text.
ExactTemplateTypeLookupAnchor exact_template_type_lookup_anchor_for_owner_node(
    const CppAstNode & id_node);
// Build a type-lookup anchor directly from a template-id syntax (e.g. an owner
// qualifier carried on a node's qualifier_template_id_syntaxes). The returned
// anchor is empty (has_argument_list == false) when the syntax has no full
// structured argument list.
ExactTemplateTypeLookupAnchor exact_template_type_lookup_anchor_for_template_id(
    const cpp_decl::TemplateIdSyntax & syntax);
const ExactTemplateTypeLookupAnchor * current_exact_template_type_lookup_anchor();
extern thread_local std::set<std::pair<std::string, std::string> >
    source_dependent_class_template_use_drops_;
bool exact_template_type_lookup_anchor_matches(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & normalized_name);
bool exact_template_type_lookup_anchor_matches_syntax(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & normalized_name);
bool exact_template_type_lookup_anchor_matches_identifier(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & identifier);
bool exact_template_type_lookup_anchor_arg_texts(
    const std::string & normalized_name,
    const std::string & identifier,
    std::vector<std::string> & arg_texts);
bool exact_template_type_lookup_anchor_arg_texts_are_full_match(
    const std::string & normalized_name);
const std::vector<cpp_decl::TemplateArgumentSyntax> *
exact_template_type_lookup_anchor_arg_syntaxes(
    const std::string & normalized_name,
    const std::string & identifier);
bool source_template_id_args_are_arity_compatible(
    const std::vector<std::string> & source_arg_texts,
    const std::vector<template_model::TemplateArgument> & arguments);
std::string function_template_decl_primary_location(
    SemanticContext & ctx,
    const semantic_model::FunctionTemplateDecl * decl);
std::string function_template_decl_location_details(
    SemanticContext & ctx,
    const semantic_model::FunctionTemplateDecl * decl);
bool function_template_trace_has_identity(
    const semantic_model::FunctionTemplateDecl * incoming_decl,
    const semantic_model::FunctionBinding * binding);
bool function_binding_template_trace_has_source(
    const semantic_model::FunctionBinding * binding);
void append_function_binding_template_trace_fields(
    std::ostream & out,
    const std::string & prefix,
    const semantic_model::FunctionBinding * binding);
void append_function_template_decl_trace_fields(
    std::ostream & out,
    SemanticContext & ctx,
    const std::string & prefix,
    const semantic_model::FunctionTemplateDecl * decl,
    const std::string & key);
std::string function_binding_source_template_primary_location(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding);
std::string function_binding_source_template_location_details(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding);
const cpp_decl::TemplateIdSyntax * template_id_syntax_for_anchor(
    const CppAstNode & node,
    const std::string & identifier);
const cpp_decl::TemplateIdSyntax * template_id_syntax_for_anchor_at_or_after_location(
    const template_api::TemplateWitnessContext & ctx,
    const CppAstNode & node,
    const std::string & identifier,
    const std::string & location);
const cpp_decl::TemplateIdSyntax * template_id_syntax_for_anchor_at_or_after_location(
    const template_api::TemplateWitnessContext & ctx,
    const std::vector<cpp_decl::TemplateArgumentSyntax> & arguments,
    const std::string & identifier,
    const std::string & location);
bool template_id_value_for_anchor(const CppAstNode & node,
                                  const std::string & identifier,
                                  cpp_decl::QualifiedName & out_name,
                                  std::vector<std::string> & out_arg_texts);
bool template_argument_texts_mention_source_bindings(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<std::string> & arg_texts);
bool template_argument_texts_mention_instantiated_class_local_type_aliases(
    semantic_model::Scope & scope,
    const std::vector<std::string> & arg_texts);
bool template_argument_texts_mention_enclosing_source_template_parameters(
    semantic_model::Scope & scope,
    const std::vector<std::string> & arg_texts);
bool template_argument_texts_mention_template_bound_scope_names(
    semantic_model::Scope & scope,
    const std::vector<std::string> & arg_texts);
bool template_argument_texts_mention_current_specialization_names(
    semantic_model::Scope & scope,
    const std::vector<std::string> & arg_texts);
bool template_argument_syntax_contains_template_id(
    const cpp_decl::TemplateArgumentSyntax & syntax);
bool current_specialization_source_argument_semantic_text(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    std::string & out);
void rewrite_current_specialization_alias_binding_texts(
    SemanticContext & ctx,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> & explicit_argument_texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * explicit_argument_syntaxes,
    std::vector<template_api::TemplateWitnessSourceBinding> & bindings,
    semantic_source_use::SourceTemplateIdOccurrence * occurrence = nullptr);
std::string template_public_use_location_or(const std::string & fallback);
semantic_model::ClassTemplateDecl * class_template_origin_decl(
    semantic_model::ClassInfo * info);
const semantic_model::ClassTemplateDecl * class_template_origin_decl(
    const semantic_model::ClassInfo * info);
bool class_has_template_origin(const semantic_model::ClassInfo * info);
const std::vector<template_model::TemplateArgument> * class_template_origin_arguments(
    const semantic_model::ClassInfo * info);
bool class_has_explicit_template_origin(
    const semantic_model::ClassInfo * info);
const CppAstNode * class_template_origin_class_node(
    const semantic_model::ClassInfo * info);
const CppAstNode * find_member_class_template_declaration_node(
    const semantic_model::ClassTemplateDecl * class_template);
bool env_flag_enabled(const char * name);

}  // namespace callsemantic
