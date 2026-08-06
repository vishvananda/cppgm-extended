#include "callsemantic/class_template_reference.h"

#include "callsemantic/source_location_utils.h"
#include "callsemantic/template_source_utils.h"
#include "class_template_mangle_info.h"
#include "callsemantic_internal.h"
#include "cpp_decl_ast.h"
#include "parser_trace.h"
#include "semantic_builtins.h"
#include "semantic_context.h"
#include "semantic_errors.h"
#include "semantic_hotspot.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"
#include "template_argument_semantics.h"
#include "template_audit.h"
#include "template_instantiation.h"
#include "template_model.h"
#include "template_services.h"
#include "types.h"
#include "witness_api.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace callsemantic {
namespace {

using namespace callsemantic_internal;
using namespace cpp_decl;
using namespace semantic_model;
using template_model::TemplateArgument;
using template_model::TemplateParameterInfo;
using semantic_utils::strip_elaborated_type_prefix;
using semantic_utils::strip_trailing_top_level_template_arguments;
using semantic_utils::trim_space;
using semantic_utils::unqualified_member_name;

bool lazy_class_template_references_enabled()
{
  return true;
}

bool raw_class_template_reference_cache_enabled()
{
  const char * value = std::getenv("CPPGM_RAW_CLASS_TEMPLATE_REFERENCE_CACHE");
  return !value || !*value || std::string(value) != "0";
}

const PartialClassTemplateSpecializationDecl * selected_partial_specialization(
    const ClassTemplateDecl & decl,
    const ClassInfo & info)
{
  if(!info.template_output_node || info.template_output_node == decl.class_node) {
    return nullptr;
  }
  for(size_t i = 0; i < decl.partial_specializations.size(); ++i) {
    if(decl.partial_specializations[i].class_node == info.template_output_node) {
      return &decl.partial_specializations[i];
    }
  }
  return nullptr;
}

void record_class_template_binding_state(
    ClassInfo & info,
    const vector<TemplateArgument> & arguments,
    const map<string, size_t> * pack_sizes,
    bool reuse_primary_arguments)
{
  if(reuse_primary_arguments) {
    reuse_primary_class_instantiation_binding_arguments(info);
  } else {
    set_class_instantiation_binding_arguments(info, arguments);
  }
  if(pack_sizes) {
    info.instantiation_binding_pack_sizes = *pack_sizes;
  } else {
    info.instantiation_binding_pack_sizes.clear();
  }
}

void bind_declaring_owner_template_arguments_into_scope(SemanticContext & ctx,
                                                        Scope & target,
                                                        const Scope * declaring_scope)
{
  ClassInfo * declared_owner =
      declaring_scope ? declaring_scope->class_info : nullptr;
  if(!declared_owner ||
     !declared_owner->source_template ||
     declared_owner->instantiation_arguments.empty()) {
    return;
  }

  const vector<TemplateParameterInfo> * parameters =
      &declared_owner->source_template->parameters;
  const vector<TemplateArgument> * arguments =
      &declared_owner->instantiation_arguments;
  const map<string, size_t> * pack_sizes = nullptr;
  if(declared_owner->has_instantiation_binding_arguments) {
    arguments = &class_instantiation_binding_arguments(*declared_owner);
    pack_sizes = &declared_owner->instantiation_binding_pack_sizes;
  }
  if(const PartialClassTemplateSpecializationDecl * partial =
         selected_partial_specialization(*declared_owner->source_template,
                                         *declared_owner)) {
    parameters = &partial->parameters;
  }

  template_api::binding::bind_template_arguments_into_scope(
      ctx,
      target,
      *parameters,
      *arguments,
      pack_sizes);
}

string join_hotspot_arg_texts(const vector<string> & arg_texts)
{
  ostringstream out;
  for(size_t i = 0; i < arg_texts.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << trim_space(arg_texts[i]);
  }
  return out.str();
}

bool binding_arg_drops_resolved_array_cv(const string & binding_arg,
                                         const string & resolved_arg)
{
  if(binding_arg.find('[') == string::npos ||
     resolved_arg.find('[') == string::npos) {
    return false;
  }
  const string binding_key = remove_space_chars(binding_arg);
  const string resolved_key = remove_space_chars(resolved_arg);
  if(binding_key == resolved_key) {
    return false;
  }
  return (resolved_key.find("const") != string::npos &&
          binding_key.find("const") == string::npos) ||
         (resolved_key.find("volatile") != string::npos &&
          binding_key.find("volatile") == string::npos);
}

class ClassTemplateReference
{
public:
  ClassTemplateReference(SemanticContext & ctx_in,
                         const ClassTemplateReferenceCallbacks & callbacks_in)
    : ctx(ctx_in), callbacks(callbacks_in)
  {}

  operator SemanticContext &() { return ctx; }
  operator const SemanticContext &() const { return ctx; }

  string value_binding_witness_entity(const ValueBinding & binding) const
  {
    if(binding.declaration_scope &&
       binding.declaration_scope->class_info &&
       !binding.declaration_scope->class_info->qualified_name.empty()) {
      return binding.declaration_scope->class_info->qualified_name + "::" +
          binding.name;
    }
    return binding.name;
  }

  string value_binding_witness_decl_location(const ValueBinding & binding) const
  {
    if(!binding.declaration_node) {
      return string();
    }
    string location =
        source_location_for_name_in_node(*binding.declaration_node,
                                         binding.name);
    if(location.empty()) {
      location = source_location_for_node(*binding.declaration_node);
    }
    return template_api::normalize_template_witness_source_location(location);
  }

  bool source_location_matches_witness_file(
      const ParsedSourceLocation & parsed,
      const std::string & file) const
  {
    const ParsedSourceLocation parsed_file =
        parse_source_location(
            template_api::normalize_template_witness_source_location(
                file + ":1:1"));
    const std::string normalized_file =
        parsed_file.valid ? parsed_file.file : file;
    return normalized_file == parsed.file;
  }

  bool source_location_is_template_header_context_for_witness(
      const std::string & location) const
  {
    const template_api::TemplateWitnessContext witness_context =
        template_witness_context();
    if(!witness::enabled(witness_context) ||
       !witness_context.session ||
       location.empty()) {
      return false;
    }

    const ParsedSourceLocation parsed =
        parse_source_location(
            template_api::normalize_template_witness_source_location(location));
    if(!parsed.valid) {
      return false;
    }

    const std::vector<template_api::TemplateWitnessTemplateHeaderContext> &
        contexts = witness_context.session->template_header_contexts;
    for(std::size_t i = 0; i < contexts.size(); ++i) {
      const template_api::TemplateWitnessTemplateHeaderContext & context =
          contexts[i];
      if(!source_location_matches_witness_file(parsed, context.file) ||
         parsed.line < context.begin_line ||
         parsed.line > context.end_line) {
        continue;
      }
      if(parsed.line == context.begin_line &&
         parsed.column > 0 &&
         parsed.column < context.begin_column) {
        continue;
      }
      if(parsed.line == context.end_line &&
         context.end_column > 0 &&
         parsed.column > context.end_column) {
        continue;
      }
      return true;
    }
    return false;
  }

  bool source_location_is_template_body_context_for_witness(
      const std::string & location) const
  {
    const template_api::TemplateWitnessContext witness_context =
        template_witness_context();
    if(!witness::enabled(witness_context) ||
       !witness_context.session ||
       location.empty()) {
      return false;
    }

    const ParsedSourceLocation parsed =
        parse_source_location(
            template_api::normalize_template_witness_source_location(location));
    if(!parsed.valid) {
      return false;
    }

    const std::vector<template_api::TemplateWitnessSourceRange> & ranges =
        witness_context.session->template_body_ranges;
    for(std::size_t i = 0; i < ranges.size(); ++i) {
      if(!source_location_matches_witness_file(parsed, ranges[i].file) ||
         parsed.line < ranges[i].begin_line ||
         parsed.line > ranges[i].end_line) {
        continue;
      }
      if(parsed.line == ranges[i].begin_line &&
         ranges[i].first_body_column > 1 &&
         parsed.column > 0 &&
         parsed.column < ranges[i].first_body_column) {
        continue;
      }
      return true;
    }
    return false;
  }

  bool class_template_decl_is_member_template(
      const ClassTemplateDecl & decl) const
  {
    return decl.declaring_scope && decl.declaring_scope->class_info;
  }

  void collect_template_argument_value_entities(
      Scope & scope,
      const CppAstNode & node,
      vector<string> & out)
  {
    if(node.kind == CppAstKind::id_expression) {
      const ValueBinding * binding = ctx.lookup_value(scope, node.value);
      if(binding &&
         binding->kind == ValueBinding::VK_VARIABLE &&
         binding->owner_class &&
         template_api::value_or_owner_has_template_identity(binding)) {
        const string entity = value_binding_witness_entity(*binding);
        if(!entity.empty() &&
           std::find(out.begin(), out.end(), entity) == out.end()) {
          out.push_back(entity);
        }
      }
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_template_argument_value_entities(scope, node.children[i], out);
    }
  }

  void collect_template_argument_value_decl_locations(
      Scope & scope,
      const CppAstNode & node,
      vector<string> & out)
  {
    if(node.kind == CppAstKind::id_expression) {
      const ValueBinding * binding = ctx.lookup_value(scope, node.value);
      if(binding &&
         binding->kind == ValueBinding::VK_VARIABLE &&
         binding->owner_class &&
         template_api::value_or_owner_has_template_identity(binding)) {
        const string location = value_binding_witness_decl_location(*binding);
        if(!location.empty() &&
           std::find(out.begin(), out.end(), location) == out.end()) {
          out.push_back(location);
        }
      }
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_template_argument_value_decl_locations(scope, node.children[i], out);
    }
  }

  vector<string> template_argument_value_entities(
      Scope & scope,
      const TemplateArgumentSyntax & syntax)
  {
    vector<string> out;
    if(syntax.expression) {
      collect_template_argument_value_entities(scope, *syntax.expression, out);
    }
    return out;
  }

  vector<string> template_argument_value_entities(
      Scope & scope,
      const TemplateArgument & argument)
  {
    vector<string> out;
    if(argument.expression) {
      collect_template_argument_value_entities(scope, *argument.expression, out);
    }
    return out;
  }

  vector<string> template_argument_value_decl_locations(
      Scope & scope,
      const TemplateArgumentSyntax & syntax)
  {
    vector<string> out;
    if(syntax.expression) {
      collect_template_argument_value_decl_locations(scope,
                                                    *syntax.expression,
                                                    out);
    }
    return out;
  }

  vector<string> template_argument_value_decl_locations(
      Scope & scope,
      const TemplateArgument & argument)
  {
    vector<string> out;
    if(argument.expression) {
      collect_template_argument_value_decl_locations(scope,
                                                    *argument.expression,
                                                    out);
    }
    return out;
  }

  bool expression_mentions_source_template_value_parameter(
      Scope & fallback_scope,
      const CppAstNode & node)
  {
    if(node.kind == CppAstKind::id_expression) {
      const ValueBinding * binding = ctx.lookup_value(fallback_scope,
                                                      node.value);
      if(binding && binding->kind == ValueBinding::VK_PARAMETER) {
        return true;
      }
      std::vector<std::string> texts;
      texts.push_back(node.value);
      if(template_argument_texts_mention_source_bindings(*this,
                                                         fallback_scope,
                                                         texts) ||
         template_argument_texts_mention_enclosing_source_template_parameters(
             fallback_scope,
             texts)) {
        return true;
      }
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(expression_mentions_source_template_value_parameter(
             fallback_scope,
             node.children[i])) {
        return true;
      }
    }
    return false;
  }

  bool value_binding_depends_on_source_template_value_parameter(
      Scope & fallback_scope,
      const ValueBinding & binding)
  {
    if(binding.dependent_template_value) {
      return true;
    }
    if(!binding.constant_initializer) {
      return false;
    }
    Scope & initializer_scope =
        binding.constant_initializer_scope ? *binding.constant_initializer_scope :
                                             fallback_scope;
    return expression_mentions_source_template_value_parameter(
        initializer_scope,
        *binding.constant_initializer);
  }

  bool value_binding_is_current_specialization_member(
      Scope & scope,
      const ValueBinding & binding)
  {
    if(!binding.owner_class || !binding.owner_class->source_template) {
      return false;
    }
    for(Scope * current = &scope; current; current = current->parent) {
      ClassInfo * info = current->class_info;
      if(!info || !info->source_template) {
        continue;
      }
      if(binding.owner_class == info ||
         binding.owner_class->source_template == info->source_template) {
        return true;
      }
    }
    return false;
  }

  bool named_type_is_current_specialization_member_type(
      Scope & scope,
      const std::string & name)
  {
    if(name.empty()) {
      return false;
    }
    for(Scope * current = &scope; current; current = current->parent) {
      ClassInfo * info = current->class_info;
      if(!info || !info->source_template) {
        continue;
      }
      auto found =
          current->named_types.find(name);
      if(found == current->named_types.end() || !found->second) {
        continue;
      }
      if(found->second->kind == Type::TK_NAMED &&
         found->second->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER) {
        return false;
      }
      return true;
    }
    return false;
  }

  bool text_mentions_current_specialization_member_type(
      Scope & scope,
      const std::string & text)
  {
    const callsemantic_internal::IdentifierTokenSet identifiers =
        callsemantic_internal::collect_identifier_tokens(text);
    for(callsemantic_internal::IdentifierTokenSet::InternedName interned_name :
        identifiers.names) {
      if(named_type_is_current_specialization_member_type(scope,
                                                         *interned_name)) {
        return true;
      }
    }
    return false;
  }

  bool class_template_use_is_current_specialization(
      Scope & scope,
      const ClassTemplateDecl & decl)
  {
    const auto class_info_matches = [&](ClassInfo * info) -> bool
    {
      if(!info) {
        return false;
      }
      const std::string current_name =
          strip_trailing_top_level_template_arguments(
              class_output_qualified_name(*info));
      return info->source_template == &decl ||
             unqualified_member_name(current_name) == decl.name;
    };
    for(Scope * current = &scope; current; current = current->parent) {
      if(class_info_matches(current->class_info)) {
        return true;
      }
      if(current->function &&
         (class_info_matches(current->function->owner_class) ||
          class_info_matches(current->function->lexical_access_class))) {
        return true;
      }
    }
    return false;
  }

