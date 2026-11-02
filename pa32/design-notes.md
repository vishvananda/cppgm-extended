# PA32 design example

These notes explain one implementation. The [assignment README](README.md)
and course fixtures define the required contract and quality bar. The pass
order, data structures and numeric limits below are design choices.

### One Design: The Course Solution's Optimization Levels

Everything in this section describes the course solution. It is one way to
meet the quality bar, and its numbers (the 1,536-instruction caller budget,
the 40-, 24- and 6-instruction callee caps, the unrolling limits and the
rest) are its choices, recorded so that its outputs can be
explained. None of them is a requirement; the course fixtures do not
compare your output's shape with the course solution's.

`lowiropt`, `cppgm++ --emit-lowir -O<n>` and the object paths
(`cppgm++ -c -O<n>` from source or from LowIR text) run one optimizer over
the typed program that the LowIR reader or the lowering builds:

- `-O0` runs no transform; the program is written back in canonical form.
- `-O1` runs three inlining waves over the direct-call graph. The first
  wave runs the per-function pipeline below on every function, the later
  waves on the functions they change, and each wave is followed by pruning
  of unreachable definitions.
- `-O2` runs the same waves with larger single-call limits and more
  admission rules, adds memory, loop, switch and branch rewrites to the
  per-function pipeline, then propagates arguments into internal functions
  and makes a final pass over every function.
- `-O3` is `-O2` plus full unrolling of small counted loops and value
  numbering across back-to-back diamonds.

Every pass bounds its work: it is close to linear in the size of the
function, or a stated cap limits it, and reaching a cap only makes the pass
do less.

### The Per-Function Pipeline

1. Slots and scalar rounds. A round splits object slots into fields,
   promotes scalar slots to SSA values, then runs scalar rounds (below).
   At most four rounds run; after the first, a round in which splitting and
   promotion change nothing ends the loop, as does running out of slots. A
   function with an object slot runs scalar rounds before its first split,
   so the copies that fill the slot are simplified first; that run counts
   as one of the four rounds.
2. Remove protected regions that nothing can unwind out of, and coalesce
   scalar copies. If either changed the body, split and promote again and
   run scalar rounds.
3. Rewrite loops; at `-O3` unroll loops; at `-O2` and above form switches.
   Scalar rounds follow if any of these changed the body.
4. Record whether the function can unwind, for its callers.
5. At `-O2` and above: let a local that every return copies whole into the
   indirect result become that result, then rewrite branch compares.
6. Compact the body, so a caller copies only live code.

A scalar round runs sparse conditional constant propagation (Wegman and
Zadeck, over executable edges), CFG cleanup, value simplification, memory
forwarding, dead-code removal, jump threading and CFG cleanup again. Rounds
repeat until one changes nothing, at most four times. A one-block function
runs value simplification, memory forwarding and dead-code removal instead
(at most four rounds), then one CFG cleanup. A pass that found nothing to
do is not run again until the body changes.

**Value simplification** walks the dominator tree once per run:

- It folds `copy`, `unary`, `binary`, `cmp` and `convert` of constants the
  way the back end computes them: integers wrap to their width; division,
  remainder and shifts fold only where they cannot trap or depend on an
  out-of-range count; `f64` arithmetic is x87 extended precision rounded
  to double.
- It applies algebraic identities (`x + 0`, `x * 1`, `x & -1`, `x - x`, a
  mask that clears no bit `x` can have, and so on, and for `f32` and `f64`
  `x * 1`, `x / 1`, `x - 0` and `x + -0`, but not `x + 0`, which turns `-0`
  into `0`), folds a `copy` of a floating value to an integer and back to
  that value, reassociates `add`,
  `mul`, `and`, `or` and `xor` chains with constants, merges constant
  `index` steps, orders commutative operands, collapses a phi whose inputs
  are all one value, and merges two phis of a block that agree on every
  input.
- A truncation of `add`, `sub`, `and`, `or`, `xor`, a `shl` by a literal below
  the narrow width, or a `mul` to two bytes or more, whose operands are
  literals or extensions of the truncation's own type, becomes that operation
  in the narrow type. C promotes `b |= 1` to `int`, so this leaves one
  instruction where three would stop a select from forming.
- It reuses a pure computation already made in the same or a dominating
  block (hashed by operation, type and operands).
- When a block's only predecessor ends in a branch, the edge's facts hold
  in every block the entered block dominates: the condition is nonzero on
  the true edge, and an edge that proves `x == k` for a literal `k` (`eq`
  taken, `ne` not taken, and unsigned tests that pin `x`) replaces `x` with
  `k`. A two-input phi that picks the value its edges prove equal
  (`min(n, max)`, `max(n, 0)`) becomes that value.
- A branch on `s <= k` for a finite literal `k` (or `ugt` not taken), where
  `s` is a square or the sum of two squares, proves each square and the value
  squared finite there: a square is at least 0 or a NaN. A sum, difference
  or product of finite values is no NaN, so `d != d` folds to false. Complex
  multiplication tests its real part so before calling `__muldc3`; in the
  Mandelbrot iteration over `std::complex`, which took 9% fewer cycles, the
  call goes.
- A nonvolatile integer or pointer load at a constant offset into a global
  of `storage=readonly`, or into an internal global the unit never writes
  (every use of its address reads through it and no other global's data
  holds it), becomes the stored constant or address; replaceable and
  thread-local globals are never read this way. `strlen` of such byte data
  with a known NUL becomes its length (at most 4,096 data items), and
  `memset(p, 0, n)` and `memcpy(d, s, n)` with a literal `n` of at most 128
  become `zeroinit` and `copyobj`; so does `memmove(d, s, n)` when its two
  ranges are in different slots or globals, or `n` apart in one.
- Two addresses at known offsets from one base (a local array's begin and
  end) subtract to the difference of the offsets and compare as the
  offsets do. `std::copy` of an array then moves a literal number of bytes.
