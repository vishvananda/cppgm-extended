# Centaur Test Intake Plan

## Status and Scope

Plan created 2026-08-02. Intake execution has reviewed 22 of 272 families:
eleven were intaked, nine are covered by existing tests, and two expose
current compiler bugs. The frozen source checkout points to:

- source: `vishvananda/cppgm-run-centaur`
- source commit: `debefcdce18b4be64a8011a85239629a47665b5f`
- planning target: `cppgm-test-intake` at
  `622fc55c4fa68bfef09e337071c0e5cdd638fd75`

Treat Centaur as an old-run evidence repository. Its HEAD timestamp is recent,
but the run began from an older assignment export and a later bulk snapshot
recorded 129 families together. Use path history and the motivating compiler
change to establish each family's age and provenance; the repository's HEAD
date provides no provenance.

Do not merge Centaur's `cppgm.tests` tree, copy an assignment directory, or
cherry-pick its production compiler changes. Intake only a reduced test when
it proves a distinct current assignment contract that is not already covered.
A Centaur test that merely passes the current compiler, uses a different STL
container, or was useful as run progress evidence is not enough.

The normal `dev/` binaries in this repository are the student reference
binaries on assignment export. Generate accepted references with the same
normal Homebrew-Clang-built binary used to validate the current compiler. Do
not build a second reference object root, clean unchanged objects, or copy a
Centaur reference as the canonical oracle.

The completed cohorts have closed four assignment-baseline duplicates, the
PA4-through-PA9 catalog, and the six PA11 semantic-cleanup families. The
retained early coverage consists of one PA4 macro paint regression, four PA8
initialization/linkage additions plus one existing PA8 conversion-family
extension, and one combined PA9 narrow/wide literal alignment regression.
PA11 adds one combined anonymous-type identity test and three focused negative
semantic tests. The zero-bound array case exposed a current compiler bug,
fixed separately in `c990a0ac2`; its audit also required teaching the
placement checker that PA10-PA12 type-semantic output does not exercise later
LowIR/runtime owners. Clang and GCC show that Centaur's PA7 inline-reopen
fixture should reject, while the PA8 README requires four-byte mock-function
alignment after a char. The current compiler accepts or emits the old behavior
for both, so their tracker rows remain `blocked-current-bug` and no incorrect
reference was added.

## Frozen History and Initial Inventory

The last assignment export before Centaur's implementation work is:

- assignment baseline:
  `de780cf591289d22b9d4d3b4e9e9c2a32f8fbfe8`
- exported `cppgm-extended` revision named by that commit:
  `faed9c7fb47a`
- first run-authored commit after the baseline:
  `f3efbbb4ecce33e751b5b5d4b68de6f2523ba0ac`

The later export commit
`ffecb81f1b7699a99d90efe19e581aad2c818dd8` changed no paths under
`pa*/tests` or `cppgm.tests/course`, so it does not change the test baseline.
If a newer Centaur snapshot is ever selected, repeat this audit before adding
rows to the tracker.

The initial source-led inventory selects live family-shaped paths added after
the baseline. The catalog groups `.t`, `.t.N`, local helper sources, inputs,
flags, and reference sidecars by family. It records standalone PA39 `*.cpp`
reducers as evidence artifacts rather than runnable assignment tests.

| Source location | Families |
|---|---:|
| `cppgm.tests/course` runnable families | 256 |
| `pa35/tests/compile` runnable families | 5 |
| `cppgm.tests/course/pa39/reducers` artifacts | 11 |
| **Total catalog rows** | **272** |

Six other assignment-local additions are sidecar-only repairs: five
`ref.program.exit_status` files in PA32 and one PA34 preprocessing reference.
They are not new test families. Centaur deleted three baseline assignment
families during the run. Do not restore those fixtures without separate proof
that their contracts remain valid.

### Inventory by Source Assignment

The source assignment records history; the proposed owner remains open.

