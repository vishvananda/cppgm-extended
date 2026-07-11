#include "template_decl_ast.h"

#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "callsemantic_internal.h"
#include "constant_value.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "parser_trace.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_utils.h"
#include "template_argument_semantics.h"
#include "template_scope.h"
#include "template_services.h"

namespace template_decl_ast {

using namespace cpp_decl;
using namespace semantic_model;

namespace {

// template-boundary-audit: begin semantic_service_access
template_api::TemplateTypeSystem & service_type_system(
    template_api::TemplateServices & services)
{
  return services.type_system;
}

bool service_evaluate_initializer_constant_value(
    template_api::TemplateServices & services,
    const template_api::TemplateConstantEvaluationRequest & request,
    constant_eval::ConstexprValue & out)
{
  return services.recursive_semantic.evaluate_initializer_constant_value(request, out);
}
// template-boundary-audit: end semantic_service_access

// template-boundary-audit: begin text_recovery_bridge
std::string format_pack_type_argument_text(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(
      type_system,
      type);
}

bool resolve_decl_ast_direct_type_lookup(
    template_api::TemplateServices & services,
    Scope & scope,
    const std::string & text,
    const QualifiedName * qualified_name,
    bool reference_class_templates_only,
    TypePtr & out)
{
  out.reset();
  std::string candidate = semantic_utils::trim_space(text);
  if(candidate.find('<') != std::string::npos) {
    return false;
  }
  candidate = semantic_utils::strip_elaborated_type_prefix(candidate);
  if(candidate.empty()) {
    return false;
  }

  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.allow_class_templates = reference_class_templates_only;
  if(qualified_name) {
    request.name = *qualified_name;
  } else {
    if(!callsemantic_internal::is_identifier_text(candidate)) {
      return false;
    }
    request.name = QualifiedName();
    request.name.name = candidate;
  }
  return service_type_system(services).resolve_direct_type_lookup(request, out) && out;
}

// template-boundary-audit: end text_recovery_bridge

bool declarator_has_parameter_pack(const CppAstNode & declarator)
{
  for(std::size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::parameter_pack) {
      return true;
    }
  }
  const CppAstNode * nested = cpp_decl::find_child(declarator, CppAstKind::nested_declarator);
  return nested && !nested->children.empty() && declarator_has_parameter_pack(nested->children[0]);
}

bool declarator_has_parameter_clause(const CppAstNode & declarator)
{
  for(std::size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::parameter_clause) {
      return true;
    }
  }
  const CppAstNode * nested = cpp_decl::find_child(declarator, CppAstKind::nested_declarator);
  return nested && !nested->children.empty() &&
         declarator_has_parameter_clause(nested->children[0]);
}

bool erase_parameter_pack_nodes(CppAstNode & current)
{
  bool removed = false;
  std::vector<CppAstNode> kept;
  kept.reserve(current.children.size());
  for(std::size_t i = 0; i < current.children.size(); ++i) {
    if(current.children[i].kind == CppAstKind::parameter_pack) {
      removed = true;
      continue;
    }
    removed = erase_parameter_pack_nodes(current.children[i]) || removed;
    kept.push_back(current.children[i]);
  }
  current.children.swap(kept);
  return removed;
}

bool node_has_parameter_pack_node(const CppAstNode & node)
{
  if(node.kind == CppAstKind::parameter_pack) {
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(node_has_parameter_pack_node(node.children[i])) {
      return true;
    }
  }
  return false;
}

std::string qualified_name_syntax_text(const QualifiedName & name)
{
  std::string out = name.rooted ? "::" : std::string();
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    out += name.qualifiers[i];
    out += "::";
  }
  out += name.name;
  return out;
}

std::string template_id_syntax_text(const TemplateIdSyntax & syntax)
{
  std::ostringstream out;
  out << qualified_name_syntax_text(syntax.name) << "<";
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    if(i < syntax.argument_syntaxes.size() &&
       !syntax.argument_syntaxes[i].text.empty()) {
      out << semantic_utils::trim_space(syntax.argument_syntaxes[i].text);
    } else {
      out << semantic_utils::trim_space(syntax.arguments[i]);
    }
  }
  out << ">";
  return out.str();
}

