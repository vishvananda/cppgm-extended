#include "semantic_consteval.h"

#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "constructor_lifecycle_service.h"
#include "parser_trace.h"
#include "semantic_builtins.h"
#include "semantic_context.h"
#include "semantic_conversion.h"
#include "semantic_class_model.h"
#include "semantic_declaration.h"
#include "semantic_lookup.h"
#include "semantic_overload.h"
#include "semantic_template_function.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"
#include "template_witness.h"

namespace semantic_consteval {

using namespace cpp_decl;
using namespace semantic_conversion;
using namespace semantic_lookup;
using namespace semantic_model;

namespace {

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const std::string & location)
      : active(!location.empty())
  {
    if(active) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedTemplateUseLocation()
  {
    if(active) {
      parser_trace::pop_use_location();
    }
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;

  bool active = false;
};

struct ScopedSuppressedTemplateUseLocation
{
  explicit ScopedSuppressedTemplateUseLocation(bool active)
      : active_(active)
  {
    if(active_) {
      parser_trace::push_use_location("\x1d");
    }
  }

  ~ScopedSuppressedTemplateUseLocation()
  {
    if(active_) {
      parser_trace::pop_use_location();
    }
  }

  ScopedSuppressedTemplateUseLocation(
      const ScopedSuppressedTemplateUseLocation &) = delete;
  ScopedSuppressedTemplateUseLocation & operator=(
      const ScopedSuppressedTemplateUseLocation &) = delete;

private:
  bool active_ = false;
};

const semantic_model::FieldInfo * first_aggregate_field(const semantic_model::ClassInfo & info)
{
  return info.fields.empty() ? nullptr : &info.fields[0];
}

const semantic_model::FieldInfo * aggregate_input_field(SemanticContext & ctx,
                                                        const semantic_model::FieldInfo & field)
{
  if(!field.is_anonymous_storage) {
    return &field;
  }
  semantic_model::ClassInfo * storage_info = ctx.class_info_for_type(field.type);
  return storage_info ? first_aggregate_field(*storage_info) : &field;
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

CppAstNode * find_child_kind_mutable(CppAstNode & node, CppAstKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

const CppAstNode * unwrap_initializer_payload(const CppAstNode & node)
{
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return &node.children[0];
  }
  return &node;
}

std::vector<const CppAstNode *> initializer_argument_nodes(const CppAstNode & node)
{
  std::vector<const CppAstNode *> args;
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return initializer_argument_nodes(node.children[0]);
  }
  if(node.kind == CppAstKind::call_expression) {
    if(const CppAstNode * argument_list = find_child_kind(node, CppAstKind::argument_list)) {
      if(argument_list->children.size() == 1 &&
         argument_list->children[0].kind == CppAstKind::braced_init_list) {
        return initializer_argument_nodes(argument_list->children[0]);
      }
      for(size_t i = 0; i < argument_list->children.size(); ++i) {
        args.push_back(&argument_list->children[i]);
      }
      return args;
    }
    if(const CppAstNode * paren_args = find_child_kind(node, CppAstKind::paren_argument_list)) {
      if(paren_args->children.size() == 1 &&
         paren_args->children[0].kind == CppAstKind::braced_init_list) {
        return initializer_argument_nodes(paren_args->children[0]);
      }
      for(size_t i = 0; i < paren_args->children.size(); ++i) {
        args.push_back(&paren_args->children[i]);
      }
      return args;
    }
    return args;
  }
  if(node.kind == CppAstKind::paren_initializer ||
     node.kind == CppAstKind::argument_list ||
     node.kind == CppAstKind::paren_argument_list ||
     node.kind == CppAstKind::braced_init_list) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      args.push_back(&node.children[i]);
    }
    return args;
  }
  args.push_back(&node);
  return args;
}

bool expand_initializer_argument_nodes(SemanticContext & ctx,
                                       Scope & scope,
                                       std::vector<const CppAstNode *> & args,
                                       std::vector<CppAstNode> & expanded_storage)
{
  bool has_pack = false;
  for(size_t i = 0; i < args.size(); ++i) {
    if(!args[i]) {
      return false;
    }
    if(args[i]->kind == CppAstKind::pack_expansion_expression) {
      has_pack = true;
    }
  }
  if(!has_pack) {
    expanded_storage.clear();
    return true;
  }

  expanded_storage.clear();
  expanded_storage.reserve(args.size());
  for(size_t i = 0; i < args.size(); ++i) {
    const CppAstNode * arg = args[i];
    if(arg->kind != CppAstKind::pack_expansion_expression) {
      expanded_storage.push_back(*arg);
      continue;
    }

    std::vector<CppAstNode> expanded_nodes;
    if(!ctx.expand_pack_argument_node(scope, *arg, expanded_nodes)) {
      return false;
    }
    expanded_storage.insert(expanded_storage.end(),
                            expanded_nodes.begin(),
                            expanded_nodes.end());
  }

  args.clear();
  args.reserve(expanded_storage.size());
  for(size_t i = 0; i < expanded_storage.size(); ++i) {
    args.push_back(&expanded_storage[i]);
  }
  return true;
}

bool initializer_arg_needs_constexpr_value_selection(const CppAstNode & node)
{
  if(node.kind == CppAstKind::pack_expansion_expression) {
    return true;
  }
  if(node.kind == CppAstKind::unary_expression &&
     node.has_token &&
     node.simple_type == OP_AMP) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(initializer_arg_needs_constexpr_value_selection(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool initializer_args_need_constexpr_value_selection(
    const std::vector<const CppAstNode *> & args)
{
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i] && initializer_arg_needs_constexpr_value_selection(*args[i])) {
      return true;
    }
  }
  return false;
}

bool scope_has_template_placeholders(SemanticContext & ctx,
                                     Scope & scope)
{
  return ctx.scope_has_template_placeholders(scope);
}

bool type_depends_on_template_parameter(SemanticContext & ctx,
                                        const TypePtr & type)
{
  return ctx.type_depends_on_template_parameter(type);
}

bool evaluate_typed_initializer_value(SemanticContext & ctx,
                                      Scope & scope,
                                      constant_eval::Evaluator & evaluator,
                                      const CppAstNode & node,
                                      const TypePtr & target,
                                      constant_eval::ConstexprValue & out);

const CppAstNode * constexpr_function_body(SemanticContext & ctx,
                                           const FunctionBinding & binding)
{
  if(!binding.body) {
    return nullptr;
  }
  return ctx.materialize_lazy_function_body(*binding.body);
}

FunctionBinding * ensure_constexpr_function_definition(SemanticContext & ctx,
                                                       FunctionBinding * binding,
                                                       Scope & use_scope)
{
  if(!binding) {
    return nullptr;
  }
  if(binding->owner_class && binding->owner_class->is_explicit_specialization) {
    return binding;
  }
  binding = semantic_template_function::acquire_function_definition_binding(
      ctx,
      binding,
      use_scope);
  if(!binding) {
    return nullptr;
  }
  semantic_template_function::note_ensured_function_definition_materialized_by_lifecycle(
      ctx,
      binding);
  return binding;
}

bool evaluate_constexpr_target_conversion(SemanticContext & ctx,
                                          Scope & scope,
                                          constant_eval::Evaluator & evaluator,
                                          const CppAstNode & expr,
                                          const constant_eval::ConstexprValue & source_value,
                                          const TypePtr & target,
                                          constant_eval::ConstexprValue & out);

bool evaluate_constexpr_value_member_conversion(SemanticContext & ctx,
                                                Scope & scope,
                                                const CppAstNode & expr,
                                                const TypePtr & target,
                                                constant_eval::ConstexprValue & out);

bool evaluate_value_initialized_type(SemanticContext & ctx,
                                     Scope & scope,
                                     constant_eval::Evaluator & evaluator,
                                     const TypePtr & type,
                                     constant_eval::ConstexprValue & out);

void collect_conversion_function_names(SemanticContext & ctx,
                                       ClassInfo & info,
                                       std::set<ClassInfo *> & visited,
                                       std::set<ClassInfo *> & visited_virtual,
                                       std::set<std::string> & out)
{
  if(!visited.insert(&info).second) {
    return;
  }
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    if(ctx.is_conversion_function_name(it->first)) {
      out.insert(it->first);
    }
  }
  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    BaseInfo & base = info.bases[i];
    if(base.is_virtual && !visited_virtual.insert(base.type).second) {
      continue;
    }
    if(base.type) {
      collect_conversion_function_names(ctx,
                                        *base.type,
                                        visited,
                                        visited_virtual,
                                        out);
    }
  }
}

void note_constexpr_value_member_conversion_operator(SemanticContext & ctx,
                                                    Scope & scope,
                                                    const CppAstNode & callee,
                                                    const std::string & callee_text,
                                                    const TypePtr & target_base)
{
  if(ctx.template_witness_context().session == nullptr || !target_base) {
    return;
  }

  TypePtr source_type;
  {
    const witness::ScopedTemplateWitnessSourceCapturePause source_capture_pause;
    source_type = ctx.lookup_type_node(scope, callee, callee_text, true);
    if(!source_type) {
      source_type = ctx.lookup_type(scope, callee_text, true);
    }
  }

  ClassInfo * source_class =
      ctx.complete_class_type(strip_top_level_cv(remove_reference_type(source_type)));
  if(!source_class) {
    return;
  }

  std::set<ClassInfo *> visited;
  std::set<ClassInfo *> visited_virtual;
  std::set<std::string> names;
  collect_conversion_function_names(ctx,
                                    *source_class,
                                    visited,
                                    visited_virtual,
                                    names);
  for(std::set<std::string>::const_iterator it = names.begin();
      it != names.end();
      ++it) {
    MemberFunctionLookupResult visible =
        semantic_lookup::lookup_member_functions(*source_class, *it);
    for(std::size_t i = 0; i < visible.functions.size(); ++i) {
      FunctionBinding * binding = visible.functions[i];
      if(!binding || !binding->is_constexpr) {
        continue;
      }
      TypePtr function_type = strip_top_level_cv(binding->type);
      if(!function_type ||
         function_type->kind != Type::TK_FUNCTION ||
         !type_equals(strip_top_level_cv(function_type->inner),
                      strip_top_level_cv(target_base))) {
        continue;
      }
      if(ensure_constexpr_function_definition(ctx, binding, scope)) {
        return;
      }
    }
  }
}

CppAstNode make_fold_bool_literal(bool value)
{
  CppAstNode node;
  node.kind = CppAstKind::keyword_literal;
  node.value = value ? "true" : "false";
  node.has_token = true;
  node.token_kind = RT_SIMPLE;
  node.simple_type = value ? KW_TRUE : KW_FALSE;
  return node;
}

