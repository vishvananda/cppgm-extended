# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.570636 | 1.394435 | 1.705699 | 0.003099 | 0.003167 | 0.003074 |
| constrained_ops.test | RUN | 0.630917 | 1.484671 | 1.912835 | 0.002860 | 0.003370 | 0.003022 |
| cpp-next_bug.test | RUN | 0.636438 | 1.577980 | 1.938540 | 0.002473 | 0.002983 | 0.002804 |
| deep_copy.test | RUN | 0.557025 | 1.401453 | 1.662496 | 0.002499 | 0.003186 | 0.002985 |
| display_expr.test | RUN | 0.642086 | 1.762163 | 2.012449 | 0.002909 | 0.003003 | 0.003120 |
| deduce_domain.test | RUN | 0.523284 | 1.193297 | 1.459457 | 0.002876 | 0.002867 | 0.003054 |
| env_var.test | RUN | 0.690518 | 2.966468 | 2.376678 | 0.003378 | 0.003453 | 0.003776 |
| examples.test | RUN | 0.667584 | 1.955160 | 2.260084 | 0.002920 | 0.003052 | 0.002458 |
| external_transforms.test | RUN | 0.659303 | 1.619957 | 2.050308 | 0.002806 | 0.002921 | 0.003018 |
| lambda.test | RUN | 0.647103 | 1.700881 | 2.014900 | 0.002702 | 0.003042 | 0.003061 |
| make_expr.test | RUN | 0.637118 | 2.725325 | 2.154197 | 0.003678 | 0.004195 | 0.003864 |
| matches.test | RUN | 0.582067 | 1.377004 | 1.813799 | 0.002970 | 0.003175 | 0.003021 |
| flatten.test | RUN | 1.067772 | 3.953947 | 4.341934 | 0.003304 | 0.003462 | 0.003099 |
| switch.test | RUN | 0.565149 | 1.413843 | 1.649877 | 0.002802 | 0.003113 | 0.002543 |
| toy_spirit.test | RUN | 0.624872 | 1.741378 | 2.140301 | 0.003042 | 0.003291 | 0.003103 |
| toy_spirit2.test | RUN | 0.674819 | 1.853457 | 2.272587 | 0.003057 | 0.003260 | 0.002742 |
| make.test | RUN | 0.557229 | 1.727320 | 1.689107 | 0.003554 | 0.003634 | 0.003585 |
| mem_ptr.test | RUN | 0.722514 | 2.866731 | 2.422860 | 0.003447 | 0.003625 | 0.003676 |
| mpl.test | RUN | 0.620180 | 1.463309 | 1.813433 | 0.002967 | 0.003289 | 0.003028 |
| noinvoke.test | RUN | 0.543064 | 1.225465 | 1.526710 | 0.003038 | 0.003259 | 0.003052 |
| pack_expansion.test | RUN | 0.564479 | 1.747063 | 2.008608 | 0.002465 | 0.002503 | 0.003292 |
| protect.test | RUN | 0.512375 | 1.294199 | 1.257346 | 0.002403 | 0.002517 | 0.002265 |
| bug2407.test | COMPILE | 0.470468 | 0.866425 | 0.932236 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.944×† | 0.911×, 0.977× | 0.997× |
| clang | 0.924×† | 0.890×, 0.957× | 0.935× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001741s | 0.001676s | 0.001680s | byte equality observed |
| constrained_ops.test | 10 | 0.001746s | 0.001622s | 0.001610s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001715s | 0.001608s | 0.001636s | byte equality observed |
| deduce_domain.test | 10 | 0.001692s | 0.001626s | 0.001629s | byte equality observed |
| deep_copy.test | 10 | 0.001682s | 0.001655s | 0.001618s | byte equality observed |
| display_expr.test | 10 | 0.001698s | 0.001675s | 0.001642s | byte equality observed |
| env_var.test | 8 | 0.002008s | 0.002024s | 0.001929s | byte equality observed |
| examples.test | 10 | 0.001784s | 0.001721s | 0.001668s | byte equality observed |
| external_transforms.test | 10 | 0.001746s | 0.001720s | 0.001714s | byte equality observed |
| flatten.test | 10 | 0.001827s | 0.001720s | 0.001700s | byte equality observed |
| lambda.test | 10 | 0.001759s | 0.001651s | 0.001703s | byte equality observed |
| make.test | 9 | 0.001882s | 0.001827s | 0.001793s | byte equality observed |
| make_expr.test | 7 | 0.002152s | 0.002057s | 0.002071s | byte equality observed |
| matches.test | 10 | 0.001710s | 0.001661s | 0.001660s | byte equality observed |
| mem_ptr.test | 8 | 0.002025s | 0.001986s | 0.001928s | byte equality observed |
| mpl.test | 10 | 0.001746s | 0.001622s | 0.001654s | byte equality observed |
| noinvoke.test | 10 | 0.001758s | 0.001631s | 0.001669s | byte equality observed |
| pack_expansion.test | 10 | 0.001815s | 0.001728s | 0.001718s | byte equality observed |
| protect.test | 10 | 0.001740s | 0.001690s | 0.001740s | byte equality observed |
| switch.test | 10 | 0.001741s | 0.001647s | 0.001675s | byte equality observed |
| toy_spirit.test | 10 | 0.001716s | 0.001677s | 0.001640s | byte equality observed |
| toy_spirit2.test | 10 | 0.001757s | 0.001689s | 0.001689s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