- At `-O2` and above an indirect call whose target is the `addr` of a
  function becomes direct, and `memcmp` of a literal length of at most 16,
  compared only with zero, becomes loads of one or two chunks of each side.

**Memory forwarding** walks the dominator tree with facts: the value a
nonvolatile load read or a store wrote at an address and type, and byte
ranges known to be zero. A load reuses a fact of its address and type (or
of the other integer or pointer type of that width, through a retyping
copy); a load in a known-zero range reads zero.

- An address is a slot, global or pointer temporary plus a constant offset.
  Distinct slots and globals never overlap, nor do disjoint ranges of one.
  A private slot, whose address reaches only loads, stores, pointer copies,
  constant `index` steps, `copyobj` and `zeroinit`, is reached by no other
  pointer. Two `typed` accesses of incompatible types, neither of a byte
  type, name different objects unless they share a base.
- Two addresses from one base by multiples of one stride, with ranges that do
  not meet modulo that stride, do not alias: they are different fields of an
  array's elements. Loop motion and block lanes use this too. A product is a
  multiple of its literal only if it cannot wrap: its other factor is `nowrap`
  or extended from 32 bits. A wrapped product keeps only the literal's power of
  two.
- A write ends every fact it may alias. A call that may write memory, an
  atomic operation, `va_start` or `va_arg` ends every fact except those of
  private slots. At most 64 facts and 16 zero ranges are live.
- A block inherits its immediate dominator's facts less whatever the
  blocks between them may write (at most 32 such blocks, none a handler).
  A handler block starts with no facts.
- Object copies whose source bytes are known constants (at most 256 bytes)
  or known stored values (at most 32 bytes) become zeroing and stores;
  writes to a private slot that nothing reads are removed.
- After the walk, a load that precedes every write in its block can become
  a phi: through a phi of addresses, when every edge's value is known; at
  a join of at most 8 predecessors, when some know the value and the
  others end in a jump to the join and receive the load (no edge is
  split); or at most 3 single-predecessor blocks below a loop header, when
  every predecessor of the header knows the value.
- At `-O2` and above a store of the value the location already holds is
  removed, a store to a slot goes when a later store in its block replaces
  all of it and nothing between them may read it, and a store to a slot
  that is dead on one side of a two-way branch moves to the other side when
  that side has no other predecessor.

**Dead-code removal** marks from the instructions that must stay and
removes the rest: unused pure values, nonvolatile loads, stack allocations,
and calls whose boundary is `readnone` or `readonly`, `unwind=no` and not
`noreturn`, dead phi cycles included.

