# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.417373 | 0.992945 | 1.132749 | 0.002597 | 0.002702 | 0.002737 |
| constrained_ops.test | RUN | 0.497541 | 1.055070 | 1.215610 | 0.002272 | 0.002548 | 0.002708 |
| cpp-next_bug.test | RUN | 0.492400 | 1.149638 | 1.325836 | 0.002304 | 0.002572 | 0.002559 |
| deep_copy.test | RUN | 0.418466 | 1.037025 | 1.071679 | 0.002483 | 0.002680 | 0.002706 |
| display_expr.test | RUN | 0.485366 | 1.228822 | 1.347206 | 0.002545 | 0.002557 | 0.002428 |
| deduce_domain.test | RUN | 0.392451 | 0.875054 | 0.997063 | 0.002705 | 0.002654 | 0.002242 |
| env_var.test | RUN | 0.515712 | 2.154684 | 1.609144 | 0.002877 | 0.003042 | 0.003068 |
| examples.test | RUN | 0.496356 | 1.383106 | 1.521382 | 0.002569 | 0.002495 | 0.002393 |
| external_transforms.test | RUN | 0.499559 | 1.222967 | 1.371042 | 0.004030 | 0.002465 | 0.002425 |
| lambda.test | RUN | 0.481880 | 1.209197 | 1.328569 | 0.002622 | 0.002599 | 0.002448 |
| make_expr.test | RUN | 0.474913 | 1.955171 | 1.445127 | 0.006134 | 0.003416 | 0.003316 |
| matches.test | RUN | 0.444751 | 0.989199 | 1.162345 | 0.002416 | 0.002556 | 0.002623 |
| flatten.test | RUN | 0.848733 | 2.854873 | 2.880538 | 0.002861 | 0.002757 | 0.002541 |
| switch.test | RUN | 0.467308 | 0.980653 | 1.105931 | 0.002723 | 0.002638 | 0.002443 |
| toy_spirit.test | RUN | 0.474302 | 1.250101 | 1.392510 | 0.002822 | 0.002693 | 0.002693 |
| toy_spirit2.test | RUN | 0.530280 | 1.338161 | 1.509724 | 0.002401 | 0.002547 | 0.002218 |
| make.test | RUN | 0.421117 | 1.296666 | 1.103220 | 0.003136 | 0.002918 | 0.002912 |
| mem_ptr.test | RUN | 0.545807 | 2.086906 | 1.589168 | 0.002919 | 0.003193 | 0.003077 |
| mpl.test | RUN | 0.468778 | 1.064903 | 1.190372 | 0.002630 | 0.002631 | 0.002537 |
| noinvoke.test | RUN | 0.399718 | 0.870171 | 1.012218 | 0.002309 | 0.002967 | 0.002548 |
| pack_expansion.test | RUN | 0.479981 | 1.261844 | 1.314490 | 0.002555 | 0.002193 | 0.002628 |
| protect.test | RUN | 0.301796 | 0.910418 | 0.749426 | 0.002024 | 0.002028 | 0.001944 |
| bug2407.test | COMPILE | 0.361668 | 0.671588 | 0.653706 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.982× | 0.973×, 0.991× | 0.997× |
| clang | 0.966× | 0.967×, 0.966× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001442s | 0.001444s | 0.001391s | byte equality observed |
| constrained_ops.test | 10 | 0.001435s | 0.001415s | 0.001385s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001441s | 0.001430s | 0.001383s | byte equality observed |
| deduce_domain.test | 10 | 0.001422s | 0.001390s | 0.001380s | byte equality observed |
| deep_copy.test | 10 | 0.001431s | 0.001411s | 0.001392s | byte equality observed |
| display_expr.test | 10 | 0.001444s | 0.001422s | 0.001391s | byte equality observed |
| env_var.test | 10 | 0.001694s | 0.001684s | 0.001657s | byte equality observed |
| examples.test | 10 | 0.001489s | 0.001440s | 0.001412s | byte equality observed |
| external_transforms.test | 10 | 0.001466s | 0.001489s | 0.001435s | byte equality observed |
| flatten.test | 10 | 0.001488s | 0.001464s | 0.001474s | byte equality observed |
| lambda.test | 10 | 0.001457s | 0.001448s | 0.001404s | byte equality observed |
| make.test | 10 | 0.001573s | 0.001563s | 0.001553s | byte equality observed |
| make_expr.test | 10 | 0.001757s | 0.001741s | 0.001734s | byte equality observed |
| matches.test | 10 | 0.001445s | 0.001430s | 0.001416s | byte equality observed |
| mem_ptr.test | 10 | 0.001694s | 0.001698s | 0.001611s | byte equality observed |
| mpl.test | 10 | 0.001451s | 0.001414s | 0.001406s | byte equality observed |
| noinvoke.test | 10 | 0.001459s | 0.001414s | 0.001402s | byte equality observed |
| pack_expansion.test | 5 | 0.001526s | 0.001459s | 0.001418s | byte equality observed |
| protect.test | 9 | 0.001494s | 0.001450s | 0.001473s | byte equality observed |
| switch.test | 10 | 0.001460s | 0.001401s | 0.001424s | byte equality observed |
| toy_spirit.test | 10 | 0.001468s | 0.001418s | 0.001404s | byte equality observed |
| toy_spirit2.test | 10 | 0.001481s | 0.001422s | 0.001397s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
