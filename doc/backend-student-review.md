# The backend from the student's seat: audit

Phase 1 of `PLAN-BACKEND-STUDENT-REVIEW.md`.  Tree `994a739a`, 2026-09-05.
Every number below was produced by a script or a suite run on that tree;
the scripts and logs are named where they matter.

## 1. What each assignment judges, per lane

| assignment | lane | fixtures | oracle |
|---|---|---|---|
| PA29 | `tests/strict` | 36 | machine-IR dump, byte equality after whitespace normalization |
| PA29 | `tests/structural` | 60 | machine-IR dump after canonicalization (free GPR and XMM renaming, memory displacements) |
| PA29 | `tests/behavior` | 88 | program stdout and exit status; the dump is kept but not compared |
| PA29 | `course/pa29/strict`, `structural`, `behavior` | 4, 25, 78 | as above |
| PA29 | `course/pa29/controls` | 19 | contract predicates (`check_pa29_native_contracts.pl`) |
| PA37 | `tests/o0` to `o3`, `course/pa37/o0` to `o3` | 3+1, 71+52, 13+23, 9 | `compare_lowir_text`: the optimized LowIR after presentation canonicalization (local names, internal symbol pairing, metadata, top-level order); every instruction, block and decision must match |
| PA37 | driver lanes (`--emit-lowir -O*`) | 3+11, 6, 1 | the same canonical comparison of the emitted LowIR |
| PA37 | `tests/object-roundtrip` | 8+17 | direct object against LowIR-replayed object, plus a section reducer |
| PA37 | `course/pa37/controls` | 27 fixtures | 29 pass-specific checker scripts plus inline-limit and survivor-property buckets |
| PA38 | `tests/o1`, `o2`, `o3` and course | 11+8, 13+3, 1+6 | structural machine-IR oracle (`.ref.cmir`) with the raw dump alongside |
| PA38 | behaviour lanes and course | 3+5, 1 | program behaviour; the course behaviour lanes also fail on a raw-dump mismatch (see 5.2) |
| PA38 | `course/pa38/controls` | 20 | 5 native checker scripts asserting placement decisions |
| PA30 to PA36 | all | 100, 32, 172, 109, 154, 87 sources | program behaviour, tool exit status, `.inspect` object questions |
| PA13 | all | | CY86 text and behaviour through the CY86 machine |
| PA39 | inception | | staged self-build reproduces (MATCH) |

Totals for the three code-producing assignments: PA29 judges 125 of 291
fixtures by machine-IR shape (43%); PA37 judges 201 of 253 sources by canonical
text (79%) and the remaining ones by controls or objects; PA38 judges 42 of
51 lane fixtures by machine-IR shape (82%) and all 20 controls by placement
decision.

## 2. What the READMEs require: contract, quality bar, or mechanism

A first-cut classifier (`classify.py`, kept with the audit logs) splits each
README into sections, drops the ones labelled non-normative (Overview,
Starter Kit, Testing, Design Notes, Out Of Scope, handoffs), splits the rest
into sentences, and tags a sentence *mechanism* when it names a pass, a
policy noun (budget, cap, pool, planner, interval, worklist, clone, wave) or
a bare number; *quality* when it bounds a quantity; *contract* when it names
a format, ABI, preservation or validity requirement.  The heuristic
over-counts mechanism in PA29 (the LowIR family list trips it) and
under-counts it where a sentence describes a pass without those nouns; the
examples in 2.1 were read by hand.

| assignment | normative sentences | mechanism | contract | quality | unclassified | non-normative sentences |
|---|---|---|---|---|---|---|
| PA13 | 130 | 12 | 52 | 0 | 66 | 67 |
| PA29 | 224 | 37 | 81 | 3 | 103 | 206 |
| PA30 | 38 | 1 | 12 | 1 | 24 | 102 |
| PA31 | 30 | 3 | 14 | 2 | 11 | 90 |
| PA32 | 24 | 3 | 11 | 1 | 9 | 87 |
| PA33 | 19 | 1 | 10 | 1 | 7 | 86 |
| PA35 | 15 | 2 | 7 | 0 | 6 | 45 |
| PA36 | 27 | 5 | 14 | 0 | 8 | 74 |
| PA37 | 292 | 136 | 55 | 6 | 95 | 143 |
| PA38 | 170 | 59 | 44 | 2 | 65 | 100 |
| PA39 | 1 | 1 | 0 | 0 | 0 | 94 |

