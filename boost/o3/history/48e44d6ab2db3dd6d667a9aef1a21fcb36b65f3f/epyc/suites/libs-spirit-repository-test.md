# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 2.156365 | 5.560438 | 7.723210 | 0.002228 | 0.002040 | 0.002134 |
| qi_repo_confix.test | RUN | 2.205486 | 7.201598 | 8.270402 | 0.002076 | 0.002456 | 0.002376 |
| qi_repo_distinct.test | RUN | 1.639832 | 3.892330 | 5.366651 | 0.002321 | 0.002232 | 0.002246 |
| qi_repo_subrule.test | RUN | 4.892889 | 18.325703 | 20.445964 | 0.002421 | 0.002303 | 0.001623 |
| qi_repo_keywords.test | RUN | 4.470752 | 15.977853 | 20.185512 | 0.001858 | 0.001647 | 0.001625 |
| qi_repo_seek.test | RUN | 1.848500 | 6.117498 | 6.225142 | 0.002104 | 0.002187 | 0.002112 |
| karma_repo_confix.test | RUN | 1.717197 | 6.116443 | 6.118807 | 0.002266 | 0.002205 | 0.001696 |
| karma_repo_subrule.test | RUN | 2.619563 | 10.650604 | 11.612100 | 0.001824 | 0.001440 | 0.001516 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.974× | 0.973×, 0.975× | 0.998× |
| clang | 0.988× | 0.995×, 0.981× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| karma_repo_confix.test | 10 | 0.001559s | 0.001595s | 0.001576s | byte equality observed |
| karma_repo_subrule.test | 10 | 0.001650s | 0.001581s | 0.001621s | byte equality observed |
| qi_repo_advance.test | 10 | 0.001570s | 0.001541s | 0.001522s | byte equality observed |
| qi_repo_confix.test | 10 | 0.001795s | 0.001769s | 0.001726s | byte equality observed |
| qi_repo_distinct.test | 10 | 0.001555s | 0.001558s | 0.001537s | byte equality observed |
| qi_repo_keywords.test | 10 | 0.001812s | 0.001720s | 0.001712s | byte equality observed |
| qi_repo_seek.test | 10 | 0.001547s | 0.001585s | 0.001525s | byte equality observed |
| qi_repo_subrule.test | 10 | 0.001676s | 0.001610s | 0.001618s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
