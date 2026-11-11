# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.243204 | 4.612606 | 3.636644 | 0.001588 | 0.002020 | 0.001616 |
| qi_repo_confix.test | RUN | 1.253222 | 4.647014 | 3.621912 | 0.001818 | 0.001954 | 0.001770 |
| qi_repo_distinct.test | RUN | 0.960831 | 2.601079 | 2.434436 | 0.002228 | 0.001853 | 0.001813 |
| qi_repo_subrule.test | RUN | 2.618947 | 15.076824 | 10.043592 | 0.002380 | 0.002017 | 0.002353 |
| qi_repo_keywords.test | RUN | 2.255349 | 15.573462 | 8.328256 | 0.002380 | 0.002526 | 0.002292 |
| qi_repo_seek.test | RUN | 1.022890 | 3.330734 | 2.898669 | 0.001546 | 0.001762 | 0.001755 |
| karma_repo_confix.test | RUN | 0.943085 | 2.892872 | 2.531221 | 0.001783 | 0.002130 | 0.001831 |
| karma_repo_subrule.test | RUN | 1.518930 | 9.577756 | 5.890090 | 0.001682 | 0.001860 | 0.001490 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
