#include "semantic_output.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "callsemantic_internal.h"
#include "callsemantic/function_registry.h"
#include "pack_parameter_analysis.h"
#include "rtti_names.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_dependent_type.h"
#include "semantic_errors.h"
#include "semantic_hotspot.h"
#include "semantic_lifetime.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_parameter_recovery.h"
#include "semantic_scope_mutation.h"
#include "semantic_statement.h"
#include "semantic_template_function.h"
#include "semantic_template_output_policy.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "symbol_linkage.h"
#include "template_api.h"
#include "template_instantiation.h"
#include "template_scope.h"
#include "parser_trace.h"
#include "witness_api.h"

using namespace std;

namespace semantic_output {

using namespace cpp_decl;
using namespace semantic_class_model;
using namespace semantic_lookup;
using namespace semantic_model;
using semantic_template_output_policy::ClassOutputReadiness;
using DumpNode = CallSemNode;

FunctionBinding * resolve_output_function_binding(SemanticContext & ctx,
                                                  FunctionBinding * binding);

namespace {

class ScopedTemplateUseLocation
{
public:
  explicit ScopedTemplateUseLocation(const std::string & location)
      : active_(!location.empty())
  {
    if(active_) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedTemplateUseLocation()
  {
    if(active_) {
      parser_trace::pop_use_location();
    }
  }

private:
  bool active_;
};

bool output_value_binding_depends_on_template_parameters(SemanticContext & ctx,
                                                         const ValueBinding & binding)
{
  if(binding.dependent_template_value ||
     ctx.type_depends_on_template_parameter(binding.type)) {
    return true;
  }
  if(binding.has_constant_value || binding.has_constexpr_value) {
    return false;
  }
  if(!binding.constant_initializer || !binding.constant_initializer_scope) {
    return false;
  }
  return ctx.text_mentions_template_placeholders(*binding.constant_initializer_scope,
                                                 node_text(*binding.constant_initializer));
}

void apply_local_static_guard_to_lifetime_actions(DumpNode & var_node)
{
  if(callsem_local_static_guard_symbol(var_node).empty()) {
    return;
  }
  for(size_t i = 0; i < var_node.children.size(); ++i) {
    if(var_node.children[i].kind == CallSemKind::constructor_action ||
       var_node.children[i].kind == CallSemKind::destructor_action) {
      var_node.children[i].is_thread_local = var_node.is_thread_local;
      set_callsem_local_static_guard_symbol(
          var_node.children[i],
          callsem_local_static_guard_symbol(var_node));
    }
  }
}

const CppAstNode * single_special_initializer(const CppAstNode * initializer)
{
  if(!initializer ||
     initializer->kind != CppAstKind::initializer ||
     initializer->children.size() != 1 ||
     initializer->children[0].kind != CppAstKind::special_initializer) {
    return nullptr;
  }
  return &initializer->children[0];
}

bool namespace_variable_type_has_class_lifetime(SemanticContext & ctx,
                                                const TypePtr & type);

std::string describe_scope_placeholder_origin(SemanticContext & ctx, Scope & scope)
{
  std::set<std::string> seen_type_names;
  std::set<std::string> seen_type_pack_names;
  std::set<std::string> seen_value_names;

  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }

    const std::string scope_name =
        semantic_trace::scope_name_for_diagnostic(*current);

    for(auto it = current->named_types.begin();
        it != current->named_types.end();
        ++it) {
      if(seen_type_names.count(it->first) != 0) {
        continue;
      }
      if(ctx.type_depends_on_template_parameter(it->second)) {
        std::ostringstream out;
        out << "type " << it->first << " in " << scope_name
            << " -> " << describe_type(it->second);
        return out.str();
      }
    }

    for(std::map<std::string, std::vector<TypePtr> >::const_iterator it =
            current->named_type_packs.begin();
        it != current->named_type_packs.end();
        ++it) {
      if(seen_type_pack_names.count(it->first) != 0) {
        continue;
      }
      for(size_t i = 0; i < it->second.size(); ++i) {
        if(!ctx.type_depends_on_template_parameter(it->second[i])) {
          continue;
        }
        std::ostringstream out;
        out << "type-pack " << it->first << "[" << i << "] in " << scope_name
            << " -> " << describe_type(it->second[i]);
        return out.str();
      }
    }

    for(std::map<std::string, ValueBinding>::const_iterator it = current->values.begin();
        it != current->values.end();
        ++it) {
      if(seen_value_names.count(it->first) != 0) {
        continue;
      }
      if(!output_value_binding_depends_on_template_parameters(ctx, it->second)) {
        continue;
      }
      std::ostringstream out;
      out << "value " << it->first << " in " << scope_name;
      if(it->second.dependent_template_value) {
        out << " marked dependent-template-value";
      } else if(ctx.type_depends_on_template_parameter(it->second.type)) {
        out << " type=" << describe_type(it->second.type);
      } else if(it->second.constant_initializer) {
        out << " initializer=" << node_text(*it->second.constant_initializer);
      }
      return out.str();
    }

    for(auto it = current->named_types.begin();
        it != current->named_types.end();
        ++it) {
      seen_type_names.insert(it->first);
    }
    for(std::map<std::string, std::vector<TypePtr> >::const_iterator it =
            current->named_type_packs.begin();
        it != current->named_type_packs.end();
        ++it) {
      seen_type_pack_names.insert(it->first);
    }
    for(std::map<std::string, ValueBinding>::const_iterator it = current->values.begin();
        it != current->values.end();
        ++it) {
      seen_value_names.insert(it->first);
    }

    if(template_api::class_has_non_dependent_source_template_identity(
           current->class_info)) {
      break;
    }
  }
  return std::string();
}

bool class_has_template_identity(const ClassInfo * info)
{
  return template_api::class_has_template_identity(info);
}

void append_dump_virtual_base_layout(DumpNode & node, const ClassInfo * info)
{
  if(!info) {
    return;
  }
  for(size_t i = 0; i < info->virtual_base_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = info->virtual_base_subobjects[i];
    if(!subobject.type) {
      continue;
    }
    mutable_callsem_virtual_base_layout(node).push_back(
        make_pair(subobject.type->qualified_name, subobject.offset));
  }
}

bool is_virtual_base_view(const ClassInfo & info, const VTableInfo & table)
{
  if(table.view_offset == 0 || !table.view_type) {
    return false;
  }
  for(size_t i = 0; i < info.virtual_base_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = info.virtual_base_subobjects[i];
    if(subobject.type == table.view_type && subobject.offset == table.view_offset) {
      return true;
    }
  }
  return false;
}

ClassInfo * class_info_for_virtual_base_layout_param(SemanticContext & ctx,
                                                     const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(base && base->kind == Type::TK_POINTER) {
    base = strip_top_level_cv(base->inner);
  }
  if(!base) {
    return nullptr;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(info && info->complete) {
    return info->virtual_base_subobjects.empty() ? nullptr : info;
  }
  info = ctx.complete_class_type(base);
  if(info) {
    return info->virtual_base_subobjects.empty() ? nullptr : info;
  }
  info = ctx.class_info_for_type(base);
  return info && !info->virtual_base_subobjects.empty() ? info : nullptr;
}

const CppAstNode * find_function_body_node(const CppAstNode & node)
{
  if(const CppAstNode * body = find_child(node, CppAstKind::compound_statement)) {
    return body;
  }
  if(const CppAstNode * body = find_child(node, CppAstKind::lazy_function_body)) {
    return body;
  }
  return find_child(node, CppAstKind::try_block);
}

symbol_linkage::FunctionRefQualifier symbol_linkage_ref_qualifier(RefQualifier qualifier)
{
  switch(qualifier) {
  case RQ_LVALUE:
    return symbol_linkage::FRQ_LVALUE;
  case RQ_RVALUE:
    return symbol_linkage::FRQ_RVALUE;
  case RQ_NONE:
    return symbol_linkage::FRQ_NONE;
  }
  return symbol_linkage::FRQ_NONE;
}

symbol_linkage::SymbolLinkage output_function_symbol_linkage(const FunctionBinding & binding);
bool node_decl_spec_contains_token(const CppAstNode * node, ETokenType token);

bool output_function_prefers_local_object_binding(const FunctionBinding & binding)
{
  return binding.symbol.prefer_local_object_binding ||
         binding.exclude_from_explicit_instantiation;
}

symbol_linkage::SymbolIdentity output_function_symbol_identity(const FunctionBinding & binding)
{
  symbol_linkage::SymbolIdentity updated = binding.symbol;
  updated.linkage = output_function_symbol_linkage(binding);
  updated.prefer_local_object_binding =
      output_function_prefers_local_object_binding(binding);
  return updated;
}

symbol_linkage::SymbolIdentity function_entry_point_symbol(const FunctionBinding & binding,
                                                           symbol_linkage::SpecialMemberEntryPointKind kind)
{
  if(!binding.is_constructor && !binding.is_destructor) {
    return output_function_symbol_identity(binding);
  }

  std::string symbol_key = binding.symbol.object_symbol;
  if(symbol_key.empty()) {
    symbol_key = binding.symbol.internal_symbol;
  }
  if(!symbol_key.empty()) {
    symbol_key += "|entry=";
    switch(kind) {
    case symbol_linkage::SMEK_BASE:
      symbol_key += "base";
      break;
    case symbol_linkage::SMEK_DELETING:
      symbol_key += "deleting";
      break;
    case symbol_linkage::SMEK_COMPLETE:
      symbol_key += "complete";
      break;
    }
  }

  symbol_linkage::FunctionSymbolOptions options;
  options.is_member_function = binding.is_method;
  options.has_implicit_object_parameter = binding.is_method;
  options.is_const_method = binding.is_const_method;
  options.is_volatile_method = binding.is_volatile_method;
  options.ref_qualifier = symbol_linkage_ref_qualifier(binding.ref_qualifier);
  options.is_constructor = binding.is_constructor;
  options.is_destructor = binding.is_destructor;
  options.is_conversion_operator = binding.is_conversion_operator;
  options.special_member_entry_point_kind = kind;
  template_api::apply_function_binding_template_symbol_options(binding, options);
  options.abi_tags = function_binding_abi_tags(binding);
  options.lookup_scope = binding.declaration_scope;
  const string qualified_name = function_binding_qualified_name_for_symbol(binding);
  const string display_name = function_binding_display_name_for_symbol(binding);
  QualifiedName qualified_name_syntax;
  const bool has_qualified_name_syntax =
      function_binding_qualified_name_syntax_for_symbol(binding,
                                                        qualified_name_syntax);
  if(!has_qualified_name_syntax && !binding.is_c_linkage) {
    throw logic_error("missing qualified-name syntax for output function entry symbol " +
                      qualified_name);
  }

  symbol_linkage::SymbolIdentity updated =
      has_qualified_name_syntax ?
          symbol_linkage::make_function_symbol_identity(
              qualified_name_syntax,
              display_name,
              binding.is_c_linkage,
              binding.type,
              options,
              symbol_key,
              output_function_symbol_linkage(binding)) :
          symbol_linkage::make_c_function_symbol_identity(
              display_name,
              output_function_symbol_linkage(binding));
  const string base_internal_symbol =
      binding.symbol.internal_symbol.empty() ?
          symbol_linkage::internal_symbol_from_name(qualified_name) :
          binding.symbol.internal_symbol;
  if(binding.is_constructor || binding.is_destructor) {
    updated.internal_symbol =
        kind == symbol_linkage::SMEK_BASE ? base_internal_symbol + "__base_entry" :
        kind == symbol_linkage::SMEK_DELETING ? base_internal_symbol + "__deleting_entry" :
                                                base_internal_symbol;
  }
  if(updated.object_symbol.empty()) {
    updated.object_symbol = binding.symbol.object_symbol;
  }
  updated.keep_internal_alias = binding.symbol.keep_internal_alias;
  updated.prefer_local_object_binding =
      output_function_prefers_local_object_binding(binding);
  return updated;
}

bool is_secondary_virtual_destructor_slot(const FunctionBinding & binding,
                                          const FunctionBinding * slot_model,
                                          size_t slot_index)
{
  const FunctionBinding & slot_binding = slot_model ? *slot_model : binding;
  return slot_binding.is_destructor &&
         slot_binding.has_virtual_slot &&
         slot_index == slot_binding.virtual_slot + 1;
}

symbol_linkage::SymbolIdentity emitted_vtable_entry_symbol(const FunctionBinding & binding,
                                                           const FunctionBinding * slot_model,
                                                           size_t slot_index)
{
  if(binding.name.empty()) {
    return binding.symbol;
  }
  if(binding.is_destructor && binding.owner_class && binding.owner_class->is_polymorphic) {
    return function_entry_point_symbol(
        binding,
        is_secondary_virtual_destructor_slot(binding, slot_model, slot_index) ?
            symbol_linkage::SMEK_DELETING :
            symbol_linkage::SMEK_COMPLETE);
  }
  if(binding.symbol.object_symbol.size() != 0) {
    return output_function_symbol_identity(binding);
  }

  symbol_linkage::FunctionSymbolOptions options;
  options.is_member_function = binding.is_method;
  options.has_implicit_object_parameter = binding.is_method;
  options.is_const_method = binding.is_const_method;
  options.is_volatile_method = binding.is_volatile_method;
  options.ref_qualifier = symbol_linkage_ref_qualifier(binding.ref_qualifier);
  options.is_constructor = binding.is_constructor;
  options.is_destructor = binding.is_destructor;
  options.is_conversion_operator = binding.is_conversion_operator;
  template_api::apply_function_binding_template_symbol_options(binding, options);
  options.abi_tags = function_binding_abi_tags(binding);
  options.lookup_scope = binding.declaration_scope;
  const string qualified_name = function_binding_qualified_name_for_symbol(binding);
  const string display_name = function_binding_display_name_for_symbol(binding);
  QualifiedName qualified_name_syntax;
  const bool has_qualified_name_syntax =
      function_binding_qualified_name_syntax_for_symbol(binding,
                                                        qualified_name_syntax);
  if(!has_qualified_name_syntax && !binding.is_c_linkage) {
    return binding.symbol;
  }
  symbol_linkage::SymbolIdentity updated =
      has_qualified_name_syntax ?
          symbol_linkage::make_function_symbol_identity(
              qualified_name_syntax,
              display_name,
              binding.is_c_linkage,
              binding.type,
              options,
              string(),
              output_function_symbol_linkage(binding)) :
          symbol_linkage::make_c_function_symbol_identity(
              display_name,
              output_function_symbol_linkage(binding));
  if(updated.object_symbol.empty()) {
    return binding.symbol;
  }

  const string base_internal_symbol =
      binding.symbol.internal_symbol.empty() ?
          symbol_linkage::internal_symbol_from_name(qualified_name) :
          binding.symbol.internal_symbol;
  updated.internal_symbol = base_internal_symbol;
  updated.keep_internal_alias = binding.symbol.keep_internal_alias;
  return updated;
}

void trace_class_output_decision(const char * phase,
                                 const ClassInfo & info,
                                 const CppAstNode & node,
                                 const FunctionBinding * binding,
                                 bool emit_definition,
                                 bool should_emit_definition)
{
  if(!parser_trace::enabled("output.class")) {
    return;
  }

  ostringstream trace;
  trace << phase
        << " class=" << info.qualified_name
        << " node_kind=" << cppast_kind_text(node.kind)
        << " node_value=" << node.value
        << " explicit_specialization="
        << (template_api::class_is_explicit_specialization(&info) ? "yes" : "no")
        << " emit_definition=" << (emit_definition ? "yes" : "no")
        << " should_emit_definition=" << (should_emit_definition ? "yes" : "no");
  if(binding) {
    const bool definition_required =
        has_output_requirement(binding->output_requirements, ORK_DEFINITION);
    trace << " binding=" << binding->name
          << " has_definition=" << (binding->has_definition ? "yes" : "no")
          << " source_template="
          << (template_api::function_binding_has_source_template_identity(binding) ?
                  "yes" :
                  "no")
          << " definition_required=" << (definition_required ? "yes" : "no")
          << " output_requirements=" << binding->output_requirements;
  } else {
    trace << " binding=<none>";
  }
  parser_trace::note("output.class", string(), trace.str());
}

bool class_has_required_member_output(const ClassInfo & info)
{
  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const FunctionBinding * binding = it->second[i];
      if(binding &&
         has_output_requirement(binding->output_requirements, ORK_DEFINITION)) {
        return true;
      }
    }
  }
  return false;
}

bool static_member_variable_definition_output_suppressed(
    const ValueBinding & binding)
{
  return template_api::value_binding_output_suppressed_by_explicit_instantiation(
      binding);
}

bool class_has_required_static_member_output(const ClassInfo & info)
{
  if(!info.member_scope) {
    return false;
  }
  for(map<string, ValueBinding>::const_iterator it = info.member_scope->values.begin();
      it != info.member_scope->values.end();
      ++it) {
    const ValueBinding & binding = it->second;
    if(binding.owner_class == &info &&
       binding.kind == ValueBinding::VK_VARIABLE &&
       !binding.definition_output_emitted &&
       !static_member_variable_definition_output_suppressed(binding) &&
       has_output_requirement(binding.output_requirements, ORK_DEFINITION)) {
      return true;
    }
  }
  return false;
}

bool class_has_required_output(const ClassInfo & info)
{
  return class_has_required_member_output(info) ||
         class_has_required_static_member_output(info);
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

const CppAstNode * find_descendant_kind(const CppAstNode & node, CppAstKind kind)
{
  if(node.kind == kind) {
    return &node;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const CppAstNode * found = find_descendant_kind(node.children[i], kind)) {
      return found;
    }
  }
  return nullptr;
}

const CppAstNode * find_function_parameter_clause_in_declarator(const CppAstNode & node)
{
  const CppAstNode * found = nullptr;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::nested_declarator && child.children.size() == 1) {
      if(const CppAstNode * nested =
             find_function_parameter_clause_in_declarator(child.children[0])) {
        found = nested;
      }
      continue;
    }
    if(child.kind == CppAstKind::parameter_clause) {
      found = &child;
    }
  }
  return found;
}

bool is_pure_virtual_function_binding(const FunctionBinding & binding)
{
  if(binding.is_pure_virtual) {
    return true;
  }

  if(!binding.is_virtual || !binding.declaration_node) {
    return false;
  }
  return callsemantic_internal::declaration_node_is_pure_virtual(binding.declaration_node);
}

void merge_output_function_binding_metadata(FunctionBinding & out,
                                           const FunctionBinding & original)
{
  if(!out.declaration_node && original.declaration_node) {
    out.declaration_node = original.declaration_node;
  }
  if(!out.parameter_syntax_node && original.parameter_syntax_node) {
    out.parameter_syntax_node = original.parameter_syntax_node;
  }
  out.is_virtual = out.is_virtual || original.is_virtual;
  out.is_virtual_specified = out.is_virtual_specified || original.is_virtual_specified;
  out.is_pure_virtual = out.is_pure_virtual || original.is_pure_virtual;
  out.is_override_specified =
      out.is_override_specified || original.is_override_specified;
  out.is_final = out.is_final || original.is_final;
  out.is_explicit_instantiation_definition =
      out.is_explicit_instantiation_definition ||
      original.is_explicit_instantiation_definition;
}

bool parameter_declaration_has_pack(const CppAstNode & parameter)
{
  const CppAstNode * declarator = find_child(parameter, CppAstKind::declarator);
  if(!declarator) {
    declarator = find_child(parameter, CppAstKind::abstract_declarator);
  }
  return declarator && find_child(*declarator, CppAstKind::parameter_pack);
}

void append_function_exception_spec_candidates(SemanticContext & ctx,
                                               Scope & function_scope,
                                               FunctionBinding & binding,
                                               DumpNode & function_node)
{
  if(!binding.function_qualifier) {
    return;
  }

  const std::string & raw_qualifier = binding.function_qualifier->value;
  if(raw_qualifier.find("throw") == std::string::npos) {
    return;
  }

  const std::string qualifier = semantic_utils::trim_space(raw_qualifier);
  if(qualifier.compare(0, 6, "throw(") != 0 || qualifier.empty() || qualifier.back() != ')') {
    return;
  }

  function_node.has_dynamic_exception_spec = true;
  if(const std::vector<CppAstNode> * type_ids =
         cppast_exception_type_id_syntaxes(*binding.function_qualifier)) {
    for(size_t i = 0; i < type_ids->size(); ++i) {
      TypePtr exception_type;
      if(!ctx.parse_type_id(function_scope, (*type_ids)[i], exception_type) ||
         !exception_type) {
        throw std::logic_error("failed to parse dynamic exception specification type for " +
                               binding.name);
      }
      ctx.note_rtti_use(exception_type, false);
      DumpNode child = make_dump_node(CallSemKind::rtti_candidate,
                                      rtti_symbol_for_type(exception_type));
      child.semantic_type = exception_type;
      function_node.children.push_back(std::move(child));
    }
    return;
  }
}

bool infer_function_parameter_pack_size_from_type_bindings(SemanticContext & ctx,
                                                           Scope & function_scope,
                                                           const CppAstNode & parameter,
                                                           std::size_t & out_pack_size)
{
  (void)ctx;
  return pack_parameter_analysis::infer_named_type_pack_size(function_scope,
                                                             parameter,
                                                             out_pack_size);
}
std::string pack_value_alias_name(const std::string & pack_name, std::size_t index)
{
  if(index == 0) {
    return pack_name;
  }
  std::ostringstream out;
  out << pack_name << "__pack" << (index + 1);
  return out.str();
}

std::string function_parameter_output_name(const FunctionBinding & binding,
                                           std::size_t index)
{
  const std::string alias_name = function_parameter_alias_name(binding, index);
  if(!alias_name.empty()) {
    return alias_name;
  }
  return function_parameter_binding_name(binding, index);
}

std::string function_parameter_display_name(const FunctionBinding & binding,
                                            std::size_t index)
{
  const std::string alias_name = function_parameter_alias_name(binding, index);
  if(!alias_name.empty()) {
    return alias_name;
  }
  if(index < binding.params.size() && binding.params[index].first.empty() &&
     template_api::function_binding_has_parameter_name_syntax_source(binding)) {
    return std::string();
  }
  return function_parameter_binding_name(binding, index);
}

std::size_t binding_explicit_parameter_offset(const FunctionBinding & binding)
{
  return function_binding_explicit_parameter_offset(binding);
}

void bind_function_parameter_lookup(Scope & function_scope,
                                    const FunctionBinding & binding,
                                    std::size_t index,
                                    const std::vector<TypePtr> * parameter_object_types = nullptr)
{
  const std::string binding_name = function_parameter_binding_name(binding, index);
  if(binding_name.empty()) {
    return;
  }

  const std::string output_name = function_parameter_output_name(binding, index);
  TypePtr parameter_type = binding.params[index].second;
  if(parameter_object_types &&
     index < parameter_object_types->size() &&
     (*parameter_object_types)[index]) {
    parameter_type = (*parameter_object_types)[index];
  }
  ValueBinding parameter(ValueBinding::VK_PARAMETER,
                         output_name,
                         parameter_type);
  const std::string alias_name = function_parameter_alias_name(binding, index);
  semantic_scope_mutation::bind_value_aliases(function_scope,
                                              binding_name,
                                              alias_name,
                                              parameter);
}

const CppAstNode * parameter_clause_from_declarator_like(const CppAstNode * declarator)
{
  if(!declarator) {
    return nullptr;
  }
  if(declarator->kind == CppAstKind::function_definition ||
     declarator->kind == CppAstKind::special_member_definition ||
     declarator->kind == CppAstKind::special_member_declaration) {
    declarator = find_child_kind(*declarator, CppAstKind::declarator);
    if(!declarator) {
      return nullptr;
    }
  }
  return find_function_parameter_clause_in_declarator(*declarator);
}

const CppAstNode * binding_parameter_clause(const FunctionBinding & binding)
{
  if(binding.has_definition || binding.body) {
    if(binding.source_template && binding.source_template->definition_declarator) {
      if(const CppAstNode * parameter_clause =
             parameter_clause_from_declarator_like(
                 binding.source_template->definition_declarator)) {
        return parameter_clause;
      }
    }
    if(const CppAstNode * parameter_clause =
           parameter_clause_from_declarator_like(binding.definition_node)) {
      return parameter_clause;
    }
  }

  if(binding.parameter_syntax_node) {
    const CppAstNode * parameter_clause =
        find_function_parameter_clause_in_declarator(*binding.parameter_syntax_node);
    if(parameter_clause) {
      return parameter_clause;
    }
  }

  if(const CppAstNode * source_template_declarator =
         template_api::function_binding_source_template_declarator(binding)) {
    const CppAstNode * parameter_clause =
        find_function_parameter_clause_in_declarator(*source_template_declarator);
    if(parameter_clause) {
      return parameter_clause;
    }
  }

  const CppAstNode * declarator = binding.definition_node ? binding.definition_node :
                                                        binding.declaration_node;
  if(!declarator) {
    return nullptr;
  }
  return parameter_clause_from_declarator_like(declarator);
}

void recover_function_parameter_aliases_from_ast(SemanticContext & ctx,
                                                 FunctionBinding & binding,
                                                 Scope * parent_scope)
{
  (void)ctx;

  const CppAstNode * parameter_clause = binding_parameter_clause(binding);
  if(!parameter_clause) {
    return;
  }

  std::vector<const CppAstNode *> parameter_nodes;
  for(std::size_t i = 0; i < parameter_clause->children.size(); ++i) {
    if(parameter_clause->children[i].kind == CppAstKind::parameter_declaration) {
      parameter_nodes.push_back(&parameter_clause->children[i]);
    }
  }
  if(parameter_nodes.empty()) {
    return;
  }

  bool has_pack_parameter = false;
  for(std::size_t i = 0; i < parameter_nodes.size(); ++i) {
    if(parameter_declaration_has_pack(*parameter_nodes[i])) {
      has_pack_parameter = true;
      break;
    }
  }
  const std::size_t explicit_offset = binding_explicit_parameter_offset(binding);
  if(!has_pack_parameter &&
     binding.params.size() != explicit_offset + parameter_nodes.size()) {
    return;
  }

  ensure_function_parameter_aliases(binding);
  Scope alias_scope(parent_scope, "<parameter-alias-recovery>", false);
  Scope * alias_scope_ptr = nullptr;
  if(parent_scope) {
    alias_scope.class_info = binding.owner_class;
    alias_scope.function = &binding;
    template_scope::overlay_scope_bindings(alias_scope,
                                           *parent_scope,
                                           template_scope::OVERLAY_TEMPLATE_BOUND_ONLY);
    alias_scope_ptr = &alias_scope;
  }

  const auto infer_pack_size =
      [&](std::size_t parameter_index, std::size_t & out_size) -> bool
      {
        return alias_scope_ptr &&
               pack_parameter_analysis::infer_named_type_pack_size(
                   *alias_scope_ptr,
                   *parameter_nodes[parameter_index],
                   out_size);
      };

  std::size_t binding_index = explicit_offset;
  for(std::size_t i = 0; i < parameter_nodes.size(); ++i) {
    if(!parameter_declaration_has_pack(*parameter_nodes[i])) {
      if(binding_index >= binding.params.size()) {
        return;
      }
      const std::string parameter_name =
          pack_parameter_analysis::parameter_declaration_name(*parameter_nodes[i]);
      if(!parameter_name.empty() &&
         (binding.parameter_aliases[binding_index].empty() ||
          binding.parameter_aliases[binding_index] == binding.params[binding_index].first)) {
        binding.parameter_aliases[binding_index] = parameter_name;
      }
      ++binding_index;
      continue;
    }

    const std::string pack_name =
        pack_parameter_analysis::parameter_declaration_name(*parameter_nodes[i]);
    if(pack_name.empty()) {
      return;
    }

    std::size_t inferred_pack_size = 0;
    bool have_inferred_pack_size = infer_pack_size(i, inferred_pack_size);
    std::size_t remaining_reserved_params = 0;
    for(std::size_t j = i + 1; j < parameter_nodes.size(); ++j) {
      if(!parameter_declaration_has_pack(*parameter_nodes[j])) {
        ++remaining_reserved_params;
        continue;
      }
      std::size_t later_pack_size = 0;
      if(infer_pack_size(j, later_pack_size)) {
        remaining_reserved_params += later_pack_size;
      }
    }
    if(binding.params.size() < binding_index + remaining_reserved_params) {
      return;
    }
    const std::size_t pack_size =
        have_inferred_pack_size ?
            inferred_pack_size :
            binding.params.size() - binding_index - remaining_reserved_params;
    if(binding.params.size() < binding_index + pack_size) {
      return;
    }
    for(std::size_t j = 0; j < pack_size; ++j) {
      const std::size_t index = binding_index + j;
      binding.parameter_aliases[index] = pack_value_alias_name(pack_name, j);
    }
    binding_index += pack_size;
  }
}

std::size_t count_remaining_fixed_function_parameters(
    const std::vector<const CppAstNode *> & parameters,
    std::size_t first_index)
{
  std::size_t count = 0;
  for(std::size_t i = first_index; i < parameters.size(); ++i) {
    if(!parameter_declaration_has_pack(*parameters[i])) {
      ++count;
    }
  }
  return count;
}

bool infer_function_parameter_value_pack_size(SemanticContext & ctx,
                                              Scope & function_scope,
                                              const FunctionBinding & binding,
                                              const std::vector<const CppAstNode *> & parameters,
                                              std::size_t parameter_index,
                                              std::size_t binding_param_index,
                                              const std::string & pack_name,
                                              std::size_t & out_pack_size)
{
  std::map<std::string, std::size_t>::const_iterator found_size =
      function_scope.named_pack_sizes.find(pack_name);
  if(found_size != function_scope.named_pack_sizes.end()) {
    out_pack_size = found_size->second;
    return true;
  }

  if(infer_function_parameter_pack_size_from_type_bindings(ctx,
                                                           function_scope,
                                                           *parameters[parameter_index],
                                                           out_pack_size)) {
    return true;
  }

  const std::size_t remaining_fixed_params =
      count_remaining_fixed_function_parameters(parameters, parameter_index + 1);
  if(binding.params.size() < binding_param_index + remaining_fixed_params) {
    return false;
  }
  out_pack_size = binding.params.size() - binding_param_index - remaining_fixed_params;
  return true;
}

