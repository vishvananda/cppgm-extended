# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.885300 | 2.018532 | 2.674882 | 0.003958 | 0.003978 | 0.003864 |
| constrained_ops.test | RUN | 0.992245 | 2.168758 | 2.960781 | 0.003751 | 0.003853 | 0.003954 |
| cpp-next_bug.test | RUN | 1.039556 | 2.255277 | 2.992157 | 0.003450 | 0.003730 | 0.003951 |
| deep_copy.test | RUN | 0.846022 | 1.997919 | 2.605807 | 0.003746 | 0.004152 | 0.003672 |
| display_expr.test | RUN | 1.040473 | 2.469389 | 3.146658 | 0.003863 | 0.003519 | 0.003903 |
| deduce_domain.test | RUN | 0.830735 | 1.702224 | 2.348625 | 0.003814 | 0.003040 | 0.003857 |
| env_var.test | RUN | 1.083299 | 4.147456 | 3.677270 | 0.004632 | 0.004538 | 0.004773 |
| examples.test | RUN | 1.046936 | 2.837966 | 3.482057 | 0.003773 | 0.003738 | 0.003567 |
| external_transforms.test | RUN | 1.030294 | 2.324926 | 3.190292 | 0.004127 | 0.003822 | 0.004031 |
| lambda.test | RUN | 1.004289 | 2.319973 | 3.227202 | 0.003517 | 0.003738 | 0.004071 |
| make_expr.test | RUN | 0.986944 | 3.931746 | 3.336107 | 0.004779 | 0.005106 | 0.005476 |
| matches.test | RUN | 0.910989 | 1.931982 | 2.768776 | 0.003789 | 0.003474 | 0.003722 |
| flatten.test | RUN | 1.573917 | 5.499144 | 6.740525 | 0.003831 | 0.003891 | 0.004004 |
| switch.test | RUN | 0.898998 | 1.939269 | 2.651058 | 0.003957 | 0.003811 | 0.003936 |
| toy_spirit.test | RUN | 0.978324 | 2.468620 | 3.322665 | 0.003809 | 0.003855 | 0.003958 |
| toy_spirit2.test | RUN | 1.075964 | 2.609206 | 3.627192 | 0.003817 | 0.003516 | 0.003409 |
| make.test | RUN | 0.854406 | 2.473072 | 2.581066 | 0.005145 | 0.004731 | 0.004201 |
| mem_ptr.test | RUN | 1.108275 | 3.985259 | 3.701261 | 0.004094 | 0.004607 | 0.004687 |
| mpl.test | RUN | 1.013246 | 2.074894 | 2.882577 | 0.003703 | 0.003749 | 0.003880 |
| noinvoke.test | RUN | 0.855724 | 1.770993 | 2.378016 | 0.003506 | 0.003690 | 0.003650 |
| pack_expansion.test | RUN | 1.038200 | 2.436792 | 3.025146 | 0.004027 | 0.002953 | 0.003818 |
| protect.test | RUN | 0.569419 | 1.894991 | 1.714884 | 0.003083 | 0.002932 | 0.002787 |
| bug2407.test | COMPILE | 0.735704 | 1.193929 | 1.399033 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.976× | 0.971×, 0.981× | 0.991× |
| clang | 0.959× | 0.954×, 0.965× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.002133s | 0.002110s | 0.002078s | byte equality observed |
| constrained_ops.test | 9 | 0.002141s | 0.002051s | 0.002109s | byte equality observed |
| cpp-next_bug.test | 9 | 0.002124s | 0.002078s | 0.002071s | byte equality observed |
| deduce_domain.test | 9 | 0.002138s | 0.002163s | 0.002085s | byte equality observed |
| deep_copy.test | 7 | 0.002200s | 0.002103s | 0.002039s | byte equality observed |
| display_expr.test | 9 | 0.002140s | 0.002087s | 0.002047s | byte equality observed |
| env_var.test | 8 | 0.002472s | 0.002455s | 0.002416s | byte equality observed |
| examples.test | 9 | 0.002168s | 0.002102s | 0.002031s | byte equality observed |
| external_transforms.test | 8 | 0.002180s | 0.002130s | 0.002082s | byte equality observed |
| flatten.test | 8 | 0.002219s | 0.002230s | 0.002129s | byte equality observed |
| lambda.test | 9 | 0.002183s | 0.002035s | 0.002045s | byte equality observed |
| make.test | 6 | 0.002333s | 0.002359s | 0.002234s | byte equality observed |
| make_expr.test | 8 | 0.002593s | 0.002518s | 0.002480s | byte equality observed |
| matches.test | 9 | 0.002158s | 0.002067s | 0.002051s | byte equality observed |
| mem_ptr.test | 8 | 0.002450s | 0.002436s | 0.002420s | byte equality observed |
| mpl.test | 9 | 0.002181s | 0.002097s | 0.002089s | byte equality observed |
| noinvoke.test | 9 | 0.002104s | 0.002054s | 0.002079s | byte equality observed |
| pack_expansion.test | 9 | 0.002277s | 0.002145s | 0.002130s | byte equality observed |
| protect.test | 9 | 0.002244s | 0.002193s | 0.002110s | byte equality observed |
| switch.test | 9 | 0.002175s | 0.002082s | 0.002078s | byte equality observed |
| toy_spirit.test | 9 | 0.002182s | 0.002131s | 0.002065s | byte equality observed |
| toy_spirit2.test | 9 | 0.002168s | 0.002201s | 0.002028s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
