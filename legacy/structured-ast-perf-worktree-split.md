# Structured-AST Performance Worktree Split

## Worktree

- Path: `/private/tmp/cppgm-structured-ast-perf-20260428`
- Branch: `codex/structured-ast-perf-20260428`
- Base: `codex/template-main-integration-20260419` at `917c2434`

This worktree is for `legacy/structured-ast-perf-plan.md` work that can run
beside the witness-emission collapse without forcing merge conflicts.

## Conflict Boundary

While the witness-emission collapse is active, keep performance work out of
these files unless there is an explicit coordination point:

- `dev/src/witness_api.*`
- `dev/src/callsemantic.cpp`
- `dev/src/template_argument_semantics.*`
- `dev/src/template_specialization.cpp`
- `dev/src/template_instantiation.cpp`
- `dev/src/semantic_overload.cpp`
- `dev/src/semantic_conversion.cpp`
- `dev/src/template_witness_renderer.cpp`

Good first performance slices that avoid the collapse surface:

- Patch 4 parser speculation churn:
  `dev/src/cppast_parser.*`
- Patch 3 AST-node size/copy reduction:
  `dev/src/cppast_ast.h`, `dev/src/cppast_parser.cpp`
- Patch 6 scope fingerprint cache fast path:
  `dev/src/template_scope.cpp`
- Patch 7 class-method emit worklist was attempted and deferred because the
  naive index-based version perturbed synthesized-method output ordering.
  Revisit only with a design that preserves the current LowIR order.

Patch 1 and Patch 8 should wait until the collapse branch is stable, because
they naturally touch `callsemantic.cpp`, template argument semantics, and
template resolution plumbing.

Progress and performance tracking live in
`legacy/structured-ast-perf-progress.md`. The benchmark runner is
`scripts/run_structured_ast_perf_benchmarks.py`.

## Imported WIP

Only the low-conflict parser-only hunks from
`/private/tmp/cppgm-blowup-fix-20260427` were seeded here:

- `RecogTokenRangeSequence`, limited to the safer nested-parser range sites
- `suppress_template_argument_fragment_syntax`, limited to inherited parser
  state and the discarded declaration probe guard
- the inherited-name-lookup empty-local short-circuit

The broader range-view change in `parse_template_argument_fragment_syntax` was
left out because it changes current PA10 output relative to the branch's stale
refs.

The mixed WIP hunks in `callsemantic.cpp`, `template_resolution.cpp`,
`template_argument_semantics.*`, `semantic_builtins.*`, and
`semantic_class_model.cpp` were intentionally left behind.

## Validation

Initial seed validation included `make -C dev` and `make test-pa6`, but those
were only weak early smokes.

`test-pa6` is only a weak legacy-recognizer smoke here. It does not exercise
the `CppAstParser`/`cppgm++` path touched by this branch, so do not count it as
the parser regression gate for performance work.

Meaningful parser gates for this worktree:

- `make -C dev cppgm++`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`

Known current caveat: the branch has baseline PA10 reference drift in a few
cases around `decltype` and placement-new parsing, also visible with the
integration compiler. Treat those as stale-ref noise unless the diff set grows.

Do not use the whole PA34 harness as a routine perf-slice gate. Use the focused
PA34 long compile benchmarks from `scripts/run_structured_ast_perf_benchmarks.py
--include-pa34-perf` for timing signal instead. Those PA34 cases may be
known-error/measurable on this branch, so they are for relative timing/RSS
movement, not correctness signoff.

Before landing larger perf slices, use the measurement discipline from
`legacy/structured-ast-perf-plan.md`: capture before/after HEADs, run fixed
benchmarks at least three times, and compare medians.
