# Plan: loosen the LowIR comparison where the contract is silent

Status: implemented 2026-09-05 (see Outcome at the end).  Opened
2026-09-05 at `db0d03cd`, from a spike run after
`PLAN-BACKEND-STUDENT-REVIEW.md` closed.

## The question

The source-to-LowIR assignments (PA15 through PA28) define the LowIR a
frontend emits as its output contract, and judge it with the `lowir_t`
comparison: the student's LowIR against the course solution's, after
presentation canonicalization.  That is the right shape for those
assignments.  The question this plan answers is narrower: does the
canonicalization cover every difference the contract does not name, or
does the comparison still reject LowIR that means the same thing spelled
differently?

## The spike

Sixteen equivalence-preserving rewrites of the course solution's own
emitted LowIR, applied to every fixture of three lanes (`pa16/tests/general`
227 fixtures, `pa19/tests/general` 229, `pa22/tests/general` 211), each
rewritten output compared with its reference by the real `lowir_t` mode.
The rewriter is now `scripts/lowir_seam_rewrite.py`, the driver became
`scripts/check_lowir_seams.py`, and the per-lane results before and after
are `doc/backend-review/lowir_seam_spike_results.txt`.

Rewrites the comparison already ignores, in all three lanes and every
fixture: temporary numbering, block label names, slot names, slot
declaration order, the order of items inside a metadata group, an extra
unused `declare function`, the order of function definitions, instruction
indentation, blank lines, parameter names.

Rewrites it rejects:

| rewrite | PA16 | PA19 | PA22 | nature |
|---|---|---|---|---|
| integer literal `0x0` for `0` | 109 of 227 | 82 of 229 | 91 of 211 | spelling; the parser and the validator both accept hex |
| operands of `cmp eq` / `cmp ne` swapped | 112 | 56 | 48 | commutative operation, either order |
| compare inverted and branch arms swapped | 96 | 57 | 64 | the sense of a conditional branch, identical control flow |
| two adjacent independent pure instructions swapped | 124 | 98 | 68 | evaluation order of side-effect-free operands |
| `%a = copy T %b` removed and uses forwarded | 4 | 1 | 0 | the LowIR family's temporary-address convention |

And one harness defect: a second metadata group on a function header,
`[unwind=may]` spelling the default, makes the comparison die with
"Can't use an undefined value as an ARRAY reference" at
`scripts/compare_results_common.pl` line 1413 (23, 16 and 4 fixtures)
instead of accepting it or reporting a validation error.

## The test each loosening must pass

A normalization is only worth having if a student can understand it from
one sentence and can predict, reading their own output, whether it will
compare equal.  When the normalization is more complicated than the
convention it protects against, it backfires: the student sees a canonical
diff of a shape they did not write, and learns the comparator instead of
the contract.  In that case the cheaper course is to state the convention
in words and enforce it.  So for each rejected rewrite the question is not
"can the comparison absorb this" but "which is simpler to teach: the
normalization, or the convention".  The rule that falls out: everything
the comparison rejects must be a convention written down in
`pa13/lowir.md`, and everything it absorbs must be describable in one
sentence there.

## What to change

Applied to the spike's findings:

1. **Integer and floating literals compare by value.**  Normalize.  The
   sentence is "a literal is its value, not its spelling"; a student
   expects nothing else, and the alternative convention ("decimal only")
   would fail people for a reason no one would defend.  Implemented in
   `canonicalize_lowir_for_compare`: parse each literal operand and reprint
   it in one form for its type (two's complement folded to the type width,
   floating values from their bits).
2. **Commutative operands compare in either order.**  Normalize.  The
   sentence is "for `eq`, `ne`, `add`, `mul`, `and`, `or`, `xor` the operand
   order does not matter"; the rule is the operation's own algebra and
   costs a student nothing to hold.  Implemented as a canonical operand
   order (constants after temporaries, then by canonical text).
