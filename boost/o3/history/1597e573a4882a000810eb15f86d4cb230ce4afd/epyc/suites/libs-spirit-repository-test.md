# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.102185 | 3.060770 | 3.567649 | 0.001598 | 0.001536 | 0.001559 |
| qi_repo_confix.test | RUN | 1.144033 | 3.948246 | 3.882175 | 0.001552 | 0.001867 | 0.001631 |
| qi_repo_distinct.test | RUN | 0.836908 | 2.143521 | 2.405946 | 0.001561 | 0.001564 | 0.001499 |
| qi_repo_subrule.test | RUN | 2.630814 | 10.403372 | 10.200935 | 0.001401 | 0.001728 | 0.001204 |
| qi_repo_keywords.test | RUN | 2.403062 | 9.322984 | 10.276803 | 0.001438 | 0.001261 | 0.001153 |
| qi_repo_seek.test | RUN | 0.938364 | 3.214021 | 2.827579 | 0.001629 | 0.001637 | 0.004950 |
| karma_repo_confix.test | RUN | 0.841678 | 3.248224 | 2.939189 | 0.001723 | 0.001548 | 0.001486 |
| karma_repo_subrule.test | RUN | 1.536518 | 6.251898 | 5.939299 | 0.001242 | 0.001142 | 0.001173 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.994×, 0.989× | 0.995× |
| clang | 0.993× | 1.001×, 0.985× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.000887s | 0.000902s | 0.000924s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.000917s | 0.000916s | 0.000907s | byte equality observed |
| qi_repo_advance.test | 10 | 0.000880s | 0.000866s | 0.000872s | byte equality observed |
| qi_repo_confix.test | 10 | 0.000986s | 0.000986s | 0.000983s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000878s | 0.000858s | 0.000885s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001003s | 0.000966s | 0.000960s | byte equality observed |
| qi_repo_seek.test | 10 | 0.000901s | 0.000894s | 0.000851s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.000941s | 0.000915s | 0.000941s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
