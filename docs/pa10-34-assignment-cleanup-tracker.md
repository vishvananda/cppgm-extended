# PA10-PA35 Assignment Cleanup Tracker

This is the working tracker for
[pa10-34-assignment-cleanup-process.md](/Users/vishvananda/cppgm/docs/pa10-34-assignment-cleanup-process.md).

It is intentionally more operational than the process doc:

- the process doc defines the passes and the rules
- this tracker records actual per-PA audit findings and deferred buildout notes

## Status Key

- `pending`: not reviewed yet
- `in_progress`: active pass work underway
- `pass_a_complete`: Pass A audit complete for this PA
- `pass_b_complete`: Pass B README / implementability complete for this PA
- `pass_c_complete`: Pass C cross-PA flow review complete for this PA
- `pass_d_complete`: deferred buildout complete for this PA

## Pass Summary

| PA | Pass A | Pass B | Pass C | Pass D | Notes |
| --- | --- | --- | --- | --- | --- |
| `pa10` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa11` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa12` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa13` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa14` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa15` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa16` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa17` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa18` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa19` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa20` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa21` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa22` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa23` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa24` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa25` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa26` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa27` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa28` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa29` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa30` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa32` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa33` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa34` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |
| `pa35` | pass_a_complete | pass_b_complete | pass_c_complete | pending | Pass A findings recorded below. |

Pass A is now complete across `pa10` through `pa35`.
Pass B is now complete across `pa10` through `pa35`.
Pass C is now complete across `pa10` through `pa35`.

## 2026-05-11 Test Grouping / Rename Pass

This pass applied the `tests/spec` policy from the strict-test cleanup:
`tests/spec/` is reserved for tests with a concrete spec anchor, and C++
source-language spec tests must start with a `// N3485 focus: ...` comment.
Broader regressions, integration reducers, hosted/library cases, ABI/link/runtime
smokes, and optimization/backend fixtures belong in `tests/general/` or another
role-specific bucket.

The reusable worker prompt is archived in
`docs/implemented/test-grouping-subagent-prompt.md`. Slice-level summaries, moved-test
lists, README changes, missing-test notes, validation, and open questions are
recorded in:

- `docs/implemented/test-grouping-notes-pa10-pa13.md`
- `docs/implemented/test-grouping-notes-pa14-pa17.md`
- `docs/implemented/test-grouping-notes-pa18-pa22.md`
- `docs/implemented/test-grouping-notes-pa23-pa29.md`
- `docs/implemented/test-grouping-notes-pa30-pa35.md`
- `docs/implemented/test-grouping-notes-pa36-pa37.md`

Coordinator summary:

- `pa10` through `pa12`: split curated N3485-anchored syntax/semantic tests into
  `tests/spec/`, moved broad parser/call-semantic regressions to
  `tests/general/`, removed empty `tests/derived/`, and updated READMEs.
- `pa13`: kept `tests/spec/` as the LowIR contract bucket anchored to
  `pa13/lowir.md`, kept debug-info role buckets, and updated the README.
- `pa14`: moved the current LowIR lowering suite to `tests/general/`; no tracked
  spec-anchored tests remain.
- `pa15` through `pa22`: retained only clause-anchored C++ language tests in
  `tests/spec/`, moved implementation/regression/interaction cases to
  `tests/general/`, updated moved path references in refs and witness refs, and
  updated READMEs.
- `pa23`: kept the intentional `tests/strict` and `tests/structural` role split.
- `pa24` through `pa33`: moved broad milestone/ABI/runtime suites from
  `tests/spec/` to `tests/general/` and updated README descriptions.
- `pa34`: kept `tests/preproc` and `tests/compile` as meaningful public role
  buckets, removed the untested internal `tests/frontier` discovery surface
  from the repository, and preserved shared numeric feature-family anchors.
- `pa35`: kept `tests/link` as the link/runtime oracle bucket and preserved
  shared numeric feature-family bands.
