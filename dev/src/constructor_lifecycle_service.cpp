#include "constructor_lifecycle_service.h"

#include "semantic_overload.h"

namespace constructor_lifecycle_service {

using namespace semantic_conversion;
using namespace semantic_model;

namespace {

ConstructorSelectionOptions derive_selection_options(
    const ConstructorSelectionProfile & profile)
{
  ConstructorSelectionOptions options;
  options.context = profile.context;
  options.instantiate_bodies = profile.instantiate_bodies;
  switch(profile.intent) {
  case ConstructorIntent::DirectInitialization:
    break;
  case ConstructorIntent::NoUserDefinedConstructors:
    options.allow_user_defined = false;
    break;
  case ConstructorIntent::ImplicitDefaultConstructorViability:
    options.allow_user_defined = false;
    options.instantiate_bodies = false;
    options.no_viable_is_expected = true;
    break;
  case ConstructorIntent::UserDefinedConversionConstructorProbe:
    options.allow_user_defined = false;
    options.allow_explicit = false;
    options.user_defined_conversion_source = true;
    options.no_viable_is_expected = true;
    break;
  case ConstructorIntent::NonExplicitConstruction:
    options.allow_explicit = false;
    options.prefer_conversion_function_object_result = true;
    options.non_explicit_construction = true;
    break;
  case ConstructorIntent::CopyListInitialization:
    options.allow_explicit = false;
    break;
  case ConstructorIntent::AggregateConstruction:
    options.allow_explicit = false;
    options.allow_aggregate = true;
    options.allow_partial_aggregate = profile.allow_partial_aggregate;
    break;
  case ConstructorIntent::AggregatePartialMatch:
    options.allow_aggregate = true;
    options.allow_partial_aggregate = profile.allow_partial_aggregate;
    break;
  }
  return options;
}

}  // namespace

ConstructorSelectionProfile direct_initialization_profile(const char * context)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::DirectInitialization;
  profile.context = context;
  return profile;
}

ConstructorSelectionProfile no_user_defined_constructor_profile(const char * context)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::NoUserDefinedConstructors;
  profile.context = context;
  return profile;
}

ConstructorSelectionProfile implicit_default_constructor_viability_profile(
    const char * context)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::ImplicitDefaultConstructorViability;
  profile.context = context;
  profile.instantiate_bodies = false;
  return profile;
}

ConstructorSelectionProfile user_defined_conversion_constructor_probe_profile(
    const char * context,
    bool instantiate_bodies)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::UserDefinedConversionConstructorProbe;
  profile.context = context;
  profile.instantiate_bodies = instantiate_bodies;
  return profile;
}

ConstructorSelectionProfile non_explicit_construction_profile(const char * context)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::NonExplicitConstruction;
  profile.context = context;
  return profile;
}

ConstructorSelectionProfile copy_list_initialization_profile(const char * context)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::CopyListInitialization;
  profile.context = context;
  return profile;
}

ConstructorSelectionProfile aggregate_construction_profile(
    const char * context,
    bool allow_partial_aggregate)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::AggregateConstruction;
  profile.context = context;
  profile.allow_partial_aggregate = allow_partial_aggregate;
  return profile;
}

ConstructorSelectionProfile aggregate_partial_match_profile(
    const char * context,
    bool allow_partial_aggregate)
{
  ConstructorSelectionProfile profile;
  profile.intent = ConstructorIntent::AggregatePartialMatch;
  profile.context = context;
  profile.allow_partial_aggregate = allow_partial_aggregate;
  return profile;
}

ConstructorSelectionOptions selection_options_for(
    const ConstructorSelectionProfile & profile)
{
  return derive_selection_options(profile);
}