| Assignment | Families | Assignment | Families |
|---|---:|---|---:|
| PA4 | 1 | PA7 | 1 |
| PA8 | 8 | PA9 | 2 |
| PA11 | 6 | PA12 | 21 |
| PA14 | 36 | PA15 | 2 |
| PA16 | 1 | PA17 | 1 |
| PA18 | 4 | PA20 | 3 |
| PA21 | 2 | PA22 | 1 |
| PA23 | 10 | PA24 | 2 |
| PA25 | 1 | PA29 | 6 |
| PA31 | 11 | PA32 | 6 |
| PA33 | 6 | PA34 | 7 |
| PA35 | 20 | PA36 | 95 |
| PA37 | 8 | PA39 reducers | 11 |
| **Total** | **272** | | |

Commit `fba28d59ebd40a29e7e64978e63a61fcd862efc1`, `Complete PA39
inception reproducibility`, introduced 129 of the 272 families in one bulk
change: PA12 16, PA14 18, PA15 1, PA16 1, PA18 2, PA20 1, PA21 1, PA22 1,
PA23 4, PA24 1, PA25 1, PA29 4, PA31 9, PA32 3, PA33 1, PA34 5, PA36 54,
and PA37 6. Those rows start as `needs-evidence`. The bulk commit alone does
not prove that each source isolated a bug or belongs at its source PA.

The remaining families span 80 introducing commits. Commit-level provenance
still does not waive reduction, duplicate, or placement review.

### Preliminary Current-Tree Overlap

An initial mechanical scan compared normalized family paths, source-blob
multisets, and descriptive stems with the planning target. It found:

- byte-identical source under a different name:
  Centaur PA18 `201-local-specialization-lifecycle-order` versus current
  `pa18/tests/general/100-nested-class-template-local-class-argument`
- byte-identical source at an earlier owner:
  Centaur PA37 `o1/200-invalid-undefined-block-target` versus current
  `pa13/tests/spec/200-bad-undefined-block-target`
- same descriptive family name but changed source:
  PA15 `200-reference-member-conditional-lvalue` and PA16
  `100-copy-constructor-default-parameter`

These four are duplicate-review leads rather than final dispositions. Exact coverage
requires comparing the oracle as well as the input. The other 268 rows have no
mechanical path, blob, or name match, but that does not establish unique
semantic coverage. Review the remaining rows against current semantic
coverage before doing reduction work.

## Intake Standard

The default answer is to leave a Centaur family out. Accept it only after all
of the following are true:

1. History identifies a stable compiler contract and, when possible, the
   failing parent and fixing commit.
2. The final reducer distinguishes the original bad behavior from the correct
   behavior; any failure is not enough for a negative test.
3. No current test already exercises the same compiler path and oracle.
4. The reducer is standard C++11 or has a justified extension,
   hosted, ABI, LowIR, or optimizer boundary.
5. The current README owns the expectation, with a minimal PA10-or-later
   clarification if needed.
6. The test is placed in the earliest current assignment that owns every
   essential feature left in the reducer.
7. The correct canonical reference can be generated independently of
   Centaur's old output.
8. Focused, placement, assignment, and applicable broader validation passes.

If any one of these remains unproved, retain an evidence disposition.

## Tracker

Use `docs/centaur-test-intake-tracker.tsv`. It contains one initial row for
each of the 272 families. The catalog fields are mechanical and must be
replaced with reviewed evidence before a row can become `ready`, `intaked`, or
a final covered/rejected disposition.

Required evidence fields are the same as the completed Fable/Mini intake:

- frozen source and target revisions, source tree, assignment baseline, and
  path-introducing commit
- every family source, helper, input, flag, and reference sidecar
- provenance class and original failure signature
- evidence location and historical failing/fixing revisions
- one-sentence primary compiler behavior and incidental features
- reduction status and final reduced path
- exact and semantic duplicate results with concrete closest tests
- proposed current assignment, suite, ownership reason, and portability class
- reference provenance, focused validation, final disposition, destination,
  and intake commit

Use these working provenance classes:

- `run-authored`: introduced in a run commit tied to one compiler change
- `bulk-run-snapshot`: one of the 129 families first recorded by `fba28d59`
- `reducer-artifact`: a PA39 standalone reducer that is evidence only
- `upstream-assignment-import`: assignment-export material, if later history
  reconstruction discovers any
- `reference-only`: sidecars without a new runnable source family

