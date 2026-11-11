# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.551776 | 1.261358 | 1.330173 | 0.002870 | 0.003258 | 0.002777 |
| constrained_ops.test | RUN | 0.628088 | 1.235877 | 1.481923 | 0.002849 | 0.002867 | 0.002905 |
| cpp-next_bug.test | RUN | 0.635249 | 1.383076 | 1.617757 | 0.002688 | 0.002998 | 0.003097 |
| deep_copy.test | RUN | 0.546333 | 1.214440 | 1.332633 | 0.003134 | 0.002973 | 0.002812 |
| display_expr.test | RUN | 0.636366 | 1.498223 | 1.549700 | 0.003150 | 0.002781 | 0.002793 |
| deduce_domain.test | RUN | 0.523151 | 1.116459 | 1.185910 | 0.002736 | 0.002737 | 0.002971 |
| env_var.test | RUN | 0.677029 | 2.513046 | 1.911984 | 0.003336 | 0.003593 | 0.003815 |
| examples.test | RUN | 0.646150 | 1.620535 | 1.835111 | 0.002964 | 0.002560 | 0.002934 |
| external_transforms.test | RUN | 0.689415 | 1.396895 | 1.567991 | 0.003103 | 0.002648 | 0.003194 |
| lambda.test | RUN | 0.688227 | 1.479268 | 1.693757 | 0.003130 | 0.002904 | 0.003481 |
| make_expr.test | RUN | 0.681249 | 2.304569 | 1.786923 | 0.003705 | 0.003841 | 0.004046 |
| matches.test | RUN | 0.602141 | 1.199984 | 1.437251 | 0.002891 | 0.003097 | 0.003359 |
| flatten.test | RUN | 1.020277 | 3.580859 | 3.547096 | 0.004460 | 0.003499 | 0.003205 |
| switch.test | RUN | 0.582142 | 1.152429 | 1.317396 | 0.003195 | 0.002924 | 0.003138 |
| toy_spirit.test | RUN | 0.637005 | 1.534072 | 1.741775 | 0.003141 | 0.003256 | 0.003284 |
| toy_spirit2.test | RUN | 0.684212 | 1.636809 | 1.868269 | 0.003059 | 0.002987 | 0.002704 |
| make.test | RUN | 0.565715 | 1.528854 | 1.359091 | 0.003385 | 0.003215 | 0.003535 |
| mem_ptr.test | RUN | 0.720209 | 2.402275 | 1.971989 | 0.003398 | 0.003542 | 0.003631 |
| mpl.test | RUN | 0.628682 | 1.219944 | 1.399202 | 0.002787 | 0.003014 | 0.003064 |
| noinvoke.test | RUN | 0.542680 | 1.039185 | 1.163123 | 0.002933 | 0.002968 | 0.002925 |
| pack_expansion.test | RUN | 0.621783 | 1.512140 | 1.471756 | 0.002472 | 0.003163 | 0.002494 |
| protect.test | RUN | 0.488327 | 1.068255 | 0.929863 | 0.002701 | 0.002468 | 0.002530 |
| bug2407.test | COMPILE | 0.362084 | 0.733109 | 0.788590 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.933×† | 0.899×, 0.966× | 0.973× |
| clang | 0.950× | 0.935×, 0.965× | 0.986× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001648s | 0.001545s | 0.001664s | byte equality observed |
| constrained_ops.test | 10 | 0.001697s | 0.001557s | 0.001569s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001725s | 0.001563s | 0.001605s | byte equality observed |
| deduce_domain.test | 10 | 0.001619s | 0.001570s | 0.001572s | byte equality observed |
| deep_copy.test | 8 | 0.001620s | 0.001563s | 0.001558s | byte equality observed |
| display_expr.test | 10 | 0.001641s | 0.001628s | 0.001550s | byte equality observed |
| env_var.test | 10 | 0.001905s | 0.001883s | 0.001819s | byte equality observed |
| examples.test | 10 | 0.001681s | 0.001583s | 0.001571s | byte equality observed |
| external_transforms.test | 10 | 0.001736s | 0.001597s | 0.001590s | byte equality observed |
| flatten.test | 10 | 0.001740s | 0.001615s | 0.001613s | byte equality observed |
| lambda.test | 10 | 0.001694s | 0.001567s | 0.001593s | byte equality observed |
| make.test | 10 | 0.001807s | 0.001744s | 0.001716s | byte equality observed |
| make_expr.test | 9 | 0.002026s | 0.001914s | 0.001980s | byte equality observed |
| matches.test | 10 | 0.001625s | 0.001593s | 0.001706s | byte equality observed |
| mem_ptr.test | 10 | 0.001908s | 0.001831s | 0.001883s | byte equality observed |
| mpl.test | 10 | 0.001605s | 0.001567s | 0.001621s | byte equality observed |
| noinvoke.test | 10 | 0.001606s | 0.001561s | 0.001595s | byte equality observed |
| pack_expansion.test | 8 | 0.001690s | 0.001604s | 0.001676s | byte equality observed |
| protect.test | 10 | 0.001717s | 0.001600s | 0.001662s | byte equality observed |
| switch.test | 10 | 0.001660s | 0.001569s | 0.001667s | byte equality observed |
| toy_spirit.test | 10 | 0.001656s | 0.001555s | 0.001750s | byte equality observed |
| toy_spirit2.test | 10 | 0.001672s | 0.001542s | 0.001719s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
