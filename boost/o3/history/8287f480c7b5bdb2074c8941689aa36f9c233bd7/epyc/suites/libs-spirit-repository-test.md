# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.629506 | 4.068543 | 5.167629 | 0.001768 | 0.001832 | 0.001734 |
| qi_repo_confix.test | RUN | 1.640873 | 5.353753 | 5.564643 | 0.001896 | 0.002091 | 0.001904 |
| qi_repo_distinct.test | RUN | 1.228554 | 2.855804 | 3.557645 | 0.001823 | 0.001883 | 0.001680 |
| qi_repo_subrule.test | RUN | 3.610922 | 12.695308 | 13.866817 | 0.002010 | 0.001980 | 0.001416 |
| qi_repo_keywords.test | RUN | 3.443445 | 12.876056 | 14.020456 | 0.001574 | 0.001478 | 0.001383 |
| qi_repo_seek.test | RUN | 1.334931 | 4.358592 | 4.217427 | 0.002090 | 0.001867 | 0.001908 |
| karma_repo_confix.test | RUN | 1.255270 | 4.379741 | 4.122396 | 0.001854 | 0.001806 | 0.001638 |
| karma_repo_subrule.test | RUN | 2.009266 | 7.935841 | 8.461967 | 0.001341 | 0.001361 | 0.001307 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.970×† | 0.934×, 1.005× | 1.091× |
| clang | 0.927×† | 0.858×, 0.996× | 0.939× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001113s | 0.001070s | 0.001128s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001184s | 0.001117s | 0.001097s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001132s | 0.001040s | 0.001063s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001279s | 0.001170s | 0.001170s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001128s | 0.001066s | 0.001050s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001282s | 0.001134s | 0.001165s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001113s | 0.001072s | 0.001042s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001197s | 0.001116s | 0.001111s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
