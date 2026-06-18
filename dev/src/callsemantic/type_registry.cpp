#include "callsemantic/type_registry.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "callsemantic_internal.h"
#include "callsemantic/source_location_utils.h"
#include "parser_trace.h"
#include "semantic_class_model.h"
#include "semantic_lookup.h"
#include "semantic_trace.h"
#include "template_api.h"
#include "witness_api.h"

namespace callsemantic {

using cpp_decl::describe_type;
using cpp_decl::make_named;
using cpp_decl::QualifiedName;
using cpp_decl::strip_top_level_cv;
using cpp_decl::Type;
using cpp_decl::TypePtr;
using semantic_model::ClassInfo;
using semantic_model::ClassTemplateDecl;
using semantic_model::FunctionBinding;
using semantic_model::Scope;

namespace {

bool source_location_in_template_header_context_for_tracking(
    const std::string & location)
{
  const template_api::TemplateWitnessSession * session =
      template_api::current_template_witness_session();
  if(!session || session->template_header_contexts.empty() || location.empty()) {
    return false;
  }
  const ParsedSourceLocation parsed =
      parse_source_location(
          template_api::normalize_template_witness_source_location(location));
  if(!parsed.valid) {
    return false;
  }
  for(std::size_t i = 0; i < session->template_header_contexts.size(); ++i) {
    const template_api::TemplateWitnessTemplateHeaderContext & context =
        session->template_header_contexts[i];
    const ParsedSourceLocation context_file =
        parse_source_location(
            template_api::normalize_template_witness_source_location(
                context.file + ":1:1"));
    const std::string normalized_context_file =
        context_file.valid ? context_file.file : context.file;
    if(normalized_context_file != parsed.file ||
       parsed.line < context.begin_line ||
       parsed.line > context.end_line) {
      continue;
    }
    if(parsed.line == context.begin_line &&
       context.begin_column > 0 &&
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

const FunctionBinding * enclosing_function(const Scope & current_scope)
{
  for(const Scope * current = &current_scope; current; current = current->parent) {
    if(current->function) {
      return current->function;
    }
  }
  return nullptr;
}

void mix_byte(unsigned long long & seed, unsigned char byte)
{
  seed ^= static_cast<unsigned long long>(byte);
  seed *= 1099511628211ULL;
}

void mix_u64(unsigned long long & seed, unsigned long long value)
{
  for(std::size_t i = 0; i < 8; ++i) {
    mix_byte(seed, static_cast<unsigned char>((value >> (i * 8)) & 0xFFU));
  }
}

void mix_text(unsigned long long & seed, const std::string & text)
{
  for(std::size_t i = 0; i < text.size(); ++i) {
    mix_byte(seed, static_cast<unsigned char>(text[i]));
  }
}

std::size_t stable_local_class_fingerprint(const Scope & current_scope,
                                           const std::string & local_name,
                                           const CppAstNode & node)
{
  unsigned long long seed = 1469598103934665603ULL;
  const FunctionBinding * function = enclosing_function(current_scope);
  if(function) {
    if(!function->symbol.object_symbol.empty()) {
      mix_text(seed, function->symbol.object_symbol);
    } else {
      mix_text(seed,
               semantic_model::function_binding_qualified_name_for_symbol(
                   *function));
      if(function->type) {
        mix_text(seed, describe_type(function->type));
      }
    }
  }
  mix_text(seed, local_name);
  mix_u64(seed, static_cast<unsigned long long>(node.token_start));
  mix_u64(seed, static_cast<unsigned long long>(node.token_end));
  return static_cast<std::size_t>(seed);
}

std::string append_symbol_member_name(const Scope & scope,
                                      const std::string & name)
{
  if(scope.class_info && !scope.class_info->qualified_name.empty()) {
    return scope.class_info->qualified_name + "::" + name;
  }
  return callsemantic_internal::normalize_qualified_name_spacing(
      semantic_lookup::scope_symbol_qualified_name(scope, name));
}

QualifiedName append_symbol_member_name_syntax(const Scope & scope,
                                               const std::string & name)
{
  if(scope.class_info &&
     !scope.class_info->symbol_qualified_name_syntax.name.empty()) {
    QualifiedName out = scope.class_info->symbol_qualified_name_syntax;
    out.qualifiers.push_back(out.name);
    out.name = name;
    return out;
  }
  return semantic_lookup::scope_symbol_qualified_name_syntax(scope, name);
}

}  // namespace

ClassInfo * class_info_for_type(const TypeRegistryState & state,
                                const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  auto found = state.classes_by_key.find(base->named_key);
  return found == state.classes_by_key.end() ? nullptr : found->second;
}

const semantic_model::ClassIndexMap &
template_named_class_index(const TypeRegistryState & state)
{
  return state.classes_by_key;
}

Scope * scope_for_type(const TypeRegistryState & state, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  auto found = state.type_scopes_by_key.find(base->named_key);
  return found == state.type_scopes_by_key.end() ? nullptr : found->second;
}

bool is_initializer_list_type(const TypeRegistryState & state,
                              const TypePtr & type,
                              TypePtr * element_type,
                              ClassInfo ** info_out)
{
  ClassInfo * info = class_info_for_type(state, type);
  if(!info || !info->is_initializer_list ||
     !info->initializer_list_element_type) {
    return false;
  }
  if(element_type) {
    *element_type = info->initializer_list_element_type;
  }
  if(info_out) {
    *info_out = info;
  }
  return true;
}

ClassInfo * create_class_info(TypeRegistryState & state,
                              const TypeRegistryCallbacks & callbacks,
                              Scope & scope,
                              const std::string & class_kind,
                              const std::string & name,
                              const CppAstNode * class_node)
{
  Scope & durable_scope = callbacks.persistent_enclosing_scope(scope);
  const std::string canonical_name =
      callsemantic_internal::normalize_qualified_name_spacing(name);
  const std::string source_qualified_name =
      callsemantic_internal::normalize_qualified_name_spacing(
          semantic_lookup::scope_qualified_name(durable_scope, canonical_name));
  const std::string symbol_qualified_name =
      append_symbol_member_name(durable_scope, canonical_name);
  QualifiedName symbol_qualified_name_syntax =
      append_symbol_member_name_syntax(durable_scope, canonical_name);
  const bool named_function_local_class =
      class_node &&
      semantic_lookup::current_function_scope(scope) != nullptr &&
      canonical_name.rfind("__local_type", 0) != 0;
  std::string qualified_name = symbol_qualified_name;
  if(named_function_local_class) {
    std::ostringstream unique_name;
    unique_name << "__local_"
                << (class_node->token_end > class_node->token_start ?
                        stable_local_class_fingerprint(
                            scope, source_qualified_name, *class_node) :
                        template_api::scope_template_instance_fingerprint(scope));
    const std::string local_suffix = unique_name.str();
    qualified_name = symbol_qualified_name + local_suffix;
    symbol_qualified_name_syntax.name += local_suffix;
  }
  const std::string type_key = std::string("class ") + qualified_name;
  const std::string display_name = class_kind + " " + source_qualified_name;

  TypePtr existing = callbacks.direct_named_type(scope, name);
  if(!existing && canonical_name != name) {
    existing = callbacks.direct_named_type(scope, canonical_name);
  }
  if(!existing && semantic_lookup::current_function_scope(scope) == nullptr) {
    auto registered = state.classes_by_key.find(type_key);
    if(registered != state.classes_by_key.end() &&
       registered->second &&
       registered->second->type) {
      existing = registered->second->type;
      scope.named_types[canonical_name] = existing;
      durable_scope.named_types[canonical_name] = existing;
      if(canonical_name != name) {
        scope.named_types[name] = existing;
        durable_scope.named_types[name] = existing;
      }
    }
  }
  if(existing) {
    TypePtr base = strip_top_level_cv(existing);
    if(!base || base->kind != Type::TK_NAMED || base->named_key != type_key) {
      throw std::logic_error(
          std::string("conflicting class binding") +
          callbacks.current_location_note(class_node) +
          callbacks.previous_class_location_note(
              "previous declaration",
              base ? class_info_for_type(state, base) : nullptr));
    }
    base->named_display = display_name;
    ClassInfo * info = class_info_for_type(state, base);
    if(!info) {
      std::ostringstream out;
      out << "missing class info";
      out << " [scope " << semantic_trace::scope_name_for_diagnostic(scope)
          << "]";
      out << " [name " << name << "]";
      out << " [type-key " << type_key << "]";
      out << " [existing-type " << describe_type(base) << "]";
      out << callbacks.current_location_note(class_node);
      throw std::logic_error(out.str());
    }
    if(class_node &&
       (!info->class_node ||
        (info->class_node->kind == CppAstKind::class_forward_declaration &&
         class_node->kind != CppAstKind::class_forward_declaration))) {
      info->class_node = class_node;
      info->is_final = class_node->is_final_specifier;
    }
    if(class_node && class_node->value.empty()) {
      info->source_is_unnamed_class = true;
      info->source_unnamed_class_node = class_node;
    }
    if(named_function_local_class) {
      info->source_is_named_function_local_class = true;
    }
    if(info->symbol_qualified_name_syntax.name.empty()) {
      info->symbol_qualified_name_syntax = symbol_qualified_name_syntax;
    }
    if(info->display_qualified_name.empty()) {
      info->display_qualified_name = source_qualified_name;
    }
    return info;
  }

  std::unique_ptr<ClassInfo> info(new ClassInfo());
  info->name = name;
  info->qualified_name = qualified_name;
  info->symbol_qualified_name_syntax = symbol_qualified_name_syntax;
  info->display_qualified_name = source_qualified_name;
  info->class_kind = class_kind;
  info->enclosing_scope = &durable_scope;
  info->class_node = class_node;
  if(class_node && class_node->value.empty()) {
    info->source_is_unnamed_class = true;
    info->source_unnamed_class_node = class_node;
  }
  info->source_is_named_function_local_class = named_function_local_class;
  info->is_final = class_node && class_node->is_final_specifier;
  info->default_access =
      callsemantic_internal::default_access_for_class_kind(class_kind);
  info->type = make_named(display_name, type_key, false);
  if(durable_scope.class_info && durable_scope.class_info->type) {
    TypePtr base = strip_top_level_cv(info->type);
    if(base && base->kind == Type::TK_NAMED) {
      base->named_member_owner_type = durable_scope.class_info->type;
      base->named_member_name = canonical_name;
    }
  }
  info->member_scope.reset(new Scope(&durable_scope, canonical_name, false));
  info->member_scope->class_info = info.get();
  info->name = canonical_name;
  info->member_scope->named_types[canonical_name] = info->type;
  scope.named_types[canonical_name] = info->type;
  durable_scope.named_types[canonical_name] = info->type;
  if(qualified_name != source_qualified_name &&
     qualified_name.find("::") == std::string::npos) {
    info->member_scope->named_types[qualified_name] = info->type;
    scope.named_types[qualified_name] = info->type;
    durable_scope.named_types[qualified_name] = info->type;
  }
  if(canonical_name != name) {
    info->member_scope->named_types[name] = info->type;
    scope.named_types[name] = info->type;
    durable_scope.named_types[name] = info->type;
  }
  state.classes_by_key[type_key] = info.get();
  ++state.classes_by_key_version;
  state.classes_by_key_epochs[type_key] = state.classes_by_key_version;
  state.classes.push_back(std::move(info));
  return state.classes.back().get();
}

ClassInfo * create_instantiated_class_info_with_internal_name(
    TypeRegistryState & state,
    const TypeRegistryCallbacks & callbacks,
    Scope & scope,
    const std::string & class_kind,
    const std::string & template_name,
    const std::string & specialization_name,
    const std::string & internal_specialization_name,
    ClassTemplateDecl * source_template,
    const CppAstNode * output_node,
    bool track_output)
{
  Scope & durable_scope = callbacks.persistent_enclosing_scope(scope);
  const std::string canonical_specialization_name =
      callsemantic_internal::normalize_qualified_name_spacing(
          specialization_name);
  const std::string canonical_internal_specialization_name =
      callsemantic_internal::normalize_qualified_name_spacing(
          internal_specialization_name);
  const std::string display_qualified_name =
      callsemantic_internal::normalize_qualified_name_spacing(
          semantic_lookup::scope_qualified_name(durable_scope,
                                                canonical_specialization_name));
  const std::string qualified_name =
      append_symbol_member_name(durable_scope,
                                canonical_internal_specialization_name);
  const QualifiedName symbol_qualified_name_syntax =
      append_symbol_member_name_syntax(durable_scope,
                                       canonical_internal_specialization_name);
  const std::string type_key = std::string("class ") + qualified_name;
  auto found = state.classes_by_key.find(type_key);
  if(found != state.classes_by_key.end()) {
    if(found->second->symbol_qualified_name_syntax.name.empty()) {
      found->second->symbol_qualified_name_syntax = symbol_qualified_name_syntax;
    }
    if(track_output && !found->second->template_instantiation_tracked) {
      found->second->template_instantiation_tracked = true;
      state.instantiated_classes.push_back(found->second);
    }
    return found->second;
  }

  std::unique_ptr<ClassInfo> info(new ClassInfo());
  info->name = template_name;
  info->qualified_name = qualified_name;
  info->symbol_qualified_name_syntax = symbol_qualified_name_syntax;
  info->display_qualified_name = display_qualified_name;
  info->class_kind = class_kind;
  info->enclosing_scope = &durable_scope;
  info->default_access =
      callsemantic_internal::default_access_for_class_kind(class_kind);
  info->source_template = source_template;
  info->template_output_node = output_node;
  info->is_final = output_node && output_node->is_final_specifier;
  info->template_instantiation_tracked = track_output;
  info->type = make_named(class_kind + " " + display_qualified_name,
                          type_key,
                          false);
  if(durable_scope.class_info && durable_scope.class_info->type) {
    TypePtr base = strip_top_level_cv(info->type);
    if(base && base->kind == Type::TK_NAMED) {
      base->named_member_owner_type = durable_scope.class_info->type;
      base->named_member_name = template_name;
    }
  }
  info->member_scope.reset(
      new Scope(&durable_scope, canonical_specialization_name, false));
  info->member_scope->class_info = info.get();
  info->member_scope->named_types[template_name] = info->type;
  scope.named_types[canonical_specialization_name] = info->type;
  durable_scope.named_types[canonical_specialization_name] = info->type;
  if(canonical_specialization_name != specialization_name) {
    scope.named_types[specialization_name] = info->type;
    durable_scope.named_types[specialization_name] = info->type;
  }
  state.classes_by_key[type_key] = info.get();
  ++state.classes_by_key_version;
  state.classes_by_key_epochs[type_key] = state.classes_by_key_version;
  state.classes.push_back(std::move(info));
  if(track_output) {
    state.instantiated_classes.push_back(state.classes.back().get());
  }
  return state.classes.back().get();
}

ClassInfo * create_instantiated_class_info(
    TypeRegistryState & state,
    const TypeRegistryCallbacks & callbacks,
    Scope & scope,
    const std::string & class_kind,
    const std::string & template_name,
    const std::string & specialization_name,
    ClassTemplateDecl * source_template,
    const CppAstNode * output_node,
    bool track_output)
{
  return create_instantiated_class_info_with_internal_name(state,
                                                          callbacks,
                                                          scope,
                                                          class_kind,
                                                          template_name,
                                                          specialization_name,
                                                          specialization_name,
                                                          source_template,
                                                          output_node,
                                                          track_output);
}

void track_instantiated_class(TypeRegistryState & state, ClassInfo * info)
{
  const std::string current_use_location = parser_trace::current_use_location();
  const std::string source_template_name =
      info->source_template ? info->source_template->name : info->name;
  const bool current_use_spells_instantiated_template =
      semantic_trace::source_location_points_at_identifier(
          current_use_location,
          source_template_name);
  const bool source_capture_header_track =
      witness::source_capture_enabled() &&
      source_location_in_template_header_context_for_tracking(
          current_use_location) &&
      !current_use_spells_instantiated_template;
  if(!info->template_instantiation_tracked) {
    info->source_capture_header_instantiation_tracked =
        source_capture_header_track;
    info->template_instantiation_tracked = true;
    state.instantiated_classes.push_back(info);
    if(info->has_late_required_static_member_output) {
      info->late_required_static_member_output_queued = true;
    }
    return;
  }
  if(!source_capture_header_track) {
    info->source_capture_header_instantiation_tracked = false;
  }
  if(info->has_late_required_static_member_output &&
     !info->late_required_static_member_output_queued) {
    info->late_required_static_member_output_queued = true;
    state.instantiated_classes.push_back(info);
  }
}

}  // namespace callsemantic