CppAstNode make_fold_binary_expression(const CppAstNode & fold,
                                       const CppAstNode & lhs,
                                       const CppAstNode & rhs)
{
  CppAstNode out = fold;
  out.kind = CppAstKind::binary_expression;
  out.children.clear();
  out.children.push_back(lhs);
  out.children.push_back(rhs);
  return out;
}

bool qualified_name_equal(const cpp_decl::QualifiedName & lhs,
                          const cpp_decl::QualifiedName & rhs)
{
  return lhs.rooted == rhs.rooted &&
         lhs.qualifiers == rhs.qualifiers &&
         lhs.name == rhs.name;
}

bool template_id_syntax_equal(const cpp_decl::TemplateIdSyntax & lhs,
                              const cpp_decl::TemplateIdSyntax & rhs)
{
  if(!qualified_name_equal(lhs.name, rhs.name) ||
     lhs.arguments != rhs.arguments ||
     lhs.argument_syntaxes.size() != rhs.argument_syntaxes.size()) {
    return false;
  }
  for(size_t i = 0; i < lhs.argument_syntaxes.size(); ++i) {
    if(lhs.argument_syntaxes[i].text != rhs.argument_syntaxes[i].text) {
      return false;
    }
    if(static_cast<bool>(lhs.argument_syntaxes[i].template_id) !=
       static_cast<bool>(rhs.argument_syntaxes[i].template_id)) {
      return false;
    }
    if(lhs.argument_syntaxes[i].template_id &&
       !template_id_syntax_equal(*lhs.argument_syntaxes[i].template_id,
                                 *rhs.argument_syntaxes[i].template_id)) {
      return false;
    }
  }
  return true;
}

bool optional_template_id_syntax_equal(
    const std::shared_ptr<cpp_decl::TemplateIdSyntax> & lhs,
    const std::shared_ptr<cpp_decl::TemplateIdSyntax> & rhs)
{
  if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
    return false;
  }
  return !lhs || template_id_syntax_equal(*lhs, *rhs);
}

bool ast_nodes_equal_for_pack_noop(const CppAstNode & lhs,
                                   const CppAstNode & rhs)
{
  if(lhs.kind != rhs.kind ||
     lhs.value != rhs.value ||
     lhs.semantic_type != rhs.semantic_type ||
     !optional_template_id_syntax_equal(lhs.template_id_syntax, rhs.template_id_syntax) ||
     lhs.qualifier_template_id_syntaxes.size() !=
         rhs.qualifier_template_id_syntaxes.size() ||
     lhs.qualifier_type_syntaxes.size() !=
         rhs.qualifier_type_syntaxes.size() ||
     lhs.children.size() != rhs.children.size()) {
    return false;
  }
  for(size_t i = 0; i < lhs.qualifier_template_id_syntaxes.size(); ++i) {
    if(!template_id_syntax_equal(lhs.qualifier_template_id_syntaxes[i],
                                 rhs.qualifier_template_id_syntaxes[i])) {
      return false;
    }
  }
  for(size_t i = 0; i < lhs.qualifier_type_syntaxes.size(); ++i) {
    if(!ast_nodes_equal_for_pack_noop(lhs.qualifier_type_syntaxes[i],
                                      rhs.qualifier_type_syntaxes[i])) {
      return false;
    }
  }
  for(size_t i = 0; i < lhs.children.size(); ++i) {
    if(!ast_nodes_equal_for_pack_noop(lhs.children[i], rhs.children[i])) {
      return false;
    }
  }
  return true;
}

bool expand_fold_operand_nodes(SemanticContext & ctx,
                               Scope & scope,
                               const CppAstNode & operand,
                               std::vector<CppAstNode> & out,
                               bool & expanded)
{
  out.clear();
  expanded = false;

  CppAstNode expansion;
  expansion.kind = CppAstKind::pack_expansion_expression;
  expansion.children.push_back(operand);
  if(!ctx.expand_pack_argument_node(scope, expansion, out)) {
    return false;
  }
  if(out.size() == 1) {
    expanded = !ast_nodes_equal_for_pack_noop(out.front(), operand);
    if(!expanded) {
      out.front() = operand;
    }
    return true;
  }

  expanded = true;
  return true;
}

bool fold_empty_identity_node(const CppAstNode & fold,
                              CppAstNode & out)
{
  if(!fold.has_token) {
    return false;
  }

  if(fold.simple_type == OP_LAND) {
    out = make_fold_bool_literal(true);
    return true;
  }
  if(fold.simple_type == OP_LOR) {
    out = make_fold_bool_literal(false);
    return true;
  }
  return false;
}

bool reduce_fold_expression_node(SemanticContext & ctx,
                                 Scope & scope,
                                 const CppAstNode & fold,
                                 CppAstNode & out)
{
  if(fold.kind != CppAstKind::fold_expression) {
    return false;
  }

  if(fold.children.size() == 2 && fold.children[0].kind == CppAstKind::ellipsis) {
    std::vector<CppAstNode> operands;
    bool expanded = false;
    if(!expand_fold_operand_nodes(ctx, scope, fold.children[1], operands, expanded)) {
      return false;
    }
    if(!expanded) {
      return false;
    }
    if(operands.empty()) {
      return fold_empty_identity_node(fold, out);
    }
    out = operands[0];
    for(size_t i = 1; i < operands.size(); ++i) {
      out = make_fold_binary_expression(fold, out, operands[i]);
    }
    return true;
  }

  if(fold.children.size() == 2 && fold.children[1].kind == CppAstKind::ellipsis) {
    std::vector<CppAstNode> operands;
    bool expanded = false;
    if(!expand_fold_operand_nodes(ctx, scope, fold.children[0], operands, expanded)) {
      return false;
    }
    if(!expanded) {
      return false;
    }
    if(operands.empty()) {
      return fold_empty_identity_node(fold, out);
    }
    out = operands.back();
    for(size_t i = operands.size(); i-- > 1;) {
      out = make_fold_binary_expression(fold, operands[i - 1], out);
    }
    return true;
  }

  if(fold.children.size() == 3 && fold.children[1].kind == CppAstKind::ellipsis) {
    std::vector<CppAstNode> lhs_nodes;
    std::vector<CppAstNode> rhs_nodes;
    bool lhs_expanded = false;
    bool rhs_expanded = false;
    if(!expand_fold_operand_nodes(ctx, scope, fold.children[0], lhs_nodes, lhs_expanded) ||
       !expand_fold_operand_nodes(ctx, scope, fold.children[2], rhs_nodes, rhs_expanded)) {
      return false;
    }
    if(lhs_expanded == rhs_expanded) {
      return false;
    }
    if(lhs_expanded) {
      if(lhs_nodes.empty()) {
        out = fold.children[2];
        return true;
      }
      out = fold.children[2];
      for(size_t i = lhs_nodes.size(); i-- > 0;) {
        out = make_fold_binary_expression(fold, lhs_nodes[i], out);
      }
      return true;
    }
    if(rhs_nodes.empty()) {
      out = fold.children[0];
      return true;
    }
    out = fold.children[0];
    for(size_t i = 0; i < rhs_nodes.size(); ++i) {
      out = make_fold_binary_expression(fold, out, rhs_nodes[i]);
    }
    return true;
  }

  return false;
}

bool evaluate_default_initialized_type(SemanticContext & ctx,
                                       Scope & scope,
                                       constant_eval::Evaluator & evaluator,
                                       const TypePtr & type,
                                       constant_eval::ConstexprValue & out);

std::string constexpr_parameter_unique_name(
    const FunctionBinding & binding,
    std::size_t index,
    std::map<std::string, std::size_t> & seen)
{
  const std::string binding_name = function_parameter_binding_name(binding, index);
  const std::string alias_name = function_parameter_alias_name(binding, index);
  const std::string base_name = !alias_name.empty() ? alias_name : binding_name;
  if(base_name.empty()) {
    return binding_name;
  }
  std::size_t & count = seen[base_name];
  ++count;
  if(count == 1) {
    return binding_name.empty() ? base_name : binding_name;
  }
  return base_name + "__pack" + std::to_string(count);
}

std::string trailing_function_parameter_pack_name(const FunctionBinding & binding,
                                                  std::size_t & pack_start)
{
  pack_start = 0;
  if(!binding.source_template ||
     !binding.source_template->has_trailing_function_parameter_pack ||
     binding.source_template->params_pattern.empty()) {
    return std::string();
  }

  const std::size_t explicit_offset =
      function_binding_explicit_parameter_offset(binding);
  const std::size_t pattern_index =
      binding.source_template->params_pattern.size() - 1;
  pack_start = explicit_offset + pattern_index;
  if(pack_start > binding.params.size()) {
    return std::string();
  }

  std::string pack_name =
      function_template_parameter_alias_name(*binding.source_template, pattern_index);
  if(pack_name.empty()) {
    pack_name = binding.source_template->params_pattern[pattern_index].first;
  }
  return pack_name;
}

std::vector<std::pair<std::string, TypePtr> >
constexpr_function_parameters_impl(FunctionBinding & binding)
{
  ensure_function_parameter_aliases(binding);
  std::vector<std::pair<std::string, TypePtr> > params = binding.params;
  std::map<std::string, std::size_t> seen;
  for(std::size_t i = 0; i < params.size(); ++i) {
    params[i].first = constexpr_parameter_unique_name(binding, i, seen);
  }
  return params;
}

Scope make_constexpr_call_scope_impl(Scope & parent,
                                     FunctionBinding * binding,
                                     bool bind_parameters)
{
  Scope scope(&parent, parent.name, false);
  scope.class_info = binding && binding->owner_class ? binding->owner_class : parent.class_info;
  scope.function = binding ? binding : parent.function;
  if(binding && bind_parameters) {
    const std::vector<std::pair<std::string, TypePtr> > params =
        constexpr_function_parameters_impl(*binding);
    std::size_t pack_start = 0;
    const std::string pack_name =
        trailing_function_parameter_pack_name(*binding, pack_start);
    for(size_t i = 0; i < params.size(); ++i) {
      const std::string binding_name = params[i].first;
      if(binding_name.empty()) {
        continue;
      }
      const std::string alias_name = function_parameter_alias_name(*binding, i);
      ValueBinding parameter(ValueBinding::VK_PARAMETER,
                             binding_name,
                             params[i].second);
      scope.values[binding_name] = parameter;
      if(!alias_name.empty() &&
         alias_name != binding_name &&
         scope.values.find(alias_name) == scope.values.end()) {
        scope.values[alias_name] = parameter;
      }
      if(!pack_name.empty() && i >= pack_start) {
        scope.named_value_packs[pack_name].push_back(parameter);
      }
    }
    if(!pack_name.empty()) {
      scope.named_pack_sizes[pack_name] = scope.named_value_packs[pack_name].size();
    }
  }
  return scope;
}

