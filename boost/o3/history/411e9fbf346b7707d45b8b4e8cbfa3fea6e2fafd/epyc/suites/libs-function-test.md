# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.221282 | 1.236570 | 0.997174 | 0.001560 | 0.001582 | 0.001766 |
| function_test_no_rtti.test | RUN | 0.219565 | 1.223820 | 1.003574 | 0.001359 | 0.001558 | 0.001411 |
| function_n_test.test | RUN | 0.209124 | 1.010591 | 0.849571 | 0.001396 | 0.001844 | 0.001630 |
| allocator_test.test | RUN | 0.172215 | 0.565512 | 0.567915 | 0.001605 | 0.001752 | 0.001684 |
| stateless_test.test | RUN | 0.128141 | 0.327740 | 0.357856 | 0.001720 | 0.001883 | 0.001608 |
| lambda_test.test | RUN | 0.281161 | 0.760006 | 0.779117 | 0.001786 | 0.001658 | 0.001669 |
| function_test_fail1.test | COMPILE_FAIL | 0.096372 | 0.209250 | 0.237559 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.099715 | 0.210783 | 0.252251 | — | — | — |
| function_30.test | COMPILE | 0.101177 | 0.219855 | 0.251982 | — | — | — |
| function_30_repeat.test | COMPILE | 0.102788 | 0.232776 | 0.248955 | — | — | — |
| function_arith_cxx98.test | RUN | 0.137369 | 0.378648 | 0.395705 | 0.001826 | 0.001577 | 0.001840 |
| function_arith_portable.test | RUN | 0.136412 | 0.362512 | 0.392845 | 0.001476 | 0.001454 | 0.001631 |
| sum_avg_cxx98.test | RUN | 0.131903 | 0.314115 | 0.369110 | 0.001205 | 0.001712 | 0.001371 |
| sum_avg_portable.test | RUN | 0.128924 | 0.280657 | 0.366444 | 0.001652 | 0.001812 | 0.001716 |
| mem_fun_cxx98.test | RUN | 0.146094 | 0.444738 | 0.458558 | 0.001714 | 0.001473 | 0.001746 |
| mem_fun_portable.test | RUN | 0.141067 | 0.394485 | 0.428082 | 0.001416 | 0.001766 | 0.001838 |
| std_bind_cxx98.test | RUN | 0.132386 | 0.377570 | 0.427626 | 0.001400 | 0.001543 | 0.001387 |
| std_bind_portable.test | RUN | 0.140698 | 0.335599 | 0.396900 | 0.001729 | 0.001774 | 0.001730 |
| function_ref_cxx98.test | RUN | 0.129750 | 0.304287 | 0.382953 | 0.001742 | 0.001609 | 0.001817 |
| function_ref_portable.test | RUN | 0.124678 | 0.284324 | 0.351982 | 0.001529 | 0.001504 | 0.001668 |
| contains_test.test | RUN | 0.156469 | 0.577276 | 0.572649 | 0.001706 | 0.001737 | 0.001820 |
| contains2_test.test | RUN | 0.150438 | 0.421660 | 0.437305 | 0.001802 | 0.001666 | 0.001656 |
| nothrow_swap.test | RUN | 0.136398 | 0.408912 | 0.425789 | 0.001607 | 0.001824 | 0.001582 |
| rvalues_test.test | RUN | 0.152209 | 0.471325 | 0.504708 | 0.001507 | 0.001672 | 0.001898 |
| function_typeof_test.test | COMPILE | 0.103153 | 0.217135 | 0.265903 | — | — | — |
| result_arg_types_test.test | RUN | 0.146871 | 0.336446 | 0.461787 | 0.001664 | 0.001864 | 0.001899 |
| result_arg_n_types_test.test | RUN | 0.157580 | 0.365633 | 0.537741 | 0.001767 | 0.001948 | 0.001584 |
| test_bad_function_call_shared.test | RUN | 0.134479 | 0.312972 | 0.369181 | 0.001689 | 0.002109 | 0.001870 |
| test_bad_function_call_static.test | RUN | 0.129422 | 0.319048 | 0.382346 | 0.001600 | 0.001932 | 0.001554 |
| mixed_cxxstd_shared.test | RUN | 0.138163 | 0.385595 | 0.408479 | 0.001644 | 0.001503 | 0.001446 |
| mixed_cxxstd_static.test | RUN | 0.144314 | 0.367070 | 0.422504 | 0.001424 | 0.001836 | 0.001766 |
| mixed_cxxstd_shared_0x.test | RUN | 0.143119 | 0.358757 | 0.420686 | 0.001717 | 0.002023 | 0.001885 |
| mixed_cxxstd_static_0x.test | RUN | 0.135420 | 0.370241 | 0.415280 | 0.001707 | 0.001862 | 0.001577 |
| mixed_cxxstd_shared_14.test | RUN | 0.135667 | 0.366298 | 0.410174 | 0.001547 | 0.001433 | 0.001623 |
| mixed_cxxstd_static_14.test | RUN | 0.139773 | 0.372191 | 0.404281 | 0.001534 | 0.001898 | 0.001668 |
| return_function_shared.test | RUN | 0.135456 | 0.369128 | 0.406563 | 0.001974 | 0.002005 | 0.001938 |
| return_function_static.test | RUN | 0.137117 | 0.365775 | 0.407332 | 0.001510 | 0.001709 | 0.001700 |
| return_function_shared_0x.test | RUN | 0.143833 | 0.377803 | 0.407101 | 0.001520 | 0.001439 | 0.001816 |
| return_function_static_0x.test | RUN | 0.136644 | 0.364576 | 0.414427 | 0.001570 | 0.001819 | 0.001708 |
| return_function_shared_14.test | RUN | 0.137258 | 0.376803 | 0.396382 | 0.001596 | 0.001606 | 0.002018 |
| return_function_static_14.test | RUN | 0.134602 | 0.366006 | 0.395743 | 0.001667 | 0.001693 | 0.001608 |
| quick.test | RUN | 0.141766 | 0.364723 | 0.384700 | 0.001710 | 0.001452 | 0.001583 |
| issue_42.test | COMPILE | 0.107770 | 0.241571 | 0.288412 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.178096 | 0.511116 | 0.592601 | 0.001685 | 0.001649 | 0.001522 |
| contains_test_no_rtti.test | RUN | 0.158331 | 0.577304 | 0.582294 | 0.001568 | 0.001778 | 0.001656 |
| contains2_test_no_rtti.test | RUN | 0.142138 | 0.415716 | 0.450853 | 0.001462 | 0.001678 | 0.001784 |
| contains3_test.test | RUN | 0.136209 | 0.331062 | 0.402958 | 0.001676 | 0.001665 | 0.001426 |
| contains3_test_no_rtti.test | RUN | 0.094851 | 0.263226 | 0.279391 | 0.001365 | 0.001271 | 0.001372 |
| issue_53.test | COMPILE | 0.091612 | 0.195540 | 0.217862 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.024× | 1.024×, 1.024× | 0.992× |
| clang | 0.991× | 1.004×, 0.977× | 1.040× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000906s | 0.000938s | 0.000918s | byte equality observed |
| contains2_test.test | 10 | 0.000936s | 0.000929s | 0.000906s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000922s | 0.000929s | 0.000893s | byte equality observed |
| contains3_test.test | 10 | 0.000908s | 0.000952s | 0.000902s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000899s | 0.000931s | 0.000909s | byte equality observed |
| contains_test.test | 10 | 0.000930s | 0.000924s | 0.000917s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000919s | 0.000904s | 0.000921s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000917s | 0.000923s | 0.000929s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000933s | 0.000944s | 0.000948s | byte equality observed |
| function_arith_portable.test | 10 | 0.000924s | 0.000926s | 0.000933s | byte equality observed |
| function_n_test.test | 10 | 0.000941s | 0.000946s | 0.000910s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000878s | 0.000903s | 0.000911s | byte equality observed |
| function_ref_portable.test | 10 | 0.000900s | 0.000886s | 0.000894s | byte equality observed |
| function_test.test | 10 | 0.000955s | 0.000962s | 0.000987s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.000948s | 0.000961s | 0.000971s | byte equality observed |
| lambda_test.test | 10 | 0.000920s | 0.000930s | 0.000941s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000910s | 0.000922s | 0.000906s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000942s | 0.000939s | 0.000911s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001004s | 0.000987s | 0.001001s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001000s | 0.001019s | 0.000998s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001000s | 0.000981s | 0.000993s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000919s | 0.000918s | 0.000921s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000929s | 0.000900s | 0.000904s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000922s | 0.000899s | 0.000895s | byte equality observed |
| nothrow_swap.test | 10 | 0.000927s | 0.000898s | 0.000900s | byte equality observed |
| quick.test | 10 | 0.000927s | 0.000918s | 0.000901s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000924s | 0.000901s | 0.000898s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000910s | 0.000896s | 0.000904s | byte equality observed |
| return_function_shared.test | 10 | 0.000996s | 0.000991s | 0.000997s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.000984s | 0.000969s | 0.000971s | byte equality observed |
| return_function_shared_14.test | 10 | 0.000981s | 0.000993s | 0.001006s | byte equality observed |
| return_function_static.test | 10 | 0.000922s | 0.000893s | 0.000935s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000917s | 0.000886s | 0.000913s | byte equality observed |
| return_function_static_14.test | 10 | 0.000927s | 0.000901s | 0.000922s | byte equality observed |
| rvalues_test.test | 10 | 0.000910s | 0.000924s | 0.000893s | byte equality observed |
| stateless_test.test | 10 | 0.000910s | 0.000899s | 0.000915s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000894s | 0.000899s | 0.000887s | byte equality observed |
| std_bind_portable.test | 10 | 0.000897s | 0.000894s | 0.000893s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000889s | 0.000891s | 0.000875s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000899s | 0.000881s | 0.000910s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001025s | 0.001018s | 0.001008s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000938s | 0.000936s | 0.000933s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
