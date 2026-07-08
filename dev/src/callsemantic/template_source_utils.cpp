#include "callsemantic/template_source_utils.h"

#include <algorithm>
#include <deque>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "callsemantic/source_location_utils.h"
#include "callsemantic_internal.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "parser_trace.h"
#include "semantic_context.h"
#include "semantic_lookup.h"
#include "semantic_model.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"

namespace callsemantic {

using namespace std;
using namespace cpp_decl;
using namespace semantic_model;
using template_model::TemplateArgument;
using template_model::TemplateParameterInfo;
using semantic_utils::strip_elaborated_type_prefix;
using semantic_utils::strip_trailing_top_level_template_arguments;
using semantic_utils::trim_space;
using semantic_utils::unqualified_member_name;
using namespace callsemantic_internal;

witness::SourceSelectionKind source_selection_kind_for_match_kind(
    template_api::MatchKind kind)
{
  switch(kind) {
  case template_api::MS_PRIMARY:
    return witness::SourceSelectionKind::Primary;
  case template_api::MS_EXPLICIT_SPECIALIZATION:
    return witness::SourceSelectionKind::ExplicitSpecialization;
  case template_api::MS_PARTIAL_SPECIALIZATION:
    return witness::SourceSelectionKind::PartialSpecialization;
  }
  return witness::SourceSelectionKind::None;
}

witness::TemplateWitnessSourceAnchor class_use_selected_decl_anchor(
    SemanticContext & ctx,
    ClassTemplateDecl * class_template,
    const template_api::ClassSpecializationSelection & selection)
{
  witness::TemplateWitnessSourceAnchor anchor;
  if(class_template == nullptr) {
    return anchor;
  }
  std::vector<const CppAstNode *> decl_candidates;
  if(selection.kind == template_api::MS_PRIMARY) {
    if(const CppAstNode * member_decl =
           find_member_class_template_declaration_node(class_template)) {
      decl_candidates.push_back(member_decl);
    }
    if(selection.class_node) {
      decl_candidates.push_back(selection.class_node);
    }
    if(class_template->class_node) {
      decl_candidates.push_back(class_template->class_node);
    }
  } else {
    if(selection.class_node) {
      decl_candidates.push_back(selection.class_node);
    }
    if(class_template->class_node) {
      decl_candidates.push_back(class_template->class_node);
    }
  }
  for(size_t i = 0; i < decl_candidates.size(); ++i) {
    if(!decl_candidates[i]) {
      continue;
    }
    if(decl_candidates[i] == class_template->class_node) {
      const semantic_model::SourceDeclAnchorCache & decl_anchor =
          semantic_trace::class_template_decl_anchor(ctx, class_template);
      anchor.location =
          semantic_model::source_decl_anchor_location(decl_anchor);
      anchor.kind = semantic_model::source_decl_anchor_has_name_location(decl_anchor) ?
          witness::TemplateWitnessSourceAnchorKind::DeclarationName :
          (!anchor.location.empty() ?
               witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration :
               witness::TemplateWitnessSourceAnchorKind::None);
    } else {
      anchor.location =
          ctx.source_location_for_name_in_node(*decl_candidates[i],
                                               class_template->name);
      if(!anchor.location.empty()) {
        anchor.kind = witness::TemplateWitnessSourceAnchorKind::DeclarationName;
      } else {
        anchor.location = ctx.source_location_for_node(*decl_candidates[i]);
        if(!anchor.location.empty()) {
          anchor.kind =
              witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
        }
      }
    }
    anchor.location =
        template_api::normalize_template_witness_source_location(anchor.location);
    if(!anchor.location.empty()) {
      break;
    }
    anchor.kind = witness::TemplateWitnessSourceAnchorKind::None;
  }
  return anchor;
}

bool scope_is_inside_source_template_context(Scope & scope)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(template_api::class_has_source_template_identity(current->class_info) ||
       template_api::function_binding_has_source_template_identity(current->function)) {
      return true;
    }
  }
  return false;
}

string canonicalize_template_parameter_redeclaration_text(
    const vector<TemplateParameterInfo> & parameters,
    const string & text)
{
  if(text.empty()) {
    return text;
  }
  string out = text;
  bool changed = false;
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].name.empty()) {
      continue;
    }
    const string replacement = string("__cppgm_tparam_") + to_string(i);
    out = replace_elaborated_identifier_token_text(out,
                                                   parameters[i].name,
                                                   replacement,
                                                   changed);
    out = replace_identifier_token_text(out,
                                        parameters[i].name,
                                        replacement,
                                        changed);
  }
  return changed ? out : text;
}

bool template_parameter_redeclarations_compatible(
    const vector<TemplateParameterInfo> & existing_parameters,
    const vector<TemplateParameterInfo> & incoming_parameters,
    size_t index)
{
  const TemplateParameterInfo & existing = existing_parameters[index];
  const TemplateParameterInfo & incoming = incoming_parameters[index];
  if(existing.kind != incoming.kind ||
     existing.parameter_pack != incoming.parameter_pack ||
     existing.template_parameter_count != incoming.template_parameter_count) {
    return false;
  }

  if(existing.kind == TemplateParameterInfo::TP_NON_TYPE) {
    if(existing.value_type && incoming.value_type &&
       semantic_lookup::same_function_template_entity_type(existing.value_type,
                                                           existing_parameters,
                                                           incoming.value_type,
                                                           incoming_parameters)) {
      return true;
    }

    const CppAstNode * existing_decl =
        existing.non_type_declarator ? existing.non_type_declarator :
                                       existing.non_type_abstract_declarator;
    const CppAstNode * incoming_decl =
        incoming.non_type_declarator ? incoming.non_type_declarator :
                                       incoming.non_type_abstract_declarator;
    const string existing_specifier_text =
        !existing.non_type_decl_specifier_text.empty() ?
            existing.non_type_decl_specifier_text :
            (existing.non_type_decl_specifier_seq ?
                 node_text(*existing.non_type_decl_specifier_seq) :
                 string());
    const string incoming_specifier_text =
        !incoming.non_type_decl_specifier_text.empty() ?
            incoming.non_type_decl_specifier_text :
            (incoming.non_type_decl_specifier_seq ?
                 node_text(*incoming.non_type_decl_specifier_seq) :
                 string());
    const string normalized_existing_specifier_text =
        canonicalize_template_parameter_redeclaration_text(
            existing_parameters, existing_specifier_text);
    const string normalized_incoming_specifier_text =
        canonicalize_template_parameter_redeclaration_text(
            incoming_parameters, incoming_specifier_text);
    if(!existing_specifier_text.empty() &&
       !incoming_specifier_text.empty() &&
       normalized_existing_specifier_text != normalized_incoming_specifier_text) {
      return false;
    }
    const string normalized_existing_declarator_text =
        existing_decl ?
            canonicalize_template_parameter_redeclaration_text(
                existing_parameters, node_text(*existing_decl)) :
            string();
    const string normalized_incoming_declarator_text =
        incoming_decl ?
            canonicalize_template_parameter_redeclaration_text(
                incoming_parameters, node_text(*incoming_decl)) :
            string();
    if(existing_decl && incoming_decl &&
       normalized_existing_declarator_text != normalized_incoming_declarator_text) {
      return false;
    }
  }

  return true;
}

bool merge_template_parameter_redeclarations(
    vector<TemplateParameterInfo> & target,
    const vector<TemplateParameterInfo> & incoming)
{
  if(target.size() != incoming.size()) {
    return false;
  }

  for(size_t i = 0; i < target.size(); ++i) {
    if(!template_parameter_redeclarations_compatible(target, incoming, i)) {
      return false;
    }
    if(target[i].name.empty()) {
      target[i].name = incoming[i].name;
    } else if(!incoming[i].name.empty() && incoming[i].name != target[i].name &&
              find(target[i].alternate_names.begin(),
                   target[i].alternate_names.end(),
                   incoming[i].name) == target[i].alternate_names.end()) {
      target[i].alternate_names.push_back(incoming[i].name);
    }
    for(size_t j = 0; j < incoming[i].alternate_names.size(); ++j) {
      const string & alias = incoming[i].alternate_names[j];
      if(alias.empty() || alias == target[i].name ||
         find(target[i].alternate_names.begin(),
              target[i].alternate_names.end(),
              alias) != target[i].alternate_names.end()) {
        continue;
      }
      target[i].alternate_names.push_back(alias);
    }
    if(!target[i].default_argument) {
      template_model::copy_template_parameter_default_argument(target[i], incoming[i]);
    }
    if(!target[i].value_type) {
      target[i].value_type = incoming[i].value_type;
    }
    if(target[i].placeholder_key.empty()) {
      target[i].placeholder_key = incoming[i].placeholder_key;
    }
    if(!target[i].non_type_decl_specifier_seq) {
      template_model::copy_template_parameter_non_type_decl_specifier_seq(
          target[i], incoming[i]);
    }
    if(target[i].non_type_decl_specifier_text.empty()) {
      target[i].non_type_decl_specifier_text = incoming[i].non_type_decl_specifier_text;
    }
    if(!target[i].non_type_declarator) {
      template_model::copy_template_parameter_non_type_declarator(
          target[i], incoming[i]);
    }
    if(!target[i].non_type_abstract_declarator) {
      template_model::copy_template_parameter_non_type_abstract_declarator(
          target[i], incoming[i]);
    }
  }

  return true;
}

