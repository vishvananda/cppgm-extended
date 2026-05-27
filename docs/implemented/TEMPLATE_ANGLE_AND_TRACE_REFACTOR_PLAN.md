# Template Angle And Trace Refactor Plan

This plan tracks the parser/template-angle cleanup and the tracing improvements
needed to make hosted frontier debugging faster and less repetitive.

## Goal

Reduce repeated hosted failures caused by nested template parsing and semantic
fragment reparsing by:

- consolidating `<...>` disambiguation into one shared implementation
- making semantic fragment reparsing use the same name/disambiguation state as
  translation-unit parsing
- adding structured decision traces that explain why a parse or semantic
  viability decision was taken

## Phase Execution Rules

Every stage in this plan should follow the same closeout process before the
next stage starts:

- run the full `pa10` through `pa32` validation sweep from the repo root
- prefer:
  - `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- if the fast worker path is not appropriate for the change under test, fall
  back to:
  - `make verify-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- do not begin the next stage with an uncommitted mixed worktree
- after validation passes, commit the completed stage before starting the next
  one

The intent is to keep each refactor stage independently validated and
checkpointed, so a later frontier investigation does not get mixed together
with partially-finished plan work.

## Stages

### Stage 1. Close The Fragment-Reparse State Gap

Status: `completed`

Problem:

- the active parser tracks visible value names and non-type template parameter
  names when deciding whether `<` opens a nested template-id
- semantic fragment reparsing currently seeds only visible type and template
  names

Planned changes:

- add parser seeding for visible value names
- add parser seeding for visible non-type template parameter names
- collect those names from semantic scopes in fragment reparsing

Success criteria:

- fragment parsers receive the same value-side disambiguation inputs as the
  translation-unit parser
- targeted parser/template regressions still pass

Completed in the current tree:

- added parser seeding for visible value names
- added parser seeding for visible non-type template parameter names
- fragment reparsing in `callsemantic.cpp` now seeds both from semantic scope

Validation:

- `make test-worker` in `pa10`
- `make test-worker` in `pa21`
- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- direct hosted compile of `HHC-396` still passes
- direct hosted compile of `HHC-397` still fails with the same allocator-traits
  ambiguity, confirming no unintended frontier movement
- commit the stage before beginning Stage 2

### Stage 2. Extract Shared Template-Id Token Parser

Status: `completed`

Problem:

- nested template-id parsing and `<` disambiguation logic live inside
  `cppast_parser.cpp`
- semantic code still uses separate text scanners

Planned changes:

- create a shared token-based template-id parser module
- move angle scanning, argument splitting, and nested-template opening checks
  there
- make the active parser call that shared module

Success criteria:

- one token-based implementation owns nested template-id parsing
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- added shared token-based parsing in `dev/src/template_angle_parser.h/.cpp`
- moved nested-angle opening checks, template-id suffix range parsing, and
  qualified-component scanning into that shared module
- updated `cppast_parser.cpp` to delegate template-id suffix and angle-clause
  parsing to the shared module

Validation:

- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage before beginning Stage 3

### Stage 3. Replace Semantic String Scanners

Status: `completed`

Problem:

- `callsemantic.cpp` still contains separate character-based template-id and
  angle scanning helpers

Planned changes:

- reimplement semantic template-id parsing on top of tokenization plus the
  shared template-id parser
- remove the duplicated character scanners

Success criteria:

- template-id parsing in semantic flows and parser flows shares one
  implementation
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- replaced semantic string-based template-id parsing in `callsemantic.cpp`
  with tokenization plus the shared template-angle parser
- updated scope-aware semantic parsing to seed visible type/template/value
  names into the shared lookup used by template-id parsing
- updated function template deduction in `template_resolution.cpp` to route
  scoped template-id parsing through the shared semantic context entrypoint

Validation:

- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage before beginning Stage 4

### Stage 4. Retire Or Delegate Legacy Cursor Parsing

Status: `completed`

Problem:

- `RecogTokenCursor::parse_template_id()` is still a separate compatibility path

Planned changes:

- delegate to the shared parser or delete the legacy helper if no longer needed

Success criteria:

- no separate legacy template-id parsing path remains active
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- updated `RecogTokenCursor::parse_template_id()` to delegate to the shared
  template-angle parser instead of using a separate legacy implementation

