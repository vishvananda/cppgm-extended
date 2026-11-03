# Binary size investigation and implementation

The target is at most 1.15 times GCC for both `.text` and the whole stripped
executable, while preserving native speed and compiler throughput. The first
equivalent-entry checkpoint meets both limits. The current frozen-source
build has 6,314,946 text bytes (1.12843 times GCC) and a 7,399,784-byte
stripped executable (1.14734 times GCC). Final performance, release and export
validation remains in progress. This is an implementation review record, not
a student handout.

## Evidence and size budget

The frozen and repository comparisons use GNU C++11, `-O3`, libstdc++ and
`-fno-strict-aliasing`; the separate default/student configurations enable
strict aliasing. Source and flags match within each pair.
The frozen comparison uses the 251 compiler translation units at
`8d4c68b9e379`; rebuilding the changed source separately checks that the saving
survives the implementation itself.

| Build | GCC `.text` | cppgm++ `.text` | cppgm++ / GCC |
| --- | ---: | ---: | ---: |
| Frozen baseline | 5,596,214 | 7,143,970 | 1.2766 |
| Frozen source, unwind checkpoint | 5,596,214 | 6,400,514 | 1.1437 |
| Frozen source, equivalent entries and strong aliases | 5,596,214 | 6,314,946 | 1.1284 |
| Changed source, first equivalent-entry checkpoint | 5,610,358 | 6,330,146 | 1.1283 |
| Changed source after unwind cleanup, repository flags | 5,603,958 | 6,408,770 | 1.1436 |
| Changed source after unwind cleanup, PA34 default flags | 5,590,294 | 6,409,394 | 1.1465 |
| Student plain flags, preceding checkpoint | 5,585,558 | 6,407,794 | 1.1472 |

The frozen text saving is now 829,024 bytes, or 11.60%. The first entry-sharing
changed-source pair is 7,420,264 versus 6,465,920 stripped bytes (1.14760 times
GCC); this predates the direct strong aliases. The preceding student plain configuration has
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

7. **Share equivalent ABI entries after inlining.** The lowering records a
   public, serialized `equivalent_entry` proof for eligible complete/base
   entries of one source function. Keep both bodies available to inlining.
   Exclude virtual-base, synthesized, replaceable and otherwise unproved cases.
   At optimized object emission, strong entries share the retained body
   directly; weak entries keep independently available forwarding definitions
   in their own COMDAT groups. O0 does not build or consult the equivalence
   index, and optimized units with no candidates stop looking up entries.
   Pruning drops proofs whose entries disappear. PA8 validates the contract;
   PA12/PA27/PA32 explain the semantic and linking obligations. Tests cover
   normal and exceptional behavior, mixed GCC/Clang linking in both orders,
   serialized object replay, pruning and a broad code-footprint bound.

The first six changes use existing typed LowIR and MIR information. Entry
sharing adds the public fact above, including reader, writer and validator
support; no optimizer relies on a private side channel. Required PA32/PA33 fixtures execute both normal and exceptional
paths, check retained work with broad quality bounds, and cover conditional
floating state through an independent scalar oracle. Student guidance discusses
the proofs and tradeoffs without publishing implementation thresholds.

## Acceptance and further work

- After reaching the size target, profile the reduction passes and remove
  avoidable analysis, allocations and repeated work. Require paired O0 and O3
  compile-cost checks, including both GNU-built and self-built compiler paths.
  Keep fast paths cheap when an optimization cannot apply; do not give back
  size or native speed to improve compile time. The initial CI comparison
  suggests O0 compile speed relative to GCC fell about 1.9%; compilation was
  not paired there, so reproduce it locally before attributing its cause.
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
- A later fixed-input GNU-built compiler comparison measures O0 instruction
  efficiency 0.9993 and O3 efficiency 1.0139. O0 timing remains inside A/A
  variation. The matched-flags self-built comparison gives O0 instruction
  efficiency 0.9912 (about 0.9% extra work) and O3 efficiency 1.0027. Its
  timing remains inconclusive across A/A spread. Each side emits byte-identical
  objects between GNU-built and self-built compilers at both levels. Retain
  and label the first self-built comparison as confounded by aliasing flags.
- EPYC CI run 41634484750 passed at `e231b55da`, before entry sharing.
  Published O3 native speed is 0.98 times GCC, self-built execution 1.00,
  O3 compilation 4.1. The prior run's 9% pugixml deficit did not repeat:
  its latest score is 0.9753 versus main's 0.9836 (about 0.8% lower, with
  A/A variation about 0.9%). Spirit is about 1.1% better than main in that
  run. Preserve the earlier observations; they do not establish a persistent
  9% regression. New entry-sharing changes need their own CI measurements.
- Entry sharing's scoped GNU-built compile-cost checks preserve O0 instruction
  counts. On a frozen native-object source with eligible entries, O3 uses
  about 0.25% fewer instructions than the preceding unwind checkpoint.
  Replacing strong forwarders with aliases adds no measurable instruction
  cost at either level. A/A timing drifts, so no timing improvement is claimed.
  Recheck cumulative GNU-built and self-built O0/O3 cost after the final build.
- Whole-file size now meets 1.15 times GCC on frozen source and the first
  changed-source entry-sharing pair. Rebuild the final changed source with
  matching flags and repeat the plain/student configuration. Keep the target
  as a gate: the whole-file margin remains much smaller than the text margin.
- The entry-sharing checkpoint passes the strict report (8,005/8,005), debug
  information, self-host through PA5, byte-identical inception, smoke tests,
  architecture audits and harness tests. The direct-alias follow-up is under
  broader validation. Finish the generated-seed campaign, student export,
  release check and paired public native measurements before acceptance.

Raw measurements, prototypes (including rejected policies) and commands live
outside the repository in
`/home/vishvananda/work/boost-1.92/validation/20261103-size-implementation`.
The original size attribution is in the adjacent
`20261103-code-size-attribution` directory. `~/comparison.md` links the full
evidence and records the final acceptance status.
