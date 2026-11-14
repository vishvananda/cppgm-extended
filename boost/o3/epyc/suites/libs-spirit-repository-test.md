# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.473976 | 4.151614 | 5.275155 | 0.001738 | 0.001776 | 0.001782 |
| qi_repo_confix.test | RUN | 1.503286 | 5.387442 | 5.736877 | 0.001702 | 0.002078 | 0.001973 |
| qi_repo_distinct.test | RUN | 1.108275 | 2.897819 | 3.602549 | 0.001886 | 0.001841 | 0.001794 |
| qi_repo_subrule.test | RUN | 3.499458 | 14.338469 | 14.581300 | 0.001915 | 0.002067 | 0.001530 |
| qi_repo_keywords.test | RUN | 3.043465 | 13.461047 | 14.872586 | 0.001636 | 0.001423 | 0.001523 |
| qi_repo_seek.test | RUN | 1.240324 | 4.488924 | 4.311204 | 0.001822 | 0.001896 | 0.001897 |
| karma_repo_confix.test | RUN | 1.145313 | 4.402502 | 4.316758 | 0.001810 | 0.001852 | 0.001827 |
| karma_repo_subrule.test | RUN | 2.005120 | 7.370799 | 9.209545 | 0.001386 | 0.001464 | 0.001416 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.957×† | 0.924×, 0.990× | 1.049× |
| clang | 0.944×† | 0.909×, 0.978× | 0.940× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001058s | 0.001050s | 0.001052s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001083s | 0.001033s | 0.001050s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001018s | 0.001033s | 0.000989s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001171s | 0.001153s | 0.001149s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001043s | 0.001028s | 0.000996s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001172s | 0.001126s | 0.001146s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001019s | 0.001010s | 0.001003s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001123s | 0.001050s | 0.001054s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
