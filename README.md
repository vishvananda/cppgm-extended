# cppgm-extended

[![Tests](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml/badge.svg)](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml)

A C++11 compiler course designed for agents, with 35 cumulative assignments from
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
<td>Boost advisory</td>
<td width="152"><a href="https://github.com/vishvananda/cppgm-extended/actions/runs/42476437427"><img alt="Boost advisory: passing" width="120" height="24" src="docs/performance/preview/badges/tests-boost.svg"></a></td>
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
<td width="152"><a href="docs/performance/preview/correctness.md"><img alt="Godot: planned" width="120" height="24" src="docs/performance/preview/badges/tests-godot.svg"></a></td>
</tr>
</tbody>
</table>

[Coverage, exclusions and test scope](docs/performance/preview/correctness.md).

## Performance

**O3 speedups; higher is better.** Colors fade from green through yellow and orange, reaching red at 0.50× or below.

Badges show **EPYC / Intel / hosted Intel**, top to bottom. `n/m` means not measured. Swipe sideways for other columns.

<table>
<thead><tr><th width="120">Workload</th><th width="152">Compile&nbsp;vs&nbsp;GCC&nbsp;↑</th><th width="152">Compile&nbsp;vs&nbsp;Clang&nbsp;↑</th><th width="152">Runtime&nbsp;vs&nbsp;GCC&nbsp;↑</th><th width="152">Runtime&nbsp;vs&nbsp;Clang&nbsp;↑</th></tr></thead>
<tbody>
<tr>
<td>Public programs</td>
<td width="152"><a href="docs/performance/preview/public.md"><img alt="public O3 compile vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 4.02×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-compile-gcc-epyc.svg"></a><br><a href="docs/performance/preview/public-intel.md"><img alt="public O3 compile vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797: 4.48×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-compile-gcc-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="public O3 compile vs gcc: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-public-compile-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/public.md"><img alt="public O3 compile vs clang; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 3.49×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-compile-clang-epyc.svg"></a><br><a href="docs/performance/preview/public-intel.md"><img alt="public O3 compile vs clang; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797: 4.58×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-compile-clang-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="public O3 compile vs clang: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-public-compile-clang-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/public.md"><img alt="public O3 runtime vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 0.99×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-runtime-gcc-epyc.svg"></a><br><a href="docs/performance/preview/public-intel.md"><img alt="public O3 runtime vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797: 0.99×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-runtime-gcc-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="public O3 runtime vs gcc: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-public-runtime-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/public.md"><img alt="public O3 runtime vs clang; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 1.00×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-runtime-clang-epyc.svg"></a><br><a href="docs/performance/preview/public-intel.md"><img alt="public O3 runtime vs clang; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797: 0.98×" width="120" height="24" src="docs/performance/preview/badges/matrix-public-runtime-clang-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="public O3 runtime vs clang: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-public-runtime-clang-hosted-intel.svg"></a></td>
</tr>
<tr>
<td>index-cpp</td>
<td width="152"><a href="docs/performance/preview/index-cpp.md"><img alt="index O3 compile vs gcc; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6: 6.77×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-compile-gcc-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="index O3 compile vs gcc: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-index-compile-gcc-intel.svg"></a><br><a href="docs/performance/preview/index-cpp-hosted-intel.md"><img alt="index O3 compile vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 6.70×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-compile-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/index-cpp.md"><img alt="index O3 compile vs clang; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6: 5.89×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-compile-clang-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="index O3 compile vs clang: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-index-compile-clang-intel.svg"></a><br><a href="docs/performance/preview/index-cpp-hosted-intel.md"><img alt="index O3 compile vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 5.67×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-compile-clang-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/index-cpp.md"><img alt="index O3 runtime vs gcc; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6: 0.99×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-runtime-gcc-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="index O3 runtime vs gcc: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-index-runtime-gcc-intel.svg"></a><br><a href="docs/performance/preview/index-cpp-hosted-intel.md"><img alt="index O3 runtime vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 1.03×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-runtime-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/index-cpp.md"><img alt="index O3 runtime vs clang; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6: 0.99×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-runtime-clang-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="index O3 runtime vs clang: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-index-runtime-clang-intel.svg"></a><br><a href="docs/performance/preview/index-cpp-hosted-intel.md"><img alt="index O3 runtime vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 1.01×" width="120" height="24" src="docs/performance/preview/badges/matrix-index-runtime-clang-hosted-intel.svg"></a></td>
</tr>
<tr>
<td>simdjson scalar</td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 compile vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 4.47×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-compile-gcc-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="scalar O3 compile vs gcc: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-compile-gcc-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 compile vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 4.43×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-compile-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 compile vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 2.33×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-compile-clang-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="scalar O3 compile vs clang: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-compile-clang-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 compile vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 2.10×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-compile-clang-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 runtime vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 0.81×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-runtime-gcc-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="scalar O3 runtime vs gcc: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-runtime-gcc-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 runtime vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 0.85×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-runtime-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 runtime vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 0.80×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-runtime-clang-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="scalar O3 runtime vs clang: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-runtime-clang-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="scalar O3 runtime vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 0.85×" width="120" height="24" src="docs/performance/preview/badges/matrix-scalar-runtime-clang-hosted-intel.svg"></a></td>
</tr>
<tr>
<td>simdjson SSE</td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 compile vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 4.49×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-compile-gcc-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="sse O3 compile vs gcc: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-compile-gcc-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 compile vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 4.34×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-compile-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 compile vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 2.56×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-compile-clang-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="sse O3 compile vs clang: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-compile-clang-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 compile vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 2.22×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-compile-clang-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 runtime vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 0.90×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-runtime-gcc-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="sse O3 runtime vs gcc: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-runtime-gcc-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 runtime vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 0.99×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-runtime-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 runtime vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15: 0.89×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-runtime-clang-epyc.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="sse O3 runtime vs clang: intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-runtime-clang-intel.svg"></a><br><a href="docs/performance/preview/simdjson.md"><img alt="sse O3 runtime vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e: 0.96×" width="120" height="24" src="docs/performance/preview/badges/matrix-sse-runtime-clang-hosted-intel.svg"></a></td>
</tr>
<tr>
<td>Internal C++ subset†</td>
<td width="152"><a href="docs/performance/preview/performance.md"><img alt="native O3 compile vs gcc: epyc not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-native-compile-gcc-epyc.svg"></a><br><a href="docs/performance/preview/internal.md"><img alt="native O3 compile vs gcc; Intel Xeon E5-2696 v4; Local shared host; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 4.46×" width="120" height="24" src="docs/performance/preview/badges/matrix-native-compile-gcc-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="native O3 compile vs gcc: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-native-compile-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/performance.md"><img alt="native O3 compile vs clang: epyc not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-native-compile-clang-epyc.svg"></a><br><a href="docs/performance/preview/internal.md"><img alt="native O3 compile vs clang; Intel Xeon E5-2696 v4; Local shared host; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 6.85×" width="120" height="24" src="docs/performance/preview/badges/matrix-native-compile-clang-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="native O3 compile vs clang: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-native-compile-clang-hosted-intel.svg"></a></td>
<td width="152">n/a</td>
<td width="152">n/a</td>
</tr>
<tr>
<td>cppgm++</td>
<td width="152"><a href="docs/performance/preview/public.md#building-the-compiler"><img alt="cppgm O3 compile vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3: 4.24×" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-compile-gcc-epyc.svg"></a><br><a href="docs/performance/preview/cppgm-local.md"><img alt="cppgm O3 compile vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; local shared host; revision 17a21ad27ace6eda20b948e57c2e78ab3d568374: 4.03×" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-compile-gcc-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="cppgm O3 compile vs gcc: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-compile-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/performance.md"><img alt="cppgm O3 compile vs clang: epyc not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-compile-clang-epyc.svg"></a><br><a href="docs/performance/preview/cppgm-local.md"><img alt="cppgm O3 compile vs clang; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; local shared host; revision 17a21ad27ace6eda20b948e57c2e78ab3d568374: 3.72×" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-compile-clang-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="cppgm O3 compile vs clang: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-compile-clang-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/public.md#paired-self-built-compiler-measurements"><img alt="cppgm O3 runtime vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3; cppgm codegen: self-built versus GCC-built compiler: 0.97×" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-runtime-gcc-epyc.svg"></a><br><a href="docs/performance/preview/cppgm-local.md"><img alt="cppgm O3 runtime vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; local shared host; revision 17a21ad27ace6eda20b948e57c2e78ab3d568374: 0.98×" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-runtime-gcc-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="cppgm O3 runtime vs gcc: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-runtime-gcc-hosted-intel.svg"></a></td>
<td width="152"><a href="docs/performance/preview/performance.md"><img alt="cppgm O3 runtime vs clang: epyc not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-runtime-clang-epyc.svg"></a><br><a href="docs/performance/preview/cppgm-local.md"><img alt="cppgm O3 runtime vs clang; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; local shared host; revision 17a21ad27ace6eda20b948e57c2e78ab3d568374: 1.00×" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-runtime-clang-intel.svg"></a><br><a href="docs/performance/preview/performance.md"><img alt="cppgm O3 runtime vs clang: hosted-intel not measured: n/m" width="120" height="24" src="docs/performance/preview/badges/matrix-cppgm-runtime-clang-hosted-intel.svg"></a></td>
</tr>
</tbody>
</table>

cppgm++ runtime measures **codegen performance**: self-built versus GCC-built or Clang-built compiler.

[O0, CPU models, revisions and per-test results](docs/performance/preview/performance.md). CPU time, except index-cpp runtime uses elapsed time. † Single-run compilation on a shared host.

## Caveats

- C++11 is the baseline; other dialects have only partial extension support.
- Targets are Linux x86-64 and optional Intel macOS; other architectures are unsupported.
- General inline assembly is unsupported; a few GNU forms are recognized.
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
35-assignment course. Today's handouts, harnesses and reference implementation
describe this version; current PA numbers do not identify original CPPGM lessons.

Copyright 2026 The cppgm-extended Authors. Project-maintained code and materials
use the Apache License 2.0; inherited material retains its original notices.
See [LICENSE](LICENSE), [NOTICE](NOTICE) and [AUTHORS](AUTHORS).

The project also explores sustained agent-assisted compiler development. Its
background is in the blog series beginning with
[I Spent 2 Billion Tokens Writing a C++ Compiler So You Don't Have To](https://medium.com/@vishvananda/i-spent-2-billion-tokens-writing-a-c-compiler-so-you-dont-have-to-d3e4eec4781e).
