# Phase 4: the course and regression lanes moved into the assignment suites

Applied on 2026-09-06 on branch `v4`, on top of the Phase 3 regeneration.

## What moved

5,282 tracked files left `cppgm.tests/course/paN/` and
`cppgm.tests/regression/paN/` by `git mv`; the `paN/course` symlinks and
the two shared directories are gone.  Destinations, by lane:

| destination | files | destination | files |
| --- | --- | --- | --- |
| pa1/tests | 70 | pa22/tests | 106 |
| pa2/tests | 39 | pa23/tests | 50 |
| pa3/tests | 38 | pa25/tests | 73 |
| pa4/tests | 142 | pa26/tests | 30 |
| pa5/tests | 29 | pa27/tests | 3 |
| pa6/tests | 64 | pa28/tests | 11 |
| pa7/tests | 71 | pa29/tests (strict, structural, behavior) | 591 |
| pa8/tests | 84 | pa29/tests/controls | 19 |
| pa9/tests | 61 | pa29/tests/regression | 707 |
| pa10/tests | 32 | pa30/tests | 66 |
| pa11/tests | 31 | pa31/tests | 73 |
| pa12/tests | 63 | pa32/tests | 105 |
| pa13/tests | 65 | pa32/tests/controls | 3 |
| pa14/tests | 16 | pa33/tests | 15 |
| pa15/tests | 44 | pa34/tests (preproc, compile, run) | 30 |
| pa15/tests/controls | 5 | pa35/tests/compile | 200 |
| pa16/tests | 189 | pa36/tests/link | 5 |
| pa16/tests/controls | 4 | pa37/tests (o0 to o3, driver, object-roundtrip, debuginfo) | 503 |
| pa17/tests | 63 | pa37/tests/regression | 768 |
| pa17/tests/controls | 10 | pa38/tests (o1 to o3, behavior, driver, debuginfo) | 227 |
| pa18/tests | 25 | pa38/tests/regression | 367 |
| pa19/tests | 81 | pa20/tests | 36 |
| pa21/tests | 68 | | |

A course fixture went to the bucket of the same name under `paN/tests/`
(flat suites stay flat).  Controls went to `paN/tests/controls/`; the
regression roots of PA29, PA37 and PA38 went to `paN/tests/regression/`
with their bucket structure intact, and the PA37 and PA38 regression
controls to `paN/tests/regression/controls/`.

Two PA1 fixtures collided with local fixtures of the same stem and were
renamed to say what distinguishes them: `100-raw-string-literal` became
`100-raw-string-literal-nested-delimiter`, and `200-header-name` became
`200-header-name-forms`.

`cppgm.tests/` keeps only `undefined/` (inputs the course leaves
unspecified; no lane runs them) and a README that says so.  The regression
lane's policy moved from `cppgm.tests/regression/README.md` into
`docs/student-export-root/TESTING_AND_REFERENCES.md`.

## Harness and Makefiles

- `scripts/run_all_tests_common.pl` and `scripts/CppgmBatchWorker.pm` prune
  `controls` and `regression` subdirectories from a sweep, so `make test`
  over `tests` reaches a control or a regression fixture only through the
  lane that knows how to judge it.
- Every assignment Makefile lost its `COURSE_*` roots and the
  `ifeq ($(COURSE_TEST_ROOT),)` fork: the suite is the one `tests` tree.
  PA15, PA16, PA17 and PA32 read their controls from `tests/controls`;
  PA29 from `tests/controls` and its regression oracles from
  `tests/regression/{strict,structural,behavior}`; PA37 and PA38 from
  `tests/regression/...` with the controls under
  `tests/regression/controls`.  PA31 re-runs the moved
  `330-host-eh-nested-catch-forward-at-o1` fixture at `-O1` after the
  default sweep, as the course lane did.  PA38's driver census root is
  `tests/driver`.
- `make ref-test` now regenerates the PA37 and PA38 regression roots too
  (PA29 already did), so a maintainer's regeneration leaves a clean tree.
- The `INCEPTION_COURSE_STAGE_paN` variables of PA39 had no reader and are
  gone.

## References

`make ref-test` after the move regenerated every reference.  Differences
against the moved course references are of three kinds: references that
embed the fixture path (`start translation unit tests/...`), the expected
byte-level drift of outputs the relaxed comparison already absorbed (the
course references were produced by an older build), and the
`.ref.stdout` of negative fixtures, which the harness rewrites from the
current diagnostics.  Success-case `.ref.stdout` files the course lane did
not track (73 of them) are tracked now, as the local suites always were.

## Results

- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report` (the CI setting):
  5,930 of 5,933 pass.  The three failures are the special member order
  violations already recorded in `ref-deltas.md`
  (`pa22/tests/general/300-member-template-assignment-not-special-member`,
  `pa28/tests/general/100-diamond-virtual-destructor-slot-merge`,
  `pa28/tests/general/100-multibase-implicit-virtual-destructor-slot-merge`):
  the compiler emits the move assignment before the copy assignment when
  the move is demanded first, and the complete-entry vtable thunk before
  the deleting-entry thunk.  `pa13/lowir.md` states the opposite order.
  One more failure is in the PA38 regression lane:
  `tests/regression/controls/459-o3-parameter-address-rematerialization`
  reports that native `-O3` did not reduce call-preserved address
  pressure.  All four reproduce in the source tree at `36b42987` and at the
  commit before the audit fix; they are compiler defects to fix before
  Phase 7, not artefacts of the move.
- `make -C pa16 test-seams` passes (18 rewrites classified).
- The PA39 selfhost lane fails on
  `dev/src/native/allocation/location_planning.cpp`
  (`assign_candidate_registers_by_colouring`: host EH protected-region
  stack underflow); also a compiler defect that predates the audit fix.
