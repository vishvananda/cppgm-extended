# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.637197 | 1.380651 | 1.680702 | 0.003315 | 0.003217 | 0.002980 |
| constrained_ops.test | RUN | 0.723682 | 1.443865 | 1.915006 | 0.002938 | 0.003275 | 0.003074 |
| cpp-next_bug.test | RUN | 0.733394 | 1.630807 | 1.942843 | 0.002633 | 0.003137 | 0.002845 |
| deep_copy.test | RUN | 0.612275 | 1.383298 | 1.648209 | 0.002633 | 0.002330 | 0.002859 |
| display_expr.test | RUN | 0.745501 | 1.699628 | 2.020558 | 0.003115 | 0.002951 | 0.003120 |
| deduce_domain.test | RUN | 0.589977 | 1.226302 | 1.444876 | 0.002817 | 0.002834 | 0.002568 |
| env_var.test | RUN | 0.800879 | 2.927663 | 2.389908 | 0.003539 | 0.003779 | 0.003738 |
| examples.test | RUN | 0.752242 | 1.911078 | 2.236756 | 0.003110 | 0.002964 | 0.003129 |
| external_transforms.test | RUN | 0.749452 | 1.610499 | 2.051964 | 0.003300 | 0.003027 | 0.002692 |
| lambda.test | RUN | 0.729740 | 1.666038 | 2.053862 | 0.003185 | 0.003030 | 0.003011 |
| make_expr.test | RUN | 0.722205 | 2.674107 | 2.190242 | 0.003386 | 0.004276 | 0.004151 |
| matches.test | RUN | 0.667704 | 1.365967 | 1.753265 | 0.003053 | 0.002932 | 0.002635 |
| flatten.test | RUN | 1.135104 | 3.870766 | 4.310317 | 0.003146 | 0.003405 | 0.003226 |
| switch.test | RUN | 0.632027 | 1.384244 | 1.662948 | 0.003093 | 0.003238 | 0.003145 |
| toy_spirit.test | RUN | 0.714461 | 1.763982 | 2.171751 | 0.002877 | 0.003126 | 0.002816 |
| toy_spirit2.test | RUN | 0.745435 | 1.790554 | 2.203943 | 0.002732 | 0.003285 | 0.003140 |
| make.test | RUN | 0.609005 | 1.704536 | 1.658668 | 0.003566 | 0.003663 | 0.003490 |
| mem_ptr.test | RUN | 0.802389 | 2.868425 | 2.438397 | 0.003522 | 0.003886 | 0.003046 |
| mpl.test | RUN | 0.704843 | 1.407753 | 1.819318 | 0.002556 | 0.003331 | 0.002965 |
| noinvoke.test | RUN | 0.595097 | 1.246273 | 1.494996 | 0.002865 | 0.003062 | 0.002882 |
| pack_expansion.test | RUN | 0.639608 | 1.724391 | 1.857516 | 0.002481 | 0.002516 | 0.002305 |
| protect.test | RUN | 0.537397 | 1.307531 | 1.136056 | 0.002393 | 0.002529 | 0.002219 |
| bug2407.test | COMPILE | 0.521011 | 0.932364 | 1.015523 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.051×† | 1.018×, 1.083× | 1.058× |
| clang | 0.954×† | 0.904×, 1.004× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001776s | 0.001858s | 0.001703s | byte equality observed |
| constrained_ops.test | 10 | 0.001819s | 0.001936s | 0.001716s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001794s | 0.001888s | 0.001736s | byte equality observed |
| deduce_domain.test | 10 | 0.001745s | 0.001887s | 0.001685s | byte equality observed |
| deep_copy.test | 10 | 0.001744s | 0.001858s | 0.001676s | byte equality observed |
| display_expr.test | 10 | 0.001752s | 0.001876s | 0.001754s | byte equality observed |
| env_var.test | 9 | 0.002071s | 0.002187s | 0.001989s | byte equality observed |
| examples.test | 10 | 0.001750s | 0.001826s | 0.001743s | byte equality observed |
| external_transforms.test | 10 | 0.001803s | 0.001936s | 0.001748s | byte equality observed |
| flatten.test | 10 | 0.001817s | 0.002006s | 0.001749s | byte equality observed |
| lambda.test | 10 | 0.001750s | 0.001829s | 0.001750s | byte equality observed |
| make.test | 10 | 0.001939s | 0.002041s | 0.001872s | byte equality observed |
| make_expr.test | 7 | 0.002173s | 0.002315s | 0.002137s | byte equality observed |
| matches.test | 10 | 0.001773s | 0.001830s | 0.001707s | byte equality observed |
| mem_ptr.test | 9 | 0.002061s | 0.002198s | 0.002034s | byte equality observed |
| mpl.test | 9 | 0.001767s | 0.001895s | 0.001711s | byte equality observed |
| noinvoke.test | 10 | 0.001868s | 0.001878s | 0.001721s | byte equality observed |
| pack_expansion.test | 9 | 0.001859s | 0.001979s | 0.001758s | byte equality observed |
| protect.test | 10 | 0.001801s | 0.001967s | 0.001782s | byte equality observed |
| switch.test | 10 | 0.001748s | 0.001858s | 0.001739s | byte equality observed |
| toy_spirit.test | 10 | 0.001723s | 0.001831s | 0.001766s | byte equality observed |
| toy_spirit2.test | 9 | 0.001805s | 0.001875s | 0.001741s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
