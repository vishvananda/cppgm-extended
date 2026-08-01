#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "callsem_output.h"
#include "cpp_decl_bridge.h"
#include "cppast_ast.h"
#include "recog_token_buffer.h"
#include "semantic_context.h"
#include "semantic_conversion.h"
#include "semantic_model.h"
#include "text_intern.h"

namespace callsemantic_internal {

using cpp_decl::TypePtr;
using semantic_model::MemberAccess;
using semantic_model::Scope;
using semantic_conversion::ValueCategory;
using DumpNode = CallSemNode;

typedef text_intern::Atom InternedTextAtom;

inline InternedTextAtom intern_text_atom(const std::string & text)
{
  return text_intern::intern(text);
}

inline InternedTextAtom intern_text_atom(const char * data, std::size_t length)
{
  return text_intern::intern(data, length);
}

inline InternedTextAtom find_text_atom(const std::string & text)
{
  return text_intern::find(text);
}

inline std::size_t interned_text_atom_count()
{
  return text_intern::atom_count();
}

inline std::size_t interned_text_atom_storage_bytes()
{
  return text_intern::storage_bytes();
}

struct TypeSpellingParts
{
  std::string before;
  std::string after;
};

std::string append_parenthesized_type_spelling_prefix(const std::string & before,
                                                      const std::string & declarator);
std::string collapse_reparseable_scope_operators(const std::string & text);
TypeSpellingParts spell_reparseable_type_argument(const TypePtr & type);
std::string reparseable_type_argument_text(const TypePtr & type);
void snapshot_function_template_debug_info(SemanticContext & ctx,
                                           semantic_model::FunctionTemplateDecl & decl);
std::string normalize_type_lookup_name(const std::string & text);
std::string normalize_qualified_name_spacing(const std::string & text);
bool has_top_level_declarator_syntax(const std::string & text);
bool has_invalid_top_level_qualified_owner_syntax(const std::string & text);
bool strip_top_level_cv_text(std::string text,
                             std::string & core,
                             bool & cv_const,
                             bool & cv_volatile);
TypePtr match_wrapped_type_text(const std::string & text,
                                const std::string & base_text,
                                const TypePtr & base_type);
bool parse_elaborated_class_lookup_name(const std::string & text,
                                        std::string & class_kind,
                                        std::string & declared_name);
bool declarator_has_parameter_pack(const CppAstNode & declarator);
bool declarator_has_trailing_function_parameter_pack(const CppAstNode & declarator);
bool is_pure_virtual_initializer(const CppAstNode & initializer);
bool subtree_contains_pure_virtual_initializer(const CppAstNode & node);
bool declaration_node_is_pure_virtual(const CppAstNode * declaration_node);
bool contains_identifier_token(const std::string & text, const std::string & name);
bool identifier_is_qualified_component(const std::string & text, std::size_t pos);
bool is_identifier_text(const std::string & text);

struct IdentifierTokenSet
{
  typedef InternedTextAtom InternedName;

  static const std::size_t maximum_globally_interned_identifier_length = 128;

  bool contains(const std::string & name) const
  {
    if(name.empty()) {
      return false;
    }
    InternedName atom = find_text_atom(name);
    if(atom && names.find(atom) != names.end()) {
      return true;
    }
    for(std::size_t i = 0; i < owned_names.size(); ++i) {
      if(*owned_names[i] == name) {
        return true;
      }
    }
    return false;
  }

  void reserve(std::size_t count)
  {
    names.reserve(count);
  }

  void insert(InternedName name)
  {
    if(!name) {
      return;
    }
    names.insert(name);
  }

  void insert(const char * data, std::size_t length)
  {
    if(length <= maximum_globally_interned_identifier_length) {
      insert(intern_text_atom(data, length));
      return;
    }
    for(std::size_t i = 0; i < owned_names.size(); ++i) {
      if(owned_names[i]->size() == length &&
         owned_names[i]->compare(0, length, data, length) == 0) {
        return;
      }
    }
    std::shared_ptr<const std::string> owned(
        new const std::string(data, length));
    names.insert(owned.get());
    owned_names.push_back(owned);
  }

