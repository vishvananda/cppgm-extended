# Template Boundary Audit Checklist

This checklist is the Stage 1 audit target for the template decision ownership
work. It defines which crossings are intentional while the implementation is
still being moved behind `template_api`.

## Semantic -> Template Crossings

Non-template semantic and output code may cross into the template subsystem only
through `dev/src/template_api.h`.

`TemplateServices` and the concrete `SemanticContextTemplateServices` adapter
are implementation glue, not the semantic-facing API. Semantic files should not
construct the adapter directly; add a narrow `template_api` wrapper when a
semantic caller needs a template operation that is currently only exposed below
the boundary.

Allowed request families:

- template argument parsing, binding, and substitution requests
- function template deduction and specialization selection requests
- class, function, variable, and nested-member instantiation requests
- output-facing template ownership/readiness queries
- explicit lifecycle/witness context requests
- template-owned lookup-identity helpers

Forbidden outside template-owned implementation code:

- direct reads of `source_template`, `template_instantiation_key`,
  `instantiation_arguments`, or specialization/suppression flags
- direct construction of template witness lifecycle events from output refresh
- direct calls into concrete implementation headers such as
  `template_instantiation.h`, `template_resolution.h`,
  `template_selection.h`, or `template_scope.h`
- direct construction of `TemplateServices` or `SemanticContextTemplateServices`
  from non-template semantic files
- owner-chain walks to infer template identity when a `template_api` query can
  answer the question

## Template -> Semantic Crossings

Template implementation code should call semantic services only through an
explicit request/result API or a focused service surface.

Allowed callback categories:

- type-system queries over already-resolved type handles
- constant evaluation and builtin trait evaluation
- entity materialization hooks that deliberately create semantic entities
- audited recursive semantic gateways where a leaf service is not yet practical

The current service surface is:

- `TemplateTypeSystem::prepare_named_type_member_scope(...)`
- `TemplateTypeSystem::resolve_direct_type_lookup(...)`
- `TemplateTypeSystem::resolve_selected_class_template_id(...)`
- `TemplateRecursiveSemanticGateway::evaluate_dependent_type_expression(...)`
- `TemplateRecursiveSemanticGateway::evaluate_semantic_builtin_type_trait(...)`
- `TemplateRecursiveSemanticGateway::evaluate_initializer_constant_value(...)`

There is no general recursive template-id type-lookup callback. If a future
case appears to need one, it should be introduced as explicitly named
temporary debt rather than hidden behind a leaf service.

Forbidden callback patterns:

- hidden full semantic lookup from a nominal leaf adapter
- arbitrary synthesized text crossing the boundary for reparsing
- witness/location callbacks outside `TemplateWitnessContext`
- output refresh side effects that become lifecycle decisions

## Current Temporary Exception

`dev/src/callsemantic.cpp` still contains both semantic orchestration and
template-owned implementation code. Until that file is split further:

- semantic/output orchestration in `callsemantic.cpp` should use `template_api`
- the in-place template-owned implementation sections may still read and write
  template internals directly
- new code should not expand that exception
- when touching a direct metadata read, first decide whether it is a semantic
  caller or template-owned implementation code

## Audit Commands

Use these searches while reviewing changes:

```sh
rg -n '#include "template_(argument_semantics|decl_ast|instantiation|resolution|selection|services|scope|function_signature).*"' dev/src/semantic_*.cpp dev/src/semantic_*.h dev/src/output_requirement_engine.* dev/src/callsemantic_phase_bridge.*
rg -n 'template_argument_semantics::|template_decl_ast::|template_selection::|template_resolution::|template_instantiation::|template_specialization::|template_function_signature::|template_scope::|SemanticContextTemplateServices|TemplateServices' dev/src/semantic_*.cpp dev/src/semantic_*.h dev/src/output_requirement_engine.* dev/src/callsemantic_phase_bridge.*
rg -n 'source_template|template_instantiation_key|instantiation_arguments|suppress_implicit_instantiation|is_explicit_specialization' dev/src/semantic_output.cpp dev/src/output_requirement_engine.cpp
rg -n '#include "template_(instantiation|resolution|selection|scope|witness|model)' dev/src/semantic_output.cpp dev/src/output_requirement_engine.cpp
rg -n 'note_.*closure_event|maybe_enter_.*closure_context|TemplateWitnessLogEventKind' dev/src/semantic_output.cpp dev/src/output_requirement_engine.cpp
```

For `callsemantic.cpp`, review matches by ownership instead of treating every
match as a failure:

```sh
rg -n 'source_template|template_instantiation_key|instantiation_arguments|note_.*closure_event|maybe_enter_.*closure_context' dev/src/callsemantic.cpp
```
