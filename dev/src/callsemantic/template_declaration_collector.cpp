#include "callsemantic/template_declaration_collector.h"

#include "callsemantic/function_registry.h"
#include "callsemantic/source_location_utils.h"
#include "callsemantic/template_body_checks.h"
#include "callsemantic/template_source_utils.h"
#include "callsemantic_internal.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "cpp_decl_model.h"
#include "cpp_scope_lookup.h"
#include "cppast_ast.h"
#include "cppast_dump.h"
#include "pack_parameter_analysis.h"
#include "parser_trace.h"
#include "semantic_class_model.h"
#include "semantic_errors.h"
#include "semantic_hotspot.h"
#include "semantic_lookup.h"
#include "semantic_output.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "symbol_linkage.h"
#include "template_api.h"
#include "template_function_signature.h"
#include "template_model.h"
#include "template_scope.h"
#include "types.h"
#include "witness_api.h"

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace callsemantic {
namespace {

using namespace callsemantic_internal;
using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using semantic_class_model::declarator_function_qualifier;
using semantic_class_model::filtered_class_member_decl_specifiers;
using semantic_class_model::MethodSyntaxInfo;
using semantic_class_model::PreparedMethodParseContext;
using semantic_lookup::scope_qualified_name;
using template_model::TemplateArgument;
using cpp_decl::TemplateArgumentSyntax;
using template_model::TemplateParameterInfo;
using semantic_utils::strip_trailing_top_level_template_arguments;
using semantic_utils::split_qualified_name_text;
using semantic_utils::trim_space;
using semantic_utils::unqualified_member_name;

const CppAstNode * function_parameter_clause_in_declarator(const CppAstNode & node)
{
  const CppAstNode * found = nullptr;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::parameter_clause) {
      found = &child;
      continue;
    }
    if(child.kind == CppAstKind::nested_declarator && child.children.size() == 1) {
      if(const CppAstNode * nested =
             function_parameter_clause_in_declarator(child.children[0])) {
        found = nested;
      }
    }
  }
  return found;
}

bool declarator_has_direct_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return true;
    }
  }
  return false;
}

bool declarator_is_transparent_parenthesized_name(const CppAstNode & node)
{
  if(node.kind != CppAstKind::declarator) {
    return false;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::ptr_operator ||
       child.kind == CppAstKind::array_suffix ||
       child.kind == CppAstKind::parameter_clause) {
      return false;
    }
  }

  if(declarator_has_direct_child_kind(node, CppAstKind::identifier)) {
    return true;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::nested_declarator &&
       child.children.size() == 1 &&
       declarator_is_transparent_parenthesized_name(child.children[0])) {
      return true;
    }
  }
  return false;
}

bool declarator_declares_function_entity(const CppAstNode & node)
{
  if(node.kind != CppAstKind::declarator) {
    return false;
  }

  const bool has_direct_parameter_clause =
      declarator_has_direct_child_kind(node, CppAstKind::parameter_clause);
  if(has_direct_parameter_clause &&
     declarator_has_direct_child_kind(node, CppAstKind::identifier)) {
    return true;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::nested_declarator ||
       child.children.size() != 1) {
      continue;
    }
    if(has_direct_parameter_clause &&
       declarator_is_transparent_parenthesized_name(child.children[0])) {
      return true;
    }
    if(declarator_declares_function_entity(child.children[0])) {
      return true;
    }
  }
  return false;
}

class TemplateDeclarationCollector
{
public:
  TemplateDeclarationCollector(SemanticContext & ctx_in,
                               TemplateDeclarationCollectorState & state_in,
                               const TemplateDeclarationCollectorServices & services_in)
    : ctx(ctx_in),
      callbacks(services_in),
      class_templates(state_in.class_templates),
      alias_templates(state_in.alias_templates),
      function_templates(state_in.function_templates),
      variable_templates(state_in.variable_templates)
  {}

  operator SemanticContext &() { return ctx; }
  operator const SemanticContext &() const { return ctx; }

  void collect_template_declaration_impl(
      Scope & scope,
      const CppAstNode & node,
      MemberAccess access,
      const vector<TemplateParameterInfo> * inherited_template_parameters)
  {
    DIAG_CONTEXT("collect_template_declaration [" + spaced_node_text(node) + "]" +
                 source_location_for_node(node));
    if(semantic_hotspot::enabled()) {
      std::ostringstream query;
      query << source_location_for_node(node)
            << " scope=" << scope.instance_id;
      if(scope.class_info) {
        query << " owner=" << scope.class_info->qualified_name;
      }
      semantic_hotspot::note_semantic_query("collect_template_declaration_scope", query.str());
    }
    std::pair<std::set<const CppAstNode *>::iterator, bool> inserted =
        scope.collected_template_declarations.insert(&node);
    if(!inserted.second) {
      record_template_parameter_clause_source_uses(scope, node);
      return;
    }
    const CppAstNode * parameters = find_child_kind(node, CppAstKind::template_parameter_clause);
    if(!parameters || node.children.empty()) {
      throw logic_error("template-declaration missing children");
    }

    vector<TemplateParameterInfo> template_parameters;
    Scope & pattern_scope = append_template_scope(scope);
    string parameter_failure;
    bool parsed_template_parameters = false;
    {
      const witness::ScopedTemplateWitnessSourceCapturePause
          source_capture_pause;
      parsed_template_parameters =
          parse_template_parameters(*parameters,
                                    template_parameters,
                                    &pattern_scope,
                                    &parameter_failure);
    }
    if(!parsed_template_parameters) {
      ostringstream out;
      out << "unsupported template-parameter-clause";
      if(!node.children.empty()) {
        out << " for " << node_text(node.children.back());
      }
      out << " [clause " << node_text(*parameters) << "]";
      if(!parameter_failure.empty()) {
        out << " [reason " << parameter_failure << "]";
      }
      throw logic_error(out.str());
    }

    vector<TemplateParameterInfo> effective_template_parameters;
    if(inherited_template_parameters) {
      effective_template_parameters = *inherited_template_parameters;
    }
    effective_template_parameters.insert(effective_template_parameters.end(),
                                         template_parameters.begin(),
                                         template_parameters.end());
    const vector<TemplateParameterInfo> & owner_template_parameters =
        inherited_template_parameters ? *inherited_template_parameters : template_parameters;

    const CppAstNode & inner = node.children.back();
    if(inner.kind == CppAstKind::template_declaration) {
      collect_template_declaration_impl(pattern_scope,
                                        inner,
                                        access,
                                        &effective_template_parameters);
      return;
    }
    if(inner.kind == CppAstKind::deduction_guide_declaration) {
      collect_deduction_guide_declaration(
          scope, inner, &pattern_scope, &template_parameters);
      return;
    }
    if(inner.kind == CppAstKind::class_specifier ||
       inner.kind == CppAstKind::class_forward_declaration) {
      const CppAstNode * offending_template_parameter_redeclaration = nullptr;
      std::string offending_template_parameter_name;
      if(class_member_redeclares_template_parameter(inner,
                                                    template_parameters,
                                                    offending_template_parameter_redeclaration,
                                                    offending_template_parameter_name)) {
        throw logic_error(string("template parameter redeclared") +
                          semantic_trace::current_location_note(
                              *this,
                              offending_template_parameter_redeclaration ?
                                  offending_template_parameter_redeclaration :
                                  &inner));
      }
      const auto owner_template_member_class_definition =
          [&](Scope & current_scope,
              const string & class_name) -> const CppAstNode *
      {
        if(!current_scope.class_info || !current_scope.class_info->source_template) {
          return nullptr;
        }
        map<string, OutOfClassMemberClassDecl>::const_iterator found =
            current_scope.class_info->source_template->member_class_definitions.find(class_name);
        if(found == current_scope.class_info->source_template->member_class_definitions.end() ||
           !found->second.class_node ||
           found->second.class_node->kind == CppAstKind::class_forward_declaration) {
          return nullptr;
        }
        return found->second.class_node;
      };
      const QualifiedName * qualified_class_name = cppast_qualified_name_syntax(inner);
      QualifiedName flattened_qualified_class_name;
      const QualifiedName * effective_qualified_class_name = qualified_class_name;
      if(qualified_class_name && qualified_class_name->qualifiers.empty()) {
        if(split_qualified_name_text(qualified_class_name->name,
                                     flattened_qualified_class_name) &&
           !flattened_qualified_class_name.qualifiers.empty()) {
          effective_qualified_class_name = &flattened_qualified_class_name;
        }
      } else if(!qualified_class_name) {
        if(split_qualified_name_text(inner.value,
                                     flattened_qualified_class_name) &&
           !flattened_qualified_class_name.qualifiers.empty()) {
          effective_qualified_class_name = &flattened_qualified_class_name;
        }
      }
      if(effective_qualified_class_name &&
         !effective_qualified_class_name->qualifiers.empty()) {
        Scope * owner_scope = &scope;
        if(effective_qualified_class_name->qualifiers.size() > 1 ||
           effective_qualified_class_name->rooted) {
          QualifiedName owner_scope_name;
          owner_scope_name.rooted = effective_qualified_class_name->rooted;
          if(effective_qualified_class_name->qualifiers.size() > 1) {
            owner_scope_name.qualifiers.assign(
                effective_qualified_class_name->qualifiers.begin(),
                effective_qualified_class_name->qualifiers.end() - 2);
            owner_scope_name.name =
                effective_qualified_class_name->qualifiers[
                    effective_qualified_class_name->qualifiers.size() - 2];
          }
          owner_scope =
              semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
                  *this, scope, owner_scope_name);
        }

        string owner_template_name;
        if(owner_scope &&
           effective_qualified_class_name->name.find('<') == string::npos &&
           split_unqualified_template_head_text(
               effective_qualified_class_name->qualifiers.back(),
               owner_template_name)) {
          ClassTemplateDecl * owner_template =
              lookup_class_template(*owner_scope, owner_template_name);
          if(owner_template) {
            OutOfClassMemberClassDecl & stored =
                owner_template->member_class_definitions[
                    effective_qualified_class_name->name];
            if(stored.class_node &&
               stored.class_node != &inner &&
               stored.class_node->kind != CppAstKind::class_forward_declaration &&
               inner.kind != CppAstKind::class_forward_declaration) {
              throw logic_error(string("duplicate out-of-class nested class definition") +
                                semantic_trace::current_location_note(*this, &inner) +
                                semantic_trace::node_location_note(
                                    *this, "previous definition", stored.class_node));
            }
            stored.declaring_scope = &scope;
            stored.pattern_scope = &pattern_scope;
            stored.parameters = template_parameters;
            if(!stored.class_node ||
               stored.class_node->kind == CppAstKind::class_forward_declaration ||
               inner.kind != CppAstKind::class_forward_declaration) {
              stored.class_node = &inner;
            }
            record_template_parameter_clause_source_uses(scope, node);
            return;
          }
        }
      }

      const TemplateIdSyntax * class_template_id = cppast_template_id_syntax(inner);
      QualifiedName specialization_name;
      vector<string> arg_texts;
      if(template_parameters.empty()) {
        if(!class_template_id) {
          throw logic_error("unsupported class explicit specialization");
        }
        specialization_name = class_template_id->name;
        if(specialization_name.qualifiers.empty()) {
          QualifiedName split_name;
          if(split_qualified_name_text(specialization_name.name, split_name) &&
             !split_name.qualifiers.empty()) {
            specialization_name = split_name;
          }
        }
        arg_texts = template_id_argument_texts_preserving_spacing(*class_template_id);
        ClassTemplateDecl * primary =
            specialization_name.rooted || !specialization_name.qualifiers.empty() ?
                semantic_lookup::lookup_class_template(*this, scope, specialization_name) :
                lookup_class_template(scope, specialization_name.name);
        if(!primary) {
          throw logic_error("unknown class template explicit specialization");
        }
        vector<TemplateArgument> arguments;
        if(!resolve_template_arguments(
               scope,
               primary->parameters,
               arg_texts,
               &class_template_id->argument_syntaxes,
               arguments,
               primary->declaring_scope)) {
          throw logic_error("invalid class explicit specialization arguments");
        }
        const string specialization_key = template_argument_key(arguments);
        std::map<std::string, ClassInfo *>::const_iterator existing_instantiation =
            primary->instantiations.find(specialization_key);
        ClassInfo * existing_info =
            existing_instantiation != primary->instantiations.end() ?
                existing_instantiation->second :
                nullptr;
        if(!existing_info) {
          std::map<std::string, ClassInfo *>::const_iterator existing_reference =
              primary->reference_instantiations.find(specialization_key);
          if(existing_reference != primary->reference_instantiations.end()) {
            existing_info = existing_reference->second;
          }
        }
        if(existing_info &&
           !existing_info->first_qualifier_use_location.empty() &&
           source_location_is_later(existing_info->first_qualifier_use_location,
                                    source_location_for_node(inner))) {
          throw ExplicitSpecializationAfterInstantiationError(
              string("explicit specialization after instantiation") +
              semantic_trace::current_location_note(*this, &inner));
        }
        ClassTemplateSpecializationDecl & stored =
            primary->explicit_specializations[specialization_key];
        const bool stored_is_definition =
            stored.class_node &&
            stored.class_node->kind != CppAstKind::class_forward_declaration;
        const bool incoming_is_definition =
            inner.kind != CppAstKind::class_forward_declaration;
        if(stored_is_definition &&
           incoming_is_definition &&
           stored.class_node != &inner) {
          throw logic_error(string("duplicate class explicit specialization definition") +
                            semantic_trace::current_location_note(*this, &inner) +
                            semantic_trace::node_location_note(
                                *this, "previous definition", stored.class_node));
        }
        bool specialization_changed = false;
        if(!stored.class_node ||
           stored.class_node->kind == CppAstKind::class_forward_declaration ||
           incoming_is_definition) {
          stored.declaring_scope =
              primary->declaring_scope ? primary->declaring_scope : &scope;
          stored.class_node = &inner;
          specialization_changed = true;
        }
        if(specialization_changed) {
          ++primary->specialization_epoch;
        }
        record_template_parameter_clause_source_uses(scope, node);
        return;
      }

      if(class_template_id) {
        specialization_name = class_template_id->name;
        if(specialization_name.qualifiers.empty()) {
          QualifiedName split_name;
          if(split_qualified_name_text(specialization_name.name, split_name) &&
             !split_name.qualifiers.empty()) {
            specialization_name = split_name;
          }
        }
        arg_texts = template_id_argument_texts_preserving_spacing(*class_template_id);
        Scope * primary_scope = &scope;
        string owner_member_template_name;
        ClassTemplateDecl * owner_template_for_member_partial =
            resolve_dependent_owner_member_class_template_owner(
                scope,
                pattern_scope,
                specialization_name,
                owner_template_parameters,
                owner_member_template_name);
        if(specialization_name.rooted || !specialization_name.qualifiers.empty()) {
          if(specialization_name.qualifiers.empty()) {
            throw logic_error("unsupported class partial specialization");
          }
          QualifiedName target_scope_name;
          target_scope_name.rooted = specialization_name.rooted;
          target_scope_name.qualifiers = specialization_name.qualifiers;
          primary_scope =
              semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
                  *this, scope, target_scope_name, true);
          if(!primary_scope) {
            throw logic_error("unknown qualified class partial specialization scope");
          }
          pattern_scope.parent = primary_scope;
          pattern_scope.class_info = primary_scope->class_info;
          pattern_scope.function = primary_scope->function;
        }
        ClassTemplateDecl * primary =
            lookup_class_template(*primary_scope, specialization_name.name);
        if(!primary) {
          throw logic_error("unknown class template partial specialization");
        }
        vector<string> normalized_arg_texts;
        if(!fill_trailing_default_template_argument_texts(pattern_scope,
                                                          primary->parameters,
                                                          arg_texts,
                                                          primary->declaring_scope,
                                                          normalized_arg_texts)) {
          throw logic_error("invalid class partial specialization arguments");
        }

        for(size_t i = 0; i < primary->partial_specializations.size(); ++i) {
          PartialClassTemplateSpecializationDecl & existing =
              primary->partial_specializations[i];
          if(existing.arg_texts != normalized_arg_texts) {
            continue;
          }

          vector<TemplateParameterInfo> merged_parameters = existing.parameters;
          if(!merge_template_parameter_redeclarations(merged_parameters, template_parameters)) {
            throw logic_error(string("conflicting class partial specialization redeclaration") +
                              semantic_trace::current_location_note(*this, &inner) +
                              semantic_trace::node_location_note(
                                  *this, "previous declaration", existing.class_node));
          }
          if(existing.class_node &&
             existing.class_node != &inner &&
             existing.class_node->kind != CppAstKind::class_forward_declaration &&
             inner.kind != CppAstKind::class_forward_declaration) {
            throw logic_error(string("duplicate class partial specialization definition") +
                              semantic_trace::current_location_note(*this, &inner) +
                              semantic_trace::node_location_note(
                                  *this, "previous definition", existing.class_node));
          }
          if((!existing.class_node ||
              existing.class_node->kind == CppAstKind::class_forward_declaration) &&
             inner.kind != CppAstKind::class_forward_declaration) {
            prefer_incoming_template_parameter_spellings(merged_parameters,
                                                         template_parameters);
          }
          existing.parameters.swap(merged_parameters);
          existing.declaring_scope = &scope;
          existing.pattern_scope = &pattern_scope;
          if(!existing.class_node ||
             existing.class_node->kind == CppAstKind::class_forward_declaration) {
            existing.class_node = &inner;
          }
          record_template_parameter_clause_source_uses(scope, node);
          record_class_template_base_source_uses(existing.class_node,
                                                 existing.pattern_scope);
          ++primary->specialization_epoch;
          if(owner_template_for_member_partial) {
            record_owner_member_class_template_partial_specialization(
                *owner_template_for_member_partial,
                owner_member_template_name,
                existing);
          }
          return;
        }

        PartialClassTemplateSpecializationDecl partial;
        partial.declaring_scope = &scope;
        partial.pattern_scope = &pattern_scope;
        partial.class_node = &inner;
        partial.parameters = template_parameters;
        partial.arg_texts = normalized_arg_texts;
        partial.arg_syntaxes =
            normalized_template_argument_syntaxes(*class_template_id,
                                                  primary->parameters,
                                                  normalized_arg_texts);
        primary->partial_specializations.push_back(partial);
        if(owner_template_for_member_partial) {
          record_owner_member_class_template_partial_specialization(
              *owner_template_for_member_partial,
              owner_member_template_name,
              primary->partial_specializations.back());
        }
        ++primary->specialization_epoch;
        record_template_parameter_clause_source_uses(scope, node);
        record_class_template_base_source_uses(&inner,
                                               &pattern_scope);
        return;
      }

