# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.300078 | 1.467561 | 1.328821 | 0.001464 | 0.001781 | 0.001845 |
| function_test_no_rtti.test | RUN | 0.299602 | 1.452476 | 1.357270 | 0.001530 | 0.001942 | 0.001567 |
| function_n_test.test | RUN | 0.283512 | 1.207042 | 1.138721 | 0.001560 | 0.001926 | 0.001844 |
| allocator_test.test | RUN | 0.227248 | 0.678315 | 0.792216 | 0.001841 | 0.001749 | 0.001867 |
| stateless_test.test | RUN | 0.182693 | 0.371502 | 0.495787 | 0.001669 | 0.001693 | 0.001812 |
| lambda_test.test | RUN | 0.395457 | 0.923911 | 1.088358 | 0.001782 | 0.001830 | 0.001669 |
| function_test_fail1.test | COMPILE_FAIL | 0.143193 | 0.255814 | 0.346349 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.141068 | 0.248598 | 0.355498 | — | — | — |
| function_30.test | COMPILE | 0.145327 | 0.277439 | 0.361239 | — | — | — |
| function_30_repeat.test | COMPILE | 0.149195 | 0.262873 | 0.380710 | — | — | — |
| function_arith_cxx98.test | RUN | 0.183459 | 0.457814 | 0.549352 | 0.001796 | 0.001551 | 0.001686 |
| function_arith_portable.test | RUN | 0.181241 | 0.431392 | 0.513741 | 0.001635 | 0.001445 | 0.001809 |
| sum_avg_cxx98.test | RUN | 0.174061 | 0.384594 | 0.471174 | 0.001774 | 0.001701 | 0.001693 |
| sum_avg_portable.test | RUN | 0.174589 | 0.344451 | 0.459465 | 0.001673 | 0.001813 | 0.001840 |
| mem_fun_cxx98.test | RUN | 0.194535 | 0.530161 | 0.617045 | 0.001666 | 0.001823 | 0.001495 |
| mem_fun_portable.test | RUN | 0.193529 | 0.476017 | 0.589585 | 0.001581 | 0.001765 | 0.001596 |
| std_bind_cxx98.test | RUN | 0.181915 | 0.453580 | 0.519693 | 0.001718 | 0.001496 | 0.001803 |
| std_bind_portable.test | RUN | 0.186820 | 0.402545 | 0.492818 | 0.001563 | 0.001893 | 0.001774 |
| function_ref_cxx98.test | RUN | 0.179534 | 0.369394 | 0.489761 | 0.001584 | 0.001756 | 0.001477 |
| function_ref_portable.test | RUN | 0.177578 | 0.351479 | 0.500065 | 0.001770 | 0.001543 | 0.001742 |
| contains_test.test | RUN | 0.209619 | 0.680006 | 0.727468 | 0.001626 | 0.001812 | 0.001736 |
| contains2_test.test | RUN | 0.201699 | 0.486704 | 0.548554 | 0.001828 | 0.001944 | 0.001622 |
| nothrow_swap.test | RUN | 0.195368 | 0.469628 | 0.540582 | 0.001686 | 0.001429 | 0.001832 |
| rvalues_test.test | RUN | 0.223147 | 0.575745 | 0.634196 | 0.001907 | 0.001890 | 0.001802 |
| function_typeof_test.test | COMPILE | 0.149054 | 0.264454 | 0.325770 | — | — | — |
| result_arg_types_test.test | RUN | 0.197650 | 0.395519 | 0.613179 | 0.001785 | 0.001855 | 0.001890 |
| result_arg_n_types_test.test | RUN | 0.210934 | 0.442604 | 0.683376 | 0.001429 | 0.001927 | 0.001921 |
| test_bad_function_call_shared.test | RUN | 0.180201 | 0.365792 | 0.510470 | 0.002042 | 0.002026 | 0.001902 |
| test_bad_function_call_static.test | RUN | 0.178818 | 0.366486 | 0.515916 | 0.001800 | 0.001792 | 0.002014 |
| mixed_cxxstd_shared.test | RUN | 0.190832 | 0.435545 | 0.536762 | 0.001445 | 0.001723 | 0.001694 |
| mixed_cxxstd_static.test | RUN | 0.189333 | 0.443498 | 0.567859 | 0.001687 | 0.001790 | 0.001669 |
| mixed_cxxstd_shared_0x.test | RUN | 0.186914 | 0.439631 | 0.570208 | 0.001846 | 0.001918 | 0.002081 |
| mixed_cxxstd_static_0x.test | RUN | 0.190677 | 0.443470 | 0.568248 | 0.001766 | 0.001811 | 0.001529 |
| mixed_cxxstd_shared_14.test | RUN | 0.211209 | 0.438393 | 0.568355 | 0.001762 | 0.001733 | 0.001740 |
| mixed_cxxstd_static_14.test | RUN | 0.199439 | 0.436909 | 0.577944 | 0.001783 | 0.001922 | 0.001746 |
| return_function_shared.test | RUN | 0.185544 | 0.435760 | 0.553870 | 0.001770 | 0.001712 | 0.002148 |
| return_function_static.test | RUN | 0.187785 | 0.437065 | 0.571845 | 0.001480 | 0.001626 | 0.001590 |
| return_function_shared_0x.test | RUN | 0.185882 | 0.448442 | 0.573743 | 0.001830 | 0.002064 | 0.002057 |
| return_function_static_0x.test | RUN | 0.183857 | 0.431134 | 0.526355 | 0.001730 | 0.001824 | 0.001851 |
| return_function_shared_14.test | RUN | 0.181556 | 0.443212 | 0.545595 | 0.001760 | 0.001768 | 0.001788 |
| return_function_static_14.test | RUN | 0.182265 | 0.439287 | 0.540372 | 0.001548 | 0.001938 | 0.001800 |
| quick.test | RUN | 0.184296 | 0.449406 | 0.542109 | 0.001647 | 0.001807 | 0.001863 |
| issue_42.test | COMPILE | 0.151868 | 0.297665 | 0.395460 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.247912 | 0.617927 | 0.819617 | 0.001591 | 0.001592 | 0.001761 |
| contains_test_no_rtti.test | RUN | 0.207126 | 0.684261 | 0.756027 | 0.001605 | 0.001753 | 0.001292 |
| contains2_test_no_rtti.test | RUN | 0.192464 | 0.513790 | 0.616678 | 0.001778 | 0.001851 | 0.001580 |
| contains3_test.test | RUN | 0.193728 | 0.399038 | 0.529586 | 0.001608 | 0.001798 | 0.001585 |
| contains3_test_no_rtti.test | RUN | 0.139064 | 0.314855 | 0.346740 | 0.001284 | 0.001389 | 0.001368 |
| issue_53.test | COMPILE | 0.118049 | 0.219822 | 0.263975 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.995×, 0.986× | 0.995× |
| clang | 0.984× | 1.003×, 0.966× | 1.009× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001033s | 0.001033s | 0.000998s | byte equality observed |
| contains2_test.test | 10 | 0.001030s | 0.001018s | 0.001015s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001022s | 0.001022s | 0.001050s | byte equality observed |
| contains3_test.test | 10 | 0.001022s | 0.001020s | 0.001062s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001038s | 0.001010s | 0.001028s | byte equality observed |
| contains_test.test | 10 | 0.001086s | 0.001025s | 0.001045s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001050s | 0.001048s | 0.001041s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001057s | 0.001059s | 0.001031s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001080s | 0.001070s | 0.001074s | byte equality observed |
| function_arith_portable.test | 10 | 0.001099s | 0.001078s | 0.001089s | byte equality observed |
| function_n_test.test | 10 | 0.001053s | 0.001060s | 0.001022s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001019s | 0.001009s | 0.001003s | byte equality observed |
| function_ref_portable.test | 10 | 0.001035s | 0.001042s | 0.000981s | byte equality observed |
| function_test.test | 10 | 0.001108s | 0.001073s | 0.001071s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001105s | 0.001068s | 0.001105s | byte equality observed |
| lambda_test.test | 10 | 0.001042s | 0.001011s | 0.001023s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001079s | 0.001015s | 0.001084s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001052s | 0.001029s | 0.001056s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001136s | 0.001094s | 0.001123s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001128s | 0.001112s | 0.001113s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001105s | 0.001101s | 0.001119s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001056s | 0.001067s | 0.001029s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001030s | 0.001034s | 0.001036s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001025s | 0.001048s | 0.001038s | byte equality observed |
| nothrow_swap.test | 10 | 0.001039s | 0.001088s | 0.001060s | byte equality observed |
| quick.test | 10 | 0.001058s | 0.001061s | 0.001003s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001036s | 0.001030s | 0.001041s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001011s | 0.001043s | 0.001017s | byte equality observed |
| return_function_shared.test | 10 | 0.001105s | 0.001102s | 0.001102s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001111s | 0.001116s | 0.001136s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001131s | 0.001123s | 0.001151s | byte equality observed |
| return_function_static.test | 10 | 0.001058s | 0.001072s | 0.001028s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001060s | 0.001090s | 0.001053s | byte equality observed |
| return_function_static_14.test | 10 | 0.001045s | 0.001053s | 0.001013s | byte equality observed |
| rvalues_test.test | 10 | 0.001056s | 0.001057s | 0.001031s | byte equality observed |
| stateless_test.test | 10 | 0.001028s | 0.001056s | 0.001008s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001023s | 0.001017s | 0.001021s | byte equality observed |
| std_bind_portable.test | 10 | 0.001007s | 0.001008s | 0.001008s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001031s | 0.001004s | 0.001039s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001011s | 0.000989s | 0.000993s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001166s | 0.001160s | 0.001143s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001053s | 0.001053s | 0.001053s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
