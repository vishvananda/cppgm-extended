# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.756887 | 1.931536 | 2.589030 | 0.003853 | 0.003867 | 0.003753 |
| constrained_ops.test | RUN | 0.835922 | 2.035374 | 2.884262 | 0.003454 | 0.004033 | 0.003505 |
| cpp-next_bug.test | RUN | 0.861144 | 2.274572 | 2.974788 | 0.003345 | 0.003943 | 0.003566 |
| deep_copy.test | RUN | 0.719240 | 1.955634 | 2.560445 | 0.003367 | 0.003945 | 0.003779 |
| display_expr.test | RUN | 0.854160 | 2.413599 | 3.000939 | 0.003348 | 0.003465 | 0.004040 |
| deduce_domain.test | RUN | 0.688659 | 1.708597 | 2.340790 | 0.003435 | 0.003760 | 0.003708 |
| env_var.test | RUN | 0.929624 | 4.096686 | 3.644446 | 0.003895 | 0.004300 | 0.004951 |
| examples.test | RUN | 0.888099 | 2.664107 | 3.417012 | 0.003561 | 0.003383 | 0.003801 |
| external_transforms.test | RUN | 0.881340 | 2.223234 | 3.087093 | 0.003290 | 0.003515 | 0.003756 |
| lambda.test | RUN | 0.837955 | 2.270465 | 3.161513 | 0.003639 | 0.003949 | 0.003858 |
| make_expr.test | RUN | 0.847591 | 3.690722 | 3.290591 | 0.004715 | 0.005192 | 0.005138 |
| matches.test | RUN | 0.755533 | 1.883372 | 2.744832 | 0.003547 | 0.003605 | 0.003790 |
| flatten.test | RUN | 1.420825 | 5.294375 | 6.380376 | 0.003850 | 0.003611 | 0.003961 |
| switch.test | RUN | 0.735332 | 1.857828 | 2.592942 | 0.003829 | 0.003512 | 0.003692 |
| toy_spirit.test | RUN | 0.836214 | 2.456550 | 3.230928 | 0.003684 | 0.003775 | 0.003856 |
| toy_spirit2.test | RUN | 0.896498 | 2.528511 | 3.510911 | 0.003556 | 0.003800 | 0.004040 |
| make.test | RUN | 0.738591 | 2.437587 | 2.592089 | 0.004349 | 0.004107 | 0.003630 |
| mem_ptr.test | RUN | 0.947513 | 3.816281 | 3.734650 | 0.004529 | 0.004821 | 0.004275 |
| mpl.test | RUN | 0.843544 | 2.081580 | 2.774749 | 0.003369 | 0.003676 | 0.003705 |
| noinvoke.test | RUN | 0.716166 | 1.638793 | 2.358618 | 0.003340 | 0.003782 | 0.003229 |
| pack_expansion.test | RUN | 0.881840 | 2.482994 | 3.111471 | 0.003385 | 0.003963 | 0.003940 |
| protect.test | RUN | 0.475020 | 1.569527 | 1.595272 | 0.002609 | 0.002725 | 0.002677 |
| bug2407.test | COMPILE | 0.645143 | 1.276189 | 1.451950 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 1.005×, 0.981× | 1.009× |
| clang | 0.987× | 0.986×, 0.988× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.002070s | 0.002044s | 0.002024s | byte equality observed |
| constrained_ops.test | 9 | 0.002048s | 0.002015s | 0.002051s | byte equality observed |
| cpp-next_bug.test | 9 | 0.002058s | 0.002055s | 0.002021s | byte equality observed |
| deduce_domain.test | 9 | 0.002052s | 0.002011s | 0.001984s | byte equality observed |
| deep_copy.test | 9 | 0.002046s | 0.002075s | 0.002028s | byte equality observed |
| display_expr.test | 9 | 0.002059s | 0.002024s | 0.002045s | byte equality observed |
| env_var.test | 8 | 0.002348s | 0.002356s | 0.002319s | byte equality observed |
| examples.test | 9 | 0.002098s | 0.002005s | 0.002000s | byte equality observed |
| external_transforms.test | 9 | 0.002030s | 0.002123s | 0.002058s | byte equality observed |
| flatten.test | 8 | 0.002157s | 0.002088s | 0.002104s | byte equality observed |
| lambda.test | 10 | 0.001995s | 0.002024s | 0.002016s | byte equality observed |
| make.test | 8 | 0.002229s | 0.002269s | 0.002175s | byte equality observed |
| make_expr.test | 8 | 0.002529s | 0.002547s | 0.002459s | byte equality observed |
| matches.test | 9 | 0.002033s | 0.001973s | 0.001993s | byte equality observed |
| mem_ptr.test | 8 | 0.002436s | 0.002428s | 0.002349s | byte equality observed |
| mpl.test | 9 | 0.001988s | 0.002014s | 0.001961s | byte equality observed |
| noinvoke.test | 9 | 0.001996s | 0.002009s | 0.002054s | byte equality observed |
| pack_expansion.test | 9 | 0.002089s | 0.002040s | 0.002075s | byte equality observed |
| protect.test | 9 | 0.002079s | 0.002067s | 0.002055s | byte equality observed |
| switch.test | 8 | 0.002019s | 0.001990s | 0.002010s | byte equality observed |
| toy_spirit.test | 9 | 0.002077s | 0.002048s | 0.001996s | byte equality observed |
| toy_spirit2.test | 9 | 0.002019s | 0.002074s | 0.002050s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