bool evaluate_builtin_or_type_trait_expression(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & expr,
                                               constant_eval::ConstexprValue & value)
{
  const auto try_evaluate_builtin_trait =
      [&](const std::string & builtin_name,
          const std::vector<TypePtr> & builtin_types) -> bool
      {
        for(size_t i = 0; i < builtin_types.size(); ++i) {
          if(type_depends_on_template_parameter(ctx, builtin_types[i])) {
            if(!scope_has_template_placeholders(ctx, scope)) {
              throw std::logic_error("builtin type trait remained dependent: " + builtin_name);
            }
            return false;
          }
        }

        long long trait_value = 0;
        if(builtin_types.size() == 1 &&
           ctx.evaluate_builtin_type_trait(scope, builtin_name, builtin_types[0], trait_value)) {
          value = constant_eval::make_integral_value(
              trait_value,
              semantic_builtins::builtin_type_trait_result_type(builtin_name));
          return true;
        }
        if(builtin_types.size() == 2 &&
           ctx.evaluate_builtin_binary_type_trait(scope,
                                                  builtin_name,
                                                  builtin_types[0],
                                                  builtin_types[1],
                                                  trait_value)) {
          value = constant_eval::make_integral_value(
              trait_value,
              semantic_builtins::builtin_type_trait_result_type(builtin_name));
          return true;
        }
        return false;
      };

  std::string builtin_name;
  std::vector<TypePtr> builtin_types;
  if(semantic_builtins::try_parse_builtin_type_trait_expression(
         ctx, scope, expr, builtin_name, builtin_types) &&
     try_evaluate_builtin_trait(builtin_name, builtin_types)) {
    return true;
  }

  if(expr.kind == CppAstKind::call_expression &&
     expr.children.size() == 2 &&
     expr.children[0].kind == CppAstKind::id_expression &&
     expr.children[0].value == "__builtin_offsetof") {
    const CppAstNode * argument_list = find_child_kind(expr, CppAstKind::argument_list);
    if(!argument_list) {
      argument_list = find_child_kind(expr, CppAstKind::paren_argument_list);
    }
    if(!argument_list || argument_list->children.size() != 2) {
      return false;
    }

    TypePtr object_type;
    if(!ctx.try_parse_builtin_type_trait_call_arg(scope, argument_list->children[0], object_type) ||
       type_depends_on_template_parameter(ctx, object_type)) {
      return false;
    }

    const CppAstNode & member = argument_list->children[1];
    if(member.kind != CppAstKind::id_expression) {
      return false;
    }

    OffsetofFieldInfo found;
    if(!ctx.lookup_offsetof_field(object_type, member.value, found) || !found.found) {
      return false;
    }
    if(found.is_bit_field) {
      throw std::logic_error("__builtin_offsetof on bit-field unsupported");
    }

    value = constant_eval::make_integral_value(static_cast<long long>(found.offset),
                                               make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }

  if(expr.kind == CppAstKind::type_trait_expression &&
     expr.simple_type == KW_NOEXCEPT &&
     expr.children.size() == 1) {
    bool is_nothrow = false;
    if(!ctx.expression_is_nothrow(scope, expr.children[0], is_nothrow)) {
      return false;
    }
    value = constant_eval::make_integral_value(is_nothrow ? 1 : 0,
                                               make_fundamental(FT_BOOL));
    return true;
  }

  if(expr.kind == CppAstKind::type_trait_expression &&
     expr.has_token &&
     expr.token_kind == RT_IDENTIFIER &&
     !expr.children.empty()) {
    if(semantic_builtins::try_parse_builtin_type_trait_expression(
           ctx, scope, expr, builtin_name, builtin_types) &&
       try_evaluate_builtin_trait(builtin_name, builtin_types)) {
      return true;
    }
    return false;
  }

  return false;
}

bool evaluate_scalar_zero_value(const TypePtr & type,
                                constant_eval::ConstexprValue & out)
{
  return constant_eval::constexpr_value_cast(
      constant_eval::make_integral_value(0, make_fundamental(FT_INT)),
      type,
      out);
}

bool find_ctor_mem_initializer(const FunctionBinding & binding,
                               const std::string & name,
                               const CppAstNode *& out)
{
  out = nullptr;
  if(!binding.ctor_initializer) {
    return false;
  }
  for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
    const CppAstNode & init = binding.ctor_initializer->children[i];
    const CppAstNode * id = find_child_kind(init, CppAstKind::mem_initializer_id);
    if(id && id->value == name) {
      out = &init;
      return true;
    }
  }
  return false;
}

bool find_ctor_base_initializer(SemanticContext & ctx,
                                Scope & scope,
                                const FunctionBinding & binding,
                                const ClassInfo & base,
                                const CppAstNode *& out)
{
  out = nullptr;
  if(!binding.ctor_initializer) {
    return false;
  }
  for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
    const CppAstNode & init = binding.ctor_initializer->children[i];
    const CppAstNode * id = find_child_kind(init, CppAstKind::mem_initializer_id);
    if(!id) {
      continue;
    }
    if(id->value == base.name || id->value == base.qualified_name) {
      out = &init;
      return true;
    }
    TypePtr named = ctx.lookup_type(scope, id->value);
    if(named && type_equals(strip_top_level_cv(named), base.type)) {
      out = &init;
      return true;
    }
  }
  return false;
}

bool append_default_arguments(SemanticContext & ctx,
                              Scope & scope,
                              constant_eval::Evaluator & evaluator,
                              const FunctionBinding & binding,
                              size_t explicit_param_offset,
                              std::vector<constant_eval::ConstexprValue> & args)
{
  while(args.size() + explicit_param_offset < binding.params.size()) {
    const size_t param_index = args.size() + explicit_param_offset;
    if(param_index >= binding.default_arguments.size() ||
       !binding.default_arguments[param_index]) {
      return false;
    }
    const CppAstNode * default_arg = binding.default_arguments[param_index];
    const CppAstNode * payload =
        default_arg->children.size() == 1 ? &default_arg->children[0] : default_arg;
    constant_eval::ConstexprValue value;
    if(!evaluator.eval_initializer(*payload, value, binding.params[param_index].second)) {
      return false;
    }
    args.push_back(value);
  }
  return true;
}

bool evaluate_constexpr_constructor(SemanticContext & ctx,
                                    Scope & scope,
                                    constant_eval::Evaluator & evaluator,
                                    FunctionBinding & binding,
                                    const TypePtr & target,
                                    const std::vector<constant_eval::ConstexprValue> & explicit_args,
                                    bool value_initialize_missing_subobjects,
                                    constant_eval::ConstexprValue & out)
{
  const bool zero_arg_default_constructor =
      binding.is_constructor &&
      binding.params.size() == 1 &&
      explicit_args.empty() &&
      (binding.is_defaulted || binding.synthesized) &&
      !binding.is_deleted;
  if((!binding.is_constexpr && !zero_arg_default_constructor) || !binding.owner_class) {
    return false;
  }
  ClassInfo * info = binding.owner_class;
  if(!info->complete) {
    info = ctx.complete_class_type(info->type);
  }
  if(!info || !info->complete) {
    return false;
  }
  if(binding.body && !binding.body->children.empty()) {
    return false;
  }

  std::vector<constant_eval::ConstexprValue> args = explicit_args;
  if(!append_default_arguments(ctx, scope, evaluator, binding, 1, args)) {
    return false;
  }
  if(args.size() + 1 != binding.params.size()) {
    return false;
  }

  Scope & ctor_scope = binding.declaration_scope ? *binding.declaration_scope : scope;
  Scope constexpr_ctor_scope = make_constexpr_call_scope(ctor_scope, &binding);
  const constant_eval::Hooks ctor_hooks = build_hooks(ctx, constexpr_ctor_scope);
  const auto evaluate_ctor_initializer =
      [&](const CppAstNode & init_node,
          const TypePtr & init_type,
          constant_eval::ConstexprValue & value) -> bool
      {
        CppAstNode body;
        body.kind = CppAstKind::compound_statement;
        CppAstNode ret;
        ret.kind = CppAstKind::return_statement;
        ret.children.push_back(init_node);
        body.children.push_back(ret);

        constant_eval::FunctionInfo info;
        info.name = binding.name;
        info.return_type = init_type;
        info.params.assign(binding.params.begin() + 1, binding.params.end());
        info.body = &body;

        constant_eval::Evaluator call_evaluator(ctor_hooks);
        return call_evaluator.call(info, args, value, &ctor_hooks);
      };

  std::vector<std::pair<std::string, constant_eval::ConstexprValue> > members;
  std::vector<bool> member_is_base;

  for(size_t i = 0; i < info->bases.size(); ++i) {
    const BaseInfo & base = info->bases[i];
    const CppAstNode * base_init = nullptr;
    find_ctor_base_initializer(ctx, ctor_scope, binding, *base.type, base_init);

    constant_eval::ConstexprValue base_value;
    if(base_init && base_init->children.size() >= 2) {
      if(!evaluate_ctor_initializer(base_init->children[1], base.type->type, base_value)) {
        return false;
      }
    } else if(!(value_initialize_missing_subobjects ?
                    evaluate_value_initialized_type(ctx,
                                                    constexpr_ctor_scope,
                                                    evaluator,
                                                    base.type->type,
                                                    base_value) :
                    evaluate_default_initialized_type(ctx,
                                                      constexpr_ctor_scope,
                                                      evaluator,
                                                      base.type->type,
                                                      base_value))) {
      return false;
    }

    members.push_back(std::make_pair(base.type->name, base_value));
    member_is_base.push_back(true);
  }

  for(size_t i = 0; i < info->fields.size(); ++i) {
    const FieldInfo & field = info->fields[i];
    const CppAstNode * mem_init = nullptr;
    find_ctor_mem_initializer(binding, field.name, mem_init);

    constant_eval::ConstexprValue field_value;
    if(mem_init && mem_init->children.size() >= 2) {
      if(!evaluate_ctor_initializer(mem_init->children[1], field.type, field_value)) {
        return false;
      }
    } else if(field.default_initializer) {
      if(!evaluate_ctor_initializer(*field.default_initializer, field.type, field_value)) {
        return false;
      }
    } else if(!(value_initialize_missing_subobjects ?
                    evaluate_value_initialized_type(ctx,
                                                    constexpr_ctor_scope,
                                                    evaluator,
                                                    field.type,
                                                    field_value) :
                    evaluate_default_initialized_type(ctx,
                                                      constexpr_ctor_scope,
                                                      evaluator,
                                                      field.type,
                                                      field_value))) {
      return false;
    }

    members.push_back(std::make_pair(field.name, field_value));
    member_is_base.push_back(false);
  }

  out = constant_eval::make_aggregate_value(target ? target : info->type, members, member_is_base);
  return true;
}

