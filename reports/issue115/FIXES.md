# Issue 115: production fixes

These changes implement the general transformations identified in the
[initial analysis](README.md). They are on `fix/issue115-lowir-gaps`, based on
LLVM worktree revision `8a56e881184e4e2161b4c2973f22e565f9e83bb1`.
The Myc frontend and frozen issue input are unchanged. No benchmark names or
function-name filters appear in the compiler.

## Changes and ownership

| Gap | Change | Owner |
|---|---|---|
| Private i1 storage read as bytes | Promote compatible byte reads with an explicit zero extension; retain escapes, volatility and mixed writes. Normalize wider register values before native i1 stores, including at O0. | PA8 representation, PA24 stores, PA33 promotion |
| Equivalent object-value transfers miss scalar replacement | Resolve SSA aliases and express memory snapshots and object writes consistently before aggregate partitioning and vector packing. Preserve snapshot timing. | PA33 |
| Positive counters still use signed remainder correction | Derive a nonnegative range from an invariant start and a proved nonwrapping add-one recurrence. Simplify power-of-two signed division/remainder only within that proof. | PA33 |
| Unused heap output keeps an entire payload computation | Prove a private destination is write-only and nonescaping. Share bounded helper copies that discard only that output while preserving results and other effects. Retain allocation/free when remaining effects could observe allocation failure. | PA33 |
| Full-width dynamic unsigned division on small actual operands | Check both operands' upper halves, use a word divide when they fit, and retain the original full-width operation otherwise. Preserve address dependencies and remap all block references. | PA34 |

The Boolean fix belongs in the LowIR optimizer, before CFG simplification and
unrolling. Handling it only during machine selection would recover the byte
value but leave the earlier optimization opportunities blocked. It does not
require a Myc frontend change or a new LowIR operation.

The allocation pass has explicit work limits: at most 32 helper copies, 512
instructions per copied body and 8,192 copied instructions per module. Call
sites share each proven parameter slice. Escapes, pointer comparisons, loads
from the destination, volatile accesses, unknown callees, multiple private
arguments, assembly and exception-sensitive helper bodies prevent slicing.
Dead output and an elidable allocator are separate proofs: an unrelated
`errno` read keeps the allocator even when its buffer writes are dead.

## Student requirements

The new required fixtures use the normal Perl batch runner in their owning
assignments. They check behavior, preserved effects and generous quality
bounds. They do not prescribe instruction sequences or benchmark-specific
patterns. The PA33 README additions state those requirements; design notes
explain general proofs and tradeoffs. This report and the diagnostic scripts
are outside the student document inventory.

The six reducers cover wider Boolean writes and byte reads; object aliases and
snapshots followed by source mutation; nonnegative, negative and wrapping
counters; dead versus read, escaped, aliased and volatile heap outputs, including
allocation-error observations; and variable division on both sides of 2^32,
including high-bit and maximum-width values.

## Base64 benchmark correction

The original encode benchmark consumes only the returned length. Removing its
unused encoded payload is valid and agrees with Clang; that timing does not
measure encoding throughput. This [source patch](base64-consume-output.patch)
compares every newly encoded byte with the prepared expected encoding before
freeing the buffer:

```c
size_t actual_len =
    b64_encode(encoded_buf, data->input_str, data->input_len);

if (actual_len != data->encoded_len ||
    memcmp(encoded_buf, data->encoded_str, actual_len) != 0) {
    abort();
}

data->result_val += (uint32_t)actual_len;
free(encoded_buf);
```

The existing checksum is unchanged. This measures encoding plus a comparison
scan; it needs no volatile accesses or compiler-specific barrier. The source
change is a separate benchmark correction, not a prerequisite for optimizing
the original legal LowIR.

## Measurements

The final original-input runs use the identical frozen LowIR, `-O3`, generic
x86-64 and the same sixteen physical cores. The compilers are immutable GCC
builds of the latest LLVM baseline and compiler change `1d02033a9`.
Clang A/A calibration precedes a baseline/candidate/candidate/baseline block;
GCC supplies an additional context run. All seven full runs pass 50/50 checks.

| Measurement | Before | After | Change |
|---|---:|---:|---:|
| Full-suite time, paired means | 119.941 s | 99.706 s | **1.203× speed; 16.9% less time** |
| Executed instructions, paired means | 913.548 billion | 777.479 billion | **14.9% fewer** |
| Speed versus Clang, context runs | 0.696× | 0.838× | higher is better |
| Speed versus GCC, context run | 0.759× | 0.914× | higher is better |

The Clang A/A totals differ by 4.0%; baseline totals by 0.5%, candidate totals
by 4.8%. Some individual cases vary much more. Thus the large aggregate gain
is clear, while small per-case changes in the [complete table](fixed-table.md)
need the pinned-core checks below. Cross-compiler ratios are context, not a
new claim of runtime parity. Do not compare these absolute times with the
initial analysis's different measurement window.

Two focused ABBA blocks per case on a separate physical core give:

| Case | Before (s) | After (s) | Speedup ↑ | Instructions after/before ↓ |
|---|---:|---:|---:|---:|
| Etc::TextRaytracer | 12.1830 | 1.8540 | 6.571× | 0.2154 |
| Base64::Encode | 2.1535 | 0.2075 | 10.378× | 0.0892 |
| Etc::LogParser | 2.0688 | 1.1943 | 1.732× | 0.6428 |
| Hash::SHA256 | 2.7668 | 1.9275 | 1.435× | 0.7729 |
| Compress::HuffDecode | 2.0960 | 1.5840 | 1.323× | 0.6614 |
| Compress::BWTEncode | 2.5848 | 2.2005 | 1.175× | 0.9579 |
| Maze::Generator | 3.7745 | 3.5845 | 1.053× | 0.8961 |
| Compress::ArithDecode | 2.3885 | 2.2957 | 1.040× | 1.0287 |
| Compress::HuffEncode | 1.5845 | 1.6538 | 0.958× | 1.0000 |
| Compress::BWTDecode | 1.4030 | 1.4265 | 0.984× | 0.9889 |
| Compress::LZWDecode | 1.7323 | 1.7112 | 1.012× | 0.9949 |
| Matmul::Single | 1.6502 | 1.6547 | 0.997× | 1.0000 |
| CLBG::Spectralnorm | 1.7450 | 1.7790 | 0.981× | 1.0000 |

The consumed-buffer Base64 control runs at **1.002× Clang** across three paired
blocks (2.388 s GM, 2.392 s Clang), with its existing checksum passing. This
is a different workload from the original length-only test; its encoding and
comparison remain live in both compilers.

### Remaining gaps and timing losses

Huffman encode is a remaining regression: about 4% slower in the focused
blocks, and about 7% with identical executable-name/argv/stack-launch conditions.
Those checks retain A/A observations too, whose individual paired blocks vary
by 1.6–9.8%. Executed instruction and branch counts are essentially unchanged,
as are branch misses. Its setup now uses fewer object copies and a smaller
frame, but the changed register assignment and branch positions also change
hot-loop encodings. A further pair of blocks shows 10.3% more cycles and 3.67×
the `idq_uops_not_delivered.core` count. This suggests a native instruction-
delivery/layout cost, not extra semantic or payload work; it does not prove a
specific alignment remedy. No benchmark-specific padding or old-frame-size
exception was added to conceal it. This branch is not a zero-regression or
all-workloads-at-parity claim.

Spectralnorm changes from a 1.9% loss in the first focused blocks to a 1.8%
gain under the fixed launcher. BWT decode changes from a 1.6% loss to 0.7%,
inside its A/A variation. Matmul single-thread instructions are unchanged and
time is within 0.3%; LZW decode improves slightly. The large compression-case
losses in one full run do not reproduce at that scale when pinned separately.

Maze generation, Huffman decode and several search/collection cases still
trail the native reference compilers substantially. The fixes address the
identified proofs and canonicalization gaps; they do not exhaust the native
loop/layout, allocation and call-cost opportunities. Inspect those remaining
loops against LLVM/native output before adding IR facts or raising budgets.

### Compiler-speed sanity check

The frozen PA36 semantic-overload workload uses its documented long-double
macro replacements, the same includes and input manifest, and immutable
compilers. Five A/A blocks, five A/B blocks and three hardware-counter blocks
per optimization level retain 80 compile timings and 24 counter observations.
Each compiler's output is deterministic; O0 A/B objects are byte-identical.
O3 output changes are expected from these optimizer fixes.

| Compile level | Instructions after/before ↓ | Paired wall time after/before ↓ | Paired user time after/before ↓ |
|---|---:|---:|---:|
| O0 | 1.00005 | 1.0119 | 1.0000 |
| O3 | 1.00815 | 0.9868 | 0.9877 |

The O0 timing change is inside the A/A spread (including a 2.1% median wall
imbalance); no clear timing regression is established. The bounded additional
O3 analysis costs about 0.8% more compile instructions for the measured runtime
gains. Peak RSS is effectively unchanged at O0 and rises about 0.08% at O3.

[fix-observations.json](fix-observations.json) preserves all 208 runtime
observations, 80 successful compile timings and 24 compile counter observations,
with commands, binary hashes and raw-file hashes. Raw stdout, stderr, counters,
immutable binaries, fixed-launcher source and earlier invalid command attempts
remain under `/mnt/vizier/cppgm-issue115-data/fixes`. The original reproduction
inputs remain under `../20261110` relative to that directory.

## Qualification

The combined source passes the full strict course report (10,595/10,595),
architecture/layout/source-set/scaffold/format audits, file audit, debug-info,
self-host through PA5 and PA36 smoke correctness. The new reducers and all
50 small index-c checks pass. A guard-page probe for wider-byte-to-i1 stores
also passes at O0 through O3.

Final qualification also passes:

- Full strict report after reference refresh: 10,595/10,595.
- Inception: rebuilt `cppgm++` is byte-identical.
- Student export: 24,193 portable reference outputs verified, including 9,738
  exit-status files; 256-document inventory, document audit and all 43 reference
  wrappers pass. The changed handouts match the validated export byte for byte.
- PA36 release check: all 17 gates pass with release margins in both course
  and student builds, using 100 generated seeds each. Text-size ratios are
  1.144 and 1.147; codegen instruction ratios are 1.0017/1.0444 (O0/O3) for
  the course build and 1.0056/1.0586 for the student build.

Logs and release-calibration snapshots are retained with the raw dataset.
The release check's generated calibration files are not included as unrelated
course-data changes in this optimization branch.

Export validation also exposed stale references inherited from the newly
qualified O1 baseline. A direct baseline compile reproduces the changed masked
Boolean reference. A separate commit regenerates affected text references
through the normal harness and removes 31 tracked ELF test executables;
portable MIR and program-outcome sidecars remain. Generated executable bytes
are not a student correctness contract.
