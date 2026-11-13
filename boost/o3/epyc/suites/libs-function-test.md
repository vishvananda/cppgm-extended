# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.364865 | 2.001448 | 2.025531 | 0.001771 | 0.001697 | 0.001674 |
| function_test_no_rtti.test | RUN | 0.361199 | 1.995592 | 1.970542 | 0.001809 | 0.001922 | 0.001932 |
| function_n_test.test | RUN | 0.329707 | 1.647829 | 1.696529 | 0.001801 | 0.002262 | 0.001661 |
| allocator_test.test | RUN | 0.269869 | 0.955366 | 1.187642 | 0.001919 | 0.002062 | 0.001942 |
| stateless_test.test | RUN | 0.210138 | 0.525941 | 0.788545 | 0.001662 | 0.002018 | 0.002131 |
| lambda_test.test | RUN | 0.452740 | 1.291910 | 1.690835 | 0.002039 | 0.001812 | 0.001760 |
| function_test_fail1.test | COMPILE_FAIL | 0.157412 | 0.354884 | 0.560535 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.166701 | 0.377734 | 0.575443 | — | — | — |
| function_30.test | COMPILE | 0.170589 | 0.379338 | 0.585866 | — | — | — |
| function_30_repeat.test | COMPILE | 0.168859 | 0.382070 | 0.572797 | — | — | — |
| function_arith_cxx98.test | RUN | 0.223781 | 0.633737 | 0.831761 | 0.001976 | 0.001832 | 0.002038 |
| function_arith_portable.test | RUN | 0.220887 | 0.598493 | 0.776642 | 0.001822 | 0.001883 | 0.002166 |
| sum_avg_cxx98.test | RUN | 0.211841 | 0.538372 | 0.717418 | 0.001754 | 0.001898 | 0.001975 |
| sum_avg_portable.test | RUN | 0.205263 | 0.472778 | 0.731422 | 0.001672 | 0.001840 | 0.002012 |
| mem_fun_cxx98.test | RUN | 0.236089 | 0.725357 | 0.925677 | 0.002074 | 0.002021 | 0.001882 |
| mem_fun_portable.test | RUN | 0.234558 | 0.663349 | 0.887522 | 0.001986 | 0.001968 | 0.001665 |
| std_bind_cxx98.test | RUN | 0.222002 | 0.624406 | 0.819581 | 0.001750 | 0.001865 | 0.002073 |
| std_bind_portable.test | RUN | 0.218235 | 0.568778 | 0.739796 | 0.001784 | 0.002088 | 0.001990 |
| function_ref_cxx98.test | RUN | 0.207861 | 0.526567 | 0.779423 | 0.001947 | 0.002098 | 0.001630 |
| function_ref_portable.test | RUN | 0.204651 | 0.478539 | 0.720507 | 0.001928 | 0.001996 | 0.001653 |
| contains_test.test | RUN | 0.251604 | 0.975298 | 1.122422 | 0.002044 | 0.002212 | 0.001884 |
| contains2_test.test | RUN | 0.226189 | 0.710178 | 0.903127 | 0.002135 | 0.002037 | 0.001976 |
| nothrow_swap.test | RUN | 0.224613 | 0.651859 | 0.886157 | 0.001758 | 0.001783 | 0.001680 |
| rvalues_test.test | RUN | 0.260326 | 0.812008 | 1.023792 | 0.002072 | 0.001965 | 0.002012 |
| function_typeof_test.test | COMPILE | 0.163068 | 0.378067 | 0.556370 | — | — | — |
| result_arg_types_test.test | RUN | 0.235528 | 0.558954 | 0.945759 | 0.002154 | 0.001571 | 0.002111 |
| result_arg_n_types_test.test | RUN | 0.259845 | 0.615499 | 1.063058 | 0.001889 | 0.001798 | 0.002104 |
| test_bad_function_call_shared.test | RUN | 0.217146 | 0.532256 | 0.796240 | 0.002068 | 0.002240 | 0.002394 |
| test_bad_function_call_static.test | RUN | 0.212895 | 0.522725 | 0.756343 | 0.001675 | 0.001818 | 0.002121 |
| mixed_cxxstd_shared.test | RUN | 0.228817 | 0.626784 | 0.863349 | 0.001843 | 0.002082 | 0.002213 |
| mixed_cxxstd_static.test | RUN | 0.215151 | 0.611588 | 0.873107 | 0.002001 | 0.001928 | 0.001774 |
| mixed_cxxstd_shared_0x.test | RUN | 0.215427 | 0.609205 | 0.885025 | 0.002169 | 0.002122 | 0.002344 |
| mixed_cxxstd_static_0x.test | RUN | 0.223339 | 0.611653 | 0.854038 | 0.001813 | 0.001756 | 0.001980 |
| mixed_cxxstd_shared_14.test | RUN | 0.223024 | 0.616841 | 0.830723 | 0.002116 | 0.001949 | 0.002430 |
| mixed_cxxstd_static_14.test | RUN | 0.224460 | 0.610960 | 0.819222 | 0.001996 | 0.002178 | 0.002084 |
| return_function_shared.test | RUN | 0.215120 | 0.616795 | 0.830078 | 0.001972 | 0.002268 | 0.002273 |
| return_function_static.test | RUN | 0.216979 | 0.604039 | 0.853489 | 0.001861 | 0.001959 | 0.001802 |
| return_function_shared_0x.test | RUN | 0.217680 | 0.603434 | 0.866754 | 0.001881 | 0.002178 | 0.002257 |
| return_function_static_0x.test | RUN | 0.217846 | 0.593061 | 0.778767 | 0.002023 | 0.001942 | 0.002103 |
| return_function_shared_14.test | RUN | 0.219208 | 0.614563 | 0.814711 | 0.001948 | 0.001960 | 0.002346 |
| return_function_static_14.test | RUN | 0.221178 | 0.599703 | 0.817064 | 0.002044 | 0.001935 | 0.002133 |
| quick.test | RUN | 0.225917 | 0.618804 | 0.825666 | 0.001658 | 0.002030 | 0.002030 |
| issue_42.test | COMPILE | 0.183475 | 0.390599 | 0.606521 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.286872 | 0.835437 | 1.250713 | 0.001921 | 0.002004 | 0.002146 |
| contains_test_no_rtti.test | RUN | 0.258804 | 0.947155 | 1.137359 | 0.001766 | 0.001960 | 0.001744 |
| contains2_test_no_rtti.test | RUN | 0.235762 | 0.701088 | 0.902646 | 0.001830 | 0.001820 | 0.002114 |
| contains3_test.test | RUN | 0.216165 | 0.571062 | 0.833948 | 0.001746 | 0.001821 | 0.002136 |
| contains3_test_no_rtti.test | RUN | 0.138370 | 0.426211 | 0.534098 | 0.001489 | 0.001370 | 0.001676 |
| issue_53.test | COMPILE | 0.125356 | 0.295901 | 0.392196 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 1.001×, 0.992× | 1.012× |
| clang | 1.010× | 1.016×, 1.005× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001168s | 0.001130s | 0.001167s | byte equality observed |
| contains2_test.test | 10 | 0.001164s | 0.001133s | 0.001138s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001132s | 0.001152s | 0.001155s | byte equality observed |
| contains3_test.test | 10 | 0.001143s | 0.001121s | 0.001181s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001122s | 0.001167s | 0.001244s | byte equality observed |
| contains_test.test | 10 | 0.001131s | 0.001113s | 0.001252s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001134s | 0.001131s | 0.001263s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001136s | 0.001212s | 0.001309s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001147s | 0.001281s | 0.001249s | byte equality observed |
| function_arith_portable.test | 10 | 0.001187s | 0.001304s | 0.001221s | byte equality observed |
| function_n_test.test | 10 | 0.001237s | 0.001284s | 0.001280s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001221s | 0.001227s | 0.001211s | byte equality observed |
| function_ref_portable.test | 10 | 0.001212s | 0.001192s | 0.001196s | byte equality observed |
| function_test.test | 10 | 0.001291s | 0.001276s | 0.001242s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001305s | 0.001275s | 0.001336s | byte equality observed |
| lambda_test.test | 10 | 0.001283s | 0.001242s | 0.001238s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001264s | 0.001273s | 0.001252s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001269s | 0.001207s | 0.001250s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001354s | 0.001329s | 0.001337s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001376s | 0.001358s | 0.001409s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001337s | 0.001413s | 0.001364s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001279s | 0.001220s | 0.001244s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001274s | 0.001234s | 0.001252s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001253s | 0.001232s | 0.001296s | byte equality observed |
| nothrow_swap.test | 10 | 0.001276s | 0.001214s | 0.001242s | byte equality observed |
| quick.test | 10 | 0.001256s | 0.001264s | 0.001204s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001240s | 0.001247s | 0.001243s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001206s | 0.001199s | 0.001234s | byte equality observed |
| return_function_shared.test | 10 | 0.001306s | 0.001302s | 0.001361s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001349s | 0.001292s | 0.001377s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001377s | 0.001317s | 0.001348s | byte equality observed |
| return_function_static.test | 10 | 0.001255s | 0.001235s | 0.001248s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001262s | 0.001236s | 0.001207s | byte equality observed |
| return_function_static_14.test | 10 | 0.001259s | 0.001262s | 0.001217s | byte equality observed |
| rvalues_test.test | 10 | 0.001273s | 0.001281s | 0.001217s | byte equality observed |
| stateless_test.test | 10 | 0.001256s | 0.001202s | 0.001223s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001226s | 0.001296s | 0.001245s | byte equality observed |
| std_bind_portable.test | 10 | 0.001216s | 0.001250s | 0.001216s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001200s | 0.001237s | 0.001164s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001191s | 0.001247s | 0.001195s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001395s | 0.001363s | 0.001368s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001306s | 0.001272s | 0.001246s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
