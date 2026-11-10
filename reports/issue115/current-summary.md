Intel Xeon E5-2696 v4, generic x86-64, -O3; original frozen issue LowIR. Paired means; all 14 full runs pass 50/50. Myc index and Base64 source patches are excluded. Higher ratios mean faster GM.

| Build | Total runtime (s) | Speed versus reference ↑ | Checks |
|---|---:|---:|---:|
| GM, original LLVM baseline | 114.161 | 1.000× baseline | 50/50 |
| GM, final fixes (same paired block) | 91.378 | 1.249× baseline | 50/50 |
| Clang C -O3 | 88.846 | 1.000× Clang | 50/50 |
| GM, final fixes (Clang paired block) | 92.979 | 0.956× Clang | 50/50 |
| GCC C -O3 | 93.209 | 1.000× GCC | 50/50 |
| GM, final fixes (GCC paired block) | 95.608 | 0.975× GCC | 50/50 |

Worst eight relative to Clang:

| Workload | GM (s) | Clang (s) | GM vs Clang ↑ | GM vs GCC ↑ |
|---|---:|---:|---:|---:|
| Compress::HuffDecode | 1.691 | 0.902 | 0.533× | 0.615× |
| Maze::Generator | 3.363 | 1.857 | 0.552× | 0.585× |
| Calculator::Ast | 3.142 | 2.289 | 0.729× | 0.875× |
| Etc::Words | 2.745 | 2.152 | 0.784× | 0.984× |
| Brainfuck::Array | 2.173 | 1.707 | 0.786× | 0.865× |
| Etc::LogParser | 1.293 | 1.020 | 0.789× | 1.887× |
| Sort::Merge | 1.408 | 1.145 | 0.813× | 0.833× |
| Maze::BFS | 2.207 | 1.808 | 0.819× | 0.664× |

Full table: [final-table.md](final-table.md). Absolute times belong to this host and run window.