bool ast_node_value_names_pack_identifier(const CppAstNode & node,
                                          const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  switch(node.kind) {
  case CppAstKind::decl_specifier:
  case CppAstKind::type_specifier:
  case CppAstKind::type_name:
  case CppAstKind::id_expression:
    break;
  default:
    return false;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    return false;
  }

  std::string value = semantic_utils::trim_space(node.value);
  if(node.kind == CppAstKind::decl_specifier ||
     node.kind == CppAstKind::type_specifier ||
     node.kind == CppAstKind::type_name ||
     value.compare(0, 9, "typename ") == 0) {
    value = semantic_utils::trim_space(
        semantic_utils::strip_elaborated_type_prefix(value));
    if(value.compare(0, 9, "typename ") == 0) {
      value = semantic_utils::trim_space(value.substr(9));
    }
  }
  return value == name && value.find("::") == std::string::npos;
}

bool template_id_syntax_mentions_pack_identifier(const TemplateIdSyntax & syntax,
                                                 const std::string & name)
{
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(callsemantic_internal::contains_identifier_token(syntax.arguments[i], name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(callsemantic_internal::contains_identifier_token(
           syntax.argument_syntaxes[i].text, name)) {
      return true;
    }
    if(syntax.argument_syntaxes[i].template_id &&
       template_id_syntax_mentions_pack_identifier(
           *syntax.argument_syntaxes[i].template_id,
           name)) {
      return true;
    }
    if(syntax.argument_syntaxes[i].type_id &&
       ast_node_value_names_pack_identifier(*syntax.argument_syntaxes[i].type_id,
                                            name)) {
      return true;
    }
  }
  return false;
}

bool ast_node_mentions_pack_identifier(const CppAstNode & node,
                                       const std::string & name)
{
  if(ast_node_value_names_pack_identifier(node, name)) {
    return true;
  }
  if(node.template_id_syntax &&
     template_id_syntax_mentions_pack_identifier(*node.template_id_syntax, name)) {
    return true;
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_pack_identifier(
           node.qualifier_template_id_syntaxes[i],
           name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_mentions_pack_identifier(node.children[i], name)) {
      return true;
    }
  }
  return false;
}

CppAstNode make_pack_type_id_node(template_api::TemplateTypeSystem & type_system,
                                  const TypePtr & type)
{
  const std::string text = format_pack_type_argument_text(type_system, type);
  CppAstNode type_id;
  type_id.kind = CppAstKind::type_id;
  type_id.value = text;

  CppAstNode specifiers;
  specifiers.kind = CppAstKind::type_specifier_seq;
  specifiers.value = text;

  CppAstNode type_name;
  type_name.kind = CppAstKind::type_name;
  type_name.value = text;
  type_name.semantic_type = type;

  specifiers.children.push_back(type_name);
  type_id.children.push_back(specifiers);
  return type_id;
}

CppAstNode clone_pack_substitution_node(const CppAstNode & source);
TemplateIdSyntax clone_template_id_syntax(const TemplateIdSyntax & source);

TemplateArgumentSyntax clone_template_argument_syntax(
    const TemplateArgumentSyntax & source)
{
  TemplateArgumentSyntax out;
  out.text = source.text;
  out.source_text = source.source_text;
  out.pack_expansion = source.pack_expansion;
  out.dependent = source.dependent;
  out.has_source_token_start = source.has_source_token_start;
  out.source_token_start = source.source_token_start;
  out.source_location_id = source.source_location_id;
  out.resolved_type = source.resolved_type;
  if(source.template_id) {
    out.template_id.reset(new TemplateIdSyntax(
        clone_template_id_syntax(*source.template_id)));
  }
  if(source.type_id) {
    out.type_id.reset(new CppAstNode(clone_pack_substitution_node(*source.type_id)));
  }
  if(source.source_type_id) {
    out.source_type_id = source.source_type_id;
  }
  if(source.expression) {
    out.expression.reset(new CppAstNode(clone_pack_substitution_node(*source.expression)));
  }
  return out;
}

TemplateIdSyntax clone_template_id_syntax(const TemplateIdSyntax & source)
{
  TemplateIdSyntax out;
  out.name = source.name;
  out.source_location_id = source.source_location_id;
  out.arguments = source.arguments;
  out.argument_syntaxes.reserve(source.argument_syntaxes.size());
  for(std::size_t i = 0; i < source.argument_syntaxes.size(); ++i) {
    out.argument_syntaxes.push_back(
        clone_template_argument_syntax(source.argument_syntaxes[i]));
  }
  return out;
}

CppAstNode clone_pack_substitution_node(const CppAstNode & source)
{
  CppAstNode out = source;
  if(source.qualified_name_syntax) {
    out.qualified_name_syntax.reset(new QualifiedName(*source.qualified_name_syntax));
  }
  if(source.template_id_syntax) {
    out.template_id_syntax.reset(new TemplateIdSyntax(
        clone_template_id_syntax(*source.template_id_syntax)));
  }
  out.qualifier_template_id_syntaxes.clear();
  out.qualifier_template_id_syntaxes.reserve(
      source.qualifier_template_id_syntaxes.size());
  for(std::size_t i = 0; i < source.qualifier_template_id_syntaxes.size(); ++i) {
    out.qualifier_template_id_syntaxes.push_back(
        clone_template_id_syntax(source.qualifier_template_id_syntaxes[i]));
  }
  out.qualifier_type_syntaxes.clear();
  out.qualifier_type_syntaxes.reserve(source.qualifier_type_syntaxes.size());
  for(std::size_t i = 0; i < source.qualifier_type_syntaxes.size(); ++i) {
    out.qualifier_type_syntaxes.push_back(
        clone_pack_substitution_node(source.qualifier_type_syntaxes[i]));
  }
  out.children.clear();
  out.children.reserve(source.children.size());
  for(std::size_t i = 0; i < source.children.size(); ++i) {
    out.children.push_back(clone_pack_substitution_node(source.children[i]));
  }
  return out;
}

void rewrite_template_id_syntax_type_pack_arguments(
    template_api::TemplateTypeSystem & type_system,
    TemplateIdSyntax & syntax,
    const std::map<std::string, TypePtr> & type_replacements)
{
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    bool changed = false;
    std::string text = syntax.arguments[i];
    for(auto it = type_replacements.begin();
        it != type_replacements.end();
        ++it) {
      text = callsemantic_internal::replace_identifier_token_text(
          text,
          it->first,
          format_pack_type_argument_text(type_system, it->second),
          changed);
    }
    if(changed) {
      syntax.arguments[i] = text;
    }
  }

  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    TemplateArgumentSyntax & argument = syntax.argument_syntaxes[i];
    bool changed = false;
    for(auto it = type_replacements.begin();
        it != type_replacements.end();
        ++it) {
      argument.text = callsemantic_internal::replace_identifier_token_text(
          argument.text,
          it->first,
          format_pack_type_argument_text(type_system, it->second),
          changed);
      if(argument.type_id &&
         ast_node_mentions_pack_identifier(*argument.type_id, it->first)) {
        argument.type_id.reset(new CppAstNode(
            make_pack_type_id_node(type_system, it->second)));
        argument.text = format_pack_type_argument_text(type_system, it->second);
      }
    }
    if(argument.template_id) {
      rewrite_template_id_syntax_type_pack_arguments(
          type_system,
          *argument.template_id,
          type_replacements);
      argument.text = template_id_syntax_text(*argument.template_id);
    }
  }
}