  bool class_use_matches_current_function_result(
      Scope & scope,
      const ClassInfo * resolved_info,
      FunctionBinding * source_function = nullptr) const
  {
    FunctionBinding * function = source_function ? source_function :
        semantic_lookup::current_function_scope(scope);
    if(!resolved_info ||
       !resolved_info->type ||
       !function) {
      return false;
    }
    const TypePtr target_type = strip_top_level_cv(resolved_info->type);
    const auto function_result_matches = [&](const TypePtr & type) -> bool
    {
      const TypePtr function_type = strip_top_level_cv(type);
      if(!function_type || function_type->kind != Type::TK_FUNCTION) {
        return false;
      }
      const TypePtr result_type = strip_top_level_cv(function_type->inner);
      if(type_equals(result_type, target_type)) {
        return true;
      }
      TypePtr resolved_result;
      return template_api::type::resolve_instantiated_dependent_type(
                 ctx,
                 scope,
                 result_type,
                 resolved_result) &&
             type_equals(strip_top_level_cv(resolved_result), target_type);
    };
    return function_result_matches(function->declared_type) ||
           function_result_matches(function->type);
  }

  bool class_use_matches_current_conversion_result(
      Scope & scope,
      const ClassInfo * resolved_info,
      FunctionBinding * source_function = nullptr) const
  {
    FunctionBinding * function = source_function ? source_function :
        semantic_lookup::current_function_scope(scope);
    return function &&
           ctx.is_conversion_function_name(function->name) &&
           class_use_matches_current_function_result(scope,
                                                     resolved_info,
                                                     function);
  }


  bool template_argument_mentions_current_specialization_member(
      Scope & scope,
      const CppAstNode & node)
  {
    if(node.kind == CppAstKind::id_expression) {
      const ValueBinding * binding = ctx.lookup_value(scope, node.value);
      if(binding && value_binding_is_current_specialization_member(scope,
                                                                   *binding)) {
        return true;
      }
      if(named_type_is_current_specialization_member_type(scope, node.value)) {
        return true;
      }
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(template_argument_mentions_current_specialization_member(
             scope,
             node.children[i])) {
        return true;
      }
    }
    return false;
  }

  bool template_argument_mentions_current_specialization_member(
      Scope & scope,
      const TemplateArgumentSyntax & syntax)
  {
    if(syntax.expression &&
       template_argument_mentions_current_specialization_member(
           scope,
           *syntax.expression)) {
      return true;
    }
    if(syntax.type_id &&
       template_argument_mentions_current_specialization_member(
           scope,
           *syntax.type_id)) {
      return true;
    }
    return !syntax.text.empty() &&
           text_mentions_current_specialization_member_type(scope,
                                                            syntax.text);
  }

  bool template_argument_syntax_mentions_source_template_context(
      Scope & scope,
      const TemplateArgumentSyntax & syntax)
  {
    if(syntax.dependent || syntax.pack_expansion) {
      return true;
    }
    if(!syntax.text.empty()) {
      std::vector<std::string> texts;
      texts.push_back(syntax.text);
      if(template_argument_texts_mention_source_bindings(*this,
                                                         scope,
                                                         texts) ||
         template_argument_texts_mention_template_bound_scope_names(scope,
                                                                   texts) ||
         template_argument_texts_mention_enclosing_source_template_parameters(
             scope,
             texts)) {
        return true;
      }
    }
    if(syntax.template_id) {
      for(size_t i = 0; i < syntax.template_id->argument_syntaxes.size(); ++i) {
        if(template_argument_syntax_mentions_source_template_context(
               scope,
               syntax.template_id->argument_syntaxes[i])) {
          return true;
        }
      }
      for(size_t i = syntax.template_id->argument_syntaxes.size();
          i < syntax.template_id->arguments.size();
          ++i) {
        std::vector<std::string> texts;
        texts.push_back(syntax.template_id->arguments[i]);
        if(template_argument_texts_mention_source_bindings(*this,
                                                           scope,
                                                           texts) ||
           template_argument_texts_mention_template_bound_scope_names(scope,
                                                                     texts) ||
           template_argument_texts_mention_enclosing_source_template_parameters(
               scope,
               texts)) {
          return true;
        }
      }
    }
    return false;
  }

  bool template_argument_syntaxes_mention_source_template_context(
      Scope & scope,
      const std::vector<TemplateArgumentSyntax> * syntaxes)
  {
    if(!syntaxes) {
      return false;
    }
    for(size_t i = 0; i < syntaxes->size(); ++i) {
      if(template_argument_syntax_mentions_source_template_context(scope,
                                                                  (*syntaxes)[i])) {
        return true;
      }
    }
    return false;
  }

  const vector<TemplateParameterInfo> *
  enclosing_source_template_parameters_for_argument_mangling(
      Scope & scope,
      const std::vector<TemplateArgumentSyntax> * syntaxes)
  {
    if(!syntaxes ||
       !template_argument_syntaxes_mention_source_template_context(scope,
                                                                  syntaxes)) {
      return nullptr;
    }
    for(Scope * current = &scope; current; current = current->parent) {
      if(current->function &&
         current->function->source_template &&
         !current->function->source_template->parameters.empty()) {
        return &current->function->source_template->parameters;
      }
      if(current->class_info &&
         current->class_info->source_template &&
         !current->class_info->source_template->parameters.empty()) {
        return &current->class_info->source_template->parameters;
      }
    }
    return nullptr;
  }

  std::string normalized_inferred_template_parameter_name(
      const std::string & raw)
  {
    std::string name = trim_space(raw);
    if(name.empty()) {
      return std::string();
    }

    static const char * semantic_prefixes[] = {
        "template-parameter ",
        "typename "
    };
    bool stripped = true;
    while(stripped) {
      stripped = false;
      name = trim_space(name);
      for(std::size_t i = 0;
          i < sizeof(semantic_prefixes) / sizeof(semantic_prefixes[0]);
          ++i) {
        const std::string prefix = semantic_prefixes[i];
        if(name.compare(0, prefix.size(), prefix) == 0) {
          name = trim_space(name.substr(prefix.size()));
          stripped = true;
        }
      }
    }

    name = trim_space(strip_elaborated_type_prefix(name));
    return is_identifier_text(name) ? name : std::string();
  }

  void add_inferred_dependent_mangle_parameter(
      std::vector<TemplateParameterInfo> & parameters,
      std::set<std::string> & seen_names,
      TemplateParameterInfo::Kind kind,
      const std::string & raw_name,
      const TypePtr & value_type,
      bool parameter_pack)
  {
    const std::string name =
        normalized_inferred_template_parameter_name(raw_name);
    if(name.empty() || seen_names.count(name) != 0) {
      return;
    }

    TemplateParameterInfo parameter;
    parameter.kind = kind;
    parameter.name = name;
    parameter.parameter_pack = parameter_pack;
    if(kind == TemplateParameterInfo::TP_TYPE) {
      parameter.placeholder_key = std::string("template-parameter ") + name;
    } else if(kind == TemplateParameterInfo::TP_NON_TYPE) {
      parameter.value_type = value_type;
    }
    parameters.push_back(parameter);
    seen_names.insert(name);
  }

  std::string direct_identifier_expression_name(const CppAstNode * expression)
  {
    if(!expression || !expression->children.empty()) {
      return std::string();
    }
    if(expression->kind != CppAstKind::id_expression &&
       expression->kind != CppAstKind::identifier) {
      return std::string();
    }
    return normalized_inferred_template_parameter_name(expression->value);
  }

  std::string dependent_value_argument_identifier(
      const TemplateArgument & argument,
      const TemplateArgumentSyntax * syntax)
  {
    const ValueBinding * value_binding = argument.rare().value_binding;
    if(value_binding) {
      std::string name = normalized_inferred_template_parameter_name(
          value_binding->name);
      if(!name.empty()) {
        return name;
      }
      name = normalized_inferred_template_parameter_name(
          value_binding->non_type_template_argument_text);
      if(!name.empty()) {
        return name;
      }
    }
    std::string name = direct_identifier_expression_name(argument.expression.get());
    if(!name.empty()) {
      return name;
    }
    if(argument.source_syntax) {
      name = direct_identifier_expression_name(
          argument.source_syntax->expression.get());
      if(!name.empty()) {
        return name;
      }
    }
    if(syntax) {
      name = direct_identifier_expression_name(syntax->expression.get());
      if(!name.empty()) {
        return name;
      }
    }
    return std::string();
  }

  void collect_inferred_dependent_mangle_parameters_from_argument(
      const TemplateArgument & argument,
      const TemplateArgumentSyntax * syntax,
      std::vector<TemplateParameterInfo> & parameters,
      std::set<std::string> & seen_names,
      std::set<const Type *> & visiting)
  {
    switch(argument.kind) {
    case TemplateArgument::TA_TYPE:
      collect_inferred_dependent_mangle_parameters_from_type(argument.type,
                                                             parameters,
                                                             seen_names,
                                                             visiting);
      return;

    case TemplateArgument::TA_VALUE:
    {
      collect_inferred_dependent_mangle_parameters_from_type(argument.type,
                                                             parameters,
                                                             seen_names,
                                                             visiting);
      if(!argument.dependent) {
        return;
      }
      const std::string name =
          dependent_value_argument_identifier(argument, syntax);
      const bool parameter_pack =
          (argument.source_syntax && argument.source_syntax->pack_expansion) ||
          (syntax && syntax->pack_expansion);
      add_inferred_dependent_mangle_parameter(
          parameters,
          seen_names,
          TemplateParameterInfo::TP_NON_TYPE,
          name,
          argument.type,
          parameter_pack);
      return;
    }

    case TemplateArgument::TA_CLASS_TEMPLATE:
    case TemplateArgument::TA_ALIAS_TEMPLATE:
      return;
    }
  }

  void collect_inferred_dependent_mangle_parameters_from_dependent_argument(
      const DependentAliasTemplateArgumentSyntax & argument,
      std::vector<TemplateParameterInfo> & parameters,
      std::set<std::string> & seen_names,
      std::set<const Type *> & visiting)
  {
    collect_inferred_dependent_mangle_parameters_from_type(argument.type,
                                                           parameters,
                                                           seen_names,
                                                           visiting);
  }

  void collect_inferred_dependent_mangle_parameters_from_type(
      const TypePtr & type,
      std::vector<TemplateParameterInfo> & parameters,
      std::set<std::string> & seen_names,
      std::set<const Type *> & visiting)
  {
    if(!type) {
      return;
    }
    const Type * raw = type.get();
    if(visiting.count(raw) != 0) {
      return;
    }
    visiting.insert(raw);

    switch(type->kind) {
    case Type::TK_NAMED: {
      const Type::NamedRareMetadata & rare = type->named_rare();
      if(type->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER) {
        add_inferred_dependent_mangle_parameter(
            parameters,
            seen_names,
            TemplateParameterInfo::TP_TYPE,
            type->named_semantic_payload.empty() ?
                named_type_display_text(type) :
                type->named_semantic_payload,
            TypePtr(),
            false);
        visiting.erase(raw);
        return;
      }
      if(rare.named_class_template_specialization_mangle_info) {
        const ClassTemplateSpecializationMangleInfo & info =
            *rare.named_class_template_specialization_mangle_info;
        for(std::size_t i = 0; i < info.arguments.size(); ++i) {
          const TemplateArgumentSyntax * syntax =
              i < info.argument_syntaxes.size() ? &info.argument_syntaxes[i] :
                                                   nullptr;
          collect_inferred_dependent_mangle_parameters_from_argument(
              info.arguments[i],
              syntax,
              parameters,
              seen_names,
              visiting);
        }
      }
      for(std::size_t i = 0;
          i < rare.named_dependent_alias_arguments.size();
          ++i) {
        collect_inferred_dependent_mangle_parameters_from_dependent_argument(
            rare.named_dependent_alias_arguments[i],
            parameters,
            seen_names,
            visiting);
      }
      for(std::size_t i = 0;
          i < rare.named_dependent_class_arguments.size();
          ++i) {
        collect_inferred_dependent_mangle_parameters_from_dependent_argument(
            rare.named_dependent_class_arguments[i],
            parameters,
            seen_names,
            visiting);
      }
      for(std::size_t i = 0;
          i < rare.named_dependent_template_template_arguments.size();
          ++i) {
        collect_inferred_dependent_mangle_parameters_from_dependent_argument(
            rare.named_dependent_template_template_arguments[i],
            parameters,
            seen_names,
            visiting);
      }
      collect_inferred_dependent_mangle_parameters_from_type(
          rare.named_dependent_qualified_owner,
          parameters,
          seen_names,
          visiting);
      collect_inferred_dependent_mangle_parameters_from_type(
          rare.named_member_owner_type,
          parameters,
          seen_names,
          visiting);
      break;
    }

    case Type::TK_CV:
    case Type::TK_ATOMIC:
    case Type::TK_POINTER:
    case Type::TK_BLOCK_POINTER:
    case Type::TK_LVALUE_REFERENCE:
    case Type::TK_RVALUE_REFERENCE:
    case Type::TK_ARRAY:
      collect_inferred_dependent_mangle_parameters_from_type(type->inner,
                                                             parameters,
                                                             seen_names,
                                                             visiting);
      break;

    case Type::TK_MEMBER_POINTER:
      collect_inferred_dependent_mangle_parameters_from_type(type->owner,
                                                             parameters,
                                                             seen_names,
                                                             visiting);
      collect_inferred_dependent_mangle_parameters_from_type(type->inner,
                                                             parameters,
                                                             seen_names,
                                                             visiting);
      break;

    case Type::TK_FUNCTION:
      collect_inferred_dependent_mangle_parameters_from_type(type->inner,
                                                             parameters,
                                                             seen_names,
                                                             visiting);
      for(std::size_t i = 0; i < type->params.size(); ++i) {
        collect_inferred_dependent_mangle_parameters_from_type(type->params[i],
                                                               parameters,
                                                               seen_names,
                                                               visiting);
      }
      break;

    case Type::TK_FUNDAMENTAL:
      break;
    }

    visiting.erase(raw);
  }

  bool infer_dependent_argument_mangle_parameters(
      const std::vector<TemplateArgument> & arguments,
      const std::vector<TemplateArgumentSyntax> * syntaxes,
      std::vector<TemplateParameterInfo> & out)
  {
    out.clear();
    std::set<std::string> seen_names;
    std::set<const Type *> visiting;
    for(std::size_t i = 0; i < arguments.size(); ++i) {
      const TemplateArgumentSyntax * syntax =
          syntaxes && i < syntaxes->size() ? &(*syntaxes)[i] : nullptr;
      collect_inferred_dependent_mangle_parameters_from_argument(arguments[i],
                                                                 syntax,
                                                                 out,
                                                                 seen_names,
                                                                 visiting);
    }
    return !out.empty();
  }

  const vector<TemplateParameterInfo> *
  dependent_argument_mangle_parameters_for_argument_mangling(
      Scope & scope,
      const std::vector<TemplateArgument> & arguments,
      const std::vector<TemplateArgumentSyntax> * syntaxes,
      std::vector<TemplateParameterInfo> & inferred_parameters)
  {
    const vector<TemplateParameterInfo> * parameters =
        enclosing_source_template_parameters_for_argument_mangling(scope,
                                                                   syntaxes);
    if(parameters) {
      return parameters;
    }
    return infer_dependent_argument_mangle_parameters(arguments,
                                                      syntaxes,
                                                      inferred_parameters) ?
        &inferred_parameters :
        nullptr;
  }

