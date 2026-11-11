# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.546565 | 1.176825 | 1.319311 | 0.003129 | 0.003234 | 0.002980 |
| constrained_ops.test | RUN | 0.593662 | 1.290331 | 1.539366 | 0.003007 | 0.003063 | 0.002886 |
| cpp-next_bug.test | RUN | 0.632034 | 1.326954 | 1.536296 | 0.002757 | 0.002800 | 0.002980 |
| deep_copy.test | RUN | 0.519388 | 1.146715 | 1.329407 | 0.003109 | 0.002642 | 0.002987 |
| display_expr.test | RUN | 0.618510 | 1.460707 | 1.580723 | 0.003007 | 0.002808 | 0.003069 |
| deduce_domain.test | RUN | 0.490909 | 0.967248 | 1.215264 | 0.002741 | 0.002799 | 0.003232 |
| env_var.test | RUN | 0.647160 | 2.460059 | 1.901206 | 0.003154 | 0.003039 | 0.003649 |
| examples.test | RUN | 0.616308 | 1.604621 | 1.816929 | 0.002796 | 0.002662 | 0.002993 |
| external_transforms.test | RUN | 0.617055 | 1.359241 | 1.595108 | 0.003137 | 0.002680 | 0.003080 |
| lambda.test | RUN | 0.595474 | 1.382949 | 1.627454 | 0.002807 | 0.003123 | 0.002864 |
| make_expr.test | RUN | 0.648224 | 2.197322 | 1.772345 | 0.003479 | 0.003832 | 0.003995 |
| matches.test | RUN | 0.590482 | 1.129479 | 1.394717 | 0.002896 | 0.002853 | 0.002952 |
| flatten.test | RUN | 0.980264 | 3.308157 | 3.441409 | 0.002906 | 0.003650 | 0.003170 |
| switch.test | RUN | 0.552524 | 1.141957 | 1.279385 | 0.002839 | 0.003151 | 0.002656 |
| toy_spirit.test | RUN | 0.602753 | 1.441747 | 1.679802 | 0.003057 | 0.003149 | 0.003350 |
| toy_spirit2.test | RUN | 0.659563 | 1.544294 | 1.772625 | 0.002971 | 0.003242 | 0.002965 |
| make.test | RUN | 0.498479 | 1.456080 | 1.249975 | 0.003508 | 0.003567 | 0.003307 |
| mem_ptr.test | RUN | 0.645011 | 2.583340 | 1.849755 | 0.003249 | 0.003518 | 0.003247 |
| mpl.test | RUN | 0.579495 | 1.261054 | 1.363803 | 0.002532 | 0.003202 | 0.003303 |
| noinvoke.test | RUN | 0.495982 | 1.062603 | 1.183490 | 0.002728 | 0.003094 | 0.003179 |
| pack_expansion.test | RUN | 0.533127 | 1.572168 | 1.551845 | 0.002608 | 0.003109 | 0.003414 |
| protect.test | RUN | 0.471338 | 1.187471 | 1.148240 | 0.002441 | 0.002489 | 0.002582 |
| bug2407.test | COMPILE | 0.440397 | 0.796870 | 0.821889 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 0.997×, 1.011× | 0.956× |
| clang | 0.963× | 0.968×, 0.959× | 1.023× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001626s | 0.001596s | 0.001560s | byte equality observed |
| constrained_ops.test | 10 | 0.001597s | 0.001590s | 0.001577s | byte equality observed |
| cpp-next_bug.test | 5 | 0.001628s | 0.001616s | 0.001556s | byte equality observed |
| deduce_domain.test | 6 | 0.001619s | 0.001606s | 0.001540s | byte equality observed |
| deep_copy.test | 6 | 0.001603s | 0.001597s | 0.001557s | byte equality observed |
| display_expr.test | 7 | 0.001603s | 0.001645s | 0.001543s | byte equality observed |
| env_var.test | 7 | 0.001864s | 0.001901s | 0.001870s | byte equality observed |
| examples.test | 7 | 0.001642s | 0.001640s | 0.001548s | byte equality observed |
| external_transforms.test | 7 | 0.001673s | 0.001661s | 0.001586s | byte equality observed |
| flatten.test | 7 | 0.001688s | 0.001714s | 0.001592s | byte equality observed |
| lambda.test | 6 | 0.001624s | 0.001638s | 0.001541s | byte equality observed |
| make.test | 7 | 0.001754s | 0.001779s | 0.001696s | byte equality observed |
| make_expr.test | 5 | 0.001979s | 0.001960s | 0.001894s | byte equality observed |
| matches.test | 7 | 0.001603s | 0.001574s | 0.001552s | byte equality observed |
| mem_ptr.test | 7 | 0.001885s | 0.001859s | 0.001846s | byte equality observed |
| mpl.test | 10 | 0.001606s | 0.001632s | 0.001573s | byte equality observed |
| noinvoke.test | 10 | 0.001621s | 0.001590s | 0.001540s | byte equality observed |
| pack_expansion.test | 10 | 0.001671s | 0.001675s | 0.001598s | byte equality observed |
| protect.test | 10 | 0.001656s | 0.001627s | 0.001585s | byte equality observed |
| switch.test | 10 | 0.001601s | 0.001605s | 0.001543s | byte equality observed |
| toy_spirit.test | 10 | 0.001577s | 0.001601s | 0.001560s | byte equality observed |
| toy_spirit2.test | 10 | 0.001615s | 0.001610s | 0.001561s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
