# Fable and Mini Test Intake Plan

## Intake Status

Both intake passes completed on 2026-08-02. The tracker contains 505 logical
families and no remaining `catalog-pa1-pa9`, `needs-evidence`,
`needs-reduction`, `ready`, or `blocked-current-bug` rows. Of those families,
339 were intaked, 21 are covered by existing tests, and six are not
regressions. The other rows record 128 assignment-export imports and 11
reference-only repairs. Every
completed decision row records the actual commit that made the decision; the
tracker does not retain contextual `(this commit)` placeholders.

The final blocked-test pass corrected every compiler bug previously recorded
by this tracker and intaked each effective reducer at its current owner. The
last three closures added typed covariant-result thunk facts and hosted PA33
runtime/symbol coverage, narrowed unused GNU-inline wrapper deferral to typed
unsupported builtins, and retained GNU `vector_size` byte width for PA34
compile-time layout validation.

We traced two post-rebase regressions across the complete intake stack. In
`c6d6cd4f8`, we added the stricter namespace-conflict check and exposed an
older inline-namespace bookkeeping bug: import copied the child namespace's
internal anonymous-namespace alias over the parent's alias. In `30db85e5c`, we
excluded that alias from the user-visible import set. In `b5600a656`, we
replaced object-symbol decoding with typed ABI construction but failed to
retain the typed mangle target for instantiated destructor vtable entries. In
`30db85e5c`, we retained that target for reconstructed destructor entries and
used the existing typed ABI facts for non-virtual and virtual-base thunks.
Global ABI-fact capture remains disabled, and mangling consumes typed data
without text parsing.

A final source-ledger audit reproduced the plan's inventory from the frozen
repositories: 286 PA10-through-PA38 course families, 59 PA1-through-PA9
families, 128 assignment-export imports, and 11 reference-only repairs. Every
PA1-through-PA9 family now has a reviewed disposition: 54 were intaked, four
are covered by another retained test, and one is not a regression. In the
first-pass ledger, every one of the 1,204 newly added course-tree files is named
by a tracker family, every claimed source sidecar exists at its recorded source
commit, and all 827 listed destination entries exist under `paN/tests`.

Canonical references came from this repository's normal Homebrew-Clang-built
`dev/` binaries, which are the student reference binaries on export. Intake
found one standards-valid PA7 parenthesized-parameter case that the current
compiler rejected. Clang and GCC confirmed the source, the shared structured
declarator parser was corrected, and the canonical reference was generated
only after the fix.

The PA4-through-PA9 second-pass batch passed 300 of 300 tests with the intended
Homebrew Clang build, and the earlier PA1-through-PA3 batches passed their
focused and assignment reports. After the PA3 and PA9 timeout optimizations,
the post-rebase root report passed 4,855 of 4,855 tests with direct LowIR text
comparison enabled. The configured PA18/PA19/PA21/PA22/PA23 strict pass compared
1,305 references with zero failures. All focused, placement, owning-assignment,
hosted, LowIR optimizer, and machine optimizer gates recorded in the tracker
passed.

The two load-sensitive stress cases were optimized from the run repositories'
implementation evidence without changing their checked output:

- PA3 `300-triple.t` now uses fixed operator classification, precedence
  climbing, reusable indexed token storage, and block-sized stdin ingestion.
  Local `time -lp` measurement fell from 19,383,152,194 to 6,208,773,797
  retired instructions and from 2.26 to 0.84 seconds. Peak footprint rose from
  about 11 MB to 32 MB because the 11 MB input is deliberately held in memory
  instead of passing through locked character-at-a-time stdio.
- PA9 `300-binary-calculator.t.1` keeps contractual statement-label addresses
  while placing executable bodies on cache lines isolated from writable data.
  Exact direct control transfers and the entry point target those bodies;
  indirect transfers retain label-address entry sleds. Local runtime fell from
  3.42 to 0.31 seconds and elapsed cycles from 12,070,404,945 to 864,494,196,
  confirming removal of self-modifying-code machine clears.

## Scope

Use the Fable and Mini run repositories as evidence sources for new regression
tests. Do not merge their `cppgm.tests` trees into this repository. In the
completed PA10-through-PA38 pass, accepted tests belong in the normal
`paN/tests` assignment suites. In the PA1-through-PA9 pass, accepted tests stay
in `cppgm.tests/course/paN`, which is the early-assignment course-test surface.

