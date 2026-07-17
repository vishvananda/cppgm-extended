#include "semantic_metrics.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace semantic_metrics {

namespace {

bool env_enabled()
{
  static const bool out = []()
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_STATS");
    return value && *value && std::string(value) != "0";
  }();
  return out;
}

bool phase_env_enabled()
{
  static const bool out = []()
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_PHASE_STATS");
    return value && *value && std::string(value) != "0";
  }();
  return out;
}

thread_local ClassDemandKind current_class_demand_kind = CDK_UNSPECIFIED;

std::string metric_token(std::string value)
{
  if(value.empty()) {
    return "<unnamed>";
  }
  for(std::size_t i = 0; i < value.size(); ++i) {
    unsigned char ch = static_cast<unsigned char>(value[i]);
    if(std::isspace(ch)) {
      value[i] = '_';
    }
  }
  return value;
}

}  // namespace

bool enabled()
{
  return env_enabled();
}

bool phase_stats_enabled()
{
  return phase_env_enabled();
}

const char * cache_kind_name(CacheKind kind)
{
  switch(kind) {
  case CK_IDENTIFIER_TOKENS:
    return "identifier_tokens";
  case CK_TEMPLATE_PLACEHOLDER_MENTIONS:
    return "template_placeholder_mentions";
  case CK_NON_NAMESPACE_BINDING_MENTIONS:
    return "non_namespace_binding_mentions";
  case CK_DEPENDENT_NON_NAMESPACE_BINDING_MENTIONS:
    return "dependent_non_namespace_binding_mentions";
  case CK_QUALIFIED_TYPE_LOOKUP:
    return "qualified_type_lookup";
  case CK_DEPENDENT_TYPE_RESOLUTION:
    return "dependent_type_resolution";
  case CK_COUNT:
    break;
  }
  return "unknown";
}

const char * class_demand_kind_name(ClassDemandKind kind)
{
  switch(kind) {
  case CDK_UNSPECIFIED:
    return "unspecified";
  case CDK_COLLECT_DECLARATIONS:
    return "collect-declarations";
  case CDK_OUTPUT_SEED:
    return "output-seed";
  case CDK_FIXPOINT_REQUIRED_DEFINITION_REFRESH:
    return "fixpoint-required-definition-refresh";
  case CDK_FIXPOINT_INSTANTIATED_TEMPLATE_OUTPUT:
    return "fixpoint-instantiated-template-output";
  case CDK_FIXPOINT_SYNTHETIC_FUNCTION_OUTPUT:
    return "fixpoint-synthetic-function-output";
  case CDK_FIXPOINT_LATE_REQUIRED_FUNCTION_OUTPUT:
    return "fixpoint-late-required-function-output";
  case CDK_FIXPOINT_LATE_REQUIRED_SYNTHESIZED_OUTPUT:
    return "fixpoint-late-required-synthesized-output";
  case CDK_FIXPOINT_CALLEE_CLOSURE:
    return "fixpoint-callee-closure";
  case CDK_VALIDATE_REQUIRED_DEFINITIONS:
    return "validate-required-definitions";
  case CDK_WITNESS_UNEMITTED_BODIES:
    return "witness-unemitted-bodies";
  case CDK_FIELD_MEMBER_OBJECT:
    return "field-member-object";
  case CDK_BASE_CLASS_COLLECTION:
    return "base-class-collection";
  case CDK_NESTED_CLASS_COLLECTION:
    return "nested-class-collection";
  case CDK_IMPLICIT_SPECIAL_MEMBERS:
    return "implicit-special-members";
  case CDK_CLASS_VIRTUALS:
    return "class-virtuals";
  case CDK_CLASS_LAYOUT:
    return "class-layout";
  case CDK_OUTPUT_LAYOUT_COMPLETION:
    return "output-layout-completion";
  case CDK_COUNT:
    break;
  }
  return "unknown";
}

ClassDemandKind current_class_demand()
{
  return current_class_demand_kind;
}