**CFG cleanup** (at most 8 rounds per run) ends a block after a `noreturn`
call, turns a branch or switch with a literal selector or a single target
into a jump (a switch compares at its selector's type), removes unreachable
blocks and conditional edges into a block that is only `unreachable`,
bypasses jump-only blocks other than the entry and handler blocks (into a
phi block of at most 32 predecessors, when the phi inputs agree), and
merges a non-handler block into its only predecessor when that predecessor
jumps to it.

**Jump threading** lets a block whose only predecessor branched on `c` take
the known side of its own branch on `c`, and lets a predecessor whose phi
inputs decide a small block's branch (at most 8 instructions besides its
phis) jump to the decided successor. At `-O2` and above, facts from
dominating branches fold the compares they decide, and a predecessor whose
recent path decides a test block's compare against a literal jumps to the
decided side (copying at most 6 of the test block's instructions). A branch
on a phi goes when every input is a nonzero literal, a value its edge is
entered only when nonzero, or another such phi. The search assumes all phis
qualify and drops those with a failing input, so a loop's phis hold each other
up. At `-O3` it runs again after loop rotation, which moves tests to latches.

**Slot promotion** turns a scalar slot (any integer up to 64 bits, `ptr`,
`f32`, `f64`) into SSA values when every use is a direct nonvolatile `load`
or `store` of its type, of the other integer type of its width, or, between
`f32` and a 32-bit integer or `f64` and `i64`, of the floating or integer type
of its width. A read of the other kind is a `copy` of the bits; a literal
changes kind with the same bits. Phis go
at the iterated dominance frontier of its stores over ordinary edges,
semi-pruned (a slot no block reads before storing gets none). One run's
phis have at most 4,096 inputs plus four per other instruction; past that,
the remaining slots keep their memory. A slot that would need a phi in a
handler block stays a slot (see also Safety Rules).

**Slot splitting** replaces an object slot with up to 16 typed field slots
and promotes them. Every use of its address (through `addr`, constant
`index` steps and pointer copies) must be a nonvolatile scalar `load` or
`store`, a `copyobj` or `zeroinit` at a constant non-negative offset, or a
call's indirect-result argument, and the typed accesses must agree where
they meet, except that an integer may carry a floating field of its width (a
small object passed by value is often read in its integer carrier): the field
stays floating. Copies and zeroing then expand per field. At `-O2` and above an
address-taken scalar slot is rewritten the same way, and eightbytes that
copies move whole may become one integer field.

**Region removal** carries the stack of active regions over ordinary edges
from the entry. A region is kept when an instruction inside it may unwind:
a `throw`, a `resume`, or a call that may unwind and is not to a callee
proven not to. Other regions lose their opener and `eh_end` instructions,
and their handlers become unreachable. Conflicting stacks at a join, or
catch and exception instructions in ordinary code, keep every region. At
`-O2` and above a cleanup whose handler only ends the region and resumes
goes even when the region can unwind.

**Copy coalescing** merges adjacent nonvolatile scalar load/store pairs
that copy a contiguous range between storage that cannot overlap (distinct
slots or globals, a private slot, two `noalias` parameters) into one
`copyobj`, and forwards copies made through a temporary slot. At `-O2` and
above an object slot of at most 64 bytes written only with zeros is zeroed
whole, and a slot filled by one copy from constant data and otherwise only
read reads that data instead.

**Loop rewriting** works on natural loops found from back edges into a
dominating header; irreducible loops and loops containing a handler block
are not reported. It makes one rewrite per scan, at most 8 scans per run,
and tries at most 16 candidate loops per scan, innermost first:

- A loop with one latch and one exit, tested by `ne` or `eq` of an
  induction variable against an invariant, whose body holds only phis,
  nontrapping pure values and control flow, and whose values are unused
  after it (except the tested value, which equals the bound), is bypassed.
- A fill loop, one nonvolatile store per pass of a repeated-byte value
  through a pointer induction stepped by the store's size, becomes a call
  to the runtime's `memset` builtin in the preheader.
- At `-O2` and above a copy loop between two pointer inductions becomes a
  `memmove` builtin call (the loop still runs when the destination starts
  inside the source), and a counted loop of at most 64 passes and 4 stores
  of literals or of values loaded from storage it does not write becomes
  preheader stores or copies (at most 256 bytes each). A few smaller
  rewrites widen fills, replace count-down counters with pointer tests and
  hoist a load that the loop only stores back.

**Branch compares** (`-O2` and above): a branch on an expanded `memcmp`
equality tests its chunks one at a time; a branch on a compare of a phi
with at most 4 predecessors gives each predecessor its own compare; a
compare used only by branches in other blocks is computed again beside
each of them. **Switch formation** (`-O2` and above) turns tests of one
value against literals (at most 24 blocks, 256 case values) into a
`switch` when that needs fewer runs of consecutive values per target.

**Tail recursion** (`-O2` and above): a call of the function itself followed
only by the return of its result becomes a jump to the entry, whose phis take
the arguments. The rewrite needs a fixed parameter list of scalars, no `eh_*`
instruction, no `stack_alloc` and no slot address that leaves the frame's own
loads and stores: a loop has one frame where the calls had many.
A call whose result an integer `add` then returns (`return a + f(x)`) becomes
the same jump with an accumulator. The accumulator is a phi of the entry that
starts at zero. That site adds `a` to it, and every other return adds it to
its value. Integer addition reassociates, so these adds drop `nowrap`. A
floating add does not reassociate and keeps its call. A recursive Fibonacci
function makes half its calls this way, which took 38% of its cycles off.

**Strength reduction** (`-O2` and above): in a loop without calls, the `mul` of
a counter by a value the loop does not compute becomes a counter of its own,
stepped by the product of that value and the step. A call on a rare path (see
the floating literals below) does not count. The counter may step by a literal
or by a value the loop does not compute. A literal that an address scale or
one `lea` gives (1, 2, 3, 4, 5, 8 or 9) stays a `mul`; 16 takes a shift and a
scale. A sign extension of a literal-step counter that does not wrap may stand
between them, or an `add` of an invariant to the counter: the new counter
starts at the product of the sum. Each loop gets at most 8 such counters. An
FFT's butterfly loop, `k += m` over 16-byte complex values at `k` and
`k + half`, lost three instructions of 34.

A counter of an invariant step that only additions of invariants read starts at
their sum, and the additions go. The counter's own `add` keeps `nowrap` only when
those additions had it and ran on every pass.

### Library Calls (`-O2`, `-O3`)

Value simplification rewrites calls of the C library's math functions that a
unit declares and does not define:

- `fabs` and `fabsf` clear the sign bit (`copy`, `and`, `copy`).
- `abs`, `labs` and `llabs` select the negation.
- `pow` and `powf` of a literal exponent 2, 1, 0 or -1 are `x * x`, `x`, `1`
  and `1 / x`.
- `sqrt` and `sqrtf` become `unary sqrt` when the operand cannot be negative:
  a square, a `sqrt`, a literal, or a sum, product or quotient of such values.
  Another operand keeps the call, which sets `errno` for a negative operand.

The final pass joins `sin` and `cos` of one value (or `sinf` and `cosf`) into
one `sincos`, which is about as costly as one of them. The first call of the
pair must dominate the others, which then read the two results.

### Inlining and Pruning

The direct-call graph gives every function definition a dense index; its
strongly connected components mark recursion (a function on a call cycle)
and give a callee-first order. Each wave visits the functions in that
order, deciding the calls in a function before visiting its callers.

- The ordinary wave inlines admitted calls into each function and then
  runs the per-function pipeline on it, so every callee is simplified
  before its callers decide.
- The single-call wave, after pruning, applies the single-call policy
  (forced callees and the `-O2` loop rules below also apply): removing
  dead callers can leave a definition with one call.
- The late wave admits small optimized bodies with a fresh caller budget.

The later waves run the pipeline only on functions they changed. `lowiropt`
and `--emit-lowir` leave a this-adjusting thunk (object name `_ZTh...` or
`_ZTv...`) as lowered.

A callee is never inlined when it is on a call cycle (apart from the
recursive rule below), is `no_inline=yes` or replaceable, is variadic,
contains `va_start`, `va_arg`, `stack_alloc` or an EH instruction
(`eh_*`, `throw`, `resume`, `exception`, `exception_selector`; but see
folded copies below), or does not match the call's arity, result type and
parameter passing. At `-O2` and above an EH callee may go inline when its
handlers only clean up and resume (no `eh_catch`, `eh_filter`,
`eh_catch_all`, `exception_selector`, `exception` or `throw`), the caller
has none of these either and may unwind, and the call is not in handler
code: the copy's records nest in the caller's frame and a resume continues
to the caller's own. The ordinary and single-call waves take such a callee
only at a loop site; the late wave takes it at any site. A callee whose only
catches are terminate guards (a landing of `eh_catch_all`, `exception`, a
call that neither returns nor unwinds, and `unreachable`: a `noexcept`
function's) and that cannot unwind may go inline at any site outside handler
code, also inside the caller's own regions: the copy's guard catches all
that the copy raises. {fmt}'s small `noexcept` helpers, each with an
assertion that may print, then went inline, and its formatting loop took
11% fewer cycles. (A copy grows its
caller, which can then miss the caps of its own callers: tramp3d's kernels
ran twice as many instructions when every site took one. Single-call copies
at other sites made a Boost unit's `-O3` compile 6% longer for no net gain
in run time.) In handler code (blocks a handler entry dominates) a
callee that may unwind stays a call. A callee without EH instructions may
be inlined inside an active region; its cloned calls stay in that region.

Sizes count a callee's instructions outside cold blocks (blocks from which
every path ends in a `noreturn` call); cold code is copied but not charged.
At a call site inside one of the caller's loops the ordinary and late-wave
caps are four times larger.

| policy | `-O1` | `-O2`, `-O3` |
|---|---|---|
| `force_inline=yes` | always, not charged | |
| trivial leaf (no call), ordinary wave | 4, not charged | |
| `inline_hint=yes` | 96 | |
| one block, one return, no call | 40 | 48 |
| other, no call outside cold code | 24 | 48 |
| calls outside cold code | 6 | |
| per-caller budget, each wave | 1,536 | |
| single-call callee | 512 | 1,024 |
| single-call budget per caller | 1,024 | 2,048 |
| late wave: leaf / hinted / other | 40 / 48 / 6 | 48 if no call |

In the late wave at `-O1` a one-block leaf uses 40 even when hinted. The
ordinary wave charges the larger of a callee's input size and its current
size. In both waves a loop-free multi-block callee whose blocks reachable
from the entry without a call include a return is also admitted when
those blocks hold at most 24 instructions (48 if hinted) and the callee at
most 96 (160 if hinted), unless its hot code calls an allocation function.
A callee with a loop and more than one call site is not admitted by the
size caps unless it is hinted (or, at `-O2`, call-free within 48).

The single-call policy, used by the ordinary wave when its caps reject a
call and by the single-call wave, takes a discardable callee (weak or
internal, no lifecycle role, not `object_root=yes`, not replaceable, not a
thread-local wrapper) with exactly one direct call and no other reference;
pruning then removes its definition. At `-O2` it also takes an internal
callee whose calls all lie in one function, when the copies together fit
its cap.

Three unit budgets bound growth, each the input's instruction count but at
least 10,240: one for single-call copies, one for copies of
`force_inline=yes` callees, and one for all other copies.

In the ordinary wave a callee with EH instructions (at most 256
instructions; with a loop only if hinted) that receives a constant
argument, an integer literal or the address of a readonly non-replaceable
global, may still fold away: a private copy receives the constants and runs
the per-function pipeline, and it replaces the call when no EH instruction
remains, it has at most 40 instructions (80 if hinted), it fits a budget of
2,048 such instructions per caller, and in handler code it cannot unwind.
A callee whose copies fail 16 times is not copied again in that wave.

At `-O2` and above the late wave also makes partial copies: for a callee
kept out of line (scalar or no result, no object or indirect-result
parameter), its side-effect-free entry and fast returning paths, calling
the callee where work begins (at most 24 instructions; 40 if hinted, 48
with at most two call sites, 8 if its hot code allocates). No copy holds a
loop: the call where work begins would run the loop again. A loop-free,
EH-free function of at most 64 instructions that calls itself at least
twice takes copies of its own body at those calls, two levels deep at most
and 320 instructions in all. A function of that size with a loop and one call
of itself takes them eight levels deep, nesting the loops.

Further rules adjust admission by call site and caller. For example, at
every level a small caller (at most 32 instructions) with a call-free path
to a return takes no callee larger than 16 except forced ones and the
function-pointer loops below, so that it stays inlinable. At `-O2` and
above a call in a block from which every path ends in a `noreturn` call or
`unreachable` takes only forced callees and callees of at most 4
instructions; callers the unit cannot inline elsewhere accept somewhat
larger callees with few call sites; and a callee whose loop calls through
a function the site passes (at most 96 instructions), or a loop around one
call (at most 12), may be copied.

Pruning follows each wave. The roots are every function definition that
is not discardable and every symbol named in global data; definitions
reachable from them through any symbol operand stay, and the other
discardable function definitions are removed.

### Virtual Calls

The lowering, not the optimizer, guesses a virtual call's target: it knows
which functions the unit overrides. It does so in every object compile, as
the LowIR of `-c` does not depend on the level (see `tests/object-roundtrip`),
and in `--emit-lowir` from `-O2`. A call of a defined, non-pure,
non-variadic function that nothing in the unit overrides compares the
table's entry with the function's address. Equal, it calls the function
directly, and the inliner can see the body; otherwise it calls through the
pointer. A wrong guess costs one compare, so the guess needs no proof: a
class of another unit that overrides the function takes the indirect path.
A function with an overrider keeps its plain indirect call, since such calls
are often wrong. Destructors and calls returning a class object are left
out. Both calls sit in one exception region, so that a throw from either
runs the same cleanups.

### Small Class Copies

In the same cases, the lowering moves a class of at most three scalar members, bases,
array elements and complex parts included, by a typed load and store per
member. A `copyobj` has no type, so it may alias every other access. It
would stop a loop from hoisting a `typed` pointer load past it. Separate
accesses also forward from the stores that made the object, where one wide
load of two narrower stores stalls. The whole `copyobj` stays for a union, a
bit-field, a table pointer, a `long double`, a `packed` or `may_alias` class,
and an object that holds a volatile member, since these need all their bytes
or have no single type for them. Padding is not copied, and need not be.

### Callee Copies (`-O2`, `-O3`)

After the ordinary wave, a call that passes a defined function for a
parameter its callee calls through calls an internal copy of the callee that
reads that function instead. The callee must have fixed arity, at most 400
instructions, and no `no_inline` or `replaceable` mark. The copy's own calls
that pass the function on, including a recursive call to the callee, call
the copy as well. The later waves can then inline the function into the
copy. A unit makes at most 32 copies, in two scans of its calls. A sort that
takes a comparison function is the usual case: in such a benchmark, the
copies took 18% of the instructions off.

### Speculative Devirtualization (`-O2`, `-O3`)

After the ordinary wave, and before the callee copies, a call is rewritten
when its callee is a load from a slot of the table that the first
argument's first word points to, and a store of a readonly table's address
into that object (its construction) dominates the call. The pass reads the
function that the latest such store's table holds in the slot. The function
must be a definition of at most 64 instructions, with no `no_inline` or
`replaceable` mark, and with the call's arity and result type. The call's
block then ends at a branch on the loaded callee being that function. One
side calls the function directly and the other side keeps the call through
the slot; a phi joins the results. The later waves can inline the direct
call. A function gets at most 8 such rewrites. A loop that calls a virtual
member function of an object it constructed before the loop is the usual
case: a call in the loop can replace the object's table pointer, so the
table must be loaded again on each pass.

### Argument Propagation (`-O2`, `-O3`)

After the waves, an internal function that is called, has parameters, no
reference other than direct calls (no address, data reference, lifecycle
role or object root), no recursion and fixed arity takes a parameter's
value when every call passes the same literal or symbol for it, then runs
the pipeline again. Parameters its body does not
read leave the signature and every call, whose callers run the pipeline
again; the boundary is internal, so no other unit sees the change. A call
with a stated signature or another argument count disqualifies the
function. Pruning follows.

### Read-Only Functions (`-O2`, `-O3`)

After argument propagation, mark the functions that write no memory their
callers can see. Such a function:

- writes only slots whose address stays in the function,
- does no volatile or atomic access and no exception work,
- is not replaceable, and
- calls only functions of this kind, or declarations stated `readonly` or
  `readnone`.

Start with every definition marked. Remove the mark from each function that
fails, then from the callers of each unmarked function, so call cycles keep the
mark. One scan of the bodies and one walk of the call graph are enough.

Forwarding, load hoisting and the other memory passes then treat a call to a
marked function as a reader: loads move across it, and stores it may read stay.
The mark is not written to LowIR text. Run forwarding again at the start of the
final pass in each function that calls a marked function, since the earlier
rounds saw those calls as writes.

### Final Pass (`-O2`, `-O3`)

After argument propagation every function gets these rewrites; if any of
them changed it, branch compares are computed again beside their branches:

- `i128` values become pairs of `i64` halves for logic, addition,
  subtraction, shifts, extensions, truncations, compares, selects, phis and
  whole loads and stores; other uses reassemble the value.
- A branch to a diamond or triangle whose arms hold at most two
  speculatable integer instructions each (no load, division or floating
  point) becomes `select`s, at most two per merge, for at most three rounds.
  A diamond with work on both arms makes only one: for two values, the branch
  runs one arm where the selects would run both.
  A merge that picks between the two `f32` or `f64` values its compare reads
  becomes `vector` `min` or `max` of those values. Those operations give the
  second operand when unordered, and compare `<` or `>`, so `<=` and `>=` need
  a literal other than zero on one side: only then is the result of equal
  operands known.
- Loads move out of loops where nothing but `store`, `copyobj` and
  `zeroinit` may write memory (no writing call, atomic operation,
  `va_start`, `va_arg` or EH instruction) and no load is volatile. A load
  moves to the preheader when no write in the loop may overlap it and it
  either runs on every entry (in the header before any call or, in a loop
  with no call, in a block on the way to every exit) or reads a
  slot, a global, or a parameter's `object_bytes` extent. A load of the second
  kind may be read on a path that had no need of it: a loop moves at most 10 of
  those, less the values already live through it (its header phis and the
  outside values it uses). Invariant integer arithmetic, one of
  whose operands is used nowhere else, moves too. So does an invariant
  conversion of an integer other than `i128` to `f32` or `f64`, whatever else
  reads its operand: it cannot trap, and its result costs less to keep than
  to convert again.
- Pure instructions whose operands a loop does not compute then move to its
  preheader, a phi or a compare excepted. An integer division needs a
  literal divisor other than 0 and, signed, other than -1, unless it runs on
  every entry to the loop (in the header, before anything that may leave).
  Load motion then runs again, up to five rounds: a pointer read through a
  chain of tables needs its arithmetic moved before the next load can move.
  In a loop with a call only multiplication and division move, since the
  result would have to survive the calls. An address that reaches only loads
  and stores, through literal steps, stays: it fits in their operands. `f80`
  values and objects stay in memory, and `i128` operations are runtime calls
  at this stage, so they stay in the loop.
- A fill loop of a run-time value whose bytes all repeat calls `memset`
  for runs of at least 128 bytes.
- Narrow loop counters that cannot pass their bound are marked `nowrap`.
- Last, a pure instruction other than a phi moves down to the block nearest
  its readers, one in no deeper loop than its own, and sits before its first
  reader there; a phi's input is read at the end of its predecessor. Visit
  blocks in reverse order so that a reader has moved before its operands are
  placed: one walk moves a whole chain. A path that reads nothing computes
  nothing, and a step ends up beside the test that reads it, which then needs
  no compare. A handler or a cycle not reported as a loop takes nothing.
- A loop without calls on its usual paths reads its floating literals, at most
  eight, from `copy` instructions before the outermost such loop: a register
  needs no load per iteration. Zero, a stored literal and a phi's input from
  outside stay. A call behind a test for not a number, or for a negative value
  under a square root, is a rare path: the allocator saves around it.
- A compare in a loop reads a symbol's address, at most four of them, from an
  `addr` in the block that dominates the outermost loop's header. Calls do not
  stop it: a general register survives a call. Read in place, the address
  takes a `lea` on every pass; a loop that tests a function pointer against a
  known target before each call ran 8% fewer instructions.
- A floating square root or division takes many cycles, and an instruction
  that reads it waits in the processor's scheduler, where it can keep the next
  root from starting. Walk each block in order. Before an instruction that
  reads a root or quotient that nothing has read yet, move up the next ones
  of the block with their pure operands, at most 24 instructions each, until
  four are unread. Nothing moves past a call, since no floating register
  survives one, and an integer division stays, since it may trap. The n-body
  step, ten roots and ten divisions in one block, took 11% fewer cycles.

### What `-O3` Adds

Full unrolling, at most 16 loops per run of the pass:

- A loop with a straight-line body (a chain of jump-ended blocks), one
  latch and its exit test in the header, whose trip count is known without
  wrapping and at most 4, and whose body times the trip count plus one is
  at most 64 instructions, becomes one block. With no call in the body the
  limits are 8 passes and 256 instructions. A loop entered from a branching
  block gets a preheader first. A pointer induction counts too when its
  start and its bound are known offsets from one slot or global (`std::fill`
  over a local array): the trip count is the distance over the step, and
  the bound may be an address the header computes.
- An innermost loop with a branching body, an integer induction and a
  literal start, step and bound giving 1 to 16 passes is copied pass by
  pass: up to 16 passes within 256 cloned instructions when it reads
  constant data or branches on values computed from the induction, or
  calls nothing and touches no frame slot; otherwise up to 8 passes within
  160 when some path through the body back to the header calls nothing.
  A body that indexes a local array by the induction may copy up to 640
  instructions: each copy names one element, and the elements then become
  registers.
- A loop containing an EH instruction, `stack_alloc` or `va_start`, or one
  that exits into a handler block, is not unrolled.

A large function holds hundreds of loops once inlining has run. The loop
passes below rewrite, in one round, every loop that does not touch another
one rewritten in that round, and only then find the loops again. So a loop
pass costs a few walks of the body, not one per loop, and no fixed count of
loops per function limits it.

Loop rotation: at `-O3` a loop whose header holds only its phis, at most four
plain instructions and a branch out of the loop is rotated. Each latch ends in
a copy of the test, and the header stays as a guard, which folds when its
operands are literals. What the body does on every pass then runs before every
exit. A phi that takes another phi of the header keeps the loop as it is.

Store motion: after rotation, a loop with no call, volatile access or
atomic operation keeps in a register each of up to four locations it stores
on every pass. The location has a loop-invariant address, which may have a
step of unknown size, an `int`, pointer or `f32`/`f64` type, and nothing else
in the loop may overlap it. Accesses to two different named objects do not
overlap, whatever their steps. A store of it
must dominate every latch and every block that exits, so the copy on the
exit writes nothing a run of the loop would not have written. The location is
loaded in the preheader and stored on each exit edge. The stores this leaves
may then leave the loop around it.

A loop with calls, none of which runs on every pass (each call's block fails
to dominate some latch), or a loop with no location stored on every pass,
keeps in a register each of up to four locations it loads instead. The
location has a loop-invariant address of known offset and the same types as
above, and no other store of the loop may overlap it. Loads of other
locations may overlap it. Storage that a call of malloc, calloc or `operator
new` in the function returned is new: a pointer that comes, through steps,
copies and phis, only from parameters, slots, globals or other such calls
does not reach it, so a store into it does not overlap a load through that
pointer. The load before the loop must not fault: the location is in a slot
or a defined global; or the function reads or writes it at the same type in
a block that dominates the loop's header, with no call after that access or
in the blocks between it and the loop (a call might free it); or, in a loop
without calls, the loop reads or writes it on every pass before it can
leave. In huffbench's encoder this keeps the input byte out of memory in the
loop that writes the output bits, and took 9% of the cycles off. In a loop without
calls, a slot's location stays in memory (the lanes of a local array that an
unrolled loop updates cost more registers than loads). Its stores stay where
they are. The register takes
the value of each store, and the location is loaded again after each call,
which may have written it. A seed that a loop updates through a global then
stays out of memory on the chain from one pass to the next, and so does a
value a loop stores only on some passes. A call that runs on every pass
would reload it on every pass, and the loop is left as it is.

