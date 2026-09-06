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

## What each handout actually teaches

Code reuse says what is safe to remove; the handouts say what is coherent to
combine.  Reading them changes the answer.

- **PA4 `macro`** is one concept: **macro replacement**.  `#define` and
  `#undef` only, object-like and function-like, and the hard part is the
  replacement algorithm — argument substitution, `#` and `##`, rescanning,
  and the no-recursion rule.
- **PA5 `preproc`** is the other half of phase 4: **the directive layer**.
  Conditional inclusion (using PA3), `#include` and its file search, `#line`,
  predefined macros, `#pragma` and `_Pragma`, `#error`.  It composes PA3 and
  PA4 into a working preprocessor, which is why it needs 47 lines of its own.
- **PA6 `recog`** is a different subject: **syntax recognition**.  The
  translation-unit grammar, the special tokens, and the mock name lookup that
  disambiguates a parse.  Nothing about macro expansion.

So "merge PA4 through PA6" would staple two subjects together.  PA4 and PA5
are two halves of one lesson — *build a preprocessor* — and merging them
gives a handout with a single deliverable and a single idea.  Adding PA6
would make it the kitchen sink the merge is supposed to avoid.

PA6 has a better home, and its own handout names it.  PA10 says: "PA10 is a
syntax assignment.  It replaces the PA6 recognizer boundary with" a
structured AST, "PA10 is not a new recognizer", and it lists "the PA6 grammar
and parsing approach as a starting point".  PA6 is a warm-up whose output
PA10 discards and whose grammar PA10 reuses — which is exactly why the
compiler links PA10's parser and never PA6's recognizer.  Recognition and
tree-building are the same activity with different artifacts, so they are one
lesson taught twice.

## Proposal

Nine early assignments become five, on both criteria.

1. **PA1, PA2, PA3 unchanged.**  Each builds machinery the compiler uses,
   with nothing thrown away, and each teaches one thing.
2. **Merge PA4 and PA5 into one preprocessor assignment.**  Macro replacement
   and the directive layer, delivering a complete phases-1-through-6 tool.
   One subject, one deliverable, and the 47-line assignment stops being a
   milestone of its own.
3. **Fold PA6 into PA10.**  PA10 already declares itself the replacement for
   the PA6 boundary and already starts from its grammar; the student solves
   recognition once, in the parser the compiler keeps.  PA6's grammar and
   ambiguity material becomes the front half of PA10's contract.
4. **Drop PA7 and PA8.**  6,087 lines of namespace, type and lookup model the
   compiler discards on the way to PA11 building the same rules for real.
   Their 110 test inputs move to PA11.
5. **Keep PA9** as PA13's execution oracle.

This differs from the shape the analysis started with — "PA4 through PA6 into
one" — because the handouts do not support it.  The code measurement and the
handout reading agree about PA5, PA7 and PA8; they disagree about where PA6
goes, and the handout reading should win, because PA6's problem is not that
it is small but that PA10 teaches it again.

## What this removes

- Four assignment slots: PA5 as a milestone of its own, PA6 as a lesson
  taught twice, and PA7 and PA8 entirely.
- 7,209 lines of student implementation that no later assignment builds on
  (PA6's 1,122, PA7's 2,500, PA8's 3,587).
- The `nsdecl` and `nsinit` tools, their source sets, and
  `dev/src/namespace_semantics/` and `dev/src/namespace_initialization/`
  from the tree — which also removes the duplication
  `docs/PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md` was written to unify, so
  that plan can be closed rather than implemented.
- The `recog` binary and `dev/src/recognition/`, which the compiler never
  linked; PA10's parser is the one it keeps.

## What it must not remove

- **PA6's coverage and its hard part.**  The 48 recognition tests and the
  ambiguity material — the special tokens and the mock name lookup that
  decides a parse — move into PA10's contract.  They are the reason
  recognition is a lesson at all, and folding the assignment must not quietly
  drop them into "the grammar is in `pa10.gram`".
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

After consolidation the chain is `pptoken → posttoken → ppexpr →
preprocessor → cy86 → cppgm++ → …`, which in the new numbering makes
**`test-through-pa6` the gold standard**: it is the target that proves
`cppgm++` itself compiles, the property CI actually cares about, and it is
today's `test-through-pa10` under a different name.  Extending to
**`test-through-pa9`** — today's `test-through-pa13` — is optional but
generally worth having, because it additionally proves `lowir2cy86`, the tool
PA13 checks LowIR execution against.

What this costs, stated plainly: three self-compilation data points leave the
ladder — `nsdecl` and `nsinit` with their assignments, and `recog` when it
folds into PA10 — of 2,500, 3,587 and 1,122 lines.  None feeds a later stage
and the gold-standard gate is unaffected, but the ladder does lose its
graduated middle: it now steps from `cy86` straight to `cppgm++`.  If that
gap turns out to matter, the merged preprocessor is the natural place to add
a checkpoint back, since it is the largest early tool that remains.

## One assignment, one binary, modes for the outputs

The merge raises a question the plan has to answer.  PA4 and PA5 run the same
translation phases and differ only in interface: `macro` reads one file on
standard input, `preproc` takes a set of files and writes an output file.
That interface difference is the whole 47 lines.

The course already has the answer in its later half.  `cppgm++` is one binary
whose stage outputs are modes: `--emit-ast`, `--emit-types`,
`--emit-semantics`, `--emit-lowir`.  Apply the same rule early:

**one assignment delivers one binary, and the intermediate outputs it used to
deliver become modes of that binary.**

The merged preprocessor assignment delivers one tool over a set of source
files, with the macro-expanded token stream available as a mode where PA4
delivered a separate program.  Every existing oracle survives as a flag, so
the tests move across rather than being rewritten; the harness already passes
per-test arguments through `CPPGM_APP_ARGS`.  PA6's `recog` binary does not
survive the fold into PA10 — PA10's deliverable is `cppgm++ --emit-ast`, and
a second recognizer is precisely what PA10's handout says it is not.

PA1, PA2 and PA3 stay separate binaries because they stay separate
assignments.  The rule is not "fewer binaries", it is "the binary list is the
assignment list".

## Naming

`ctrlexpr` is the one early tool named after a grammar production rather than
what it does, and it reads as an abbreviation of an abbreviation.  It
evaluates the controlling expression of a conditional inclusion — a
preprocessing expression — so **`ppexpr`** says the same thing, parallels
`pptoken` directly, and puts the three phase-ordered front-end tools in an
obvious family: `pptoken` (phases 1–3), `ppexpr` (conditional-inclusion
expressions), `posttoken` (phase 7 tokenization).

The merged tool keeps **`recog`**: it is the assignment's final deliverable
and the verdict students are asked for, and the earlier outputs are modes
underneath it rather than the point.

A rename is churn — the tool, the source set, the ladder map, the handout,
the tests and the export's copied paths — but Stage B rewrites all of those
anyway, so it costs almost nothing if it rides along with the renumbering and
is not worth a separate pass otherwise.

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
- Does folding PA6 into PA10 make PA10 too large?  Recognition and
  tree-building are one activity, but solving the parse ambiguities and
  designing an AST in a single lesson is a lot to ask at once; the
  alternative is keeping recognition as its own lesson and accepting that
  its code is thrown away.
- Is `ppexpr` the right name, and are there other early tools whose names
  should be revisited while Stage B is rewriting them anyway?