      Scope * class_template_scope = &scope;
      string class_template_name = inner.value;
      if(effective_qualified_class_name &&
         !effective_qualified_class_name->qualifiers.empty() &&
         effective_qualified_class_name->name.find('<') == string::npos) {
        bool namespace_qualified_class_template = true;
        for(size_t i = 0; i < effective_qualified_class_name->qualifiers.size(); ++i) {
          if(effective_qualified_class_name->qualifiers[i].find('<') != string::npos) {
            namespace_qualified_class_template = false;
            break;
          }
        }
        if(namespace_qualified_class_template) {
          QualifiedName target_scope_name;
          target_scope_name.rooted = effective_qualified_class_name->rooted;
          target_scope_name.qualifiers = effective_qualified_class_name->qualifiers;
          Scope * resolved_scope =
              semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
                  *this, scope, target_scope_name);
          if(!resolved_scope) {
            throw logic_error("unknown qualified class template scope");
          }
          class_template_scope = resolved_scope;
          class_template_name = effective_qualified_class_name->name;
          pattern_scope.parent = class_template_scope;
          pattern_scope.class_info = class_template_scope->class_info;
          pattern_scope.function = class_template_scope->function;
        }
      }

      const CppAstNode * effective_class_node = &inner;
      if(inner.kind == CppAstKind::class_forward_declaration) {
        if(class_template_scope == &scope) {
          const CppAstNode * member_definition =
              owner_template_member_class_definition(scope, inner.value);
          if(member_definition) {
            effective_class_node = member_definition;
          }
        }
      }
      if(effective_class_node &&
         effective_class_node->kind == CppAstKind::class_specifier) {
        const CppAstNode * offending_lookup_node = nullptr;
        std::string offending_lookup_name;
        bool has_invalid_nondependent_lookup = false;
        {
          const witness::ScopedTemplateWitnessSourceCapturePause
              source_capture_pause;
          has_invalid_nondependent_lookup =
              class_member_body_has_invalid_nondependent_lookup(
                  *this,
                  pattern_scope,
                  *effective_class_node,
                  template_parameters,
                  offending_lookup_node,
                  offending_lookup_name);
        }
        if(has_invalid_nondependent_lookup) {
          throw logic_error(string("unknown id-expression ") + offending_lookup_name +
                            semantic_trace::current_location_note(
                                *this,
                                offending_lookup_node ? offending_lookup_node :
                                                        effective_class_node));
        }
      }

      ClassTemplateDecl * existing =
          direct_class_template(*class_template_scope, class_template_name);
      const auto restore_reference_reset_witness_static_member_metadata =
          [&](ClassTemplateDecl & target)
      {
        ClassInfo * owner = class_template_scope->class_info;
        if(!owner) {
          return;
        }
        std::map<std::string, ClassTemplateDecl *>::iterator found =
            owner->reference_reset_witness_class_templates.find(target.name);
        if(found == owner->reference_reset_witness_class_templates.end() ||
           !found->second ||
           found->second == &target) {
          return;
        }
        ClassTemplateDecl & source = *found->second;
        target.witness_static_member_definitions.insert(
            source.static_member_definitions.begin(),
            source.static_member_definitions.end());
        const std::size_t partial_count =
            std::min(target.partial_specializations.size(),
                     source.partial_specializations.size());
        for(std::size_t i = 0; i < partial_count; ++i) {
          target.partial_specializations[i].witness_static_member_definitions.insert(
              source.partial_specializations[i].static_member_definitions.begin(),
              source.partial_specializations[i].static_member_definitions.end());
        }
        owner->reference_reset_witness_class_templates.erase(found);
      };
      if(existing) {
        vector<TemplateParameterInfo> merged_parameters = existing->parameters;
        if(!merge_template_parameter_redeclarations(merged_parameters, template_parameters)) {
          throw logic_error(string("conflicting class template redeclaration") +
                            semantic_trace::current_location_note(*this, &inner) +
                            semantic_trace::node_location_note(
                                *this, "previous declaration", existing->class_node));
        }
        if(existing->class_node &&
           existing->class_node != effective_class_node &&
           existing->class_node->kind != CppAstKind::class_forward_declaration &&
           effective_class_node->kind != CppAstKind::class_forward_declaration) {
          throw logic_error(string("duplicate class template definition") +
                            semantic_trace::current_location_note(*this, &inner) +
                            semantic_trace::node_location_note(
                                *this, "previous definition", existing->class_node));
        }
        if((!existing->class_node ||
            existing->class_node->kind == CppAstKind::class_forward_declaration) &&
           effective_class_node->kind != CppAstKind::class_forward_declaration) {
          prefer_incoming_template_parameter_spellings(merged_parameters, template_parameters);
        }
        existing->parameters.swap(merged_parameters);
        existing->pattern_scope = &pattern_scope;
        existing->comes_from_standard_include_path =
            existing->comes_from_standard_include_path ||
            ctx.node_comes_from_standard_include_path(effective_class_node);
        const bool replacing_primary_forward_declaration =
            existing->class_node &&
            existing->class_node->kind == CppAstKind::class_forward_declaration &&
            effective_class_node->kind != CppAstKind::class_forward_declaration;
        if(!existing->class_node ||
           existing->class_node->kind == CppAstKind::class_forward_declaration) {
          existing->class_node = effective_class_node;
        }
        if(replacing_primary_forward_declaration) {
          ++existing->specialization_epoch;
        }
        restore_reference_reset_witness_static_member_metadata(*existing);
        if(class_template_scope->class_info &&
           class_template_scope->class_info->source_template) {
          merge_pending_member_class_template_partial_specializations(
              *class_template_scope->class_info->source_template,
              class_template_name,
              class_template_scope->class_info,
              *existing);
        }
        record_template_parameter_clause_source_uses(scope, node);
        return;
      }

      unique_ptr<ClassTemplateDecl> decl(new ClassTemplateDecl());
      decl->declaring_scope = class_template_scope;
      decl->pattern_scope = &pattern_scope;
      decl->name = class_template_name;
      decl->class_node = effective_class_node;
      decl->parameters = template_parameters;
      decl->comes_from_standard_include_path =
          ctx.node_comes_from_standard_include_path(effective_class_node);
      class_template_scope->class_templates[decl->name] = decl.get();
      restore_reference_reset_witness_static_member_metadata(*decl);
      if(class_template_scope->class_info &&
         class_template_scope->class_info->source_template) {
        merge_pending_member_class_template_partial_specializations(
            *class_template_scope->class_info->source_template,
            class_template_name,
            class_template_scope->class_info,
            *decl);
      }
      class_templates.push_back(std::move(decl));
      record_template_parameter_clause_source_uses(scope, node);
      record_class_template_base_source_uses(effective_class_node,
                                             &pattern_scope);
      return;
    }

    if(inner.kind == CppAstKind::alias_declaration) {
      const CppAstNode * type_id = find_child_kind(inner, CppAstKind::type_id);
      if(!type_id) {
        throw logic_error("alias template missing type-id");
      }
      string type_id_text = node_text(*type_id);
      if(type_id_text.empty()) {
        type_id_text = spaced_node_text(*type_id);
      }
      const bool allow_deferred_type_pattern =
          (!type_id_text.empty() &&
           (text_mentions_template_placeholders(pattern_scope, type_id_text) ||
            text_mentions_dependent_non_namespace_binding_names(pattern_scope, type_id_text) ||
            type_id_text.find("...") != string::npos ||
            type_id_text.find("template ") != string::npos));

      unique_ptr<AliasTemplateDecl> decl(new AliasTemplateDecl());
      decl->declaring_scope = &scope;
      decl->pattern_scope = &pattern_scope;
      decl->name = inner.value;
      decl->type_id = type_id;
      decl->parameters = template_parameters;
      map<string, AliasTemplateDecl *>::iterator previous_alias =
          scope.alias_templates.find(decl->name);
      const bool had_previous_alias =
          previous_alias != scope.alias_templates.end();
      AliasTemplateDecl * previous_alias_decl =
          had_previous_alias ? previous_alias->second : nullptr;
      scope.alias_templates[decl->name] = decl.get();
      bool alias_template_committed = false;
      struct AliasTemplateRegistrationGuard
      {
        Scope & scope;
        string name;
        bool had_previous;
        AliasTemplateDecl * previous;
        bool & committed;

        ~AliasTemplateRegistrationGuard()
        {
          if(committed) {
            return;
          }
          if(had_previous) {
            scope.alias_templates[name] = previous;
          } else {
            scope.alias_templates.erase(name);
          }
        }
      } alias_template_registration_guard{
          scope,
          decl->name,
          had_previous_alias,
          previous_alias_decl,
          alias_template_committed};
      try {
        const ScopedSuppressedTemplateUseLocation suppressed_use_location;
        parse_type_id(pattern_scope,
                      *type_id,
                      decl->resolved_type_pattern,
                      true);
      } catch(const TemplateSubstitutionFailure &) {
        if(!allow_deferred_type_pattern) {
          throw;
        }
      }
      alias_templates.push_back(std::move(decl));
      alias_template_committed = true;
      record_template_parameter_clause_source_uses(scope, node);
      return;
    }

    if((inner.kind == CppAstKind::special_member_definition ||
        inner.kind == CppAstKind::special_member_declaration) &&
       !scope.class_info) {
      record_template_parameter_clause_source_uses(scope, node);
      const CppAstNode * declarator = find_child_kind(inner, CppAstKind::declarator);
      if(!declarator) {
        throw logic_error("templated special-member missing declarator");
      }

      Scope * parse_scope = resolve_qualified_function_parse_scope(pattern_scope, *declarator);
      Scope overlay_scope(parse_scope, "", false);
      overlay_scope.class_info = parse_scope->class_info;
      overlay_scope.function = parse_scope->function;
      overlay_direct_scope_bindings(overlay_scope, pattern_scope);
      Scope * effective_parse_scope = parse_scope == &pattern_scope ? parse_scope : &overlay_scope;
      if(is_conversion_function_name(inner.value)) {
        string name;
        TypePtr type;
        MethodSyntaxInfo syntax;
        vector<pair<string, TypePtr> > params;
        if(!semantic_class_model::parse_conversion_operator_signature(*this,
                                                                      *effective_parse_scope,
                                                                      inner,
                                                                      name,
                                                                      type,
                                                                      params,
                                                                      nullptr,
                                                                      &syntax)) {
          throw logic_error("unsupported templated conversion-operator signature");
        }

        if(!template_parameters.empty()) {
          FunctionTemplateDecl * template_decl =
              resolve_out_of_class_method_template(pattern_scope,
                                                   inner.value,
                                                   name,
                                                   effective_template_parameters,
                                                   params,
                                                   syntax.is_const_method,
                                                   syntax.is_volatile_method,
                                                   syntax.ref_qualifier);
          if(template_decl) {
            if(inner.kind == CppAstKind::special_member_definition) {
              if(template_decl->body) {
                throw logic_error(string("duplicate templated conversion-operator definition") +
                                  semantic_trace::current_location_note(*this, declarator) +
                                  semantic_trace::node_location_note(
                                      *this,
                                      "previous definition",
                                      template_decl->body ? template_decl->body
                                                          : template_decl->declarator));
              }
              template_decl->body = find_function_body_node(inner);
              template_decl->definition_node = &inner;
              template_decl->definition_inner = &inner;
              template_decl->definition_specifiers =
                  find_child_kind(inner, CppAstKind::decl_specifier_seq);
              template_decl->definition_declarator = declarator;
              record_definition_parameter_aliases(*template_decl, params);
            }
            return;
          }
        }

        FunctionBinding * binding = nullptr;
        const callsemantic::ScopedExactTemplateTypeLookupAnchor conv_owner_anchor(
            owner_lookup_anchor_for_node(inner));
        if(!resolve_out_of_class_named_method_binding(pattern_scope,
                                                      inner.value,
                                                      name,
                                                      type,
                                                      syntax.is_const_method,
                                                      syntax.is_volatile_method,
                                                      syntax.ref_qualifier,
                                                      binding)) {
          throw logic_error("missing templated conversion-operator binding");
        }

        if(inner.kind == CppAstKind::special_member_definition) {
          if(binding->has_definition) {
            throw logic_error(string("duplicate class member definition") +
                              semantic_trace::current_location_note(*this, declarator) +
                              semantic_trace::previous_function_location_note(
                                  *this, "previous definition", binding));
          }
          binding->body = find_function_body_node(inner);
          record_definition_parameter_aliases(*binding, params);
          binding->has_definition = binding->body != nullptr;
          binding->definition_node = declarator;
          binding->definition_abi_tags.clear();
          append_function_declaration_abi_tags(binding->definition_abi_tags, &inner);
        }
        if(!template_parameters.empty()) {
          ClassInfo * owner = binding->owner_class;
          if(owner && owner->member_scope) {
            ClassTemplateDecl * owner_template_decl = owner->source_template;
            if(!owner_template_decl && owner->enclosing_scope) {
              owner_template_decl =
                  lookup_class_template(*owner->enclosing_scope, owner->name);
            }
            if(owner_template_decl &&
               out_of_class_special_member_template_parameters_match(*owner->member_scope,
                                                                    owner_template_decl->parameters,
                                                                    pattern_scope,
                                                                    owner_template_parameters)) {
              const QualifiedName * qualified_member = cppast_qualified_name_syntax(inner);
              if(qualified_member && qualified_member->qualifiers.size() > 0) {
                vector<OutOfClassMemberFunctionDecl> & stored_defs =
                    owner_template_decl->member_function_definitions[qualified_member->name];
                bool already_stored = false;
                for(size_t stored_index = 0; stored_index < stored_defs.size(); ++stored_index) {
                  if(stored_defs[stored_index].declarator == declarator) {
                    already_stored = true;
                    break;
                  }
                }
                if(!already_stored) {
                  OutOfClassMemberFunctionDecl stored;
                  stored.declaring_scope = &scope;
                  stored.pattern_scope = &pattern_scope;
                  stored.qualified_name = inner.value;
                  stored.qualified_name_syntax = *qualified_member;
                  stored.specifiers = find_child_kind(inner, CppAstKind::decl_specifier_seq);
                  stored.declarator = declarator;
                  stored.body = find_function_body_node(inner);
                  stored.ctor_initializer = nullptr;
                  stored.declared_type_pattern = type;
                  stored.params = params;
                  stored.is_const_method = syntax.is_const_method;
                  stored.is_volatile_method = syntax.is_volatile_method;
                  stored.ref_qualifier = syntax.ref_qualifier;
                  stored.parameters = owner_template_parameters;
                  stored_defs.push_back(stored);
                  invalidate_out_of_class_definition_caches(*owner_template_decl);
                }
              }
            }
          }
        }
        return;
      }
      vector<pair<string, TypePtr> > params;
      const CppAstNode * parameter_clause =
          find_child_kind(*declarator, CppAstKind::parameter_clause);
      if(parameter_clause &&
           !parse_parameter_clause(*effective_parse_scope, *parameter_clause, params, nullptr, true)) {
        const CppAstNode * special_member_body = find_function_body_node(inner);
        if(special_member_body &&
           special_member_body->kind == CppAstKind::lazy_function_body) {
          return;
        }
        ostringstream out;
        out << "unsupported templated special-member parameter-clause";
        out << " [parse-scope "
            << scope_qualified_name(*effective_parse_scope, "<here>") << "]";
        out << " [parse-bindings "
            << describe_scope_bindings_for_diagnostic(*effective_parse_scope) << "]";
        if(parse_scope && parse_scope->class_info) {
          out << " [owner-class " << parse_scope->class_info->qualified_name << "]";
          }
          throw logic_error(out.str());
        }

      QualifiedName parsed_special_member;
      const QualifiedName * qualified_special_member =
          parse_out_of_class_member_qualified_name(inner.value, parsed_special_member) ?
              &parsed_special_member :
              nullptr;
      QualifiedName declarator_qualified_special_member;
      const CppAstNode * declarator_identifier =
          declarator ? find_child_kind(*declarator, CppAstKind::identifier) : nullptr;
      if(declarator_identifier &&
         declarator_identifier->value.find("::") != string::npos &&
         declarator_identifier->value.find("operator") != string::npos &&
         split_qualified_name_text(declarator_identifier->value,
                                   declarator_qualified_special_member) &&
         !declarator_qualified_special_member.qualifiers.empty()) {
        qualified_special_member = &declarator_qualified_special_member;
      }

        auto record_out_of_class_special_member_for_owner_template =
            [&](ClassInfo * owner,
                bool exclude_from_explicit_instantiation,
                FunctionTemplateDecl * member_template_decl) -> bool
        {
          if(!owner && qualified_special_member) {
            owner = resolve_out_of_class_owner_class(pattern_scope,
                                                     *qualified_special_member);
          }
          if(qualified_special_member == nullptr) {
            return false;
          }
          const QualifiedName & qualified_member = *qualified_special_member;

          ClassTemplateDecl * owner_template_decl = nullptr;
          Scope * owner_template_scope = nullptr;
          if(owner && owner->member_scope) {
            owner_template_decl = owner->source_template;
            if(!owner_template_decl && owner->enclosing_scope) {
              owner_template_decl =
                  lookup_class_template(*owner->enclosing_scope, owner->name);
            }
            owner_template_scope = owner->member_scope.get();
          }
          if(!owner_template_decl) {
            for(std::size_t qualifier_index = qualified_member.qualifiers.size();
                qualifier_index > 0;
                --qualifier_index) {
              std::string owner_template_name;
              if(!split_unqualified_template_head_text(
                     qualified_member.qualifiers[qualifier_index - 1],
                     owner_template_name)) {
                continue;
              }

              Scope * owner_lookup_scope = &pattern_scope;
              if(qualifier_index > 1 || qualified_member.rooted) {
                QualifiedName owner_scope_name;
                owner_scope_name.rooted = qualified_member.rooted;
                if(qualifier_index > 1) {
                  owner_scope_name.qualifiers.assign(qualified_member.qualifiers.begin(),
                                                     qualified_member.qualifiers.begin() +
                                                         (qualifier_index - 1));
                  owner_scope_name.name = qualified_member.qualifiers[qualifier_index - 2];
                }
                owner_lookup_scope =
                    semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
                        *this, pattern_scope, owner_scope_name);
              }
              if(!owner_lookup_scope) {
                continue;
              }

              owner_template_decl =
                  lookup_class_template(*owner_lookup_scope, owner_template_name);
              if(owner_template_decl) {
                owner_template_scope = owner_lookup_scope;
                break;
              }
            }
          }
          const bool partial_owner =
              owner_template_decl &&
              owner &&
              owner->template_output_node &&
              owner->template_output_node != owner_template_decl->class_node;
          PartialClassTemplateSpecializationDecl * owner_partial_decl =
              partial_owner && owner_template_decl ?
                  find_partial_specialization_decl(*owner_template_decl, owner) :
                  nullptr;
          Scope * owner_partial_scope =
              owner_partial_decl ?
                  (owner_partial_decl->pattern_scope ?
                       owner_partial_decl->pattern_scope :
                       owner_partial_decl->declaring_scope) :
                  nullptr;
          const bool matches_owner_template_parameters =
              owner_template_decl &&
              owner_template_scope &&
              out_of_class_special_member_template_parameters_match(*owner_template_scope,
                                                                   owner_template_decl->parameters,
                                                                   pattern_scope,
                                                                   owner_template_parameters);
          const bool matches_partial_owner_template_parameters =
              owner_partial_scope &&
              out_of_class_special_member_template_parameters_match(*owner_partial_scope,
                                                                   owner_partial_decl->parameters,
                                                                   pattern_scope,
                                                                   owner_template_parameters);
            if(!owner_template_decl ||
               (!matches_owner_template_parameters &&
                !matches_partial_owner_template_parameters)) {
            return false;
          }

          if(member_template_decl) {
            std::vector<OutOfClassMemberFunctionTemplateDefinition> & stored_defs =
                (matches_partial_owner_template_parameters && owner_partial_decl ?
                     owner_partial_decl->member_function_template_definitions[qualified_member.name] :
                     owner_template_decl->member_function_template_definitions[qualified_member.name]);
            for(std::size_t stored_index = 0; stored_index < stored_defs.size(); ++stored_index) {
              if(stored_defs[stored_index].declaration == member_template_decl) {
                stored_defs[stored_index].definition_node = &inner;
                stored_defs[stored_index].definition_specifiers =
                    find_child_kind(inner, CppAstKind::member_specifiers);
                stored_defs[stored_index].definition_declarator = declarator;
                stored_defs[stored_index].body =
                    find_function_body_node(inner);
                stored_defs[stored_index].ctor_initializer =
                    find_child_kind(inner, CppAstKind::ctor_initializer);
                stored_defs[stored_index].parameter_aliases_pattern =
                    member_template_decl->parameter_aliases_pattern;
                invalidate_out_of_class_definition_caches(*owner_template_decl);
                return true;
              }
            }
            OutOfClassMemberFunctionTemplateDefinition stored_def;
            stored_def.declaration = member_template_decl;
            stored_def.definition_node = &inner;
            stored_def.definition_specifiers =
                find_child_kind(inner, CppAstKind::member_specifiers);
            stored_def.definition_declarator = declarator;
            stored_def.body = find_function_body_node(inner);
            stored_def.ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
            stored_def.parameter_aliases_pattern = member_template_decl->parameter_aliases_pattern;
            stored_defs.push_back(stored_def);
            invalidate_out_of_class_definition_caches(*owner_template_decl);
              emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                        inner.value,
                                                        &inner,
                                                        owner,
                                                        &owner_template_parameters);
            return true;
          }

          std::vector<OutOfClassMemberFunctionDecl> & stored_defs =
              (matches_partial_owner_template_parameters && owner_partial_decl ?
                   owner_partial_decl->member_function_definitions[qualified_member.name] :
                   owner_template_decl->member_function_definitions[qualified_member.name]);
          for(std::size_t stored_index = 0; stored_index < stored_defs.size(); ++stored_index) {
            if(stored_defs[stored_index].declarator == declarator) {
              return true;
            }
          }

          OutOfClassMemberFunctionDecl stored;
          stored.declaring_scope = &scope;
          stored.pattern_scope = &pattern_scope;
          stored.qualified_name = inner.value;
          stored.qualified_name_syntax = qualified_member;
          stored.owner_output_node =
              matches_partial_owner_template_parameters && owner_partial_decl ?
                  owner_partial_decl->class_node :
                  (owner && owner->template_output_node ?
                       owner->template_output_node :
                       (owner && owner->class_node ? owner->class_node :
                                                    owner_template_decl->class_node));
          stored.specifiers = find_child_kind(inner, CppAstKind::decl_specifier_seq);
          stored.declarator = declarator;
          stored.body = find_function_body_node(inner);
          stored.ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
          stored.declared_type_pattern = TypePtr();
          stored.params = params;
          stored.is_const_method = false;
          stored.is_volatile_method = false;
          stored.ref_qualifier = RQ_NONE;
          stored.exclude_from_explicit_instantiation = exclude_from_explicit_instantiation;
          stored.parameters = owner_template_parameters;
          stored_defs.push_back(stored);
          invalidate_out_of_class_definition_caches(*owner_template_decl);
            emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                      qualified_member,
                                                      inner.value,
                                                      &inner,
                                                      owner,
                                                      &owner_template_parameters);
          return true;
        };

        if(!template_parameters.empty()) {
            FunctionTemplateDecl * template_decl =
              qualified_special_member ?
                  resolve_out_of_class_special_member_template(pattern_scope,
                                                               *qualified_special_member,
                                                               effective_template_parameters,
                                                               params) :
                  nullptr;
        if(template_decl) {
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "record-out-of-class-special-member-template name=" << inner.value
                  << " exclude="
                  << (template_decl->exclude_from_explicit_instantiation ? "yes" : "no");
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          if(inner.kind == CppAstKind::special_member_definition) {
            if(template_decl->body) {
              const CppAstNode * current_body =
                  find_function_body_node(inner);
              const bool same_body_location =
                  current_body &&
                  template_decl->body &&
                  source_location_for_node(*template_decl->body) ==
                      source_location_for_node(*current_body);
              const bool same_declarator_location =
                  template_decl->declarator &&
                  declarator &&
                  source_location_for_node(*template_decl->declarator) ==
                      source_location_for_node(*declarator);
              if(same_body_location || same_declarator_location) {
                return;
              }
              throw logic_error(string("duplicate templated special-member definition") +
                                " [selected-template-params " +
                                describe_template_parameter_infos(template_decl->parameters) + "]" +
                                " [incoming-template-params " +
                                describe_template_parameter_infos(template_parameters) + "]" +
                                semantic_trace::current_location_note(*this, declarator) +
                                semantic_trace::node_location_note(
                                    *this,
                                    "previous definition",
                                    template_decl->body ? template_decl->body
                                                        : template_decl->declarator));
            }
            template_decl->body = find_function_body_node(inner);
            template_decl->definition_node = &inner;
            template_decl->definition_inner = &inner;
            template_decl->definition_specifiers =
                find_child_kind(inner, CppAstKind::member_specifiers);
            template_decl->definition_declarator = declarator;
            record_definition_parameter_aliases(*template_decl, params);
            template_decl->ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
          }
            record_out_of_class_special_member_for_owner_template(
                template_decl->declaring_scope ? template_decl->declaring_scope->class_info
                                               : parse_scope->class_info,
                template_decl->exclude_from_explicit_instantiation,
                template_decl);
            if(qualified_special_member) {
              emit_out_of_class_owner_class_use_if_needed(
                  pattern_scope,
                  *qualified_special_member,
                  inner.value,
                  &inner,
                  template_decl->declaring_scope ?
                      template_decl->declaring_scope->class_info :
                      parse_scope->class_info,
                  &owner_template_parameters);
            } else {
              emit_out_of_class_owner_class_use_if_needed(
                  pattern_scope,
                  inner.value,
                  &inner,
                  template_decl->declaring_scope ?
                      template_decl->declaring_scope->class_info :
                      parse_scope->class_info,
                  &owner_template_parameters);
            }
          return;
        }

        FunctionBinding * binding = nullptr;
        if(!(qualified_special_member &&
             resolve_out_of_class_special_member_binding(pattern_scope,
                                                        *qualified_special_member,
                                                        params,
                                                        binding))) {
          if(record_out_of_class_special_member_for_owner_template(nullptr, false, nullptr)) {
              if(qualified_special_member) {
                emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                            *qualified_special_member,
                                                            inner.value,
                                                            &inner,
                                                            nullptr,
                                                            &owner_template_parameters);
              } else {
                emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                            inner.value,
                                                            &inner,
                                                            nullptr,
                                                            &owner_template_parameters);
              }
            return;
          }
          throw logic_error(string("missing templated special-member binding") +
                              (qualified_special_member ?
                                   describe_out_of_class_special_member_template_lookup(
                                       pattern_scope,
                                       *qualified_special_member,
                                       effective_template_parameters,
                                       params) :
                                   describe_out_of_class_special_member_template_lookup(
                                       pattern_scope,
                                       inner.value,
                                       effective_template_parameters,
                                       params)) +
                              (qualified_special_member ?
                                   describe_out_of_class_special_member_binding_lookup(
                                       pattern_scope,
                                       *qualified_special_member,
                                       params) :
                                   describe_out_of_class_special_member_binding_lookup(
                                       pattern_scope,
                                       inner.value,
                                       params)));
        }
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "record-out-of-class-special-member-binding name=" << inner.value
                  << " exclude="
                  << (binding->exclude_from_explicit_instantiation ? "yes" : "no");
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          if(qualified_special_member) {
            emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                        *qualified_special_member,
                                                        inner.value,
                                                        &inner,
                                                        binding ? binding->owner_class : nullptr,
                                                        &owner_template_parameters);
          } else {
            emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                        inner.value,
                                                        &inner,
                                                        binding ? binding->owner_class : nullptr,
                                                        &owner_template_parameters);
          }

          if(inner.kind == CppAstKind::special_member_definition) {
            const CppAstNode * current_body = find_function_body_node(inner);
            if(!explicit_function_nothrow_specifications_match(
                   *binding,
                   declarator_function_qualifier(*declarator))) {
              throw logic_error(string("mismatched function exception specification [tdc-a]") +
                                semantic_trace::current_location_note(*this, declarator) +
                                semantic_trace::previous_function_location_note(
                                    *this, "previous declaration", binding));
            }
            if(binding->has_definition) {
              const bool same_lazy_definition_replay =
                  current_body &&
                  current_body->kind == CppAstKind::lazy_function_body &&
                  binding->body &&
                  binding->body->kind == CppAstKind::lazy_function_body &&
                  ((binding->definition_node &&
                    declarator &&
                    source_location_for_node(*binding->definition_node) ==
                        source_location_for_node(*declarator)) ||
                   source_location_for_node(*binding->body) ==
                       source_location_for_node(*current_body));
              if(same_lazy_definition_replay) {
                return;
              }
            throw logic_error(string("duplicate class member definition") +
                              semantic_trace::current_location_note(*this, declarator) +
                              semantic_trace::previous_function_location_note(
                                  *this, "previous definition", binding));
          }
          refresh_definition_parameter_names(*binding, params);
          binding->body = current_body;
          record_definition_parameter_aliases(*binding, params);
            binding->ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
            binding->has_definition = binding->body != nullptr;
            binding->definition_node = declarator;
          binding->definition_abi_tags.clear();
          append_function_declaration_abi_tags(binding->definition_abi_tags, &inner);
          upgrade_function_symbol_linkage(
              binding,
              function_symbol_linkage(pattern_scope,
                                      &inner,
                                      binding->body,
                                      false,
                                      binding->function_qualifier,
                                      template_api::function_binding_registration_identity(
                                          *binding),
                                      false,
                                      binding->lexical_access_class));
          }
            record_out_of_class_special_member_for_owner_template(
                binding->owner_class,
                binding->exclude_from_explicit_instantiation,
                nullptr);
          return;
        }

      FunctionBinding * binding = nullptr;
      if(!(qualified_special_member &&
           resolve_out_of_class_special_member_binding(pattern_scope,
                                                      *qualified_special_member,
                                                      params,
                                                      binding))) {
        throw logic_error("missing templated special-member binding");
      }
      emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                  *qualified_special_member,
                                                  inner.value,
                                                  &inner,
                                                  binding ? binding->owner_class : nullptr);
      if(template_parameters.empty() &&
         binding->owner_class &&
         binding->owner_class->is_explicit_specialization) {
        throw logic_error(string("member of explicit class specialization must not use template<>") +
                          semantic_trace::current_location_note(*this, declarator));
      }

      if(inner.kind == CppAstKind::special_member_definition) {
        const CppAstNode * current_body = find_function_body_node(inner);
        if(!explicit_function_nothrow_specifications_match(
               *binding,
               declarator_function_qualifier(*declarator))) {
          throw logic_error(string("mismatched function exception specification [tdc-b]") +
                            semantic_trace::current_location_note(*this, declarator) +
                            semantic_trace::previous_function_location_note(
                                *this, "previous declaration", binding));
        }
        if(binding->has_definition) {
          const bool same_lazy_definition_replay =
              current_body &&
              current_body->kind == CppAstKind::lazy_function_body &&
              binding->body &&
              binding->body->kind == CppAstKind::lazy_function_body &&
              ((binding->definition_node &&
                declarator &&
                source_location_for_node(*binding->definition_node) ==
                    source_location_for_node(*declarator)) ||
               source_location_for_node(*binding->body) ==
                   source_location_for_node(*current_body));
          if(same_lazy_definition_replay) {
            return;
          }
          throw logic_error(string("duplicate class member definition") +
                            semantic_trace::current_location_note(*this, declarator) +
                            semantic_trace::previous_function_location_note(
                                *this, "previous definition", binding));
        }
        refresh_definition_parameter_names(*binding, params);
        binding->body = current_body;
        record_definition_parameter_aliases(*binding, params);
        binding->ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
        binding->has_definition = binding->body != nullptr;
        binding->definition_node = declarator;
        binding->definition_abi_tags.clear();
        append_function_declaration_abi_tags(binding->definition_abi_tags, &inner);
        upgrade_function_symbol_linkage(
            binding,
            function_symbol_linkage(pattern_scope,
                                    &inner,
                                    binding->body,
                                    false,
                                    binding->function_qualifier,
                                    template_api::function_binding_registration_identity(
                                        *binding),
                                    false,
                                    binding->lexical_access_class));
      }
      return;
    }

    const CppAstNode * specifiers = nullptr;
    const CppAstNode * declarator = nullptr;
    const CppAstNode * initializer = nullptr;
    const CppAstNode * body = nullptr;
    const CppAstNode * ctor_initializer = nullptr;
    bool special_member_template = false;
    bool special_member_is_constructor = false;
    bool special_member_is_destructor = false;
    bool special_member_is_conversion = false;
    if(inner.kind == CppAstKind::function_definition) {
      specifiers = find_child_kind(inner, CppAstKind::decl_specifier_seq);
      declarator = find_child_kind(inner, CppAstKind::declarator);
      body = find_function_body_node(inner);
      ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
    } else if(scope.class_info &&
              (inner.kind == CppAstKind::special_member_definition ||
               inner.kind == CppAstKind::special_member_declaration)) {
      special_member_template = true;
      specifiers = find_child_kind(inner, CppAstKind::member_specifiers);
      declarator = find_child_kind(inner, CppAstKind::declarator);
      body = find_function_body_node(inner);
      ctor_initializer = find_child_kind(inner, CppAstKind::ctor_initializer);
      const string class_name =
          strip_trailing_top_level_template_arguments(
              unqualified_member_name(scope.class_info->name));
      const string member_name = unqualified_member_name(inner.value);
      const string stripped_member_name =
          strip_trailing_top_level_template_arguments(member_name);
      special_member_is_constructor =
          member_name == class_name || stripped_member_name == class_name;
      if(member_name == (string("~") + class_name)) {
        special_member_is_destructor = true;
      } else if(member_name.size() > 1 && member_name[0] == '~') {
        special_member_is_destructor =
            strip_trailing_top_level_template_arguments(member_name.substr(1)) ==
            class_name;
      }
      special_member_is_conversion = is_conversion_function_name(inner.value);
      if(!special_member_is_constructor &&
         !special_member_is_destructor &&
         !special_member_is_conversion) {
        return;
      }
      if(!declarator) {
        throw logic_error("invalid templated special member");
      }
    } else if(inner.kind == CppAstKind::simple_declaration) {
      specifiers = find_child_kind(inner, CppAstKind::decl_specifier_seq);
      const CppAstNode * declarators =
          find_child_kind(inner, CppAstKind::init_declarator_list);
      if(declarators && declarators->children.size() == 1 &&
         !declarators->children[0].children.empty()) {
        declarator = &declarators->children[0].children[0];
        if(declarators->children[0].children.size() > 1) {
          initializer = &declarators->children[0].children[1];
        }
      }
      if(scope.class_info && specifiers &&
         any_of(specifiers->children.begin(), specifiers->children.end(),
                [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); }) &&
         !declarator) {
        for(size_t i = 0; i < specifiers->children.size(); ++i) {
          if(specifiers->children[i].kind == CppAstKind::class_forward_declaration ||
             specifiers->children[i].kind == CppAstKind::class_specifier) {
            return;
          }
        }
      }
    }
    if((!specifiers && !special_member_template) || !declarator) {
      ostringstream out;
      out << "unsupported template declaration";
      out << " [inner kind=" << cppast_kind_text(inner.kind) << "]";
      out << " [inner text " << node_text(inner) << "]";
      out << " [has specifiers " << (specifiers ? "yes" : "no") << "]";
      out << " [has declarator " << (declarator ? "yes" : "no") << "]";
      if(!inner.children.empty()) {
        out << " [children";
        for(size_t i = 0; i < inner.children.size(); ++i) {
          out << (i == 0 ? " " : ", ") << cppast_kind_text(inner.children[i].kind);
        }
        out << "]";
        if(node_text(inner).empty()) {
          out << " [inner ast={" << describe_cppast_translation_unit(inner) << "}]";
        }
      }
      throw logic_error(out.str());
    }
    record_template_parameter_clause_source_uses(scope, node);
    PreparedMethodParseContext prepared_special_member_method;
    if(special_member_template) {
      semantic_class_model::prepare_method_parse_context(specifiers,
                                                         *declarator,
                                                         prepared_special_member_method,
                                                         true,
                                                         true);
    }
    if(body && !effective_template_parameters.empty()) {
      const std::set<std::string> effective_template_parameter_names =
          template_parameter_names(effective_template_parameters);
      const CppAstNode * offending_template_parameter_redeclaration = nullptr;
      std::string offending_template_parameter_name;
      if(subtree_alias_redeclares_template_parameter(*body,
                                                     effective_template_parameter_names,
                                                     offending_template_parameter_redeclaration,
                                                     offending_template_parameter_name)) {
        throw logic_error(string("template parameter redeclared") +
                          semantic_trace::current_location_note(
                              *this,
                              offending_template_parameter_redeclaration ?
                                  offending_template_parameter_redeclaration :
                                  body));
      }
    }

    if(template_parameters.empty() && !special_member_template) {
      const CppAstNode * function_identifier =
          find_descendant_kind(*declarator, CppAstKind::identifier);
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "candidate-out-of-class-member declarator=" << node_text(*declarator)
              << " identifier="
              << (function_identifier ? function_identifier->value : std::string("<none>"));
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      const QualifiedName * qualified_member_syntax =
          function_identifier ? cppast_qualified_name_syntax(*function_identifier) : nullptr;
      QualifiedName text_qualified_member_syntax;
      if(function_identifier &&
         function_identifier->value.find("::") != string::npos &&
         split_qualified_name_text(function_identifier->value,
                                   text_qualified_member_syntax) &&
         !text_qualified_member_syntax.qualifiers.empty() &&
         (!qualified_member_syntax ||
          qualified_member_syntax->qualifiers.empty() ||
          (function_identifier->value.find("operator") != string::npos &&
           qualified_member_syntax->name != text_qualified_member_syntax.name))) {
        qualified_member_syntax = &text_qualified_member_syntax;
      }
      if(function_identifier &&
         qualified_member_syntax &&
         !qualified_member_syntax->qualifiers.empty()) {
        const QualifiedName & qualified_member = *qualified_member_syntax;
        string owner_template_name;
        const bool owner_is_template_id =
            split_unqualified_template_head_text(qualified_member.qualifiers.back(),
                                                 owner_template_name);
	        string owner_name;
	        if(qualified_member.rooted) {
	          owner_name += "::";
	        }
	        for(size_t i = 0; i < qualified_member.qualifiers.size(); ++i) {
	          if(!owner_name.empty() && owner_name != "::") {
	            owner_name += "::";
	          }
	          owner_name += qualified_member.qualifiers[i];
	        }
	        if(owner_is_template_id &&
	           declarator &&
	           !find_child_kind(*declarator, CppAstKind::parameter_clause)) {
	          QualifiedName owner_template_id_name;
	          vector<string> owner_arg_texts;
	          if(semantic_utils::split_top_level_template_id_text(
	                 qualified_member.qualifiers.back(),
	                 owner_template_id_name,
	                 owner_arg_texts)) {
	            Scope * owner_lookup_scope = &scope;
	            if(qualified_member.qualifiers.size() > 1 || qualified_member.rooted) {
	              QualifiedName owner_scope_name;
	              owner_scope_name.rooted = qualified_member.rooted;
	              if(qualified_member.qualifiers.size() > 1) {
	                owner_scope_name.qualifiers.assign(
	                    qualified_member.qualifiers.begin(),
	                    qualified_member.qualifiers.end() - 2);
	                owner_scope_name.name =
	                    qualified_member.qualifiers[qualified_member.qualifiers.size() - 2];
	              }
	              owner_lookup_scope =
	                  semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
	                      *this,
	                      scope,
	                      owner_scope_name);
	            }
	            ClassTemplateDecl * owner_template =
	                owner_lookup_scope ?
	                    lookup_class_template(*owner_lookup_scope,
	                                          owner_template_id_name.name) :
	                    nullptr;
	            vector<TemplateArgument> owner_arguments;
	            if(owner_template &&
	               resolve_template_arguments(
	                   scope,
	                   owner_template->parameters,
	                   owner_arg_texts,
	                   owner_arguments,
	                   owner_template->declaring_scope)) {
	              owner_template->explicit_static_member_specialization_keys.insert(
	                  make_pair(qualified_member.name,
	                            template_argument_key(owner_arguments)));
	            }
	          }
	        }
	        ClassInfo * owner =
	            resolve_qualified_owner_class_from_template_id_syntax(
	                scope,
	                qualified_member,
	                function_identifier,
	                inner.kind == CppAstKind::function_definition ?
	                    QualifiedOwnerClassResolution::Complete :
	                    QualifiedOwnerClassResolution::ReferenceMembers);
	        if(!owner) {
	          owner = resolve_qualified_owner_class(
	              scope,
	              owner_name,
	              inner.kind == CppAstKind::function_definition ?
	                  QualifiedOwnerClassResolution::Complete :
	                  QualifiedOwnerClassResolution::ReferenceMembers);
	        }
        if(owner) {
          PreparedMethodParseContext prepared_owner_method;
          semantic_class_model::prepare_method_parse_context(specifiers,
                                                             *declarator,
                                                             prepared_owner_method,
                                                             true,
                                                             true);

          bool is_typedef = false;
          TypePtr base;
          const bool parsed_base =
              inner.kind == CppAstKind::function_definition ?
                  parse_function_definition_base(*owner->member_scope,
                                                 *prepared_owner_method.parse_specifiers_node(),
                                                 prepared_owner_method.parse_declarator_node(),
                                                 *body,
                                                 prepared_owner_method.syntax.is_const_method,
                                                 prepared_owner_method.syntax.is_volatile_method,
                                                 is_typedef,
                                                 base) :
                  parse_trailing_return_base(*owner->member_scope,
                                             *prepared_owner_method.parse_specifiers_node(),
                                             prepared_owner_method.parse_declarator_node(),
                                             is_typedef,
                                             base,
                                             true);
          TypePtr declared_type;
          string parsed_name;
          if(parsed_base && !is_typedef &&
             parse_declarator(*owner->member_scope,
                              prepared_owner_method.parse_declarator_node(),
                              base,
                              parsed_name,
                              declared_type,
                              true)) {
            ValueBinding * static_binding = nullptr;
            if(strip_top_level_cv(declared_type)->kind != Type::TK_FUNCTION &&
               owner_is_template_id &&
               resolve_out_of_class_static_member_binding(scope,
                                                          qualified_member,
                                                          static_binding)) {
              emit_out_of_class_owner_class_use_if_needed(scope,
                                                          qualified_member,
                                                          function_identifier->value,
                                                          declarator);
              if(!static_binding->declaration_node) {
                static_binding->declaration_node = declarator;
              }
              static_binding->definition_node = declarator;
              static_binding->has_storage_definition = true;
	              static_binding->is_explicit_specialization = true;
	              if(owner->source_template) {
                  owner->source_template->explicit_static_member_specializations.insert(
                      static_binding->name);
                  const std::string specialization_key =
                      template_api::class_template_effective_instantiation_key(
                          *this, *owner);
                  owner->source_template->explicit_static_member_specialization_keys.insert(
                      make_pair(static_binding->name, specialization_key));
	              }
              if(initializer && initializer->children.size() == 1) {
                static_binding->constant_initializer = initializer;
                static_binding->constant_initializer_scope = owner->member_scope.get();
                static_binding->has_constexpr_value = false;
                static_binding->constexpr_value = constant_eval::ConstexprValue();
                static_binding->has_constant_value = false;
                static_binding->constant_value = 0;
                constant_eval::ConstexprValue value;
                if(evaluate_initializer_constant_value(*owner->member_scope,
                                                       *initializer,
                                                       static_binding->type,
                                                       value)) {
                  static_binding->has_constexpr_value = true;
                  static_binding->constexpr_value = value;
                  long long integral = 0;
                  if(constant_eval::constexpr_value_to_integral(value, integral)) {
                    static_binding->has_constant_value = true;
                    static_binding->constant_value = integral;
                  }
                }
              }
              return;
            }
            FunctionBinding * binding = nullptr;
            const callsemantic::ScopedExactTemplateTypeLookupAnchor
                method_owner_anchor(
                    function_identifier
                        ? owner_lookup_anchor_for_node(*function_identifier)
                        : callsemantic::ExactTemplateTypeLookupAnchor());
            if(resolve_out_of_class_method_binding_with_resolution(
                   scope,
                   qualified_member,
                   declared_type,
                   prepared_owner_method.syntax.is_const_method,
                   prepared_owner_method.syntax.is_volatile_method,
                   prepared_owner_method.syntax.ref_qualifier,
                   binding,
                   inner.kind == CppAstKind::function_definition ?
                       QualifiedOwnerClassResolution::Complete :
                       QualifiedOwnerClassResolution::ReferenceMembers)) {
                if(binding &&
                   binding->owner_class &&
                   binding->owner_class->source_template &&
                   !binding->owner_class->instantiation_arguments.empty() &&
                   !template_arguments_are_dependent(
                       binding->owner_class->instantiation_arguments)) {
                  ClassInfo * specifier_owner = class_info_for_type(
                      strip_top_level_cv(remove_reference_type(base)));
                  if(specifier_owner == binding->owner_class &&
                     prepared_owner_method.parse_specifiers_node()) {
                    const std::string qualified_template_name =
                        strip_trailing_top_level_template_arguments(
                            semantic_model::class_output_qualified_name(
                                *binding->owner_class));
                    const std::string source_template_name =
                        unqualified_member_name(qualified_template_name);
                    const std::string use_location =
                        template_api::normalize_template_witness_source_location(
                            source_location_for_name_in_subtree(
                                *prepared_owner_method.parse_specifiers_node(),
                                !source_template_name.empty() ?
                                    source_template_name :
                                    qualified_template_name));
                    if(!use_location.empty()) {
                      const std::string key =
                          template_api::class_template_effective_instantiation_key(
                              *this,
                              *binding->owner_class);
                      const template_api::specialization::ClassSpecializationSelection
                          selection =
                              template_api::specialization::select_class_specialization(
                                  *this,
                                  *binding->owner_class->source_template,
                                  scope,
                                  key,
                                  binding->owner_class->instantiation_arguments);
                      witness::ClassUseEmitRequest request;
                      request.location = use_location;
                      request.template_name = qualified_template_name;
                      request.selection =
                          source_selection_kind_for_match_kind(selection.kind);
                      request.use_anchor_present = true;
                      request.use_anchor_location = use_location;
                      witness::set_selected_decl_anchor(
                          request.selected_decl_location,
                          request.selected_decl_anchor,
                          class_use_selected_decl_anchor(
                              binding->owner_class->source_template,
                              selection));
                      template_api::append_template_witness_source_bindings(
                          *this,
                          request.bindings,
                          binding->owner_class->source_template->parameters,
                          binding->owner_class->instantiation_arguments,
                          "explicit");
                      if(selection.parameters &&
                         selection.parameters !=
                             &binding->owner_class->source_template->parameters) {
                        template_api::append_template_witness_source_bindings(
                            *this,
                            request.specialization_bindings,
                            *selection.parameters,
                            selection.arguments,
                            "deduced");
                      }
                      request.ownership = witness::SourceUseOwnership::SourceOwned;
                      witness::emit_class_use(request);
                        emit_nested_class_use_source_events_from_location(scope,
                                                                          request.location,
                                                                          witness::SourceUseOwnership::SourceOwned,
                                                                          request.template_name);
                    }
                  }
                }
                emit_out_of_class_owner_class_use_if_needed(scope,
                                                            qualified_member,
                                                            function_identifier->value,
                                                            function_identifier ?
                                                                function_identifier :
                                                                declarator,
                                                            binding ? binding->owner_class :
                                                                      nullptr);
	                if(inner.kind == CppAstKind::function_definition) {
	                  const bool explicit_specialization_overrides_primary =
	                      owner_is_template_id &&
	                    binding->owner_class &&
	                    binding->owner_class->source_template &&
	                    binding->definition_node &&
	                    binding->definition_node == binding->declaration_node &&
	                    binding->definition_node != declarator;
	                if(owner_is_template_id &&
	                   binding->owner_class &&
	                   binding->owner_class->source_template) {
	                  binding->is_explicit_specialization = true;
	                }
	                if(binding->has_definition &&
	                   !explicit_specialization_overrides_primary) {
	                  throw logic_error(string("duplicate class member definition") +
	                                    semantic_trace::current_location_note(*this, declarator) +
                                    semantic_trace::previous_function_location_note(
                                        *this, "previous definition", binding));
                }
                vector<pair<string, TypePtr> > definition_params;
                const CppAstNode * definition_parameter_clause =
                    find_child_kind(*declarator, CppAstKind::parameter_clause);
                if(definition_parameter_clause &&
                   !parse_parameter_clause(*owner->member_scope,
                                           *definition_parameter_clause,
                                           definition_params,
                                           nullptr,
                                           true)) {
                  throw logic_error("unsupported out-of-class method parameter-clause");
                }
                refresh_definition_parameter_names(*binding, definition_params);
                binding->body = body;
                record_definition_parameter_aliases(*binding, definition_params);
                binding->ctor_initializer = ctor_initializer;
                binding->has_definition = true;
                binding->definition_node = declarator;
                binding->definition_abi_tags.clear();
                append_function_declaration_abi_tags(binding->definition_abi_tags, &inner);
                if(!binding->function_qualifier) {
                  binding->function_qualifier =
                      prepared_owner_method.syntax.function_qualifier;
                }
              }
              return;
            }
          }
          if(inner.kind != CppAstKind::function_definition) {
            return;
          }
        }
      }
    }

    Scope * parse_scope = &pattern_scope;
    if(!special_member_template) {
      parse_scope = resolve_qualified_function_parse_scope(pattern_scope, *declarator);
    }
    Scope overlay_scope(parse_scope, "", false);
    if(!special_member_template && parse_scope != &pattern_scope) {
      overlay_scope.class_info = parse_scope->class_info;
      overlay_scope.function = parse_scope->function;
      overlay_direct_scope_bindings(overlay_scope, pattern_scope);
      parse_scope = &overlay_scope;
    }
    Scope * template_parameter_parent =
        parse_scope->parent ? parse_scope->parent : parse_scope;
    Scope template_parameter_parse_scope(template_parameter_parent, "", false);
    template_parameter_parse_scope.class_info = parse_scope->class_info;
    template_parameter_parse_scope.function = parse_scope->function;
    overlay_direct_scope_bindings(template_parameter_parse_scope, *parse_scope);
    bool template_parameter_hides_class_value = false;
    for(size_t parameter_index = 0; parameter_index < template_parameters.size(); ++parameter_index) {
      const TemplateParameterInfo & parameter = template_parameters[parameter_index];
      if(parameter.name.empty()) {
        continue;
      }
      if(parse_scope->class_info &&
         semantic_lookup::lookup_member_value(*parse_scope->class_info, parameter.name).binding) {
        template_parameter_hides_class_value = true;
      }
      template_scope::erase_template_parameter_binding(template_parameter_parse_scope,
                                                       parameter.name);

      if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
        string placeholder_payload = parameter.placeholder_key;
        static const char template_parameter_prefix[] = "template-parameter ";
        if(placeholder_payload.compare(0,
                                       sizeof(template_parameter_prefix) - 1,
                                       template_parameter_prefix) == 0) {
          placeholder_payload =
              placeholder_payload.substr(sizeof(template_parameter_prefix) - 1);
        }
        template_scope::bind_named_type(
            template_parameter_parse_scope,
            parameter.name,
            make_semantic_named(string("typename ") + parameter.name,
                                Type::NSK_TEMPLATE_PARAMETER,
                                placeholder_payload.empty() ?
                                    parameter.name :
                                    placeholder_payload,
                                true));
        if(parameter.parameter_pack) {
          template_parameter_parse_scope.template_bound_type_pack_names.insert(parameter.name);
          template_scope::bump_binding_fingerprint_epoch(template_parameter_parse_scope);
        }
      } else if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
        template_scope::bind_non_type_value(template_parameter_parse_scope,
                                            parameter.name,
                                            parameter.value_type,
                                            0,
                                            true);
        if(parameter.parameter_pack) {
          template_parameter_parse_scope.template_bound_value_pack_names.insert(
              parameter.name);
          template_scope::bump_binding_fingerprint_epoch(template_parameter_parse_scope);
        }
      } else if(parameter.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
        template_scope::bind_template_template_placeholder(template_parameter_parse_scope,
                                                          parameter.name);
      }
    }
    // Member-template parameters shadow class and inherited member values while
    // parsing the signature; direct class-scope bindings were overlaid above.
    if(template_parameter_hides_class_value) {
      template_parameter_parse_scope.class_info = nullptr;
    }
    Scope * function_template_parse_scope =
        template_parameters.empty() ? parse_scope : &template_parameter_parse_scope;

    bool is_typedef = false;
    string name;
    TypePtr type;
    vector<pair<string, TypePtr> > params;
    vector<const CppAstNode *> default_args;
    CppAstNode result_type_pattern;
    vector<const CppAstNode *> parameter_declarations_pattern;
    bool has_trailing_function_parameter_pack = false;
    const bool method_like_template =
        parse_scope->class_info &&
        declarator_declares_function_entity(*declarator);
    const bool nonmember_function_like_template =
        !special_member_template &&
        !method_like_template &&
        declarator_declares_function_entity(*declarator);
    const bool is_friend_template =
        parse_scope->class_info &&
        specifiers &&
        any_of(specifiers->children.begin(), specifiers->children.end(),
               [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); });
    if(is_friend_template) {
      ClassInfo * friend_owner_info =
          pattern_scope.class_info ? pattern_scope.class_info :
                                     parse_scope->class_info;
      if(!friend_owner_info) {
        return;
      }
      ClassInfo & friend_owner = *friend_owner_info;
      Scope * friend_scope =
          semantic_lookup::unqualified_friend_entity_scope(friend_owner);
      if(!friend_scope) {
        return;
      }

      CppAstNode resolved_specifiers;
      if(!prepare_namespace_scope_specifiers(*friend_owner.member_scope,
                                            *specifiers,
                                            nullptr,
                                            true,
                                            true,
                                            resolved_specifiers)) {
        return;
      }

      try {
        PreparedMethodParseContext prepared_friend_method;
        semantic_class_model::prepare_method_parse_context(&resolved_specifiers,
                                                           *declarator,
                                                           prepared_friend_method,
                                                           true,
                                                           true);
        template_api::signature::ParsedFunctionTemplateSignature parsed =
            template_api::signature::parse_function_template_signature(
                *this,
                *function_template_parse_scope,
                inner.value,
                *declarator,
                filtered_class_member_decl_specifiers(resolved_specifiers),
                prepared_friend_method.parse_declarator_node(),
                true);
        Scope * entity_scope = friend_scope;
        string entity_name = parsed.name;
        const CppAstNode * friend_identifier =
            find_descendant_kind(*declarator, CppAstKind::identifier);
        const QualifiedName * friend_name_syntax =
            friend_identifier ? cppast_qualified_name_syntax(*friend_identifier) : nullptr;
        const bool qualified_friend_name =
            friend_name_syntax && !friend_name_syntax->qualifiers.empty();
        if(qualified_friend_name) {
          entity_scope =
              semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
                  *this,
                  *friend_owner.member_scope,
                  *friend_name_syntax);
          if(!entity_scope) {
            return;
          }
          entity_name = friend_name_syntax->name;
        }
        record_friend_function_template_declaration(friend_owner,
                                                    *entity_scope,
                                                    pattern_scope,
                                                    entity_name,
                                                    template_parameters,
                                                    parsed.type,
                                                    parsed.params,
                                                    parsed.result_type_pattern,
                                                    parsed.default_arguments,
                                                    parsed.parameter_declarations,
                                                    prepared_friend_method.syntax.decl_static,
                                                    prepared_friend_method.syntax.is_const_method,
                                                    prepared_friend_method.syntax.is_volatile_method,
                                                    prepared_friend_method.syntax.ref_qualifier,
                                                    qualified_friend_name,
                                                    specifiers,
                                                    declarator,
                                                    body,
                                                    &node);
      } catch(const logic_error &) {
        return;
      }
      return;
    }
    if(special_member_template) {
      if(special_member_is_conversion) {
        if(!semantic_class_model::parse_conversion_operator_signature(*this,
                                                                      *function_template_parse_scope,
                                                                      inner,
                                                                      name,
                                                                      type,
                                                                      params,
                                                                      &default_args,
                                                                      nullptr)) {
          throw logic_error("invalid templated conversion operator");
        }
      } else {
        name = template_api::signature::normalize_special_member_template_name(
            *this, inner.value, special_member_is_constructor, special_member_is_destructor);
        type = TypePtr();
      }
    } else if(method_like_template) {
      PreparedMethodParseContext prepared_template_method;
      semantic_class_model::prepare_method_parse_context(specifiers,
                                                         *declarator,
                                                         prepared_template_method,
                                                         true,
                                                         true);
      template_api::FunctionTemplateSignatureParseResult parsed_result =
          template_api::signature::try_parse_function_template_signature(
              ctx,
              *function_template_parse_scope,
              inner.value,
              *declarator,
              *prepared_template_method.parse_specifiers_node(),
              prepared_template_method.parse_declarator_node(),
              false);
      if(!parsed_result.ok()) {
        const bool can_defer_out_of_class_member_signature =
            parsed_result.status ==
                template_api::FunctionTemplateSignatureParseStatus::
                    UnsupportedDeclarator ||
            parsed_result.status ==
                template_api::FunctionTemplateSignatureParseStatus::
                    UnsupportedParameterClause;
        if(body &&
           can_defer_out_of_class_member_signature &&
           store_deferred_out_of_class_member_function_definition(
               scope,
               pattern_scope,
               owner_template_parameters,
               inner,
               specifiers,
               *declarator,
               body,
               ctor_initializer,
               prepared_template_method.syntax)) {
          return;
        }
        if(body && body->kind == CppAstKind::lazy_function_body) {
          return;
        }
        throw logic_error(parsed_result.diagnostic);
      }
      const template_api::ParsedFunctionTemplateSignature & parsed =
          parsed_result.signature;
      name = parsed.name;
      type = parsed.type;
      params = parsed.params;
      default_args = parsed.default_arguments;
      result_type_pattern = parsed.result_type_pattern;
      parameter_declarations_pattern = parsed.parameter_declarations;
      has_trailing_function_parameter_pack =
          pack_parameter_analysis::declarator_has_trailing_template_parameter_pack(
              parsed.effective_declarator,
              template_parameters);
      if(parse_scope->class_info) {
        const string class_name = parse_scope->class_info->name;
        const string unqualified = unqualified_member_name(name);
        bool parsed_is_constructor = false;
        bool parsed_is_destructor = false;
        if(unqualified == class_name) {
          parsed_is_constructor = true;
        } else if(unqualified == string("~") + class_name) {
          parsed_is_destructor = true;
        } else {
          const string stripped_unqualified =
              strip_trailing_top_level_template_arguments(unqualified);
          if(stripped_unqualified == class_name) {
            parsed_is_constructor = true;
          } else if(unqualified.size() > 1 && unqualified[0] == '~') {
            const string destructor_target = unqualified.substr(1);
            if(strip_trailing_top_level_template_arguments(destructor_target) ==
               class_name) {
              parsed_is_destructor = true;
            }
          }
        }
        if(parsed_is_constructor || parsed_is_destructor) {
          name = template_api::signature::normalize_special_member_template_name(
              *this, name, parsed_is_constructor, parsed_is_destructor);
          special_member_is_constructor = parsed_is_constructor;
          special_member_is_destructor = parsed_is_destructor;
        }
      }
    } else if(nonmember_function_like_template) {
      if(!parse_variable_declaration_type(*parse_scope,
                                          *specifiers,
                                          *declarator,
                                          initializer,
                                          true,
                                          name,
                                          type,
                                          is_typedef,
                                          true) ||
         !type || is_typedef) {
        template_api::FunctionTemplateSignatureParseResult parsed_result =
            template_api::signature::try_parse_function_template_signature(
                ctx,
                *function_template_parse_scope,
                inner.value,
                *declarator,
                *specifiers,
                *declarator,
                false);
        if(parsed_result.ok()) {
          const template_api::ParsedFunctionTemplateSignature & parsed =
              parsed_result.signature;
          name = parsed.name;
          type = parsed.type;
          is_typedef = false;
          params = parsed.params;
          default_args = parsed.default_arguments;
          result_type_pattern = parsed.result_type_pattern;
          parameter_declarations_pattern = parsed.parameter_declarations;
          has_trailing_function_parameter_pack =
              pack_parameter_analysis::declarator_has_trailing_template_parameter_pack(
                  parsed.effective_declarator,
                  template_parameters);
        } else {
          ostringstream out;
          out << "unsupported template declarator";
          TypePtr debug_base;
          bool debug_typedef = false;
          if(!parse_decl_spec(*specifiers, *parse_scope, debug_typedef, debug_base, true)) {
            out << " [decl-specifier-seq]";
          } else if(debug_typedef) {
            out << " [typedef]";
          } else {
            const CppAstNode filtered_declarator = filtered_function_declarator(*declarator);
            string debug_name;
            TypePtr debug_type;
            if(!parse_declarator(*parse_scope, filtered_declarator, debug_base,
                                 debug_name, debug_type, true)) {
              out << " [declarator " << node_text(filtered_declarator) << "]";
            } else {
              out << " [post-parse]";
            }
          }
          throw logic_error(out.str());
        }
      }
    } else if(!parse_variable_declaration_type(*parse_scope, *specifiers, *declarator, initializer,
                                               true, name, type, is_typedef, true) ||
              !type || is_typedef) {
      ostringstream out;
      out << "unsupported template declarator";
      TypePtr debug_base;
      bool debug_typedef = false;
      if(!parse_decl_spec(*specifiers, *parse_scope, debug_typedef, debug_base, true)) {
        out << " [decl-specifier-seq]";
      } else if(debug_typedef) {
        out << " [typedef]";
      } else {
        const CppAstNode filtered_declarator = filtered_function_declarator(*declarator);
        string debug_name;
        TypePtr debug_type;
        if(!parse_declarator(*parse_scope, filtered_declarator, debug_base,
                             debug_name, debug_type, true)) {
          out << " [declarator " << node_text(filtered_declarator) << "]";
        } else {
          out << " [post-parse]";
        }
      }
      throw logic_error(out.str());
    }
    if(!method_like_template &&
       !special_member_template &&
       type &&
       strip_top_level_cv(type)->kind == Type::TK_FUNCTION &&
       type_mentions_unowned_template_parameters(type, template_parameters)) {
      string reparsed_name;
      TypePtr reparsed_type;
      bool reparsed_is_typedef = false;
      if(parse_variable_declaration_type(*parse_scope,
                                         *specifiers,
                                         *declarator,
                                         initializer,
                                         true,
                                         reparsed_name,
                                         reparsed_type,
                                         reparsed_is_typedef,
                                         false) &&
         reparsed_type &&
         !reparsed_is_typedef) {
        name = reparsed_name;
        type = reparsed_type;
      }
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "parsed-template-declaration inner-kind=" << cppast_kind_text(inner.kind)
            << " method-like=" << (method_like_template ? "yes" : "no")
            << " parse-scope="
            << semantic_trace::scope_name_for_diagnostic(*parse_scope)
            << " name=" << name
            << " type=" << describe_type(type);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }

    if(!special_member_template &&
       type &&
       strip_top_level_cv(type)->kind == Type::TK_FUNCTION) {
      const CppAstNode * function_identifier =
          find_descendant_kind(*declarator, CppAstKind::identifier);
      const QualifiedName * qualified_member_syntax =
          function_identifier ? cppast_qualified_name_syntax(*function_identifier) : nullptr;
      QualifiedName text_qualified_member_syntax;
      if(function_identifier &&
         function_identifier->value.find("::") != string::npos &&
         split_qualified_name_text(function_identifier->value,
                                   text_qualified_member_syntax) &&
         !text_qualified_member_syntax.qualifiers.empty() &&
         (!qualified_member_syntax ||
          qualified_member_syntax->qualifiers.empty() ||
          (function_identifier->value.find("operator") != string::npos &&
           qualified_member_syntax->name != text_qualified_member_syntax.name))) {
        qualified_member_syntax = &text_qualified_member_syntax;
      }
      if(qualified_member_syntax &&
         !qualified_member_syntax->qualifiers.empty()) {
        const QualifiedName & qualified_member = *qualified_member_syntax;
        const string qualified_member_text = qualified_name_syntax_text(qualified_member);
        QualifiedName owner_name_syntax;
        owner_name_syntax.rooted = qualified_member.rooted;
        owner_name_syntax.qualifiers.assign(qualified_member.qualifiers.begin(),
                                            qualified_member.qualifiers.end() - 1);
        owner_name_syntax.name = qualified_member.qualifiers.back();
        const string owner_name = qualified_name_syntax_text(owner_name_syntax);

        ClassInfo * owner =
            resolve_qualified_owner_class_from_template_id_syntax(
                pattern_scope,
                qualified_member,
                function_identifier);
        if(!owner) {
          owner = resolve_qualified_owner_class(pattern_scope, owner_name);
        }
        if(owner && owner->member_scope) {
          string owner_template_name;
          const bool owner_is_template_id =
              split_unqualified_template_head_text(qualified_member.qualifiers.back(),
                                                   owner_template_name);
          ClassTemplateDecl * owner_template_decl = owner->source_template;
          if(!owner_template_decl &&
             owner->enclosing_scope &&
             owner->enclosing_scope->class_info) {
            owner_template_decl =
                owner->enclosing_scope->class_info->source_template;
          }
          if(!owner_template_decl && owner->enclosing_scope) {
            owner_template_decl = lookup_class_template(
                *owner->enclosing_scope,
                owner_is_template_id ? owner_template_name : owner->name);
          }
          const bool partial_owner =
              owner_template_decl &&
              owner->template_output_node &&
              owner->template_output_node != owner_template_decl->class_node;
          PartialClassTemplateSpecializationDecl * owner_partial_decl =
              partial_owner && owner_template_decl ?
                  find_partial_specialization_decl(*owner_template_decl, owner) :
                  nullptr;
          Scope * owner_partial_scope =
              owner_partial_decl ?
                  (owner_partial_decl->pattern_scope ?
                       owner_partial_decl->pattern_scope :
                       owner_partial_decl->declaring_scope) :
                  nullptr;

          if(partial_owner &&
             owner->template_output_node &&
             !owner->reference_member_collection_in_progress &&
             !owner->full_member_collection_in_progress &&
             !owner->template_instantiation_in_progress) {
            semantic_class_model::populate_class_info(*this,
                                                      *owner,
                                                      *owner->template_output_node);
          }

          Scope owner_overlay_scope(owner->member_scope.get(), "", false);
          owner_overlay_scope.class_info = owner->member_scope->class_info;
          owner_overlay_scope.function = owner->member_scope->function;
          template_api::binding::overlay_ancestor_scope_bindings(
              owner_overlay_scope,
              pattern_scope,
              nullptr);
          Scope * effective_owner_parse_scope = owner->member_scope.get();
          if(effective_owner_parse_scope != &pattern_scope) {
            effective_owner_parse_scope = &owner_overlay_scope;
          }

          PreparedMethodParseContext prepared_owner_method;
          semantic_class_model::prepare_method_parse_context(specifiers,
                                                             *declarator,
                                                             prepared_owner_method,
                                                             true,
                                                             true);

          bool owner_is_typedef = false;
          TypePtr owner_base;
          const CppAstNode filtered_owner_declarator =
              filtered_function_declarator(prepared_owner_method.parse_declarator_node());
          const bool parsed_base =
              inner.kind == CppAstKind::function_definition ?
                  parse_function_definition_base(*effective_owner_parse_scope,
                                                 *prepared_owner_method.parse_specifiers_node(),
                                                 prepared_owner_method.parse_declarator_node(),
                                                 *body,
                                                 prepared_owner_method.syntax.is_const_method,
                                                 prepared_owner_method.syntax.is_volatile_method,
                                                 owner_is_typedef,
                                                 owner_base) :
                  parse_trailing_return_base(*effective_owner_parse_scope,
                                             *prepared_owner_method.parse_specifiers_node(),
                                             prepared_owner_method.parse_declarator_node(),
                                             owner_is_typedef,
                                             owner_base,
                                             true);
          TypePtr declared_type;
          string parsed_name;
          const bool parsed_declarator =
              parsed_base &&
              !owner_is_typedef &&
              parse_declarator(*effective_owner_parse_scope,
                               filtered_owner_declarator,
                               owner_base,
                               parsed_name,
                               declared_type,
                               true);
          if(parsed_declarator) {
            FunctionTemplateDecl * template_decl =
                  resolve_out_of_class_method_template(pattern_scope,
                                                       qualified_member,
                                                       effective_template_parameters,
                                                       params,
                                                       prepared_owner_method.syntax.is_const_method,
                                                       prepared_owner_method.syntax.is_volatile_method,
                                                     prepared_owner_method.syntax.ref_qualifier);
            const bool matches_owner_template_parameters =
                  owner_template_decl &&
                  !partial_owner &&
                  out_of_class_special_member_template_parameters_match(
                      *owner->member_scope,
                      owner_template_decl->parameters,
                      pattern_scope,
                      owner_template_parameters);
            const bool matches_partial_owner_template_parameters =
                owner_partial_scope &&
                out_of_class_special_member_template_parameters_match(
                    *owner_partial_scope,
                    owner_partial_decl->parameters,
                    pattern_scope,
                    owner_template_parameters);
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "collect-out-of-class-member qualified-name="
                    << qualified_member_text
                    << " owner=" << owner->qualified_name
                    << " owner-template="
                    << (owner_template_decl ? owner_template_decl->name : std::string("<none>"))
                    << " partial-owner=" << (partial_owner ? "yes" : "no")
                    << " has-body=" << (body ? "yes" : "no")
                    << " method-template=" << (template_decl ? "yes" : "no")
                    << " matches-owner-params="
                    << (matches_owner_template_parameters ? "yes" : "no")
                    << " matches-partial-owner-params="
                    << (matches_partial_owner_template_parameters ? "yes" : "no");
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            const auto member_exclude_from_explicit_instantiation =
                [&]() -> bool
                {
                  FunctionBinding * resolved_member_declaration = nullptr;
                  return resolve_out_of_class_method_binding_with_resolution(
                             pattern_scope,
                             qualified_member,
                             declared_type,
                             prepared_owner_method.syntax.is_const_method,
                             prepared_owner_method.syntax.is_volatile_method,
                             prepared_owner_method.syntax.ref_qualifier,
                             resolved_member_declaration,
                             QualifiedOwnerClassResolution::ReferenceMembers) &&
                         resolved_member_declaration &&
                         resolved_member_declaration->exclude_from_explicit_instantiation;
                };
            if(template_decl) {
              if(body) {
                if(template_decl->body) {
                  const bool same_body_location =
                      source_location_for_node(*template_decl->body) ==
                      source_location_for_node(*body);
                  const bool same_declarator_location =
                      template_decl->declarator &&
                      declarator &&
                      source_location_for_node(*template_decl->declarator) ==
                          source_location_for_node(*declarator);
                  if(same_body_location || same_declarator_location) {
                    return;
                  }
                  throw logic_error(string("duplicate templated class member definition") +
                                    semantic_trace::current_location_note(*this, declarator) +
                                    semantic_trace::node_location_note(
                                        *this,
                                        "previous definition",
                                        template_decl->body ? template_decl->body
                                                            : template_decl->declarator));
                }
                template_decl->body = body;
                template_decl->definition_node = &inner;
                template_decl->definition_inner = &inner;
                template_decl->definition_specifiers = specifiers;
                template_decl->definition_declarator = declarator;
                template_decl->ctor_initializer = ctor_initializer;
                record_definition_parameter_aliases(*template_decl, params);
                if(template_decl->parameter_declarations_pattern.empty() &&
                   !parameter_declarations_pattern.empty()) {
                  template_decl->parameter_declarations_pattern =
                      parameter_declarations_pattern;
                }
                if(template_decl->result_type_pattern.kind == CppAstKind::invalid &&
                   result_type_pattern.kind != CppAstKind::invalid) {
                  template_decl->result_type_pattern = result_type_pattern;
                }
                if(owner_template_decl) {
                  vector<OutOfClassMemberFunctionTemplateDefinition> & stored_defs =
                      (owner_partial_decl ?
                           owner_partial_decl->member_function_template_definitions[qualified_member.name] :
                           owner_template_decl->member_function_template_definitions[qualified_member.name]);
                  bool stored = false;
                  for(size_t def_index = 0; def_index < stored_defs.size(); ++def_index) {
                    if(stored_defs[def_index].declaration == template_decl) {
                      stored_defs[def_index].definition_node = &inner;
                      stored_defs[def_index].definition_specifiers = specifiers;
                      stored_defs[def_index].definition_declarator = declarator;
                      stored_defs[def_index].body = body;
                      stored_defs[def_index].ctor_initializer = ctor_initializer;
                      stored_defs[def_index].parameter_aliases_pattern =
                          template_decl->parameter_aliases_pattern;
                      stored = true;
                      break;
                    }
                  }
                  if(!stored) {
                    OutOfClassMemberFunctionTemplateDefinition stored_def;
                    stored_def.declaration = template_decl;
                    stored_def.definition_node = &inner;
                    stored_def.definition_specifiers = specifiers;
                    stored_def.definition_declarator = declarator;
                    stored_def.body = body;
                    stored_def.ctor_initializer = ctor_initializer;
                    stored_def.parameter_aliases_pattern =
                        template_decl->parameter_aliases_pattern;
                    stored_defs.push_back(stored_def);
                  }
                  invalidate_out_of_class_definition_caches(*owner_template_decl);
                }
              }
              if(!template_decl->declarator) {
                template_decl->specifiers = specifiers;
                template_decl->declarator = declarator;
              }
              return;
            }
            if(owner_template_decl && matches_owner_template_parameters) {
              OutOfClassMemberFunctionDecl stored;
              stored.declaring_scope = &scope;
              stored.pattern_scope = &pattern_scope;
              stored.qualified_name = qualified_member_text;
              stored.qualified_name_syntax = qualified_member;
              stored.owner_output_node =
                  owner->template_output_node ? owner->template_output_node :
                  (owner->class_node ? owner->class_node :
                                       owner_template_decl->class_node);
              stored.specifiers = specifiers;
              stored.declarator = declarator;
              stored.body = body;
              stored.ctor_initializer = ctor_initializer;
              stored.declared_type_pattern = declared_type;
              stored.params = params;
              stored.is_const_method = prepared_owner_method.syntax.is_const_method;
              stored.is_volatile_method = prepared_owner_method.syntax.is_volatile_method;
                stored.ref_qualifier = prepared_owner_method.syntax.ref_qualifier;
                stored.exclude_from_explicit_instantiation =
                    member_exclude_from_explicit_instantiation();
                stored.parameters = owner_template_parameters;
                owner_template_decl->member_function_definitions[qualified_member.name].push_back(
                    stored);
              invalidate_out_of_class_definition_caches(*owner_template_decl);
              if(parser_trace::enabled("template.resolve")) {
                std::ostringstream trace;
                trace << "record-out-of-class-member template="
                      << owner_template_decl->name
                      << " qualified-name=" << qualified_member_text
                      << " mode=primary-owner-match";
                parser_trace::note("template.resolve", std::string(), trace.str());
              }
                emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                            qualified_member,
                                                            qualified_member_text,
                                                            &inner,
                                                            owner,
                                                            &owner_template_parameters);
                if(declarator) {
                  emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                              qualified_member,
                                                              qualified_member_text,
                                                              declarator,
                                                              owner,
                                                              &owner_template_parameters);
                }
              return;
            }
            if(owner_partial_decl && matches_partial_owner_template_parameters) {
              OutOfClassMemberFunctionDecl stored;
              stored.declaring_scope = &scope;
              stored.pattern_scope = &pattern_scope;
              stored.qualified_name = qualified_member_text;
              stored.qualified_name_syntax = qualified_member;
              stored.owner_output_node = owner_partial_decl->class_node;
              stored.specifiers = specifiers;
              stored.declarator = declarator;
              stored.body = body;
              stored.ctor_initializer = ctor_initializer;
              stored.declared_type_pattern = declared_type;
              stored.params = params;
              stored.is_const_method = prepared_owner_method.syntax.is_const_method;
              stored.is_volatile_method = prepared_owner_method.syntax.is_volatile_method;
              stored.ref_qualifier = prepared_owner_method.syntax.ref_qualifier;
              stored.exclude_from_explicit_instantiation =
                  member_exclude_from_explicit_instantiation();
              stored.parameters = owner_template_parameters;
              owner_partial_decl->member_function_definitions[qualified_member.name].push_back(
                  stored);
              invalidate_out_of_class_definition_caches(*owner_template_decl);
              if(parser_trace::enabled("template.resolve")) {
                std::ostringstream trace;
                trace << "record-out-of-class-member template="
                      << owner_template_decl->name
                      << " qualified-name=" << qualified_member_text
                      << " mode=partial-owner-match";
                parser_trace::note("template.resolve", std::string(), trace.str());
              }
                emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                            qualified_member,
                                                            qualified_member_text,
                                                            &inner,
                                                            owner,
                                                            &owner_template_parameters);
                if(declarator) {
                  emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                              qualified_member,
                                                              qualified_member_text,
                                                              declarator,
                                                              owner,
                                                              &owner_template_parameters);
                }
              return;
            }
            const bool explicit_owner_specialization =
                template_parameters.empty() &&
                owner_template_decl &&
                owner_is_template_id;
            if(explicit_owner_specialization) {
              OutOfClassMemberFunctionDecl stored;
              stored.declaring_scope = &scope;
              stored.pattern_scope = &pattern_scope;
              stored.qualified_name = qualified_member_text;
              stored.qualified_name_syntax = qualified_member;
              stored.owner_output_node =
                  owner && owner->template_output_node ? owner->template_output_node :
                  (owner && owner->class_node ? owner->class_node :
                                               owner_template_decl->class_node);
              stored.specifiers = specifiers;
              stored.declarator = declarator;
              stored.body = body;
              stored.ctor_initializer = ctor_initializer;
              stored.declared_type_pattern = declared_type;
              stored.params = params;
              stored.is_const_method = prepared_owner_method.syntax.is_const_method;
              stored.is_volatile_method = prepared_owner_method.syntax.is_volatile_method;
              stored.ref_qualifier = prepared_owner_method.syntax.ref_qualifier;
              stored.exclude_from_explicit_instantiation =
                  member_exclude_from_explicit_instantiation();
              stored.parameters = owner_template_parameters;
              owner_template_decl->member_function_definitions[qualified_member.name].push_back(
                  stored);
              invalidate_out_of_class_definition_caches(*owner_template_decl);
                emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                            qualified_member,
                                                            qualified_member_text,
                                                            &inner,
                                                            owner,
                                                            &owner_template_parameters);
              return;
            }
            if(!template_decl) {
              FunctionBinding * binding = nullptr;
              if(resolve_out_of_class_method_binding_with_resolution(
                     pattern_scope,
                     qualified_member,
                     declared_type,
                     prepared_owner_method.syntax.is_const_method,
                     prepared_owner_method.syntax.is_volatile_method,
                     prepared_owner_method.syntax.ref_qualifier,
                     binding,
                     body ?
                         QualifiedOwnerClassResolution::Complete :
                         QualifiedOwnerClassResolution::ReferenceMembers)) {
                  emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                              qualified_member,
                                                              qualified_member_text,
                                                              &inner,
                                                              binding ? binding->owner_class : nullptr,
                                                              &owner_template_parameters);
                if(parser_trace::enabled("template.resolve")) {
                  std::ostringstream trace;
                  trace << "collect-out-of-class-member-binding qualified-name="
                        << qualified_member_text
                        << " owner=" << owner->qualified_name
                        << " found=yes";
                  parser_trace::note("template.resolve", std::string(), trace.str());
                }
                if(body) {
                  const bool explicit_specialization_overrides_primary =
                      template_parameters.empty() &&
                      owner_is_template_id &&
                      binding->owner_class &&
                      binding->owner_class->source_template &&
                      binding->definition_node &&
                      binding->definition_node == binding->declaration_node &&
                      binding->definition_node != declarator;
	                  if(binding->has_definition &&
	                     !explicit_specialization_overrides_primary) {
	                    throw logic_error(string("duplicate class member definition") +
	                                      semantic_trace::current_location_note(*this, declarator) +
	                                      semantic_trace::previous_function_location_note(
	                                          *this, "previous definition", binding));
	                  }
	                  if(template_parameters.empty() &&
	                     owner_is_template_id &&
	                     binding->owner_class &&
	                     binding->owner_class->source_template) {
	                    binding->is_explicit_specialization = true;
	                  }
	                  refresh_definition_parameter_names(*binding, params);
	                  binding->body = body;
	                  record_definition_parameter_aliases(*binding, params);
                  binding->ctor_initializer = ctor_initializer;
                  binding->has_definition = true;
                  binding->definition_node = &node;
                  binding->definition_abi_tags.clear();
                  append_function_declaration_abi_tags(binding->definition_abi_tags, &node);
                  binding->parameter_syntax_node = declarator;
                  if(!binding->function_qualifier) {
                    binding->function_qualifier =
                        prepared_owner_method.syntax.function_qualifier;
                  }
                  upgrade_function_symbol_linkage(
                      binding,
                      function_symbol_linkage(pattern_scope,
                                              specifiers,
                                              body,
                                              false,
                                              binding->function_qualifier,
                                              template_api::function_binding_registration_identity(
                                                  *binding),
                                              false,
                                              binding->lexical_access_class));
                } else if(!binding->declaration_node) {
                  binding->declaration_node = &node;
                  binding->declaration_abi_tags.clear();
                  append_function_declaration_abi_tags(binding->declaration_abi_tags, &node);
                  binding->parameter_syntax_node = declarator;
                }
                return;
              }
              if(parser_trace::enabled("template.resolve")) {
                std::ostringstream trace;
                trace << "collect-out-of-class-member-binding qualified-name="
                      << qualified_member_text
                      << " owner=" << owner->qualified_name
                      << " found=no";
                parser_trace::note("template.resolve", std::string(), trace.str());
              }
              if(partial_owner && !prepared_owner_method.syntax.decl_static) {
                FunctionRegistrationRequest request;
                request.owner_class = owner;
                request.name = qualified_member.name;
                request.declared_type = declared_type;
                request.params = params;
                request.body = body;
                request.ctor_initializer = ctor_initializer;
                request.declaration_node = &node;
                request.parameter_syntax_node = declarator;
                request.function_qualifier =
                    prepared_owner_method.syntax.function_qualifier;
                request.semantic_flags =
                    semantic_class_model::class_function_options(
                        owner->default_access,
                        &prepared_owner_method.syntax,
                        false,
                        false,
                        specifiers && decl_spec_contains_token(*specifiers, KW_CONSTEXPR));
                if(parser_trace::enabled("template.resolve")) {
                  std::ostringstream trace;
                  trace << "record-out-of-class-member qualified-name="
                        << qualified_member_text
                        << " mode=partial-owner-register";
                  parser_trace::note("template.resolve", std::string(), trace.str());
                }
                register_function_entity(request);
                return;
              }
            }
          }
        }
      }
    }

    if(!special_member_template && strip_top_level_cv(type)->kind != Type::TK_FUNCTION) {
      const CppAstNode * static_member_identifier =
          find_descendant_kind(*declarator, CppAstKind::identifier);
      const QualifiedName * static_member_name_syntax =
          static_member_identifier ?
              cppast_qualified_name_syntax(*static_member_identifier) :
              nullptr;
      if(static_member_name_syntax &&
         !static_member_name_syntax->qualifiers.empty()) {
        const QualifiedName & static_member_name = *static_member_name_syntax;
        ClassTemplateDecl * owner_template = nullptr;
        Scope * owner_scope = nullptr;
        size_t owner_template_qualifier_index = static_member_name.qualifiers.size();
        string owner_template_name;
        for(size_t candidate_count = static_member_name.qualifiers.size();
            candidate_count > 0;
            --candidate_count) {
          const size_t candidate_index = candidate_count - 1;
          string candidate_template_name;
          if(!split_unqualified_template_head_text(
                 static_member_name.qualifiers[candidate_index],
                 candidate_template_name)) {
            continue;
          }

          Scope * candidate_scope = &pattern_scope;
          if(static_member_name.rooted) {
            while(candidate_scope->parent) {
              candidate_scope = candidate_scope->parent;
            }
          }
          if(candidate_index > 0 || static_member_name.rooted) {
            QualifiedName owner_scope_name;
            owner_scope_name.rooted = static_member_name.rooted;
            if(candidate_index > 0) {
              owner_scope_name.qualifiers.assign(
                  static_member_name.qualifiers.begin(),
                  static_member_name.qualifiers.begin() + candidate_index - 1);
              owner_scope_name.name =
                  static_member_name.qualifiers[candidate_index - 1];
            }
            candidate_scope =
                semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
                    *this, pattern_scope, owner_scope_name, true);
          }
          if(!candidate_scope) {
            continue;
          }

          ClassTemplateDecl * candidate_template =
              lookup_class_template(*candidate_scope, candidate_template_name);
          if(!candidate_template) {
            continue;
          }
          owner_template = candidate_template;
          owner_scope = candidate_scope;
          owner_template_name = candidate_template_name;
          owner_template_qualifier_index = candidate_index;
          break;
        }

        if(owner_template) {
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "collect-out-of-class-static-member name=" << name
                  << " owner-scope="
                  << semantic_trace::scope_name_for_diagnostic(*owner_scope)
                  << " owner-template=" << owner_template_name
                  << " found=yes";
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
            QualifiedName owner_name_syntax;
            owner_name_syntax.rooted = static_member_name.rooted;
            if(!static_member_name.qualifiers.empty()) {
              owner_name_syntax.qualifiers.assign(static_member_name.qualifiers.begin(),
                                                  static_member_name.qualifiers.end() - 1);
              owner_name_syntax.name = static_member_name.qualifiers.back();
            }
            const string owner_name = qualified_name_syntax_text(owner_name_syntax);
            ClassInfo * owner = resolve_qualified_owner_class(pattern_scope,
                                                              owner_name);
            ClassInfo * template_owner = owner;
            while(template_owner &&
                  template_owner->source_template != owner_template) {
              template_owner =
                  template_owner->enclosing_scope ?
                      template_owner->enclosing_scope->class_info :
                      nullptr;
            }
            const bool partial_owner =
                template_owner &&
                template_owner->template_output_node &&
                template_owner->template_output_node != owner_template->class_node;
            PartialClassTemplateSpecializationDecl * owner_partial_decl =
                partial_owner ?
                    find_partial_specialization_decl(*owner_template, template_owner) :
                    nullptr;
            Scope * owner_partial_scope =
                owner_partial_decl ?
                    (owner_partial_decl->pattern_scope ?
                         owner_partial_decl->pattern_scope :
                         owner_partial_decl->declaring_scope) :
                    nullptr;
            const bool matches_partial_owner_template_parameters =
                owner_partial_scope &&
                out_of_class_special_member_template_parameters_match(
                    *owner_partial_scope,
                    owner_partial_decl->parameters,
                    pattern_scope,
                    template_parameters);
            const bool has_storage_definition =
                !specifiers ||
                !decl_spec_contains_token(*specifiers, KW_EXTERN) ||
                decl_spec_contains_token(*specifiers, KW_CONSTEXPR) ||
                initializer != nullptr;
            string member_definition_key;
            for(size_t i = owner_template_qualifier_index + 1;
                i < static_member_name.qualifiers.size();
                ++i) {
              if(!member_definition_key.empty()) {
                member_definition_key += "::";
              }
              member_definition_key += static_member_name.qualifiers[i];
            }
            if(!member_definition_key.empty()) {
              member_definition_key += "::";
            }
            member_definition_key += static_member_name.name;
            std::map<std::string, OutOfClassStaticMemberDecl> & static_defs =
                matches_partial_owner_template_parameters && owner_partial_decl ?
                    owner_partial_decl->static_member_definitions :
                    owner_template->static_member_definitions;
            OutOfClassStaticMemberDecl & stored =
                static_defs[member_definition_key];
            if(has_storage_definition &&
               stored.has_storage_definition &&
               stored.declarator &&
               stored.declarator != declarator) {
              throw logic_error(string("duplicate class static member definition") +
                                semantic_trace::current_location_note(*this, declarator) +
                                semantic_trace::node_location_note(
                                    *this, "previous definition", stored.declarator));
            }
            stored.declaring_scope = &scope;
            stored.pattern_scope = &pattern_scope;
            stored.node = &inner;
            stored.specifiers = specifiers;
            stored.declarator = declarator;
            stored.initializer = initializer;
            stored.parameters = template_parameters;
            stored.has_storage_definition = has_storage_definition;
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "record-out-of-class-static-member template=" << owner_template->name
                    << " member=" << member_definition_key
                    << " partial="
                    << (matches_partial_owner_template_parameters ? "yes" : "no")
                    << " has-definition=" << (has_storage_definition ? "yes" : "no")
                    << " initializer=" << (initializer ? node_text(*initializer) : std::string("<none>"));
              parser_trace::note("template.resolve", std::string(), trace.str());
            }

            ValueBinding * out_of_class_static_member = nullptr;
            if(resolve_out_of_class_static_member_binding(pattern_scope,
                                                         name,
                                                         out_of_class_static_member)) {
                emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                            static_member_name,
                                                            qualified_name_syntax_text(static_member_name),
                                                            &inner,
                                                            out_of_class_static_member &&
                                                                    out_of_class_static_member->owner_class ?
                                                                out_of_class_static_member->owner_class :
                                                              nullptr,
                                                          &template_parameters);
              if(!out_of_class_static_member->declaration_node) {
                out_of_class_static_member->declaration_node = declarator;
              }
              if(has_storage_definition) {
                out_of_class_static_member->definition_node = declarator;
                out_of_class_static_member->has_storage_definition = true;
                if(initializer && initializer->children.size() == 1) {
                  out_of_class_static_member->constant_initializer = initializer;
                  out_of_class_static_member->constant_initializer_scope = &pattern_scope;
                  constant_eval::ConstexprValue value;
                  const witness::ScopedTemplateWitnessSourceCapturePause
                      source_capture_pause;
                  if(evaluate_initializer_constant_value(pattern_scope,
                                                         *initializer,
                                                         out_of_class_static_member->type,
                                                         value)) {
                    out_of_class_static_member->has_constexpr_value = true;
                    out_of_class_static_member->constexpr_value = value;
                    long long integral = 0;
                    if(constant_eval::constexpr_value_to_integral(value, integral)) {
                      out_of_class_static_member->has_constant_value = true;
                      out_of_class_static_member->constant_value = integral;
                    }
                  }
                }
              }
            }
            return;
        }
      }

      const CppAstNode * variable_template_identifier =
          declarator ? find_descendant_kind(*declarator, CppAstKind::identifier) : nullptr;
      const TemplateIdSyntax * variable_template_id =
          variable_template_identifier ?
              cppast_template_id_syntax(*variable_template_identifier) :
              nullptr;
      QualifiedName specialization_name;
      vector<string> arg_texts;
      const bool has_explicit_template_id = variable_template_id != nullptr;
      if(variable_template_id) {
        specialization_name = variable_template_id->name;
        arg_texts = template_id_argument_texts_preserving_spacing(*variable_template_id);
      }

      if(template_parameters.empty()) {
        if(!has_explicit_template_id || specialization_name.rooted ||
           !specialization_name.qualifiers.empty()) {
          throw logic_error("unsupported variable template explicit specialization");
        }

        VariableTemplateDecl * primary = lookup_variable_template(scope, specialization_name.name);
        if(!primary) {
          throw logic_error("unknown variable template explicit specialization");
        }

        vector<TemplateArgument> arguments;
        if(!resolve_template_arguments(
               scope,
               primary->parameters,
               arg_texts,
               &variable_template_id->argument_syntaxes,
               arguments,
               primary->declaring_scope)) {
          throw logic_error("invalid variable template explicit specialization arguments");
        }

        VariableTemplateSpecializationDecl explicit_specialization;
        explicit_specialization.declaring_scope = &scope;
        explicit_specialization.pattern_scope = &pattern_scope;
        explicit_specialization.specifiers = specifiers;
        explicit_specialization.declarator = declarator;
        explicit_specialization.initializer = initializer;
        explicit_specialization.parameters = template_parameters;
        explicit_specialization.arg_texts = arg_texts;
        primary->explicit_specializations[template_argument_key(arguments)] =
            explicit_specialization;
        return;
      }

      if(has_explicit_template_id) {
        if(specialization_name.rooted || !specialization_name.qualifiers.empty()) {
          throw logic_error("unsupported variable template partial specialization");
        }
        VariableTemplateDecl * primary = lookup_variable_template(scope, specialization_name.name);
        if(!primary) {
          throw logic_error("unknown variable template partial specialization");
        }
        vector<string> normalized_arg_texts;
        {
          const witness::ScopedTemplateWitnessSourceCapturePause
              source_capture_pause;
          if(!fill_trailing_default_template_argument_texts(pattern_scope,
                                                            primary->parameters,
                                                            arg_texts,
                                                            primary->declaring_scope,
                                                            normalized_arg_texts)) {
            throw logic_error("invalid variable template partial specialization arguments");
          }
        }

        VariableTemplateSpecializationDecl partial;
        partial.declaring_scope = &scope;
        partial.pattern_scope = &pattern_scope;
        partial.specifiers = specifiers;
        partial.declarator = declarator;
        partial.initializer = initializer;
        partial.parameters = template_parameters;
        partial.arg_texts = normalized_arg_texts;
        partial.arg_syntaxes =
            normalized_template_argument_syntaxes(*variable_template_id,
                                                  primary->parameters,
                                                  normalized_arg_texts);
        primary->partial_specializations.push_back(partial);
        return;
      }

      unique_ptr<VariableTemplateDecl> decl(new VariableTemplateDecl());
      decl->declaring_scope = &scope;
      decl->pattern_scope = &pattern_scope;
      decl->name = name;
      decl->specifiers = specifiers;
      decl->declarator = declarator;
      decl->initializer = initializer;
      decl->parameters = template_parameters;
      decl->type_pattern = type;
      decl->comes_from_standard_include_path =
          ctx.node_comes_from_standard_include_path(declarator) ||
          ctx.node_comes_from_standard_include_path(specifiers) ||
          ctx.node_comes_from_standard_include_path(initializer);
      scope.variable_templates[decl->name] = decl.get();
      variable_templates.push_back(std::move(decl));
      return;
    }

    if(!special_member_template && !method_like_template) {
      const CppAstNode * parameter_clause =
          function_parameter_clause_in_declarator(*declarator);
      if(parameter_clause) {
        result_type_pattern =
            template_api::signature::build_function_result_type_pattern(
                *specifiers, *declarator);
        {
          const witness::ScopedTemplateWitnessSourceCapturePause
              source_capture_pause;
          template_api::signature::parse_function_template_parameter_clause(
              *this,
              *function_template_parse_scope,
              inner.value,
              *parameter_clause,
              params,
              default_args);
        }
        parameter_declarations_pattern =
            parameter_declarations_from_clause(*parameter_clause);
        TypePtr function_type = strip_top_level_cv(type);
        if(function_type &&
           function_type->kind == Type::TK_FUNCTION &&
           function_type->params.size() == params.size()) {
          for(size_t i = 0; i < params.size(); ++i) {
            params[i].second = function_type->params[i];
          }
        }
        has_trailing_function_parameter_pack =
            pack_parameter_analysis::declarator_has_trailing_template_parameter_pack(
                *declarator,
                template_parameters);
      }
    } else if(special_member_template) {
      const CppAstNode * parameter_clause =
          find_child_kind(prepared_special_member_method.parse_declarator_node(),
                          CppAstKind::parameter_clause);
      if(parameter_clause) {
        {
          const witness::ScopedTemplateWitnessSourceCapturePause
              source_capture_pause;
          template_api::signature::parse_function_template_parameter_clause(
              *this,
              *function_template_parse_scope,
              inner.value,
              *parameter_clause,
              params,
              default_args);
        }
        const CppAstNode * source_parameter_clause =
            declarator ? find_child_kind(*declarator, CppAstKind::parameter_clause) :
                         nullptr;
        parameter_declarations_pattern =
            source_parameter_clause ?
                parameter_declarations_from_clause(*source_parameter_clause) :
                vector<const CppAstNode *>();
        has_trailing_function_parameter_pack =
            pack_parameter_analysis::declarator_has_trailing_template_parameter_pack(
                prepared_special_member_method.parse_declarator_node(),
                template_parameters);
      }
    }

    const auto validate_dependent_member_template_disambiguators =
        [this, &params, body]() -> void
        {
          if(!body) {
            return;
          }

          std::set<std::string> dependent_param_names;
          for(size_t i = 0; i < params.size(); ++i) {
            if(params[i].first.empty() ||
               !type_depends_on_template_parameter(params[i].second)) {
              continue;
            }
            dependent_param_names.insert(params[i].first);
          }
          if(dependent_param_names.empty()) {
            return;
          }

          const std::string template_prefix = "template ";
          std::function<void(const CppAstNode &)> walk =
              [&](const CppAstNode & current)
              {
                if(current.kind == CppAstKind::member_expression &&
                   current.children.size() == 2 &&
                   current.children[0].kind == CppAstKind::id_expression &&
                   current.children[1].kind == CppAstKind::identifier &&
                   dependent_param_names.count(current.children[0].value) != 0) {
                  const std::string member_text =
                      semantic_utils::trim_space(current.children[1].value);
                  const bool has_template_keyword =
                      member_text.size() > template_prefix.size() &&
                      member_text.compare(0, template_prefix.size(), template_prefix) == 0;
                  if(!has_template_keyword) {
                    if(cppast_template_id_syntax(current.children[1]) != nullptr) {
                      throw logic_error(
                          std::string("dependent member template requires template keyword") +
                          semantic_trace::current_location_note(*this, &current));
                    }
                  }
                }

                for(size_t i = 0; i < current.children.size(); ++i) {
                  walk(current.children[i]);
                }
              };

          walk(*body);
        };
    validate_dependent_member_template_disambiguators();

    const CppAstNode * function_template_identifier =
        declarator ? find_descendant_kind(*declarator, CppAstKind::identifier) : nullptr;
    const QualifiedName * function_template_name_syntax =
        function_template_identifier ?
            cppast_qualified_name_syntax(*function_template_identifier) :
            nullptr;
    const TemplateIdSyntax * function_template_id =
        function_template_identifier ?
            cppast_template_id_syntax(*function_template_identifier) :
            nullptr;

    QualifiedName specialization_name;
    vector<string> arg_texts;
    const bool has_explicit_template_id = function_template_id != nullptr;
    if(function_template_id) {
      specialization_name = function_template_id->name;
      arg_texts = function_template_id->arguments;
    }
    const auto build_function_template_deduction_args =
        [&]() -> vector<ExprInfo>
        {
          vector<ExprInfo> deduction_args;
          deduction_args.reserve(params.size());
          for(size_t i = 0; i < params.size(); ++i) {
            ExprInfo arg;
            arg.type = params[i].second;
            TypePtr arg_base = strip_top_level_cv(arg.type);
            if(arg_base && arg_base->kind == Type::TK_LVALUE_REFERENCE) {
              arg.category = VC_LVALUE;
            } else if(arg_base && arg_base->kind == Type::TK_RVALUE_REFERENCE) {
              arg.category = VC_XVALUE;
            } else {
              arg.category = VC_PRVALUE;
            }
            deduction_args.push_back(arg);
          }
          return deduction_args;
        };
    const auto function_template_specialization_type_matches =
        [&](FunctionTemplateDecl & decl,
            const vector<TemplateArgument> & arguments) -> bool
        {
          TypePtr instantiated_type;
          if(!decl.type_pattern ||
             !template_api::type::substitute_type(decl.type_pattern,
                                                  decl.parameters,
                                                  arguments,
                                                  instantiated_type) ||
             !instantiated_type) {
            return false;
          }
          TypePtr resolved_instantiated_type;
          if(template_api::type::resolve_instantiated_dependent_type(ctx,
                                                                     scope,
                                                                     instantiated_type,
                                                                     resolved_instantiated_type) &&
             resolved_instantiated_type) {
            instantiated_type = resolved_instantiated_type;
          }
          return callsemantic::types_equivalent_for_member_binding(instantiated_type, type);
        };
    const auto record_explicit_function_specialization_binding =
        [&](const template_api::TemplateInstantiationResult & result) -> void
        {
          FunctionBinding * binding = result.function_binding;
          if(!binding) {
            return;
          }
          refresh_definition_parameter_names(*binding, params);
          record_definition_parameter_aliases(*binding, params);
          if(!body) {
            return;
          }
          binding->body = body;
          binding->has_definition = true;
          binding->definition_node = &inner;
          binding->definition_abi_tags.clear();
          append_function_declaration_abi_tags(binding->definition_abi_tags, &inner);
          binding->parameter_syntax_node = declarator;
          binding->definition_suppresses_declaration_abi_tags = false;
        };
    if(template_parameters.empty()) {
      vector<FunctionTemplateDecl *> templates;
      if(has_explicit_template_id) {
        if(specialization_name.rooted || !specialization_name.qualifiers.empty()) {
          throw logic_error("unsupported function explicit specialization");
        }
        templates = lookup_function_templates(scope, specialization_name.name);
        const vector<ExprInfo> deduction_args =
            build_function_template_deduction_args();
        for(size_t i = 0; i < templates.size(); ++i) {
          vector<TemplateArgument> explicit_arguments;
          if(!resolve_template_arguments(scope,
                                         templates[i]->parameters,
                                         arg_texts,
                                         &function_template_id->argument_syntaxes,
                                         explicit_arguments,
                                         templates[i]->declaring_scope)) {
            continue;
          }
          template_api::TemplateFunctionDeductionRequest deduction_request;
          deduction_request.decl = templates[i];
          deduction_request.args = &deduction_args;
          deduction_request.resolution_scope = &scope;
          deduction_request.explicit_arguments = &explicit_arguments;
          template_api::TemplateFunctionDeductionResult deduction_result;
          if(!template_api::deduce_function_template(ctx,
                                                     deduction_request,
                                                     deduction_result)) {
            continue;
          }
          if(!function_template_specialization_type_matches(*templates[i],
                                                           deduction_result.arguments)) {
            continue;
          }
          record_explicit_function_specialization_binding(
              acquire_function_template(*templates[i],
                                        deduction_result.arguments,
                                        &scope,
                                        deduction_result.pack_sizes.empty() ?
                                            nullptr :
                                            &deduction_result.pack_sizes,
                                        true,
                                        true,
                                        body,
                                        &node,
                                        specifiers &&
                                            decl_spec_contains_token(*specifiers,
                                                                    KW_CONSTEXPR)));
          return;
        }
      } else {
        templates =
            (function_template_name_syntax &&
             (function_template_name_syntax->rooted ||
              !function_template_name_syntax->qualifiers.empty())) ?
                lookup_function_templates(scope, *function_template_name_syntax) :
                lookup_function_templates(scope, name);
        vector<ExprInfo> deduction_args = build_function_template_deduction_args();
        for(size_t i = 0; i < templates.size(); ++i) {
          vector<TemplateArgument> arguments;
          map<string, size_t> pack_sizes;
          if(!deduce_function_template_arguments(
                 *templates[i], deduction_args, arguments, &scope, &pack_sizes)) {
            continue;
          }
          record_explicit_function_specialization_binding(
              acquire_function_template(*templates[i],
                                        arguments,
                                        nullptr,
                                        &pack_sizes,
                                        true,
                                        true,
                                        body,
                                        &node,
                                        specifiers &&
                                            decl_spec_contains_token(*specifiers,
                                                                    KW_CONSTEXPR)));
          return;
        }
      }
      throw logic_error("unknown function template explicit specialization");
    }

    if(has_explicit_template_id) {
      throw logic_error("partial function specialization unsupported");
    }

    const bool candidate_is_static_member =
        special_member_template ? false :
        (method_like_template ? decl_spec_contains_token(*specifiers, KW_STATIC) :
                                false);
    struct FunctionTemplateDeclTraits
    {
      MemberAccess access = MA_PUBLIC;
      bool is_constructor = false;
      bool is_destructor = false;
      bool is_static_member = false;
      bool is_constexpr = false;
      bool is_explicit = false;
      bool is_const_method = false;
      bool is_volatile_method = false;
      RefQualifier ref_qualifier = RQ_NONE;
      bool decl_virtual = false;
      bool is_override = false;
      bool is_final = false;
      bool exclude_from_explicit_instantiation = false;
    };
    const bool has_candidate_method_syntax =
        !special_member_template && method_like_template;
    PreparedMethodParseContext prepared_candidate_method;
    semantic_class_model::prepare_method_parse_context(specifiers,
                                                       *declarator,
                                                       prepared_candidate_method,
                                                       has_candidate_method_syntax,
                                                       true);
    const FunctionTemplateDeclTraits candidate_traits =
        [&]() -> FunctionTemplateDeclTraits
        {
          FunctionTemplateDeclTraits traits;
          traits.access = access;
          traits.is_constructor = special_member_is_constructor;
          traits.is_destructor = special_member_is_destructor;
          traits.is_static_member = candidate_is_static_member;
          traits.is_constexpr = specifiers && decl_spec_contains_token(*specifiers, KW_CONSTEXPR);
          traits.exclude_from_explicit_instantiation =
              declaration_marks_exclude_from_explicit_instantiation(&inner);
          if(special_member_template) {
            traits.is_explicit = prepared_special_member_method.syntax.decl_explicit;
            traits.is_const_method = prepared_special_member_method.syntax.is_const_method;
            traits.is_volatile_method = prepared_special_member_method.syntax.is_volatile_method;
            traits.ref_qualifier = prepared_special_member_method.syntax.ref_qualifier;
            traits.decl_virtual = prepared_special_member_method.syntax.decl_virtual;
            traits.is_override = prepared_special_member_method.syntax.is_override;
            traits.is_final = prepared_special_member_method.syntax.is_final;
            return traits;
          }
          if(has_candidate_method_syntax) {
            traits.is_explicit = prepared_candidate_method.syntax.decl_explicit;
            traits.is_const_method = prepared_candidate_method.syntax.is_const_method;
            traits.is_volatile_method = prepared_candidate_method.syntax.is_volatile_method;
            traits.ref_qualifier = prepared_candidate_method.syntax.ref_qualifier;
            traits.decl_virtual = prepared_candidate_method.syntax.decl_virtual;
            traits.is_override = prepared_candidate_method.syntax.is_override;
            traits.is_final = prepared_candidate_method.syntax.is_final;
          }
          return traits;
        }();
    const vector<const CppAstNode *> normalized_default_args =
        normalize_default_arguments(params, default_args);
    vector<FunctionTemplateDecl *> existing_templates = direct_function_templates(scope, name);
    for(size_t i = 0; i < existing_templates.size(); ++i) {
      FunctionTemplateDecl * existing = existing_templates[i];
      if(!existing) {
        continue;
      }
      if(existing->inner == &inner && existing->declaring_scope == &scope) {
        return;
      }
      if(existing->body && body) {
        continue;
      }
      if(!function_template_entities_match(*existing,
                                           scope,
                                           pattern_scope,
                                           name,
                                           template_parameters,
                                           type,
                                           special_member_template,
                                           candidate_traits.is_static_member,
                                           candidate_traits.is_const_method,
                                           candidate_traits.is_volatile_method,
                                           candidate_traits.ref_qualifier)) {
        continue;
      }
      if(parser_trace::enabled("destroy.collect") &&
         name == "destroy" &&
         scope.class_info &&
         scope.class_info->qualified_name.find("allocator_traits") != string::npos) {
        ostringstream trace;
        trace << "collect-function-template-merge"
              << " class=" << scope.class_info->qualified_name
              << " name=" << name
              << " existing-template-params=" << describe_template_parameter_infos(existing->parameters)
              << " incoming-template-params=" << describe_template_parameter_infos(template_parameters)
              << " existing-param-count=" << existing->params_pattern.size()
              << " incoming-param-count=" << params.size();
        for(size_t j = 0; j < existing->params_pattern.size(); ++j) {
          trace << " existing-param[" << j << "]="
                << describe_type(existing->params_pattern[j].second);
        }
        for(size_t j = 0; j < params.size(); ++j) {
          trace << " incoming-param[" << j << "]="
                << describe_type(params[j].second);
        }
        parser_trace::note("destroy.collect", std::string(), trace.str());
      }
      vector<TemplateParameterInfo> merged_parameters = existing->parameters;
      if(!merge_template_parameter_redeclarations(merged_parameters, template_parameters)) {
        // Keep the existing parameter spellings for function templates. Their
        // dependent parameter/return types are already modeled against the
        // original placeholder names, and renaming only the parameter list
        // leaves the declaration internally inconsistent for later deduction.
      }
      existing->parameters.swap(merged_parameters);
      existing->is_constexpr = existing->is_constexpr || candidate_traits.is_constexpr;
      existing->access = candidate_traits.access;
      existing->is_constructor = existing->is_constructor || candidate_traits.is_constructor;
      existing->is_destructor = existing->is_destructor || candidate_traits.is_destructor;
      existing->is_explicit = existing->is_explicit || candidate_traits.is_explicit;
      existing->is_static_member = existing->is_static_member || candidate_traits.is_static_member;
      existing->is_const_method =
          existing->is_const_method || candidate_traits.is_const_method;
      existing->is_volatile_method =
          existing->is_volatile_method || candidate_traits.is_volatile_method;
      existing->ref_qualifier =
          existing->ref_qualifier != RQ_NONE ? existing->ref_qualifier :
                                               candidate_traits.ref_qualifier;
      existing->decl_virtual = existing->decl_virtual || candidate_traits.decl_virtual;
      existing->is_override = existing->is_override || candidate_traits.is_override;
      existing->is_final = existing->is_final || candidate_traits.is_final;
      existing->exclude_from_explicit_instantiation =
          existing->exclude_from_explicit_instantiation ||
          candidate_traits.exclude_from_explicit_instantiation;
      if(!existing->declaration_node) {
        existing->declaration_node = &node;
      }
      if(body && !existing->body) {
        existing->inner = &inner;
        existing->specifiers = specifiers;
        existing->declarator = declarator;
        existing->body = body;
        existing->ctor_initializer = ctor_initializer;
        record_definition_parameter_aliases(*existing, params);
      } else {
        if(!existing->inner) {
          existing->inner = &inner;
        }
        if(!existing->specifiers) {
          existing->specifiers = specifiers;
        }
        if(!existing->declarator) {
          existing->declarator = declarator;
        }
      }
      if(existing->default_arguments_pattern.size() < normalized_default_args.size()) {
        existing->default_arguments_pattern.resize(normalized_default_args.size(), nullptr);
      }
      if(existing->default_arguments_pattern.size() == normalized_default_args.size()) {
        for(size_t j = 0; j < existing->default_arguments_pattern.size(); ++j) {
          if(!existing->default_arguments_pattern[j] && normalized_default_args[j]) {
            existing->default_arguments_pattern[j] = normalized_default_args[j];
          }
        }
      }
      if(existing->parameter_declarations_pattern.empty() &&
         !parameter_declarations_pattern.empty()) {
        existing->parameter_declarations_pattern = parameter_declarations_pattern;
      }
      if(existing->result_type_pattern.kind == CppAstKind::invalid &&
         result_type_pattern.kind != CppAstKind::invalid) {
        existing->result_type_pattern = result_type_pattern;
      }
      ensure_function_template_parameter_aliases(*existing);
      snapshot_function_template_debug_info(*this, *existing);
      inherit_pending_friend_function_template_access(*existing);
      return;
    }

    unique_ptr<FunctionTemplateDecl> decl(new FunctionTemplateDecl());
    decl->declaring_scope = &scope;
    decl->pattern_scope = &pattern_scope;
    decl->name = name;
    decl->declaration_node = &node;
    decl->is_constexpr = candidate_traits.is_constexpr;
    decl->access = candidate_traits.access;
    decl->is_constructor = candidate_traits.is_constructor;
    decl->is_destructor = candidate_traits.is_destructor;
    decl->is_explicit = candidate_traits.is_explicit;
    decl->is_static_member = candidate_traits.is_static_member;
    decl->is_const_method = candidate_traits.is_const_method;
    decl->is_volatile_method = candidate_traits.is_volatile_method;
    decl->ref_qualifier = candidate_traits.ref_qualifier;
    decl->decl_virtual = candidate_traits.decl_virtual;
    decl->is_override = candidate_traits.is_override;
    decl->is_final = candidate_traits.is_final;
    decl->exclude_from_explicit_instantiation =
        candidate_traits.exclude_from_explicit_instantiation;
    decl->inner = &inner;
    decl->specifiers = specifiers;
    decl->declarator = declarator;
    decl->body = body;
    decl->ctor_initializer = ctor_initializer;
    decl->parameters = template_parameters;
    decl->type_pattern = type;
    decl->params_pattern = params;
    decl->result_type_pattern = result_type_pattern;
    decl->parameter_declarations_pattern = parameter_declarations_pattern;
    initialize_function_template_parameter_aliases(*decl);
    decl->default_arguments_pattern = normalized_default_args;
    decl->has_trailing_function_parameter_pack = has_trailing_function_parameter_pack;
    decl->trailing_function_parameter_pack_analyzed = false;
    if(parser_trace::enabled("destroy.collect") &&
       name == "destroy" &&
       scope.class_info &&
       scope.class_info->qualified_name.find("allocator_traits") != string::npos) {
      auto direct_named_binding = [](Scope & trace_scope, const std::string & key) -> std::string
      {
        std::map<std::string, TypePtr>::const_iterator found = trace_scope.named_types.find(key);
        return found == trace_scope.named_types.end() ? std::string("<none>") :
                                                        describe_type(found->second);
      };
      auto first_ancestor_named_binding = [&](Scope & trace_scope,
                                              const std::string & key) -> std::string
      {
        Scope * current = &trace_scope;
        while(current) {
          std::map<std::string, TypePtr>::const_iterator found = current->named_types.find(key);
          if(found != current->named_types.end()) {
            return semantic_trace::scope_name_for_diagnostic(*current) + "=" +
                   describe_type(found->second);
          }
          current = current->parent;
        }
        return std::string("<none>");
      };
      ostringstream trace;
      trace << "collect-function-template-create"
            << " class=" << scope.class_info->qualified_name
            << " name=" << name
            << " parse-scope=" << semantic_trace::scope_name_for_diagnostic(*function_template_parse_scope)
            << " direct-_Tp=" << direct_named_binding(*function_template_parse_scope, "_Tp")
            << " ancestor-_Tp=" << first_ancestor_named_binding(*function_template_parse_scope, "_Tp")
            << " template-params=" << describe_template_parameter_infos(template_parameters)
            << " param-count=" << params.size();
      for(size_t j = 0; j < params.size(); ++j) {
        trace << " param[" << j << "]=" << describe_type(params[j].second);
      }
      parser_trace::note("destroy.collect", std::string(), trace.str());
    }
    snapshot_function_template_debug_info(*this, *decl);
    inherit_pending_friend_function_template_access(*decl);
    semantic_lookup::direct_function_template_slot(scope, name).push_back(decl.get());
    function_templates.push_back(std::move(decl));
  }

  bool store_deferred_out_of_class_member_function_definition(
      Scope & scope,
      Scope & pattern_scope,
      const vector<TemplateParameterInfo> & owner_template_parameters,
      const CppAstNode & inner,
      const CppAstNode * specifiers,
      const CppAstNode & declarator,
      const CppAstNode * body,
      const CppAstNode * ctor_initializer,
      const MethodSyntaxInfo & syntax)
  {
    const CppAstNode * function_identifier =
        find_descendant_kind(declarator, CppAstKind::identifier);
    const QualifiedName * qualified_member =
        function_identifier ? cppast_qualified_name_syntax(*function_identifier) : nullptr;
    if(!qualified_member || qualified_member->qualifiers.empty()) {
      return false;
    }

    QualifiedName owner_name_syntax;
    owner_name_syntax.rooted = qualified_member->rooted;
    owner_name_syntax.qualifiers.assign(qualified_member->qualifiers.begin(),
                                        qualified_member->qualifiers.end() - 1);
    owner_name_syntax.name = qualified_member->qualifiers.back();
    const string owner_name = qualified_name_syntax_text(owner_name_syntax);
    ClassInfo * owner =
        resolve_qualified_owner_class(pattern_scope,
                                      owner_name,
                                      QualifiedOwnerClassResolution::ReferenceMembers);
    if(!owner || !owner->member_scope) {
      return false;
    }

    string owner_template_name;
    const bool owner_is_template_id =
        split_unqualified_template_head_text(qualified_member->qualifiers.back(),
                                             owner_template_name);
    ClassTemplateDecl * owner_template_decl = owner->source_template;
    if(!owner_template_decl &&
       owner->enclosing_scope &&
       owner->enclosing_scope->class_info) {
      owner_template_decl = owner->enclosing_scope->class_info->source_template;
    }
    if(!owner_template_decl && owner->enclosing_scope) {
      owner_template_decl =
          lookup_class_template(*owner->enclosing_scope,
                                owner_is_template_id ? owner_template_name : owner->name);
    }
    if(!owner_template_decl ||
       !out_of_class_special_member_template_parameters_match(*owner->member_scope,
                                                              owner_template_decl->parameters,
                                                              pattern_scope,
                                                              owner_template_parameters)) {
      return false;
    }

    OutOfClassMemberFunctionDecl stored;
    stored.declaring_scope = &scope;
    stored.pattern_scope = &pattern_scope;
    stored.qualified_name = qualified_name_syntax_text(*qualified_member);
    stored.qualified_name_syntax = *qualified_member;
    stored.owner_output_node =
        owner->template_output_node ? owner->template_output_node :
        (owner->class_node ? owner->class_node : owner_template_decl->class_node);
    stored.specifiers = specifiers;
    stored.declarator = &declarator;
    stored.body = body;
    stored.ctor_initializer = ctor_initializer;
    stored.declared_type_pattern = TypePtr();
    stored.is_const_method = syntax.is_const_method;
    stored.is_volatile_method = syntax.is_volatile_method;
    stored.ref_qualifier = syntax.ref_qualifier;
    stored.parameters = owner_template_parameters;
    owner_template_decl->member_function_definitions[qualified_member->name].push_back(stored);
    invalidate_out_of_class_definition_caches(*owner_template_decl);
    emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                *qualified_member,
                                                stored.qualified_name,
                                                &inner,
                                                owner,
                                                &owner_template_parameters);
    emit_out_of_class_owner_class_use_if_needed(pattern_scope,
                                                *qualified_member,
                                                stored.qualified_name,
                                                &declarator,
                                                owner,
                                                &owner_template_parameters);
    return true;
  }

