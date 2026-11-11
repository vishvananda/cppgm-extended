# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.852633 | 1.935073 | 2.670710 | 0.003754 | 0.003995 | 0.003997 |
| constrained_ops.test | RUN | 0.949026 | 2.021073 | 2.932684 | 0.003437 | 0.003868 | 0.003970 |
| cpp-next_bug.test | RUN | 0.966856 | 2.236706 | 3.059135 | 0.003359 | 0.003726 | 0.003550 |
| deep_copy.test | RUN | 0.836055 | 1.899961 | 2.582654 | 0.003616 | 0.003630 | 0.004040 |
| display_expr.test | RUN | 0.998617 | 2.383158 | 3.089646 | 0.003792 | 0.003551 | 0.003873 |
| deduce_domain.test | RUN | 0.806217 | 1.685169 | 2.412731 | 0.003742 | 0.003445 | 0.003811 |
| env_var.test | RUN | 1.085413 | 3.970465 | 3.627091 | 0.004396 | 0.004361 | 0.004297 |
| examples.test | RUN | 0.997216 | 2.630722 | 3.544448 | 0.003639 | 0.003321 | 0.004097 |
| external_transforms.test | RUN | 1.010561 | 2.325096 | 3.135478 | 0.003347 | 0.003459 | 0.004328 |
| lambda.test | RUN | 0.960669 | 2.261981 | 3.211986 | 0.003408 | 0.003297 | 0.003949 |
| make_expr.test | RUN | 0.977073 | 3.752450 | 3.408907 | 0.004606 | 0.004873 | 0.005013 |
| matches.test | RUN | 0.919687 | 1.889451 | 2.795515 | 0.003369 | 0.003206 | 0.003911 |
| flatten.test | RUN | 1.540768 | 5.227460 | 6.674827 | 0.003564 | 0.003608 | 0.004220 |
| switch.test | RUN | 0.858580 | 1.877786 | 2.640314 | 0.003968 | 0.003506 | 0.004036 |
| toy_spirit.test | RUN | 1.000775 | 2.498059 | 3.331150 | 0.003734 | 0.003836 | 0.003711 |
| toy_spirit2.test | RUN | 1.017075 | 2.530355 | 3.422730 | 0.003501 | 0.003617 | 0.003936 |
| make.test | RUN | 0.841529 | 2.402102 | 2.626189 | 0.003781 | 0.004594 | 0.004318 |
| mem_ptr.test | RUN | 1.088042 | 3.822730 | 3.770384 | 0.004523 | 0.004325 | 0.004213 |
| mpl.test | RUN | 0.950256 | 1.984685 | 2.850516 | 0.003308 | 0.003543 | 0.003753 |
| noinvoke.test | RUN | 0.792801 | 1.690286 | 2.413179 | 0.003442 | 0.003769 | 0.003491 |
| pack_expansion.test | RUN | 0.892097 | 2.479867 | 3.007261 | 0.002953 | 0.003762 | 0.002804 |
| protect.test | RUN | 0.721105 | 1.877591 | 1.779292 | 0.002896 | 0.002778 | 0.002767 |
| bug2407.test | COMPILE | 0.685366 | 1.044705 | 1.376489 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.983× | 0.978×, 0.989× | 0.999× |
| clang | 0.967× | 0.963×, 0.971× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.002131s | 0.002067s | 0.002073s | byte equality observed |
| constrained_ops.test | 9 | 0.002129s | 0.002104s | 0.002011s | byte equality observed |
| cpp-next_bug.test | 8 | 0.002063s | 0.002008s | 0.002022s | byte equality observed |
| deduce_domain.test | 9 | 0.002084s | 0.002052s | 0.001979s | byte equality observed |
| deep_copy.test | 9 | 0.002063s | 0.002058s | 0.001968s | byte equality observed |
| display_expr.test | 9 | 0.002082s | 0.002058s | 0.002014s | byte equality observed |
| env_var.test | 8 | 0.002452s | 0.002361s | 0.002358s | byte equality observed |
| examples.test | 9 | 0.002066s | 0.002108s | 0.002015s | byte equality observed |
| external_transforms.test | 9 | 0.002085s | 0.002052s | 0.002137s | byte equality observed |
| flatten.test | 8 | 0.002175s | 0.002143s | 0.002084s | byte equality observed |
| lambda.test | 9 | 0.002065s | 0.002036s | 0.002081s | byte equality observed |
| make.test | 8 | 0.002226s | 0.002212s | 0.002238s | byte equality observed |
| make_expr.test | 8 | 0.002541s | 0.002578s | 0.002463s | byte equality observed |
| matches.test | 9 | 0.002089s | 0.002021s | 0.001994s | byte equality observed |
| mem_ptr.test | 8 | 0.002408s | 0.002363s | 0.002357s | byte equality observed |
| mpl.test | 9 | 0.002077s | 0.002073s | 0.002043s | byte equality observed |
| noinvoke.test | 8 | 0.002053s | 0.002072s | 0.002046s | byte equality observed |
| pack_expansion.test | 8 | 0.002139s | 0.002158s | 0.002109s | byte equality observed |
| protect.test | 9 | 0.002187s | 0.002176s | 0.002060s | byte equality observed |
| switch.test | 8 | 0.002149s | 0.002066s | 0.001982s | byte equality observed |
| toy_spirit.test | 9 | 0.002186s | 0.002052s | 0.001997s | byte equality observed |
| toy_spirit2.test | 8 | 0.002110s | 0.002037s | 0.002066s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
