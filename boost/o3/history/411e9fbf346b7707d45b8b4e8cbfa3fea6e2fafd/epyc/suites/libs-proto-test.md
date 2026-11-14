# libs/proto/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| calculator.test | RUN | 0.468524 | 1.054558 | 1.226108 | 0.002727 | 0.002772 | 0.002706 |
| constrained_ops.test | RUN | 0.519783 | 1.115606 | 1.415773 | 0.002960 | 0.002822 | 0.002672 |
| cpp-next_bug.test | RUN | 0.558503 | 1.193326 | 1.408486 | 0.002860 | 0.002694 | 0.002828 |
| deep_copy.test | RUN | 0.458066 | 1.088605 | 1.194798 | 0.002777 | 0.002431 | 0.002965 |
| display_expr.test | RUN | 0.537828 | 1.279268 | 1.505565 | 0.002663 | 0.002764 | 0.002584 |
| deduce_domain.test | RUN | 0.438675 | 0.902725 | 1.131611 | 0.002947 | 0.002691 | 0.002657 |
| env_var.test | RUN | 0.576408 | 2.225783 | 1.828642 | 0.003399 | 0.003245 | 0.003657 |
| examples.test | RUN | 0.567481 | 1.440623 | 1.732000 | 0.002676 | 0.002529 | 0.002871 |
| external_transforms.test | RUN | 0.540642 | 1.286973 | 1.539037 | 0.002791 | 0.002689 | 0.002789 |
| lambda.test | RUN | 0.535391 | 1.271761 | 1.566771 | 0.002640 | 0.002635 | 0.002961 |
| make_expr.test | RUN | 0.529945 | 2.061707 | 1.668958 | 0.003371 | 0.003593 | 0.003703 |
| matches.test | RUN | 0.486498 | 1.052074 | 1.371681 | 0.002819 | 0.002772 | 0.002935 |
| flatten.test | RUN | 0.888385 | 3.038529 | 3.368615 | 0.003126 | 0.003309 | 0.003273 |
| switch.test | RUN | 0.469629 | 1.053934 | 1.282061 | 0.002987 | 0.002769 | 0.002859 |
| toy_spirit.test | RUN | 0.522631 | 1.328918 | 1.656416 | 0.002741 | 0.002904 | 0.002900 |
| toy_spirit2.test | RUN | 0.568336 | 1.430322 | 1.744636 | 0.002653 | 0.002750 | 0.003019 |
| make.test | RUN | 0.460754 | 1.347469 | 1.245593 | 0.003413 | 0.003230 | 0.003535 |
| mem_ptr.test | RUN | 0.605950 | 2.177483 | 1.865837 | 0.003092 | 0.003411 | 0.003620 |
| mpl.test | RUN | 0.534595 | 1.098276 | 1.374665 | 0.002805 | 0.002814 | 0.002779 |
| noinvoke.test | RUN | 0.466284 | 0.925677 | 1.165356 | 0.002735 | 0.002339 | 0.003110 |
| pack_expansion.test | RUN | 0.553737 | 1.306430 | 1.497323 | 0.002330 | 0.002145 | 0.002460 |
| protect.test | RUN | 0.388250 | 0.963014 | 1.027650 | 0.002433 | 0.002057 | 0.002339 |
| bug2407.test | COMPILE | 0.327659 | 0.683638 | 0.763137 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.985×, 1.007× | 1.026× |
| clang | 1.007×† | 1.050×, 0.964× | 1.084× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| calculator.test | 10 | 0.001555s | 0.001572s | 0.001459s | byte equality observed |
| constrained_ops.test | 10 | 0.001533s | 0.001474s | 0.001458s | byte equality observed |
| cpp-next_bug.test | 10 | 0.001533s | 0.001502s | 0.001460s | byte equality observed |
| deduce_domain.test | 10 | 0.001532s | 0.001538s | 0.001471s | byte equality observed |
| deep_copy.test | 10 | 0.001514s | 0.001513s | 0.001458s | byte equality observed |
| display_expr.test | 10 | 0.001524s | 0.001525s | 0.001450s | byte equality observed |
| env_var.test | 10 | 0.001750s | 0.001800s | 0.001732s | byte equality observed |
| examples.test | 10 | 0.001532s | 0.001552s | 0.001511s | byte equality observed |
| external_transforms.test | 10 | 0.001551s | 0.001614s | 0.001505s | byte equality observed |
| flatten.test | 10 | 0.001589s | 0.001600s | 0.001525s | byte equality observed |
| lambda.test | 10 | 0.001530s | 0.001551s | 0.001497s | byte equality observed |
| make.test | 10 | 0.001647s | 0.001673s | 0.001651s | byte equality observed |
| make_expr.test | 10 | 0.001848s | 0.001908s | 0.001834s | byte equality observed |
| matches.test | 10 | 0.001515s | 0.001551s | 0.001499s | byte equality observed |
| mem_ptr.test | 10 | 0.001748s | 0.001794s | 0.001737s | byte equality observed |
| mpl.test | 10 | 0.001535s | 0.001511s | 0.001505s | byte equality observed |
| noinvoke.test | 10 | 0.001542s | 0.001522s | 0.001476s | byte equality observed |
| pack_expansion.test | 10 | 0.001546s | 0.001552s | 0.001542s | byte equality observed |
| protect.test | 10 | 0.001558s | 0.001565s | 0.001545s | byte equality observed |
| switch.test | 10 | 0.001499s | 0.001521s | 0.001503s | byte equality observed |
| toy_spirit.test | 10 | 0.001502s | 0.001501s | 0.001524s | byte equality observed |
| toy_spirit2.test | 10 | 0.001518s | 0.001496s | 0.001494s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
