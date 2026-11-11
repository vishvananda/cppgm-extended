# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.624468 | 4.109594 | 5.310804 | 0.001786 | 0.001857 | 0.001815 |
| qi_repo_confix.test | RUN | 1.662597 | 5.319411 | 5.680623 | 0.001749 | 0.002172 | 0.002021 |
| qi_repo_distinct.test | RUN | 1.221385 | 2.870280 | 3.657353 | 0.001880 | 0.001876 | 0.002122 |
| qi_repo_subrule.test | RUN | 3.626117 | 12.975336 | 16.131749 | 0.001584 | 0.001545 | 0.001816 |
| qi_repo_keywords.test | RUN | 3.258014 | 13.165641 | 13.191909 | 0.001670 | 0.001548 | 0.001575 |
| qi_repo_seek.test | RUN | 1.334002 | 4.368276 | 4.229323 | 0.001736 | 0.001925 | 0.001824 |
| karma_repo_confix.test | RUN | 1.226611 | 4.443408 | 4.182766 | 0.001965 | 0.001978 | 0.001846 |
| karma_repo_subrule.test | RUN | 2.027074 | 8.292245 | 9.471745 | 0.001450 | 0.001534 | 0.001348 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988×† | 1.059×, 0.917× | 1.024× |
| clang | 1.027×† | 1.146×, 0.908× | 0.925× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001099s | 0.001096s | 0.001154s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001099s | 0.001055s | 0.001152s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001035s | 0.001072s | 0.001089s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001206s | 0.001197s | 0.001277s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001046s | 0.001022s | 0.001125s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001204s | 0.001170s | 0.001278s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001060s | 0.001028s | 0.001103s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001118s | 0.001085s | 0.001177s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