std::vector<const CppAstNode *> function_parameter_declarations(
    const CppAstNode & parameter_clause)
{
  std::vector<const CppAstNode *> parameters;
  for(size_t i = 0; i < parameter_clause.children.size(); ++i) {
    if(parameter_clause.children[i].kind == CppAstKind::parameter_declaration) {
      parameters.push_back(&parameter_clause.children[i]);
    }
  }
  return parameters;
}

std::vector<TypePtr> recover_function_parameter_object_types(
    SemanticContext & ctx,
    Scope & function_scope,
    const FunctionBinding & binding)
{
  std::vector<TypePtr> out;
  out.reserve(binding.params.size());
  for(std::size_t i = 0; i < binding.params.size(); ++i) {
    out.push_back(binding.params[i].second);
  }

  const CppAstNode * parameter_clause = binding_parameter_clause(binding);
  if(!parameter_clause) {
    return out;
  }

  std::vector<std::pair<std::string, TypePtr> > parsed_params;
  std::vector<TypePtr> explicit_parameter_object_types;
  bool parsed = false;
  try {
    parsed = ctx.parse_parameter_clause(function_scope,
                                        *parameter_clause,
                                        parsed_params,
                                        nullptr,
                                        false,
                                        &explicit_parameter_object_types);
  } catch(const std::logic_error &) {
    return out;
  }
  if(!parsed) {
    return out;
  }

  const std::size_t explicit_offset = binding_explicit_parameter_offset(binding);
  if(binding.params.size() != explicit_offset + explicit_parameter_object_types.size()) {
    return out;
  }
  for(std::size_t i = 0; i < explicit_parameter_object_types.size(); ++i) {
    if(explicit_parameter_object_types[i]) {
      out[explicit_offset + i] = explicit_parameter_object_types[i];
    }
  }
  return out;
}

/*
    The function-body output path sees already-instantiated function parameters,
    including any implicit object parameter. Walk the source parameter clause in
    lockstep with those concrete bindings so a pack may be followed by ordinary
    parameters, e.g. Args... args, Token token.
*/
void bind_function_parameter_pack_sizes(SemanticContext & ctx,
                                        Scope & function_scope,
                                        const FunctionBinding & binding)
{
  const CppAstNode * parameter_clause = binding_parameter_clause(binding);
  if(!parameter_clause) {
    return;
  }

  const std::vector<const CppAstNode *> parameters =
      function_parameter_declarations(*parameter_clause);
  if(parameters.empty()) {
    return;
  }

  std::size_t binding_param_index = binding_explicit_parameter_offset(binding);
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameter_declaration_has_pack(*parameters[i])) {
      if(binding_param_index < binding.params.size()) {
        ++binding_param_index;
      }
      continue;
    }

    const std::string pack_name =
        pack_parameter_analysis::parameter_declaration_name(*parameters[i]);
    if(pack_name.empty()) {
      return;
    }

    std::size_t pack_size = 0;
    if(!infer_function_parameter_value_pack_size(ctx,
                                                 function_scope,
                                                 binding,
                                                 parameters,
                                                 i,
                                                 binding_param_index,
                                                 pack_name,
                                                 pack_size)) {
      return;
    }
    semantic_scope_mutation::bind_named_pack_size(function_scope, pack_name, pack_size);
    binding_param_index += pack_size;
  }
}

void bind_function_parameter_value_packs(SemanticContext & ctx,
                                         Scope & function_scope,
                                         const FunctionBinding & binding)
{
  const CppAstNode * parameter_clause = binding_parameter_clause(binding);
  if(!parameter_clause) {
    return;
  }

  const std::vector<const CppAstNode *> parameters =
      function_parameter_declarations(*parameter_clause);
  if(parameters.empty()) {
    return;
  }

  std::size_t binding_param_index = binding_explicit_parameter_offset(binding);
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameter_declaration_has_pack(*parameters[i])) {
      if(binding_param_index < binding.params.size()) {
        ++binding_param_index;
      }
      continue;
    }

    const std::string pack_name =
        pack_parameter_analysis::parameter_declaration_name(*parameters[i]);
    if(pack_name.empty()) {
      return;
    }

    std::size_t pack_size = 0;
    if(!infer_function_parameter_value_pack_size(ctx,
                                                 function_scope,
                                                 binding,
                                                 parameters,
                                                 i,
                                                 binding_param_index,
                                                 pack_name,
                                                 pack_size)) {
      return;
    }
    if(binding.params.size() < binding_param_index + pack_size) {
      return;
    }

    std::vector<ValueBinding> pack_bindings;
    pack_bindings.reserve(pack_size);
    for(std::size_t j = 0; j < pack_size; ++j) {
      const TypePtr & element_type = binding.params[binding_param_index + j].second;
      const std::string alias_name = pack_value_alias_name(pack_name, j);
      pack_bindings.push_back(ValueBinding(ValueBinding::VK_PARAMETER, alias_name, element_type));
    }
    semantic_scope_mutation::bind_value_pack(function_scope, pack_name, pack_bindings);
    binding_param_index += pack_size;
  }
}

bool is_same_class_reference_parameter(const TypePtr & class_type,
                                       const TypePtr & param_type,
                                       Type::Kind ref_kind);

FunctionBinding * find_copy_constructor_binding(ClassInfo & info);
FunctionBinding * find_or_ensure_copy_constructor_binding(SemanticContext & ctx,
                                                          ClassInfo & info);

bool variable_declaration_is_definition(const CppAstNode & specifiers,
                                        const CppAstNode * initializer,
                                        const TypePtr & type,
                                        bool is_c_linkage = false,
                                        bool linkage_has_braces = false)
{
  TypePtr base = strip_top_level_cv(type);
  if(base && base->kind == Type::TK_FUNCTION) {
    return false;
  }
  if(is_c_linkage && !linkage_has_braces && initializer == nullptr &&
     !decl_spec_contains_token(specifiers, KW_CONSTEXPR)) {
    return false;
  }
  return !decl_spec_contains_token(specifiers, KW_EXTERN) ||
         decl_spec_contains_token(specifiers, KW_CONSTEXPR) ||
         initializer != nullptr;
}

ClassInfo * lookup_declared_class_info(SemanticContext & ctx,
                                       Scope & scope,
                                       const string & name)
{
  TypePtr type = ctx.lookup_type(scope, name);
  return type ? ctx.class_info_for_type(type) : nullptr;
}

bool has_user_declared_destructor(const ClassInfo & info)
{
  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      if(it->second[i]->is_destructor && !it->second[i]->synthesized) {
        return true;
      }
    }
  }
  return false;
}

bool is_function_local_class_info(const ClassInfo & info)
{
  return !info.is_lambda_closure &&
         info.class_kind != "union" &&
         info.enclosing_scope &&
         current_function_scope(*info.enclosing_scope) != nullptr;
}

bool is_same_class_reference_parameter(const TypePtr & class_type,
                                       const TypePtr & param_type,
                                       Type::Kind ref_kind)
{
  TypePtr base = strip_top_level_cv(param_type);
  if(!base || base->kind != ref_kind) {
    return false;
  }
  return semantic_conversion::same_type_with_compatible_top_cv(base->inner, class_type);
}

bool is_trivially_destructible_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_destructible_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(is_named_enum_type(ctx, base)) {
    return true;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete || has_user_declared_destructor(*info)) {
    return false;
  }
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(!is_trivially_destructible_type(ctx, info->bases[i].type->type)) {
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_destructible_type(ctx, info->fields[i].type)) {
      return false;
    }
  }
  return true;
}

bool is_trivially_copy_constructible_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_copy_constructible_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  return semantic_class_model::is_trivially_copy_constructible_type_for_host_abi(ctx, base);
}

bool is_complete_class_value_type_for_output(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED || !base->named_has_layout) {
    return false;
  }
  return base->named_key.compare(0, 6, "class ") == 0 ||
         base->named_key.compare(0, 7, "struct ") == 0 ||
         base->named_key.compare(0, 6, "union ") == 0;
}

ClassInfo * complete_class_object_type_for_output(SemanticContext & ctx,
                                                  const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED ||
     (base->named_key.compare(0, 6, "class ") != 0 &&
      base->named_key.compare(0, 7, "struct ") != 0 &&
      base->named_key.compare(0, 6, "union ") != 0)) {
    return nullptr;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(info && info->complete) {
    return info;
  }
  info = ctx.complete_class_type(base);
  return info && info->complete ? info : nullptr;
}

bool is_indirect_value_type_for_output(const TypePtr & type)
{
  return is_complete_class_value_type_for_output(type);
}

Scope * function_output_resolution_scope(const FunctionBinding * function)
{
  if(!function) {
    return nullptr;
  }
  if(function->declaration_scope) {
    return function->declaration_scope;
  }
  if(function->owner_class && function->owner_class->member_scope) {
    return function->owner_class->member_scope.get();
  }
  return nullptr;
}

TypePtr resolve_output_support_type(SemanticContext & ctx,
                                    Scope * scope,
                                    const TypePtr & type)
{
  if(!scope || !type || !ctx.type_depends_on_template_parameter(type)) {
    return type;
  }

  TypePtr resolved;
  if(semantic_dependent_type::resolve_instantiated_dependent_type(ctx,
                                                                  *scope,
                                                                  type,
                                                                  resolved) &&
     resolved) {
    return resolved;
  }
  return type;
}

TypePtr exception_object_type_for_output(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return TypePtr();
  }
  if(is_reference_type(base)) {
    return strip_top_level_cv(base->inner);
  }
  return base;
}

FunctionBinding * find_destructor_binding(ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find("~" + info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding && binding->is_destructor) {
      return binding;
    }
  }
  return nullptr;
}

void require_nontrivial_destructor_definition(SemanticContext & ctx,
                                              ClassInfo & info)
{
  if(is_trivially_destructible_type(ctx, info.type) &&
     !class_has_template_identity(&info)) {
    return;
  }
  if(FunctionBinding * dtor = find_destructor_binding(info)) {
    ctx.require_function_definition(dtor, OutputReason::SyntheticDependency);
  }
}

void require_temporary_destructor_definition(SemanticContext & ctx,
                                             ClassInfo & info)
{
  ensure_implicit_special_members(ctx, info);
  FunctionBinding * dtor = find_destructor_binding(info);
  if(dtor) {
    ctx.require_function_definition(dtor, OutputReason::SyntheticDependency);
  }
}

FunctionBinding * find_copy_constructor_binding(ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding &&
       binding->is_constructor &&
       binding->params.size() == 2 &&
       is_same_class_reference_parameter(info.type,
                                         binding->params[1].second,
                                         Type::TK_LVALUE_REFERENCE)) {
      return binding;
    }
  }
  return nullptr;
}

FunctionBinding * find_or_ensure_copy_constructor_binding(SemanticContext & ctx,
                                                          ClassInfo & info)
{
  if(FunctionBinding * binding = find_copy_constructor_binding(info)) {
    return binding;
  }
  return ctx.ensure_implicit_copy_constructor(info);
}

FunctionBinding * find_move_constructor_binding(ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding &&
       binding->is_constructor &&
       binding->params.size() == 2 &&
       is_same_class_reference_parameter(info.type,
                                         binding->params[1].second,
                                         Type::TK_RVALUE_REFERENCE)) {
      return binding;
    }
  }
  return nullptr;
}

FunctionBinding * find_or_ensure_move_constructor_binding(SemanticContext & ctx,
                                                          ClassInfo & info)
{
  if(FunctionBinding * binding = find_move_constructor_binding(info)) {
    return binding;
  }
  return ctx.ensure_implicit_move_constructor(info);
}

bool special_member_constructor_can_use_host_object_symbol(SemanticContext & ctx,
                                                          const FunctionBinding & binding)
{
  if(!binding.owner_class ||
     !binding.is_constructor ||
     binding.symbol.object_symbol.empty() ||
     ctx.emit_all_source_function_definitions()) {
    return false;
  }
  const symbol_linkage::SymbolLinkage linkage =
      output_function_symbol_linkage(binding);
  const bool host_owned =
      linkage == symbol_linkage::SL_EXTERNAL ||
      template_api::function_binding_output_suppressed_by_explicit_instantiation(
          binding);
  if(!host_owned) {
    return false;
  }
  const CppAstNode * definition_node =
      binding.definition_node ? binding.definition_node : binding.declaration_node;
  return ctx.definition_comes_from_standard_include_path(definition_node,
                                                        binding.body,
                                                        binding.is_defaulted);
}

void require_host_constructor_declaration(SemanticContext & ctx,
                                          FunctionBinding * binding)
{
  if(!binding) {
    return;
  }
  add_output_requirement(binding->output_requirements, ORK_DECLARATION);
  if(symbol_linkage::has_exported_object_symbol(binding->symbol)) {
    add_output_requirement(binding->output_requirements, ORK_EXPORT);
  }
  ctx.refresh_required_function_definition(binding, false);
}

void require_constructor_definition_if_needed(SemanticContext & ctx,
                                              FunctionBinding * binding,
                                              OutputReason reason)
{
  if(!binding) {
    return;
  }
  if(special_member_constructor_can_use_host_object_symbol(ctx, *binding)) {
    require_host_constructor_declaration(ctx, binding);
    return;
  }
  ctx.require_function_definition(binding, reason);
}

bool special_member_binding_has_trivial_lifecycle_output(SemanticContext & ctx,
                                                         FunctionBinding & binding)
{
  if(!binding.owner_class) {
    return false;
  }

  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);

  if(binding.is_destructor &&
     implicit_like &&
     is_trivially_destructible_type(ctx, binding.owner_class->type)) {
    return true;
  }

  if(binding.is_constructor &&
     binding.params.size() == 2 &&
     implicit_like &&
     (is_same_class_reference_parameter(binding.owner_class->type,
                                        binding.params[1].second,
                                        Type::TK_LVALUE_REFERENCE) ||
      is_same_class_reference_parameter(binding.owner_class->type,
                                        binding.params[1].second,
                                        Type::TK_RVALUE_REFERENCE)) &&
     is_trivially_copy_constructible_type(ctx, binding.owner_class->type)) {
    return true;
  }

  return false;
}

bool is_int128_integral_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base &&
         base->kind == Type::TK_FUNDAMENTAL &&
         (base->fundamental == FT_INT128 || base->fundamental == FT_UINT128);
}

string constexpr_floating_literal_text(long double value, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  EFundamentalType fundamental =
      base && base->kind == Type::TK_FUNDAMENTAL ? base->fundamental : FT_DOUBLE;

  long double emitted_value = value;
  if(fundamental == FT_FLOAT) {
    emitted_value = static_cast<float>(value);
  } else if(fundamental == FT_DOUBLE) {
    emitted_value = static_cast<double>(value);
  }

  ostringstream out;
  out << setprecision(20) << emitted_value;
  string text = out.str();
  if(fundamental == FT_FLOAT) {
    text += "f";
  } else if(fundamental == FT_LONG_DOUBLE) {
    text += "L";
  }
  return text;
}

bool make_constexpr_scalar_literal_node(SemanticContext & ctx,
                                        const constant_eval::ConstexprValue & value,
                                        const TypePtr & type,
                                        DumpNode & out)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || is_int128_integral_type(type)) {
    return false;
  }

  if(is_floating_type(base)) {
    constant_eval::ConstexprValue converted = value;
    if(converted.kind != constant_eval::ConstexprValue::CV_FLOATING &&
       !constant_eval::constexpr_value_cast(value, type, converted)) {
      return false;
    }
    if(converted.kind != constant_eval::ConstexprValue::CV_FLOATING) {
      return false;
    }
    out = make_dump_node(
        CallSemKind::literal,
        constexpr_floating_literal_text(converted.floating_value, type));
    out.semantic_type = type;
    out.value_category = CVC_PRVALUE;
    return true;
  }

  if(is_integral_type(base) || is_named_enum_type(ctx, base)) {
    long long constant_value = 0;
    if(!constant_eval::constexpr_value_to_integral(value, constant_value)) {
      return false;
    }
    out = make_dump_node(CallSemKind::literal, to_string(constant_value));
    set_callsem_int_value(out, constant_value);
    out.semantic_type = type;
    out.value_category = CVC_PRVALUE;
    return true;
  }

  return false;
}

bool is_indirect_class_reference_type_for_output(const TypePtr & type)
{
  return is_reference_type(type) &&
         is_complete_class_value_type_for_output(remove_reference_type(type));
}

bool is_std_qualified_name_for_output(const std::string & name)
{
  return name.compare(0, 5, "std::") == 0;
}

bool is_std_implementation_detail_name_for_output(const std::string & name)
{
  return name.compare(0, 6, "std::_") == 0 ||
         name.compare(0, 7, "std::__") == 0;
}

std::string canonical_host_runtime_rtti_name_for_output(std::string name)
{
  const std::string abi_marker = "[abi:";
  const std::size_t abi_pos = name.find(abi_marker);
  if(abi_pos != std::string::npos) {
    name.erase(abi_pos);
  }
  const std::string libcxx_prefix = "std::__1::";
  if(name.compare(0, libcxx_prefix.size(), libcxx_prefix) == 0) {
    name = std::string("std::") + name.substr(libcxx_prefix.size());
  }
  const std::string libstdcxx_cxx11_prefix = "std::__cxx11::";
  if(name.compare(0,
                  libstdcxx_cxx11_prefix.size(),
                  libstdcxx_cxx11_prefix) == 0) {
    name = std::string("std::") + name.substr(libstdcxx_cxx11_prefix.size());
  }
  return name;
}

bool is_host_runtime_rtti_name_for_output(const std::string & input_name)
{
  const std::string name = canonical_host_runtime_rtti_name_for_output(input_name);
  return name == "std::exception" ||
         name == "std::bad_exception" ||
         name == "std::bad_alloc" ||
         name == "std::bad_array_new_length" ||
         name == "std::bad_cast" ||
         name == "std::bad_typeid" ||
         name == "std::logic_error" ||
         name == "std::runtime_error" ||
         name == "std::domain_error" ||
         name == "std::invalid_argument" ||
         name == "std::length_error" ||
         name == "std::out_of_range" ||
         name == "std::range_error" ||
         name == "std::overflow_error" ||
         name == "std::underflow_error" ||
         name == "std::bad_function_call" ||
         name == "std::system_error" ||
         name == "std::ios_base::failure";
}

bool should_implicitly_move_return_object_for_output(const CallSemNode & node)
{
  return node.kind == CallSemKind::id_expression &&
         node.implicit_return_move_eligible &&
         is_complete_class_value_type_for_output(node.semantic_type);
}

const CallSemNode * find_callsem_child(const CallSemNode & node, CallSemKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

void collect_value_return_statements_for_output(const CallSemNode & node,
                                                std::vector<const CallSemNode *> & out)
{
  if(node.kind == CallSemKind::function_definition) {
    return;
  }
  if(node.kind == CallSemKind::return_statement && !node.children.empty()) {
    out.push_back(&node);
    return;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_value_return_statements_for_output(node.children[i], out);
  }
}

bool contains_other_named_local_variable_for_output(const CallSemNode & node,
                                                    const std::string & name,
                                                    const CallSemNode * ignore)
{
  if(node.kind == CallSemKind::function_definition) {
    return false;
  }
  if(node.kind == CallSemKind::variable &&
     &node != ignore &&
     node.text == name) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(contains_other_named_local_variable_for_output(node.children[i], name, ignore)) {
      return true;
    }
  }
  return false;
}

bool uses_indirect_return_boundary_for_output(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED || is_reference_type(type)) {
    return is_indirect_value_type_for_output(type);
  }

  bool direct_object = base->named_has_layout && base->named_size != 0;
  if(direct_object) {
    for(size_t i = 0; i < base->named_host_abi_chunks.size(); ++i) {
      if(base->named_host_abi_chunks[i].kind != Type::HostAbiChunk::HC_INTEGER ||
         base->named_host_abi_chunks[i].size == 0 ||
         base->named_host_abi_chunks[i].size > 8) {
        direct_object = false;
        break;
      }
    }
  }

  return !direct_object && is_indirect_value_type_for_output(type);
}

const CallSemNode * find_named_return_slot_variable_for_output(const CallSemNode & function_node,
                                                               const TypePtr & function_type)
{
  TypePtr base_function_type = strip_top_level_cv(function_type);
  if(!base_function_type || base_function_type->kind != Type::TK_FUNCTION) {
    return nullptr;
  }

  TypePtr return_type = base_function_type->inner;
  if(!return_type ||
     !uses_indirect_return_boundary_for_output(return_type) ||
     !is_complete_class_value_type_for_output(return_type)) {
    return nullptr;
  }

  const CallSemNode * body = find_callsem_child(function_node, CallSemKind::compound_statement);
  if(!body) {
    return nullptr;
  }

  std::vector<const CallSemNode *> returns;
  collect_value_return_statements_for_output(*body, returns);
  if(returns.empty()) {
    return nullptr;
  }

  std::string candidate_name;
  for(size_t i = 0; i < returns.size(); ++i) {
    const CallSemNode & expr = returns[i]->children[0];
    if(expr.kind != CallSemKind::id_expression ||
       expr.text.empty() ||
       !expr.implicit_return_move_eligible ||
       !semantic_conversion::same_type_with_compatible_top_cv(
           strip_top_level_cv(expr.semantic_type),
           strip_top_level_cv(return_type))) {
      return nullptr;
    }
    if(i == 0) {
      candidate_name = expr.text;
    } else if(expr.text != candidate_name) {
      return nullptr;
    }
  }

  const CallSemNode * candidate = nullptr;
  for(size_t i = 0; i < body->children.size(); ++i) {
    const CallSemNode & child = body->children[i];
    if(child.kind != CallSemKind::simple_declaration) {
      continue;
    }
    for(size_t j = 0; j < child.children.size(); ++j) {
      const CallSemNode & variable = child.children[j];
      if(variable.kind != CallSemKind::variable ||
         variable.text != candidate_name) {
        continue;
      }
      if(candidate != nullptr ||
         variable.is_static_storage ||
         is_reference_type(variable.semantic_type) ||
         !is_complete_class_value_type_for_output(variable.semantic_type) ||
         !semantic_conversion::same_type_with_compatible_top_cv(
             strip_top_level_cv(variable.semantic_type),
             strip_top_level_cv(return_type))) {
        return nullptr;
      }
      candidate = &variable;
    }
  }

  if(candidate == nullptr) {
    return nullptr;
  }

  if(contains_other_named_local_variable_for_output(*body, candidate_name, candidate)) {
    return nullptr;
  }

  return candidate;
}

bool return_statement_expr_materializes_directly_for_output(
    const CallSemNode * function_node,
    const TypePtr & function_type,
    const CallSemNode & expr)
{
  TypePtr base_function_type = strip_top_level_cv(function_type);
  if(!base_function_type || base_function_type->kind != Type::TK_FUNCTION) {
    return false;
  }

  TypePtr return_type = base_function_type->inner;
  if(!is_indirect_value_type_for_output(return_type)) {
    return false;
  }

  if(is_complete_class_value_type_for_output(expr.semantic_type) &&
     (expr.kind == CallSemKind::closure_object ||
      expr.kind == CallSemKind::initializer_list_object)) {
    return true;
  }
  if(expr.kind == CallSemKind::call_expression &&
     (is_indirect_value_type_for_output(expr.semantic_type) ||
      is_complete_class_value_type_for_output(expr.semantic_type))) {
    return true;
  }

  if(function_node &&
     expr.kind == CallSemKind::id_expression &&
     expr.implicit_return_move_eligible) {
    const CallSemNode * candidate =
        find_named_return_slot_variable_for_output(*function_node, function_type);
    if(candidate && candidate->text == expr.text) {
      return true;
    }
  }

  return false;
}

bool variable_initializer_expr_materializes_directly_for_output(
    const CallSemNode & variable,
    const CallSemNode & expr)
{
  if(variable.kind != CallSemKind::variable ||
     is_reference_type(variable.semantic_type) ||
     !semantic_conversion::same_type_with_compatible_top_cv(
         strip_top_level_cv(variable.semantic_type),
         strip_top_level_cv(expr.semantic_type))) {
    return false;
  }

  if(expr.kind == CallSemKind::closure_object ||
     expr.kind == CallSemKind::initializer_list_object) {
    return true;
  }
  if(expr.kind == CallSemKind::call_expression &&
     (is_indirect_value_type_for_output(expr.semantic_type) ||
      is_complete_class_value_type_for_output(expr.semantic_type))) {
    return true;
  }
  if(expr.kind == CallSemKind::statement_expression ||
     expr.kind == CallSemKind::conditional_expression) {
    return true;
  }

  return expr.kind == CallSemKind::binary_expression &&
         callsem_has_token(expr, OP_COMMA);
}

void collect_required_storage_value_to_target_support(SemanticContext & ctx,
                                                      const TypePtr & target_type,
                                                      const CallSemNode & node,
                                                      Scope * resolution_scope);

void collect_required_variable_initializer_support(SemanticContext & ctx,
                                                   const CallSemNode & node,
                                                   Scope * resolution_scope)
{
  if(node.kind != CallSemKind::variable ||
     node.children.empty() ||
     is_reference_type(node.semantic_type)) {
    return;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CallSemNode & child = node.children[i];
    if(child.kind == CallSemKind::constructor_action ||
       child.kind == CallSemKind::destructor_action ||
       child.kind == CallSemKind::expression_statement) {
      continue;
    }
    if(variable_initializer_expr_materializes_directly_for_output(node, child)) {
      continue;
    }
    collect_required_storage_value_to_target_support(ctx,
                                                     node.semantic_type,
                                                     child,
                                                     resolution_scope);
  }
}

void collect_required_return_statement_support(SemanticContext & ctx,
                                               const FunctionBinding & function,
                                               const CallSemNode & node,
                                               const CallSemNode * function_node)
{
  if(node.kind != CallSemKind::return_statement || node.children.empty()) {
    return;
  }

  TypePtr function_type = strip_top_level_cv(function.type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return;
  }

  TypePtr return_type = function_type->inner;
  if(!is_indirect_value_type_for_output(return_type)) {
    return;
  }

  const CallSemNode & expr = node.children[0];
  if(return_statement_expr_materializes_directly_for_output(function_node,
                                                            function.type,
                                                            expr)) {
    return;
  }

  ClassInfo * info = nullptr;
  FunctionBinding * ctor = nullptr;
  if(is_indirect_class_reference_type_for_output(expr.semantic_type)) {
    info = ctx.complete_class_type(return_type);
    if(info && expr.value_category == CVC_XVALUE) {
      ctor = find_or_ensure_move_constructor_binding(ctx, *info);
    }
    if(!ctor) {
      ctor = info ? find_or_ensure_copy_constructor_binding(ctx, *info) : nullptr;
    }
  } else if(is_complete_class_value_type_for_output(expr.semantic_type)) {
    info = ctx.complete_class_type(expr.semantic_type);
    if(should_implicitly_move_return_object_for_output(expr)) {
      ctor = info ? find_or_ensure_move_constructor_binding(ctx, *info) : nullptr;
    }
    if(!ctor) {
      ctor = info ? find_or_ensure_copy_constructor_binding(ctx, *info) : nullptr;
    }
  }
  require_constructor_definition_if_needed(ctx,
                                           ctor,
                                           OutputReason::SyntheticDependency);
}

void collect_required_exception_runtime_support(SemanticContext & ctx,
                                                const CallSemNode & node)
{
  if(node.kind == CallSemKind::throw_statement && !node.children.empty()) {
    const TypePtr throw_type = exception_object_type_for_output(node.children[0].semantic_type);
    ClassInfo * info = ctx.complete_class_type(throw_type);
    if(!info || !info->complete) {
      return;
    }
    require_nontrivial_destructor_definition(ctx, *info);
    const CallSemNode & expr = node.children[0];
    const bool requires_runtime_copy =
        expr.value_category == CVC_LVALUE ||
        (strip_top_level_cv(expr.semantic_type) &&
         is_reference_type(strip_top_level_cv(expr.semantic_type)));
    if(requires_runtime_copy) {
      require_constructor_definition_if_needed(
          ctx,
          find_or_ensure_copy_constructor_binding(ctx, *info),
          OutputReason::SyntheticDependency);
    }
    return;
  }

  if(node.kind != CallSemKind::catch_handler) {
    return;
  }

  TypePtr catch_type = strip_top_level_cv(node.semantic_type);
  if(!catch_type || is_reference_type(catch_type)) {
    return;
  }
  ClassInfo * info = ctx.complete_class_type(catch_type);
  if(!info || !info->complete) {
    return;
  }
  require_constructor_definition_if_needed(
      ctx,
      find_or_ensure_copy_constructor_binding(ctx, *info),
      OutputReason::SyntheticDependency);
  require_nontrivial_destructor_definition(ctx, *info);
}

bool is_discarded_class_prvalue_materialization(const CallSemNode & node)
{
  if(node.kind != CallSemKind::call_expression &&
     node.kind != CallSemKind::closure_object &&
     node.kind != CallSemKind::initializer_list_object) {
    return false;
  }
  TypePtr type = strip_top_level_cv(node.semantic_type);
  return type && !is_reference_type(type);
}

bool is_full_expression_class_temporary_materialization(const CallSemNode & node)
{
  if(!is_complete_class_value_type_for_output(node.semantic_type) ||
     node.value_category == CVC_LVALUE ||
     is_reference_type(node.semantic_type)) {
    return false;
  }

  if(node.kind == CallSemKind::call_expression ||
     node.kind == CallSemKind::closure_object ||
     node.kind == CallSemKind::initializer_list_object ||
     node.kind == CallSemKind::statement_expression ||
     node.kind == CallSemKind::conditional_expression) {
    return true;
  }

  return node.kind == CallSemKind::binary_expression &&
         callsem_has_token(node, OP_COMMA);
}

