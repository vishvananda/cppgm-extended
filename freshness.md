# Benchmark measurement age

Compared with main [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66).

An asterisk marks a result at least 10 commits behind main, or outside its history. Commit distance includes documentation and workflow changes; it does not establish a performance change. Missing measurements remain `n/m`. The metric value and its performance color are retained.

| Workload | Host | CPU | Measured commit | Commit date (UTC) | Age | Results |
|---|---|---|---|---|---|---|
| Public programs | EPYC | AMD EPYC 7763 64-Core Processor | [`a0ff7d04a`](https://github.com/vishvananda/cppgm-extended/commit/a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74) | 2026-11-11T06:15:55Z | 104 commits | [Results](epyc/report.md) |
| cppgm++ | EPYC | AMD EPYC 9V45 96-Core Processor | [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66) | 2026-11-13T01:12:25Z | 0 commits | [Results](epyc/self-build.md) |
| index-cpp | EPYC | AMD EPYC 9V45 96-Core Processor | [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66) | 2026-11-13T01:12:25Z | 0 commits | [Results](index-cpp/epyc/report.md) |
| simdjson | EPYC | AMD EPYC 9V74 80-Core Processor | [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66) | 2026-11-13T01:12:25Z | 0 commits | [Results](simdjson/epyc/report.md) |
| godot | EPYC | AMD EPYC 9V45 96-Core Processor | [`1ddd7507e`](https://github.com/vishvananda/cppgm-extended/commit/1ddd7507ee861649d43d1e72dcde7bb17a85a691) | 2026-11-11T08:34:52Z | 99 commits | [Results](godot/epyc/report.md) |
| boost/o3 | EPYC | AMD EPYC 7763 64-Core Processor | [`e2831a513`](https://github.com/vishvananda/cppgm-extended/commit/e2831a513b668c602faa8098e55eb22ee08a670f) | 2026-11-12T01:20:29Z | 26 commits | [Results](boost/o3/epyc/report.md) |
| internal | EPYC | AMD EPYC 7763 64-Core Processor | [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66) | 2026-11-13T01:12:25Z | 0 commits | [Results](internal/epyc/report.md) |
| Public programs | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) | 2026-11-09T18:26:07Z | 202 commits | [Results](intel/report.md) |
| cppgm++ | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) | 2026-11-09T18:26:07Z | 202 commits | [Results](intel/self-build.md) |
| index-cpp | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) | 2026-11-09T18:26:07Z | 202 commits | [Results](index-cpp/intel/report.md) |
| simdjson | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`9ca649bf5`](https://github.com/vishvananda/cppgm-extended/commit/9ca649bf59cb1e14979655d1669ade353a9ae881) | 2026-11-09T22:07:51Z | 195 commits | [Results](simdjson/intel/report.md) |
| godot | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`9ca649bf5`](https://github.com/vishvananda/cppgm-extended/commit/9ca649bf59cb1e14979655d1669ade353a9ae881) | 2026-11-09T22:07:51Z | 195 commits | [Results](godot/intel/report.md) |
| boost/o3 | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`93343ea0e`](https://github.com/vishvananda/cppgm-extended/commit/93343ea0ebdcf068d87b88027a6eca8abf32111f) | 2026-11-09T19:15:37Z | 200 commits | [Results](boost/o3/intel/report.md) |
| internal | Intel | Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz | [`783dbfcf0`](https://github.com/vishvananda/cppgm-extended/commit/783dbfcf01360d923ff590d2048a0c1764761cc3) | 2026-11-08T11:35:18Z | 358 commits | [Results](internal/intel/report.md) |
| Public programs | CI Intel | INTEL(R) XEON(R) PLATINUM 8573C | [`e2831a513`](https://github.com/vishvananda/cppgm-extended/commit/e2831a513b668c602faa8098e55eb22ee08a670f) | 2026-11-12T01:20:29Z | 26 commits | [Results](github-intel/report.md) |
| cppgm++ | CI Intel | Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz | [`8287f480c`](https://github.com/vishvananda/cppgm-extended/commit/8287f480c7b5bdb2074c8941689aa36f9c233bd7) | 2026-11-10T01:26:25Z | 191 commits | [Results](github-intel/self-build.md) |
| index-cpp | CI Intel | INTEL(R) XEON(R) PLATINUM 8573C | [`da7046f6f`](https://github.com/vishvananda/cppgm-extended/commit/da7046f6f5518cdabc9d2d0f2cd153e228c7b1bb) | 2026-11-09T20:44:51Z | 197 commits | [Results](index-cpp/github-intel/report.md) |
| simdjson | CI Intel | Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz | [`535ed4f12`](https://github.com/vishvananda/cppgm-extended/commit/535ed4f12c6c6470e67367b113376f7751e4e65d) | 2026-11-10T22:24:49Z | 131 commits | [Results](simdjson/github-intel/report.md) |
| godot | CI Intel | Intel(R) Xeon(R) 6973P-C | [`e2831a513`](https://github.com/vishvananda/cppgm-extended/commit/e2831a513b668c602faa8098e55eb22ee08a670f) | 2026-11-12T01:20:29Z | 26 commits | [Results](godot/github-intel/report.md) |
| boost/o3 | CI Intel | INTEL(R) XEON(R) PLATINUM 8573C | [`0ad08c5a4`](https://github.com/vishvananda/cppgm-extended/commit/0ad08c5a4a0d0f49f5fb256dacff17aaec4167ad) | 2026-11-10T05:57:08Z | 165 commits | [Results](boost/o3/github-intel/report.md) |
| internal | CI Intel | Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz | [`abac2e233`](https://github.com/vishvananda/cppgm-extended/commit/abac2e23330b0743dffd1fd60cec74e466af8bf1) | 2026-11-09T20:24:39Z | 198 commits | [Results](internal/github-intel/report.md) |