PA37's "Optimization Levels" section alone is 719 lines and 71 bullets
naming 63 numbers; PA38's is 320 lines and 38 bullets.  The object
assignments (PA30 to PA36) have almost no mechanism sentences and mostly
non-normative text: they describe the contract and let the design go.

### 2.1 Mechanism sentences, read by hand

PA37 (all normative, all canonical-text enforced):

- "conservative inlining of small direct calls, including `unwind=no`
  callees inside EH regions only when the caller EH shape can be preserved"
- "a deterministic 768-instruction whole-caller inlining budget, charged by
  the greater of a callee's original and simplified instruction counts"
- "a trivial-leaf exemption from that budget: a leaf body of at most four
  instructions is substituted even when the caller's growth budget is
  exhausted"
- "a separate definition-removing path for a weak or internal function that
  has exactly one direct call ... may admit a body of at most 512
  instructions, uses a 1,024-instruction budget per caller"
- "safe normalization of commutative integer operations and reversible
  compare directions so equivalent expressions reuse the same producer"

PA38 (all normative, all shape-enforced or control-enforced):

- "coalesce block-local integer and floating-point register copies"
- "rematerialize cheap integer immediates into supported arithmetic,
  zero-compare, and call-argument instruction forms"
- "let a single-use scalar value from an acyclic merge `phi` share its frame
  home with a later acyclic merge `phi` that consumes it"
- "keep a frequently reused, iteration-local scalar call result available
  to later iterations across an intervening call"
- "after complete MIR liveness is available, recolor a callee-saved physical
  register"

PA29 (normative, shape-enforced in the strict and structural lanes):

- "When that value fits the target comparison's immediate field, encode the
  comparison directly instead of first copying the value to a scratch
  register"
- "Small leaf scalar expressions should normally stay in registers, and
  ordinary `f32` / `f64` operations should stay on the floating-register
  path"
- "Lowering operations with fixed scratch registers, including integer
  comparisons, division, and shifts, must preserve still-live frame
  addresses"

The first PA29 sentence is a contract for a strict fixture and a mechanism
for a student with a different instruction selector; the second is a
quality bar written as a mechanism; the third is a correctness contract.
The three kinds are interleaved in one list with one verb, "must".

## 3. Design-freedom probes

For each component, a design a student could reasonably choose, and the
lanes it would fail on this tree.  Where a perturbation in section 4
measured the same axis, the measured number is given.

| component | alternative design | fails |
|---|---|---|
| PA38 allocation | linear-scan with spilling, no planner | every structural lane (42) and every control that names a planner decision (about 15 of 20); behaviour lanes pass |
| PA38 allocation | graph colouring | same as above |
| PA29 frame | frame pointer always, or any other slot layout | measured: a uniform 16-byte shift fails 48 PA29 and 18 PA38 fixtures (4.4) |
| PA29 selection | `lea` for pointer addition where we emit `add` | structural lane, whose canonicalization preserves opcode family by design |
| PA29 selection | `cmov` for empty diamonds | PA38 structural lanes wherever a diamond exists; PA29 unaffected at -O0 |
| PA29 scratch | `r10` as the encoder scratch instead of `r11` | strict lane; structural lane passes because both are in the renamed free-GPR class |
| PA29 phi | sequential transfers through one temporary instead of parallel copies | strict and structural lanes on every phi fixture |
| PA37 midend | classic SSA GVN and SCCP instead of our rule set | the canonical-text lanes wherever the instruction sequence, a reused expression or a block differs (names are canonicalized, nothing else is): nearly all 201 |
| PA37 inlining | per-call cost model, no caller budget | every fixture whose inlining decisions differ, and the inline-limit controls |
| PA37 layout | reverse-postorder block order everywhere | measured: 25 PA37 fixtures (4.3) |

The behaviour lanes and the object assignments admit all of these designs.

## 4. Mutation audit

Four correctness-preserving perturbations of our own backend, each built
and run through the PA29, PA37 and PA38 suites with `KEEP_GOING=1` and then
reverted (`mutate.py`; per-lane pass counts in `mutations.txt`).  Baseline:
every lane green.

