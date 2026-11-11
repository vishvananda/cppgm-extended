# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.461901 | 0.991664 | 1.161779 | 0.002478 | 0.002712 | 0.002760 |
| constrained_ops.test | RUN | 0.518175 | 1.050969 | 1.313172 | 0.002707 | 0.002562 | 0.002499 |
| cpp-next_bug.test | RUN | 0.548006 | 1.131808 | 1.364596 | 0.002417 | 0.002651 | 0.002805 |
| deep_copy.test | RUN | 0.452999 | 1.023009 | 1.246156 | 0.002601 | 0.002507 | 0.002415 |
| display_expr.test | RUN | 0.535442 | 1.228257 | 1.421598 | 0.002698 | 0.002636 | 0.002397 |
| deduce_domain.test | RUN | 0.446892 | 0.869405 | 1.041313 | 0.002333 | 0.002614 | 0.002682 |
| env_var.test | RUN | 0.580290 | 2.142779 | 1.648031 | 0.003044 | 0.003119 | 0.003255 |
| examples.test | RUN | 0.571264 | 1.348233 | 1.574356 | 0.002364 | 0.002387 | 0.002649 |
| external_transforms.test | RUN | 0.543691 | 1.221179 | 1.398856 | 0.002909 | 0.002500 | 0.002372 |
| lambda.test | RUN | 0.537866 | 1.194832 | 1.439293 | 0.002612 | 0.002687 | 0.002312 |
| make_expr.test | RUN | 0.541841 | 1.949936 | 1.502463 | 0.003135 | 0.003461 | 0.003417 |
| matches.test | RUN | 0.495907 | 0.980524 | 1.241311 | 0.002621 | 0.002575 | 0.002604 |
| flatten.test | RUN | 0.838689 | 2.833070 | 3.091058 | 0.002555 | 0.002900 | 0.002484 |
| switch.test | RUN | 0.469370 | 0.986747 | 1.171020 | 0.002784 | 0.002621 | 0.002786 |
| toy_spirit.test | RUN | 0.523076 | 1.245838 | 1.523566 | 0.002741 | 0.002690 | 0.002766 |
| toy_spirit2.test | RUN | 0.571284 | 1.337446 | 1.630063 | 0.002340 | 0.002670 | 0.002589 |
| make.test | RUN | 0.442584 | 1.249116 | 1.154677 | 0.003016 | 0.003048 | 0.003159 |
| mem_ptr.test | RUN | 0.576469 | 2.137574 | 1.693548 | 0.002910 | 0.003333 | 0.005242 |
| mpl.test | RUN | 0.517148 | 1.082804 | 1.370557 | 0.002376 | 0.002925 | 0.004085 |
| noinvoke.test | RUN | 0.439259 | 0.916402 | 1.194314 | 0.002417 | 0.002870 | 0.002806 |
| pack_expansion.test | RUN | 0.470369 | 1.389547 | 1.493638 | 0.002149 | 0.002993 | 0.002159 |
| protect.test | RUN | 0.394297 | 0.994340 | 1.063962 | 0.002045 | 0.002200 | 0.002098 |
| bug2407.test | COMPILE | 0.380630 | 0.593935 | 0.750120 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.075×† | 1.102×, 1.047× | 0.949× |
| clang | 0.992× | 0.987×, 0.997× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001631s | 0.002076s | 0.001704s | byte equality observed |
| constrained_ops.test | 10 | 0.001603s | 0.002070s | 0.001730s | byte equality observed |
| cpp-next_bug.test | 9 | 0.001641s | 0.001583s | 0.001745s | byte equality observed |
| deduce_domain.test | 10 | 0.001611s | 0.001584s | 0.001640s | byte equality observed |
| deep_copy.test | 10 | 0.001629s | 0.001581s | 0.001694s | byte equality observed |
| display_expr.test | 10 | 0.001656s | 0.001566s | 0.001651s | byte equality observed |
| env_var.test | 10 | 0.001872s | 0.001818s | 0.001937s | byte equality observed |
| examples.test | 10 | 0.001641s | 0.001573s | 0.001703s | byte equality observed |
| external_transforms.test | 10 | 0.001631s | 0.001596s | 0.001683s | byte equality observed |
| flatten.test | 10 | 0.001687s | 0.001600s | 0.001748s | byte equality observed |
| lambda.test | 10 | 0.001608s | 0.001598s | 0.001680s | byte equality observed |
| make.test | 10 | 0.001790s | 0.001764s | 0.001795s | byte equality observed |
| make_expr.test | 10 | 0.002001s | 0.001984s | 0.002007s | byte equality observed |
| matches.test | 10 | 0.001650s | 0.001612s | 0.001633s | byte equality observed |
| mem_ptr.test | 10 | 0.001924s | 0.001906s | 0.001915s | byte equality observed |
| mpl.test | 10 | 0.001640s | 0.001650s | 0.001651s | byte equality observed |
| noinvoke.test | 10 | 0.001640s | 0.001604s | 0.001696s | byte equality observed |
| pack_expansion.test | 10 | 0.001676s | 0.001667s | 0.001674s | byte equality observed |
| protect.test | 10 | 0.001660s | 0.001624s | 0.001690s | byte equality observed |
| switch.test | 10 | 0.001619s | 0.001575s | 0.001646s | byte equality observed |
| toy_spirit.test | 10 | 0.001606s | 0.001557s | 0.001656s | byte equality observed |
| toy_spirit2.test | 10 | 0.001622s | 0.001552s | 0.001647s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