- `pa36` and `pa37`: kept existing optimizer/backend optimization-level,
  driver, and debug-info role buckets; updated READMEs and bucket descriptions.

Follow-up numbering and citation audit:

- Restored pa1-pa9-style shared numeric feature-family anchors where the first
  pass had incorrectly treated prefixes as mostly unique sequence numbers.
- Updated the worker prompt to state that broad anchors such as `100`, `200`,
  and `300` are expected group prefixes, with in-between values reserved for
  deliberate subfamilies rather than duplicate avoidance.
- Checked every remaining `tests/spec/*.t` N3485 citation against
  `doc/n3485.txt` headings and corrected stale or nonexistent chapter labels,
  including dependent-type, function-call deduction, overloaded-operator,
  static-assert declaration, class-partial-ordering, and translation-unit
  grammar anchors.
- Moved variable-template tests out of `tests/spec/` because N3485 has no
  `temp.var` chapter to cite.

Validation notes for this pass:

- Worker slice validation passed for `pa10` through `pa33`, focused affected
  anchors in `pa34` and `pa35`, and full `pa36`/`pa37` test plus debug-info
  targets. Detailed commands and counts are in the slice notes.
- Coordinator strict validation with `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1` passed
  for `pa18 pa19 pa21 pa22`.
- Coordinator root validation passed with lowir direct text compare enabled:
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed `2787 / 2787`.
- Missing-test follow-up added nine focused PA10-PA12 N3485 spec owners. The
  same root validation then passed `2796 / 2796`.
- Strict duplicate cleanup removed 30 canonical-token duplicate general tests
  from `pa18`, `pa19`, `pa21`, and `pa22`; strict validation with LowIR direct
  text compare passed, and the same root validation then passed `2766 / 2766`.

## Pass C Transition Findings

- `pa13` / `pa14+`
  - fixed the pre-split handoff so native backend ownership now points at `PA23`, not
    the old `PA22`
  - fixed the later source-level complete-program handoff so it now points at `PA30`,
    not the old pre-split numbering
- `pa19` / `pa20` / `pa21` / `pa22` / `pa23`
  - fixed the pre-split handoff that incorrectly made `PA20` the native-backend retarget
  - `PA19` now hands off to `PA20 constexpr`, then `PA21`/`PA22` template completion,
    then `PA23 nativebackend`
- `pa25` / `pa26`
  - no ownership move was needed; the private-EH to language-closure handoff is coherent
- `pa30` / `pa32` / `pa33`
  - host object interop, then host ABI/runtime ownership, remains a clean monotonic sequence
  - clarified that broader hosted-header compatibility is later `PA34` / `PA35` work, not
    part of the `PA32` handoff
- `pa33` / `pa34` / `pa35`
  - host ABI/runtime, then hosted compile compatibility, then hosted emitted-code
    link/runtime remains the intended progression

No confident cross-PA test rehomes were identified during Pass C. The remaining cleanup work
is Pass D deferred buildout, which should wait until the LowIR/backend boundary settles.

The entries below are retained as the recorded audit findings that now feed
Pass D deferred test buildout.

## Pass A Audit Entries

### `pa10`

- `owner/boundary summary`:
  - first `cppgm++` assignment
  - owns `--emit-ast`
  - syntax tree boundary over the PA10 grammar, not semantic classification
- `primary oracle`:
  - checked-in AST text refs