| perturbation | what changed | PA29 fixtures failed | PA37 fixtures failed | PA38 fixtures failed |
|---|---|---|---|---|
| M1 register pool order | the reactive and planner callee-saved pools reversed | 0 of 291 | 0 of 218 | 7 of 71: 3 structural, 3 raw-dump in course behaviour lanes, 1 control |
| M2 inline size cap 40 to 41 | one policy number | 0 | 0 | 1 control (459, an O3 pipeline fixture) |
| M3 reverse-postorder block order always | block layout only; cold sinking undone | 0 | 25 of 218: `tests/o1` 8, course o1 10, course o2 5, course driver o1 1; control 390 stops the control bucket | 1 control (459) |
| M4 frame padding of 16 bytes | every frame displacement shifts; alignment kept | 48 of 291: 14 strict, 34 structural (22 of 60 and 12 of 25) | 0 | 18 of 71: 14 structural, 3 raw-dump, 1 control |

What the four say:

- The register pool order is genuinely free in PA29: the structural
  canonicalization does what its README promises for register choice.  In
  PA38 it is not free, because the planner's choice of which value gets a
  register changes with the order and the course behaviour lanes compare
  the raw dump.
- One policy number in PA37 (the cap) moved nothing in PA37's own
  references and one O3 control elsewhere.  The 63 numbers are not all
  binding on the checked-in fixtures; they are binding on the student's
  *outputs* for any program that reaches them, which the self-build does.
- Block order is not free in PA37: 25 canonical-text fixtures and one control
  encode the layout our cold-sinking pass produces.  A student whose
  optimizer emits reverse postorder, which the native lowering accepts,
  fails them.
- Frame layout is not free anywhere the shape is compared, including the
  structural lanes whose canonicalization was meant to hide displacements:
  the prologue's frame size and the `frame` section lines are not
  canonicalized, so a uniform shift fails 40% of the structural fixtures.

## 5. Contradiction audit: promises the harness does not keep

1. **PA37 "Validation Modes"** says "Exact textual LowIR matching is not a
   PA37 grading requirement unless a test explicitly says so", and the
   harness keeps that letter: PA37 runs in `lowir_t` mode, whose
   `compare_lowir_text` validates both outputs and compares them after
   canonicalizing local value and block names, internal symbol names by
   position, metadata groups and top-level order.  What it does not relax
   is anything a reader of that sentence would expect: the instruction
   sequence, the block structure, and every inlining, promotion and
   forwarding decision must match ours.  M3 (block order alone) fails 25
   fixtures.  The promise is kept for presentation and broken for shape.
2. **PA38 "Testing"** says the behaviour lanes "do not compare a machine-IR
   oracle".  `pa38/scripts/compare_results.pl` maps every root whose path
   contains `behavior` to the `mir_t` mode, whose strict dump comparison
   runs whenever `x.ref.mir` exists, and the course behaviour fixtures all
   carry one.  Under M1 and M4, fixtures in `course/pa38/behavior/o1` and
   `o2` failed with "machine IR dumps do not match (.mir)" while their
   programs behaved identically.
3. **PA29 "Testing"** says the structural canonicalization "hides exact
   stack/frame displacement numbers in memory operands".  It renames the
   operands' displacements but not the frame size in the prologue nor the
   `frame` section, so M4 fails 34 of 85 structural fixtures.
4. **PA29 "Testing"** says "the exact instruction inventory is target- and
   lowering-dependent".  125 fixtures compare it.
5. **PA38 "Validation Modes"** says exact textual matching "is not a PA38
   grading requirement unless a test explicitly makes that shape part of the
   oracle".  Every fixture in `tests/o1` and `tests/o2` carries both a raw
   and a canonical dump, and nothing in the fixture says so.

## 6. Effort audit

Lines of our implementation, which is the upper bound a student matching our
shape would have to approach, against the lines of the harness that
describes the shape:

| component | lines |
|---|---|
| `dev/src/native/lowering` (PA29 baseline lowering) | 8,575 |
| `dev/src/native/allocation` (PA38 planner and forwarding) | 3,157 |
| `dev/src/native/frame`, `analysis`, `eh`, `encoding`, `driver` | 504, 1,602, 1,099, 2,874, 1,380 |
| `dev/src/native/mir`, `object` (support code in the starter kit) | 4,072, 6,865 |
| `dev/src/lowir/optimize` (PA37) | 25,302 |
| `dev/src/lowir/analysis`, `model`, `io` (support) | 2,030, 2,348, 4,272 |
| harness (`run_all_tests_common.pl`, `compare_results_common.pl`) | 4,754 |
| checker scripts (`scripts/check_*.pl`, 40 files) | 9,412 |