The first intake pass covered PA10 through PA38. The second pass started from
the PA1-through-PA9 catalog below and applied the same evidence, reduction,
duplication, ownership, README, reference, and validation requirements with
the early-assignment destination and harness rules stated below.

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
exported. Generate intake references with the normal `dev/` binary built from
the frozen intake target commit by the intended host compiler. Use that same
binary for reference generation and current-tree validation. Do not create a
separate reference object root, and do not clean or rebuild unchanged objects
solely for intake. References from Fable and Mini remain evidence for
reconstructing the intended behavior; they are not the canonical source for a
newly intaked fixture.

## Intake Rules

1. Treat an upstream test as evidence, not as a ready-to-copy fixture.
2. Track one logical test family per row. A family includes its `.t`, `.t.N`,
   headers, input files, arguments, environment files, and reference sidecars.
3. Reduce the test to one compiler claim before deciding where it belongs.
4. Put the result in the earliest assignment that owns the primary claim and
   every feature that remains in the reducer.
5. For PA10 through PA38, add the result under `paN/tests`, never under
   `cppgm.tests/course`. For PA1 through PA9, keep accepted tests under
   `cppgm.tests/course/paN`, even when the source family originally appeared in
   a run repository's assignment-local `paN/tests` tree.
6. Reject exact and near duplicates unless the new test reaches a distinct
   semantic, lowering, ABI, runtime, or optimizer path.
7. Prefer standard C++11 and test-owned helper types. Keep hosted or
   implementation-specific dependencies only when they are the behavior under
   test.
8. Regenerate references through the repository's reference workflow, using
   this tree's normal relevant `dev/` binary built with the intended host
   compiler. Verify the binary and build configuration before use, but do not
   clean or rebuild unchanged objects. Do not copy a run reference as the new
   canonical oracle.
9. Keep PA1 through PA9 at `catalog-pa1-pa9` during the first pass. During the
   second pass, replace each catalog disposition with its reviewed result.

### PA1 Through PA9 Course-Test Rules

- Keep accepted families in the flat `cppgm.tests/course/paN` directory used
  by that assignment's harness. Do not move them into `paN/tests` and do not
  retain a second assignment-local copy from a run repository.
- Treat the numeric filename prefix as a coverage band, not a globally unique
  sequence number. Choose the band by comparing the reduced behavior with the
  nearest tests in both `paN/tests` and `cppgm.tests/course/paN`; reuse a band
  when several descriptive names belong at the same complexity or boundary
  level. Do not preserve source-run numbers such as `460`, `900`, or `901`
  merely because they were the next sequential identifier in that run.
- Normalize the existing PA1-through-PA9 course families in the same pass so
  the directory has one coherent scheme. Rename each complete family
  atomically, including references, statuses, streams, inputs, environment
  files, headers, and numbered units. Update path-sensitive reference text and
  validate the renamed family through its real harness. These mechanical
  renames do not create new intake candidates or tracker rows.
- Give every accepted family a descriptive behavior name after the band. A
  filename must remain understandable without consulting the tracker.
- Preserve the assignment's actual early harness contract. PA1 through PA4
  compare their token or expression output and exit status, PA2 through PA4
  may also retain diagnostic streams, PA5 has preprocessing path and multi-file
  sidecars, PA6 checks recognition output, and PA7 through PA9 use their own
  namespace or CY86 formats. Retain every stdout, stderr, status, input,
  argument, header, or numbered translation-unit sidecar required by that
  harness.
- Generate canonical references with this tree's normal Homebrew-Clang-built
  relevant `dev/` binary and keep failure output when the assignment export
  compares it. Do not copy run references or discard an apparently redundant
  stream without checking the early harness.
- Do not run the semantic/LowIR placement audit for PA1 through PA9. Those
  files are inputs to milestone-specific frontends, so later C++ feature
  spellings are token data rather than evidence of later semantic ownership.
  Determine ownership by reading the fixed assignment README, adjacent tests,
  and the real early harness instead.
- Treat the PA1-through-PA9 READMEs as fixed assignment specifications during
  ordinary intake. If a candidate exercises behavior not required there, do
  not add it as a new student obligation; place an intentional hosted/GNU
  extension in its later owner when one exists, otherwise reject it. An
  explicitly approved correction to an erroneous or ambiguous assignment
  contract is a separate change: validate it against the relevant standard
  compilers, update the instructions and checked-in oracle together, and
  record that evidence in the tracker.
- When a run reference and the current compiler agree on behavior that
  contradicts the fixed README, treat that agreement as a compiler bug rather
  than validation. Reduce the contradiction, fix the normal `dev/` compiler,
  and add the smallest positive or negative course regression required by the
  existing specification. Generate its canonical reference only after the fix.

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

## PA1 Through PA9 Second-Pass Catalog