  bool template_parameter_list_covers(
      const std::vector<TemplateParameterInfo> & existing,
      const std::vector<TemplateParameterInfo> & needed)
  {
    for(std::size_t i = 0; i < needed.size(); ++i) {
      const TemplateParameterInfo & required = needed[i];
      bool found = false;
      for(std::size_t j = 0; j < existing.size(); ++j) {
        const TemplateParameterInfo & candidate = existing[j];
        if(candidate.kind == required.kind &&
           candidate.name == required.name) {
          found = true;
          break;
        }
      }
      if(!found) {
        return false;
      }
    }
    return true;
  }

  bool fast_existing_class_template_mangle_context_current(
      Scope & scope,
      const std::vector<TemplateArgument> & arguments,
      const std::vector<TemplateArgumentSyntax> * syntaxes,
      const ClassInfo & info)
  {
    std::vector<TemplateParameterInfo> inferred_parameters;
    const std::vector<TemplateParameterInfo> * needed =
        dependent_argument_mangle_parameters_for_argument_mangling(
            scope,
            arguments,
            syntaxes,
            inferred_parameters);
    if(!needed || needed->empty()) {
      return true;
    }
    std::shared_ptr<const ClassTemplateSpecializationMangleInfo> existing =
        info.type ?
            named_type_class_template_specialization_mangle_info_const(info.type) :
            std::shared_ptr<const ClassTemplateSpecializationMangleInfo>();
    return existing &&
           template_parameter_list_covers(existing->mangle_parameters, *needed);
  }

  bool template_argument_syntaxes_mention_current_specialization_member(
      Scope & scope,
      const std::vector<TemplateArgumentSyntax> * syntaxes)
  {
    if(!syntaxes) {
      return false;
    }
    for(size_t i = 0; i < syntaxes->size(); ++i) {
      if(template_argument_mentions_current_specialization_member(scope,
                                                                  (*syntaxes)[i])) {
        return true;
      }
    }
    return false;
  }

  bool template_argument_mentions_current_specialization_member(
      Scope & scope,
      const TemplateArgument & argument)
  {
    if(argument.expression &&
       template_argument_mentions_current_specialization_member(
           scope,
           *argument.expression)) {
      return true;
    }
    return !argument.text.empty() &&
           text_mentions_current_specialization_member_type(scope,
                                                            argument.text);
  }

  bool raw_reference_cache_allowed(const ClassTemplateDecl & decl) const
  {
    (void)decl;
    return raw_class_template_reference_cache_enabled() &&
           !parser_trace::enabled("template.resolve") &&
           !witness::source_capture_enabled(template_witness_context());
  }

  std::string raw_reference_cache_key(const ClassTemplateDecl & decl,
                                      Scope & use_scope,
                                      const vector<string> & arg_texts) const
  {
    std::size_t reserve = 64;
    for(size_t i = 0; i < arg_texts.size(); ++i) {
      reserve += arg_texts[i].size() + 24;
    }
    std::string key;
    key.reserve(reserve);
    key += std::to_string(decl.specialization_epoch);
    key.push_back('@');
    for(const Scope * scope = &use_scope; scope; scope = scope->parent) {
      key += std::to_string(scope->instance_id);
      key.push_back('.');
      key += std::to_string(scope->binding_fingerprint_epoch);
      key.push_back('/');
    }
    key.push_back('|');
    for(size_t i = 0; i < arg_texts.size(); ++i) {
      key += std::to_string(arg_texts[i].size());
      key.push_back(':');
      key += arg_texts[i];
      key.push_back(';');
    }
    return key;
  }

  bool raw_reference_cache_info_usable(ClassTemplateDecl & decl,
                                       ClassInfo * info) const
  {
    return info &&
           info->source_template == &decl &&
           !info->reentrant_primary_selection &&
           !info->template_instantiation_in_progress &&
           !info->full_member_collection_in_progress &&
           !info->reference_member_collection_in_progress &&
           fast_existing_class_template_output_usable(decl, *info);
  }

  bool fast_existing_class_template_selection_current(
      const ClassTemplateDecl & decl,
      const string & key,
      const ClassInfo & info) const
  {
    if(info.reentrant_primary_selection) {
      return false;
    }
    map<string, ClassTemplateSpecializationDecl>::const_iterator explicit_found =
        decl.explicit_specializations.find(key);
    if(explicit_found != decl.explicit_specializations.end()) {
      return info.is_explicit_specialization &&
             info.template_output_node == explicit_found->second.class_node;
    }

    if(info.is_explicit_specialization) {
      return false;
    }

    if(info.instantiation_specialization_epoch == decl.specialization_epoch) {
      return true;
    }

    // Partial specializations can affect any key, so keep the fast path only
    // for templates whose current selection must still be the primary.
    return decl.partial_specializations.empty();
  }

  bool fast_existing_class_template_output_usable(
      const ClassTemplateDecl & decl,
      const ClassInfo & info) const
  {
    if(!info.template_output_node) {
      return false;
    }
    if(info.template_output_node->kind != CppAstKind::class_forward_declaration) {
      return true;
    }
    return callbacks.class_template_declarations_complete &&
           callbacks.class_template_declarations_complete() &&
           info.template_output_node == decl.class_node &&
           info.instantiation_specialization_epoch == decl.specialization_epoch;
  }

  void remember_raw_reference_cache(ClassTemplateDecl & decl,
                                    const std::string & cache_key,
                                    ClassInfo * info) const
  {
    if(cache_key.empty() ||
       !raw_reference_cache_info_usable(decl, info)) {
      return;
    }
    decl.fast_reference_cache[cache_key] = info;
  }

  bool is_fast_template_argument_lookup_text(
      const string & text,
      const TemplateArgumentSyntax * syntax)
  {
    const string trimmed = trim_space(text);
    if(trimmed.empty()) {
      return false;
    }

    const string stripped = strip_elaborated_type_prefix(trimmed);
    if(is_identifier_text(stripped)) {
      return true;
    }
    if(stripped == "true" || stripped == "false") {
      return true;
    }
    if(!stripped.empty() &&
       std::isdigit(static_cast<unsigned char>(stripped[0]))) {
      unsigned long long value = 0;
      std::string ud_suffix;
      try {
        return classify_int(stripped, value, ud_suffix) != FT_VOID &&
               ud_suffix.empty();
      } catch(const std::logic_error &) {
      }
    }

    const string unqualified = trim_space(unqualified_member_name(stripped));
    if(unqualified != stripped && is_identifier_text(unqualified)) {
      return true;
    }

    if(stripped.find('<') == string::npos || stripped[stripped.size() - 1] != '>') {
      return false;
    }
    const string template_head = strip_trailing_top_level_template_arguments(stripped);
    if(template_head == stripped || template_head.empty()) {
      return false;
    }
    if(is_identifier_text(template_head)) {
      return true;
    }
    return syntax && syntax->template_id && !syntax->template_id->name.name.empty();
  }

  bool try_fast_existing_class_template_instantiation(ClassTemplateDecl & decl,
                                                      Scope & use_scope,
                                                      const vector<string> & arg_texts,
                                                      const vector<TemplateArgumentSyntax> * arg_syntaxes,
                                                      vector<TemplateArgument> & arguments,
                                                      string & key,
                                                      ClassInfo *& info)
  {
    info = nullptr;
    arguments.clear();
    key.clear();
    const auto unresolved = [&]() -> bool
    {
      info = nullptr;
      arguments.clear();
      key.clear();
      return false;
    };
    const bool lazy_references = lazy_class_template_references_enabled();
    if(decl.instantiations.empty() &&
       (!lazy_references || decl.reference_instantiations.empty())) {
      return false;
    }

    template_argument_semantics::ExpandedTemplateArgumentInputs expanded_inputs;
    template_api::with_template_services(
        ctx,
        [&](template_api::TemplateServices & services)
        {
          expanded_inputs =
              template_argument_semantics::expand_template_argument_inputs(
                  services, use_scope, arg_texts, arg_syntaxes);
          return true;
        });
    const vector<string> & expanded_texts = expanded_inputs.texts;

    const bool trailing_pack =
        !decl.parameters.empty() && decl.parameters.back().parameter_pack;
    if((!trailing_pack && expanded_texts.size() != decl.parameters.size()) ||
       (trailing_pack && expanded_texts.size() + 1 < decl.parameters.size())) {
      return false;
    }

    for(size_t i = 0; i < expanded_texts.size(); ++i) {
      if(!is_fast_template_argument_lookup_text(expanded_texts[i],
                                                expanded_inputs.syntax_for(i))) {
        return false;
      }
      if(arg_syntaxes &&
         expanded_texts[i].find('<') != string::npos &&
         !expanded_inputs.syntax_for(i)) {
        return false;
      }
    }

    Scope bound_scope(&use_scope, "", false);
    size_t text_index = 0;
    for(size_t i = 0; i < decl.parameters.size(); ++i) {
      if(decl.parameters[i].parameter_pack) {
        if(i + 1 != decl.parameters.size()) {
          return false;
        }
        while(text_index < expanded_texts.size()) {
          TemplateArgument arg;
          if(!resolve_template_argument(use_scope,
                                        bound_scope,
                                        decl.parameters[i],
                                        expanded_texts[text_index],
                                        expanded_inputs.syntax_for(text_index),
                                        arg)) {
            return unresolved();
          }
          arguments.push_back(arg);
          ++text_index;
        }
        break;
      }

      if(text_index >= expanded_texts.size()) {
        return unresolved();
      }

      TemplateArgument arg;
      if(!resolve_template_argument(use_scope,
                                    bound_scope,
                                    decl.parameters[i],
                                    expanded_texts[text_index],
                                    expanded_inputs.syntax_for(text_index),
                                    arg)) {
        return unresolved();
      }
      arguments.push_back(arg);
      bind_single_template_argument_into_scope(bound_scope, decl.parameters[i], arg);
      ++text_index;
    }

    canonicalize_simple_dependent_argument_texts(arguments);
    note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_key_builds);
    key = template_instantiation::class_template_argument_key_for_instantiation(
        ctx, decl, arguments);
    auto found = decl.instantiations.find(key);
    if(found != decl.instantiations.end()) {
      info = found->second;
      return info != nullptr;
    }
    if(lazy_references) {
      auto reference_found =
          decl.reference_instantiations.find(key);
      if(reference_found != decl.reference_instantiations.end()) {
        info = reference_found->second;
        return info != nullptr;
      }
    }
    return false;
  }

  std::string template_argument_anchor_identifier_for_source(
      const std::string & text)
  {
    const std::string trimmed = trim_space(text);
    std::string without_args = trimmed;
    const std::string stripped = strip_trailing_top_level_template_arguments(trimmed);
    if(stripped != trimmed && !stripped.empty()) {
      without_args = stripped;
      without_args = trim_space(without_args);
    }
    const std::string identifier = unqualified_member_name(without_args);
    if(is_identifier_text(identifier)) {
      return identifier;
    }
    if(is_identifier_text(trimmed)) {
      return trimmed;
    }
    return std::string();
  }

  std::string source_template_name_location_from_argument_syntaxes(
      const std::vector<TemplateArgumentSyntax> * syntaxes,
      const std::string & template_identifier) const
  {
    if(!syntaxes || syntaxes->empty() || template_identifier.empty()) {
      return std::string();
    }
    std::size_t argument_token = 0;
    bool have_argument_token = false;
    for(std::size_t i = 0; i < syntaxes->size(); ++i) {
      if((*syntaxes)[i].source_location_id != 0) {
        const std::string argument_location =
            template_api::normalize_template_witness_source_location(
                template_api::template_witness_detail::
                    source_location_for_location_id(
                        template_witness_context(),
                        (*syntaxes)[i].source_location_id));
        if(!argument_location.empty() &&
           token_index_for_source_location(argument_location,
                                           trim_space((*syntaxes)[i].text),
                                           argument_token)) {
          have_argument_token = true;
          break;
        }
      }
      if((*syntaxes)[i].has_source_token_start) {
        argument_token = (*syntaxes)[i].source_token_start;
        have_argument_token = true;
        break;
      }
    }
    if(!have_argument_token || argument_token == 0) {
      return std::string();
    }

    std::size_t open_token = argument_token;
    bool found_open = false;
    while(open_token > 0) {
      --open_token;
      const RecogToken & token = peek_token(open_token);
      if(token.source == "<") {
        found_open = true;
        break;
      }
      if(token.source == ";" || token.source == "{" ||
         token.source == "}" || token.source == "(" ||
         token.source == ")") {
        return std::string();
      }
    }
    if(!found_open || open_token == 0) {
      return std::string();
    }

    std::size_t name_token = open_token;
    while(name_token > 0) {
      --name_token;
      const RecogToken & token = peek_token(name_token);
      if(token.is_identifier() && token.source == template_identifier) {
        return template_api::normalize_template_witness_source_location(
            source_location_for_token_index(name_token));
      }
      if(token.source == "::" || token.is_identifier()) {
        continue;
      }
      break;
    }
    return std::string();
  }

  std::string compact_token_range_lookup_text(std::size_t begin,
                                              std::size_t end) const
  {
    std::string text;
    std::string previous;
    for(std::size_t i = begin; i < end; ++i) {
      const RecogToken & token = peek_token(i);
      if(token.is_eof()) {
        break;
      }
      if(!previous.empty() &&
         !token.source.empty() &&
         (std::isalnum(static_cast<unsigned char>(previous[previous.size() - 1])) ||
          previous[previous.size() - 1] == '_') &&
         (std::isalnum(static_cast<unsigned char>(token.source[0])) ||
          token.source[0] == '_')) {
        text.push_back(' ');
      }
      text += token.source;
      previous = token.source;
    }
    return text;
  }

  bool source_template_id_arguments_match_instantiation(
      std::size_t template_token,
      const std::vector<TemplateArgument> & arguments) const
  {
    std::size_t open_token = 0;
    if(!find_next_token_source_on_same_line(template_token, "<", open_token)) {
      return false;
    }
    std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
    if(!template_argument_token_ranges_from_open(open_token, arg_ranges) ||
       arg_ranges.size() > arguments.size()) {
      return false;
    }
    for(std::size_t i = 0; i < arg_ranges.size(); ++i) {
      if(compact_token_range_lookup_text(arg_ranges[i].first,
                                         arg_ranges[i].second) !=
         compact_lookup_text(template_argument_text(arguments[i]))) {
        return false;
      }
    }
    return true;
  }

