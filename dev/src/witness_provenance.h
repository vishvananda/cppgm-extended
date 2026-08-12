#pragma once

#include <string>

namespace semantic_source_use {
struct SemanticSourceUse;
}

namespace template_api {
struct TemplateLifecycleEvent;
struct TemplateWitnessSession;
}

namespace witness_provenance {

enum class WitnessUpstreamRoute
{
  Unknown = 0,
  AliasCanonicalOccurrence,
  ClassResolvedTemplateId,
  ClassDeclarationTypeSource,
  ClassExplicitSpecializationSource,
  ClassQualifiedValueSource,
  ClassNestedSourceTemplateId,
  FunctionConstantValueLookup,
  FunctionConversion,
  FunctionDeclval,
  FunctionOverloadResolution,
  VariableDirectInstantiation,
  VariableInitializerReplay,
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

void note_source_use_publication(
    template_api::TemplateWitnessSession * session,
    const semantic_source_use::SemanticSourceUse & use);

void note_lifecycle_publication(
    template_api::TemplateWitnessSession & session,
    const template_api::TemplateLifecycleEvent & event);

void finish_session(const template_api::TemplateWitnessSession & session,
                    const std::string & source_path);

}  // namespace witness_provenance

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
#define CPPGM_WITNESS_PROVENANCE_PARAMETER(parameter) parameter,
#define CPPGM_WITNESS_PROVENANCE_ARGUMENT(argument) argument,
#else
#define CPPGM_WITNESS_PROVENANCE_PARAMETER(parameter)
#define CPPGM_WITNESS_PROVENANCE_ARGUMENT(argument)
#endif