bool hidden_class_target_prefers_move_constructor_for_output(
    const TypePtr & target_type,
    const TypePtr & source_type,
    CallValueCategory source_value_category)
{
  if(source_value_category == CVC_LVALUE) {
    return false;
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
  TypePtr source_base = strip_top_level_cv(remove_reference_type(source_type));
  if(!target_base || !source_base) {
    return false;
  }

  return semantic_conversion::same_type_with_compatible_top_cv(target_base, source_base);
}

void require_hidden_class_transfer_constructor_for_output(SemanticContext & ctx,
                                                          const TypePtr & target_type,
                                                          const CallSemNode * source_node = nullptr,
                                                          Scope * resolution_scope = nullptr)
{
  TypePtr effective_target_type =
      resolve_output_support_type(ctx, resolution_scope, target_type);
  TypePtr effective_source_type =
      source_node ?
          resolve_output_support_type(ctx, resolution_scope, source_node->semantic_type) :
          TypePtr();

  if(ctx.type_depends_on_template_parameter(effective_target_type) ||
     (effective_source_type &&
      ctx.type_depends_on_template_parameter(effective_source_type))) {
    return;
  }

  ClassInfo * info = ctx.complete_class_type(effective_target_type);
  if(!info || !info->complete) {
    return;
  }

  FunctionBinding * ctor = nullptr;
  const char * action = "require-hidden-copy-support";
  if(source_node &&
     hidden_class_target_prefers_move_constructor_for_output(effective_target_type,
                                                            effective_source_type,
                                                            source_node->value_category)) {
    FunctionBinding * move = find_or_ensure_move_constructor_binding(ctx, *info);
    if(move && !move->is_deleted) {
      ctor = move;
      action = "require-hidden-move-support";
    }
  }
  if(!ctor) {
    FunctionBinding * copy = find_or_ensure_copy_constructor_binding(ctx, *info);
    if(copy && !copy->is_deleted) {
      ctor = copy;
    }
  }
  if(ctor) {
    if(source_node && parser_trace::enabled("output.require")) {
      ostringstream trace;
      trace << "action=" << action
            << " class=" << info->qualified_name
            << " symbol=" << ctor->symbol.internal_symbol
            << " target-type=" << describe_type(target_type)
            << " effective-target-type=" << describe_type(effective_target_type)
            << " node-kind=" << callsem_kind_text(source_node->kind)
            << " node-type="
            << (source_node->semantic_type ? describe_type(source_node->semantic_type)
                                           : string("<none>"))
            << " effective-node-type="
            << (effective_source_type ? describe_type(effective_source_type) :
                                      string("<none>"))
            << " node-vc=" << static_cast<int>(source_node->value_category)
            << " child-count=" << source_node->children.size();
      parser_trace::note("output.require", string(), trace.str());
    }
    require_constructor_definition_if_needed(ctx,
                                             ctor,
                                             OutputReason::SyntheticDependency);
  }
}

void collect_required_storage_value_to_target_support(SemanticContext & ctx,
                                                      const TypePtr & target_type,
                                                      const CallSemNode & node,
                                                      Scope * resolution_scope)
{
  const TypePtr effective_target_type =
      resolve_output_support_type(ctx, resolution_scope, target_type);
  const TypePtr effective_node_type =
      resolve_output_support_type(ctx, resolution_scope, node.semantic_type);

  if(node.kind == CallSemKind::braced_init_list) {
    TypePtr target_base = strip_top_level_cv(effective_target_type);
    if(target_base && target_base->kind == Type::TK_ARRAY) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        collect_required_storage_value_to_target_support(ctx,
                                                         target_base->inner,
                                                         node.children[i],
                                                         resolution_scope);
      }
      return;
    }
  }

  if(node.kind == CallSemKind::statement_expression) {
    if(node.children.size() == 2) {
      collect_required_storage_value_to_target_support(ctx,
                                                       effective_target_type,
                                                       node.children[1],
                                                       resolution_scope);
    }
    return;
  }

  if(node.kind == CallSemKind::binary_expression &&
     callsem_has_token(node, OP_COMMA)) {
    if(node.children.size() == 2) {
      collect_required_storage_value_to_target_support(ctx,
                                                       effective_target_type,
                                                       node.children[1],
                                                       resolution_scope);
    }
    return;
  }

  if(node.kind == CallSemKind::conditional_expression) {
    if(node.children.size() == 3) {
      collect_required_storage_value_to_target_support(ctx,
                                                       effective_target_type,
                                                       node.children[1],
                                                       resolution_scope);
      collect_required_storage_value_to_target_support(ctx,
                                                       effective_target_type,
                                                       node.children[2],
                                                       resolution_scope);
    }
    return;
  }

  if(is_reference_type(effective_target_type)) {
    if(node.value_category != CVC_LVALUE &&
       !is_reference_type(effective_node_type)) {
      ClassInfo * info =
          complete_class_object_type_for_output(ctx, effective_node_type);
      if(info) {
        require_temporary_destructor_definition(ctx, *info);
      }
    }
    return;
  }

  if(node.kind == CallSemKind::closure_object ||
     node.kind == CallSemKind::initializer_list_object) {
    return;
  }

  if(node.kind == CallSemKind::call_expression &&
     (is_indirect_value_type_for_output(effective_node_type) ||
      is_complete_class_value_type_for_output(effective_node_type))) {
    return;
  }

  require_hidden_class_transfer_constructor_for_output(ctx,
                                                       effective_target_type,
                                                       &node,
                                                       resolution_scope);
}

FunctionBinding * find_direct_call_target_binding(SemanticContext & ctx,
                                                  const CallSemNode & node);

void collect_required_call_argument_materialization_support(SemanticContext & ctx,
                                                            const CallSemNode & node,
                                                            Scope * resolution_scope)
{
  if(node.kind != CallSemKind::call_expression || node.children.size() < 2) {
    return;
  }

  FunctionBinding * binding = find_direct_call_target_binding(ctx, node.children[0]);
  const size_t arg_count = node.children.size() - 1;
  Scope * parameter_resolution_scope = resolution_scope;
  std::vector<TypePtr> indirect_param_types;
  const std::vector<std::pair<std::string, TypePtr> > * binding_params = nullptr;
  if(binding) {
    binding_params = &binding->params;
    if(Scope * binding_scope = function_output_resolution_scope(binding)) {
      parameter_resolution_scope = binding_scope;
    }
  } else {
    TypePtr target_type = strip_top_level_cv(remove_reference_type(node.children[0].semantic_type));
    if(target_type && target_type->kind == Type::TK_POINTER) {
      target_type = strip_top_level_cv(target_type->inner);
    }
    if(!target_type || target_type->kind != Type::TK_FUNCTION) {
      return;
    }
    indirect_param_types = target_type->params;
  }

  const size_t param_count =
      binding_params ? binding_params->size() : indirect_param_types.size();
  const size_t count = min(arg_count, param_count);
  for(size_t i = 0; i < count; ++i) {
    const TypePtr & param_type =
        binding_params ? (*binding_params)[i].second : indirect_param_types[i];
    collect_required_storage_value_to_target_support(ctx,
                                                     param_type,
                                                     node.children[i + 1],
                                                     parameter_resolution_scope);
  }
}

void collect_required_parameter_materialization_support(SemanticContext & ctx,
                                                        const FunctionBinding & binding)
{
  for(size_t i = 0; i < binding.params.size(); ++i) {
    const TypePtr & param_type = binding.params[i].second;
    TypePtr param_base = strip_top_level_cv(param_type);
    if(is_reference_type(param_type) ||
       !is_complete_class_value_type_for_output(param_type) ||
       (param_base && param_base->kind == Type::TK_NAMED && param_base->named_is_empty)) {
      continue;
    }

    ClassInfo * info = ctx.complete_class_type(param_type);
    if(!info || !info->complete) {
      continue;
    }

    if(FunctionBinding * dtor = find_destructor_binding(*info)) {
      if(!special_member_binding_has_trivial_lifecycle_output(ctx, *dtor)) {
        ctx.require_function_definition(dtor, OutputReason::SyntheticDependency);
      }
    }
  }
}

void collect_required_new_expression_argument_materialization_support(
    SemanticContext & ctx,
    const CallSemNode & node,
    Scope * resolution_scope)
{
  if(node.kind != CallSemKind::new_expression ||
     node.children.size() < 3 ||
     node.children[1].kind != CallSemKind::callee) {
    return;
  }

  TypePtr function_type = strip_top_level_cv(node.children[1].semantic_type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return;
  }

  for(size_t i = 2; i < node.children.size(); ++i) {
    const size_t param_index = i - 1;
    if(param_index >= function_type->params.size()) {
      break;
    }
    collect_required_storage_value_to_target_support(ctx,
                                                     function_type->params[param_index],
                                                     node.children[i],
                                                     resolution_scope);
  }
}

void collect_required_special_class_materialization_support(SemanticContext & ctx,
                                                            const CallSemNode & node,
                                                            Scope * resolution_scope)
{
  TypePtr effective_node_type =
      resolve_output_support_type(ctx, resolution_scope, node.semantic_type);
  if(!is_complete_class_value_type_for_output(effective_node_type)) {
    return;
  }
  if(node.value_category == CVC_LVALUE || is_reference_type(effective_node_type)) {
    return;
  }

  if(node.kind != CallSemKind::statement_expression &&
     node.kind != CallSemKind::conditional_expression &&
     !(node.kind == CallSemKind::binary_expression &&
       callsem_has_token(node, OP_COMMA))) {
    return;
  }

  collect_required_storage_value_to_target_support(ctx,
                                                   effective_node_type,
                                                   node,
                                                   resolution_scope);
}

void collect_required_full_expression_temporary_support(SemanticContext & ctx,
                                                        const CallSemNode & node,
                                                        const FunctionBinding * active_function,
                                                        const CallSemNode * parent,
                                                        const CallSemNode * function_node,
                                                        Scope * resolution_scope)
{
  TypePtr effective_node_type =
      resolve_output_support_type(ctx, resolution_scope, node.semantic_type);
  if(!is_full_expression_class_temporary_materialization(node)) {
    return;
  }
  TypePtr function_type;
  if(active_function) {
    function_type = active_function->type;
  } else if(function_node) {
    function_type = function_node->semantic_type;
  }
  if(function_type &&
     parent &&
     parent->kind == CallSemKind::return_statement &&
     !parent->children.empty() &&
     &parent->children[0] == &node &&
     return_statement_expr_materializes_directly_for_output(function_node,
                                                            function_type,
                                                            node)) {
    return;
  }
  if(parent &&
     parent->kind == CallSemKind::variable &&
     variable_initializer_expr_materializes_directly_for_output(*parent, node)) {
    return;
  }

  ClassInfo * info = ctx.complete_class_type(effective_node_type);
  if(!info || !info->complete) {
    return;
  }
  require_temporary_destructor_definition(ctx, *info);
}

bool ast_subtree_contains_kind(const CppAstNode * node, CppAstKind kind)
{
  if(!node) {
    return false;
  }
  if(node->kind == kind) {
    return true;
  }
  for(size_t i = 0; i < node->children.size(); ++i) {
    if(ast_subtree_contains_kind(&node->children[i], kind)) {
      return true;
    }
  }
  return false;
}

void collect_required_discarded_temporary_support(SemanticContext & ctx,
                                                  const CallSemNode & node,
                                                  Scope * resolution_scope)
{
  TypePtr effective_node_type =
      resolve_output_support_type(ctx, resolution_scope, node.semantic_type);
  if(!is_discarded_class_prvalue_materialization(node)) {
    return;
  }

  ClassInfo * info = ctx.complete_class_type(effective_node_type);
  if(!info || !info->complete) {
    return;
  }
  require_temporary_destructor_definition(ctx, *info);
}

void collect_required_constructor_unwind_support(SemanticContext & ctx,
                                                 const FunctionBinding & active_function,
                                                 const CallSemNode & node)
{
  if(node.kind != CallSemKind::constructor_action ||
     node.children.size() != 1 ||
     !active_function.is_constructor ||
     !ast_subtree_contains_kind(active_function.body, CppAstKind::throw_statement)) {
    return;
  }

  const CallSemNode & call = node.children[0];
  if(call.kind != CallSemKind::call_expression || call.children.size() < 2) {
    return;
  }

  const CallSemNode & target_arg = call.children[1];
  TypePtr target_ptr_type = strip_top_level_cv(target_arg.semantic_type);
  if(!target_ptr_type || target_ptr_type->kind != Type::TK_POINTER || !target_ptr_type->inner) {
    return;
  }

  TypePtr target_type = strip_top_level_cv(target_ptr_type->inner);
  ClassInfo * info = ctx.complete_class_type(target_type);
  if(!info || !info->complete) {
    return;
  }

  require_nontrivial_destructor_definition(ctx, *info);
}

void collect_required_delete_expression_support(SemanticContext & ctx,
                                                const CallSemNode & node)
{
  if(node.kind != CallSemKind::call_expression ||
     !callsem_has_token(node, KW_DELETE) ||
     node.children.size() < 2) {
    return;
  }

  TypePtr pointer_type =
      strip_top_level_cv(remove_reference_type(node.children[1].semantic_type));
  if(!pointer_type || pointer_type->kind != Type::TK_POINTER) {
    return;
  }

  ClassInfo * info = ctx.complete_class_type(pointer_type->inner);
  if(!info || !info->complete) {
    return;
  }
  require_nontrivial_destructor_definition(ctx, *info);
}

std::size_t count_required_callee_scan_nodes(const CallSemNode & node)
{
  std::size_t out = 1;
  if(node.kind == CallSemKind::if_statement) {
    const CallSemNode * condition = nullptr;
    const CallSemNode * then_node = nullptr;
    const CallSemNode * else_node = nullptr;
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemNode & child = node.children[i];
      if(child.kind == CallSemKind::condition) {
        condition = &child;
      } else if(child.kind == CallSemKind::then_node) {
        then_node = &child;
      } else if(child.kind == CallSemKind::else_node) {
        else_node = &child;
      } else {
        out += count_required_callee_scan_nodes(child);
      }
    }
    if(condition) {
      out += count_required_callee_scan_nodes(*condition);
    }
    if(then_node) {
      out += count_required_callee_scan_nodes(*then_node);
    }
    if(else_node) {
      out += count_required_callee_scan_nodes(*else_node);
    }
    return out;
  }
  vector<const CallSemNode *> children;
  append_callsem_recursive_input_children(node, children);
  for(size_t i = 0; i < children.size(); ++i) {
    out += count_required_callee_scan_nodes(*children[i]);
  }
  return out;
}

void note_required_callee_rescan(SemanticContext & ctx, const CallSemNode & node)
{
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    counters->rescanned_emitted_nodes += count_required_callee_scan_nodes(node);
  }
}

FunctionBinding * find_direct_call_target_binding(SemanticContext & ctx,
                                                  const CallSemNode & node)
{
  if(!node.semantic_type) {
    return nullptr;
  }

  if(node.kind == CallSemKind::callee) {
    if(FunctionBinding * binding =
           ctx.find_function_by_symbol(callsem_symbol(node), node.text, node.semantic_type)) {
      return binding;
    }
  }

  if(node.kind != CallSemKind::id_expression &&
     node.kind != CallSemKind::member_expression &&
     node.kind != CallSemKind::callee) {
    return nullptr;
  }

  if(callsem_symbol(node).internal_symbol.empty()) {
    return nullptr;
  }

  TypePtr function_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return nullptr;
  }

  FunctionBinding * binding =
      ctx.find_function_by_symbol(callsem_symbol(node), node.text, node.semantic_type);
  if(binding) {
    return binding;
  }
  return ctx.find_function_by_symbol(callsem_symbol(node), node.text, function_type);
}

FunctionBinding * find_function_address_target_binding(SemanticContext & ctx,
                                                       const CallSemNode & node)
{
  if(node.kind != CallSemKind::unary_expression ||
     !callsem_has_token(node, OP_AMP) ||
     node.children.size() != 1) {
    return nullptr;
  }

  const CallSemNode & operand = node.children[0];
  TypePtr function_type = strip_top_level_cv(remove_reference_type(operand.semantic_type));
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return nullptr;
  }

  if(!callsem_symbol(node).internal_symbol.empty()) {
    if(FunctionBinding * binding =
           ctx.find_function_by_symbol(callsem_symbol(node), operand.text, operand.semantic_type)) {
      return binding;
    }
    if(FunctionBinding * binding =
           ctx.find_function_by_symbol(callsem_symbol(node), operand.text, function_type)) {
      return binding;
    }
  }

  return find_direct_call_target_binding(ctx, operand);
}

void collect_required_callees_from_node(SemanticContext & ctx,
                                        const CallSemNode & node,
                                        const FunctionBinding * active_function = nullptr,
                                        const CallSemNode * parent = nullptr,
                                        const CallSemNode * function_node = nullptr)
{
  if(!active_function && node.kind == CallSemKind::function_definition) {
    return;
  }
  if(node.kind == CallSemKind::function_definition) {
    function_node = &node;
  }
  if(!active_function && node.kind == CallSemKind::function_definition && node.semantic_type) {
    active_function = ctx.find_function_by_symbol(callsem_symbol(node), node.text, node.semantic_type);
    if(!active_function) {
      TypePtr function_type = strip_top_level_cv(node.semantic_type);
      active_function = ctx.find_function_by_symbol(callsem_symbol(node), node.text, function_type);
    }
  }
  if(active_function) {
    if(node.kind == CallSemKind::function_definition) {
      collect_required_parameter_materialization_support(ctx, *active_function);
    }
    collect_required_return_statement_support(ctx, *active_function, node, function_node);
    collect_required_constructor_unwind_support(ctx, *active_function, node);
  }
  Scope * output_resolution_scope = function_output_resolution_scope(active_function);
  collect_required_special_class_materialization_support(ctx, node, output_resolution_scope);
  collect_required_full_expression_temporary_support(
      ctx, node, active_function, parent, function_node, output_resolution_scope);
  collect_required_variable_initializer_support(ctx, node, output_resolution_scope);
  collect_required_exception_runtime_support(ctx, node);
  collect_required_delete_expression_support(ctx, node);
  if(node.kind == CallSemKind::unary_expression &&
     callsem_has_token(node, OP_AMP)) {
    FunctionBinding * binding = find_function_address_target_binding(ctx, node);
    ctx.require_function_definition(binding, OutputReason::FunctionIdUse);
  }
  if(node.kind == CallSemKind::expression_statement && !node.children.empty()) {
    collect_required_discarded_temporary_support(ctx,
                                                 node.children[0],
                                                 output_resolution_scope);
  }
  if(node.kind == CallSemKind::if_statement) {
    const CallSemNode * condition = nullptr;
    const CallSemNode * then_node = nullptr;
    const CallSemNode * else_node = nullptr;
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemNode & child = node.children[i];
      if(child.kind == CallSemKind::condition) {
        condition = &child;
      } else if(child.kind == CallSemKind::then_node) {
        then_node = &child;
      } else if(child.kind == CallSemKind::else_node) {
        else_node = &child;
      } else {
        collect_required_callees_from_node(ctx, child, active_function, &node, function_node);
      }
    }
    if(condition) {
      collect_required_callees_from_node(ctx, *condition, active_function, &node, function_node);
    }
    if(then_node) {
      collect_required_callees_from_node(ctx, *then_node, active_function, &node, function_node);
    }
    if(else_node) {
      collect_required_callees_from_node(ctx, *else_node, active_function, &node, function_node);
    }
    return;
  }
  if(node.kind == CallSemKind::call_expression && !node.children.empty()) {
    const CallSemNode & target = node.children[0];
    FunctionBinding * binding = find_direct_call_target_binding(ctx, target);
    const string active_runtime_bridge =
        active_function ?
            runtime_bridge_symbol_for_object_symbol(active_function->symbol.object_symbol) :
            "";
    if(callsem_runtime_bridge_symbol(target).empty() ||
       callsem_runtime_bridge_symbol(target) == active_runtime_bridge) {
      // Treat every emitted callee as a definition seed and let the centralized
      // callable-emission planner decide whether it upgrades to a real emitted
      // definition or a declaration-only/runtime dependency.
      ctx.require_function_definition(binding, OutputReason::DirectCall);
    }
    collect_required_call_argument_materialization_support(ctx,
                                                           node,
                                                           output_resolution_scope);
  }
  if(node.kind == CallSemKind::new_expression && node.children.size() >= 2) {
    const CallSemNode & target = node.children[1];
    FunctionBinding * binding = find_direct_call_target_binding(ctx, target);
    ctx.require_function_definition(binding, OutputReason::NewExpression);
    collect_required_new_expression_argument_materialization_support(ctx,
                                                                     node,
                                                                     output_resolution_scope);
  }
  vector<const CallSemNode *> children;
  append_callsem_recursive_input_children(node, children);
  for(size_t i = 0; i < children.size(); ++i) {
    collect_required_callees_from_node(
        ctx, *children[i], active_function, &node, function_node);
  }
}

void note_witness_body_constructor_closures(SemanticContext & ctx,
                                            const CallSemNode & node)
{
  if(node.kind == CallSemKind::call_expression && !node.children.empty()) {
    FunctionBinding * binding = find_direct_call_target_binding(ctx, node.children[0]);
    if(binding && binding->is_constructor) {
      semantic_lifetime::note_constructor_witness_closure(ctx, binding);
    }
  }

  std::vector<const CallSemNode *> children;
  append_callsem_recursive_input_children(node, children);
  for(std::size_t i = 0; i < children.size(); ++i) {
    note_witness_body_constructor_closures(ctx, *children[i]);
  }
}

void emit_embedded_class_specifier_output(SemanticContext & ctx,
                                          OutputState & state,
                                          Scope & scope,
                                          const CppAstNode & specifiers,
                                          DumpNode & out)
{
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.kind != CppAstKind::class_specifier &&
       child.kind != CppAstKind::class_forward_declaration) {
      continue;
    }
    if(child.value.empty()) {
      continue;
    }
    ClassInfo * info = lookup_declared_class_info(ctx, scope, child.value);
    if(!info) {
      continue;
    }
    const CppAstNode * emit_node =
        info->template_output_node ? info->template_output_node : info->class_node;
    if(!emit_node) {
      continue;
    }
    analyze_class_output_from_info(ctx, state, *info, *emit_node, out);
  }
}

const CppAstNode * function_binding_decl_specifiers(const FunctionBinding & binding)
{
  const CppAstNode * source = binding.definition_node ? binding.definition_node : binding.declaration_node;
  return source ? find_child_kind(*source, CppAstKind::decl_specifier_seq) : nullptr;
}

bool function_binding_is_inline(const FunctionBinding & binding)
{
  if(binding.is_inline) {
    return true;
  }
  const CppAstNode * specifiers = function_binding_decl_specifiers(binding);
  if(specifiers && decl_spec_contains_token(*specifiers, KW_INLINE)) {
    return true;
  }
  return binding.declaration_node != binding.definition_node &&
         node_decl_spec_contains_token(binding.declaration_node, KW_INLINE);
}

bool node_decl_spec_contains_token(const CppAstNode * node,
                                   ETokenType token)
{
  if(!node) {
    return false;
  }
  const CppAstNode * specifiers =
      node->kind == CppAstKind::decl_specifier_seq ||
              node->kind == CppAstKind::member_specifiers ?
          node :
          find_child_kind(*node, CppAstKind::decl_specifier_seq);
  if(!specifiers) {
    specifiers = find_child_kind(*node, CppAstKind::member_specifiers);
  }
  return specifiers && decl_spec_contains_token(*specifiers, token);
}

bool binding_defines_inline_like_class_member(const FunctionBinding & binding)
{
  return binding.owner_class &&
         !node_decl_spec_contains_token(binding.declaration_node, KW_FRIEND) &&
         (binding.body != nullptr || binding.is_defaulted) &&
         binding.declaration_node &&
         binding.definition_node &&
         binding.declaration_node == binding.definition_node;
}

bool binding_defines_inline_like_friend(const FunctionBinding & binding)
{
  return binding.lexical_access_class &&
         node_decl_spec_contains_token(binding.declaration_node, KW_FRIEND) &&
         (binding.body != nullptr || binding.is_defaulted) &&
         binding.declaration_node &&
         binding.definition_node &&
         binding.declaration_node == binding.definition_node;
}

bool binding_has_out_of_class_member_definition(const FunctionBinding & binding)
{
  return binding.owner_class &&
         binding.has_definition &&
         binding.definition_node &&
         binding.definition_node != binding.declaration_node;
}

bool scope_has_internal_namespace_linkage(const Scope * scope)
{
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope && current->name == "<unnamed>") {
      return true;
    }
  }
  return false;
}

symbol_linkage::SymbolLinkage output_function_symbol_linkage(const FunctionBinding & binding)
{
  const CppAstNode * declaration =
      binding.definition_node ? binding.definition_node : binding.declaration_node;
  const bool explicit_specialization_binding =
      binding.is_explicit_specialization ||
      template_api::class_is_explicit_specialization(binding.owner_class);
  const bool linkage_template_identity =
      template_api::function_binding_has_linkage_template_identity(&binding);
  if(scope_has_internal_namespace_linkage(binding.declaration_scope) ||
     (!binding.owner_class &&
      node_decl_spec_contains_token(declaration, KW_STATIC))) {
    return symbol_linkage::SL_INTERNAL;
  }
  if((binding.synthesized && symbol_linkage::has_weak_linkage(binding.symbol)) ||
     (binding.odr_mergeable_definition &&
      (linkage_template_identity || !explicit_specialization_binding)) ||
     binding_defines_inline_like_class_member(binding) ||
     binding_defines_inline_like_friend(binding) ||
     binding.is_inline ||
     binding.is_constexpr ||
     node_decl_spec_contains_token(declaration, KW_INLINE) ||
     node_decl_spec_contains_token(declaration, KW_CONSTEXPR)) {
    return symbol_linkage::SL_WEAK;
  }
  if(linkage_template_identity) {
    return symbol_linkage::SL_WEAK;
  }
  if(binding.is_c_linkage) {
    return symbol_linkage::SL_EXTERNAL;
  }
  if(explicit_specialization_binding) {
    return symbol_linkage::SL_EXTERNAL;
  }
  return binding.symbol.linkage;
}

enum VTableKeyFunctionKind
{
  VTK_NONE,
  VTK_EXTERNAL,
  VTK_LOCAL
};

struct VTableKeyFunctionDecision
{
  VTableKeyFunctionDecision()
      : kind(VTK_NONE)
  {
  }

  VTableKeyFunctionKind kind;
  string name;
};

VTableKeyFunctionDecision vtable_key_function_decision(
    SemanticContext & ctx,
    ClassInfo & info,
    const ClassOutputReadiness & class_output_readiness)
{
  VTableKeyFunctionDecision decision;
  if(class_output_readiness.templated_context ||
     !symbol_linkage::has_external_vtable_symbol_candidate(info.type)) {
    return decision;
  }

  for(std::size_t table_index = 0; table_index < info.vtables.size(); ++table_index) {
    const VTableInfo & table = info.vtables[table_index];
    for(std::size_t i = 0; i < table.slots.size(); ++i) {
      FunctionBinding * binding =
          semantic_template_function::acquire_required_function_definition_binding(
              ctx, table.slots[i].function, *info.member_scope);
      binding = resolve_output_function_binding(ctx, binding);
      if(!binding ||
         !binding->is_virtual ||
         binding->owner_class != &info ||
         binding->is_pure_virtual ||
         binding->is_deleted ||
         template_api::function_binding_has_source_template_identity(binding) ||
         output_function_symbol_linkage(*binding) != symbol_linkage::SL_EXTERNAL) {
        continue;
      }

      decision.name = binding->name;
      const CppAstNode * definition_node =
          binding->definition_node ? binding->definition_node : binding->declaration_node;
      if(binding_has_out_of_class_member_definition(*binding) &&
         !ctx.definition_comes_from_standard_include_path(definition_node,
                                                          binding->body,
                                                          binding->is_defaulted)) {
        decision.kind = VTK_LOCAL;
      } else {
        decision.kind = VTK_EXTERNAL;
      }
      return decision;
    }
  }

  return decision;
}

symbol_linkage::SymbolLinkage output_variable_symbol_linkage(const ValueBinding & binding)
{
  if(binding.is_c_linkage) {
    return symbol_linkage::SL_EXTERNAL;
  }
  if(scope_has_internal_namespace_linkage(binding.declaration_scope)) {
    return symbol_linkage::SL_INTERNAL;
  }
  if(binding.owner_class &&
     (!binding.has_storage_definition ||
      static_member_variable_definition_output_suppressed(binding))) {
    return symbol_linkage::SL_EXTERNAL;
  }
  if(template_api::value_or_owner_has_template_identity(&binding)) {
    return symbol_linkage::SL_WEAK;
  }
  const CppAstNode * declaration =
      binding.definition_node ? binding.definition_node : binding.declaration_node;
  if(binding.owner_class &&
     (node_decl_spec_contains_token(declaration, KW_INLINE) ||
      node_decl_spec_contains_token(declaration, KW_CONSTEXPR))) {
    return symbol_linkage::SL_WEAK;
  }
  if(!binding.owner_class &&
     node_decl_spec_contains_token(declaration, KW_INLINE)) {
    return symbol_linkage::SL_WEAK;
  }
  return binding.symbol.linkage;
}

symbol_linkage::SymbolIdentity static_member_variable_output_symbol_identity(
    const ClassInfo & info,
    const ValueBinding & binding)
{
  if(binding.variable_template_instantiation &&
     binding.variable_template_instantiation->source_template) {
    const VariableTemplateDecl & source_template =
        *binding.variable_template_instantiation->source_template;
    return symbol_linkage::make_static_member_variable_template_symbol_identity(
        info,
        binding.name,
        source_template.name,
        binding.variable_template_instantiation->arguments,
        source_template.parameters,
        binding.is_c_linkage,
        output_variable_symbol_linkage(binding));
  }
  return symbol_linkage::make_static_member_variable_symbol_identity(
      info,
      binding.name,
      binding.is_c_linkage,
      output_variable_symbol_linkage(binding));
}

