# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.079299 | 3.048329 | 3.493294 | 0.001394 | 0.001606 | 0.001554 |
| qi_repo_confix.test | RUN | 1.089867 | 3.963356 | 3.762619 | 0.001450 | 0.001816 | 0.001537 |
| qi_repo_distinct.test | RUN | 0.803357 | 2.122929 | 2.350096 | 0.001610 | 0.001577 | 0.001479 |
| qi_repo_subrule.test | RUN | 2.592331 | 10.403506 | 11.248831 | 0.001786 | 0.001299 | 0.001550 |
| qi_repo_keywords.test | RUN | 2.260545 | 10.011685 | 9.174554 | 0.001334 | 0.001211 | 0.001211 |
| qi_repo_seek.test | RUN | 0.885292 | 3.264881 | 2.860887 | 0.001347 | 0.001664 | 0.001566 |
| karma_repo_confix.test | RUN | 0.841889 | 3.231407 | 2.822734 | 0.001454 | 0.001530 | 0.001602 |
| karma_repo_subrule.test | RUN | 1.434490 | 5.412430 | 6.372170 | 0.001164 | 0.001223 | 0.001072 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.980× | 0.988×, 0.972× | 1.007× |
| clang | 0.978× | 0.979×, 0.977× | 0.984× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.000892s | 0.000894s | 0.000879s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.000911s | 0.000889s | 0.000908s | byte equality observed |
| qi_repo_advance.test | 10 | 0.000871s | 0.000847s | 0.000843s | byte equality observed |
| qi_repo_confix.test | 10 | 0.000969s | 0.000959s | 0.000970s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000868s | 0.000864s | 0.000841s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.000992s | 0.000962s | 0.000970s | byte equality observed |
| qi_repo_seek.test | 10 | 0.000877s | 0.000876s | 0.000861s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.000934s | 0.000912s | 0.000929s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
