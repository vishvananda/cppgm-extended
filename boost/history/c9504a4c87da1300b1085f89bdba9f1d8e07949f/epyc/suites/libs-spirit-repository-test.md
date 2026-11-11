# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.582461 | 5.273595 | 5.162385 | 0.001916 | 0.001812 | 0.001968 |
| qi_repo_confix.test | RUN | 1.600769 | 5.165677 | 5.187219 | 0.001962 | 0.002010 | 0.001971 |
| qi_repo_distinct.test | RUN | 1.248805 | 3.085583 | 3.615977 | 0.002012 | 0.002002 | 0.001901 |
| qi_repo_subrule.test | RUN | 3.265670 | 18.278097 | 13.931384 | 0.001890 | 0.001878 | 0.002532 |
| qi_repo_keywords.test | RUN | 2.748925 | 17.253449 | 12.533566 | 0.002520 | 0.002538 | 0.002824 |
| qi_repo_seek.test | RUN | 1.311944 | 3.730856 | 4.191612 | 0.001856 | 0.001802 | 0.002098 |
| karma_repo_confix.test | RUN | 1.182112 | 3.324188 | 3.675721 | 0.001985 | 0.001862 | 0.002140 |
| karma_repo_subrule.test | RUN | 1.873337 | 9.019498 | 7.097778 | 0.001656 | 0.001845 | 0.001748 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
