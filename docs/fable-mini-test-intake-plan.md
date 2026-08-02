# Fable and Mini Test Intake Plan

## Scope

Use the Fable and Mini run repositories as evidence sources for new regression
tests. Do not merge their `cppgm.tests` trees into this repository. Accepted
tests belong in the normal `paN/tests` assignment suites.

The first intake pass covers PA10 through PA38. PA1 through PA9 are cataloged
below and remain untouched until a separate second pass.

Source snapshots used for the initial inventory:

- Fable: `vishvananda/cppgm-run-fable` at
  `77b0e742ab1bdca2ea6c7b83d6890d60e7fc65ed`
- Mini: `vishvananda/cppgm-run-mini` at
  `e4e82cf8f8b19851f356aa6464a7b04186c5a176`
- Intake base: `cppgm-extended` at
  `b40c7e61724380e8ad1be3a67501323c42ecff3f`

Record new source and target commits if the intake starts from newer snapshots.
Never edit the run repositories. Use their tests, plans, audit notes, commits,
and Ralph records to reconstruct the failure that motivated each test.

Test intake changes should contain tests, canonical references, and tracker or
assignment documentation only. A test that exposes a current compiler bug is a
separate implementation task. Do not weaken the test or change production code
as part of an intake batch.

This repository is the source of the student reference binaries when it is
exported. Generate intake references with a clean host-compiler build of the
relevant `dev/` binary from the frozen intake target commit. Use that same
binary for reference generation and current-tree validation. References from
Fable and Mini remain evidence for reconstructing the intended behavior; they
are not the canonical source for a newly intaked fixture.

## Intake Rules

1. Treat an upstream test as evidence, not as a ready-to-copy fixture.
2. Track one logical test family per row. A family includes its `.t`, `.t.N`,
   headers, input files, arguments, environment files, and reference sidecars.
3. Reduce the test to one compiler claim before deciding where it belongs.
4. Put the result in the earliest assignment that owns the primary claim and
   every feature that remains in the reducer.
5. Add the result under `paN/tests`, never under `cppgm.tests/course`.
6. Reject exact and near duplicates unless the new test reaches a distinct
   semantic, lowering, ABI, runtime, or optimizer path.
7. Prefer standard C++11 and test-owned helper types. Keep hosted or
   implementation-specific dependencies only when they are the behavior under
   test.
8. Regenerate references through the repository's reference workflow, using a
   clean host build of this tree's relevant reference binary. Do not use a
   stale in-tree binary or copy a run reference as the new canonical oracle.
9. Keep PA1 through PA9 at `catalog-pa1-pa9` during the first pass.

## Course-Tree Inventory

The inventory first selects paths added after each run's assignment baseline.
No later assignment export added a path under either run's
`cppgm.tests/course` tree. It then compares those run-authored paths with the
clean intake base. The counts below are candidate units, not accepted tests.
Numbered `.t.N` files are coalesced into one family. A standalone
`*.reducer.cpp` or `*.reducer.lowir` file counts as a separate evidence unit.

No PA10-or-later candidate has the same relative path or byte-identical runnable
source as a test in the clean intake base. This removes only exact duplicates.
Every candidate still needs the semantic near-duplicate review described below.

| Assignment | Fable | Mini |
|---|---:|---:|
| PA10 | 1 | 17 |
| PA11 | 2 | 5 |
| PA12 | 2 | 28 |
| PA13 | 0 | 6 |
| PA14 | 0 | 7 |
| PA15 | 0 | 6 |
| PA16 | 0 | 18 |
| PA18 | 3 | 35 |
| PA19 | 1 | 10 |
| PA20 | 2 | 13 |
| PA21 | 0 | 14 |
| PA22 | 1 | 11 |
| PA23 | 0 | 1 |
| PA25 | 0 | 4 |
| PA27 | 0 | 2 |
| PA28 | 6 | 29 |
| PA29 | 0 | 1 |
| PA30 | 0 | 5 |
| PA31 | 0 | 17 |
| PA33 | 0 | 1 |
| PA34 | 0 | 1 |
| PA35 | 5 | 1 |
| PA36 | 22 | 2 |
| PA37 | 0 | 4 |
| PA38 | 0 | 3 |
| **Total** | **45** | **241** |

