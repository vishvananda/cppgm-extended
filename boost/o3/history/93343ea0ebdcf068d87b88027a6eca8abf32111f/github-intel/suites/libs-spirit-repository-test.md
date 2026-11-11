# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.972491 | 5.280105 | 6.450824 | 0.001770 | 0.001709 | 0.001617 |
| qi_repo_confix.test | RUN | 2.044202 | 7.058392 | 6.965216 | 0.001778 | 0.001932 | 0.001780 |
| qi_repo_distinct.test | RUN | 1.525956 | 3.743053 | 4.394536 | 0.001766 | 0.001781 | 0.001701 |
| qi_repo_subrule.test | RUN | 4.389319 | 16.539011 | 19.372280 | 0.001376 | 0.001308 | 0.001874 |
| qi_repo_keywords.test | RUN | 4.334398 | 16.006044 | 17.859090 | 0.001459 | 0.001385 | 0.001276 |
| qi_repo_seek.test | RUN | 1.629389 | 5.701132 | 5.168614 | 0.001711 | 0.001757 | 0.001727 |
| karma_repo_confix.test | RUN | 1.493437 | 5.600917 | 5.111737 | 0.001889 | 0.001806 | 0.001780 |
| karma_repo_subrule.test | RUN | 2.390274 | 9.966904 | 8.575163 | 0.001330 | 0.001249 | 0.001283 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.999× | 0.996×, 1.002× | 0.992× |
| clang | 1.000× | 0.993×, 1.006× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001007s | 0.001053s | 0.001037s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001081s | 0.001039s | 0.001064s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001030s | 0.001015s | 0.001081s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001078s | 0.001100s | 0.001069s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001018s | 0.001075s | 0.001067s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001145s | 0.001145s | 0.001103s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001036s | 0.001014s | 0.001018s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001104s | 0.001047s | 0.001037s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
