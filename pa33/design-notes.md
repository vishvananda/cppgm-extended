# PA33 design example

These notes explain one implementation. The [assignment README](README.md)
and course fixtures define the required contract and quality bar. The pass
order, data structures and numeric limits below are design choices.

### One Design: The Course Solution's Optimization Levels

Everything in this section describes the course solution: reactive
placement over a nine-register pool, a whole-function planner at `-O2` and
above, and the placement decisions listed below.  It is one way to meet the
quality bar, recorded so that its dumps are explainable.  None of it is a
requirement; the course fixtures do not compare your placement with the
course solution's, and the regression lane pins these dumps for the
compiler team.

Where a different allocator plugs in.  The course solution separates
what may live in a planned register from where it lives.  The first is a
candidate list built from the function's facts (`FunctionFacts` in the
course solution's `native/analysis/function.h`, not part of the starter kit:
each value's definition and
last-use positions, the spans that extend an interval over layout
backedges and exception regions, the positions at which each register is
clobbered, and the flags that say whether a value is live across a call
or across a block edge); the second is `plan_value_locations` in the course solution's
`native/allocation/location_planning.cpp`, which returns a
per-value timeline of planned locations that the reactive walk then
honours.  The colouring allocator is a second assignment at that seam,
taking the same candidates and facts.  An allocator that takes the same
inputs and returns such a timeline is the smallest complete replacement;
one that also replaces the reactive walk is a full one.

The course solution implements these backend optimization levels:

`-O1` is the local machine-improvement level. It includes these
semantic-preserving rewrites where safe:

- remove unconditional jumps to the immediately following block
- coalesce block-local integer and floating-point register copies
- remove redundant move chains and simple return shuffles
- remove integer and floating-point moves whose source and destination are the
  same physical register
- clean up call-result and call-argument copies
- retain register copies that are still required for ABI call-argument setup,
  distinct bulk-copy source/destination operands, or values live into successor
  blocks
- rematerialize cheap integer immediates into supported arithmetic,
  zero-compare, and call-argument instruction forms
- collapse conditional-branch plus unconditional-jump block tails when one
  target is the natural fallthrough block
- rewrite zero-comparison branches into direct `test reg, reg` machine IR when
  the backend supports that shape
- fold frame-address temporaries back into direct frame operands or direct
  `lea` call-argument setup where safe
- use one three-operand `lea` for an O1+ 64-bit integer or pointer addition
  when the result cannot overwrite its still-live left input and the right
  input is a register or signed 32-bit displacement
- keep safe frame, local-global, and constant-index storage addresses as
  rematerializable MIR operands across their complete use interval; observing
  the pointer value, clobbering a carrier, variable indexing, or arithmetic on
  the pointer requires materialization
- when constant-index lowering changes from an intact incoming parameter
  register to a preselected parameter home, retain the setup transfer before
  any materialized or deferred use of that home.  Reusing the ABI carrier is
  also valid while it remains intact; optimized MIR must never name an
  uninitialized alternate home
- place an unplanned edge-live integer or pointer identity copy directly into
  its required frame home, and likewise allow an eligible scalar call result
  to be stored from its ABI return register; planned and exact-forward values
  retain their selected locations
- let a single-use scalar value from an acyclic merge `phi` share its frame
  home with a later acyclic merge `phi` that consumes it on one incoming edge.
  At O3, the source may also have completed earlier uses when its final use is
  exactly that predecessor's merge transfer; O2 and below retain the
  single-use boundary.  The shared home removes an identity edge transfer and
  every alternate incoming edge still writes that home.  Inside a cyclic
  region, this is safe only when both non-loop-carried merges belong to the
  same cycle, so the source is refreshed before each dynamic use.  A
  loop-carried merge, a loop invariant feeding a repeated merge, a source used
  after its transfer, or a representation change must retain an independent
  home unless a different register-resident implementation makes the transfer
  unnecessary
