# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.271690 | 1.460337 | 1.334978 | 0.001593 | 0.001690 | 0.001710 |
| function_test_no_rtti.test | RUN | 0.268349 | 1.447784 | 1.347236 | 0.001406 | 0.001686 | 0.001337 |
| function_n_test.test | RUN | 0.247789 | 1.197467 | 1.139215 | 0.001567 | 0.001676 | 0.001719 |
| allocator_test.test | RUN | 0.203751 | 0.690150 | 0.785213 | 0.001738 | 0.001746 | 0.001710 |
| stateless_test.test | RUN | 0.162759 | 0.377238 | 0.483967 | 0.001845 | 0.001564 | 0.001732 |
| lambda_test.test | RUN | 0.344327 | 0.908610 | 1.083878 | 0.001792 | 0.001726 | 0.001860 |
| function_test_fail1.test | COMPILE_FAIL | 0.117556 | 0.260395 | 0.357160 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.129376 | 0.252138 | 0.345919 | — | — | — |
| function_30.test | COMPILE | 0.129549 | 0.258681 | 0.362524 | — | — | — |
| function_30_repeat.test | COMPILE | 0.122498 | 0.281932 | 0.376381 | — | — | — |
| function_arith_cxx98.test | RUN | 0.165821 | 0.448153 | 0.548288 | 0.001896 | 0.001560 | 0.001686 |
| function_arith_portable.test | RUN | 0.161884 | 0.419459 | 0.518522 | 0.001414 | 0.001688 | 0.001748 |
| sum_avg_cxx98.test | RUN | 0.154351 | 0.365994 | 0.471844 | 0.001622 | 0.001762 | 0.001682 |
| sum_avg_portable.test | RUN | 0.157860 | 0.332722 | 0.457275 | 0.001720 | 0.001762 | 0.001711 |
| mem_fun_cxx98.test | RUN | 0.174426 | 0.522931 | 0.617504 | 0.001778 | 0.001598 | 0.001426 |
| mem_fun_portable.test | RUN | 0.166647 | 0.477506 | 0.590531 | 0.001520 | 0.001506 | 0.001507 |
| std_bind_cxx98.test | RUN | 0.164766 | 0.429374 | 0.512403 | 0.001254 | 0.001525 | 0.001755 |
| std_bind_portable.test | RUN | 0.161196 | 0.409305 | 0.519133 | 0.001746 | 0.001768 | 0.001616 |
| function_ref_cxx98.test | RUN | 0.153622 | 0.370512 | 0.458427 | 0.001718 | 0.001359 | 0.001732 |
| function_ref_portable.test | RUN | 0.151776 | 0.336850 | 0.453026 | 0.001571 | 0.001716 | 0.001592 |
| contains_test.test | RUN | 0.190003 | 0.696252 | 0.734277 | 0.001248 | 0.001756 | 0.001455 |
| contains2_test.test | RUN | 0.175808 | 0.502794 | 0.570287 | 0.001810 | 0.001666 | 0.001594 |
| nothrow_swap.test | RUN | 0.162951 | 0.481052 | 0.595649 | 0.001728 | 0.001808 | 0.001670 |
| rvalues_test.test | RUN | 0.185076 | 0.563227 | 0.636081 | 0.001768 | 0.001650 | 0.001484 |
| function_typeof_test.test | COMPILE | 0.127419 | 0.263585 | 0.330935 | — | — | — |
| result_arg_types_test.test | RUN | 0.172617 | 0.405599 | 0.596109 | 0.001494 | 0.001884 | 0.001730 |
| result_arg_n_types_test.test | RUN | 0.190252 | 0.446049 | 0.673538 | 0.001762 | 0.001890 | 0.001719 |
| test_bad_function_call_shared.test | RUN | 0.166203 | 0.368541 | 0.511218 | 0.001791 | 0.001770 | 0.001780 |
| test_bad_function_call_static.test | RUN | 0.170793 | 0.369445 | 0.480019 | 0.001647 | 0.001721 | 0.001676 |
| mixed_cxxstd_shared.test | RUN | 0.165374 | 0.445569 | 0.543445 | 0.001673 | 0.001763 | 0.001824 |
| mixed_cxxstd_static.test | RUN | 0.164261 | 0.442252 | 0.534621 | 0.001519 | 0.001712 | 0.001617 |
| mixed_cxxstd_shared_0x.test | RUN | 0.162815 | 0.449612 | 0.571117 | 0.001844 | 0.002019 | 0.001978 |
| mixed_cxxstd_static_0x.test | RUN | 0.164871 | 0.439210 | 0.509061 | 0.001590 | 0.001718 | 0.001733 |
| mixed_cxxstd_shared_14.test | RUN | 0.166635 | 0.445311 | 0.557345 | 0.001677 | 0.001872 | 0.001942 |
| mixed_cxxstd_static_14.test | RUN | 0.169116 | 0.438634 | 0.564881 | 0.001580 | 0.001706 | 0.001749 |
| return_function_shared.test | RUN | 0.160263 | 0.435726 | 0.542624 | 0.001661 | 0.001815 | 0.001856 |
| return_function_static.test | RUN | 0.158790 | 0.440039 | 0.535839 | 0.001599 | 0.001552 | 0.001584 |
| return_function_shared_0x.test | RUN | 0.164478 | 0.447467 | 0.541282 | 0.001710 | 0.001960 | 0.001871 |
| return_function_static_0x.test | RUN | 0.157720 | 0.427668 | 0.507651 | 0.001560 | 0.001685 | 0.001608 |
| return_function_shared_14.test | RUN | 0.157709 | 0.442818 | 0.531986 | 0.001423 | 0.001587 | 0.002013 |
| return_function_static_14.test | RUN | 0.164737 | 0.433058 | 0.515469 | 0.001729 | 0.001780 | 0.001818 |
| quick.test | RUN | 0.164054 | 0.437885 | 0.559740 | 0.001736 | 0.001822 | 0.001679 |
| issue_42.test | COMPILE | 0.126633 | 0.291845 | 0.343026 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.211687 | 0.606261 | 0.800940 | 0.001574 | 0.001527 | 0.001427 |
| contains_test_no_rtti.test | RUN | 0.188924 | 0.686630 | 0.738926 | 0.001580 | 0.001689 | 0.001312 |
| contains2_test_no_rtti.test | RUN | 0.174069 | 0.513472 | 0.601782 | 0.001502 | 0.001198 | 0.001650 |
| contains3_test.test | RUN | 0.163567 | 0.393560 | 0.545579 | 0.001667 | 0.001584 | 0.001442 |
| contains3_test_no_rtti.test | RUN | 0.124886 | 0.335424 | 0.347968 | 0.001306 | 0.001287 | 0.001273 |
| issue_53.test | COMPILE | 0.100858 | 0.240645 | 0.257804 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 0.998×, 1.004× | 0.989× |
| clang | 1.007× | 1.009×, 1.004× | 1.012× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000967s | 0.000972s | 0.000981s | byte equality observed |
| contains2_test.test | 10 | 0.000954s | 0.000969s | 0.000959s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000961s | 0.001004s | 0.000996s | byte equality observed |
| contains3_test.test | 10 | 0.000968s | 0.001013s | 0.000986s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000964s | 0.000973s | 0.001025s | byte equality observed |
| contains_test.test | 10 | 0.000953s | 0.000999s | 0.001000s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000964s | 0.001033s | 0.000991s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000965s | 0.001012s | 0.000999s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001012s | 0.001002s | 0.001010s | byte equality observed |
| function_arith_portable.test | 10 | 0.001007s | 0.001021s | 0.001048s | byte equality observed |
| function_n_test.test | 10 | 0.001025s | 0.001017s | 0.001010s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000992s | 0.000995s | 0.001012s | byte equality observed |
| function_ref_portable.test | 10 | 0.001004s | 0.000968s | 0.000956s | byte equality observed |
| function_test.test | 10 | 0.001060s | 0.001048s | 0.001034s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001025s | 0.001060s | 0.001027s | byte equality observed |
| lambda_test.test | 10 | 0.000983s | 0.000995s | 0.001011s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001005s | 0.001020s | 0.000999s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000995s | 0.000970s | 0.001030s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001102s | 0.001090s | 0.001058s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001129s | 0.001059s | 0.001096s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001082s | 0.001076s | 0.001071s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000991s | 0.001016s | 0.001004s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000990s | 0.000990s | 0.001013s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000982s | 0.001002s | 0.001002s | byte equality observed |
| nothrow_swap.test | 10 | 0.000998s | 0.001006s | 0.001017s | byte equality observed |
| quick.test | 10 | 0.000997s | 0.001015s | 0.000997s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000982s | 0.001002s | 0.001027s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000994s | 0.000988s | 0.000998s | byte equality observed |
| return_function_shared.test | 10 | 0.001070s | 0.001103s | 0.001090s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001082s | 0.001042s | 0.001128s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001067s | 0.001071s | 0.001048s | byte equality observed |
| return_function_static.test | 10 | 0.001011s | 0.000993s | 0.001029s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000998s | 0.000995s | 0.001001s | byte equality observed |
| return_function_static_14.test | 10 | 0.001001s | 0.001005s | 0.001012s | byte equality observed |
| rvalues_test.test | 10 | 0.000988s | 0.000990s | 0.001006s | byte equality observed |
| stateless_test.test | 10 | 0.000989s | 0.000998s | 0.000989s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000971s | 0.001001s | 0.000985s | byte equality observed |
| std_bind_portable.test | 10 | 0.001033s | 0.000983s | 0.000978s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000982s | 0.000986s | 0.000936s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000988s | 0.000981s | 0.000975s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001112s | 0.001107s | 0.001114s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001014s | 0.001030s | 0.001011s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