The checker scripts are a third of the optimizer's size.  They exist to
describe our passes' outputs to the harness; a student writing a different
optimizer gets nothing from them but failures, and a student writing ours
gets a specification in Perl.

A contract-level PA29 backend (lowering, frame, encoding, objects, without
the planner) is on the order of 15,000 lines in our tree; a contract-level
PA37 optimizer that reaches the quality bar with a textbook design (SCCP,
GVN, a cost-model inliner, LICM, SSA construction) would be a fraction of
25,000 lines, because most of ours is the specific rule set the references
encode.

## 7. Decision

The numbers support the plan's proposal without amendment:

- The contract-shape set is small.  Of PA29's 125 shape fixtures, 7 have
  an ABI, stack-argument, callee-save or alignment subject by name; the rest
  test lowering choices that the behaviour lanes already cover.  PA38's
  contract-shape set is the unwind and debug-metadata fixtures.
- The structural canonicalization is worth keeping for the contract-shape
  set and must learn the frame size and the `frame` section before it can
  keep its README's promise.
- PA37 needs the behaviour floor most: its fixtures are hand-written
  LowIR with no program to run, so nothing today checks that an
  optimization preserved behaviour except the object-roundtrip lane.
- The split into a course suite and a regression lane is the precondition
  for everything else; the canonical references are our memory and stay.

Phases 2 to 5 proceed as written.  Phase 6 (a second implementation) is
budgeted separately.

## 8. What the plan's phases changed, and the audit rerun

Phases 2 to 5 landed in the four commits after this audit
(`8b5f15fe`, `afb1206b`, `8775c4c8` and the correction `440ede04`).  The
mutation audit was rerun on the result with the course and regression
lanes counted apart.

| perturbation | course fixtures failed before | course fixtures failed after | regression fixtures failed after |
|---|---|---|---|
| M1 register pool order | 7 | 0 | 7 |
| M3 reverse-postorder block order | 25 | 0 | 26 |
| M4 frame padding of 16 bytes | 66 | 0 | 18 |

The one course failure that survived the migration, an exception-placement
fixture in PA38 under M1, went away when its structural oracle was
replaced by contract lines (exception regions, calls, callee-saved
preservation, memory accesses, generated by `scripts/make_ir_contract.pl`);
the four PA29 contract-shape fixtures that failed M4 went away when the
structural canonicalization learned the frame's total size, which made the
PA29 README's promise (5.3) true.  `make test-variants` runs the course
suites under the three variants of `dev/src/backend_variant.h`, which are
the perturbations M1, M3 and M4 made selectable, and passes in PA29, PA37
and PA38.

What each assignment's course suite judges by now:

- PA37: exit status, structural validity, the lowering floor (every
  optimizer output must lower through `cppgm++ -c -O0` when its input
  does), and an `x.ref.expect` on every fixture: a size budget generated
  from the course solution's output at 10% plus one, and outcome lines on
  the fixtures that have them.  The canonical-text references and the 27
  controls moved to the regression lane with the survivor-property
  checks.
- PA29: behaviour everywhere; an `x.ref.expect` budget on every strict and
  structural fixture except the seven whose shape is the ABI (object ABI,
  stack arguments beyond six, the mixed GPR and XMM call ABIs, call setup
  without preserve, pointer alignment, callee-saved retention), which keep
  the canonical dump; the contract properties stay.
- PA38: behaviour everywhere; an `x.ref.expect` budget or contract lines
  on every machine-IR fixture; `make test-perf` holds the behaviour
  programs within 10% of the course solution's Cachegrind count.  The 20
  placement controls moved to the regression lane.