The Mini total includes 27 standalone reducer artifacts. Those files are not
harness tests and must not be imported as such. PA17, PA24, PA26, PA32, and
PA39 have no new path candidates in these snapshots.

## Assignment-Local History Audit

Both run repositories contain repeated assignment-export commits before the
compiler run begins. Use the final export before the first run-authored commit
as the local-test baseline:

- Fable baseline: `e8b5419e5002ad15a068a914d151a64eda0be31f`
- Mini baseline: `a069a0c8df57c866d1d5fbd32341f3d420591fb9`

The Fable first-parent history contains a later bulk assignment import. Commit
`0ec93c8ed109b679ba8765c7731f6698b8ef0943`, `Merge current assignment
export`, merged second parent `596a240a2207783c5626d3b5f597cd6a05aeb415`,
`Export assignments from cppgm-extended c1247d6c70a6`. The merge added 336
tracked files under `pa*/tests`, including 128 source families:

| Assignment | Imported files | Source families |
|---|---:|---:|
| PA15 | 4 | 2 |
| PA18 | 3 | 1 |
| PA19 | 18 | 7 |
| PA21 | 79 | 32 |
| PA22 | 79 | 30 |
| PA23 | 68 | 25 |
| PA25 | 7 | 5 |
| PA26 | 2 | 1 |
| PA28 | 4 | 4 |
| PA29 | 4 | 1 |
| PA32 | 10 | 2 |
| PA34 | 10 | 3 |
| PA35 | 8 | 8 |
| PA36 | 4 | 1 |
| PA37 | 13 | 6 |
| PA38 | 23 | 0 |
| **Total** | **336** | **128** |

Exclude every path first introduced by this merge from Fable intake. These are
upstream assignment additions, even if a later Fable commit changed a reference
sidecar. Reconstruct the exclusion set with:

```sh
git diff --name-only --diff-filter=A 0ec93c8^1 0ec93c8 -- 'pa*/tests/**'
```

Fable added 17 test source families under `pa*/tests` outside that merge: 12 in
PA2 and 5 in PA3. They belong in the deferred PA1-through-PA9 catalog. Fable
also added five missing `ref.program.exit_status` files to existing PA32 tests
in `344c0b4f95198f4dbceb06994a40a71e3140d2a6`; those files do not represent new
tests.

Mini added no test source family under `pa*/tests` after its assignment
baseline. Its only additions there are six `ref.stdout` files for existing
PA25 tests in `beca5e2793628813d600b9d96e7bb0922d0ea430`. Treat them as
reference repairs, not intake candidates.

After provenance filtering, neither run contributes a PA10-through-PA38 test
source from `pa*/tests`. The PA10-through-PA38 candidate inventory comes from
run-authored `cppgm.tests/course` additions only.

## Deferred PA1 Through PA9 Catalog

These names record the logical source families that are absent by path from the
clean intake base. Do not reduce, relocate, regenerate references for, or import
them during the first pass.

### Fable

- Assignment-local PA2: `460-hex-float`, `470-ud-number-shapes`,
  `480-string-encoding-edges`, `490-string-encode-error`,
  `500-invalid-char-flush`, `510-text-suffix-ascii`,
  `520-string-check-order`, `530-float-range`, `540-ud-number-hunt`,
  `550-ud-number-exponent`, `560-ppnumber-sign-shapes`,
  `570-binary-literals`
- Assignment-local PA3: `400-end-of-input`, `410-recovery`,
  `420-line-poison`, `430-error-carry`, `440-defined`
- PA6: `deep-template-arg-nest-bad`, `template-name-lt-commits-bad`
- PA7: `array-completion`, `bound-forms`, `deep-paren-nest`,
  `directive-anchor-nested`, `directive-anchor-sibling`, `directive-chain`,
  `directive-transitive-extension`, `fn-void-typedef`, `inline-alias-deep`,
  `inline-qualified-lookup`, `nsname-lookup`, `param-paren-shapes`,
  `ref-collapse-chain`, `using-declaration-reuse`
- PA8: `constexpr-crosstu`, `constexpr-qualified`, `cv-through-typedef`,
  `deep-namespaces`, `threadlocal-agree`, `threadlocal-mismatch`,
  `typedef-fn-decl`, `typedef-fn-def`

