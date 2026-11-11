# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.540331 | 1.045547 | 1.115872 | 0.003985 | 0.004100 | 0.003346 |
| constrained_ops.test | RUN | 0.597124 | 1.164718 | 1.282975 | 0.003599 | 0.004136 | 0.003397 |
| cpp-next_bug.test | RUN | 0.613642 | 1.228472 | 1.313326 | 0.003260 | 0.003861 | 0.003485 |
| deep_copy.test | RUN | 0.523906 | 1.120795 | 1.124275 | 0.003511 | 0.003944 | 0.003489 |
| display_expr.test | RUN | 0.628860 | 1.287288 | 1.341455 | 0.003705 | 0.004163 | 0.003128 |
| deduce_domain.test | RUN | 0.553866 | 0.960976 | 1.046574 | 0.003836 | 0.003909 | 0.003560 |
| env_var.test | RUN | 0.698320 | 1.621568 | 1.572588 | 0.004050 | 0.004574 | 0.004513 |
| examples.test | RUN | 0.689007 | 1.615022 | 1.608874 | 0.003804 | 0.003591 | 0.003891 |
| external_transforms.test | RUN | 0.623595 | 1.511230 | 1.466110 | 0.003705 | 0.003643 | 0.003449 |
| lambda.test | RUN | 0.608892 | 1.415338 | 1.467286 | 0.003743 | 0.003673 | 0.003346 |
| make_expr.test | RUN | 0.595744 | 1.433412 | 1.446307 | 0.004311 | 0.005108 | 0.004267 |
| matches.test | RUN | 0.556260 | 1.166135 | 1.304265 | 0.003665 | 0.003942 | 0.003842 |
| flatten.test | RUN | 0.957241 | 5.160205 | 3.143245 | 0.004006 | 0.003992 | 0.004253 |
| switch.test | RUN | 0.556662 | 1.050797 | 1.228997 | 0.003984 | 0.003936 | 0.003727 |
| toy_spirit.test | RUN | 0.585587 | 1.392776 | 1.456611 | 0.003831 | 0.005031 | 0.003655 |
| toy_spirit2.test | RUN | 0.647848 | 1.559285 | 1.593627 | 0.003691 | 0.003864 | 0.003767 |
| make.test | RUN | 0.540001 | 1.010061 | 1.126124 | 0.004018 | 0.004377 | 0.003933 |
| mem_ptr.test | RUN | 0.676573 | 1.699409 | 1.596567 | 0.004392 | 0.004601 | 0.004158 |
| mpl.test | RUN | 0.617834 | 1.119428 | 1.263955 | 0.003591 | 0.003636 | 0.003447 |
| noinvoke.test | RUN | 0.524776 | 0.917364 | 1.072032 | 0.003716 | 0.003802 | 0.003609 |
| pack_expansion.test | RUN | 0.526207 | 1.135045 | 1.179018 | 0.003266 | 0.002996 | 0.002880 |
| protect.test | RUN | 0.502268 | 0.918577 | 0.954981 | 0.003127 | 0.003072 | 0.002760 |
| bug2407.test | COMPILE | 0.474768 | 0.810086 | 0.839005 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
