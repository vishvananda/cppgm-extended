# Pointer bounds, loop state, and packed floating loads

The candidate is rebased onto the committed LLVM aggregate checkpoint
`cf3e0fca5a1718a5b524a62369f75276ff2c6a28`. The comparison separates that
checkpoint from the subsequent floating-load repair. This branch also contains
PR126's conditional-store work, the explicit pointer-bounds fact, cyclic-phi
simplification, and the dynamic internal-test-count badge.

## General optimizations

`index inbounds` carries an allocation/one-past and no-address-wrap promise
independently of projection metadata. It permits a derived pointer's null
comparison to use its base. The frontend states it for defined object-pointer
arithmetic; speculative motion discards promises not true on every new path.
The reader, writer, validator, contract ledger, Perl checker and PA8/PA10/PA33
fixtures exercise both proof-bearing and ordinary wrapping addresses. The
instruction record remains 64 bytes.

Ordinary joins in a function with persistent code addresses can forward private
storage without inventing phis at indirectly addressed entries. A separate,
bounded paired-graph proof merges equal loop state, including cycles whose
external seeds and updates agree. Different seeds, edges and updates remain
distinct. This removes the duplicated Bubblesort state introduced by earlier
codegen changes; its instruction count falls from 188.8M to 126.5M in the
pre-LLVM comparison.

The aggregate checkpoint exposed another missing cleanup. A small by-value
record containing a character and a float became an integer carrier; reading
its float then loaded the whole integer, shifted and truncated it, and copied
the bits to a floating register. The repair recognizes a single-use ordinary
load, a truncation and optional byte-aligned constant right shift, and a
floating representation copy. The floating field must be smaller than the carrier. It reads that field directly,
including a field in a 128-bit carrier.

The read moves only within one block across at most sixteen pure instructions.
Stores, calls, assembly, atomic/volatile accesses and exception boundaries stop
it. Volatile and atomic source loads keep their widths; shared carriers and
non-byte-aligned projections keep their original work. The new load inherits
no alias type from the wider integer access. Numeric integer-to-float
conversion remains numeric. There is no source-name or benchmark-name test.
The existing simplifier supplies use counts and block ownership, so the repair
adds neither a whole-function scan nor a new analysis allocation.

An initial extension to integer-only projections passed correctness but changed
later inlining/register allocation: Treesort executed 3.6% more instructions,
FloatMM 1.4% more and Minisat 1% more. That extension is excluded. A broader floating prototype also folded full-width bitcasts; an EPYC Godot
string check measured 0.977× with stable calibration. The accepted rule is
limited to actual narrowing. The accepted floating projection affects five of the 81 public executables; the other 76,
including those three programs, remain byte-identical to the aggregate baseline.

## Floating repair: isolated results

All speed/efficiency ratios below are higher-is-better: baseline divided by
candidate. Instruction efficiency describes executed work, not measured speed.
The baseline is the aggregate build, not GCC or a historical release.

| Public workload | Instruction efficiency ↑ | EPYC CPU speed ↑ |
| --- | ---: | ---: |
| Fasta 1 | 1.277× | 1.150× |
| Fasta 2 | 1.194× | 1.024× |
| Fasta 3 | 1.038× | 1.026× |
| Fasta 6 | 0.980× | 0.998× |

All 81 output checks pass. Their instruction-efficiency geometric mean is
1.0054×, including unchanged executables. Bullet is the fifth changed executable
and is effectively neutral (0.04% fewer instructions).

Fasta 1's three clean EPYC CPU ratios are 1.150, 1.153, 1.138;
same-binary calibrations are 1.0024 and
0.9904. Fasta 6's additional instructions produce
no measurable CPU-time loss: its paired ratios are
0.9980, 1.0000, 0.9959, with calibrations
1.0062 and 0.9959.
A binary upload overlapped an earlier campaign; that complete campaign is
retained separately and excluded from these timing claims. No whole-suite
runtime score is inferred from instruction counts.

On the frozen PA36 compile input, the repair's compiler instruction efficiency
is 1.0000× at `-O0` and 0.9967× at `-O3`: about 0.33% extra compiler instructions
at `-O3`. Objects match exactly at `-O0`. The `-O3` wall-time ratio is 1.0080×,
with same-binary calibration 1.0363×; that small timing difference is not a
reliable speed claim. RSS changes remain below 0.2%.

The final full Godot engine was rebuilt with the frozen repair compiler.
The comparison against the aggregate build, on the same EPYC VM, is:

| Godot workload | Elapsed-time speed ↑ | A/A before ↑ | A/A after ↑ |
| --- | ---: | ---: | ---: |
| dispatch | 1.003× | 0.999× | 1.010× |
| arrays | 0.977× | 1.013× | 0.997× |
| strings | 0.998× | 0.980× | 0.987× |
| geometry | 0.996× | 1.000× | 1.003× |
| scene | 1.001× | 1.034× | 0.988× |

