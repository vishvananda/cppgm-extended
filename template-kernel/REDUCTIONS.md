# Template Kernel Reductions

This file tracks existing compiler/validation coverage that has been reduced
into the standalone template-kernel surface.

The goal is to make `template-kernel/tests/` the first landing place for
template regressions that can be expressed at this boundary.

## Current Reductions

| Kernel case | Source coverage | Focus |
| --- | --- | --- |
| `008-default-template-argument-merge` | `validation/tests/014-default-template-argument-merge.cpp`, `pa21/tests/spec/200-forward-declared-class-template-default-merge.t` | default template argument merge across redeclarations |
| `009-partial-specialization-uses-primary-default` | `pa21/tests/spec/215-partial-specialization-uses-primary-default-argument.t` | partial specialization matching after primary default binding |
| `010-function-template-defaulted-class-template-arg-deduction` | `pa22/tests/spec/438-function-template-defaulted-class-template-arg-deduction.t` | function deduction over a type that depends on a defaulted class-template argument |
| `011-nondeduced-context-only` | `validation/tests/112-nondeduced-context-only.cpp` | parameter used only in a non-deduced context must fail deduction |
| `013-ref-vs-const-ref-partial-ordering` | `validation/tests/035-partial-ordering-ref-vs-const-ref.cpp` | reference-sensitive overload selection between `T&` and `const T&` |
| `014-ambiguous-cv-pointer-partial-ordering` | `validation/tests/113-ambiguous-cv-pointer-partial-ordering.cpp` | cv-symmetric pointer templates remain ambiguous |
| `015-pointer-qualification-deduction` | `validation/tests/037-pointer-qualification-deduction.cpp` | pointer deduction accepts qualification conversion on the pointee |
| `016-pa35-function-reference-parameter-shape` | `pa35/tests/link/701-hosted-function-reference-parameter-link-smoke.t.1` | function-reference-shaped argument binds through `const Fn&` |
| `017-pa34-forward-array-shape` | `pa34/tests/compile/656-forward-array-string-pair.t` | forwarding reference preserves an lvalue array shape |
| `018-function-shape-reference-deduction` | `validation/tests/034-function-reference-deduction.cpp` | function-reference shape decomposes into return and parameter types |
| `019-array-bound-nontype-deduction` | `validation/tests/033-array-reference-deduction.cpp` | array-bound deduction binds a bounded integer nontype parameter |
| `020-bool-int-sentinel-enable-if-shape` | `pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t` | bool trait result plus defaulted `int = 0` sentinel splits viable candidates |

## Synthetic Coverage

| Kernel case | Purpose |
| --- | --- |
| `012-stats-scale-repeated-queries` | repeated-query counter coverage for the stats/performance surface |

## Reduction Rules

When adding a new reduction:

1. Keep the `.tkq` case at the first owned template boundary.
2. Prefer structural declarations and queries over source-faithful syntax.
3. Preserve the semantic point of the original test, not its full incidental
   surrounding code.
4. Add a source comment near the top of the `.tkq` file.
5. Keep the `.ref` output deterministic and focused on the observable template
   decision.

## Notes On Test 542

`pa34/tests/compile/542-local-functor-std-function-assignment.t` is not one
kernel case. It pulls in several reusable template forms.

The current kernel reductions that cover the most important pieces are:

- `016-pa35-function-reference-parameter-shape`
- `020-bool-int-sentinel-enable-if-shape`

That split is intentional. The top-level `std::function` assignment surface is
less useful pedagogically than the smaller recurring template mechanisms it
instantiates.
