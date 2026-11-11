# Corrected Myc follow-up: independent integer fields

This continues [the byte-update and address work](CORRECTED_OUTLIERS.md), comparing
against that checkpoint (`93beb5c39`) on the same corrected Myc/index-c source.
Ratios below compare checkpoint time/work with candidate time/work; higher is
better. They are not new GCC or Clang scores.

## General change

Aggregate splitting treated same-width signed and unsigned accesses as an
incompatible overlap. In the arithmetic decoder this packed a byte and an
adjacent bit position into one 64-bit carrier. Every bit then required shifts,
masks and reinsertion to preserve the other field. The existing scalar-slot
promoter already handles these integer views correctly.

The change shares that compatibility predicate with aggregate splitting and
copy-field matching. It preserves each access's declared signedness while
letting adjacent fields remain independent scalar values. Boolean `i1` is
excluded: it has one value bit despite occupying a storage byte. Different-width
overlaps still use representation-preserving carriers; escaping and volatile
storage retain their existing barriers. There is no new pass, analysis allocation,
LowIR vocabulary, source-pattern match or benchmark-name test.

## Focused observations

Intel Xeon E5-2696 v4, generic x86-64 `-O3`, fixed argv0, ASLR disabled,
CPU30/NUMA1. Two A/A calibrations precede two ABBA blocks per case; the repeat
has three ABBA blocks. The final JSON repeat uses CPU43/NUMA1; the other
focused controls use CPU30. Every selected workload passes its checksum. Instructions
and cycles cover preparation and warmup too; benchmark time excludes them.

| Workload | Runtime vs checkpoint ↑ | Instructions saved | Cycles saved |
|---|---:|---:|---:|
| Compress::ArithDecode | 1.044× | 5.03% | 2.19% |
| Etc::LogParser | 0.993× | -0.00% | -0.16% |
| Etc::NeuralNet | 1.017× | -0.00% | 0.02% |
| Brainfuck::Array | 0.959× | -0.00% | -3.21% |
| Compress::HuffDecode | 0.963× | 0.00% | 0.01% |
| Compress::HuffEncode | 0.997× | -0.00% | -0.01% |
| Brainfuck::Array (repeat) | 1.020× | 0.00% | -0.03% |
| Compress::ArithDecode (repeat) | 1.040× | 5.03% | 2.28% |
| Compress::HuffDecode (repeat) | 0.968× | 0.00% | 1.69% |
| Compress::BWTEncode | 1.004× | 0.00% | 0.16% |
| Json::ParseMapping | 0.892× | 0.00% | -1.45% |
| Json::Generate | 1.048× | 0.00% | 3.47% |
| Json::ParseMapping (repeat) | 1.012× | -0.00% | 0.89% |

Arithmetic decoding saves 5.03% of instructions in both campaigns. Its clean
repeat saves about 2.28% of cycles, with only 0.017% A/A cycle variation.
The apparent 3.2% Brainfuck cycle loss in the first campaign disappears in the
repeat (0.03% more cycles); its executed instruction count is unchanged.
Huffman decoding's lower wall ratio is not accompanied by a cycle loss:
cycles are unchanged initially and improve on repeat. These observations do
not establish a whole-suite wall-time gain or universal timing neutrality.

Full-suite outlier controls leave BWT encoding and both JSON workloads with
unchanged instruction counts. BWT's cycle efficiency is 1.002×. The first JSON
mapping campaign has a 1.4% cycle loss and strong intra-block wall drift; its
three-block repeat on CPU43 instead gives 1.009× cycle efficiency. That loss
does not persist across the repeat. JSON generation's apparent cycle gain is
also not attributed to less generated work. All observations, including the
unfavorable block, are retained.

## Compile cost and public check

The same immutable PA36 semantic-overload input has A/A calibration and two
ABBA blocks at each optimization level. Candidate/checkpoint compiler
instruction ratios are `0.999993` at `-O0` and `1.000005`
at `-O3`; all `-O0` objects match between compilers, and each compiler is
deterministic at `-O3`. Compile work is essentially unchanged; noisy compiler
wall/cycle differences are not a speed claim.

The standard six-program public screen passes against GCC at `-O0` and `-O3`,
with identical output hashes and unchanged executable sizes. Most instrumented
counts agree to startup noise. FASTA's first runner observation increased about
0.1%; its generated instruction sequence differs only in one signed reload.
A controlled repeat with fixed argv0, disabled ASLR and identical launch
conditions gives the same instruction count within 0.000005%, with identical
output hashes. The earlier count difference is not reproduced under that
control; no small native timing improvement is claimed.

