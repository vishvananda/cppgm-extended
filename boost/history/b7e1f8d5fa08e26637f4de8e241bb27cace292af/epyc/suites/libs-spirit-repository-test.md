# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.517073 | 5.278016 | 5.086541 | 0.001772 | 0.001852 | 0.001627 |
| qi_repo_confix.test | RUN | 1.509967 | 5.163168 | 5.034923 | 0.001688 | 0.002020 | 0.002034 |
| qi_repo_distinct.test | RUN | 1.175595 | 3.110147 | 3.521854 | 0.001885 | 0.001872 | 0.001790 |
| qi_repo_subrule.test | RUN | 3.065167 | 18.196567 | 11.947484 | 0.001804 | 0.001888 | 0.001892 |
| qi_repo_keywords.test | RUN | 2.686405 | 17.236542 | 11.541622 | 0.002418 | 0.002460 | 0.002584 |
| qi_repo_seek.test | RUN | 1.286770 | 3.792993 | 4.049873 | 0.001788 | 0.001867 | 0.001812 |
| karma_repo_confix.test | RUN | 1.146998 | 3.313209 | 3.586689 | 0.001888 | 0.001871 | 0.001926 |
| karma_repo_subrule.test | RUN | 1.779680 | 9.112108 | 7.502407 | 0.001545 | 0.002066 | 0.001560 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
