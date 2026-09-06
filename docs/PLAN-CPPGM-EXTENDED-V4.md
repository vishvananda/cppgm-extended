# Plan: make this implementation the reference in cppgm-extended (v4)

Status: in progress on branch `v4` (2026-09-06).  Phases 0 to 6 have landed;
Phase 7 is under way: the export script and CI are updated, and the CI drive
across the four host cells surfaced the compiler defects recorded in
`docs/v4/moves.md` ("Phase 7: the host cells"), all fixed in the source tree
and synced here.  Phase 8 is deferred.  The trackers are in `docs/v4/`
(`README.md` indexes them): `ref-deltas.md` (Phase 3), `moves.md` (Phases 4
and 5 and the selfhost fixes), `readme-review.md` (Phase 6).

## Where the two trees stand

`~/cppgm-extended` is the maintainer repository: the full compiler under
`dev/src`, the assignment handouts, the maintainer tooling, and the export
script that produces the student repository (`cppgm-assignments`).  This
tree (`cppgm-run-v3codex`) began as that export: 55 export commits, the last
on 2026-08-20 from extended commit `e539cdd6`, and 945 commits of our own
since.  In that time the implementation here was rebuilt and restructured,
the harnesses and contracts changed, and the tests grew mostly in course
lanes.  The two trees now differ as follows (measured 2026-09-06 against
extended `main` at `785f55ace`; the checkout there sits on `fast-tests`,
one commit ahead).

Implementation and build:

- `dev/src`: 282 flat files there against a subdirectory layout here
  (`abi`, `lowir`, `lowering`, `native`, `semantic`, `syntax`, ...); one
  file in common.  The move is a replacement, not a merge.
- `dev/Makefile`, `dev/frontend_source_sets.mk`, every `dev/*.cpp` scaffold
  and `gen_builtin_host_config.pl` differ.  The export script generates the
  student `dev/Makefile` and an empty source-set manifest from heredocs,
  and CI checks the generated Makefile's relink stability
  (`scripts/tests/test_exported_dev_makefile.py`).
- Root `Makefile`: 268 diff lines.  Extended has `test-strict`,
  `ref-test-strict`; here there are the toolchain cells (`test-cells`,
  `with-clang-*`), `test-variants`, `test-telemetry-off`, `asan-build`,
  the `audit-*` targets, `reference-binaries`, `setup`.
- `scripts/`: eight shared harness scripts differ (among them
  `compare_results_common.pl`, `run_all_tests_common.pl`, the native and
  host-compat workers); 151 files exist on one side only: maintainer
  tooling there (audits, witness generation, perf validation, boost
  frontier), new student-facing tooling here (`expect_ir.pl`,
  `make_ir_expect.pl`, `make_ir_contract.pl`, `make_ir_outcome.pl`,
  `check_ir_envelope.pl`, `check_lowir_seams.py`, `lowir_seam_rewrite.py`,
  `scripts/tests/*.py`).  The export excludes every `.py` file today.
- Root documents: `TESTING_AND_REFERENCES.md` differs (40 lines); README,
  PROJECT_LAYOUT and AGENTS are still the exported ones.

Tests, by tracked file, per assignment (identical / modified in place /
only here / only there):

