# Optimization improvement exercises

These exercises extend the [PA32 design discussion](design-notes.md) and
[PA33's machine optimization](../pa33/design-notes.md). The optional exercises
have correctness checks and diagnostic measurements, with no required speedup
or instruction sequence. They do not add vectorization, interchange, scheduling
or alignment requirements to either assignment.

## Required kernels and what they measure

`make test-kernels` runs four small, original kernel families under
`tests/kernels/`: a bounded search, aliased and disjoint field updates, narrow
arithmetic, and values live across a cold call. The ordinary course suite runs
these too. Their `.quality` files give the optimization level, work size,
input scenarios and maximum instruction ratio. Every scenario must return the
scalar oracle's result and leave the same observable data.

PA32 lowers each source with the supplied frontend at `-O0`, applies your
`lowiropt`, then makes an object using the supplied backend at `-O0`. This
isolates LowIR optimization from your native optimizer. PA33 reuses the inputs
through your complete optimized source/object path. Both link each object to
one identical host-built driver. The native driver also supplies nonzero,
unspecified upper bits on its narrow count argument to check the ABI boundary.

The driver checks zero work, short runs, bounds around loop widths, and longer
runs against an independent scalar oracle. Cachegrind then counts a measured
run and a zero-work run. The difference removes startup cost. Each core scenario
must stay within the published ratio to GCC; the check never grades elapsed
time, register names or a mnemonic. Counts and all command observations remain
under `obj/quality/`.

## Optional experiments

Run from PA32 or PA33:

```sh
make test-improvements
```

PA32 measures LowIR optimization through the supplied `-O0` backend. PA33
measures the combined compiler path. Both execute the programs and report
instruction counts for the longest disjoint scenario; no speed threshold is
imposed on this optional target. Correctness covers every published scenario.

| Input in `tests/improvements/` | Question | Correctness cases |
| --- | --- | --- |
| `600-vector-overlap.t` | Can a loop run in wider lanes with a scalar fallback? | Zero lengths, tails through 68 elements, disjoint output, identical ranges and overlap in either direction. |
| `600-mixed-width-lanes.t` | Can narrowing expose more parallel elements? | Byte wrapping, widened operands and short tails. |
| `600-adjacent-store-groups.t` | Can independent store trees share packed work? | Both output groups, changing inputs and zero rounds. |
| `600-strided-matrix.t` | Would a different traversal improve memory access? | Each cell's ordered floating sum and final output. |
| `600-ordered-floating-sum.t` | Can independent work be packed while preserving reduction order? | Cancellation, signed zero, tails and repeated rounds. |
| `600-independent-math.t` | Can independent divisions and roots overlap? | Positive square-root arguments, dependent consumers and changing inputs. |

The shared scalar driver is in `tests/kernels/600-support.h`. Arithmetic NaNs
are compared by classification, without demanding one payload. The separate
representation fixtures check bit-preserving copies. Extend a small test with
negative values, infinities, NaNs or an aliasing pattern before experimenting
with a transform whose proof depends on them.

Inspect the output and verify the same results before interpreting a count.
Packing can increase live ranges and spills. Interchanging a floating reduction
must preserve its operation order unless the input grants another semantics.
Instruction counts cannot reveal division latency, branch prediction or fetch
alignment costs.

## Reproducible timing

For a longer diagnostic run from the repository root:

```sh
python3 scripts/check_optimization_quality.py pa32/tests/improvements \
  --mode native --compiler dev/cppgm++ --report-only \
  --iterations 1000000 --timing-rounds 6 --out obj/quality-experiments
```

Compilation finishes before measurement. The runner freezes input copies,
records tool and executable hashes, checks outputs, runs A/A calibration and
retains every paired ABBA observation. Compilation time and peak RSS are also
recorded for diagnosis. Keep noisy observations; repeat a whole block when
necessary rather than choosing its best sample.

Use the same source revision, arguments and toolchain when comparing CPU lanes.
A change can help one CPU and hurt another. Compare the complete workload and
compiler cost alongside the particular kernel that motivated it. Timings are
reports for investigation, not course completion criteria.