ScopedClassDemand::ScopedClassDemand(ClassDemandKind kind)
  : previous_(current_class_demand_kind)
{
  current_class_demand_kind = kind;
}

ScopedClassDemand::~ScopedClassDemand()
{
  current_class_demand_kind = previous_;
}

AnalyzerCounters::AnalyzerCounters()
{
  class_info_for_type_by_demand.fill(0);
  complete_class_type_by_demand.fill(0);
  complete_class_type_materializations_by_demand.fill(0);
  class_populate_by_demand.fill(0);
  statement_analysis_by_demand.fill(0);
  expression_analysis_by_demand.fill(0);
  function_body_emit_by_demand.fill(0);
  function_body_statement_by_demand.fill(0);
  member_object_completion_by_parent_demand.fill(0);
  member_object_complete_type_by_parent_demand.fill(0);
  member_object_direct_populate_by_parent_demand.fill(0);
  reference_member_scope_prepare_by_demand.fill(0);
  reference_member_scope_ensure_by_demand.fill(0);
  reference_member_collection_by_demand.fill(0);
}

void AnalyzerCounters::note_cache_hit(CacheKind kind)
{
  ++caches[static_cast<std::size_t>(kind)].hits;
}

void AnalyzerCounters::note_cache_miss(CacheKind kind)
{
  ++caches[static_cast<std::size_t>(kind)].misses;
}

void AnalyzerCounters::note_output_seed_class_materialization(
    const std::string & class_name,
    std::size_t ast_children)
{
  ClassWorkCounter & counter =
      output_seed_class_materializations[class_name];
  ++counter.count;
  counter.ast_children += ast_children;
}

void AnalyzerCounters::note_reference_member_collection(
    const std::string & class_name,
    std::size_t ast_children)
{
  ClassWorkCounter & counter =
      reference_member_collections_by_class[class_name];
  ++counter.count;
  counter.ast_children += ast_children;
}

void AnalyzerCounters::note_reference_before_full_collection(
    const std::string & class_name,
    std::size_t full_ast_children)
{
  ReferenceBeforeFullCounter & counter =
      reference_before_full_by_class[class_name];
  ++counter.count;
  std::map<std::string, ClassWorkCounter>::const_iterator reference =
      reference_member_collections_by_class.find(class_name);
  if(reference != reference_member_collections_by_class.end()) {
    counter.reference_ast_children += reference->second.ast_children;
  }
  counter.full_ast_children += full_ast_children;
}

void AnalyzerCounters::note_member_object_completion_class(
    const std::string & class_name,
    std::size_t ast_children)
{
  MemberObjectCompletionCounter & counter =
      member_object_completions_by_class[class_name];
  ++counter.count;
  counter.ast_children += ast_children;
}

void AnalyzerCounters::note_member_object_completion_complete_type_call(
    const std::string & class_name)
{
  ++member_object_completions_by_class[class_name].complete_type_calls;
}

void AnalyzerCounters::note_member_object_completion_layout_sync(
    const std::string & class_name)
{
  ++member_object_completions_by_class[class_name].layout_syncs;
}

