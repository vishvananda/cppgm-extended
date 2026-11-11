# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.616611 | 1.287147 | 1.640731 | 0.003948 | 0.004416 | 0.004042 |
| constrained_ops.test | RUN | 0.708789 | 1.385027 | 1.841548 | 0.003711 | 0.004136 | 0.003942 |
| cpp-next_bug.test | RUN | 0.716531 | 1.470928 | 1.864371 | 0.003414 | 0.003944 | 0.003956 |
| deep_copy.test | RUN | 0.613460 | 1.274897 | 1.615376 | 0.003536 | 0.003757 | 0.003632 |
| display_expr.test | RUN | 0.727877 | 1.458572 | 1.898279 | 0.003832 | 0.004007 | 0.004171 |
| deduce_domain.test | RUN | 0.588802 | 1.116885 | 1.486753 | 0.003898 | 0.004009 | 0.003968 |
| env_var.test | RUN | 0.748277 | 1.914954 | 2.140291 | 0.004454 | 0.005267 | 0.005137 |
| examples.test | RUN | 0.723018 | 1.835653 | 2.142653 | 0.003885 | 0.004162 | 0.004246 |
| external_transforms.test | RUN | 0.737366 | 1.694771 | 1.996766 | 0.004131 | 0.004387 | 0.004043 |
| lambda.test | RUN | 0.706662 | 1.658596 | 1.949988 | 0.003974 | 0.004012 | 0.004002 |
| make_expr.test | RUN | 0.703667 | 1.610493 | 1.957425 | 0.004660 | 0.005448 | 0.005204 |
| matches.test | RUN | 0.648936 | 1.372403 | 1.770811 | 0.003690 | 0.004322 | 0.004233 |
| flatten.test | RUN | 1.059131 | 5.757475 | 4.289665 | 0.003992 | 0.004175 | 0.004531 |
| switch.test | RUN | 0.640951 | 1.284106 | 1.594638 | 0.003787 | 0.004304 | 0.004124 |
| toy_spirit.test | RUN | 0.674296 | 1.608263 | 1.975829 | 0.003659 | 0.004358 | 0.003980 |
| toy_spirit2.test | RUN | 0.740255 | 1.817214 | 2.275227 | 0.003586 | 0.004272 | 0.004221 |
| make.test | RUN | 0.583346 | 1.193397 | 1.487578 | 0.003939 | 0.004227 | 0.004455 |
| mem_ptr.test | RUN | 0.765120 | 1.933828 | 2.142652 | 0.004307 | 0.005082 | 0.004551 |
| mpl.test | RUN | 0.706546 | 1.435284 | 1.766714 | 0.003550 | 0.003343 | 0.004250 |
| noinvoke.test | RUN | 0.589919 | 1.099287 | 1.495466 | 0.003703 | 0.004229 | 0.004182 |
| pack_expansion.test | RUN | 0.628819 | 1.502914 | 1.555233 | 0.003257 | 0.003230 | 0.003235 |
| protect.test | RUN | 0.588576 | 1.094622 | 1.331676 | 0.003007 | 0.003419 | 0.003086 |
| bug2407.test | COMPILE | 0.565324 | 1.009239 | 1.222263 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