bool is_unrequired_constexpr_static_member_definition(const ValueBinding & binding)
{
  return binding.owner_class &&
         binding.kind == ValueBinding::VK_VARIABLE &&
         !has_output_requirement(binding.output_requirements, ORK_DEFINITION) &&
         (binding.requires_constant_initializer ||
          node_decl_spec_contains_token(binding.declaration_node, KW_CONSTEXPR) ||
          node_decl_spec_contains_token(binding.definition_node, KW_CONSTEXPR));
}

bool is_static_const_integral_member_with_initializer(SemanticContext & ctx,
                                                      const ValueBinding & binding)
{
  if(!binding.owner_class ||
     binding.kind != ValueBinding::VK_VARIABLE ||
     !binding.constant_initializer ||
     !type_is_const_object(binding.type)) {
    return false;
  }

  TypePtr base = strip_top_level_cv(remove_reference_type(binding.type));
  if(base && is_integral_type(base)) {
    return true;
  }
  if(base && base->kind == Type::TK_NAMED) {
    ClassInfo * info = ctx.class_info_for_type(base);
    return info && info->class_kind == "enum";
  }
  return false;
}

void analyze_required_class_static_member_output(SemanticContext & ctx,
                                                 OutputState & state,
                                                 ClassInfo & info,
                                                 DumpNode & out)
{
  (void)state;
  if(!template_api::class_has_template_identity(&info)) {
    info.has_late_required_static_member_output = false;
    info.late_required_static_member_output_queued = false;
    return;
  }

  for(map<string, ValueBinding>::iterator it = info.member_scope->values.begin();
      it != info.member_scope->values.end();
      ++it) {
    ValueBinding & binding = it->second;
    const bool source_capture_header_static_member_output =
        info.source_capture_header_instantiation_tracked &&
        witness::source_capture_enabled(ctx.template_witness_context()) &&
        !has_output_requirement(binding.output_requirements, ORK_DEFINITION);
    const bool witness_only_unrequired_integral_constant =
        witness::source_capture_enabled(ctx.template_witness_context()) &&
        !class_has_required_member_output(info) &&
        ((binding.name == "value" &&
          !binding.witness_static_member_definition_source_captured) ||
         (binding.name != "value" &&
          !binding.witness_member_value_instantiation_noted)) &&
        !binding.is_explicit_specialization &&
        !template_api::class_is_explicit_specialization(&info) &&
        !has_output_requirement(binding.output_requirements, ORK_DEFINITION) &&
        is_static_const_integral_member_with_initializer(ctx, binding);
    if(binding.kind != ValueBinding::VK_VARIABLE ||
       binding.owner_class != &info ||
       !binding.definition_node ||
       binding.definition_output_emitted ||
       static_member_variable_definition_output_suppressed(binding) ||
       source_capture_header_static_member_output ||
       witness_only_unrequired_integral_constant ||
       is_unrequired_constexpr_static_member_definition(binding)) {
      if(source_capture_header_static_member_output &&
         binding.kind == ValueBinding::VK_VARIABLE &&
         binding.owner_class == &info &&
         binding.definition_node &&
         !binding.definition_output_emitted) {
        template_instantiation::replay_witness_static_member_definition_if_needed(
            ctx,
            binding,
            &info);
      }
      continue;
    }

    const string qualified_name = scope_symbol_qualified_name(*info.member_scope, binding.name);
    DumpNode var_node = make_dump_node(CallSemKind::variable, qualified_name);
    var_node.semantic_type = binding.type;
    var_node.is_extern_declaration = !binding.has_storage_definition;
    var_node.is_thread_local = binding.is_thread_local;
    var_node.is_static_storage = binding.is_thread_local;
    if(info.member_scope) {
      set_callsem_qualified_name_syntax(
          var_node,
          scope_symbol_qualified_name_syntax(*info.member_scope, binding.name));
    }
    symbol_linkage::SymbolIdentity symbol = binding.symbol;
    if(!binding.symbol.internal_symbol.empty()) {
      if(binding.is_thread_local &&
         binding.owner_class &&
         symbol.thread_local_wrapper_object_symbol.empty()) {
        symbol.thread_local_wrapper_object_symbol =
            symbol_linkage::thread_local_wrapper_object_symbol_for_static_member_variable(
                *binding.owner_class,
                binding.name);
      }
      set_dump_symbol(var_node, symbol);
    } else {
      symbol = static_member_variable_output_symbol_identity(info, binding);
      if(binding.is_thread_local && symbol.thread_local_wrapper_object_symbol.empty()) {
        symbol.thread_local_wrapper_object_symbol =
            symbol_linkage::thread_local_wrapper_object_symbol_for_static_member_variable(
                info,
                binding.name);
      }
      set_dump_symbol(var_node, symbol);
    }
    if(binding.is_thread_local &&
       !var_node.is_extern_declaration &&
       ctx.complete_class_type(binding.type) &&
       !symbol.internal_symbol.empty()) {
      set_callsem_local_static_guard_symbol(
          var_node,
          symbol_linkage::thread_local_guard_internal_symbol(symbol.internal_symbol));
    }

    constant_eval::ConstexprValue constexpr_value;
    DumpNode literal_node;
    Scope * initializer_scope =
        binding.constant_initializer_scope ?
            binding.constant_initializer_scope :
            info.member_scope.get();
    const bool class_lifetime_type =
        namespace_variable_type_has_class_lifetime(ctx, binding.type);
    const bool needs_default_lifetime_actions =
        class_lifetime_type &&
        !binding.constant_initializer &&
        (!semantic_class_model::is_trivially_default_constructible_type_for_host_abi(
             ctx,
             binding.type) ||
         !semantic_class_model::is_trivially_destructible_type_for_host_abi(
             ctx,
             binding.type));
    if(binding.constant_initializer &&
       initializer_scope &&
       !ctx.complete_class_type(binding.type) &&
       ctx.evaluate_initializer_constant_value(*initializer_scope,
                                               *binding.constant_initializer,
                                               binding.type,
                                               constexpr_value) &&
       make_constexpr_scalar_literal_node(ctx, constexpr_value, binding.type, literal_node)) {
      var_node.children.push_back(std::move(literal_node));
    } else if(!var_node.is_extern_declaration &&
              initializer_scope &&
              class_lifetime_type &&
              (binding.constant_initializer || needs_default_lifetime_actions)) {
      semantic_lifetime::analyze_object_lifetime_actions(
          ctx,
          *initializer_scope,
          binding.name,
          binding.type,
          binding.constant_initializer,
          var_node,
          ctx.source_location_for_name_in_node(*binding.definition_node, binding.name));
    } else if(binding.constant_initializer &&
              initializer_scope &&
              !var_node.is_extern_declaration) {
      semantic_lifetime::analyze_initializer(ctx,
                                             *initializer_scope,
                                             binding.type,
                                             *binding.constant_initializer,
                                             var_node);
    }

    apply_local_static_guard_to_lifetime_actions(var_node);
    out.children.push_back(std::move(var_node));
    binding.definition_output_emitted = true;
  }

  info.has_late_required_static_member_output = false;
  info.late_required_static_member_output_queued = false;
}

bool namespace_variable_type_has_class_lifetime(SemanticContext & ctx,
                                                const TypePtr & type)
{
  TypePtr direct_type = strip_top_level_cv(remove_reference_type(type));
  while(direct_type && direct_type->kind == Type::TK_ARRAY) {
    direct_type = strip_top_level_cv(direct_type->inner);
  }
  return direct_type && ctx.complete_class_type(direct_type);
}

FunctionBinding * find_namespace_function_binding_by_node_recursive(Scope & scope,
                                                                    const CppAstNode & init_decl);

bool should_emit_free_function_definition(SemanticContext & ctx,
                                          const FunctionBinding & binding)
{
  const bool definition_required =
      has_output_requirement(binding.output_requirements, ORK_DEFINITION);
  const CppAstNode * declaration =
      binding.definition_node ? binding.definition_node : binding.declaration_node;
  const bool definition_from_standard_include =
      ctx.definition_comes_from_standard_include_path(declaration,
                                                      binding.body,
                                                      binding.is_defaulted);
  const bool has_out_of_class_member_definition =
      binding_has_out_of_class_member_definition(binding);
  const bool eager_out_of_class_member_definition =
      has_out_of_class_member_definition &&
      !class_has_template_identity(binding.owner_class) &&
      !template_api::class_is_explicit_specialization(binding.owner_class);
  const bool hidden_friend_source_definition =
      binding.lexical_access_class &&
      !binding.owner_class &&
      binding.has_definition &&
      binding.body;
  const bool template_owned_hidden_friend_source_definition =
      hidden_friend_source_definition &&
      template_api::class_has_template_identity(binding.lexical_access_class);
  const bool lazy_inline_free_definition =
      !binding.owner_class &&
      !binding.lexical_access_class &&
      (binding.is_inline ||
       node_decl_spec_contains_token(declaration, KW_INLINE) ||
       node_decl_spec_contains_token(declaration, KW_CONSTEXPR));
  if(binding.is_deleted) {
    return false;
  }
  if((definition_from_standard_include || lazy_inline_free_definition) &&
     !ctx.emit_all_source_function_definitions() &&
     !definition_required &&
     !binding.synthesized) {
    return false;
  }
  if(binding.is_explicit_instantiation_definition) {
    return true;
  }
  if(template_api::function_binding_output_suppressed_by_explicit_instantiation(
         binding)) {
    return false;
  }
  if(binding.is_explicit_specialization) {
    return true;
  }
  if(symbol_linkage::has_weak_linkage(binding.symbol)) {
    if(eager_out_of_class_member_definition) {
      return true;
    }
    if(hidden_friend_source_definition) {
      if(template_owned_hidden_friend_source_definition &&
         !ctx.emit_all_source_function_definitions()) {
        return definition_required || binding.synthesized;
      }
      return true;
    }
    if(ctx.emit_all_source_function_definitions()) {
      return binding.has_definition || binding.synthesized;
    }
    return definition_required || binding.synthesized;
  }
  return true;
}

bool should_emit_namespace_variable_definition(SemanticContext & ctx,
                                               const ValueBinding * binding,
                                               const CppAstNode & declaration_node,
                                               const CppAstNode * initializer,
                                               bool is_definition)
{
  if(!is_definition) {
    return true;
  }
  if(!binding) {
    return true;
  }
  if(has_output_requirement(binding->output_requirements, ORK_DEFINITION)) {
    return true;
  }
  const CppAstNode * source_node =
      binding->definition_node ? binding->definition_node :
      binding->declaration_node ? binding->declaration_node :
                                  &declaration_node;
  if(ctx.node_comes_from_standard_include_path(source_node) ||
     ctx.node_comes_from_standard_include_path(initializer)) {
    if(namespace_variable_type_has_class_lifetime(ctx, binding->type)) {
      return true;
    }
    return false;
  }
  return true;
}

bool class_has_immediate_friend_definition_output(SemanticContext & ctx,
                                                  ClassInfo & info,
                                                  const CppAstNode & node)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::function_definition) {
      continue;
    }
    const CppAstNode * specifiers = find_child_kind(child, CppAstKind::decl_specifier_seq);
    if(!specifiers ||
       !any_of(specifiers->children.begin(), specifiers->children.end(),
               [](const CppAstNode & spec)
               { return node_has_simple_type(spec, KW_FRIEND); })) {
      continue;
    }

    Scope * friend_scope = unqualified_friend_entity_scope(info);
    FunctionBinding * binding =
        friend_scope ? find_namespace_function_binding_by_node_recursive(*friend_scope, child) :
                       nullptr;
    if(!binding || should_emit_free_function_definition(ctx, *binding)) {
      return true;
    }
  }
  return false;
}

bool should_emit_class_vtables(
    SemanticContext & ctx,
    ClassInfo & info,
    const ClassInfo * output_owner,
    const ClassOutputReadiness & class_output_readiness)
{
  if(info.vtables.empty()) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=no reason=no-vtables");
    }
    return false;
  }
  if(semantic_template_output_policy::implicit_instantiation_definition_suppressed(&info)) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=no reason=explicit-instantiation-suppression");
    }
    return false;
  }
  const bool suppress_implicit_instantiation_vtables =
      class_output_readiness.templated_context &&
      class_output_readiness.suppress_implicit_definition;
  if(suppress_implicit_instantiation_vtables) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=no reason=explicit-instantiation-suppression");
    }
    return false;
  }
  const bool has_external_vtable_symbol =
      symbol_linkage::has_external_vtable_symbol_candidate(info.type);
  const VTableKeyFunctionDecision key_function =
      vtable_key_function_decision(ctx, info, class_output_readiness);
  const bool has_nonlocal_virtual_owner = key_function.kind == VTK_EXTERNAL;
  if(has_nonlocal_virtual_owner && info.rtti_required) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=no reason=external-key-function-rtti");
    }
    return false;
  }
  if(info.rtti_required) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=yes reason=rtti-required");
    }
    return true;
  }
  if(key_function.kind == VTK_LOCAL) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=yes reason=local-key-function=" +
                             key_function.name);
    }
    return true;
  }
  for(std::size_t table_index = 0; table_index < info.vtables.size(); ++table_index) {
    const VTableInfo & table = info.vtables[table_index];
    for(std::size_t i = 0; i < table.slots.size(); ++i) {
      FunctionBinding * binding =
          semantic_template_function::acquire_required_function_definition_binding(
              ctx, table.slots[i].function, *info.member_scope);
      binding = resolve_output_function_binding(ctx, binding);
      if(!binding || !binding->is_virtual) {
        continue;
      }
      if(semantic_template_output_policy::should_emit_instantiated_class_method_definition(
             class_output_readiness, *binding) &&
         template_api::function_binding_has_template_or_body_definition_source(*binding)) {
        if(suppress_implicit_instantiation_vtables &&
           template_api::function_binding_bypasses_explicit_instantiation_suppression(
               *binding)) {
          continue;
        }
        if(parser_trace::enabled("output.class")) {
          parser_trace::note("output.class",
                             string(),
                             string("vtable-decision class=") + info.qualified_name +
                                 " emit=yes required-slot=" + binding->name);
        }
        return true;
      }
    }
  }
  for(map<string, vector<FunctionBinding *> >::iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(!binding ||
         binding->owner_class != &info ||
         binding->is_deleted ||
         (!binding->is_constructor && !binding->is_destructor)) {
        continue;
      }
      if(has_nonlocal_virtual_owner) {
        continue;
      }
      if(!suppress_implicit_instantiation_vtables &&
         semantic_template_output_policy::should_emit_instantiated_class_method_definition(
             class_output_readiness, *binding) &&
         (template_api::function_binding_has_template_or_body_definition_source(*binding) ||
          binding->synthesized)) {
        if(parser_trace::enabled("output.class")) {
          parser_trace::note("output.class",
                             string(),
                             string("vtable-decision class=") + info.qualified_name +
                                 " emit=yes required-special-member=" + binding->name);
        }
        return true;
      }
      const bool has_out_of_class_special_member_definition =
          binding->has_definition &&
          binding->definition_node &&
          binding->definition_node != binding->declaration_node;
      if(!has_out_of_class_special_member_definition) {
        continue;
      }
      if(!output_owner) {
        if(parser_trace::enabled("output.class")) {
          parser_trace::note("output.class",
                             string(),
                             string("vtable-decision class=") + info.qualified_name +
                                 " emit=yes required-special-member=" + binding->name);
        }
        return true;
      }
      if(semantic_template_output_policy::should_emit_instantiated_class_method_definition(
             class_output_readiness, *binding)) {
        if(parser_trace::enabled("output.class")) {
          parser_trace::note("output.class",
                             string(),
                             string("vtable-decision class=") + info.qualified_name +
                                 " emit=yes required-special-member=" + binding->name);
        }
        return true;
      }
    }
  }
  if(has_nonlocal_virtual_owner && parser_trace::enabled("output.class")) {
    parser_trace::note("output.class",
                       string(),
                       string("vtable-decision class=") + info.qualified_name +
                           " emit=no reason=external-key-function");
  }
  if(has_external_vtable_symbol &&
     !has_nonlocal_virtual_owner &&
     is_std_qualified_name_for_output(info.qualified_name) &&
     !is_std_implementation_detail_name_for_output(info.qualified_name)) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=yes reason=local-vtable-owner");
    }
    return true;
  }
  if(parser_trace::enabled("output.class")) {
    parser_trace::note("output.class",
                       string(),
                       string("vtable-decision class=") + info.qualified_name +
                           " emit=no reason=no-required-slots");
  }
  return false;
}

bool should_emit_class_vtables(SemanticContext & ctx, ClassInfo & info)
{
  if(info.vtables.empty()) {
    if(parser_trace::enabled("output.class")) {
      parser_trace::note("output.class",
                         string(),
                         string("vtable-decision class=") + info.qualified_name +
                             " emit=no reason=no-vtables");
    }
    return false;
  }
  const ClassInfo * output_owner =
      semantic_template_output_policy::effective_class_output_owner(info);
  const ClassOutputReadiness class_output_readiness =
      output_owner ? semantic_template_output_policy::class_output_readiness(ctx, info) :
                     ClassOutputReadiness();
  return should_emit_class_vtables(ctx, info, output_owner, class_output_readiness);
}

bool class_rtti_has_external_key_function(SemanticContext & ctx, ClassInfo & info)
{
  if(info.vtables.empty()) {
    return false;
  }
  const ClassInfo * output_owner =
      semantic_template_output_policy::effective_class_output_owner(info);
  const ClassOutputReadiness class_output_readiness =
      output_owner ? semantic_template_output_policy::class_output_readiness(ctx, info) :
                     ClassOutputReadiness();
  return vtable_key_function_decision(ctx, info, class_output_readiness).kind ==
         VTK_EXTERNAL;
}

long long compute_vtable_entry_result_adjustment(SemanticContext & ctx,
                                                 const FunctionBinding & slot_binding,
                                                 const FunctionBinding & final_binding)
{
  TypePtr slot_type = strip_top_level_cv(slot_binding.type);
  TypePtr final_type = strip_top_level_cv(final_binding.type);
  if(!slot_type || !final_type ||
     slot_type->kind != Type::TK_FUNCTION ||
     final_type->kind != Type::TK_FUNCTION ||
     type_equals(slot_type->inner, final_type->inner)) {
    return 0;
  }

  TypePtr slot_return = strip_top_level_cv(slot_type->inner);
  TypePtr final_return = strip_top_level_cv(final_type->inner);
  if(!slot_return || !final_return) {
    return 0;
  }

  const bool pointer_form =
      slot_return->kind == Type::TK_POINTER &&
      final_return->kind == Type::TK_POINTER;
  const bool reference_form =
      slot_return->kind == Type::TK_LVALUE_REFERENCE &&
      final_return->kind == Type::TK_LVALUE_REFERENCE;
  if(!pointer_form && !reference_form) {
    return 0;
  }

  TypePtr slot_object = strip_top_level_cv(slot_return->inner);
  TypePtr final_object = strip_top_level_cv(final_return->inner);
  if(!slot_object || !final_object || type_equals(slot_object, final_object)) {
    return 0;
  }

  ClassInfo * slot_class = ctx.complete_class_type(slot_object);
  if(!slot_class) {
    slot_class = ctx.class_info_for_type(slot_object);
  }
  ClassInfo * final_class = ctx.complete_class_type(final_object);
  if(!final_class) {
    final_class = ctx.class_info_for_type(final_object);
  }
  if(!slot_class || !final_class) {
    return 0;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(!find_unique_base_path(*final_class, slot_class, offset, access)) {
    return 0;
  }
  return static_cast<long long>(offset);
}

void append_named_function_candidates(ostringstream & out,
                                      Scope & scope,
                                      const string & name)
{
  const vector<FunctionBinding *> * found = find_direct_function_set(scope, name);
  if(!found) {
    out << " [candidates <none>]";
    return;
  }

  out << " [candidates";
  for(size_t i = 0; i < found->size(); ++i) {
    FunctionBinding * binding = (*found)[i];
    out << " " << binding->name << ":" << describe_type(binding->type);
    if(binding->ref_qualifier != RQ_NONE) {
      out << " refq=" << static_cast<int>(binding->ref_qualifier);
    }
  }
  out << "]";
}

FunctionBinding * find_namespace_function_binding_by_node(Scope & scope,
                                                          const string & name,
                                                          const CppAstNode & init_decl)
{
  const vector<FunctionBinding *> * found = find_direct_function_set(scope, name);
  if(!found) {
    return nullptr;
  }

  for(size_t i = 0; i < found->size(); ++i) {
    FunctionBinding * binding = (*found)[i];
    if(binding->declaration_node == &init_decl || binding->definition_node == &init_decl) {
      return binding;
    }
  }

  return found->size() == 1 ? (*found)[0] : nullptr;
}

FunctionBinding * find_namespace_function_binding_by_node_recursive(Scope & scope,
                                                                    const CppAstNode & init_decl)
{
  for(map<string, vector<FunctionBinding *> >::iterator it = scope.function_sets.begin();
      it != scope.function_sets.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(binding->declaration_node == &init_decl || binding->definition_node == &init_decl) {
        return binding;
      }
    }
  }

  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    FunctionBinding * nested =
        find_namespace_function_binding_by_node_recursive(*scope.namespace_children[i], init_decl);
    if(nested) {
      return nested;
    }
  }

  return nullptr;
}

string function_binding_output_lookup_name(const FunctionBinding & binding)
{
  return binding.display_name.empty() ?
      semantic_utils::unqualified_member_name(canonical_function_lookup_name(binding.name)) :
      binding.display_name;
}

bool function_binding_has_output_definition_source(const FunctionBinding & binding)
{
  return binding.has_definition ||
         binding.definition_node ||
         binding.body ||
         (binding.source_template && binding.source_template->body);
}

Scope * root_scope_for_output_lookup(Scope * scope)
{
  Scope * current = scope;
  while(current && current->parent) {
    current = current->parent;
  }
  return current;
}

Scope * qualified_function_namespace_scope_for_output_lookup(
    FunctionBinding & binding)
{
  Scope * seed = binding.declaration_scope;
  if(!seed && binding.source_template) {
    seed = binding.source_template->declaring_scope;
  }
  Scope * root = root_scope_for_output_lookup(seed);
  if(!root) {
    return nullptr;
  }

  QualifiedName qualified;
  if(!function_binding_qualified_name_syntax_for_symbol(binding, qualified)) {
    return nullptr;
  }
  if(qualified.qualifiers.empty()) {
    return nullptr;
  }

  QualifiedName namespace_name;
  namespace_name.rooted = true;
  if(qualified.qualifiers.size() == 1) {
    namespace_name.name = qualified.qualifiers[0];
  } else {
    namespace_name.qualifiers.assign(qualified.qualifiers.begin(),
                                     qualified.qualifiers.end() - 1);
    namespace_name.name = qualified.qualifiers.back();
  }
  return semantic_lookup::lookup_namespace_name(*root, namespace_name);
}

bool function_binding_types_match_for_output(const FunctionBinding & candidate,
                                             const FunctionBinding & binding)
{
  return candidate.ref_qualifier == binding.ref_qualifier &&
         callsemantic::types_equivalent_for_member_binding(candidate.type,
                                                           binding.type);
}

bool function_template_parameter_shapes_match(const FunctionTemplateDecl & candidate,
                                              const FunctionTemplateDecl & source)
{
  if(candidate.parameters.size() != source.parameters.size()) {
    return false;
  }
  for(size_t i = 0; i < candidate.parameters.size(); ++i) {
    if(candidate.parameters[i].kind != source.parameters[i].kind ||
       candidate.parameters[i].parameter_pack != source.parameters[i].parameter_pack) {
      return false;
    }
  }
  return true;
}

FunctionBinding * find_defined_namespace_function_binding_for_output(
    FunctionBinding & binding,
    Scope & namespace_scope,
    const string & lookup_name)
{
  const vector<FunctionBinding *> * found =
      find_direct_function_set(namespace_scope, lookup_name);
  if(!found) {
    return nullptr;
  }
  for(size_t i = 0; i < found->size(); ++i) {
    FunctionBinding * candidate = (*found)[i];
    if(candidate == &binding ||
       candidate->owner_class ||
       !function_binding_has_output_definition_source(*candidate) ||
       !function_binding_types_match_for_output(*candidate, binding)) {
      continue;
    }
    return candidate;
  }
  return nullptr;
}

FunctionBinding * materialize_defined_namespace_function_template_for_output(
    SemanticContext & ctx,
    FunctionBinding & binding,
    Scope & namespace_scope,
    const string & lookup_name)
{
  if(!binding.source_template) {
    return nullptr;
  }
  const vector<FunctionTemplateDecl *> * templates =
      find_direct_function_template_set(namespace_scope, lookup_name);
  if(!templates) {
    return nullptr;
  }

  for(size_t i = 0; i < templates->size(); ++i) {
    FunctionTemplateDecl * candidate_template = (*templates)[i];
    if(!candidate_template ||
       candidate_template == binding.source_template ||
       !candidate_template->body ||
       !function_template_parameter_shapes_match(*candidate_template,
                                                 *binding.source_template)) {
      continue;
    }

    FunctionBinding * candidate = nullptr;
    try {
      candidate =
          semantic_template_function::acquire_function_template_binding(
              ctx,
              *candidate_template,
              binding.instantiation_arguments,
              &namespace_scope,
              binding.instantiation_pack_sizes.empty() ?
                  nullptr :
                  &binding.instantiation_pack_sizes,
              true,
              nullptr);
    } catch(const TemplateSubstitutionFailure &) {
      continue;
    }
    if(candidate == &binding ||
       !candidate ||
       candidate->owner_class ||
       !function_binding_has_output_definition_source(*candidate) ||
       !function_binding_types_match_for_output(*candidate, binding)) {
      continue;
    }
    return candidate;
  }
  return nullptr;
}

FunctionBinding * find_defined_namespace_function_for_output_binding(
    SemanticContext & ctx,
    FunctionBinding & binding,
    const string & lookup_name)
{
  if(binding.owner_class ||
     lookup_name.empty() ||
     !binding.source_template ||
     binding.source_template->body ||
     binding.has_definition ||
     binding.definition_node ||
     binding.body) {
    return nullptr;
  }
  Scope * namespace_scope =
      qualified_function_namespace_scope_for_output_lookup(binding);
  if(!namespace_scope) {
    return nullptr;
  }
  if(FunctionBinding * candidate =
         find_defined_namespace_function_binding_for_output(binding,
                                                            *namespace_scope,
                                                            lookup_name)) {
    return candidate;
  }
  return materialize_defined_namespace_function_template_for_output(ctx,
                                                                   binding,
                                                                   *namespace_scope,
                                                                   lookup_name);
}

string canonical_variable_output_name(const string & parsed_name,
                                      const ValueBinding * binding)
{
  if(!binding || parsed_name.find("::") == string::npos || !binding->declaration_scope) {
    return string();
  }

  if(binding->name.find("::") != string::npos) {
    return binding->name;
  }
  return scope_qualified_name(*binding->declaration_scope, binding->name);
}

void set_dump_qualified_name_syntax_from_scope(DumpNode & node,
                                               const Scope * scope,
                                               const string & name)
{
  if(!scope || name.empty()) {
    return;
  }
  set_callsem_qualified_name_syntax(node, scope_qualified_name_syntax(*scope, name));
}

void set_dump_qualified_name_syntax_from_function_binding(DumpNode & node,
                                                          const FunctionBinding & binding)
{
  QualifiedName qualified;
  if(function_binding_qualified_name_syntax_for_symbol(binding, qualified)) {
    set_callsem_qualified_name_syntax(node, qualified);
  }
}

const ValueBinding * find_namespace_value_binding_by_node(Scope & scope,
                                                          const CppAstNode & init_decl)
{
  for(map<string, ValueBinding>::const_iterator it = scope.values.begin();
      it != scope.values.end();
      ++it) {
    const ValueBinding & binding = it->second;
    if(binding.declaration_node == &init_decl || binding.definition_node == &init_decl) {
      return &binding;
    }
  }

  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    const ValueBinding * nested =
        find_namespace_value_binding_by_node(*scope.namespace_children[i], init_decl);
    if(nested) {
      return nested;
    }
  }

  return nullptr;
}

using MemberBindingByNodeMap = map<const CppAstNode *, FunctionBinding *>;

ClassInfo * canonicalize_output_class_info(SemanticContext & ctx, ClassInfo * info)
{
  if(!info) {
    return nullptr;
  }
  ClassInfo * complete = info->type ? ctx.complete_class_type(info->type) : nullptr;
  return complete ? complete : info;
}

void note_class_member_binding_by_node(MemberBindingByNodeMap & out,
                                       const CppAstNode * node,
                                       FunctionBinding * binding)
{
  if(!node) {
    return;
  }
  MemberBindingByNodeMap::iterator found = out.find(node);
  if(found == out.end()) {
    out[node] = binding;
    return;
  }
  if(found->second != binding) {
    found->second = nullptr;
  }
}

MemberBindingByNodeMap build_class_member_binding_by_node_map(ClassInfo & info)
{
  MemberBindingByNodeMap out;
  for(map<string, vector<FunctionBinding *> >::iterator it = info.methods.begin();
      it != info.methods.end(); ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * candidate = it->second[i];
      note_class_member_binding_by_node(out, candidate->definition_node, candidate);
      note_class_member_binding_by_node(out, candidate->declaration_node, candidate);
    }
  }
  if(info.member_scope) {
    for(map<string, vector<FunctionBinding *> >::iterator it =
            info.member_scope->function_sets.begin();
        it != info.member_scope->function_sets.end(); ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        FunctionBinding * candidate = it->second[i];
        if(!candidate ||
           candidate->owner_class != &info ||
           candidate->is_method ||
           candidate->is_constructor ||
           candidate->is_destructor) {
          continue;
        }
        note_class_member_binding_by_node(out, candidate->definition_node, candidate);
        note_class_member_binding_by_node(out, candidate->declaration_node, candidate);
      }
    }
  }
  return out;
}

FunctionBinding * find_class_member_binding_by_node(const MemberBindingByNodeMap & by_node,
                                                    const CppAstNode & node)
{
  MemberBindingByNodeMap::const_iterator found = by_node.find(&node);
  return found == by_node.end() ? nullptr : found->second;
}

