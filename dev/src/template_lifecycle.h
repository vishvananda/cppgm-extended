#pragma once

namespace template_api {

enum class TemplateLifecycleTransitionKind
{
  None,
  DefinitionRequired,
  DefinitionEnsured,
  DefinitionMaterialized,
  Instantiated,
  Finalized,
  AnonymousMemberInstantiated,
};

enum class TemplateLifecycleCause
{
  None,
  TrackInstantiation,
  RequireDefinition,
  EnsureDefinition,
  FinalizeClass,
  ExplicitInstantiationDeclaration,
  ExplicitInstantiationDefinition,
  ExplicitSpecialization,
  ImplicitUse,
};

}  // namespace template_api