bool evaluate_class_typed_initializer(SemanticContext & ctx,
                                      Scope & scope,
                                      constant_eval::Evaluator & evaluator,
                                      const CppAstNode & node,
                                      const TypePtr & target,
                                      constant_eval::ConstexprValue & out)
{
  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  ClassInfo * info = ctx.complete_class_type(target_base);
  if(!info || !info->complete) {
    return false;
  }

  std::vector<const CppAstNode *> args = initializer_argument_nodes(node);
  std::vector<CppAstNode> expanded_arg_storage;
  if(!expand_initializer_argument_nodes(ctx, scope, args, expanded_arg_storage)) {
    return false;
  }
  const bool expanded_initializer_pack = !expanded_arg_storage.empty();
  const bool prefer_evaluated_constructor_args =
      expanded_initializer_pack ||
      initializer_args_need_constexpr_value_selection(args);
  if(node.kind == CppAstKind::braced_init_list && ctx.can_synthesize_aggregate_constructor(*info)) {
    std::vector<std::pair<std::string, constant_eval::ConstexprValue> > members;
    std::vector<bool> member_is_base;
    size_t arg_index = 0;

    for(size_t i = 0; i < info->bases.size(); ++i) {
      constant_eval::ConstexprValue base_value;
      if(arg_index < args.size()) {
        if(!evaluate_typed_initializer_value(ctx,
                                             scope,
                                             evaluator,
                                             *args[arg_index],
                                             info->bases[i].type->type,
                                             base_value)) {
          return false;
        }
        ++arg_index;
      } else if(!evaluate_value_initialized_type(ctx,
                                                 scope,
                                                 evaluator,
                                                 info->bases[i].type->type,
                                                 base_value)) {
        return false;
      }
      members.push_back(std::make_pair(info->bases[i].type->name, base_value));
      member_is_base.push_back(true);
    }

    const std::size_t aggregate_count =
        semantic_class_model::aggregate_element_count(*info);
    if(args.size() - arg_index > aggregate_count) {
      return false;
    }
    for(size_t i = 0; i < info->fields.size(); ++i) {
      const FieldInfo & field = info->fields[i];
      const FieldInfo * input_field = aggregate_input_field(ctx, field);
      constant_eval::ConstexprValue field_value;
      if(arg_index < args.size() && i < aggregate_count) {
        if(!evaluate_typed_initializer_value(ctx,
                                             scope,
                                             evaluator,
                                             *args[arg_index],
                                             input_field->type,
                                             field_value)) {
          return false;
        }
        ++arg_index;
      } else if(input_field->default_initializer) {
        if(!evaluate_typed_initializer_value(ctx,
                                             scope,
                                             evaluator,
                                             *input_field->default_initializer,
                                             input_field->type,
                                             field_value)) {
          return false;
        }
      } else if(!evaluate_value_initialized_type(ctx,
                                                 scope,
                                                 evaluator,
                                                 input_field->type,
                                                 field_value)) {
        return false;
      }
      members.push_back(std::make_pair(field.name, field_value));
      member_is_base.push_back(false);
      if(info->class_kind == "union") {
        break;
      }
    }

    out = constant_eval::make_aggregate_value(target, members, member_is_base);
    return true;
  }

  std::vector<ExprInfo> converted;
  std::vector<const CppAstNode *> chosen_args = args;
  std::vector<constant_eval::ConstexprValue> evaluated_args;
  if(args.size() == 1) {
    constant_eval::ConstexprValue direct_value;
    if(evaluator.eval_initializer(*args[0], direct_value) &&
       constant_eval::constexpr_value_cast(direct_value, target, out)) {
      out.type = target;
      return true;
    }
  }
  const ConstructorSelectionOptions ctor_options =
      constructor_lifecycle_service::selection_options_for(
          constructor_lifecycle_service::direct_initialization_profile(
              "constexpr construction"));
  bool selected_from_evaluated_args = false;
  const auto try_select_from_evaluated_args =
      [&]() -> FunctionBinding *
      {
        if(node.kind == CppAstKind::braced_init_list) {
          return nullptr;
        }
        if(selected_from_evaluated_args) {
          return nullptr;
        }
        std::vector<ExprInfo> evaluated_arg_infos;
        evaluated_args.clear();
        evaluated_args.reserve(args.size());
        evaluated_arg_infos.reserve(args.size());
        for(size_t i = 0; i < args.size(); ++i) {
          constant_eval::ConstexprValue value;
          if(!evaluator.eval_initializer(*args[i], value)) {
            evaluated_args.clear();
            return nullptr;
          }
          ExprInfo info_arg;
          info_arg.type = value.type;
          info_arg.category = VC_PRVALUE;
          if(value.kind == constant_eval::ConstexprValue::CV_NULLPTR) {
            info_arg.null_pointer_constant = true;
          } else if(value.kind == constant_eval::ConstexprValue::CV_INTEGRAL) {
            long long integral_value = 0;
            info_arg.null_pointer_constant =
                constant_eval::constexpr_value_to_integral(value, integral_value) &&
                integral_value == 0;
          }
          evaluated_args.push_back(value);
          evaluated_arg_infos.push_back(info_arg);
        }
        converted.clear();
        FunctionBinding * selected =
            ctx.select_constructor_from_exprs(scope,
                                              *info,
                                              evaluated_arg_infos,
                                              converted,
                                              nullptr,
                                              ctor_options);
        selected_from_evaluated_args = selected != nullptr;
        if(!selected_from_evaluated_args) {
          evaluated_args.clear();
        }
        return selected;
      };
  FunctionBinding * ctor =
      prefer_evaluated_constructor_args ? try_select_from_evaluated_args() : nullptr;
  if(!ctor) {
    ctor =
        (node.kind == CppAstKind::braced_init_list) ?
            ctx.select_constructor_for_direct_braced_init(scope,
                                                          *info,
                                                          node,
                                                          converted,
                                                          ctor_options) :
            ctx.select_constructor(scope,
                                   *info,
                                   args,
                                   converted,
                                   ctor_options);
  }
  if(!ctor) {
    ctor = try_select_from_evaluated_args();
  }
  if(!ctor) {
    return false;
  }
  if(args.size() == 1 && (ctor->is_copy_constructor || ctor->is_move_constructor)) {
    ExprInfo source_expr = ctx.analyze_expression(scope, *args[0]);
    TypePtr source_type = strip_top_level_cv(remove_reference_type(source_expr.type));
    // If the source already denotes a T, only a real constexpr T value should
    // succeed here. Falling back to copy/move ctor selection on a non-constexpr
    // runtime expression (for example an allocator parameter) just re-enters
    // the same path indefinitely.
    if(source_type && type_equals(source_type, target_base)) {
      return false;
    }
  }
  if(node.kind == CppAstKind::braced_init_list &&
     ctor->params.size() == 2 &&
     ctx.is_initializer_list_type(ctor->params[1].second, nullptr, nullptr)) {
    chosen_args.assign(1, &node);
  }

  std::vector<constant_eval::ConstexprValue> explicit_args;
  explicit_args.reserve(chosen_args.size());
  for(size_t i = 0; i < chosen_args.size(); ++i) {
    constant_eval::ConstexprValue value =
        selected_from_evaluated_args && i < evaluated_args.size() ?
            evaluated_args[i] :
            constant_eval::ConstexprValue();
    if(!selected_from_evaluated_args || i >= evaluated_args.size()) {
      if(!evaluator.eval_initializer(*chosen_args[i], value, i + 1 < ctor->params.size()
                                                              ? ctor->params[i + 1].second
                                                              : TypePtr())) {
        if(!selected_from_evaluated_args) {
          FunctionBinding * evaluated_ctor = try_select_from_evaluated_args();
          if(evaluated_ctor && evaluated_args.size() == chosen_args.size()) {
            ctor = evaluated_ctor;
            explicit_args = evaluated_args;
            break;
          }
        }
        return false;
      }
    }
    explicit_args.push_back(value);
  }

  return evaluate_constexpr_constructor(ctx,
                                        scope,
                                        evaluator,
                                        *ctor,
                                        target,
                                        explicit_args,
                                        args.empty(),
                                        out);
}

bool evaluate_array_typed_initializer(SemanticContext & ctx,
                                      Scope & scope,
                                      constant_eval::Evaluator & evaluator,
                                      const CppAstNode & node,
                                      const TypePtr & target,
                                      constant_eval::ConstexprValue & out)
{
  TypePtr array_type = strip_top_level_cv(remove_reference_type(target));
  if(!array_type || array_type->kind != Type::TK_ARRAY || !array_type->inner) {
    return false;
  }

  const CppAstNode * payload = unwrap_initializer_payload(node);
  if(!payload) {
    return false;
  }

  if(payload->kind == CppAstKind::braced_init_list) {
    std::vector<const CppAstNode *> args = initializer_argument_nodes(*payload);
    std::vector<CppAstNode> expanded_arg_storage;
    if(!expand_initializer_argument_nodes(ctx, scope, args, expanded_arg_storage)) {
      return false;
    }

    const size_t bound = array_type->has_bound ? array_type->bound : args.size();
    if(args.size() > bound) {
      return false;
    }
    std::vector<constant_eval::ConstexprValue> elements;
    elements.reserve(bound);
    for(size_t i = 0; i < args.size(); ++i) {
      constant_eval::ConstexprValue element;
      if(!evaluate_typed_initializer_value(ctx,
                                           scope,
                                           evaluator,
                                           *args[i],
                                           array_type->inner,
                                           element)) {
        return false;
      }
      elements.push_back(element);
    }
    for(size_t i = args.size(); i < bound; ++i) {
      constant_eval::ConstexprValue element;
      if(!evaluate_value_initialized_type(ctx, scope, evaluator, array_type->inner, element)) {
        return false;
      }
      elements.push_back(element);
    }
    TypePtr out_type = array_type->has_bound ? target : make_array(array_type->inner, true, bound);
    out = constant_eval::make_array_value(out_type, elements);
    return true;
  }

  constant_eval::ConstexprValue value;
  if(!evaluator.eval_expr(*payload, value) || value.kind != constant_eval::ConstexprValue::CV_ARRAY) {
    return false;
  }
  if(array_type->has_bound && value.array_elements.size() > array_type->bound) {
    return false;
  }

  std::vector<constant_eval::ConstexprValue> elements = value.array_elements;
  if(array_type->has_bound) {
    while(elements.size() < array_type->bound) {
      constant_eval::ConstexprValue element;
      if(!evaluate_value_initialized_type(ctx, scope, evaluator, array_type->inner, element)) {
        return false;
      }
      elements.push_back(element);
    }
    out = constant_eval::make_array_value(target, elements);
    return true;
  }

  out = constant_eval::make_array_value(
      make_array(array_type->inner, true, elements.size()),
      elements);
  return true;
}

