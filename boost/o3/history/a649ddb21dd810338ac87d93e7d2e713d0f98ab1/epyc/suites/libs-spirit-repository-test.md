# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.984079 | 5.603193 | 8.005816 | 0.002030 | 0.002163 | 0.002248 |
| qi_repo_confix.test | RUN | 2.012427 | 7.241513 | 8.664791 | 0.002072 | 0.002372 | 0.002101 |
| qi_repo_distinct.test | RUN | 1.476063 | 3.918478 | 5.443020 | 0.002483 | 0.002130 | 0.002382 |
| qi_repo_subrule.test | RUN | 4.548099 | 17.641195 | 23.699785 | 0.002249 | 0.002262 | 0.002359 |
| qi_repo_keywords.test | RUN | 4.062051 | 17.190323 | 19.571629 | 0.001725 | 0.001547 | 0.001749 |
| qi_repo_seek.test | RUN | 1.650508 | 6.132937 | 6.484483 | 0.001816 | 0.002072 | 0.002288 |
| karma_repo_confix.test | RUN | 1.602105 | 5.983797 | 6.399957 | 0.002024 | 0.002270 | 0.002258 |
| karma_repo_subrule.test | RUN | 2.448803 | 9.299639 | 13.008561 | 0.001723 | 0.001587 | 0.001742 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.982× | 0.977×, 0.987× | 1.009× |
| clang | 0.983× | 0.988×, 0.978× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001571s | 0.001552s | 0.001549s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001575s | 0.001556s | 0.001563s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001522s | 0.001536s | 0.001537s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001723s | 0.001728s | 0.001721s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001531s | 0.001477s | 0.001514s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001742s | 0.001621s | 0.001657s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001527s | 0.001504s | 0.001530s | byte equality observed |
| qi_repo_subrule.test | 9 | 0.001613s | 0.001574s | 0.001548s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