These names record the logical source families that are absent by path from the
clean intake base. They were untouched during the first pass and form the
complete starting queue for the second pass.

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

The second pass started from this catalog and repeated the evidence,
duplication, reduction, ownership, and reference checks. It also accounted for
the distinct PA1 through PA9 harness formats before moving any file.

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
- portability class: `standard-c++11`, `post-c++11`, `gnu-extension`,
  `lowir-contract`, `hosted-portable`, or
  `hosted-implementation-specific`; use `post-c++11` only when a rejected or
  covered source depends on later-standard language forms that are outside the
  C++11 assignment contract, and use `lowir-contract` only for a raw LowIR
  fixture whose portability boundary is the checked-in LowIR specification
  rather than a host C++ implementation
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
- `intaked`: accepted in the active pass's destination (`cppgm.tests/course/paN`
  for PA1 through PA9, otherwise `paN/tests`) with all required checks passing

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
2. Search `cppgm.tests/course` as duplicate evidence. It is also the intake
   destination for PA1 through PA9, so compare against the live destination
   again immediately before landing an early-assignment batch.
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
3. Read the candidate assignment README. For PA10 and later, also read the
   placement tracker used by `scripts/audit_pa_feature_placement.py`.
4. Compare the primary assertion with the assignment's stated student-facing
   requirements. PA1-through-PA9 READMEs are fixed during ordinary intake:
   reject or move behavior they do not cover unless an assignment-contract
   correction has been explicitly approved and validated. For PA10 and later,
   make the smallest clear contract edit needed when an accepted behavior adds
   a student expectation. Do not
   prescribe an implementation strategy or promise a diagnostic format unless
   that exact text is intentionally part of the exported oracle. Record the
   README section reviewed, and any edit made, in the tracker validation
   evidence for the family.
5. Select the earliest assignment that owns the primary assertion and all
   remaining support features.
6. If a later feature is incidental, reduce it away. If it is essential, move
   the test to that later owner.
7. Choose the local suite by matching adjacent tests and harness behavior.

Use these destination rules:

| Range | Normal destination |
|---|---|
| PA1-PA9 | flat `cppgm.tests/course/paN`, using the nearest assignment-local numbering band and the assignment's early harness sidecars |
| PA10-PA27 | `paN/tests/spec` for a narrow assignment or N3485 rule; `paN/tests/general` for cross-rule regressions |
| PA28 | `behavior`, `structural`, or `strict`, based on the asserted native-backend contract |
| PA29 | `general`, with all required translation units and link/run sidecars |
| PA30 | `abi` |
| PA31 and PA33 | `general` |
| PA34 | `preproc`, `compile`, or `run` |
| PA35 | `compile` |
| PA36 | `link` |
| PA37 | `o0`, `o1`, `o2`, `driver/o1`, `driver/o2`, `object-roundtrip`, or the matching debug-info bucket |
| PA38 | `o1`, `o2`, or the matching debug-info bucket |

Treat PA35 and PA36 as expensive hosted integration tiers, not default homes
for student-added regressions. Put a new PA35 intake test in `compile`, and make
it assert a distinct compile-time behavior of a hosted C++ header workload;
mere successful compilation of an unused construct is not enough when existing
PA35 coverage already reaches the same header machinery. Put a new PA36 intake
test in `link`, and make it execute and check distinct runtime or link behavior
emitted from hosted headers. Reduce header-independent language, ABI, EH, or
backend bugs to their earlier owning assignment. Keep a PA35 or PA36 test only
when that hosted integration is essential and the case materially increases
coverage over the existing suite. Do not add a PA35/PA36 test merely to retain
the source repository's original placement. When the new assertion extends the
same hosted-header path as an existing family, prefer strengthening that family
without adding another test process, provided the combined runtime oracle stays
focused and preserves the original assertion.

Follow the numbering and naming style of the selected local suite. Rename
generic source-run names such as `900-*` or `final-audit-*` when they do not
describe the reduced behavior. For PA10 and later, do not keep a second copy
under `cppgm.tests/course`. For PA1 through PA9, that course directory is the
only accepted destination.

For PA10 and later, the placement audit is a conservative review tool, not an
owner oracle. Read every source unit manually, especially `.t.N` files and
shared headers that a static scan may not associate with the marker file. Do
not use the semantic/LowIR placement audit for PA1 through PA9.

Do not leave a known false-positive placement finding attached to an accepted
intake test. Narrow or otherwise improve the placement detector when its rule
conflates the owning behavior with incidental syntax. If the match is caused
only by an incidental local identifier, rename that identifier instead. Rerun
the audit and require the accepted fixture to be clear of the false finding;
recording a manual waiver is not sufficient.

