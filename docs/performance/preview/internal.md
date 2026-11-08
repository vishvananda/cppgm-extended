# Internal C++ comparison observations

CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz. Architecture: x86_64. Four workers per role; shared host; no affinity pin.

Fixed common positive cohort: **5614 fixture groups / 5724 C++ translation units**. Each compiler emits a valid object for every unit at O0 and O3.

This is a qualified source/object projection, not the complete original stage/IR/ABI/driver harness. Compiler and input hashes, raw qualification disagreements and exclusions are retained.

## Object compilation

Summed child CPU seconds, including compiler-driver startup and required subprocesses; linking, execution and harness overhead excluded. Speedup is native/cppgm, higher is better.

| Opt | cppgm CPU s | GCC CPU s | Clang CPU s | vs GCC | vs Clang |
|---|---:|---:|---:|---:|---:|
| O0 | 46.921 | 186.799 | 295.703 | 3.981× | 6.302× |
| O3 | 50.269 | 224.273 | 344.306 | 4.461× | 6.849× |

## Execution observations

The fresh matched normal-exit cohort contains **1,210 controls**. All three compilers pass all 1,210 at both levels before these totals can be compared. Processes and startup are included; these are suite-execution observations, not steady-state application/code-generation scores.

| Opt | cppgm CPU s | GCC CPU s | Clang CPU s |
|---|---:|---:|---:|
| O0 | 3.604615 | 3.361923 | 3.442534 |
| O3 | 3.275626 | 2.931634 | 3.305209 |

Before overlap selection, the original 1,241 runtime controls produced the following course-oracle results. Native disagreements are not automatically compiler bugs: original course contracts and vendor language epochs/runtime policies need individual review.

| Opt | cppgm | GCC | Clang |
|---|---:|---:|---:|
| O0 | 1241/1241 | 1221/1241 | 1238/1241 |
| O3 | 1241/1241 | 1220/1241 | 1233/1241 |

Thirty-one controls are outside the normal-exit overlap: differing native outcomes at either optimization level, or explicit termination checks. Their original results remain visible; none is counted as a pass in its original incompatible oracle.

## Scope and resources

The initial inventory had 9,963 tracked .t fixtures; 11 PR follow-up fixtures bring the inventory to 9,974. Of those, 8,129 groups have C++ source projections and 1,845 are token/preprocessor, ABI-fact, LowIR or driver-only contracts. Early parser/type milestone acceptance is not a full-object correctness oracle. The benchmark cohort is selected by declared positive contracts, supported metadata and successful cross-compiler qualification, before timing.

Language modes in the cohort: {'gnu++11': 5596, 'c++11': 2, 'c++17': 12, 'c++14': 4}. Per-source modes are retained for every compiler.

All roles use the same 30 s CPU limit, 2 GiB address-space limit, 256 MiB output-file limit and 64 MiB stack limit. Raw v1 observations remain: Clang hit the old 64 MiB output cap on two 64 MiB arrays, and an inlined array probe hit the old 8 MiB stack. All roles were rerun with the identical new resource contract. Two specified course-runtime terminate exits (134) are checked against permitted host SIGABRT termination separately; they are excluded from normal-exit timing.

GCC 15 had two internal compiler errors in the initial broad qualification scan; crashes are retained and never counted as successful rejection. The complete qualification scan, including unsupported metadata and native mismatches, is not replaced by this overlap subset.

## Details

- [PA7: 166 groups](details-v1/pa7.md)
- [PA10: 207 groups](details-v1/pa10.md)
- [PA11: 454 groups](details-v1/pa11.md)
- [PA12: 352 groups](details-v1/pa12.md)
- [PA13: 39 groups](details-v1/pa13.md)
- [PA14: 435 groups](details-v1/pa14.md)
- [PA15: 290 groups](details-v1/pa15.md)
- [PA16: 296 groups](details-v1/pa16.md)
- [PA17: 511 groups](details-v1/pa17.md)
- [PA18: 532 groups](details-v1/pa18.md)
- [PA19: 418 groups](details-v1/pa19.md)
- [PA20: 203 groups](details-v1/pa20.md)
- [PA21: 279 groups](details-v1/pa21.md)
- [PA22: 168 groups](details-v1/pa22.md)
- [PA23: 84 groups](details-v1/pa23.md)
- [PA25: 134 groups](details-v1/pa25.md)
- [PA26: 37 groups](details-v1/pa26.md)
- [PA27: 138 groups](details-v1/pa27.md)
- [PA28: 90 groups](details-v1/pa28.md)
- [PA29: 443 groups](details-v1/pa29.md)
- [PA30: 133 groups](details-v1/pa30.md)
- [PA31: 116 groups](details-v1/pa31.md)
- [PA32: 89 groups](details-v1/pa32.md)

[Input and resource metadata](comparison-metadata-v1.json) · [Scope and local evidence](../../../reports/internal-native-comparison.txt) · [Runtime exclusions](internal-exclusions.json)

These are single complete-run observations under load. They are not ABBA/A-A calibrated estimates and are not the normal frozen/self compiler regression gate.