bool evaluate_value_initialized_type(SemanticContext & ctx,
                                     Scope & scope,
                                     constant_eval::Evaluator & evaluator,
                                     const TypePtr & type,
                                     constant_eval::ConstexprValue & out)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      return false;
    }
    std::vector<constant_eval::ConstexprValue> elements;
    elements.reserve(base->bound);
    for(size_t i = 0; i < base->bound; ++i) {
      constant_eval::ConstexprValue element;
      if(!evaluate_value_initialized_type(ctx, scope, evaluator, base->inner, element)) {
        return false;
      }
      elements.push_back(element);
    }
    out = constant_eval::make_array_value(type, elements);
    return true;
  }

  if(ctx.complete_class_type(base)) {
    CppAstNode empty;
    empty.kind = CppAstKind::braced_init_list;
    return evaluate_class_typed_initializer(ctx, scope, evaluator, empty, type, out);
  }

  return evaluate_scalar_zero_value(type, out);
}

bool evaluate_default_initialized_type(SemanticContext & ctx,
                                       Scope & scope,
                                       constant_eval::Evaluator & evaluator,
                                       const TypePtr & type,
                                       constant_eval::ConstexprValue & out)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      return false;
    }
    std::vector<constant_eval::ConstexprValue> elements;
    elements.reserve(base->bound);
    for(size_t i = 0; i < base->bound; ++i) {
      constant_eval::ConstexprValue element;
      if(!evaluate_default_initialized_type(ctx, scope, evaluator, base->inner, element)) {
        return false;
      }
      elements.push_back(element);
    }
    out = constant_eval::make_array_value(type, elements);
    return true;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info) {
    std::vector<ExprInfo> no_args;
    std::vector<ExprInfo> converted;
    const ConstructorSelectionOptions ctor_options =
        constructor_lifecycle_service::selection_options_for(
            constructor_lifecycle_service::direct_initialization_profile(
                "constexpr default construction"));
    FunctionBinding * ctor =
        ctx.select_constructor_from_exprs(scope,
                                          *info,
                                          no_args,
                                          converted,
                                          nullptr,
                                          ctor_options);
    if(!ctor) {
      return false;
    }
    std::vector<constant_eval::ConstexprValue> explicit_args;
    return evaluate_constexpr_constructor(ctx,
                                          scope,
                                          evaluator,
                                          *ctor,
                                          type,
                                          explicit_args,
                                          false,
                                          out);
  }

  return false;
}