Use the existing final dispositions from the Fable/Mini tracker, plus
`evidence-artifact` for a raw reducer that is not itself a harness test:
`needs-evidence`, `needs-reduction`, `covered-exact`, `covered-near`,
`not-a-regression`, `ready`, `blocked-reference`, `blocked-current-bug`, and
`intaked`. A covered or rejected row must name the concrete evidence; it is not
a shortcut for an expensive review.

## Review Procedure

### 1. Reconstruct Provenance

For every family:

1. Read all source and sidecar files at the frozen source commit.
2. Read the introducing commit, its parent diff, the assignment's `plan.md`
   and `audit.md`, and any root plan that names the behavior.
3. Classify the original symptom: parse rejection, wrong semantic output, bad
   LowIR, object or symbol mismatch, link failure, runtime failure, optimizer
   miscompile, crash, timeout, or progress-only fixture.
4. Run the smallest historical fail/pass comparison supported by the source
   history. For a bulk-snapshot family, find evidence earlier than `fba28d59`
   or leave it `needs-evidence`.
5. Reject progress probes, stress inputs, implementation-shape snapshots, and
   old workarounds that do not encode a stable current contract.

Do not infer provenance from a passing Centaur reference. First establish the
intended behavior from the assignment contract, standard compilers, or the
relevant backend specification.

### 2. Apply the Duplicate Gate Twice

Search before reduction and immediately before landing:

1. Compare exact family paths and all source/oracle sidecars.
2. Search `pa*/tests`, `cppgm.tests/course`, the completed Fable/Mini tracker,
   and recent target history by behavior, operators, type/value-category
   shapes, symbols, LowIR operations, EH structure, and expected result.
3. Normalize comments, local names, helper types, declaration order, and
   irrelevant output before comparing sources.
4. Compare the compiler path and discriminating oracle. A longer program or a
   different hosted container is not new coverage.
5. Keep a candidate only for a material boundary such as different lookup,
   substitution, value category, lifetime edge, ABI class, relocation, EH
   cleanup, register-pressure condition, or optimizer safety condition.

The 129-family bulk snapshot and the PA35/PA36 groups receive no presumption of
uniqueness. Prefer strengthening one existing focused family when that adds a
small distinct assertion without creating another expensive process.

### 3. Reduce and Establish Portability

- Keep one primary assertion.
- Remove unrelated declarations, templates, units, runtime work, and output.
- Replace hosted containers, traits, allocators, iterators, strings, and
  implementation-private types with small test-owned C++11 types unless the
  hosted interaction is the behavior under test.
- Preserve multiple units only for linkage, ODR, initialization order, ABI,
  object, or EH behavior that cannot be shown in one unit.
- Validate positive language reducers with Homebrew Clang and GCC using
  `-std=c++11 -pedantic-errors`; use GNU mode only for an intentional extension.
- If the result established by Clang, GCC, or the relevant specification
  disagrees with the current compiler,
  record `blocked-current-bug`. Fix production code separately; do not weaken
  the reducer or encode the bug into a reference.

Raw PA39 reducers never land under PA39 merely because that is where Centaur
stored them. Convert a useful reducer into the earliest owning assignment's
real harness format. Otherwise mark it `evidence-artifact` or
`not-a-regression`.

### 4. Place Against the Current Assignment Arc

The source assignment is a hint only. Read the current `paN/README.md` and
adjacent tests after reduction.

- PA1 through PA9 accepted tests stay in `cppgm.tests/course/paN`; do not run
  the semantic/LowIR placement audit for them and do not edit their READMEs
  during ordinary intake.
- PA10 through PA27 use the earliest current `spec` or `general` owner.
- PA28 through PA33 must assert their actual backend, separate-compilation,
  ABI, object, or EH layer rather than incidental source syntax.
- PA34 owns intentional preprocessing/hosted extension behavior.
- PA35 accepts only distinct compile-time hosted-header behavior.
- PA36 accepts only distinct hosted link or runtime behavior. Apply the
  strongest reduction and earlier-placement review to its 95 source families
  because PA35/PA36 are already slow.
- PA37 and PA38 require an optimizer-specific unsafe transform or output
  contract; a language or LowIR validity bug belongs earlier.

