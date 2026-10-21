# Native value operands and literal-pool safety

The PA24 native selector used `Value.index` as a literal-pool index before
checking whether a wide copy/store operand was a slot or symbol. Slot copies
could crash even though their value was subsequently replaced with a frame
operand. Symbol operands could silently read unrelated literals: adding a dead
`const i128 99` changed the bits stored by `store i128 @g, $slot`.

Memory-class operands now share one kind-based dispatch for copy, store, call
arguments and returns. Slots read their contents, temporaries retain their
selected locations, and only numeric operands reach literal lookup. Unsupported
symbol-as-wide-value forms reject with an ordinary backend input error. Object
copy/store continues to accept address registers for by-address parameters;
numeric object copy/store operands reject instead of reaching the encoder.

The associated representation-boundary checks also fix integer-carrier copies
into floating registers: `movd`/`movq` preserve bits, without numeric conversion.
Floating symbol values and switch selectors reject normally. Constant
`stack_alloc` sizes that cannot use the aligned signed-32-bit adjustment reject
before emission, and frame-resident dynamic sizes are loaded before adjustment.

Sixteen executed PA24 behavior fixtures cover wide slot contents, floating
carriers at O0/O3, unrelated-literal isolation, and ordinary input rejection.
References were generated with the course runner. The audit's 45 formerly
crashing/internal-error probes now produce 22 successful compilations and 23
ordinary rejections, with no signal or internal-error exit. The two additional
symbol-store probes that previously succeeded with unstable values now reject.

## Validation and performance

Local validation includes the strict course report (7,072/7,072 before the
additional passing frame-resident-size regression), debug-info lanes,
self-hosted PA1–PA5, PA35 smoke, architecture/file/format audits, and test
placement. The four existing header-body file-audit warnings are unchanged.
The upstream PR runs the complete compiler/toolchain and inception matrices.

Performance uses immutable GCC-built binaries from main `b6ebdf57` and this
patch, with the unchanged manifest-verified PA35 workload. Every measured
baseline/patched object is byte-identical. Per optimization level, two A/A
calibration blocks and four alternating ABBA/BAAB blocks retain 24 hardware
counter observations. Ratios are geometric means of within-block means.

| Level | Instructions patched/baseline | Cycles patched/baseline |
| --- | --- | --- |
| O0 | 0.999996 | 1.012653 |
| O3 | 1.000009 | 1.000962 |

Instruction differences are smaller than A/A variation (up to 0.0057% at O0
and 0.0033% at O3). Cycle samples are noisy on this shared host; paired-block
intervals include parity. There is no measured instruction-count regression
on this workload, and these results do not establish a throughput speedup.
The kind check is confined to memory-class operands, leaving the hot numeric
literal accessor unchanged.

Raw observations, binary hashes, probe receipts and validation logs are retained
on Alpha in `~/work/maintenance/native-values-20261021/`. Only the final
`paired-perf.json` measures the final patch; `paired-perf-initial.json` preserves
an earlier, broader guard placement for comparison.
