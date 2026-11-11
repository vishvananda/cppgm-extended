# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.399124 | 3.541239 | 4.238832 | 0.004378 | 0.001866 | 0.001982 |
| qi_repo_confix.test | RUN | 1.425594 | 4.570189 | 4.601609 | 0.003563 | 0.002066 | 0.002088 |
| qi_repo_distinct.test | RUN | 1.051655 | 2.459243 | 2.807630 | 0.002256 | 0.001775 | 0.001934 |
| qi_repo_subrule.test | RUN | 3.274927 | 12.862497 | 13.564790 | 0.001808 | 0.002180 | 0.001938 |
| qi_repo_keywords.test | RUN | 2.963960 | 11.212061 | 12.707933 | 0.001664 | 0.001629 | 0.001420 |
| qi_repo_seek.test | RUN | 1.158934 | 3.716607 | 3.424489 | 0.001698 | 0.002097 | 0.001936 |
| karma_repo_confix.test | RUN | 1.071512 | 3.702310 | 3.458360 | 0.001849 | 0.002050 | 0.002010 |
| karma_repo_subrule.test | RUN | 1.721057 | 8.050870 | 5.970357 | 0.001380 | 0.001461 | 0.001574 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 0.994×, 1.006× | 0.972× |
| clang | 1.011× | 1.005×, 1.016× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.000991s | 0.001009s | 0.001022s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001021s | 0.001015s | 0.001032s | byte equality observed |
| qi_repo_advance.test | 10 | 0.000966s | 0.000980s | 0.000989s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001087s | 0.001087s | 0.001116s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000966s | 0.000958s | 0.000979s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001132s | 0.001102s | 0.001088s | byte equality observed |
| qi_repo_seek.test | 10 | 0.000987s | 0.000988s | 0.000978s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001063s | 0.001056s | 0.001071s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
