# Boost compilation and test execution

[Results by host](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/report.md)
· [CI runs and artifacts](https://github.com/vishvananda/cppgm-extended/actions/workflows/boost.yml)

This lane runs on main pushes, pull requests and manual requests. It retains the
previous advisory's exact Boost 1.92.0 selection: **1,818 declarations in 17
suites**, using the original C++11 debug settings, including the GDB target's
`-Og`. The archive is pinned by SHA-256.
Accumulators, Asio, Container, Function, Fusion, JSON, MP11, MPL, Optional,
Proto, QVM, Spirit Repository, TypeErasure, TypeTraits, UUID, Variant2 and
Xpressive run with cppgm, GCC and Clang.

Each compiler must pass the original compile, link, run and expected-failure
oracles. JSON includes the actual GDB pretty-printer integration. Missing
variants, compiler crashes and mismatched native statuses fail the check.
There is no performance cutoff; timings publish only after all three compilers
validate the same selection.

The debugger source gets identical volatile breakpoint anchors for all compilers.
They keep comment-based breakpoints inside the variable's scope on Clang;
all original expressions and expected values remain. Both source hashes are
recorded. MPL's `issue_69` and TypeTraits' `enum_UDT` have fixed underlying types
so their `-1` probes are valid constant expressions on current Clang. These
adaptations apply to all three compilers. Other sources and optimization flags
are unchanged.

Compile speed is **reference CPU / cppgm CPU**, summed across the same C++ test
compilations. Configuration probes, library dependencies, C compilation, linking and B2 overhead are
excluded. Reports also show native test-execution CPU, including process startup
and GDB. These are correctness tests, not application benchmarks, so their
execution times are separate from the README's runtime column.

Each compiler completes one fresh campaign with the same worker budget.
Small timing differences need repeated campaigns. Per-suite tables link to
per-test times, commands, reviewed outcomes, archive/compiler hashes and exact
CPU, toolchain and standard-library identities. Shared version-1 records retain
`compile` and `test-execution` phases separately. EPYC and hosted Intel results
retain separate histories and latest measurements.

PR results appear in the job summary and artifact. Successful main runs publish
reports and compile badges beside the other benchmarks. Artifacts retain
14 days of diagnostics; published results retain history by commit and host.

To reproduce on Linux with GCC, Clang, GDB and the verified archive installed:

```sh
make -C dev cppgm++
python3 scripts/boost_benchmarks.py run \
  --archive /path/to/boost_1_92_0.tar.bz2 --output obj/boost-local --jobs 4
```

Use a fresh output directory. The runner extracts the archive itself;
`--compiler`, `--gcc` and `--clang` select the C++ drivers. C and final linking
use the corresponding host toolchain. This lane is independent of student
assignment requirements.
