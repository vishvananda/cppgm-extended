# Machine-IR Register Allocation Plan

## Purpose

This document defines the concrete execution plan for adding **real register
allocation** to the native/object backend.

The goal is to replace the current stack-first lowering of ordinary temporaries
with an actual register allocation strategy while preserving the existing
LowIR/frontend contract.

This is not just a perf cleanup. It is also a backend correctness and quality
project:

- the current backend spills almost every temporary through the frame
- self-hosted code is much larger and slower than host `clang++ -O0`
- future `PA35` optimization work should not start from a backend that still
  behaves like a stack machine for ordinary scalar temporaries

## Main Decision

Register allocation should happen in the **machine-IR stage**, not in LowIR and
not in the final assembler/object writer.

That is the right layer because:

- LowIR should stay target-neutral
- register classes, call clobbers, and callee-saved behavior are x64-specific
- the assembler/object layer should consume already-allocated machine code-ish
  instructions rather than make policy decisions

In the current codebase, that means the main implementation lives around:

- [lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
- [machine_ir.h](/Users/vishvananda/cppgm/dev/src/machine_ir.h)
- [machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/machine_ir.cpp)
- [lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp)

## Current Problem

Today the backend does the following:

1. `build_layout(...)` allocates frame storage for params, slots, and every temp.
2. most lowering paths load values into hard-coded scratch registers
3. arithmetic happens in those scratch registers
4. results get stored straight back to frame slots
5. later uses reload the same values again

This is visible directly in:

- [lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
  `build_layout(...)`
- [lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
  `emit_load_value(...)`
- [lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
  `emit_store_temp(...)`

So the real current backend model is:

- params/slots: frame-backed storage
- temps: also frame-backed storage
- registers: mostly transient scratch/ABI registers

That is the part this plan intends to change.

## End State

The intended end state is:

- **addressable storage** remains frame/global based
  - params copied to param slots when needed
  - explicit LowIR slots remain storage
  - globals remain globals
- **ordinary SSA-like temps** become allocatable values
  - preferably held in registers
  - spilled to frame only when pressure requires it
- calls obey ABI clobber rules explicitly
- the backend tracks and saves/restores any used callee-saved registers
- `machine_ir` dumps remain readable enough to explain where values live

The important architectural rule is:

- do not confuse “thing with an address” and “ordinary computed value”

That means this plan is about temps first, not about eliminating slots or
changing LowIR ownership.

## Invariants

The implementation must preserve these invariants.

1. LowIR remains the same boundary for now.
2. LowIR `slot`s remain addressable storage and are not silently turned into
   non-addressable registers by the backend.
3. Register allocation only changes where ordinary temps live.
4. Calls remain ABI-correct for:
   - argument registers
   - return registers
   - caller-saved clobbers
   - callee-saved preservation
5. The object backend must never see “half allocated” machine IR.
   It should either see:
   - the current fully physical form
   - or a fully allocated replacement form
6. `f80` scratch handling must keep working.
   Register-allocation work must not overlap the existing scratch area used by
   float/x87 helper paths.

## Recommended Allocation Strategy

The right rollout is **incremental but real**.

### Stage A. Conservative Integer/Pointer Temp Allocation

First allocate only non-floating scalar temps:

- `i1`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `ptr`

The first safe landing may use a very narrow caller-saved pool for temps that
do **not** live across calls, for example:

- `r8`
- `r9`

That avoids inventing a fake unwind story for callee-saved register restoration
before we have explicit unwind metadata.

After that lands, broaden to a conservative callee-saved pool such as:

- `rbx`
- `r12`
- `r13`
- `r14`
- `r15`

Why this staged approach:

- the caller-saved no-live-across-call slice is the smallest ABI-safe first
  landing
- the later callee-saved slice avoids fighting the current scratch-heavy
  lowering style
- together they get real register residency online without a full machine
  rewrite first

### Stage B. Liveness/Intervals Over LowIR Temps

Compute a deterministic live interval for each temp over the emitted function
instruction order.

For the first slice, interval quality may be conservative:

- use function block order
- use first-def / last-use numbering
- overapproximation is acceptable
- incorrect underapproximation is not

This is enough for a first linear-scan allocator over the existing backend.

### Stage C. Spill Fallback

Any temp that cannot be assigned a register remains on the current frame path.

That means the first allocator rollout should be a **hybrid backend**:

- allocated temps: register resident
- unallocated temps: current frame-backed behavior

This keeps the project shippable during the transition.

### Stage D. Save/Restore Of Used Callee-Saved Registers

The machine IR function model should explicitly record which callee-saved regs a
function uses, and the object backend should save/restore them in prologue and
epilogue.

This must be explicit backend metadata, not an implicit guess in the assembler.

### Stage E. Direct Register Use In Lowering

After the first conservative allocator lands, stop immediately copying
register-resident temps back into scratch registers when that is unnecessary.

Examples:

- if a temp already lives in `rbx`, use `rbx` directly where possible
- remove redundant `mov scratch, assigned_reg` hops
- coalesce obvious `mov reg, reg` no-ops

This stage is where the allocator starts turning into a real quality win
instead of “less stack traffic but still many extra moves”.

### Stage F. Caller-Saved And Initial XMM Expansion

After the callee-saved GPR path is solid:

1. add caller-saved GPR support with explicit live-across-call handling
2. add the first `xmm` residency path for ordinary `f32` / `f64` temps
3. keep `f80` on the existing scratch/x87 path until there is a clear reason to
   generalize it

## What Should Not Happen First

Do **not** start by:

- rewriting LowIR to become register-based
- teaching the assembler to allocate registers
- trying to optimize slots/address-taken locals away in the same patch series
- starting with XMM/f80 before ordinary scalar temps are working
- mixing this with PA35 high-level LowIR optimizations

Those are either wrong-layer changes or too much scope for the first landing.

## Concrete Execution Phases

## Phase 1. Backend Scaffolding

Add the metadata and plumbing needed for allocation:

- machine-IR function metadata for:
  - scratch area size
  - used callee-saved registers
- object-backend save/restore support for those registers
- readable machine-IR dump support for the new metadata

This phase should be behavior-preserving by itself.

## Phase 2. Conservative GPR Temp Allocation

Implement the first real allocator in `lowir_machine_ir.cpp`:

- collect temp defs/uses
- compute live intervals
- first assign no-live-across-call `i64` / `ptr` temps to a narrow caller-saved
  pool
- then broaden to the callee-saved pool once explicit preservation is in place
- leave the rest spilled

Lowering helpers should then:

- load spilled temps from frame as before
- read allocated temps from their assigned register
- write allocated temp defs directly to their assigned register
- omit frame bindings for temps that are fully register-resident

This is the first phase that should visibly reduce stack traffic.

## Phase 3. Call-Aware Expansion

Broaden the allocator to use caller-saved registers where profitable.

That requires:

- call-site clobber modeling
- spill/reload around calls when needed
- or interval splitting that keeps live-across-call values in callee-saved regs

This phase should stay after the first callee-saved slice is stable.

## Phase 4. Initial XMM Allocation

Add register allocation for `f32` / `f64` temps:

- define the allocatable `xmm` pool
- model call clobbers
- keep `f80` on the current x87/scratch path for now

Do not block the scalar allocator on this phase.

## Phase 5. Coalescing And Cleanup

Once values are really register-resident:

- remove redundant reg-to-reg moves
- reduce forced scratch-register hops
- narrow frame usage to:
  - actual slots
  - params copied to slots
  - spills
  - backend scratch area

## Validation Plan

Each phase should validate at three levels.

### 1. Structural Backend Validation

- `make -C dev cppgm++ lowir2native`
- direct `machine_ir` dumps for reduced cases
- compare stack size / frame bindings before and after

### 2. Focused Runtime Validation

Use reduced cases that are sensitive to:

- value preservation across calls
- nested arithmetic temp pressure
- branch-heavy temp lifetimes
- floating-point temp handling once XMM allocation starts

### 3. Perf/Size Revalidation

Recheck the existing hot self-host cases:

- `pa3/tests/300-triple.t`
- `pa32` self-host `ctrlexpr-self`

Success should look like:

- smaller `calculator.o` code
- fewer frame temp bindings in machine IR
- materially reduced self-host runtime without waiting for PA35

## Completion Criteria

This plan is complete when:

1. ordinary scalar temps are no longer universally frame-backed
2. used callee-saved regs are explicit and ABI-correct
3. machine-IR dumps make register residency visible for both scalar temps and
   the initial `f32` / `f64` XMM path
4. the self-host/backend perf gap shrinks for the current PA3 hot path
5. the backend is ready for the later `PA22` machine-quality tranche without
   still behaving like a pure stack machine for ordinary temporaries

## Current Landing

The current implementation has landed the intended initial register-allocation
tranche:

- machine-IR function metadata now records backend scratch usage and the used
  callee-saved GPR set
- prologue/epilogue and EH save/restore paths explicitly preserve used
  callee-saved temp registers
- ordinary scalar temps now allocate across the full integer/pointer scalar set
  instead of being universally frame-backed
- live-across-call temps stay in callee-saved GPRs, while shorter-lived temps
  can use caller-saved GPRs
- `f32` / `f64` temps have an initial XMM residency path, while `f80` stays on
  the existing x87/scratch path
- lowering now reuses assigned temp registers directly and includes a small
  local reload-elimination peephole instead of forcing every value back through
  frame slots

Additional follow-on work is already visible in the current dirty tree:

- width-aware integer compare/test selection
- explicit sign/zero-extension machine-IR operations
- direct integer register destinations for some float-to-int conversion paths

That work is in the right direction, but it belongs to the next plan boundary:

- [pa23-machine-ir-quality-plan.md](/Users/vishvananda/cppgm/docs/implemented/pa23-machine-ir-quality-plan.md)

That follow-on quality tranche should own:

- width-aware integer compare/branch and conversion cleanup
- direct compare-to-branch shape rules
- complete `f32` / `f64` XMM-resident quality, including direct XMM moves and
  fewer scratch-memory bounce paths
- the more flexible public MIR oracle needed to test those choices without
  freezing exact physical register names

The final follow-up for *this* plan was:

- perf revalidation on the PA3 self-host hot path

That revalidation is now complete, so this plan is ready to archive.
