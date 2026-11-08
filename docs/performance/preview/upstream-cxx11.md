# Upstream C++11 profiles

Compiler source: `783dbfcf01360d923ff590d2048a0c1764761cc3`; [hosted CI and retained manifests](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441294). The jobs tested GitHub merge commit `c9774d6a969d0e28c251e35f4ad9a005c6cafcc6`. The exact CPU model was not captured; these timings remain outside named hardware tables.

### GCC C++11 language checks

| Compiler | Executed / selected | Passed | Suite elapsed (s) |
|---|---:|---:|---:|
| cppgm | 3968 / 3968 | 3968 | 14.570 |
| upstream | 3968 / 3968 | 3968 | 30.297 |

cppgm / upstream elapsed ratio: 0.481 (lower is faster). Passing identical suite.

Self-contained GCC g++.dg/cpp0x language translations and zero-exit runtime checks; native diagnostic text, counts and recovery are not correctness obligations.

Selected: 2678 acceptance, 1290 required-diagnostic configurations; 162 acceptance configurations include linking/execution.

778 source/RUN exclusions are recorded in manifest.json; they are not passes.
Workers: 2; order: cppgm, upstream; fresh output files, OS caches unchanged.
Setup and builds excluded; compilation, required linking/execution and result checks included.


### CLANG C++11 language checks

| Compiler | Executed / selected | Passed | Suite elapsed (s) |
|---|---:|---:|---:|
| cppgm | 815 / 815 | 815 | 2.271 |
| upstream | 815 / 815 | 815 | 10.701 |

cppgm / upstream elapsed ratio: 0.212 (lower is faster). Passing identical suite.

Positive object acceptance and active-error required-diagnostic candidates from self-contained explicit C++11/0x and implicit-mode syntax RUNs requiring separate native qualification; both ISO and GNU spellings retain their declared dialect; no full Clang-suite claim; LLVM pipelines add separately qualified object projections with unmet original oracles; audited local configs, frozen literal input closures and reviewed optional warning controls add separately qualified language projections; standalone LLVM-only actions add strictly qualified positive object acceptance

Selected: 310 acceptance, 505 required-diagnostic configurations; 0 acceptance configurations include linking/execution.

15465 source/RUN exclusions are recorded in manifest.json; they are not passes.
Workers: 2; order: cppgm, upstream; fresh output files, OS caches unchanged.
Setup and builds excluded; compilation, required linking/execution and result checks included.