### Mini

- PA4: `600-rescan-boundaries`
- PA5: `500-predefined-argument-expansion`
- PA6: `decl-specifiers-without-type`, `operator-template-angle-boundary`
- PA7: `parenthesized-array-abstract`
- PA8: `constant-conversion-linkage`, `invalid-cv-reference`,
  `invalid-rvalue-reference`, `invalid-string-pointer`,
  `invalid-type-specifiers`, `mismatched-thread-local`,
  `missing-constexpr-initializer`
- PA9: `900-empty-program`, `901-long-double-label-alignment`,
  `902-negated-wchar-extension`, `903-unparenthesized-negative-immediate`,
  `904-unparenthesized-label-offset`, `905-negative-memory-literal`

The second pass must start from this catalog and repeat the evidence,
duplication, reduction, ownership, and reference checks. It must also account
for the distinct PA1 through PA9 harness formats before moving any file.

## Tracker

Create `docs/fable-mini-test-intake-tracker.tsv` when intake work begins. Use
one row per logical family, including raw reducer artifacts. Store paths
relative to the source repository so every decision remains reproducible.

Required fields:

- source run, source commit, source assignment, and original family path
- source tree, assignment baseline, and path-introducing commit
- provenance class: `run-authored`, `upstream-assignment-import`, or
  `reference-only`
- all source and reference sidecars in the family
- original failure signature and evidence location
- fixing commit or first known passing commit, when available
- primary compiler behavior and incidental language features
- reduction status and reduced source path
- duplicate search results and closest existing tests
- proposed assignment, suite, and ownership reason
- portability class: `standard-c++11`, `gnu-extension`, `lowir-contract`,
  `hosted-portable`, or `hosted-implementation-specific`; use
  `lowir-contract` only for a raw LowIR fixture whose portability boundary is
  the checked-in LowIR specification rather than a host C++ implementation
- reference provenance and focused validation results
- final disposition, destination, and intake commit

Use these dispositions:

- `catalog-pa1-pa9`: recorded for the second pass; no first-pass action
- `excluded-upstream-import`: introduced by an assignment export or its merge
- `reference-only`: adds or repairs sidecars for an existing test family
- `needs-evidence`: the motivating failure or contract is not yet proved
- `needs-reduction`: unique behavior found, but the source is still too broad
- `covered-exact`: an existing test has the same source and oracle
- `covered-near`: an existing focused test already exercises the same path
- `not-a-regression`: stress input, progress fixture, or artifact without a
  stable compiler contract
- `ready`: reduced, placed, portable or justified, and referenceable
- `blocked-reference`: the required canonical reference cannot be produced
- `blocked-current-bug`: the reducer exposes an unfixed target bug
- `intaked`: accepted in `paN/tests` with all required checks passing

`covered-exact`, `covered-near`, and `not-a-regression` rows must cite concrete
evidence. Do not use them as shortcuts for difficult cases.

## Intake Procedure

### 1. Freeze and Group the Source Material

At the start of each batch:

1. Record the Fable, Mini, and target commits in the tracker.
2. Build the relevant target binary from the recorded target commit with the
   host compiler. Use the normal `dev/` build and reuse the resulting binary
   for reference generation and test execution. Record the compiler identity,
   binary hash, and build command with the batch validation evidence.
3. Generate tracked-file manifests for each source `cppgm.tests/course/paN`
   tree and each `paN/tests` tree.
4. Compare both trees with the run's assignment baseline. Attribute each new
   path to the commit that first introduced it.
5. Exclude paths introduced by `Export assignments from cppgm-extended`
   commits or merges of those exports. Exclude reference-only repairs from the
   test-family queue.
6. Compare the remaining run-authored paths with the intake target. Record an
   existing path as covered before starting semantic duplicate review.
7. Group the remaining files by logical stem. Include `.t`, `.t.N`,
   `.shared.h`, other local headers, `.stdin`, `.args`, `.env`, and every
   `ref.*` sidecar.
8. Separate runnable families from `*.reducer.cpp`, `*.reducer.lowir`, plan
   notes, audit notes, and generated output.
9. Ignore `.my*`, `testout`, logs, binaries, object files, and other generated
   run state.

