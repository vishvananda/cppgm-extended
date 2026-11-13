# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.511846 | 4.217686 | 5.429341 | 0.001590 | 0.001815 | 0.001797 |
| qi_repo_confix.test | RUN | 1.532972 | 5.428460 | 5.879232 | 0.001803 | 0.002106 | 0.002029 |
| qi_repo_distinct.test | RUN | 1.132542 | 2.951524 | 3.701027 | 0.002120 | 0.001924 | 0.001595 |
| qi_repo_subrule.test | RUN | 3.655428 | 14.030759 | 16.540716 | 0.001997 | 0.001922 | 0.001928 |
| qi_repo_keywords.test | RUN | 3.044025 | 12.600242 | 13.980767 | 0.001571 | 0.001512 | 0.001588 |
| qi_repo_seek.test | RUN | 1.244342 | 4.444624 | 4.353722 | 0.001762 | 0.001870 | 0.001834 |
| karma_repo_confix.test | RUN | 1.141017 | 4.424691 | 4.342795 | 0.001984 | 0.001807 | 0.001866 |
| karma_repo_subrule.test | RUN | 2.031302 | 8.412451 | 9.804919 | 0.001426 | 0.003798 | 0.001464 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993×† | 0.935×, 1.052× | 0.980× |
| clang | 1.000×† | 1.037×, 0.964× | 1.066× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001163s | 0.001227s | 0.001134s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001200s | 0.001204s | 0.001234s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001145s | 0.001174s | 0.001091s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001329s | 0.001353s | 0.001305s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001179s | 0.001207s | 0.001190s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001331s | 0.001302s | 0.001320s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001189s | 0.001179s | 0.001178s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001257s | 0.001233s | 0.001262s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