void analyze_function_binding_output_impl(SemanticContext & ctx,
                                          OutputState & state,
                                          Scope & scope,
                                          FunctionBinding & binding,
                                          DumpNode & out);

void analyze_class_output_from_info_impl(SemanticContext & ctx,
                                         OutputState & state,
                                         ClassInfo & info,
                                         const CppAstNode & node,
                                         DumpNode & out);

void analyze_vtable_output(SemanticContext & ctx,
                           OutputState & state,
                           ClassInfo & info,
                           DumpNode & out);

void analyze_declaration_output_impl(SemanticContext & ctx,
                                     OutputState & state,
                                     Scope & scope,
                                     const CppAstNode & node,
                                     DumpNode & out,
                                     bool is_c_linkage,
                                     bool linkage_has_braces = false);

void analyze_function_body_for_witness_semantics_impl(SemanticContext & ctx,
                                                      Scope & scope,
                                                      FunctionBinding & binding);

void note_witness_body_constructor_closures(SemanticContext & ctx,
                                            const CallSemNode & node);

struct ScopedDefinitionOutput
{
  ScopedDefinitionOutput(bool & in_progress,
                         bool & emitted)
      : in_progress_(in_progress),
        emitted_(emitted),
        active_(true)
  {
    in_progress_ = true;
  }

  ~ScopedDefinitionOutput()
  {
    if(active_) {
      in_progress_ = false;
    }
  }

  void finish()
  {
    emitted_ = true;
    in_progress_ = false;
    active_ = false;
  }

private:
  bool & in_progress_;
  bool & emitted_;
  bool active_;
};

void analyze_required_vtable_output(SemanticContext & ctx,
                                    OutputState & state,
                                    ClassInfo & info,
                                    DumpNode & out,
                                    FunctionBinding * trigger_binding = nullptr)
{
  (void)trigger_binding;
  const bool emit_vtables = should_emit_class_vtables(ctx, info);
  bool already_emitted = true;
  for(size_t table_index = 0; table_index < info.vtables.size(); ++table_index) {
    if(state.emitted_vtable_output.count(info.vtables[table_index].key) == 0) {
      already_emitted = false;
      break;
    }
  }

  if(!emit_vtables ||
     info.vtable_output_emitted ||
     info.vtable_output_in_progress) {
    if(parser_trace::enabled("output.class")) {
      std::string reason = !emit_vtables ?
                               "should-emit-false" :
                               (info.vtable_output_emitted ? "already-emitted" :
                                                             "in-progress");
      parser_trace::note("output.class",
                         string(),
                         string("vtable-output-skip class=") + info.qualified_name +
                             " reason=" + reason);
    }
    return;
  }
  if(already_emitted) {
    info.vtable_output_emitted = true;
    return;
  }
  if(parser_trace::enabled("output.class")) {
    parser_trace::note("output.class",
                       string(),
                       string("vtable-output-emit class=") + info.qualified_name +
                           " reason=required-member");
  }
  ScopedDefinitionOutput vtable_output_guard(info.vtable_output_in_progress,
                                             info.vtable_output_emitted);
  analyze_vtable_output(ctx, state, info, out);
  vtable_output_guard.finish();
}

namespace {

bool function_binding_has_trivial_lifecycle_output(SemanticContext & ctx,
                                                   FunctionBinding & binding)
{
  return special_member_binding_has_trivial_lifecycle_output(ctx, binding);
}

bool function_binding_has_trivial_default_constructor_object_output(
    SemanticContext & ctx,
    FunctionBinding & binding)
{
  if(!binding.owner_class ||
     !binding.is_constructor ||
     binding.params.size() != 1) {
    return false;
  }

  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  return implicit_like &&
         semantic_class_model::is_trivially_default_constructible_type_for_host_abi(
             ctx,
             binding.owner_class->type);
}

bool function_binding_has_object_trivial_lifecycle_output(SemanticContext & ctx,
                                                          FunctionBinding & binding)
{
  return function_binding_has_trivial_lifecycle_output(ctx, binding) ||
         function_binding_has_trivial_default_constructor_object_output(ctx, binding);
}
}

void analyze_function_declaration_output(SemanticContext & ctx,
                                         FunctionBinding & binding,
                                         DumpNode & out)
{
  DumpNode decl_node = make_dump_node(CallSemKind::function_declaration, binding.name);
  set_callsem_resolved_name(decl_node, function_output_name(binding));
  set_dump_qualified_name_syntax_from_function_binding(decl_node, binding);
  decl_node.semantic_type = binding.type;
  decl_node.is_c_linkage = binding.is_c_linkage;
  decl_node.is_virtual_member_function = binding.is_virtual;
  decl_node.is_conversion_operator = binding.is_conversion_operator;
  decl_node.trivial_lifecycle = function_binding_has_trivial_lifecycle_output(ctx, binding);
  set_dump_symbol(decl_node,
                  function_entry_point_symbol(binding, symbol_linkage::SMEK_COMPLETE));
  out.children.push_back(std::move(decl_node));
  binding.output_emitted = true;
}

void analyze_function_binding_output_impl(SemanticContext & ctx,
                                          OutputState & state,
                                          Scope & scope,
                                          FunctionBinding & binding,
                                          DumpNode & out)
{
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << binding.name;
    if(binding.owner_class) {
      query << " owner=" << binding.owner_class->qualified_name;
    }
    const std::string template_trace_key =
        template_api::function_binding_template_trace_key(&binding);
    if(!template_trace_key.empty()) {
      query << " inst=" << template_trace_key;
    }
    semantic_hotspot::note_semantic_query("analyze_function_binding_output", query.str());
  }
  if(parser_trace::enabled("lifetime.init")) {
    std::ostringstream trace;
    trace << "emit-binding binding=" << static_cast<void *>(&binding)
          << " function=" << binding.name
          << " source_template="
          << (template_api::function_binding_has_source_template_identity(&binding) ?
                  "yes" :
                  "no")
          << " inst_key=" << template_api::function_binding_template_trace_key(&binding)
          << " ctor_init=" << (binding.ctor_initializer ? "yes" : "no")
          << " inst_use="
          << (binding.instantiation_use_location.empty() ?
                  std::string("<none>") :
                  binding.instantiation_use_location)
          << " synthesized=" << (binding.synthesized ? "yes" : "no")
          << " inline=" << (function_binding_is_inline(binding) ? "yes" : "no")
          << " definition_required="
          << (has_output_requirement(binding.output_requirements, ORK_DEFINITION) ? "yes" :
                                                                               "no")
          << " output_requirements=" << binding.output_requirements
          << " deleted=" << (binding.is_deleted ? "yes" : "no")
          << " aggregate_ctor=" << (binding.is_aggregate_constructor ? "yes" : "no")
          << " has_definition=" << (binding.has_definition ? "yes" : "no");
    parser_trace::note("lifetime.init", std::string(), trace.str());
  }
  if(!binding.has_definition) {
    if(binding.output_emitted) {
      return;
    }
    DumpNode decl_node = make_dump_node(CallSemKind::function_declaration, binding.name);
    set_callsem_resolved_name(decl_node, function_output_name(binding));
    set_dump_qualified_name_syntax_from_function_binding(decl_node, binding);
    decl_node.semantic_type = binding.type;
    decl_node.is_c_linkage = binding.is_c_linkage;
    decl_node.is_conversion_operator = binding.is_conversion_operator;
    set_dump_symbol(decl_node,
                    function_entry_point_symbol(binding, symbol_linkage::SMEK_COMPLETE));
    out.children.push_back(std::move(decl_node));
    binding.output_emitted = true;
    return;
  }
  const bool definition_required =
      has_output_requirement(binding.output_requirements, ORK_DEFINITION);
  const bool declaration_node_is_definition_syntax =
      binding.declaration_node &&
      (binding.declaration_node->kind == CppAstKind::function_definition ||
       binding.declaration_node->kind == CppAstKind::special_member_definition);
  const bool tracked_template_has_body =
      semantic_template_output_policy::function_has_tracked_template_body(
          binding,
          declaration_node_is_definition_syntax);
  if(!definition_required &&
     tracked_template_has_body &&
     !binding.synthesized) {
    if(!binding.output_emitted) {
      analyze_function_declaration_output(ctx, binding, out);
    }
    return;
  }
  if(binding.definition_output_emitted || binding.definition_output_in_progress) {
    if(semantic_hotspot::enabled()) {
      std::ostringstream query;
      query << binding.name;
      if(binding.owner_class) {
        query << " owner=" << binding.owner_class->qualified_name;
      }
      const std::string template_trace_key =
          template_api::function_binding_template_trace_key(&binding);
      if(!template_trace_key.empty()) {
        query << " inst=" << template_trace_key;
      }
      semantic_hotspot::note_semantic_query("revisit_function_definition_output", query.str());
    }
    return;
  }
  if(!semantic_template_output_policy::function_instantiation_arguments_complete(ctx, binding)) {
    if(parser_trace::enabled("lifetime.init")) {
      std::ostringstream trace;
      trace << "skip-dependent-template-binding function=" << binding.name
            << " inst_key=" << template_api::function_binding_template_trace_key(&binding);
      parser_trace::note("lifetime.init", std::string(), trace.str());
    }
    return;
  }
  DIAG_CONTEXT("analyze_function_binding_output [" + binding.name +
               (binding.owner_class ?
                    (std::string(" owner=") + binding.owner_class->qualified_name) :
                    std::string()) +
               (!template_api::function_binding_template_trace_key(&binding).empty() ?
                    (std::string(" inst=") +
                     template_api::function_binding_template_trace_key(&binding)) :
                    std::string()) + "]");
  ScopedDefinitionOutput output_guard(binding.definition_output_in_progress,
                                      binding.definition_output_emitted);

  Scope * parent_scope = nullptr;
  if(binding.is_method) {
    parent_scope = binding.declaration_scope ?
                       binding.declaration_scope :
                       binding.owner_class->member_scope.get();
  } else if(binding_defines_inline_like_friend(binding) &&
            binding.lexical_access_class &&
            binding.lexical_access_class->member_scope) {
    parent_scope = binding.lexical_access_class->member_scope.get();
  } else {
    parent_scope = binding.declaration_scope ? binding.declaration_scope : &scope;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "emit-binding-scope binding=" << static_cast<void *>(&binding)
          << " function=" << binding.name
          << " decl_scope="
          << (binding.declaration_scope ?
                  semantic_trace::scope_name_for_diagnostic(*binding.declaration_scope) :
                  std::string("<none>"))
          << " decl_bindings="
          << (binding.declaration_scope ?
                  semantic_trace::scope_bindings_for_diagnostic(*binding.declaration_scope) :
                  std::string("<none>"))
          << " parent_scope="
          << (parent_scope ? semantic_trace::scope_name_for_diagnostic(*parent_scope) :
                             std::string("<none>"))
          << " parent_bindings="
          << (parent_scope ? semantic_trace::scope_bindings_for_diagnostic(*parent_scope) :
                             std::string("<none>"));
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  recover_function_parameter_aliases_from_ast(ctx, binding, parent_scope);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "function-output-binding name=" << binding.name
          << " source-template="
          << template_api::function_binding_source_template_debug_identity(&binding)
          << " params={";
    for(size_t i = 0; i < binding.params.size(); ++i) {
      if(i != 0) {
        trace << ", ";
      }
      trace << (binding.params[i].first.empty() ? std::string("<empty>") : binding.params[i].first)
            << ":" << describe_type(binding.params[i].second);
      if(i < binding.parameter_aliases.size()) {
        trace << " alias="
              << (binding.parameter_aliases[i].empty() ?
                      std::string("<empty>") :
                      binding.parameter_aliases[i]);
      }
    }
    trace << "}";
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  const auto emit_function_variant =
      [&](const symbol_linkage::SymbolIdentity & function_symbol,
          symbol_linkage::SpecialMemberEntryPointKind entry_point_kind) -> void
      {
        DumpNode function_node = make_dump_node(CallSemKind::function_definition, binding.name);
        set_callsem_resolved_name(function_node, function_output_name(binding));
        set_dump_qualified_name_syntax_from_function_binding(function_node, binding);
        function_node.semantic_type = binding.type;
        function_node.is_c_linkage = binding.is_c_linkage;
        function_node.is_virtual_member_function = binding.is_virtual;
        function_node.is_conversion_operator = binding.is_conversion_operator;
        function_node.is_semantically_nothrow = ctx.function_binding_is_nothrow(binding);
        function_node.is_explicit_instantiation_definition =
            binding.is_explicit_instantiation_definition;
        function_node.trivial_lifecycle =
            function_binding_has_trivial_lifecycle_output(ctx, binding);
        function_node.object_trivial_lifecycle =
            function_binding_has_object_trivial_lifecycle_output(ctx, binding);
        set_callsem_abi_tags(function_node, function_binding_abi_tags(binding));
        if(binding.declaration_node) {
          set_dump_source_location(function_node, *binding.declaration_node);
          set_dump_token(function_node, *binding.declaration_node);
        } else if(binding.body) {
          set_dump_source_location(function_node, *binding.body);
          set_dump_token(function_node, *binding.body);
        }
        bool explicit_nothrow = false;
        if(ctx.explicit_function_nothrow(binding, explicit_nothrow) &&
           explicit_nothrow) {
          function_node.is_explicit_nothrow = true;
        }
        if(binding.is_constructor || binding.is_destructor) {
          function_node.has_special_member_entry_point_kind = true;
          set_callsem_special_member_entry_point_kind(function_node, entry_point_kind);
          if(entry_point_kind == symbol_linkage::SMEK_BASE &&
             binding.owner_class &&
             class_needs_vtt(*binding.owner_class)) {
            function_node.uses_vtt_parameter = true;
            set_callsem_vtt_symbol(
                function_node,
                symbol_linkage::internal_symbol_from_name(
                    binding.owner_class->qualified_name + "::__vtt"));
          }
        }
        if(binding.owner_class) {
          for(size_t i = 0; i < binding.owner_class->virtual_base_subobjects.size(); ++i) {
            const SubobjectInfo & subobject = binding.owner_class->virtual_base_subobjects[i];
            if(!subobject.type) {
              continue;
            }
            mutable_callsem_virtual_base_layout(function_node).push_back(
                make_pair(subobject.type->qualified_name, subobject.offset));
          }
        }
        set_dump_symbol(function_node, function_symbol);
        if((binding.is_constructor || binding.is_destructor) &&
           entry_point_kind == symbol_linkage::SMEK_COMPLETE &&
           (!binding.owner_class || !class_has_virtual_bases(*binding.owner_class))) {
          const symbol_linkage::SymbolIdentity base_entry_symbol =
              function_entry_point_symbol(binding, symbol_linkage::SMEK_BASE);
          if(symbol_linkage::has_exported_object_symbol(base_entry_symbol) &&
             base_entry_symbol.object_symbol != function_symbol.object_symbol) {
            std::vector<std::string> object_aliases;
            object_aliases.push_back(base_entry_symbol.object_symbol);
            set_callsem_object_aliases(function_node, object_aliases);
          }
        }

        DumpNode body_node = make_dump_node(CallSemKind::compound_statement);
        DumpNode constructor_try_prefix = make_dump_node(CallSemKind::compound_statement);
        const std::size_t expected_function_children = binding.params.size() + 1;
        if(expected_function_children > 1) {
          function_node.children.reserve(expected_function_children);
        }
        const bool constructor_function_try_block =
            binding.is_constructor &&
            binding.body &&
            binding.body->kind == CppAstKind::try_block;

        Scope function_scope(parent_scope);
        function_scope.class_info = binding.owner_class;
        function_scope.function = &binding;
        template_scope::overlay_scope_bindings(function_scope,
                                               *parent_scope,
                                               template_scope::OVERLAY_TEMPLATE_BOUND_ONLY);
        const auto emit_function_body_and_collect =
            [&]() -> void
        {
          if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
            ++counters->function_body_emit_by_demand[
                static_cast<std::size_t>(semantic_metrics::current_class_demand())];
          }
          const std::vector<TypePtr> parameter_object_types =
              recover_function_parameter_object_types(ctx, function_scope, binding);
          for(size_t i = 0; i < binding.params.size(); ++i) {
            bind_function_parameter_lookup(function_scope,
                                           binding,
                                           i,
                                           &parameter_object_types);
            DumpNode param_node =
                make_dump_node(CallSemKind::parameter, function_parameter_display_name(binding, i));
            param_node.semantic_type = binding.params[i].second;
            append_dump_virtual_base_layout(
                param_node,
                class_info_for_virtual_base_layout_param(ctx, binding.params[i].second));
            function_node.children.push_back(std::move(param_node));
          }
          bind_function_parameter_pack_sizes(ctx, function_scope, binding);
          bind_function_parameter_value_packs(ctx, function_scope, binding);
          append_function_exception_spec_candidates(ctx, function_scope, binding, function_node);

          TypePtr function_type = strip_top_level_cv(binding.type);
          if(binding.is_constructor) {
            semantic_lifetime::append_constructor_generated_statements(
                ctx,
                function_scope,
                binding,
                entry_point_kind,
                constructor_function_try_block ? constructor_try_prefix : body_node);
          }
          if(binding.is_copy_assignment) {
            semantic_lifetime::append_copy_assignment_generated_statements(
                ctx, function_scope, binding, body_node);
          }
          if(binding.is_move_assignment) {
            semantic_lifetime::append_move_assignment_generated_statements(
                ctx, function_scope, binding, body_node);
          }
          if(binding.body) {
            if(binding.body->kind == CppAstKind::lazy_function_body) {
              binding.body = ctx.materialize_lazy_function_body(*binding.body);
            }
            if(binding.body->kind != CppAstKind::compound_statement &&
               binding.body->kind != CppAstKind::try_block) {
              ostringstream outmsg;
              outmsg << "function body must be compound-statement or try-block";
              outmsg << " [body kind " << cppast_kind_text(binding.body->kind) << "]";
              outmsg << semantic_trace::current_location_note(ctx, binding.body);
              if(binding.declaration_node) {
                outmsg << " [decl kind " << cppast_kind_text(binding.declaration_node->kind)
                       << "]";
                outmsg << semantic_trace::node_location_note(
                    ctx, "declaration", binding.declaration_node);
              }
              if(binding.definition_node) {
                outmsg << " [def kind " << cppast_kind_text(binding.definition_node->kind) << "]";
                outmsg << semantic_trace::node_location_note(
                    ctx, "definition", binding.definition_node);
              }
              throw logic_error(outmsg.str());
            }
            if(binding.cached_body_output &&
               body_node.children.empty() &&
               !binding.is_constructor &&
               !binding.is_copy_assignment &&
               !binding.is_move_assignment &&
               !binding.is_destructor &&
               binding.body->kind == CppAstKind::compound_statement) {
              body_node = std::move(*binding.cached_body_output);
              binding.cached_body_output.reset();
            } else {
              try {
                if(binding.body->kind == CppAstKind::compound_statement) {
                  if(semantic_metrics::AnalyzerCounters * counters =
                         ctx.performance_counters()) {
                    counters->function_body_statement_by_demand[
                        static_cast<std::size_t>(
                            semantic_metrics::current_class_demand())] +=
                        binding.body->children.size();
                  }
                  for(size_t i = 0; i < binding.body->children.size(); ++i) {
                    semantic_statement::analyze_statement(
                        ctx,
                        function_scope,
                        function_type->inner,
                        binding.body->children[i],
                        body_node);
                  }
                } else {
                  if(semantic_metrics::AnalyzerCounters * counters =
                         ctx.performance_counters()) {
                    ++counters->function_body_statement_by_demand[
                        static_cast<std::size_t>(
                            semantic_metrics::current_class_demand())];
                  }
                  semantic_statement::analyze_statement(
                      ctx, function_scope, function_type->inner, *binding.body, body_node);
                  if(constructor_function_try_block) {
                    if(body_node.children.size() != 1 ||
                       body_node.children[0].kind != CallSemKind::try_statement) {
                      throw logic_error("constructor function-try-block output shape");
                    }
                    DumpNode & try_node = body_node.children[0];
                    DumpNode * try_body = nullptr;
                    for(size_t i = 0; i < try_node.children.size(); ++i) {
                      if(try_node.children[i].kind == CallSemKind::compound_statement) {
                        try_body = &try_node.children[i];
                        break;
                      }
                    }
                    if(!try_body) {
                      throw logic_error("constructor function-try-block missing body");
                    }
                    try_node.text = "constructor_function_try";
                    try_body->children.insert(try_body->children.begin(),
                                              std::make_move_iterator(
                                                  constructor_try_prefix.children.begin()),
                                              std::make_move_iterator(
                                                  constructor_try_prefix.children.end()));
                  }
                }
              } catch(const logic_error & e) {
                ostringstream outmsg;
                outmsg << e.what();
                outmsg << " [function " << binding.name << "]";
                outmsg << " [is_method " << (binding.is_method ? "yes" : "no") << "]";
                outmsg << " [body_parent_scope " << scope_qualified_name(*parent_scope, "<here>")
                       << "]";
                outmsg << " [body_parent_bindings "
                       << ctx.describe_scope_bindings_for_diagnostic(*parent_scope) << "]";
                outmsg << " [body_function_scope "
                       << scope_qualified_name(function_scope, "<here>") << "]";
                outmsg << " [body_function_bindings "
                       << ctx.describe_scope_bindings_for_diagnostic(function_scope) << "]";
                throw logic_error(outmsg.str());
              }
            }
          }
          if(binding.is_destructor) {
            semantic_lifetime::append_destructor_generated_statements(
                ctx, function_scope, binding, entry_point_kind, body_node);
          }
          if((function_node.is_explicit_nothrow ||
              function_node.is_semantically_nothrow) &&
             !function_node.has_dynamic_exception_spec) {
            set<FunctionBinding *> visiting;
            if(ctx.callsem_node_can_throw(function_scope, body_node, visiting)) {
              function_node.needs_noexcept_terminate = true;
            }
          }
          function_node.children.push_back(std::move(body_node));
          note_required_callee_rescan(ctx, function_node);
          collect_required_callees_from_node(ctx, function_node, &binding);
        };
        const template_api::ScopedTemplateWitnessEntryContext entry_context =
            template_api::maybe_enter_function_body_materialization_context(ctx,
                                                                            &binding);
        const ScopedTemplateUseLocation instantiation_use_location(
            binding.instantiation_use_location);
        emit_function_body_and_collect();
        const std::size_t emitted_index = out.children.size();
        out.children.push_back(std::move(function_node));
        if(out.kind == CallSemKind::translation_unit &&
           state.emitted_output_callee_scan_index == emitted_index) {
          state.emitted_output_callee_scan_index = emitted_index + 1;
        }
      };

  const bool emit_base_entry =
      binding.owner_class &&
      (binding.is_constructor || binding.is_destructor);
  if(emit_base_entry) {
    emit_function_variant(function_entry_point_symbol(binding, symbol_linkage::SMEK_BASE),
                          symbol_linkage::SMEK_BASE);
  }
  if(binding.is_destructor && binding.has_virtual_slot) {
    emit_function_variant(function_entry_point_symbol(binding, symbol_linkage::SMEK_DELETING),
                          symbol_linkage::SMEK_DELETING);
  }
  emit_function_variant(function_entry_point_symbol(binding, symbol_linkage::SMEK_COMPLETE),
                        symbol_linkage::SMEK_COMPLETE);
  output_guard.finish();
  binding.output_emitted = true;
  binding.cached_body_output.reset();
  if(binding_has_out_of_class_member_definition(binding) ||
     (binding.owner_class &&
      binding.owner_class->is_polymorphic &&
      (binding.is_constructor || binding.is_destructor))) {
    analyze_required_vtable_output(ctx, state, *binding.owner_class, out, &binding);
  }
}

void analyze_function_body_for_witness_semantics_impl(SemanticContext & ctx,
                                                      Scope & scope,
                                                      FunctionBinding & binding)
{
  if(!witness::source_capture_enabled(ctx.template_witness_context()) ||
     !binding.body ||
     binding.is_constructor ||
     binding.is_destructor ||
     binding.is_copy_assignment ||
     binding.is_move_assignment ||
     binding.definition_output_in_progress ||
     binding.definition_output_emitted ||
     !semantic_template_output_policy::function_instantiation_arguments_complete(ctx, binding)) {
    return;
  }

  TypePtr function_type = strip_top_level_cv(binding.type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return;
  }

  Scope * parent_scope = binding.is_method ?
                             (binding.declaration_scope ?
                                  binding.declaration_scope :
                                  (binding.owner_class ?
                                       binding.owner_class->member_scope.get() :
                                       nullptr)) :
                             (binding.declaration_scope ? binding.declaration_scope : &scope);
  if(!parent_scope) {
    return;
  }

  recover_function_parameter_aliases_from_ast(ctx, binding, parent_scope);

  Scope function_scope(parent_scope);
  function_scope.class_info = binding.owner_class;
  function_scope.function = &binding;
  template_scope::overlay_scope_bindings(function_scope,
                                         *parent_scope,
                                         template_scope::OVERLAY_TEMPLATE_BOUND_ONLY);
  const std::vector<TypePtr> parameter_object_types =
      recover_function_parameter_object_types(ctx, function_scope, binding);
  for(std::size_t i = 0; i < binding.params.size(); ++i) {
    bind_function_parameter_lookup(function_scope,
                                   binding,
                                   i,
                                   &parameter_object_types);
  }
  bind_function_parameter_pack_sizes(ctx, function_scope, binding);
  bind_function_parameter_value_packs(ctx, function_scope, binding);

  DumpNode body_node = make_dump_node(CallSemKind::compound_statement);
  const template_api::ScopedTemplateWitnessEntryContext entry_context =
      template_api::maybe_enter_function_body_materialization_context(ctx, &binding);
  if(binding.body->kind == CppAstKind::lazy_function_body) {
    binding.body = ctx.materialize_lazy_function_body(*binding.body);
  }
  if(binding.body->kind == CppAstKind::compound_statement) {
    for(std::size_t i = 0; i < binding.body->children.size(); ++i) {
      semantic_statement::analyze_statement(ctx,
                                            function_scope,
                                            function_type->inner,
                                            binding.body->children[i],
                                            body_node);
    }
    note_witness_body_constructor_closures(ctx, body_node);
    return;
  }

  semantic_statement::analyze_statement(ctx,
                                        function_scope,
                                        function_type->inner,
                                        *binding.body,
                                        body_node);
  note_witness_body_constructor_closures(ctx, body_node);
}

void analyze_class_simple_declaration_output(SemanticContext & ctx,
                                             ClassInfo & info,
                                             const MemberBindingByNodeMap & bindings_by_node,
                                             const CppAstNode & node,
                                             DumpNode & out,
                                             set<FunctionBinding *> & emitted)
{
  const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child_kind(node, CppAstKind::init_declarator_list);
  if(!specifiers || !declarators || !class_member_specifiers_supported(*specifiers, true)) {
    return;
  }

  bool all_bound_function_decls = !declarators->children.empty();
  bool has_bound_function_decl = false;
  bool has_unbound_function_like_decl = false;
  for(size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      all_bound_function_decls = false;
      break;
    }
    if(find_class_member_binding_by_node(bindings_by_node, init_decl)) {
      has_bound_function_decl = true;
      continue;
    }
    all_bound_function_decls = false;
    if(find_function_parameter_clause_in_declarator(init_decl.children[0])) {
      has_unbound_function_like_decl = true;
      break;
    }
  }
  if(all_bound_function_decls || !has_unbound_function_like_decl) {
    if(!has_bound_function_decl) {
      return;
    }
    for(size_t i = 0; i < declarators->children.size(); ++i) {
      const CppAstNode & init_decl = declarators->children[i];
      if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
        continue;
      }
      FunctionBinding * binding =
          find_class_member_binding_by_node(bindings_by_node, init_decl);
      if(!binding) {
        continue;
      }
      analyze_function_declaration_output(ctx, *binding, out);
      emitted.insert(binding);
    }
    return;
  }

  PreparedClassMemberDeclarationContext prepared_decl;
  if(!prepare_class_member_declaration_context(ctx,
                                               *info.member_scope,
                                               *specifiers,
                                               declarators,
                                               false,
                                               false,
                                               false,
                                               prepared_decl)) {
    return;
  }

  if(!prepared_decl.parsed_decl_spec || prepared_decl.declaration_is_typedef) {
    return;
  }

  for(size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      continue;
    }
    if(FunctionBinding * binding =
           find_class_member_binding_by_node(bindings_by_node, init_decl)) {
      analyze_function_declaration_output(ctx, *binding, out);
      emitted.insert(binding);
      continue;
    }

    string name;
    TypePtr declared_type;
    PreparedMethodParseContext prepared_method;
    prepare_method_parse_context(&prepared_decl.resolved_specifiers,
                                 init_decl.children[0],
                                 prepared_method);
    if(!ctx.parse_declarator(*info.member_scope,
                             prepared_method.parse_declarator_node(),
                             prepared_decl.base,
                             name,
                             declared_type) ||
       name.empty()) {
      continue;
    }

    TypePtr stripped = strip_top_level_cv(declared_type);
    if(!stripped || stripped->kind != Type::TK_FUNCTION) {
      continue;
    }

    FunctionBinding * binding = nullptr;
    if(decl_spec_contains_token(prepared_decl.resolved_specifiers, KW_STATIC)) {
      binding = ctx.find_exact_function(*info.member_scope, name, declared_type);
    } else {
      binding = ctx.find_equivalent_class_function(
          info, name,
          method_function_type(info.type,
                               prepared_method.syntax.is_const_method,
                               prepared_method.syntax.is_volatile_method,
                               declared_type),
          prepared_method.syntax.ref_qualifier);
    }
    if(!binding) {
      ostringstream outmsg;
      outmsg << "missing member function binding";
      outmsg << " [class " << info.qualified_name << "]";
      outmsg << " [member " << name << "]";
      outmsg << " [declared_type " << describe_type(declared_type) << "]";
      append_named_function_candidates(outmsg, *info.member_scope, name);
      throw logic_error(outmsg.str());
    }
    analyze_function_declaration_output(ctx, *binding, out);
    emitted.insert(binding);
  }
}