- when a one-use scalar merge is consumed immediately as a call argument,
  allow one same-typed, one-use incoming temporary to be defined directly in
  the merge's frame home.  This removes the incoming temporary's separate
  home and its identity edge transfer; every alternate edge must still write
  the merge home.  In a cyclic region, the incoming definition and merge must
  belong to the same cyclic component so the value is refreshed before every
  dynamic transfer.  A loop invariant, a value with another use or a different
  representation, or a merge with an intervening consumer keeps the ordinary
  independent-home path.  No particular register or instruction sequence is
  required
- keep a frequently reused, iteration-local scalar call result available to
  branch comparisons throughout one cyclic choice region.  The defining call
  must execute before every dynamic use, dominate every use, and have all of
  those uses in the same cyclic component; a loop invariant or a value used
  outside that component does not satisfy the proof.  When later comparisons
  cross another call, reserve enough call-preserved register capacity for the
  bounded region and keep overlapping cyclic allocations from consuming it.
  If the proof or capacity is unavailable, retain the ordinary frame-home
  path; no particular physical register is required
- at O2 and above, honor that proven cyclic-result placement when the defining
  call has enough arguments to create ordinary call-result pressure.  Call
  arguments are no longer live once the call has completed, so retire their
  allocation state before placing the result.  Earlier reactive call results
  whose lifetimes overlap the region must avoid registers claimed by a future
  whole-function plan.  An unproved result, unavailable capacity, or O1 keeps
  the established frame-home fallback; no particular physical register is
  required
- permit loop-carried integer or pointer `phi` values in a guarded fast arm
  to reuse call-preserved register capacity when the fast arm cannot reach a
  call, the values die before the function's first call, and the sibling
  call-bearing arm already creates full preserved-register pressure.  This
  exception must not add save/restore capacity.  A loop whose header can
  reach a call, a function without the existing preserved pressure, or an
  otherwise unproved path relationship retains the ordinary frame homes; no
  particular physical registers are required
- permit a bypassable, call-free loop to begin loop-carried integer or pointer
  `phi` residency on its first incoming edge, after any call-bearing prefix,
  when the complete incoming-edge-through-backedge interval has no call or
  fixed-register clobber.  A `phi` is loop-carried only when a predecessor
  laid out at or after it belongs to the same cycle; a merge whose late
  predecessor is an acyclic block (a critical-edge split block placed at the
  end of the function) keeps the ordinary frame home, since nothing in an
  acyclic region pins the register between the merge's reads and that
  predecessor's transfer.  This local residency must use caller-saved capacity,
  must not add a function-wide save/restore, and must not overlap another
  planned owner of that capacity.  When an earlier call-bearing prefix already
  creates full call-preserved pressure, the planner should prefer otherwise
  free caller-saved argument capacity for the local interval so unrelated
  caller-saved temporaries do not block activation.  A call in the interval,
  unavailable capacity at the incoming edge, added preserved-register
  pressure, or an unproved span retains the ordinary frame-home transfers; no
  particular physical register is required
- forward a scalar compiler temporary from its defining register into an
  immediately following integer comparison when that comparison is the
  temporary's only use, both operations have the same machine type, and the
  other comparison operand is encodable with a register.  The final MIR drops
  the now-unneeded frame store and compares the register directly.  Volatile,
  debug-visible, multi-use, unannotated, non-adjacent, or otherwise unproved
  frame values retain their stable homes
- after complete MIR liveness is available, recolor a callee-saved physical
  register to an otherwise noninterfering caller-saved register when every
  occurrence is explicit, none of its live ranges crosses a clobber of the
  replacement, and debug or implicit-register state does not depend on the
  original register.  Call liveness should use the call's exact annotated
  argument set when present.  At O1 this is required for the profitable
  even-save call-function case, where removing one save also removes the
  SysV call-alignment padding adjustment.  An odd-save call function may be
  recolored only when at least two independently safe callee-saved colors have
  distinct caller-saved destinations, so the pair removes both saves without
  adding alignment padding.  Leaf functions, an odd count with fewer than two
  safe colors, call-crossing ranges, and unproved interference retain their
  original placement.  No particular caller-saved register is required
- reuse a final-use pointer register as the destination of an eligible scalar
  load only when the effective address is consumed before the destination is
  overwritten, the pointer has no surviving aliases or edge use, and the load
  result needs the ordinary stable edge home
