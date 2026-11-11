# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.419593 | 2.069098 | 1.782918 | 0.001824 | 0.002535 | 0.002212 |
| function_test_no_rtti.test | RUN | 0.554334 | 2.127012 | 1.690572 | 0.001733 | 0.002197 | 0.001640 |
| function_n_test.test | RUN | 0.410394 | 1.785995 | 1.471992 | 0.002155 | 0.002182 | 0.003102 |
| allocator_test.test | RUN | 0.268198 | 0.998758 | 0.966640 | 0.001680 | 0.001672 | 0.002089 |
| stateless_test.test | RUN | 0.221963 | 0.493199 | 0.663458 | 0.001578 | 0.002544 | 0.002470 |
| lambda_test.test | RUN | 0.512620 | 1.209273 | 1.392518 | 0.001728 | 0.002923 | 0.001986 |
| function_test_fail1.test | COMPILE_FAIL | 0.153286 | 0.334073 | 0.454728 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.270235 | 0.427776 | 0.456394 | — | — | — |
| function_30.test | COMPILE | 0.162290 | 0.346447 | 0.469868 | — | — | — |
| function_30_repeat.test | COMPILE | 0.179985 | 0.346492 | 0.470441 | — | — | — |
| function_arith_cxx98.test | RUN | 0.320604 | 0.591307 | 0.728350 | 0.001538 | 0.001763 | 0.001664 |
| function_arith_portable.test | RUN | 0.218123 | 0.558463 | 0.677427 | 0.001687 | 0.001974 | 0.001957 |
| sum_avg_cxx98.test | RUN | 0.193938 | 0.494633 | 0.639752 | 0.001554 | 0.001661 | 0.001515 |
| sum_avg_portable.test | RUN | 0.313291 | 0.458557 | 0.613225 | 0.001928 | 0.002125 | 0.001624 |
| mem_fun_cxx98.test | RUN | 0.274132 | 0.706157 | 0.806581 | 0.001632 | 0.001780 | 0.001752 |
| mem_fun_portable.test | RUN | 0.360517 | 0.642222 | 0.764664 | 0.001602 | 0.002016 | 0.001728 |
| std_bind_cxx98.test | RUN | 0.248847 | 0.588527 | 0.718058 | 0.001826 | 0.001743 | 0.002056 |
| std_bind_portable.test | RUN | 0.227538 | 0.536590 | 0.675302 | 0.001737 | 0.001838 | 0.001630 |
| function_ref_cxx98.test | RUN | 0.210170 | 0.509906 | 0.637502 | 0.001630 | 0.002154 | 0.001848 |
| function_ref_portable.test | RUN | 0.216361 | 0.471343 | 0.609878 | 0.001750 | 0.001870 | 0.001802 |
| contains_test.test | RUN | 0.304432 | 0.978138 | 0.977928 | 0.001881 | 0.002102 | 0.001638 |
| contains2_test.test | RUN | 0.293118 | 0.723645 | 0.763577 | 0.001670 | 0.002062 | 0.003237 |
| nothrow_swap.test | RUN | 0.235556 | 0.690798 | 0.744746 | 0.001788 | 0.002052 | 0.002755 |
| rvalues_test.test | RUN | 0.318841 | 0.775196 | 0.862946 | 0.001703 | 0.001722 | 0.001726 |
| function_typeof_test.test | COMPILE | 0.199555 | 0.368772 | 0.500640 | — | — | — |
| result_arg_types_test.test | RUN | 0.281040 | 0.590345 | 0.782455 | 0.001915 | 0.001962 | 0.002164 |
| result_arg_n_types_test.test | RUN | 0.296885 | 0.619486 | 0.880794 | 0.001604 | 0.001764 | 0.002356 |
| test_bad_function_call_shared.test | RUN | 0.293276 | 0.498707 | 0.650331 | 0.001552 | 0.002180 | 0.002542 |
| test_bad_function_call_static.test | RUN | 0.229774 | 0.489553 | 0.670584 | 0.001782 | 0.001768 | 0.001723 |
| mixed_cxxstd_shared.test | RUN | 0.225770 | 0.576771 | 0.706823 | 0.001822 | 0.002251 | 0.002188 |
| mixed_cxxstd_static.test | RUN | 0.228488 | 0.580483 | 0.714161 | 0.001599 | 0.001697 | 0.001691 |
| mixed_cxxstd_shared_0x.test | RUN | 0.235983 | 0.575655 | 0.738229 | 0.002202 | 0.002129 | 0.002438 |
| mixed_cxxstd_static_0x.test | RUN | 0.254918 | 0.578168 | 0.729783 | 0.001580 | 0.002260 | 0.001506 |
| mixed_cxxstd_shared_14.test | RUN | 0.273172 | 0.594040 | 0.739527 | 0.001792 | 0.001808 | 0.002110 |
| mixed_cxxstd_static_14.test | RUN | 0.264149 | 0.607178 | 0.724296 | 0.001656 | 0.001898 | 0.001654 |
| return_function_shared.test | RUN | 0.321099 | 0.585460 | 0.724730 | 0.001930 | 0.002142 | 0.001978 |
| return_function_static.test | RUN | 0.227805 | 0.572819 | 0.722413 | 0.001638 | 0.002389 | 0.001922 |
| return_function_shared_0x.test | RUN | 0.267582 | 0.587811 | 0.728496 | 0.002014 | 0.002246 | 0.001894 |
| return_function_static_0x.test | RUN | 0.225271 | 0.576728 | 0.707352 | 0.002112 | 0.003081 | 0.002166 |
| return_function_shared_14.test | RUN | 0.217662 | 0.574074 | 0.719322 | 0.001695 | 0.001996 | 0.003734 |
| return_function_static_14.test | RUN | 0.224188 | 0.592487 | 0.732771 | 0.001535 | 0.002263 | 0.001988 |
| quick.test | RUN | 0.214215 | 0.589865 | 0.686613 | 0.001788 | 0.002004 | 0.002048 |
| issue_42.test | COMPILE | 0.182038 | 0.385713 | 0.497395 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.298133 | 0.847424 | 1.043619 | 0.001686 | 0.002004 | 0.001526 |
| contains_test_no_rtti.test | RUN | 0.268399 | 0.929541 | 0.940484 | 0.001884 | 0.003152 | 0.001984 |
| contains2_test_no_rtti.test | RUN | 0.228633 | 0.666171 | 0.755470 | 0.001683 | 0.002464 | 0.001802 |
| contains3_test.test | RUN | 0.219591 | 0.526053 | 0.661922 | 0.002211 | 0.002064 | 0.002884 |
| contains3_test_no_rtti.test | RUN | 0.224170 | 0.515521 | 0.700012 | 0.001718 | 0.002304 | 0.001888 |
| issue_53.test | COMPILE | 0.195560 | 0.419742 | 0.575092 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.030× | 1.013×, 1.046× | 1.017× |
| clang | 0.971×† | 0.932×, 1.010× | 1.029× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 6 | 0.001642s | 0.001726s | 0.001584s | byte equality observed |
| contains2_test.test | 7 | 0.001765s | 0.001633s | 0.001608s | byte equality observed |
| contains2_test_no_rtti.test | 8 | 0.001612s | 0.001832s | 0.001607s | byte equality observed |
| contains3_test.test | 7 | 0.001704s | 0.001711s | 0.001619s | byte equality observed |
| contains3_test_no_rtti.test | 8 | 0.001668s | 0.001614s | 0.001642s | byte equality observed |
| contains_test.test | 7 | 0.001602s | 0.001713s | 0.001587s | byte equality observed |
| contains_test_no_rtti.test | 8 | 0.001634s | 0.001730s | 0.001587s | byte equality observed |
| fn_eq_bind_test.test | 7 | 0.001677s | 0.001660s | 0.001706s | byte equality observed |
| function_arith_cxx98.test | 7 | 0.001661s | 0.001649s | 0.001616s | byte equality observed |
| function_arith_portable.test | 8 | 0.001692s | 0.001802s | 0.001593s | byte equality observed |
| function_n_test.test | 8 | 0.001639s | 0.001672s | 0.001690s | byte equality observed |
| function_ref_cxx98.test | 8 | 0.001620s | 0.001607s | 0.001572s | byte equality observed |
| function_ref_portable.test | 7 | 0.001611s | 0.001598s | 0.001514s | byte equality observed |
| function_test.test | 8 | 0.001649s | 0.001757s | 0.001793s | byte equality observed |
| function_test_no_rtti.test | 8 | 0.001736s | 0.001940s | 0.001666s | byte equality observed |
| lambda_test.test | 7 | 0.001641s | 0.001638s | 0.001604s | byte equality observed |
| mem_fun_cxx98.test | 7 | 0.001672s | 0.001690s | 0.001662s | byte equality observed |
| mem_fun_portable.test | 7 | 0.001636s | 0.001706s | 0.001571s | byte equality observed |
| mixed_cxxstd_shared.test | 7 | 0.001753s | 0.001891s | 0.001845s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 6 | 0.001762s | 0.001744s | 0.001789s | byte equality observed |
| mixed_cxxstd_shared_14.test | 6 | 0.001758s | 0.001809s | 0.001763s | byte equality observed |
| mixed_cxxstd_static.test | 8 | 0.001626s | 0.001655s | 0.001620s | byte equality observed |
| mixed_cxxstd_static_0x.test | 7 | 0.001646s | 0.001705s | 0.001614s | byte equality observed |
| mixed_cxxstd_static_14.test | 7 | 0.001680s | 0.001674s | 0.001609s | byte equality observed |
| nothrow_swap.test | 7 | 0.001672s | 0.001734s | 0.001764s | byte equality observed |
| quick.test | 8 | 0.001608s | 0.001634s | 0.001588s | byte equality observed |
| result_arg_n_types_test.test | 8 | 0.001597s | 0.001674s | 0.001640s | byte equality observed |
| result_arg_types_test.test | 9 | 0.001628s | 0.001602s | 0.001647s | byte equality observed |
| return_function_shared.test | 8 | 0.001764s | 0.001820s | 0.001800s | byte equality observed |
| return_function_shared_0x.test | 6 | 0.001727s | 0.001754s | 0.001893s | byte equality observed |
| return_function_shared_14.test | 7 | 0.001701s | 0.001781s | 0.001757s | byte equality observed |
| return_function_static.test | 7 | 0.001642s | 0.001588s | 0.001643s | byte equality observed |
| return_function_static_0x.test | 8 | 0.001642s | 0.001658s | 0.001624s | byte equality observed |
| return_function_static_14.test | 8 | 0.001670s | 0.001566s | 0.001732s | byte equality observed |
| rvalues_test.test | 7 | 0.001616s | 0.001717s | 0.001658s | byte equality observed |
| stateless_test.test | 8 | 0.001615s | 0.001707s | 0.001679s | byte equality observed |
| std_bind_cxx98.test | 8 | 0.001596s | 0.001566s | 0.001577s | byte equality observed |
| std_bind_portable.test | 6 | 0.001563s | 0.001536s | 0.001561s | byte equality observed |
| sum_avg_cxx98.test | 7 | 0.001587s | 0.001745s | 0.001540s | byte equality observed |
| sum_avg_portable.test | 7 | 0.001510s | 0.001538s | 0.001621s | byte equality observed |
| test_bad_function_call_shared.test | 7 | 0.001698s | 0.001779s | 0.001757s | byte equality observed |
| test_bad_function_call_static.test | 6 | 0.001608s | 0.001809s | 0.001724s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
