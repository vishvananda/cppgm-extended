#include "callsemantic_phase_bridge.h"

#include "cpp_decl_bridge.h"
#include "template_api.h"
#include "template_scope.h"

using namespace std;

namespace callsemantic_phase_bridge {

using namespace cpp_decl;
using namespace semantic_model;

namespace {

bool sizeof_type_id_bound_is_dependent(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node)
{
  if(node.kind != CppAstKind::sizeof_expression ||
     node.children.size() != 1 ||
     node.children[0].kind != CppAstKind::type_id) {
    return false;
  }

  TypePtr operand_type;
  try {
    if(!ctx.parse_type_id(scope, node.children[0], operand_type, true) ||
       !operand_type) {
      return false;
    }
  } catch(const std::logic_error &) {
    return false;
  }

  return ctx.scope_has_template_placeholders(scope) &&
         ctx.sizeof_depends_on_template_parameters(operand_type);
}

bool sizeof_pack_bound_is_dependent(Scope & scope, const CppAstNode & node)
{
  return node.kind == CppAstKind::sizeof_pack_expression &&
         node.children.size() == 1 &&
         node.children[0].kind == CppAstKind::identifier &&
         (template_api::type::scope_has_type_parameter_pack_name(
              scope,
              node.children[0].value) ||
          template_scope::scope_has_value_parameter_pack_name(
              scope,
              node.children[0].value));
}

}  // namespace

cpp_decl::AstDeclHooks make_decl_hooks(SemanticContext & ctx,
                                       Scope & scope,
                                       bool reference_class_templates_only)
{
  cpp_decl::AstDeclHooks hooks;
  hooks.lookup_type = [&ctx, &scope, reference_class_templates_only](const string & name) {
    return ctx.lookup_type(scope, name, reference_class_templates_only);
  };
  hooks.parse_decltype_specifier = [&ctx, &scope](const CppAstNode & node, TypePtr & out) {
    return ctx.parse_decltype_specifier(scope, node, out);
  };
  hooks.evaluate_constant_expression =
      [&ctx, &scope](const CppAstNode & node, long long & out)
      {
        return ctx.evaluate_constant_expression(scope, node, out);
      };
  hooks.array_bound_is_dependent =
      [&ctx, &scope](const CppAstNode & node)
      {
        const std::string text = node_text(node);
        return sizeof_type_id_bound_is_dependent(ctx, scope, node) ||
               sizeof_pack_bound_is_dependent(scope, node) ||
               (!text.empty() &&
                (ctx.text_mentions_template_placeholders(scope, text) ||
                 ctx.text_mentions_dependent_non_namespace_binding_names(scope, text)));
      };
  hooks.expand_parameter_clause_packs =
      [&ctx, &scope](const CppAstNode & node,
                     CppAstNode & expanded_clause,
                     vector<const CppAstNode *> * default_args_out)
      {
        return template_api::signature::expand_parameter_clause_pack_patterns(
            ctx,
            scope,
            node,
            expanded_clause,
            default_args_out);
      };
  hooks.type_name_is_parameter_pack =
      [&scope](const string & name)
      {
        return template_api::type::scope_has_type_parameter_pack_name(scope, name);
      };
  hooks.normalize_function_parameters = true;
  return hooks;
}

}  // namespace callsemantic_phase_bridge
