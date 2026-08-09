#pragma once

#include "class_template_mangle_parameters.h"
#include <algorithm>
#include <cstdlib>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "parser_trace.h"
#include "class_template_mangle_info.h"
#include "resolved_source_semantics.h"
#include "semantic_context.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "template_api.h"
#include "template_service_interfaces.h"
#include "template_instantiation.h"
#include "template_selection_api.h"
#include "template_selection.h"
#include "template_witness.h"

namespace template_api {

inline std::string qualified_name_text(const cpp_decl::QualifiedName & qualified)
{
  std::ostringstream out;
  if(qualified.rooted) {
    out << "::";
  }
  for(std::size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    out << qualified.qualifiers[i] << "::";
  }
  out << qualified.name;
  return out.str();
}

inline const char * elaborated_type_keyword_text(TemplateElaboratedTypeKind kind)
{
  switch(kind) {
  case TETK_CLASS: return "class";
  case TETK_STRUCT: return "struct";
  case TETK_UNION: return "union";
  case TETK_ENUM: return "enum";
  case TETK_ENUM_CLASS: return "enum class";
  case TETK_ENUM_STRUCT: return "enum struct";
  case TETK_NONE: break;
  }
  return "";
}

inline semantic_model::Scope * root_scope(semantic_model::Scope & scope)
{
  semantic_model::Scope * current = &scope;
  while(current->parent) {
    current = current->parent;
  }
  return current;
}

inline bool scope_is_within(const semantic_model::Scope & scope,
                            const semantic_model::Scope * ancestor)
{
  for(const semantic_model::Scope * current = &scope; current; current = current->parent) {
    if(current == ancestor) {
      return true;
    }
  }
  return false;
}

inline cpp_decl::TypePtr lookup_direct_named_type_in_inline_namespaces_impl(
    semantic_model::Scope & scope,
    const std::string & name,
    std::set<const semantic_model::Scope *> & visited)
{
  if(!visited.insert(&scope).second) {
    return cpp_decl::TypePtr();
  }

  auto found = scope.named_types.find(name);
  if(found != scope.named_types.end()) {
    return found->second;
  }

  if(scope.namespace_scope) {
    for(std::size_t i = 0; i < scope.namespace_children.size(); ++i) {
      semantic_model::Scope & child = *scope.namespace_children[i];
      if(!child.inline_namespace && child.name != "<unnamed>") {
        continue;
      }
      cpp_decl::TypePtr nested =
          lookup_direct_named_type_in_inline_namespaces_impl(child, name, visited);
      if(nested) {
        return nested;
      }
    }
  }

  for(std::size_t i = 0; i < scope.using_directives.size(); ++i) {
    if(!scope.using_directives[i]) {
      continue;
    }
    cpp_decl::TypePtr nested = lookup_direct_named_type_in_inline_namespaces_impl(
        *scope.using_directives[i], name, visited);
    if(nested) {
      return nested;
    }
  }

  return cpp_decl::TypePtr();
}

inline cpp_decl::TypePtr lookup_direct_named_type_in_inline_namespaces(
    semantic_model::Scope & scope,
    const std::string & name)
{
  std::set<const semantic_model::Scope *> visited;
  return lookup_direct_named_type_in_inline_namespaces_impl(scope, name, visited);
}

inline cpp_decl::TypePtr lookup_named_type_for_semantic_use(
    TemplateTypeSystem & type_system,
    semantic_model::Scope & lexical_scope,
    semantic_model::Scope & target_scope,
    const std::string & name,
    bool ensure_current_reference_members = true)
{
  if(target_scope.class_info) {
    cpp_decl::TypePtr member;
    if(type_system.resolve_member_type_lookup(lexical_scope,
                                              *target_scope.class_info,
                                              name,
                                              ensure_current_reference_members,
                                              member)) {
      return member;
    }
    return cpp_decl::TypePtr();
  }
  return lookup_direct_named_type_in_inline_namespaces(target_scope, name);
}

inline semantic_model::Scope * resolve_direct_namespace_in_inline_namespaces_impl(
    semantic_model::Scope & scope,
    const std::string & name,
    std::set<const semantic_model::Scope *> & visited)
{
  if(!visited.insert(&scope).second) {
    return nullptr;
  }

  auto found =
      scope.namespace_bindings.find(name);
  if(found != scope.namespace_bindings.end()) {
    return found->second;
  }

  if(scope.namespace_scope) {
    for(std::size_t i = 0; i < scope.namespace_children.size(); ++i) {
      semantic_model::Scope & child = *scope.namespace_children[i];
      if(!child.inline_namespace && child.name != "<unnamed>") {
        continue;
      }
      semantic_model::Scope * nested =
          resolve_direct_namespace_in_inline_namespaces_impl(child, name, visited);
      if(nested) {
        return nested;
      }
    }
  }

  for(std::size_t i = 0; i < scope.using_directives.size(); ++i) {
    if(!scope.using_directives[i]) {
      continue;
    }
    semantic_model::Scope * nested = resolve_direct_namespace_in_inline_namespaces_impl(
        *scope.using_directives[i], name, visited);
    if(nested) {
      return nested;
    }
  }

  return nullptr;
}

inline semantic_model::Scope * resolve_direct_namespace_in_inline_namespaces(
    semantic_model::Scope & scope,
    const std::string & name)
{
  std::set<const semantic_model::Scope *> visited;
  return resolve_direct_namespace_in_inline_namespaces_impl(scope, name, visited);
}

inline bool elaborated_kind_matches_type(const TemplateSemanticModelView & model,
                                         TemplateElaboratedTypeKind kind,
                                         const cpp_decl::TypePtr & type)
{
  if(kind == TETK_NONE) {
    return true;
  }

  template_api::TemplateNamedTypeMetadata info;
  if(!describe_named_type_metadata(model, type, info)) {
    return false;
  }

  switch(kind) {
  case TETK_CLASS:
    return info.class_kind == "class";
  case TETK_STRUCT:
    return info.class_kind == "struct";
  case TETK_UNION:
    return info.class_kind == "union";
  case TETK_ENUM:
  case TETK_ENUM_CLASS:
  case TETK_ENUM_STRUCT:
    return info.class_kind == "enum";
  case TETK_NONE:
    break;
  }
  return false;
}

inline bool try_leaf_prepare_named_type_member_scope(TemplateTypeSystem & type_system,
                                                     TemplateEnvironmentHandle scope,
                                                     const cpp_decl::TypePtr & type,
                                                     semantic_model::Scope *& out)
{
  out = nullptr;
  if(!type) {
    return false;
  }
  if(type_system.prepare_named_type_member_scope(scope, type, out) && out) {
    return true;
  }

  semantic_model::ClassInfo * info =
      find_named_type_class_info(type_system.model, type);
  if(!info || !info->member_scope) {
    return false;
  }
  out = info->member_scope.get();
  return true;
}

struct ScopedTemplateDependentTypeExprUseLocation
{
  explicit ScopedTemplateDependentTypeExprUseLocation(
      const std::string & location)
    : active(!location.empty())
  {
    if(active) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedTemplateDependentTypeExprUseLocation()
  {
    if(active) {
      parser_trace::pop_use_location();
    }
  }

  ScopedTemplateDependentTypeExprUseLocation(
      const ScopedTemplateDependentTypeExprUseLocation &) = delete;
  ScopedTemplateDependentTypeExprUseLocation & operator=(
      const ScopedTemplateDependentTypeExprUseLocation &) = delete;

  bool active = false;
};

inline bool try_leaf_resolve_type_lookup(TemplateTypeSystem & type_system,
                                         const TemplateTypeLookupRequest & request,
                                         cpp_decl::TypePtr & out)
{
  out.reset();
  if(!request.scope || request.name.name.empty()) {
    return false;
  }

  const auto apply_request_cv =
      [&](const cpp_decl::TypePtr & type) -> cpp_decl::TypePtr
  {
    if(!type) {
      return type;
    }
    return (request.top_const || request.top_volatile) ?
        cpp_decl::apply_cv(type, request.top_const, request.top_volatile) :
        type;
  };
  const auto lookup_member_type =
      [&](semantic_model::Scope & lookup_scope,
          semantic_model::ClassInfo & owner,
          const std::string & name) -> cpp_decl::TypePtr
  {
    const bool ensure_current_reference_members =
        !(((owner.full_member_collection_in_progress ||
            owner.reference_member_collection_in_progress) &&
           owner.member_scope &&
           scope_is_within(lookup_scope, owner.member_scope.get())));
    cpp_decl::TypePtr member;
    if(!type_system.resolve_member_type_lookup(*request.scope,
                                               owner,
                                               name,
                                               ensure_current_reference_members,
                                               member) ||
       !member ||
       !elaborated_kind_matches_type(type_system.model,
                                     request.elaborated_kind,
                                     member)) {
      return cpp_decl::TypePtr();
    }
    return apply_request_cv(member);
  };

  if(request.name.rooted || !request.name.qualifiers.empty()) {
    semantic_model::Scope * current = request.name.rooted ?
        root_scope(*request.scope) :
        nullptr;
    std::size_t qualifier_index = 0;
    if(!request.name.qualifiers.empty() && !request.name.rooted) {
      const std::string & first = request.name.qualifiers[0];
      for(semantic_model::Scope * probe = request.scope; probe; probe = probe->parent) {
        semantic_model::Scope * direct_namespace =
            resolve_direct_namespace_in_inline_namespaces(*probe, first);
        if(direct_namespace) {
          current = direct_namespace;
          qualifier_index = 1;
          break;
        }

        cpp_decl::TypePtr qualifier_type =
            lookup_direct_named_type_in_inline_namespaces(*probe, first);
        if(!qualifier_type) {
          continue;
        }

        if(!try_leaf_prepare_named_type_member_scope(type_system,
                                                     make_template_environment(*probe),
                                                     qualifier_type,
                                                     current)) {
          return false;
        }
        qualifier_index = 1;
        break;
      }
      if(!current) {
        return false;
      }
    } else if(!current) {
      current = request.scope;
    }
    while(current && qualifier_index < request.name.qualifiers.size()) {
      semantic_model::Scope * direct_namespace =
          resolve_direct_namespace_in_inline_namespaces(
              *current, request.name.qualifiers[qualifier_index]);
      if(direct_namespace) {
        current = direct_namespace;
        ++qualifier_index;
        continue;
      }

      cpp_decl::TypePtr qualifier_type =
          lookup_direct_named_type_in_inline_namespaces(
              *current, request.name.qualifiers[qualifier_index]);
      if(!qualifier_type) {
        return false;
      }

      if(!try_leaf_prepare_named_type_member_scope(type_system,
                                                   make_template_environment(*request.scope),
                                                   qualifier_type,
                                                   current)) {
        return false;
      }
      ++qualifier_index;
    }

    if(!current) {
      return false;
    }

    if(current->class_info) {
      out = lookup_member_type(*current, *current->class_info, request.name.name);
      return out != nullptr;
    }

    cpp_decl::TypePtr direct =
        lookup_direct_named_type_in_inline_namespaces(*current, request.name.name);
    if(!direct ||
       !elaborated_kind_matches_type(type_system.model, request.elaborated_kind, direct)) {
      return false;
    }
    out = apply_request_cv(direct);
    return out != nullptr;
  }

  for(semantic_model::Scope * current = request.scope; current; current = current->parent) {
    cpp_decl::TypePtr direct =
        lookup_direct_named_type_in_inline_namespaces(*current, request.name.name);
    if(direct &&
       elaborated_kind_matches_type(type_system.model, request.elaborated_kind, direct)) {
      out = apply_request_cv(direct);
      return out != nullptr;
    }
    if(current->class_info) {
      out = lookup_member_type(*current, *current->class_info, request.name.name);
      if(out) {
        return true;
      }
    }
    const bool has_lexical_class =
        !current->class_info &&
        current->function &&
        current->function->lexical_access_class;
    if(has_lexical_class) {
      out = lookup_member_type(*current,
                               *current->function->lexical_access_class,
                               request.name.name);
      if(out) {
        return true;
      }
    }
  }

  return false;
}

enum RecursiveTemplateSemanticQueryOperation
{
  RTSQ_INITIALIZER = 1,
  RTSQ_DECLTYPE,
  RTSQ_TYPEOF,
  RTSQ_BUILTIN_TRAIT_BASE = 16
};

// A fixed nesting limit rejects valid trailing-decltype chains and can turn
// the first rejected inner query into exponential substitution retries.  Keep
// the active typed/source identity instead: only the same semantic query in
// the same instantiation scope is a cycle.  The high cap remains an emergency
// guard for a chain whose identities never repeat.
struct RecursiveTemplateSemanticQueryKey
{
  const semantic_model::Scope * scope = nullptr;
  unsigned operation = 0;
  CppAstKind node_kind = CppAstKind::invalid;
  std::size_t token_start = 0;
  std::size_t token_end = 0;
  uint32_t source_location_id = 0;
  const cpp_decl::Type * first_type = nullptr;
  const cpp_decl::Type * second_type = nullptr;
  std::size_t type_count = 0;