void prefer_incoming_template_parameter_spellings(
    vector<TemplateParameterInfo> & target,
    const vector<TemplateParameterInfo> & incoming)
{
  for(size_t i = 0; i < target.size() && i < incoming.size(); ++i) {
    if(!incoming[i].name.empty()) {
      if(!target[i].name.empty() &&
         target[i].name != incoming[i].name &&
         find(target[i].alternate_names.begin(),
              target[i].alternate_names.end(),
              target[i].name) == target[i].alternate_names.end()) {
        target[i].alternate_names.push_back(target[i].name);
      }
      target[i].name = incoming[i].name;
    }
    for(size_t j = 0; j < incoming[i].alternate_names.size(); ++j) {
      const string & alias = incoming[i].alternate_names[j];
      if(alias.empty() || alias == target[i].name ||
         find(target[i].alternate_names.begin(),
              target[i].alternate_names.end(),
              alias) != target[i].alternate_names.end()) {
        continue;
      }
      target[i].alternate_names.push_back(alias);
    }
    if(!incoming[i].placeholder_key.empty()) {
      target[i].placeholder_key = incoming[i].placeholder_key;
    }
    if(incoming[i].value_type) {
      target[i].value_type = incoming[i].value_type;
    }
    if(incoming[i].non_type_decl_specifier_seq) {
      template_model::copy_template_parameter_non_type_decl_specifier_seq(
          target[i], incoming[i]);
    }
    if(!incoming[i].non_type_decl_specifier_text.empty()) {
      target[i].non_type_decl_specifier_text = incoming[i].non_type_decl_specifier_text;
    }
    if(incoming[i].non_type_declarator) {
      template_model::copy_template_parameter_non_type_declarator(
          target[i], incoming[i]);
    }
    if(incoming[i].non_type_abstract_declarator) {
      template_model::copy_template_parameter_non_type_abstract_declarator(
          target[i], incoming[i]);
    }
  }
}

void record_definition_parameter_aliases(
    FunctionBinding & binding,
    const vector<pair<string, TypePtr> > & params)
{
  ensure_function_parameter_aliases(binding);
  const size_t offset = function_binding_explicit_parameter_offset(binding);
  if(binding.params.size() != params.size() + offset) {
    return;
  }
  for(size_t i = 0; i < params.size(); ++i) {
    binding.parameter_aliases[i + offset] =
        params[i].first.empty() ? binding.params[i + offset].first : params[i].first;
  }
}

void record_definition_parameter_aliases(
    FunctionTemplateDecl & decl,
    const vector<pair<string, TypePtr> > & params)
{
  ensure_function_template_parameter_aliases(decl);
  if(decl.params_pattern.size() != params.size()) {
    return;
  }
  for(size_t i = 0; i < params.size(); ++i) {
    const string generated_name = string("arg") + to_string(i);
    const bool existing_unnamed =
        decl.params_pattern[i].first.empty() ||
        decl.params_pattern[i].first == generated_name ||
        decl.params_pattern[i].first.compare(0, 7, "__param") == 0;
    if(existing_unnamed && !params[i].first.empty()) {
      decl.params_pattern[i].first = params[i].first;
    }
    decl.parameter_aliases_pattern[i] =
        params[i].first.empty() ? decl.params_pattern[i].first : params[i].first;
  }
}

void invalidate_out_of_class_definition_caches(ClassTemplateDecl & decl)
{
  for(auto it = decl.instantiations.begin();
      it != decl.instantiations.end();
      ++it) {
    if(!it->second) {
      continue;
    }
    it->second->out_of_class_member_function_template_definitions_applied = false;
    it->second->out_of_class_member_function_definitions_applied = false;
    it->second->out_of_class_special_member_definitions_applied = false;
    it->second->out_of_class_static_member_definitions_applied = false;
  }
  for(auto it = decl.reference_instantiations.begin();
      it != decl.reference_instantiations.end();
      ++it) {
    if(!it->second) {
      continue;
    }
    it->second->out_of_class_member_function_template_definitions_applied = false;
    it->second->out_of_class_member_function_definitions_applied = false;
    it->second->out_of_class_special_member_definitions_applied = false;
    it->second->out_of_class_static_member_definitions_applied = false;
  }
}

PartialClassTemplateSpecializationDecl * find_partial_specialization_decl(
    ClassTemplateDecl & decl,
    const ClassInfo * owner)
{
  if(!owner ||
     !owner->template_output_node ||
     owner->template_output_node == decl.class_node) {
    return nullptr;
  }
  for(size_t i = 0; i < decl.partial_specializations.size(); ++i) {
    PartialClassTemplateSpecializationDecl & partial =
        decl.partial_specializations[i];
    if(partial.class_node == owner->template_output_node) {
      return &partial;
    }
  }
  return nullptr;
}

const CppAstNode * find_function_body_node(const CppAstNode & node)
{
  if(const CppAstNode * body = find_child_kind(node, CppAstKind::compound_statement)) {
    return body;
  }
  if(const CppAstNode * body = find_child_kind(node, CppAstKind::lazy_function_body)) {
    return body;
  }
  return find_child_kind(node, CppAstKind::try_block);
}

const CppAstNode * find_descendant_kind(const CppAstNode & node, CppAstKind kind)
{
  if(node.kind == kind) {
    return &node;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(const CppAstNode * found = find_descendant_kind(node.children[i], kind)) {
      return found;
    }
  }
  return nullptr;
}

const TemplateIdSyntax * first_template_id_syntax_in_subtree(
    const CppAstNode & node)
{
  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    return syntax;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const TemplateIdSyntax * found =
           first_template_id_syntax_in_subtree(node.children[i])) {
      return found;
    }
  }
  return nullptr;
}

bool qualified_template_id_syntax_in_subtree(const TemplateIdSyntax & syntax)
{
  if(syntax.name.rooted || !syntax.name.qualifiers.empty()) {
    return true;
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(syntax.argument_syntaxes[i].template_id &&
       qualified_template_id_syntax_in_subtree(
           *syntax.argument_syntaxes[i].template_id)) {
      return true;
    }
  }
  return false;
}

bool qualified_template_id_syntax_in_subtree(const CppAstNode & node)
{
  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    if(qualified_template_id_syntax_in_subtree(*syntax)) {
      return true;
    }
  }
  if(!node.qualifier_template_id_syntaxes.empty()) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(qualified_template_id_syntax_in_subtree(node.children[i])) {
      return true;
    }
  }
  return false;
}

string qualified_name_syntax_text(const QualifiedName & name)
{
  string out = name.rooted ? "::" : string();
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    out += name.qualifiers[i];
    out += "::";
  }
  out += name.name;
  return out;
}

string repair_compacted_template_argument_expression_spacing(const string & text)
{
  string out = text;
  size_t pos = 0;
  while((pos = out.find(")>>", pos)) != string::npos) {
    out.replace(pos, 3, ") >> ");
    pos += 5;
  }
  return out;
}

string compact_source_spelling_key(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(!std::isspace(static_cast<unsigned char>(text[i]))) {
      out.push_back(text[i]);
    }
  }
  return out;
}

string template_id_syntax_text_preserving_spacing(const TemplateIdSyntax & syntax);

string template_argument_syntax_text_preserving_spacing(
    const TemplateArgumentSyntax & argument)
{
  const string original_text = trim_space(
      repair_compacted_template_argument_expression_spacing(argument.text));
  if(argument.template_id) {
    const string template_id_text =
        template_id_syntax_text_preserving_spacing(*argument.template_id);
    if(argument.type_id || argument.expression) {
      const auto compact =
          [](const string & text)
          {
            string out;
            out.reserve(text.size());
            for(size_t i = 0; i < text.size(); ++i) {
              if(!std::isspace(static_cast<unsigned char>(text[i]))) {
                out.push_back(text[i]);
              }
            }
            return out;
          };
      if(compact(original_text) == compact(template_id_text)) {
        return template_id_text;
      }
      return original_text;
    }
    return template_id_text;
  }
  if(argument.type_id || argument.expression) {
    return original_text;
  }
  return original_text;
}

string template_argument_syntax_witness_source_text(
    const TemplateArgumentSyntax & argument)
{
  if(!trim_space(argument.source_text).empty()) {
    return trim_space(argument.source_text);
  }
  if(argument.type_id) {
    const string type_text = trim_space(cpp_decl::node_text(*argument.type_id));
    if(!type_text.empty()) {
      return type_text;
    }
  }
  if(argument.expression) {
    const string expression_text =
        trim_space(callsemantic_internal::describe_expression_for_diagnostic(
            *argument.expression));
    if(!expression_text.empty() &&
       compact_source_spelling_key(expression_text) ==
           compact_source_spelling_key(argument.text)) {
      return expression_text;
    }
  }
  return template_argument_syntax_text_preserving_spacing(argument);
}

string template_id_syntax_text_preserving_spacing(const TemplateIdSyntax & syntax)
{
  ostringstream out;
  out << qualified_name_syntax_text(syntax.name) << "<";
  const size_t syntax_count =
      std::min(syntax.argument_syntaxes.size(), syntax.arguments.size());
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    string text;
    if(i < syntax_count) {
      text = template_argument_syntax_text_preserving_spacing(syntax.argument_syntaxes[i]);
    }
    out << (text.empty() ?
                trim_space(repair_compacted_template_argument_expression_spacing(
                    syntax.arguments[i])) :
                text);
  }
  out << ">";
  return out.str();
}

