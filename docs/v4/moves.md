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

- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report` (the CI setting) first
  passed 5,930 of 5,933.  The three failures were the special member order
  violations already recorded in `ref-deltas.md`
  (`pa22/tests/general/300-member-template-assignment-not-special-member`,
  `pa28/tests/general/100-diamond-virtual-destructor-slot-merge`,
  `pa28/tests/general/100-multibase-implicit-virtual-destructor-slot-merge`):
  the compiler emitted the move assignment before the copy assignment when
  the move was demanded first, and the complete-entry vtable thunk before
  the deleting-entry thunk, against the order `pa13/lowir.md` states.  A
  presentation pass in the lowering driver now orders each special member
  family (source commit "Open a constructor loop's cleanup segment before
  the loop"); the three references were regenerated and the report passes
  5,934 of 5,934.
- The PA39 selfhost lane failed on
  `dev/src/native/allocation/location_planning.cpp`: an array of more than
  eight objects with destructors, declared while another such object was
  alive, made the emitted construction loop open the enclosing cleanup
  region once per element and close it once.  The same source commit opens
  the segment before the loop; `pa16/tests/general/300-constructor-array-loop-enclosing-cleanup`
  is the new fixture, and no other reference changed.
- The PA38 regression control
  `tests/regression/controls/459-o3-parameter-address-rematerialization`
  reported that native `-O3` did not reduce call-preserved address
  pressure.  It last passed at `f59dcabf` and failed from `7b97d1e9` ("Keep
  a replayed index base live until its last replay"), which deliberately
  keeps only the base of a replayed index address across a call from `-O1`
  on, so the O2 baseline already has the pressure the control expected O3
  to remove.  The control now pins that decision (only the base is
  preserved at either level, no derived address is rebuilt before the
  call, O3 adds no homes or frame) and the PA38 handout paragraph says the
  same.
- `make -C pa16 test-seams` passes (18 rewrites classified).

## Phase 5: placement and numbering

The placement auditor (`scripts/audit_pa_feature_placement.py`) is now the
only judge of where a fixture sits, and it judges the whole tree:

- The course lane is gone from it (`--include-course`/`--no-course`
  removed); every `paN/tests/**/*.t` outside `regression/` and
  `controls/` is a placed fixture.  The regression lane pins the course
  solution's own outputs and the controls are judged by their checkers,
  so neither is a placed suite.
- PA1 to PA9 must carry a three-digit prefix; from PA10 on the prefix is a
  cluster, a multiple of one hundred (the rule that already existed).
- A numbered companion unit (`x.t.1`) of a host-interop lane (PA31 to
  PA34, PA36) is compiled by the host compiler, so its hosted includes are
  not early.

After the move the auditor reported 740 findings (93 fixtures using a
feature before its owning assignment or cluster, 647 hygiene findings).
`scripts/v4_renumber.py` (a scratch tool, not kept) turned the findings
into 620 renames, applied with `git mv` together with every sidecar:

| reason | fixtures |
| --- | ---: |
| individual number to its cluster (`320-x` to `300-x`) | 253 |
| flat course fixture into the suite's `general/` bucket | 297 (overlapping the rows above and below) |
| unprefixed LowIR-input fixture, cluster by the words its numbered siblings use | 120 |
| unprefixed source fixture, cluster of the latest feature its assignment owns | 43 |
| same assignment, later owning cluster | 29 |
| feature owned by a later assignment: moved there | 24 |
| PA25 fixture whose reference carries unwind lowering: moved to PA26 | 1 |

Two PA17 fixtures used `__attribute__((noinline))` only decoratively; the
attribute went instead of the fixtures.  The ledgers
(`doc/lowir-contract-ledger.tsv`, `doc/compiler-refactor-output-cases.tsv`),
the survivor-property checkers, the PA29 and PA31 Makefiles, the PA13
handout and the backend review records name the fixtures by their new
paths.  `make ref-test` and `make ref-test-debuginfo` regenerated the
references that embed a path.

After the renames and three further moves the auditor found nothing
(`--fail-on-early` exits 0), and the byte-exact `make test-report` passes
with the regenerated references.

## The selfhost lane

`make -C pa39 test-through-pa10 CXX=../dev/cppgm++` (a CI gate) failed at
PA9: every test of the self-compiled `cy86` assembler died with "invalid
ELF header size".  Two compiler defects, both older than the move, both
fixed in the source tree and synced here:

- The optimizer forwarded a load through a phi of addresses to the value a
  retyping store had stored (`store i64 %n` with `%n` a pointer
  difference, as `vector::size()` inlined into `_M_check_len` leaves it),
  so its own -O1 output carried an `i64` phi with a `ptr` incoming and the
  LowIR reader rejected it.  `pa37/tests/regression/o1/549-retyping-store-through-address-phi`
  and `pa38/tests/behavior/o1/500-retyping-store-through-address-phi` hold
  the shape.  `make -C pa38 ref-test` now regenerates the behaviour buckets
  it had skipped, and the behaviour lane's `.ref.mir`/`.ref.cmir` dumps are
  ignored like its `.ref.program`.
- The native backend folded a constant index into a local's frame operand
  even when the result reached one past the local's last byte, abstract
  offset zero, which the encoder reads as the caller's frame and does not
  move past the saved registers: the `last` pointer of a local array passed
  to a range insert arrived 8 bytes too high whenever the function preserved
  a register.  `pa29/tests/behavior/200-one-past-local-array-call-argument`
  holds the shape (it fails before the fix at `-O0`).

To reach the defects, `cppgm++ --emit-*` now accepts `-I`, `-D`, `-U` and
`--hosted`, so one translation unit of the compiler itself can be emitted
as LowIR and optimized or lowered on its own; the reader's phi mismatch
error names the value, both types and the function.

One frontend defect the lane exposed is recorded rather than fixed: our
compiler cannot resolve `std::pair::swap` for a pair whose first member is
itself a pair (`std::sort` over
`std::vector<std::pair<std::pair<int, std::size_t>, std::size_t>>` fails
with "no viable overload for swap"; the inner pair's specialization
receives two class-template identities).  The ordering pass was rewritten
around it; the eight-line reducer is
`docs/v4/reducers/nested-pair-sort.cpp`, to become a PA35 compile fixture
once the frontend resolves it.

## Phase 7: the export

`scripts/export_student_repo.sh` copies the student support files at their
paths in this tree (`dev/src/abi/itanium/abi_mangle*.h`,
`dev/src/preprocess/tokens/*PPTokenStream.h`, `dev/src/support/not_implemented.h`,
`dev/src/support/tool_help_text.h`, `dev/src/support/testing/test_runner.cpp`,
`dev/src/ir_symbol_model.h`), writes the student `frontend_source_sets.mk`
with the test runner's source id, ships the two seams scripts PA16's lane
runs, and no longer validates a witness lane.  The scaffolds include those
paths.  A local run exported 18,651 verified reference files (7,059 exit
statuses, 411 retained failed-case diagnostics), packaged the reference
bundle, and the exported `dev/` builds its scaffolds.  CI gained an
`audits` job (the architecture audits, the file audit, `make test-harness`).