Counter widening: registers hold narrow values extended, so each `int` addition
that feeds an index costs an extension. An `int` counter whose step cannot wrap
equals, extended, a 64-bit counter of the same step, and so does its sum with
values the loop does not change, if those additions cannot wrap either. Widen
such a counter when an `index` or an extension reads it or such a sum. A value
in the sum goes into the base before the loop, and a literal into the access.
What still reads the `int`, such as the exit test, reads a `convert trunc exact`:
the wide value fits, so the truncation needs no instruction and its reader can
use the wide register. Run this after strength reduction, which reads the
narrow products. An unsigned counter that cannot wrap widens by zero extension.

Square roots: `sqrt` differs from the instruction only where it sets `errno`, for
a negative operand. Emit the instruction always and call on the negative path
alone. The cold call then costs little: nothing else in the loop has to live in
memory around it, and of all memory it writes only `errno`, an `int`, so loads
of other types can still leave the loop. Skip the test when the operand cannot
be negative.

Loop entries: a loop entered from several blocks has no preheader, and most loop
passes need one. Give it one first: a new block the entries jump to, with phis
that merge the values they bring. A header phi that only keeps its value then
folds, and the loop can rotate.

Packed parts: a pair of `int` fields copied as one 64-bit value is two values
or-ed together. A truncation or a right shift needs only the operand with bits
it keeps, so a field read back from the pair is the value stored in it.

