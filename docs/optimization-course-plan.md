# Optimization coverage and course strategy

This is a maintainer plan, outside the student export. It reviews the
optimization branch against PA8, PA32 and PA33's current handouts. The existing
completion requirements remain in those handouts and their course fixtures.

## Findings

PA32 already separates the interface contract, outcome/size expectations and
the solution's design. PA33 similarly grades execution, MIR envelopes and four
Cachegrind kernels. These are the right mechanisms for further improvements.
The detailed design notes have kept up much better than the introductory
handouts and course coverage. Many new reducers live only in `regression/`.

The export treats these lanes differently: PA24 and PA32 retain their
regressions as optional material, excluded from exported `make test`; PA33
removes its regression fixtures and maintainer tooling entirely. Thus a new
regression does not add a student requirement or guarantee student coverage.
Copying all regression predicates into the course suite would pin implementation
choices that the handouts deliberately leave open.

The numbering auditor used the course-fixture collector, which excluded
regressions. It also missed standalone object-roundtrip `.cpp` inputs.
Numbering now covers those lanes, including nested controls, while feature
placement still considers the course contract separately. Existing prefixes
were normalized to their hundred cluster, with all companions, links and live
checker/ledger references moved together. This included PA24's instances of
the same omission. Descriptive suffixes continue to distinguish cases.

## LowIR boundary

There are public format changes on this branch, even though most optimization
work is a consumer of existing LowIR:

| Form or fact | Student-facing obligation | Coverage |
| --- | --- | --- |
| `unary sqrt f32/f64` | Preserve IEEE value semantics without the library's `errno` effect; negative inputs still have defined floating results. | PA8 `300-sqrt`; PA32 math/guard reducers; explain guarded fallback in `lowir.md`. |
| `convert trunc exact` | Read/write the proof that the value fits the destination's signedness; reject it on other conversions. | New PA8 positive and rejection fixtures; PA32 counter widening. |
| Floating/integer bit `copy` | Preserve the representation of `f32`/32-bit integer and `f64`/`i64`; this is not numeric conversion. | New PA8 `300-floating-bit-copies`; existing native carrier behavior checks. |
| NaN payload literals | Preserve observable floating representations through bit-copy folding and text replay. | PA8 payload roundtrip and rejection fixtures; PA32 constant-carrier execution. |
| `typed` aliasing | Respect character access, same-width integer compatibility, unions, `may_alias` and disabled strict aliasing. | Existing format rules and PA29 ownership; PA32 memory-proof guidance needs examples. |
| `v128` and `vector` | Existing PA29 vocabulary, now also produced by optional PA32 transformations; native input must roundtrip. | PA29/PA33 vector cases already exist. No new vector instruction vocabulary is needed for these optimizations. |

The README, format reference and grammar now describe the missing forms more
clearly. The grammar pages are regenerated from `pa8.gram`. Internal tables of
allocation and inferred read-only functions are analyses reconstructed from the
typed unit; they are not new serialized facts or hidden frontend requirements.

## Coverage to add by behavior

The implementation below follows this inventory. Positive cases and guard
cases share behavior oracles and generous work/size bounds. Exact solution
shapes remain in `regression/`.

