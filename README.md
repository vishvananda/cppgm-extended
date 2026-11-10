# cppgm-extended

[![Tests](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml/badge.svg)](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml)

A C++ compiler course designed for agents, with 36 cumulative assignments from
preprocessing to native code, optimization and self-hosting. This repository
contains a high-performance reference implementation in `dev/`; `pa1/` through
`pa35/` hold the handouts, tests and wrappers.

For how the reference compiler was built and its performance results, read
[How I Built a Compiler That Is Faster Than GCC and Clang](https://medium.com/@vishvananda/how-i-built-a-compiler-that-is-faster-than-gcc-and-clang-4c8c1d4818fa).

To take the course, start with
[cppgm-assignments](https://github.com/vishvananda/cppgm-assignments), the export
containing scaffolds, handouts, tests and downloadable reference tools.
The export's references are generated from this compiler.

## Correctness

<table>
<thead><tr><th>Suite</th><th width="152">Result&nbsp;/&nbsp;details</th></tr></thead>
<tbody>
<tr>
<td>Internal assignment tests</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/actions/runs/42476441247"><img alt="Internal assignment tests: 9,832 / 9,832" width="120" height="24" src="docs/performance/preview/badges/tests-internal.svg"></a></td>
</tr>
<tr>
<td>GCC C++11 profile</td>
<td width="152"><a href="docs/performance/preview/upstream-cxx11.md"><img alt="GCC C++11 profile: 3,968 / 3,968" width="120" height="24" src="docs/performance/preview/badges/tests-gcc.svg"></a></td>
</tr>
<tr>
<td>Clang C++11 profile</td>
<td width="152"><a href="docs/performance/preview/upstream-cxx11.md"><img alt="Clang C++11 profile: 815 / 815" width="120" height="24" src="docs/performance/preview/badges/tests-clang.svg"></a></td>
</tr>
<tr>
<td>Internal C++ subset · compile / run</td>
<td width="152"><a href="docs/performance/preview/internal.md"><img alt="Internal C++ subset · all three compilers: 5,614 / 1,210" width="120" height="24" src="docs/performance/preview/badges/tests-native.svg"></a></td>
</tr>
<tr>
<td>Public programs</td>
<td width="152"><a href="docs/performance/preview/public.md"><img alt="Public programs: 81 / 81" width="120" height="24" src="docs/performance/preview/badges/tests-public.svg"></a></td>
</tr>
<tr>
<td>index-cpp</td>
<td width="152"><a href="docs/performance/preview/index-cpp.md"><img alt="index-cpp: 50 / 50" width="120" height="24" src="docs/performance/preview/badges/tests-index.svg"></a></td>
</tr>
<tr>
<td>simdjson · scalar + SSE</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/actions/runs/42476441186"><img alt="simdjson · scalar + SSE: passing" width="120" height="24" src="docs/performance/preview/badges/tests-simd.svg"></a></td>
</tr>
<tr>
<td>Boost · 17 suites</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/actions/workflows/boost.yml"><img alt="Boost tests; all three compilers: 1,818 / 1,818" width="120" height="24" src="docs/performance/preview/badges/tests-boost.svg"></a></td>
</tr>
<tr>
<td>Self-build + inception</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/actions/runs/42476437424"><img alt="Self-build + inception: passing" width="120" height="24" src="docs/performance/preview/badges/tests-self.svg"></a></td>
</tr>
<tr>
<td>Student export</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/actions/runs/42476441242"><img alt="Student export: passing" width="120" height="24" src="docs/performance/preview/badges/tests-export.svg"></a></td>
</tr>
<tr>
<td>Godot</td>
<td width="152"><a href="docs/performance/preview/godot.md"><img alt="Godot: 5 / 5 checked workloads, all three compilers" width="120" height="24" src="docs/performance/preview/badges/tests-godot.svg"></a></td>
</tr>
</tbody>
</table>

[Coverage, exclusions and test scope](docs/performance/preview/correctness.md).

## Performance

**O3 speedups; higher is better.** Colors fade from green through yellow and orange, reaching red at 0.50× or below.

Badges show **EPYC / Intel / CI Intel**, top to bottom; CI Intel means GitHub hosted. `n/m` means unmeasured; `*` marks results 10+ commits behind main or outside its history. [Measurement age](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/freshness.md) lists exact revisions. Swipe sideways for other columns.

<table>
<thead><tr><th width="120">Workload</th><th width="152">Compile&nbsp;vs&nbsp;GCC&nbsp;↑</th><th width="152">Compile&nbsp;vs&nbsp;Clang&nbsp;↑</th><th width="152">Runtime&nbsp;vs&nbsp;GCC&nbsp;↑</th><th width="152">Runtime&nbsp;vs&nbsp;Clang&nbsp;↑</th></tr></thead>
<tbody>
<tr>
<td>Public programs</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public compile vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-compile.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public compile vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-compile.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public compile vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/o3-compile.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public compile vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public compile vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public compile vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/o3-compile-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public runtime vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-generated.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public runtime vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-generated.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public runtime vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/o3-generated.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public runtime vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-generated-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public runtime vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-generated-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md"><img alt="public runtime vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/o3-generated-clang.svg"></a></td>
</tr>
<tr>
<td>index-cpp</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index compile vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/epyc-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index compile vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/intel-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index compile vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/github-intel-compile-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index compile vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/epyc-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index compile vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/intel-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index compile vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/github-intel-compile-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index runtime vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/epyc-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index runtime vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/intel-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index runtime vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/github-intel-runtime-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index runtime vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/epyc-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index runtime vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/intel-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md"><img alt="index runtime vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/index-cpp/github-intel-runtime-clang.svg"></a></td>
</tr>
<tr>
<td>simdjson scalar</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar compile vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-scalar-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar compile vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-scalar-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar compile vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-scalar-compile-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar compile vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-scalar-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar compile vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-scalar-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar compile vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-scalar-compile-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar runtime vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-scalar-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar runtime vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-scalar-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar runtime vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-scalar-runtime-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar runtime vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-scalar-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar runtime vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-scalar-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="scalar runtime vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-scalar-runtime-clang.svg"></a></td>
</tr>
<tr>
<td>simdjson SSE</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse compile vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-sse-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse compile vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-sse-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse compile vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-sse-compile-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse compile vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-sse-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse compile vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-sse-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse compile vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-sse-compile-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse runtime vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-sse-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse runtime vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-sse-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse runtime vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-sse-runtime-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse runtime vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/epyc-sse-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse runtime vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/intel-sse-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md"><img alt="sse runtime vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/simdjson/github-intel-sse-runtime-clang.svg"></a></td>
</tr>
<tr>
<td>Internal C++ subset†</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md"><img alt="internal compile vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/internal/epyc-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md"><img alt="internal compile vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/internal/intel-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md"><img alt="internal compile vs gcc: CI Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/internal/github-intel-compile-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md"><img alt="internal compile vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/internal/epyc-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md"><img alt="internal compile vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/internal/intel-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md"><img alt="internal compile vs clang: CI Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/internal/github-intel-compile-clang.svg"></a></td>
<td width="152">n/a</td>
<td width="152">n/a</td>
</tr>
<tr>
<td><a href="benchmarks/boost/README.md">Boost tests · O3†</a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost compile CPU vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/epyc-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost compile CPU vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/intel-compile-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost compile CPU vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/github-intel-compile-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost compile CPU vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/epyc-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost compile CPU vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/intel-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost compile CPU vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/github-intel-compile-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost native test CPU vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/epyc-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost native test CPU vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/intel-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost native test CPU vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/github-intel-runtime-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost native test CPU vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/epyc-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost native test CPU vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/intel-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/boost/o3/report.md"><img alt="Boost native test CPU vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/boost/o3/github-intel-runtime-clang.svg"></a></td>
</tr>
<tr>
<td>Godot headless†</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot build vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/epyc-build-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot build vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/intel-build-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot build vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/github-intel-build-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot build vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/epyc-build-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot build vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/intel-build-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot build vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/github-intel-build-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot runtime vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/epyc-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot runtime vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/intel-runtime-gcc.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot runtime vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/github-intel-runtime-gcc.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot runtime vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/epyc-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot runtime vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/intel-runtime-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md"><img alt="godot runtime vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/godot/github-intel-runtime-clang.svg"></a></td>
</tr>
<tr>
<td>cppgm++</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm compile vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/self-compile.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm compile vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/self-compile.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm compile vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/self-compile.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm compile vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/self-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm compile vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/self-compile-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm compile vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/self-compile-clang.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm runtime vs gcc: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/self-built.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm runtime vs gcc: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/self-built.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm runtime vs gcc: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/self-built.svg"></a></td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm runtime vs clang: EPYC" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/self-built-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm runtime vs clang: Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/self-built-clang.svg"></a><br><a href="https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/self-build-hosts.md"><img alt="cppgm runtime vs clang: hosted Intel" width="120" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/github-intel/self-built-clang.svg"></a></td>
</tr>
</tbody>
</table>

cppgm++ runtime measures **codegen performance**: self-built versus GCC-built or Clang-built compiler.

[Internal C++](benchmarks/internal/README.md) measures the fixed qualified course-source compilation cohort. [O0 and per-host public results](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/public-hosts.md). CPU time, except index-cpp runtime uses elapsed time. Godot reports a full build, including SCons and linking. [Boost](benchmarks/boost/README.md) measures C++ test compilation and paired native test replays, with assertions enabled. GDB is correctness-only. † Single-run build/compile observations; small differences need repeats.

[Refreshing the dedicated Intel measurements](docs/performance/intel-benchmarks.md).

## Caveats

- The [required C++11/14/17 profile](LANGUAGE_PROFILE.md) covers selected features, not full conformance to every edition.
- Targets are Linux x86-64 and optional Intel macOS; other architectures are unsupported.
- [Integrated assembly](pa36/README.md) is required before hosted compatibility; its documented ISA profile remains bounded.
- No C++ standard library is bundled; use a host-installed library.
- The compiler may contain bugs.
- Code generation may be poor for unfamiliar program shapes.

## Build and validate

The course target is Linux x86-64. Install a C++11 host compiler, GNU make,
Bash, Perl and Python 3; later checks also use binutils and a debugger.

```sh
make CXX=g++ CPPGM_HOST_CXX=g++
make test-pa8
make test-report
make inception
```

Intel macOS support and its optional checks are described in
[the macOS guide](docs/intel-macos.md).

See [ROADMAP.md](ROADMAP.md) for the assignment sequence,
[AGENTS.md](AGENTS.md) for contributor instructions, and
[TESTING_AND_REFERENCES.md](TESTING_AND_REFERENCES.md) for validation and
reference policy. Personal experiments go in `student.tests/`.

## Origins and license

The original C++ Grandmaster Certification course supplied early assignment
material, tests and starter code. The cppgm-extended authors have substantially
revised, combined, renumbered and extended that material into this independent
36-assignment course. Today's handouts, harnesses and reference implementation
describe this version; current PA numbers do not identify original CPPGM lessons.

Copyright 2026 The cppgm-extended Authors. Project-maintained code and materials
use the Apache License 2.0; inherited material retains its original notices.
See [LICENSE](LICENSE), [NOTICE](NOTICE) and [AUTHORS](AUTHORS).

The project also explores sustained agent-assisted compiler development. Its
background is in the blog series beginning with
[I Spent 2 Billion Tokens Writing a C++ Compiler So You Don't Have To](https://medium.com/@vishvananda/i-spent-2-billion-tokens-writing-a-c-compiler-so-you-dont-have-to-d3e4eec4781e).