  bool source_template_id_argument_texts_at_location(
      const std::string & use_location,
      const std::string & template_identifier,
      std::vector<std::string> & out) const
  {
    out.clear();
    std::size_t template_token = 0;
    if(use_location.empty() ||
       template_identifier.empty() ||
       !token_index_for_source_location(use_location,
                                        template_identifier,
                                        template_token)) {
      return false;
    }
    std::size_t open_token = 0;
    if(!find_next_token_source_on_same_line(template_token, "<", open_token)) {
      return false;
    }
    std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
    if(!template_argument_token_ranges_from_open(open_token, arg_ranges)) {
      return false;
    }
    out.reserve(arg_ranges.size());
    for(std::size_t i = 0; i < arg_ranges.size(); ++i) {
      out.push_back(compact_token_range_lookup_text(arg_ranges[i].first,
                                                    arg_ranges[i].second));
    }
    if(out.size() == 1 && trim_space(out[0]).empty()) {
      out.clear();
    }
    return true;
  }

  ClassInfo * source_member_template_owner_class_from_syntax(
      Scope & use_scope,
      const std::string & use_location,
      const std::string & template_identifier)
  {
    const auto resolve_owner =
        [&](Scope & lookup_scope,
            const TemplateIdSyntax & syntax,
            const std::string & template_text) -> ClassInfo *
        {
          if(syntax.name.qualifiers.empty() ||
             unqualified_member_name(syntax.name.name) != template_identifier) {
            return nullptr;
          }
          CppAstNode node;
          node.kind = CppAstKind::type_name;
          node.value = template_text;
          set_cppast_qualified_name_syntax(node, syntax.name);
          set_cppast_qualifier_template_id_syntaxes(
              node,
              syntax.qualifier_template_id_syntaxes);

          const witness::ScopedTemplateWitnessSourceCapturePause source_pause;
          TypePtr owner_type =
              semantic_lookup::resolve_qualified_owner_type_node(ctx,
                                                                 lookup_scope,
                                                                 syntax.name,
                                                                 node);
          ClassInfo * owner = ctx.class_info_for_type(owner_type);
          if(!owner) {
            owner = ctx.complete_class_type(owner_type);
          }
          return owner;
        };

    if(!callbacks.template_id_syntax_at_location) {
      return nullptr;
    }
    const TemplateIdSyntax * source_syntax =
        callbacks.template_id_syntax_at_location(use_location,
                                                 template_identifier);
    return source_syntax ?
        resolve_owner(use_scope, *source_syntax, std::string()) :
        nullptr;
  }

  std::string source_template_name_location_on_use_line(
      const std::string & use_location,
      const std::string & template_identifier,
      const std::vector<TemplateArgument> & arguments) const
  {
    if(use_location.empty() || template_identifier.empty()) {
      return std::string();
    }
    const ParsedSourceLocation parsed =
        parse_source_location(
            template_api::normalize_template_witness_source_location(
                use_location));
    if(!parsed.valid || parsed.line <= 0) {
      return std::string();
    }
    std::ostringstream line_start;
    line_start << parsed.file << ":" << parsed.line << ":1";
    std::string search_location = line_start.str();
    for(;;) {
      const std::string candidate =
          template_api::normalize_template_witness_source_location(
              template_api::template_witness_detail::
                  source_location_for_identifier_token_on_or_after(
                      template_witness_context(),
                      search_location,
                      template_identifier,
                      true,
                      true));
      if(candidate.empty()) {
        return std::string();
      }
      std::size_t template_token = 0;
      if(!token_index_for_source_location(candidate,
                                          template_identifier,
                                          template_token)) {
        return std::string();
      }
      if(source_template_id_arguments_match_instantiation(template_token,
                                                          arguments)) {
        return candidate;
      }
      std::size_t open_token = 0;
      if(!find_next_token_source_on_same_line(template_token, "<", open_token)) {
        return std::string();
      }
      const std::string next_search_location =
          template_api::normalize_template_witness_source_location(
              source_location_for_token_index(open_token));
      if(next_search_location.empty() ||
         next_search_location == search_location) {
        return std::string();
      }
      search_location = next_search_location;
    }
  }

  bool source_location_is_in_function_parameter_clause(
      const std::string & location,
      const std::string & template_identifier) const
  {
    std::size_t token_index = 0;
    if(location.empty() ||
       template_identifier.empty() ||
       !token_index_for_source_location(location,
                                        template_identifier,
                                        token_index)) {
      return false;
    }
    const ParsedSourceLocation parsed =
        parse_source_location(
            template_api::normalize_template_witness_source_location(location));
    if(!parsed.valid) {
      return false;
    }
    while(token_index > 0) {
      --token_index;
      const std::string token_location =
          template_api::normalize_template_witness_source_location(
              source_location_for_token_index(token_index));
      const ParsedSourceLocation token_parsed =
          parse_source_location(token_location);
      if(!token_parsed.valid ||
         token_parsed.file != parsed.file ||
         token_parsed.line != parsed.line) {
        return false;
      }
      const RecogToken & token = peek_token(token_index);
      if(token.source == "(") {
        return true;
      }
      if(token.source == ")" || token.source == ";" ||
         token.source == "=" || token.source == "{") {
        return false;
      }
    }
    return false;
  }

  bool exact_template_type_lookup_anchor_arguments_match(
      const ExactTemplateTypeLookupAnchor & anchor,
      const std::vector<TemplateArgument> & arguments) const
  {
    if(!anchor.has_argument_list) {
      return true;
    }
    const std::vector<std::string> & anchor_arg_texts =
        exact_template_type_lookup_anchor_texts(anchor);
    if(anchor_arg_texts.size() > arguments.size()) {
      return false;
    }
    for(std::size_t i = 0; i < anchor_arg_texts.size(); ++i) {
      if(compact_lookup_text(anchor_arg_texts[i]) !=
         compact_lookup_text(template_argument_text(arguments[i]))) {
        return false;
      }
    }
    return true;
  }

  std::vector<std::string> class_template_argument_source_locations_for_current_use(
      const std::string & template_name,
      const std::vector<TemplateParameterInfo> & parameters,
      const std::vector<std::string> & arg_texts)
  {
    std::vector<std::string> locations(arg_texts.size());
    if(arg_texts.empty() ||
       (!witness::source_capture_enabled(template_witness_context()) &&
        !parser_trace::enabled("template.resolve"))) {
      return locations;
    }

    const std::string unqualified_template_name =
        unqualified_member_name(template_name);
    const std::string template_identifier =
        unqualified_template_name.empty() ? template_name : unqualified_template_name;
    std::string template_location =
        template_api::normalize_template_witness_source_location(
            parser_trace::current_use_location());
    if(!source_location_points_at_identifier(template_location,
                                             template_identifier)) {
      template_location =
          template_api::normalize_template_witness_source_location(
              template_api::template_witness_detail::
                  source_location_for_identifier_token_on_or_after(
                  template_witness_context(),
                  template_location,
                  template_identifier,
                  true,
                  true));
    }

    std::size_t template_token = 0;
    if(!token_index_for_source_location(template_location,
                                        template_identifier,
                                        template_token)) {
      return locations;
    }
    std::size_t open_token = 0;
    if(!find_next_token_source_on_same_line(template_token, "<", open_token)) {
      return locations;
    }
    std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
    if(!template_argument_token_ranges_from_open(open_token, arg_ranges)) {
      return locations;
    }

    for(std::size_t arg_index = 0;
        arg_index < arg_texts.size() && arg_index < arg_ranges.size();
        ++arg_index) {
      const std::size_t arg_begin = arg_ranges[arg_index].first;
      const std::size_t arg_end = arg_ranges[arg_index].second;
      const bool trailing_pack_argument =
          !parameters.empty() &&
          parameters.back().parameter_pack &&
          arg_index + 1 >= parameters.size();
      const std::size_t parameter_index =
          trailing_pack_argument ? parameters.size() - 1 : arg_index;
      const bool source_needed =
          parameter_index < parameters.size() &&
          parameters[parameter_index].kind == TemplateParameterInfo::TP_NON_TYPE;
      const std::string identifier =
          source_needed ?
              template_argument_anchor_identifier_for_source(arg_texts[arg_index]) :
              std::string();
      const bool require_template_open =
          arg_texts[arg_index].find('<') != std::string::npos;
      if(!identifier.empty()) {
        for(std::size_t found = arg_begin; found < arg_end; ++found) {
          const RecogToken & token = peek_token(found);
          if(!(token.is_identifier() && token.source == identifier)) {
            continue;
          }
          if(require_template_open &&
             (found + 1 >= arg_end || peek_token(found + 1).source != "<")) {
            continue;
          }
          locations[arg_index] =
              template_api::normalize_template_witness_source_location(
                  source_location_for_token_index(found));
          break;
        }
      }
      if(locations[arg_index].empty()) {
        for(std::size_t fallback = arg_begin; fallback < arg_end; ++fallback) {
          if(peek_token(fallback).location_id == 0) {
            continue;
          }
          locations[arg_index] =
              template_api::normalize_template_witness_source_location(
                  source_location_for_token_index(fallback));
          break;
        }
      }
    }
    return locations;
  }

  ClassInfo * reference_class_template_instantiation(ClassTemplateDecl & decl,
                                                     Scope & use_scope,
                                                     const vector<string> & arg_texts)
  {
    return reference_class_template_instantiation_with_syntax(
        decl, use_scope, arg_texts, nullptr);
  }

