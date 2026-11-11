# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.459768 | 3.656770 | 4.269677 | 0.002198 | 0.002176 | 0.001852 |
| qi_repo_confix.test | RUN | 1.507793 | 4.726702 | 4.674581 | 0.001805 | 0.002048 | 0.001976 |
| qi_repo_distinct.test | RUN | 1.099519 | 2.466469 | 2.895121 | 0.002351 | 0.001918 | 0.001688 |
| qi_repo_subrule.test | RUN | 3.427365 | 12.113032 | 13.939529 | 0.002436 | 0.002047 | 0.002084 |
| qi_repo_keywords.test | RUN | 2.922657 | 11.901596 | 11.287455 | 0.001836 | 0.001582 | 0.001718 |
| qi_repo_seek.test | RUN | 1.189586 | 3.857442 | 3.461870 | 0.001964 | 0.002050 | 0.001950 |
| karma_repo_confix.test | RUN | 1.075877 | 3.894985 | 3.477845 | 0.002022 | 0.001923 | 0.001945 |
| karma_repo_subrule.test | RUN | 2.009063 | 7.658283 | 8.238734 | 0.001588 | 0.001534 | 0.001405 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.948× | 0.942×, 0.954× | 1.029× |
| clang | 0.920×† | 0.871×, 0.969× | 0.987× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001045s | 0.000981s | 0.000997s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001051s | 0.000986s | 0.001022s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001004s | 0.000949s | 0.000958s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001109s | 0.001072s | 0.001092s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000982s | 0.000969s | 0.000981s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001134s | 0.001062s | 0.001098s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001004s | 0.000954s | 0.000989s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001123s | 0.001017s | 0.001057s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