vector<string> template_id_argument_texts_preserving_spacing(
    const TemplateIdSyntax & syntax)
{
  vector<string> out;
  if(syntax.arguments.size() == 1 &&
     trim_space(syntax.arguments[0]).empty()) {
    if(syntax.argument_syntaxes.empty() ||
       template_argument_syntax_text_preserving_spacing(
           syntax.argument_syntaxes[0]).empty()) {
      return out;
    }
  }
  if(syntax.argument_syntaxes.size() == 1 &&
     syntax.arguments.empty() &&
     template_argument_syntax_text_preserving_spacing(
         syntax.argument_syntaxes[0]).empty()) {
    return out;
  }
  out.reserve(syntax.arguments.size());
  const size_t syntax_count =
      std::min(syntax.argument_syntaxes.size(), syntax.arguments.size());
  for(size_t i = 0; i < syntax_count; ++i) {
    string text =
        template_argument_syntax_text_preserving_spacing(syntax.argument_syntaxes[i]);
    out.push_back(
        text.empty() ?
            trim_space(repair_compacted_template_argument_expression_spacing(
                syntax.arguments[i])) :
            text);
  }
  for(size_t i = syntax_count; i < syntax.arguments.size(); ++i) {
    out.push_back(
        trim_space(repair_compacted_template_argument_expression_spacing(
            syntax.arguments[i])));
  }
  return out;
}

vector<string> template_id_argument_witness_source_texts(
    const TemplateIdSyntax & syntax)
{
  vector<string> out = template_id_argument_texts_preserving_spacing(syntax);
  const size_t limit = std::min(out.size(), syntax.argument_syntaxes.size());
  for(size_t i = 0; i < limit; ++i) {
    const string source_text =
        template_argument_syntax_witness_source_text(syntax.argument_syntaxes[i]);
    if(!source_text.empty()) {
      out[i] = source_text;
    }
  }
  return out;
}

vector<const CppAstNode *> parameter_declarations_from_clause(
    const CppAstNode & parameter_clause)
{
  vector<const CppAstNode *> out;
  for(size_t i = 0; i < parameter_clause.children.size(); ++i) {
    if(parameter_clause.children[i].kind == CppAstKind::parameter_declaration) {
      out.push_back(&parameter_clause.children[i]);
    }
  }
  return out;
}

vector<TemplateArgumentSyntax> normalized_template_argument_syntaxes(
    const TemplateIdSyntax & source_syntax,
    const vector<TemplateParameterInfo> & primary_parameters,
    const vector<string> & normalized_arg_texts)
{
  vector<TemplateArgumentSyntax> out;
  out.reserve(normalized_arg_texts.size());
  const size_t explicit_count =
      std::min(source_syntax.argument_syntaxes.size(), normalized_arg_texts.size());
  for(size_t i = 0; i < explicit_count; ++i) {
    TemplateArgumentSyntax argument = source_syntax.argument_syntaxes[i];
    argument.text = normalized_arg_texts[i];
    out.push_back(argument);
  }
  for(size_t i = explicit_count; i < normalized_arg_texts.size(); ++i) {
    TemplateArgumentSyntax argument;
    argument.text = normalized_arg_texts[i];
    if(i < primary_parameters.size() &&
       primary_parameters[i].default_argument) {
      if(primary_parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
         !primary_parameters[i].default_argument->children.empty() &&
         primary_parameters[i].default_argument->children[0].kind == CppAstKind::type_id) {
        argument.type_id.reset(
            new CppAstNode(primary_parameters[i].default_argument->children[0]));
      }
      if(primary_parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
         !primary_parameters[i].default_argument->children.empty()) {
        argument.expression.reset(
            new CppAstNode(primary_parameters[i].default_argument->children[0]));
      }
      if(const TemplateIdSyntax * default_template_id =
             first_template_id_syntax_in_subtree(
                 *primary_parameters[i].default_argument)) {
        argument.template_id.reset(new TemplateIdSyntax(*default_template_id));
      }
    }
    out.push_back(argument);
  }
  return out;
}

size_t count_scope_operators(const string & text)
{
  size_t count = 0;
  for(size_t i = 0; i + 1 < text.size(); ++i) {
    if(text[i] == ':' && text[i + 1] == ':') {
      ++count;
      ++i;
    }
  }
  return count;
}

bool should_prefer_named_key_for_instantiation_identity(const TypePtr & type)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return false;
  }
  const string display_text = trim_space(strip_elaborated_type_prefix(type->named_display));
  const string key_text = trim_space(strip_elaborated_type_prefix(type->named_key));
  if(key_text.empty()) {
    return false;
  }
  if(key_text.find("template-parameter ") == 0 ||
     key_text.find("dependent ") == 0 ||
     key_text.find("builtin ") == 0) {
    return false;
  }
  if(key_text.find("__local_") != string::npos) {
    return true;
  }
  return count_scope_operators(key_text) > count_scope_operators(display_text);
}


ScopedTemplateUseLocation::ScopedTemplateUseLocation(const std::string & location)
  : active_(!location.empty())
{
  if(active_) {
    parser_trace::push_use_location(location);
  }
}

ScopedTemplateUseLocation::~ScopedTemplateUseLocation()
{
  if(active_) {
    parser_trace::pop_use_location();
  }
}

thread_local std::deque<ExactTemplateTypeLookupAnchor>
    exact_template_type_lookup_anchors_;
thread_local std::set<std::pair<std::string, std::string> >
    source_dependent_class_template_use_drops_;

ScopedExactTemplateTypeLookupAnchor::ScopedExactTemplateTypeLookupAnchor(
    const ExactTemplateTypeLookupAnchor & anchor)
  : active_(!anchor.template_text.empty() &&
            !anchor.identifier.empty())
{
  if(active_) {
    exact_template_type_lookup_anchors_.push_back(anchor);
  }
}

ScopedExactTemplateTypeLookupAnchor::~ScopedExactTemplateTypeLookupAnchor()
{
  if(active_) {
    exact_template_type_lookup_anchors_.pop_back();
  }
}

ScopedSuppressedTemplateUseLocation::ScopedSuppressedTemplateUseLocation(
    bool active)
  : active_(active)
{
  if(active_) {
    parser_trace::push_use_location("\x1d");
  }
}

ScopedSuppressedTemplateUseLocation::~ScopedSuppressedTemplateUseLocation()
{
  if(active_) {
    parser_trace::pop_use_location();
  }
}

std::string compact_lookup_text(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(std::isspace(static_cast<unsigned char>(text[i]))) {
      continue;
    }
    out.push_back(text[i]);
  }
  return out;
}

bool identifier_token_match_at(const std::string & line,
                               std::size_t pos,
                               const std::string & identifier)
{
  if(identifier.empty() || pos + identifier.size() > line.size() ||
     line.compare(pos, identifier.size(), identifier) != 0) {
    return false;
  }
  const bool left_ok =
      pos == 0 ||
      !(std::isalnum(static_cast<unsigned char>(line[pos - 1])) ||
        line[pos - 1] == '_');
  const std::size_t after = pos + identifier.size();
  const bool right_ok =
      after >= line.size() ||
      !(std::isalnum(static_cast<unsigned char>(line[after])) ||
        line[after] == '_');
  return left_ok && right_ok;
}

std::string template_lookup_fragment_text(const std::string & lookup_name)
{
  const std::string trimmed = trim_space(lookup_name);
  if(trimmed.empty()) {
    return std::string();
  }
  if(trimmed.find('<') != std::string::npos &&
     trimmed.back() == '>') {
    return trimmed;
  }

  std::string remaining = trimmed;
  while(!remaining.empty()) {
    const std::string component = trim_space(unqualified_member_name(remaining));
    const std::string stripped =
        strip_trailing_top_level_template_arguments(component);
    if(stripped != component && !stripped.empty()) {
      return component;
    }
    const std::size_t split = semantic_utils::top_level_scope_split(remaining);
    if(split == std::string::npos) {
      break;
    }
    remaining = trim_space(remaining.substr(0, split));
  }
  return std::string();
}

std::string template_lookup_fragment_identifier(
    const std::string & template_fragment)
{
  if(template_fragment.empty()) {
    return std::string();
  }
  const std::string without_args =
      strip_trailing_top_level_template_arguments(trim_space(template_fragment));
  const std::string identifier = unqualified_member_name(without_args);
  return !identifier.empty() ? identifier : without_args;
}

bool node_has_template_id_qualifier_syntax(const CppAstNode & node)
{
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(!node.qualifier_template_id_syntaxes[i].name.name.empty()) {
      return true;
    }
  }
  return false;
}

const ExactTemplateTypeLookupAnchor * current_exact_template_type_lookup_anchor()
{
  for(std::size_t i = exact_template_type_lookup_anchors_.size(); i > 0; --i) {
    const ExactTemplateTypeLookupAnchor & anchor =
        exact_template_type_lookup_anchors_[i - 1];
    if(!anchor.template_text.empty() &&
       !anchor.identifier.empty()) {
      return &anchor;
    }
  }
  return nullptr;
}

bool exact_template_type_lookup_anchor_matches(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & normalized_name)
{
  if(anchor.location.empty() ||
     anchor.compact_key.empty() ||
     normalized_name.empty()) {
    return false;
  }
  return anchor.compact_key == compact_lookup_text(normalized_name);
}

bool exact_template_type_lookup_anchor_matches_syntax(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & normalized_name)
{
  if(anchor.compact_key.empty() ||
     normalized_name.empty()) {
    return false;
  }
  return anchor.compact_key == compact_lookup_text(normalized_name);
}

bool exact_template_type_lookup_anchor_matches_identifier(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & identifier)
{
  if(anchor.location.empty() ||
     anchor.identifier.empty() ||
     identifier.empty()) {
    return false;
  }
  const std::string anchor_unqualified =
      unqualified_member_name(anchor.identifier);
  const std::string identifier_unqualified =
      unqualified_member_name(identifier);
  return anchor.identifier == identifier ||
         (!anchor_unqualified.empty() &&
          anchor_unqualified == identifier) ||
         (!anchor_unqualified.empty() &&
          !identifier_unqualified.empty() &&
          anchor_unqualified == identifier_unqualified);
}