  ClassInfo * reference_class_template_instantiation_with_syntax(
      ClassTemplateDecl & decl,
      Scope & use_scope,
      const vector<string> & arg_texts,
      const vector<TemplateArgumentSyntax> * arg_syntaxes,
      template_api::ClassTemplateSourceUseMode source_use_mode =
          template_api::ClassTemplateSourceUseMode::EmitClassUse)
  {
    vector<TemplateArgumentSyntax> stable_arg_syntaxes;
    if(arg_syntaxes) {
      stable_arg_syntaxes = *arg_syntaxes;
      arg_syntaxes = &stable_arg_syntaxes;
    }
    DIAG_CONTEXT("reference_class_template_instantiation [" + decl.name +
                 ", args=" + to_string(arg_texts.size()) + "]");
    note_performance_counter(
        &semantic_metrics::AnalyzerCounters::class_template_reference_requests);
    if(semantic_hotspot::enabled()) {
      ostringstream query;
      query << decl.name << "<" << join_hotspot_arg_texts(arg_texts) << ">";
      semantic_hotspot::note_semantic_query("reference_class_template_instantiation", query.str());
    }
    if(is_builtin_initializer_list_template(decl)) {
      return instantiate_class_template(decl, use_scope, arg_texts, arg_syntaxes);
    }

    const bool use_raw_reference_cache = raw_reference_cache_allowed(decl);
    const std::string raw_cache_key =
        use_raw_reference_cache ? raw_reference_cache_key(decl, use_scope, arg_texts) :
                                  std::string();
    if(use_raw_reference_cache) {
      auto cached =
          decl.fast_reference_cache.find(raw_cache_key);
      if(cached != decl.fast_reference_cache.end() &&
         raw_reference_cache_info_usable(decl, cached->second)) {
        return cached->second;
      }
    }

    vector<TemplateArgument> arguments;
    string key;
    ClassInfo * info = nullptr;
    note_performance_counter(
        &semantic_metrics::AnalyzerCounters::class_template_fast_existing_attempts);
    const bool fast_existing =
        try_fast_existing_class_template_instantiation(
            decl, use_scope, arg_texts, arg_syntaxes, arguments, key, info);
    if(fast_existing) {
      note_performance_counter(
          &semantic_metrics::AnalyzerCounters::class_template_fast_existing_hits);
    } else {
      note_performance_counter(
          &semantic_metrics::AnalyzerCounters::class_template_fast_existing_misses);
    }
    const bool fast_resolved_miss =
        !fast_existing && (!key.empty() || !arguments.empty());
    if(!fast_existing && !fast_resolved_miss) {
      const std::vector<std::string> arg_source_locations =
          class_template_argument_source_locations_for_current_use(decl.name,
                                                                   decl.parameters,
                                                                   arg_texts);
      const template_api::ScopedTemplateArgumentSourceLocations
          template_argument_source_locations(arg_texts, arg_source_locations);
      if(!resolve_template_arguments(
             use_scope, decl.parameters, arg_texts, arg_syntaxes, arguments, decl.declaring_scope)) {
        return nullptr;
      }
      canonicalize_simple_dependent_argument_texts(arguments);
      note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_key_builds);
      key = template_instantiation::class_template_argument_key_for_instantiation(
          ctx, decl, arguments);
    }
    const bool dependent_arguments = template_arguments_are_dependent(arguments);
    const bool fast_existing_requires_mangle_refresh =
        dependent_arguments &&
        info &&
        !fast_existing_class_template_mangle_context_current(use_scope,
                                                             arguments,
                                                             arg_syntaxes,
                                                             *info);
    const bool fast_existing_requires_dependency_refresh =
        info && info->dependent_instantiation && !dependent_arguments;
    if(fast_existing &&
       info &&
       fast_existing_class_template_output_usable(decl, *info) &&
       fast_existing_class_template_selection_current(decl, key, *info) &&
       !fast_existing_requires_mangle_refresh &&
       !fast_existing_requires_dependency_refresh &&
       !parser_trace::enabled("template.resolve") &&
       !witness::source_capture_enabled(template_witness_context())) {
      remember_raw_reference_cache(decl, raw_cache_key, info);
      return info;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "reference-class-template name=" << decl.name
            << " key=" << key
            << " arg-count=" << arguments.size()
            << " dependent=" << (dependent_arguments ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    template_api::ClassSpecializationSelection specialization;
    specialization = template_api::specialization::select_class_specialization(
        *this,
        decl,
        use_scope,
        key,
        arguments,
        dependent_arguments ? &arg_texts : nullptr);
    if(specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION &&
       specialization.class_node) {
      const std::string use_location =
          template_api::normalize_template_witness_source_location(
              parser_trace::current_order_use_location());
      const std::string specialization_location =
          source_location_for_node(*specialization.class_node);
      if(!use_location.empty() &&
         !specialization_location.empty() &&
         source_location_is_later(use_location, specialization_location)) {
        throw ExplicitSpecializationAfterInstantiationError(
            string("explicit specialization after instantiation") +
            " [use " + use_location + "]" +
            " [specialization " + specialization_location + "]");
      }
    }
    ClassInfo * referenced_info =
        reference_selected_class_template_instantiation_with_key(decl,
                                                                 use_scope,
                                                                 arguments,
                                                                 specialization,
                                                                 &arg_texts,
                                                                 &key,
                                                                 source_use_mode,
                                                                 arg_syntaxes,
                                                                 &dependent_arguments);
    remember_raw_reference_cache(decl, raw_cache_key, referenced_info);
    return referenced_info;
  }

    ClassInfo * reference_selected_class_template_instantiation(
        ClassTemplateDecl & decl,
        Scope & use_scope,
        const vector<TemplateArgument> & arguments,
        const template_api::ClassSpecializationSelection & specialization,
        const vector<string> * source_arg_texts = nullptr,
        template_api::ClassTemplateSourceUseMode source_use_mode =
            template_api::ClassTemplateSourceUseMode::EmitClassUse,
        const vector<TemplateArgumentSyntax> * source_arg_syntaxes = nullptr,
        const string * precomputed_key = nullptr,
        FunctionBinding * source_function = nullptr)
  {
    return reference_selected_class_template_instantiation_with_key(decl,
                                                                    use_scope,
                                                                    arguments,
                                                                    specialization,
                                                                    source_arg_texts,
                                                                    precomputed_key,
                                                                    source_use_mode,
                                                                    source_arg_syntaxes,
                                                                    nullptr,
                                                                    source_function);
  }

    ClassInfo * reference_selected_class_template_instantiation_with_key(
        ClassTemplateDecl & decl,
        Scope & use_scope,
        const vector<TemplateArgument> & arguments,
        const template_api::ClassSpecializationSelection & specialization,
        const vector<string> * source_arg_texts,
        const string * precomputed_key,
        template_api::ClassTemplateSourceUseMode source_use_mode =
            template_api::ClassTemplateSourceUseMode::EmitClassUse,
        const vector<TemplateArgumentSyntax> * source_arg_syntaxes = nullptr,
        const bool * precomputed_dependent_arguments = nullptr,
        FunctionBinding * source_function = nullptr)
  {
    const CppAstNode * class_node = specialization.class_node;
    Scope * binding_scope = specialization.binding_scope;
    const vector<TemplateParameterInfo> * bound_parameters = specialization.parameters;
    const vector<TemplateArgument> * bound_arguments = &specialization.arguments;
    const std::map<std::string, std::size_t> * bound_pack_sizes = &specialization.pack_sizes;
    string computed_key;
    if(!precomputed_key) {
      note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_key_builds);
      computed_key =
          template_instantiation::class_template_argument_key_for_instantiation(
              ctx, decl, arguments);
    }
    const string & key = precomputed_key ? *precomputed_key : computed_key;
    const bool dependent_arguments =
        precomputed_dependent_arguments ?
            *precomputed_dependent_arguments :
            template_arguments_are_dependent(arguments);
    if(!source_arg_syntaxes &&
       dependent_arguments &&
       specialization.argument_syntaxes &&
       specialization.argument_syntaxes->size() == arguments.size()) {
      source_arg_syntaxes = specialization.argument_syntaxes;
    }
    std::vector<TemplateParameterInfo> inferred_dependent_mangle_parameters;
    const vector<TemplateParameterInfo> *
        dependent_argument_mangle_parameters =
            dependent_arguments ?
                dependent_argument_mangle_parameters_for_argument_mangling(
                    use_scope,
                    arguments,
                    source_arg_syntaxes,
                    inferred_dependent_mangle_parameters) :
                nullptr;
    const bool selected_partial_mangle_context =
        bound_parameters &&
        bound_parameters != &decl.parameters &&
        bound_arguments;
    const vector<TemplateParameterInfo> * selected_mangle_parameters =
        selected_partial_mangle_context ? bound_parameters :
                                          dependent_argument_mangle_parameters;
    const vector<TemplateArgument> * selected_mangle_arguments =
        selected_partial_mangle_context ? bound_arguments : nullptr;
    const map<string, size_t> * selected_mangle_pack_sizes =
        selected_partial_mangle_context ? bound_pack_sizes : nullptr;

    const CppAstNode * class_key = find_child_kind(*class_node, CppAstKind::class_key);
    if(!class_key) {
      throw logic_error("class template missing class-key");
    }

    string specialization_name;
    string internal_specialization_name;
    bool specialization_name_ready = false;
    bool internal_specialization_name_ready = false;
    const auto ensure_specialization_name = [&]() -> const string &
    {
      if(!specialization_name_ready) {
        note_performance_counter(
            &semantic_metrics::AnalyzerCounters::class_template_specialization_name_builds);
        specialization_name = display_template_specialization_name(decl.name,
                                                                   arguments);
        specialization_name_ready = true;
      }
      return specialization_name;
    };
    const auto ensure_internal_specialization_name = [&]() -> const string &
    {
      if(!internal_specialization_name_ready) {
        note_performance_counter(
            &semantic_metrics::AnalyzerCounters::class_template_specialization_name_builds);
        internal_specialization_name =
            template_instantiation::class_specialization_name_for_instantiation(
                ctx, decl.name, arguments, key);
        internal_specialization_name_ready = true;
      }
      return internal_specialization_name;
    };
    const auto note_class_use = [&](ClassInfo * resolved_info) -> void
    {
      const std::string use_location = parser_trace::current_use_location();
      const bool trace_enabled = parser_trace::enabled("template.resolve");
      const template_api::TemplateWitnessContext witness_context =
          template_witness_context();
      const bool source_capture_enabled =
          witness::source_capture_enabled(witness_context);
      const bool syntax_backed_source_capture_enabled =
          source_capture_enabled || witness::enabled(witness_context);
      const bool owner_reference_source_text_enabled =
          syntax_backed_source_capture_enabled;
      if(use_location.empty() || !resolved_info) {
        return;
      }
      const auto emit_nested_source_argument_uses = [&]() -> void
      {
        if(!witness::class_use_recording_enabled()) {
          return;
        }
        if(syntax_backed_source_capture_enabled &&
           source_arg_syntaxes &&
           callbacks.emit_nested_class_use_source_events_from_syntaxes) {
          callbacks.emit_nested_class_use_source_events_from_syntaxes(
              use_scope,
              *source_arg_syntaxes,
              witness::SourceUseOwnership::SourceOwned,
              decl.name);
        }
      };
      const auto location_within_node = [&](const std::string & location,
                                            const CppAstNode * node) -> bool
      {
        if(!(node && !location.empty() && node->token_end > node->token_start)) {
          return false;
        }
        const ParsedSourceLocation parsed_location =
            parse_physical_source_location(location);
        const ParsedSourceLocation parsed_begin =
            parse_physical_source_location(source_location_for_node(*node));
        const ParsedSourceLocation parsed_end =
            parse_physical_source_location(
                source_location_for_token_index(node->token_end - 1));
        if(!parsed_location.valid || !parsed_begin.valid || !parsed_end.valid ||
           parsed_location.file != parsed_begin.file ||
           parsed_location.file != parsed_end.file) {
          return false;
        }
        if(parsed_location.line < parsed_begin.line ||
           parsed_location.line > parsed_end.line) {
          return false;
        }
        if(parsed_location.line == parsed_begin.line &&
           parsed_location.column < parsed_begin.column) {
          return false;
        }
        if(parsed_location.line == parsed_end.line &&
           parsed_location.column > parsed_end.column) {
          return false;
        }
        return true;
      };
      const bool nested_argument_recovery =
          template_api::class_template_source_use_recovers_nested_arguments(
              source_use_mode);
      const bool suppress_nested_arguments =
          template_api::class_template_source_use_suppresses_nested_arguments(
              source_use_mode);
      const auto repair_bindings_from_resolved_instantiation =
          [&](std::vector<template_api::TemplateWitnessSourceBinding> & bindings,
              semantic_source_use::SourceTemplateIdOccurrence * occurrence)
          {
            if(!resolved_info) {
              return;
            }
            const std::size_t limit =
                std::min(bindings.size(),
                         std::max(resolved_info->instantiation_arguments.size(),
                                  resolved_info->instantiation_arg_texts.size()));
            for(std::size_t i = 0; i < limit; ++i) {
              if(bindings[i].pack_binding) {
                continue;
              }
              std::string resolved_arg =
                  i < resolved_info->instantiation_arg_texts.size() ?
                      resolved_info->instantiation_arg_texts[i] :
                      std::string();
              if(resolved_arg.empty() &&
                 i < resolved_info->instantiation_arguments.size()) {
                resolved_arg =
                    template_api::template_witness_source_argument_text(
                        ctx,
                        resolved_info->instantiation_arguments[i]);
              }
              if(binding_arg_drops_resolved_array_cv(bindings[i].arg,
                                                     resolved_arg)) {
                bindings[i].arg = resolved_arg;
                if(occurrence &&
                   i < occurrence->arguments.size()) {
                  occurrence->arguments[i].text = resolved_arg;
                  occurrence->arguments[i].semantic_text = resolved_arg;
                }
              }
            }
          };
      if((dependent_arguments && !suppress_nested_arguments) ||
         nested_argument_recovery) {
        emit_nested_source_argument_uses();
      }
      if(template_api::class_template_source_use_is_semantic_lookup_only(
             source_use_mode)) {
        return;
      }
      if(dependent_arguments) {
        return;
      }
      if(!trace_enabled && !syntax_backed_source_capture_enabled) {
        return;
      }
      const bool explicit_specialization_source_use =
          specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION ||
          resolved_info->is_explicit_specialization;
      const bool nested_recovery_is_qualified_member_owner =
          owner_reference_source_text_enabled &&
          nested_argument_recovery &&
          callbacks.template_id_at_location_is_qualified_member_owner &&
          callbacks.template_id_at_location_is_qualified_member_owner(
              use_location);
      if(nested_argument_recovery &&
         !explicit_specialization_source_use &&
         !nested_recovery_is_qualified_member_owner &&
         !trace_enabled) {
        return;
      }
      const std::string source_template_name =
          unqualified_member_name(decl.name);
      const std::string anchor_identifier =
          !source_template_name.empty() ?
              source_template_name :
              decl.name;
      const string & use_specialization_name = ensure_specialization_name();
      std::string exact_use_location;
      const bool current_use_location_spells_template =
          source_location_points_at_identifier(use_location,
                                               anchor_identifier);
      if(const ExactTemplateTypeLookupAnchor * anchor =
             current_exact_template_type_lookup_anchor()) {
        if(exact_template_type_lookup_anchor_matches(*anchor, use_specialization_name)) {
          exact_use_location = anchor->location;
        } else if(!current_use_location_spells_template &&
                  exact_template_type_lookup_anchor_matches_identifier(
                      *anchor,
                      anchor_identifier) &&
                  exact_template_type_lookup_anchor_arguments_match(*anchor,
                                                                    arguments)) {
          exact_use_location = anchor->location;
        }
      }
      if(!exact_use_location.empty() && !use_location.empty() &&
         !anchor_identifier.empty()) {
        const std::string syntax_use_location =
            source_template_name_location_from_argument_syntaxes(
                source_arg_syntaxes,
                anchor_identifier);
        if(!syntax_use_location.empty()) {
          exact_use_location = syntax_use_location;
        }
      }
      if(!exact_use_location.empty() &&
         !use_location.empty() &&
         !anchor_identifier.empty()) {
        const std::string line_use_location =
            source_template_name_location_on_use_line(use_location,
                                                      anchor_identifier,
                                                      arguments);
        if(!line_use_location.empty()) {
          const ParsedSourceLocation parsed_exact =
              parse_source_location(exact_use_location);
          const ParsedSourceLocation parsed_use =
              parse_source_location(use_location);
          if(exact_use_location.empty() ||
             !parsed_exact.valid ||
             !parsed_use.valid ||
             parsed_exact.file != parsed_use.file ||
             parsed_exact.line != parsed_use.line) {
            exact_use_location = line_use_location;
          }
        }
      }
      if(exact_use_location.empty() && !use_location.empty() &&
         !anchor_identifier.empty()) {
        exact_use_location =
            source_template_name_location_from_argument_syntaxes(
                source_arg_syntaxes,
                anchor_identifier);
      }
      if(exact_use_location.empty() && !use_location.empty() &&
         !anchor_identifier.empty()) {
        exact_use_location =
            template_api::normalize_template_witness_source_location(
                template_api::template_witness_detail::
                    source_location_for_identifier_token_on_or_after(
                    template_witness_context(),
                    use_location,
                    anchor_identifier,
                    true,
                    use_specialization_name.find('<') != std::string::npos));
      }
      const std::string chosen_use_location =
          !exact_use_location.empty() ? exact_use_location :
                                       use_location;
      const bool source_use_spells_template =
          source_location_points_at_identifier(chosen_use_location,
                                               anchor_identifier);
      const bool source_template_id_is_qualified_member_owner =
          owner_reference_source_text_enabled &&
          callbacks.template_id_at_location_is_qualified_member_owner &&
          callbacks.template_id_at_location_is_qualified_member_owner(
              chosen_use_location);
      if(source_capture_enabled && !source_use_spells_template) {
        return;
      }
      const bool source_use_in_template_header =
          source_location_is_template_header_context_for_witness(
              chosen_use_location);
      const bool source_use_in_template_body =
          source_location_is_template_body_context_for_witness(
              chosen_use_location);
      const bool source_use_mentions_template_context =
          template_argument_syntaxes_mention_source_template_context(
              use_scope,
              source_arg_syntaxes);
      const bool source_use_mentions_current_specialization_member =
          template_argument_syntaxes_mention_current_specialization_member(
              use_scope,
              source_arg_syntaxes);
      const bool source_use_mentions_current_specialization_name =
          source_arg_texts &&
          template_argument_texts_mention_current_specialization_names(
              use_scope,
              *source_arg_texts);
      const bool replayed_template_body_source_use =
          source_use_in_template_body &&
          !scope_has_template_placeholders(use_scope) &&
          !scope_is_inside_source_template_context(use_scope);
      const bool replayed_source_template_body_source_use =
          source_use_in_template_body &&
          !scope_has_template_placeholders(use_scope) &&
          scope_is_inside_source_template_context(use_scope) &&
          (source_use_mentions_template_context ||
           !source_use_mentions_current_specialization_member ||
           source_use_mentions_current_specialization_name);
      const bool replayed_current_specialization_member_body_source_use =
          source_use_in_template_body &&
          explicit_specialization_source_use &&
          source_use_mentions_current_specialization_member &&
          !scope_has_template_placeholders(use_scope) &&
          scope_is_inside_source_template_context(use_scope);
      const bool replayed_template_header_source_use =
          source_use_in_template_header &&
          !scope_has_template_placeholders(use_scope) &&
          !scope_is_inside_source_template_context(use_scope) &&
          source_location_is_in_function_parameter_clause(chosen_use_location,
                                                          anchor_identifier);
      const bool replayed_source_template_header_source_use =
          source_use_in_template_header &&
          !scope_has_template_placeholders(use_scope) &&
          scope_is_inside_source_template_context(use_scope) &&
          (source_use_mentions_template_context ||
           !source_use_mentions_current_specialization_member);
      const bool replayed_source_template_header_member_expression_source_use =
          source_use_in_template_header &&
          !scope_has_template_placeholders(use_scope) &&
          scope_is_inside_source_template_context(use_scope) &&
          source_use_mentions_current_specialization_member &&
          !source_use_mentions_template_context;
      const bool dependent_member_template_pattern_source_use =
          source_use_in_template_body &&
          class_template_decl_is_member_template(decl) &&
          !template_api::template_witness_qualified_member_type_lookup_active() &&
          (scope_has_template_placeholders(use_scope) ||
           scope_is_inside_source_template_context(use_scope));
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "class-use-source-gate template=" << decl.name
              << " location=" << chosen_use_location
              << " body=" << (source_use_in_template_body ? "yes" : "no")
              << " header=" << (source_use_in_template_header ? "yes" : "no")
              << " mentions-template-context="
              << (source_use_mentions_template_context ? "yes" : "no")
              << " mentions-current-member="
              << (source_use_mentions_current_specialization_member ? "yes" : "no")
              << " placeholders=" << (scope_has_template_placeholders(use_scope) ? "yes" : "no")
              << " inside-source-template="
              << (scope_is_inside_source_template_context(use_scope) ? "yes" : "no")
              << " source-syntaxes=" << (source_arg_syntaxes ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(witness::enabled(witness_context) &&
         (replayed_current_specialization_member_body_source_use ||
          (!explicit_specialization_source_use &&
           (replayed_template_header_source_use ||
            replayed_source_template_header_source_use ||
            replayed_template_body_source_use ||
            replayed_source_template_body_source_use ||
            replayed_source_template_header_member_expression_source_use ||
            dependent_member_template_pattern_source_use ||
            (template_api::function_binding_has_source_template_identity(
                 use_scope.function) &&
             source_use_mentions_template_context))))) {
        return;
      }
      const std::pair<std::string, std::string> source_dependent_drop_key(
          chosen_use_location,
          decl.name + "|" + key);
      if(source_dependent_class_template_use_drops_.count(
             source_dependent_drop_key) != 0) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             chosen_use_location,
                             "class-use-source-drop reason=previous-dependent-drop");
        }
        return;
      }
      std::vector<std::string> anchored_arg_texts;
      const std::vector<std::string> * binding_arg_texts = source_arg_texts;
      bool binding_arg_texts_from_source_template_id = false;
      bool binding_arg_texts_from_exact_anchor = false;
      bool binding_arg_texts_from_concrete_replay = false;
      std::vector<std::string> recovered_source_arg_texts;
      std::vector<std::string> concrete_replay_arg_texts;
      const auto concrete_source_replay_allowed = [&]() -> bool
      {
        if(scope_has_template_placeholders(use_scope) ||
           !scope_is_inside_source_template_context(use_scope) ||
           specialization.kind != template_api::MS_PRIMARY ||
           template_arguments_are_dependent(arguments)) {
          return false;
        }
        return source_use_in_template_header ||
               !source_use_in_template_body ||
               !source_use_mentions_template_context;
      };
      const auto use_concrete_replay_arg_texts = [&]() -> bool
      {
        if(!concrete_source_replay_allowed()) {
          return false;
        }
        concrete_replay_arg_texts.clear();
        concrete_replay_arg_texts.reserve(arguments.size());
        for(std::size_t i = 0; i < arguments.size(); ++i) {
          concrete_replay_arg_texts.push_back(
              template_argument_text(arguments[i]));
        }
        binding_arg_texts = &concrete_replay_arg_texts;
        binding_arg_texts_from_source_template_id = true;
        binding_arg_texts_from_concrete_replay = true;
        return true;
      };
      if(exact_template_type_lookup_anchor_arg_texts(use_specialization_name,
                                                     anchor_identifier,
                                                     anchored_arg_texts)) {
        binding_arg_texts = &anchored_arg_texts;
        binding_arg_texts_from_exact_anchor =
            exact_template_type_lookup_anchor_arg_texts_are_full_match(
                use_specialization_name);
      }
      const std::vector<std::string> * source_template_arg_texts =
          template_api::current_template_id_source_arguments_ptr(
              chosen_use_location,
              anchor_identifier);
      if(source_template_arg_texts) {
        if(template_argument_texts_mention_template_bound_scope_names(
               use_scope,
               *source_template_arg_texts) ||
           template_argument_texts_mention_enclosing_source_template_parameters(
               use_scope,
               *source_template_arg_texts)) {
          if(use_concrete_replay_arg_texts()) {
            source_template_arg_texts = nullptr;
          } else {
          if(parser_trace::enabled("template.resolve")) {
            parser_trace::note("template.resolve",
                               chosen_use_location,
                               "class-use-source-drop reason=source-template-args-dependent");
          }
          source_dependent_class_template_use_drops_.insert(
              source_dependent_drop_key);
          return;
          }
        }
        if(source_template_arg_texts) {
          binding_arg_texts = source_template_arg_texts;
          binding_arg_texts_from_source_template_id =
              source_template_id_args_are_arity_compatible(*source_template_arg_texts,
                                                          arguments);
        }
      }
      if(!source_template_arg_texts &&
         source_use_spells_template &&
         source_template_id_argument_texts_at_location(chosen_use_location,
                                                       anchor_identifier,
                                                       recovered_source_arg_texts)) {
        binding_arg_texts = &recovered_source_arg_texts;
        binding_arg_texts_from_source_template_id =
            source_template_id_args_are_arity_compatible(recovered_source_arg_texts,
                                                        arguments);
      }
      if(source_capture_enabled &&
         source_use_spells_template &&
         !source_arg_syntaxes &&
         !source_template_arg_texts &&
         !binding_arg_texts_from_source_template_id &&
         !binding_arg_texts_from_exact_anchor) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             chosen_use_location,
                             "class-use-source-drop reason=no-source-argument-syntax");
        }
        return;
      }
      const bool binding_arg_texts_are_dependent =
          binding_arg_texts &&
          ((!binding_arg_texts_from_source_template_id &&
            template_argument_texts_mention_source_bindings(*this,
                                                            use_scope,
                                                            *binding_arg_texts)) ||
           template_argument_texts_mention_instantiated_class_local_type_aliases(
               use_scope,
               *binding_arg_texts) ||
           template_argument_texts_mention_template_bound_scope_names(
               use_scope,
               *binding_arg_texts) ||
           template_argument_texts_mention_enclosing_source_template_parameters(
               use_scope,
               *binding_arg_texts));
      if(binding_arg_texts_are_dependent &&
         !binding_arg_texts_from_concrete_replay &&
         !use_concrete_replay_arg_texts()) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             chosen_use_location,
                             "class-use-source-drop reason=binding-args-dependent");
        }
        source_dependent_class_template_use_drops_.insert(
            source_dependent_drop_key);
        return;
      }
      if(binding_arg_texts_from_source_template_id && binding_arg_texts &&
         !binding_arg_texts_from_concrete_replay) {
        bool drop_noncanonical_source_arguments =
            source_arg_syntaxes &&
            template_argument_syntaxes_mention_source_template_context(
                use_scope,
                source_arg_syntaxes);
        if(!drop_noncanonical_source_arguments && source_arg_syntaxes) {
          for(std::size_t i = 0; i < source_arg_syntaxes->size(); ++i) {
            if((*source_arg_syntaxes)[i].dependent) {
              drop_noncanonical_source_arguments = true;
              break;
            }
          }
        }
        if(!drop_noncanonical_source_arguments &&
           !source_template_id_is_qualified_member_owner &&
           !source_arg_syntaxes &&
           binding_arg_texts == &recovered_source_arg_texts) {
          const std::size_t compare_count =
              std::min(binding_arg_texts->size(), arguments.size());
          for(std::size_t i = 0; i < compare_count; ++i) {
            const std::string resolved_arg_text =
                arguments[i].kind == TemplateArgument::TA_TYPE &&
                arguments[i].type ?
                    describe_type(arguments[i].type) :
                    template_argument_text(arguments[i]);
            if(compact_lookup_text((*binding_arg_texts)[i]) !=
               compact_lookup_text(resolved_arg_text)) {
              drop_noncanonical_source_arguments = true;
              break;
            }
          }
        }
        if(drop_noncanonical_source_arguments) {
          if(parser_trace::enabled("template.resolve")) {
            parser_trace::note("template.resolve",
                               chosen_use_location,
                               "class-use-source-drop reason=noncanonical-source-arguments");
          }
          source_dependent_class_template_use_drops_.insert(
              source_dependent_drop_key);
          return;
        }
      }
      if(!source_use_spells_template &&
         (location_within_node(chosen_use_location, class_node) ||
          location_within_node(chosen_use_location,
                               resolved_info->source_template ?
                                   resolved_info->source_template->class_node :
                                   nullptr))) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             chosen_use_location,
                             "class-use-source-drop reason=inside-template-declaration");
        }
        return;
      }
      const std::string note_location =
          prefer_later_source_location(resolved_info->first_qualifier_use_location,
                                       chosen_use_location);
      const std::string public_location =
          template_api::normalize_template_witness_source_location(
              !note_location.empty() ? note_location : chosen_use_location);

      std::string qualified_template_name =
          !resolved_info->qualified_name.empty() ?
              strip_trailing_top_level_template_arguments(
                  template_api::class_witness_output_qualified_name(
                      ctx,
                      *resolved_info)) :
              decl.name;
      if(class_template_decl_is_member_template(decl) &&
         decl.declaring_scope) {
        const std::string declaring_scope_name =
            template_api::class_template_witness_qualified_name(ctx, decl);
        if(!declaring_scope_name.empty()) {
          qualified_template_name = declaring_scope_name;
        }
      }
      const std::string qualified_template_identifier =
          unqualified_member_name(qualified_template_name);
      if(source_capture_enabled &&
         !qualified_template_identifier.empty() &&
         qualified_template_identifier != anchor_identifier &&
         !source_location_points_at_identifier(chosen_use_location,
                                               qualified_template_identifier)) {
        return;
      }
      if(witness::enabled(witness_context) &&
         !explicit_specialization_source_use &&
         source_use_in_template_body &&
         class_template_decl_is_member_template(decl) &&
         !template_api::template_witness_qualified_member_type_lookup_active()) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             chosen_use_location,
                             "class-use-source-drop reason=member-template-body");
        }
        return;
      }
      if(witness::enabled(witness_context) &&
         !explicit_specialization_source_use &&
         class_template_decl_is_member_template(decl) &&
         !scope_has_template_placeholders(use_scope) &&
         scope_is_inside_source_template_context(use_scope) &&
         resolved_info->enclosing_scope &&
         resolved_info->enclosing_scope->class_info) {
        ClassInfo * source_owner =
            source_member_template_owner_class_from_syntax(use_scope,
                                                           chosen_use_location,
                                                           anchor_identifier);
        if(source_owner &&
           !semantic_lookup::is_same_or_derived(
               source_owner,
               resolved_info->enclosing_scope->class_info)) {
          if(parser_trace::enabled("template.resolve")) {
            parser_trace::note("template.resolve",
                               chosen_use_location,
                               "class-use-source-drop reason=member-template-owner source=" +
                                   source_owner->qualified_name +
                                   " selected=" +
                                   resolved_info->enclosing_scope->class_info
                                       ->qualified_name);
          }
          return;
        }
      }
      std::string selected_decl_location;
      {
        std::vector<const CppAstNode *> decl_candidates;
        if(specialization.kind == template_api::MS_PRIMARY) {
          if(const CppAstNode * member_decl =
                 find_member_class_template_declaration_node(resolved_info->source_template)) {
            decl_candidates.push_back(member_decl);
          }
          if(resolved_info->template_output_node) {
            decl_candidates.push_back(resolved_info->template_output_node);
          }
          if(resolved_info->source_template &&
             resolved_info->source_template->class_node) {
            decl_candidates.push_back(resolved_info->source_template->class_node);
          }
          if(class_node) {
            decl_candidates.push_back(class_node);
          }
        } else {
          if(class_node) {
            decl_candidates.push_back(class_node);
          }
          if(resolved_info->source_template &&
             resolved_info->source_template->class_node) {
            decl_candidates.push_back(resolved_info->source_template->class_node);
          }
          if(resolved_info->template_output_node) {
            decl_candidates.push_back(resolved_info->template_output_node);
          }
        }
        for(size_t i = 0; i < decl_candidates.size(); ++i) {
          if(!decl_candidates[i]) {
            continue;
          }
          selected_decl_location =
              source_location_for_name_in_node(*decl_candidates[i], decl.name);
          if(selected_decl_location.empty()) {
            selected_decl_location = source_location_for_node(*decl_candidates[i]);
          }
          selected_decl_location =
              template_api::normalize_template_witness_source_location(
                  selected_decl_location);
          if(!selected_decl_location.empty()) {
            break;
          }
        }
      }
      if(!selected_decl_location.empty() &&
         chosen_use_location == selected_decl_location) {
        return;
      }

      const std::string selection_text =
          specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION ?
              "explicit" :
              (specialization.kind == template_api::MS_PARTIAL_SPECIALIZATION ?
                   "partial" :
                   "primary");
      if(syntax_backed_source_capture_enabled &&
         (!nested_argument_recovery ||
          explicit_specialization_source_use ||
          source_template_id_is_qualified_member_owner)) {
        template_api::ClassSpecializationSelection visible_selection =
            specialization;
        const bool nested_partial_source_argument =
            source_use_spells_template &&
            specialization.kind == template_api::MS_PARTIAL_SPECIALIZATION &&
            callbacks.template_id_at_location_is_nested &&
            callbacks.template_id_at_location_is_nested(chosen_use_location) &&
            !(callbacks.template_id_at_location_is_qualified_member_owner &&
              callbacks.template_id_at_location_is_qualified_member_owner(
                  chosen_use_location));
        if(nested_partial_source_argument) {
          visible_selection.kind = template_api::MS_PRIMARY;
          visible_selection.parameters = &decl.parameters;
          visible_selection.arguments = arguments;
        }
        witness::ClassUseEmitRequest request;
        request.location = chosen_use_location;
        request.template_name = qualified_template_name;
        request.selection =
            source_selection_kind_for_match_kind(visible_selection.kind);
        if(explicit_specialization_source_use) {
          request.origin =
              witness::ClassUseEmissionOrigin::ExplicitSpecializationSource;
        }
        if(nested_partial_source_argument) {
          const witness::TemplateWitnessSourceAnchor primary_decl_anchor =
              class_use_selected_decl_anchor(ctx, &decl, visible_selection);
          request.selected_decl_location = primary_decl_anchor.location;
          request.selected_decl_anchor = primary_decl_anchor;
        } else {
          witness::set_selected_decl_anchor(request.selected_decl_location,
                                            request.selected_decl_anchor,
                                            selected_decl_location,
                                            true);
        }
        if(source_use_spells_template) {
          request.use_anchor_present = true;
          request.use_anchor_location = chosen_use_location;
          const bool source_use_is_current_specialization =
              class_template_use_is_current_specialization(use_scope, decl) ||
              location_within_node(chosen_use_location, class_node);
          const bool conversion_result_type_use =
              class_use_matches_current_conversion_result(use_scope,
                                                          resolved_info,
                                                          source_function) ||
              (callbacks.template_id_at_location_is_conversion_operator_result &&
               callbacks.template_id_at_location_is_conversion_operator_result(
                   chosen_use_location));
          const bool function_result_type_use =
              class_use_matches_current_function_result(use_scope,
                                                        resolved_info,
                                                        source_function);
          const bool binding_arg_texts_from_source_syntax =
              source_arg_syntaxes != nullptr;
          const bool binding_arg_texts_are_semantic_fallback =
              binding_arg_texts &&
              !binding_arg_texts_from_source_syntax &&
              !binding_arg_texts_from_source_template_id &&
              !binding_arg_texts_from_exact_anchor &&
              source_template_id_args_are_arity_compatible(*binding_arg_texts,
                                                          arguments);
          if(binding_arg_texts &&
             (binding_arg_texts_from_source_template_id ||
              binding_arg_texts_from_source_syntax ||
              binding_arg_texts_from_exact_anchor ||
              binding_arg_texts_are_semantic_fallback)) {
            semantic_source_use::SourceTemplateIdOccurrence occurrence =
                witness::make_source_template_id_occurrence(chosen_use_location,
                                                            *binding_arg_texts);
            occurrence.synthesized = binding_arg_texts_are_semantic_fallback;
            occurrence.in_template_body =
                scope_has_template_placeholders(use_scope) ||
                scope_is_inside_source_template_context(use_scope) ||
                witness::current_template_witness_entry_context().origin ==
                    witness::TemplateWitnessOrigin::Closure;
            occurrence.current_specialization_use =
                source_use_is_current_specialization;
            occurrence.conversion_result_type_use =
                conversion_result_type_use;
            occurrence.function_result_type_use =
                function_result_type_use;
            occurrence.has_dependent_argument =
                template_argument_texts_mention_source_bindings(*this,
                                                               use_scope,
                                                               *binding_arg_texts) ||
                template_argument_texts_mention_instantiated_class_local_type_aliases(
                    use_scope,
                    *binding_arg_texts) ||
                template_argument_texts_mention_template_bound_scope_names(
                    use_scope,
                    *binding_arg_texts) ||
                template_argument_texts_mention_enclosing_source_template_parameters(
                    use_scope,
                    *binding_arg_texts);
            if(!occurrence.current_specialization_use &&
               occurrence.in_template_body &&
               occurrence.has_dependent_argument &&
               explicit_specialization_source_use &&
               !selected_decl_location.empty() &&
               source_location_is_later(selected_decl_location,
                                        chosen_use_location)) {
              occurrence.current_specialization_use = true;
            }
            occurrence.has_current_specialization_argument =
                template_argument_texts_mention_current_specialization_names(
                    use_scope,
                    *binding_arg_texts);
            for(std::size_t i = 0; i < occurrence.arguments.size(); ++i) {
              std::vector<std::string> arg_texts;
              arg_texts.push_back(occurrence.arguments[i].text);
              occurrence.arguments[i].dependent =
                  template_argument_texts_mention_source_bindings(*this,
                                                                 use_scope,
                                                                 arg_texts) ||
                  template_argument_texts_mention_instantiated_class_local_type_aliases(
                      use_scope,
                      arg_texts) ||
                  template_argument_texts_mention_template_bound_scope_names(
                      use_scope,
                      arg_texts) ||
                  template_argument_texts_mention_enclosing_source_template_parameters(
                      use_scope,
                      arg_texts);
              if(occurrence.arguments[i].dependent) {
                occurrence.has_dependent_argument = true;
              }
              occurrence.arguments[i].current_specialization =
                  template_argument_texts_mention_current_specialization_names(
                      use_scope,
                      arg_texts);
              if(!occurrence.arguments[i].current_specialization) {
                if(source_arg_syntaxes && i < source_arg_syntaxes->size()) {
                  occurrence.arguments[i].current_specialization =
                      template_argument_mentions_current_specialization_member(
                          use_scope,
                          (*source_arg_syntaxes)[i]);
                } else if(i < arguments.size()) {
                  occurrence.arguments[i].current_specialization =
                      template_argument_mentions_current_specialization_member(
                          use_scope,
                          arguments[i]);
                }
              }
              if(occurrence.arguments[i].current_specialization) {
                occurrence.has_current_specialization_argument = true;
              }
              if(source_arg_syntaxes && i < source_arg_syntaxes->size()) {
                occurrence.arguments[i].referenced_value_entities =
                    template_argument_value_entities(use_scope,
                                                     (*source_arg_syntaxes)[i]);
                occurrence.arguments[i].referenced_value_decl_locations =
                    template_argument_value_decl_locations(
                        use_scope,
                        (*source_arg_syntaxes)[i]);
              }
              if(i < arguments.size()) {
                if(occurrence.arguments[i].referenced_value_entities.empty()) {
                  occurrence.arguments[i].referenced_value_entities =
                      template_argument_value_entities(use_scope, arguments[i]);
                }
                if(occurrence.arguments[i].referenced_value_decl_locations.empty()) {
                  occurrence.arguments[i].referenced_value_decl_locations =
                      template_argument_value_decl_locations(use_scope, arguments[i]);
                }
              }
            }
            if(occurrence.in_template_body &&
               binding_arg_texts_from_source_syntax &&
               !source_use_is_current_specialization &&
               !occurrence.has_dependent_argument &&
               !occurrence.has_current_specialization_argument &&
               template_argument_syntaxes_mention_source_template_context(
                   use_scope,
                   source_arg_syntaxes)) {
              occurrence.synthesized = true;
            }
            request.template_id_occurrence = occurrence;
          }
        }
        if(binding_arg_texts) {
          template_api::append_template_witness_source_bindings(
              *this,
              request.bindings,
              decl.parameters,
              arguments,
              *binding_arg_texts,
              "explicit",
              "defaulted");
        } else {
          template_api::append_template_witness_source_bindings(
              *this,
              request.bindings,
              decl.parameters,
              arguments,
              "explicit");
        }
        repair_bindings_from_resolved_instantiation(
            request.bindings,
            &request.template_id_occurrence);
        if(!nested_partial_source_argument &&
           bound_parameters &&
           bound_parameters != &decl.parameters) {
          template_api::append_template_witness_source_bindings(
              *this,
              request.specialization_bindings,
              *bound_parameters,
              *bound_arguments,
              "deduced");
        }
        request.ownership = witness::SourceUseOwnership::SourceOwned;
        if(request.origin ==
               witness::ClassUseEmissionOrigin::ResolvedTemplateId &&
           source_use_spells_template &&
           class_template_decl_is_member_template(decl)) {
          request.origin = witness::ClassUseEmissionOrigin::NestedSourceTemplateId;
        }
        const bool parent_class_use_records =
            witness::class_use_recording_enabled(witness_context,
                                                 request.origin);
        CPPGM_SET_WITNESS_PRODUCER(
            request,
            witness::WitnessProducerSite::ClassTemplateReference02);
        witness::emit_class_use(witness_context, request);
        if(parent_class_use_records &&
           !suppress_nested_arguments &&
           source_arg_syntaxes &&
           callbacks.emit_nested_class_use_source_events_from_syntaxes) {
          callbacks.emit_nested_class_use_source_events_from_syntaxes(
              use_scope,
              *source_arg_syntaxes,
              witness::SourceUseOwnership::SourceOwned,
              request.template_name);
        }
      }
      if(trace_enabled) {
        const auto append_binding_map =
            [&](std::ostringstream & out,
                const std::vector<TemplateParameterInfo> & parameters_to_use,
                const std::vector<TemplateArgument> & arguments_to_use) -> void
        {
          for(size_t i = 0; i < parameters_to_use.size() && i < arguments_to_use.size(); ++i) {
            if(i != 0) {
              out << ",";
            }
            out << (parameters_to_use[i].name.empty() ?
                        std::string("$") + std::to_string(i + 1) :
                        parameters_to_use[i].name)
                << ":" << template_argument_text(arguments_to_use[i]);
          }
        };

        std::ostringstream primary_bindings;
        append_binding_map(primary_bindings, decl.parameters, arguments);

        std::ostringstream specialize_bindings;
        if(bound_parameters && bound_parameters != &decl.parameters) {
          append_binding_map(specialize_bindings, *bound_parameters, *bound_arguments);
        }

        std::ostringstream trace;
        trace << "class-use"
              << " name=" << qualified_template_name
              << " resolved={" << resolved_info->qualified_name << "}"
              << " kind=" << selection_text
              << " decl=" << (!selected_decl_location.empty() ? selected_decl_location :
                               std::string("<none>"))
              << " bindings={" << primary_bindings.str() << "}";
        if(!specialize_bindings.str().empty()) {
          trace << " specialize_bindings={" << specialize_bindings.str() << "}";
        }
        parser_trace::note("template.resolve", public_location, trace.str());
      }
    };

    if(is_builtin_initializer_list_template(decl)) {
      ClassInfo * builtin_info =
          instantiate_selected_class_template(decl, use_scope, arguments, specialization);
      note_class_use(builtin_info);
      return builtin_info;
    }

    ClassInfo * info = nullptr;
    const bool lazy_references = lazy_class_template_references_enabled();
    auto found = decl.instantiations.find(key);
    auto reference_found =
        lazy_references ?
            decl.reference_instantiations.find(key) :
            decl.reference_instantiations.end();
    const bool found_existing =
        found != decl.instantiations.end() ||
        reference_found != decl.reference_instantiations.end();
    if(found_existing) {
      note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_hits);
      info = found != decl.instantiations.end() ? found->second : reference_found->second;
      if(semantic_hotspot::enabled()) {
        semantic_hotspot::note_semantic_query("reference_class_template_instantiation_hit",
                                              ensure_specialization_name());
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "reference-class-template-hit name=" << decl.name
              << " key=" << key;
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      template_audit::set_creation_context(
          *info, "reference_class_template_instantiation [" + decl.name + "]");
      const bool upgrades_dependent_reference =
          info->dependent_instantiation && !dependent_arguments;
      if(upgrades_dependent_reference) {
        if(semantic_hotspot::enabled()) {
          semantic_hotspot::note_semantic_query(
              "reference_class_template_instantiation_reset",
              ensure_specialization_name());
        }
        note_performance_counter(
            &semantic_metrics::AnalyzerCounters::class_template_resets);
        reset_instantiated_class_info(*info, decl.name, class_node);
        bind_declaring_owner_template_arguments_into_scope(ctx,
                                                           *info->member_scope,
                                                           binding_scope);
        template_api::binding::bind_template_arguments_into_scope(
            *this,
            *info->member_scope,
            *bound_parameters,
            *bound_arguments,
            bound_pack_sizes);
      }
      if(info->template_output_node != class_node) {
        const bool existing_is_forward =
            info->template_output_node &&
            info->template_output_node->kind == CppAstKind::class_forward_declaration;
        const bool selected_is_forward =
            class_node &&
            class_node->kind == CppAstKind::class_forward_declaration;
        const bool selected_demotes_specialization_to_primary =
            info->template_output_node &&
            class_node == decl.class_node &&
            info->template_output_node != decl.class_node &&
            !existing_is_forward &&
            !selected_is_forward;
        const bool selected_demotes_definition_to_forward =
            !existing_is_forward && selected_is_forward;
        const auto has_emitted_member_function =
            [&]() -> bool
            {
              for(map<string, vector<FunctionBinding *> >::const_iterator it =
                      info->methods.begin();
                  it != info->methods.end();
                  ++it) {
                for(size_t i = 0; i < it->second.size(); ++i) {
                  FunctionBinding * binding = it->second[i];
                  if(binding &&
                     (binding->output_emitted || binding->definition_output_emitted)) {
                    return true;
                  }
                }
              }
              if(info->member_scope) {
                for(map<string, vector<FunctionBinding *> >::const_iterator it =
                        info->member_scope->function_sets.begin();
                    it != info->member_scope->function_sets.end();
                    ++it) {
                  for(size_t i = 0; i < it->second.size(); ++i) {
                    FunctionBinding * binding = it->second[i];
                    if(binding &&
                       (binding->output_emitted || binding->definition_output_emitted)) {
                      return true;
                    }
                  }
                }
              }
              return false;
            };
        if(!selected_demotes_specialization_to_primary &&
           !selected_demotes_definition_to_forward &&
           !has_emitted_member_function()) {
          if(semantic_hotspot::enabled()) {
            semantic_hotspot::note_semantic_query("reference_class_template_instantiation_reset",
                                                  ensure_specialization_name());
          }
          note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_resets);
          reset_instantiated_class_info(*info, decl.name, class_node);
          bind_declaring_owner_template_arguments_into_scope(ctx,
                                                             *info->member_scope,
                                                             binding_scope);
          template_api::binding::bind_template_arguments_into_scope(*this,
                                                                   *info->member_scope,
                                                                   *bound_parameters,
                                                                   *bound_arguments,
                                                                   bound_pack_sizes);
        }
      }
      info->reentrant_primary_selection = specialization.reentrant_primary;
      const bool needs_instantiation_argument_refresh =
          template_api::record_class_template_instantiation_state(
              ctx,
              *info,
              key,
              arguments,
              specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION,
              decl.suppress_implicit_instantiation_definitions.find(key) !=
                      decl.suppress_implicit_instantiation_definitions.end() ||
                  (!decl.suppress_implicit_instantiation_definitions.empty() &&
                   decl.suppress_implicit_instantiation_definitions.find(
                       template_argument_key(arguments)) !=
                       decl.suppress_implicit_instantiation_definitions.end()),
              dependent_arguments,
              dependent_arguments ? source_arg_texts : nullptr,
              dependent_arguments ? source_arg_syntaxes : nullptr,
              selected_mangle_parameters,
              selected_mangle_arguments,
              selected_mangle_pack_sizes);
      if(needs_instantiation_argument_refresh) {
        note_performance_counter(
            &semantic_metrics::AnalyzerCounters::class_template_canonical_arg_text_builds);
      }
      record_class_template_binding_state(*info,
                                          *bound_arguments,
                                          bound_pack_sizes,
                                          specialization.kind ==
                                              template_api::MS_PRIMARY);
      record_selected_class_template_base_source_uses(decl, specialization);
      note_class_use(info);
      return info;
    }

    Scope * inst_scope =
        &template_api::binding::bind_class_template_arguments_for_instantiation(
            *this,
            *binding_scope,
            use_scope,
            *bound_parameters,
            *bound_arguments,
            bound_pack_sizes);
    const string create_specialization_name =
        dependent_arguments ? ensure_specialization_name() : string();
    const string & create_internal_specialization_name =
        ensure_internal_specialization_name();
    info = create_instantiated_class_info_with_internal_name(*inst_scope,
                                                             node_text(*class_key),
                                                             decl.name,
                                                             create_specialization_name,
                                                             create_internal_specialization_name,
                                                             &decl,
                                                             class_node,
                                                             false);
    info->reentrant_primary_selection = specialization.reentrant_primary;
    note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_creates);
    if(semantic_hotspot::enabled()) {
      std::ostringstream create_decl;
      create_decl << "name=" << decl.name
                  << " loc="
                  << (class_node ? source_location_for_node(*class_node) :
                                   std::string("<none>"))
                  << " selection="
                  << (class_node == decl.class_node ? "primary" : "specialization")
                  << " dependent=" << (dependent_arguments ? 1 : 0)
                  << " demand="
                  << semantic_metrics::class_demand_kind_name(
                         semantic_metrics::current_class_demand());
      semantic_hotspot::note_semantic_query(
          "reference_class_template_instantiation_create_decl",
          create_decl.str());
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "reference-class-template-create name=" << decl.name
            << " key=" << key;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(lazy_references) {
      decl.reference_instantiations[key] = info;
    } else {
      decl.instantiations[key] = info;
    }
    template_audit::set_creation_context(
        *info, "reference_class_template_instantiation [" + decl.name + "]");
    const bool refreshed_instantiation_arguments =
        template_api::record_class_template_instantiation_state(
            ctx,
            *info,
            key,
            arguments,
            specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION,
            decl.suppress_implicit_instantiation_definitions.find(key) !=
                    decl.suppress_implicit_instantiation_definitions.end() ||
                (!decl.suppress_implicit_instantiation_definitions.empty() &&
                 decl.suppress_implicit_instantiation_definitions.find(
                     template_argument_key(arguments)) !=
                     decl.suppress_implicit_instantiation_definitions.end()),
            dependent_arguments,
            dependent_arguments ? source_arg_texts : nullptr,
            dependent_arguments ? source_arg_syntaxes : nullptr,
            selected_mangle_parameters,
            selected_mangle_arguments,
            selected_mangle_pack_sizes);
    if(refreshed_instantiation_arguments) {
      note_performance_counter(
          &semantic_metrics::AnalyzerCounters::class_template_canonical_arg_text_builds);
    }
    record_class_template_binding_state(*info,
                                        *bound_arguments,
                                        bound_pack_sizes,
                                        specialization.kind ==
                                            template_api::MS_PRIMARY);
    bind_declaring_owner_template_arguments_into_scope(ctx,
                                                       *info->member_scope,
                                                       binding_scope);
    template_api::binding::bind_template_arguments_into_scope(*this,
                                                             *info->member_scope,
                                                             *bound_parameters,
                                                             *bound_arguments,
                                                             bound_pack_sizes);
    record_selected_class_template_base_source_uses(decl, specialization);
    if(lazy_references) {
      auto stored = decl.reference_instantiations.find(key);
      if(stored != decl.reference_instantiations.end()) {
        borrow_class_instantiation_key(*info, stored->first);
      }
    } else {
      auto stored = decl.instantiations.find(key);
      if(stored != decl.instantiations.end()) {
        borrow_class_instantiation_key(*info, stored->first);
      }
    }
    note_class_use(info);
    return info;
  }


  ClassInfo * instantiate_class_template(
      ClassTemplateDecl & decl,
      Scope & use_scope,
      const std::vector<std::string> & arg_texts,
      const std::vector<TemplateArgumentSyntax> * arg_syntaxes)
  {
    return instantiate_class_template_impl(decl, use_scope, arg_texts, arg_syntaxes);
  }

