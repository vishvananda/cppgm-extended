# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.638295 | 1.369709 | 1.687152 | 0.003184 | 0.003271 | 0.003118 |
| constrained_ops.test | RUN | 0.726626 | 1.433851 | 1.911808 | 0.003074 | 0.003203 | 0.003105 |
| cpp-next_bug.test | RUN | 0.724926 | 1.598457 | 1.933984 | 0.002791 | 0.002959 | 0.002709 |
| deep_copy.test | RUN | 0.604918 | 1.350764 | 1.635267 | 0.002975 | 0.003025 | 0.002843 |
| display_expr.test | RUN | 0.743802 | 1.686082 | 2.024571 | 0.003091 | 0.002939 | 0.003072 |
| deduce_domain.test | RUN | 0.609895 | 1.205482 | 1.462818 | 0.002849 | 0.002855 | 0.002748 |
| env_var.test | RUN | 0.785842 | 2.911432 | 2.378810 | 0.003318 | 0.003419 | 0.003639 |
| examples.test | RUN | 0.744625 | 1.815100 | 2.256186 | 0.002881 | 0.002938 | 0.003109 |
| external_transforms.test | RUN | 0.729820 | 1.645499 | 2.061201 | 0.003292 | 0.003132 | 0.002841 |
| lambda.test | RUN | 0.698856 | 1.655304 | 2.057604 | 0.002750 | 0.002813 | 0.003001 |
| make_expr.test | RUN | 0.706358 | 2.642368 | 2.193478 | 0.003707 | 0.004033 | 0.004101 |
| matches.test | RUN | 0.640504 | 1.349472 | 1.717293 | 0.003008 | 0.002892 | 0.003011 |
| flatten.test | RUN | 1.145516 | 3.869797 | 4.339421 | 0.002984 | 0.003167 | 0.003184 |
| switch.test | RUN | 0.631142 | 1.346828 | 1.717518 | 0.003151 | 0.002966 | 0.003202 |
| toy_spirit.test | RUN | 0.694286 | 1.759619 | 2.073521 | 0.002998 | 0.003159 | 0.003294 |
| toy_spirit2.test | RUN | 0.777406 | 1.811489 | 2.308988 | 0.002884 | 0.003072 | 0.002897 |
| make.test | RUN | 0.614187 | 1.748328 | 1.652790 | 0.003462 | 0.003391 | 0.003394 |
| mem_ptr.test | RUN | 0.794520 | 2.772992 | 2.395112 | 0.003463 | 0.003766 | 0.003833 |
| mpl.test | RUN | 0.727661 | 1.461170 | 1.815831 | 0.002775 | 0.003126 | 0.002880 |
| noinvoke.test | RUN | 0.590268 | 1.166155 | 1.510036 | 0.002878 | 0.003149 | 0.003069 |
| pack_expansion.test | RUN | 0.733207 | 1.770972 | 1.969039 | 0.002432 | 0.003343 | 0.003205 |
| protect.test | RUN | 0.544937 | 1.213487 | 1.218816 | 0.002463 | 0.002415 | 0.002286 |
| bug2407.test | COMPILE | 0.406891 | 0.918776 | 0.860757 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.978× | 0.982×, 0.974× | 0.990× |
| clang | 0.980×† | 0.902×, 1.058× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001741s | 0.001735s | 0.001717s | byte equality observed |
| constrained_ops.test | 9 | 0.001754s | 0.001687s | 0.001738s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001731s | 0.001655s | 0.001739s | byte equality observed |
| deduce_domain.test | 9 | 0.001723s | 0.001716s | 0.001711s | byte equality observed |
| deep_copy.test | 10 | 0.001744s | 0.001687s | 0.001762s | byte equality observed |
| display_expr.test | 10 | 0.001711s | 0.001724s | 0.001716s | byte equality observed |
| env_var.test | 10 | 0.002020s | 0.001978s | 0.002018s | byte equality observed |
| examples.test | 10 | 0.001777s | 0.001692s | 0.001726s | byte equality observed |
| external_transforms.test | 10 | 0.001755s | 0.001756s | 0.001780s | byte equality observed |
| flatten.test | 10 | 0.001807s | 0.001742s | 0.001731s | byte equality observed |
| lambda.test | 10 | 0.001732s | 0.001677s | 0.001696s | byte equality observed |
| make.test | 10 | 0.001931s | 0.001849s | 0.001845s | byte equality observed |
| make_expr.test | 9 | 0.002146s | 0.002074s | 0.002096s | byte equality observed |
| matches.test | 10 | 0.001743s | 0.001677s | 0.001730s | byte equality observed |
| mem_ptr.test | 9 | 0.002022s | 0.001973s | 0.001982s | byte equality observed |
| mpl.test | 10 | 0.001760s | 0.001674s | 0.001703s | byte equality observed |
| noinvoke.test | 10 | 0.001713s | 0.001727s | 0.001663s | byte equality observed |
| pack_expansion.test | 10 | 0.001830s | 0.001774s | 0.001756s | byte equality observed |
| protect.test | 10 | 0.001786s | 0.001778s | 0.001727s | byte equality observed |
| switch.test | 10 | 0.001738s | 0.001688s | 0.001748s | byte equality observed |
| toy_spirit.test | 10 | 0.001710s | 0.001682s | 0.001707s | byte equality observed |
| toy_spirit2.test | 9 | 0.001758s | 0.001702s | 0.001740s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
