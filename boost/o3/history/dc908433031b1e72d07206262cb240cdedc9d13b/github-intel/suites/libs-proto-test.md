# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.536191 | 1.398594 | 1.501667 | 0.002470 | 0.002559 | 0.002050 |
| constrained_ops.test | RUN | 0.593010 | 1.506380 | 1.772532 | 0.002049 | 0.002280 | 0.002290 |
| cpp-next_bug.test | RUN | 0.611403 | 1.628107 | 1.771916 | 0.002305 | 0.002485 | 0.002171 |
| deep_copy.test | RUN | 0.511555 | 1.400624 | 1.476985 | 0.002204 | 0.002270 | 0.002100 |
| display_expr.test | RUN | 0.631812 | 1.756266 | 1.855754 | 0.002684 | 0.002239 | 0.002084 |
| deduce_domain.test | RUN | 0.499303 | 1.245547 | 1.383997 | 0.002380 | 0.002565 | 0.001930 |
| env_var.test | RUN | 0.679146 | 3.034141 | 2.251077 | 0.002643 | 0.002725 | 0.002486 |
| examples.test | RUN | 0.650969 | 1.950642 | 2.112193 | 0.002293 | 0.002263 | 0.002160 |
| external_transforms.test | RUN | 0.636273 | 1.681328 | 1.888509 | 0.002560 | 0.002182 | 0.002195 |
| lambda.test | RUN | 0.617889 | 1.696657 | 1.925182 | 0.002222 | 0.002478 | 0.002127 |
| make_expr.test | RUN | 0.612622 | 2.814746 | 2.013713 | 0.002631 | 0.002963 | 0.002697 |
| matches.test | RUN | 0.553097 | 1.397269 | 1.620127 | 0.002204 | 0.002535 | 0.002141 |
| flatten.test | RUN | 1.063182 | 3.991716 | 4.001519 | 0.002505 | 0.002598 | 0.002040 |
| switch.test | RUN | 0.539684 | 1.400351 | 1.538581 | 0.002440 | 0.002304 | 0.002023 |
| toy_spirit.test | RUN | 0.611967 | 1.771492 | 1.949961 | 0.002283 | 0.002490 | 0.002206 |
| toy_spirit2.test | RUN | 0.654624 | 1.875392 | 2.131352 | 0.002297 | 0.002470 | 0.002251 |
| make.test | RUN | 0.526458 | 1.779292 | 1.527867 | 0.002427 | 0.002591 | 0.002313 |
| mem_ptr.test | RUN | 0.697750 | 2.954703 | 2.233880 | 0.002606 | 0.002609 | 0.002523 |
| mpl.test | RUN | 0.608195 | 1.466720 | 1.673883 | 0.002269 | 0.002490 | 0.001950 |
| noinvoke.test | RUN | 0.514801 | 1.254499 | 1.380707 | 0.002302 | 0.002387 | 0.002215 |
| pack_expansion.test | RUN | 0.634596 | 1.853493 | 1.840537 | 0.001853 | 0.002556 | 0.002286 |
| protect.test | RUN | 0.481092 | 1.310453 | 0.995778 | 0.001738 | 0.001674 | 0.001588 |
| bug2407.test | COMPILE | 0.345626 | 0.785166 | 1.003101 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.985× | 0.994×, 0.975× | 1.038× |
| clang | 0.962× | 0.966×, 0.958× | 0.978× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001472s | 0.001421s | 0.001383s | byte equality observed |
| constrained_ops.test | 10 | 0.001422s | 0.001376s | 0.001358s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001391s | 0.001370s | 0.001346s | byte equality observed |
| deduce_domain.test | 10 | 0.001386s | 0.001353s | 0.001339s | byte equality observed |
| deep_copy.test | 10 | 0.001384s | 0.001352s | 0.001353s | byte equality observed |
| display_expr.test | 10 | 0.001389s | 0.001409s | 0.001379s | byte equality observed |
| env_var.test | 10 | 0.001557s | 0.001494s | 0.001515s | byte equality observed |
| examples.test | 10 | 0.001417s | 0.001384s | 0.001377s | byte equality observed |
| external_transforms.test | 10 | 0.001446s | 0.001448s | 0.001380s | byte equality observed |
| flatten.test | 10 | 0.001433s | 0.001413s | 0.001377s | byte equality observed |
| lambda.test | 10 | 0.001410s | 0.001387s | 0.001387s | byte equality observed |
| make.test | 10 | 0.001465s | 0.001474s | 0.001426s | byte equality observed |
| make_expr.test | 10 | 0.001601s | 0.001578s | 0.001525s | byte equality observed |
| matches.test | 10 | 0.001370s | 0.001435s | 0.001371s | byte equality observed |
| mem_ptr.test | 10 | 0.001546s | 0.001529s | 0.001506s | byte equality observed |
| mpl.test | 10 | 0.001416s | 0.001347s | 0.001364s | byte equality observed |
| noinvoke.test | 10 | 0.001404s | 0.001417s | 0.001363s | byte equality observed |
| pack_expansion.test | 10 | 0.001419s | 0.001403s | 0.001358s | byte equality observed |
| protect.test | 10 | 0.001432s | 0.001442s | 0.001324s | byte equality observed |
| switch.test | 10 | 0.001404s | 0.001394s | 0.001323s | byte equality observed |
| toy_spirit.test | 10 | 0.001389s | 0.001379s | 0.001327s | byte equality observed |
| toy_spirit2.test | 10 | 0.001404s | 0.001394s | 0.001335s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
