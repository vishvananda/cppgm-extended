# Benchmark measurement age

Compared with main [`01bac6195`](https://github.com/vishvananda/cppgm-extended/commit/01bac61953a982df28933dfde0fa03eb65838ea4).

An asterisk marks a result at least 10 commits behind main, or outside its history. Commit distance includes documentation and workflow changes; it does not establish a performance change. Missing measurements remain `n/m`. The metric value and its performance color are retained.

| Workload | Host | CPU | Measured commit | Commit date (UTC) | Age | Results |
|---|---|---|---|---|---|---|
| Public programs | EPYC | AMD EPYC 9V74 80-Core Processor | [`a00938892`](https://github.com/vishvananda/cppgm-extended/commit/a0093889216091a98b4ab4561769544b02092c58) | 2026-11-13T20:44:29Z | 22 commits | [Results](epyc/report.md) |
| cppgm++ | EPYC | AMD EPYC 9V45 96-Core Processor | [`a00938892`](https://github.com/vishvananda/cppgm-extended/commit/a0093889216091a98b4ab4561769544b02092c58) | 2026-11-13T20:44:29Z | 22 commits | [Results](epyc/self-build.md) |
| index-cpp | EPYC | AMD EPYC 9V45 96-Core Processor | [`a00938892`](https://github.com/vishvananda/cppgm-extended/commit/a0093889216091a98b4ab4561769544b02092c58) | 2026-11-13T20:44:29Z | 22 commits | [Results](index-cpp/epyc/report.md) |
| simdjson | EPYC | AMD EPYC 7763 64-Core Processor | [`01bac6195`](https://github.com/vishvananda/cppgm-extended/commit/01bac61953a982df28933dfde0fa03eb65838ea4) | 2026-11-14T05:05:24Z | 0 commits | [Results](simdjson/epyc/report.md) |
| godot | EPYC | AMD EPYC 9V74 80-Core Processor | [`b81455bc7`](https://github.com/vishvananda/cppgm-extended/commit/b81455bc7d42635d12519c11c3788a20b5fad2f4) | 2026-11-13T19:02:49Z | 24 commits | [Results](godot/epyc/report.md) |
| boost/o3 | EPYC | AMD EPYC 9V45 96-Core Processor | [`b81455bc7`](https://github.com/vishvananda/cppgm-extended/commit/b81455bc7d42635d12519c11c3788a20b5fad2f4) | 2026-11-13T19:02:49Z | 24 commits | [Results](boost/o3/epyc/report.md) |
| internal | EPYC | AMD EPYC 9V74 80-Core Processor | [`01bac6195`](https://github.com/vishvananda/cppgm-extended/commit/01bac61953a982df28933dfde0fa03eb65838ea4) | 2026-11-14T05:05:24Z | 0 commits | [Results](internal/epyc/report.md) |
| wasm | EPYC | AMD EPYC 7763 64-Core Processor | [`b81455bc7`](https://github.com/vishvananda/cppgm-extended/commit/b81455bc7d42635d12519c11c3788a20b5fad2f4) | 2026-11-13T19:02:49Z | 24 commits | [Results](wasm/epyc/report.md) |
| wasm-selfbuild | EPYC | AMD EPYC 7763 64-Core Processor | [`a00938892`](https://github.com/vishvananda/cppgm-extended/commit/a0093889216091a98b4ab4561769544b02092c58) | 2026-11-13T20:44:29Z | 22 commits | [Results](wasm-selfbuild/epyc/report.md) |
| Public programs | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) | 2026-11-09T18:26:07Z | 309 commits | [Results](intel/report.md) |
| cppgm++ | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) | 2026-11-09T18:26:07Z | 309 commits | [Results](intel/self-build.md) |
| index-cpp | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) | 2026-11-09T18:26:07Z | 309 commits | [Results](index-cpp/intel/report.md) |
| simdjson | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`9ca649bf5`](https://github.com/vishvananda/cppgm-extended/commit/9ca649bf59cb1e14979655d1669ade353a9ae881) | 2026-11-09T22:07:51Z | 302 commits | [Results](simdjson/intel/report.md) |
| godot | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`9ca649bf5`](https://github.com/vishvananda/cppgm-extended/commit/9ca649bf59cb1e14979655d1669ade353a9ae881) | 2026-11-09T22:07:51Z | 302 commits | [Results](godot/intel/report.md) |
| boost/o3 | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`93343ea0e`](https://github.com/vishvananda/cppgm-extended/commit/93343ea0ebdcf068d87b88027a6eca8abf32111f) | 2026-11-09T19:15:37Z | 307 commits | [Results](boost/o3/intel/report.md) |
| internal | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`783dbfcf0`](https://github.com/vishvananda/cppgm-extended/commit/783dbfcf01360d923ff590d2048a0c1764761cc3) | 2026-11-08T11:35:18Z | 465 commits | [Results](internal/intel/report.md) |
| Public programs | CI Intel | Intel(R) Xeon(R) 6973P-C | [`393d1b1d0`](https://github.com/vishvananda/cppgm-extended/commit/393d1b1d09089ffdade676f4e7cbad8759c15d2a) | 2026-11-13T02:37:20Z | 55 commits | [Results](github-intel/report.md) |
| cppgm++ | CI Intel | Intel(R) Xeon(R) 6975P-C | [`b81455bc7`](https://github.com/vishvananda/cppgm-extended/commit/b81455bc7d42635d12519c11c3788a20b5fad2f4) | 2026-11-13T19:02:49Z | 24 commits | [Results](github-intel/self-build.md) |
| index-cpp | CI Intel | INTEL(R) XEON(R) PLATINUM 8573C | [`dc9084330`](https://github.com/vishvananda/cppgm-extended/commit/dc908433031b1e72d07206262cb240cdedc9d13b) | 2026-11-13T08:14:23Z | 43 commits | [Results](index-cpp/github-intel/report.md) |
| simdjson | CI Intel | INTEL(R) XEON(R) PLATINUM 8573C | [`dc9084330`](https://github.com/vishvananda/cppgm-extended/commit/dc908433031b1e72d07206262cb240cdedc9d13b) | 2026-11-13T08:14:23Z | 43 commits | [Results](simdjson/github-intel/report.md) |
| godot | CI Intel | Intel(R) Xeon(R) 6975P-C | [`b858d4082`](https://github.com/vishvananda/cppgm-extended/commit/b858d4082cddcb8e1e930d786f3f1139c6996e6d) | 2026-11-13T17:33:12Z | 28 commits | [Results](godot/github-intel/report.md) |
| boost/o3 | CI Intel | INTEL(R) XEON(R) PLATINUM 8573C | [`dc9084330`](https://github.com/vishvananda/cppgm-extended/commit/dc908433031b1e72d07206262cb240cdedc9d13b) | 2026-11-13T08:14:23Z | 43 commits | [Results](boost/o3/github-intel/report.md) |
| internal | CI Intel | Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz | [`abac2e233`](https://github.com/vishvananda/cppgm-extended/commit/abac2e23330b0743dffd1fd60cec74e466af8bf1) | 2026-11-09T20:24:39Z | 305 commits | [Results](internal/github-intel/report.md) |
| wasm | CI Intel | Intel(R) Xeon(R) 6975P-C | [`01bac6195`](https://github.com/vishvananda/cppgm-extended/commit/01bac61953a982df28933dfde0fa03eb65838ea4) | 2026-11-14T05:05:24Z | 0 commits | [Results](wasm/github-intel/report.md) |
