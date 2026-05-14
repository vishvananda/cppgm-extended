#include "pack_parameter_analysis.h"

#include <cctype>
#include <set>
#include <stdexcept>

#include "cpp_decl_bridge.h"
#include "cpp_decl_model.h"

namespace pack_parameter_analysis {

using namespace cpp_decl;
using namespace semantic_model;

namespace {

bool is_identifier_spelling(const std::string & text)
{
  if(text.empty()) {
    return false;
  }

  const unsigned char first = static_cast<unsigned char>(text[0]);
  if(!(std::isalpha(first) || first == '_')) {
    return false;
  }

  for(std::size_t i = 1; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(!(std::isalnum(ch) || ch == '_')) {
      return false;
    }
  }

  return true;
}

void collect_identifiers(const CppAstNode & node, std::set<std::string> & out);
void collect_identifiers(const TemplateIdSyntax & syntax, std::set<std::string> & out);
void collect_identifiers(const TemplateArgumentSyntax & syntax,
                         std::set<std::string> & out);

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

void collect_identifiers(const CppAstNode & node, std::set<std::string> & out)
{
  if(!node.value.empty() && is_identifier_spelling(node.value)) {
    out.insert(node.value);
  }
  if(node.template_id_syntax) {
    collect_identifiers(*node.template_id_syntax, out);
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    collect_identifiers(node.qualifier_template_id_syntaxes[i], out);
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    collect_identifiers(node.qualifier_type_syntaxes[i], out);
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_identifiers(node.children[i], out);
  }
}

void collect_identifiers(const TemplateArgumentSyntax & syntax,
                         std::set<std::string> & out)
{
  if(syntax.template_id) {
    collect_identifiers(*syntax.template_id, out);
  }
  if(syntax.type_id) {
    collect_identifiers(*syntax.type_id, out);
  }
  if(syntax.expression) {
    collect_identifiers(*syntax.expression, out);
  }
}

void collect_identifiers(const TemplateIdSyntax & syntax,
                         std::set<std::string> & out)
{
  for(std::size_t i = 0; i < syntax.name.qualifiers.size(); ++i) {
    if(is_identifier_spelling(syntax.name.qualifiers[i])) {
      out.insert(syntax.name.qualifiers[i]);
    }
  }
  if(is_identifier_spelling(syntax.name.name)) {
    out.insert(syntax.name.name);
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    collect_identifiers(syntax.argument_syntaxes[i], out);
  }
}

std::set<std::string> referenced_parameter_type_identifiers(const CppAstNode & parameter)
{
  std::set<std::string> identifiers;
  collect_identifiers(parameter, identifiers);

  const std::string parameter_name = parameter_declaration_name(parameter);
  if(!parameter_name.empty()) {
    identifiers.erase(parameter_name);
  }
  return identifiers;
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
  const std::set<std::string> identifiers =
      referenced_parameter_type_identifiers(parameter);
  std::vector<std::pair<std::string, const std::vector<TypePtr> *> > packs;
  std::set<std::string> seen_pack_names;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(const auto & pack : current->named_type_packs) {
      if(pack.first.empty() ||
         identifiers.count(pack.first) == 0 ||
         !seen_pack_names.insert(pack.first).second) {
        continue;
      }
      packs.push_back(std::make_pair(pack.first, &pack.second));
    }
  }
  return packs;
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
