# CPPGM Template Witness Adapter Prototype

This document describes the experimental `cppgm`-side witness adapter added in:

- [dev/cppgm_template_witness.py](/private/tmp/cppgm-template-kernel-20260416/dev/cppgm_template_witness.py)
- [dev/compare_template_witness.py](/private/tmp/cppgm-template-kernel-20260416/dev/compare_template_witness.py)

The goal is to push `cppgm` into a witness shape that is comparable with the
patched Clang witness and close enough to a student-facing assignment surface.

## Purpose

The patched Clang build already emits a clean witness for:

- class-template uses
- alias-template uses
- variable-template uses
- function-template call winners and bindings

`cppgm` already knows much more internally, but today that information is only
visible through the raw `template.resolve` trace.

This adapter turns that raw trace into a normalized JSON witness without
editing the dirty compiler worktree first.

## Current Surface

The adapter runs `cppgm++` with:

- `CPPGM_TRACE=template.resolve`
- `CPPGM_TRACE_LIVE=1`
- `CPPGM_TRACE_FILE=<basename by default>`
- optional `CPPGM_TRACE_SYMBOL=<substring>`

It then parses the trace into `template_witness_v1` JSON with:

- `class_use`
- `alias_use`
- `variable_use`
- `function_call`

The strongest event today is `function_call`.

For focused cases like `479-dependent-variable-template-empty-pack-enable-if-selection.t`,
the adapter can recover:

- selected function-template call
- use-site location
- deduced bindings
- owner/outer explicit bindings
- unnamed/defaulted trailing arguments reconstructed from the instantiation key
- dropped candidate plus a normalized `deduce_finalize_failed` reason

That is already close to the assignment-facing contract we actually care about.

## Reliability Split

The current prototype is not equally strong across all event kinds.

Strong:

- `function_call`
  - winner selection
  - location
  - bindings
  - loser/failure capture for common deduction-finalization failures

Medium:

- `class_use`
  - selection kind when `class-specialization` is traced
  - primary use inference from qualified call expressions

Heuristic:

- `alias_use`
  - inferred from alias-instantiation trace lines
  - location comes from the surrounding trace context, not a dedicated alias
    use-site hook
- `variable_use`
  - inferred from variable-template instantiation trace lines
  - location is contextual

This is still useful for the assignment buildout because the `function_call`
surface is the hardest important student-facing decision and it is already
recoverable.

## Commands

Generate a `cppgm` witness for test `479`:

```bash
cd /private/tmp/cppgm-template-kernel-20260416
./dev/cppgm_template_witness.py \
  --cppgm /private/tmp/cppgm-clang-postlowir-20260414/dev/cppgm++ \
  --input /private/tmp/cppgm-clang-postlowir-20260414/pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t \
  --symbol-filter construct \
  -o /tmp/cppgm-479-witness.json
```

Generate a partial witness for `542` even if `cppgm` still fails:

```bash
cd /private/tmp/cppgm-template-kernel-20260416
./dev/cppgm_template_witness.py \
  --cppgm /private/tmp/cppgm-clang-postlowir-20260414/dev/cppgm++ \
  --input /private/tmp/cppgm-clang-postlowir-20260414/pa33/tests/compile/542-local-functor-std-function-assignment.t \
  --symbol-filter operator= \
  --allow-failure \
  -o /tmp/cppgm-542-witness.json
```

Compare a patched Clang witness against the `cppgm` witness on the common
fields:

```bash
cd /private/tmp/cppgm-template-kernel-20260416
./dev/compare_template_witness.py \
  --left /tmp/clang-479-witness.json \
  --right /tmp/cppgm-479-witness.json
```

Observed current result on `479`:

- `match: no comparable-field mismatches`

That means the current adapter and comparator already line up with the patched
Clang witness on the important overlapping decisions:

- selected call
- class-template owner
- named bindings such as `Alloc=int` and `Tp=int`
- empty-pack binding shape

The remaining differences on this case are expected side-surface differences:

- Clang emits two explicit alias uses; the `cppgm` adapter currently collapses
  the symbol-filtered alias story into one deferred alias event
- `cppgm` emits variable-template instantiations that the current Clang witness
  does not

## Why This Is Useful For The Assignment

This adapter gives us a practical intermediate step between:

- raw compiler-internal trace data
- the final student-facing assignment contract

That matters because it lets us do all of the following before committing to a
full assignment rewrite:

- diff `cppgm` decisions against Clang on real source tests
- identify which student-visible facts are already recoverable
- prove that the student-facing witness is not just a synthetic kernel artifact
- keep complex cases reduced into `.tkq` while still validating the reductions
  against original-source witnesses

## Limits

This adapter is intentionally not pretending to be the final oracle.

It still depends on:

- the current `template.resolve` trace wording
- trace-context heuristics for alias/class/variable locations
- the underlying `cppgm` compile getting far enough to emit the relevant trace

Hosted-heavy failures like `542` still expose real compiler gaps. The adapter
does not hide that.

Current `542` result:

- with `--symbol-filter operator=` the adapter preserves the failure summary but
  no student-facing template event is emitted before the current `cppgm`
  failure
- without a symbol filter the trace reaches many hosted-header template events,
  but they are too noisy to count as a good assignment oracle for this case yet

So `542` is a good stress/discovery case, but not yet a strong witness-driven
assignment oracle on the `cppgm` side.

## Recommended Next Step

If this witness shape proves valuable, the next step should be to move the most
important `function_call` fields behind a direct `cppgm++` JSON witness mode
instead of keeping them only as a trace adapter.

That would make the assignment-facing contract:

- more stable
- less heuristic
- easier to compare directly with the patched Clang witness and `tmplsolve`
