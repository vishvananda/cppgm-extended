# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 2.166770 | 5.762574 | 7.974752 | 0.002212 | 0.002126 | 0.002218 |
| qi_repo_confix.test | RUN | 2.281632 | 7.413839 | 8.625531 | 0.002358 | 0.002565 | 0.002417 |
| qi_repo_distinct.test | RUN | 1.716434 | 3.994485 | 5.422213 | 0.002232 | 0.002242 | 0.002248 |
| qi_repo_subrule.test | RUN | 4.872467 | 18.394927 | 20.669309 | 0.002412 | 0.002474 | 0.001762 |
| qi_repo_keywords.test | RUN | 4.647468 | 17.400832 | 20.839650 | 0.001856 | 0.001822 | 0.001738 |
| qi_repo_seek.test | RUN | 1.891857 | 6.364474 | 6.516300 | 0.002095 | 0.002317 | 0.002587 |
| karma_repo_confix.test | RUN | 1.700150 | 6.299271 | 6.363132 | 0.002130 | 0.002357 | 0.002189 |
| karma_repo_subrule.test | RUN | 2.606189 | 11.068711 | 12.356265 | 0.001798 | 0.001684 | 0.001800 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.973× | 0.967×, 0.980× | 0.992× |
| clang | 0.981× | 0.978×, 0.984× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001614s | 0.001607s | 0.001599s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001659s | 0.001651s | 0.001655s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001574s | 0.001567s | 0.001560s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001777s | 0.001752s | 0.001793s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001570s | 0.001564s | 0.001537s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001875s | 0.001770s | 0.001701s | byte equality observed |
| qi_repo_seek.test | 8 | 0.001602s | 0.001517s | 0.001548s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001694s | 0.001619s | 0.001657s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