  std::unordered_set<InternedName> names;
  // Large synthesized identifiers are generally unique. Keep them alive only
  // as long as the token set that needs them instead of retaining them in the
  // process-global atom pool. Shared ownership preserves pointer stability
  // when cached token sets are copied.
  std::vector<std::shared_ptr<const std::string> > owned_names;
};

IdentifierTokenSet collect_identifier_tokens(const std::string & text);
void maybe_complete_sizeof_type(SemanticContext & ctx, const TypePtr & type);
const CppAstNode * unwrap_initializer_payload(const CppAstNode * initializer);
bool infer_unknown_bound_array_size(const TypePtr & type,
                                    const CppAstNode * initializer,
                                    std::size_t & out_bound);
bool infer_unknown_bound_array_size(SemanticContext & ctx,
                                    Scope & scope,
                                    const TypePtr & type,
                                    const CppAstNode * initializer,
                                    std::size_t & out_bound);
TypePtr apply_initializer_array_bound(const TypePtr & type,
                                      const CppAstNode * initializer);
TypePtr apply_initializer_array_bound(SemanticContext & ctx,
                                      Scope & scope,
                                      const TypePtr & type,
                                      const CppAstNode * initializer);
std::string recog_token_text_for_span(const RecogToken & token);
bool recog_token_text_needs_separator(const RecogToken & lhs,
                                      const RecogToken & rhs);
std::string remove_space_chars(std::string text);
bool is_builtin_operator_function_name(const std::string & name);
bool is_literal_operator_function_name(const std::string & name);
bool is_conversion_function_name(const std::string & name);
std::string append_diagnostic_context_message(const std::string & message);
std::string replace_identifier_token_text(const std::string & text,
                                          const std::string & name,
                                          const std::string & replacement,
                                          bool & changed);
std::string replace_elaborated_identifier_token_text(const std::string & text,
                                                     const std::string & name,
                                                     const std::string & replacement,
                                                     bool & changed);
template<typename TokenSource>
std::string spaced_token_span_text(const TokenSource & tokens,
                                   std::size_t start,
                                   std::size_t end)
{
  std::vector<RecogToken> span = cpp_decl::slice_recog_tokens(tokens, start, end);
  if(!span.empty() && span.back().is_eof()) {
    span.pop_back();
  }

  std::string out;
  for(std::size_t i = 0; i < span.size(); ++i) {
    if(i != 0 && recog_token_text_needs_separator(span[i - 1], span[i])) {
      out += ' ';
    }
    out += recog_token_text_for_span(span[i]);
  }
  return out;
}

template<typename TokenSource>
std::string fully_spaced_token_span_text(const TokenSource & tokens,
                                         std::size_t start,
                                         std::size_t end)
{
  std::vector<RecogToken> span = cpp_decl::slice_recog_tokens(tokens, start, end);
  if(!span.empty() && span.back().is_eof()) {
    span.pop_back();
  }

  std::string out;
  for(std::size_t i = 0; i < span.size(); ++i) {
    if(i != 0) {
      out += ' ';
    }
    out += recog_token_text_for_span(span[i]);
  }
  return out;
}

CallValueCategory to_call_value_category(ValueCategory category);

template<typename Result>
bool lookup_result_present(const Result & result)
{
  return static_cast<bool>(result);
}

const CppAstNode * find_child_kind(const CppAstNode & node,
                                   CppAstKind kind,
                                   std::size_t ordinal = 0);
MemberAccess default_access_for_class_kind(const std::string & class_kind);
MemberAccess access_from_node(const CppAstNode & node);
std::string describe_expression_for_diagnostic(const CppAstNode & node);
std::string describe_scope_bindings_for_diagnostic(const Scope & scope);
void set_expr_metadata(DumpNode & node,
                       const TypePtr & type,
                       ValueCategory category);

}  // namespace callsemantic_internal
