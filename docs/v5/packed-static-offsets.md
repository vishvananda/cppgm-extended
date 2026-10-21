# Packed scalar images and wide symbol operands

The C++ layout builders already record member offsets and explicit padding.
LowIR structured data, however, naturally aligns every typed item. Keeping an
`i32` item at packed byte offset five therefore inserted three unwanted bytes
in the native image. An earlier dynamic observer could read the wrong static
value even though initialization had been evaluated correctly.

`PreserveStaticOffsets` checks the finished typed data at the publication
boundary, after either evaluated or directly folded aggregate initialization.
It rewrites only unaligned scalar items as their little-endian representation
bytes, including floating payloads and wide integer padding. Aligned items and
LowIR's existing natural-alignment contract remain intact. Reference-bound
constant temporaries use the same normalization. The pass is linear; the
replacement vector is allocated only when an unaligned scalar is found.

The native selector also rejects a symbol address used as an `i128` value
before its operand reaches a wide encoder. This covers arithmetic, comparison,
unary operations and truncation. The former truncation success read an unset
immediate field as zero; it was not valid address truncation. Pointer operations
must state their actual source type. Unsupported wide-symbol operands now
produce ordinary input errors rather than internal invariant failures.

The new PA11 control checks packed scalar tails, over-wide signed fields,
arrays, floating fields, constant temporaries and observation before dynamic
initialization. Nine executed PA24 rejection fixtures cover the wide-symbol
boundary. All 182 accepted programs in the Rust audit's object-image matrices
now execute successfully, including the 50 previously failing cases; two
alignment-reduction sources remain ordinary rejections.

Local validation: strict report **7,083/7,083**, debug-info lanes, self-hosted
PA1–PA5, PA35 smoke, architecture/file/format and placement audits. The existing
five header-weight warnings remain warnings. PR CI validates the complete
compiler/standard-library matrix and inception.

Frozen GCC-built binaries from `3ef7bd25` and this patch produce byte-identical
objects for the manifest-verified PA35 workload at O0 and O3. Each level has two
A/A calibration blocks and four alternating ABBA/BAAB blocks, retaining all 24
observations. Geometric instruction ratios are **1.000002 at O0** and
**0.999999 at O3**, within A/A variation; no instruction-count regression is
measured. Cycle ratios (1.010851 and 0.979274) are noisy on the shared host and
do not establish a throughput change.

Raw observations, binary hashes and matrix replays are retained on Alpha under
`~/work/maintenance/packed-data-wide-20261021/`.