- omit the frame pointer only when the final MIR has no dynamic stack, host EH,
  scratch area, debug variable location, or frame operand; functions rejected
  by any of those guards keep it
- keep semantic returns distinct in MIR while using direct, unshared physical
  epilogues at O1+; O0 retains the shared-epilogue policy

`-O2` is the whole-function machine-improvement level. It must include all
`-O1` work and additionally:

- select direct scalar and unaligned-vector chunks for fixed object copies
  larger than 32 and no larger than 64 bytes, even when the source type has
  weak alignment. Serialize that encoding choice in MIR so dump and object
  paths agree. The PA24 O0 and PA33 O1 paths keep their compact
  string-operation encoding, and copies larger than 64 bytes retain the
  compact fallback
- collapse adjacent integer-normalization instructions when the later
  normalization provably subsumes the earlier result. A narrow scalar load
  immediately followed on the same register by a same-width sign or zero
  extension may instead select the corresponding signed or unsigned load.
  A repeated extension with the same signedness is redundant, and a wider
  extension is redundant after a narrower zero extension because the value
  is already nonnegative. Do not cross an intervening instruction, and retain
  a narrower sign extension followed by a wider zero extension because it
  changes the represented value. O0 and O1 retain their established MIR
- in a function with no conditional branch, improve block layout by following
  unconditional jump traces so likely successors become natural fallthrough
  blocks; preserve the established order in functions with conditional
  branches so an existing conditional fallthrough is not displaced
- retain a LowIR value in one physical register through joins or backedges
  when its complete interval conflicts with no fixed-register use; a value
  that crosses a call may remain in a callee-saved GPR, while an XMM value may
  remain register-resident only when it crosses no call
- consider single-use values as well as repeatedly used values for planned
  residency, and make the target ABI's otherwise available call-preserved GPR
  capacity eligible for bounded call-crossing intervals; no particular
  physical register is required
- plan call-free integer or pointer intervals in otherwise available
  caller-saved registers, including inside an exception-bearing function when
  the complete interval reaches no call; a call-reaching interval must instead
  be recomputed, stored, or placed in call-safe capacity
- use a frame home when exception flow or register pressure prevents a stable
  whole-function register placement; cyclic placement must leave register
  capacity for values first produced inside the cycle and must not evict a
  location that an already-emitted backedge will use on its next iteration
- allocate and initialize a planned edge value's fallback frame home only when
  an actual eviction or home-reading consumer requires it; a value that stays
  resident for its complete interval must not acquire an unused eager home
- remove callee-saved register preservation that is no longer needed after
  optimization
- recompute final stack reservation from the surviving frame state
- treat a cached-address carrier as pinning its current physical-register
  lifetime, not that register name for the rest of the function.  After the
  carrier and every address that replays it are finished and the register is
  physically free, a later unrelated value may begin a new register lifetime
  without inheriting the old carrier's release restriction.  A carrier still
  referenced by a cached address remains pinned

The same interval machinery is available at every optimizing level for the
bounded O1 placement classes used by the checked fixtures. Loop-carried phis
may claim stable callee-saved homes for unavoidable loops and for the bounded
path-disjoint guarded-fast-arm case above; a cursor load may then take over
its dead phi-home address register. Loop invariants and
ordinary call-free values may remain in conflict-free registers, including in
an exception-bearing function when their interval crosses no call. Values
that require a frame fallback may acquire a second caller-saved register
segment only after their final call when one acyclic block dominates every
remaining use. Interval-end and final-use release must respect backedge and
backward-EH spans, cached address carriers, aliases, parameters, and fixed
register effects.

`-O3` must accept the LowIR produced by PA32's maximum optimization level and
apply all `-O2` machine improvements. It additionally performs one bounded
post-liveness recoloring step. A callee-saved physical-register color may be
removed when every occurrence is explicit, no debug range refers to it, and
each connected source-live region has one noninterfering replacement. A
region may span control-flow edges, but all blocks connected by source
liveness use the same replacement. Disjoint regions may independently choose
a caller-saved register or an already-used callee-saved color that will remain
preserved. Exact instruction and block-boundary liveness must prove that the
replacement does not hold another live value and is not clobbered while the
source is live. A call clobber, implicit use, debug dependency, or unproved
interference rejects the complete source color; partial elimination is not
permitted.

