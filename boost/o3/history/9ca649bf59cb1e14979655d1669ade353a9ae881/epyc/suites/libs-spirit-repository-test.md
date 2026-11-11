# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.593782 | 4.089246 | 5.199169 | 0.001752 | 0.001988 | 0.001771 |
| qi_repo_confix.test | RUN | 1.629573 | 5.282207 | 5.599227 | 0.001740 | 0.002034 | 0.001888 |
| qi_repo_distinct.test | RUN | 1.243811 | 2.838428 | 3.543285 | 0.001838 | 0.001806 | 0.001579 |
| qi_repo_subrule.test | RUN | 3.661506 | 13.738488 | 16.011408 | 0.001962 | 0.002006 | 0.001864 |
| qi_repo_keywords.test | RUN | 3.453552 | 13.120385 | 12.630000 | 0.001547 | 0.001472 | 0.001417 |
| qi_repo_seek.test | RUN | 1.321273 | 4.396440 | 4.183680 | 0.001863 | 0.001822 | 0.001745 |
| karma_repo_confix.test | RUN | 1.205830 | 4.331299 | 4.208511 | 0.001883 | 0.001867 | 0.001815 |
| karma_repo_subrule.test | RUN | 2.061034 | 7.063454 | 9.244927 | 0.001387 | 0.001386 | 0.001382 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.064×† | 0.986×, 1.141× | 1.008× |
| clang | 1.069× | 1.067×, 1.070× | 0.975× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001104s | 0.001236s | 0.001249s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001155s | 0.001264s | 0.001266s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001082s | 0.001188s | 0.001205s | byte equality observed |
| qi_repo_confix.test | 9 | 0.001210s | 0.001386s | 0.001369s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001062s | 0.001222s | 0.001192s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001230s | 0.001292s | 0.001359s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001049s | 0.001186s | 0.001195s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001152s | 0.001263s | 0.001286s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
