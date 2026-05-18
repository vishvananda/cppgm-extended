#include "pack_parameter_analysis.h"

#include <cctype>
#include <set>
#include <stdexcept>

#include "cpp_decl_bridge.h"
#include "cpp_decl_model.h"

namespace pack_parameter_analysis {

using namespace cpp_decl;
using namespace semantic_model;
using template_model::TemplateParameterInfo;

namespace {

bool identifier_char(unsigned char ch)
{
  return std::isalnum(ch) || ch == '_';
}

bool contains_identifier_token(const std::string & text,
                               const std::string & name)
{
  if(text.empty() || name.empty()) {
    return false;
  }
  std::size_t pos = text.find(name);
  while(pos != std::string::npos) {
    const bool left_ok =
        pos == 0 ||
        !identifier_char(static_cast<unsigned char>(text[pos - 1]));
    const std::size_t end = pos + name.size();
    const bool right_ok =
        end >= text.size() ||
        !identifier_char(static_cast<unsigned char>(text[end]));
    if(left_ok && right_ok) {
      return true;
    }
    pos = text.find(name, pos + 1);
  }
  return false;
}

bool template_argument_syntax_mentions_identifier(
    const TemplateArgumentSyntax & syntax,
    const std::string & name);
bool template_id_syntax_mentions_identifier(const TemplateIdSyntax & syntax,
                                            const std::string & name);
bool ast_node_mentions_identifier(const CppAstNode & node,
                                  const std::string & name);

bool template_argument_syntax_mentions_identifier(
    const TemplateArgumentSyntax & syntax,
    const std::string & name)
{
  if(!syntax.text.empty() && contains_identifier_token(syntax.text, name)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_identifier(*syntax.template_id, name)) {
    return true;
  }
  if(syntax.type_id && ast_node_mentions_identifier(*syntax.type_id, name)) {
    return true;
  }
  return syntax.expression &&
         ast_node_mentions_identifier(*syntax.expression, name);
}

bool template_id_syntax_mentions_identifier(const TemplateIdSyntax & syntax,
                                            const std::string & name)
{
  for(std::size_t i = 0; i < syntax.name.qualifiers.size(); ++i) {
    if(syntax.name.qualifiers[i] == name) {
      return true;
    }
  }
  if(syntax.name.name == name) {
    return true;
  }
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(contains_identifier_token(syntax.arguments[i], name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_identifier(
           syntax.argument_syntaxes[i],
           name)) {
      return true;
    }
  }
  return false;
}

bool ast_node_mentions_identifier(const CppAstNode & node,
                                  const std::string & name)
{
  if(node.kind == CppAstKind::identifier && node.value == name) {
    return true;
  }
  if(!node.value.empty() && contains_identifier_token(node.value, name)) {
    return true;
  }
  if(node.template_id_syntax &&
     template_id_syntax_mentions_identifier(*node.template_id_syntax, name)) {
    return true;
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_identifier(
           node.qualifier_template_id_syntaxes[i],
           name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_mentions_identifier(node.qualifier_type_syntaxes[i], name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_mentions_identifier(node.children[i], name)) {
      return true;
    }
  }
  return false;
}

bool declarator_has_parameter_pack(const CppAstNode & declarator)
{
  if(cpp_decl::find_child(declarator, CppAstKind::parameter_pack)) {
    return true;
  }
  const CppAstNode * nested =
      cpp_decl::find_child(declarator, CppAstKind::nested_declarator);
  return nested &&
         !nested->children.empty() &&
         declarator_has_parameter_pack(nested->children[0]);
}

std::string last_identifier_in_subtree(const CppAstNode & node)
{
  std::string out;
  if(node.kind == CppAstKind::identifier && !node.value.empty()) {
    out = node.value;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const std::string nested = last_identifier_in_subtree(node.children[i]);
    if(!nested.empty()) {
      out = nested;
    }
  }
  return out;
}

}  // namespace

std::string parameter_declaration_name(const CppAstNode & parameter)
{
  const CppAstNode * declarator =
      cpp_decl::find_child(parameter, CppAstKind::declarator);
  if(!declarator) {
    declarator = cpp_decl::find_child(parameter, CppAstKind::abstract_declarator);
  }
  return declarator ? last_identifier_in_subtree(*declarator) : std::string();
}

std::vector<std::pair<std::string, const std::vector<TypePtr> *> >
referenced_named_type_packs(Scope & scope, const CppAstNode & parameter)
{
  const std::string parameter_name = parameter_declaration_name(parameter);
  std::vector<std::pair<std::string, const std::vector<TypePtr> *> > packs;
  std::set<std::string> seen_pack_names;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(const auto & pack : current->named_type_packs) {
      if(pack.first.empty() ||
         pack.first == parameter_name ||
         !seen_pack_names.insert(pack.first).second ||
         !ast_node_mentions_identifier(parameter, pack.first)) {
        continue;
      }
      packs.push_back(std::make_pair(pack.first, &pack.second));
    }
  }
  return packs;
}

bool parameter_references_template_parameter_pack(
    const CppAstNode & parameter,
    const std::vector<TemplateParameterInfo> & template_parameters)
{
  const std::string parameter_name = parameter_declaration_name(parameter);
  for(std::size_t i = 0; i < template_parameters.size(); ++i) {
    if(template_parameters[i].parameter_pack &&
       !template_parameters[i].name.empty() &&
       template_parameters[i].name != parameter_name &&
       ast_node_mentions_identifier(parameter, template_parameters[i].name)) {
      return true;
    }
  }
  return false;
}

bool declarator_has_trailing_template_parameter_pack(
    const CppAstNode & declarator,
    const std::vector<TemplateParameterInfo> & template_parameters)
{
  const CppAstNode * parameter_clause =
      cpp_decl::find_child(declarator, CppAstKind::parameter_clause);
  if(!parameter_clause || parameter_clause->children.empty()) {
    return false;
  }
  const CppAstNode & last = parameter_clause->children.back();
  if(last.kind != CppAstKind::parameter_declaration) {
    return false;
  }
  const CppAstNode * declarator_child =
      cpp_decl::find_child(last, CppAstKind::declarator);
  const CppAstNode * abstract =
      cpp_decl::find_child(last, CppAstKind::abstract_declarator);
  if(!(declarator_child && declarator_has_parameter_pack(*declarator_child)) &&
     !(abstract && declarator_has_parameter_pack(*abstract))) {
    return false;
  }
  return parameter_references_template_parameter_pack(last, template_parameters);
}

bool infer_named_type_pack_size(Scope & scope,
                                const CppAstNode & parameter,
                                std::size_t & out_pack_size)
{
  const auto packs = referenced_named_type_packs(scope, parameter);
  if(packs.empty()) {
    return false;
  }

  const std::size_t pack_size = packs[0].second->size();
  for(std::size_t i = 1; i < packs.size(); ++i) {
    if(packs[i].second->size() != pack_size) {
      throw std::logic_error("function parameter pack size mismatch");
    }
  }
  out_pack_size = pack_size;
  return true;
}

}  // namespace pack_parameter_analysis
