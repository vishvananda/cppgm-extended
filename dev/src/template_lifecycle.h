#pragma once

namespace template_api {

enum class TemplateLifecycleEntityKind
{
  None,
  Function,
  Class,
  Value,
};

enum class TemplateLifecycleTransitionKind
{
  None,
  Acquired,
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
  ExternTemplateSuppressed,
  NoEagerInstantiationSuppressed,
  ClassFinalizationMemberMaterialization,
};

}  // namespace template_api