Do not infer the test count from `.ref` files. Multi-translation-unit tests can
have several sources and one reference family.

### 2. Prove What the Test Represents

Find the test in the source run, then inspect its surrounding evidence:

1. Read the source and every sidecar.
2. Search the run's `plan.md`, `audit.md`, commit messages, and diffs for the
   test name or feature.
3. Use the Ralph run record when the repository history does not identify the
   failing checkpoint.
4. Record whether the original symptom was parse rejection, wrong semantic
   output, bad LowIR, object mismatch, link failure, runtime failure, optimizer
   miscompile, timeout, or crash.
5. When history permits, prove that the reduced behavior fails before the fix
   and passes at or after the fix. For negative tests, compare the intended
   rejection and exit status rather than treating any failure as equivalent.

A test added only to measure checkpoint progress is not automatically a useful
regression. The tracker must state the compiler contract that would be lost if
the test were omitted.

### 3. Check Existing and Recently Added Coverage

Run this review before spending time on reduction and again immediately before
adding the test:

1. Search `pa*/tests` by filename terms, operators, type shapes, relevant
   standard concepts, expected symbols, and output fragments.
2. Search `cppgm.tests/course` as duplicate evidence even though it is not an
   intake destination.
3. Check recent target history for tests added after the source run snapshot.
4. Compare normalized sources after removing comments, renaming local
   identifiers, and discarding irrelevant declarations.
5. Compare the asserted result, not just the input. Two sources are near
   duplicates when they drive the same compiler path and distinguish the same
   correct result from the same failure.

Keep a new test only when it adds a material boundary, such as a different
value category, lookup context, specialization relation, lifetime edge,
calling convention class, relocation form, EH path, register-pressure
condition, or optimization safety condition. A different STL container or a
larger real-world input is not enough by itself.

The initial hash scan found no byte-identical candidate source in the target.
It does not establish unique coverage.

### 4. Reduce and Generalize

Start from the source family or raw reducer and produce the smallest test that
still proves the original compiler issue.

- Keep one primary assertion per test.
- Remove unrelated declarations, overloads, translation units, runtime work,
  template parameters, and output.
- Replace STL containers, strings, traits, function wrappers, iterators, and
  allocators with small test-owned types when the compiler behavior is a
  language rule.
- Remove private libstdc++ and libc++ names, vendor namespace spellings,
  absolute paths, compiler version strings, and unstable diagnostic wording.
- Prefer compile-time assertions or a small return code over formatted output.
- Preserve multiple translation units only for separate-compilation, symbol,
  linkage, ODR, initialization-order, EH, or ABI behavior that cannot be shown
  in one unit.
- Preserve LowIR or machine pressure only when it is essential to the backend
  or optimizer bug. Turn raw reducer artifacts into complete assignment
  fixtures with a clear oracle.

For a positive standard-language reducer, validate the source independently
with both Clang and GCC in C++11 mode when both are available:

```sh
clang++ -x c++ -std=c++11 -pedantic-errors -fsyntax-only <test>.t
g++ -x c++ -std=c++11 -pedantic-errors -fsyntax-only <test>.t
```

Use `gnu++11` only for an intentional GNU extension and record that decision.
Do not turn a libstdc++ implementation accident into a general language test.

If the bug truly concerns hosted compatibility, retain the smallest header and
library interaction that exposes it. Mark the row `hosted-portable` when the
contract comes from the C++ library interface. Mark it
`hosted-implementation-specific` only when CPPGM explicitly supports that host
ABI or library behavior. State why a header-free reducer is insufficient and
avoid assertions about private STL type names or layouts unless that private
fact is the supported ABI contract.

### 5. Select the Owning Assignment and Suite

The source run's assignment number is a hint. It is not placement authority.

1. Name the primary assertion in one sentence.
2. List every language, runtime, ABI, backend, or optimizer feature left in the
   reducer.
3. Read the candidate assignment README and the placement tracker used by
   `scripts/audit_pa_feature_placement.py`.
4. Select the earliest assignment that owns the primary assertion and all
   remaining support features.
5. If a later feature is incidental, reduce it away. If it is essential, move
   the test to that later owner.
6. Choose the local suite by matching adjacent tests and harness behavior.

Use these destination rules:

