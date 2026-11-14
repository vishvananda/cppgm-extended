# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.220981 | 3.604386 | 4.420656 | 0.001303 | 0.001404 | 0.001535 |
| qi_repo_confix.test | RUN | 1.263090 | 4.862606 | 4.852718 | 0.001250 | 0.001278 | 0.001442 |
| qi_repo_distinct.test | RUN | 0.912023 | 2.533714 | 3.011385 | 0.001373 | 0.001306 | 0.001294 |
| qi_repo_subrule.test | RUN | 2.850693 | 11.802952 | 14.009559 | 0.001288 | 0.001030 | 0.001540 |
| qi_repo_keywords.test | RUN | 2.583006 | 11.841365 | 13.418245 | 0.001057 | 0.001005 | 0.001184 |
| qi_repo_seek.test | RUN | 1.002574 | 3.926768 | 3.534354 | 0.001097 | 0.001381 | 0.001376 |
| karma_repo_confix.test | RUN | 0.937854 | 3.927952 | 3.551031 | 0.001322 | 0.001383 | 0.001520 |
| karma_repo_subrule.test | RUN | 1.581987 | 7.342619 | 6.769524 | 0.000896 | 0.000973 | 0.001051 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 0.985×, 1.004× | 1.009× |
| clang | 0.965× | 0.962×, 0.969× | 0.985× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.000846s | 0.000855s | 0.000816s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.000872s | 0.000842s | 0.000839s | byte equality observed |
| qi_repo_advance.test | 10 | 0.000841s | 0.000822s | 0.000797s | byte equality observed |
| qi_repo_confix.test | 10 | 0.000865s | 0.000855s | 0.000836s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000810s | 0.000814s | 0.000791s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.000947s | 0.000911s | 0.000904s | byte equality observed |
| qi_repo_seek.test | 10 | 0.000829s | 0.000823s | 0.000805s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.000879s | 0.000882s | 0.000888s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