For syntax-tree, witness, ABI-fact, or other representation-layer suites, the
audit may initially detect a later semantic or lowering feature merely because
its syntax appears in the input. When the oracle asserts only the earlier
representation contract and the owning README requires that syntax, encode
that distinction in the detector so the audit reports the fixture clean. Do
not retain a representation-layer waiver. Move the test when its oracle really
depends on the later feature.

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
  understood, then generate the local canonical references from this
  repository's normal host-built tool. This tree becomes the student reference
  binary on export.
- Build the normal `dev/` tool from the frozen target commit with the intended
  host compiler when it is not already current. Do not clean first and do not
  create a separate reference object root. Pass that same binary through the
  assignment's `ref-test` workflow and use it for the current-tree checks. Once
  the tool is current, skip the redundant dev rebuild in focused runs. For
  example:

  ```sh
  make -C dev cppgm++ CXX="$CPPGM_HOST_CXX" \
    CPPGM_HOST_CXX="$CPPGM_HOST_CXX"
  make -C paNN ref-test TEST='tests/<suite>/<test>.t' \
    REF_TEST_APP=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
  make -C paNN check TEST='tests/<suite>/<test>.t' \
    CPPGM_SKIP_DEV_REBUILD=1
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

  For PA30 ABI-name tests, validate the spelling against the intended Homebrew
  Clang before accepting a reference. Start from a small real C++ declaration,
  compile it with Clang and inspect the exact emitted symbol, then run the same
  source through `dev/cppgm++ --emit-abi-facts`. Trim that emitted fact file to
  the single name under test and require `dev/abimangle` to reproduce Clang's
  symbol exactly. `c++filt` is useful for checking that a symbol denotes the
  intended entity, but the Clang symbol is the oracle.

  Treat the emitted text as semantic-model evidence, not as the preferred
  checked-in fixture spelling. After proving the emitted facts reproduce the
  Clang symbol, minimize the reducer within the PA30 fact grammar and prove the
  minimized facts produce the same symbol. Omit a `case` label for a one-case
  file, replace serializer-generated `__abi_*` binders with short descriptive
  identifiers, and prefer compact `function`, `function path`, and inline type
  forms when they describe the same target. Keep separate definitions and
  references when their graph relationship is what the reducer tests; binder
  spellings themselves never participate in the ABI name.

  If `abimangle` or `--emit-abi-facts` disagrees with Clang, record a current
  compiler bug instead of generating the mismatching local reference. If the
  current pipeline agrees with Clang but a source-run reference does not, treat
  the source-run reference as incorrect and do not preserve it.

  Establish the correctly configured host-compiler build once when the frozen
  source commit or compiler configuration changes. During test-only intake
  batches, reuse the existing normal object tree for reference generation and
  root `test-report` validation; do not run a clean target merely to validate a
  new test or README edit. Incremental frontend relinking is expected and does
  not require recompiling unchanged objects.
- A source-run reference may be compared against the normal target build as a
  provenance and compatibility control, but it must not replace local
  reference generation.
- If the normal target build disagrees with independently established intended
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
  REF_TEST_APP=../dev/<host-built-tool> CPPGM_SKIP_DEV_REBUILD=1
```

Later assignments route tests by bucket. Check that Makefile before generating
references rather than assuming the generic command is sufficient.

### 7. Validate an Intake Candidate

Run the narrowest checks first:

1. Independent C++11 validation or the documented hosted/extension check.
2. Historical fail/pass proof when the source history supports it.
3. Reference generation from the recorded normal host-built target binary.
4. Re-read the owning assignment README against the final reduced source and
   oracle. For PA1 through PA9, require the behavior to be an existing explicit
   student expectation and do not edit the README during ordinary intake. If
   an assignment-contract correction was explicitly approved, validate the
   corrected rule independently and update the README and checked-in oracle in
   the same batch. For PA10 and later, land a minimal clarification when the
   accepted behavior adds an expectation.
5. Focused current-compiler execution:

   ```sh
   make -C paNN check TEST='tests/<suite>/<test>.t'
   ```