bool evaluate_typed_initializer_value(SemanticContext & ctx,
                                      Scope & scope,
                                      constant_eval::Evaluator & evaluator,
                                      const CppAstNode & node,
                                      const TypePtr & target,
                                      constant_eval::ConstexprValue & out)
{
  if(!target) {
    return false;
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(!target_base) {
    return false;
  }

  if(is_reference_type(strip_top_level_cv(target))) {
    constant_eval::ConstexprValue referred;
    if(!evaluate_typed_initializer_value(ctx, scope, evaluator, node, target_base, referred)) {
      return false;
    }
    referred.type = target;
    out = referred;
    return true;
  }

  if(target_base->kind == Type::TK_ARRAY) {
    return evaluate_array_typed_initializer(ctx, scope, evaluator, node, target, out);
  }

  const CppAstNode * payload = unwrap_initializer_payload(node);
  if(!payload) {
    return false;
  }

  if(ctx.complete_class_type(target_base) &&
     payload->kind != CppAstKind::paren_initializer &&
     payload->kind != CppAstKind::paren_argument_list &&
     payload->kind != CppAstKind::braced_init_list) {
    constant_eval::ConstexprValue value;
    if(evaluator.eval_expr(*payload, value)) {
      if(constant_eval::constexpr_value_cast(value, target, out) ||
         evaluate_constexpr_target_conversion(ctx,
                                             scope,
                                             evaluator,
                                             *payload,
                                             value,
                                             target,
                                             out)) {
        out.type = target;
        return true;
      }
      CppAstNode direct_init;
      direct_init.kind = CppAstKind::paren_initializer;
      direct_init.children.push_back(*payload);
      if(evaluate_class_typed_initializer(ctx,
                                          scope,
                                          evaluator,
                                          direct_init,
                                          target,
                                          out)) {
        out.type = target;
        return true;
      }
    }
  }

  if(ctx.complete_class_type(target_base)) {
    return evaluate_class_typed_initializer(ctx, scope, evaluator, node, target, out);
  }

  if(payload->kind == CppAstKind::paren_initializer ||
     payload->kind == CppAstKind::paren_argument_list ||
     payload->kind == CppAstKind::braced_init_list) {
    if(payload->children.empty()) {
      return evaluate_scalar_zero_value(target, out);
    }
    if(payload->children.size() != 1) {
      return false;
    }
    constant_eval::ConstexprValue value;
    if(!evaluator.eval_expr(payload->children[0], value)) {
      return false;
    }
    return constant_eval::constexpr_value_cast(value, target, out);
  }

  constant_eval::ConstexprValue value;
  if(evaluate_constexpr_value_member_conversion(ctx, scope, *payload, target, out)) {
    out.type = target;
    return true;
  }
  if(!evaluator.eval_expr(*payload, value)) {
    return false;
  }
  if(!constant_eval::constexpr_value_cast(value, target, out) &&
     !evaluate_constexpr_target_conversion(ctx,
                                          scope,
                                          evaluator,
                                          *payload,
                                          value,
                                          target,
                                          out)) {
    return false;
  }
  out.type = target;
  return true;
}

bool evaluate_constexpr_overloaded_operator_expression(SemanticContext & ctx,
                                                       Scope & scope,
                                                       constant_eval::Evaluator & evaluator,
                                                       const CppAstNode & expr,
                                                       constant_eval::ConstexprValue & out)
{
  const auto overloaded_operator_name = [&]() -> std::string
  {
    if(expr.kind == CppAstKind::binary_expression && expr.children.size() == 2) {
      if(node_has_simple_type(expr, OP_PLUS)) return "operator+";
      if(node_has_simple_type(expr, OP_MINUS)) return "operator-";
      if(node_has_simple_type(expr, OP_STAR)) return "operator*";
      if(node_has_simple_type(expr, OP_DIV)) return "operator/";
      if(node_has_simple_type(expr, OP_MOD)) return "operator%";
      if(node_has_simple_type(expr, OP_BOR)) return "operator|";
      if(node_has_simple_type(expr, OP_XOR)) return "operator^";
      if(node_has_simple_type(expr, OP_AMP)) return "operator&";
      if(node_has_simple_type(expr, OP_LSHIFT)) return "operator<<";
      if(node_has_simple_type(expr, OP_RSHIFT)) return "operator>>";
      if(node_has_simple_type(expr, OP_EQ)) return "operator==";
      if(node_has_simple_type(expr, OP_NE)) return "operator!=";
      if(node_has_simple_type(expr, OP_LT)) return "operator<";
      if(node_has_simple_type(expr, OP_GT)) return "operator>";
      if(node_has_simple_type(expr, OP_LE)) return "operator<=";
      if(node_has_simple_type(expr, OP_GE)) return "operator>=";
      return std::string();
    }
    if(expr.kind == CppAstKind::unary_expression && expr.children.size() == 1) {
      if(node_has_simple_type(expr, OP_PLUS)) return "operator+";
      if(node_has_simple_type(expr, OP_MINUS)) return "operator-";
      if(node_has_simple_type(expr, OP_STAR)) return "operator*";
      if(node_has_simple_type(expr, OP_AMP)) return "operator&";
      if(node_has_simple_type(expr, OP_LNOT)) return "operator!";
      if(node_has_simple_type(expr, OP_COMPL)) return "operator~";
    }
    return std::string();
  }();
  if(overloaded_operator_name.empty()) {
    return false;
  }

  const auto try_synthetic_call =
      [&](const CppAstNode & synthetic_call,
          bool treat_first_operand_as_implicit_object) -> bool
      {
        ExprInfo analyzed;
        try {
          analyzed = ctx.analyze_call_expression(scope, synthetic_call);
        } catch(const std::logic_error &) {
          return false;
        }

        if(analyzed.node.kind != CallSemKind::call_expression ||
           analyzed.node.children.empty() ||
           analyzed.node.children[0].kind != CallSemKind::callee ||
           !analyzed.node.children[0].semantic_type) {
          return false;
        }

        const CallSemNode & resolved_callee = analyzed.node.children[0];
        FunctionBinding * binding =
            ctx.find_function_by_symbol(callsem_symbol(resolved_callee),
                                        resolved_callee.text,
                                        resolved_callee.semantic_type);
        if(!binding) {
          binding = ctx.find_exact_function(scope,
                                            resolved_callee.text,
                                            resolved_callee.semantic_type);
        }
        if(!binding || !binding->is_constexpr) {
          return false;
        }
        binding = ensure_constexpr_function_definition(ctx, binding, scope);
        if(!binding) {
          return false;
        }

        TypePtr function_type = strip_top_level_cv(binding->type);
        if(!function_type || function_type->kind != Type::TK_FUNCTION) {
          return false;
        }

        const std::size_t explicit_param_offset = binding->is_method ? 1u : 0u;
        std::vector<const CppAstNode *> explicit_arg_nodes;
        constant_eval::ConstexprValue implicit_object;
        if(expr.kind == CppAstKind::binary_expression) {
          if(treat_first_operand_as_implicit_object && binding->is_method) {
            constant_eval::ConstexprValue lhs;
            if(!evaluator.eval_expr(expr.children[0], lhs)) {
              return false;
            }
            implicit_object = lhs;
            explicit_arg_nodes.push_back(&expr.children[1]);
          } else {
            explicit_arg_nodes.push_back(&expr.children[0]);
            explicit_arg_nodes.push_back(&expr.children[1]);
          }
        } else {
          if(treat_first_operand_as_implicit_object && binding->is_method) {
            constant_eval::ConstexprValue operand;
            if(!evaluator.eval_expr(expr.children[0], operand)) {
              return false;
            }
            implicit_object = operand;
          } else {
            explicit_arg_nodes.push_back(&expr.children[0]);
          }
        }

        if(function_type->params.size() != explicit_arg_nodes.size() + explicit_param_offset) {
          return false;
        }

        std::vector<constant_eval::ConstexprValue> args;
        args.reserve(explicit_arg_nodes.size());
        for(std::size_t i = 0; i < explicit_arg_nodes.size(); ++i) {
          const TypePtr & target = function_type->params[i + explicit_param_offset];
          constant_eval::ConstexprValue value;
          if(target &&
             evaluate_typed_initializer_value(ctx,
                                              scope,
                                              evaluator,
                                              *explicit_arg_nodes[i],
                                              target,
                                              value)) {
            args.push_back(value);
            continue;
          }
          if(!evaluator.eval_expr(*explicit_arg_nodes[i], value)) {
            return false;
          }
          args.push_back(value);
        }

        Scope & call_scope = binding->declaration_scope ? *binding->declaration_scope : scope;
        Scope constexpr_call_scope = make_constexpr_call_scope(call_scope, binding);
        const constant_eval::Hooks call_hooks = build_hooks(ctx, constexpr_call_scope);

        constant_eval::FunctionInfo info;
        info.name = binding->name;
        info.return_type = function_type->inner;
        info.params.assign(binding->params.begin() + explicit_param_offset,
                           binding->params.end());
        info.body = constexpr_function_body(ctx, *binding);
        info.variadic = function_type->variadic || function_type->prototype_relaxed;
        info.is_method = binding->is_method;
        if(treat_first_operand_as_implicit_object && binding->is_method) {
          info.has_implicit_object = true;
          info.implicit_object = implicit_object;
        }
        return evaluator.call(info, args, out, &call_hooks);
      };

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = overloaded_operator_name;

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  arguments.children.push_back(expr.children[0]);
  if(expr.kind == CppAstKind::binary_expression) {
    arguments.children.push_back(expr.children[1]);
  }

  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.children.push_back(callee);
  call.children.push_back(arguments);
  if(try_synthetic_call(call, false)) {
    return true;
  }

  CppAstNode member_callee;
  member_callee.kind = CppAstKind::member_expression;
  member_callee.has_token = true;
  member_callee.token_kind = RT_SIMPLE;
  member_callee.simple_type = OP_DOT;
  member_callee.value = ".";
  member_callee.children.push_back(expr.children[0]);
  CppAstNode member_name;
  member_name.kind = CppAstKind::identifier;
  member_name.value = overloaded_operator_name;
  member_callee.children.push_back(member_name);

  CppAstNode member_args;
  member_args.kind = CppAstKind::paren_argument_list;
  if(expr.kind == CppAstKind::binary_expression) {
    member_args.children.push_back(expr.children[1]);
  }

  CppAstNode member_call;
  member_call.kind = CppAstKind::call_expression;
  member_call.children.push_back(member_callee);
  member_call.children.push_back(member_args);
  return try_synthetic_call(member_call, true);
}

bool constexpr_base_expression_is_lvalue(const CppAstNode & expr)
{
  const CppAstNode * current = &expr;
  while(current->kind == CppAstKind::parenthesized_expression &&
        current->children.size() == 1) {
    current = &current->children[0];
  }
  return current->kind == CppAstKind::id_expression ||
         current->kind == CppAstKind::member_expression ||
         current->kind == CppAstKind::subscript_expression;
}

bool evaluate_constexpr_member_call_expression(SemanticContext & ctx,
                                               Scope & scope,
                                               constant_eval::Evaluator & evaluator,
                                               const CppAstNode & expr,
                                               constant_eval::ConstexprValue & out)
{
  if(expr.kind != CppAstKind::call_expression || expr.children.empty()) {
    return false;
  }

  const CppAstNode * callee = &expr.children[0];
  while(callee->kind == CppAstKind::parenthesized_expression &&
        callee->children.size() == 1) {
    callee = &callee->children[0];
  }
  if(callee->kind != CppAstKind::member_expression ||
     callee->children.size() != 2 ||
     !node_has_simple_type(*callee, OP_DOT) ||
     (callee->children[1].kind != CppAstKind::identifier &&
      callee->children[1].kind != CppAstKind::id_expression)) {
    return false;
  }

  constant_eval::ConstexprValue implicit_object;
  if(!evaluator.eval_expr(callee->children[0], implicit_object)) {
    return false;
  }

  TypePtr object_type = remove_reference_type(implicit_object.type);
  const bool object_is_const =
      object_type && object_type->kind == Type::TK_CV && object_type->cv_const;
  TypePtr class_type = strip_top_level_cv(object_type);
  if(!class_type) {
    class_type = strip_top_level_cv(implicit_object.type);
  }
  ClassInfo * info = class_type ? ctx.class_info_for_type(class_type) : nullptr;
  if(!info) {
    return false;
  }

  const std::string member_name = callee->children[1].value;
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info->methods.find(member_name);
  if(found == info->methods.end() || found->second.empty()) {
    return false;
  }

  const CppAstNode * argument_list = find_child_kind(expr, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(expr, CppAstKind::paren_argument_list);
  }

  std::vector<constant_eval::ConstexprValue> arg_values;
  if(argument_list) {
    for(size_t i = 0; i < argument_list->children.size(); ++i) {
      constant_eval::ConstexprValue value;
      if(!evaluator.eval_expr(argument_list->children[i], value)) {
        return false;
      }
      arg_values.push_back(value);
    }
  }

  const bool base_is_lvalue = constexpr_base_expression_is_lvalue(callee->children[0]);
  FunctionBinding * selected = nullptr;
  TypePtr selected_function_type;
  const std::size_t explicit_param_offset = 1u;
  for(size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * candidate = found->second[i];
    if(!candidate || !candidate->is_method || !candidate->is_constexpr || !candidate->body) {
      continue;
    }
    if(object_is_const && !candidate->is_const_method) {
      continue;
    }
    if(candidate->ref_qualifier == RQ_LVALUE && !base_is_lvalue) {
      continue;
    }
    if(candidate->ref_qualifier == RQ_RVALUE && base_is_lvalue) {
      continue;
    }

    TypePtr function_type = strip_top_level_cv(candidate->type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION ||
       function_type->params.size() < explicit_param_offset) {
      continue;
    }

    std::size_t required_params = function_type->params.size();
    while(required_params > explicit_param_offset &&
          required_params - 1 < candidate->default_arguments.size() &&
          candidate->default_arguments[required_params - 1]) {
      --required_params;
    }
    const std::size_t total_args = function_type->params.size() - explicit_param_offset;
    const std::size_t required_args = required_params - explicit_param_offset;
    if(arg_values.size() < required_args || arg_values.size() > total_args) {
      continue;
    }

    bool matches = true;
    for(size_t arg_index = 0; arg_index < arg_values.size(); ++arg_index) {
      const std::size_t param_index = explicit_param_offset + arg_index;
      const TypePtr & param_type =
          param_index < candidate->params.size() ?
              candidate->params[param_index].second :
              function_type->params[param_index];
      constant_eval::ConstexprValue converted;
      if(!constant_eval::constexpr_value_cast(arg_values[arg_index], param_type, converted)) {
        matches = false;
        break;
      }
    }
    if(!matches) {
      continue;
    }
    if(selected) {
      return false;
    }
    selected = candidate;
    selected_function_type = function_type;
  }

  if(!selected || !selected_function_type) {
    return false;
  }

  selected = ensure_constexpr_function_definition(ctx, selected, scope);
  if(!selected) {
    return false;
  }
  selected_function_type = strip_top_level_cv(selected->type);
  if(!selected_function_type || selected_function_type->kind != Type::TK_FUNCTION) {
    return false;
  }

  Scope & call_scope = selected->declaration_scope ? *selected->declaration_scope : scope;
  Scope constexpr_default_arg_scope =
      make_constexpr_call_scope(call_scope, selected, false);
  const constant_eval::Hooks default_arg_hooks =
      build_hooks(ctx, constexpr_default_arg_scope);
  Scope constexpr_call_scope = make_constexpr_call_scope(call_scope, selected);
  const constant_eval::Hooks call_hooks = build_hooks(ctx, constexpr_call_scope);
  std::vector<constant_eval::ConstexprValue> final_args = arg_values;
  for(size_t param_index = final_args.size() + explicit_param_offset;
      param_index < selected->params.size();
      ++param_index) {
    if(param_index >= selected->default_arguments.size() ||
       !selected->default_arguments[param_index]) {
      return false;
    }
    const CppAstNode * default_arg = selected->default_arguments[param_index];
    const CppAstNode * payload =
        default_arg->children.size() == 1 ? &default_arg->children[0] : default_arg;
    constant_eval::ConstexprValue value;
    constant_eval::Evaluator default_evaluator(default_arg_hooks);
    if(!default_evaluator.eval_initializer(*payload, value, selected->params[param_index].second)) {
      return false;
    }
    final_args.push_back(value);
  }

  constant_eval::FunctionInfo info_out;
  info_out.name = selected->name;
  info_out.return_type = selected_function_type->inner;
  info_out.params.assign(selected->params.begin() + explicit_param_offset,
                         selected->params.end());
  info_out.body = constexpr_function_body(ctx, *selected);
  info_out.variadic =
      selected_function_type->variadic || selected_function_type->prototype_relaxed;
  info_out.is_method = true;
  info_out.has_implicit_object = true;
  info_out.implicit_object = implicit_object;
  return evaluator.call(info_out, final_args, out, &call_hooks);
}

bool evaluate_constexpr_target_conversion(SemanticContext & ctx,
                                          Scope & scope,
                                          constant_eval::Evaluator & evaluator,
                                          const CppAstNode & expr,
                                          const constant_eval::ConstexprValue & source_value,
                                          const TypePtr & target,
                                          constant_eval::ConstexprValue & out)
{
  semantic_conversion::ExprInfo analyzed;
  try
  {
    analyzed = ctx.analyze_expression(scope, expr);
  }
  catch(const std::logic_error &)
  {
    return false;
  }

  semantic_conversion::ExprInfo converted;
  semantic_conversion::ConversionRank rank = semantic_conversion::CR_BAD;
  const ArgumentConversionOptions conversion_options(
      true,
      true,
      false,
      false);
  if(!ctx.try_argument_conversion(scope,
                                  target,
                                  analyzed,
                                  converted,
                                  rank,
                                  conversion_options) ||
     converted.node.kind != CallSemKind::call_expression ||
     converted.node.children.empty() ||
     converted.node.children[0].kind != CallSemKind::callee ||
     !converted.node.children[0].semantic_type) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }

  const CallSemNode & resolved_callee = converted.node.children[0];
  FunctionBinding * binding =
      ctx.find_function_by_symbol(callsem_symbol(resolved_callee),
                                  resolved_callee.text,
                                  resolved_callee.semantic_type);
  if(!binding) {
    binding = ctx.find_exact_function(scope,
                                      resolved_callee.text,
                                      resolved_callee.semantic_type);
  }
  if(!binding || !binding->is_constexpr) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }
  binding = ensure_constexpr_function_definition(ctx, binding, scope);
  if(!binding) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }

  TypePtr function_type = strip_top_level_cv(binding->type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }

  const std::size_t explicit_param_offset = binding->is_method ? 1u : 0u;
  if(function_type->params.size() != explicit_param_offset ||
     converted.node.children.size() != 1 + explicit_param_offset) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }

  Scope & call_scope = binding->declaration_scope ? *binding->declaration_scope : scope;
  Scope constexpr_call_scope = make_constexpr_call_scope(call_scope, binding);
  const constant_eval::Hooks call_hooks = build_hooks(ctx, constexpr_call_scope);

  constant_eval::FunctionInfo info;
  info.name = binding->name;
  info.return_type = function_type->inner;
  info.params.assign(binding->params.begin() + explicit_param_offset,
                     binding->params.end());
  info.body = constexpr_function_body(ctx, *binding);
  info.variadic = function_type->variadic || function_type->prototype_relaxed;
  info.is_method = binding->is_method;
  if(binding->is_method) {
    info.has_implicit_object = true;
    info.implicit_object = source_value;
  }

  constant_eval::ConstexprValue converted_value;
  if(!evaluator.call(info,
                     std::vector<constant_eval::ConstexprValue>(),
                     converted_value,
                     &call_hooks)) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }

  if(!constant_eval::constexpr_value_cast(converted_value, target, out)) {
    return evaluate_constexpr_value_member_conversion(ctx, scope, expr, target, out);
  }
  out.type = target;
  return true;
}

