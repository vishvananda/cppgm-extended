# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.522442 | 1.092890 | 1.149596 | 0.003917 | 0.004136 | 0.003952 |
| constrained_ops.test | RUN | 0.583453 | 1.164222 | 1.304761 | 0.002838 | 0.004154 | 0.003234 |
| cpp-next_bug.test | RUN | 0.589564 | 1.286935 | 1.377291 | 0.003333 | 0.003947 | 0.003676 |
| deep_copy.test | RUN | 0.502471 | 1.062220 | 1.169960 | 0.003349 | 0.003750 | 0.003642 |
| display_expr.test | RUN | 0.577284 | 1.290187 | 1.371801 | 0.003585 | 0.003638 | 0.003697 |
| deduce_domain.test | RUN | 0.486651 | 0.969076 | 1.070153 | 0.003718 | 0.003901 | 0.003577 |
| env_var.test | RUN | 0.605720 | 1.628396 | 1.527430 | 0.004060 | 0.004725 | 0.004651 |
| examples.test | RUN | 0.590380 | 1.552618 | 1.579355 | 0.003580 | 0.003317 | 0.003888 |
| external_transforms.test | RUN | 0.603170 | 1.477523 | 1.515443 | 0.003564 | 0.003662 | 0.003978 |
| lambda.test | RUN | 0.585868 | 1.383268 | 1.523951 | 0.003656 | 0.003262 | 0.003637 |
| make_expr.test | RUN | 0.552121 | 1.371587 | 1.498908 | 0.004298 | 0.005031 | 0.005000 |
| matches.test | RUN | 0.519453 | 1.246438 | 1.364592 | 0.003417 | 0.003759 | 0.003637 |
| flatten.test | RUN | 0.925540 | 5.231214 | 3.369521 | 0.003662 | 0.003917 | 0.004383 |
| switch.test | RUN | 0.526743 | 1.136604 | 1.230257 | 0.003098 | 0.003871 | 0.003763 |
| toy_spirit.test | RUN | 0.559090 | 1.465382 | 1.531083 | 0.003586 | 0.003918 | 0.003705 |
| toy_spirit2.test | RUN | 0.627030 | 1.527579 | 1.714023 | 0.003492 | 0.003615 | 0.004110 |
| make.test | RUN | 0.508285 | 1.027874 | 1.216654 | 0.004004 | 0.004264 | 0.003997 |
| mem_ptr.test | RUN | 0.631809 | 1.697006 | 1.655349 | 0.003933 | 0.004343 | 0.004250 |
| mpl.test | RUN | 0.586943 | 1.175993 | 1.341738 | 0.003230 | 0.003198 | 0.003603 |
| noinvoke.test | RUN | 0.499183 | 0.926574 | 1.049907 | 0.003554 | 0.003973 | 0.003792 |
| pack_expansion.test | RUN | 0.560795 | 1.252659 | 1.262124 | 0.002824 | 0.003015 | 0.003195 |
| protect.test | RUN | 0.362205 | 0.930756 | 1.094196 | 0.002891 | 0.003174 | 0.003164 |
| bug2407.test | COMPILE | 0.442156 | 0.822915 | 0.997456 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
