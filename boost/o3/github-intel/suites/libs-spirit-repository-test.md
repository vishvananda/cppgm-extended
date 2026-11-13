# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.477391 | 4.219397 | 4.834748 | 0.001422 | 0.001445 | 0.001290 |
| qi_repo_confix.test | RUN | 1.508560 | 5.486661 | 5.324845 | 0.001406 | 0.001578 | 0.001347 |
| qi_repo_distinct.test | RUN | 1.094274 | 2.964825 | 3.311898 | 0.001473 | 0.001358 | 0.001231 |
| qi_repo_subrule.test | RUN | 3.543154 | 14.492778 | 13.486011 | 0.001538 | 0.001534 | 0.001066 |
| qi_repo_keywords.test | RUN | 3.216539 | 13.783500 | 13.338702 | 0.001233 | 0.001146 | 0.001135 |
| qi_repo_seek.test | RUN | 1.199375 | 4.576455 | 3.903810 | 0.001290 | 0.001443 | 0.001315 |
| karma_repo_confix.test | RUN | 1.100819 | 4.579575 | 3.958530 | 0.001496 | 0.001381 | 0.001368 |
| karma_repo_subrule.test | RUN | 1.805735 | 6.991135 | 8.270867 | 0.001074 | 0.001115 | 0.001025 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.992× | 0.989×, 0.994× | 1.009× |
| clang | 0.966× | 0.974×, 0.958× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.000938s | 0.000954s | 0.000923s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.000965s | 0.000948s | 0.000965s | byte equality observed |
| qi_repo_advance.test | 10 | 0.000937s | 0.000947s | 0.000882s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001010s | 0.000967s | 0.000944s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000893s | 0.000894s | 0.000888s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001066s | 0.001009s | 0.001009s | byte equality observed |
| qi_repo_seek.test | 10 | 0.000922s | 0.000915s | 0.000875s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.000958s | 0.000950s | 0.000950s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