Two later commits (`a9b5bf5d`, `cd098654`) added the driver-lane
behaviour floor (a source fixture's emitted LowIR is built into a program
and run against the source built at the lane's level), derived outcome
lines for 78 PA37 course fixtures (`scripts/make_ir_outcome.pl`: what the
course solution removed stays removed), and the top-level
`make test-variants`.  The behaviour floor found a compiler defect on its
first run: the object path rejected `return f64 inf`, which the driver
emits for `__builtin_inf`, because the literal suffix stripper took the
`f` of `inf` for a float suffix.

What is not done:

- Outcome lines are derived (calls, loads, stores, phis and definitions
  removed) or hand-written on 86 course fixtures; the fixtures whose
  subject is a reordering or a rewrite rather than a removal (hoisting,
  forwarding, threading) have budgets only, and their outcome lines are
  still to be written from their names and references.
- PA37's handwritten LowIR fixtures have no program to run; the lowering
  floor is their behaviour check.
- The One Design sections keep the old bullets' wording, including "must",
  under a preamble that makes them non-normative; the bullets themselves
  have not been rewritten.
- `make test-variants` runs from the top level and per assignment; it is
  not part of `make test-cells`.
- Phase 6, a second implementation against the migrated suite, has not
  started.

## 9. Phase 6: a second implementation

Phase 6 asked for a backend that is not ours passing the course suite,
written by someone who has not read ours, with their notes becoming README
changes.  It was done in two steps.

**A first probe by the same author.**  `CPPGM_BACKEND_VARIANT=colouring`
assigns planned registers by Chaitin-style interference-graph colouring at
the planner's seam (`plan_value_locations`), taking the same candidates,
pools and clobber facts as the course solution's claim-driven linear scan.
It passes the PA29, PA37 and PA38 course suites, stays within the
performance envelope on all 15 behaviour programs, and fails ten of our
pinned regression dumps.  One budget (`426-staged-home-selection`) was
widened to 25% under the rule that the envelope is the loosest limit
admitting the accepted variants.  Its value was to make the seam explicit:
`dev/src/native/allocation/planning_seam.h` now states the facts an
allocator receives and the contract a plan must respect, and the PA38
README's One Design names it.

**The newcomer trial.**  A fresh agent with no access to this work was
given `doc/backend-review/newcomer-brief.md`: the PA38 README, the seam
header, the facts header, the register and model headers, the fixtures and
their sidecars, and the four commands that define success; it was
forbidden the course solution's allocator and lowering.  It wrote
`dev/src/native/allocation/linear_scan.cpp`, a linear-scan allocator of
297 lines, selected by `CPPGM_BACKEND_VARIANT=linear-scan`.  It passes
the PA38 course suite, the performance envelope (15 of 15 within 10%),
and the PA37 and PA29 course suites; on the guarded loop of
`420-loop-and-eh-placement` it keeps three phis in registers where the
course solution keeps them in the frame (15 instructions, one load, no
stores, against 22 with five loads and six stores).  Its notes are
`doc/backend-review/newcomer-notes.md`.

What the notes found, and what changed because of them:

- Two contract sidecars pinned the course solution's placement after all:
  my contract generator had emitted load and store floors and a
  `preserve` ceiling for every kept fixture.  The floors were meant for
  the volatile fixture and the ceiling for the callee-saved-prune
  fixture; on the exception fixtures they forced the newcomer to add a
  rule whose only purpose was to reproduce the reference's frame homes.
  The generator now emits them only on request (`--memory`,
  `--preserve`), the sidecars were regenerated, and the newcomer removed
  its rule and still passes.
- The `preserve` metric was vacuous: the evaluator looked for the line in
  the `abi` section and the dumps print it under `frame`.  Fixed.
- `make test-perf` measured whatever programs the last course run had
  left, so a course run that stopped early gave stale numbers.  It now
  regenerates the behaviour programs first.
- The seam header lacked six facts a newcomer needed: that the layout is
  post-split and what loop-carried means; where a phi's segment begins
  and whether its conflict interval may begin at the transfer; that
  endpoints are inclusive; that `uses` is a count and the position lists
  are sorted; that object-typed values are excluded; where a phi
  operand's use is recorded.  All six are in the header now, with a
  closing paragraph on which values the reactive walk already places
  well, so the experiment the newcomer ran to learn it is not needed
  again.

What it does not cover: the trial replaced the allocator at its seam, not
the whole of PA29 and PA38 (the lowering, encoding and object paths were
the course solution's).  A trial of that size is an assignment, not a
review; the seam trial is the evidence this plan can give that the
migrated suite and the README admit a design that is not ours.

A defect the variants caught on the way: the pool-order variant had
replaced two arrays by pointers and left two `sizeof` element counts
behind, so the reactive pool had held two registers instead of seven and
five since `8775c4c8`.  Correct code, worse allocation; neither the
regression lane's small fixtures nor the self-build noticed.  Fixed with
explicit counts.  The regression lane pins shapes on fixtures too small to
exercise register pressure; the performance envelope on the self-build
would be the check that sees such a change.