Validation:

- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage before beginning Stage 5

### Stage 5. Add Structured Trace Core

Status: `completed`

Problem:

- current tracing is stack-oriented and often misses the exact decision facts
  behind parser and template failures

Planned changes:

- add a structured ring-buffer trace system with categories such as:
  - `parser.angle`
  - `parser.fragment`
  - `template.resolve`
  - `template.instantiate`
  - `overload`
- add env-controlled filtering and dump-on-error behavior

Success criteria:

- a hosted failure can dump recent high-signal decision events without enabling
  noisy live logging everywhere
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- added `dev/src/parser_trace.h/.cpp`
- implemented a filtered thread-local ring buffer with:
  - category filters via `CPPGM_TRACE`
  - optional file/symbol filters
  - trace limit control
  - optional live logging
  - dump-on-error support via `CPPGM_TRACE_ON_ERROR`
- parser top-level failures now append the buffered trace, and semantic
  top-level callsemantic failures now do the same through the shared
  diagnostic-context message path

Validation:

- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage before beginning Stage 6

### Stage 6. Add High-Value Decision Trace Points

Status: `completed`

Planned changes:

- trace nested-angle open/close decisions
- trace fragment parse attempts and fallbacks
- trace template candidate substitution/drop reasons
- trace overload ambiguity retention reasons

Success criteria:

- parser/template failures include materially better decision context than the
  current ad hoc tracing
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- added `parser.angle` events in the shared template-angle parser for:
  - nested-angle opening decisions
  - unknown-nested-template-id boundary analysis
- added `parser.fragment` events for:
  - template-id suffix parsing
  - fragment parse ambiguity decisions in `parse_primary_expression`
- added `template.resolve` events in overload and deduction paths for:
  - template candidate enumeration
  - deduction success/failure
  - non-dependent-parameter skip decisions
- added `overload` events for candidate comparison and final winner/ambiguity
  selection

Validation:

- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage before beginning Stage 7

### Stage 7. Optional Local Trace Triggers

Status: `deferred (optional)`

Planned changes:

- add optional local trace trigger pragmas for reduced repro files

Success criteria:

- local reduced repros can request focused trace dumps without enabling broad
  global tracing
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Current decision:

- deferred for now because the env-filtered trace controls and dump-on-error
  path already cover the main debugging workflow for frontier and reduction
  work
- this stage remains explicitly optional and does not block completion of the
  required parser-angle consolidation/audit program

### Stage 8. Build A Template-Angle Ambiguity Audit Matrix

Status: `completed`

Planned changes:

- add a dedicated parser/template audit block that covers:
  - known type vs known value before `<`
  - nested template-ids
  - dependent names
  - `decltype`, `sizeof`, casts, base clauses, and default template arguments
  - fragment parse contexts as well as TU parsing

Success criteria:

- a fix to template-angle disambiguation is validated against a deliberate audit
  matrix rather than only the current hosted smoke
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- added `TEMPLATE_ANGLE_AUDIT_MATRIX.md`
- documented parser and semantic fragment/reparse audit rows
- added parser audit cases for template-id followers that had not been covered
  explicitly:
  - `pa10/tests/spec/246-template-id-function-pointer-argument.t`
  - `pa10/tests/spec/247-template-id-function-pointer-initializer.t`

Validation:

- `make test-worker` in `pa10`
- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage before beginning Stage 9

### Stage 9. Tighten Frontier Process Integration

Status: `completed`

Planned changes:

- require the angle audit and hosted sweep whenever the frontier lands in a
  parser/template family

Success criteria:

- the hosted frontier process explicitly requires this audit/commit discipline
  for parser/template families
- stage closes only after `make verify-fast-pa10-31 ...` passes and the result
  is committed

Completed in the current tree:

- updated `HOSTED_HEADER_FRONTIER_PROCESS.md` so parser/template frontier items
  must run `TEMPLATE_ANGLE_AUDIT_MATRIX.md` as part of closure/discovery
- recorded that parser/template items are not ready to close until the relevant
  audit rows, the active smoke, and the hosted sweep have all been rerun

Validation:

- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- commit the stage to close the required plan work