bool substitute_type_pack_node_ast(
    template_api::TemplateTypeSystem & type_system,
    const CppAstNode & node,
    const std::map<std::string, TypePtr> & type_replacements,
    CppAstNode & out)
{
  out = clone_pack_substitution_node(node);
  if(out.template_id_syntax) {
    rewrite_template_id_syntax_type_pack_arguments(
        type_system,
        *out.template_id_syntax,
        type_replacements);
    out.value = template_id_syntax_text(*out.template_id_syntax);
  }
  for(std::size_t i = 0; i < out.qualifier_template_id_syntaxes.size(); ++i) {
    rewrite_template_id_syntax_type_pack_arguments(
        type_system,
        out.qualifier_template_id_syntaxes[i],
        type_replacements);
  }

  for(auto it = type_replacements.begin();
      it != type_replacements.end();
      ++it) {
    if(ast_node_value_names_pack_identifier(out, it->first)) {
      out.semantic_type = it->second;
      out.value = format_pack_type_argument_text(type_system, it->second);
      out.qualified_name_syntax.reset();
      out.template_id_syntax.reset();
      out.qualifier_template_id_syntaxes.clear();
      out.qualifier_type_syntaxes.clear();
      break;
    }
  }

  std::vector<CppAstNode> children;
  children.reserve(node.children.size());
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    CppAstNode child;
    if(!substitute_type_pack_node_ast(type_system,
                                      node.children[i],
                                      type_replacements,
                                      child)) {
      return false;
    }
    children.push_back(child);
  }
  out.children.swap(children);
  return true;
}

