# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.448186 | 0.912510 | 1.042476 | 0.003418 | 0.003641 | 0.003082 |
| constrained_ops.test | RUN | 0.524445 | 1.010790 | 1.189614 | 0.003042 | 0.003628 | 0.002873 |
| cpp-next_bug.test | RUN | 0.529374 | 1.071139 | 1.226779 | 0.002956 | 0.003315 | 0.003314 |
| deep_copy.test | RUN | 0.444902 | 0.944387 | 1.018177 | 0.002891 | 0.003413 | 0.003239 |
| display_expr.test | RUN | 0.526309 | 1.086641 | 1.241126 | 0.003349 | 0.003529 | 0.003193 |
| deduce_domain.test | RUN | 0.433208 | 0.823362 | 0.951095 | 0.003276 | 0.003355 | 0.003169 |
| env_var.test | RUN | 0.548355 | 1.445343 | 1.357427 | 0.003501 | 0.004096 | 0.003889 |
| examples.test | RUN | 0.527746 | 1.345426 | 1.361957 | 0.003224 | 0.003468 | 0.003434 |
| external_transforms.test | RUN | 0.532137 | 1.294529 | 1.316946 | 0.003297 | 0.003191 | 0.003287 |
| lambda.test | RUN | 0.525449 | 1.208980 | 1.296547 | 0.003257 | 0.003355 | 0.002988 |
| make_expr.test | RUN | 0.508667 | 1.190350 | 1.305142 | 0.003983 | 0.004266 | 0.003264 |
| matches.test | RUN | 0.474776 | 1.010005 | 1.173683 | 0.003112 | 0.003291 | 0.002949 |
| flatten.test | RUN | 0.779674 | 4.569593 | 2.798886 | 0.003214 | 0.003731 | 0.003691 |
| switch.test | RUN | 0.466956 | 0.923915 | 1.074226 | 0.003357 | 0.003524 | 0.003295 |
| toy_spirit.test | RUN | 0.493363 | 1.199774 | 1.309063 | 0.003278 | 0.003743 | 0.003325 |
| toy_spirit2.test | RUN | 0.549483 | 1.394686 | 1.412973 | 0.002920 | 0.003439 | 0.003389 |
| make.test | RUN | 0.428055 | 0.899298 | 1.016292 | 0.003493 | 0.003771 | 0.003452 |
| mem_ptr.test | RUN | 0.543253 | 1.527616 | 1.446378 | 0.003643 | 0.004519 | 0.003872 |
| mpl.test | RUN | 0.514748 | 1.086477 | 1.186640 | 0.003112 | 0.003354 | 0.003241 |
| noinvoke.test | RUN | 0.434363 | 0.863284 | 0.937974 | 0.003088 | 0.003503 | 0.003253 |
| pack_expansion.test | RUN | 0.518384 | 1.111647 | 1.085006 | 0.002719 | 0.002597 | 0.002650 |
| protect.test | RUN | 0.407246 | 0.840705 | 0.953124 | 0.002500 | 0.002700 | 0.002490 |
| bug2407.test | COMPILE | 0.289811 | 0.736843 | 0.846770 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