bool exact_template_type_lookup_anchor_matches_identifier_syntax(
    const ExactTemplateTypeLookupAnchor & anchor,
    const std::string & identifier)
{
  if(anchor.identifier.empty() ||
     identifier.empty()) {
    return false;
  }
  const std::string anchor_unqualified =
      unqualified_member_name(anchor.identifier);
  const std::string identifier_unqualified =
      unqualified_member_name(identifier);
  return anchor.identifier == identifier ||
         (!anchor_unqualified.empty() &&
          anchor_unqualified == identifier) ||
         (!anchor_unqualified.empty() &&
          !identifier_unqualified.empty() &&
          anchor_unqualified == identifier_unqualified);
}

bool exact_template_type_lookup_anchor_arg_texts(
    const std::string & normalized_name,
    const std::string & identifier,
    std::vector<std::string> & arg_texts)
{
  arg_texts.clear();
  const ExactTemplateTypeLookupAnchor * anchor =
      current_exact_template_type_lookup_anchor();
  if(!(anchor && !normalized_name.empty())) {
    return false;
  }
  if(!(exact_template_type_lookup_anchor_matches(*anchor, normalized_name) ||
       exact_template_type_lookup_anchor_matches_identifier(*anchor,
                                                            identifier)) ||
     !anchor->has_argument_list) {
    return false;
  }
  arg_texts = exact_template_type_lookup_anchor_texts(*anchor);
  return true;
}

const std::vector<std::string> & exact_template_type_lookup_anchor_texts(
    const ExactTemplateTypeLookupAnchor & anchor)
{
  return anchor.arg_texts_ref ? *anchor.arg_texts_ref : anchor.arg_texts;
}

const std::vector<TemplateArgumentSyntax> *
exact_template_type_lookup_anchor_syntaxes(
    const ExactTemplateTypeLookupAnchor & anchor)
{
  return anchor.arg_syntaxes_ref ? anchor.arg_syntaxes_ref : &anchor.arg_syntaxes;
}

bool exact_template_type_lookup_anchor_arg_texts_are_full_match(
    const std::string & normalized_name)
{
  const ExactTemplateTypeLookupAnchor * anchor =
      current_exact_template_type_lookup_anchor();
  return anchor &&
         anchor->has_argument_list &&
         exact_template_type_lookup_anchor_matches(*anchor, normalized_name);
}

const std::vector<TemplateArgumentSyntax> *
exact_template_type_lookup_anchor_arg_syntaxes(
    const std::string & normalized_name,
    const std::string & identifier)
{
  const ExactTemplateTypeLookupAnchor * anchor =
      current_exact_template_type_lookup_anchor();
  if(!(anchor && !normalized_name.empty())) {
    return nullptr;
  }
  if(!(exact_template_type_lookup_anchor_matches_syntax(*anchor, normalized_name) ||
     exact_template_type_lookup_anchor_matches_identifier_syntax(*anchor,
                                                                 identifier)) ||
     !anchor->has_argument_list) {
    return nullptr;
  }
  const std::vector<std::string> & arg_texts =
      exact_template_type_lookup_anchor_texts(*anchor);
  const std::vector<TemplateArgumentSyntax> * arg_syntaxes =
      exact_template_type_lookup_anchor_syntaxes(*anchor);
  if(!arg_syntaxes || arg_syntaxes->size() != arg_texts.size()) {
    return nullptr;
  }
  return arg_syntaxes;
}

bool source_template_id_args_are_arity_compatible(
    const std::vector<std::string> & source_arg_texts,
    const std::vector<TemplateArgument> & arguments)
{
  return source_arg_texts.size() <= arguments.size();
}

string function_template_decl_primary_location(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl)
{
  return semantic_trace::template_decl_primary_location(ctx, decl);
}

string function_template_decl_location_details(
    SemanticContext & ctx,
    const FunctionTemplateDecl * decl)
{
  return semantic_trace::template_decl_location_details(ctx, decl);
}

string function_template_decl_source_owner_name(const FunctionTemplateDecl * decl)
{
  return decl &&
             decl->declaring_scope &&
             decl->declaring_scope->class_info ?
         decl->declaring_scope->class_info->qualified_name :
         string("<none>");
}

bool function_template_trace_has_identity(
    const FunctionTemplateDecl * incoming_decl,
    const FunctionBinding * binding)
{
  return incoming_decl || (binding && binding->source_template);
}

bool function_binding_template_trace_has_source(const FunctionBinding * binding)
{
  return binding && binding->source_template;
}

void append_function_binding_template_trace_fields(
    ostream & out,
    const string & prefix,
    const FunctionBinding * binding)
{
  const FunctionTemplateDecl * decl = binding ? binding->source_template : nullptr;
  out << " " << prefix << "_key="
      << (binding ? binding->template_instantiation_key : string())
      << " " << prefix << "_source_template=" << static_cast<const void *>(decl)
      << " " << prefix << "_source_owner="
      << function_template_decl_source_owner_name(decl);
}

void append_function_template_decl_trace_fields(
    ostream & out,
    SemanticContext & ctx,
    const string & prefix,
    const FunctionTemplateDecl * decl,
    const string & key)
{
  out << " " << prefix << "_source_template=" << static_cast<const void *>(decl)
      << " " << prefix << "_key=" << key
      << " " << prefix << "_source_owner="
      << function_template_decl_source_owner_name(decl)
      << " " << prefix << "_decl_loc="
      << function_template_decl_primary_location(ctx, decl);
}

string function_binding_source_template_primary_location(
    SemanticContext & ctx,
    const FunctionBinding * binding)
{
  return function_template_decl_primary_location(
      ctx,
      binding ? binding->source_template : nullptr);
}

string function_binding_source_template_location_details(
    SemanticContext & ctx,
    const FunctionBinding * binding)
{
  return function_template_decl_location_details(
      ctx,
      binding ? binding->source_template : nullptr);
}

bool template_id_syntax_matches_identifier_text(
    const std::string & syntax_name,
    const std::string & identifier)
{
  if(syntax_name.empty() || identifier.empty()) {
    return false;
  }
  const std::string syntax_unqualified =
      unqualified_member_name(syntax_name);
  const std::string identifier_unqualified =
      unqualified_member_name(identifier);
  return syntax_name == identifier ||
         (!syntax_unqualified.empty() &&
          syntax_unqualified == identifier) ||
         (!syntax_unqualified.empty() &&
          !identifier_unqualified.empty() &&
          syntax_unqualified == identifier_unqualified);
}

bool template_id_syntax_matches_identifier(
    const TemplateIdSyntax & syntax,
    const std::string & identifier)
{
  return template_id_syntax_matches_identifier_text(syntax.name.name,
                                                   identifier);
}

const TemplateIdSyntax * template_id_syntax_for_anchor(
    const CppAstNode & node,
    const std::string & identifier);

void collect_template_id_syntaxes_for_anchor(
    const CppAstNode & node,
    const std::string & identifier,
    std::vector<const TemplateIdSyntax *> & out);

const TemplateIdSyntax * template_id_syntax_for_anchor(
    const TemplateIdSyntax & syntax,
    const std::string & identifier)
{
  if(template_id_syntax_matches_identifier(syntax, identifier)) {
    return &syntax;
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    const cpp_decl::TemplateArgumentSyntax & argument =
        syntax.argument_syntaxes[i];
    if(argument.template_id) {
      if(const TemplateIdSyntax * nested =
             template_id_syntax_for_anchor(*argument.template_id, identifier)) {
        return nested;
      }
    }
    if(argument.type_id) {
      if(const TemplateIdSyntax * nested =
             template_id_syntax_for_anchor(*argument.type_id, identifier)) {
        return nested;
      }
    }
    if(argument.expression) {
      if(const TemplateIdSyntax * nested =
             template_id_syntax_for_anchor(*argument.expression, identifier)) {
        return nested;
      }
    }
  }
  return nullptr;
}

const TemplateIdSyntax * template_id_syntax_for_anchor(
    const CppAstNode & node,
    const std::string & identifier)
{
  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    if(const TemplateIdSyntax * nested =
           template_id_syntax_for_anchor(*syntax, identifier)) {
      return nested;
    }
  }
  if(const CppAstNode * conversion_type_id =
         cppast_conversion_type_id_syntax(node)) {
    if(const TemplateIdSyntax * nested =
           template_id_syntax_for_anchor(*conversion_type_id, identifier)) {
      return nested;
    }
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    const TemplateIdSyntax & syntax = node.qualifier_template_id_syntaxes[i];
    if(const TemplateIdSyntax * nested =
           template_id_syntax_for_anchor(syntax, identifier)) {
      return nested;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const TemplateIdSyntax * syntax =
           template_id_syntax_for_anchor(node.children[i], identifier)) {
      return syntax;
    }
  }
  return nullptr;
}

void collect_template_id_syntaxes_for_anchor(
    const TemplateIdSyntax & syntax,
    const std::string & identifier,
    std::vector<const TemplateIdSyntax *> & out)
{
  if(template_id_syntax_matches_identifier(syntax, identifier)) {
    out.push_back(&syntax);
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    const cpp_decl::TemplateArgumentSyntax & argument =
        syntax.argument_syntaxes[i];
    if(argument.template_id) {
      collect_template_id_syntaxes_for_anchor(*argument.template_id,
                                              identifier,
                                              out);
    }
    if(argument.type_id) {
      collect_template_id_syntaxes_for_anchor(*argument.type_id,
                                              identifier,
                                              out);
    }
    if(argument.expression) {
      collect_template_id_syntaxes_for_anchor(*argument.expression,
                                              identifier,
                                              out);
    }
  }
}