bool evaluate_constexpr_value_member_conversion(SemanticContext & ctx,
                                                Scope & scope,
                                                const CppAstNode & expr,
                                                const TypePtr & target,
                                                constant_eval::ConstexprValue & out)
{
  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(!target_base) {
    return false;
  }
  const bool scalar_target =
      is_integral_type(target_base) ||
      is_named_enum_type(ctx, target_base);
  if(!scalar_target || expr.kind != CppAstKind::call_expression) {
    return false;
  }

  const CppAstNode * callee = &expr.children[0];
  if(callee->kind == CppAstKind::parenthesized_expression &&
     callee->children.size() == 1) {
    callee = &callee->children[0];
  }
  if(callee->kind != CppAstKind::id_expression) {
    return false;
  }

  const CppAstNode * argument_list = find_child_kind(expr, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(expr, CppAstKind::paren_argument_list);
  }
  if(argument_list) {
    if(argument_list->children.empty()) {
      // ok
    } else if(argument_list->children.size() == 1 &&
              argument_list->children[0].kind == CppAstKind::braced_init_list &&
              argument_list->children[0].children.empty()) {
      // ok
    } else {
      return false;
    }
  }

  const std::string callee_text = semantic_utils::trim_space(callee->value);
  if(callee_text.empty()) {
    return false;
  }

  constant_eval::ConstexprValue member_value;
  const TemplateIdSyntax * callee_template_id = cppast_template_id_syntax(*callee);
  bool found_member_value = false;
  if(callee_template_id) {
    found_member_value =
        ctx.lookup_constant_template_member_value(scope,
                                                  *callee_template_id,
                                                  "value",
                                                  callee_text + "::value",
                                                  member_value);
  }
  if(!found_member_value) {
    TypePtr callee_type = ctx.lookup_type_node(scope, *callee, callee_text, true);
    if(!callee_type &&
       callee->qualifier_template_id_syntaxes.empty() &&
       callee->qualifier_type_syntaxes.empty()) {
      callee_type = ctx.lookup_type(scope, callee_text, true);
    }
    if(callee_type) {
      found_member_value =
          ctx.lookup_constant_value(scope, callee_text + "::value", member_value);
    }
  }
  if(!found_member_value ||
     !constant_eval::constexpr_value_cast(member_value, target, out)) {
    return false;
  }
  note_constexpr_value_member_conversion_operator(ctx,
                                                  scope,
                                                  *callee,
                                                  callee_text,
                                                  target_base);
  out.type = target;
  return true;
}

bool evaluate_constexpr_function_address_expression(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & expr,
    constant_eval::ConstexprValue & out)
{
  if(expr.kind != CppAstKind::unary_expression ||
     expr.children.size() != 1 ||
     !node_has_simple_type(expr, OP_AMP)) {
    return false;
  }

  const CppAstNode * operand = &expr.children[0];
  if(operand->kind == CppAstKind::parenthesized_expression &&
     operand->children.size() == 1) {
    operand = &operand->children[0];
  }
  if(operand->kind != CppAstKind::id_expression) {
    return false;
  }

  std::vector<FunctionBinding *> functions;
  const TemplateIdSyntax * template_id = cppast_template_id_syntax(*operand);
  const QualifiedName * qualified = cppast_qualified_name_syntax(*operand);
  if(template_id) {
    functions = ctx.lookup_function_template_id_node(
        scope,
        *operand,
        *template_id,
        semantic_policy::without_body_instantiation());
  } else if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    functions = ctx.lookup_qualified_functions(scope, *qualified);
  } else {
    functions = ctx.lookup_functions(scope,
                                     operand->value,
                                     semantic_policy::without_body_instantiation());
  }
  if(functions.size() != 1 || !functions[0]) {
    return false;
  }

  FunctionBinding * function = functions[0];
  if(function->is_method) {
    return false;
  }
  TypePtr function_type = strip_top_level_cv(function->type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return false;
  }

  std::string identity = function->symbol.object_symbol;
  if(identity.empty()) {
    identity = function->symbol.internal_symbol;
  }
  if(identity.empty()) {
    identity = function_binding_qualified_name_for_symbol(*function);
  }
  out = constant_eval::make_pointer_value(make_pointer(function_type), identity, 0);
  return true;
}

bool evaluate_default_special_expression(SemanticContext & ctx,
                                         Scope & scope,
                                         constant_eval::Evaluator & evaluator,
                                         const CppAstNode & expr,
                                         constant_eval::ConstexprValue & value)
{
  if(expr.kind == CppAstKind::call_expression) {
    CppAstNode expanded_call = expr;
    CppAstNode * argument_list =
        find_child_kind_mutable(expanded_call, CppAstKind::argument_list);
    if(!argument_list) {
      argument_list =
          find_child_kind_mutable(expanded_call, CppAstKind::paren_argument_list);
    }
    bool changed = false;
    if(argument_list) {
      std::vector<CppAstNode> expanded_args;
      expanded_args.reserve(argument_list->children.size());
      for(size_t i = 0; i < argument_list->children.size(); ++i) {
        const CppAstNode & argument = argument_list->children[i];
        if(argument.kind != CppAstKind::pack_expansion_expression) {
          expanded_args.push_back(argument);
          continue;
        }
        std::vector<CppAstNode> expanded_nodes;
        if(!ctx.expand_pack_argument_node(scope, argument, expanded_nodes)) {
          return false;
        }
        expanded_args.insert(expanded_args.end(),
                             expanded_nodes.begin(),
                             expanded_nodes.end());
        changed = true;
      }
      if(changed) {
        argument_list->children.swap(expanded_args);
        return evaluator.eval_expr(expanded_call, value);
      }
    }
  }

  if(expr.kind == CppAstKind::fold_expression) {
    CppAstNode reduced;
    if(reduce_fold_expression_node(ctx, scope, expr, reduced)) {
      return evaluator.eval_expr(reduced, value);
    }
    if(scope_has_template_placeholders(ctx, scope)) {
      return false;
    }
  }

  if(evaluate_builtin_or_type_trait_expression(ctx, scope, expr, value)) {
    return true;
  }

  if(evaluate_constexpr_function_address_expression(ctx, scope, expr, value)) {
    return true;
  }

  if(evaluate_constexpr_overloaded_operator_expression(ctx, scope, evaluator, expr, value)) {
    return true;
  }

  if(evaluate_constexpr_member_call_expression(ctx, scope, evaluator, expr, value)) {
    return true;
  }

  if(expr.kind == CppAstKind::member_expression && expr.children.size() == 2 &&
     (expr.children[1].kind == CppAstKind::id_expression ||
      expr.children[1].kind == CppAstKind::identifier)) {
    constant_eval::ConstexprValue base;
    if(expr.children[0].kind == CppAstKind::id_expression &&
       expr.children[0].value == "this") {
      if(!evaluator.current_this_object(base)) {
        return false;
      }
    } else if(!evaluator.eval_expr(expr.children[0], base)) {
      return false;
    }
    return constant_eval::aggregate_member_value(base, expr.children[1].value, value);
  }

  if(expr.kind == CppAstKind::subscript_expression && expr.children.size() == 2) {
    constant_eval::ConstexprValue base;
    constant_eval::ConstexprValue index;
    if(!evaluator.eval_expr(expr.children[0], base) ||
       !evaluator.eval_expr(expr.children[1], index)) {
      return false;
    }
    long long integral_index = 0;
    if(!constant_eval::constexpr_value_to_integral(index, integral_index) || integral_index < 0) {
      return false;
    }
    return constant_eval::array_element_value(base,
                                              static_cast<size_t>(integral_index),
                                              value);
  }

  return false;
}

}  // namespace

Scope make_constexpr_call_scope(Scope & parent,
                                FunctionBinding * binding,
                                bool bind_parameters)
{
  return make_constexpr_call_scope_impl(parent, binding, bind_parameters);
}

std::vector<std::pair<std::string, TypePtr> >
constexpr_function_parameters(FunctionBinding & binding)
{
  return constexpr_function_parameters_impl(binding);
}

constant_eval::ConstexprValue make_constexpr_string_literal_value(
    const std::string & text,
    const std::string & storage_identity)
{
  const TypePtr char_type = make_fundamental(FT_CHAR);
  const TypePtr const_char_type = make_cv(char_type, true, false);
  std::vector<constant_eval::ConstexprValue> elements;
  elements.reserve(text.size() + 1);
  for(std::size_t i = 0; i < text.size(); ++i) {
    elements.push_back(
        constant_eval::make_integral_value(
            static_cast<unsigned char>(text[i]), char_type));
  }
  elements.push_back(constant_eval::make_integral_value(0, char_type));
  return constant_eval::make_array_value(
      make_array(const_char_type, true, elements.size()),
      elements,
      storage_identity);
}