Quotients: `x % y` next to `x / y` is `x - (x / y) * y`, so one divide serves
both, for a variable divisor as well. When the remainder comes first, as in the
digit loop `d = x % 10; x /= 10;`, move the quotient up to it.

Private slots: a slot whose address reaches only its own accesses, copies and
constant steps is reached by no other pointer. A write to such a slot does not
keep a load through a pointer in its loop.

Allocations: memory forwarding treats the result of `malloc`, `calloc` or
`operator new`, when the callee is declared and not defined here, as storage no
earlier pointer reaches. It is apart from every slot, global, parameter and
other allocation. An allocation whose address never leaves, by a store, a call,
a phi or a return, is apart from every other pointer and survives calls. The
allocation call itself ends only facts about an `int` behind a pointer, which
may be `errno`.

Possible bits: the simplifier tracks which bits a value can have through masks,
shifts, extensions, truncations and remainders by a literal. A value below a
positive literal divisor is its own remainder, and its quotient is zero. A
compare against a positive literal above every possible value folds. A signed
division or remainder of a value that cannot be negative, by a power of two, is
a shift or a mask. A signed halving of `x * (x + k)` for an odd `k` is an
arithmetic shift, as one factor is even whatever wraps. Floating `add` and `mul`
commute for value numbering: only which NaN payload comes out could differ. A
floating division by a power of two whose reciprocal is exact (`x / 2`) is a
multiplication by that reciprocal, which rounds alike and costs far less. An extension of a truncation that lost no bits is the
original value.