void collect_template_id_syntaxes_for_anchor(
    const std::vector<TemplateArgumentSyntax> & arguments,
    const std::string & identifier,
    std::vector<const TemplateIdSyntax *> & out)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    const TemplateArgumentSyntax & argument = arguments[i];
    if(argument.template_id) {
      collect_template_id_syntaxes_for_anchor(*argument.template_id,
                                              identifier,
                                              out);
    }
    if(argument.type_id) {
      collect_template_id_syntaxes_for_anchor(*argument.type_id,
                                              identifier,
                                              out);
    }
    if(argument.expression) {
      collect_template_id_syntaxes_for_anchor(*argument.expression,
                                              identifier,
                                              out);
    }
  }
}

void collect_template_id_syntaxes_for_anchor(
    const CppAstNode & node,
    const std::string & identifier,
    std::vector<const TemplateIdSyntax *> & out)
{
  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    collect_template_id_syntaxes_for_anchor(*syntax, identifier, out);
  }
  if(const CppAstNode * conversion_type_id =
         cppast_conversion_type_id_syntax(node)) {
    collect_template_id_syntaxes_for_anchor(*conversion_type_id, identifier, out);
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    collect_template_id_syntaxes_for_anchor(
        node.qualifier_template_id_syntaxes[i],
        identifier,
        out);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_template_id_syntaxes_for_anchor(node.children[i], identifier, out);
  }
}

const TemplateIdSyntax * template_id_syntax_for_anchor_at_or_after_location(
    const template_api::TemplateWitnessContext & ctx,
    const CppAstNode & node,
    const std::string & identifier,
    const std::string & location)
{
  std::vector<const TemplateIdSyntax *> syntaxes;
  collect_template_id_syntaxes_for_anchor(node, identifier, syntaxes);
  if(syntaxes.empty()) {
    return nullptr;
  }

  const ParsedSourceLocation target =
      parse_source_location(
          template_api::normalize_template_witness_source_location(location));
  if(!target.valid) {
    return syntaxes.front();
  }

  const TemplateIdSyntax * best = nullptr;
  ParsedSourceLocation best_location;
  for(size_t i = 0; i < syntaxes.size(); ++i) {
    const std::string syntax_location =
        template_api::template_witness_detail::source_location_for_location_id(
            ctx,
            syntaxes[i]->source_location_id);
    const ParsedSourceLocation parsed =
        parse_source_location(
            template_api::normalize_template_witness_source_location(
                syntax_location));
    if(!parsed.valid ||
       parsed.file != target.file ||
       parsed.line != target.line ||
       parsed.column < target.column) {
      continue;
    }
    if(!best || parsed.column < best_location.column) {
      best = syntaxes[i];
      best_location = parsed;
    }
  }

  return best ? best : syntaxes.front();
}

const TemplateIdSyntax * template_id_syntax_for_anchor_at_or_after_location(
    const template_api::TemplateWitnessContext & ctx,
    const std::vector<TemplateArgumentSyntax> & arguments,
    const std::string & identifier,
    const std::string & location)
{
  std::vector<const TemplateIdSyntax *> syntaxes;
  collect_template_id_syntaxes_for_anchor(arguments, identifier, syntaxes);
  if(syntaxes.empty()) {
    return nullptr;
  }

  const ParsedSourceLocation target =
      parse_source_location(
          template_api::normalize_template_witness_source_location(location));
  if(!target.valid) {
    return syntaxes.front();
  }

  const TemplateIdSyntax * best = nullptr;
  ParsedSourceLocation best_location;
  for(size_t i = 0; i < syntaxes.size(); ++i) {
    const std::string syntax_location =
        template_api::template_witness_detail::source_location_for_location_id(
            ctx,
            syntaxes[i]->source_location_id);
    const ParsedSourceLocation parsed =
        parse_source_location(
            template_api::normalize_template_witness_source_location(
                syntax_location));
    if(!parsed.valid ||
       parsed.file != target.file ||
       parsed.line != target.line ||
       parsed.column < target.column) {
      continue;
    }
    if(!best || parsed.column < best_location.column) {
      best = syntaxes[i];
      best_location = parsed;
    }
  }

  return best ? best : syntaxes.front();
}

bool template_id_value_for_anchor(
    const CppAstNode & node,
    const std::string & identifier,
    QualifiedName & out_name,
    std::vector<std::string> & out_arg_texts)
{
  if(!node.value.empty()) {
    QualifiedName template_id;
    std::vector<std::string> arg_texts;
    if(semantic_utils::split_top_level_template_id_text(node.value,
                                                        template_id,
                                                        arg_texts) &&
       !arg_texts.empty() &&
       template_id_syntax_matches_identifier_text(qualified_name_syntax_text(template_id),
                                                  identifier)) {
      out_name = template_id;
      out_arg_texts = arg_texts;
      return true;
    }
  }
  if(const CppAstNode * conversion_type_id =
         cppast_conversion_type_id_syntax(node)) {
    if(template_id_value_for_anchor(*conversion_type_id,
                                    identifier,
                                    out_name,
                                    out_arg_texts)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(template_id_value_for_anchor(node.children[i],
                                    identifier,
                                    out_name,
                                    out_arg_texts)) {
      return true;
    }
  }
  return false;
}

bool template_argument_texts_mention_source_bindings(
    SemanticContext & ctx,
    Scope & scope,
    const std::vector<std::string> & arg_texts)
{
  for(size_t i = 0; i < arg_texts.size(); ++i) {
    if(ctx.text_mentions_template_placeholders(scope, arg_texts[i]) ||
       ctx.text_mentions_dependent_non_namespace_binding_names(scope, arg_texts[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_texts_mention_instantiated_class_local_type_aliases(
    Scope & scope,
    const std::vector<std::string> & arg_texts)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(!(current->class_info &&
         current->class_info->source_template &&
         !current->class_info->dependent_instantiation)) {
      continue;
    }
    for(auto it =
            current->named_types.begin();
        it != current->named_types.end();
        ++it) {
      const std::string & name = it->first;
      if(name.empty() ||
         !it->second ||
         current->template_bound_type_names.count(name) != 0 ||
         name == current->class_info->name ||
         name == current->class_info->source_template->name) {
        continue;
      }
      for(std::size_t i = 0; i < arg_texts.size(); ++i) {
        if(contains_identifier_token(arg_texts[i], name)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool text_mentions_identifier_token(const std::string & text,
                                    const std::string & identifier)
{
  if(text.empty() || identifier.empty()) {
    return false;
  }
  std::size_t pos = 0;
  while(pos < text.size()) {
    pos = text.find(identifier, pos);
    if(pos == std::string::npos) {
      return false;
    }
    if(identifier_token_match_at(text, pos, identifier)) {
      return true;
    }
    ++pos;
  }
  return false;
}

bool text_mentions_bare_identifier_token(const std::string & text,
                                         const std::string & identifier)
{
  if(text.empty() || identifier.empty()) {
    return false;
  }
  std::size_t pos = 0;
  while(pos < text.size()) {
    pos = text.find(identifier, pos);
    if(pos == std::string::npos) {
      return false;
    }
    if(identifier_token_match_at(text, pos, identifier)) {
      std::size_t after = pos + identifier.size();
      while(after < text.size() &&
            std::isspace(static_cast<unsigned char>(text[after]))) {
        ++after;
      }
      if(after >= text.size() || text[after] != '<') {
        return true;
      }
    }
    ++pos;
  }
  return false;
}

bool template_argument_texts_mention_parameters(
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < arg_texts.size(); ++i) {
    for(std::size_t j = 0; j < parameters.size(); ++j) {
      if(text_mentions_identifier_token(arg_texts[i], parameters[j].name)) {
        return true;
      }
      for(std::size_t k = 0; k < parameters[j].alternate_names.size(); ++k) {
        if(text_mentions_identifier_token(arg_texts[i],
                                          parameters[j].alternate_names[k])) {
          return true;
        }
      }
    }
  }
  return false;
}

bool template_argument_texts_mention_enclosing_source_template_parameters(
    Scope & scope,
    const std::vector<std::string> & arg_texts)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->class_info &&
       current->class_info->source_template &&
       template_argument_texts_mention_parameters(
           arg_texts,
           current->class_info->source_template->parameters)) {
      return true;
    }
    if(current->function &&
       current->function->source_template &&
       template_argument_texts_mention_parameters(
           arg_texts,
           current->function->source_template->parameters)) {
      return true;
    }
  }
  return false;
}

bool template_argument_texts_mention_template_bound_scope_names(
    Scope & scope,
    const std::vector<std::string> & arg_texts)
{
  const auto text_mentions_any_name =
      [&](const std::set<std::string> & names) -> bool
  {
    for(std::size_t i = 0; i < arg_texts.size(); ++i) {
      for(std::set<std::string>::const_iterator it = names.begin();
          it != names.end();
          ++it) {
        if(text_mentions_identifier_token(arg_texts[i], *it)) {
          return true;
        }
      }
    }
    return false;
  };
  for(Scope * current = &scope; current; current = current->parent) {
    if(text_mentions_any_name(current->template_bound_type_names) ||
       text_mentions_any_name(current->template_bound_type_pack_names) ||
       text_mentions_any_name(current->template_bound_value_names) ||
       text_mentions_any_name(current->template_bound_template_names)) {
      return true;
    }
  }
  return false;
}

bool template_argument_texts_mention_current_specialization_names(
    Scope & scope,
    const std::vector<std::string> & arg_texts)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(!(current->class_info && current->class_info->source_template)) {
      continue;
    }
    const std::string class_name = current->class_info->name;
    const std::string template_name = current->class_info->source_template->name;
    for(std::size_t i = 0; i < arg_texts.size(); ++i) {
      if(text_mentions_identifier_token(arg_texts[i], class_name) ||
         text_mentions_identifier_token(arg_texts[i], template_name)) {
        return true;
      }
    }
  }
  return false;
}

bool template_argument_texts_mention_bare_current_specialization_names(
    Scope & scope,
    const std::vector<std::string> & arg_texts)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(!(current->class_info && current->class_info->source_template)) {
      continue;
    }
    const std::string class_name = current->class_info->name;
    const std::string template_name = current->class_info->source_template->name;
    for(std::size_t i = 0; i < arg_texts.size(); ++i) {
      if(text_mentions_bare_identifier_token(arg_texts[i], class_name) ||
         text_mentions_bare_identifier_token(arg_texts[i], template_name)) {
        return true;
      }
    }
  }
  return false;
}

std::vector<std::string> current_specialization_parameter_texts(
    const ClassTemplateDecl & source_template)
{
  std::vector<std::string> texts;
  texts.reserve(source_template.parameters.size());
  for(std::size_t i = 0; i < source_template.parameters.size(); ++i) {
    std::string text = source_template.parameters[i].name.empty() ?
        std::string("$") + std::to_string(i + 1) :
        source_template.parameters[i].name;
    if(source_template.parameters[i].parameter_pack &&
       (text.size() < 3 || text.substr(text.size() - 3) != "...")) {
      text += "...";
    }
    texts.push_back(text);
  }
  return texts;
}

std::string join_template_source_arguments(const std::vector<std::string> & args)
{
  std::ostringstream out;
  for(std::size_t i = 0; i < args.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << args[i];
  }
  return out.str();
}

bool ast_contains_kind(const CppAstNode & node, CppAstKind kind)
{
  if(node.kind == kind) {
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_contains_kind(node.children[i], kind)) {
      return true;
    }
  }
  return false;
}

