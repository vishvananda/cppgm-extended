# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.267049 | 1.397134 | 1.127393 | 0.001728 | 0.002104 | 0.001895 |
| function_test_no_rtti.test | RUN | 0.266279 | 1.413551 | 1.139038 | 0.001442 | 0.002044 | 0.001457 |
| function_n_test.test | RUN | 0.250162 | 1.182309 | 0.953856 | 0.001944 | 0.001652 | 0.001996 |
| allocator_test.test | RUN | 0.199766 | 0.668583 | 0.627545 | 0.001872 | 0.001999 | 0.001903 |
| stateless_test.test | RUN | 0.161788 | 0.361421 | 0.394953 | 0.001842 | 0.001806 | 0.002000 |
| lambda_test.test | RUN | 0.346192 | 0.844594 | 0.873332 | 0.001887 | 0.002102 | 0.001844 |
| function_test_fail1.test | COMPILE_FAIL | 0.125085 | 0.236736 | 0.267862 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.128231 | 0.241704 | 0.264559 | — | — | — |
| function_30.test | COMPILE | 0.130269 | 0.249048 | 0.287862 | — | — | — |
| function_30_repeat.test | COMPILE | 0.129693 | 0.241385 | 0.293430 | — | — | — |
| function_arith_cxx98.test | RUN | 0.171503 | 0.428123 | 0.465579 | 0.002160 | 0.001718 | 0.001920 |
| function_arith_portable.test | RUN | 0.162445 | 0.391556 | 0.444159 | 0.001696 | 0.001671 | 0.001888 |
| sum_avg_cxx98.test | RUN | 0.161112 | 0.348255 | 0.392507 | 0.001638 | 0.001892 | 0.001840 |
| sum_avg_portable.test | RUN | 0.161288 | 0.346045 | 0.365430 | 0.001848 | 0.001974 | 0.001878 |
| mem_fun_cxx98.test | RUN | 0.182027 | 0.516033 | 0.479481 | 0.001853 | 0.002042 | 0.001700 |
| mem_fun_portable.test | RUN | 0.180914 | 0.471240 | 0.436820 | 0.001896 | 0.001595 | 0.001674 |
| std_bind_cxx98.test | RUN | 0.171638 | 0.428382 | 0.440549 | 0.001246 | 0.001950 | 0.001680 |
| std_bind_portable.test | RUN | 0.172213 | 0.392449 | 0.384732 | 0.001982 | 0.001812 | 0.001840 |
| function_ref_cxx98.test | RUN | 0.168549 | 0.344909 | 0.371216 | 0.001948 | 0.001820 | 0.001893 |
| function_ref_portable.test | RUN | 0.171305 | 0.312411 | 0.370318 | 0.001801 | 0.001562 | 0.001710 |
| contains_test.test | RUN | 0.194340 | 0.638581 | 0.556593 | 0.001643 | 0.001722 | 0.001692 |
| contains2_test.test | RUN | 0.179164 | 0.468852 | 0.452679 | 0.001846 | 0.002048 | 0.001546 |
| nothrow_swap.test | RUN | 0.176203 | 0.431652 | 0.431066 | 0.001749 | 0.001994 | 0.001854 |
| rvalues_test.test | RUN | 0.200183 | 0.553032 | 0.485069 | 0.001637 | 0.001878 | 0.001904 |
| function_typeof_test.test | COMPILE | 0.134728 | 0.246704 | 0.293053 | — | — | — |
| result_arg_types_test.test | RUN | 0.177483 | 0.364481 | 0.480737 | 0.001589 | 0.001843 | 0.001608 |
| result_arg_n_types_test.test | RUN | 0.198068 | 0.405802 | 0.556330 | 0.001822 | 0.002004 | 0.001870 |
| test_bad_function_call_shared.test | RUN | 0.168015 | 0.351633 | 0.402259 | 0.001796 | 0.002093 | 0.001920 |
| test_bad_function_call_static.test | RUN | 0.171793 | 0.332182 | 0.378031 | 0.001886 | 0.002112 | 0.001608 |
| mixed_cxxstd_shared.test | RUN | 0.174926 | 0.406420 | 0.425809 | 0.001899 | 0.001838 | 0.001655 |
| mixed_cxxstd_static.test | RUN | 0.178746 | 0.409949 | 0.443904 | 0.001951 | 0.001705 | 0.001948 |
| mixed_cxxstd_shared_0x.test | RUN | 0.178272 | 0.409916 | 0.446274 | 0.001854 | 0.002373 | 0.002062 |
| mixed_cxxstd_static_0x.test | RUN | 0.172285 | 0.421301 | 0.430030 | 0.001608 | 0.001924 | 0.001898 |
| mixed_cxxstd_shared_14.test | RUN | 0.198419 | 0.427414 | 0.436223 | 0.002200 | 0.001977 | 0.001814 |
| mixed_cxxstd_static_14.test | RUN | 0.195712 | 0.413743 | 0.428654 | 0.001731 | 0.001918 | 0.001848 |
| return_function_shared.test | RUN | 0.170349 | 0.403220 | 0.438792 | 0.001641 | 0.002238 | 0.002012 |
| return_function_static.test | RUN | 0.167991 | 0.421600 | 0.443485 | 0.002028 | 0.001590 | 0.001662 |
| return_function_shared_0x.test | RUN | 0.168044 | 0.430550 | 0.458676 | 0.001777 | 0.002030 | 0.002265 |
| return_function_static_0x.test | RUN | 0.168238 | 0.414269 | 0.438409 | 0.001758 | 0.001812 | 0.002060 |
| return_function_shared_14.test | RUN | 0.166335 | 0.449951 | 0.460278 | 0.002235 | 0.001871 | 0.001838 |
| return_function_static_14.test | RUN | 0.171012 | 0.428931 | 0.436135 | 0.001684 | 0.001879 | 0.001694 |
| quick.test | RUN | 0.172186 | 0.408400 | 0.422450 | 0.001849 | 0.002254 | 0.001852 |
| issue_42.test | COMPILE | 0.135440 | 0.257225 | 0.293267 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.220971 | 0.564763 | 0.643960 | 0.001854 | 0.001845 | 0.001998 |
| contains_test_no_rtti.test | RUN | 0.189970 | 0.644256 | 0.602949 | 0.001639 | 0.001923 | 0.001556 |
| contains2_test_no_rtti.test | RUN | 0.176421 | 0.490244 | 0.479452 | 0.001752 | 0.001698 | 0.001790 |
| contains3_test.test | RUN | 0.170298 | 0.369561 | 0.419522 | 0.001526 | 0.001896 | 0.001628 |
| contains3_test_no_rtti.test | RUN | 0.131035 | 0.318173 | 0.271009 | 0.001385 | 0.001491 | 0.001344 |
| issue_53.test | COMPILE | 0.116292 | 0.213828 | 0.213381 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.035×† | 1.067×, 1.004× | 0.994× |
| clang | 1.000× | 1.001×, 1.000× | 0.980× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000958s | 0.000961s | 0.000979s | byte equality observed |
| contains2_test.test | 10 | 0.000958s | 0.000953s | 0.000982s | byte equality observed |
| contains2_test_no_rtti.test | 8 | 0.000980s | 0.000978s | 0.000967s | byte equality observed |
| contains3_test.test | 8 | 0.000930s | 0.000973s | 0.000963s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000969s | 0.000996s | 0.000966s | byte equality observed |
| contains_test.test | 10 | 0.000958s | 0.000965s | 0.000956s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000970s | 0.000978s | 0.000971s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000961s | 0.000974s | 0.000969s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000976s | 0.000993s | 0.000989s | byte equality observed |
| function_arith_portable.test | 10 | 0.000992s | 0.000994s | 0.000980s | byte equality observed |
| function_n_test.test | 10 | 0.000984s | 0.000976s | 0.000979s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000914s | 0.000946s | 0.000950s | byte equality observed |
| function_ref_portable.test | 10 | 0.000930s | 0.000943s | 0.000966s | byte equality observed |
| function_test.test | 10 | 0.001019s | 0.001014s | 0.001014s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.000992s | 0.001022s | 0.001053s | byte equality observed |
| lambda_test.test | 10 | 0.000975s | 0.000982s | 0.001007s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000943s | 0.000964s | 0.000986s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000938s | 0.000968s | 0.000986s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001021s | 0.001050s | 0.001052s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001033s | 0.001038s | 0.001064s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001033s | 0.001064s | 0.001058s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000947s | 0.000984s | 0.000989s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000952s | 0.000968s | 0.000975s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000967s | 0.000976s | 0.000979s | byte equality observed |
| nothrow_swap.test | 10 | 0.000961s | 0.000962s | 0.000971s | byte equality observed |
| quick.test | 10 | 0.000940s | 0.000977s | 0.000973s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000940s | 0.000949s | 0.000960s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000947s | 0.000934s | 0.000976s | byte equality observed |
| return_function_shared.test | 10 | 0.001054s | 0.001031s | 0.001075s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001015s | 0.001041s | 0.001060s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001019s | 0.001018s | 0.001053s | byte equality observed |
| return_function_static.test | 10 | 0.000937s | 0.000955s | 0.001001s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000945s | 0.000978s | 0.001005s | byte equality observed |
| return_function_static_14.test | 10 | 0.000950s | 0.000946s | 0.000992s | byte equality observed |
| rvalues_test.test | 10 | 0.000955s | 0.000957s | 0.000969s | byte equality observed |
| stateless_test.test | 10 | 0.000956s | 0.000987s | 0.000991s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000939s | 0.000937s | 0.000952s | byte equality observed |
| std_bind_portable.test | 10 | 0.000932s | 0.000933s | 0.000924s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000926s | 0.000932s | 0.000942s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000937s | 0.000949s | 0.000938s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001056s | 0.001072s | 0.001075s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000969s | 0.000994s | 0.000986s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
