# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.258012 | 3.258691 | 4.142602 | 0.001793 | 0.001761 | 0.001779 |
| qi_repo_confix.test | RUN | 1.279714 | 4.328741 | 4.458410 | 0.001661 | 0.001964 | 0.001879 |
| qi_repo_distinct.test | RUN | 0.934687 | 2.276322 | 2.768241 | 0.001754 | 0.001732 | 0.001630 |
| qi_repo_subrule.test | RUN | 3.052046 | 11.451779 | 12.884644 | 0.001592 | 0.001820 | 0.001834 |
| qi_repo_keywords.test | RUN | 2.724185 | 10.954233 | 10.267492 | 0.001600 | 0.001388 | 0.001530 |
| qi_repo_seek.test | RUN | 1.026860 | 3.463579 | 3.112000 | 0.001447 | 0.001726 | 0.001830 |
| karma_repo_confix.test | RUN | 0.950200 | 3.391311 | 3.181443 | 0.001730 | 0.001730 | 0.001735 |
| karma_repo_subrule.test | RUN | 1.642824 | 5.996041 | 7.611613 | 0.001406 | 0.001391 | 0.001200 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.966×† | 0.905×, 1.028× | 1.091× |
| clang | 1.013× | 1.008×, 1.017× | 1.024× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.000917s | 0.000949s | 0.000949s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.000951s | 0.000953s | 0.000971s | byte equality observed |
| qi_repo_advance.test | 10 | 0.000926s | 0.000932s | 0.000910s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001031s | 0.001047s | 0.001038s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000905s | 0.000911s | 0.000931s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001035s | 0.001050s | 0.001035s | byte equality observed |
| qi_repo_seek.test | 10 | 0.000921s | 0.000937s | 0.000929s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.000982s | 0.000999s | 0.000996s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