O3 may also turn an exact final direct call into a sibling transfer using only
facts derivable from serialized LowIR. The enclosing function must return void
or an integer/pointer scalar, have fixed arity, at most six direct scalar
parameters, and have no local slots, dynamic stack allocation, variadic state,
or exception operations. Its only call must target a direct fixed-arity
function, pass the enclosing function's original parameters once in the same
order, and be followed immediately by that block's return. A void call must be
followed by a void return. A scalar call must have exactly the enclosing return
type, and that exact call result must be returned; the enclosing function must
also contain at least one other normal return.

For that shape, optimized MIR records a terminal `sibling_call` rather than an
ordinary call followed by a return. Native emission restores the complete
current frame and callee-saved state before jumping to the direct target, so
the target returns directly to the original caller. The transfer has no stack
arguments and creates no ordinary call-alignment requirement. A changed or
reordered argument, an indirect or additional call, local storage, an
unsupported parameter representation, or any exceptional/stack boundary keeps
the ordinary call and return. O0 through O2 also keep the ordinary form.

Two narrow O3 machine cleanups preserve the benefit of PA32's terminal-query
split without assigning semantic meaning to helper names. In a one-block
integer helper with exactly one call, a normalized call result may remain in
the ABI return carrier when every later use is a same-width store or the final
return and at least one such store exists. Any independent use of that carrier
must be explicit, defined before use, and recolorable to one register unused
through the suffix. The rewrite removes only the result move and redundant
normalization; implicit effects, debug locations, a second call, a mismatched
store width, or another result use retain the ordinary form.

In an exact three-block scalar sibling wrapper, O3 may also omit an eager copy
of an incoming register parameter. The entry's later uses, the sibling arm,
and the fast return arm must make the two register lifetimes explicit. The
fast arm may use the incoming carrier directly when it has no conflicting
local lifetime; one later write-only local lifetime may instead be recolored
to a register unused throughout that arm. A hidden or implicit use, additional
predecessor, call, debug range, unproved definition, or unavailable scratch
register rejects the complete rewrite. MIR control-flow analysis treats a
`sibling_call` as terminal even when its block is not last in layout.

With `--stats`, report `machine_opt_terminal_call_results` and
`machine_opt_sibling_parameter_retains`; both remain zero below O3. The focused
control identifies the wrapper, helper, producer, and changed-result guard by
their call/store/return relationships. It checks carrier relationships and
behavior at all four levels and through driver replay without requiring a
particular helper name, physical register, or complete MIR sequence.

Call-boundary facts that have no machine-level encoding, including PA8
`query=stable_prefix`, remain valid input at every PA33 optimization level.
PA32 may consume the fact and rewrite calls before native lowering; PA33 must
also accept a surviving marked definition without inventing a hidden side
channel or changing behavior. The focused control exercises direct backend
lowering at `-O0` through `-O3` and the combined `cppgm++ -O3` replay path.

PA33 must likewise preserve the benefit of PA32's O3 parameter-address
rematerialization. When several positive constant-offset field addresses from
one bounded pointer parameter are used only after a call, PA32 may move those
address definitions below the call. Given either form of that LowIR, native
selection preserves the common base across the call once and uses direct
base-plus-displacement memory operands afterward: from `-O1` a
base-plus-constant index address is replayed from its base at each consumer
rather than held in a second register, so the staged O2 form already keeps
only the base in a call-preserved home. It must not recreate derived
addresses before the call and consume separate call-preserved homes for
them, at either level, and the rematerialized O3 form must not add
call-preserved homes or frame size over the O2 baseline. The focused control
runs `lowiropt` before `lowir2native` because direct `lowir2native` does not
itself perform PA32 optimization, and it also checks the combined
`cppgm++ -O3` path and generated behavior.