bool expand_parameter_declaration_pack_nodes(
    template_api::TemplateTypeSystem & type_system,
    semantic_model::Scope & scope,
    const CppAstNode & parameter,
    std::vector<CppAstNode> & expanded_parameters)
{
  expanded_parameters.clear();
  if(parameter.kind != CppAstKind::parameter_declaration) {
    return false;
  }

  const CppAstNode * declarator = cpp_decl::find_child(parameter, CppAstKind::declarator);
  if(!declarator) {
    declarator = cpp_decl::find_child(parameter, CppAstKind::abstract_declarator);
  }
  if(!(node_has_parameter_pack_node(parameter) ||
       (declarator && declarator_has_parameter_pack(*declarator)))) {
    return false;
  }

  CppAstNode stripped = parameter;
  if(!erase_parameter_pack_nodes(stripped)) {
    return false;
  }

  std::vector<std::pair<std::string, const std::vector<TypePtr> *> > packs;
  std::set<std::string> seen_pack_names;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(std::map<std::string, std::vector<TypePtr> >::const_iterator pack =
            current->named_type_packs.begin();
        pack != current->named_type_packs.end();
        ++pack) {
      const std::string & pack_name = pack->first;
      if(pack_name.empty() ||
         seen_pack_names.count(pack_name) != 0 ||
         !ast_node_mentions_pack_identifier(stripped, pack_name)) {
        continue;
      }
      seen_pack_names.insert(pack_name);
      packs.push_back(std::make_pair(pack_name, &pack->second));
    }
  }

  if(packs.empty()) {
    return false;
  }

  const std::size_t pack_size = packs[0].second->size();
  for(std::size_t i = 1; i < packs.size(); ++i) {
    if(packs[i].second->size() != pack_size) {
      return false;
    }
  }

  for(std::size_t i = 0; i < pack_size; ++i) {
    std::map<std::string, TypePtr> type_replacements;
    for(std::size_t j = 0; j < packs.size(); ++j) {
      type_replacements[packs[j].first] = (*(packs[j].second))[i];
    }
    CppAstNode expanded;
    if(!substitute_type_pack_node_ast(type_system,
                                      stripped,
                                      type_replacements,
                                      expanded)) {
      return false;
    }
    expanded_parameters.push_back(expanded);
  }
  return true;
}

bool expand_parameter_clause_pack_patterns_impl(
    template_api::TemplateTypeSystem & type_system,
    semantic_model::Scope & semantic_scope,
    const CppAstNode & node,
    CppAstNode & expanded_clause,
    std::vector<const CppAstNode *> * default_args_out)
{
  expanded_clause = node;
  expanded_clause.children.clear();
  if(default_args_out) {
    default_args_out->clear();
  }

  bool changed = false;
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    std::vector<CppAstNode> expanded_parameters;
    if(expand_parameter_declaration_pack_nodes(
           type_system,
           semantic_scope,
           child,
           expanded_parameters)) {
      if(cpp_decl::find_child(child, CppAstKind::default_argument)) {
        return false;
      }
      expanded_clause.children.insert(expanded_clause.children.end(),
                                      expanded_parameters.begin(),
                                      expanded_parameters.end());
      if(default_args_out) {
        default_args_out->insert(
            default_args_out->end(), expanded_parameters.size(), nullptr);
      }
      changed = true;
      continue;
    }

    expanded_clause.children.push_back(child);
    if(default_args_out && child.kind == CppAstKind::parameter_declaration) {
      default_args_out->push_back(
          cpp_decl::find_child(child, CppAstKind::default_argument));
    }
  }

  return changed;
}