void analyze_special_member_output(SemanticContext & ctx,
                                   const MemberBindingByNodeMap & bindings_by_node,
                                   OutputState & state,
                                   ClassInfo & info,
                                   const ClassOutputReadiness & class_output_readiness,
                                   const CppAstNode & node,
                                   DumpNode & out,
                                   set<FunctionBinding *> & emitted,
                                   bool emit_definition,
                                   bool force_declaration = false)
{
  FunctionBinding * binding = find_class_member_binding_by_node(bindings_by_node, node);
  string expected_type_text = string("<node-bound>");
  if(!binding) {
    const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
    if(!declarator) {
      throw logic_error("special member missing declarator");
    }

    vector<pair<string, TypePtr> > params;
    const CppAstNode * parameter_clause =
        find_child_kind(*declarator, CppAstKind::parameter_clause);
    if(parameter_clause &&
       !ctx.parse_parameter_clause(
           *info.member_scope, *parameter_clause, params, nullptr, false)) {
      throw logic_error("unsupported special-member parameter-clause");
    }

    vector<TypePtr> effective_params;
    effective_params.push_back(make_pointer(info.type));
    for(size_t i = 0; i < params.size(); ++i) {
      effective_params.push_back(params[i].second);
    }

    TypePtr expected_type =
        make_function(make_fundamental(FT_VOID), effective_params, false);
    expected_type_text = describe_type(expected_type);
    binding = ctx.find_equivalent_class_function(info, node.value, expected_type);
  }
  if(!binding) {
    throw logic_error("missing special member binding");
  }
  const bool should_emit_definition =
      !force_declaration &&
      (emit_definition || binding->is_defaulted) &&
      semantic_template_output_policy::should_emit_instantiated_class_method_definition(
          class_output_readiness, *binding);
  trace_class_output_decision("special-member",
                              info,
                              node,
                              binding,
                              emit_definition && !force_declaration,
                              should_emit_definition);
  if(should_emit_definition) {
    analyze_function_binding_output_impl(ctx, state, *info.member_scope, *binding, out);
  } else {
    analyze_function_declaration_output(ctx, *binding, out);
  }
  emitted.insert(binding);
}

void analyze_conversion_operator_output(SemanticContext & ctx,
                                        const MemberBindingByNodeMap & bindings_by_node,
                                        OutputState & state,
                                        ClassInfo & info,
                                        const ClassOutputReadiness & class_output_readiness,
                                        const CppAstNode & node,
                                        DumpNode & out,
                                        set<FunctionBinding *> & emitted,
                                        bool emit_definition)
{
  FunctionBinding * binding = find_class_member_binding_by_node(bindings_by_node, node);
  if(!binding) {
    std::string member_name;
    TypePtr declared_type;
    std::vector<std::pair<std::string, TypePtr> > params;
    MethodSyntaxInfo syntax;
    if(!parse_conversion_operator_signature(ctx,
                                            *info.member_scope,
                                            node,
                                            member_name,
                                            declared_type,
                                            params,
                                            nullptr,
                                            &syntax)) {
      throw logic_error("conversion operator missing children");
    }

    binding = ctx.find_exact_class_function(
        info, member_name,
        method_function_type(info.type,
                             syntax.is_const_method,
                             syntax.is_volatile_method,
                             declared_type),
        syntax.ref_qualifier);
  }
  if(!binding) {
    throw logic_error("missing conversion operator binding");
  }

  if(emit_definition &&
     semantic_template_output_policy::should_emit_instantiated_class_method_definition(
         class_output_readiness, *binding)) {
    analyze_function_binding_output_impl(ctx, state, *info.member_scope, *binding, out);
  } else {
    analyze_function_declaration_output(ctx, *binding, out);
  }
  emitted.insert(binding);
}

void append_rtti_base_output_nodes(const ClassInfo & info, DumpNode & node)
{
  for(size_t base_index = 0; base_index < info.bases.size(); ++base_index) {
    const BaseInfo & base = info.bases[base_index];
    if(!base.type) {
      continue;
    }
    DumpNode base_node = make_dump_node(CallSemKind::rtti_base,
                                        base.type->qualified_name);
    base_node.semantic_type = base.type->type;
    set_callsem_int_value(base_node, static_cast<long long>(base.offset));
    base_node.is_public_access = base.access == MA_PUBLIC;
    base_node.is_virtual_base_subobject = base.is_virtual;
    node.children.push_back(std::move(base_node));
  }
}

void append_vtable_output_node(SemanticContext & ctx,
                               OutputState & state,
                               ClassInfo & info,
                               const VTableInfo & table,
                               const TypePtr & rtti_type,
                               const symbol_linkage::SymbolIdentity * table_symbol,
                               symbol_linkage::SymbolLinkage primary_vtable_linkage,
                               DumpNode & out)
{
  if(!state.emitted_vtable_output.insert(table.key).second) {
    return;
  }

  DumpNode table_node = make_dump_node(CallSemKind::vtable_definition, table.key);
  table_node.semantic_type = rtti_type;
  table_node.is_primary_vtable = table.view_offset == 0;
  table_node.is_virtual_base_subobject = is_virtual_base_view(info, table);
  table_node.uses_extended_vtable_layout = table.use_extended_layout;
  set_callsem_uint_value(table_node, table.view_offset);
  append_dump_virtual_base_layout(table_node, &info);
  if(table_symbol) {
    set_dump_symbol(table_node, *table_symbol);
  } else if(table_node.is_primary_vtable &&
            rtti_type &&
            strip_top_level_cv(rtti_type) &&
            strip_top_level_cv(rtti_type)->kind == Type::TK_NAMED) {
    const TypePtr rtti_base = strip_top_level_cv(rtti_type);
    const string object_symbol = symbol_linkage::vtable_object_symbol_for_type(rtti_base);
    if(object_symbol.empty()) {
      throw logic_error("failed to mangle vtable symbol for " + describe_type(rtti_base));
    }
    string internal_symbol =
        symbol_linkage::internal_symbol_from_name(table.key + "::vtable");
    if(symbol_linkage::type_needs_structural_internal_symbol(rtti_base)) {
      const string structural_symbol =
          symbol_linkage::internal_symbol_from_type_encoding("__vtable_type",
                                                             rtti_base);
      if(!structural_symbol.empty()) {
        internal_symbol = structural_symbol;
      }
    }
    set_dump_symbol(
        table_node,
        symbol_linkage::make_object_symbol_identity(
            internal_symbol,
            object_symbol,
            primary_vtable_linkage));
  }
  if(table_node.is_primary_vtable) {
    ClassInfo * rtti_info = rtti_type ? ctx.class_info_for_type(rtti_type) : nullptr;
    append_rtti_base_output_nodes(rtti_info ? *rtti_info : info, table_node);
  }
  for(size_t i = 0; i < table.slots.size(); ++i) {
    const VTableSlotInfo & slot = table.slots[i];
    FunctionBinding * base_virtual =
        table.view_type && i < table.view_type->vtable_entries.size() ?
            table.view_type->vtable_entries[i] :
            nullptr;
    FunctionBinding * slot_function =
        semantic_template_function::acquire_required_function_definition_binding(
            ctx, slot.function, *info.member_scope);
    slot_function = resolve_output_function_binding(ctx, slot_function);
    if(!slot_function) {
      throw logic_error("missing vtable slot function");
    }
    const bool slot_is_pure_virtual =
        is_pure_virtual_function_binding(*slot_function);
    ctx.require_function_definition(slot_function,
                                    OutputReason::VTableSlot,
                                    !slot_function->is_deleted && !slot_is_pure_virtual);
    DumpNode entry_node = make_dump_node(CallSemKind::vtable_entry, slot_function->name);
    entry_node.semantic_type =
        base_virtual && base_virtual->type ? base_virtual->type : slot_function->type;
    if(slot_is_pure_virtual) {
      set_callsem_symbol(
          entry_node,
          symbol_linkage::make_c_function_symbol_identity("__cxa_pure_virtual"));
    } else {
      const symbol_linkage::SpecialMemberEntryPointKind entry_point_kind =
          is_secondary_virtual_destructor_slot(*slot_function, base_virtual, i) ?
              symbol_linkage::SMEK_DELETING :
              symbol_linkage::SMEK_COMPLETE;
      set_dump_symbol(entry_node, emitted_vtable_entry_symbol(*slot_function,
                                                              base_virtual,
                                                              i));
      set_dump_qualified_name_syntax_from_function_binding(entry_node, *slot_function);
      entry_node.is_virtual_member_function = slot_function->is_method;
      entry_node.is_constructor = slot_function->is_constructor;
      entry_node.is_destructor = slot_function->is_destructor;
      entry_node.is_conversion_operator = slot_function->is_conversion_operator;
      entry_node.is_const_method = slot_function->is_const_method;
      entry_node.is_volatile_method = slot_function->is_volatile_method;
      set_callsem_abi_tags(entry_node, function_binding_abi_tags(*slot_function));
      const symbol_linkage::FunctionRefQualifier ref_qualifier =
          symbol_linkage_ref_qualifier(slot_function->ref_qualifier);
      if(ref_qualifier != symbol_linkage::FRQ_NONE) {
        entry_node.has_function_ref_qualifier = true;
        set_callsem_function_ref_qualifier(entry_node, ref_qualifier);
      }
      if(slot_function->is_constructor || slot_function->is_destructor) {
        entry_node.has_special_member_entry_point_kind = true;
        set_callsem_special_member_entry_point_kind(entry_node, entry_point_kind);
      }
    }
    set_callsem_uint_value(entry_node, i);
    if(table_node.uses_extended_vtable_layout || slot.this_adjust != 0) {
      set_callsem_int_value(entry_node, slot.this_adjust);
    }
    if(base_virtual) {
      if(table_node.uses_extended_vtable_layout && base_virtual->is_destructor) {
        set_callsem_resolved_name(entry_node, base_virtual->name);
      }
      const long long result_adjust =
          compute_vtable_entry_result_adjustment(ctx, *base_virtual, *slot_function);
      if(result_adjust != 0) {
        entry_node.has_result_adjust = true;
        set_callsem_result_adjust(entry_node, result_adjust);
      }
    }
    table_node.children.push_back(std::move(entry_node));
  }
  out.children.push_back(std::move(table_node));
}

void analyze_vtt_output(SemanticContext & ctx,
                        ClassInfo & info,
                        DumpNode & out)
{
  if(!class_needs_vtt(info)) {
    return;
  }

  const string vtt_object_symbol = symbol_linkage::vtt_object_symbol(info);
  if(vtt_object_symbol.empty()) {
    throw logic_error("failed to mangle VTT symbol for " + info.qualified_name);
  }

  vector<pair<string, unsigned long long> > entries;
  collect_vtt_entries(ctx, info, entries);
  if(entries.empty()) {
    return;
  }

  DumpNode vtt_node = make_dump_node(CallSemKind::vtt_definition, info.qualified_name);
  set_dump_symbol(vtt_node,
                  symbol_linkage::make_object_symbol_identity(
                      symbol_linkage::internal_symbol_from_name(info.qualified_name + "::__vtt"),
                      vtt_object_symbol,
                      symbol_linkage::SL_WEAK));
  for(size_t i = 0; i < entries.size(); ++i) {
    DumpNode entry_node = make_dump_node(CallSemKind::vtt_entry, entries[i].first);
    set_callsem_uint_value(entry_node, entries[i].second);
    vtt_node.children.push_back(std::move(entry_node));
  }
  out.children.push_back(std::move(vtt_node));
}

void analyze_vtable_output(SemanticContext & ctx,
                           OutputState & state,
                           ClassInfo & info,
                           DumpNode & out)
{
  const ClassInfo * output_owner =
      semantic_template_output_policy::effective_class_output_owner(info);
  const ClassOutputReadiness class_output_readiness =
      output_owner ? semantic_template_output_policy::class_output_readiness(ctx, info) :
                     ClassOutputReadiness();
  const VTableKeyFunctionDecision key_function =
      vtable_key_function_decision(ctx, info, class_output_readiness);
  const symbol_linkage::SymbolLinkage primary_vtable_linkage =
      key_function.kind == VTK_LOCAL ? symbol_linkage::SL_EXTERNAL :
                                       symbol_linkage::SL_WEAK;

  for(size_t table_index = 0; table_index < info.vtables.size(); ++table_index) {
    const VTableInfo & table = info.vtables[table_index];
    append_vtable_output_node(ctx,
                              state,
                              info,
                              table,
                              info.type,
                              nullptr,
                              primary_vtable_linkage,
                              out);
  }

  vector<VTableInfo> construction_tables;
  collect_construction_vtables(ctx, info, construction_tables);
  for(size_t i = 0; i < construction_tables.size(); ++i) {
    const VTableInfo & table = construction_tables[i];
    // Each construction-vtable section gets a distinct internal symbol derived
    // from its (dynamic, base, offset, section) key.  The Itanium `_ZTC`
    // mangling keys only on (dynamic, offset, base) and so collides between a
    // base's secondary section and a deeper base's primary section in a
    // multi-level diamond; an internal per-section symbol avoids that while
    // the recursive VTT references the same key.  (Hosted/cross-module interop,
    // which needs the canonical `_ZTC` group symbol, also requires modeling the
    // external base vtable layout and is handled separately.)
    const symbol_linkage::SymbolIdentity table_symbol =
        symbol_linkage::make_internal_symbol_identity(
            symbol_linkage::internal_symbol_from_name(table.key + "::vtable"),
            symbol_linkage::SL_WEAK);
    append_vtable_output_node(ctx,
                              state,
                              info,
                              table,
                              table.view_type ? table.view_type->type : info.type,
                              &table_symbol,
                              symbol_linkage::SL_WEAK,
                              out);
  }

  analyze_vtt_output(ctx, info, out);
}

void analyze_class_output_from_info_impl(SemanticContext & ctx,
                                         OutputState & state,
                                         ClassInfo & info,
                                         const CppAstNode & node,
                                         DumpNode & out)
{
  if(node.kind == CppAstKind::class_forward_declaration) {
    return;
  }
  const bool source_capture_output =
      witness::source_capture_enabled(ctx.template_witness_context());
  const bool has_required_or_friend_output =
      class_has_required_output(info) ||
      class_has_immediate_friend_definition_output(ctx, info, node);
  if(!info.complete &&
     !source_capture_output &&
     !has_required_or_friend_output) {
    return;
  }
  if(!info.complete) {
    ctx.complete_class_type(info.type);
  }
  if(!info.complete) {
    throw logic_error("class output requested for incomplete class " + info.qualified_name);
  }
  if(semantic_template_output_policy::implicit_instantiation_definition_suppressed(&info)) {
    return;
  }
  if(info.definition_output_emitted || info.definition_output_in_progress) {
    if(info.definition_output_emitted &&
       !info.definition_output_in_progress &&
       info.has_late_required_static_member_output) {
      analyze_required_class_static_member_output(ctx, state, info, out);
    }
    if(semantic_hotspot::enabled()) {
      semantic_hotspot::note_semantic_query("revisit_class_definition_output",
                                            info.qualified_name);
    }
    return;
  }
  ScopedDefinitionOutput output_guard(info.definition_output_in_progress,
                                      info.definition_output_emitted);

  const ClassInfo * output_owner =
      semantic_template_output_policy::effective_class_output_owner(info);
  ClassOutputReadiness class_output_readiness;
  bool class_output_readiness_loaded = false;
  const auto class_output_readiness_for_info =
      [&]() -> const ClassOutputReadiness &
      {
        if(!class_output_readiness_loaded) {
          if(output_owner) {
            class_output_readiness =
                semantic_template_output_policy::class_output_readiness(ctx, info);
          }
          class_output_readiness_loaded = true;
        }
        return class_output_readiness;
      };

  const bool emit_class_vtables =
      !info.vtables.empty() &&
      should_emit_class_vtables(ctx,
                                info,
                                output_owner,
                                class_output_readiness_for_info());
  if(emit_class_vtables) {
    analyze_required_vtable_output(ctx, state, info, out);
  }

  const MemberBindingByNodeMap bindings_by_node =
      build_class_member_binding_by_node_map(info);
  set<FunctionBinding *> emitted;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::class_key ||
       child.kind == CppAstKind::base_clause ||
       child.kind == CppAstKind::access_specifier) {
      continue;
    }
    if(child.kind == CppAstKind::simple_declaration) {
      analyze_class_simple_declaration_output(ctx,
                                             info,
                                             bindings_by_node,
                                             child,
                                             out,
                                             emitted);
      continue;
    }
    if(child.kind == CppAstKind::function_definition) {
      DIAG_CONTEXT("output_class_member_function_definition [" + node_text(child) + "]" +
                   ctx.source_location_for_node(child));
      const CppAstNode * specifiers = find_child_kind(child, CppAstKind::decl_specifier_seq);
      const CppAstNode * declarator = find_child_kind(child, CppAstKind::declarator);
      if(!specifiers || !declarator) {
        throw logic_error("member function definition missing children");
      }
      const bool is_friend_definition =
          any_of(specifiers->children.begin(), specifiers->children.end(),
                 [](const CppAstNode & spec)
                 { return node_has_simple_type(spec, KW_FRIEND); });
      if(is_friend_definition) {
        Scope * friend_scope = unqualified_friend_entity_scope(info);
        FunctionBinding * binding =
            friend_scope ?
                find_namespace_function_binding_by_node_recursive(*friend_scope, child) :
                nullptr;
        if(!binding) {
          throw logic_error("missing friend function binding");
        }
        if(!should_emit_free_function_definition(ctx, *binding)) {
          emitted.insert(binding);
          continue;
        }
        analyze_function_binding_output_impl(ctx,
                                             state,
                                             *binding->declaration_scope,
                                             *binding,
                                             out);
        emitted.insert(binding);
        continue;
      }
      if(FunctionBinding * binding =
             find_class_member_binding_by_node(bindings_by_node, child)) {
        if(!semantic_template_output_policy::should_emit_instantiated_class_method_definition(
               class_output_readiness_for_info(),
               *binding)) {
          emitted.insert(binding);
          continue;
        }
        analyze_function_binding_output_impl(ctx, state, *info.member_scope, *binding, out);
        emitted.insert(binding);
        continue;
      }

      PreparedClassMemberFunctionDefinition prepared;
      if(!prepare_class_member_function_definition(ctx, info, child, false, prepared)) {
        throw logic_error(string("unsupported member function definition ") + node_text(child));
      }

      FunctionBinding * binding = nullptr;
      if(prepared.is_static_member) {
        binding =
            ctx.find_exact_function(*info.member_scope, prepared.name, prepared.declared_type);
      } else {
        binding = ctx.find_equivalent_class_function(
            info, prepared.name,
            method_function_type(info.type,
                                 prepared.method.syntax.is_const_method,
                                 prepared.method.syntax.is_volatile_method,
                                 prepared.declared_type),
            prepared.method.syntax.ref_qualifier);
      }
      if(!binding) {
        ostringstream outmsg;
        outmsg << "missing member function binding";
        outmsg << " [class " << info.qualified_name << "]";
        outmsg << " [member " << prepared.name << "]";
        outmsg << " [declared_type " << describe_type(prepared.declared_type) << "]";
        append_named_function_candidates(outmsg, *info.member_scope, prepared.name);
        throw logic_error(outmsg.str());
      }
      const bool should_emit_definition =
          semantic_template_output_policy::should_emit_instantiated_class_method_definition(
              class_output_readiness_for_info(),
              *binding);
      trace_class_output_decision("member-function",
                                  info,
                                  child,
                                  binding,
                                  true,
                                  should_emit_definition);
      if(!should_emit_definition) {
        emitted.insert(binding);
        continue;
      }
      analyze_function_binding_output_impl(ctx, state, *info.member_scope, *binding, out);
      emitted.insert(binding);
      continue;
    }
    if(child.kind == CppAstKind::special_member_definition ||
       child.kind == CppAstKind::special_member_declaration) {
      if(child.value.compare(0, 8, "operator") == 0 && child.value != "operator=") {
        analyze_conversion_operator_output(
            ctx,
            bindings_by_node,
            state,
            info,
            class_output_readiness_for_info(),
            child,
            out,
            emitted,
            child.kind == CppAstKind::special_member_definition);
        continue;
      }
      analyze_special_member_output(ctx,
                                    bindings_by_node,
                                    state,
                                    info,
                                    class_output_readiness_for_info(),
                                    child,
                                    out,
                                    emitted,
                                    child.kind == CppAstKind::special_member_definition,
                                    false);
      continue;
    }
    if(child.kind == CppAstKind::class_specifier ||
       child.kind == CppAstKind::class_forward_declaration) {
      if(child.value.empty()) {
        continue;
      }
      ClassInfo * nested = lookup_declared_class_info(ctx, *info.member_scope, child.value);
      if(!nested) {
        ostringstream outmsg;
        outmsg << "missing class info";
        outmsg << " [scope "
               << semantic_trace::scope_name_for_diagnostic(*info.member_scope)
               << "]";
        outmsg << " [nested " << child.value << "]";
        outmsg << semantic_trace::current_location_note(ctx, &child);
        throw logic_error(outmsg.str());
      }
      if(class_has_required_member_output(*nested)) {
        analyze_class_output_from_info_impl(ctx, state, *nested, child, out);
      } else {
        // Preserve the source-order nested-definition scan without forcing
        // unused nested template members to instantiate.
        nested->definition_output_emitted = true;
      }
      continue;
    }
  }

  for(map<string, vector<FunctionBinding *> >::iterator it = info.methods.begin();
      it != info.methods.end(); ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const bool should_emit_synthesized =
          it->second[i]->synthesized &&
          !it->second[i]->is_deleted &&
          has_output_requirement(it->second[i]->output_requirements, ORK_DEFINITION);
      const bool should_emit_defaulted =
          it->second[i]->is_defaulted &&
          semantic_template_output_policy::should_emit_instantiated_class_method_definition(
              class_output_readiness_for_info(),
              *it->second[i]);
      if(!emitted.count(it->second[i]) &&
         (should_emit_synthesized || should_emit_defaulted)) {
        analyze_function_binding_output_impl(ctx, state, *info.member_scope, *it->second[i], out);
      }
    }
  }

  analyze_required_class_static_member_output(ctx, state, info, out);
  output_guard.finish();
}

void analyze_function_definition(SemanticContext & ctx,
                                 OutputState & state,
                                 Scope & scope,
                                 const CppAstNode & node,
                                 DumpNode & out)
{
  const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
  const CppAstNode * body = find_function_body_node(node);
  if(!declarator || !body) {
    throw logic_error("function-definition missing declarator or body");
  }

  string name;
  TypePtr type;
  bool method_like_definition = false;
  {
    const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
    bool is_typedef = false;
    Scope * parse_scope = ctx.resolve_qualified_function_parse_scope(scope, *declarator);
    method_like_definition =
        parse_scope->class_info &&
        find_child_kind(*declarator, CppAstKind::parameter_clause) != nullptr;
    PreparedMethodParseContext prepared_method;
    prepare_method_parse_context(specifiers,
                                 *declarator,
                                 prepared_method,
                                 method_like_definition,
                                 true);
    TypePtr base;
    const CppAstNode filtered_declarator =
        ctx.filter_function_declarator(prepared_method.parse_declarator_node());
    if(!specifiers ||
       !ctx.parse_function_definition_base(*parse_scope,
                                          *prepared_method.parse_specifiers_node(),
                                          prepared_method.parse_declarator_node(),
                                          *body,
                                          prepared_method.has_method_syntax &&
                                              prepared_method.syntax.is_const_method,
                                          prepared_method.has_method_syntax &&
                                              prepared_method.syntax.is_volatile_method,
                                          is_typedef,
                                          base) ||
       is_typedef ||
       !ctx.parse_declarator(*parse_scope, filtered_declarator, base, name, type)) {
      ostringstream outmsg;
      outmsg << "unsupported function-definition";
      if(specifiers) {
        outmsg << " [specifiers " << node_text(*specifiers) << "]";
      }
      outmsg << " [declarator " << node_text(*declarator) << "]";
      outmsg << " [filtered " << node_text(filtered_declarator) << "]";
      throw logic_error(outmsg.str());
    }
  }

  Scope * function_definition_scope = &scope;
  string function_definition_name = name;
  const CppAstNode * function_identifier =
      find_descendant_kind(*declarator, CppAstKind::identifier);
  const QualifiedName * function_name_syntax =
      function_identifier ? cppast_qualified_name_syntax(*function_identifier) : nullptr;
  if(!method_like_definition && function_name_syntax &&
     !semantic_lookup::resolve_qualified_namespace_entity_target(ctx,
                                                                 scope,
                                                                 *function_name_syntax,
                                                                 function_definition_scope,
                                                                 function_definition_name)) {
    throw logic_error(string("missing qualified function definition target") +
                      semantic_trace::current_location_note(ctx, function_identifier));
  }

  FunctionBinding * binding =
      ctx.find_exact_function(*function_definition_scope, function_definition_name, type);
  if((!binding || !binding->has_definition) &&
     !ctx.resolve_out_of_class_method_binding_from_declarator_syntax(
          scope,
          name,
          function_identifier,
          type,
          declarator_is_const_method(*declarator),
          declarator_is_volatile_method(*declarator),
          declarator_ref_qualifier(*declarator),
          binding)) {
    throw logic_error("missing function binding");
  }
  if(!should_emit_free_function_definition(ctx, *binding)) {
    return;
  }
  analyze_function_binding_output_impl(ctx, state, scope, *binding, out);
}

void analyze_special_member_definition(SemanticContext & ctx,
                                       OutputState & state,
                                       Scope & scope,
                                       const CppAstNode & node,
                                       DumpNode & out)
{
  const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
  if(!declarator) {
    throw logic_error("special-member-definition missing declarator");
  }

  // The collection phase already resolved this definition's owner class and
  // target binding; reuse it instead of re-resolving the owner from text.
  if(FunctionBinding * collected = ctx.out_of_class_definition_binding(node)) {
    analyze_function_binding_output_impl(ctx, state, scope, *collected, out);
    return;
  }

  if(ctx.is_conversion_function_name(node.value)) {
    Scope * parse_scope = ctx.resolve_qualified_function_parse_scope(scope, *declarator);
    std::string member_name;
    TypePtr declared_type;
    std::vector<std::pair<std::string, TypePtr> > params;
    MethodSyntaxInfo syntax;
    if(!parse_conversion_operator_signature(ctx,
                                            *parse_scope,
                                            node,
                                            member_name,
                                            declared_type,
                                            params,
                                            nullptr,
                                            &syntax)) {
      throw logic_error("unsupported out-of-class conversion operator");
    }

    FunctionBinding * binding = nullptr;
    const CppAstNode * identifier = find_child(*declarator, CppAstKind::identifier);
    const QualifiedName * qualified_name =
        identifier ? cppast_qualified_name_syntax(*identifier) : nullptr;
    const bool resolved =
        qualified_name ?
            ctx.resolve_out_of_class_method_binding(scope,
                                                    *qualified_name,
                                                    declared_type,
                                                    syntax.is_const_method,
                                                    syntax.is_volatile_method,
                                                    syntax.ref_qualifier,
                                                    binding) :
            ctx.resolve_out_of_class_method_binding(scope,
                                                    node.value,
                                                    declared_type,
                                                    syntax.is_const_method,
                                                    syntax.is_volatile_method,
                                                    syntax.ref_qualifier,
                                                    binding);
    if(!resolved) {
      throw logic_error("missing conversion operator binding");
    }
    analyze_function_binding_output_impl(ctx, state, scope, *binding, out);
    return;
  }

  Scope * parse_scope = ctx.resolve_qualified_function_parse_scope(scope, *declarator);
  vector<pair<string, TypePtr> > params;
  const CppAstNode * parameter_clause = find_child_kind(*declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(*parse_scope, *parameter_clause, params, nullptr)) {
    throw logic_error("unsupported special-member parameter-clause");
  }

  FunctionBinding * binding = nullptr;
  if(!ctx.resolve_out_of_class_special_member_binding(scope, node.value, params, binding)) {
    throw logic_error("missing special member binding");
  }
  analyze_function_binding_output_impl(ctx, state, scope, *binding, out);
}

void analyze_namespace_definition_output(SemanticContext & ctx,
                                         OutputState & state,
                                         Scope & scope,
                                         const CppAstNode & node,
                                         DumpNode & out)
{
  Scope * target = nullptr;
  if(node.value == "<unnamed>") {
    target = ctx.find_named_namespace_child(scope, node.value);
  } else {
    target = resolve_direct_namespace(scope, node.value);
  }
  if(!target) {
    throw logic_error("missing namespace scope");
  }

  DumpNode namespace_node = make_dump_node(CallSemKind::namespace_definition, node.value);
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::inline_node) {
      namespace_node.is_inline_namespace = true;
      break;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::inline_node) {
      continue;
    }
    if(node.children[i].kind == CppAstKind::empty_declaration) {
      continue;
    }
    analyze_declaration_output_impl(ctx, state, *target, node.children[i], namespace_node, false);
  }

  out.children.push_back(std::move(namespace_node));
}