## Rejected experiments and next gaps

A direct-local-address nonnull proof removes more Brainfuck instructions but
changes Huffman encoding's allocation and loop shape, producing a reproducible
roughly 11% cycle loss. It is excluded.

A diagnostic dense-switch rewrite of Brainfuck's pure comparison ladder also
passes all 50 small checksum checks, but executes 0.71% more instructions and
about 15.6% more cycles than the matched optimized-IR control. The existing
switch policy is unchanged; this workload does not justify a broader rule.

Arithmetic decoding still has signed midpoint division in a bounded binary
search. Replacing that work requires proving bounds across the mutually
updated loop phis and its backedge guard. Signed source types alone are not a
proof: Myc's integer operations wrap unless LowIR states otherwise. LogParser's
regex state and NeuralNet's dot-product live ranges remain separate gaps.

A diagnostic replacement of only that midpoint divide with a logical shift,
compared with the same reoptimized-IR control on CPU43, passes all 50 small
checks and the full arithmetic workload. It saves 13.17% of arithmetic-decode
instructions and 4.50% of cycles (1.047× cycle efficiency); A/A cycles differ
0.30%. This source-informed experiment is not a production proof or a compiler
change. Its larger payoff motivates the next optimizer investigation:

1. Infer a candidate interval from constant external seeds of a small loop-phi
   group. Treat all group intervals as simultaneous hypotheses.
2. Evaluate incoming arithmetic conservatively and refine the continuing edge
   with its actual comparison. Verify that every external seed and every
   continuing update stays inside the proposed intervals before accepting any
   result. Reject unknown inputs, overflow, unsupported operations and exceeded
   query budgets. Wrapping additions cannot inherit nonnegativity merely from
   nonnegative operands.
3. Reuse the existing loop/counter analysis and examine only relevant arithmetic
   dependencies, rather than adding an unconditional whole-function range pass.
   Check independent loop shapes, swapped guards, different seeds, widths,
   zero-work paths and wrapping counterexamples, plus compile-cost/public gates.

No frontend special case or new serialized fact has been demonstrated necessary.

## Qualification

The new PA33 fixture checks recurrent neighboring fields, signed/unsigned
8/16/32-bit views, conditional whole-object copies, genuine width overlap,
Boolean conversion and volatile observations. It runs before and after
optimization through the standard Perl harness and uses broad work bounds,
without prescribing instructions or registers. Student notes describe the
requirements and proof obligations.

The strict report passes 10,630/10,630. Debug-info, PA36 smoke, self-host through
PA5 and byte-identical inception pass. Architecture, source-set, scaffold,
format, file, placement, harness and student-export checks pass. The fresh
export includes the new fixture, its references and student notes; all 43
reference wrappers install and execute, and the new reducer passes using
those supplied tools. GCC15 and Clang21 compile the changed modules with warnings as errors. The Clang probe selects the installed GCC15 headers explicitly; the first
unconfigured probe could not find the standard headers. The export identified
one changed existing PA33 arithmetic snapshot; it is regenerated with the
normal reference target.
The final compiler emits the same Myc object as the measured candidate.

Every observation, compiler/input hash, fixed-launch control and diagnostic
binary is retained under `/mnt/vizier/cppgm-issue115-data/corrected-outliers-20261111`. The [JSON record](field-views-20261111.json)
contains the summaries and provenance. The fresh whole-suite paired comparison
is recorded there; earlier Clang ratios are not multiplied by
these focused improvements.

## Full corrected-suite control

Two checkpoint A/A observations precede one checkpoint/candidate/candidate/checkpoint block. All six runs pass all 50 checks.

| Measurement | Checkpoint mean | Candidate mean | Efficiency ↑ |
|---|---:|---:|---:|
| seconds | 94.3428 | 92.4715 | 1.0202× |
| instructions, billion | 788.8376 | 788.3751 | 1.0006× |
| cycles, billion | 427.8393 | 428.8206 | 0.9977× |

The full suite saves 0.0586% of instructions. Its measured 2.02% wall gain is smaller than the 4.02% A/A wall spread; its 0.23% cycle increase is smaller than the 0.28% A/A cycle spread. These controls establish reduced work, not an aggregate runtime speedup. All observations and case times are retained.

The [coupled-loop-bound follow-up](LOOP_BOUNDS.md) implements the midpoint proof, records its qualification and compares the remaining arithmetic gap with GCC and Clang.