void apply_selection_profile(ConstructorSelectionOptions & options,
                             const ConstructorSelectionProfile & profile)
{
  const ConstructorSelectionOptions derived = derive_selection_options(profile);
  if((!options.context || !options.context[0]) && derived.context && derived.context[0]) {
    options.context = derived.context;
  }
  options.allow_user_defined =
      options.allow_user_defined && derived.allow_user_defined;
  options.instantiate_bodies =
      options.instantiate_bodies && derived.instantiate_bodies;
  options.prefer_conversion_function_object_result =
      options.prefer_conversion_function_object_result ||
      derived.prefer_conversion_function_object_result;
  options.allow_aggregate =
      options.allow_aggregate || derived.allow_aggregate;
  options.allow_partial_aggregate =
      options.allow_partial_aggregate || derived.allow_partial_aggregate;
  options.allow_explicit =
      options.allow_explicit && derived.allow_explicit;
  options.initializer_list_only =
      options.initializer_list_only || derived.initializer_list_only;
  options.user_defined_conversion_source =
      options.user_defined_conversion_source ||
      derived.user_defined_conversion_source;
  options.non_explicit_construction =
      options.non_explicit_construction || derived.non_explicit_construction;
  options.no_viable_is_expected =
      options.no_viable_is_expected || derived.no_viable_is_expected;
}

ConstructorSelectionResult select_constructor_from_exprs(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const std::vector<ExprInfo> & source_args,
    const ConstructorSelectionOptions & options)
{
  ConstructorSelectionResult result;
  select_constructor_from_exprs_into(ctx, scope, info, source_args, result, options);
  return result;
}

void select_constructor_from_exprs_into(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const std::vector<ExprInfo> & source_args,
    ConstructorSelectionResult & out,
    const ConstructorSelectionOptions & options)
{
  out = ConstructorSelectionResult();
  out.ctor = semantic_overload::select_constructor_from_exprs(ctx,
                                                              scope,
                                                              info,
                                                              source_args,
                                                              out.converted_args,
                                                              &out.ranks,
                                                              options);
}

ConstructorSelectionResult select_constructor(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const std::vector<const CppAstNode *> & arg_nodes,
    const ConstructorSelectionOptions & options)
{
  ConstructorSelectionResult result;
  select_constructor_into(ctx, scope, info, arg_nodes, result, options);
  return result;
}

void select_constructor_into(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const std::vector<const CppAstNode *> & arg_nodes,
    ConstructorSelectionResult & out,
    const ConstructorSelectionOptions & options)
{
  out = ConstructorSelectionResult();
  out.ctor = semantic_overload::select_constructor(ctx,
                                                   scope,
                                                   info,
                                                   arg_nodes,
                                                   out.converted_args,
                                                   options);
}

ConstructorSelectionResult select_constructor_for_direct_braced_init(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const CppAstNode & direct_braced_init,
    const ConstructorSelectionOptions & options)
{
  ConstructorSelectionResult result;
  select_constructor_for_direct_braced_init_into(ctx,
                                                 scope,
                                                 info,
                                                 direct_braced_init,
                                                 result,
                                                 options);
  return result;
}

void select_constructor_for_direct_braced_init_into(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const CppAstNode & direct_braced_init,
    ConstructorSelectionResult & out,
    const ConstructorSelectionOptions & options)
{
  out = ConstructorSelectionResult();
  out.ctor = semantic_overload::select_constructor_for_direct_braced_init(ctx,
                                                                          scope,
                                                                          info,
                                                                          direct_braced_init,
                                                                          out.converted_args,
                                                                          options);
}

bool selected_constructor_allows_direct_materialization(
    const ClassInfo & target,
    const ConstructorSelectionResult & selection)
{
  if(!selection.ctor ||
     (!selection.ctor->is_copy_constructor &&
      !selection.ctor->is_move_constructor) ||
     selection.converted_args.size() != 1) {
    return false;
  }

  const ExprInfo & source = selection.converted_args[0];
  cpp_decl::TypePtr source_type =
      strip_top_level_cv(remove_reference_type(source.type));
  cpp_decl::TypePtr target_type = strip_top_level_cv(target.type);
  const bool direct_class_value =
      source.node.kind == CallSemKind::call_expression ||
      source.node.kind == CallSemKind::closure_object ||
      source.node.kind == CallSemKind::initializer_list_object;
  return source.category == VC_PRVALUE &&
         source_type && target_type &&
         type_equals(source_type, target_type) &&
         direct_class_value;
}