| Range | Normal destination |
|---|---|
| PA10-PA27 | `paN/tests/spec` for a narrow assignment or N3485 rule; `paN/tests/general` for cross-rule regressions |
| PA28 | `behavior`, `structural`, or `strict`, based on the asserted native-backend contract |
| PA29 | `general`, with all required translation units and link/run sidecars |
| PA30 | `abi` |
| PA31 and PA33 | `general` |
| PA34 | `preproc`, `compile`, or `run` |
| PA35 | `compile` or `run` |
| PA36 | `link` |
| PA37 | `o0`, `o1`, `o2`, `driver/o1`, `driver/o2`, `object-roundtrip`, or the matching debug-info bucket |
| PA38 | `o1`, `o2`, or the matching debug-info bucket |

Follow the numbering and naming style of the selected local suite. Rename
generic source-run names such as `900-*` or `final-audit-*` when they do not
describe the reduced behavior. Do not keep a second copy under
`cppgm.tests/course`.

The placement audit is a conservative review tool, not an owner oracle. Read
every source unit manually, especially `.t.N` files and shared headers that a
static scan may not associate with the marker file.

For syntax-tree, witness, ABI-fact, or other representation-layer suites, the
audit may detect a later semantic or lowering feature merely because its syntax
appears in the input. That is not by itself a placement failure when the test's
oracle asserts only the earlier representation contract and the owning README
requires that syntax. Record each such detector result and the manual layer
review. Move the test only when its oracle actually depends on the later
feature. Do not treat this representation-layer rule as a general waiver for a
real later-feature dependency.

### 6. Construct the Local Fixture and References

Build a fresh fixture in the destination suite instead of copying the source
family wholesale.

- Copy only source and input sidecars that remain necessary after reduction.
- Match the destination harness's conventions for `.t`, `.t.N`, `.shared.h`,
  `.stdin`, `.args`, `.env`, compile status, implementation status, program
  status, stdout, stderr, and canonical LowIR or object facts.
- Retain a failing fixture's generated `.ref.stdout` or `.ref.stderr` sidecar
  when the assignment export compares that file set. Do not drop such a
  sidecar merely because the local assignment README treats diagnostic text as
  non-semantic; export compatibility is part of the fixture contract. Record
  the generating reference binary so any exact diagnostic oracle remains
  reproducible.
- Keep source-run references as evidence until the expected result is
  understood, then generate the local canonical references from a clean host
  build of this repository's relevant tool. This tree becomes the student
  reference binary on export.
- Build the normal `dev/` tool from the frozen target commit, then pass that
  same binary through the assignment's `ref-test` workflow and use it for the
  current-tree checks. For example:

  ```sh
  make -C dev cppgm++ CXX="$CPPGM_HOST_CXX" \
    CPPGM_HOST_CXX="$CPPGM_HOST_CXX"
  make -C paNN ref-test TEST='tests/<suite>/<test>.t' \
    REF_TEST_APP=../dev/cppgm++
  make -C paNN check TEST='tests/<suite>/<test>.t'
  ```

  On macOS, use the repository's intended Homebrew LLVM compiler explicitly:
  `/usr/local/opt/llvm/bin/clang++` when present, otherwise
  `/opt/homebrew/opt/llvm/bin/clang++`. Pass that path as both `CXX` and
  `CPPGM_HOST_CXX` for the build and validation commands. If either compiler
  setting changes, discard the generated contents of the normal `obj/` root
  before rebuilding so objects from a different host compiler cannot be
  reused. This is still the normal development build, not a separate
  reference object root.

  Use the assignment's corresponding frontend or backend binary instead of
  `cppgm++` when that is the actual oracle. Record the target commit, host
  compiler, build command, and resulting binary hash in the tracker or batch
  validation record. Do not maintain a separate reference object tree when the
  normal `dev/` binary is also the compiler under test.
- A source-run reference may be compared against the clean target build as a
  provenance and compatibility control, but it must not replace local
  reference generation.
- If the clean target build disagrees with independently established intended
  behavior, mark the row `blocked-current-bug` and handle the production fix
  separately before regenerating the reference. Do not encode a known compiler
  bug into the oracle.
