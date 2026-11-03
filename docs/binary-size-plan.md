# Binary size investigation and implementation

The target is at most 1.15 times GCC's `.text`, the existing PA35 code-size
metric, while preserving native speed and compiler throughput. Whole executable
size is reported separately: the current candidate meets the code-size target
but its stripped ELF is still 1.174 times GCC's. This is an implementation
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
| Frozen source, candidate code generator | 5,596,214 | 6,428,114 | 1.1487 |
| Changed source, candidate self-build | 5,600,054 | 6,431,922 | 1.1485 |

The frozen saving is 715,856 bytes, or 10.02%. The changed-source pair has
8,140 bytes of headroom below 1.15 times GCC. This margin is small; size must
remain a gate, including after future source or host-toolchain changes.

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
  passes 17/17 gates; its complete release-margin check is still running.
  The public native geometric mean is 1.0025 times baseline CPU speed, but a
  ten-block Spirit rerun is 0.956 times baseline with good A/A calibration.
  That outlier is not performance-neutral; the candidate is not accepted for
  main under a requirement that every program retain its speed.
- Recheck the EPYC CI score before merging. Local Intel results cannot prove
  performance on another microarchitecture.
- For a **whole stripped executable** target of 1.15 times GCC, another
  approximately 156 KB must go. Prioritize unwind-table and repeated ABI-entry
  metadata, then redundant address/copy instructions. Do not obtain the saving
  by deleting required unwind information or accepting poorer hot code.
- Remove unused machine epilogues before pursuing broader sharing. A private
  prototype omits the shared return sequence when every emitted path already
  returns or transfers control, saving another 27,600 frozen compiler text
  bytes (1.1437 times GCC). This is unreachable machine code outside the
  explicit LowIR block graph. Keep implicit fallthrough, nonduplicated returns
  and required unwind rules; host EH and runtime validation are still pending
  for this separate prototype.
- A diagnostic survey finds about 104 KB of apparently matching constructor
  and destructor entry bodies. Sharing is a later option only with typed proof
  of equivalent base/complete-object behavior, including tail padding, virtual
  bases, EH, symbol replacement and COMDAT. Matching instruction bytes is not
  sufficient proof.

Raw measurements, prototypes (including rejected policies) and commands live
outside the repository in
`/home/vishvananda/work/boost-1.92/validation/20261103-size-implementation`.
The original size attribution is in the adjacent
`20261103-code-size-attribution` directory. `~/comparison.md` links the full
evidence and records the final acceptance status.
