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

  ClassCallsemantic06,
  ClassCallsemantic07,
  ClassCallsemantic08,
  ClassCallsemantic10,
  ClassCallsemantic13,
  ClassTemplateReference02,
  ClassConstantValueLookup02,
  ClassConstantValueLookup03,
  ClassTemplateInstantiation,

  AliasTemplateArgumentSemantics02,
  AliasCallsemantic02,
  AliasCallsemantic03,

  FunctionSemanticTemplateFunction,

  VariableTemplateInstantiation,

  LifecycleTemplateApi01,
  LifecycleTemplateApi02,
  LifecycleTemplateApi03,
  LifecycleTemplateApi04,
  LifecycleTemplateApi05,
  LifecycleTemplateApi06,
  LifecycleTemplateApi07,
  LifecycleTemplateApi09,
  LifecycleSemanticClassModel,
  LifecycleTemplateArgumentSemantics02,
  LifecycleCallsemantic01,
  LifecycleCallsemantic02,
  LifecycleConstantValueLookup02,
};

enum class WitnessUpstreamRoute
{
  NestedClassUseFromAstNode,
  NestedClassUseFromTemplateArguments,
  StaticMemberDefinitionClassUseFromAstNode,
  DeducedClassUseFromResolvedAliasType,
};

const char * producer_site_name(WitnessProducerSite site);
const char * upstream_route_name(WitnessUpstreamRoute route);

bool enabled();

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

void note_upstream_route(template_api::TemplateWitnessSession * session,
                         WitnessUpstreamRoute route);

struct RendererEventLineage
{
  std::uint64_t event_id = 0;
  std::uint64_t table_row_id = 0;
  std::vector<WitnessProducerSite> producers;
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

void finish_session(const template_api::TemplateWitnessSession & session,
                    const std::string & source_path);

}  // namespace witness_provenance

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
#define CPPGM_NOTE_WITNESS_UPSTREAM_ROUTE(session, route) \
  witness_provenance::note_upstream_route(session, route)
#define CPPGM_SET_WITNESS_PRODUCER(object, site) \
  ((object).producer_site = (site))
#define CPPGM_WITNESS_PROVENANCE_PARAMETER(parameter) parameter,
#define CPPGM_WITNESS_PROVENANCE_ARGUMENT(argument) argument,
#define CPPGM_NOTE_TEMPLATE_WITNESS_LOG_EVENT(site, ...) \
  template_api::note_template_witness_log_event(site, __VA_ARGS__)
#else
#define CPPGM_NOTE_WITNESS_UPSTREAM_ROUTE(session, route) ((void)0)
#define CPPGM_SET_WITNESS_PRODUCER(object, site) ((void)0)
#define CPPGM_WITNESS_PROVENANCE_PARAMETER(parameter)
#define CPPGM_WITNESS_PROVENANCE_ARGUMENT(argument)
#define CPPGM_NOTE_TEMPLATE_WITNESS_LOG_EVENT(site, ...) \
  template_api::note_template_witness_log_event(__VA_ARGS__)
#endif