void analyze_declaration_output_impl(SemanticContext & ctx,
                                     OutputState & state,
                                     Scope & scope,
                                     const CppAstNode & node,
                                     DumpNode & out,
                                     bool is_c_linkage,
                                     bool linkage_has_braces)
{
  CppAstNode synthetic_decl;
  std::string synthetic_type_name;
  std::string synthetic_storage_name;
  if(synthesize_anonymous_union_storage_declaration(node,
                                                    synthetic_decl,
                                                    synthetic_type_name,
                                                    synthetic_storage_name)) {
    analyze_declaration_output_impl(ctx,
                                    state,
                                    scope,
                                    synthetic_decl,
                                    out,
                                    is_c_linkage,
                                    linkage_has_braces);
    return;
  }

  if(node.kind == CppAstKind::class_specifier ||
     node.kind == CppAstKind::class_forward_declaration) {
    if(node.value.empty()) {
      throw logic_error("anonymous classes unsupported");
    }
    ClassInfo * info = lookup_declared_class_info(ctx, scope, node.value);
    if(!info) {
      ostringstream outmsg;
      outmsg << "missing class info";
      outmsg << " [scope " << semantic_trace::scope_name_for_diagnostic(scope) << "]";
      outmsg << " [class " << node.value << "]";
      outmsg << semantic_trace::current_location_note(ctx, &node);
      throw logic_error(outmsg.str());
    }
    analyze_class_output_from_info_impl(ctx, state, *info, node, out);
    return;
  }
  if(node.kind == CppAstKind::enum_specifier) {
    return;
  }
  if(node.kind == CppAstKind::explicit_instantiation_declaration ||
     node.kind == CppAstKind::explicit_instantiation_definition) {
    return;
  }
  if(node.kind == CppAstKind::template_declaration) {
    if(is_c_linkage) {
      throw logic_error("extern \"C\" templates unsupported");
    }
    return;
  }
  if(node.kind == CppAstKind::deduction_guide_declaration) {
    return;
  }
  if(node.kind == CppAstKind::static_assert_declaration) {
    return;
  }
  if(node.kind == CppAstKind::simple_declaration) {
    const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
    const CppAstNode * declarators = find_child_kind(node, CppAstKind::init_declarator_list);
    if(!specifiers) {
      throw logic_error("namespace-scope simple-declaration missing decl-specifier-seq");
    }

    PreparedDeclarationSpecifiers prepared_specifiers;
    if(!ctx.prepare_namespace_scope_declaration_specifiers(scope,
                                                           *specifiers,
                                                           declarators,
                                                           false,
                                                           false,
                                                           prepared_specifiers)) {
      throw logic_error("unsupported namespace-scope embedded type-specifier");
    }
    emit_embedded_class_specifier_output(
        ctx, state, scope, prepared_specifiers.resolved_specifiers, out);

    if(!prepared_specifiers.parsed_decl_spec && !declarators) {
      throw logic_error("unsupported namespace-scope decl-specifier-seq");
    }

    if(!declarators) {
      return;
    }

    for(size_t i = 0; i < declarators->children.size(); ++i) {
      const CppAstNode & init_decl = declarators->children[i];
      string name;
      TypePtr type;
      const CppAstNode * initializer =
          init_decl.children.size() > 1 ? &init_decl.children[1] : nullptr;
      bool is_typedef = prepared_specifiers.declaration_is_typedef;
      FunctionBinding * rebound_function_binding =
          find_namespace_function_binding_by_node_recursive(scope, init_decl);
      const ValueBinding * rebound_value_binding =
          find_namespace_value_binding_by_node(scope, init_decl);
      Scope * parse_scope =
          semantic_lookup::resolve_qualified_variable_parse_scope(ctx,
                                                                  scope,
                                                                  init_decl.children[0]);
      CppAstNode stripped_function_style_declarator;
      CppAstNode synthesized_function_style_initializer;
      const CppAstNode * effective_declarator = &init_decl.children[0];
      if(!initializer &&
         rebound_value_binding &&
         rebound_value_binding->type &&
         strip_top_level_cv(rebound_value_binding->type)->kind != Type::TK_FUNCTION) {
        std::string recovery_error;
        if(semantic_parameter_recovery::recover_function_style_initializer_declarator(
               init_decl.children[0],
               stripped_function_style_declarator,
               synthesized_function_style_initializer,
               recovery_error)) {
          effective_declarator = &stripped_function_style_declarator;
          initializer = &synthesized_function_style_initializer;
        }
      }
      if(init_decl.children.empty() ||
         !ctx.parse_variable_declaration_type(*parse_scope,
                                             prepared_specifiers.resolved_specifiers,
                                             *effective_declarator,
                                             initializer, true, name, type, is_typedef)) {
        if(rebound_value_binding &&
           rebound_value_binding->type &&
           strip_top_level_cv(rebound_value_binding->type)->kind != Type::TK_FUNCTION) {
          name = rebound_value_binding->name;
          type = rebound_value_binding->type;
          is_typedef = false;
        } else if(rebound_function_binding) {
          name = rebound_function_binding->name;
          type = rebound_function_binding->type;
          is_typedef = false;
        } else {
          throw logic_error("unsupported namespace-scope declaration output");
        }
      }
      if(!rebound_value_binding && name.find("::") != string::npos) {
        QualifiedName qualified_name;
        if(semantic_utils::split_qualified_name_text(name, qualified_name)) {
          rebound_value_binding =
              semantic_lookup::lookup_qualified_value_binding(ctx, scope, qualified_name);
        }
      }

      if(is_typedef) {
        DumpNode alias_node = make_dump_node(CallSemKind::type_alias, name);
        alias_node.semantic_type = type;
        out.children.push_back(std::move(alias_node));
      } else if(type && strip_top_level_cv(type)->kind == Type::TK_FUNCTION) {
        if(single_special_initializer(initializer)) {
          FunctionBinding * method_binding = nullptr;
          const CppAstNode * function_identifier =
              find_descendant_kind(init_decl.children[0], CppAstKind::identifier);
          string out_of_class_lookup_name = name;
          if(function_identifier &&
             function_identifier->value.find("::") != string::npos &&
             function_identifier->value.find("operator") != string::npos &&
             out_of_class_lookup_name.find("operator") == string::npos) {
            out_of_class_lookup_name = function_identifier->value;
          }
          if(ctx.resolve_out_of_class_method_binding_from_declarator_syntax(
                 scope,
                 out_of_class_lookup_name,
                 function_identifier,
                 type,
                 declarator_is_const_method(init_decl.children[0]),
                 declarator_is_volatile_method(init_decl.children[0]),
                 declarator_ref_qualifier(init_decl.children[0]),
                 method_binding)) {
            analyze_function_binding_output_impl(ctx,
                                                 state,
                                                 method_binding->declaration_scope ?
                                                     *method_binding->declaration_scope :
                                                     scope,
                                                 *method_binding,
                                                 out);
            continue;
          }
        }
        FunctionBinding * binding = rebound_function_binding ?
            rebound_function_binding :
            find_namespace_function_binding_by_node(scope, name, init_decl);
        if(!binding) {
          const QualifiedName qualified_name =
              semantic_lookup::scope_qualified_name_syntax(scope, name);
          vector<FunctionBinding *> qualified =
              ctx.lookup_qualified_functions(scope, qualified_name);
          if(qualified.size() == 1) {
            binding = qualified[0];
          }
        }
        TypePtr emitted_type = binding ? binding->type : type;
        DumpNode decl_node = make_dump_node(CallSemKind::function_declaration,
                                            scope_qualified_name(scope, name));
        if(binding) {
          set_callsem_resolved_name(decl_node, function_output_name(*binding));
        }
        set_dump_qualified_name_syntax_from_scope(
            decl_node,
            binding && binding->declaration_scope ? binding->declaration_scope : &scope,
            binding ? binding->name : name);
        decl_node.semantic_type = emitted_type;
        decl_node.is_c_linkage = is_c_linkage;
        if(binding) {
          set_dump_symbol(decl_node, binding->symbol);
        }
        out.children.push_back(std::move(decl_node));
        if(binding) {
          binding->output_emitted = true;
        }
      } else {
        const ValueBinding * binding = rebound_value_binding ? rebound_value_binding :
                                                           ctx.lookup_value(scope, name);
        TypePtr emitted_type = binding ? binding->type : type;
        Scope * binding_scope =
            binding && binding->declaration_scope ?
                binding->declaration_scope :
                binding && binding->owner_class && binding->owner_class->member_scope ?
                    binding->owner_class->member_scope.get() :
                    nullptr;
        Scope * initializer_scope =
            binding && binding->constant_initializer_scope ? binding->constant_initializer_scope :
            binding_scope ? binding_scope : &scope;
        const bool is_definition =
            variable_declaration_is_definition(*specifiers,
                                               initializer,
                                               emitted_type,
                                               is_c_linkage,
                                               linkage_has_braces);
        if(!should_emit_namespace_variable_definition(ctx,
                                                      binding,
                                                      init_decl,
                                                      initializer,
                                                      is_definition)) {
          continue;
        }
        DumpNode var_node = make_dump_node(CallSemKind::variable, name);
        var_node.semantic_type = emitted_type;
        var_node.is_c_linkage = is_c_linkage;
        var_node.is_extern_declaration = !is_definition;
        var_node.is_thread_local = binding && binding->is_thread_local;
        var_node.is_static_storage = var_node.is_thread_local;
        set_dump_qualified_name_syntax_from_scope(
            var_node,
            binding_scope ? binding_scope : &scope,
            binding ? binding->name : name);
        if(binding) {
          set_callsem_resolved_name(var_node,
                                    canonical_variable_output_name(name, binding));
          symbol_linkage::SymbolIdentity symbol = binding->symbol;
          if(symbol.internal_symbol.empty() &&
             binding->owner_class &&
             binding->owner_class->member_scope) {
            symbol = static_member_variable_output_symbol_identity(
                *binding->owner_class,
                *binding);
          }
          if(binding->is_thread_local &&
             binding->owner_class &&
             symbol.thread_local_wrapper_object_symbol.empty()) {
            symbol.thread_local_wrapper_object_symbol =
                symbol_linkage::thread_local_wrapper_object_symbol_for_static_member_variable(
                    *binding->owner_class,
                    binding->name);
          }
          set_dump_symbol(var_node, symbol);
          if(binding->is_thread_local &&
             is_definition &&
             ctx.complete_class_type(emitted_type) &&
             !symbol.internal_symbol.empty()) {
            set_callsem_local_static_guard_symbol(
                var_node,
                symbol_linkage::thread_local_guard_internal_symbol(
                    symbol.internal_symbol));
          }
        }
        if(is_definition) {
          constant_eval::ConstexprValue constexpr_value;
          DumpNode literal_node;
          TypePtr constant_base = strip_top_level_cv(remove_reference_type(emitted_type));
          const std::string lifetime_object_name = binding ? binding->name : name;
          const bool has_constant_initializer =
              initializer &&
              !is_reference_type(emitted_type) &&
              constant_base &&
              !ctx.complete_class_type(emitted_type) &&
              ctx.evaluate_initializer_constant_value(*initializer_scope,
                                                     *initializer,
                                                     emitted_type,
                                                     constexpr_value) &&
              make_constexpr_scalar_literal_node(ctx,
                                                 constexpr_value,
                                                 emitted_type,
                                                 literal_node);
          if(has_constant_initializer) {
            var_node.children.push_back(std::move(literal_node));
          } else if(ctx.complete_class_type(emitted_type)) {
            semantic_lifetime::analyze_object_lifetime_actions(
                ctx,
                *initializer_scope,
                lifetime_object_name,
                emitted_type,
                initializer,
                var_node,
                ctx.source_location_for_name_in_node(init_decl, lifetime_object_name));
          } else if(initializer) {
            semantic_lifetime::analyze_initializer(
                ctx, *initializer_scope, emitted_type, *initializer, var_node);
          }
        }
        apply_local_static_guard_to_lifetime_actions(var_node);
        out.children.push_back(std::move(var_node));
      }
    }
    return;
  }

  if(node.kind == CppAstKind::alias_declaration) {
    TypePtr alias = ctx.lookup_type(scope, node.value);
    if(!alias) {
      throw logic_error("unknown alias");
    }
    DumpNode alias_node = make_dump_node(CallSemKind::type_alias, node.value);
    alias_node.semantic_type = alias;
    out.children.push_back(std::move(alias_node));
    return;
  }

  if(node.kind == CppAstKind::function_definition) {
    if(is_c_linkage) {
      FunctionBinding * binding = nullptr;
      const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
      if(!declarator) {
        throw logic_error("function-definition missing declarator or body");
      }

      string name;
      TypePtr type;
      {
        const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
        bool is_typedef = false;
        TypePtr base;
        const CppAstNode filtered_declarator = ctx.filter_function_declarator(*declarator);
        if(!specifiers || !ctx.parse_decl_spec(*specifiers, scope, is_typedef, base) || is_typedef ||
           !ctx.parse_declarator(scope, filtered_declarator, base, name, type)) {
          throw logic_error("unsupported function-definition");
        }
      }
      binding = ctx.find_exact_function(scope, name, type);
      if(!binding || !binding->has_definition) {
        throw logic_error("missing function binding");
      }
      if(!should_emit_free_function_definition(ctx, *binding)) {
        return;
      }
      analyze_function_binding_output_impl(ctx, state, scope, *binding, out);
      return;
    }
    analyze_function_definition(ctx, state, scope, node, out);
    return;
  }
  if(node.kind == CppAstKind::special_member_definition ||
     (node.kind == CppAstKind::special_member_declaration &&
      !ctx.is_conversion_function_name(node.value) &&
      find_child_kind(node, CppAstKind::special_definition))) {
    analyze_special_member_definition(ctx, state, scope, node, out);
    return;
  }

  if(node.kind == CppAstKind::namespace_definition) {
    if(is_c_linkage) {
      throw logic_error("extern \"C\" namespaces unsupported");
    }
    analyze_namespace_definition_output(ctx, state, scope, node, out);
    return;
  }
  if(node.kind == CppAstKind::linkage_specification) {
    const bool child_c_linkage = node.value == "C";
    for(size_t i = 0; i < node.children.size(); ++i) {
      analyze_declaration_output_impl(ctx,
                                      state,
                                      scope,
                                      node.children[i],
                                      out,
                                      child_c_linkage,
                                      node.linkage_has_braces);
    }
    return;
  }

  if(node.kind == CppAstKind::namespace_alias_definition ||
     node.kind == CppAstKind::using_directive ||
     node.kind == CppAstKind::using_declaration) {
    return;
  }

  throw logic_error("unsupported declaration in PA12 output");
}

}  // namespace

void analyze_function_binding_output(SemanticContext & ctx,
                                     OutputState & state,
                                     Scope & scope,
                                     FunctionBinding & binding,
                                     DumpNode & out)
{
  analyze_function_binding_output_impl(ctx, state, scope, binding, out);
}

void analyze_function_body_for_witness_semantics(SemanticContext & ctx,
                                                 Scope & scope,
                                                 FunctionBinding & binding)
{
  analyze_function_body_for_witness_semantics_impl(ctx, scope, binding);
}

void analyze_class_output_from_info(SemanticContext & ctx,
                                    OutputState & state,
                                    ClassInfo & info,
                                    const CppAstNode & node,
                                    DumpNode & out)
{
  analyze_class_output_from_info_impl(ctx, state, info, node, out);
}

void analyze_declaration_output(SemanticContext & ctx,
                                OutputState & state,
                                Scope & scope,
                                const CppAstNode & node,
                                DumpNode & out,
                                bool is_c_linkage,
                                bool linkage_has_braces)
{
  analyze_declaration_output_impl(ctx, state, scope, node, out, is_c_linkage, linkage_has_braces);
}

void analyze_instantiated_template_output(SemanticContext & ctx,
                                          OutputState & state,
                                          DumpNode & out)
{
  enum OutputAttempt
  {
    OAT_DONE,
    OAT_PENDING
  };

  std::vector<ClassInfo *> class_retry;
  std::vector<FunctionBinding *> function_retry;

  const auto try_emit_instantiated_class =
      [&](ClassInfo * info) -> OutputAttempt
      {
        semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
        if(counters) {
          ++counters->instantiated_class_output_scans;
        }
        const size_t previous_output_count = out.children.size();
        if(!info) {
          return OAT_DONE;
        }
        if(info->definition_output_emitted &&
           !info->definition_output_in_progress &&
           !info->has_late_required_static_member_output) {
          return OAT_DONE;
        }
        const CppAstNode * tracked_node =
            info->template_output_node ? info->template_output_node : info->class_node;
        if(!tracked_node || !info->member_scope ||
           ctx.scope_has_template_placeholders(*info->member_scope)) {
          return OAT_PENDING;
        }
        if(!info->complete &&
           !witness::source_capture_enabled(ctx.template_witness_context()) &&
           !class_has_required_output(*info) &&
           !class_has_immediate_friend_definition_output(ctx, *info, *tracked_node)) {
          return OAT_DONE;
        }
        ClassInfo * complete = ctx.complete_class_type(info->type);
        if(!complete || !complete->complete) {
          ostringstream outmsg;
          outmsg << "tracked instantiated class incomplete for output";
          outmsg << " [class " << info->qualified_name << "]";
          throw logic_error(outmsg.str());
        }
        const CppAstNode * emit_node =
            complete->template_output_node ? complete->template_output_node : complete->class_node;
        if(!emit_node) {
          return OAT_PENDING;
        }
        analyze_class_output_from_info_impl(ctx, state, *complete, *emit_node, out);
        if(counters && out.children.size() != previous_output_count) {
          ++counters->instantiated_class_output_emits;
        }
        return OAT_DONE;
      };

  const auto try_emit_instantiated_function =
      [&](FunctionBinding * binding) -> OutputAttempt
      {
        semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
        if(counters) {
          ++counters->instantiated_function_output_scans;
        }
        const size_t previous_output_count = out.children.size();
        binding = resolve_output_function_binding(ctx, binding);
        if(!binding) {
          return OAT_DONE;
        }
        if(binding->definition_output_emitted && !binding->definition_output_in_progress) {
          return OAT_DONE;
        }

        Scope * emit_scope = binding->declaration_scope;
        if(emit_scope &&
           semantic_template_output_policy::function_needs_template_definition_acquisition(
               *binding)) {
          FunctionBinding * upgraded =
              semantic_template_function::acquire_required_function_definition_binding(
                  ctx, binding, *emit_scope);
          if(upgraded) {
            binding = resolve_output_function_binding(ctx, upgraded);
            emit_scope = binding ? binding->declaration_scope : nullptr;
          }
        }
        if(!binding || !emit_scope) {
          return OAT_PENDING;
        }

        const bool definition_required =
            has_output_requirement(binding->output_requirements, ORK_DEFINITION);
        const bool declaration_node_is_definition_syntax =
            binding->declaration_node &&
            (binding->declaration_node->kind == CppAstKind::function_definition ||
             binding->declaration_node->kind == CppAstKind::special_member_definition);
        const bool tracked_template_has_body =
            semantic_template_output_policy::function_has_tracked_template_body(
                *binding,
                declaration_node_is_definition_syntax);
        const bool explicit_specialization_definition =
            binding->is_explicit_specialization &&
            binding->has_definition &&
            binding->body &&
            should_emit_free_function_definition(ctx, *binding);
        if(!definition_required &&
           !tracked_template_has_body &&
           !binding->synthesized &&
           !explicit_specialization_definition) {
          return OAT_PENDING;
        }

        const bool needs_late_definition =
            binding->has_definition &&
            has_output_requirement(binding->output_requirements, ORK_DEFINITION) &&
            !binding->definition_output_emitted;
        if(binding->output_emitted && !needs_late_definition) {
          return OAT_DONE;
        }

        if(!semantic_template_output_policy::function_instantiation_arguments_complete(ctx,
                                                                                      *binding)) {
          return OAT_PENDING;
        }

        analyze_function_binding_output_impl(ctx, state, *emit_scope, *binding, out);
        if(counters && out.children.size() != previous_output_count) {
          ++counters->instantiated_function_output_emits;
        }
        if(!binding->output_emitted && !binding->definition_output_emitted) {
          return OAT_PENDING;
        }
        return OAT_DONE;
      };

  if(!state.pending_instantiated_class_output.empty()) {
    std::vector<ClassInfo *> pending;
    pending.swap(state.pending_instantiated_class_output);
    for(size_t i = 0; i < pending.size(); ++i) {
      if(try_emit_instantiated_class(pending[i]) == OAT_PENDING) {
        class_retry.push_back(pending[i]);
      }
    }
  }
  if(!state.pending_instantiated_function_output.empty()) {
    std::vector<FunctionBinding *> pending;
    pending.swap(state.pending_instantiated_function_output);
    for(size_t i = 0; i < pending.size(); ++i) {
      if(try_emit_instantiated_function(pending[i]) == OAT_PENDING) {
        function_retry.push_back(pending[i]);
      }
    }
  }

  while(state.instantiated_class_output_index < state.instantiated_classes.size() ||
        state.instantiated_function_output_index < state.instantiated_functions.size()) {
    while(state.instantiated_class_output_index < state.instantiated_classes.size()) {
      ClassInfo * info = state.instantiated_classes[state.instantiated_class_output_index++];
      if(try_emit_instantiated_class(info) == OAT_PENDING) {
        class_retry.push_back(info);
      }
    }

    while(state.instantiated_function_output_index < state.instantiated_functions.size()) {
      FunctionBinding * binding =
          state.instantiated_functions[state.instantiated_function_output_index++];
      if(try_emit_instantiated_function(binding) == OAT_PENDING) {
        function_retry.push_back(binding);
      }
    }
  }

  state.pending_instantiated_class_output.swap(class_retry);
  state.pending_instantiated_function_output.swap(function_retry);
}

void analyze_synthetic_function_output(SemanticContext & ctx,
                                       OutputState & state,
                                       DumpNode & out)
{
  while(state.synthetic_function_output_index < state.synthetic_functions.size()) {
    FunctionBinding * binding =
        state.synthetic_functions[state.synthetic_function_output_index++];
    semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
    if(counters) {
      ++counters->synthetic_function_output_scans;
    }
    const size_t previous_output_count = out.children.size();
    if(!binding || !binding->declaration_scope) {
      continue;
    }
    if(binding->output_emitted) {
      continue;
    }
    analyze_function_binding_output_impl(ctx, state, *binding->declaration_scope, *binding, out);
    if(counters && out.children.size() != previous_output_count) {
      ++counters->synthetic_function_output_emits;
    }
  }
}

FunctionBinding * resolve_output_function_binding(SemanticContext & ctx,
                                                  FunctionBinding * binding)
{
  if(!binding || !ctx.function_binding_is_live(binding)) {
    return nullptr;
  }
  const string lookup_name =
      function_binding_output_lookup_name(*binding);
  const bool class_method_binding =
      binding->owner_class &&
      (binding->is_method || binding->is_constructor || binding->is_destructor);

  FunctionBinding * resolved = nullptr;
  if(class_method_binding && !lookup_name.empty()) {
    resolved = ctx.find_exact_class_function(*binding->owner_class,
                                             lookup_name,
                                             binding->type,
                                             binding->ref_qualifier);
  }
  if(!resolved) {
    resolved = ctx.find_function_by_symbol(binding->symbol, binding->name, binding->type);
  }
  if(!resolved) {
    return binding;
  }

  FunctionBinding * upgraded = nullptr;
  const bool resolved_class_method =
      resolved->owner_class &&
      (resolved->is_method || resolved->is_constructor || resolved->is_destructor);
  if(resolved_class_method) {
    if(!lookup_name.empty()) {
      upgraded = template_api::find_defined_class_function_matching_template_identity(
          ctx,
          *resolved->owner_class,
          lookup_name,
          *resolved);
    }
  } else if(!resolved->owner_class && resolved->declaration_scope) {
    const string lookup_name = function_binding_output_lookup_name(*resolved);
    if(!lookup_name.empty()) {
      upgraded = template_api::find_defined_function_matching_template_identity(
          ctx,
          *resolved->declaration_scope,
          lookup_name,
          *resolved);
    }
  }
  FunctionBinding * namespace_upgraded = nullptr;
  if(!resolved->owner_class) {
    namespace_upgraded =
        find_defined_namespace_function_for_output_binding(ctx,
                                                           *resolved,
                                                           lookup_name);
  }
  if(upgraded &&
     upgraded != resolved &&
     (upgraded->declaration_node || upgraded->definition_node || upgraded->body ||
      upgraded->has_definition)) {
    resolved = upgraded;
  } else if(namespace_upgraded) {
    resolved = namespace_upgraded;
  } else if(parser_trace::enabled("output.require") &&
            resolved->owner_class &&
            resolved->name.find("operator[]") != std::string::npos &&
            resolved->owner_class->qualified_name.find("_Map_base<") != std::string::npos) {
    std::ostringstream trace;
    trace << "resolve-output-binding-miss"
          << " owner=" << resolved->owner_class->qualified_name
          << " name=" << resolved->name
          << " display-name=" << resolved->display_name
          << " lookup-name=" << lookup_name
          << " refq=" << static_cast<int>(resolved->ref_qualifier);
    std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
        resolved->owner_class->methods.find(lookup_name);
    if(found == resolved->owner_class->methods.end()) {
      trace << " lookup-candidates=<none>";
    } else {
      trace << " lookup-candidates=" << found->second.size();
      for(size_t i = 0; i < found->second.size(); ++i) {
        FunctionBinding * candidate = found->second[i];
        trace << " [cand" << i
              << " name=" << candidate->name
              << " display=" << candidate->display_name
              << " has-def=" << (candidate->has_definition ? "yes" : "no")
              << " body=" << (candidate->body ? "yes" : "no")
              << " refq=" << static_cast<int>(candidate->ref_qualifier)
              << " type=" << describe_type(candidate->type)
              << "]";
      }
    }
    std::map<std::string, std::vector<FunctionBinding *> >::const_iterator op_found =
        resolved->owner_class->methods.find("operator[]");
    if(op_found != resolved->owner_class->methods.end() &&
       op_found != found) {
      trace << " op-candidates=" << op_found->second.size();
      for(size_t i = 0; i < op_found->second.size(); ++i) {
        FunctionBinding * candidate = op_found->second[i];
        trace << " [op" << i
              << " name=" << candidate->name
              << " display=" << candidate->display_name
              << " has-def=" << (candidate->has_definition ? "yes" : "no")
              << " body=" << (candidate->body ? "yes" : "no")
              << " refq=" << static_cast<int>(candidate->ref_qualifier)
              << " type=" << describe_type(candidate->type)
              << "]";
      }
    }
    parser_trace::note("output.require", std::string(), trace.str());
  }
  if(resolved && resolved->symbol.object_symbol.empty()) {
    FunctionBinding * exported =
        ctx.find_function_by_symbol(symbol_linkage::SymbolIdentity(),
                                    resolved->name,
                                    resolved->type);
    if(exported &&
       !exported->symbol.object_symbol.empty() &&
       (resolved->owner_class == nullptr || exported->owner_class == resolved->owner_class)) {
      resolved = exported;
    }
  }
  if(resolved != binding) {
    merge_output_function_binding_metadata(*resolved, *binding);
    resolved->output_requirements |= binding->output_requirements;
  }
  return resolved;
}

void analyze_late_required_function_output(SemanticContext & ctx,
                                           OutputState & state,
                                           DumpNode & out)
{
  enum OutputAttempt
  {
    OAT_DONE,
    OAT_PENDING
  };

  struct FunctionOutputAttempt
  {
    FunctionBinding * binding;
    OutputAttempt attempt;
  };

  const auto append_pending_unique =
      [](std::vector<FunctionBinding *> & pending, FunctionBinding * binding)
      {
        if(!binding) {
          return;
        }
        if(std::find(pending.begin(), pending.end(), binding) == pending.end()) {
          pending.push_back(binding);
        }
      };

  auto emit_required_free_function =
      [&](FunctionBinding * binding) -> FunctionOutputAttempt {
        semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
        if(counters) {
          ++counters->late_required_function_output_scans;
        }
        const size_t previous_output_count = out.children.size();
        binding = resolve_output_function_binding(ctx, binding);
        if(!binding ||
           binding->owner_class ||
           binding->is_deleted ||
           ((!has_output_requirement(binding->output_requirements, ORK_DEFINITION)) &&
            !binding->synthesized)) {
          return FunctionOutputAttempt{binding, OAT_DONE};
        }

        Scope * emit_scope = binding->declaration_scope;
        if(emit_scope &&
           semantic_template_output_policy::
               function_needs_source_template_definition_acquisition(*binding)) {
          FunctionBinding * upgraded =
              semantic_template_function::acquire_required_function_definition_binding(
                  ctx, binding, *emit_scope);
          if(upgraded) {
            binding = resolve_output_function_binding(ctx, upgraded);
            emit_scope = binding->declaration_scope;
          }
        }
        if(!binding || !emit_scope) {
          return FunctionOutputAttempt{binding, OAT_PENDING};
        }

        const bool needs_late_definition =
            binding->has_definition &&
            has_output_requirement(binding->output_requirements, ORK_DEFINITION) &&
            !binding->definition_output_emitted;
        if(binding->output_emitted && !needs_late_definition) {
          return FunctionOutputAttempt{binding, OAT_DONE};
        }

        analyze_function_binding_output_impl(ctx, state, *emit_scope, *binding, out);
        if(counters && out.children.size() != previous_output_count) {
          ++counters->late_required_function_output_emits;
        }
        if(!binding->output_emitted && !binding->definition_output_emitted) {
          return FunctionOutputAttempt{binding, OAT_PENDING};
        }
        return FunctionOutputAttempt{binding, OAT_DONE};
      };

  while(state.deferred_constexpr_function_output_index <
        state.deferred_constexpr_functions.size()) {
    FunctionBinding * binding =
        state.deferred_constexpr_functions[
            state.deferred_constexpr_function_output_index++];
    if(!ctx.function_binding_is_live(binding)) {
      continue;
    }
    semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
    if(counters) {
      ++counters->late_required_function_output_scans;
    }
    if((!has_output_requirement(binding->output_requirements, ORK_DEFINITION)) ||
       binding->definition_output_emitted) {
      continue;
    }
    if(!binding->declaration_scope) {
      append_pending_unique(state.pending_late_required_functions, binding);
      continue;
    }
    const size_t previous_output_count = out.children.size();
    analyze_function_binding_output_impl(ctx, state, *binding->declaration_scope, *binding, out);
    if(counters && out.children.size() != previous_output_count) {
      ++counters->late_required_function_output_emits;
    }
    if(!binding->output_emitted && !binding->definition_output_emitted) {
      append_pending_unique(state.pending_late_required_functions, binding);
    }
  }

  std::vector<FunctionBinding *> retry;
  if(!state.pending_late_required_functions.empty()) {
    std::vector<FunctionBinding *> pending;
    pending.swap(state.pending_late_required_functions);
    for(size_t i = 0; i < pending.size(); ++i) {
      FunctionOutputAttempt result = emit_required_free_function(pending[i]);
      if(result.attempt == OAT_PENDING) {
        append_pending_unique(retry, result.binding);
      }
    }
  }

  while(state.late_required_function_output_index <
        state.required_function_definitions.size()) {
    const size_t index = state.late_required_function_output_index++;
    FunctionOutputAttempt result =
        emit_required_free_function(state.required_function_definitions[index]);
    state.required_function_definitions[index] = result.binding;
    if(result.attempt == OAT_PENDING) {
      append_pending_unique(retry, result.binding);
    }
  }

  state.pending_late_required_functions.swap(retry);
}

