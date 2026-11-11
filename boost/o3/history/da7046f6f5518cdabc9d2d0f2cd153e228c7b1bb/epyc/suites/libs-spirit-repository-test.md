# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.159091 | 3.029664 | 3.750356 | 0.001603 | 0.001498 | 0.001815 |
| qi_repo_confix.test | RUN | 1.213200 | 3.967986 | 4.079495 | 0.001525 | 0.001728 | 0.001706 |
| qi_repo_distinct.test | RUN | 0.879774 | 2.108084 | 2.525969 | 0.001682 | 0.001644 | 0.001664 |
| qi_repo_subrule.test | RUN | 2.665802 | 10.351228 | 12.008373 | 0.001675 | 0.001712 | 0.001703 |
| qi_repo_keywords.test | RUN | 2.604945 | 9.098087 | 9.735668 | 0.001356 | 0.001608 | 0.001491 |
| qi_repo_seek.test | RUN | 0.976976 | 3.184310 | 3.031572 | 0.001363 | 0.001704 | 0.001558 |
| karma_repo_confix.test | RUN | 0.885040 | 3.154357 | 3.038446 | 0.001614 | 0.001686 | 0.001738 |
| karma_repo_subrule.test | RUN | 1.412375 | 6.215455 | 7.018965 | 0.001184 | 0.001174 | 0.001163 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.935×† | 1.049×, 0.822× | 0.968× |
| clang | 0.999× | 1.016×, 0.982× | 0.967× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001026s | 0.001011s | 0.001037s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001062s | 0.001019s | 0.001088s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001004s | 0.000969s | 0.001003s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001122s | 0.001074s | 0.001134s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.000987s | 0.000972s | 0.001007s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001143s | 0.001072s | 0.001135s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001011s | 0.000990s | 0.001014s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001073s | 0.001024s | 0.001082s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