CallableEmissionDecision require_selected_constructor(
    OutputRequirementContext & ctx,
    const ConstructorSelectionResult & result,
    OutputReason reason,
    bool mark_output_required)
{
  return require_lifecycle_function(ctx, result.ctor, reason, mark_output_required);
}

CallableEmissionDecision require_lifecycle_function(OutputRequirementContext & ctx,
                                                    FunctionBinding * binding,
                                                    OutputReason reason,
                                                    bool mark_output_required)
{
  CallableEmissionDecision emission =
      ctx.decide_callable_emission(binding,
                                   reason,
                                   mark_output_required &&
                                       binding &&
                                       !binding->is_deleted);
  ctx.require_function_definition(binding, reason, emission.mark_output_required_now);
  return emission;
}

ConstructorActionResult prepare_selected_constructor_action(
    SemanticContext & ctx,
    const ExprInfo & object_ptr,
    const ConstructorSelectionResult & selection,
    bool trivial_lifecycle,
    OutputReason reason,
    bool mark_output_required)
{
  ConstructorActionResult result;
  prepare_selected_constructor_action_into(ctx,
                                           object_ptr,
                                           selection,
                                           trivial_lifecycle,
                                           reason,
                                           result,
                                           mark_output_required);
  return result;
}

void prepare_selected_constructor_action_into(
    SemanticContext & ctx,
    const ExprInfo & object_ptr,
    const ConstructorSelectionResult & selection,
    bool trivial_lifecycle,
    OutputReason reason,
    ConstructorActionResult & out,
    bool mark_output_required)
{
  out = ConstructorActionResult();
  out.ctor = selection.ctor;
  out.trivial_lifecycle = trivial_lifecycle;
  if(!out.ctor) {
    return;
  }

  out.call_args.reserve(1 + selection.converted_args.size());
  out.call_args.push_back(object_ptr);
  out.call_args.insert(out.call_args.end(),
                       selection.converted_args.begin(),
                       selection.converted_args.end());
  prepare_lifecycle_call_into(ctx,
                              out.ctor,
                              out.call_args,
                              out.trivial_lifecycle,
                              reason,
                              out,
                              mark_output_required);
}

ConstructorActionResult prepare_lifecycle_call(SemanticContext & ctx,
                                               FunctionBinding * binding,
                                               const std::vector<ExprInfo> & call_args,
                                               bool trivial_lifecycle,
                                               OutputReason reason,
                                               bool mark_output_required)
{
  ConstructorActionResult result;
  prepare_lifecycle_call_into(ctx,
                              binding,
                              call_args,
                              trivial_lifecycle,
                              reason,
                              result,
                              mark_output_required);
  return result;
}

void prepare_lifecycle_call_into(SemanticContext & ctx,
                                 FunctionBinding * binding,
                                 const std::vector<ExprInfo> & call_args,
                                 bool trivial_lifecycle,
                                 OutputReason reason,
                                 ConstructorActionResult & out,
                                 bool mark_output_required)
{
  std::vector<ExprInfo> normalized_call_args = call_args;
  out = ConstructorActionResult();
  out.ctor = binding;
  out.call_args.swap(normalized_call_args);
  out.trivial_lifecycle = trivial_lifecycle;
  if(!out.ctor) {
    return;
  }

  if(!out.trivial_lifecycle) {
    out.emission = require_lifecycle_function(ctx,
                                              out.ctor,
                                              reason,
                                              mark_output_required);
  }
  out.call_expr = ctx.make_direct_call_expr(*out.ctor,
                                            out.call_args,
                                            mark_output_required &&
                                                !out.trivial_lifecycle);
}

bool value_initialization_requires_zero_init(const FunctionBinding & binding)
{
  if(!binding.is_constructor) {
    return false;
  }
  if(binding.synthesized || binding.is_aggregate_constructor) {
    return true;
  }
  if(!binding.is_defaulted) {
    return false;
  }

  // An explicitly defaulted constructor on its first declaration is not
  // user-provided, so value-initialization still performs the zero-init phase.
  // Later out-of-class `= default` definitions keep a distinct declaration node
  // and should not take this path.
  return !binding.definition_node || binding.definition_node == binding.declaration_node;
}

}  // namespace constructor_lifecycle_service