The Godot geometric mean is 0.995×. Arrays is the remaining unfavorable
screen: its blocks are 0.975×, 1.006× and 0.977×. This does not establish
performance neutrality for arrays; CI and focused repeats should confirm
whether its roughly 2% shift persists. The other changes are small relative
to the calibration spread. Full-width bitcast folding and the broader integer
projection experiment remain excluded.

## Rebase context and remaining gaps

The aggregate checkpoint against the previous LLVM tip (`47182b779`) changes
31 public executables. All 81 output checks pass, with 0.9955× instruction
efficiency across all 81: gains in fmt, nlohmann-json, tramp3d and Minisat coexist
with the Fasta regressions above. On the five EPYC Godot workloads, its median
ratios have geometric mean 1.0151×; some individual changes are within calibration
noise.

The frozen Myc input passes all 50 checks. Its six EPYC whole-suite runs give
0.9941× for aggregate versus previous LLVM, with 1.0031× same-binary calibration.
Individual cases show more drift than the total: for example, the two baseline
Template::Parse observations range from 1.097 to 1.314 seconds. Its apparent
0.848× case ratio needs focused interleaved repeats before attribution.
The floating repair then produces identical allocated code/data bytes and
identical section addresses for Myc, excluding the non-executed build-id note;
all 50 correctness checks still pass.

The earlier final-shape inliner change is a separate tradeoff. It substantially
improves tramp3d, while the local public check finds real losses in Mandelbrot 0
and re2-test-re2. Those losses precede the aggregate and floating repairs.
The Fasta repair does not resolve them. General inlining cost/benefit and
backend handling of packed integer fields remain follow-up work.

## Protocol and evidence

Native instruction counts use the local Intel host and frozen executable/input
pairs. EPYC timing uses a quiet two-core AMD EPYC 9B14 VM, immutable binaries,
fixed runtime libraries, per-run output hashes, same-binary calibration and
three ABBA blocks. Each Fasta leg aggregates a fixed number of executions to
at least two seconds. Identical verified output files share storage, while
every observation and output path is retained.

Fasta CPU time comes from the child's user+system counters. The auxiliary Python
parent wait time is quantized for these short child processes and is excluded
from the claims. Godot uses its workload's internal elapsed time. Myc uses the
unmodified frozen LowIR input, fixed argv0, fixed libraries and disabled ASLR.
No Myc index-source correction or Base64 consumed-buffer correction is mixed
into these comparisons.

Evidence is retained under `/mnt/vizier/cppgm-godot/20261110/`:

- `aggregate-provenance.json`, `narrow-float-loads-provenance.json`, frozen
  compilers, source diffs and the new module's recorded source;
- `public-aggregate/`, `public-narrow-float-loads/`, immutable output-checked
  executables, native counters and manifest/source-pin receipts;
- `aggregate-comparison-observations.tgz`,
  `aggregate-fasta-myc-observations.tgz`,
  `narrow-float-final-observations.tgz`, every paired observation and calibration;
- `compile-guard-narrow-float-*.json`, retained AA/ABBA observations, native
  compiler counters and output identity checks;
- `myc-narrow-float-equivalence.json` and all Myc correctness/timing observations;
- `packed-load-regression-proof.txt`: the pre-fix compiler fails the three
  floating-field quality bounds; the repair passes the normal Perl runner,
  runtime checks and optimizer pass validation.

The PA33 examples state behavior and broad work envelopes, allowing alternative
instruction sequences. They cover high/low fields, wide carriers, negative
zero, NaN payloads, subnormals, numeric conversion, shared values, intervening
writes/calls and volatile/atomic widths. The student design note explains the
requirements; this measurement record is excluded from the student export.

## Qualification

Compiler source commit: `3253385a23eeb155c546cf7bec18c40f2f5e780e`.
Measured compiler SHA256:
`bd743d829d121d644a782962bfdae36fc2fe91f3b53bfc9e3b10ebe40c318a8e`.
The source patch was frozen before measurement; the subsequent commit adds no
compiler edits.

The strict report passes 10,626/10,626. Architecture/source-set/scaffold/format,
IR-expectation, file, placement and harness audits pass. GCC and Clang 21 compile
the new module with warnings as errors. PA36 smoke, debug-info, self-host through
PA5 and byte-identical inception pass. A fresh student export passes the
256-document inventory and installation/execution of all 43 reference wrappers.
The pre-fix regression proof, positive/negative runtime fixtures and optimizer
pass validation pass. Source examples and references are regenerated through
the normal harness; references were not edited by hand.