std::string normalize_current_specialization_pattern_arg_text(
    const std::string & text,
    const TemplateArgumentSyntax * syntax)
{
  if(!(syntax && syntax->type_id &&
       ast_contains_kind(*syntax->type_id, CppAstKind::parameter_clause))) {
    return text;
  }
  int angle_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '<') {
      ++angle_depth;
      continue;
    }
    if(text[i] == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(text[i] != '(' || angle_depth != 0) {
      continue;
    }
    std::size_t previous = i;
    while(previous > 0 &&
          std::isspace(static_cast<unsigned char>(text[previous - 1]))) {
      --previous;
    }
    if(previous == 0 || previous != i) {
      return text;
    }
    std::string out = text;
    out.insert(i, " ");
    return out;
  }
  return text;
}

std::vector<std::string> current_specialization_partial_arg_texts(
    const PartialClassTemplateSpecializationDecl & partial)
{
  std::vector<std::string> out = partial.arg_texts;
  const std::size_t count = std::min(out.size(), partial.arg_syntaxes.size());
  for(std::size_t i = 0; i < count; ++i) {
    out[i] = normalize_current_specialization_pattern_arg_text(
        out[i],
        &partial.arg_syntaxes[i]);
  }
  return out;
}

std::string current_specialization_source_replacement_text(
    const ClassInfo & info,
    const std::string & type_name)
{
  if(!info.source_template) {
    return std::string();
  }
  std::vector<std::string> arg_texts;
  if(info.is_explicit_specialization) {
    arg_texts = info.instantiation_arg_texts;
  } else if(info.template_output_node &&
            info.source_template->class_node &&
            info.template_output_node != info.source_template->class_node) {
    for(std::size_t i = 0;
        i < info.source_template->partial_specializations.size();
        ++i) {
      const PartialClassTemplateSpecializationDecl & partial =
          info.source_template->partial_specializations[i];
      if(partial.class_node == info.template_output_node &&
         !partial.arg_texts.empty()) {
        arg_texts = current_specialization_partial_arg_texts(partial);
        break;
      }
    }
  }
  if(arg_texts.empty() && !info.is_explicit_specialization) {
    arg_texts = current_specialization_parameter_texts(*info.source_template);
  }
  if(arg_texts.empty()) {
    return std::string();
  }
  return type_name + "<" + join_template_source_arguments(arg_texts) + ">";
}

std::string rewrite_current_specialization_names_in_source_text(
    Scope & scope,
    const std::string & source_text)
{
  if(source_text.empty()) {
    return source_text;
  }

  std::string out = source_text;
  for(Scope * current = &scope; current; current = current->parent) {
    ClassInfo * info = current->class_info;
    if(!info) {
      continue;
    }
    const std::string template_name =
        info->source_template ?
            info->source_template->name :
            strip_trailing_top_level_template_arguments(info->name);
    if(!template_name.empty()) {
      const std::string replacement =
          current_specialization_source_replacement_text(*info, template_name);
      if(!replacement.empty()) {
        bool changed = false;
        out = replace_identifier_token_text(out,
                                            template_name,
                                            replacement,
                                            changed);
      }
    }
    const std::string class_name =
        strip_trailing_top_level_template_arguments(info->name);
    if(!class_name.empty() && class_name != template_name) {
      const std::string replacement =
          current_specialization_source_replacement_text(*info, class_name);
      if(!replacement.empty()) {
        bool changed = false;
        out = replace_identifier_token_text(out,
                                            class_name,
                                            replacement,
                                            changed);
      }
    }
  }
  return out;
}

std::string current_specialization_source_binding_text(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateArgument & argument,
    const std::string & explicit_text)
{
  const std::string source_text = trim_space(explicit_text);
  if(source_text.empty() || source_text.find('<') != std::string::npos) {
    return std::string();
  }

  const std::string type_name =
      trim_space(strip_elaborated_type_prefix(source_text));
  const std::string unqualified_type_name =
      unqualified_member_name(type_name).empty() ?
          type_name :
          unqualified_member_name(type_name);
  for(Scope * current = &scope; current; current = current->parent) {
    ClassInfo * info = current->class_info;
    if(!info || !info->source_template) {
      continue;
    }
    if(unqualified_type_name != info->source_template->name &&
       unqualified_type_name != info->name) {
      continue;
    }
    return current_specialization_source_replacement_text(*info, type_name);
  }
  if(argument.kind == TemplateArgument::TA_TYPE && argument.type) {
    ClassInfo * info = ctx.class_info_for_type(argument.type);
    if(info && info->source_template && !info->instantiation_arg_texts.empty() &&
       (unqualified_type_name == info->source_template->name ||
        unqualified_type_name == info->name)) {
      return type_name + "<" +
          join_template_source_arguments(info->instantiation_arg_texts) + ">";
    }
  }
  for(Scope * current = &scope; current; current = current->parent) {
    std::map<std::string, ClassTemplateDecl *>::const_iterator found =
        current->class_templates.find(unqualified_type_name);
    if(found == current->class_templates.end() || !found->second) {
      continue;
    }
    const std::vector<std::string> arg_texts =
        current_specialization_parameter_texts(*found->second);
    if(arg_texts.empty()) {
      return std::string();
    }
    return type_name + "<" + join_template_source_arguments(arg_texts) + ">";
  }
  return std::string();
}

struct CurrentSpecializationSourceTokenReplacement
{
  std::size_t begin = 0;
  std::size_t end = 0;
  std::string text;
};

bool source_text_piece_needs_separator(const std::string & left,
                                       const std::string & right)
{
  if(left.empty() || right.empty()) {
    return false;
  }
  const char left_ch = left[left.size() - 1];
  const char right_ch = right[0];
  return (std::isalnum(static_cast<unsigned char>(left_ch)) ||
          left_ch == '_') &&
         (std::isalnum(static_cast<unsigned char>(right_ch)) ||
          right_ch == '_');
}

bool current_specialization_source_replacement_for_node(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    std::string & out)
{
  out.clear();
  if(node.value.empty()) {
    return false;
  }
  if(node.kind != CppAstKind::identifier &&
     node.kind != CppAstKind::id_expression &&
     node.kind != CppAstKind::type_name &&
     node.kind != CppAstKind::decl_specifier &&
     node.kind != CppAstKind::type_specifier) {
    return false;
  }
  TemplateArgument argument;
  argument.kind = TemplateArgument::TA_TYPE;
  argument.type = node.semantic_type;
  out = current_specialization_source_binding_text(ctx,
                                                   scope,
                                                   argument,
                                                   node.value);
  return !out.empty();
}

void collect_current_specialization_source_replacements(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    std::vector<CurrentSpecializationSourceTokenReplacement> & replacements);

void collect_current_specialization_source_replacements(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateArgumentSyntax & syntax,
    std::vector<CurrentSpecializationSourceTokenReplacement> & replacements);

void collect_current_specialization_source_replacements(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateIdSyntax & syntax,
    std::vector<CurrentSpecializationSourceTokenReplacement> & replacements)
{
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       syntax.argument_syntaxes[i],
                                                       replacements);
  }
}

void collect_current_specialization_source_replacements(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateArgumentSyntax & syntax,
    std::vector<CurrentSpecializationSourceTokenReplacement> & replacements)
{
  if(syntax.template_id) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       *syntax.template_id,
                                                       replacements);
  }
  if(syntax.type_id) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       *syntax.type_id,
                                                       replacements);
  }
  if(syntax.expression) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       *syntax.expression,
                                                       replacements);
  }
}