cpp_decl::AstDeclHooks make_decl_hooks(template_api::TemplateServices & services,
                                       semantic_model::Scope & semantic_scope,
                                       semantic_model::Scope & parse_scope,
                                       bool reference_class_templates_only,
                                       bool re_resolve_dependent_semantic_types = false,
                                       bool bind_parameter_names = false);

semantic_model::Scope make_parameter_clause_scope(semantic_model::Scope & parent)
{
  semantic_model::Scope scope(&parent, "<parameter-clause>", false);
  scope.class_info = parent.class_info;
  scope.function = parent.function;
  scope.namespace_scope = parent.namespace_scope;
  return scope;
}

TypePtr lookup_decl_ast_type_node(template_api::TemplateServices & services,
                                  Scope & semantic_scope,
                                  const CppAstNode & node,
                                  bool reference_class_templates_only,
                                  const std::string & source_location = std::string())
{
  const std::string lookup_name =
      node.kind == CppAstKind::type_name && !node.value.empty() ?
          node.value :
          node_text(node);
  TypePtr out =
      template_argument_semantics::lookup_exact_local_type_name(
          services, semantic_scope, lookup_name);
  if(out) {
    return out;
  }
  out = template_argument_semantics::lookup_structured_type_node(
      services,
      semantic_scope,
      node,
      lookup_name,
      reference_class_templates_only,
      source_location);
  if(out) {
    return out;
  }
  resolve_decl_ast_direct_type_lookup(services,
                                      semantic_scope,
                                      lookup_name,
                                      cppast_qualified_name_syntax(node),
                                      reference_class_templates_only,
                                      out);
  return out;
}

TypePtr lookup_decl_ast_type_name(template_api::TemplateServices & services,
                                  Scope & semantic_scope,
                                  const std::string & name,
                                  bool reference_class_templates_only)
{
  TypePtr out =
      template_argument_semantics::lookup_exact_local_type_name(
          services, semantic_scope, name);
  if(out) {
    return out;
  }
  resolve_decl_ast_direct_type_lookup(services,
                                      semantic_scope,
                                      name,
                                      nullptr,
                                      reference_class_templates_only,
                                      out);
  return out;
}

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const std::string & location)
  {
    parser_trace::push_use_location(location);
  }

  ~ScopedTemplateUseLocation()
  {
    parser_trace::pop_use_location();
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;
};

std::string template_public_use_location_or(
    const template_api::TemplateWitnessContext & witness_context,
    const std::string & fallback)
{
  if(parser_trace::use_location_suppressed()) {
    return std::string();
  }
  if(!witness_context.public_use_location.empty()) {
    return witness_context.public_use_location;
  }
  const std::string current = parser_trace::current_use_location();
  return !current.empty() ? current : fallback;
}

bool sizeof_type_id_bound_is_dependent(template_api::TemplateServices & services,
                                       Scope & semantic_scope,
                                       Scope & parse_scope,
                                       bool reference_class_templates_only,
                                       const CppAstNode & node)
{
  if(node.kind != CppAstKind::sizeof_expression ||
     node.children.size() != 1 ||
     node.children[0].kind != CppAstKind::type_id) {
    return false;
  }

  TypePtr operand_type;
  try {
    if(!cpp_decl::parse_type_id_ast(
           node.children[0],
           make_decl_hooks(services,
                           semantic_scope,
                           parse_scope,
                           reference_class_templates_only),
           operand_type) ||
       !operand_type) {
      return false;
    }
  } catch(const std::logic_error &) {
    return false;
  }

  if(services.semantic_context) {
    return services.semantic_context->scope_has_template_placeholders(semantic_scope) &&
           services.semantic_context->sizeof_depends_on_template_parameters(operand_type);
  }
  return template_argument_semantics::type_depends_on_template_parameter(
      service_type_system(services),
      operand_type);
}

bool sizeof_pack_bound_is_dependent(Scope & scope, const CppAstNode & node)
{
  return node.kind == CppAstKind::sizeof_pack_expression &&
         node.children.size() == 1 &&
         node.children[0].kind == CppAstKind::identifier &&
         (template_scope::scope_has_type_parameter_pack_name(scope,
                                                             node.children[0].value) ||
          template_scope::scope_has_value_parameter_pack_name(scope,
                                                              node.children[0].value));
}

