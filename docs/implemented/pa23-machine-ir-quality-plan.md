# PA23 Machine-IR Quality Plan

## Purpose

This document defines the intended next buildout tranche for `PA23`.

`PA23` already owns the transition from:

- `LowIR -> CY86`

to:

- `LowIR -> machine IR -> native`

That means `PA23` is already the right public home for backend-shape and
backend-quality requirements such as:

- direct compare/branch lowering
- avoiding unnecessary stack traffic for ordinary temps
- preserving native-width integer behavior without pointless widening churn
- using the floating register path for ordinary `f32` / `f64` work
- exposing a machine-IR oracle that is strong enough to test quality without
  over-constraining harmless register-choice differences

This is **not** the same as the later `PA36` optimization assignment.

`PA36` should still own:

- LowIR optimization
- `-O*` levels
- canonical simplification and propagation over LowIR

This plan is specifically about finishing the `PA23` machine-backend contract
so later work does not build on top of a backend that is still effectively a
stack machine.

It is intentionally the follow-on to the initial landing described in
[machine-ir-register-allocation-plan.md](/Users/vishvananda/cppgm/docs/implemented/machine-ir-register-allocation-plan.md).

That earlier plan owns bringing real temp residency online at all:

- GPR temp allocation
- callee-saved / caller-saved ABI correctness
- initial `f32` / `f64` XMM residency
- readable MIR location metadata

This plan owns what comes next:

- width-aware integer cleanup after regalloc exists
- direct compare/branch quality rules
- completion of the ordinary `f32` / `f64` XMM fast path
- the public MIR oracle needed to test that quality without over-constraining
  register choice

## Why This Needs A Dedicated Tranche

The current `PA23` test surface is good at proving backend correctness, but it
is not yet a good public oracle for backend quality.

Today the deterministic `.mir` dump often still reflects the old stack-first
lowering style:

- compare results materialized into stack temps before branching
- integer temps widened through extra shifts/sign-extends in cases that should
  lower more directly
- almost every ordinary temporary assigned a frame slot
- `f32` / `f64` compare paths often treated as "compute bool, store bool, reload
  bool, branch on bool" instead of a cleaner compare-to-branch flow

That makes the current state awkward for students:

- if we keep exact raw `.mir` matching, better register allocation and better
  compare lowering will break the existing oracle
- if we stop checking machine IR structurally, then the assignment loses its
  backend-owned proof surface

So the missing piece is a more flexible machine-IR oracle together with a small
set of explicitly required backend quality rules.

## Main Recommendation

Treat this as an **extension of PA23** or an immediate follow-on `PA23`
quality tranche.

The public contract should become:

1. `PA23` still owns correctness of direct `LowIR -> machine IR -> native`
2. `PA23` also owns a small, explicit set of direct-lowering quality rules
3. those quality rules are tested through a more flexible MIR oracle than exact
   raw register-by-register matching

This is a better fit than moving the work to `PA36`, because:

- register allocation is target-specific machine-backend behavior
- compare/branch lowering quality is also first a machine-backend concern
- `PA36` should remain the first **LowIR optimization** assignment, not the
  first place students discover whether the backend still spills everything

## Public Contract Additions For PA23

The `PA23` README should explicitly call out a small set of required machine-IR
quality rules.

These rules should be narrow, concrete, and testable.

## Integer Lowering Rules

For the supported scalar subset, `PA23` should require:

1. Direct branch-on-compare lowering.
   - If a compare result is used only by a branch, the backend should lower it
     directly to machine compare plus conditional branch.
   - It should not first spill the boolean result to a stack temp and reload it
     just to branch.

2. Width-appropriate integer compare lowering.
   - `i32` / `u32` compares should lower through a 32-bit machine-width path
     when semantically valid.
   - The backend should not route ordinary 32-bit comparisons through an
     obviously wider spill-heavy path by default.

3. No default stackification of ordinary trivial temps.
   - In small leaf scalar functions, non-address-taken temps should remain
     register-resident unless pressure requires a spill.

4. Spills only when pressure or ABI boundaries require them.
   - Ordinary temps may spill when register pressure is real or when a value
     must survive a clobbering call boundary.
   - They should not spill purely because the backend still uses a stack-first
     model for every temporary.

5. Compare materialization only when demanded by later uses.
   - If a compare result is consumed as a value, materializing a boolean is
     acceptable.
   - If the compare only drives control flow, the direct branch form should be
     preferred.

## Floating-Point Lowering Rules

The same quality contract should explicitly include float work.

For the supported floating subset, `PA23` should require:

1. `f32` / `f64` arithmetic stays on the floating register path.
   - Ordinary `f32` / `f64` arithmetic should not be lowered by converting to
     integer temporaries or by round-tripping through integer compare/materialize
     forms unless the source operation truly demands it.

2. `f32` / `f64` compare-to-branch lowering is direct when possible.
   - If a floating compare result is consumed only by a branch, the backend
     should lower it as directly as the target model allows, rather than always
     forcing "materialize bool, spill bool, reload bool, compare-to-zero".

3. Width-appropriate float register use.
   - `f32` and `f64` operations should use the ordinary machine floating
     register path and preserve the source width distinction in MIR.

4. `f80` is allowed a more conservative path.
   - `f80` does not need to meet the same residency expectations as `f32` /
     `f64`.
   - But it should still avoid gratuitous integer-side distortion, and the
     README should say clearly that `f80` remains the scratch/x87-oriented
     floating special case.

5. Float conversions stay structurally visible.
   - `f32`/`f64`/integer conversion chains should remain visible in MIR so the
     tests can prove the right width and conversion family are being used.

## Oracle Design