Step sinking: in a one-block loop, a counter's step that runs before the last
read of its phi holds both values at once, and the back edge must copy one to
the other. The step moves after that read, with the compare only the branch
reads when that reads the step. Nothing else may read the step before there.

Loop unswitching: at `-O3`, after rotation, an innermost loop of at most 64
instructions with a branch that has both sides in the loop and tests a value
the loop does not compute, or a compare of two such values, is copied. The
preheader makes the test, and each copy keeps one path. A branch with a side
that leaves the loop is not a candidate, since it runs at most once as it
stands. A loop with a call that may unwind stays. A phi on an exit takes the
copy's value on the copy's edge; every other read past the loop goes through a
slot, and the promotion that follows removes it. After the copy, a test that
blocked hoisting or lanes is gone from both paths. Each loop is copied once.

Loop interchange: after store motion at `-O3`, a nest of two rotated loops may
swap when the inner loop folds one sum and the outer loop stores it, as in a
matrix product written `for j { s = 0; for k { s += a[k] * b[k][j]; } r[j] = s; }`.
The result is a loop that stores each sum's start, then a nest whose inner loop
adds one term to every cell. Each cell takes the same operations in the same
order, so this also holds for floating sums. No fixture requires this pass.

- The inner count and counters do not depend on the outer pass.
- The cell steps by one element with the outer counter, and the inner loop reads
  only named objects other than the cell's.
