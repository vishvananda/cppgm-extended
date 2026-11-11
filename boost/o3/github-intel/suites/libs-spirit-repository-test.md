# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.891531 | 5.031864 | 5.976342 | 0.001807 | 0.001824 | 0.001836 |
| qi_repo_confix.test | RUN | 1.939396 | 6.669514 | 6.543604 | 0.001843 | 0.002119 | 0.001882 |
| qi_repo_distinct.test | RUN | 1.429207 | 3.529366 | 4.061222 | 0.002004 | 0.001890 | 0.001733 |
| qi_repo_subrule.test | RUN | 4.230832 | 17.373055 | 16.825576 | 0.001539 | 0.001976 | 0.001408 |
| qi_repo_keywords.test | RUN | 4.314409 | 14.931693 | 16.408939 | 0.001624 | 0.001481 | 0.001402 |
| qi_repo_seek.test | RUN | 1.566183 | 5.413569 | 4.850704 | 0.001754 | 0.001822 | 0.001818 |
| karma_repo_confix.test | RUN | 1.458742 | 5.388571 | 4.809541 | 0.001820 | 0.001770 | 0.001870 |
| karma_repo_subrule.test | RUN | 2.425582 | 10.167804 | 10.228168 | 0.001442 | 0.001365 | 0.001330 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.856× | 0.861×, 0.850× | 0.966× |
| clang | 0.994× | 0.974×, 1.013× | 1.026× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001278s | 0.001072s | 0.001271s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001302s | 0.001124s | 0.001265s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001231s | 0.001049s | 0.001204s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001334s | 0.001149s | 0.001277s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001224s | 0.001095s | 0.001206s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001405s | 0.001219s | 0.001360s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001249s | 0.001063s | 0.001197s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001352s | 0.001164s | 0.001319s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