  bool operator==(const RecursiveTemplateSemanticQueryKey & other) const
  {
    return scope == other.scope &&
           operation == other.operation &&
           node_kind == other.node_kind &&
           token_start == other.token_start &&
           token_end == other.token_end &&
           source_location_id == other.source_location_id &&
           first_type == other.first_type &&
           second_type == other.second_type &&
           type_count == other.type_count;
  }
};

inline std::vector<RecursiveTemplateSemanticQueryKey> &
active_recursive_template_semantic_queries()
{
  static thread_local std::vector<RecursiveTemplateSemanticQueryKey> active;
  return active;
}

inline RecursiveTemplateSemanticQueryKey recursive_template_expression_query_key(
    const semantic_model::Scope * scope,
    unsigned operation,
    const CppAstNode & node,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr())
{
  RecursiveTemplateSemanticQueryKey key;
  key.scope = scope;
  key.operation = operation;
  key.node_kind = node.kind;
  key.token_start = node.token_start;
  key.token_end = node.token_end;
  key.source_location_id = node.source_location_id;
  key.first_type = target_type.get();
  key.type_count = target_type ? 1 : 0;
  return key;
}

struct ScopedRecursiveTemplateSemanticQuery
{
  static const std::size_t kMaxDepth = 256;

  bool active = false;