private:
  string source_location_for_node(const CppAstNode & node) const
  {
    return ctx.source_location_for_node(node);
  }

  string source_location_for_name_in_node(const CppAstNode & node,
                                          const string & name,
                                          bool prefer_last = false) const
  {
    return ctx.source_location_for_name_in_node(node, name, prefer_last);
  }

  string spaced_node_text(const CppAstNode & node) const
  {
    return callbacks.source_services->spaced_node_text(node);
  }

  string source_location_for_name_in_subtree(const CppAstNode & node,
                                             const string & name,
                                             bool prefer_last = false) const
  {
    return callbacks.source_services->source_location_for_name_in_subtree(
        node, name, prefer_last);
  }

  string earliest_qualified_use_location_for_prefix(const string & prefix) const
  {
    return callbacks.source_services->earliest_qualified_use_location_for_prefix(
        prefix);
  }

  string template_argument_key(const vector<TemplateArgument> & arguments) const
  {
    return template_api::template_argument_identity_key(ctx, arguments);
  }

  bool template_arguments_are_dependent(
      const vector<TemplateArgument> & arguments) const
  {
    return template_api::template_arguments_are_dependent(ctx, arguments);
  }

  bool parse_out_of_class_member_qualified_name(const string & qualified_name,
                                                QualifiedName & out)
  {
    return callbacks.out_of_class_services->parse_out_of_class_member_qualified_name(
        qualified_name, out);
  }

  ClassInfo * resolve_out_of_class_owner_class(
      Scope & scope,
      const QualifiedName & qualified,
      QualifiedOwnerClassResolution resolution = QualifiedOwnerClassResolution::Complete)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_owner_class(
        scope, qualified, resolution);
  }

  ClassInfo * resolve_qualified_owner_class(
      Scope & scope,
      const string & owner_name,
      QualifiedOwnerClassResolution resolution = QualifiedOwnerClassResolution::Complete)
  {
    return callbacks.out_of_class_services->resolve_qualified_owner_class(
        scope, owner_name, resolution);
  }

  // Build an owner-lookup anchor from a node's trailing qualifier template-id, so
  // a by-name owner resolution downstream resolves the owner's arguments from
  // structure instead of reparsed text.
  callsemantic::ExactTemplateTypeLookupAnchor owner_lookup_anchor_for_node(
      const CppAstNode & id_node)
  {
    callsemantic::ExactTemplateTypeLookupAnchor anchor;
    const QualifiedName * qn = cppast_qualified_name_syntax(id_node);
    if(!qn || qn->qualifiers.empty()) {
      return anchor;
    }
    const TemplateIdSyntax * ts =
        cppast_qualifier_template_id_syntax(id_node, qn->qualifiers.size() - 1);
    if(!ts || ts->name.name.empty() || ts->arguments.empty() ||
       ts->argument_syntaxes.size() != ts->arguments.size()) {
      return anchor;
    }
    anchor.template_text =
        callsemantic::template_id_syntax_text_preserving_spacing(*ts);
    anchor.identifier =
        callsemantic::template_lookup_fragment_identifier(anchor.template_text);
    if(anchor.identifier.empty()) {
      anchor.identifier = ts->name.name;
    }
    anchor.compact_key = callsemantic::compact_lookup_text(anchor.template_text);
    anchor.arg_texts = ts->arguments;
    anchor.arg_syntaxes = ts->argument_syntaxes;
    anchor.has_argument_list = true;
    return anchor;
  }

  ClassInfo * resolve_qualified_owner_class_from_template_id_syntax(
      Scope & scope,
      const QualifiedName & qualified_member,
      const CppAstNode * function_identifier,
      QualifiedOwnerClassResolution resolution = QualifiedOwnerClassResolution::Complete)
  {
    if(!function_identifier || qualified_member.qualifiers.empty()) {
      return nullptr;
    }

    const size_t owner_qualifier_index = qualified_member.qualifiers.size() - 1;
    const TemplateIdSyntax * owner_template_id =
        cppast_qualifier_template_id_syntax(*function_identifier,
                                            owner_qualifier_index);
    if(!owner_template_id) {
      owner_template_id = template_id_syntax_for_anchor(
          *function_identifier,
          strip_trailing_top_level_template_arguments(
              qualified_member.qualifiers[owner_qualifier_index]));
    }
    if(!owner_template_id || owner_template_id->name.name.empty()) {
      return nullptr;
    }

    Scope * owner_lookup_scope = &scope;
    if(qualified_member.rooted || qualified_member.qualifiers.size() > 1) {
      QualifiedName owner_scope_name;
      owner_scope_name.rooted = qualified_member.rooted;
      if(qualified_member.qualifiers.size() > 1) {
        owner_scope_name.qualifiers.assign(qualified_member.qualifiers.begin(),
                                           qualified_member.qualifiers.end() - 2);
        owner_scope_name.name =
            qualified_member.qualifiers[qualified_member.qualifiers.size() - 2];
      }
      owner_lookup_scope =
          semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
              ctx,
              scope,
              owner_scope_name);
    }
    if(!owner_lookup_scope) {
      return nullptr;
    }

    string owner_template_name =
        unqualified_member_name(owner_template_id->name.name);
    if(owner_template_name.empty()) {
      owner_template_name = owner_template_id->name.name;
    }
    ClassTemplateDecl * owner_template =
        lookup_class_template(*owner_lookup_scope, owner_template_name);
    if(!owner_template) {
      return nullptr;
    }

    const vector<string> owner_arg_texts =
        template_id_argument_texts_preserving_spacing(*owner_template_id);
    vector<TemplateArgument> owner_arguments;
    if(!resolve_template_arguments(scope,
                                   owner_template->parameters,
                                   owner_arg_texts,
                                   &owner_template_id->argument_syntaxes,
                                   owner_arguments,
                                   owner_template->declaring_scope)) {
      return nullptr;
    }

    const string key = template_argument_key(owner_arguments);
    const vector<string> * dependent_source_arg_texts =
        template_arguments_are_dependent(owner_arguments) ? &owner_arg_texts : nullptr;
    const template_api::specialization::ClassSpecializationSelection selection =
        template_api::specialization::select_class_specialization(
            ctx,
            *owner_template,
            scope,
            key,
            owner_arguments,
            dependent_source_arg_texts);
    if(!selection.class_node) {
      return nullptr;
    }

    const string owner_text =
        template_id_syntax_text_preserving_spacing(*owner_template_id);
    const bool reference_members_only =
        resolution == QualifiedOwnerClassResolution::ReferenceMembers ||
        (ctx.scope_has_template_placeholders(scope) &&
         text_mentions_template_placeholders(scope, owner_text));
    ClassInfo * info = reference_members_only ?
        ctx.reference_selected_class_template_instantiation(
            *owner_template,
            scope,
            owner_arguments,
            selection,
            &owner_arg_texts,
            template_api::ClassTemplateSourceUseMode::EmitClassUse,
            &owner_template_id->argument_syntaxes) :
        ctx.instantiate_selected_class_template(*owner_template,
                                                scope,
                                                owner_arguments,
                                                selection);
    if(info && reference_members_only) {
      ctx.ensure_class_reference_members(*info);
    }
    return info;
  }

  bool resolve_out_of_class_named_method_binding(
      Scope & scope,
      const QualifiedName & qualified,
      const string & member_name,
      const TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier,
      FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution = QualifiedOwnerClassResolution::Complete)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_named_method_binding(
        scope,
        qualified,
        member_name,
        declared_type,
        is_const_method,
        is_volatile_method,
        ref_qualifier,
        out,
        resolution);
  }

  bool resolve_out_of_class_named_method_binding(
      Scope & scope,
      const string & qualified_name,
      const string & member_name,
      const TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier,
      FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution = QualifiedOwnerClassResolution::Complete)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_named_method_binding(
        scope,
        qualified_name,
        member_name,
        declared_type,
        is_const_method,
        is_volatile_method,
        ref_qualifier,
        out,
        resolution);
  }

  bool out_of_class_special_member_template_parameters_match(
      Scope & lhs_scope,
      const vector<TemplateParameterInfo> & lhs,
      Scope & rhs_scope,
      const vector<TemplateParameterInfo> & rhs) const
  {
    return callbacks.out_of_class_services->
        out_of_class_special_member_template_parameters_match(
        lhs_scope, lhs, rhs_scope, rhs);
  }

  bool out_of_class_special_member_template_param_types_match(
      const TypePtr & lhs,
      const vector<TemplateParameterInfo> & lhs_parameters,
      const TypePtr & rhs,
      const vector<TemplateParameterInfo> & rhs_parameters) const
  {
    return callbacks.out_of_class_services->
        out_of_class_special_member_template_param_types_match(
        lhs, lhs_parameters, rhs, rhs_parameters);
  }

  FunctionTemplateDecl * resolve_out_of_class_special_member_template(
      Scope & scope,
      const QualifiedName & qualified,
      const vector<TemplateParameterInfo> & template_parameters,
      const vector<pair<string, TypePtr> > & params)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_special_member_template(
        scope, qualified, template_parameters, params);
  }

  FunctionTemplateDecl * resolve_out_of_class_special_member_template(
      Scope & scope,
      const string & qualified_name,
      const vector<TemplateParameterInfo> & template_parameters,
      const vector<pair<string, TypePtr> > & params)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_special_member_template(
        scope, qualified_name, template_parameters, params);
  }

  string describe_template_parameter_infos(
      const vector<TemplateParameterInfo> & parameters) const
  {
    return callbacks.out_of_class_services->describe_template_parameter_infos(
        parameters);
  }

  string describe_out_of_class_special_member_template_lookup(
      Scope & scope,
      const QualifiedName & qualified,
      const vector<TemplateParameterInfo> & template_parameters,
      const vector<pair<string, TypePtr> > & params)
  {
    return callbacks.out_of_class_services->
        describe_out_of_class_special_member_template_lookup(
        scope, qualified, template_parameters, params);
  }

  string describe_out_of_class_special_member_template_lookup(
      Scope & scope,
      const string & qualified_name,
      const vector<TemplateParameterInfo> & template_parameters,
      const vector<pair<string, TypePtr> > & params)
  {
    return callbacks.out_of_class_services->
        describe_out_of_class_special_member_template_lookup(
        scope, qualified_name, template_parameters, params);
  }

  string describe_out_of_class_special_member_binding_lookup(
      Scope & scope,
      const QualifiedName & qualified,
      const vector<pair<string, TypePtr> > & params)
  {
    return callbacks.out_of_class_services->
        describe_out_of_class_special_member_binding_lookup(
        scope, qualified, params);
  }

  string describe_out_of_class_special_member_binding_lookup(
      Scope & scope,
      const string & qualified_name,
      const vector<pair<string, TypePtr> > & params)
  {
    return callbacks.out_of_class_services->
        describe_out_of_class_special_member_binding_lookup(
        scope, qualified_name, params);
  }

  bool explicit_function_nothrow_specifications_match(
      FunctionBinding & existing,
      const CppAstNode * qualifier)
  {
    return callbacks.function_policy->explicit_function_nothrow_specifications_match(
        existing, qualifier);
  }

  bool declaration_marks_exclude_from_explicit_instantiation(
      const CppAstNode * declaration_node) const
  {
    return callbacks.function_policy->
        declaration_marks_exclude_from_explicit_instantiation(
        declaration_node);
  }

  symbol_linkage::SymbolLinkage function_symbol_linkage(
      Scope & scope,
      const CppAstNode * declaration_node,
      const CppAstNode * body,
      bool is_c_linkage,
      const CppAstNode * function_qualifier,
      const FunctionTemplateRegistrationIdentity & template_identity,
      bool is_defaulted = false,
      const ClassInfo * lexical_access_class = nullptr,
      bool in_class_member_definition_context = false) const
  {
    return callbacks.function_policy->function_symbol_linkage(
        scope,
        declaration_node,
        body,
        is_c_linkage,
        function_qualifier,
        template_identity,
        is_defaulted,
        lexical_access_class,
        in_class_member_definition_context);
  }

  void upgrade_function_symbol_linkage(FunctionBinding & binding,
                                       const string & qualified_name,
                                       const string & name,
                                       const TypePtr & type,
                                       symbol_linkage::SymbolLinkage linkage)
  {
    callbacks.function_policy->upgrade_function_symbol_linkage(
        binding, qualified_name, name, type, linkage);
  }

  witness::TemplateWitnessSourceAnchor class_use_selected_decl_anchor(
      ClassTemplateDecl * class_template,
      const template_api::ClassSpecializationSelection & selection)
  {
    return callsemantic::class_use_selected_decl_anchor(
        ctx, class_template, selection);
  }

  void emit_nested_class_use_source_events_from_location(
      Scope & scope,
      const string & location,
      witness::SourceUseOwnership ownership,
      const string & skip_exact_template_name)
  {
    callbacks.source_services->emit_nested_class_use_source_events_from_location(
        scope, location, ownership, skip_exact_template_name);
  }

  void overlay_direct_scope_bindings(Scope & target, const Scope & source) const
  {
    template_api::binding::overlay_direct_scope_bindings(target, source);
  }

  template_api::TemplateWitnessContext template_witness_context() const
  {
    return ctx.template_witness_context();
  }

  Scope & append_template_scope(Scope & parent)
  {
    return ctx.append_template_scope(parent);
  }

  bool parse_template_parameters(const CppAstNode & clause,
                                 vector<TemplateParameterInfo> & out,
                                 Scope * placeholder_scope = nullptr,
                                 string * failure_reason = nullptr)
  {
    return callbacks.declaration_services->parse_template_parameters(
        clause, out, placeholder_scope, failure_reason);
  }

  void collect_deduction_guide_declaration(
      Scope & scope,
      const CppAstNode & node,
      Scope * pattern_scope,
      const vector<TemplateParameterInfo> * parameters)
  {
    callbacks.declaration_services->collect_deduction_guide_declaration(
        scope, node, pattern_scope, parameters);
  }

  void record_template_parameter_clause_source_uses(Scope & scope,
                                                    const CppAstNode & node)
  {
    callbacks.declaration_services->record_template_parameter_clause_source_uses(
        scope, node);
  }

  void record_class_template_base_source_uses(const CppAstNode * class_node,
                                              Scope * pattern_scope)
  {
    callbacks.declaration_services->record_class_template_base_source_uses(
        class_node, pattern_scope);
  }

  ClassTemplateDecl * direct_class_template(Scope & scope, const string & name)
  {
    return semantic_lookup::lookup_direct_class_template(scope, name);
  }

  bool merge_template_parameter_redeclarations(
      vector<TemplateParameterInfo> & target,
      const vector<TemplateParameterInfo> & incoming)
  {
    return callsemantic::merge_template_parameter_redeclarations(target, incoming);
  }

  void prefer_incoming_template_parameter_spellings(
      vector<TemplateParameterInfo> & target,
      const vector<TemplateParameterInfo> & incoming)
  {
    callsemantic::prefer_incoming_template_parameter_spellings(target, incoming);
  }

  bool merge_partial_class_template_specialization(
      ClassTemplateDecl & target,
      const PartialClassTemplateSpecializationDecl & incoming)
  {
    for(size_t i = 0; i < target.partial_specializations.size(); ++i) {
      PartialClassTemplateSpecializationDecl & existing =
          target.partial_specializations[i];
      if(existing.class_node == incoming.class_node ||
         existing.arg_texts == incoming.arg_texts) {
        bool changed = false;
        if(!existing.class_node ||
           existing.class_node->kind == CppAstKind::class_forward_declaration) {
          existing.class_node = incoming.class_node;
          changed = true;
        }
        if(existing.arg_syntaxes.empty() && !incoming.arg_syntaxes.empty()) {
          existing.arg_syntaxes = incoming.arg_syntaxes;
          changed = true;
        }
        if(existing.parameters.empty() && !incoming.parameters.empty()) {
          existing.parameters = incoming.parameters;
          changed = true;
        }
        if(!existing.declaring_scope && incoming.declaring_scope) {
          existing.declaring_scope = incoming.declaring_scope;
          changed = true;
        }
        if(!existing.pattern_scope && incoming.pattern_scope) {
          existing.pattern_scope = incoming.pattern_scope;
          changed = true;
        }
        if(changed) {
          ++target.specialization_epoch;
        }
        return changed;
      }
    }
    target.partial_specializations.push_back(incoming);
    ++target.specialization_epoch;
    return true;
  }

  void merge_pending_member_class_template_partial_specializations(
      ClassTemplateDecl & owner_template,
      const string & member_template_name,
      ClassInfo * owner_info,
      ClassTemplateDecl & member_template)
  {
    map<string, vector<PartialClassTemplateSpecializationDecl> >::const_iterator
        pending =
            owner_template.member_class_template_partial_specializations.find(
                member_template_name);
    if(pending == owner_template.member_class_template_partial_specializations.end()) {
      return;
    }
    for(size_t i = 0; i < pending->second.size(); ++i) {
      if(owner_info) {
        merge_member_class_template_partial_into_owner_info(
            owner_info, member_template_name, pending->second[i]);
      } else {
        merge_partial_class_template_specialization(member_template,
                                                   pending->second[i]);
      }
    }
  }

  void merge_member_class_template_partial_into_owner_info(
      ClassInfo * info,
      const string & member_template_name,
      const PartialClassTemplateSpecializationDecl & partial)
  {
    if(!info || !info->member_scope) {
      return;
    }
    map<string, ClassTemplateDecl *>::iterator found =
        info->member_scope->class_templates.find(member_template_name);
    if(found == info->member_scope->class_templates.end() || !found->second) {
      return;
    }
    PartialClassTemplateSpecializationDecl adapted = partial;
    Scope * partial_scope =
        adapted.pattern_scope ? adapted.pattern_scope : adapted.declaring_scope;
    if(partial_scope &&
       info->source_template &&
       !info->instantiation_arguments.empty()) {
      Scope & owner_bound_scope = append_template_scope(*partial_scope);
      owner_bound_scope.class_info = info;
      template_api::binding::bind_template_arguments_into_scope(
          ctx,
          owner_bound_scope,
          info->source_template->parameters,
          info->instantiation_arguments,
          nullptr);
      adapted.pattern_scope = &owner_bound_scope;
    }
    merge_partial_class_template_specialization(*found->second, adapted);
  }

  void record_owner_member_class_template_partial_specialization(
      ClassTemplateDecl & owner_template,
      const string & member_template_name,
      const PartialClassTemplateSpecializationDecl & partial)
  {
    vector<PartialClassTemplateSpecializationDecl> & pending =
        owner_template.member_class_template_partial_specializations[
            member_template_name];
    bool found_pending = false;
    for(size_t i = 0; i < pending.size(); ++i) {
      if(pending[i].class_node == partial.class_node ||
         pending[i].arg_texts == partial.arg_texts) {
        pending[i] = partial;
        found_pending = true;
        break;
      }
    }
    if(!found_pending) {
      pending.push_back(partial);
    }

    for(map<string, ClassInfo *>::iterator it = owner_template.instantiations.begin();
        it != owner_template.instantiations.end();
        ++it) {
      merge_member_class_template_partial_into_owner_info(
          it->second, member_template_name, partial);
    }
    for(map<string, ClassInfo *>::iterator it = owner_template.reference_instantiations.begin();
        it != owner_template.reference_instantiations.end();
        ++it) {
      merge_member_class_template_partial_into_owner_info(
          it->second, member_template_name, partial);
    }
    invalidate_out_of_class_definition_caches(owner_template);
  }

  ClassTemplateDecl * resolve_dependent_owner_member_class_template_owner(
      Scope & scope,
      Scope & pattern_scope,
      const QualifiedName & specialization_name,
      const vector<TemplateParameterInfo> & owner_template_parameters,
      string & member_template_name)
  {
    member_template_name.clear();
    if(specialization_name.qualifiers.empty()) {
      return nullptr;
    }

    string owner_template_name;
    if(!split_unqualified_template_head_text(specialization_name.qualifiers.back(),
                                             owner_template_name)) {
      return nullptr;
    }

    Scope * owner_lookup_scope = &scope;
    if(specialization_name.qualifiers.size() > 1 || specialization_name.rooted) {
      QualifiedName owner_scope_name;
      owner_scope_name.rooted = specialization_name.rooted;
      if(specialization_name.qualifiers.size() > 1) {
        owner_scope_name.qualifiers.assign(specialization_name.qualifiers.begin(),
                                           specialization_name.qualifiers.end() - 2);
        owner_scope_name.name =
            specialization_name.qualifiers[specialization_name.qualifiers.size() - 2];
      }
      owner_lookup_scope =
          semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
              *this, scope, owner_scope_name, true);
    }
    if(!owner_lookup_scope) {
      return nullptr;
    }

    ClassTemplateDecl * owner_template =
        lookup_class_template(*owner_lookup_scope, owner_template_name);
    if(!owner_template) {
      return nullptr;
    }
    Scope * owner_pattern_scope =
        owner_template->pattern_scope ? owner_template->pattern_scope :
        owner_template->declaring_scope;
    if(owner_pattern_scope &&
       !out_of_class_special_member_template_parameters_match(
           *owner_pattern_scope,
           owner_template->parameters,
           pattern_scope,
           owner_template_parameters)) {
      return nullptr;
    }

    member_template_name = specialization_name.name;
    return owner_template;
  }

  bool resolve_template_arguments(
      Scope & scope,
      const vector<TemplateParameterInfo> & parameters,
      const vector<string> & texts,
      const vector<TemplateArgumentSyntax> * syntaxes,
      vector<TemplateArgument> & out,
      Scope * default_argument_scope = nullptr)
  {
    return template_api::resolve_template_arguments(
        ctx, scope, parameters, texts, syntaxes, out, default_argument_scope);
  }

  bool resolve_template_arguments(
      Scope & scope,
      const vector<TemplateParameterInfo> & parameters,
      const vector<string> & texts,
      vector<TemplateArgument> & out,
      Scope * default_argument_scope = nullptr)
  {
    return resolve_template_arguments(
        scope, parameters, texts, nullptr, out, default_argument_scope);
  }

  bool fill_trailing_default_template_argument_texts(
      Scope & pattern_scope,
      const vector<TemplateParameterInfo> & parameters,
      const vector<string> & texts,
      Scope * default_argument_scope,
      vector<string> & out)
  {
    return callbacks.declaration_services->fill_trailing_default_template_argument_texts(
        pattern_scope, parameters, texts, default_argument_scope, out);
  }

  vector<const CppAstNode *> normalize_default_arguments(
      const vector<pair<string, TypePtr> > & params,
      const vector<const CppAstNode *> & default_arguments)
  {
    vector<const CppAstNode *> normalized_defaults(params.size(), nullptr);
    for(size_t i = 0; i < params.size() && i < default_arguments.size(); ++i) {
      normalized_defaults[i] = default_arguments[i];
    }
    return normalized_defaults;
  }

  vector<FunctionTemplateDecl *> direct_function_templates(Scope & scope,
                                                           const string & name)
  {
    return semantic_lookup::lookup_direct_function_templates(scope, name);
  }

  bool function_template_entities_match(
      const FunctionTemplateDecl & existing,
      Scope & candidate_entity_scope,
      Scope & candidate_template_scope,
      const string & candidate_name,
      const vector<TemplateParameterInfo> & candidate_parameters,
      const TypePtr & candidate_type,
      bool candidate_special_member_template,
      bool candidate_is_static_member,
      bool candidate_is_const_method,
      bool candidate_is_volatile_method,
      RefQualifier candidate_ref_qualifier)
  {
    return callbacks.function_policy->function_template_entities_match(
        existing,
        candidate_entity_scope,
        candidate_template_scope,
        candidate_name,
        candidate_parameters,
        candidate_type,
        candidate_special_member_template,
        candidate_is_static_member,
        candidate_is_const_method,
        candidate_is_volatile_method,
        candidate_ref_qualifier);
  }

  void record_definition_parameter_aliases(
      FunctionBinding & binding,
      const vector<pair<string, TypePtr> > & params)
  {
    callsemantic::record_definition_parameter_aliases(binding, params);
  }

  void record_definition_parameter_aliases(
      FunctionTemplateDecl & decl,
      const vector<pair<string, TypePtr> > & params)
  {
    callsemantic::record_definition_parameter_aliases(decl, params);
  }

  void ensure_function_template_parameter_aliases(FunctionTemplateDecl & decl)
  {
    semantic_model::ensure_function_template_parameter_aliases(decl);
  }

  void initialize_function_template_parameter_aliases(FunctionTemplateDecl & decl)
  {
    semantic_model::initialize_function_template_parameter_aliases(decl);
  }

  void inherit_pending_friend_function_template_access(FunctionTemplateDecl & decl)
  {
    callbacks.function_policy->inherit_pending_friend_function_template_access(
        decl);
  }

  void invalidate_out_of_class_definition_caches(ClassTemplateDecl & decl)
  {
    callsemantic::invalidate_out_of_class_definition_caches(decl);
  }

  PartialClassTemplateSpecializationDecl * find_partial_specialization_decl(
      ClassTemplateDecl & decl,
      const ClassInfo * owner)
  {
    return callsemantic::find_partial_specialization_decl(decl, owner);
  }

  void append_function_declaration_abi_tags(vector<string> & tags,
                                            const CppAstNode * declaration_node)
  {
    ::append_function_declaration_abi_tags(tags, declaration_node);
  }

  void refresh_definition_parameter_names(FunctionBinding & binding,
                                          const FunctionBinding & source)
  {
    callbacks.function_policy->refresh_definition_parameter_names(binding, source);
  }

  void refresh_definition_parameter_names(
      FunctionBinding & binding,
      const vector<pair<string, TypePtr> > & params)
  {
    callbacks.function_policy->refresh_definition_parameter_names(binding, params);
  }

  void emit_out_of_class_owner_class_use_if_needed(
      Scope & scope,
      const QualifiedName & qualified,
      const string & qualified_name,
      const CppAstNode * anchor_node = nullptr,
      ClassInfo * owner_override = nullptr,
      const vector<TemplateParameterInfo> * canonical_parameters = nullptr)
  {
    callbacks.source_services->emit_out_of_class_owner_class_use_if_needed(
        scope, qualified, qualified_name, anchor_node, owner_override, canonical_parameters);
  }

  void emit_out_of_class_owner_class_use_if_needed(
      Scope & scope,
      const string & qualified_name,
      const CppAstNode * anchor_node = nullptr,
      ClassInfo * owner_override = nullptr,
      const vector<TemplateParameterInfo> * canonical_parameters = nullptr)
  {
    callbacks.source_services->emit_out_of_class_owner_class_use_if_needed(
        scope, qualified_name, anchor_node, owner_override, canonical_parameters);
  }

  FunctionTemplateDecl * resolve_out_of_class_method_template(
      Scope & scope,
      const QualifiedName & qualified,
      const vector<TemplateParameterInfo> & template_parameters,
      const vector<pair<string, TypePtr> > & params,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_method_template(
        scope, qualified, template_parameters, params, is_const_method, is_volatile_method, ref_qualifier);
  }

  FunctionTemplateDecl * resolve_out_of_class_method_template(
      Scope & scope,
      const string & qualified_name,
      const string & member_name,
      const vector<TemplateParameterInfo> & template_parameters,
      const vector<pair<string, TypePtr> > & params,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_method_template(
        scope, qualified_name, member_name, template_parameters, params, is_const_method, is_volatile_method, ref_qualifier);
  }

  bool resolve_out_of_class_method_binding_with_resolution(
      Scope & scope,
      const QualifiedName & qualified,
      const TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier,
      FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution)
  {
    return callbacks.out_of_class_services->
        resolve_out_of_class_method_binding_with_resolution(
        scope, qualified, declared_type, is_const_method, is_volatile_method, ref_qualifier, out, resolution);
  }

  bool resolve_out_of_class_method_binding_with_resolution(
      Scope & scope,
      const string & qualified_name,
      const TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier,
      FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution)
  {
    return callbacks.out_of_class_services->
        resolve_out_of_class_method_binding_with_resolution(
        scope, qualified_name, declared_type, is_const_method, is_volatile_method, ref_qualifier, out, resolution);
  }

  bool resolve_out_of_class_static_member_binding(Scope & scope,
                                                  const QualifiedName & qualified,
                                                  ValueBinding *& out)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_static_member_binding(
        scope, qualified, out);
  }

  bool resolve_out_of_class_static_member_binding(Scope & scope,
                                                  const string & qualified_name,
                                                  ValueBinding *& out)
  {
    return callbacks.out_of_class_services->resolve_out_of_class_static_member_binding(
        scope, qualified_name, out);
  }

  CppAstNode filtered_function_declarator(const CppAstNode & declarator) const
  {
    return callbacks.function_policy->filtered_function_declarator(declarator);
  }

  template_api::TemplateInstantiationResult acquire_function_template(
      FunctionTemplateDecl & decl,
      const vector<TemplateArgument> & arguments,
      Scope * use_scope = nullptr,
      const map<string, size_t> * pack_sizes = nullptr,
      bool include_body = true,
      bool explicit_specialization = false,
      const CppAstNode * body_override = nullptr,
      const CppAstNode * definition_node_override = nullptr,
      bool explicit_specialization_is_constexpr = false,
      bool prefer_overload_suffix = false,
      ClassInfo * active_owner = nullptr,
      template_api::TemplateInstantiationIntent intent =
          template_api::TemplateInstantiationIntent::LookupOnly)
  {
    template_api::TemplateFunctionInstantiationRequest request;
    request.decl = &decl;
    request.arguments = arguments;
    request.active_owner = active_owner;
    request.use_scope = use_scope ? template_api::make_template_environment(*use_scope) :
                                    template_api::TemplateEnvironmentHandle();
    request.body_override = body_override;
    request.definition_node_override = definition_node_override;
    request.explicit_specialization = explicit_specialization;
    request.explicit_specialization_is_constexpr =
        explicit_specialization_is_constexpr;
    request.include_body = include_body;
    request.prefer_overload_suffix = prefer_overload_suffix;
    request.intent = intent;
    if(pack_sizes) {
      request.pack_sizes = *pack_sizes;
      request.has_pack_sizes = true;
    }
    return template_api::acquire_function_instantiation(ctx, request);
  }

  bool deduce_function_template_arguments(FunctionTemplateDecl & decl,
                                          const vector<ExprInfo> & args,
                                          vector<TemplateArgument> & out,
                                          Scope * use_scope = nullptr,
                                          map<string, size_t> * pack_sizes_out = nullptr)
  {
    template_api::TemplateFunctionDeductionRequest request;
    request.decl = &decl;
    request.args = &args;
    request.use_scope = use_scope;
    template_api::TemplateFunctionDeductionResult result;
    const bool okay = template_api::deduce_function_template(ctx, request, result);
    if(okay) {
      out = result.arguments;
      if(pack_sizes_out) {
        *pack_sizes_out = result.pack_sizes;
      }
    }
    return okay;
  }

  void record_friend_function_template_declaration(
      ClassInfo & info,
      Scope & entity_scope,
      Scope & template_scope,
      const string & name,
      const vector<TemplateParameterInfo> & template_parameters,
      const TypePtr & type_pattern,
      const vector<pair<string, TypePtr> > & params_pattern,
      const CppAstNode & result_type_pattern,
      const vector<const CppAstNode *> & default_arguments_pattern,
      const vector<const CppAstNode *> & parameter_declarations_pattern,
      bool is_static_member,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier,
      bool qualified_friend_name,
      const CppAstNode * specifiers,
      const CppAstNode * declarator,
      const CppAstNode * body,
      const CppAstNode * declaration_node)
  {
    callbacks.function_policy->record_friend_function_template_declaration(
        info,
        entity_scope,
        template_scope,
        name,
        template_parameters,
        type_pattern,
        params_pattern,
        result_type_pattern,
        default_arguments_pattern,
        parameter_declarations_pattern,
        is_static_member,
        is_const_method,
        is_volatile_method,
        ref_qualifier,
        qualified_friend_name,
        specifiers,
        declarator,
        body,
        declaration_node);
  }

  ClassTemplateDecl * lookup_class_template(Scope & scope, const string & name)
  {
    return ctx.lookup_class_template(scope, name);
  }

  vector<FunctionTemplateDecl *> lookup_function_templates(Scope & scope,
                                                           const string & name)
  {
    return ctx.lookup_function_templates(scope, name);
  }

  vector<FunctionTemplateDecl *> lookup_function_templates(Scope & scope,
                                                           const QualifiedName & name)
  {
    return ctx.lookup_qualified_function_templates(scope, name);
  }

  VariableTemplateDecl * lookup_variable_template(Scope & scope, const string & name)
  {
    return ctx.lookup_variable_template(scope, name);
  }

  AliasTemplateDecl * lookup_alias_template(Scope & scope, const string & name)
  {
    return ctx.lookup_alias_template(scope, name);
  }

  bool parse_type_id(Scope & scope,
                     const CppAstNode & node,
                     TypePtr & type,
                     bool reference_class_templates_only = false,
                     bool record_class_template_use = true)
  {
    return ctx.parse_type_id(scope,
                             node,
                             type,
                             reference_class_templates_only,
                             record_class_template_use);
  }

  bool parse_decl_spec(const CppAstNode & node,
                       Scope & scope,
                       bool & is_typedef,
                       TypePtr & out,
                       bool reference_class_templates_only = false)
  {
    return ctx.parse_decl_spec(node, scope, is_typedef, out, reference_class_templates_only);
  }

  bool parse_declarator(Scope & scope,
                        const CppAstNode & declarator,
                        const TypePtr & base,
                        string & name,
                        TypePtr & type,
                        bool reference_class_templates_only = false)
  {
    return ctx.parse_declarator(
        scope, declarator, base, name, type, reference_class_templates_only);
  }

  bool parse_parameter_clause(Scope & scope,
                              const CppAstNode & node,
                              vector<pair<string, TypePtr> > & params,
                              vector<const CppAstNode *> * default_args_out = nullptr,
                              bool reference_class_templates_only = false)
  {
    return ctx.parse_parameter_clause(
        scope, node, params, default_args_out, reference_class_templates_only);
  }

  bool parse_function_definition_base(Scope & scope,
                                      const CppAstNode & specifiers,
                                      const CppAstNode & declarator,
                                      const CppAstNode & body,
                                      bool is_const_method,
                                      bool is_volatile_method,
                                      bool & is_typedef,
                                      TypePtr & base,
                                      bool reference_class_templates_only = false)
  {
    return ctx.parse_function_definition_base(scope,
                                              specifiers,
                                              declarator,
                                              body,
                                              is_const_method,
                                              is_volatile_method,
                                              is_typedef,
                                              base,
                                              reference_class_templates_only);
  }

  bool parse_trailing_return_base(Scope & scope,
                                  const CppAstNode & specifiers,
                                  const CppAstNode & declarator,
                                  bool & is_typedef,
                                  TypePtr & base,
                                  bool reference_class_templates_only)
  {
    return ctx.parse_trailing_return_base(
        scope, specifiers, declarator, is_typedef, base, reference_class_templates_only);
  }

  bool parse_variable_declaration_type(Scope & scope,
                                       const CppAstNode & specifiers,
                                       const CppAstNode & declarator,
                                       const CppAstNode * initializer,
                                       bool allow_auto,
                                       string & name,
                                       TypePtr & type,
                                       bool & is_typedef,
                                       bool allow_unnamed = false)
  {
    return ctx.parse_variable_declaration_type(
        scope, specifiers, declarator, initializer, allow_auto, name, type, is_typedef, allow_unnamed);
  }

  bool template_parameter_name_is_owned(
      const string & name,
      const vector<TemplateParameterInfo> & parameters) const
  {
    if(name.empty()) {
      return true;
    }
    for(size_t i = 0; i < parameters.size(); ++i) {
      const string placeholder_payload =
          parameters[i].placeholder_key.find("template-parameter ") == 0 ?
              parameters[i].placeholder_key.substr(19) :
              parameters[i].placeholder_key;
      if(parameters[i].name == name ||
         placeholder_payload == name) {
        return true;
      }
      if(!parameters[i].name.empty() &&
         name.compare(0, parameters[i].name.size(), parameters[i].name) == 0 &&
         name.size() > parameters[i].name.size() &&
         name[parameters[i].name.size()] == '#') {
        return true;
      }
      for(size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
        const string & alternate = parameters[i].alternate_names[j];
        if(alternate == name ||
           (!alternate.empty() &&
            name.compare(0, alternate.size(), alternate) == 0 &&
            name.size() > alternate.size() &&
            name[alternate.size()] == '#')) {
          return true;
        }
      }
    }
    return false;
  }

  bool bare_type_id_template_parameter_name(
      const CppAstNode & type_id,
      string & out) const
  {
    out.clear();
    if(type_id.kind != CppAstKind::type_id ||
       type_id.children.size() != 1) {
      return false;
    }
    const CppAstNode & specifiers = type_id.children[0];
    if(specifiers.kind != CppAstKind::type_specifier_seq ||
       specifiers.children.size() != 1) {
      return false;
    }
    const CppAstNode & type_name = specifiers.children[0];
    if(type_name.kind != CppAstKind::type_name) {
      return false;
    }
    const string name = trim_space(type_name.value);
    if(!callsemantic_internal::is_identifier_text(name)) {
      return false;
    }
    out = name;
    return true;
  }

  bool template_argument_syntax_names_unowned_template_parameter(
      const TemplateArgumentSyntax & syntax,
      const vector<TemplateParameterInfo> & parameters) const
  {
    string name;
    if(syntax.type_id &&
       bare_type_id_template_parameter_name(*syntax.type_id, name) &&
       !template_parameter_name_is_owned(name, parameters)) {
      return true;
    }
    return false;
  }

  bool template_id_syntax_mentions_unowned_template_parameters(
      const TemplateIdSyntax & syntax,
      const vector<TemplateParameterInfo> & parameters,
      set<const Type *> & visiting) const
  {
    for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
      if(template_argument_syntax_mentions_unowned_template_parameters(
             syntax.argument_syntaxes[i], parameters, visiting)) {
        return true;
      }
    }
    return false;
  }

  bool template_argument_syntax_mentions_unowned_template_parameters(
      const TemplateArgumentSyntax & syntax,
      const vector<TemplateParameterInfo> & parameters,
      set<const Type *> & visiting) const
  {
    if(template_argument_syntax_names_unowned_template_parameter(
           syntax, parameters)) {
      return true;
    }
    if(syntax.type_id &&
       syntax.type_id->semantic_type &&
       type_mentions_unowned_template_parameters_impl(
           syntax.type_id->semantic_type, parameters, visiting)) {
      return true;
    }
    if(syntax.expression &&
       syntax.expression->semantic_type &&
       type_mentions_unowned_template_parameters_impl(
           syntax.expression->semantic_type, parameters, visiting)) {
      return true;
    }
    if(syntax.template_id &&
       template_id_syntax_mentions_unowned_template_parameters(
           *syntax.template_id, parameters, visiting)) {
      return true;
    }
    return false;
  }

  bool dependent_alias_arguments_mention_unowned_template_parameters(
      const vector<DependentAliasTemplateArgumentSyntax> & arguments,
      const vector<TemplateParameterInfo> & parameters,
      set<const Type *> & visiting) const
  {
    for(size_t i = 0; i < arguments.size(); ++i) {
      if(arguments[i].type &&
         type_mentions_unowned_template_parameters_impl(
             arguments[i].type, parameters, visiting)) {
        return true;
      }
      if(template_argument_syntax_mentions_unowned_template_parameters(
             arguments[i].syntax, parameters, visiting)) {
        return true;
      }
    }
    return false;
  }

  bool type_mentions_unowned_template_parameters_impl(
      const TypePtr & type,
      const vector<TemplateParameterInfo> & parameters,
      set<const Type *> & visiting) const
  {
    if(!type) {
      return false;
    }
    const Type * key = type.get();
    if(!visiting.insert(key).second) {
      return false;
    }
    struct Guard {
      set<const Type *> & visiting;
      const Type * key;
      ~Guard() { visiting.erase(key); }
    } guard = { visiting, key };

    switch(type->kind) {
    case Type::TK_FUNDAMENTAL:
      return false;

    case Type::TK_NAMED:
    {
      if(named_type_is_template_parameter(type) &&
         !template_parameter_name_is_owned(
             named_type_semantic_payload(type), parameters)) {
        return true;
      }

      void * alias_template_decl = nullptr;
      vector<DependentAliasTemplateArgumentSyntax> alias_arguments;
      if(named_type_dependent_alias_template(type,
                                             alias_template_decl,
                                             alias_arguments) &&
         dependent_alias_arguments_mention_unowned_template_parameters(
             alias_arguments, parameters, visiting)) {
        return true;
      }

      void * class_template_decl = nullptr;
      vector<DependentAliasTemplateArgumentSyntax> class_arguments;
      if(named_type_dependent_class_template(type,
                                             class_template_decl,
                                             class_arguments) &&
         dependent_alias_arguments_mention_unowned_template_parameters(
             class_arguments, parameters, visiting)) {
        return true;
      }

      TypePtr owner;
      vector<string> members;
      vector<TemplateIdSyntax> member_template_ids;
      bool leading_typename = false;
      if(named_type_dependent_qualified_member(type,
                                               owner,
                                               members,
                                               leading_typename,
                                               &member_template_ids)) {
        (void)members;
        (void)leading_typename;
        if(type_mentions_unowned_template_parameters_impl(
               owner, parameters, visiting)) {
          return true;
        }
        for(size_t i = 0; i < member_template_ids.size(); ++i) {
          if(template_id_syntax_mentions_unowned_template_parameters(
                 member_template_ids[i], parameters, visiting)) {
            return true;
          }
        }
      }
      return false;
    }

    case Type::TK_CV:
    case Type::TK_ATOMIC:
    case Type::TK_POINTER:
    case Type::TK_BLOCK_POINTER:
    case Type::TK_LVALUE_REFERENCE:
    case Type::TK_RVALUE_REFERENCE:
    case Type::TK_ARRAY:
      return type_mentions_unowned_template_parameters_impl(
          type->inner, parameters, visiting);

    case Type::TK_MEMBER_POINTER:
      return type_mentions_unowned_template_parameters_impl(
                 type->owner, parameters, visiting) ||
             type_mentions_unowned_template_parameters_impl(
                 type->inner, parameters, visiting);

    case Type::TK_FUNCTION:
      if(type_mentions_unowned_template_parameters_impl(
             type->inner, parameters, visiting)) {
        return true;
      }
      for(size_t i = 0; i < type->params.size(); ++i) {
        if(type_mentions_unowned_template_parameters_impl(
               type->params[i], parameters, visiting)) {
          return true;
        }
      }
      return false;
    }
    return false;
  }

  bool type_mentions_unowned_template_parameters(
      const TypePtr & type,
      const vector<TemplateParameterInfo> & parameters) const
  {
    set<const Type *> visiting;
    return type_mentions_unowned_template_parameters_impl(
        type, parameters, visiting);
  }

  bool prepare_namespace_scope_specifiers(Scope & scope,
                                          const CppAstNode & specifiers,
                                          const CppAstNode * declarators,
                                          bool collect_embedded_types,
                                          bool collect_named_forward_declarations,
                                          CppAstNode & out)
  {
    return ctx.prepare_namespace_scope_specifiers(scope,
                                                 specifiers,
                                                 declarators,
                                                 collect_embedded_types,
                                                 collect_named_forward_declarations,
                                                 out);
  }

  Scope * resolve_qualified_function_parse_scope(Scope & scope,
                                                 const CppAstNode & declarator)
  {
    return ctx.resolve_qualified_function_parse_scope(scope, declarator);
  }

  bool resolve_out_of_class_special_member_binding(
      Scope & scope,
      const QualifiedName & qualified,
      const vector<pair<string, TypePtr> > & params,
      FunctionBinding *& out)
  {
    return ctx.resolve_out_of_class_special_member_binding(scope, qualified, params, out);
  }

  bool resolve_out_of_class_special_member_binding(
      Scope & scope,
      const string & qualified_name,
      const vector<pair<string, TypePtr> > & params,
      FunctionBinding *& out)
  {
    return ctx.resolve_out_of_class_special_member_binding(scope, qualified_name, params, out);
  }

  bool resolve_out_of_class_method_binding(
      Scope & scope,
      const QualifiedName & qualified,
      const TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      RefQualifier ref_qualifier,
      FunctionBinding *& out)
  {
    return ctx.resolve_out_of_class_method_binding(
        scope, qualified, declared_type, is_const_method, is_volatile_method, ref_qualifier, out);
  }

  FunctionBinding * register_function_entity(const FunctionRegistrationRequest & request)
  {
    return ctx.register_function_entity(request);
  }

  bool evaluate_initializer_constant_value(Scope & scope,
                                           const CppAstNode & initializer,
                                           const TypePtr & target,
                                           constant_eval::ConstexprValue & value)
  {
    return ctx.evaluate_initializer_constant_value(scope, initializer, target, value);
  }

  bool text_mentions_template_placeholders(Scope & scope, const string & text) const
  {
    return ctx.text_mentions_template_placeholders(scope, text);
  }

  bool text_mentions_dependent_non_namespace_binding_names(Scope & scope,
                                                           const string & text) const
  {
    return ctx.text_mentions_dependent_non_namespace_binding_names(scope, text);
  }

  bool type_depends_on_template_parameter(const TypePtr & type) const
  {
    return ctx.type_depends_on_template_parameter(type);
  }

  ClassInfo * class_info_for_type(const TypePtr & type) const
  {
    return ctx.class_info_for_type(type);
  }

  TypePtr lookup_type(Scope & scope,
                      const string & name,
                      bool reference_class_templates_only = false)
  {
    return ctx.lookup_type(scope, name, reference_class_templates_only);
  }

  void emit_nested_class_use_source_events_from_location(Scope & scope,
                                                         const string & location,
                                                         witness::SourceUseOwnership ownership)
  {
    ctx.emit_nested_class_use_source_events_from_location(
        scope, location, ownership);
  }

  void upgrade_function_symbol_linkage(FunctionBinding * binding,
                                       symbol_linkage::SymbolLinkage linkage)
  {
    ctx.upgrade_function_symbol_linkage(binding, linkage);
  }

  SemanticContext & ctx;
  const TemplateDeclarationCollectorServices & callbacks;
  vector<unique_ptr<ClassTemplateDecl> > & class_templates;
  vector<unique_ptr<AliasTemplateDecl> > & alias_templates;
  vector<unique_ptr<FunctionTemplateDecl> > & function_templates;
  vector<unique_ptr<VariableTemplateDecl> > & variable_templates;
};

}  // namespace

void collect_template_declaration_impl(
    SemanticContext & ctx,
    TemplateDeclarationCollectorState & state,
    const TemplateDeclarationCollectorServices & callbacks,
    Scope & scope,
    const CppAstNode & node,
    MemberAccess access,
    const vector<TemplateParameterInfo> * inherited_template_parameters)
{
  TemplateDeclarationCollector collector(ctx, state, callbacks);
  collector.collect_template_declaration_impl(
      scope, node, access, inherited_template_parameters);
}

}  // namespace callsemantic
