#pragma once

#include <functional>
#include <string>
#include <vector>

#include "constant_value.h"
#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"
#include "template_api.h"
#include "template_model.h"
#include "witness_api.h"

class SemanticContext;

namespace resolved_source_semantics {
struct ResolvedQualifiedId;
}

namespace callsemantic {

struct ConstantValueLookupCallbacks
{
  std::function<std::string(const CppAstNode &, const std::string &, bool)>
      source_location_for_name_in_subtree;
  std::function<std::string(const std::string &)>
      earliest_qualified_use_location_for_prefix;
  std::function<std::string(const std::string &)>
      earliest_qualified_use_location_for_value;
};

bool materialize_constant_binding_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::ValueBinding & binding,
    constant_eval::ConstexprValue & value);

bool lookup_constant_template_id_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateIdSyntax & template_id,
    const std::string & display_name,
    constant_eval::ConstexprValue & out);

void record_constexpr_direct_function_call_source_use(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const CppAstNode & callee,
    const resolved_source_semantics::ResolvedQualifiedId & selected_call,
    const cpp_decl::TemplateIdSyntax * template_id_syntax,
    std::size_t explicit_arg_count);

bool lookup_constant_template_member_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateIdSyntax & qualifier_template_id,
    const std::string & member_name,
    const std::string & display_name,
    constant_eval::ConstexprValue & out);

bool lookup_constant_value_node(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const std::string & name,
    const CppAstNode * node,
    constant_eval::ConstexprValue & out);

bool lookup_constant_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const std::string & name,
    constant_eval::ConstexprValue & out);

bool lookup_constant_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const std::string & name,
    long long & out);

semantic_model::Scope * resolve_qualified_scope_for_node(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    semantic_model::Scope & scope,
    const cpp_decl::QualifiedName & qualified,
    const CppAstNode & node,
    bool allow_dependent_class_qualifiers);

}  // namespace callsemantic
