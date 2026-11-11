# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.643288 | 1.392303 | 1.711431 | 0.003244 | 0.003172 | 0.003138 |
| constrained_ops.test | RUN | 0.745265 | 1.453738 | 1.895461 | 0.003012 | 0.003121 | 0.002932 |
| cpp-next_bug.test | RUN | 0.729928 | 1.558300 | 1.946698 | 0.002881 | 0.002991 | 0.002941 |
| deep_copy.test | RUN | 0.615182 | 1.418560 | 1.700314 | 0.002601 | 0.003016 | 0.003113 |
| display_expr.test | RUN | 0.753663 | 1.702032 | 1.926696 | 0.003253 | 0.002599 | 0.003240 |
| deduce_domain.test | RUN | 0.595471 | 1.207312 | 1.561689 | 0.002974 | 0.002905 | 0.002564 |
| env_var.test | RUN | 0.799817 | 2.895098 | 2.420328 | 0.003529 | 0.003700 | 0.003765 |
| examples.test | RUN | 0.760447 | 1.893950 | 2.267630 | 0.003061 | 0.003198 | 0.003284 |
| external_transforms.test | RUN | 0.746986 | 1.677251 | 2.065439 | 0.002846 | 0.002917 | 0.002963 |
| lambda.test | RUN | 0.720363 | 1.649044 | 2.068202 | 0.002919 | 0.003112 | 0.002979 |
| make_expr.test | RUN | 0.716876 | 2.666864 | 2.185903 | 0.003770 | 0.004235 | 0.004182 |
| matches.test | RUN | 0.662906 | 1.373107 | 1.809602 | 0.002964 | 0.003029 | 0.002518 |
| flatten.test | RUN | 1.155803 | 3.938586 | 4.405856 | 0.003241 | 0.003594 | 0.003255 |
| switch.test | RUN | 0.635101 | 1.366165 | 1.702336 | 0.003299 | 0.003148 | 0.003224 |
| toy_spirit.test | RUN | 0.735694 | 1.730793 | 2.188438 | 0.003124 | 0.003041 | 0.003083 |
| toy_spirit2.test | RUN | 0.754144 | 1.838515 | 2.244671 | 0.002919 | 0.003158 | 0.003063 |
| make.test | RUN | 0.611770 | 1.680151 | 1.677991 | 0.003540 | 0.003689 | 0.003565 |
| mem_ptr.test | RUN | 0.795532 | 2.859809 | 2.481743 | 0.003650 | 0.003850 | 0.002808 |
| mpl.test | RUN | 0.719011 | 1.423249 | 1.819311 | 0.002958 | 0.003282 | 0.002813 |
| noinvoke.test | RUN | 0.603463 | 1.227819 | 1.525508 | 0.003119 | 0.003193 | 0.002782 |
| pack_expansion.test | RUN | 0.651570 | 1.788754 | 2.038253 | 0.002570 | 0.002387 | 0.002416 |
| protect.test | RUN | 0.551298 | 1.222692 | 1.053903 | 0.003458 | 0.002520 | 0.002286 |
| bug2407.test | COMPILE | 0.535011 | 0.874316 | 1.183490 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.981×† | 0.972×, 0.991× | 1.059× |
| clang | 0.983×† | 1.062×, 0.905× | 0.958× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001760s | 0.001698s | 0.001673s | byte equality observed |
| constrained_ops.test | 10 | 0.001729s | 0.001725s | 0.001704s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001756s | 0.001699s | 0.001723s | byte equality observed |
| deduce_domain.test | 10 | 0.001710s | 0.001708s | 0.001697s | byte equality observed |
| deep_copy.test | 10 | 0.001751s | 0.001665s | 0.001713s | byte equality observed |
| display_expr.test | 10 | 0.001728s | 0.001684s | 0.001692s | byte equality observed |
| env_var.test | 9 | 0.002032s | 0.002000s | 0.002032s | byte equality observed |
| examples.test | 10 | 0.001767s | 0.001695s | 0.001732s | byte equality observed |
| external_transforms.test | 10 | 0.001770s | 0.001726s | 0.001791s | byte equality observed |
| flatten.test | 10 | 0.001766s | 0.001805s | 0.001755s | byte equality observed |
| lambda.test | 10 | 0.001753s | 0.001667s | 0.001731s | byte equality observed |
| make.test | 10 | 0.001922s | 0.001854s | 0.001873s | byte equality observed |
| make_expr.test | 9 | 0.002157s | 0.002081s | 0.002134s | byte equality observed |
| matches.test | 10 | 0.001719s | 0.001667s | 0.001690s | byte equality observed |
| mem_ptr.test | 9 | 0.002015s | 0.001983s | 0.002006s | byte equality observed |
| mpl.test | 10 | 0.001711s | 0.001678s | 0.001746s | byte equality observed |
| noinvoke.test | 10 | 0.001726s | 0.001669s | 0.001698s | byte equality observed |
| pack_expansion.test | 10 | 0.001797s | 0.001734s | 0.001763s | byte equality observed |
| protect.test | 10 | 0.001788s | 0.001753s | 0.001821s | byte equality observed |
| switch.test | 9 | 0.001705s | 0.001663s | 0.001772s | byte equality observed |
| toy_spirit.test | 9 | 0.001717s | 0.001684s | 0.001694s | byte equality observed |
| toy_spirit2.test | 9 | 0.001724s | 0.001708s | 0.001697s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