constant_eval::Hooks build_hooks(SemanticContext & ctx,
                                 Scope & scope)
{
  constant_eval::Hooks hooks;
  hooks.lookup_external_value =
      [&ctx, &scope](const std::string & name,
                     const CppAstNode * node,
                     constant_eval::ConstexprValue & value)
      {
        if(name == "__func__" ||
           name == "__FUNCTION__" ||
           name == "__PRETTY_FUNCTION__") {
          if(FunctionBinding * current_function = current_function_scope(scope)) {
            const std::string text =
                name == "__PRETTY_FUNCTION__" ?
                    semantic_model::predefined_pretty_function_text(*current_function) :
                    function_binding_display_name_for_symbol(*current_function);
            value = make_constexpr_string_literal_value(text, name);
            return true;
          }
        }

        std::string source_anchor = name;
        std::vector<std::string> lookup_arg_texts;
        const auto compact_template_text =
            [](const std::string & text) -> std::string
        {
          std::string out;
          out.reserve(text.size());
          for(std::size_t i = 0; i < text.size(); ++i) {
            if(!std::isspace(static_cast<unsigned char>(text[i]))) {
              out.push_back(text[i]);
            }
          }
          return out;
        };
        const TemplateIdSyntax * lookup_template_id =
            (node != nullptr && node->kind == CppAstKind::id_expression) ?
                cppast_template_id_syntax(*node) :
                nullptr;
        const TemplateIdSyntax * lookup_qualifier_template_id = nullptr;
        std::string lookup_template_member_name;
        if(node != nullptr &&
           node->kind == CppAstKind::id_expression &&
           !node->qualifier_template_id_syntaxes.empty()) {
          QualifiedName structured_lookup_name;
          if(semantic_utils::split_qualified_name_text(name, structured_lookup_name) &&
             !structured_lookup_name.qualifiers.empty() &&
             !structured_lookup_name.name.empty()) {
            lookup_qualifier_template_id =
                &node->qualifier_template_id_syntaxes.back();
            lookup_template_member_name = structured_lookup_name.name;
          }
        }
        const bool lookup_is_template_id = lookup_template_id != nullptr;
        const bool lookup_qualifier_template_compatible =
            lookup_qualifier_template_id != nullptr &&
            node != nullptr &&
            node->kind == CppAstKind::id_expression &&
            compact_template_text(node->value) == compact_template_text(name);
        if(lookup_template_id) {
          source_anchor = lookup_template_id->name.name;
          lookup_arg_texts = lookup_template_id->arguments;
        } else if(lookup_qualifier_template_compatible &&
                  lookup_qualifier_template_id != nullptr) {
          source_anchor = lookup_qualifier_template_id->name.name;
          lookup_arg_texts = lookup_qualifier_template_id->arguments;
        }
        const bool source_node_compatible =
            !lookup_is_template_id ||
            node == nullptr ||
            node->kind != CppAstKind::id_expression ||
            compact_template_text(node->value) == compact_template_text(name);
        std::string use_location;
        if(source_node_compatible && node != nullptr) {
          use_location = ctx.source_location_for_name_in_node(*node, source_anchor);
          if(!semantic_trace::source_location_points_at_identifier(use_location,
                                                                   source_anchor)) {
            use_location.clear();
          }
        }
        if(use_location.empty() && source_node_compatible && node != nullptr) {
          const std::string node_location = ctx.source_location_for_node(*node);
          if(semantic_trace::source_location_points_at_identifier(node_location,
                                                                  source_anchor)) {
            use_location = node_location;
          }
        }
        if(use_location.empty() && source_node_compatible) {
          const std::string trace_location = parser_trace::current_use_location();
          if(semantic_trace::source_location_points_at_identifier(trace_location,
                                                                  source_anchor)) {
            use_location = trace_location;
          }
        }
        const template_api::ScopedTemplateIdSourceArguments source_arg_guard(
            (lookup_is_template_id || lookup_qualifier_template_compatible) ?
                use_location :
                std::string(),
            source_anchor,
            (lookup_is_template_id || lookup_qualifier_template_compatible) ?
                std::move(lookup_arg_texts) :
                std::vector<std::string>());
        const ScopedSuppressedTemplateUseLocation suppress_incompatible_use_location(
            lookup_is_template_id && !source_node_compatible);
        const ScopedTemplateUseLocation use_location_guard(use_location);
        const parser_trace::ScopedOrderUseLocation order_use_location_guard(
            use_location);
        if(lookup_template_id &&
           source_node_compatible &&
           (node == nullptr ||
            (node->qualifier_template_id_syntaxes.empty() &&
             node->qualifier_type_syntaxes.empty()))) {
          return ctx.lookup_constant_template_id_value(scope,
                                                       *lookup_template_id,
                                                       name,
                                                       value);
        }
        if(lookup_qualifier_template_compatible) {
          const bool prefer_structured_qualified_lookup =
              node != nullptr &&
              node->kind == CppAstKind::id_expression &&
              node->qualifier_template_id_syntaxes.size() > 1;
          if(prefer_structured_qualified_lookup &&
             ctx.lookup_constant_value_node(scope, name, node, value)) {
            return true;
          }
          if(ctx.lookup_constant_template_member_value(
                 scope,
                 *lookup_qualifier_template_id,
                 lookup_template_member_name,
                 name,
                 value)) {
            return true;
          }
          if(!prefer_structured_qualified_lookup &&
             ctx.lookup_constant_value_node(scope, name, node, value)) {
            return true;
          }
        }
        return ctx.lookup_constant_value_node(scope, name, node, value);
      };
  hooks.lookup_type = [&ctx, &scope](const std::string & name)
  {
    return ctx.lookup_type(scope, name, false);
  };
  hooks.lookup_type_node = [&ctx, &scope](const CppAstNode & node)
  {
    return ctx.lookup_type_node(scope, node, node.value, false);
  };
  hooks.parse_type_id = [&ctx, &scope](const CppAstNode & type_id, TypePtr & type)
  {
    if(!ctx.parse_type_id(scope, type_id, type)) {
      return false;
    }
    TypePtr base = strip_top_level_cv(remove_reference_type(type));
    if(base && base->kind == Type::TK_NAMED && !base->named_has_layout) {
      ClassInfo * info = ctx.complete_class_type(base);
      if(info && info->type) {
        base->named_complete = info->type->named_complete;
        base->named_has_layout = info->type->named_has_layout;
        base->named_alignment = info->type->named_alignment;
        base->named_size = info->type->named_size;
        base->named_is_empty = info->type->named_is_empty;
        base->named_host_abi_chunks = info->type->named_host_abi_chunks;
        base->named_lambda_mangle = info->type->named_lambda_mangle;
        base->named_class_template_specialization_mangle_info =
            info->type->named_class_template_specialization_mangle_info;
      }
    }
    return true;
  };
  hooks.evaluate_sizeof_operand = [&ctx, &scope](const CppAstNode & expr, std::size_t & size)
  {
    return ctx.evaluate_sizeof_operand_for_consteval(scope, expr, size);
  };
  hooks.lookup_pack_size = [&ctx, &scope](const std::string & name, std::size_t & pack_size)
  {
    return ctx.lookup_pack_size(scope, name, pack_size);
  };
  hooks.evaluate_special_expression =
      [&ctx, &scope](constant_eval::Evaluator & evaluator,
                     const CppAstNode & expr,
                     constant_eval::ConstexprValue & out)
      {
        return evaluate_default_special_expression(ctx, scope, evaluator, expr, out);
      };
  hooks.evaluate_typed_initializer =
      [&ctx, &scope](constant_eval::Evaluator & evaluator,
                     const CppAstNode & node,
                     const TypePtr & target,
                     constant_eval::ConstexprValue & out)
      {
        return evaluate_typed_initializer_value(ctx, scope, evaluator, node, target, out);
      };
  hooks.parse_local_declaration =
      [&ctx, &scope](const CppAstNode & decl,
                     std::vector<constant_eval::LocalDeclaration> & locals,
                     std::string & error)
      {
        return ctx.parse_constexpr_local_declaration(scope, decl, locals, error);
      };
  hooks.process_semantic_declaration =
      [&ctx, &scope](const CppAstNode & decl, std::string & error)
      {
        try {
          if(decl.kind == CppAstKind::using_directive) {
            semantic_declaration::collect_using_directive(ctx, scope, decl);
            return true;
          }
          if(decl.kind == CppAstKind::using_declaration) {
            semantic_declaration::collect_using_declaration(ctx, scope, decl);
            return true;
          }
          if(decl.kind == CppAstKind::namespace_alias_definition) {
            semantic_declaration::collect_namespace_alias_definition(ctx, scope, decl);
            return true;
          }
        } catch(const std::logic_error & ex) {
          error = ex.what();
          return false;
        }
        error = "unsupported constexpr semantic declaration";
        return false;
      };
  hooks.evaluate_call =
      [&ctx, &scope](constant_eval::Evaluator & evaluator,
                     const CppAstNode & call,
                     const std::vector<constant_eval::ConstexprValue> & args,
                     constant_eval::ConstexprValue & value)
      {
        return ctx.evaluate_constant_call_expression_value(scope, evaluator, call, args, value);
      };
  return hooks;
}

bool evaluate_expression_value(SemanticContext & ctx,
                               Scope & scope,
                               const CppAstNode & node,
                               constant_eval::ConstexprValue & out)
{
  constant_eval::Evaluator evaluator(build_hooks(ctx, scope));
  return evaluator.eval_expr(node, out);
}

bool evaluate_expression_integral(SemanticContext & ctx,
                                  Scope & scope,
                                  const CppAstNode & node,
                                  long long & out)
{
  constant_eval::ConstexprValue value;
  return evaluate_expression_value(ctx, scope, node, value) &&
         constant_eval::constexpr_value_to_integral(value, out);
}

bool evaluate_initializer_value(SemanticContext & ctx,
                                Scope & scope,
                                const CppAstNode & initializer,
                                constant_eval::ConstexprValue & out)
{
  constant_eval::Evaluator evaluator(build_hooks(ctx, scope));
  return evaluator.eval_initializer(initializer, out);
}

bool evaluate_initializer_value(SemanticContext & ctx,
                                Scope & scope,
                                const CppAstNode & initializer,
                                const TypePtr & target,
                                constant_eval::ConstexprValue & out)
{
  constant_eval::Evaluator evaluator(build_hooks(ctx, scope));
  return evaluator.eval_initializer(initializer, out, target);
}

bool evaluate_default_initialized_value(SemanticContext & ctx,
                                        Scope & scope,
                                        const TypePtr & target,
                                        constant_eval::ConstexprValue & out)
{
  constant_eval::Evaluator evaluator(build_hooks(ctx, scope));
  return evaluate_default_initialized_type(ctx, scope, evaluator, target, out);
}

bool evaluate_initializer_integral(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & initializer,
                                   long long & out)
{
  constant_eval::ConstexprValue value;
  return evaluate_initializer_value(ctx, scope, initializer, value) &&
         constant_eval::constexpr_value_to_integral(value, out);
}

}  // namespace semantic_consteval