| Improvement family | Useful course check | Essential counterexamples |
| --- | --- | --- |
| Memory forwarding, typed fields, private and allocated storage, table hoisting | PA32 behavior plus an upper bound on repeated loads in the hot function; source/object replay. | Aliased pointers, byte accesses, overlapping fields, escaping/self pointers, zero-trip loops, calls that free or modify storage. |
| Object copying and clearing, floating carriers, static initialization | PA32 runtime values and call/size envelopes where they express the intended outcome. | Padding and alignment, negative zero/NaN bits, volatile members, externally visible initialization order, address escape. |
| Counter widening, strength reduction, quotient reuse, possible bits | PA32 boundary-value execution; a generous total-work envelope for repeated arithmetic. | Wrapping unsigned arithmetic, signed limits, negative operands, zero divisors where defined, multiple exits/latches. |
| Math calls and guards | PA32 runtime results, `errno` on negative square roots, retained observable effects. | NaNs, infinities, signed zero, a user definition with the same spelling, guarded versus unguarded operands. |
| Inlining, partial copies, specialization and guarded virtual calls | PA32 behavior and bounds on residual work in a hot caller. | Overridden targets, replaceable/noinline definitions, recursion, cleanup/catch paths, large cold paths and total growth. |
| Small-loop unrolling and early-exit searches | PA32 runtime input and a dynamic work comparison; bound code growth separately. | First/last/no match, empty/one-element loops, descending steps, unknown bounds, reads beyond the end. |
| Rotation, store motion and unswitching | PA32 results across trip counts, aliasing and call paths; load/store budgets where justified. | Conditional stores, values live after a loop, overlapping memory, exceptions and zero trips. |
| Loop and block vectorization, interchange | Optional PA32 experiments with scalar-oracle execution and measured work. | Every tail length, alignment, overlap in both directions, mixed widths, integer overflow, ordered floating reductions. |
| Register allocation, phi/select sharing and cold-call splitting | PA33 behavior, broad frame/memory/instruction bounds and a repeated kernel. | Fixed-register clobbers, cycles on edges, EH successors, spill pressure and reloads that would overwrite a phi. |
| Narrow values, high products and constant/word division | PA33 numerical/ABI boundary checks, then dynamic work bounds. | Unspecified incoming upper bits, signed extrema, extensions through phis, live dividend reuse, signed/unsigned high halves. |
| Floating memory operands and long-latency scheduling | PA33 execution and diagnostic timing on independent math chains. | Operand order, NaNs, aliasing barriers, calls and XMM pressure. Instruction counts alone cannot grade latency. |
| Loop layout, frame arrangement and object placement | PA33 ABI/unwind/debug checks and diagnostic native timing. | Stack alignment, alternate exits, short search loops, relocation and branch-range limits. Do not grade a particular padding pattern. |

## Grading approach

1. Add correctness coverage first, in lanes the owning assignment already runs.
   PA32 can optimize handwritten LowIR, then execute both forms with the
   reference backend at `-O0`. Source cases also check the serialized boundary.
   PA33 executes the same LowIR through the student's native backend. Add
   meaningful runtime inputs rather than functions returning folded constants.
2. Add static expectations only for stable outcomes: observable effects kept,
   avoidable repeated memory work removed, and total instruction/block/frame
   growth bounded with room for alternative implementations. Do not require
   `adc`, `mul`, `cmov`, a particular vector opcode, a register, or an order of
   passes. An instruction sequence is not a performance specification.
3. Extend dynamic coverage with a small number of original, dependency-free
   kernels: bounded search, aliased/disjoint field updates, a narrow arithmetic
   chain, and values crossing a cold call. Each needs a scalar oracle and edge
   cases, one common host-built driver, startup subtraction and recorded GCC
   instruction counts. Establish tolerances from several valid implementations
   before adding a completion gate. Keep PA32's LowIR-quality kernels under
   PA32; PA33 may reuse them to measure the combined path, as it currently
   reuses PA35's kernels. Avoid making a late assignment the sole owner of an
   earlier requirement.
4. Keep vectorization/interchange and hardware-sensitive scheduling/layout in
   a documented optional lane initially. The present handouts explicitly leave
   them out of scope. If a later quality gate needs them, first change the
   stated quality bar and provide the inputs, oracle and measured limits in
   the same patch. A speed/work bound may encourage them without prescribing
   which transformation achieves it.
5. Measure compiler cost as well as generated work. Preserve immutable binaries,
   frozen sources, output hashes, A/A calibration and paired observations. Use
   both CPU lanes for timing evidence; use deterministic counters for small
   work changes. Do not grade wall-time on one CPU model or equate fewer
   instructions with lower latency.

## Suggested delivery order

