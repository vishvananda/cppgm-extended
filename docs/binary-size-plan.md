# Binary size investigation and implementation

The target is at most 1.15 times GCC's `.text`, the existing PA35 code-size
metric, while preserving native speed and compiler throughput. Whole executable
size is reported separately: the last complete changed-source build meets the
code-size target but its stripped ELF is still 1.163 times GCC's. Subsequent
unwind changes are measured on frozen source below. This is an implementation
review record, not a student handout.

## Evidence and size budget

All comparisons use GNU C++11, `-O3`, libstdc++ and
`-fno-strict-aliasing`, with identical source and flags within each pair.
The frozen comparison uses the 251 compiler translation units at
`8d4c68b9e379`; rebuilding the changed source separately checks that the saving
survives the implementation itself.

| Build | GCC `.text` | cppgm++ `.text` | cppgm++ / GCC |
| --- | ---: | ---: | ---: |
| Frozen baseline | 5,596,214 | 7,143,970 | 1.2766 |
| Frozen source, candidate code generator | 5,596,214 | 6,400,514 | 1.1437 |
| Changed source, repository build flags | 5,599,798 | 6,407,202 | 1.1442 |
| Changed source, student build flags | 5,585,558 | 6,407,794 | 1.1472 |

The frozen saving is 743,456 bytes, or 10.41%. The student configuration has
15,598 bytes of headroom below 1.15 times GCC. This margin is small; size must
remain a gate, including after future source or host-toolchain changes.

The ordered ablations save 53,776 bytes from adjacent zero stores, 26,944
from EH-aware dead stores, 635,136 from the inlining/promotion policy, and
27,600 from unused machine epilogues. These are incremental measurements in
that order; the transformations can interact.

Reachability checks find no disconnected LowIR blocks in three representative
inputs, including exception edges. Linker garbage collection removes only
47,904 compiler text bytes. The main excess is larger retained function
families: in a Boost container program, cppgm++ emits 33% more decoded
instructions at nearly the same average instruction length. Extra moves and
address calculations account for roughly two thirds of its extra decoded
bytes. A deliberately restrictive inlining control greatly reduces size,
confirming duplication as an important cause, but loses useful optimization.

## Implemented candidate

1. **Preserve hot inlining and reduce repeated expansion.** Recognize fast
   returning wrappers without treating an early return around a loop as cold.
   Bound additional copies of larger call-containing callees outside loops in
   already large callers. Preserve small accessors, call-free callees, loop and
   handler sites, mandatory and constant-folded copies, and copies that remove
   the callee definition. Retain the existing global caller and unit budgets.
2. **Avoid converting conditional floating state into spill traffic.** For a
   larger set of accessed floating fields, retain addressable fields whose
   promotion needs additional non-header merges inside a loop. Preserve ordinary
   scalar promotion and header-only loop state. Earlier blanket field limits
   regressed other numerical programs and were rejected.
3. **Remove redundant stores.** Combine complete, adjacent ordinary zero stores
   into bounded bulk zeroing without crossing holes, nonzero values, volatile
   accesses or atomics. Remove dead frame stores in host exception-bearing
   functions using handler liveness at each throwing instruction, with bounded
   analysis memory and work. A later normal overwrite cannot kill a value a
   handler observes.
4. **Remove unreachable shared epilogues.** When every emitted exit already
   returns or transfers control, omit the unused common return sequence and
   its unwind rows. Keep implicit fallthrough and shared-return branches
   conservative. Preserve call-frame and stack-argument rules for calls after
   the last normal return, including cleanup landings.

5. **Share external EH references.** Use hidden weak COMDAT indirection cells
   for non-local personality and type-information symbols. The linker can then
   merge repeated cells and common unwind records. Keep cells for local type
   information private. Preserve ELF symbol visibility in the object model.
   This saves 13,216 allocated section bytes on the frozen compiler, including
   6,432 bytes of unwind records and 5,088 bytes of dynamic relocations; file
   alignment leaves a 9,888-byte stripped-file saving. All 251 objects retain
   identical executable-section bytes and relocation targets. The frozen
   stripped executable is now 7,493,992 bytes, or 1.16194 times GCC.
