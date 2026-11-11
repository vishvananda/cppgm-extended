# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.220060 | 4.736660 | 3.896027 | 0.001876 | 0.001910 | 0.001672 |
| qi_repo_confix.test | RUN | 1.230215 | 4.583910 | 3.877421 | 0.001834 | 0.002164 | 0.002050 |
| qi_repo_distinct.test | RUN | 0.958491 | 2.692680 | 2.620963 | 0.001832 | 0.001811 | 0.001772 |
| qi_repo_subrule.test | RUN | 2.558477 | 16.338489 | 10.712278 | 0.001812 | 0.002390 | 0.002422 |
| qi_repo_keywords.test | RUN | 2.185821 | 13.845090 | 9.468417 | 0.002853 | 0.002424 | 0.002482 |
| qi_repo_seek.test | RUN | 1.049465 | 3.177937 | 3.105801 | 0.001932 | 0.001910 | 0.001846 |
| karma_repo_confix.test | RUN | 0.920171 | 2.745795 | 2.740314 | 0.001706 | 0.002058 | 0.001860 |
| karma_repo_subrule.test | RUN | 1.534477 | 9.610465 | 5.411342 | 0.001645 | 0.001630 | 0.001590 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