bool text_bound_is_dependent(template_api::TemplateServices & services,
                             Scope & scope,
                             const CppAstNode & node)
{
  if(!services.semantic_context) {
    return false;
  }
  const std::string text = node_text(node);
  return !text.empty() &&
         (services.semantic_context->text_mentions_template_placeholders(scope, text) ||
          services.semantic_context->text_mentions_dependent_non_namespace_binding_names(
              scope,
              text));
}

cpp_decl::AstDeclHooks make_decl_hooks(template_api::TemplateServices & services,
                                       semantic_model::Scope & semantic_scope,
                                       semantic_model::Scope & parse_scope,
                                       bool reference_class_templates_only,
                                       bool re_resolve_dependent_semantic_types,
                                       bool bind_parameter_names)
{
  cpp_decl::AstDeclHooks hooks;
  template_api::TemplateTypeSystem * const type_system = &service_type_system(services);
  if(re_resolve_dependent_semantic_types) {
    hooks.ignore_semantic_type =
        [type_system](const TypePtr & type)
        {
          return template_argument_semantics::type_depends_on_template_parameter(
              *type_system,
              type);
        };
  }
  hooks.lookup_type_node =
      [&services, &semantic_scope, reference_class_templates_only](
          const CppAstNode & node)
      {
        const std::string inherited_use_location =
            template_api::normalize_template_witness_source_location(
                template_public_use_location_or(services.witness_context,
                                                std::string()));
        const std::string node_use_location =
            template_api::normalize_template_witness_source_location(
                template_api::preferred_fragment_use_location(
                    services.witness_context,
                    node));
        const ScopedTemplateUseLocation use_location_guard(
            template_api::template_witness_detail::prefer_later_source_location(
                inherited_use_location,
                node_use_location));
        const template_api::ScopedTemplateWitnessSourceTypeLookup
            source_type_lookup_guard;
        const QualifiedName * qualified_lookup =
            cppast_qualified_name_syntax(node);
        const template_api::ScopedTemplateWitnessQualifiedMemberTypeLookup
            qualified_member_type_lookup_guard(
                qualified_lookup &&
                !qualified_lookup->qualifiers.empty() &&
                semantic_utils::unqualified_member_name(
                    qualified_lookup->name) == "type");
        return lookup_decl_ast_type_node(
            services,
            semantic_scope,
            node,
            reference_class_templates_only,
            template_api::template_witness_detail::prefer_later_source_location(
                inherited_use_location,
                node_use_location));
      };
  hooks.lookup_type =
      [&services, &semantic_scope, reference_class_templates_only](const std::string & name)
      {
        return lookup_decl_ast_type_name(services,
                                         semantic_scope,
                                         name,
                                         reference_class_templates_only);
      };
  hooks.parse_decltype_specifier =
      [&services, &semantic_scope](const CppAstNode & node, TypePtr & out)
      {
        return template_argument_semantics::parse_decltype_or_typeof_node(
                   services, semantic_scope, node, out) &&
               out != nullptr;
      };
  hooks.evaluate_constant_expression =
      [&services, &semantic_scope](const CppAstNode & node, long long & out)
      {
        constant_eval::ConstexprValue value;
        template_api::TemplateConstantEvaluationRequest request;
        request.scope = &semantic_scope;
        request.expr = node;
        request.target_type = make_fundamental(FT_INT);
        return (template_argument_semantics::evaluate_constant_expression_leaf(
                    services, semantic_scope, node, value, request.target_type) ||
                service_evaluate_initializer_constant_value(
                    services, request, value)) &&
               constant_eval::constexpr_value_to_integral(value, out);
      };
  hooks.array_bound_is_dependent =
      [&services, &semantic_scope, &parse_scope, reference_class_templates_only](
          const CppAstNode & node)
      {
        return sizeof_type_id_bound_is_dependent(services,
                                                 semantic_scope,
                                                 parse_scope,
                                                 reference_class_templates_only,
                                                 node) ||
               sizeof_pack_bound_is_dependent(semantic_scope, node) ||
               text_bound_is_dependent(services, semantic_scope, node);
      };
  hooks.expand_parameter_clause_packs =
      [type_system, &semantic_scope](
          const CppAstNode & node,
          CppAstNode & expanded_clause,
          std::vector<const CppAstNode *> * default_args_out)
      {
        return expand_parameter_clause_pack_patterns_impl(*type_system,
                                                         semantic_scope,
                                                         node,
                                                         expanded_clause,
                                                         default_args_out);
      };
  hooks.type_name_is_parameter_pack =
      [&semantic_scope](const std::string & name)
      {
        return template_scope::scope_has_type_parameter_pack_name(semantic_scope, name);
      };
  if(bind_parameter_names) {
    hooks.bind_parameter_name =
        [&semantic_scope](const std::string & name, const TypePtr & type)
        {
          template_scope::bind_parameter_value(semantic_scope, name, type);
        };
  }
  hooks.normalize_function_parameters = true;
  return hooks;
}

}  // namespace

