# simdjson scalar and SSE comparison

[Results by host](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/report.md)
· [Last EPYC results](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/epyc/report.md)
· [CI runs and artifacts](https://github.com/vishvananda/cppgm-extended/actions/workflows/simdjson.yml)
· [Initial analysis](../../docs/performance/simdjson-20261107.md)

This reference-compiler benchmark runs on each push and pull request. Successful
main results publish beside the public and index-cpp reports on the
`benchmark-badges` branch, with history by measured commit. PR results appear
in the job summary and its artifact. It is separate from the course's required
performance gates and from the public-suite aggregate.

GitHub can assign Intel or AMD runners. Hosted Intel results use `github-intel`,
separate from EPYC and the dedicated Intel runner's `intel` path. Each report
names its CPU. History retains each host separately at each measured commit;
compare results on the same CPU model when assessing a compiler change.

The source is unmodified simdjson v5.1.1, pinned to
`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`. Each compiler builds
`src/simdjson.cpp` at `-std=c++17 -O3 -g0 -DNDEBUG`, once with only the
scalar fallback and once with only Westmere SSE4.2/PCLMUL/POPCNT. The SSE
profile adds `-mpopcnt` for all three compilers; scalar keeps the baseline ISA.
Haswell and Icelake are disabled. The host needs x86-64 with those capabilities.

Compile measurements cover preprocessing through object emission, excluding
fetching, compiler construction and linking. Runtime uses one fixed GCC-built
[driver](parse.cpp) per backend linked to each compiler's library object.
It measures repeated DOM parsing of upstream `twitter.json` and
`citm_catalog.json`: input stays hot, the parser is reused, and the initial
allocation, file reading and JSON rendering are outside the timer. This
isolates library code generation; it is not an end-to-end application build
or a cold-input benchmark. Canonical JSON output must match across all six
library builds before results publish.

Ratios are **reference time / cppgm time**, so higher is better. Both GCC and
Clang comparisons report CPU and wall time. A pilot selects enough identical
repeats to exceed 350 ms of GCC CPU time per workload. Three paired ABBA/BAAB
blocks follow warmup, with A/A calibration before and after each comparison.
The report uses the median block ratio, flags calibration differences above
2%, and retains every observation. Small differences need confirmation.

To reproduce on Linux:

```sh
make -C dev cppgm++ -j "$(nproc)" CXX=g++ CPPGM_HOST_CXX=g++
python3 scripts/simdjson_ci.py run --output obj/simdjson-local --cpus 0
```

Choose an available quiet CPU. The output directory must be new. `--gcc` and
`--clang` select installed host compilers; `--source` can reuse a clean checkout
at the pinned commit. Artifacts include source/input/compiler/header hashes,
commands, compile logs, raw observations and per-block ratios. Runtime errors
or mismatched output fail the job; no speed threshold suppresses a regression.