6. For PA10 and later, run the placement audit over normal assignment tests.
   Skip this step for PA1 through PA9:

   ```sh
   python3 scripts/audit_pa_feature_placement.py \
     --pa paNN \
     --no-course \
     --markdown-out /tmp/paNN-placement-audit.md \
     --json-out /tmp/paNN-placement-audit.json \
     --fail-on-early
   ```

   The complete affected-assignment audit must exit successfully. Do not carry
   a finding merely because it predates the intake batch. Fix detector false
   positives, rename incidental trigger identifiers, reduce later features, or
   move genuine later-owner tests until the audit is clean. If a cleanup moves
   a test to another assignment, run and retain the audit for both the source
   and destination assignments.

   For intake into PA18 through PA23, also emit the template-placement view and
   inspect every accepted test's `Concepts For Review` and `Composition
   Concepts` columns:

   ```sh
   python3 scripts/audit_pa_feature_placement.py \
     --pa paNN \
     --no-course \
     --template-placement \
     --markdown-out /tmp/paNN-template-placement.md \
     --json-out /tmp/paNN-template-placement.json
   ```

   PA22's ordinary owner view filters prerequisite template
   mechanisms so a focused deduction or SFINAE test can remain at its
   single-feature owner. The composition view retains those prerequisites.
   When two or more concepts are essential to the assertion together, move the
   test to PA23 and choose its cluster from the reported combination. This is a
   manual review lead rather than a hard placement failure because template
   fixtures often use earlier mechanisms only as assertion scaffolding.

   The audit scans numbered host-test sources such as `.t.1` and `.t.2`.
   Generating LowIR for each later-assignment test makes the normal audit slow.
   Probe one host test only if source and history review leave its placement
   ambiguous because its checked oracle has no LowIR. The generated unit runs
   through the same source-and-reference feature detector used for checked
   LowIR. Repeat the final option for other ambiguous anchors:

   ```sh
   python3 scripts/audit_pa_feature_placement.py \
     --pa paNN \
     --no-course \
     --lowir-probe-app dev/cppgm++ \
     --probe-lowir-test paNN/tests/<suite>/<test>.t \
     --markdown-out /tmp/paNN-placement-audit.md \
     --json-out /tmp/paNN-placement-audit.json \
     --fail-on-early
   ```

   Use this targeted fallback when history links a regression to cleanup,
   unwind, protected-region merging, LSDA, or another EH-sensitive compiler
   change but the source has no explicit `try`, `catch`, or `throw`. If the
   probe finds EH or another later-owned LowIR feature, preserve that assertion
   in the destination test or keep the test at the later milestone. A failed
   probe makes `--fail-on-early` fail; it provides no evidence that the test
   lacks the generated behavior.

7. The full owning assignment report:

   ```sh
   ACTIVE_TEST_REPORT_PAS='paNN' make test-report
   ```

8. For LowIR-sensitive tests, repeat the focused and owner checks with:

   ```sh
   CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ACTIVE_TEST_REPORT_PAS='paNN' \
     make test-report
   ```

9. Run `make test-report-through-paNN` when shared frontend behavior, lowering,
   object generation, hosted runtime behavior, or optimizer behavior makes
   earlier preservation relevant.
10. Run the PA37 or PA38 debug-info and bucket-specific targets when the test
   belongs to those optional surfaces.
11. Finish with `git diff --check` and inspect `git status --short` for
    generated artifacts.

For PA10 and later, a placement audit pass does not replace the manual
ownership review. A failure requires more reduction or a later owner. Do not
suppress a real finding to keep the proposed path.

### 8. Land Small, Reviewable Batches

Process tests by reduced behavior and owning assignment, not by copying a whole
source directory.

Recommended order:

1. PA10 through PA27 portable language and semantic reducers
2. PA28 through PA33 backend, separate-compilation, ABI, and object reducers
3. PA34 through PA36 hosted preprocessing, compile, runtime, and link reducers
4. PA37 and PA38 optimizer and machine-backend reducers
5. PA1 through PA9 by owning assignment, using their course-test destinations
   and assignment-specific harnesses

Before each batch, rerun the duplicate search against the current target. New
tests may have landed since the initial inventory.

Use one commit per feature cluster or owning assignment. Each commit should
contain only:

- reduced tests in the destination appropriate to the active pass
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
- it lives in `cppgm.tests/course/paN` for PA1 through PA9 or the correct
  `paN/tests` suite for PA10 through PA38
- all necessary multi-file and input sidecars are present
- its references come from the correct canonical source
- the focused test and owning assignment report pass; for PA10 and later, the
  placement audit plus manual layer review also pass
- broader LowIR, through-PA, hosted, debug-info, or optimization checks pass
  when applicable
- no generated files or source-run artifacts remain in the change

The first pass is complete when every PA10-through-PA38 candidate has a
disposition and every accepted test has landed through the gates above. The
second pass is complete when all 59 PA1-through-PA9 catalog rows have reviewed
dispositions, every accepted family has landed in its normalized course-test
name with the required early-harness sidecars, and PA1 through PA9 pass both
focused and assignment-level validation.