bool expand_parameter_clause_pack_patterns(
    template_api::TemplateServices & services,
    semantic_model::Scope & semantic_scope,
    const CppAstNode & node,
    CppAstNode & expanded_clause,
    std::vector<const CppAstNode *> * default_args_out)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  return expand_parameter_clause_pack_patterns_impl(type_system,
                                                   semantic_scope,
                                                   node,
                                                   expanded_clause,
                                                   default_args_out);
}

bool parse_parameter_clause(template_api::TemplateServices & services,
                            semantic_model::Scope & parse_scope,
                            semantic_model::Scope & semantic_scope,
                            const CppAstNode & node,
                            std::vector<std::pair<std::string, TypePtr> > & params,
                            std::vector<const CppAstNode *> * default_args_out,
                            bool * variadic_out,
                            bool reference_class_templates_only)
{
  semantic_model::Scope parameter_scope = make_parameter_clause_scope(semantic_scope);
  return cpp_decl::parse_parameter_clause_ast(
      node,
      make_decl_hooks(services,
                      parameter_scope,
                      parse_scope,
                      reference_class_templates_only,
                      false,
                      true),
      params,
      default_args_out,
      variadic_out);
}

bool parse_declarator(template_api::TemplateServices & services,
                      semantic_model::Scope & parse_scope,
                      semantic_model::Scope & semantic_scope,
                      const CppAstNode & declarator,
                      const cpp_decl::TypePtr & base,
                      std::string & name,
                      cpp_decl::TypePtr & type,
                      bool reference_class_templates_only)
{
  if(!declarator_has_parameter_clause(declarator)) {
    return cpp_decl::parse_declarator_ast(
        declarator,
        make_decl_hooks(services,
                        semantic_scope,
                        parse_scope,
                        reference_class_templates_only),
        base,
        name,
        type);
  }
  semantic_model::Scope parameter_scope = make_parameter_clause_scope(semantic_scope);
  return cpp_decl::parse_declarator_ast(
      declarator,
      make_decl_hooks(services,
                      parameter_scope,
                      parse_scope,
                      reference_class_templates_only,
                      false,
                      true),
      base,
      name,
      type);
}

bool parse_abstract_declarator(template_api::TemplateServices & services,
                               semantic_model::Scope & parse_scope,
                               semantic_model::Scope & semantic_scope,
                               const CppAstNode & abstract_declarator,
                               const cpp_decl::TypePtr & base,
                               cpp_decl::TypePtr & type,
                               bool reference_class_templates_only)
{
  return cpp_decl::parse_abstract_declarator_ast(
      abstract_declarator,
      make_decl_hooks(services,
                      semantic_scope,
                      parse_scope,
                      reference_class_templates_only),
      base,
      type);
}

bool parse_type_specifier_seq(template_api::TemplateServices & services,
                              semantic_model::Scope & parse_scope,
                              semantic_model::Scope & semantic_scope,
                              const CppAstNode & specifiers,
                              cpp_decl::TypePtr & out,
                              bool reference_class_templates_only,
                              bool re_resolve_dependent_semantic_types)
{
  return cpp_decl::parse_type_specifier_seq_ast(
      specifiers,
      make_decl_hooks(services,
                      semantic_scope,
                      parse_scope,
                      reference_class_templates_only,
                      re_resolve_dependent_semantic_types),
      out);
}

