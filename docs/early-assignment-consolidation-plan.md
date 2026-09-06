# Early assignment consolidation plan (PA1–PA9)

Status: proposed.  Date: 2026-09-06.

The early assignments were designed as a ladder of standalone tools.  Some of
those tools turned out to be work a student does once and the compiler then
throws away: their implementation is linked by exactly one assignment and by
nothing in `cppgm++`.  This plan consolidates on that criterion — **how much
of what the assignment makes the student build does the production compiler
actually use** — and leaves the rest of the arc alone.

It is separate from `docs/assignment-restructure-plan.md`, which is the
historical decision record for the middle and late assignments (PA18–22,
PA24–25, PA34–35).  That plan is not changed by this one.

## The measurement

For each early assignment: the tool it delivers, its test inputs, the
production sources its tool links that no earlier tool linked, and how much
of that the production compiler (`cppgm++`, 232 sources) links too.  Source
sets come from `dev/frontend_source_sets.mk`; line counts are the `.cpp`
files in `dev/src`.

| PA | tool | test inputs | new production code | reused by `cppgm++` |
| --- | --- | ---: | ---: | --- |
| PA1 | `pptoken` | 54 | 1,592 | **all of it** |
| PA2 | `posttoken` | 26 | 1,825 | **all of it** |
| PA3 | `ctrlexpr` | 20 | 1,357 | **all of it** |
| PA4 | `macro` | 75 | 4,193 | **all of it** |
| PA5 | `preproc` | 72 | **47** | wiring only |
| PA6 | `recog` | 48 | 1,122 | **none** |
| PA7 | `nsdecl` | 43 | 2,500 | **none** |
| PA8 | `nsinit` | 67 | 3,587 | **none** |
| PA9 | `cy86` | 20 | 2,176 | **none** |

Each assignment is also one self-host ladder checkpoint, in the same order.

Two facts stand out.

**PA1–PA4 are the load-bearing quarter.**  Every line a student writes for
them is preprocessing machinery `cppgm++` links and depends on for the rest
of the course.  Nothing here is a candidate.

**PA5 asks for 47 new lines.**  Full preprocessing is the phase-1-through-4
pipeline the student already built in PA4, wired to a driver
(`preprocess/tool_support`).  It is a milestone, not a body of work.

**PA6, PA7 and PA8 build 7,209 lines the compiler never links.**  `recog`
has its own recognizer, while `cppgm++` parses with `syntax/parser` (2,952
lines) — a second, unrelated implementation.  `nsdecl` and `nsinit` carry
their own namespace, type and lookup model, which PA11 then builds properly
and which the compiler uses instead; `docs/PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md`
exists precisely because that duplication is a maintenance problem.  Each of
these implementations is included by exactly one driver and by nothing else
in the tree.

**PA9 is throwaway by the same measure but earns its place.**  Its 2,176
lines are not linked by `cppgm++` either, but `lowir2cy86` → `cy86` is the
execution oracle PA13 checks LowIR against, so the course needs it standing.

## Proposal

Nine early assignments become five.

1. **PA1, PA2, PA3 unchanged.**  Each builds machinery the compiler uses,
   with nothing thrown away.
2. **Merge PA4, PA5 and PA6 into one preprocessing-and-recognition
   assignment.**  PA5 contributes 47 lines of wiring and belongs with the
   macro processor that does the work; PA6 completes the same pipeline by
   recognizing what comes out of it.  The merged assignment delivers the
   full preprocessor and the recognizer, and the self-host ladder keeps the
   `recog` stage it stages through today.
3. **Drop PA7 and PA8.**  They ask for 6,087 lines of namespace, type and
   lookup model that the compiler discards, on the way to PA11 building the
   same language rules for real.  Their 110 test inputs are the valuable
   part and move to PA11, where the production model has to satisfy them.
4. **Keep PA9** as PA13's execution oracle.

## What this removes

- Four assignment slots.
- 6,087 lines of student implementation that no later assignment builds on.
- The `nsdecl` and `nsinit` tools, their source sets, and
  `dev/src/namespace_semantics/` and `dev/src/namespace_initialization/`
  from the tree — which also removes the duplication
  `docs/PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md` was written to unify, so
  that plan can be closed rather than implemented.  This is the part to
  decide deliberately: the same code is two self-host ladder checkpoints
  (see below).

## What it must not remove

