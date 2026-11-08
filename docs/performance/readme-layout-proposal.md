# README layout proposal

Keep correctness coverage separate from performance. A green correctness row does
not imply that a compile/runtime benchmark exists, and a fast benchmark does not
establish language conformance. Every result links to its measured revision and
complete record. This proposal accompanies the working
[report format and renderer](results-format.md).

## Correctness and external-build overview

| Workload | Configuration / scope | What a pass establishes | Compile comparison | Runtime comparison |
|---|---|---|---|---|
| Internal assignment tests | Required PA1–PA33 contracts | All declared stage, IR, ABI, driver and execution oracles | Qualified C++ subset separately | Declared executable tests separately |
| Internal C++ overlap | Same positive source cohort for cppgm, GCC and Clang | Object compilation; runtime controls checked separately | O0 / O3, both baselines | Separate execution observations; not application microbenchmarks |
| GCC C++11 | Pinned, source-reviewed profile | Acceptance, ordinary required diagnostics and declared execution | GCC baseline; label suite versus compile CPU | Declared execution checks; benchmark not yet measured |
| Clang C++11 | Pinned, source-reviewed profile | Compile-only acceptance and ordinary required diagnostics | Clang baseline; label suite versus compile CPU | Not applicable to this compile-only profile |
| Boost | Pinned upstream libraries and their test actions | Actual declared build/run/rejection oracles | Not yet measured against native compilers | Not yet measured |
| Public programs | Declared program and input manifest | Built programs have matching output | O0 / O3, both baselines | O0 / O3, both baselines; included counts shown |
| Compiler self-build | Course source and build configuration | Complete compiler rebuild and validation | Build time against GCC and Clang | Codegen: self-built versus GCC-built / Clang-built compiler |
| index-cpp | Pinned source, full inputs, 50 checksums | All original workload checksums | Measured optimization levels only | Both baselines, all 50 workloads in detail |
| simdjson | Scalar and SSE configurations separately | Canonical DOM output for both JSON inputs | One library build per backend | twitter and citm inputs separately |
| Godot | Planned; presets not yet declared | No pass claimed | Not measured | Not measured |

The live version should show exact selected/executed/passed/excluded counts and
CI links, generated from result records. The table above describes the scope and
layout, rather than publishing stale manual pass counts.

## Performance summary

Use one compact README table with a row per workload/configuration and separate
compile/runtime columns for GCC and Clang. Stack processor-labeled badges within
each metric cell: EPYC, local/dedicated Intel, then hosted Intel. Gray badges mark
missing measurements. Colors vary continuously from green through yellow and
orange to red as speed decreases.

Linked detail tables retain exact CPU models and environments. For example, **AMD
EPYC 7763 / GitHub hosted** and **AMD EPYC 9V74 / GitHub hosted** are different
sections, as are **dedicated Xeon E5-2696 v4** and **hosted Xeon Platinum 8370C**.
The CPU list comes from records, so adding a new processor needs no README logic.

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Revision / details |
|---|---|---|---|---|---|---|
| One explicit aggregate or featured workload | Declared level | Speedup or status | Speedup or status | Speedup or status | Speedup or status | Measured commit and linked table |

CPU time is the primary summary metric where it is measured. A runtime benchmark
whose oracle measures loop elapsed time must retain that label; do not relabel it
CPU time. Show complete-build or suite elapsed time explicitly when compilation
has not been isolated. Show `not measured`, `not applicable`, `partial`, `failed`
or `planned` rather than an unexplained dash. No overall score combines hosts,
optimization levels or compiler baselines.

simdjson has one row per backend. Its two runtime inputs can have a declared
aggregate with the count shown, or remain linked individual results; compilation
is never counted twice just because there are two runtime inputs. Godot will use
one row per agreed build/benchmark preset, with its scene/input tests in detail.

## Linked detail and provenance

Each workload page expands configuration → host → optimization → test/input.
Tables show absolute cppgm/GCC/Clang times, both speedups, correctness results,
units, included/excluded counts and estimator. The paired speedup remains the
original paired estimator rather than a quotient of independently summarized
absolute times. Raw observations and calibration warnings stay linked.

Keep source version/input hashes, compiler source revision/binary hashes,
compiler/library versions, full flags, ISA, workers, affinity and resource limits
alongside the tables. The cppgm++ runtime result measures codegen performance:
the self-built compiler runs against GCC-built and Clang-built compilers on
identical inputs.

## Adding data

A workload entry declares named configurations and supported optimization levels.
Its producer emits versioned records and raw evidence. The generic renderer
validates coverage and identity, creates summary rows and linked test tables,
and leaves unavailable combinations visible. The existing publishers should
export this contract rather than acquire more workload-specific README code.

The prototype renderer is exercised on actual published simdjson observations:
two backends, two exact host profiles, both native baselines, compilation and two
runtime inputs, with CPU and elapsed metrics kept separate. Existing publisher
migration remains a follow-up; the prototype does not claim it has occurred.

The completed local [internal-source comparison](../../reports/internal-native-comparison.txt)
provides a second concrete dataset: 5,614 common positive fixture groups at O0
and O3, plus 1,210 matched normal-exit controls. Its linked evidence retains
the broad qualification results, exclusions and per-fixture observations.
Those single-run CPU observations must keep their quality label when imported.
