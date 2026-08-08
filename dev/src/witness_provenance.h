#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace semantic_source_use {
struct SemanticSourceUse;
struct SemanticSourceUseTable;
}

namespace template_api {
struct TemplateLifecycleEvent;
struct TemplateWitnessSession;
}

namespace witness_provenance {

// These identifiers are deliberately independent of source line numbers. They
// remain stable while the experiment moves or deletes individual producers.
enum class WitnessProducerSite
{
  Unknown = 0,

  ClassTemplateReference02 = 6,

  AliasCanonicalOccurrence = 11,

  FunctionSemanticTemplateFunction,

  VariableTemplateInstantiation,

  LifecycleTransitionObserver01,
};

const char * producer_site_name(WitnessProducerSite site);

enum class WitnessUpstreamRoute
{
  Unknown = 0,
  AliasCanonicalOccurrence,
};

const char * upstream_route_name(WitnessUpstreamRoute route);

bool enabled();

class ScopedUpstreamRoute
{
public:
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  explicit ScopedUpstreamRoute(WitnessUpstreamRoute route);
  ~ScopedUpstreamRoute();
#else
  explicit ScopedUpstreamRoute(WitnessUpstreamRoute) {}
  ~ScopedUpstreamRoute() = default;
#endif

  ScopedUpstreamRoute(const ScopedUpstreamRoute &) = delete;
  ScopedUpstreamRoute & operator=(const ScopedUpstreamRoute &) = delete;

private:
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  WitnessUpstreamRoute previous_ = WitnessUpstreamRoute::Unknown;
#endif
};

class ScopedSourceUseAttempt
{
public:
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  ScopedSourceUseAttempt(
      template_api::TemplateWitnessSession * session,
      semantic_source_use::SemanticSourceUseTable * table,
      WitnessProducerSite producer,
      const semantic_source_use::SemanticSourceUse & use);
  ~ScopedSourceUseAttempt();
#else
  ScopedSourceUseAttempt(
      template_api::TemplateWitnessSession *,
      semantic_source_use::SemanticSourceUseTable *,
      WitnessProducerSite,
      const semantic_source_use::SemanticSourceUse &)
  {}
  ~ScopedSourceUseAttempt() = default;
#endif

  ScopedSourceUseAttempt(const ScopedSourceUseAttempt &) = delete;
  ScopedSourceUseAttempt & operator=(const ScopedSourceUseAttempt &) = delete;

private:
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  template_api::TemplateWitnessSession * session_ = nullptr;
  semantic_source_use::SemanticSourceUseTable * table_ = nullptr;
  std::uint64_t token_ = 0;
#endif
};

class ScopedLifecycleAttempt
{
public:
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  ScopedLifecycleAttempt(template_api::TemplateWitnessSession & session,
                         WitnessProducerSite producer,
                         const template_api::TemplateLifecycleEvent & event);
  ~ScopedLifecycleAttempt();
#else
  ScopedLifecycleAttempt(template_api::TemplateWitnessSession &,
                         WitnessProducerSite,
                         const template_api::TemplateLifecycleEvent &)
  {}
  ~ScopedLifecycleAttempt() = default;
#endif

  ScopedLifecycleAttempt(const ScopedLifecycleAttempt &) = delete;
  ScopedLifecycleAttempt & operator=(const ScopedLifecycleAttempt &) = delete;

private:
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  template_api::TemplateWitnessSession * session_ = nullptr;
  std::uint64_t token_ = 0;
#endif
};

struct RendererEventLineage
{
  std::uint64_t event_id = 0;
  std::uint64_t table_row_id = 0;
  std::vector<WitnessProducerSite> producers;
  std::vector<WitnessUpstreamRoute> upstream_routes;
};

std::vector<RendererEventLineage> renderer_table_lineages(
    const template_api::TemplateWitnessSession & session);

void note_renderer_action(const template_api::TemplateWitnessSession & session,
                          const std::string & source_path,
                          const std::string & pass,
                          const std::string & action,
                          const RendererEventLineage & lineage,
                          const std::string & kind,
                          const std::string & location,
                          const std::string & template_name,
                          const std::string & changed_fields = std::string());

void note_renderer_final_visible(
    const template_api::TemplateWitnessSession & session,
    const std::string & source_path,
    const RendererEventLineage & lineage,
    const std::string & kind,
    const std::string & location,
    const std::string & template_name);

void note_semantic_consolidation(
    const template_api::TemplateWitnessSession & session,
    const std::string & family,
    std::size_t completed_candidates,
    std::size_t early_repeats,
    std::size_t prepublication_merges,
    std::size_t collected_occurrences,
    std::size_t published_occurrences);

void finish_session(const template_api::TemplateWitnessSession & session,
                    const std::string & source_path);

}  // namespace witness_provenance

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
#define CPPGM_SET_WITNESS_PRODUCER(object, site) \
  ((object).producer_site = (site))
#define CPPGM_WITNESS_PROVENANCE_PARAMETER(parameter) parameter,
#define CPPGM_WITNESS_PROVENANCE_ARGUMENT(argument) argument,
#define CPPGM_NOTE_TEMPLATE_WITNESS_LIFECYCLE_EVENT(site, event) \
  template_api::note_template_witness_lifecycle_event((event), (site))
#else
#define CPPGM_SET_WITNESS_PRODUCER(object, site) ((void)0)
#define CPPGM_WITNESS_PROVENANCE_PARAMETER(parameter)
#define CPPGM_WITNESS_PROVENANCE_ARGUMENT(argument)
#define CPPGM_NOTE_TEMPLATE_WITNESS_LIFECYCLE_EVENT(site, event) \
  template_api::note_template_witness_lifecycle_event((event))
#endif