- `secondary smokes`:
  - none
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-180`: minimal and early syntax forms
  - `100-231`: later syntax growth and corner syntax owners
  - `300`: focused spec-anchored ambiguity and type-id syntax follow-ups
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README says `tests/derived/` is part of the shipped suite, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - numbering is locally readable, but the negative/rejection surface is not isolated into its own visible band
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - added `300-declaration-statement-ambiguity`, `300-template-id-less-expression`, and `300-type-id-expression-contexts`
  - remaining parse/rejection owners are deferred until a smaller malformed-input audit
- `defer reason`:
  - remaining syntax-edge candidates should be kept small enough to avoid duplicating later semantic ownership
- `next actions`:
  - Pass B: align README with the final single-`tests/` layout
  - later Pass A: verify whether any post-`200` cases belong in `pa11` instead

### `pa11`

- `owner/boundary summary`:
  - first semantic layer over the PA10 AST
  - owns scopes, declarations, and canonical types
  - not yet full call semantics
- `primary oracle`:
  - checked-in type/scope dump refs
- `secondary smokes`:
  - none
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-185`: basic declarations, namespace/class scope, and lookup surface
  - `100-210`: first negative and boundary cases
  - `200-297`: later enum, template-parameter, constexpr, and using/lookup growth
  - `300`: focused spec-anchored lookup/declaration negative follow-ups
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README again claims a `tests/derived/` suite, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - the former duplicate `296-*` numbering defect has been repaired as `296` / `298`
  - later additions are still locally coherent, but the family bands are no longer obvious from numbering alone
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - added `300-namespace-alias-non-namespace-bad`, `300-opaque-enum-redecl-underlying-bad`, and `300-using-declaration-template-id-bad`
  - realistic inline-namespace / alias / value-lookup reducers remain general unless reduced to single-clause tests
- `defer reason`:
  - remaining lookup negatives need clause-by-clause reduction before being frozen as public spec tests
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout and clarify the value-name `using` surface

### `pa12`

- `owner/boundary summary`:
  - first call-semantics stage
  - procedural calls, builtin operators, control flow, and bounded conversion/overload rules
  - class-aware call resolution still deferred
- `primary oracle`:
  - checked-in call-semantics dump refs
- `secondary smokes`:
  - none
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-188`: core call/builtin/conversion surface
  - `100-195`: early negative cases
  - `200-248`: control-flow and expression-structure extensions
  - `200-339`: later semantic growth and repaired corner cases
  - `300`: focused spec-anchored call/conversion negative follow-ups
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - the former duplicate `220-*` numbering defect has been repaired as `220` / `223`
  - the `294+` region is understandable as later intake, but the banding is now patchy rather than obviously intentional
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - added `300-condition-declaration-scope`, `300-compound-assignment-lvalue-bad`, and `300-nullptr-pointer-conversion`
  - additional overload-ranking and pointer-comparison reductions remain deferred because broader current regressions already cover them
- `defer reason`:
  - many remaining plausible tests bleed into later object-model or template ownership, so they should be classified before being frozen
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout and tighten wording around the bounded non-class call surface

### `pa13`

- `owner/boundary summary`:
  - first LowIR backend scaffold
  - owns LowIR text parsing and CY86 translation
  - does not yet own native output, separate compilation, or host linking
- `primary oracle`:
  - checked-in CY86 text refs
- `secondary smokes`:
  - manual runtime validation through `cy86` is useful but not the committed primary oracle
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-188`: core runnable scaffold surface
  - `100-236`: negative, EH, float, atomic, and conversion expansion
- `pass_a findings`:
  - primary oracle still matches the scaffold assignment boundary
  - README claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - the former duplicate `190-*` numbering defect has been repaired as `190` / `191`
  - the current suite mixes the original scaffold core and later LowIR surface growth into one band; this is still acceptable, but the numbering should make the later growth easier to scan
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `200-299` for any future LowIR-syntax owners that survive later LowIR evolution decisions
- `defer reason`:
  - LowIR evolution is still active, so new syntax-sensitive tests should not be frozen yet
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout and clarify that direct CY86 text remains the primary oracle and runtime execution is secondary

### `pa14`

