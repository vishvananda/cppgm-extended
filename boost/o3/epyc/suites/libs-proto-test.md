# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.571853 | 1.377276 | 1.731165 | 0.002900 | 0.003290 | 0.003089 |
| constrained_ops.test | RUN | 0.657068 | 1.487772 | 1.950448 | 0.002555 | 0.003249 | 0.002954 |
| cpp-next_bug.test | RUN | 0.648495 | 1.553809 | 1.986090 | 0.002837 | 0.003226 | 0.002900 |
| deep_copy.test | RUN | 0.548248 | 1.395255 | 1.679632 | 0.002842 | 0.003253 | 0.003056 |
| display_expr.test | RUN | 0.644902 | 1.768176 | 2.074835 | 0.003051 | 0.002763 | 0.003285 |
| deduce_domain.test | RUN | 0.519750 | 1.202172 | 1.483063 | 0.002753 | 0.002623 | 0.002980 |
| env_var.test | RUN | 0.726255 | 2.969341 | 2.493714 | 0.003579 | 0.003433 | 0.003894 |
| examples.test | RUN | 0.697661 | 2.014755 | 2.318264 | 0.002481 | 0.002985 | 0.003072 |
| external_transforms.test | RUN | 0.669104 | 1.685447 | 2.091270 | 0.003168 | 0.002903 | 0.003087 |
| lambda.test | RUN | 0.653180 | 1.705795 | 2.156048 | 0.003094 | 0.003263 | 0.003343 |
| make_expr.test | RUN | 0.643627 | 2.757170 | 2.254803 | 0.003782 | 0.004020 | 0.004257 |
| matches.test | RUN | 0.579872 | 1.375740 | 1.822557 | 0.002906 | 0.002882 | 0.003156 |
| flatten.test | RUN | 1.079693 | 4.020484 | 4.442324 | 0.003162 | 0.003338 | 0.003360 |
| switch.test | RUN | 0.586269 | 1.397015 | 1.758938 | 0.003064 | 0.003072 | 0.003251 |
| toy_spirit.test | RUN | 0.640947 | 1.801847 | 2.184509 | 0.003132 | 0.003138 | 0.003279 |
| toy_spirit2.test | RUN | 0.693332 | 1.892741 | 2.356547 | 0.002901 | 0.003151 | 0.002880 |
| make.test | RUN | 0.542944 | 1.816213 | 1.726443 | 0.003743 | 0.003462 | 0.003511 |
| mem_ptr.test | RUN | 0.696534 | 2.947947 | 2.475869 | 0.003522 | 0.003926 | 0.003273 |
| mpl.test | RUN | 0.638698 | 1.540491 | 1.816855 | 0.002966 | 0.003236 | 0.002663 |
| noinvoke.test | RUN | 0.531664 | 1.258347 | 1.558882 | 0.003298 | 0.003398 | 0.003104 |
| pack_expansion.test | RUN | 0.676289 | 1.891323 | 1.928414 | 0.002437 | 0.003228 | 0.003378 |
| protect.test | RUN | 0.418932 | 1.262641 | 1.142850 | 0.003136 | 0.002581 | 0.002397 |
| bug2407.test | COMPILE | 0.514565 | 0.934266 | 1.044824 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.994× | 0.996×, 0.992× | 0.998× |
| clang | 1.063×† | 1.144×, 0.981× | 1.012× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001740s | 0.001825s | 0.001847s | byte equality observed |
| constrained_ops.test | 10 | 0.001733s | 0.001811s | 0.001845s | byte equality observed |
| cpp-next_bug.test | 9 | 0.001714s | 0.001765s | 0.001842s | byte equality observed |
| deduce_domain.test | 10 | 0.001717s | 0.001863s | 0.001871s | byte equality observed |
| deep_copy.test | 10 | 0.001731s | 0.001836s | 0.001847s | byte equality observed |
| display_expr.test | 9 | 0.001705s | 0.001783s | 0.001842s | byte equality observed |
| env_var.test | 9 | 0.002018s | 0.002170s | 0.002093s | byte equality observed |
| examples.test | 10 | 0.001739s | 0.001959s | 0.001818s | byte equality observed |
| external_transforms.test | 10 | 0.001762s | 0.001936s | 0.001869s | byte equality observed |
| flatten.test | 10 | 0.001785s | 0.001959s | 0.001916s | byte equality observed |
| lambda.test | 10 | 0.001736s | 0.001836s | 0.001830s | byte equality observed |
| make.test | 10 | 0.001895s | 0.002035s | 0.002020s | byte equality observed |
| make_expr.test | 9 | 0.002124s | 0.002414s | 0.002260s | byte equality observed |
| matches.test | 10 | 0.001713s | 0.001861s | 0.001843s | byte equality observed |
| mem_ptr.test | 9 | 0.001986s | 0.002137s | 0.002160s | byte equality observed |
| mpl.test | 8 | 0.001736s | 0.001701s | 0.001850s | byte equality observed |
| noinvoke.test | 10 | 0.001716s | 0.001848s | 0.001818s | byte equality observed |
| pack_expansion.test | 10 | 0.001776s | 0.001915s | 0.001893s | byte equality observed |
| protect.test | 10 | 0.001766s | 0.001870s | 0.001899s | byte equality observed |
| switch.test | 10 | 0.001767s | 0.001858s | 0.001831s | byte equality observed |
| toy_spirit.test | 10 | 0.001710s | 0.001841s | 0.001877s | byte equality observed |
| toy_spirit2.test | 10 | 0.001777s | 0.001886s | 0.001829s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