private:
  SemanticContext & ctx;
  const ClassTemplateReferenceCallbacks & callbacks;

  template_api::TemplateWitnessContext template_witness_context() const
  {
    return ctx.template_witness_context();
  }

  semantic_metrics::AnalyzerCounters * performance_counters()
  {
    return ctx.performance_counters();
  }

  void note_performance_counter(
      std::size_t semantic_metrics::AnalyzerCounters::* field,
      std::size_t amount = 1)
  {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      (counters->*field) += amount;
    }
  }

  std::vector<std::string> expand_bound_type_pack_texts(
      Scope & scope,
      const std::vector<std::string> & texts)
  {
    return ctx.expand_bound_type_pack_texts(scope, texts);
  }

  std::vector<std::string> expand_bound_expression_pack_texts(
      Scope & scope,
      const std::string & text)
  {
    return ctx.expand_bound_expression_pack_texts(scope, text);
  }

  bool resolve_template_arguments(Scope & scope,
                                  const std::vector<TemplateParameterInfo> & parameters,
                                  const std::vector<std::string> & texts,
                                  const std::vector<TemplateArgumentSyntax> * syntaxes,
                                  std::vector<TemplateArgument> & out,
                                  Scope * default_argument_declaring_scope)
  {
    return template_api::resolve_template_arguments(ctx,
                                                    scope,
                                                    parameters,
                                                    texts,
                                                    syntaxes,
                                                    out,
                                                    default_argument_declaring_scope);
  }

  bool resolve_template_argument(Scope & argument_scope,
                                 Scope & parameter_scope,
                                 const TemplateParameterInfo & parameter,
                                 const std::string & text,
                                 const TemplateArgumentSyntax * syntax,
                                 TemplateArgument & out)
  {
    return template_api::resolution::resolve_template_argument(ctx,
                                                              argument_scope,
                                                              parameter_scope,
                                                              parameter,
                                                              text,
                                                              syntax,
                                                              out);
  }

  void bind_single_template_argument_into_scope(
      Scope & scope,
      const TemplateParameterInfo & parameter,
      const TemplateArgument & argument)
  {
    ctx.bind_single_template_argument_into_scope(scope, parameter, argument);
  }

  void canonicalize_simple_dependent_argument_texts(
      std::vector<TemplateArgument> & arguments) const
  {
    template_api::canonicalize_simple_dependent_argument_texts(ctx, arguments);
  }

  bool template_arguments_are_dependent(
      const std::vector<TemplateArgument> & arguments) const
  {
    return template_api::template_arguments_are_dependent(ctx, arguments);
  }

  std::string template_argument_key(const std::vector<TemplateArgument> & arguments) const
  {
    return template_api::template_argument_identity_key(ctx, arguments);
  }

  std::string template_argument_text(const TemplateArgument & argument) const
  {
    return template_model::template_argument_text(
        argument,
        [this](const TypePtr & type)
        {
          return template_api::type::lookup_text_for_type_argument(ctx, type);
        });
  }

  std::vector<std::string> canonical_instantiation_arg_texts(
      const std::vector<TemplateArgument> & arguments) const
  {
    return template_api::canonical_template_argument_texts(ctx, arguments);
  }

  std::string display_template_specialization_name(
      const std::string & name,
      const std::vector<TemplateArgument> & arguments) const
  {
    return template_api::display_specialization_name_for_instantiation(
        ctx, name, arguments);
  }

  bool source_location_points_at_identifier(const std::string & location,
                                            const std::string & identifier) const
  {
    return template_api::template_witness_detail::
        source_location_points_at_identifier_token(ctx.template_witness_context(),
                                                   location,
                                                   identifier);
  }

  std::string source_location_for_token_index(std::size_t index) const
  {
    return callbacks.source_location_for_token_index(index);
  }

  bool token_index_for_source_location(const std::string & location,
                                       const std::string & token_source,
                                       std::size_t & out_index) const
  {
    return callbacks.token_index_for_source_location(location, token_source, out_index);
  }

  bool find_next_token_source_on_same_line(std::size_t start_index,
                                           const std::string & token_source,
                                           std::size_t & out_index) const
  {
    return callbacks.find_next_token_source_on_same_line(start_index,
                                                         token_source,
                                                         out_index);
  }

  bool template_argument_token_ranges_from_open(
      std::size_t open_index,
      std::vector<std::pair<std::size_t, std::size_t> > & ranges) const
  {
    return callbacks.template_argument_token_ranges_from_open(open_index, ranges);
  }

  const RecogToken & peek_token(std::size_t index) const
  {
    return callbacks.peek_token(index);
  }

  std::string source_location_for_name_in_node(const CppAstNode & node,
                                               const std::string & name,
                                               bool prefer_last = false) const
  {
    return ctx.source_location_for_name_in_node(node, name, prefer_last);
  }

  std::string source_location_for_node(const CppAstNode & node) const
  {
    return ctx.source_location_for_node(node);
  }

  ClassInfo * instantiate_class_template_impl(
      ClassTemplateDecl & decl,
      Scope & use_scope,
      const std::vector<std::string> & arg_texts,
      const std::vector<TemplateArgumentSyntax> * arg_syntaxes)
  {
    vector<TemplateArgument> arguments;
    const std::vector<std::string> arg_source_locations =
        class_template_argument_source_locations_for_current_use(decl.name,
                                                                 decl.parameters,
                                                                 arg_texts);
    const template_api::ScopedTemplateArgumentSourceLocations
        template_argument_source_locations(arg_texts, arg_source_locations);
    if(!resolve_template_arguments(
           use_scope, decl.parameters, arg_texts, arg_syntaxes, arguments, decl.declaring_scope)) {
      return nullptr;
    }
    canonicalize_simple_dependent_argument_texts(arguments);
    note_performance_counter(&semantic_metrics::AnalyzerCounters::class_template_key_builds);
    const std::string key =
        template_instantiation::class_template_argument_key_for_instantiation(
            ctx, decl, arguments);
    const template_api::ClassSpecializationSelection specialization =
        template_api::specialization::select_class_specialization(
            ctx,
            decl,
            use_scope,
            key,
            arguments);
    return instantiate_selected_class_template(decl, use_scope, arguments, specialization);
  }

  bool is_builtin_initializer_list_template(ClassTemplateDecl & decl) const
  {
    return ctx.is_builtin_initializer_list_template(decl);
  }

  ClassInfo * instantiate_selected_class_template(
      ClassTemplateDecl & decl,
      Scope & use_scope,
      const std::vector<TemplateArgument> & arguments,
      const template_api::ClassSpecializationSelection & specialization)
  {
    return ctx.instantiate_selected_class_template(decl, use_scope, arguments, specialization);
  }

  AliasTemplateDecl * lookup_alias_template(Scope & scope,
                                            const QualifiedName & name)
  {
    return ctx.lookup_alias_template(scope, name);
  }

  ClassTemplateDecl * lookup_class_template(Scope & scope,
                                            const QualifiedName & name)
  {
    return ctx.lookup_class_template(scope, name);
  }

  bool scope_has_template_placeholders(Scope & scope) const
  {
    return ctx.scope_has_template_placeholders(scope);
  }

  bool scope_is_inside_source_template_context(Scope & scope) const
  {
    return callsemantic::scope_is_inside_source_template_context(scope);
  }

  void record_selected_class_template_base_source_uses(
      ClassTemplateDecl & decl,
      const template_api::ClassSpecializationSelection & specialization)
  {
    callbacks.record_selected_class_template_base_source_uses(decl, specialization);
  }

  ClassInfo * create_instantiated_class_info_with_internal_name(
      Scope & scope,
      const std::string & class_kind,
      const std::string & template_name,
      const std::string & specialization_name,
      const std::string & internal_specialization_name,
      ClassTemplateDecl * source_template,
      const CppAstNode * output_node,
      bool track_output)
  {
    ClassTemplateInfoCreationRequest request;
    request.scope = &scope;
    request.class_kind = class_kind;
    request.template_name = template_name;
    request.specialization_name = specialization_name;
    request.internal_specialization_name = internal_specialization_name;
    request.template_decl = source_template;
    request.output_node = output_node;
    request.track_output = track_output;
    return ctx.create_instantiated_class_info(request);
  }

  void reset_instantiated_class_info(ClassInfo & info,
                                     const std::string & template_name,
                                     const CppAstNode * output_node)
  {
    ctx.reset_instantiated_class_info(info, template_name, output_node);
  }

  std::string make_template_specialization_name(
      const std::string & name,
      const std::vector<TemplateArgument> & arguments) const
  {
    return ctx.make_template_specialization_name(name, arguments);
  }
};

}  // namespace