void AnalyzerCounters::dump(std::ostream & out) const
{
  std::size_t reference_before_full_count = 0;
  std::size_t reference_before_full_reference_children = 0;
  std::size_t reference_before_full_full_children = 0;
  for(std::map<std::string, ReferenceBeforeFullCounter>::const_iterator it =
          reference_before_full_by_class.begin();
      it != reference_before_full_by_class.end();
      ++it) {
    reference_before_full_count += it->second.count;
    reference_before_full_reference_children += it->second.reference_ast_children;
    reference_before_full_full_children += it->second.full_ast_children;
  }

  out << "semantic-metrics"
      << " template-instantiation-requests=" << template_instantiation_requests
      << " template-definition-upgrades=" << template_definition_upgrades
      << " required-definition-requests=" << required_definition_requests
      << " required-definition-upgrades=" << required_definition_upgrades
      << " fixpoint-iterations=" << fixpoint_iterations
      << " output-seed-nodes=" << output_seed_nodes
      << " output-seed-output-appends=" << output_seed_output_appends
      << " output-seed-state-changes=" << output_seed_state_changes
      << " required-definition-refresh-scans="
      << required_definition_refresh_scans
      << " required-definition-refresh-updates="
      << required_definition_refresh_updates
      << " instantiated-class-output-scans="
      << instantiated_class_output_scans
      << " instantiated-class-output-emits="
      << instantiated_class_output_emits
      << " instantiated-function-output-scans="
      << instantiated_function_output_scans
      << " instantiated-function-output-emits="
      << instantiated_function_output_emits
      << " synthetic-function-output-scans="
      << synthetic_function_output_scans
      << " synthetic-function-output-emits="
      << synthetic_function_output_emits
      << " late-required-function-output-scans="
      << late_required_function_output_scans
      << " late-required-function-output-emits="
      << late_required_function_output_emits
      << " late-synthesized-class-scans="
      << late_synthesized_class_scans
      << " late-synthesized-method-scans="
      << late_synthesized_method_scans
      << " late-synthesized-method-emits="
      << late_synthesized_method_emits
      << " late-synthesized-static-function-scans="
      << late_synthesized_static_function_scans
      << " late-synthesized-static-function-emits="
      << late_synthesized_static_function_emits
      << " late-synthesized-rtti-scans="
      << late_synthesized_rtti_scans
      << " late-synthesized-rtti-emits="
      << late_synthesized_rtti_emits
      << " emitted-output-callee-top-level-scans="
      << emitted_output_callee_top_level_scans
      << " required-definition-validation-scans="
      << required_definition_validation_scans
      << " rescanned-emitted-nodes=" << rescanned_emitted_nodes
      << " overload-candidate-sets=" << overload_candidate_sets
      << " overload-candidate-attempts=" << overload_candidate_attempts
      << " overload-viable-candidates=" << overload_viable_candidates
      << " overload-candidate-refresh-attempts="
      << overload_candidate_refresh_attempts
      << " overload-candidate-refresh-successes="
      << overload_candidate_refresh_successes
      << " conversion-attempts=" << conversion_attempts
      << " adl-associated-collections=" << adl_associated_collections
      << " adl-associated-type-visits=" << adl_associated_type_visits
      << " adl-associated-scope-outputs=" << adl_associated_scope_outputs
      << " adl-associated-function-outputs="
      << adl_associated_function_outputs
      << " adl-associated-template-outputs="
      << adl_associated_template_outputs
      << " adl-associated-scope-cache-hits="
      << adl_associated_scope_cache_hits
      << " adl-associated-scope-cache-misses="
      << adl_associated_scope_cache_misses
      << " adl-associated-scope-cache-entries="
      << adl_associated_scope_cache_entries
      << " adl-associated-scope-cache-uncacheable="
      << adl_associated_scope_cache_uncacheable
      << " adl-associated-scope-cache-clears="
      << adl_associated_scope_cache_clears
      << " candidate-identity-builds=" << candidate_identity_builds
      << " candidate-identity-chars=" << candidate_identity_chars
      << " candidate-partial-order-comparisons="
      << candidate_partial_order_comparisons
      << " candidate-partial-order-acceptance-checks="
      << candidate_partial_order_acceptance_checks
      << " scope-cache-key-calls=" << scope_cache_key_calls
      << " semantic-only-type-text-resolutions=" << semantic_only_type_text_resolutions
      << " fragment-fallback-type-text-resolutions="
      << fragment_fallback_type_text_resolutions
      << " function-symbol-upgrade-requests="
      << function_symbol_upgrade_requests
      << " function-symbol-upgrade-redeclaration-skips="
      << function_symbol_upgrade_redeclaration_skips
      << " function-template-deduction-cache-allowed-checks="
      << function_template_deduction_cache_allowed_checks
      << " function-template-deduction-cache-uncacheable-arg-rejects="
      << function_template_deduction_cache_uncacheable_arg_rejects
      << " function-template-deduction-cache-key-builds="
      << function_template_deduction_cache_key_builds
      << " function-template-deduction-cache-key-args="
      << function_template_deduction_cache_key_args
      << " function-template-deduction-cache-use-scope-sensitive-keys="
      << function_template_deduction_cache_use_scope_sensitive_keys
      << " function-template-deduction-cache-hits="
      << function_template_deduction_cache_hits
      << " function-template-deduction-cache-misses="
      << function_template_deduction_cache_misses
      << " function-template-deduction-cache-entries="
      << function_template_deduction_cache_entries
      << " function-template-deduction-cache-clears="
      << function_template_deduction_cache_clears
      << " function-template-deduction-cacheable-type-checks="
      << function_template_deduction_cacheable_type_checks
      << " function-template-deduction-cacheable-type-hits="
      << function_template_deduction_cacheable_type_hits
      << " function-template-deduction-cacheable-type-scans="
      << function_template_deduction_cacheable_type_scans
      << " function-template-deduction-cacheable-type-entries="
      << function_template_deduction_cacheable_type_entries
      << " function-template-deduction-cacheable-type-clears="
      << function_template_deduction_cacheable_type_clears
      << " resolve-template-argument-calls=" << resolve_template_argument_calls
      << " resolve-template-argument-single-bound-type-fast-hits="
      << resolve_template_argument_single_bound_type_fast_hits
      << " resolve-template-argument-simple-fast-successes="
      << resolve_template_argument_simple_fast_successes
      << " resolve-template-argument-simple-fast-failures="
      << resolve_template_argument_simple_fast_failures
      << " resolve-template-argument-simple-fast-unsupported="
      << resolve_template_argument_simple_fast_unsupported
      << " resolve-template-argument-simple-fast-expensive-successes="
      << resolve_template_argument_simple_fast_expensive_successes
      << " resolve-template-argument-simple-fast-expensive-failures="
      << resolve_template_argument_simple_fast_expensive_failures
      << " resolve-template-argument-simple-fast-expensive-unsupported="
      << resolve_template_argument_simple_fast_expensive_unsupported
      << " resolve-template-argument-fast-entry-key-builds="
      << resolve_template_argument_fast_entry_key_builds
      << " resolve-template-argument-pre-expansion-simple-type-successes="
      << resolve_template_argument_pre_expansion_simple_type_successes
      << " resolve-template-argument-pre-expansion-bound-member-failures="
      << resolve_template_argument_pre_expansion_bound_member_failures
      << " resolve-template-argument-pre-expansion-trait-bound-member-failures="
      << resolve_template_argument_pre_expansion_trait_bound_member_failures
      << " resolve-template-argument-bound-member-failures="
      << resolve_template_argument_bound_member_failures
      << " bound-member-failure-cache-hits="
      << bound_member_failure_cache_hits
      << " bound-member-failure-cache-entries="
      << bound_member_failure_cache_entries
      << " standard-enable-if-member-type-successes="
      << standard_enable_if_member_type_successes
      << " standard-enable-if-member-type-failures="
      << standard_enable_if_member_type_failures
      << " standard-conditional-member-type-successes="
      << standard_conditional_member_type_successes
      << " resolve-template-argument-fast-cache-hits="
      << resolve_template_argument_fast_cache_hits
      << " resolve-template-argument-cache-hits="
      << resolve_template_argument_cache_hits
      << " resolve-template-argument-cache-misses="
      << resolve_template_argument_cache_misses
      << " resolve-template-argument-success-entries="
      << resolve_template_argument_success_entries
      << " resolve-template-argument-failure-entries="
      << resolve_template_argument_failure_entries
      << " resolve-template-argument-key-builds="
      << resolve_template_argument_key_builds
      << " resolve-template-argument-key-text-count="
      << resolve_template_argument_key_text_count
      << " resolve-template-argument-key-text-chars="
      << resolve_template_argument_key_text_chars
      << " resolve-template-argument-expanded-texts="
      << resolve_template_argument_expanded_texts
      << " resolve-template-argument-source-location-calls="
      << resolve_template_argument_source_location_calls
      << " class-template-reference-requests="
      << class_template_reference_requests
      << " class-template-fast-existing-attempts="
      << class_template_fast_existing_attempts
      << " class-template-fast-existing-hits="
      << class_template_fast_existing_hits
      << " class-template-fast-existing-misses="
      << class_template_fast_existing_misses
      << " class-template-key-builds=" << class_template_key_builds
      << " class-template-specialization-name-builds="
      << class_template_specialization_name_builds
      << " class-template-hits=" << class_template_hits
      << " class-template-creates=" << class_template_creates
      << " class-template-resets=" << class_template_resets
      << " class-template-canonical-arg-text-builds="
      << class_template_canonical_arg_text_builds
      << " dependent-class-template-type-shortcuts="
      << dependent_class_template_type_shortcuts
      << " class-info-for-type-definitely-not-class-skips="
      << class_info_for_type_definitely_not_class_skips
      << " class-info-for-type-calls=" << class_info_for_type_calls
      << " class-info-for-type-pointer-cache-hits="
      << class_info_for_type_pointer_cache_hits
      << " class-info-for-type-named-key-cache-hits="
      << class_info_for_type_named_key_cache_hits
      << " class-info-for-type-map-lookups="
      << class_info_for_type_map_lookups
      << " class-info-for-type-map-hits=" << class_info_for_type_map_hits
      << " class-info-for-type-map-misses=" << class_info_for_type_map_misses
      << " complete-class-type-definitely-not-class-skips="
      << complete_class_type_definitely_not_class_skips
      << " complete-class-type-calls=" << complete_class_type_calls
      << " complete-class-type-no-class=" << complete_class_type_no_class
      << " complete-class-type-already-complete="
      << complete_class_type_already_complete
      << " complete-class-type-layout-self-sync-skips="
      << complete_class_type_layout_self_sync_skips
      << " complete-class-type-in-progress="
      << complete_class_type_in_progress
      << " complete-class-type-materializations="
      << complete_class_type_materializations
      << " member-object-completion-calls="
      << member_object_completion_calls
      << " member-object-completion-already-layout="
      << member_object_completion_already_layout
      << " member-object-completion-dependent="
      << member_object_completion_dependent
      << " member-object-completion-no-class="
      << member_object_completion_no_class
      << " member-object-completion-complete-type-calls="
      << member_object_completion_complete_type_calls
      << " member-object-completion-direct-populates="
      << member_object_completion_direct_populates
      << " member-object-completion-layout-syncs="
      << member_object_completion_layout_syncs
      << " reference-member-scope-prepares="
      << reference_member_scope_prepares
      << " reference-member-scope-ensures="
      << reference_member_scope_ensures
      << " reference-member-collections="
      << reference_member_collections
      << " output-seed-materialized-classes="
      << output_seed_class_materializations.size()
      << " reference-before-full-classes="
      << reference_before_full_by_class.size()
      << " reference-before-full-collections="
      << reference_before_full_count
      << " reference-before-full-reference-children="
      << reference_before_full_reference_children
      << " reference-before-full-full-children="
      << reference_before_full_full_children
      << " member-object-completion-classes="
      << member_object_completions_by_class.size()
      << " instantiated-class-output-readiness-calls="
      << instantiated_class_output_readiness_calls
      << "\n";

  for(std::size_t i = 0; i < class_info_for_type_by_demand.size(); ++i) {
    const ClassDemandKind kind = static_cast<ClassDemandKind>(i);
    const std::size_t class_info_count = class_info_for_type_by_demand[i];
    const std::size_t complete_count = complete_class_type_by_demand[i];
    const std::size_t materialize_count =
        complete_class_type_materializations_by_demand[i];
    const std::size_t populate_count = class_populate_by_demand[i];
    const std::size_t statement_count = statement_analysis_by_demand[i];
    const std::size_t expression_count = expression_analysis_by_demand[i];
    const std::size_t function_body_count = function_body_emit_by_demand[i];
    const std::size_t function_body_statement_count =
        function_body_statement_by_demand[i];
    const std::size_t member_object_count =
        member_object_completion_by_parent_demand[i];
    const std::size_t member_object_complete_count =
        member_object_complete_type_by_parent_demand[i];
    const std::size_t member_object_direct_populate_count =
        member_object_direct_populate_by_parent_demand[i];
    const std::size_t member_scope_prepare_count =
        reference_member_scope_prepare_by_demand[i];
    const std::size_t member_scope_ensure_count =
        reference_member_scope_ensure_by_demand[i];
    const std::size_t reference_collection_count =
        reference_member_collection_by_demand[i];
    if(class_info_count == 0 &&
       complete_count == 0 &&
       materialize_count == 0 &&
       populate_count == 0 &&
       statement_count == 0 &&
       expression_count == 0 &&
       function_body_count == 0 &&
       function_body_statement_count == 0 &&
       member_object_count == 0 &&
       member_object_complete_count == 0 &&
       member_object_direct_populate_count == 0 &&
       member_scope_prepare_count == 0 &&
       member_scope_ensure_count == 0 &&
       reference_collection_count == 0) {
      continue;
    }
    out << "semantic-class-demand"
        << " name=" << class_demand_kind_name(kind)
        << " class-info-for-type=" << class_info_count
        << " complete-class-type=" << complete_count
        << " complete-class-materializations=" << materialize_count
        << " populate-class-info=" << populate_count
        << " statement-analyses=" << statement_count
        << " expression-analyses=" << expression_count
        << " function-body-emits=" << function_body_count
        << " function-body-source-statements=" << function_body_statement_count
        << " member-object-completions=" << member_object_count
        << " member-object-complete-type-calls=" << member_object_complete_count
        << " member-object-direct-populates="
        << member_object_direct_populate_count
        << " reference-member-scope-prepares=" << member_scope_prepare_count
        << " reference-member-scope-ensures=" << member_scope_ensure_count
        << " reference-member-collections=" << reference_collection_count
        << "\n";
  }

  for(std::size_t i = 0; i < caches.size(); ++i) {
    const CacheCounter & cache = caches[i];
    if(cache.hits == 0 && cache.misses == 0) {
      continue;
    }
    out << "semantic-cache"
        << " name=" << cache_kind_name(static_cast<CacheKind>(i))
        << " hits=" << cache.hits
        << " misses=" << cache.misses
        << "\n";
  }

  typedef std::pair<std::string, ClassWorkCounter> ClassWorkEntry;
  std::vector<ClassWorkEntry> output_seed_entries(
      output_seed_class_materializations.begin(),
      output_seed_class_materializations.end());
  std::sort(output_seed_entries.begin(),
            output_seed_entries.end(),
            [](const ClassWorkEntry & lhs, const ClassWorkEntry & rhs)
            {
              if(lhs.second.ast_children != rhs.second.ast_children) {
                return lhs.second.ast_children > rhs.second.ast_children;
              }
              if(lhs.second.count != rhs.second.count) {
                return lhs.second.count > rhs.second.count;
              }
              return lhs.first < rhs.first;
            });
  const std::size_t output_seed_limit =
      std::min<std::size_t>(10, output_seed_entries.size());
  for(std::size_t i = 0; i < output_seed_limit; ++i) {
    out << "semantic-output-seed-class-materialization"
        << " rank=" << (i + 1)
        << " class=" << metric_token(output_seed_entries[i].first)
        << " count=" << output_seed_entries[i].second.count
        << " ast-children=" << output_seed_entries[i].second.ast_children
        << "\n";
  }

  typedef std::pair<std::string, ReferenceBeforeFullCounter>
      ReferenceBeforeFullEntry;
  std::vector<ReferenceBeforeFullEntry> reference_before_full_entries(
      reference_before_full_by_class.begin(),
      reference_before_full_by_class.end());
  std::sort(reference_before_full_entries.begin(),
            reference_before_full_entries.end(),
            [](const ReferenceBeforeFullEntry & lhs,
               const ReferenceBeforeFullEntry & rhs)
            {
              if(lhs.second.full_ast_children != rhs.second.full_ast_children) {
                return lhs.second.full_ast_children > rhs.second.full_ast_children;
              }
              if(lhs.second.reference_ast_children !=
                 rhs.second.reference_ast_children) {
                return lhs.second.reference_ast_children >
                       rhs.second.reference_ast_children;
              }
              if(lhs.second.count != rhs.second.count) {
                return lhs.second.count > rhs.second.count;
              }
              return lhs.first < rhs.first;
            });
  const std::size_t reference_before_full_limit =
      std::min<std::size_t>(10, reference_before_full_entries.size());
  for(std::size_t i = 0; i < reference_before_full_limit; ++i) {
    const ReferenceBeforeFullCounter & counter =
        reference_before_full_entries[i].second;
    out << "semantic-reference-before-full"
        << " rank=" << (i + 1)
        << " class=" << metric_token(reference_before_full_entries[i].first)
        << " count=" << counter.count
        << " reference-children=" << counter.reference_ast_children
        << " full-children=" << counter.full_ast_children
        << "\n";
  }

  typedef std::pair<std::string, MemberObjectCompletionCounter>
      MemberObjectCompletionEntry;
  std::vector<MemberObjectCompletionEntry> member_object_entries(
      member_object_completions_by_class.begin(),
      member_object_completions_by_class.end());
  std::sort(member_object_entries.begin(),
            member_object_entries.end(),
            [](const MemberObjectCompletionEntry & lhs,
               const MemberObjectCompletionEntry & rhs)
            {
              if(lhs.second.complete_type_calls != rhs.second.complete_type_calls) {
                return lhs.second.complete_type_calls > rhs.second.complete_type_calls;
              }
              if(lhs.second.layout_syncs != rhs.second.layout_syncs) {
                return lhs.second.layout_syncs > rhs.second.layout_syncs;
              }
              if(lhs.second.count != rhs.second.count) {
                return lhs.second.count > rhs.second.count;
              }
              if(lhs.second.ast_children != rhs.second.ast_children) {
                return lhs.second.ast_children > rhs.second.ast_children;
              }
              return lhs.first < rhs.first;
            });
  const std::size_t member_object_limit =
      std::min<std::size_t>(10, member_object_entries.size());
  for(std::size_t i = 0; i < member_object_limit; ++i) {
    const MemberObjectCompletionCounter & counter =
        member_object_entries[i].second;
    out << "semantic-member-object-completion-class"
        << " rank=" << (i + 1)
        << " class=" << metric_token(member_object_entries[i].first)
        << " count=" << counter.count
        << " complete-type-calls=" << counter.complete_type_calls
        << " layout-syncs=" << counter.layout_syncs
        << " ast-children=" << counter.ast_children
        << "\n";
  }
}

ScopedPhaseTimer::ScopedPhaseTimer(const char * name, const std::string & detail)
    : name_(name),
      detail_(detail),
      enabled_(phase_stats_enabled()),
      start_(enabled_ ? Clock::now() : Clock::time_point())
{
}

ScopedPhaseTimer::~ScopedPhaseTimer()
{
  if(!enabled_) {
    return;
  }
  const auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_);
  std::cerr << "semantic-phase"
            << " name=" << name_;
  if(!detail_.empty()) {
    std::cerr << " detail=" << detail_;
  }
  std::cerr << " ms=" << elapsed.count() << "\n";
}

}  // namespace semantic_metrics