The current `--dump-machine-ir` path is the correct starting point, but exact
raw MIR text should not remain the only long-term oracle once register
allocation quality matters.

The recommended public testing shape is:

1. raw MIR remains available for debugging
2. canonical MIR becomes the primary structural test oracle for quality-focused
   cases
3. runtime behavior remains a secondary oracle

## Why Canonical MIR Is Better

We do not want to over-constrain harmless backend choices such as:

- exact physical register choice
- exact stack offsets
- block numbering differences that do not change structure

But we do want to preserve and test:

- opcode family
- operand width
- direct compare/branch shape
- whether a value is register-resident or stack-backed
- whether a spill/reload exists at all
- whether a floating operation stays on the floating path
- whether a call boundary forces preservation/spill behavior

So the structural test oracle should normalize away free choices while keeping
backend-quality decisions visible.

## Recommended Canonicalization Rules

The canonical MIR oracle should preserve:

- instruction opcode and width
- direct vs indirect call shape
- direct compare-to-branch vs materialized-bool shape
- register vs stack vs immediate location class
- spill and reload events
- callee-saved/caller-saved preservation shape
- floating operation family (`f32`, `f64`, `f80`)
- explicit conversion families

It should normalize:

- specific physical GPR names where they are interchangeable
- specific physical XMM names where they are interchangeable
- exact stack offsets
- exact temp numbering when that numbering is not semantically meaningful

Examples of what should remain distinguishable:

- direct `cmp` + `jne` versus `cmp`, `setcc`, spill, reload, `cmp 0`, branch
- `f32`/`f64` compare on floating registers versus integer-side emulation
- no spill in a trivial leaf case versus obvious spill/reload churn

## Test Strategy

The public tests should stay simple.

The best shape is:

- tiny LowIR inputs
- canonical MIR refs
- normal runtime/exit-status checks

This avoids forcing students to write MIR-inspect scripts while still giving the
assignment a structural backend oracle.

## Integer Test Families

The first quality tranche should add small, explicit LowIR tests for:

1. `i32` compare directly feeding branch
   - require direct compare/conditional branch shape
   - forbid spill of the compare result

2. `u32` compare directly feeding branch
   - same as above, but explicitly unsigned

3. compare result used as a value more than once
   - allow boolean materialization
   - still require no unnecessary stack round-trip if the value can stay in a
     register

4. trivial leaf arithmetic chain
   - prove ordinary temps are not all frame-backed by default

5. call-clobber pressure case
   - prove values survive calls correctly
   - allow spill only where ABI pressure justifies it

6. mixed-width integer compare/conversion case
   - make width handling visible and intentional rather than accidental

## Floating Test Families

The same tranche should add small floating-specific tests for:

1. `f32` compare directly feeding branch
   - direct floating compare/branch shape
   - no mandatory bool spill/reload chain

2. `f64` compare directly feeding branch
   - same as above for `f64`

3. simple `f32` arithmetic chain
   - prove ordinary `f32` arithmetic stays on the floating register path

4. simple `f64` arithmetic chain
   - same for `f64`

5. float compare result used as a value
   - allow materialization where the source program actually needs it
   - still preserve visible floating compare behavior in MIR

6. integer/float conversion chain
   - prove the backend uses the intended conversion family and keeps widths
     visible

7. mixed integer/float branch case
   - expose interaction between compare lowering and later control flow

8. conservative `f80` case
   - keep one or two tests proving `f80` correctness and structural lowering
   - do not require `f80` to meet the same register-quality bar as `f32`/`f64`

## README Changes Needed

The `PA23` README should be updated in three places.

### 1. Output / Oracle

The README should distinguish:

- raw machine-IR dumps for debugging
- canonical machine-IR dumps for structural testing

If we keep a single dump flag publicly, the README should still say that the
tested MIR form is canonicalized and intentionally hides free register-choice
differences.

### 2. Assignment Boundary

The README should explicitly add the direct-lowering quality rules described
above, including:

- direct compare-to-branch lowering
- no default spill of trivial ordinary temps
- width-appropriate integer compare behavior
- floating-register-path ownership for ordinary `f32` / `f64` work

### 3. Testing

The README should explain that the machine-IR oracle is intentionally:

- structural enough to prove backend ownership
- flexible enough not to freeze one exact allocator choice

## Buildout Sequence

Recommended implementation order:

1. define the canonical MIR test surface and normalization rules
2. update the `PA23` README to call out the required quality rules explicitly
3. add the smallest integer quality tests first
4. add the matching floating quality tests
5. only then broaden the register-allocation/performance-oriented backend work

This keeps the assignment contract clear before the implementation gets more
ambitious.

## Relationship To Register Allocation

This plan does not require a perfect allocator before the tests can exist.

The first bar is narrower:

- direct compare/branch quality
- avoiding obviously unnecessary stackification
- preserving ordinary `f32` / `f64` machine paths

That means the tests can start by enforcing "not obviously stack-first
everywhere" before demanding a mature allocator.

Later register-allocation improvements can then tighten the same test surface
instead of introducing a second incompatible oracle.

## Relationship To PA36

This plan should land before or alongside later LowIR optimization work, not as
part of it.

`PA36` should still assume:

- `PA23` already owns a credible machine-backend path
- `PA23` already owns the first machine-IR quality rules

That keeps the curriculum split coherent:

- `PA23`: direct backend correctness and basic machine-backend quality
- `PA36`: LowIR optimization and `-O*`

## End State

When this plan is complete:

- `PA23` will no longer be only "native code that happens to work"
- it will also define a small, explicit machine-backend quality contract
- integer and floating compare/lowering quality will both be part of the public
  backend surface
- the MIR oracle will be strong enough to test backend structure without
  freezing accidental allocator details
