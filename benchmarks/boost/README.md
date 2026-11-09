# Boost compilation and native test runtime

[Results by host](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md)
· [CI summaries and artifacts](https://github.com/vishvananda/cppgm-extended/actions/workflows/boost.yml)

This lane runs on main pushes, pull requests and manual requests. All three
compilers validate **1,818 declarations in 17 suites** from the pinned Boost
1.92.0 archive: Accumulators, Asio, Container, Function, Fusion, JSON, MP11,
MPL, Optional, Proto, QVM, Spirit Repository, TypeErasure, TypeTraits, UUID,
Variant2 and Xpressive. The selection matches the previous advisory.

Performance builds use **O3 with assertions enabled**, full inlining and no
debug information. Boost.Build's `debug` variant keeps assertions; explicit
optimization properties select O3. The JSON GDB pretty-printer test retains its
required Og/debug settings with inlining disabled and participates only in correctness validation.
Every original compile, link, run and expected-failure oracle must pass for all
three compilers before results publish. There is no performance cutoff.

Compile speed is **reference CPU / cppgm CPU**, summed across the same C++ test
compilations, including expected rejections. It excludes configuration probes,
library dependencies, C compilation, linking, B2 and the GDB test. Compilation
uses one fresh campaign per compiler; small differences need repeats.

Runtime uses **1,375 successful native test programs**. GDB and the two
expected-failure executions remain correctness-only. Commands, arguments,
working directories, executable hashes and the case set are frozen after the
build. The replay method follows the earlier local Boost comparison:

- Two warmups per compiler, four A/A observations per reference, then two
  mirrored ABBA/BAAB blocks against each of GCC and Clang.
- Programs short in both CPU and wall time repeat up to ten times per observation, with the same count
  for all compilers. Each program's CPU is divided by its repetition count
  before summing, preserving the weight of one complete test pass.
- `wait4` measures each native child's CPU independently, including process
  startup. Executable hashing, output verification, Python and worker overhead
  stay outside that CPU measurement. Batch wall time is diagnostic only.
- Every execution must pass its original native test oracle. Output hashes
  record which programs remain byte-identical across the complete campaign;
  tests can also print addresses, timing or compiler-specific names.
  Every observation and output hash is retained, including failures.

The runtime speedup is the median reference/cppgm ratio of paired block CPU
means. Reports flag A/A drift or paired-block spread above 5%; these unit tests
measure native test execution, including startup, rather than application
steady-state throughput. Histories keep CPU models and measured revisions
separate. O3 results have their own URLs; [historical debug results](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/report.md) retain their original labels.

Source adaptations apply identically to all compilers, with original and
adapted hashes recorded. Volatile breakpoint anchors and no-inline debugger settings keep JSON's comment-based
GDB stops in scope. Fixed enum underlying types make MPL's `issue_69` and
TypeTraits' `enum_UDT` negative-value probes valid constant expressions. All
original test oracles remain.

Per-suite tables link to per-test times, commands, reviewed outcomes and exact
archive/compiler, CPU, toolchain and standard-library identities. Shared records
separate `compile`, single-pass `test-execution` diagnostics and paired `runtime`.
PR results appear in the job summary and artifact; successful main pushes publish
reports and badges. Artifacts retain 14 days of diagnostics and publication
retains history by commit and host.

To reproduce on Linux with GCC, Clang, GDB and the verified archive installed:

```sh
make -C dev cppgm++
python3 scripts/boost_benchmarks.py run \
  --archive /path/to/boost_1_92_0.tar.bz2 --output obj/boost-local --jobs 4
```

Use a fresh output directory. The runner extracts the archive itself;
`--compiler`, `--gcc` and `--clang` select the C++ drivers. C and final linking
use the corresponding host toolchain. `--runtime-blocks` increases paired
repetitions. This lane is independent of student assignment requirements.