void collect_current_specialization_source_replacements(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    std::vector<CurrentSpecializationSourceTokenReplacement> & replacements)
{
  const template_api::TemplateWitnessContext witness_context =
      ctx.template_witness_context();
  if(witness_context.token_sequence &&
     cpp_decl::has_valid_node_span(*witness_context.token_sequence, node)) {
    std::string replacement_text;
    if(current_specialization_source_replacement_for_node(ctx,
                                                          scope,
                                                          node,
                                                          replacement_text)) {
      CurrentSpecializationSourceTokenReplacement replacement;
      replacement.begin = node.token_start;
      replacement.end = node.token_end;
      replacement.text = replacement_text;
      replacements.push_back(replacement);
      return;
    }
  }

  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       *syntax,
                                                       replacements);
  }
  if(const CppAstNode * conversion_type_id =
         cppast_conversion_type_id_syntax(node)) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       *conversion_type_id,
                                                       replacements);
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    collect_current_specialization_source_replacements(
        ctx,
        scope,
        node.qualifier_type_syntaxes[i],
        replacements);
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    collect_current_specialization_source_replacements(
        ctx,
        scope,
        node.qualifier_template_id_syntaxes[i],
        replacements);
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_current_specialization_source_replacements(ctx,
                                                       scope,
                                                       node.children[i],
                                                       replacements);
  }
}

std::vector<CurrentSpecializationSourceTokenReplacement>
filter_current_specialization_source_replacements_for_root(
    const CppAstNode & root,
    std::vector<CurrentSpecializationSourceTokenReplacement> replacements)
{
  std::sort(replacements.begin(),
            replacements.end(),
            [](const CurrentSpecializationSourceTokenReplacement & lhs,
               const CurrentSpecializationSourceTokenReplacement & rhs)
            {
              if(lhs.begin != rhs.begin) {
                return lhs.begin < rhs.begin;
              }
              return lhs.end > rhs.end;
            });
  std::vector<CurrentSpecializationSourceTokenReplacement> filtered;
  std::size_t covered_until = root.token_start;
  for(std::size_t i = 0; i < replacements.size(); ++i) {
    if(replacements[i].text.empty() ||
       replacements[i].begin < root.token_start ||
       replacements[i].end > root.token_end ||
       replacements[i].begin < covered_until ||
       replacements[i].end <= replacements[i].begin) {
      continue;
    }
    filtered.push_back(replacements[i]);
    covered_until = replacements[i].end;
  }
  return filtered;
}

bool apply_current_specialization_source_replacements(
    const template_api::TemplateWitnessContext & witness_context,
    const CppAstNode & root,
    std::vector<CurrentSpecializationSourceTokenReplacement> replacements,
    std::string & out)
{
  out.clear();
  if(!witness_context.token_sequence ||
     !cpp_decl::has_valid_node_span(*witness_context.token_sequence, root)) {
    return false;
  }
  if(!root.value.empty()) {
    const std::string span_text =
        callsemantic_internal::spaced_token_span_text(
            *witness_context.token_sequence,
            root.token_start,
            root.token_end);
    if(compact_source_spelling_key(span_text) !=
       compact_source_spelling_key(root.value)) {
      return false;
    }
  }
  replacements =
      filter_current_specialization_source_replacements_for_root(root,
                                                                replacements);
  if(replacements.empty()) {
    return false;
  }

  std::size_t replacement_index = 0;
  std::string previous_piece;
  const auto append_piece =
      [&](const std::string & piece) -> void
  {
    if(piece.empty()) {
      return;
    }
    if(!out.empty() &&
       source_text_piece_needs_separator(previous_piece, piece)) {
      out += ' ';
    }
    out += piece;
    previous_piece = piece;
  };

  for(std::size_t token_index = root.token_start;
      token_index < root.token_end;) {
    while(replacement_index < replacements.size() &&
          replacements[replacement_index].end <= token_index) {
      ++replacement_index;
    }
    if(replacement_index < replacements.size() &&
       replacements[replacement_index].begin == token_index) {
      append_piece(replacements[replacement_index].text);
      token_index = replacements[replacement_index].end;
      ++replacement_index;
      continue;
    }

    const RecogToken & token = witness_context.token_sequence->peek(token_index);
    if(token.is_eof()) {
      break;
    }
    append_piece(callsemantic_internal::recog_token_text_for_span(token));
    ++token_index;
  }
  return !out.empty();
}

bool current_specialization_source_argument_semantic_text_from_template_id(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateIdSyntax & syntax,
    std::string & out);

bool current_specialization_source_argument_semantic_text(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateArgumentSyntax & syntax,
    std::string & out);

