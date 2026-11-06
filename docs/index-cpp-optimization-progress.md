# index-cpp optimization investigation records

Maintainer-only companion to [the plan](index-cpp-optimization-plan.md).
The original plan remains the scope; the first fix does not close the group.

## Measurement setup

Baseline compiler is the immutable `60e73459e` snapshot from
`/home/vishvananda/work/index-cpp/validation/20261105-comparison/`.
The compatibility branch's subsequent optional-test refactor changes no compiler
source. Upstream source remains pinned and unchanged.

Current receipts live under
`/home/vishvananda/work/index-cpp/optimization/20261105/`:

- `profiles/`: sampled cycles for the first three workloads on all three
  compilers, plus relevant machine code.
- `index-O3.lowir`, `lowir.stderr`: baseline optimized typed IR and work census.
- `variadic-inline-paired/`: all observations from A/A followed by three
  ABBA/BAAB blocks for Brainfuck recursion, Calculator AST and CRC32.
- `variadic-inline-safe-frozen/`: source/header-validated frozen compiler
  instruction and memory checks at `-O0` and `-O3` for the corrected candidate.
- `public-baseline*`, `public-variadic-inline*`: existing public sentinels,
  built with the respective immutable compiler and both reference compilers.

The reusable `scripts/index_cpp_benchmarks.py` retains process counters, raw
output, hashes and calibration separately from upstream loop wall time. Full
inputs run during measurement; every executable first passes all 50 `TEST=1`
checks. Later receipts include host load snapshots and explicit drift warnings.

## Brainfuck recursion: unused variadic arguments

Baseline sampled cycles: 37.3% in a helper returning its first argument,
30.7% in variant dispatch, 19.8% in the recursive loop visitor and 9.9% in
the outer visitation wrapper. The first helper is a library lambda with a C
ellipsis, not a function that reads a `va_list`.

The baseline LowIR body is simply `return ptr %__v`, with a variadic boundary.
The native body nevertheless stores six GPRs and eight whole XMM registers,
allocates 224 stack bytes and keeps a frame pointer. The inliner treats every
variadic signature as unsafe, so it cannot remove this call.

The first candidate substitutes named parameters when a body contains no
`va_start`, `va_arg`, dynamic stack allocation or other existing cloning restriction.
Extra arguments remain evaluated in the caller. Existing exception, aliasing,
ABI, replacement, recursion and profitability checks continue to apply.
No budget was raised and no source/library name participates in admission.

Review found another fixed-arity assumption in folded-copy specialization:
unnamed argument positions could be mistaken for ordinary temporary IDs. A
cleanup-bearing reducer with many constant unnamed arguments made the initial
prototype return an incorrect result. Restricting constant substitution and
dispatch-target inspection to named parameters fixes the reproducer. Both
versions emit byte-identical index-cpp objects; the runtime observations apply
to the corrected candidate too. The unsafe prototype is not an accepted fix.

Coverage includes unused arguments with observable effects, floating/stack
arguments, a volatile input, a throwing argument with destructor cleanup,
`noinline`, real `va_start`/`va_arg`, and the folded-copy negative case. The
driver fixture requires removal of the small identity call and loose size
bounds, rather than a particular instruction sequence.

Measured speed against the baseline is about 1.71x in loop wall time and
1.825x in process instructions (45.2% fewer). The three instruction-count
ratios agree closely. Wall-time blocks range from 1.42x to 1.83x; A/A drift
reached 26%, so the precise wall-time gain needs a quieter confirmation run.

Remaining opportunities: collapse justified forwarding wrappers within a call
cycle without recursive expansion; inspect dispatch/state reloads after that;
omit register-save work for non-inlined variadic bodies that never use it.
Keep those mechanisms separate from the first inlining fix.

## Calculator AST: variant movement and destruction

Baseline sampling places substantial time in variant move construction,
destruction and dispatch, with parser routines and allocation also visible.
This workload constructs/replaces trees; it does not evaluate their visitors.
The unused-ellipsis helper appears in those variant paths as well.

The shared first fix measures about 1.22x loop-wall speed and 1.324x process
instruction speed (24.5% fewer instructions). Wall calibration drift was 4.1%;
the instruction counts are stable. Investigate the residual recursive ownership
and move/dispatch paths before adding scalar replacement or cleanup changes.
Preserve allocation observability, exception behavior and destructor order.

## CRC32: a confirmed loop-idiom gap

Baseline LowIR already unrolls the eight bit steps. Seven become selects; the
last remains a branch and phi. GCC also performs the arithmetic recurrence.
Clang emits a table lookup combined with a shift, processing two bytes per
outer iteration. This is a different transformation, not merely faster
instruction selection for the existing arithmetic.

An independent pointer/length reducer confirms that Clang's
`LoopIdiomRecognizePass` introduces the table (`crc-clang-passes.log`,
`crc-clang.ll`), without a special target ISA. A general implementation must
derive its table from the recurrence's typed operations and constants, prove
the rewrite for all input words, bound compile-time/table size, and cover
different polynomials, word widths and iteration counts. No benchmark table
or expected checksum may be embedded in the optimizer.

The first inlining fix leaves CRC32 process instructions unchanged; its
observed wall ratio, 1.004x, is within calibration noise.

## Compiler cost and checkpoints

The initial inlining candidate changes frozen compile instruction counts by
less than 0.003% at both levels, within A/A noise. Peak RSS changes by less
than 0.1%; both frozen object hashes are identical across baseline and candidate.
Compile CPU timings are noisy, including 6–11% A/A drift, and cannot establish
a wall-time improvement or regression. Recheck the corrected compiler receipt
before accepting the change.

Index-cpp `.text` grows from 387,154 to 391,618 bytes (1.15%). That cost must
remain visible alongside the runtime gain; existing PA35 size gates remain
unchanged. The baseline was already larger than the two references.

The initial prototype passed the full strict report (8,038/8,038), debug info,
self-host through PA5, PA35 quick/smoke and architecture/format/file audits.
The corrected candidate is being revalidated, including the stronger reducer.
Public sentinels cover recursion, containers, JSON, regex, hashing and floating
point. Student-export validation follows the shipped fixture/handout changes.

## Outstanding work

Finish accepting the corrected first mechanism with current-state receipts,
then continue the original queue: Brainfuck's residual call chain, CRC32 and
Calculator AST diagnoses/fixes, followed by a full 50-workload reranking.
Investigate Base64 decoding, Huffman decoding and Nbody, then the named parser,
interpreter, SHA256, maze and container workloads using that refreshed evidence.
Retain existing workload wins and every repeatable individual regression.
