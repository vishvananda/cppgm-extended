# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.763588 | 1.720991 | 2.003526 | 0.003375 | 0.002559 | 0.003049 |
| constrained_ops.test | RUN | 0.861872 | 1.937143 | 2.393020 | 0.003203 | 0.003272 | 0.002919 |
| cpp-next_bug.test | RUN | 0.852137 | 2.057632 | 2.422147 | 0.002790 | 0.002574 | 0.002606 |
| deep_copy.test | RUN | 0.708821 | 1.754324 | 2.078825 | 0.003017 | 0.003182 | 0.002446 |
| display_expr.test | RUN | 0.852771 | 2.311205 | 2.572659 | 0.003249 | 0.002649 | 0.002592 |
| deduce_domain.test | RUN | 0.826495 | 1.551491 | 1.936979 | 0.003892 | 0.002762 | 0.003175 |
| env_var.test | RUN | 0.941981 | 3.746923 | 2.962671 | 0.003738 | 0.003313 | 0.003342 |
| examples.test | RUN | 0.867176 | 2.490940 | 2.988577 | 0.003401 | 0.003031 | 0.002757 |
| external_transforms.test | RUN | 0.895046 | 2.095602 | 2.546845 | 0.002794 | 0.002789 | 0.002721 |
| lambda.test | RUN | 0.888685 | 2.197952 | 2.590109 | 0.003379 | 0.002958 | 0.002785 |
| make_expr.test | RUN | 0.831479 | 3.467122 | 2.712731 | 0.003656 | 0.003225 | 0.003564 |
| matches.test | RUN | 0.749246 | 1.769578 | 2.344076 | 0.003633 | 0.003834 | 0.002580 |
| flatten.test | RUN | 1.438107 | 5.173461 | 5.632997 | 0.003302 | 0.002870 | 0.004389 |
| switch.test | RUN | 0.738110 | 1.766782 | 2.122774 | 0.003383 | 0.002661 | 0.003836 |
| toy_spirit.test | RUN | 0.867007 | 2.324761 | 2.708971 | 0.003156 | 0.002702 | 0.003697 |
| toy_spirit2.test | RUN | 0.948234 | 2.494442 | 2.897794 | 0.003351 | 0.002706 | 0.002579 |
| make.test | RUN | 0.751588 | 2.237592 | 2.050451 | 0.003671 | 0.002792 | 0.003300 |
| mem_ptr.test | RUN | 0.947179 | 3.675416 | 3.020564 | 0.004085 | 0.003411 | 0.002982 |
| mpl.test | RUN | 0.830632 | 1.879855 | 2.267626 | 0.003248 | 0.003183 | 0.002514 |
| noinvoke.test | RUN | 0.702767 | 1.671069 | 1.904098 | 0.003829 | 0.002597 | 0.003023 |
| pack_expansion.test | RUN | 0.864225 | 2.360822 | 2.593278 | 0.003435 | 0.002992 | 0.004288 |
| protect.test | RUN | 0.739376 | 1.722460 | 2.162304 | 0.004540 | 0.002690 | 0.003011 |
| bug2407.test | COMPILE | 0.670046 | 1.231986 | 1.692898 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.967× | 0.971×, 0.964× | 0.990× |
| clang | 0.933× | 0.914×, 0.952× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 5 | 0.002738s | 0.002582s | 0.002572s | byte equality observed |
| constrained_ops.test | 6 | 0.002672s | 0.002575s | 0.002499s | byte equality observed |
| cpp-next_bug.test | 5 | 0.002710s | 0.002758s | 0.002479s | byte equality observed |
| deduce_domain.test | 6 | 0.002674s | 0.002604s | 0.002500s | byte equality observed |
| deep_copy.test | 7 | 0.003022s | 0.002617s | 0.002763s | byte equality observed |
| display_expr.test | 5 | 0.002688s | 0.002634s | 0.002510s | byte equality observed |
| env_var.test | 6 | 0.003175s | 0.003013s | 0.003204s | byte equality observed |
| examples.test | 5 | 0.002685s | 0.002613s | 0.002516s | byte equality observed |
| external_transforms.test | 5 | 0.003080s | 0.002596s | 0.002696s | byte equality observed |
| flatten.test | 6 | 0.002828s | 0.002684s | 0.002604s | byte equality observed |
| lambda.test | 5 | 0.002648s | 0.002720s | 0.002795s | byte equality observed |
| make.test | 5 | 0.002853s | 0.002810s | 0.002942s | byte equality observed |
| make_expr.test | 5 | 0.003296s | 0.003084s | 0.003052s | byte equality observed |
| matches.test | 6 | 0.002638s | 0.002634s | 0.002733s | byte equality observed |
| mem_ptr.test | 4 | 0.003026s | 0.003004s | 0.002839s | byte equality observed |
| mpl.test | 5 | 0.002774s | 0.002529s | 0.002521s | byte equality observed |
| noinvoke.test | 5 | 0.002749s | 0.002629s | 0.002578s | byte equality observed |
| pack_expansion.test | 5 | 0.002731s | 0.002678s | 0.002616s | byte equality observed |
| protect.test | 5 | 0.002704s | 0.002816s | 0.002636s | byte equality observed |
| switch.test | 5 | 0.002683s | 0.002590s | 0.002494s | byte equality observed |
| toy_spirit.test | 5 | 0.002701s | 0.002645s | 0.002530s | byte equality observed |
| toy_spirit2.test | 5 | 0.002646s | 0.002600s | 0.002467s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