For PA10 and later, run `scripts/audit_pa_feature_placement.py` over every
touched assignment, but treat it as a conservative review tool rather than the
owner oracle. Inspect `.t.N`, headers, and helpers by hand. If an old late-PA
test preserved generated EH behavior, keep a reduced EH
assertion or its later owner. When the checked fixture has no LowIR and source
plus history remains ambiguous, use the audit's targeted `--probe-lowir-test`
fallback rather than generating LowIR for an entire late suite.

Do not retain placement false positives. Improve the detector when a rule is
too broad, or rename an incidental identifier that triggers it, then require a
clean audit. Do not paper over a finding with a waiver.

For every accepted PA10-or-later test, review the README for the final owner.
If the behavior is not already a student expectation, make the smallest clear
contract edit needed. Do not prescribe an implementation technique or promise
unstable diagnostic wording.

### 5. Construct Canonical Fixtures and References

Build a new local fixture from the reducer instead of copying a Centaur family
wholesale. Preserve every sidecar required by the destination harness,
including failure stdout or stderr that assignment export compares.

Use `/usr/local/opt/llvm/bin/clang++` on this host for both `CXX` and
`CPPGM_HOST_CXX`. Build the relevant normal `dev/` binary once when source or
compiler configuration changed; otherwise reuse it. Do not run `clean`, do not
force unchanged objects to rebuild, and do not make a reference-only object
root. Record the target commit, build command, compiler version, and binary
hash for each batch.

For ordinary source-to-LowIR cases, this tree's normal `dev/cppgm++` is both
the current compiler and canonical reference generator. Use the actual
frontend/backend binary when an assignment owns a narrower tool.

Special oracle rules remain mandatory:

- Witness refs come from the patched-Clang witness path, never from
  `cppgm++`.
- PA30 spellings must match symbols emitted by Homebrew Clang. Use `c++filt`
  to verify the entity and `cppgm++ --emit-abi-facts` plus `abimangle` to prove
  the fact grammar reproduces that exact symbol. Minimize emitted facts: omit
  unnecessary one-case labels and replace `__abi_*` binders with short useful
  names.
- Treat LowIR text drift as a semantic decision, not automatic reference
  churn.
- Preserve MIR with backend behavior tests when it is part of the suite's
  useful reference record. Remove it only after proving the destination
  convention and oracle do not use it; do not strip MIR merely to make a
  copied family smaller.
- Keep exact failure streams when export checks their text.

If the current correctly configured compiler and Clang/GCC disagree on
standard behavior, treat that as a compiler bug. If they agree and Centaur's
old reference differs, Centaur's reference is wrong.

### 6. Validate and Land Small Batches

For each accepted family:

1. Prove independent language/host/ABI validity.
2. Generate the canonical reference twice and require byte stability.
3. Run the focused fixture with `CPPGM_SKIP_DEV_REBUILD=1` when the dev binary
   is already current.
4. Re-read the final README and run the full current assignment harness.
5. For PA10 and later, run the complete affected-assignment placement audit;
   use a targeted LowIR probe only for an ambiguous late test.
6. Run `ACTIVE_TEST_REPORT_PAS='paNN' make test-report` from the root. Use the
   root report without a clean so unchanged compiler objects stay reused.
7. Enable direct LowIR comparison for LowIR-sensitive cases and run applicable
   through-PA, hosted, PA37, PA38, debug-info, or strict gates.
8. Finish each batch with `git diff --check`, a tracker consistency check, and
   inspection for `.my`, `testout`, logs, binaries, or generated objects.

Land by reduced behavior and current owner, not by Centaur directory. Keep
production fixes separate from test-intake commits. A useful processing order
is portable PA10-PA27 reducers, PA28-PA33 backend/ABI/EH cases, PA34-PA36
cases that pass the hosted gates, optimizer cases, and the PA4-PA9
course candidates.

## Completion Criteria

The Centaur intake is complete when all 272 tracker rows have evidence-backed
dispositions and no `needs-evidence`, `needs-reduction`, or `ready` row
remains. Every accepted test must be distinct, reduced, portable or explicitly
bounded, placed at its current earliest owner, documented in the README when
needed, generated from the correct canonical reference source, and passing all
focused and applicable broad checks.

Completion does not require importing any fixed percentage of the catalog. A
small accepted set is the expected outcome for this older run.