void analyze_late_required_synthesized_output(SemanticContext & ctx,
                                              OutputState & state,
                                              DumpNode & out)
{
  enum OutputAttempt
  {
    OAT_DONE,
    OAT_PENDING
  };

  std::unordered_map<ClassInfo *, ClassOutputReadiness> class_output_readiness_cache;
  const auto class_output_readiness_for =
      [&](ClassInfo & info) -> const ClassOutputReadiness &
      {
        std::unordered_map<ClassInfo *, ClassOutputReadiness>::iterator cached =
            class_output_readiness_cache.find(&info);
        if(cached != class_output_readiness_cache.end()) {
          return cached->second;
        }
        ClassOutputReadiness readiness =
            semantic_template_output_policy::class_output_readiness(ctx, info);
        return class_output_readiness_cache.insert(
            std::make_pair(&info, readiness)).first->second;
      };

  const auto append_pending_unique =
      [](std::vector<FunctionBinding *> & pending, FunctionBinding * binding)
      {
        if(!binding) {
          return;
        }
        if(std::find(pending.begin(), pending.end(), binding) == pending.end()) {
          pending.push_back(binding);
        }
      };

  auto try_emit_required_class_static_function =
      [&](FunctionBinding * binding) -> OutputAttempt
      {
        semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
        if(counters) {
          ++counters->late_synthesized_static_function_scans;
        }
        if(!ctx.function_binding_is_live(binding)) {
          return OAT_DONE;
        }
        FunctionBinding * requested_binding = binding;
        if(!binding || !binding->owner_class || binding->is_method ||
           binding->is_constructor || binding->is_destructor) {
          return OAT_DONE;
        }
        if(!has_output_requirement(binding->output_requirements, ORK_DEFINITION) ||
           binding->is_deleted ||
           binding->definition_output_emitted) {
          return OAT_DONE;
        }

        ClassInfo * binding_owner = binding->owner_class;
        Scope * emit_scope =
            binding_owner && binding_owner->member_scope ?
                binding_owner->member_scope.get() :
                binding->declaration_scope;
        if(emit_scope &&
           semantic_template_output_policy::
               function_needs_template_definition_acquisition(*binding)) {
          FunctionBinding * upgraded =
              semantic_template_function::acquire_required_function_definition_binding(
                  ctx, binding, *emit_scope);
          if(upgraded) {
            upgraded->output_requirements |= binding->output_requirements;
            upgraded->is_explicit_instantiation_definition =
                upgraded->is_explicit_instantiation_definition ||
                binding->is_explicit_instantiation_definition;
            binding = upgraded;
          }
        }
        if(!binding ||
           binding->owner_class != binding_owner ||
           binding->is_method ||
           binding->is_constructor ||
           binding->is_destructor ||
           !has_output_requirement(binding->output_requirements, ORK_DEFINITION) ||
           binding->is_deleted ||
           binding->definition_output_emitted ||
           !binding->has_definition) {
          return OAT_DONE;
        }
        if(!emit_scope) {
          return OAT_PENDING;
        }
        const ClassOutputReadiness & class_output_readiness =
            class_output_readiness_for(*binding_owner);
        if(!semantic_template_output_policy::
               should_emit_instantiated_class_method_definition(
                   class_output_readiness, *binding)) {
          return OAT_PENDING;
        }
        const size_t previous_output_count = out.children.size();
        analyze_function_binding_output_impl(ctx, state, *emit_scope, *binding, out);
        if(counters && out.children.size() != previous_output_count) {
          ++counters->late_synthesized_static_function_emits;
        }
        if(requested_binding && requested_binding != binding) {
          requested_binding->output_emitted =
              requested_binding->output_emitted || binding->output_emitted;
          requested_binding->definition_output_emitted =
              requested_binding->definition_output_emitted ||
              binding->definition_output_emitted;
        }
        if(!binding->output_emitted && !binding->definition_output_emitted) {
          return OAT_PENDING;
        }
        return OAT_DONE;
      };

  auto try_emit_required_class_method =
      [&](FunctionBinding * binding) -> OutputAttempt
      {
        semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
        if(counters) {
          ++counters->late_synthesized_method_scans;
        }
        if(!ctx.function_binding_is_live(binding)) {
          return OAT_DONE;
        }
        FunctionBinding * requested_binding = binding;
        const size_t previous_output_count = out.children.size();
        if(!binding || !binding->owner_class ||
           (!binding->is_method && !binding->is_constructor && !binding->is_destructor)) {
          return OAT_DONE;
        }
        const bool declaration_required =
            has_output_requirement(binding->output_requirements, ORK_DECLARATION);
        const bool definition_required =
            has_output_requirement(binding->output_requirements, ORK_DEFINITION);
        if(!definition_required) {
          if(declaration_required && !binding->is_deleted && !binding->output_emitted) {
            analyze_function_declaration_output(ctx, *binding, out);
            if(counters && out.children.size() != previous_output_count) {
              ++counters->late_synthesized_method_emits;
            }
          }
          return OAT_DONE;
        }
        if(binding->is_deleted) {
          return OAT_DONE;
        }
        if(binding->output_emitted &&
           (!binding->has_definition || binding->definition_output_emitted)) {
          return OAT_DONE;
        }

        ClassInfo * binding_owner = binding->owner_class;
        Scope * binding_emit_scope =
            binding_owner && binding_owner->member_scope ?
                binding_owner->member_scope.get() :
                binding->declaration_scope;
        if(binding_emit_scope &&
           semantic_template_output_policy::
               function_needs_template_definition_acquisition(*binding)) {
          binding =
              semantic_template_function::acquire_required_function_definition_binding(
                  ctx, binding, *binding_emit_scope);
        }
        binding = resolve_output_function_binding(ctx, binding);
        binding_owner = binding && binding->owner_class ? binding->owner_class : binding_owner;
        binding_owner = canonicalize_output_class_info(ctx, binding_owner);
        if(binding &&
           binding_owner &&
           binding->owner_class != binding_owner &&
           (binding->is_method || binding->is_constructor || binding->is_destructor) &&
           !function_binding_output_lookup_name(*binding).empty()) {
          const string owner_lookup_name = function_binding_output_lookup_name(*binding);
          FunctionBinding * owner_binding =
              template_api::find_defined_class_function_matching_template_identity(
                  ctx,
                  *binding_owner,
                  owner_lookup_name,
                  *binding);
          if(!owner_binding &&
             template_api::function_binding_has_empty_template_identity(*binding)) {
            owner_binding = ctx.find_equivalent_class_function(*binding_owner,
                                                               owner_lookup_name,
                                                               binding->type,
                                                               binding->ref_qualifier);
          }
          if(owner_binding) {
            owner_binding->output_requirements |= binding->output_requirements;
            owner_binding->is_explicit_instantiation_definition =
                owner_binding->is_explicit_instantiation_definition ||
                binding->is_explicit_instantiation_definition;
            binding = owner_binding;
          }
        }
        binding_emit_scope =
            binding_owner && binding_owner->member_scope ?
                binding_owner->member_scope.get() :
                binding_emit_scope;
        const bool needs_late_definition =
            binding &&
            binding->has_definition &&
            has_output_requirement(binding->output_requirements, ORK_DEFINITION) &&
            !binding->definition_output_emitted;
        if(!binding || (binding->output_emitted && !needs_late_definition)) {
          return OAT_DONE;
        }
        if(!binding_owner || !binding_emit_scope) {
          return OAT_PENDING;
        }
        analyze_required_vtable_output(ctx, state, *binding_owner, out, binding);
        const ClassOutputReadiness & class_output_readiness =
            class_output_readiness_for(*binding_owner);
        if(!semantic_template_output_policy::should_emit_instantiated_class_method_definition(
               class_output_readiness, *binding)) {
          return OAT_PENDING;
        }
        analyze_function_binding_output_impl(ctx, state, *binding_emit_scope, *binding, out);
        if(counters && out.children.size() != previous_output_count) {
          ++counters->late_synthesized_method_emits;
        }
        if(requested_binding && requested_binding != binding) {
          requested_binding->output_emitted =
              requested_binding->output_emitted || binding->output_emitted;
          requested_binding->definition_output_emitted =
              requested_binding->definition_output_emitted ||
              binding->definition_output_emitted;
        }
        if(!binding->output_emitted && !binding->definition_output_emitted) {
          return OAT_PENDING;
        }
        return OAT_DONE;
      };

  std::vector<FunctionBinding *> method_retry;
  std::vector<FunctionBinding *> static_function_retry;

  if(!state.pending_late_required_class_methods.empty()) {
    std::vector<FunctionBinding *> pending;
    pending.swap(state.pending_late_required_class_methods);
    for(size_t i = 0; i < pending.size(); ++i) {
      if(try_emit_required_class_method(pending[i]) == OAT_PENDING) {
        append_pending_unique(method_retry, pending[i]);
      }
    }
  }
  if(!state.pending_late_required_class_static_functions.empty()) {
    std::vector<FunctionBinding *> pending;
    pending.swap(state.pending_late_required_class_static_functions);
    for(size_t i = 0; i < pending.size(); ++i) {
      if(try_emit_required_class_static_function(pending[i]) == OAT_PENDING) {
        append_pending_unique(static_function_retry, pending[i]);
      }
    }
  }

  while(state.late_synthesized_class_scan_index < state.classes.size()) {
    ClassInfo * info = state.classes[state.late_synthesized_class_scan_index++].get();
    if(!info) {
      continue;
    }
    const bool scan_methods = info->has_late_required_class_method_output;
    const bool scan_static_functions =
        info->has_late_required_class_static_function_output;
    if(!scan_methods && !scan_static_functions) {
      continue;
    }
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->late_synthesized_class_scans;
    }
    if(scan_methods) {
      for(map<string, vector<FunctionBinding *> >::const_iterator it = info->methods.begin();
          it != info->methods.end(); ++it) {
        for(size_t j = 0; j < it->second.size(); ++j) {
          FunctionBinding * binding = it->second[j];
          if(!binding ||
             !has_output_requirement(binding->output_requirements, ORK_DEFINITION)) {
            continue;
          }
          if(try_emit_required_class_method(binding) == OAT_PENDING) {
            append_pending_unique(method_retry, binding);
          }
        }
      }
    }
    if(scan_static_functions && info->member_scope) {
      Scope * emit_scope = info->member_scope.get();
      for(map<string, vector<FunctionBinding *> >::const_iterator it =
              emit_scope->function_sets.begin();
          it != emit_scope->function_sets.end();
          ++it) {
        for(size_t j = 0; j < it->second.size(); ++j) {
          FunctionBinding * binding = it->second[j];
          if(!binding ||
             !has_output_requirement(binding->output_requirements, ORK_DEFINITION)) {
            continue;
          }
          if(try_emit_required_class_static_function(binding) == OAT_PENDING) {
            append_pending_unique(static_function_retry, binding);
          }
        }
      }
    }
  }

  while(state.late_required_class_method_output_index <
        state.late_required_class_methods.size()) {
    FunctionBinding * binding =
        state.late_required_class_methods[state.late_required_class_method_output_index++];
    if(try_emit_required_class_method(binding) == OAT_PENDING) {
      append_pending_unique(method_retry, binding);
    }
  }
  while(state.late_required_class_static_function_output_index <
        state.late_required_class_static_functions.size()) {
    FunctionBinding * binding =
        state.late_required_class_static_functions[
            state.late_required_class_static_function_output_index++];
    if(try_emit_required_class_static_function(binding) == OAT_PENDING) {
      append_pending_unique(static_function_retry, binding);
    }
  }

  state.pending_late_required_class_methods.swap(method_retry);
  state.pending_late_required_class_static_functions.swap(static_function_retry);

  for(size_t i = 0; i < state.rtti_required_classes.size(); ++i) {
    ClassInfo * required = state.rtti_required_classes[i];
    if(!required) {
      continue;
    }
    ClassInfo & info = *required;
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->late_synthesized_rtti_scans;
    }
    if(info.rtti_required) {
      const size_t previous_output_count = out.children.size();
      analyze_required_vtable_output(ctx, state, info, out);
      if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
        if(out.children.size() != previous_output_count) {
          ++counters->late_synthesized_rtti_emits;
        }
      }
    }
  }

  for(set<string>::const_iterator it = state.emitted_rtti_symbols.begin();
      it != state.emitted_rtti_symbols.end(); ++it) {
    if(!state.emitted_rtti_output.insert(*it).second) {
      continue;
    }
    DumpNode node = make_dump_node(CallSemKind::rtti_definition, *it);
    auto type_it =
        state.emitted_rtti_types.find(*it);
    if(type_it != state.emitted_rtti_types.end()) {
      node.semantic_type = type_it->second;
      if(ClassInfo * info = ctx.class_info_for_type(type_it->second)) {
        if((info->is_polymorphic &&
            is_host_runtime_rtti_name_for_output(info->qualified_name)) ||
           (is_host_runtime_rtti_name_for_output(info->qualified_name) &&
            class_rtti_has_external_key_function(ctx, *info))) {
          continue;
        }
        append_dump_virtual_base_layout(node, info);
        append_rtti_base_output_nodes(*info, node);
      }
    }
    out.children.push_back(std::move(node));
  }
}

void expand_required_function_definition_closure(SemanticContext & ctx,
                                                 OutputState & state)
{
  while(state.required_function_definition_refresh_index <
        state.required_function_definitions.size()) {
    const size_t function_index = state.required_function_definition_refresh_index++;
    FunctionBinding * previous = state.required_function_definitions[function_index];
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->required_definition_refresh_scans;
    }
    FunctionBinding * binding = ctx.refresh_required_function_definition(
        state.required_function_definitions[function_index],
        false);
    state.required_function_definitions[function_index] = binding;
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      if(binding != previous) {
        ++counters->required_definition_refresh_updates;
      }
    }
    if(!binding || binding->is_deleted) {
      continue;
    }
  }
}

void expand_emitted_output_callee_closure(SemanticContext & ctx,
                                          OutputState & state,
                                          const CallSemNode & out)
{
  while(state.emitted_output_callee_scan_index < out.children.size()) {
    const CallSemNode & child = out.children[state.emitted_output_callee_scan_index++];
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->emitted_output_callee_top_level_scans;
    }
    note_required_callee_rescan(ctx, child);
    collect_required_callees_from_node(ctx, child);
  }
}

namespace {

std::string required_definition_validation_skip_reason(SemanticContext & ctx,
                                                       FunctionBinding & binding)
{
  if(binding.is_deleted) {
    return "deleted";
  }
  if(binding.owner_class &&
     is_function_local_class_info(*binding.owner_class)) {
    const bool implicit_like =
        binding.synthesized ||
        binding.is_defaulted ||
        (!binding.declaration_node && !binding.definition_node && !binding.body);
    if(binding.is_destructor &&
       implicit_like &&
       is_trivially_destructible_type(ctx, binding.owner_class->type)) {
      return "function-local-class trivial destructor";
    }
    if(binding.is_constructor &&
       binding.params.size() == 2 &&
       implicit_like &&
       (is_same_class_reference_parameter(binding.owner_class->type,
                                          binding.params[1].second,
                                          Type::TK_LVALUE_REFERENCE) ||
        is_same_class_reference_parameter(binding.owner_class->type,
                                          binding.params[1].second,
                                          Type::TK_RVALUE_REFERENCE)) &&
       is_trivially_copy_constructible_type(ctx, binding.owner_class->type)) {
      return "function-local-class trivial copy/move constructor";
    }
  }
  if(!semantic_template_output_policy::function_instantiation_arguments_complete(ctx, binding)) {
    std::ostringstream out;
    out << "template instantiation still output-dependent";
    if(binding.declaration_scope &&
       ctx.scope_has_template_placeholders(*binding.declaration_scope)) {
      const std::string origin =
          describe_scope_placeholder_origin(ctx, *binding.declaration_scope);
      if(!origin.empty()) {
        out << " placeholder-origin=" << origin;
      }
    }
    return out.str();
  }
  if(binding.owner_class) {
    const ClassOutputReadiness class_output_readiness =
        semantic_template_output_policy::class_output_readiness(ctx, *binding.owner_class);
    if(class_output_readiness.suppress_implicit_definition) {
      if(semantic_template_output_policy::
             function_obeys_implicit_instantiation_definition_suppression(binding)) {
        return "explicit instantiation suppression";
      }
    }
    if(semantic_template_output_policy::
           owner_class_instantiation_waits_for_non_dependent_arguments(
               ctx,
               *binding.owner_class)) {
      return "owner class instantiation not fully non-dependent";
    }
  }
  if(!binding.has_definition &&
     !binding.synthesized &&
     !binding.is_defaulted) {
    const bool declaration_node_is_definition_syntax =
        binding.declaration_node &&
        (binding.declaration_node->kind == CppAstKind::function_definition ||
         binding.declaration_node->kind == CppAstKind::special_member_definition);
    const bool declaration_only_function_template =
        semantic_template_output_policy::function_is_declaration_only_template(binding);
    const bool declaration_only_user_function =
        semantic_template_output_policy::function_is_declaration_only_user_function(
            binding,
            declaration_node_is_definition_syntax);
    if(binding.name.compare(0, 10, "__builtin_") == 0 ||
       binding.name.compare(0, 13, "__c11_atomic_") == 0 ||
       binding.name.compare(0, 9, "__atomic_") == 0 ||
       declaration_only_user_function ||
       declaration_only_function_template ||
       binding.is_c_linkage) {
      return "declaration-only function does not require emitted definition";
    }
  }
  return std::string();
}

std::string required_function_label(const FunctionBinding & binding)
{
  std::ostringstream out;
  out << binding.name;
  if(binding.owner_class) {
    out << " [owner " << binding.owner_class->qualified_name << "]";
  }
  const std::string template_trace_key =
      template_api::function_binding_template_trace_key(&binding);
  if(!template_trace_key.empty()) {
    out << " [inst " << template_trace_key << "]";
  }
  out << " [type " << describe_type(binding.type) << "]";
  return out.str();
}

std::string explain_function_output_state_impl(SemanticContext & ctx,
                                               FunctionBinding & binding)
{
  std::ostringstream out;
  out << "output-required="
      << (has_output_requirement(binding.output_requirements, ORK_DEFINITION) ? "yes" : "no")
      << " has-definition=" << (binding.has_definition ? "yes" : "no")
      << " definition-output-emitted=" << (binding.definition_output_emitted ? "yes" : "no")
      << " definition-output-in-progress="
      << (binding.definition_output_in_progress ? "yes" : "no")
      << " synthesized=" << (binding.synthesized ? "yes" : "no")
      << " deleted=" << (binding.is_deleted ? "yes" : "no");

  if(template_api::function_binding_has_source_template_identity(&binding)) {
    const bool instantiation_args_dependent =
        semantic_template_output_policy::function_instantiation_arguments_dependent(ctx, binding);
    const bool type_dependent = ctx.type_depends_on_template_parameter(binding.type);
    const bool scope_has_placeholders =
        binding.declaration_scope &&
        ctx.scope_has_template_placeholders(*binding.declaration_scope);
    const std::string signature_text = describe_type(binding.type);

    out << " source-template=yes"
        << " instantiation-args-dependent="
        << (instantiation_args_dependent ? "yes" : "no")
        << " instantiation-ready="
        << (semantic_template_output_policy::function_instantiation_arguments_complete(ctx, binding) ?
                "yes" :
                "no")
        << " type-dependent=" << (type_dependent ? "yes" : "no")
        << " scope-placeholders=" << (scope_has_placeholders ? "yes" : "no")
        << " signature-mentions-template="
        << (template_api::function_binding_signature_mentions_source_template_parameter(
                binding,
                signature_text) ?
                "yes" :
                "no");

    if(scope_has_placeholders) {
      const std::string origin =
          describe_scope_placeholder_origin(ctx, *binding.declaration_scope);
      if(!origin.empty()) {
        out << " placeholder-origin=" << origin;
      }
    }
  } else {
    out << " source-template=no";
  }

  if(binding.owner_class) {
    const ClassOutputReadiness readiness =
        semantic_template_output_policy::class_output_readiness(ctx, *binding.owner_class);
    out << " owner-class-ready=" << (readiness.complete ? "yes" : "no")
        << " owner-class-nondependent=" << (readiness.non_dependent ? "yes" : "no")
        << " owner-class-suppress-implicit="
        << (readiness.suppress_implicit_definition ? "yes" : "no")
        << " owner-class-placeholder-block="
        << (readiness.output_blocked_by_placeholders ? "yes" : "no");
  }

  const std::string skip_reason =
      required_definition_validation_skip_reason(ctx, binding);
  if(!skip_reason.empty()) {
    out << " validation-skip=" << skip_reason;
  }
  return out.str();
}

}  // namespace

std::string explain_function_output_state(SemanticContext & ctx,
                                          FunctionBinding & binding)
{
  return explain_function_output_state_impl(ctx, binding);
}

void validate_required_function_definition_closure(SemanticContext & ctx,
                                                   OutputState & state)
{
  for(size_t i = 0; i < state.required_function_definitions.size(); ++i) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->required_definition_validation_scans;
    }
    FunctionBinding * binding =
        resolve_output_function_binding(ctx, state.required_function_definitions[i]);
    state.required_function_definitions[i] = binding;
    if(!binding ||
       !has_output_requirement(binding->output_requirements, ORK_DEFINITION)) {
      continue;
    }
    if(binding->is_method && !binding->owner_class) {
      throw logic_error("required method missing owner class for output validation");
    }
    if(!witness::source_capture_enabled(ctx.template_witness_context()) &&
       binding->definition_output_emitted &&
       !binding->definition_output_in_progress) {
      continue;
    }

    Scope * emit_scope =
        binding->owner_class && binding->owner_class->member_scope ?
            binding->owner_class->member_scope.get() :
            binding->declaration_scope;
    if(binding->is_method && !emit_scope) {
      throw logic_error("required method missing emit scope for output validation");
    }

    FunctionBinding * resolved = binding;
    if(emit_scope) {
      FunctionBinding * upgraded =
          semantic_template_function::acquire_required_function_definition_binding(
              ctx, binding, *emit_scope);
      if(upgraded) {
        resolved = resolve_output_function_binding(ctx, upgraded);
      }
    }
    if(!resolved) {
      throw logic_error("required function validation resolved to null binding");
    }
    const std::string audit = explain_function_output_state_impl(ctx, *resolved);
    const std::string skip_reason =
        required_definition_validation_skip_reason(ctx, *resolved);
    if(!skip_reason.empty()) {
      if(parser_trace::enabled("output.audit")) {
        std::ostringstream trace;
        trace << "action=skip-required-definition-validation"
              << " function=" << resolved->name
              << " symbol=" << resolved->symbol.internal_symbol
              << " reason=" << skip_reason
              << " detail=" << audit;
        parser_trace::note("output.audit", std::string(), trace.str());
      }
      continue;
    }
    if(!resolved->has_definition && !resolved->synthesized) {
      throw logic_error("required function missing definition for output validation " +
                        required_function_label(*resolved) + " [audit " + audit + "]");
    }
    if(!resolved->definition_output_emitted) {
      if(parser_trace::enabled("output.audit")) {
        std::ostringstream trace;
        trace << "action=required-definition-not-emitted"
              << " function=" << resolved->name
              << " symbol=" << resolved->symbol.internal_symbol
              << " detail=" << audit;
        parser_trace::note("output.audit", std::string(), trace.str());
      }
      throw logic_error("required function definition not emitted " +
                        required_function_label(*resolved) + " [audit " + audit + "]");
    }
  }
}

namespace {

bool named_type_key_is_class_object(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base &&
         base->kind == Type::TK_NAMED &&
         (base->named_key.compare(0, 6, "class ") == 0 ||
          base->named_key.compare(0, 7, "struct ") == 0 ||
          base->named_key.compare(0, 6, "union ") == 0);
}

ClassInfo * output_class_info_for_named_type(SemanticContext & ctx,
                                             const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED ||
     base->named_key.compare(0, 5, "enum ") == 0 ||
     base->named_key.compare(0, 11, "enum class ") == 0 ||
     base->named_key.compare(0, 12, "enum struct ") == 0) {
    return nullptr;
  }

  if(ClassInfo * info = ctx.class_info_for_type(base)) {
    return info;
  }

  const string unprefixed =
      semantic_utils::strip_elaborated_type_prefix(base->named_key);
  if(unprefixed.empty() ||
     (unprefixed == base->named_key && named_type_key_is_class_object(base))) {
    return nullptr;
  }

  static const char * const prefixes[] = {"class ", "struct ", "union "};
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    TypePtr candidate(new Type(*base));
    candidate->named_key = string(prefixes[i]) + unprefixed;
    if(ClassInfo * info = ctx.class_info_for_type(candidate)) {
      return info;
    }
  }
  return nullptr;
}

void synchronize_output_named_type_layout(const TypePtr & type,
                                          const ClassInfo & info)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED || !info.type) {
    return;
  }
  TypePtr info_base = strip_top_level_cv(info.type);
  if(!info_base || info_base->kind != Type::TK_NAMED) {
    return;
  }
  const string base_key =
      semantic_utils::strip_elaborated_type_prefix(base->named_key);
  const string info_key =
      semantic_utils::strip_elaborated_type_prefix(info_base->named_key);
  if(base_key != info_key) {
    return;
  }
  base->named_display = info_base->named_display;
  base->named_key = info_base->named_key;
  base->named_semantic_kind = info_base->named_semantic_kind;
  base->named_semantic_payload = info_base->named_semantic_payload;
  base->named_complete = info_base->named_complete;
  base->named_has_layout = info_base->named_has_layout;
  base->named_alignment = info_base->named_alignment;
  base->named_size = info_base->named_size;
  base->named_is_empty = info_base->named_is_empty;
  base->named_host_abi_chunks = info_base->named_host_abi_chunks;
  base->named_lambda_mangle = info_base->named_lambda_mangle;
  base->named_class_template_specialization_mangle_info =
      info_base->named_class_template_specialization_mangle_info;
  base->named_member_owner_type = info_base->named_member_owner_type;
  base->named_member_name = info_base->named_member_name;
}

void complete_output_object_layout_type(SemanticContext & ctx,
                                        const TypePtr & type,
                                        std::set<std::string> & active,
                                        bool allow_completion)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return;
  }

  switch(base->kind) {
  case Type::TK_ARRAY:
    complete_output_object_layout_type(ctx, base->inner, active, allow_completion);
    return;

  case Type::TK_ATOMIC:
  case Type::TK_CV:
    complete_output_object_layout_type(ctx, base->inner, active, allow_completion);
    return;

  case Type::TK_NAMED:
  {
    if(base->named_has_layout) {
      return;
    }
    ClassInfo * info = output_class_info_for_named_type(ctx, base);
    if(!info && !named_type_key_is_class_object(base)) {
      return;
    }
    if(info) {
      if(info->complete) {
        synchronize_output_named_type_layout(base, *info);
        return;
      }
    }
    if(!allow_completion) {
      return;
    }
    if(!active.insert(base->named_key).second) {
      return;
    }
    ClassInfo * completed = info && info->type ?
        ctx.complete_class_type(info->type) :
        ctx.complete_class_type(base);
    active.erase(base->named_key);
    if(completed && completed->complete) {
      synchronize_output_named_type_layout(base, *completed);
    }
    return;
  }

  default:
    return;
  }
}

void complete_output_abi_layout_type(SemanticContext & ctx,
                                     const TypePtr & type,
                                     std::set<std::string> & active)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return;
  }
  if(base->kind == Type::TK_FUNCTION) {
    complete_output_abi_layout_type(ctx, base->inner, active);
    for(size_t i = 0; i < base->params.size(); ++i) {
      complete_output_abi_layout_type(ctx, base->params[i], active);
    }
    return;
  }
  if(is_reference_type(base)) {
    complete_output_object_layout_type(ctx, base->inner, active, false);
    return;
  }
  if(base->kind == Type::TK_POINTER || base->kind == Type::TK_BLOCK_POINTER) {
    complete_output_object_layout_type(ctx, base->inner, active, false);
    return;
  }
  if(base->kind == Type::TK_MEMBER_POINTER) {
    complete_output_object_layout_type(ctx, base->owner, active, false);
    complete_output_object_layout_type(ctx, base->inner, active, false);
    return;
  }
  complete_output_object_layout_type(ctx, base, active, true);
}

void complete_output_layout_types_impl(SemanticContext & ctx,
                                       CallSemNode & node,
                                       std::set<std::string> & active)
{
  if(node.kind != CallSemKind::type_alias) {
    complete_output_abi_layout_type(ctx, node.semantic_type, active);
    complete_output_abi_layout_type(ctx, callsem_vtt_owner_type(node), active);
    complete_output_abi_layout_type(ctx, callsem_materialization_source_type(node), active);
    complete_output_abi_layout_type(ctx, callsem_conversion_source_type(node), active);
    complete_output_abi_layout_type(ctx, callsem_initializer_list_element_type(node), active);
    complete_output_abi_layout_type(ctx, callsem_typeid_operand_type(node), active);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    complete_output_layout_types_impl(ctx, node.children[i], active);
  }
  if(callsem_lowered_condition_test(node)) {
    complete_output_layout_types_impl(ctx, *callsem_lowered_condition_test(node), active);
  }
}

}  // namespace

void complete_output_layout_types(SemanticContext & ctx,
                                  CallSemNode & out)
{
  std::set<std::string> active;
  complete_output_layout_types_impl(ctx, out, active);
}

}  // namespace semantic_output
