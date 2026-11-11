# Corrected Myc outliers: address calculations and byte updates

This round starts from main `04d285786`, with both the Myc index-width correction and the consumed-buffer Base64 source correction applied. The two largest gaps in that comparison were Brainfuck array interpretation (0.744× Clang) and Huffman decoding (0.766× Clang). Those ratios describe the earlier comparison window; the measurements below compare this candidate directly with main on identical corrected inputs.

The candidate saves **6.62% of Brainfuck array instructions and 4.78% of cycles**, and **5.63% of Huffman decode instructions and 4.87% of cycles** in focused paired runs. These follow from typed widths, existing address facts and liveness; no source-name matching or new LowIR fact is used.

## General changes

- **Same-width integer memory updates (PA34).** An unsigned byte load followed by an unsigned update and a signed byte store has the same stored low bits. The selector previously rejected the signedness difference, leaving a separate load, operation and store. It now admits the supported low-bit arithmetic/bitwise updates across signed views of the same width. Boolean operations remain separate, old values with other users remain available, and volatile or intervening effects prevent fusion.
- **Reuse folded load addresses (PA34).** A scalar load reads its base and index before writing its destination. The allocator already handled a direct address but missed a dying component of a folded address. Reusing that register also lets preceding single-use computations share it. Huffman decode loses a register copy at each decoded bit. Live address components are preserved.
- **Scaled conditional fields (PA33).** When one bit selects between fields, absorb an avoidable left shift into the address element scale when both byte offsets permit it. Negative offsets and unaligned fields retain exact byte addressing. Only the selected field is read. This removes three shift/address operations in the unrolled Huffman loop.
- **Stable loop-window phase (PA34).** Functions whose existing compact-loop policy depends on a 64-byte window now start at a stable phase in that window. Previously their 32-byte starting alignment allowed shrinking preceding functions to add 32 bytes of internal loop-entry padding. That produced a 1.8% instruction increase in arithmetic decode despite identical useful instructions. The corrected policy restores its original instruction count and leaves functions without that window requirement on their existing policy.

## Focused measurements

Intel Xeon E5-2696 v4, core 30 / NUMA node 1, generic x86-64 `-O3`, fixed argv[0], ASLR disabled. Each case has two main A/A calibrations followed by two main/candidate/candidate/main blocks. All checks pass. Runtime ratios are main seconds / candidate seconds, higher is better. User counters cover preparation, warmup and the selected benchmark; the benchmark timer excludes preparation and warmup.

| Workload | Runtime vs main ↑ | Instructions saved | Cycles saved | A/A wall spread |
|---|---:|---:|---:|---:|
| Brainfuck::Array | 1.038× | 6.62% | 4.78% | 3.2% |
| Compress::HuffDecode | 1.056× | 5.63% | 4.87% | 5.6% |
| Compress::ArithDecode | 1.073× | 0.00% | 0.36% | 0.9% |
| Compress::HuffEncode | 1.179× | -0.00% | 1.52% | 29.6% |
| Etc::LogParser | 1.126× | 0.00% | -0.14% | 1.5% |
| Etc::NeuralNet | 1.132× | 0.00% | 0.02% | 26.2% |

Brainfuck cycle efficiency improves 1.048× and 1.052× in its two blocks; its A/A cycles differ 0.29%. Huffman decode improves 1.050× and 1.053×, with a 0.18% A/A cycle spread. The remaining cases have unchanged executed work. Scheduling and shared-host frequency effects cause large wall changes without matching cycle changes; their apparent wall speedups are not established compiler gains.

The scale-only experiment saves 2.41% of Huffman decode instructions. The additional allocator change brings the combined saving to 5.63%. The direct-local-address nonnull experiment remains excluded: it reduces Brainfuck work further, but its earlier Huffman timing sensitivity needs a separate controlled investigation.

## Full corrected suite

One full main/candidate/candidate/main block follows two main A/A calibrations, on physical cores 30–33 / NUMA node 1. Every run passes all 50 checks. These are sums of timed cases, not geometric means.

| Measurement | Main mean | Candidate mean | Main / candidate ↑ |
|---|---:|---:|---:|
| Timed cases, seconds | 99.8733 | 103.0035 | 0.9696× |
| User cycles, billion | 432.9579 | 433.5107 | 0.9987× |
| User instructions, billion | 791.7908 | 788.7346 | 1.0039× |

Full-suite A/A timed totals are 110.9534 and 100.3352 seconds (10.6% range over the smaller run). This calibration is evidence of host drift, not a confidence interval. The earlier 0.970× Clang aggregate is not recomputed by multiplying separate-window ratios.

The full block reports 3.1% slower timed cases despite 0.39% fewer instructions and only 0.13% more user cycles. The largest apparent losses were therefore rechecked with A/A calibration and two ABBA blocks per case on core 30. Their 16–29% full-suite wall losses do not reproduce: binary trees and NGram have unchanged instructions and cycle efficiencies of 1.004× and 1.002×; Jaro measures 1.008× in time and cycles with unchanged instructions; LZW decode measures 1.037× in time and 1.018× in cycles. The batch establishes reduced work and the two focused cycle improvements, not a precise whole-suite runtime gain.

## Compile cost and public sanity checks

The frozen PA36 semantic-overload workload has two A/A calibrations and two ABBA blocks per optimization level. All `-O0` objects are byte-identical between compilers; each compiler is deterministic at `-O3`.

| Compile level | Candidate / main instructions |
|---|---:|
| `-O0` | 1.000023 |
| `-O3` | 0.999926 |

The measured compile work is essentially unchanged. These counter checks do not establish a small wall-time improvement.

The standard public runner checks Coyote Huffman, Stanford Bubblesort and FloatMM, binary trees, FASTA 1 and n-body 1 against GCC at both `-O0` and `-O3`. All outputs agree. Executable sizes are unchanged in all six; the five instrumented runtime counts are identical apart from 14 startup instructions in two programs. The full public and EPYC campaigns were not repeated.

## Qualification and remaining gaps

The strict report passes 10,629/10,629 tests. Debug-info checks, self-host through PA5, byte-identical inception, PA36 smoke, architecture/source-set/scaffold/format/file/placement/harness audits, and student-export validation pass. Three course fixtures exercise width/signedness updates, folded-address lifetimes and scaled field choices using execution and quality envelopes. The PA33 and PA34 handouts describe student requirements; the design notes discuss the relevant proofs. Existing PA34 loop-phi snapshots are regenerated for the changed register reuse.

The remaining profiles point to interpreter dispatch and retained tape state in Brainfuck, regex-machine state/control traffic in LogParser, signed midpoint/range work in arithmetic decoding, and dot-product live ranges plus libm cost in NeuralNet. NeuralNet spends roughly 30% of Clang samples and 25% of GM samples in exp/libm; that share alone does not explain the remaining generated-code gap. These are follow-up investigations, not completed optimizations or evidence for a new IR extension.

Raw commands, hashes, all observations, calibration runs, profiles and disassembly are retained in `/mnt/vizier/cppgm-issue115-data/corrected-outliers-20261111`. The final compiler emits a byte-identical Myc object to the measured frozen candidate; [the JSON record](corrected-outliers-20261111.json) includes that identity and the detailed per-case full-suite results.