- `owner/boundary summary`:
  - first C++ to LowIR lowering assignment
  - owns the procedural lowering boundary over the PA12 semantic subset
  - not yet the object-model/helper-emission stage
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later `lowir2native` / `lowir2cy86` execution is validation aid, not the primary oracle
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-180`: early procedural lowering
  - `100-216`: later procedural/runtime-adjacent repairs and corner lowering
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - numbering is locally coherent and does not currently show duplicate IDs
  - the one explicit negative owner (`100-bad-switch`) is mixed into the positive flow rather than living in a visible later rejection band
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `300-349` for any additional deterministic procedural-lowering rejection owners that remain clearly in `pa14`
- `defer reason`:
  - LowIR evolution is still active, so new lowering-shape tests should wait until the intended IR boundary is stable
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout
  - later Pass A: decide whether the negative surface should be renumbered into a clearer later band

### `pa15`

- `owner/boundary summary`:
  - first basic object-model lowering assignment
  - owns class layout, non-virtual methods, constructors/destructors, member access, and single inheritance without polymorphism
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later `lowir2native` / `lowir2cy86` execution is validation aid, not the primary oracle
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-189`: early object-model and lifetime surface
  - `100-273`: later class/layout/operator/member-pointer growth
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - the former duplicate numbering defects have been repaired as:
    - `191` for inherited-base typedef lookup
    - `193` for base-field access
    - `274` for the overloaded unary-deref base-ref return owner
    - `275` for member-pointer base-to-derived data access
  - the suite is still locally readable as one long object-model band rather than separate milestone families
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `200-349` for any later object-model owners that remain truly PA15 rather than PA16+ value-semantics/polymorphism work
- `defer reason`:
  - many plausible missing tests border directly on PA16 value semantics, so ownership should stay audit-first
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout

### `pa16`

- `owner/boundary summary`:
  - non-polymorphic value-semantics extension over PA15
  - owns copy/value helpers, pass-by-value/return-by-value, delegating/out-of-class special members, and the bounded temporary-materialization surface
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later `lowir2native` / `lowir2cy86` execution is validation aid, not the primary oracle
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `200-280`: core copy/value-semantics surface
  - `200-352`: later value-semantics and repaired corner cases
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - the former duplicate numbering defects have been repaired by moving:
    - `rvalue-reference-call-pass-by-value` to `242`
    - `enum-class-nonmember-operator-bitand` to `286`
    - `temporary-functor-call` to `289`
  - starting the suite at `200` is fine here because the assignment reads as a follow-on tranche rather than a fresh binary/tool boundary
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `300-399` for any remaining clearly non-polymorphic value-semantics owners after the broader cleanup settles
- `defer reason`:
  - this surface is highly sensitive to later object-model and LowIR boundary decisions, so adding more tests now would likely churn
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout

### `pa17`

- `owner/boundary summary`:
  - first polymorphic lowering tranche
  - owns virtual dispatch, vtables/vpointers, virtual destructors, and bounded `override` / `final` checking
  - explicitly not yet RTTI, MI adjustment, or virtual inheritance
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later `lowir2native` / `lowir2cy86` execution is validation aid, not the primary oracle
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `300-405`: one focused polymorphism band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - numbering is sparse but coherent for a focused follow-on milestone
  - README contains at least one likely implementability typo to revisit in Pass B:
    - “PA13 `lowir2native`” should almost certainly refer to the later native validation path
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `400-449` for any remaining focused single-inheritance polymorphism owners that do not belong in PA18+ template or later RTTI/MI milestones
- `defer reason`:
  - later RTTI / MI / ABI milestones need to settle ownership before more virtual-corner tests are frozen here
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout
  - Pass B: fix the likely `lowir2native` path typo and any similar implementability drift

### `pa18`

- `owner/boundary summary`:
  - first template-instantiation tranche over the completed non-template language stack
  - owns first-tier templates, deduction for supported function templates, and on-demand instantiation
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native/CY86 execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-214`: one coherent first-template band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README still claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - numbering is sparse but coherent, with no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `200-299` for any additional clearly first-tier template owners that remain outside the later specialization/SFINAE splits
- `defer reason`:
  - later template-split cleanup should settle exact ownership before more tests are frozen here
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout

### `pa19`

- `owner/boundary summary`:
  - metaprogramming extension over PA18
  - owns integral non-type template parameters/arguments, explicit specialization, and pragmatic compile-time value support before full constexpr
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native/CY86 execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-196`: one coherent metaprogramming band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README still claims `tests/derived/`, but the current tree has `0` derived tests
  - cleanup target should be a single `tests/` directory, not `tests/spec/` plus empty `tests/derived/`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `200-249` for any remaining clearly PA19 metaprogramming-only owners after the constexpr/template split is finalized