  explicit ScopedRecursiveTemplateSemanticQuery(
      const RecursiveTemplateSemanticQueryKey & key)
  {
    std::vector<RecursiveTemplateSemanticQueryKey> & queries =
        active_recursive_template_semantic_queries();
    if(queries.size() >= kMaxDepth ||
       std::find(queries.begin(), queries.end(), key) != queries.end()) {
      return;
    }
    queries.push_back(key);
    active = true;
  }

  ~ScopedRecursiveTemplateSemanticQuery()
  {
    if(!active) {
      return;
    }
    std::vector<RecursiveTemplateSemanticQueryKey> & queries =
        active_recursive_template_semantic_queries();
    if(!queries.empty()) {
      queries.pop_back();
    }
  }
};

class SemanticContextTemplateServices : public TemplateTypeSystem,
                                        public TemplateRecursiveSemanticGateway
{
public:
  explicit SemanticContextTemplateServices(SemanticContext & ctx)
    : ctx_(ctx)
  {
    model.classes_by_key = &ctx_.template_named_class_index();
  }

  bool prepare_named_type_member_scope(TemplateEnvironmentHandle scope,
                                       const cpp_decl::TypePtr & type,
                                       semantic_model::Scope *& out) override
  {
    out = nullptr;
    if(!scope.valid() || !type) {
      return false;
    }
    const semantic_metrics::ClassDemandKind demand =
        semantic_metrics::current_class_demand();
    if(semantic_metrics::AnalyzerCounters * counters =
           ctx_.performance_counters()) {
      ++counters->reference_member_scope_prepares;
      ++counters->reference_member_scope_prepare_by_demand[
          static_cast<std::size_t>(demand)];
    }
    semantic_model::ClassInfo * info = ctx_.class_info_for_type(type);
    if(!info) {
      return false;
    }
    if(!info->reference_members_collected &&
       !info->reference_type_members_collected &&
       !info->reference_member_collection_in_progress &&
       !(info->full_member_collection_in_progress &&
         info->member_scope &&
         scope_is_within(scope.require(), info->member_scope.get()))) {
      if(semantic_metrics::AnalyzerCounters * counters =
             ctx_.performance_counters()) {
        ++counters->reference_member_scope_ensures;
        ++counters->reference_member_scope_ensure_by_demand[
            static_cast<std::size_t>(demand)];
      }
      ctx_.ensure_class_reference_type_members(*info);
    }
    out = info->member_scope.get();
    return out != nullptr;
  }

  bool complete_named_type_member_scope(TemplateEnvironmentHandle scope,
                                        const cpp_decl::TypePtr & type,
                                        semantic_model::Scope *& out) override
  {
    out = nullptr;
    if(!scope.valid() || !type) {
      return false;
    }
    semantic_model::ClassInfo * info = ctx_.class_info_for_type(type);
    if(!info) {
      return false;
    }
    if(!info->complete &&
       !info->template_instantiation_in_progress &&
       !info->full_member_collection_in_progress &&
       !info->reference_member_collection_in_progress) {
      info = ctx_.complete_class_type(type);
    }
    if(!info) {
      return false;
    }
    out = info->member_scope.get();
    return out != nullptr;
  }

  bool evaluate_initializer_constant_value(const TemplateConstantEvaluationRequest & request,
                                           constant_eval::ConstexprValue & out) override
  {
    const ScopedRecursiveTemplateSemanticQuery query(
        recursive_template_expression_query_key(
            request.scope, RTSQ_INITIALIZER, request.expr, request.target_type));
    if(!query.active) {
      return false;
    }
    if(!request.scope) {
      return false;
    }
    if(request.target_type) {
      return ctx_.evaluate_initializer_constant_value(*request.scope,
                                                      request.expr,
                                                      request.target_type,
                                                      out);
    }
    return ctx_.evaluate_initializer_constant_value(*request.scope, request.expr, out);
  }

  bool resolve_member_type_lookup(semantic_model::Scope & lexical_scope,
                                  semantic_model::ClassInfo & owner,
                                  const std::string & name,
                                  bool ensure_current_reference_members,
                                  cpp_decl::TypePtr & out) override
  {
    out.reset();
    semantic_lookup::MemberTypeLookupResult member =
        semantic_lookup::lookup_member_type(ctx_,
                                            owner,
                                            name,
                                            ensure_current_reference_members,
                                            &lexical_scope);
    if(!member.type) {
      return false;
    }
    out = member.type;
    return true;
  }

  bool resolve_direct_type_lookup(const TemplateTypeLookupRequest & request,
                                  cpp_decl::TypePtr & out) override
  {
    if(!request.scope || request.name.name.empty()) {
      return false;
    }
    out.reset();
    if(try_leaf_resolve_type_lookup(*this, request, out)) {
      return true;
    }
    if(request.elaborated_kind != TETK_NONE &&
       request.name.name.find('<') == std::string::npos) {
      bool has_template_id_qualifier = false;
      for(size_t i = 0; i < request.name.qualifiers.size(); ++i) {
        if(request.name.qualifiers[i].find('<') != std::string::npos) {
          has_template_id_qualifier = true;
          break;
        }
      }
      if(!has_template_id_qualifier) {
        out = ctx_.maybe_introduce_elaborated_type(
            *request.scope,
            elaborated_type_keyword_text(request.elaborated_kind),
            request.name);
        if(out && (request.top_const || request.top_volatile)) {
          out = cpp_decl::apply_cv(out, request.top_const, request.top_volatile);
        }
      }
    }
    return out != nullptr;
  }

  bool resolve_selected_class_template_id(
      const TemplateSelectedClassTemplateIdRequest & request,
      cpp_decl::TypePtr & out) override
  {
    out.reset();
    if(!request.lookup.scope ||
       request.lookup.name.name.empty() ||
       !request.class_template) {
      return false;
    }
    template_api::TemplateServices bundle = this->bundle();
    const bool arguments_dependent =
        request.resolved_arguments_dependent ||
        template_api::template_arguments_are_dependent(
            ctx_, request.resolved_arguments);
    semantic_model::Scope * source_scope =
        request.argument_scope ? request.argument_scope : request.lookup.scope;
    if(parser_trace::enabled("template.resolve") &&
       request.class_template->name == "as_child") {
      std::ostringstream trace;
      trace << "selected-class-template-service"
            << " defer="
            << (request.lookup.defer_dependent_class_template_id ? "yes" : "no")
            << " carried-dependent="
            << (request.resolved_arguments_dependent ? "yes" : "no")
            << " dependent=" << (arguments_dependent ? "yes" : "no")
            << " allow="
            << (request.lookup.allow_class_templates ? "yes" : "no")
            << " source-capture="
            << (template_api::semantic_source_use_capture_enabled() ?
                    "yes" :
                    "no");
      parser_trace::note(
          "template.resolve", request.lookup.source_location, trace.str());
    }
    if(request.lookup.allow_class_templates && arguments_dependent) {
      if(bundle.witness_context.session != nullptr) {
        semantic_model::Scope * selection_scope = source_scope;
        if(source_scope &&
           source_scope->class_info &&
           source_scope->class_info->source_template == request.class_template) {
          for(std::size_t i = 0;
              i < request.class_template->partial_specializations.size();
              ++i) {
            semantic_model::PartialClassTemplateSpecializationDecl & partial =
                request.class_template->partial_specializations[i];
            if(source_scope->class_info->template_output_node &&
               partial.class_node == source_scope->class_info->template_output_node &&
               partial.pattern_scope) {
              selection_scope = partial.pattern_scope;
              break;
            }
          }
        }
        const std::string key =
            template_instantiation::class_template_argument_key_for_instantiation(
                ctx_, *request.class_template, request.resolved_arguments);
        const template_selection::ClassSpecializationSelection internal_selection =
            template_selection::select_class_specialization(
                bundle,
                *request.class_template,
                template_api::make_template_environment(*selection_scope),
                key,
                request.resolved_arguments);
        const template_api::ClassSpecializationSelection selection =
            template_api::to_api_class_specialization_selection(internal_selection);
        resolved_source_semantics::ResolvedClassTemplateIdView resolved;
        resolved.origin = request.class_template;
        resolved.instance = source_scope ? source_scope->class_info : nullptr;
        resolved.use_scope = source_scope;
        resolved.arguments = &request.resolved_arguments;
        resolved.selection = &selection;
        resolved.source_argument_texts = &request.source_arg_texts;
        resolved.source_argument_syntaxes = &request.source_arg_syntaxes;
        resolved.source_syntax = request.lookup.source_syntax;
        resolved.source_location = request.lookup.source_location.empty() ?
            nullptr : &request.lookup.source_location;
        resolved.instantiation_key = &key;
        resolved.source_use_mode = request.lookup.source_use_mode;
        resolved.dependent_arguments = true;
        if(resolved.source_syntax) {
          resolved.source_is_nested_template_argument =
              resolved.source_syntax->source_is_nested_template_argument;
          resolved.source_is_qualified_member_owner =
              resolved.source_syntax->source_is_qualified_member_owner;
        }
        ctx_.observe_resolved_class_template_id(resolved);
      }
      out = make_dependent_class_template_type(request);
      if(out && (request.lookup.top_const || request.lookup.top_volatile)) {
        out = cpp_decl::apply_cv(out, request.lookup.top_const, request.lookup.top_volatile);
      }
      if(out) {
        if(semantic_metrics::AnalyzerCounters * counters =
               ctx_.performance_counters()) {
          ++counters->dependent_class_template_type_shortcuts;
        }
        return true;
      }
    }
    const std::string key =
        template_instantiation::class_template_argument_key_for_instantiation(
            ctx_, *request.class_template, request.resolved_arguments);
    const template_selection::ClassSpecializationSelection internal_selection =
        template_selection::select_class_specialization(
            bundle,
            *request.class_template,
            template_api::make_template_environment(*source_scope),
            key,
            request.resolved_arguments);
    const template_api::ClassSpecializationSelection selection =
        template_api::to_api_class_specialization_selection(internal_selection);
    semantic_model::ClassInfo * info =
        request.lookup.allow_class_templates ?
            ctx_.reference_selected_class_template_instantiation(
                *request.class_template,
                *request.lookup.scope,
                request.resolved_arguments,
                selection,
                request.source_arg_texts.empty() ?
                    nullptr :
                    &request.source_arg_texts,
                request.lookup.source_use_mode,
                request.source_arg_syntaxes.empty() ?
                    nullptr :
                    &request.source_arg_syntaxes,
                &key,
                request.argument_scope ?
                    semantic_lookup::current_function_scope(
                        *request.argument_scope) :
                    nullptr,
                request.lookup.source_syntax) :
            ctx_.instantiate_selected_class_template(
                *request.class_template,
                *request.lookup.scope,
                request.resolved_arguments,
                selection);
    if(!info || !info->type) {
      return false;
    }

    out = info->type;
    if(out && (request.lookup.top_const || request.lookup.top_volatile)) {
      out = cpp_decl::apply_cv(out, request.lookup.top_const, request.lookup.top_volatile);
    }
    return out != nullptr;
  }

  bool evaluate_dependent_type_expression(const TemplateDependentTypeExprRequest & request,
                                  cpp_decl::TypePtr & out) override
  {
    const ScopedRecursiveTemplateSemanticQuery query(
        recursive_template_expression_query_key(
            request.scope,
            request.kind == TDTEK_DECLTYPE ? RTSQ_DECLTYPE : RTSQ_TYPEOF,
            request.operand));
    if(!query.active) {
      return false;
    }
    if(!request.scope) {
      return false;
    }
    semantic_model::Scope & scope = *request.scope;
    try {
      const ScopedTemplateDependentTypeExprUseLocation use_location(
          request.use_location);
      std::string order_use_location;
      order_use_location =
          template_api::normalize_template_witness_source_location(
              ctx_.source_location_for_node(request.operand));
      if(order_use_location.empty()) {
        order_use_location = request.use_location;
      }
      const parser_trace::ScopedOrderUseLocation order_use_location_guard(
          order_use_location);
      semantic_conversion::ExprInfo info =
          request.operand.kind == CppAstKind::call_expression ?
              ctx_.analyze_call_expression(scope,
                                           request.operand,
                                           semantic_policy::without_body_instantiation()) :
              ctx_.analyze_expression_without_output_materialization(
                  scope,
                  request.operand);
      if(!info.type) {
        return false;
      }
      if(request.operand.kind == CppAstKind::call_expression &&
         ctx_.type_depends_on_template_parameter(info.type)) {
        cpp_decl::TypePtr resolved_type;
        if(template_api::type::resolve_instantiated_dependent_type(
               ctx_, scope, info.type, resolved_type) &&
           resolved_type &&
           !ctx_.type_depends_on_template_parameter(resolved_type)) {
          info.type = resolved_type;
        }
      }
      if(request.kind == TDTEK_TYPEOF_EXPR) {
        out = info.type;
        return true;
      }
      if(!request.operand_was_parenthesized &&
         (request.operand.kind == CppAstKind::id_expression ||
          request.operand.kind == CppAstKind::member_expression)) {
        if(request.operand.kind == CppAstKind::id_expression) {
          const cpp_decl::QualifiedName * qualified =
              cppast_qualified_name_syntax(request.operand);
          const semantic_model::ValueBinding * binding =
              qualified && (qualified->rooted || !qualified->qualifiers.empty()) ?
                  semantic_lookup::lookup_qualified_value_binding_node(ctx_,
                                                                       scope,
                                                                       *qualified,
                                                                       request.operand) :
                  ctx_.lookup_value(scope, request.operand.value);
          if(binding && binding->type) {
            out = binding->type;
          } else {
            std::vector<semantic_model::FunctionBinding *> functions =
                ctx_.lookup_functions(scope,
                                      request.operand.value,
                                      semantic_policy::without_body_instantiation());
            out = functions.size() == 1 && functions[0] ? functions[0]->type : info.type;
          }
        } else {
          out = info.type;
        }
      } else if(info.category == semantic_conversion::VC_LVALUE) {
        out = cpp_decl::make_lvalue_reference_raw(info.type);
      } else if(info.category == semantic_conversion::VC_XVALUE) {
        cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(info.type);
        if(base && base->kind == cpp_decl::Type::TK_LVALUE_REFERENCE) {
          out = cpp_decl::make_lvalue_reference_raw(base->inner);
        } else if(base && base->kind == cpp_decl::Type::TK_RVALUE_REFERENCE) {
          out = cpp_decl::make_rvalue_reference_raw(base->inner);
        } else {
          out = cpp_decl::make_rvalue_reference_raw(info.type);
        }
      } else {
        out = info.type;
      }
      return out != nullptr;
    } catch(const std::logic_error & error) {
      if(parser_trace::enabled("template.resolve")) {
        parser_trace::note("template.resolve",
                           request.use_location,
                           std::string("dependent-type-expression-failed error=") +
                               error.what());
      }
      return false;
    }
  }

  bool evaluate_semantic_builtin_type_trait(const TemplateSemanticBuiltinTraitRequest & request,
                                            long long & out) override
  {
    RecursiveTemplateSemanticQueryKey query_key;
    query_key.scope = request.scope;
    query_key.operation =
        RTSQ_BUILTIN_TRAIT_BASE + static_cast<unsigned>(request.trait);
    query_key.type_count = request.types.size();
    query_key.first_type = request.types.empty() ? nullptr : request.types[0].get();
    query_key.second_type = request.types.size() < 2 ? nullptr : request.types[1].get();
    const ScopedRecursiveTemplateSemanticQuery query(query_key);
    if(!query.active) {
      return false;
    }
    if(!request.scope) {
      return false;
    }
    const char * name = nullptr;
    switch(request.trait) {
    case TSBTT_IS_CONSTRUCTIBLE: name = "__is_constructible"; break;
    case TSBTT_IS_NOTHROW_CONSTRUCTIBLE: name = "__is_nothrow_constructible"; break;
    case TSBTT_IS_ASSIGNABLE: name = "__is_assignable"; break;
    case TSBTT_IS_NOTHROW_ASSIGNABLE: name = "__is_nothrow_assignable"; break;
    case TSBTT_IS_TRIVIALLY_ASSIGNABLE: name = "__is_trivially_assignable"; break;
    case TSBTT_IS_CONVERTIBLE: name = "__is_convertible"; break;
    case TSBTT_IS_NOTHROW_CONVERTIBLE: name = "__is_nothrow_convertible"; break;
    }

    if(!name) {
      return false;
    }

    switch(request.trait) {
    case TSBTT_IS_CONSTRUCTIBLE:
    case TSBTT_IS_NOTHROW_CONSTRUCTIBLE:
      return ctx_.evaluate_builtin_type_trait(
          *request.scope, name, request.types, out);

    case TSBTT_IS_ASSIGNABLE:
    case TSBTT_IS_NOTHROW_ASSIGNABLE:
    case TSBTT_IS_TRIVIALLY_ASSIGNABLE:
    case TSBTT_IS_CONVERTIBLE:
    case TSBTT_IS_NOTHROW_CONVERTIBLE:
      return request.types.size() == 2 &&
             ctx_.evaluate_builtin_binary_type_trait(
                 *request.scope, name, request.types[0], request.types[1], out);

    default:
      return request.types.size() == 1 &&
             ctx_.evaluate_builtin_type_trait(*request.scope, name, request.types[0], out);
    }
  }

  TemplateServices bundle()
  {
    return TemplateServices(*this,
                            *this,
                            ctx_.template_witness_context(),
                            &ctx_,
                            ctx_.performance_counters());
  }

private:
  static CppAstNode make_concrete_non_type_argument_expression(
      const template_model::TemplateArgument & argument)
  {
    CppAstNode function_expression;
    if(try_make_function_non_type_argument_expression(argument,
                                                     function_expression)) {
      return function_expression;
    }

    CppAstNode literal;
    literal.kind = CppAstKind::literal;
    literal.semantic_type = argument.type;

    if(argument.value >= 0) {
      literal.value = std::to_string(
          static_cast<unsigned long long>(argument.value));
      return literal;
    }

    unsigned long long magnitude =
        static_cast<unsigned long long>(-(argument.value + 1)) + 1ULL;
    literal.value = std::to_string(magnitude);

    CppAstNode unary;
    unary.kind = CppAstKind::unary_expression;
    unary.value = "-";
    unary.has_token = true;
    unary.token_kind = RT_SIMPLE;
    unary.simple_type = OP_MINUS;
    unary.semantic_type = argument.type;
    unary.children.push_back(literal);
    return unary;
  }

  static bool try_make_function_non_type_argument_expression(
      const template_model::TemplateArgument & argument,
      CppAstNode & out)
  {
    const semantic_model::FunctionBinding * function =
        argument.rare().function_value;
    if(!function) {
      return false;
    }

    cpp_decl::QualifiedName qualified;
    if(!semantic_model::function_binding_qualified_name_syntax_for_symbol(
           *function,
           qualified)) {
      return false;
    }

    CppAstNode id;
    id.kind = CppAstKind::id_expression;
    id.value = qualified_name_text(qualified);
    id.semantic_type = function->declared_type ?
        function->declared_type :
        function->type;
    set_cppast_qualified_name_syntax(id, qualified);

    if(!function->is_method) {
      out = id;
      return true;
    }

    CppAstNode unary;
    unary.kind = CppAstKind::unary_expression;
    unary.value = "&";
    unary.has_token = true;
    unary.token_kind = RT_SIMPLE;
    unary.simple_type = OP_AMP;
    unary.semantic_type = argument.type;
    unary.children.push_back(id);
    out = unary;
    return true;
  }

  static bool concrete_non_type_argument_expression_needs_refresh(
      const template_model::TemplateArgument & argument,
      const cpp_decl::TemplateArgumentSyntax & syntax)
  {
    const semantic_model::FunctionBinding * function =
        argument.rare().function_value;
    if(!function || !function->is_method) {
      return false;
    }
    if(!syntax.expression) {
      return true;
    }
    const CppAstNode & expression = *syntax.expression;
    return expression.kind != CppAstKind::unary_expression ||
           !expression.has_token ||
           expression.simple_type != OP_AMP;
  }

  static void attach_concrete_non_type_argument_expression(
      const template_model::TemplateArgument & argument,
      cpp_decl::TemplateArgumentSyntax & syntax)
  {
    if(argument.kind != template_model::TemplateArgument::TA_VALUE ||
       argument.dependent ||
       (syntax.expression &&
        !concrete_non_type_argument_expression_needs_refresh(argument, syntax))) {
      return;
    }
    syntax.expression.reset(new CppAstNode(
        make_concrete_non_type_argument_expression(argument)));
  }

  static std::vector<cpp_decl::TemplateArgumentSyntax>
  normalized_class_template_argument_syntaxes(
      const std::vector<template_model::TemplateArgument> & arguments,
      const std::vector<cpp_decl::TemplateArgumentSyntax> & source_syntaxes)
  {
    std::vector<cpp_decl::TemplateArgumentSyntax> syntaxes(arguments.size());
    for(std::size_t i = 0; i < arguments.size(); ++i) {
      if(i < source_syntaxes.size()) {
        syntaxes[i] = source_syntaxes[i];
      } else if(arguments[i].source_syntax) {
        syntaxes[i] = *arguments[i].source_syntax;
      }
      if(syntaxes[i].text.empty()) {
        syntaxes[i].text = arguments[i].text;
      }
      if(arguments[i].kind == template_model::TemplateArgument::TA_TYPE &&
         arguments[i].type &&
         !syntaxes[i].resolved_type) {
        syntaxes[i].resolved_type = arguments[i].type;
      }
      attach_concrete_non_type_argument_expression(arguments[i], syntaxes[i]);
    }
    return syntaxes;
  }

  cpp_decl::TypePtr make_dependent_class_template_type(
      const TemplateSelectedClassTemplateIdRequest & request)
  {
    std::vector<std::string> argument_texts = request.source_arg_texts;
    if(argument_texts.size() != request.resolved_arguments.size()) {
      argument_texts =
          template_api::canonical_template_argument_texts(ctx_,
                                                         request.resolved_arguments);
    }

    const std::string display_name =
        template_api::display_specialization_name_for_instantiation(
            ctx_,
            request.class_template->name,
            request.resolved_arguments);
    cpp_decl::TypePtr type =
        cpp_decl::make_semantic_named(display_name,
                                      cpp_decl::Type::NSK_DEPENDENT_TYPE,
                                      display_name,
                                      true);
    std::vector<cpp_decl::DependentAliasTemplateArgumentSyntax> arguments;
    arguments.reserve(request.resolved_arguments.size());
    for(std::size_t i = 0; i < request.resolved_arguments.size(); ++i) {
      cpp_decl::DependentAliasTemplateArgumentSyntax argument;
      argument.text =
          i < argument_texts.size() ? argument_texts[i] :
                                      template_model::template_argument_text(
                                          request.resolved_arguments[i],
                                          [this](const cpp_decl::TypePtr & arg_type)
                                          {
                                            return ctx_.instantiation_identity_text_for_type_argument(
                                                arg_type);
                                          });
      if(request.resolved_arguments[i].kind == template_model::TemplateArgument::TA_TYPE) {
        argument.type = request.resolved_arguments[i].type;
      } else if(request.resolved_arguments[i].kind ==
                template_model::TemplateArgument::TA_VALUE) {
        const template_model::TemplateArgument::RareData & rare =
            request.resolved_arguments[i].rare();
        argument.type = request.resolved_arguments[i].type;
        argument.function_value = rare.function_value;
        argument.function_internal_symbol =
            rare.function_internal_symbol;
        argument.value_binding = rare.value_binding;
        argument.value = request.resolved_arguments[i].value;
        argument.has_non_type_value = true;
        argument.dependent_value = request.resolved_arguments[i].dependent;
      }
      if(i < request.source_arg_syntaxes.size()) {
        argument.syntax = request.source_arg_syntaxes[i];
      } else if(request.resolved_arguments[i].kind !=
                template_model::TemplateArgument::TA_TYPE) {
        argument.syntax.text = argument.text;
      }
      if(request.resolved_arguments[i].kind == template_model::TemplateArgument::TA_TYPE &&
         request.resolved_arguments[i].type) {
        argument.syntax.resolved_type = request.resolved_arguments[i].type;
        if(argument.syntax.text.empty()) {
          argument.syntax.text = argument.text;
        }
      }
      if(request.resolved_arguments[i].dependent) {
        argument.syntax.dependent = true;
      }
      if(request.resolved_arguments[i].kind == template_model::TemplateArgument::TA_VALUE &&
         request.resolved_arguments[i].expression &&
         !argument.syntax.expression) {
        argument.syntax.text = argument.text;
        argument.syntax.source_location_id =
            request.resolved_arguments[i].expression->source_location_id;
        argument.syntax.expression.reset(
            new CppAstNode(*request.resolved_arguments[i].expression));
      }
      attach_concrete_non_type_argument_expression(request.resolved_arguments[i],
                                                  argument.syntax);
      argument.source_defaulted = request.resolved_arguments[i].source_defaulted;
      arguments.push_back(argument);
    }
    cpp_decl::set_named_type_dependent_class_template(
        type,
        request.class_template,
        arguments);
    std::shared_ptr<cpp_decl::ClassTemplateSpecializationMangleInfo> mangle_info(
        new cpp_decl::ClassTemplateSpecializationMangleInfo());
    mangle_info->class_template_decl = request.class_template;
    mangle_info->template_name = request.class_template->name;
    if(request.class_template->declaring_scope) {
      mangle_info->template_name_syntax =
          semantic_lookup::scope_qualified_name_syntax(
              *request.class_template->declaring_scope,
              request.class_template->name);
      const std::string qualified =
          semantic_lookup::scope_qualified_name(
              *request.class_template->declaring_scope,
              request.class_template->name);
      const std::string suffix = std::string("::") + request.class_template->name;
      if(qualified == request.class_template->name) {
        mangle_info->template_scope_prefix.clear();
      } else if(qualified.size() > suffix.size() &&
                qualified.compare(qualified.size() - suffix.size(),
                                  suffix.size(),
                                  suffix) == 0) {
        mangle_info->template_scope_prefix =
            qualified.substr(0, qualified.size() - suffix.size());
      }
    }
    mangle_info->template_parameters.share(
        semantic_model::retained_class_template_mangle_parameters(
            *request.class_template));
    mangle_info->arguments = request.resolved_arguments;
    mangle_info->argument_syntaxes =
        normalized_class_template_argument_syntaxes(request.resolved_arguments,
                                                   request.source_arg_syntaxes);
    cpp_decl::set_named_type_class_template_specialization_mangle_info(
        type,
        mangle_info);
    return type;
  }

  SemanticContext & ctx_;
};

template <typename Callback>
inline auto with_template_services(SemanticContext & ctx, Callback callback)
    -> decltype(callback(std::declval<TemplateServices &>()))
{
  SemanticContextTemplateServices storage(ctx);
  TemplateServices services = storage.bundle();
  return callback(services);
}

template <typename Callback>
inline auto with_template_type_system(SemanticContext & ctx, Callback callback)
    -> decltype(callback(std::declval<TemplateTypeSystem &>()))
{
  SemanticContextTemplateServices storage(ctx);
  return callback(storage);
}

}  // namespace template_api
