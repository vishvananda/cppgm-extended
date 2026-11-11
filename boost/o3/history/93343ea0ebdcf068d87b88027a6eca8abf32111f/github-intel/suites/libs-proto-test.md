# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.774286 | 1.779004 | 2.016162 | 0.002913 | 0.002985 | 0.002714 |
| constrained_ops.test | RUN | 0.868045 | 1.888776 | 2.318923 | 0.002687 | 0.002846 | 0.002756 |
| cpp-next_bug.test | RUN | 0.923468 | 2.061626 | 2.345830 | 0.002859 | 0.002964 | 0.002701 |
| deep_copy.test | RUN | 0.749880 | 1.773450 | 1.983287 | 0.002815 | 0.002794 | 0.002662 |
| display_expr.test | RUN | 0.932099 | 2.214905 | 2.445894 | 0.002758 | 0.002796 | 0.002836 |
| deduce_domain.test | RUN | 0.746403 | 1.583320 | 1.825024 | 0.002669 | 0.002940 | 0.002761 |
| env_var.test | RUN | 0.985879 | 3.785655 | 2.906120 | 0.003363 | 0.003267 | 0.003154 |
| examples.test | RUN | 0.940671 | 2.447813 | 2.736822 | 0.002744 | 0.002760 | 0.002803 |
| external_transforms.test | RUN | 0.914433 | 2.116685 | 2.460008 | 0.002826 | 0.002810 | 0.002903 |
| lambda.test | RUN | 0.883570 | 2.136689 | 2.480892 | 0.002666 | 0.002789 | 0.002778 |
| make_expr.test | RUN | 0.879735 | 3.526726 | 2.627681 | 0.003291 | 0.003492 | 0.003339 |
| matches.test | RUN | 0.803440 | 1.757197 | 2.145178 | 0.002978 | 0.002861 | 0.002864 |
| flatten.test | RUN | 1.389766 | 4.995462 | 5.165590 | 0.002982 | 0.002907 | 0.002885 |
| switch.test | RUN | 0.776559 | 1.757031 | 2.018860 | 0.002941 | 0.002719 | 0.002808 |
| toy_spirit.test | RUN | 0.863008 | 2.234064 | 2.550139 | 0.002911 | 0.002997 | 0.002379 |
| toy_spirit2.test | RUN | 0.928394 | 2.347537 | 2.715531 | 0.002976 | 0.002945 | 0.002672 |
| make.test | RUN | 0.742009 | 2.240800 | 1.989895 | 0.003177 | 0.003120 | 0.003060 |
| mem_ptr.test | RUN | 0.973811 | 3.693606 | 2.968307 | 0.003067 | 0.003184 | 0.002988 |
| mpl.test | RUN | 0.869321 | 1.870964 | 2.212483 | 0.002888 | 0.003262 | 0.002729 |
| noinvoke.test | RUN | 0.719256 | 1.576949 | 1.815675 | 0.002681 | 0.002933 | 0.002592 |
| pack_expansion.test | RUN | 0.865950 | 2.236292 | 2.424354 | 0.002215 | 0.001972 | 0.002851 |
| protect.test | RUN | 0.600673 | 1.621111 | 1.210597 | 0.002170 | 0.001906 | 0.001980 |
| bug2407.test | COMPILE | 0.484470 | 1.090825 | 1.303102 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.974× | 0.984×, 0.964× | 0.991× |
| clang | 0.965× | 0.963×, 0.967× | 1.009× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.001617s | 0.001586s | 0.001559s | byte equality observed |
| constrained_ops.test | 10 | 0.001614s | 0.001525s | 0.001618s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001590s | 0.001592s | 0.001533s | byte equality observed |
| deduce_domain.test | 10 | 0.001635s | 0.001515s | 0.001554s | byte equality observed |
| deep_copy.test | 10 | 0.001606s | 0.001580s | 0.001531s | byte equality observed |
| display_expr.test | 10 | 0.001607s | 0.001568s | 0.001580s | byte equality observed |
| env_var.test | 10 | 0.001763s | 0.001701s | 0.001684s | byte equality observed |
| examples.test | 10 | 0.001612s | 0.001611s | 0.001583s | byte equality observed |
| external_transforms.test | 10 | 0.001597s | 0.001600s | 0.001540s | byte equality observed |
| flatten.test | 10 | 0.001637s | 0.001586s | 0.001564s | byte equality observed |
| lambda.test | 10 | 0.001617s | 0.001583s | 0.001576s | byte equality observed |
| make.test | 10 | 0.001665s | 0.001614s | 0.001649s | byte equality observed |
| make_expr.test | 10 | 0.001885s | 0.001827s | 0.001728s | byte equality observed |
| matches.test | 10 | 0.001588s | 0.001587s | 0.001558s | byte equality observed |
| mem_ptr.test | 10 | 0.001755s | 0.001741s | 0.001686s | byte equality observed |
| mpl.test | 10 | 0.001566s | 0.001592s | 0.001577s | byte equality observed |
| noinvoke.test | 10 | 0.001654s | 0.001612s | 0.001576s | byte equality observed |
| pack_expansion.test | 10 | 0.001596s | 0.001591s | 0.001613s | byte equality observed |
| protect.test | 8 | 0.001691s | 0.001578s | 0.001588s | byte equality observed |
| switch.test | 10 | 0.001616s | 0.001538s | 0.001538s | byte equality observed |
| toy_spirit.test | 10 | 0.001603s | 0.001611s | 0.001550s | byte equality observed |
| toy_spirit2.test | 10 | 0.001650s | 0.001527s | 0.001580s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