- `defer reason`:
  - some plausible gaps may actually belong in PA20/PA21 after the split cleanup
- `next actions`:
  - Pass B: switch README to the final single-`tests/` layout

### `pa20`

- `owner/boundary summary`:
  - full `constexpr` / constant-evaluation milestone over the implemented language surface
  - still LowIR output, not a new backend/tool boundary
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native/CY86 execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-443`: broad constexpr/constant-evaluation band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README already describes `tests/spec` rather than `tests/spec` plus `tests/derived`
  - cleanup target should still be a single `tests/` directory rather than `tests/spec`
  - numbering is broad but coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `400-499` for any remaining pure constexpr owners after the PA21/PA22 split settles
- `defer reason`:
  - constant-evaluation gaps often overlap template ownership, so they should be classified before new tests are added
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa21`

- `owner/boundary summary`:
  - first half of template completion
  - specialization/entity graph, selection, and ownership
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native/CY86 execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-437`: specialization/entity surface
- `pass_a findings`:
  - split boundary looks correct and readable in the README
  - README already treats `tests/spec` as the active subset rather than claiming `tests/derived`
  - cleanup target should still be a single `tests/` directory rather than `tests/spec`
  - the former duplicate `100-189` extern-template owners have been repaired as:
    - `200-204` for class/constructor/member-function/static-data declarations
    - `211` for the operator-function declaration
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `400-499` for any further specialization/entity owners that survive the split audit
- `defer reason`:
  - the PA21/PA22 split is fresh enough that new template tests should wait until the ownership pass fully settles
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa22`

- `owner/boundary summary`:
  - second half of template completion
  - full deduction/substitution/SFINAE completion over the implemented surface
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native/CY86 execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-471`: deduction/substitution/SFINAE surface
- `pass_a findings`:
  - split boundary looks correct and readable in the README
  - README already treats `tests/spec` as the active subset rather than claiming `tests/derived`
  - cleanup target should still be a single `tests/` directory rather than `tests/spec`
  - the former duplicate `346-*` / `347-*` numbering defects have been repaired as:
    - `300-347` for the friend-access owners
    - `300-353` for the builtin-initializer-list and current-specialization-alias owners
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `400-549` for any later deduction/substitution owners that remain clearly outside backend/toolchain work
- `defer reason`:
  - this assignment sits directly against the LowIR/backend boundary, so new tests should wait until the cleanup and LowIR evolution decisions settle
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa23`

- `owner/boundary summary`:
  - first real native backend milestone
  - owns LowIR to native executable plus MIR dump
- `primary oracle`:
  - split oracle by design:
    - `tests/strict`: raw MIR-oriented checks
    - `tests/structural`: canonical/structural MIR checks
- `secondary smokes`:
  - generated native program runtime
- `current harness mode`:
  - multi-family harness with separate strict/structural comparisons plus runtime
- `current numbering bands`:
  - `strict`: `100-640`
  - `structural`: `200-710`
- `pass_a findings`:
  - this is a legitimate multi-directory assignment; it should not collapse to one flat `tests/` directory
  - the current `strict` / `structural` split matches a real oracle distinction and should be kept
  - numbering is coherent and has no duplicate IDs in either family
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D only if future backend-quality work reopens this assignment
- `defer reason`:
  - PA23 already has an explicit quality/buildout history and should not grow casually during the general cleanup pass
- `next actions`:
  - Pass B: ensure the README and harness text keep the strict/structural distinction explicit

### `pa24`

- `owner/boundary summary`:
  - `cpplink` object/link pipeline over compiler-owned objects
  - first practical separate-compilation/link contract
