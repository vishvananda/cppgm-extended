# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.408279 | 0.999707 | 1.077504 | 0.002675 | 0.002703 | 0.002456 |
| constrained_ops.test | RUN | 0.445242 | 1.094220 | 1.207696 | 0.002584 | 0.002661 | 0.002282 |
| cpp-next_bug.test | RUN | 0.477116 | 1.123085 | 1.325148 | 0.002415 | 0.002415 | 0.002398 |
| deep_copy.test | RUN | 0.390777 | 0.985152 | 1.059212 | 0.002379 | 0.002421 | 0.002280 |
| display_expr.test | RUN | 0.463519 | 1.222564 | 1.306857 | 0.002290 | 0.002377 | 0.002194 |
| deduce_domain.test | RUN | 0.372753 | 0.881762 | 0.993121 | 0.002468 | 0.002296 | 0.002394 |
| env_var.test | RUN | 0.495453 | 2.123293 | 1.588277 | 0.002828 | 0.002962 | 0.002984 |
| examples.test | RUN | 0.486704 | 1.363909 | 1.475923 | 0.002474 | 0.002414 | 0.002414 |
| external_transforms.test | RUN | 0.480693 | 1.186758 | 1.342569 | 0.002430 | 0.002387 | 0.002098 |
| lambda.test | RUN | 0.453715 | 1.185960 | 1.348588 | 0.002506 | 0.002562 | 0.002222 |
| make_expr.test | RUN | 0.452336 | 1.931940 | 1.436968 | 0.002952 | 0.003417 | 0.003236 |
| matches.test | RUN | 0.432631 | 0.983297 | 1.174024 | 0.002505 | 0.002544 | 0.002467 |
| flatten.test | RUN | 0.770877 | 2.858596 | 2.897574 | 0.002712 | 0.002810 | 0.002396 |
| switch.test | RUN | 0.401824 | 0.987366 | 1.087121 | 0.002460 | 0.002560 | 0.002249 |
| toy_spirit.test | RUN | 0.455034 | 1.259226 | 1.420893 | 0.002544 | 0.002610 | 0.002675 |
| toy_spirit2.test | RUN | 0.495564 | 1.326327 | 1.467158 | 0.002413 | 0.002628 | 0.002521 |
| make.test | RUN | 0.405469 | 1.243077 | 1.073835 | 0.002997 | 0.002887 | 0.002919 |
| mem_ptr.test | RUN | 0.512429 | 2.058491 | 1.594923 | 0.002859 | 0.003183 | 0.002938 |
| mpl.test | RUN | 0.465365 | 1.061393 | 1.204568 | 0.002118 | 0.002676 | 0.002265 |
| noinvoke.test | RUN | 0.398715 | 0.866420 | 0.991942 | 0.002210 | 0.002676 | 0.002464 |
| pack_expansion.test | RUN | 0.477456 | 1.288296 | 1.308725 | 0.002140 | 0.002462 | 0.002561 |
| protect.test | RUN | 0.281216 | 0.899442 | 0.717092 | 0.002008 | 0.001938 | 0.001874 |
| bug2407.test | COMPILE | 0.337598 | 0.645717 | 0.649681 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.967× | 0.974×, 0.959× | 1.005× |
| clang | 0.965× | 0.967×, 0.963× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001455s | 0.001412s | 0.001399s | byte equality observed |
| constrained_ops.test | 10 | 0.001407s | 0.001381s | 0.001366s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001425s | 0.001375s | 0.001386s | byte equality observed |
| deduce_domain.test | 10 | 0.001414s | 0.001382s | 0.001365s | byte equality observed |
| deep_copy.test | 10 | 0.001429s | 0.001392s | 0.001382s | byte equality observed |
| display_expr.test | 8 | 0.001452s | 0.001411s | 0.001393s | byte equality observed |
| env_var.test | 10 | 0.001664s | 0.001623s | 0.001626s | byte equality observed |
| examples.test | 10 | 0.001454s | 0.001408s | 0.001381s | byte equality observed |
| external_transforms.test | 10 | 0.001439s | 0.001428s | 0.001417s | byte equality observed |
| flatten.test | 10 | 0.001507s | 0.001446s | 0.001430s | byte equality observed |
| lambda.test | 10 | 0.001440s | 0.001375s | 0.001390s | byte equality observed |
| make.test | 10 | 0.001574s | 0.001544s | 0.001511s | byte equality observed |
| make_expr.test | 10 | 0.001760s | 0.001728s | 0.001713s | byte equality observed |
| matches.test | 10 | 0.001437s | 0.001392s | 0.001390s | byte equality observed |
| mem_ptr.test | 10 | 0.001661s | 0.001645s | 0.001609s | byte equality observed |
| mpl.test | 10 | 0.001439s | 0.001394s | 0.001404s | byte equality observed |
| noinvoke.test | 10 | 0.001479s | 0.001388s | 0.001385s | byte equality observed |
| pack_expansion.test | 10 | 0.001485s | 0.001455s | 0.001398s | byte equality observed |
| protect.test | 10 | 0.001501s | 0.001426s | 0.001409s | byte equality observed |
| switch.test | 10 | 0.001424s | 0.001414s | 0.001378s | byte equality observed |
| toy_spirit.test | 10 | 0.001420s | 0.001385s | 0.001379s | byte equality observed |
| toy_spirit2.test | 10 | 0.001426s | 0.001403s | 0.001379s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