- Each load steps by zero or one element with the outer counter, and at least
  one steps by more than one element with the inner counter. Otherwise the inner
  loop could already run in lanes.

Loop vectorization: last at `-O3`, an innermost loop of one block may also run
in sixteen-byte lanes of the x86-64 baseline. No fixture requires this pass;
vectorization stays outside the course contract.

- The loop has a counter of a literal step and an invariant bound, and every
  other phi is another counter or an integer reduction (`add`, `sub`, `and`,
  `or` or `xor`), or a floating sum or product. Lanes would change the order of
  floating operations, so that value stays scalar and takes the lanes one at a
  time, in pass order. This needs a floating `mul`, `div` or `sqrt` in the lanes
  to pay for the moves.
- A sum may live in memory instead of a phi: a load and a store of one address
  that does not change, or a phi's sum stored on each pass. The lanes load it
  first and store it last, and the guard below tests that the arrays are apart
  from those bytes.
- A 64-bit phi that is the sign extension of a 32-bit counter that does not wrap
  counts with it, when both start at the same value.
- Before the loop is read, `add`, `sub` and the bit operators that only a
  truncation to `i8`, `u8`, `i16` or `u16` reads move to the narrow type, as does
  `mul` to the two-byte types. A value from outside the loop is truncated where
  it is defined. Their lanes are then narrower, and more of them
  fit in a register.
- Each load and store reads or writes consecutive elements, and all of them
  step in one direction. In a loop that steps down, the lanes hold their passes
  in the opposite order, so no lane needs a shuffle. Each operation
  needs a lane form: `add`, `sub`, the bit operators, shifts by a literal, `mul`
  of `i16` and `i32` and of the floating types, floating `div`, `neg`, `bitnot`,
  `sqrt`, `select` of a compare, integer `sext`, `zext` and `trunc`, `sitofp`
  from `i32`, `fptosi` to `i32`, `fpext` and `fptrunc`. The baseline has no lane
  form of `uitofp`, `fptoui` or of a conversion between `i64` and a floating
  type.
- Values may have different sizes. The widest value sets the number of lanes. A
  narrower value keeps that number of lanes at the bottom of its register, and a
  load or store of it reads or writes only those bytes. Two `i32` lanes move to
  the even lanes for one `pmuludq`; four need two. A literal power of two
  is a `pslld`.
- Two accesses of one array must keep their order. Accesses of one size at the
  same element or at least one wide pass apart do, and so does a load of the same
  size that reads ahead of a later store. Other accesses of one array leave the
  loop scalar.
- Accesses of two different named objects, or typed accesses of incompatible
  types, do not overlap. Other pairs get a test of their ranges in a guard before
  the wide loop, up to eight tests.

When code after the loop reads a value of its body, the wide loop stops with at
least one pass left. The original loop then always runs last, so that code reads
the loop's values as before, and the wide loop only changes the values the scalar
loop starts from. Otherwise the wide loop may run every pass, and a test after it
skips the scalar loop when none are left. The scalar loop also runs alone when
there are too few passes or a range test fails.

Value numbering across diamonds: when a diamond's merge heads a second
diamond on the same condition, and none of the arms or the first merge
writes memory, a phi of the second whose arm inputs compute what a phi of
the first takes from the same sides (compared to depth 4) becomes that phi.
At most 1,024 phi pairs are compared per pair of diamonds.