| pa | same | diff | here | there | notes |
|---|---|---|---|---|---|
| pa1-pa4 | 279 | 0 | 0 | 0 | |
| pa5 | 235 | 0 | 33 | 0 | |
| pa6-pa7 | 228 | 0 | 0 | 0 | |
| pa8 | 137 | 0 | 20 | 0 | |
| pa9-pa10 | 538 | 0 | 11 | 0 | |
| pa11 | 204 | 5 | 17 | 0 | |
| pa12 | 530 | 1 | 29 | 0 | |
| pa13 | 306 | 11 | 118 | 27 | 27 there: `spec/200-bad-*` cases |
| pa14 | 345 | 1 | 2 | 0 | |
| pa15 | 407 | 14 | 13 | 0 | |
| pa16 | 916 | 32 | 20 | 6 | |
| pa17 | 853 | 33 | 23 | 3 | |
| pa18 | 111 | 1 | 4 | 0 | |
| pa19 | 1128 | 19 | 17 | 279 | there: `.ref.witness` |
| pa20 | 630 | 12 | 6 | 158 | there: `.ref.witness` |
| pa21 | 478 | 17 | 15 | 4 | |
| pa22 | 1179 | 29 | 16 | 295 | there: `.ref.witness` |
| pa23 | 1525 | 54 | 15 | 387 | there: `.ref.witness` |
| pa24 | 1645 | 19 | 7 | 417 | there: `.ref.witness` |
| pa25 | 461 | 16 | 4 | 0 | |
| pa26 | 373 | 39 | 5 | 5 | |
| pa27 | 365 | 9 | 3 | 22 | there: `.ref.witness` |
| pa28 | 151 | 13 | 0 | 0 | |
| pa29 | 714 | 173 | 179 | 0 | `.mir`, `.ref.cmir`, `.ref.expect` |
| pa30 | 505 | 4 | 0 | 0 | |
| pa31 | 103 | 11 | 2 | 0 | |
| pa32 | 814 | 49 | 16 | 0 | |
| pa33 | 549 | 18 | 0 | 0 | |
| pa34 | 1514 | 16 | 110 | 0 | |
| pa35 | 412 | 9 | 1 | 0 | |
| pa36 | 429 | 3 | 33 | 0 | |
| pa37 | 211 | 30 | 176 | 3 | `.ref.expect`, `.ref.ir` |
| pa38 | 148 | 64 | 56 | 0 | `.mir`, `.ref.expect` |