- `primary oracle`:
  - runtime/link behavior over generated objects
- `secondary smokes`:
  - object inspection is useful but not the primary contract here
- `current harness mode`:
  - source-driven compile/link/run with checked-in expected outputs
- `current numbering bands`:
  - `100-195`: one coherent small link/runtime band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README already no longer depends on `tests/derived`
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `200-249` if later cleanup reveals small missing separate-compilation/link owners
- `defer reason`:
  - later host-toolchain assignments may still cause ownership moves
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa25`

- `owner/boundary summary`:
  - private EH/runtime ABI path through `cppeh`
  - compiler-owned EH runtime surface, not host EH interop
- `primary oracle`:
  - source-driven compile/link/run over the private EH path
- `secondary smokes`:
  - normalized private-EH object facts are a useful review aid but not the public first oracle
- `current harness mode`:
  - source-driven compile/link/run with checked-in expected outputs
- `current numbering bands`:
  - `100-140`: one tight EH owner band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - README already no longer depends on `tests/derived`
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D only if later private-EH cleanup reveals a genuine owner gap
- `defer reason`:
  - this assignment was recently reviewed explicitly and does not need speculative growth
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa26`

- `owner/boundary summary`:
  - first post-EH language/value-semantics extension over the LowIR path
  - currently a broad source-to-LowIR milestone rather than a new backend/tool boundary
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-232`: one broad language/lowering band
- `pass_a findings`:
  - boundary/oracle remain plausible, but this assignment likely needs a more careful later audit because the scope is broad
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - the former duplicate `216-*` numbering defect has been repaired by moving the conditional-constructor-conversion owner to `205`
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `200-299` pending later scope clarification
- `defer reason`:
  - this assignment sits in a large semantic/lowering stretch that should not grow until the broader audit clarifies ownership around neighboring milestones
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa27`

- `owner/boundary summary`:
  - later language/lowering extension over the same `cppgm++ --emit-lowir` boundary
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-210`: one compact band
- `pass_a findings`:
  - boundary/oracle remain plausible
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
- `defer reason`:
  - adjacent PA26/PA28 ownership should be reviewed before new tests are added
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa28`

- `owner/boundary summary`:
  - later language/lowering extension over the same `cppgm++ --emit-lowir` boundary
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-190`: one very small band
- `pass_a findings`:
  - boundary/oracle remain plausible
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
- `defer reason`:
  - the small suite size means ownership clarity matters more than adding volume right now
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa29`

- `owner/boundary summary`:
  - final source-to-LowIR stage before the practical driver/toolchain assignments
- `primary oracle`:
  - checked-in LowIR text refs
- `secondary smokes`:
  - later native execution remains secondary validation
- `current harness mode`:
  - file-based tool, `.ref` text compare plus exit status
- `current numbering bands`:
  - `100-151`: one compact late-lowering band
- `pass_a findings`:
  - boundary/oracle remain plausible
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
- `defer reason`:
  - this assignment is immediately adjacent to the practical driver/toolchain transition, so ownership should be settled before adding anything
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa30`

- `owner/boundary summary`:
  - first practical `cppgm++` compile/link driver boundary
  - owns compile mode plus default link mode through the compiler driver itself
- `primary oracle`:
  - compile/link/run behavior
- `secondary smokes`:
  - direct object inspection is secondary here
- `current harness mode`:
  - source-driven compile/link/run with checked-in runtime outputs
- `current numbering bands`:
  - `100-305`: one broad practical-driver band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
  - reserve `300-399` for any remaining driver-mode owners after PA32/PA33/PA34/PA35 handoffs are rechecked
- `defer reason`:
  - this assignment now sits at the base of a split host-driver sequence, so new tests should wait for the full cross-PA review
- `next actions`:
  - Pass B: normalize README wording to the final single-`tests/` layout

### `pa32`

- `owner/boundary summary`:
  - host object/toolchain interoperability assignment
  - ordinary host-linkable object-file contract, not host C++ ABI correctness
- `primary oracle`:
  - compile plus external host link/run behavior
- `secondary smokes`:
  - object inspection is useful but secondary
- `current harness mode`:
  - source-driven compile with external host final link
- `current numbering bands`:
  - `100-199`: one compact host-object band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
  - README heading style differs from many neighboring assignments (`#` vs `##`), which is minor but should be normalized in Pass B
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D
- `defer reason`:
  - host-object vs host-ABI vs hosted-compat ownership needs the later cross-PA pass before new tests are frozen
