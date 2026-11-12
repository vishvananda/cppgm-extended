# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 2.015144 | 5.498584 | 7.845100 | 0.002286 | 0.002230 | 0.002208 |
| qi_repo_confix.test | RUN | 2.055317 | 7.232494 | 8.489473 | 0.002269 | 0.002323 | 0.003091 |
| qi_repo_distinct.test | RUN | 1.539521 | 3.878282 | 5.432896 | 0.002067 | 0.002303 | 0.003055 |
| qi_repo_subrule.test | RUN | 4.750780 | 18.577586 | 23.805203 | 0.002206 | 0.002468 | 0.001987 |
| qi_repo_keywords.test | RUN | 4.201298 | 17.383852 | 18.522343 | 0.001840 | 0.001682 | 0.001687 |
| qi_repo_seek.test | RUN | 1.748828 | 6.090094 | 6.401635 | 0.002228 | 0.002323 | 0.002209 |
| karma_repo_confix.test | RUN | 1.613037 | 6.046983 | 6.310669 | 0.002202 | 0.002302 | 0.002142 |
| karma_repo_subrule.test | RUN | 2.574635 | 9.518296 | 13.553528 | 0.002092 | 0.001574 | 0.001641 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.005× | 0.987×, 1.023× | 0.997× |
| clang | 0.992× | 0.985×, 1.000× | 1.031× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001586s | 0.001615s | 0.001577s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001625s | 0.001657s | 0.001610s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001583s | 0.001572s | 0.001502s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001736s | 0.001778s | 0.001762s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001555s | 0.001574s | 0.001578s | byte equality observed |
| qi_repo_keywords.test | 9 | 0.001798s | 0.001739s | 0.001723s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001543s | 0.001590s | 0.001563s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001658s | 0.001617s | 0.001631s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