After each eliminated color, recompute complete MIR liveness. An eliminated
color cannot become a later destination, and a surviving callee-saved color
used as a destination cannot itself be eliminated. This monotonic policy
bounds the iteration by the five available callee-saved GPR colors and avoids
recoloring cycles. O1 and O2 retain the existing whole-function policy. With
`--stats`, report `machine_opt_block_recolor_candidates`,
`machine_opt_block_recolor_registers`, and
`machine_opt_block_recolor_blocks`; all three remain zero below O3.

O3 also performs two bounded common-path memory rewrites after register
liveness is complete:

- Two adjacent, nonvolatile 64-bit field transfers within one block may become
  one direct 16-byte copy when both source fields and both destination fields
  are adjacent ranges of the same base object and the complete 16-byte source
  and destination ranges do not overlap. The scalar transfers may be direct,
  or each may pass through a distinct private frame stage whose binding has
  exactly that store and reload and is not debug-visible. The resulting fixed
  copy keeps its direct base-plus-offset operands and does not consume those
  pointer carriers. Different or unproved bases, overlapping or nonadjacent
  ranges, volatile accesses, an escaping or multiply used stage, and any
  intervening instruction or later scalar-carrier use retain scalar
  transfers. O0 through O2 retain the scalar form
- When a scalar load is immediately stored to a private frame stage, reloaded
  only to drive a zero guard, and all remaining uses of that binding are
  nonvolatile reloads in the sole one- or two-block fallthrough arm, O3 may
  test the defining register and move the store to the beginning of that
  consuming arm. Every block on the path must have one direct predecessor and
  must leave the carrier intact. A bypass therefore performs no eager frame
  traffic, while the consuming arm still receives the original representation
  before any reload. A debug-visible, volatile, escaping, multiply written,
  externally reachable, carrier-clobbering, or otherwise unproved stage is not
  moved. O0 through O2 retain the eager store and guard reload

After final O3 machine cleanup, a function containing at least 64 MIR
instructions requests 16-byte native entry alignment. Smaller functions and
all functions at O2 and below retain the two-byte minimum needed by the
existing native layout. Record the requested alignment in serialized MIR and
honor it in executable emission and relocatable-object text partitioning,
including separate weak/COMDAT text sections. The policy is based only on the
final machine-function size; it must not recognize source names or particular
programs.

All levels must preserve valid debug metadata. Optimizations may choose to be
more conservative when a rewrite would make source locations misleading.


### Design Notes

Whole-function placement can reuse the dense LowIR definition, use, last-use,
call-clobber, and edge-liveness facts built for baseline lowering. A compact
interval table indexed by LowIR value ID and fixed-size tables indexed by
physical register keep allocation linear or near-linear without string keys.

Treat ABI argument and result registers, division and shift registers, calls,
and exception edges as explicit constraints. Prefer a callee-saved register
only when its save/restore cost is lower than the frame traffic it avoids.
When a proof is unavailable, retain the ordinary frame-home path. The
optimized machine-IR dump must contain the final physical registers, frame
bindings, edge copies, and callee-save list used by native encoding.

A definition-time frame copy can be reused if acyclic pressure later evicts a
retained register. Inside a cycle, earlier machine instructions execute again
after a backedge, so plan enough headroom up front instead of changing their
assumed location while lowering a later instruction.


### Bounded string-length prefix

   At `-O1` or higher, a direct one-pointer call returning `i64` whose LowIR
   declaration or definition carries `object=cppgm_builtin_strlen` may select
   a bounded native prefix operation. MIR records that selected machine fact
   on the call as `strlen_prefix=16`; `-O0`, ordinary functions, indirect
   calls, and incompatible signatures retain the ordinary call form.

   The x86-64 encoding may inspect one 16-byte SSE2 word only when that load
   remains within the current 4 KiB page. If the word contains a zero byte, it
   returns the first zero's byte offset. A page-edge address or a prefix
   without a zero retains the original direct call as its fallback. The MIR
   operation keeps the conservative call argument, clobber, unwind, and result
   facts because the fallback is still a real call; its vector temporaries are
   caller-saved encoding scratch rather than allocator-visible values.
