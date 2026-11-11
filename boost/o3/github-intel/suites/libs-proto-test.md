# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.732921 | 1.685322 | 1.842853 | 0.003011 | 0.003002 | 0.002859 |
| constrained_ops.test | RUN | 0.841407 | 1.808219 | 2.167392 | 0.003005 | 0.002950 | 0.002816 |
| cpp-next_bug.test | RUN | 0.843931 | 1.935058 | 2.150472 | 0.002981 | 0.002680 | 0.002883 |
| deep_copy.test | RUN | 0.702818 | 1.739332 | 1.835952 | 0.002710 | 0.002674 | 0.002956 |
| display_expr.test | RUN | 0.863632 | 2.069194 | 2.262145 | 0.003047 | 0.002609 | 0.002875 |
| deduce_domain.test | RUN | 0.678365 | 1.490808 | 1.674231 | 0.003126 | 0.002852 | 0.002873 |
| env_var.test | RUN | 0.929180 | 3.593879 | 2.699585 | 0.003441 | 0.003235 | 0.003336 |
| examples.test | RUN | 0.869941 | 2.308177 | 2.540781 | 0.002786 | 0.002599 | 0.002656 |
| external_transforms.test | RUN | 0.859266 | 1.999171 | 2.291027 | 0.002752 | 0.002549 | 0.003019 |
| lambda.test | RUN | 0.822231 | 2.004586 | 2.295366 | 0.003036 | 0.002908 | 0.002568 |
| make_expr.test | RUN | 0.830094 | 3.313652 | 2.460348 | 0.003376 | 0.003693 | 0.003381 |
| matches.test | RUN | 0.742698 | 1.650998 | 1.996151 | 0.003135 | 0.003068 | 0.002809 |
| flatten.test | RUN | 1.367486 | 4.732164 | 5.007175 | 0.003235 | 0.003131 | 0.003021 |
| switch.test | RUN | 0.736224 | 1.678306 | 1.926011 | 0.003108 | 0.002882 | 0.002383 |
| toy_spirit.test | RUN | 0.840262 | 2.113741 | 2.424911 | 0.002904 | 0.003082 | 0.002896 |
| toy_spirit2.test | RUN | 0.889478 | 2.228091 | 2.625007 | 0.002898 | 0.002876 | 0.002838 |
| make.test | RUN | 0.729227 | 2.125568 | 1.858159 | 0.003279 | 0.003035 | 0.003055 |
| mem_ptr.test | RUN | 0.934697 | 3.479005 | 2.771172 | 0.003285 | 0.003490 | 0.003380 |
| mpl.test | RUN | 0.829383 | 1.789699 | 2.051735 | 0.002953 | 0.002928 | 0.002915 |
| noinvoke.test | RUN | 0.706100 | 1.445914 | 1.708692 | 0.003024 | 0.003023 | 0.002829 |
| pack_expansion.test | RUN | 0.743754 | 2.198117 | 2.284263 | 0.002524 | 0.003251 | 0.003030 |
| protect.test | RUN | 0.637854 | 1.477555 | 1.329902 | 0.002246 | 0.002156 | 0.002078 |
| bug2407.test | COMPILE | 0.612461 | 1.071461 | 1.289146 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.984× | 0.978×, 0.989× | 1.000× |
| clang | 0.966× | 0.961×, 0.970× | 0.982× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 9 | 0.001716s | 0.001719s | 0.001613s | byte equality observed |
| constrained_ops.test | 10 | 0.001676s | 0.001617s | 0.001615s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001641s | 0.001655s | 0.001581s | byte equality observed |
| deduce_domain.test | 10 | 0.001648s | 0.001614s | 0.001676s | byte equality observed |
| deep_copy.test | 10 | 0.001648s | 0.001617s | 0.001577s | byte equality observed |
| display_expr.test | 10 | 0.001666s | 0.001616s | 0.001602s | byte equality observed |
| env_var.test | 10 | 0.001834s | 0.001835s | 0.001809s | byte equality observed |
| examples.test | 10 | 0.001737s | 0.001640s | 0.001598s | byte equality observed |
| external_transforms.test | 10 | 0.001736s | 0.001669s | 0.001611s | byte equality observed |
| flatten.test | 10 | 0.001687s | 0.001711s | 0.001640s | byte equality observed |
| lambda.test | 10 | 0.001631s | 0.001678s | 0.001597s | byte equality observed |
| make.test | 9 | 0.001781s | 0.001772s | 0.001728s | byte equality observed |
| make_expr.test | 10 | 0.001933s | 0.001917s | 0.001873s | byte equality observed |
| matches.test | 10 | 0.001673s | 0.001597s | 0.001573s | byte equality observed |
| mem_ptr.test | 10 | 0.001886s | 0.001846s | 0.001802s | byte equality observed |
| mpl.test | 10 | 0.001635s | 0.001594s | 0.001637s | byte equality observed |
| noinvoke.test | 10 | 0.001654s | 0.001658s | 0.001619s | byte equality observed |
| pack_expansion.test | 10 | 0.001738s | 0.001700s | 0.001667s | byte equality observed |
| protect.test | 10 | 0.001672s | 0.001731s | 0.001665s | byte equality observed |
| switch.test | 10 | 0.001645s | 0.001642s | 0.001618s | byte equality observed |
| toy_spirit.test | 10 | 0.001682s | 0.001619s | 0.001623s | byte equality observed |
| toy_spirit2.test | 10 | 0.001692s | 0.001659s | 0.001671s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
