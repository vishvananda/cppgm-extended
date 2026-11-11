# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.913719 | 5.481582 | 6.626053 | 0.002097 | 0.003519 | 0.001988 |
| qi_repo_confix.test | RUN | 1.922687 | 7.167946 | 7.164669 | 0.003440 | 0.002860 | 0.002469 |
| qi_repo_distinct.test | RUN | 1.426667 | 3.705198 | 4.362341 | 0.002702 | 0.002724 | 0.002431 |
| qi_repo_subrule.test | RUN | 4.383294 | 18.384542 | 21.428497 | 0.003423 | 0.002124 | 0.002368 |
| qi_repo_keywords.test | RUN | 5.071937 | 18.566735 | 21.694375 | 0.002826 | 0.002068 | 0.002442 |
| qi_repo_seek.test | RUN | 1.569244 | 5.759758 | 5.264064 | 0.002926 | 0.002334 | 0.001948 |
| karma_repo_confix.test | RUN | 1.499033 | 5.759409 | 5.386776 | 0.002613 | 0.002020 | 0.002206 |
| karma_repo_subrule.test | RUN | 3.488330 | 12.741819 | 15.245294 | 0.003464 | 0.002641 | 0.003075 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.964× | 0.955×, 0.972× | 1.025× |
| clang | 0.958× | 0.952×, 0.965× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 8 | 0.001704s | 0.001756s | 0.001660s | byte equality observed |
| karma_repo_subrule.test | 6 | 0.001836s | 0.001707s | 0.001727s | byte equality observed |
| qi_repo_advance.test | 8 | 0.001609s | 0.001822s | 0.001602s | byte equality observed |
| qi_repo_confix.test | 7 | 0.001897s | 0.001994s | 0.001834s | byte equality observed |
| qi_repo_distinct.test | 8 | 0.001787s | 0.001646s | 0.001684s | byte equality observed |
| qi_repo_keywords.test | 8 | 0.001957s | 0.001863s | 0.001744s | byte equality observed |
| qi_repo_seek.test | 8 | 0.001724s | 0.001885s | 0.001587s | byte equality observed |
| qi_repo_subrule.test | 7 | 0.001820s | 0.001731s | 0.001782s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