std::vector<std::string> class_template_argument_source_locations_for_current_use(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    const std::string & template_name,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & arg_texts)
{
  ClassTemplateReference ref(ctx, callbacks);
  return ref.class_template_argument_source_locations_for_current_use(
      template_name, parameters, arg_texts);
}

ClassInfo * reference_class_template_instantiation(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<std::string> & arg_texts)
{
  ClassTemplateReference ref(ctx, callbacks);
  return ref.reference_class_template_instantiation(decl, use_scope, arg_texts);
}

ClassInfo * reference_class_template_instantiation_with_syntax(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::ClassTemplateSourceUseMode source_use_mode)
{
  ClassTemplateReference ref(ctx, callbacks);
  return ref.reference_class_template_instantiation_with_syntax(
      decl, use_scope, arg_texts, arg_syntaxes, source_use_mode);
}

ClassInfo * reference_selected_class_template_instantiation(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<TemplateArgument> & arguments,
    const template_api::ClassSpecializationSelection & specialization,
    const std::vector<std::string> * source_arg_texts,
    template_api::ClassTemplateSourceUseMode source_use_mode,
    const std::vector<TemplateArgumentSyntax> * source_arg_syntaxes,
    const std::string * precomputed_key,
    FunctionBinding * source_function)
{
  ClassTemplateReference ref(ctx, callbacks);
  return ref.reference_selected_class_template_instantiation(
      decl, use_scope, arguments, specialization, source_arg_texts,
      source_use_mode, source_arg_syntaxes, precomputed_key, source_function);
}

ClassInfo * reference_selected_class_template_instantiation_with_key(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<TemplateArgument> & arguments,
    const template_api::ClassSpecializationSelection & specialization,
    const std::vector<std::string> * source_arg_texts,
    const std::string * precomputed_key,
    template_api::ClassTemplateSourceUseMode source_use_mode,
    const std::vector<TemplateArgumentSyntax> * source_arg_syntaxes)
{
  ClassTemplateReference ref(ctx, callbacks);
  return ref.reference_selected_class_template_instantiation_with_key(
      decl, use_scope, arguments, specialization, source_arg_texts, precomputed_key,
      source_use_mode, source_arg_syntaxes);
}

ClassInfo * instantiate_class_template_with_syntax(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes)
{
  ClassTemplateReference ref(ctx, callbacks);
  return ref.instantiate_class_template(decl, use_scope, arg_texts, arg_syntaxes);
}

}  // namespace callsemantic