- **Coverage.**  PA7 and PA8's 110 test inputs assert real language rules
  (namespace declaration and reopening, aliases, using declarations and
  directives, linkage, initialization order).  They move to PA11 and must
  pass against the production model before PA7 and PA8 are deleted, not
  after.
- **PA9's oracle role.**  PA13 compares LowIR execution against `cy86`.

## The ladder mirrors the assignments

`pa39` maps checkpoints to assignments one to one
(`INCEPTION_PRIMARY_STAGE_*`) and chains them linearly
(`INCEPTION_PREV_*`), so the ladder already is a mirror of the assignment
list.  It follows the structure rather than constraining it: when an
assignment goes, its checkpoint goes with it, and the chain closes over the
gap.  That settles what to do with `nsdecl` and `nsinit` — they are dropped
with PA7 and PA8, not kept as orphan fixtures, and `recog` chains straight
to `cy86`.

The same rule applies to the merge: one assignment, one checkpoint.  Whether
`macro` and `preproc` survive as intermediate binaries inside the merged
assignment is a sub-decision of the merge, but the ladder carries one
checkpoint for it either way.

After consolidation the chain is `pptoken → posttoken → ctrlexpr →
(merged preprocessing and recognition) → cy86 → cppgm++ → …`, which in the
new numbering makes **`test-through-pa6` the gold standard**: it is the
target that proves `cppgm++` itself compiles, the property CI actually
cares about, and it is today's `test-through-pa10` under a different name.
Extending to **`test-through-pa9`** — today's `test-through-pa13` — is
optional but generally worth having, because it additionally proves
`lowir2cy86`, the tool PA13 checks LowIR execution against.

What the drop costs, stated plainly: two self-compilation data points of
2,500 and 3,587 lines.  Neither feeds a later stage, and the gold-standard
gate is unaffected.

## Strategy: content first, numbering last

Same discipline as the later restructure: a lesson's content and a lesson's
number are separate changes, and doing them together puts every content
review in a tree whose paths are moving.

**Stage A, in place, at today's numbers.**

- A1: move PA7 and PA8's tests to PA11 and make them pass against the
  production model.  Nothing is deleted yet; PA7 and PA8 keep running.
- A2: retire the PA7 and PA8 contracts, delete the two tools, their source
  sets and their implementations, and close
  `docs/PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md` as overtaken.
- A3: fold PA5's contract into PA4's handout and PA6's into the same
  assignment, leaving all three directories where they are until Stage B.
  The merged contract is written here.

**Stage B, one mechanical pass.**  Renumber, move the directories, rewrite
cross-references, handouts and `ROADMAP.md`, retarget the ladder's
checkpoint-to-assignment map and CI's `test-through-*` target, and re-run the
export.  A test that moves during Stage B is a Stage A escape and is fixed in
Stage A.

Stage A already removes the two dropped checkpoints from `CHECKPOINTS` and
closes the chain (`INCEPTION_PREV_cy86 = recog`), because that has to happen
when the tools are deleted; Stage B only renames what is left.

## Gates

Unchanged from `docs/PLAN-CPPGM-EXTENDED-V4.md`: byte-exact
`CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report-nobuild`,
`make test-debuginfo-nobuild`, `make inception`, `make test-variants`,
`make -C pa38 test-perf`, `make -C pa39 test-through-pa10` in all four CI
flavors, every `make audit-*`, the file audit, `make test-harness`,
`python3 scripts/audit_pa_feature_placement.py --fail-on-early`, and
`scripts/export_student_repo.sh`.

The ladder gate is the one to watch, and it moves with the structure: the
gold standard stays "the compiler compiles itself" (`test-through-pa10`
today, `test-through-pa6` after renumbering), with the optional extension
through `lowir2cy86`.

## Open questions

- Does the merged PA4–PA6 stay one assignment, or does the recognizer keep
  its own milestone with the merge limited to PA4 and PA5?  The data says
  PA5 has no body of work; PA6 has 1,122 lines, small but real.
- Do PA7 and PA8's tests all belong in PA11, or do the linkage and
  initialization-order tests want PA12 instead?
- Is any part of `nsdecl`/`nsinit` worth keeping as a reference
  implementation for the PA11 handout, or does deleting it entirely leave
  the right amount of room for the student?
- Does the merged assignment still deliver `macro` and `preproc` as
  intermediate binaries, or only the recognizer?
