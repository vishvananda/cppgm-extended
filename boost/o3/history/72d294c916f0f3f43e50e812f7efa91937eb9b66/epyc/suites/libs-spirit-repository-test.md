# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.919377 | 5.475686 | 7.508359 | 0.001904 | 0.002022 | 0.002136 |
| qi_repo_confix.test | RUN | 1.965284 | 7.158423 | 8.215550 | 0.002048 | 0.002394 | 0.002340 |
| qi_repo_distinct.test | RUN | 1.458382 | 3.849941 | 5.279643 | 0.002060 | 0.002124 | 0.002046 |
| qi_repo_subrule.test | RUN | 4.685279 | 17.195384 | 19.668039 | 0.002094 | 0.002308 | 0.001514 |
| qi_repo_keywords.test | RUN | 3.939964 | 16.843946 | 19.758392 | 0.001675 | 0.001621 | 0.001606 |
| qi_repo_seek.test | RUN | 1.586382 | 6.082278 | 6.117752 | 0.001813 | 0.002213 | 0.001766 |
| karma_repo_confix.test | RUN | 1.463924 | 6.018038 | 5.966267 | 0.001810 | 0.002113 | 0.001842 |
| karma_repo_subrule.test | RUN | 2.514234 | 9.604286 | 11.895871 | 0.001635 | 0.001462 | 0.001506 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.986×, 0.993× | 1.010× |
| clang | 0.988× | 0.985×, 0.991× | 1.015× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001510s | 0.001513s | 0.001504s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001604s | 0.001547s | 0.001551s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001510s | 0.001486s | 0.001495s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001735s | 0.001717s | 0.001697s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001527s | 0.001482s | 0.001512s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001732s | 0.001703s | 0.001686s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001518s | 0.001509s | 0.001498s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001576s | 0.001599s | 0.001593s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