The LowIR contract/grammar fixtures, warning fix, numbering and auditor repair
can land first. Next add the memory/counter/math correctness pairs and broad
expectations, then the native ABI/liveness cases. Add the small dynamic kernels
after calibration. Finish with the optional vector/scheduling exercises and
links from each handout to the relevant design-note sections. Regenerate
references with the supplied tools, run the course/harness/debug/placement
checks, and validate the student export for each batch; inspect the exported
lanes as well as the source checkout.

## Implementation record

The public design notes now discuss proofs, tradeoffs and experiments. They no
longer publish private helper APIs, implementation caps, tuning histories or
benchmark-specific recipes. `audit-student-documents` scans every reviewed
student prose file, including both design notes and exported root overrides;
the export client validator runs the same check on the actual exported tree.
The document inventory is explicit, so this plan and calibration record stay
out of the export.

| Family | Delivered behavior or measurement coverage |
| --- | --- |
| Memory, aliasing, private storage and allocations | PA32 `600-memory-proof-boundaries`, `600-type-aliasing-permissions`, host `600-disabled-strict-aliasing`, `600-load-after-release`, `600-zero-trip-reference`; field-update kernel and existing table-load outcome checks. |
| Object copies, clearing, carriers and initialization | PA32 `600-object-representations` and LowIR `600-bit-carrier-constants`; direct/text object replay. |
| Counters, strength, quotients and possible bits | PA32 `600-counter-boundaries` and the narrow-arithmetic kernel, including runtime wrapping and signed negative operands. |
| Library math and guards | PA32 `600-library-math-effects` and `600-user-math-definition`, including errno, NaNs, infinity, negative zero and user definitions. |
| Calls, specialization, virtual dispatch and EH | PA32 `600-call-and-cleanup-boundaries`, `600-handler-visible-loop-state`; PA33 reuses the source/object behavior path. Existing call-removal and growth envelopes remain the quality requirements. |
| Bounded searches | PA32's original bounded-search kernel, reverse/early-exit cases in `600-loop-exit-and-memory-effects`, and host `600-bounded-search-pages` with protected pages before/after the valid range (including empty and one-element ranges). |
| Rotation, store motion and unswitching | PA32 `600-loop-exit-and-memory-effects`, zero-trip/null cases, conditional stores and calls. |
| Optional packing and interchange | Six original inputs under PA32 `tests/improvements`, with shared scalar oracle, all published tails and overlap directions, ordered reductions, optional counts/timing and no speed gate. |
| Native allocation, phis and cold calls | PA33 behavior `600-phi-and-reload-edge`, `600-select-and-phi-lifetimes`; shared cold-call-state kernel. |
| Narrow values, products and division | PA33 behavior `600-narrow-value-readers`, `600-wide-product-results`, `600-word-quotients-and-remainders`; shared driver dirties the upper half of the count register. |
| Floating operand order and latency | PA33 behavior `600-floating-operand-order`; optional independent-math chains and ordered floating reductions. |
| Layout, frames, objects and debug/unwind | Existing PA33 ABI/EH/debug lanes remain required; the optional timing lane measures whole generated kernels without requiring padding or block order. |

PA32 owns the four new required kernel families. PA33 reuses them through the
combined optimized compiler. One GCC-built driver links both candidate and
host objects and checks them against an independent scalar oracle. All core
scenarios receive startup-subtracted Cachegrind comparisons. The
[calibration record](optimization-course-calibration.md) preserves measured
GCC/Clang and optimizer-level alternatives and the rationale for each limit.
Correct but unoptimized search controls fail both mode-specific work limits.

`pa32/improvements.md`, exported and linked from both handouts/design notes,
gives required/optional commands and explains the limits. Both manual CPU
lanes accept `course_quality` for the same frozen inputs, scalar checks,
Cachegrind counts, A/A calibration, six paired timing blocks and retained
observations. The Intel helper forwards it with `--course-quality`.

The new correctness coverage found and fixed a real constant-propagation
problem: an equal-width floating/integer representation copy must retype a
folded literal. Exposing a nondefault NaN payload also requires its public
text spelling to preserve those bits. PA8's format, grammar, generated grammar
pages and fixtures cover it; the Perl comparison validator now recognizes a
payload's parentheses inside all direct and indirect call forms.