bool parse_type_id(template_api::TemplateServices & services,
                   semantic_model::Scope & parse_scope,
                   semantic_model::Scope & semantic_scope,
                   const CppAstNode & type_id,
                   cpp_decl::TypePtr & out,
                   bool reference_class_templates_only)
{
  const std::string inherited_use_location =
      template_api::normalize_template_witness_source_location(
          template_public_use_location_or(services.witness_context,
                                          std::string()));
  const std::string type_id_use_location =
      template_api::normalize_template_witness_source_location(
          template_api::preferred_fragment_use_location(services.witness_context,
                                                       type_id));
  const ScopedTemplateUseLocation use_location_guard(
      template_api::template_witness_detail::prefer_later_source_location(
          inherited_use_location,
          type_id_use_location));
  return cpp_decl::parse_type_id_ast(
      type_id,
      make_decl_hooks(services, semantic_scope, parse_scope, reference_class_templates_only),
      out);
}

bool parse_trailing_return_base(template_api::TemplateServices & services,
                                semantic_model::Scope & scope,
                                const CppAstNode & specifiers,
                                const CppAstNode & declarator,
                                bool & is_typedef,
                                cpp_decl::TypePtr & out,
                                bool reference_class_templates_only)
{
  const CppAstNode * trailing =
      cpp_decl::find_child(declarator, CppAstKind::trailing_return_type);
  if(!trailing) {
    return cpp_decl::parse_decl_spec_ast(
        specifiers,
        make_decl_hooks(services, scope, scope, reference_class_templates_only),
        is_typedef,
        out);
  }
  if(!cpp_decl::decl_spec_contains_token(specifiers, KW_AUTO)) {
    return false;
  }

  is_typedef = false;
  semantic_model::Scope return_scope(&scope, "<trailing-return>", false);
  return_scope.class_info = scope.class_info;
  return_scope.namespace_scope = scope.namespace_scope;
  FunctionBinding synthetic_function;
  if(scope.class_info &&
     scope.class_info->type &&
     !cpp_decl::decl_spec_contains_token(specifiers, KW_STATIC)) {
    const bool is_const_method =
        semantic_class_model::declarator_is_const_method(declarator);
    const bool is_volatile_method =
        semantic_class_model::declarator_is_volatile_method(declarator);
    TypePtr this_type =
        make_pointer(make_cv(scope.class_info->type,
                             is_const_method,
                             is_volatile_method));
    if(this_type) {
      synthetic_function.name = "<trailing-return>";
      synthetic_function.owner_class = scope.class_info;
      synthetic_function.lexical_access_class = scope.class_info;
      synthetic_function.is_method = true;
      synthetic_function.is_const_method = is_const_method;
      synthetic_function.is_volatile_method = is_volatile_method;
      synthetic_function.ref_qualifier =
          semantic_class_model::declarator_ref_qualifier(declarator);
      synthetic_function.type =
          make_function(make_fundamental(FT_VOID),
                        std::vector<TypePtr>(1, this_type),
                        false);
      return_scope.function = &synthetic_function;
      return_scope.values["this"] =
          ValueBinding(ValueBinding::VK_PARAMETER, "this", this_type);
    }
  }
  const CppAstNode * parameter_clause =
      cpp_decl::find_child(declarator, CppAstKind::parameter_clause);
  if(parameter_clause) {
    std::vector<std::pair<std::string, TypePtr> > params;
    if(!parse_parameter_clause(services,
                               scope,
                               scope,
                               *parameter_clause,
                               params,
                               nullptr,
                               nullptr,
                               reference_class_templates_only)) {
      return false;
    }
    for(std::size_t i = 0; i < params.size(); ++i) {
      if(params[i].first.empty()) {
        continue;
      }
      template_scope::bind_value(
          return_scope,
          params[i].first,
          ValueBinding(ValueBinding::VK_PARAMETER, params[i].first, params[i].second));
    }
  }

  const CppAstNode * trailing_type_id =
      cpp_decl::find_child(*trailing, CppAstKind::type_id);
  if(!trailing_type_id ||
     !parse_type_id(services,
                    scope,
                    return_scope,
                    *trailing_type_id,
                    out,
                    reference_class_templates_only)) {
    return false;
  }
  out = apply_cv(out,
                 cpp_decl::decl_spec_contains_token(specifiers, KW_CONST),
                 cpp_decl::decl_spec_contains_token(specifiers, KW_VOLATILE));
  return static_cast<bool>(out);
}

}  // namespace template_decl_ast
