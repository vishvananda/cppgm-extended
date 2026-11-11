# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.649458 | 1.265645 | 1.638739 | 0.004870 | 0.004380 | 0.004160 |
| constrained_ops.test | RUN | 0.747356 | 1.376325 | 1.851046 | 0.003704 | 0.004287 | 0.003803 |
| cpp-next_bug.test | RUN | 0.758685 | 1.453583 | 1.862658 | 0.003772 | 0.004100 | 0.004261 |
| deep_copy.test | RUN | 0.646304 | 1.275806 | 1.627253 | 0.003871 | 0.003835 | 0.003865 |
| display_expr.test | RUN | 0.764888 | 1.538080 | 1.910247 | 0.004064 | 0.004045 | 0.003676 |
| deduce_domain.test | RUN | 0.634434 | 1.091043 | 1.492908 | 0.003980 | 0.003819 | 0.003771 |
| env_var.test | RUN | 0.797617 | 1.966496 | 2.180076 | 0.004702 | 0.005148 | 0.004677 |
| examples.test | RUN | 0.806774 | 1.792731 | 2.191213 | 0.003857 | 0.004372 | 0.004297 |
| external_transforms.test | RUN | 0.809998 | 1.725431 | 2.065390 | 0.004222 | 0.003939 | 0.004011 |
| lambda.test | RUN | 0.739324 | 1.618434 | 2.046073 | 0.004008 | 0.003909 | 0.003402 |
| make_expr.test | RUN | 0.723497 | 1.606510 | 2.021571 | 0.004943 | 0.005548 | 0.005297 |
| matches.test | RUN | 0.669780 | 1.412781 | 1.842324 | 0.004026 | 0.004325 | 0.003873 |
| flatten.test | RUN | 1.172274 | 5.719204 | 4.274786 | 0.004067 | 0.004500 | 0.004385 |
| switch.test | RUN | 0.669689 | 1.245669 | 1.677734 | 0.003990 | 0.004222 | 0.004203 |
| toy_spirit.test | RUN | 0.736830 | 1.644025 | 2.030511 | 0.003974 | 0.004130 | 0.004002 |
| toy_spirit2.test | RUN | 0.761487 | 1.825673 | 2.270571 | 0.004167 | 0.004185 | 0.004537 |
| make.test | RUN | 0.637257 | 1.226945 | 1.498842 | 0.004306 | 0.004354 | 0.004570 |
| mem_ptr.test | RUN | 0.801565 | 1.906620 | 2.123481 | 0.004445 | 0.004812 | 0.004555 |
| mpl.test | RUN | 0.745660 | 1.340235 | 1.825950 | 0.003777 | 0.003232 | 0.003923 |
| noinvoke.test | RUN | 0.628023 | 1.159625 | 1.502340 | 0.003837 | 0.004216 | 0.004138 |
| pack_expansion.test | RUN | 0.627240 | 1.429474 | 1.818126 | 0.003178 | 0.003273 | 0.003410 |
| protect.test | RUN | 0.550697 | 1.125012 | 1.140938 | 0.003145 | 0.003363 | 0.003146 |
| bug2407.test | COMPILE | 0.540823 | 0.971491 | 1.193784 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