3. **Branch polarity.**  State the convention; do not normalize.  The
   normalization ("a compare whose only use is the following branch is
   rewritten to the positive operation with its arms swapped") has a
   precondition, a table and a rewrite, and a student who fails near a
   branch for another reason would read a diff of code they did not
   emit.  The convention is one sentence and follows the source: "a
   conditional branch tests the comparison the source wrote, in the
   source's sense; `!=` is `cmp ne`, not an inverted `cmp eq` with the arms
   exchanged".  Our frontend already follows it.  Write it in
   `pa13/lowir.md` and the PA15 README.
4. **Copies of temporaries.**  State the convention; do not normalize.  The
   population is five fixtures in 667, and the copies are not decoration:
   they carry the family's temporary-address convention that the object
   path relies on, so an output without them is not equivalent for the
   stage that consumes it.  Forwarding them in the comparison would hide a
   real deviation.  `pa13/lowir.md` already documents the convention;
   PA15's README should point at it where copies first appear.
5. **The crash.**  Fix regardless; it is not a loosening.  A function
   header with more than one metadata group must either parse, as the
   grammar says it does, or fail validation with a message, and the
   signature table must not be consulted for a name it never recorded.
6. **Order of independent pure instructions.**  State the convention; do
   not normalize.  The normalization (a dependency-respecting canonical
   sort of the pure instructions of each block) is the most complicated of
   the six and the hardest to read back from a diff, and it would still
   leave side-effect order pinned, so a frontend with a different argument
   evaluation order would fail anyway on every call with two effectful
   arguments.  The convention is simple and is what the references already
   enforce: "instructions appear in the order the source evaluates them;
   where the language leaves the order of independent operands open, the
   course fixes it left to right".  Today that rule lives only in the
   references; it goes into `pa13/lowir.md` and the PA15 README in words.

The net change to the comparison is therefore small: two normalizations
that a student would assume anyway, and one crash fix.  The larger change
is to the text: three conventions the references have always enforced
become sentences a student can read before they fail.

## Method

1. Write the two canonicalizations with a fixture-free unit test: the
   literal and commutative rewrite modes of `lowir_seam_rewrite.py` applied
   to a small LowIR file must canonicalize to the same text as the
   original.
2. Rerun the spike over every `tests` lane of PA15 through PA28 and PA37
   (not just the three sampled) before and after.  The literal and
   commutative rewrites must reach zero rejections; the rewrites already
   at zero must stay there; the three convention rewrites (branch
   polarity, copy elision, pure-instruction order) are expected to keep
   failing, and the spike's report labels them as enforced conventions
   with a pointer to the sentence that states each one.
3. Run every assignment's `make test` in all three cells: no existing
   reference may start failing, since the two normalizations only merge
   spellings the references never distinguished.
4. Update the text.  `pa13/lowir.md` gets one section with two lists:
   what the relaxed comparison absorbs (the current list plus literals by
   value and commutative operand order, each in one sentence) and the
   conventions it enforces (branch sense, copies of temporaries,
   evaluation order of independent operands, each in one sentence).  The
   PA15 README, which introduces `--emit-lowir` testing, points at that
   section; PA37's Validation Modes section repeats the two lists.
5. Keep the spike as a lane.  The spike driver becomes a `make
   test-seams` target on one representative assignment, run in CI, with
   the invariant that every rewrite it reports as rejected is a written
   convention and every rewrite it reports as absorbed is a written
   normalization; a new rejection with no sentence behind it fails the
   lane.

## Exit criteria

- Literals by value and commutative operand order landed; the spike
  reports zero rejections for those rewrites on every PA15 to PA28 lane
  and on PA37; the crash is fixed.
- The three conventions (branch sense, copies of temporaries, evaluation
  order of independent operands) are stated in `pa13/lowir.md` and the
  PA15 README in one sentence each, and the spike lane labels their
  rewrites as enforced conventions.
- All cells pass with no reference change.
- `pa13/lowir.md` lists, in one place, what the comparison absorbs and
  what it enforces, and nothing the comparison rejects is outside that
  list.

## Risks

- A canonicalization that merges two spellings the references *do*
  distinguish would let a wrong output pass.  Items 1 to 4 merge only
  forms with identical semantics by construction; the per-cell run is the
  guard.
- A later reader may be tempted to normalize the three conventions after
  all; the conceptual-cost test above is the reason not to, and belongs in
  the `pa13/lowir.md` section so the reason travels with the rule.
- Stating the evaluation-order convention in words may expose fixtures
  where our own frontend does not follow it (a right-to-left case, an
  operand hoisted for a reason); the spike over all lanes will show them,
  and each is either a frontend fix or a documented exception.

## Outcome

Everything above landed in one change on 2026-09-05.

- `scripts/compare_results_common.pl`: literals compare by value (integers
  folded to the operand type's width and signedness, `f32`/`f64` through
  Perl's own correctly rounded parse and `%a`, `f80` and out-of-range
  spellings through exact big-integer arithmetic loaded only when needed,
  signed zero and subnormals kept distinct); `add`, `mul`, `and`, `or`,
  `xor`, `cmp eq` and `cmp ne` compare with their operands in either order;
  a function header with invalid metadata is reported and still recorded,
  so the call check no longer dies on it.  The unit tests in
  `scripts/tests/test_compare_lowir_results.py` cover each.
- Two further seams the all-lane spike exposed, fixed in the same pass:
  layout (indentation, blank lines) was only absorbed through the
  projection fallback, and is now canonicalized directly; internal
  functions that neither metadata nor a unique shape paired were paired
  by emission position, which a stale reference or two same-shape helpers
  defeated.  They are now paired in the order the already-paired program
  first refers to them, walking paired globals and functions; the pairing
  signature also ignores parameter names.  Both became sentences in the
  absorbed list.
- Cost.  The comparison runs many times under `make test-report`, so it
  was measured against the previous harness on five lanes: identical
  outputs skip canonicalization altogether (most course outputs are
  byte-identical to their reference), the big-integer module loads only on
  the rare slow path, the literal pass skips lines with no literal, and the
  layout and literal passes share the one loop that already renames locals.
  Net: PA24 general 0.96 s to 0.95 s, PA37 o1 unchanged, PA19 general
  0.94 s to 0.96 s, PA16 general 1.10 s to 1.15 s, memory unchanged.
- Text.  `pa13/lowir.md` gained "What The Comparison Absorbs And What It
  Enforces": one list of what the comparison absorbs (names, order,
  layout, unused declarations, internal symbol pairing, later-stage
  metadata, literals by value, commutative operand order), one list of the
  conventions it enforces (branch sense follows the source, a retype is a
  `copy`, instructions follow the source's evaluation order), and the
  reason the second list is not normalized.  The PA15 README states the
  three conventions and points at the section; PA37's Validation Modes
  repeats the two lists; PA16's README describes `make test-seams`.
- Lane.  `make -C pa16 test-seams` (run by `make -C pa16 test`) runs the
  harness unit tests and `scripts/check_lowir_seams.py` on
  `pa16/tests/general`: every mode of the rewriter is classified as
  presentation, normalization or convention and carries the phrase of the
  sentence in `pa13/lowir.md` that states it; the lane fails when an
  absorbed rewrite is rejected, a convention rewrite is accepted where it
  changed a compared output, or a sentence has gone missing.
- Result.  All 29 lanes (every PA15 to PA28 `tests` lane and PA37 `o0`,
  `o1`, `o2`) pass the invariant: 15 presentation and normalization
  rewrites accepted everywhere, the 3 convention rewrites rejected wherever
  they changed a compared output.  Every assignment's `make test` passes
  in all three cells with no reference changed.