bool node_contains_template_id_syntax(const CppAstNode & node)
{
  if(node.template_id_syntax || !node.qualifier_template_id_syntaxes.empty()) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     node_contains_template_id_syntax(*node.conversion_type_id_syntax)) {
    return true;
  }
  if(node.base_type_syntax &&
     node_contains_template_id_syntax(*node.base_type_syntax)) {
    return true;
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(node_contains_template_id_syntax(node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(node_contains_template_id_syntax(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_contains_template_id(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.template_id) {
    return true;
  }
  if(syntax.type_id && node_contains_template_id_syntax(*syntax.type_id)) {
    return true;
  }
  if(syntax.expression && node_contains_template_id_syntax(*syntax.expression)) {
    return true;
  }
  return false;
}

bool current_specialization_source_argument_semantic_text_from_node(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    std::string & out)
{
  if(current_specialization_source_replacement_for_node(ctx, scope, node, out)) {
    return true;
  }
  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    if(current_specialization_source_argument_semantic_text_from_template_id(
           ctx,
           scope,
           *syntax,
           out)) {
      return true;
    }
  }
  if(!node.qualifier_template_id_syntaxes.empty() &&
     node.qualified_name_syntax) {
    bool changed = false;
    std::ostringstream text;
    if(node.qualified_name_syntax->rooted) {
      text << "::";
    }
    for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
      if(i != 0) {
        text << "::";
      }
      std::string qualifier_text;
      if(current_specialization_source_argument_semantic_text_from_template_id(
             ctx,
             scope,
             node.qualifier_template_id_syntaxes[i],
             qualifier_text)) {
        changed = true;
      } else {
        qualifier_text =
            template_id_syntax_text_preserving_spacing(
                node.qualifier_template_id_syntaxes[i]);
      }
      text << qualifier_text;
    }
    if(!node.qualified_name_syntax->name.empty()) {
      text << "::" << node.qualified_name_syntax->name;
    }
    if(changed) {
      out = text.str();
      return true;
    }
  }

  std::vector<CurrentSpecializationSourceTokenReplacement> replacements;
  collect_current_specialization_source_replacements(ctx,
                                                     scope,
                                                     node,
                                                     replacements);
  return apply_current_specialization_source_replacements(
      ctx.template_witness_context(),
      node,
      replacements,
      out);
}

bool current_specialization_source_argument_semantic_text_from_template_id(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateIdSyntax & syntax,
    std::string & out)
{
  bool changed = false;
  std::ostringstream text;
  text << qualified_name_syntax_text(syntax.name) << "<";
  const std::size_t count =
      std::max(syntax.arguments.size(), syntax.argument_syntaxes.size());
  for(std::size_t i = 0; i < count; ++i) {
    if(i != 0) {
      text << ", ";
    }
    std::string arg_text;
    if(i < syntax.argument_syntaxes.size()) {
      changed |= current_specialization_source_argument_semantic_text(
          ctx,
          scope,
          syntax.argument_syntaxes[i],
          arg_text);
    }
    if(arg_text.empty()) {
      arg_text = i < syntax.arguments.size() ? syntax.arguments[i] :
                                               std::string();
    }
    text << arg_text;
  }
  text << ">";
  if(!changed) {
    return false;
  }
  out = text.str();
  return true;
}

bool current_specialization_source_argument_semantic_text(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateArgumentSyntax & syntax,
    std::string & out)
{
  out.clear();
  bool changed = false;
  if(syntax.template_id) {
    changed = current_specialization_source_argument_semantic_text_from_template_id(
        ctx,
        scope,
        *syntax.template_id,
        out);
  } else if(syntax.expression) {
    changed = current_specialization_source_argument_semantic_text_from_node(
        ctx,
        scope,
        *syntax.expression,
        out);
  } else if(syntax.type_id) {
    changed = current_specialization_source_argument_semantic_text_from_node(
        ctx,
        scope,
        *syntax.type_id,
        out);
  }
  if(changed &&
     syntax.pack_expansion &&
     (out.size() < 3 || out.substr(out.size() - 3) != "...")) {
    out += "...";
  }
  return changed;
}

void mark_occurrence_current_specialization_argument(
    semantic_source_use::SourceTemplateIdOccurrence * occurrence,
    std::size_t index,
    const std::string & semantic_text)
{
  if(!occurrence || index >= occurrence->arguments.size()) {
    return;
  }
  occurrence->arguments[index].current_specialization = true;
  if(!semantic_text.empty()) {
    occurrence->arguments[index].semantic_text = semantic_text;
  }
  occurrence->has_current_specialization_argument = true;
}

std::string join_alias_pack_binding_arguments(
    const std::vector<std::string> & pack_arguments)
{
  std::ostringstream out;
  out << "<";
  for(std::size_t i = 0; i < pack_arguments.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << pack_arguments[i];
  }
  out << ">";
  return out.str();
}

void set_alias_pack_binding_argument(
    template_api::TemplateWitnessSourceBinding & binding,
    std::size_t index,
    const std::string & text)
{
  if(!binding.pack_binding) {
    return;
  }
  if(binding.pack_arguments.size() <= index) {
    binding.pack_arguments.resize(index + 1);
  }
  binding.pack_arguments[index] = text;
}

void rebuild_alias_pack_binding_text(
    template_api::TemplateWitnessSourceBinding & binding)
{
  if(!binding.pack_binding || binding.pack_arguments.empty()) {
    return;
  }
  if(!binding.pack_aggregate && binding.pack_arguments.size() == 1) {
    binding.arg = binding.pack_arguments[0];
    return;
  }
  binding.arg = join_alias_pack_binding_arguments(binding.pack_arguments);
}

bool current_specialization_alias_binding_argument_text(
    SemanticContext & ctx,
    Scope & use_scope,
    const TemplateArgument & argument,
    const std::string & explicit_argument_text,
    const TemplateArgumentSyntax * explicit_argument_syntax,
    std::string & out)
{
  if(argument.kind == TemplateArgument::TA_TYPE) {
    const std::string rewritten =
        current_specialization_source_binding_text(ctx,
                                                   use_scope,
                                                   argument,
                                                   explicit_argument_text);
    if(!rewritten.empty()) {
      out = rewritten;
      return true;
    }
  }

  const std::string rewritten_source =
      rewrite_current_specialization_names_in_source_text(
          use_scope,
          trim_space(explicit_argument_text));
  if(rewritten_source != trim_space(explicit_argument_text)) {
    out = rewritten_source;
    return true;
  }

  std::vector<std::string> arg_texts;
  arg_texts.push_back(explicit_argument_text);
  if(!template_argument_texts_mention_bare_current_specialization_names(
         use_scope,
         arg_texts)) {
    return false;
  }
  if(!explicit_argument_syntax ||
     template_argument_syntax_contains_template_id(*explicit_argument_syntax)) {
    return false;
  }

  out = template_api::template_witness_source_argument_text(ctx, argument);
  return !out.empty();
}

void rewrite_current_specialization_alias_binding_texts(
    SemanticContext & ctx,
    Scope & use_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<std::string> & explicit_argument_texts,
    const std::vector<TemplateArgumentSyntax> * explicit_argument_syntaxes,
    std::vector<template_api::TemplateWitnessSourceBinding> & bindings,
    semantic_source_use::SourceTemplateIdOccurrence * occurrence)
{
  std::size_t arg_index = 0;
  std::size_t explicit_index = 0;
  for(std::size_t i = 0; i < parameters.size() && i < bindings.size(); ++i) {
    if(parameters[i].parameter_pack) {
      std::size_t trailing_non_pack = 0;
      for(std::size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameters[j].parameter_pack) {
          ++trailing_non_pack;
        }
      }
      if(arguments.size() < arg_index + trailing_non_pack) {
        break;
      }
      const std::size_t pack_end = arguments.size() - trailing_non_pack;
      const std::size_t pack_count = pack_end - arg_index;
      const std::size_t remaining_explicit =
          explicit_argument_texts.size() > explicit_index ?
              explicit_argument_texts.size() - explicit_index :
              0;
      const std::size_t explicit_pack_count =
          std::min(pack_count,
                   remaining_explicit > trailing_non_pack ?
                       remaining_explicit - trailing_non_pack :
	                       0);
	      if(explicit_pack_count == 1) {
	        std::string rewritten;
	        const TemplateArgumentSyntax * syntax =
	            explicit_argument_syntaxes &&
	                    explicit_index < explicit_argument_syntaxes->size() ?
	                &(*explicit_argument_syntaxes)[explicit_index] :
	                nullptr;
	        if(current_specialization_alias_binding_argument_text(
	             ctx,
	             use_scope,
	             arguments[arg_index],
	             explicit_argument_texts[explicit_index],
	             syntax,
	             rewritten)) {
          bindings[i].arg = rewritten;
          set_alias_pack_binding_argument(bindings[i], 0, rewritten);
          rebuild_alias_pack_binding_text(bindings[i]);
          mark_occurrence_current_specialization_argument(occurrence,
                                                          explicit_index,
                                                          bindings[i].arg);
        }
      } else if(explicit_pack_count > 1) {
        bool changed_any = false;
        for(std::size_t j = 0; j < explicit_pack_count; ++j) {
          const std::size_t argument_index = arg_index + j;
          const std::size_t source_index = explicit_index + j;
          if(argument_index >= arguments.size() ||
             source_index >= explicit_argument_texts.size()) {
            break;
          }
          std::string rewritten;
          const TemplateArgumentSyntax * syntax =
              explicit_argument_syntaxes &&
                      source_index < explicit_argument_syntaxes->size() ?
                  &(*explicit_argument_syntaxes)[source_index] :
                  nullptr;
          if(!current_specialization_alias_binding_argument_text(
                 ctx,
                 use_scope,
                 arguments[argument_index],
                 explicit_argument_texts[source_index],
                 syntax,
                 rewritten)) {
            continue;
          }
          set_alias_pack_binding_argument(bindings[i], j, rewritten);
          mark_occurrence_current_specialization_argument(occurrence,
                                                          source_index,
                                                          rewritten);
          changed_any = true;
        }
        if(changed_any) {
          rebuild_alias_pack_binding_text(bindings[i]);
        }
      }
      arg_index = pack_end;
      explicit_index += explicit_pack_count;
      continue;
    }

    if(arg_index >= arguments.size()) {
      break;
    }
    if(explicit_index < explicit_argument_texts.size()) {
      std::string rewritten;
      const TemplateArgumentSyntax * syntax =
          explicit_argument_syntaxes &&
                  explicit_index < explicit_argument_syntaxes->size() ?
              &(*explicit_argument_syntaxes)[explicit_index] :
              nullptr;
      if(current_specialization_alias_binding_argument_text(
             ctx,
             use_scope,
             arguments[arg_index],
             explicit_argument_texts[explicit_index],
             syntax,
             rewritten)) {
        bindings[i].arg = rewritten;
        mark_occurrence_current_specialization_argument(occurrence,
                                                        explicit_index,
                                                        bindings[i].arg);
      }
      ++explicit_index;
    }
    ++arg_index;
  }
}


std::string template_public_use_location_or(const std::string & fallback)
{
  if(parser_trace::use_location_suppressed()) {
    return std::string();
  }
  const std::string current = parser_trace::current_use_location();
  return !current.empty() ? current : fallback;
}

ClassTemplateDecl * class_template_origin_decl(ClassInfo * info)
{
  return info ? info->source_template : nullptr;
}

const ClassTemplateDecl * class_template_origin_decl(const ClassInfo * info)
{
  return info ? info->source_template : nullptr;
}

bool class_has_template_origin(const ClassInfo * info)
{
  return class_template_origin_decl(info) != nullptr;
}

const vector<TemplateArgument> * class_template_origin_arguments(const ClassInfo * info)
{
  return info && !info->instantiation_arguments.empty() ?
      &info->instantiation_arguments :
      nullptr;
}

bool class_has_explicit_template_origin(const ClassInfo * info)
{
  return info &&
         info->is_explicit_specialization &&
         class_has_template_origin(info);
}

const CppAstNode * class_template_origin_class_node(const ClassInfo * info)
{
  const ClassTemplateDecl * origin = class_template_origin_decl(info);
  return origin ? origin->class_node : nullptr;
}

const CppAstNode * find_member_class_template_declaration_node(
    const ClassTemplateDecl * class_template)
{
  if(!(class_template &&
       class_template->declaring_scope &&
       class_template->declaring_scope)) {
    return nullptr;
  }
  std::vector<const CppAstNode *> owner_class_nodes;
  for(Scope * current = class_template->declaring_scope; current; current = current->parent) {
    if(!current->class_info) {
      continue;
    }
    const ClassInfo * owner_info = current->class_info;
    if(owner_info->source_template && owner_info->source_template->class_node) {
      owner_class_nodes.push_back(owner_info->source_template->class_node);
    }
    if(owner_info->template_output_node) {
      owner_class_nodes.push_back(owner_info->template_output_node);
    }
    if(owner_info->class_node) {
      owner_class_nodes.push_back(owner_info->class_node);
    }
  }
  if(owner_class_nodes.empty()) {
    return nullptr;
  }
  const std::function<const CppAstNode *(const CppAstNode &)> search =
      [&](const CppAstNode & current) -> const CppAstNode *
  {
    for(size_t i = 0; i < current.children.size(); ++i) {
      const CppAstNode & child = current.children[i];
      if(child.kind == CppAstKind::template_declaration) {
        for(size_t j = 0; j < child.children.size(); ++j) {
          const CppAstNode & inner = child.children[j];
          if((inner.kind == CppAstKind::class_specifier ||
              inner.kind == CppAstKind::class_forward_declaration) &&
             inner.value == class_template->name) {
            return &inner;
          }
        }
      }
      if(const CppAstNode * found = search(child)) {
        return found;
      }
    }
    return nullptr;
  };
  std::set<const CppAstNode *> seen;
  for(size_t i = 0; i < owner_class_nodes.size(); ++i) {
    if(!(owner_class_nodes[i] && seen.insert(owner_class_nodes[i]).second)) {
      continue;
    }
    if(const CppAstNode * found = search(*owner_class_nodes[i])) {
      return found;
    }
  }
  return nullptr;
}

const CppAstNode * find_non_template_member_class_declaration_node(
    const ClassInfo * info)
{
  if(!(info &&
       info->enclosing_scope &&
       info->enclosing_scope->class_info &&
       info->enclosing_scope->class_info->source_template &&
       info->enclosing_scope->class_info->source_template->class_node)) {
    return nullptr;
  }

  const ClassInfo * owner_info = info->enclosing_scope->class_info;
  const CppAstNode * owner_node = owner_info->source_template->class_node;
  const std::function<const CppAstNode *(const CppAstNode &)> search =
      [&](const CppAstNode & current) -> const CppAstNode *
  {
    for(size_t i = 0; i < current.children.size(); ++i) {
      const CppAstNode & child = current.children[i];
      if((child.kind == CppAstKind::class_specifier ||
          child.kind == CppAstKind::class_forward_declaration) &&
         child.value == info->name) {
        return &child;
      }
      if(const CppAstNode * found = search(child)) {
        return found;
      }
    }
    return nullptr;
  };
  return search(*owner_node);
}

bool env_flag_enabled(const char * name)
{
  const char * value = std::getenv(name);
  return value != nullptr && *value != '\0' && string(value) != "0";
}


}  // namespace callsemantic
