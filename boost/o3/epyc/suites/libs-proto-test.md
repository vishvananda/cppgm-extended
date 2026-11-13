# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.535875 | 1.360939 | 1.691752 | 0.003049 | 0.003074 | 0.002920 |
| constrained_ops.test | RUN | 0.626520 | 1.442256 | 1.874676 | 0.002822 | 0.003116 | 0.003223 |
| cpp-next_bug.test | RUN | 0.648346 | 1.558790 | 1.967838 | 0.002750 | 0.002892 | 0.003142 |
| deep_copy.test | RUN | 0.539496 | 1.390907 | 1.649831 | 0.002815 | 0.002782 | 0.002839 |
| display_expr.test | RUN | 0.634655 | 1.694649 | 2.006974 | 0.002846 | 0.003059 | 0.003071 |
| deduce_domain.test | RUN | 0.511256 | 1.211546 | 1.535272 | 0.003067 | 0.003009 | 0.003051 |
| env_var.test | RUN | 0.681968 | 2.877912 | 2.299086 | 0.003446 | 0.003340 | 0.003694 |
| examples.test | RUN | 0.652601 | 1.874877 | 2.241922 | 0.002892 | 0.002782 | 0.002735 |
| external_transforms.test | RUN | 0.664864 | 1.647583 | 2.060721 | 0.002852 | 0.002807 | 0.002667 |
| lambda.test | RUN | 0.640541 | 1.629626 | 2.043332 | 0.002997 | 0.002991 | 0.003061 |
| make_expr.test | RUN | 0.621408 | 2.665080 | 2.159138 | 0.003624 | 0.004296 | 0.003851 |
| matches.test | RUN | 0.563898 | 1.354124 | 1.801805 | 0.002901 | 0.003077 | 0.003003 |
| flatten.test | RUN | 1.047204 | 3.868475 | 4.382799 | 0.003201 | 0.003121 | 0.002790 |
| switch.test | RUN | 0.543986 | 1.351421 | 1.666884 | 0.002932 | 0.003069 | 0.002859 |
| toy_spirit.test | RUN | 0.618986 | 1.726354 | 2.114208 | 0.003128 | 0.003236 | 0.003323 |
| toy_spirit2.test | RUN | 0.665227 | 1.791567 | 2.345424 | 0.002941 | 0.003183 | 0.002726 |
| make.test | RUN | 0.544783 | 1.701418 | 1.683549 | 0.003496 | 0.003539 | 0.003488 |
| mem_ptr.test | RUN | 0.712792 | 2.832337 | 2.393446 | 0.003492 | 0.003826 | 0.003758 |
| mpl.test | RUN | 0.631411 | 1.421880 | 1.834801 | 0.002992 | 0.003195 | 0.003003 |
| noinvoke.test | RUN | 0.517061 | 1.195096 | 1.573427 | 0.003064 | 0.003237 | 0.003007 |
| pack_expansion.test | RUN | 0.638577 | 1.725291 | 1.999372 | 0.002468 | 0.002471 | 0.003210 |
| protect.test | RUN | 0.399032 | 1.288392 | 1.295731 | 0.002284 | 0.002413 | 0.002234 |
| bug2407.test | COMPILE | 0.486360 | 0.879575 | 0.979761 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.976×† | 0.980×, 0.973× | 0.931× |
| clang | 0.969× | 0.970×, 0.968× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.001706s | 0.001701s | 0.001674s | byte equality observed |
| constrained_ops.test | 10 | 0.001661s | 0.001637s | 0.001655s | byte equality observed |
| cpp-next_bug.test | 8 | 0.001718s | 0.001676s | 0.001610s | byte equality observed |
| deduce_domain.test | 10 | 0.001680s | 0.001634s | 0.001674s | byte equality observed |
| deep_copy.test | 10 | 0.001673s | 0.001641s | 0.001662s | byte equality observed |
| display_expr.test | 10 | 0.001681s | 0.001672s | 0.001630s | byte equality observed |
| env_var.test | 9 | 0.002000s | 0.001945s | 0.001943s | byte equality observed |
| examples.test | 10 | 0.001730s | 0.001677s | 0.001639s | byte equality observed |
| external_transforms.test | 10 | 0.001704s | 0.001652s | 0.001660s | byte equality observed |
| flatten.test | 10 | 0.001756s | 0.001742s | 0.001694s | byte equality observed |
| lambda.test | 10 | 0.001667s | 0.001632s | 0.001625s | byte equality observed |
| make.test | 8 | 0.001861s | 0.001807s | 0.001774s | byte equality observed |
| make_expr.test | 9 | 0.002058s | 0.002043s | 0.002048s | byte equality observed |
| matches.test | 10 | 0.001662s | 0.001658s | 0.001632s | byte equality observed |
| mem_ptr.test | 9 | 0.001996s | 0.001925s | 0.001933s | byte equality observed |
| mpl.test | 10 | 0.001675s | 0.001625s | 0.001634s | byte equality observed |
| noinvoke.test | 9 | 0.001739s | 0.001599s | 0.001628s | byte equality observed |
| pack_expansion.test | 10 | 0.001736s | 0.001695s | 0.001685s | byte equality observed |
| protect.test | 10 | 0.001753s | 0.001717s | 0.001681s | byte equality observed |
| switch.test | 10 | 0.001668s | 0.001648s | 0.001621s | byte equality observed |
| toy_spirit.test | 10 | 0.001684s | 0.001656s | 0.001688s | byte equality observed |
| toy_spirit2.test | 10 | 0.001707s | 0.001655s | 0.001645s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