- Do not regenerate witness references from `cppgm++`. Witness refs must come
  from the patched-Clang witness path. Preserve an existing golden witness when
  relocating a test unless that oracle has been independently re-established.
- Treat LowIR text drift as a semantic review item before updating a reference.
- Do not add `.my*`, diffs, logs, temporary reductions, or build products.

For a focused local reference run, use the assignment path selected by its
Makefile, for example:

```sh
make -C paNN ref-test TEST='tests/<suite>/<test>.t' \
  REF_TEST_APP=../dev/<clean-host-built-tool>
```

Later assignments route tests by bucket. Check that Makefile before generating
references rather than assuming the generic command is sufficient.

### 7. Validate an Intake Candidate

Run the narrowest checks first:

1. Independent C++11 validation or the documented hosted/extension check.
2. Historical fail/pass proof when the source history supports it.
3. Reference generation from the clean host-built target binary recorded for
   the batch.
4. Focused current-compiler execution:

   ```sh
   make -C paNN check TEST='tests/<suite>/<test>.t'
   ```

5. Placement audit over normal assignment tests:

   ```sh
   python3 scripts/audit_pa_feature_placement.py \
     --pa paNN \
     --no-course \
     --markdown-out /tmp/paNN-placement-audit.md \
     --json-out /tmp/paNN-placement-audit.json \
     --fail-on-early
   ```

   If the repository-wide command also reports pre-existing findings, inspect
   the JSON row for every new family. The intake gate passes only when each new
   finding is either fixed or explicitly shown to be a representation-layer
   false positive under the rule above; retain the full audit output as batch
   evidence.

6. The full owning assignment report:

   ```sh
   ACTIVE_TEST_REPORT_PAS='paNN' make test-report
   ```

7. For LowIR-sensitive tests, repeat the focused and owner checks with:

   ```sh
   CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ACTIVE_TEST_REPORT_PAS='paNN' \
     make test-report
   ```

8. Run `make test-report-through-paNN` when shared frontend behavior, lowering,
   object generation, hosted runtime behavior, or optimizer behavior makes
   earlier preservation relevant.
9. Run the PA37 or PA38 debug-info and bucket-specific targets when the test
   belongs to those optional surfaces.
10. Finish with `git diff --check` and inspect `git status --short` for
    generated artifacts.

A placement audit pass does not replace the manual ownership review. A failure
requires more reduction or a later owner. Do not suppress a real finding to
keep the proposed path.

### 8. Land Small, Reviewable Batches

Process tests by reduced behavior and owning assignment, not by copying a whole
source directory.

Recommended order:

1. PA10 through PA27 portable language and semantic reducers
2. PA28 through PA33 backend, separate-compilation, ABI, and object reducers
3. PA34 through PA36 hosted preprocessing, compile, runtime, and link reducers
4. PA37 and PA38 optimizer and machine-backend reducers
5. PA1 through PA9 only after a second-pass plan reviews their harnesses and
   placement risks

Before each batch, rerun the duplicate search against the current target. New
tests may have landed since the initial inventory.

Use one commit per feature cluster or owning assignment. Each commit should
contain only:

- reduced tests in `paN/tests`
- required canonical references and input sidecars
- tracker rows for those tests
- a small README clarification when assignment ownership was genuinely unclear

Do not combine unrelated reducers, bulk source-run references, production
compiler fixes, or generated output in an intake commit.

## Acceptance Criteria

A PA10-through-PA38 source family is complete when it has one recorded
disposition and the evidence supports that disposition.

An accepted test must satisfy all of these conditions:

- it represents a reproducible compiler regression or stable contract
- it is reduced enough that the asserted behavior is clear
- no existing test already covers the same compiler path and oracle
- its C++11, GNU-extension, or hosted-specific status is explicit
- it lives in the correct `paN/tests` suite
- all necessary multi-file and input sidecars are present
- its references come from the correct canonical source
- the focused test, placement audit plus manual layer review, and owning
  assignment report pass
- broader LowIR, through-PA, hosted, debug-info, or optimization checks pass
  when applicable
- no generated files or source-run artifacts remain in the change

The first pass is complete when every PA10-through-PA38 candidate has a
disposition, every accepted test has landed through the gates above, and the
PA1-through-PA9 catalog remains unchanged for its separate review.