- `next actions`:
  - Pass B: normalize README heading style and single-`tests/` wording

### `pa33`

- `owner/boundary summary`:
  - host C++ ABI/runtime interoperability assignment
  - builds on PA32 host-linkable objects and owns the host ABI/runtime behavior after host link
- `primary oracle`:
  - compile plus external host link/run behavior
- `secondary smokes`:
  - normalized EH/RTTI/vtable object facts and disassembly reviews are important secondary signals
- `current harness mode`:
  - source-driven compile with external host final link
- `current numbering bands`:
  - `100-230`: one host-ABI band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - cleanup target should be a single `tests/` directory rather than `tests/spec`
  - numbering is coherent and has no duplicate IDs
  - README heading style differs from many neighboring assignments (`#` vs `##`), which is minor but should be normalized in Pass B
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D only if later host-ABI review reveals genuine uncovered owner cases
- `defer reason`:
  - the assignment recently had dedicated EH and RTTI/vtable review plans; growth should stay deliberate rather than speculative
- `next actions`:
  - Pass B: normalize README heading style and single-`tests/` wording

### `pa34`

- `owner/boundary summary`:
  - hosted source/header compatibility assignment
  - preprocessing and compile acceptance over hosted/vendor surfaces
- `primary oracle`:
  - legitimately split:
    - `tests/preproc`
    - `tests/compile`
- `secondary smokes`:
  - former `tests/frontier` discovery files were not part of the tested surface and were removed from the repository
- `current harness mode`:
  - multi-family hosted harness with distinct preprocess/compile flows
- `current numbering bands`:
  - `preproc`: shared `300` and `400` hosted-preprocessor families
  - `compile`: shared `500`, `600`, and `700` hosted-compile families
- `pass_a findings`:
  - this is a legitimate multi-directory assignment; `preproc` and `compile` reflect real different public test roles
  - the former internal frontier files were untested by the PA34 harness and should not remain in the public repository
  - compile owners now use shared hundred-family anchors (`500`, `600`, `700`) rather than uniqueness-driven local sequences
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D only if hosted compatibility cleanup reveals specific missing owner families
- `defer reason`:
  - hosted compatibility tests are expensive and ownership-sensitive, so they should not grow casually during the audit pass
- `next actions`:
  - Pass B: keep `preproc` / `compile` explicit

### `pa35`

- `owner/boundary summary`:
  - hosted header-emission and hosted link/runtime compatibility assignment
  - keeps the PA34 hosted environment but raises the contract from compile acceptance to host-linked correctness
- `primary oracle`:
  - hosted compile plus external host link/run behavior
- `secondary smokes`:
  - normalized object/symbol inspection is useful secondary signal
- `current harness mode`:
  - currently one public `tests/link` family
- `current numbering bands`:
  - `600-723`: one hosted link band
- `pass_a findings`:
  - boundary and primary oracle look correct
  - because there is only one public family, the final cleanup target should probably be a single `tests/` directory rather than `tests/link`
  - numbering is coherent and has no duplicate IDs
- `misplaced tests`:
  - none identified yet
- `planned new tests`:
  - deferred to Pass D only if later hosted-link cleanup reveals real uncovered owner cases
- `defer reason`:
  - hosted link/runtime growth is expensive and tightly coupled to neighboring hosted milestones
- `next actions`:
  - Pass B: decide whether `tests/link` should collapse into final `tests/` since there is no second public family