The 700 modified files are 325 `.ref`, 130 `.mir`, 91 `.ref.cmir`,
69 `.stdout`, 10 `.ref.inspect`, and only 30 source files (22 `.t`,
8 `.h`) plus 45 multi-unit inputs.  The 1,590 files only there are 1,567
`.ref.witness` (the strict lane's Clang-derived oracle, untracked here)
and 23 files of 10 PA13 fixtures.  The 1,000 files only here are mostly
sidecars (`.ref.expect`, `.stdout`, `.mir`, exit statuses) and 95 `.t`.

Course and regression lanes here (`cppgm.tests/course`, `cppgm.tests/regression`),
counted as test units and as files:

| pa | course units | course files | regression units |
|---|---|---|---|
| pa1-pa9 | 127 | 598 | 0 |
| pa10-pa14 | 73 | 207 | 0 |
| pa15-pa18 | 116 | 340 | 0 |
| pa19-pa24 | 102 | 341 | 0 |
| pa25-pa28 | 34 | 117 | 0 |
| pa29 | 126 | 610 | 125 |
| pa30-pa34 | 55 | 292 | 0 |
| pa35-pa36 | 51 | 205 | 0 |
| pa37 | 117 | 503 | 221 |
| pa38 | 39 | 227 | 65 |

Extended's `cppgm.tests/course` holds only the PA1-PA9 tests (576 files)
and `.gitkeep` placeholders: its own audit rule already says PA10+ course
tests must move into the suites.  Buckets in use here:
`pa15/controls`, `pa16/controls`, `pa17/controls`, `pa29/{behavior,controls,strict,structural}`,
`pa32/controls`, `pa34/{compile,preproc,run}`, `pa35/compile`, `pa36/link`,
`pa37/{o0,o1,o2,o3,driver,debuginfo,object-roundtrip}`,
`pa38/{o1,o2,o3,behavior,driver,debuginfo}`, and the regression lane's
`pa37/controls`, `pa38/controls`.

Handouts and wrappers (diff lines against extended):

| pa | README | Makefile | pa | README | Makefile |
|---|---|---|---|---|---|
| pa10 | 8 | 4 | pa25 | 2 | 25 |
| pa11 | 27 | 4 | pa26 | 9 | 4 |
| pa12 | 7 | 4 | pa28 | 17 | 4 |
| pa13 | 132 | 23 | pa29 | 466 | 109 |
| pa15 | 76 | 23 | pa31 | 33 | 11 |
| pa16 | 44 | 43 | pa32 | 15 | 20 |
| pa17 | 79 | 23 | pa33 | 8 | 6 |
| pa18 | 5 | 4 | pa34 | 90 | 44 |
| pa19 | 16 | 27 | pa35 | 0 | 26 |
| pa20 | 3 | 27 | pa36 | 0 | 26 |
| pa21 | 13 | 4 | pa37 | 1090 | 183 |
| pa22 | 31 | 27 | pa38 | 566 | 159 |
| pa23 | 6 | 27 | pa39 | 0 | 43 |

The four-line Makefile deltas are one export-time rewrite (the test-runner
source id); the large ones (pa13, pa16, pa29, pa34, pa37, pa38) are ours:
course roots, regression lane, expectation sidecars, floors, seams,
variants.  `pa13/lowir.md` differs too (the contract sections added here).

CI in extended (`.github/workflows`): `tests.yml` runs the placement audit
(`scripts/audit_pa_feature_placement.py --fail-on-early`) and the exported
dev Makefile check, builds in four flavors (gcc/libstdc++ and clang/libc++
on ubuntu 24.04 and 26.04), then per flavor runs `test-report`,
`test-strict`, `test-debuginfo`, and PA39 `test-through-pa10`;
`selfhost.yml` and `inception.yml` compare the self-built compiler;
`export-assignments.yml` runs the export after a green `tests` run,
publishes the reference-binary bundle, and pushes to `cppgm-assignments`.
Two of those lanes are not green for this implementation today: the strict
witness lane, whose `--witness` mode this compiler no longer has (it is
purged in `v4`, Principle 5), and the PA13 debug-info lane (4 failures
against reference-generated refs, regenerated in Phase 3).

## Principles

1. **One branch, nothing on `main` until the end.**  Work happens on `v4`
   in `~/cppgm-extended`, branched from `main` (`785f55ace`).  It merges
   when every CI job is green and the export is produced from it, built,
   and passes its own suites.
2. **The reference is this tree at one pinned commit.**  Every `.ref`,
   `.mir`, `.cmir`, `.stdout`, `.ref.ir` and debug-info reference in `v4`
   is regenerated from that implementation, never hand-edited, and every
   difference from the old reference is written down once as a contract
   change or a fix (Phase 3).  No source reshaping to match old refs.
3. **A test lives in exactly one place**: `paN/tests/<bucket>/`, with a
   three-digit cluster prefix.  `cppgm.tests/course`, the `paN/course`
   symlinks and the `COURSE_TEST_ROOT` Makefile logic go away.
4. **The student handout wins.**  A README change made here is kept only
   if it makes the assignment clearer for the person implementing it, by
   the rules in extended's `docs/implemented/v3/pa10-37-readme-handout-audit.md`; the
   contract-and-envelope material (Quality Bar, One Design, Validation
   Modes, the LowIR absorbs/enforces lists) stays because it is exactly
   that, but each rewritten README is read once more as a handout.
5. **The witness lane is gone.**  This implementation removed `--witness`;
   `v4` purges the strict lane with it: the 1,567 `.ref.witness` files,
   `test-strict`/`ref-test-strict`, `run_witness_tests.pl`,
   `compare_witness_results.pl`, `validation/templates`, the
   `clang_witness_json_to_ref.py` tooling, and the `test-strict` CI job.
6. **Extended's conventions hold** (`AGENTS.md` there): production code in
   `dev/src`; plans and trackers in `docs/`, completed plans in
   `docs/implemented/`; `make test-report` is the broad surface;
   commit messages in the repository's style.

## Phases

### Phase 0: Inventory and freeze

- Pin the source commit here (`v3codex-ref = <sha>`); nothing later than
  it enters `v4` until the plan closes.
- Generate the inventories the phases below consume and commit them under
  `docs/v4/` in extended as trackers: (a) the per-PA test-file diff above
  with every modified source file listed; (b) the course and regression
  catalog, one row per test unit with its bucket; (c) the README and
  Makefile diff table; (d) the scripts and dev inventory.  A small
  `scripts/v4_inventory.sh` produces them, so they can be regenerated as
  phases land.

### Phase 1: Branch and clear the decks

- Create `v4` from `main`.  Decide the one `fast-tests` commit
  (Decision 3) before anything else touches the harness.
- Delete the plan files that describe the old implementation: root
  `plan.md`, `PA23_FEATURE_BACKFILL_PLAN.md`,
  `pa23_feature_backfill_tracker.tsv`; `docs/*` plans and trackers about
  the old compiler's internals; `docs/implemented/` (87 records) and
  `legacy/` (49 files).  Keep, because they describe the assignments and
  process that survive: `ROADMAP.md`, `docs/student-export-root/`,
  `docs/assignment-numbering-migration-2026-08.md`,
  `docs/assignment-restructure-plan.md` (the deferred last step),
  `docs/implemented/v3/pa10-37-readme-handout-audit.md`,
  `docs/implemented/v3/pa10-34-assignment-cleanup-process.md`, the placement decision
  records (`pa14-pa16`, `pa17-pa19`, `pa20-pa22`), the PA15-PA23
  contract-test audit plan and tracker, `performance-regression-validation.md`.
  Each kept file gets one pass to remove references to the old layout.
- Inventory `analysis/`, `benchmarks/`, `bootstrap-selfhost-reducers/`,
  `template-kernel/`, `validation/`: keep what a surviving lane or script
  still reads, delete the rest.  The witness purge goes here: every
  `.ref.witness`, `validation/templates`, the witness scripts and the
  strict targets in the root and PA19-PA24/PA27 Makefiles, the
  `test-strict` CI job, and every README sentence that names the lane.
- Rewrite `AGENTS.md` for the new `dev/src` layout and lanes.
- Our own `PLAN-*.md` files: the completed ones are archived once under
  `docs/implemented/v3/` (their outcomes are the design record of the new
  implementation); the open ones (`PLAN-CODEGEN-AND-SELFHOST-OPTIMIZATION`,
  `PLAN-CLANG-LIBCXX-SUPPORT`, `PLAN-EARLY-SEMANTIC-CORE-UNIFICATION`,
  `PLAN-O1-PARITY`) move to `docs/` and into the implementation tracker.
  `doc/backend-review/` moves with them.

### Phase 2: Move the implementation and the build

- Replace `dev/src` wholesale with ours; bring `dev/Makefile`,
  `dev/frontend_source_sets.mk`, `dev/gen_builtin_host_config.pl`,
  `dev/backend_variant.h` and friends, and the scaffolds
  (`dev/*-scaffold.cpp` in extended are what the export installs as the
  student `dev/<tool>.cpp`; regenerate them from ours).
- Root `Makefile`: keep extended's `test-strict`, `ref-test-*`,
  `test-report` family and the CI knobs; add the cells, variants,
  telemetry, audit and reference-binary targets.  `docker/`: carry our
  `clang-libcxx` image beside `clang22-libcxx` or replace it.
- `scripts/`: our versions of the eight shared harness scripts, minus the
  export-only patches (portability rewrites the export applies at copy
  time must not be baked into the maintainer copies), plus the new tools.
  Extended's maintainer scripts that read the old `dev/src` layout
  (`audit_*` for callsemantic, visibility, template boundaries) are
  deleted or re-pointed in Phase 1's inventory.
- Per-PA Makefiles: ours, with the course-root logic removed in Phase 4.
- Gate: `make` in all four CI flavors (use the docker images locally),
  `make test-report`, `make test-debuginfo` after regenerating its refs,
  `make -C pa39 test-through-pa10`, `make inception`, and every
  `audit-*` target in the root Makefile plus
  `perl scripts/cppgm_file_audit.pl --paths dev` (the file audit is not
  wired to a target; Phase 5 adds one and puts it in CI).

### Phase 3: Migrate the in-place test changes

- Port the source-side changes: the 22 `.t`, 8 `.h` and 45 multi-unit
  inputs modified here, and the 95 new `.t` under `paN/tests`, with their
  sidecars.  The 10 PA13 `spec/200-bad-*` fixtures that exist only there
  are reviewed: keep them if our compiler rejects them the same way, else
  record why they went.
- Regenerate every reference from the pinned implementation
  (`make ref-test`, `make ref-test-debuginfo`), then diff twice: against
  this tree's refs (must be identical, or canonically equal under
  `pa13/lowir.md`'s absorbs list) and against extended's old refs.  Every
  old-versus-new delta is classified once in `docs/v4/ref-deltas.md` as
  (a) a LowIR contract change with the section of `pa13/lowir.md` that
  states it, (b) a compiler fix with the fixture that shows it, or (c) a
  presentation difference the comparison absorbs.  A delta that fits no
  class is a bug to fix before the phase closes.
- The witness refs are already gone (Phase 1); the export's
  regenerated-reference check drops its `.ref.witness` case with them.

### Phase 4: Move the course and regression tests into the suites

- Mapping: `cppgm.tests/course/paN/<bucket>/x.*` moves to
  `paN/tests/<bucket>/x.*`; a flat course lane moves into the PA's
  default lane (`tests/` or `tests/general/` as the PA uses).  PA1-PA9
  move too, into their flat `tests/`.
- Numbering: every moved test gets the cluster the placement audit
  assigns for its primary feature (`(feature, earliest PA, first
  cluster)` table) and a slug that does not collide; where a course test
  duplicates a suite test, the better one stays and the tracker records
  the drop.  Tests the audit moves to an earlier PA move there.
- The regression lane (Decision 2) becomes `paN/tests/regression/` for
  PA29, PA37 and PA38, still run by `make test-regression` after the
  suite, still outside the contract.
- Remove `cppgm.tests/course`, the `paN/course` symlinks, the
  `COURSE_TEST_ROOT`/`COURSE_CONTROL_TEST_ROOT` Makefile logic, and
  `cppgm.tests/README.md`; re-point `doc/lowir-contract-ledger.tsv` and
  every README Testing section that names a course path.
- Gate: `make test-report` green with the same pass counts as before the
  move (5,981 here plus whatever the PA1-PA9 moves add), `test-cells`
  green, `make -C pa16 test-seams` green.

### Phase 5: Extend the test auditor

- Teach `audit_pa_feature_placement.py` the fixture shapes it has not
  seen: `.ref.expect`, `.ref.ir`, `.ref.mir`, `.ref.cmir`, `.ref.inspect`,
  behaviour and driver buckets, controls, regression lanes; the cluster
  hygiene rule extends from PA10+ to PA1-PA9 and to the backend and
  optimizer assignments; course-lane options (`--include-course`,
  `--no-course`) go away.
- Add feature rows for what this implementation added to the contracts
  (the LowIR metadata and conventions in `pa13/lowir.md`, the optimizer
  capabilities PA37's README requires, the PA38 planning seam) so a test
  placed before its owner is flagged.
- Gate: the CI audit job green with `--fail-on-early` over the moved
  suites; `scripts/tests/*.py` wired into a `make test-harness` target.

### Phase 6: README reconciliation

- For each README that differs (table above, largest first: PA37, PA38,
  PA29, PA13, PA34, PA17, PA15, PA16): a row in `docs/v4/readme-review.md`
  naming what changed, whether the handout is clearer for a student, and
  what was cut or restored.  Apply the handout audit's forward-reference
  rule and check Assignment Boundary and Out Of Scope against the tests
  now in the suite (Phase 4 moves tests; boundaries follow).
- `ROADMAP.md`, `PROJECT_LAYOUT.md`, `TESTING_AND_REFERENCES.md` and the
  export root docs updated for the lanes that exist now.

### Phase 7: Export and CI

- `scripts/export_student_repo.sh`: the copied-path lists (new shared
  scripts, `dev` support headers for the new layout, scaffolds), the
  `.py` exclusion (allow the named student tools or port them to Perl),
  the generated `dev/Makefile` and source-set heredocs, the reference
  binary build, and the regenerated-reference drift check.  Update
  `scripts/tests/test_exported_dev_makefile.py` to match.
- Run the export locally from `v4`, build the exported tree in the gcc
  and clang/libc++ flavors, run its `make test-report` and `make test`,
  download-free (bundle produced locally).
- Push `v4`, open the PR, and drive every job green.  Merge; let
  `export-assignments.yml` publish; verify the published
  `cppgm-assignments` builds and passes from a clean clone.

### Phase 7b: pre-combination audits

Four audits that must close before the assignments are combined (added
2026-09-06, after the CI drive):

1. **Student scaffolding headers.**  Audit every scaffolding header the
   handouts give students (the `dev/src` support headers named in each
   `paN/README.md` starter kit, the LowIR and machine-IR model/serialization
   headers, the harness-facing interfaces).  Each header must be one our
   implementation actually consumes on its real path; a header we ship but
   do not use is not a valid contract and is either retired or the
   implementation is rewired to use it.  Expect light rewriting of the
   LowIR/MIR serialization so the student-facing readers and writers are
   the ones the compiler itself uses, and update any header whose shape
   drifted from the implementation.
2. **Assignment export.**  Re-run `scripts/export_student_repo.sh` from
   `v4` and confirm the exported tree still builds and passes its suites in
   the gcc and clang/libc++ flavors from a clean clone, with the reference
   bundle -- the Phase 7 check, repeated after every change to the shared
   scripts, scaffolds, or copied-path lists.
3. **Diagnostics.**  Audit every error message the compiler can emit
   (`ThrowSemanticError`, `ThrowSource`, the lexical, preprocessing,
   parsing, lowering, and native diagnostics): each must say what is wrong
   in terms a student recognizes and carry source-location information
   (file, line, column; the instantiation or inlining context where that
   is what locates the problem).  Messages that name only internal
   identifiers, or none at all, are rewritten; the audit records the
   inventory so it can be re-run.
4. **Tests from `~/work/fable`.**  That tree carries tests this one does
   not (about 4.7k `.t` files against 6k here, with per-assignment
   differences in both directions, e.g. pa22 400 vs 343, pa28 183 vs 45).
   Pull the new ones in through a full audit: place each in the assignment
   and cluster the placement auditor accepts (`--fail-on-early` stays
   clean), update the source and expected output for our compiler changes
   (reference movement classified as in `docs/v4/ref-deltas.md`), confirm
   our compiler passes each, and drop any that duplicate an existing test
   (same shape and oracle) rather than adding a second copy.

### Phase 8 (deferred): assignment combination

`docs/assignment-restructure-plan.md` combines the 38 accreted lessons into
a 39-lesson arc: fold `cpplink`, refocus the exceptions-metadata lesson onto
`.gcc_except_table` emission, make the LowIR band contiguous, split
templating into five lessons with an integration lesson, and re-axis the
hosted area onto what a test can actually verify.  It runs in two stages --
**Stage A rewrites content in place at today's numbers, Stage B renumbers
once when the content is settled** -- so every content review happens in a
tree whose paths are still, and the rename is a single mechanical diff.  Not
started until Phases 0 to 7b are closed and `main` carries the new
reference.

## Decisions

1. **The strict witness lane: purged** (decided 2026-09-06).  The compiler
   no longer has `--witness`; the lane, its refs and tooling leave `v4`
   in Phase 1.
2. **Regression lane home.**  Recommendation: `paN/tests/regression/`,
   one test root per assignment; the alternative is keeping
   `cppgm.tests/regression` as the only shared lane.
3. **The `fast-tests` commit** (`8e0bb9169`: batched test runner and
   workers, 11 files).  Take it into `v4` before Phase 2 or leave it on its
   branch; it touches the same harness files we replace.
4. **PA39 in CI.**  This tree treats PA39 as experimental in `make test`;
   extended's CI runs the self-host ladder and inception.  Confirm our
   ladder passes in all four flavors before Phase 2 closes, or scope the
   job.

## Exit criteria

- `v4` CI green: placement audit, four builds, `test-report`,
  `test-debuginfo`, `test-through-pa10` in every flavor, inception compare,
  and the audits (`make audit-*`, the file audit) as a CI job.
- The export produced from `v4` builds and passes its suites in the gcc
  and clang/libc++ flavors from a clean clone, with the reference bundle,
  and `export-assignments.yml` validates that export on every pull request
  rather than only after a merge to `main`.
- No `cppgm.tests/course`; every test under `paN/tests` with a valid
  cluster; the auditor clean; the seams lane green.
- Every reference delta against the old reference classified; every
  README difference reviewed; the trackers under `docs/v4/` complete.
- Phase 7b closed: the scaffolding headers are the ones the implementation
  uses, the export is re-verified, every diagnostic is useful and located,
  and the `~/work/fable` tests are merged through the audit.

## Risks

- The regenerated references differ from the old ones in ways the
  classification cannot explain: that is a compiler defect surfacing, and
  fixing it is in scope; reshaping source or hand-editing refs is not.
- Moving 700 course units renumbers paths that READMEs, ledgers and the
  regression lane name; the inventory and a path-check script keep them
  aligned, and the move is one commit per assignment so it can be redone.
- The export's portability rewrites and its `.py` exclusion were written
  for the old scripts; a student tree that cannot run `make test-seams` or
  `expect_ir.pl` fails silently until Phase 7's clean-clone run.
- CI flavors on ubuntu 26.04 have not been run here; the docker images
  cover 24.04 only.  Build them first.
