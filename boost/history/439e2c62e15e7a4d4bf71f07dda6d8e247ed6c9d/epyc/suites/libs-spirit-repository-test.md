# libs/spirit/repository/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| qi_repo_advance.test | RUN | 1.124008 | 4.081854 | 3.349445 | 0.001451 | 0.001588 | 0.001607 |
| qi_repo_confix.test | RUN | 1.099692 | 4.039192 | 3.327000 | 0.001683 | 0.001907 | 0.001569 |
| qi_repo_distinct.test | RUN | 0.858947 | 2.367703 | 2.291683 | 0.002144 | 0.001690 | 0.001534 |
| qi_repo_subrule.test | RUN | 2.226942 | 14.076508 | 8.047312 | 0.001984 | 0.001653 | 0.002138 |
| qi_repo_keywords.test | RUN | 1.832358 | 11.958178 | 7.832257 | 0.002030 | 0.002098 | 0.002117 |
| qi_repo_seek.test | RUN | 0.909382 | 2.734245 | 2.649670 | 0.001618 | 0.001670 | 0.001568 |
| karma_repo_confix.test | RUN | 0.827623 | 2.478098 | 2.343689 | 0.001646 | 0.001541 | 0.001712 |
| karma_repo_subrule.test | RUN | 1.273619 | 8.516070 | 4.926307 | 0.001414 | 0.001372 | 0.001333 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