Block lanes: at `-O3`, after the loop passes, adjacent floating stores of one
block may become one sixteen-byte store. No fixture requires this pass either.
Two `f64` stores or four `f32` stores form a group, and the pass walks up from
what they store:

- Lanes of one operation (`add`, `sub`, `mul`, `div` or `sqrt`) become a lane
  operation. `add` and `mul` may swap one lane's operands to match the others.
- Loads of adjacent memory become one load.
- Anything else is gathered into a vector from the values computed as before.
  The same value in every lane is one shuffle.

A scalar that something else reads stays, and a node with no scalar to remove
is gathered instead. A group packs only if it removes more instructions than it
adds. All of it runs just before the group's last store. A store moves down to
that point only past accesses it cannot overlap, and a load only if nothing it
passes may write its memory. A store of the group that ran before the load
must not overlap it either. The vector accesses carry no `typed` fact, so loads
that loops may hoist have already moved. Volatile accesses and anything that
can leave the block, such as a call, stop a group.

### Safety Rules

These are the facts that stop a transformation in the course solution:

- Volatile accesses are never removed, reused, merged, hoisted or
  promoted; a slot with one stays in memory.
- A call whose boundary may write memory, an atomic operation, `va_start`
  and `va_arg` are barriers for every memory fact except those of private
  slots. Only calls that are `readnone` or `readonly`, `unwind=no` and not
  `noreturn` are removed when unused.
- A slot whose address escapes (stored as a value, passed to a call, used
  as a phi input or behind a variable index) is treated as reachable
  through any pointer and is neither promoted nor split (splitting does
  accept a call's indirect-result argument).
- Handler edges leave from the opener's block, so dominance alone does not
  say what a handler has run. Value reuse in code a handler reaches never
  takes a value computed after the first opener of its block, and slot
  promotion keeps a slot in memory when the value a handler's entry would
  see might differ from the slot's value at the throw point (for example,
  one stored inside an active region and live at a handler's entry).
  Memory forwarding starts every handler block empty, no phi is placed in
  a handler block, and handler blocks are never bypassed or merged.
- `replaceable=yes` and `no_inline=yes` callees stay calls; a replaceable
  global is never read as a constant; only discardable definitions are
  pruned.
- A moved instruction loses facts that may not hold where it now runs: a
  load placed where it did not always run loses `typed`, and arithmetic
  moved out of a loop loses `nowrap` and `exact`.
- A scalar temporary passed to a `by_address` parameter stands for storage
  holding its value. Before any pass at `-O1` and above it becomes that
  storage, a slot stored just before the call whose address the call
  passes, so propagation never passes the value itself as the pointer.

### Design Notes

Give function definitions dense indices, keep call edges in adjacency
arrays, and compute strongly connected components once for recursion and a
callee-first order; compare symbols by typed id, not by spelling. Keep
object roots for address, lifecycle and ABI requirements, not for ordinary
calls, so a definition can disappear when inlining removes its last call.
Since cloned calls only reach functions already visited, one callee-first
traversal per wave needs no whole-program retries. Keep each inlining
policy's budget separate, so exhausting one does not suppress another.

Carry the active-region stack as one interned value per block. Key a memory
location by base, offset and type, and undo facts when the dominator walk
leaves a subtree instead of copying tables per block. Share CFG and
dominator facts among passes until one changes the graph. Serialized LowIR
needs every definition before its uses in layout; when value reuse or block
merging breaks that, reorder the blocks (reverse postorder, or a stable
topological order of definition blocks when that would break an order the
input relied on).

The optimizer's own speed matters once it compiles the compiler. In the
course solution, each function body carries a version that changes only
when a pass changes the body, and a pass reports a change only when it made
one: the report decides whether another round runs, so a pass that reports
work it did not do buys a whole extra round of every pass. A pass that ran
at the current version and found nothing is not run again. Analyses (the
CFG, dominators, block maps, use counts) are kept between passes at the same
version, and a checking mode (`--verify-passes`) validates the program after
every pass and checks that the kept block maps and use counts are what a
fresh computation gives. The per-function containers of the larger
passes live in pools reused from one function to the next. No per-function
pass scans the whole program: calls, callers and symbols are indexed once
per wave. None of this changes the output; together it removed a large
share of the optimizer's own work on the compiler's sources.

### Diagnostic controls

`lowiropt` and `cppgm++` (the compile, link and `--emit-lowir` modes)
accept repeatable `--inline-limit <name>=<positive-integer>` options. The
names are `caller-budget` (default 1,536), `once-cap` (the single-call cap:
512, or 1,024 at `-O2` and above), `once-caller-budget` (1,024, or 2,048)
and `hint-late-cap` (the late wave's cap for a hinted callee, 48). A value
given replaces the default at every level; a zero, malformed or unknown
value is rejected. Only that number changes: recursion, EH, signature,
`no_inline` and growth checks still apply, and `-O0` ignores the options.

`lowiropt --stats` (and `cppgm++ --emit-lowir -O1` or higher with
`--stats`) writes one `pa32_opt_stats` record to standard error with the
fields `functions` (definitions optimized; 0 at `-O0`), `instructions_in`,
`instructions_out`, `blocks_in`, `blocks_out`, `folded`, `propagated`,
`reused`, `dead`, `branches_folded`, `blocks_removed`, `blocks_merged`,
`slots_promoted`, `phis_inserted`, `calls_inlined`, `inlined_instructions`,
`functions_pruned`, `loads_reused`, `stores_removed`, `loops_removed`,
`loops_unrolled`, `invariants_hoisted`, `stores_sunk`, `specialized` and
`optimize_ms`; `lowiropt` appends
`parse_ms`, `validate_ms`, `write_ms` and `peak_rss_kb`. The counts are
observational only.
