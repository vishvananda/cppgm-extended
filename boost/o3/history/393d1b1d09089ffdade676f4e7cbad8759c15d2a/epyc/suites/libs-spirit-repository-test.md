# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.458904 | 4.114567 | 5.207921 | 0.001660 | 0.001818 | 0.001808 |
| qi_repo_confix.test | RUN | 1.562105 | 5.242117 | 5.665062 | 0.001930 | 0.002024 | 0.001966 |
| qi_repo_distinct.test | RUN | 1.147155 | 2.849266 | 3.565274 | 0.001857 | 0.001881 | 0.001808 |
| qi_repo_subrule.test | RUN | 3.389627 | 13.875503 | 15.986397 | 0.001517 | 0.001938 | 0.001862 |
| qi_repo_keywords.test | RUN | 3.118577 | 12.190470 | 13.342733 | 0.001509 | 0.001437 | 0.001489 |
| qi_repo_seek.test | RUN | 1.220038 | 4.418184 | 4.195210 | 0.001752 | 0.001978 | 0.001790 |
| karma_repo_confix.test | RUN | 1.129168 | 4.255805 | 4.118629 | 0.001554 | 0.001837 | 0.001789 |
| karma_repo_subrule.test | RUN | 1.967007 | 8.279134 | 8.780013 | 0.001357 | 0.001363 | 0.001282 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.956×† | 0.923×, 0.989× | 0.942× |
| clang | 1.016×† | 0.977×, 1.055× | 1.019× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001044s | 0.001045s | 0.001050s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001091s | 0.001047s | 0.001075s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001023s | 0.001015s | 0.001002s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001155s | 0.001142s | 0.001148s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001024s | 0.000979s | 0.000982s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001153s | 0.001092s | 0.001105s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001038s | 0.001044s | 0.001048s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001062s | 0.001057s | 0.001101s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
