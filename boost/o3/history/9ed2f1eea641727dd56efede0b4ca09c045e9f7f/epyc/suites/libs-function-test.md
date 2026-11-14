# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.275095 | 1.508765 | 1.354298 | 0.001568 | 0.001662 | 0.001627 |
| function_test_no_rtti.test | RUN | 0.271734 | 1.475922 | 1.360324 | 0.001648 | 0.001738 | 0.001641 |
| function_n_test.test | RUN | 0.250726 | 1.227586 | 1.147837 | 0.001584 | 0.001677 | 0.001842 |
| allocator_test.test | RUN | 0.205397 | 0.712357 | 0.790436 | 0.001575 | 0.001784 | 0.001630 |
| stateless_test.test | RUN | 0.159810 | 0.382412 | 0.480268 | 0.001774 | 0.001676 | 0.001792 |
| lambda_test.test | RUN | 0.350416 | 0.914535 | 1.050698 | 0.001784 | 0.001749 | 0.001682 |
| function_test_fail1.test | COMPILE_FAIL | 0.120300 | 0.265283 | 0.364870 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.123964 | 0.261040 | 0.366883 | — | — | — |
| function_30.test | COMPILE | 0.127399 | 0.273953 | 0.379364 | — | — | — |
| function_30_repeat.test | COMPILE | 0.128900 | 0.286337 | 0.383429 | — | — | — |
| function_arith_cxx98.test | RUN | 0.167816 | 0.456705 | 0.545852 | 0.001768 | 0.001571 | 0.001790 |
| function_arith_portable.test | RUN | 0.166365 | 0.433589 | 0.525059 | 0.001588 | 0.001646 | 0.001805 |
| sum_avg_cxx98.test | RUN | 0.157570 | 0.369747 | 0.484117 | 0.001527 | 0.001714 | 0.001750 |
| sum_avg_portable.test | RUN | 0.155638 | 0.349013 | 0.481173 | 0.001754 | 0.001826 | 0.001737 |
| mem_fun_cxx98.test | RUN | 0.181230 | 0.533747 | 0.630960 | 0.001802 | 0.001544 | 0.001608 |
| mem_fun_portable.test | RUN | 0.177260 | 0.478931 | 0.529717 | 0.001494 | 0.001792 | 0.001590 |
| std_bind_cxx98.test | RUN | 0.165189 | 0.451497 | 0.522785 | 0.001588 | 0.001562 | 0.001712 |
| std_bind_portable.test | RUN | 0.170012 | 0.403700 | 0.493310 | 0.001722 | 0.001838 | 0.001680 |
| function_ref_cxx98.test | RUN | 0.158189 | 0.368493 | 0.492121 | 0.001608 | 0.001758 | 0.001506 |
| function_ref_portable.test | RUN | 0.156059 | 0.340978 | 0.424570 | 0.001568 | 0.001435 | 0.001666 |
| contains_test.test | RUN | 0.193381 | 0.693843 | 0.736311 | 0.001557 | 0.001852 | 0.001537 |
| contains2_test.test | RUN | 0.181102 | 0.499616 | 0.570477 | 0.001716 | 0.001730 | 0.001693 |
| nothrow_swap.test | RUN | 0.166420 | 0.485491 | 0.549743 | 0.001763 | 0.001880 | 0.001676 |
| rvalues_test.test | RUN | 0.192641 | 0.568910 | 0.635494 | 0.001703 | 0.001681 | 0.001592 |
| function_typeof_test.test | COMPILE | 0.126329 | 0.269937 | 0.319785 | — | — | — |
| result_arg_types_test.test | RUN | 0.180966 | 0.412282 | 0.600347 | 0.001692 | 0.001656 | 0.001318 |
| result_arg_n_types_test.test | RUN | 0.191187 | 0.445453 | 0.689417 | 0.001592 | 0.001915 | 0.001851 |
| test_bad_function_call_shared.test | RUN | 0.163700 | 0.386404 | 0.506235 | 0.001359 | 0.001780 | 0.001916 |
| test_bad_function_call_static.test | RUN | 0.162114 | 0.386466 | 0.513246 | 0.001604 | 0.001654 | 0.001399 |
| mixed_cxxstd_shared.test | RUN | 0.169683 | 0.458596 | 0.548048 | 0.001971 | 0.001812 | 0.002001 |
| mixed_cxxstd_static.test | RUN | 0.170320 | 0.465118 | 0.560313 | 0.001793 | 0.001719 | 0.001849 |
| mixed_cxxstd_shared_0x.test | RUN | 0.168358 | 0.468243 | 0.576593 | 0.001925 | 0.002073 | 0.001928 |
| mixed_cxxstd_static_0x.test | RUN | 0.162583 | 0.440574 | 0.570321 | 0.001677 | 0.001771 | 0.001584 |
| mixed_cxxstd_shared_14.test | RUN | 0.170823 | 0.456119 | 0.566189 | 0.001919 | 0.001986 | 0.001908 |
| mixed_cxxstd_static_14.test | RUN | 0.172376 | 0.455190 | 0.569008 | 0.001811 | 0.001859 | 0.001694 |
| return_function_shared.test | RUN | 0.169123 | 0.450508 | 0.512336 | 0.001617 | 0.002139 | 0.001864 |
| return_function_static.test | RUN | 0.168016 | 0.456505 | 0.575381 | 0.001802 | 0.001848 | 0.001524 |
| return_function_shared_0x.test | RUN | 0.171231 | 0.470591 | 0.555785 | 0.001834 | 0.002044 | 0.002045 |
| return_function_static_0x.test | RUN | 0.167809 | 0.434618 | 0.569014 | 0.001758 | 0.001667 | 0.001775 |
| return_function_shared_14.test | RUN | 0.170362 | 0.430613 | 0.571132 | 0.001916 | 0.001952 | 0.001964 |
| return_function_static_14.test | RUN | 0.164833 | 0.439029 | 0.589834 | 0.001580 | 0.001858 | 0.001824 |
| quick.test | RUN | 0.169723 | 0.443821 | 0.548586 | 0.001877 | 0.001700 | 0.001785 |
| issue_42.test | COMPILE | 0.132404 | 0.289599 | 0.350022 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.229675 | 0.613705 | 0.785685 | 0.001622 | 0.001721 | 0.001452 |
| contains_test_no_rtti.test | RUN | 0.195014 | 0.687136 | 0.762902 | 0.001741 | 0.001728 | 0.001367 |
| contains2_test_no_rtti.test | RUN | 0.183873 | 0.517923 | 0.612749 | 0.001570 | 0.001689 | 0.001358 |
| contains3_test.test | RUN | 0.174104 | 0.403253 | 0.526141 | 0.001473 | 0.001679 | 0.001445 |
| contains3_test_no_rtti.test | RUN | 0.135185 | 0.334465 | 0.357988 | 0.001413 | 0.001354 | 0.001292 |
| issue_53.test | COMPILE | 0.120887 | 0.230220 | 0.259708 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 1.003×, 0.998× | 0.992× |
| clang | 0.999× | 1.009×, 0.988× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000968s | 0.000982s | 0.000984s | byte equality observed |
| contains2_test.test | 10 | 0.000970s | 0.001003s | 0.000971s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000949s | 0.000976s | 0.001003s | byte equality observed |
| contains3_test.test | 10 | 0.000954s | 0.000965s | 0.000997s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000951s | 0.001026s | 0.000996s | byte equality observed |
| contains_test.test | 10 | 0.000960s | 0.001031s | 0.000993s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000993s | 0.001003s | 0.001006s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000995s | 0.001005s | 0.001015s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000991s | 0.001014s | 0.001015s | byte equality observed |
| function_arith_portable.test | 10 | 0.001027s | 0.001011s | 0.001028s | byte equality observed |
| function_n_test.test | 10 | 0.001019s | 0.001010s | 0.001035s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000990s | 0.000986s | 0.000977s | byte equality observed |
| function_ref_portable.test | 10 | 0.000980s | 0.000949s | 0.000969s | byte equality observed |
| function_test.test | 10 | 0.001053s | 0.001054s | 0.001043s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001066s | 0.001048s | 0.001070s | byte equality observed |
| lambda_test.test | 10 | 0.001013s | 0.000999s | 0.000992s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000999s | 0.000999s | 0.000979s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001017s | 0.000999s | 0.000984s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001078s | 0.001089s | 0.001061s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001089s | 0.001096s | 0.001099s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001084s | 0.001095s | 0.001096s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001015s | 0.001035s | 0.001007s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001028s | 0.001029s | 0.001008s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001018s | 0.001013s | 0.001042s | byte equality observed |
| nothrow_swap.test | 10 | 0.001009s | 0.001002s | 0.001016s | byte equality observed |
| quick.test | 10 | 0.001036s | 0.001011s | 0.001017s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001018s | 0.000992s | 0.000994s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001019s | 0.000990s | 0.000998s | byte equality observed |
| return_function_shared.test | 10 | 0.001083s | 0.001083s | 0.001120s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001085s | 0.001068s | 0.001083s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001106s | 0.001102s | 0.001079s | byte equality observed |
| return_function_static.test | 10 | 0.001016s | 0.001012s | 0.000998s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001002s | 0.001034s | 0.001006s | byte equality observed |
| return_function_static_14.test | 10 | 0.000999s | 0.001035s | 0.001023s | byte equality observed |
| rvalues_test.test | 10 | 0.001003s | 0.001003s | 0.000986s | byte equality observed |
| stateless_test.test | 10 | 0.001007s | 0.001024s | 0.000993s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000982s | 0.000972s | 0.001000s | byte equality observed |
| std_bind_portable.test | 10 | 0.000989s | 0.000981s | 0.000965s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000990s | 0.000984s | 0.000987s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000976s | 0.000992s | 0.000970s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001131s | 0.001116s | 0.001109s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001037s | 0.001073s | 0.001045s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