6. **End unwind coverage at the code boundary.** Keep the encoder's instruction
   extent separate from trailing jump-table data. Restore saved body rules
   after an epilogue only when more instructions follow it. On the frozen
   compiler, all decoded frame rules within 9,233 code ranges and all 343
   stack-argument transition addresses and values match the preceding
   implementation. The FDEs exclude 40,710 bytes of trailing data in 317
   functions. Unwind records shrink by 568 bytes; file alignment gives another
   4,096-byte saving, to 7,489,896 bytes (1.16131 times GCC). PA26/PA33 and PA35
   smoke checks pass. Scoped compile instructions remain unchanged; timing is
   inconclusive because A/A calibration drifts by up to 16.6% in cycles.

These changes use existing typed LowIR and MIR information and need no format
extension. Required PA32/PA33 fixtures execute both normal and exceptional
paths, check retained work with broad quality bounds, and cover conditional
floating state through an independent scalar oracle. Student guidance discusses
the proofs and tradeoffs without publishing implementation thresholds.

## Acceptance and further work

- Freeze both binaries and inputs, verify outputs, calibrate A/A, then retain
  mirrored ABBA observations. Check the 81 public native programs, compiler
  execution at O0/O3, PA35 kernels and the affected Boost container workload.
  Use instructions to explain changes; use calibrated timing and cycles to
  assess speed. Shared-host measurements with failed calibration are inconclusive.
- Require affected suites, the strict report, debug-info checks, self-host
  through PA5, byte-identical inception, PA35 smoke and release checks, 2,000
  generated seeds, all architecture/format/placement audits, harness tests and
  a validated student export. The strict report passes 7,990/7,990, and
  debug-info, architecture audits, self-host through PA5, inception, harness,
  export and all 6,002 generated programs at O0–O3 pass. PA35's course build
  passed 17/17 gates before epilogue cleanup. Its complete release check
  reported insufficient text and lookup-kernel margins; the initial student
  build also exceeded the text limit. The revised builds both meet the text
  limit, but still lack the release check's additional 1% size margin. The
  lookup result (1.2827 times GCC's instructions) is unchanged and lacks its
  required 2% margin. Do not relax the gates or edit the calibration to hide
  these findings.
- The revised candidate matches all 81 public outputs, with 1.0032 times
  baseline instruction efficiency and no individual increase above 1% in the
  counter screen. Six mirrored paired blocks give about 0.998 times baseline
  aggregate CPU speed, within this shared host's noise. FASTA6's four threads
  needed a four-core rerun: its single-core A/A instruction drift was 41%.
  All original observations are retained. A targeted
  Spirit rerun is about 0.982 times baseline CPU speed; that outlier remains
  under review. The candidate is not accepted for main under a requirement
  that every program retain its speed.
- The complete 251-file rebuild using the actual changed compiler at
  `6464fa34e` has instruction efficiency 1.0066 and cycle efficiency 1.0141
  against the original self-built compiler. All six mirrored blocks improve
  cycles beyond the A/A block spread. CPU and wall-time efficiencies of
  1.0076 and 1.0042 remain within shared-host A/A variation. Every repeated
  object and linked frozen compiler reproduces its side's expected hash.
- EPYC CI run 41624091729 passed at `6464fa34e`: the published O3 native score
  is 0.98 times GCC, self-built execution 0.99, and O3 compile speed 4.1.
  This is evidence for that revision; subsequent changes need their own CI
  checks. The EH-cell change passes 424 PA26–PA28 tests and mixed GCC/Clang
  external-EH controls, including PIC shared links without text relocations.
  Its scoped paired compile instructions are unchanged; timing is within
  A/A variation. A pre-existing mismatch between Clang and cppgm for
  same-named anonymous-namespace exception types is recorded separately and
  does not establish a regression from this change.
- For a **whole stripped executable** target of 1.15 times GCC, another
  approximately 73 KB must go on the frozen comparison. Prioritize unwind-table
  and repeated ABI-entry
  metadata, then redundant address/copy instructions. Do not obtain the saving
  by deleting required unwind information or accepting poorer hot code.
- A diagnostic survey finds about 104 KB of apparently matching constructor
  and destructor entry bodies. Sharing is a later option only with typed proof
  of equivalent base/complete-object behavior, including tail padding, virtual
  bases, EH, symbol replacement and COMDAT. Matching instruction bytes is not
  sufficient proof.
  If sharing needs new facts across the LowIR boundary, document their public
  semantics and test direct and serialized routes. The current changes add
  no LowIR facts.

Raw measurements, prototypes (including rejected policies) and commands live
outside the repository in
`/home/vishvananda/work/boost-1.92/validation/20261103-size-implementation`.
The original size attribution is in the adjacent
`20261103-code-size-attribution` directory. `~/comparison.md` links the full
evidence and records the final acceptance status.
