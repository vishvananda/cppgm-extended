# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.365481 | 1.967753 | 1.964351 | 0.001882 | 0.002325 | 0.002026 |
| function_test_no_rtti.test | RUN | 0.372641 | 2.008954 | 1.978884 | 0.001930 | 0.002140 | 0.001968 |
| function_n_test.test | RUN | 0.338232 | 1.618013 | 1.675994 | 0.001798 | 0.001839 | 0.002020 |
| allocator_test.test | RUN | 0.279979 | 0.961881 | 1.190865 | 0.001849 | 0.002057 | 0.002038 |
| stateless_test.test | RUN | 0.212503 | 0.528425 | 0.729718 | 0.001790 | 0.001917 | 0.002047 |
| lambda_test.test | RUN | 0.461287 | 1.287679 | 1.630606 | 0.002050 | 0.002172 | 0.002070 |
| function_test_fail1.test | COMPILE_FAIL | 0.164332 | 0.357745 | 0.556988 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.167668 | 0.372450 | 0.542339 | — | — | — |
| function_30.test | COMPILE | 0.174171 | 0.398101 | 0.573786 | — | — | — |
| function_30_repeat.test | COMPILE | 0.171979 | 0.392660 | 0.583558 | — | — | — |
| function_arith_cxx98.test | RUN | 0.221451 | 0.652468 | 0.851195 | 0.001932 | 0.001974 | 0.001881 |
| function_arith_portable.test | RUN | 0.220665 | 0.604065 | 0.781484 | 0.002080 | 0.002014 | 0.002010 |
| sum_avg_cxx98.test | RUN | 0.215596 | 0.539403 | 0.735759 | 0.001920 | 0.001920 | 0.001880 |
| sum_avg_portable.test | RUN | 0.209318 | 0.480502 | 0.727220 | 0.001808 | 0.002142 | 0.001988 |
| mem_fun_cxx98.test | RUN | 0.243784 | 0.747425 | 0.941833 | 0.001864 | 0.002040 | 0.001876 |
| mem_fun_portable.test | RUN | 0.232618 | 0.669753 | 0.896986 | 0.002056 | 0.001990 | 0.002034 |
| std_bind_cxx98.test | RUN | 0.225302 | 0.628486 | 0.815776 | 0.001833 | 0.001700 | 0.001881 |
| std_bind_portable.test | RUN | 0.230079 | 0.569959 | 0.766544 | 0.001806 | 0.002220 | 0.001952 |
| function_ref_cxx98.test | RUN | 0.210735 | 0.514139 | 0.761491 | 0.001944 | 0.002012 | 0.001614 |
| function_ref_portable.test | RUN | 0.204389 | 0.470352 | 0.750326 | 0.001696 | 0.001862 | 0.001978 |
| contains_test.test | RUN | 0.263598 | 0.962594 | 1.134518 | 0.001916 | 0.002100 | 0.001887 |
| contains2_test.test | RUN | 0.238645 | 0.709428 | 0.901056 | 0.001854 | 0.002124 | 0.001682 |
| nothrow_swap.test | RUN | 0.223646 | 0.657778 | 0.875289 | 0.001786 | 0.002163 | 0.002104 |
| rvalues_test.test | RUN | 0.261975 | 0.817245 | 1.013621 | 0.001836 | 0.001949 | 0.002061 |
| function_typeof_test.test | COMPILE | 0.164815 | 0.371190 | 0.570809 | — | — | — |
| result_arg_types_test.test | RUN | 0.244263 | 0.578740 | 0.876178 | 0.002119 | 0.001927 | 0.001851 |
| result_arg_n_types_test.test | RUN | 0.251971 | 0.617274 | 1.081849 | 0.001782 | 0.002182 | 0.001965 |
| test_bad_function_call_shared.test | RUN | 0.219543 | 0.537015 | 0.776421 | 0.002060 | 0.002020 | 0.001918 |
| test_bad_function_call_static.test | RUN | 0.212552 | 0.518289 | 0.705167 | 0.001706 | 0.002062 | 0.002012 |
| mixed_cxxstd_shared.test | RUN | 0.222395 | 0.637686 | 0.876849 | 0.002072 | 0.002146 | 0.002216 |
| mixed_cxxstd_static.test | RUN | 0.221150 | 0.620313 | 0.838501 | 0.002043 | 0.001928 | 0.002074 |
| mixed_cxxstd_shared_0x.test | RUN | 0.222069 | 0.617345 | 0.888710 | 0.002214 | 0.002340 | 0.002219 |
| mixed_cxxstd_static_0x.test | RUN | 0.223361 | 0.605713 | 0.769543 | 0.001804 | 0.001888 | 0.002041 |
| mixed_cxxstd_shared_14.test | RUN | 0.235924 | 0.611489 | 0.888880 | 0.002160 | 0.001954 | 0.002265 |
| mixed_cxxstd_static_14.test | RUN | 0.226351 | 0.630227 | 0.846262 | 0.002028 | 0.002316 | 0.001996 |
| return_function_shared.test | RUN | 0.219320 | 0.604877 | 0.869530 | 0.002089 | 0.001884 | 0.002205 |
| return_function_static.test | RUN | 0.220326 | 0.605446 | 0.839863 | 0.001704 | 0.001930 | 0.002064 |
| return_function_shared_0x.test | RUN | 0.222496 | 0.615355 | 0.846284 | 0.001894 | 0.002353 | 0.002239 |
| return_function_static_0x.test | RUN | 0.222827 | 0.597609 | 0.812763 | 0.001892 | 0.001946 | 0.002017 |
| return_function_shared_14.test | RUN | 0.219208 | 0.613840 | 0.861698 | 0.002118 | 0.002360 | 0.002229 |
| return_function_static_14.test | RUN | 0.220361 | 0.607533 | 0.870980 | 0.001915 | 0.001943 | 0.002053 |
| quick.test | RUN | 0.230130 | 0.620955 | 0.877070 | 0.001946 | 0.001824 | 0.002034 |
| issue_42.test | COMPILE | 0.182163 | 0.419791 | 0.532264 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.290328 | 0.845301 | 1.220098 | 0.002008 | 0.002151 | 0.001729 |
| contains_test_no_rtti.test | RUN | 0.261548 | 0.972455 | 1.148519 | 0.001808 | 0.001674 | 0.001415 |
| contains2_test_no_rtti.test | RUN | 0.235808 | 0.687808 | 0.945560 | 0.001754 | 0.002034 | 0.001841 |
| contains3_test.test | RUN | 0.218484 | 0.570225 | 0.800722 | 0.001808 | 0.001718 | 0.001888 |
| contains3_test_no_rtti.test | RUN | 0.160885 | 0.443982 | 0.530133 | 0.001576 | 0.001453 | 0.001364 |
| issue_53.test | COMPILE | 0.124722 | 0.279995 | 0.392480 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.005×, 1.004× | 0.999× |
| clang | 0.995× | 1.009×, 0.981× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001171s | 0.001147s | 0.001159s | byte equality observed |
| contains2_test.test | 10 | 0.001160s | 0.001149s | 0.001134s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001166s | 0.001142s | 0.001136s | byte equality observed |
| contains3_test.test | 10 | 0.001159s | 0.001154s | 0.001141s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001155s | 0.001152s | 0.001187s | byte equality observed |
| contains_test.test | 10 | 0.001183s | 0.001179s | 0.001238s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001182s | 0.001199s | 0.001278s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001215s | 0.001275s | 0.001285s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001324s | 0.001283s | 0.001252s | byte equality observed |
| function_arith_portable.test | 10 | 0.001293s | 0.001275s | 0.001210s | byte equality observed |
| function_n_test.test | 10 | 0.001264s | 0.001317s | 0.001230s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001274s | 0.001229s | 0.001194s | byte equality observed |
| function_ref_portable.test | 10 | 0.001256s | 0.001210s | 0.001222s | byte equality observed |
| function_test.test | 10 | 0.001310s | 0.001273s | 0.001310s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001374s | 0.001285s | 0.001311s | byte equality observed |
| lambda_test.test | 10 | 0.001252s | 0.001238s | 0.001253s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001270s | 0.001247s | 0.001288s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001239s | 0.001305s | 0.001257s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001318s | 0.001312s | 0.001394s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001320s | 0.001314s | 0.001392s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001353s | 0.001332s | 0.001370s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001301s | 0.001222s | 0.001282s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001252s | 0.001238s | 0.001277s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001231s | 0.001223s | 0.001246s | byte equality observed |
| nothrow_swap.test | 10 | 0.001257s | 0.001313s | 0.001263s | byte equality observed |
| quick.test | 10 | 0.001233s | 0.001237s | 0.001311s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001273s | 0.001234s | 0.001239s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001233s | 0.001283s | 0.001230s | byte equality observed |
| return_function_shared.test | 10 | 0.001353s | 0.001359s | 0.001349s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001336s | 0.001388s | 0.001340s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001313s | 0.001372s | 0.001353s | byte equality observed |
| return_function_static.test | 10 | 0.001220s | 0.001280s | 0.001224s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001276s | 0.001255s | 0.001263s | byte equality observed |
| return_function_static_14.test | 10 | 0.001255s | 0.001268s | 0.001242s | byte equality observed |
| rvalues_test.test | 10 | 0.001276s | 0.001279s | 0.001246s | byte equality observed |
| stateless_test.test | 10 | 0.001255s | 0.001226s | 0.001256s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001235s | 0.001197s | 0.001280s | byte equality observed |
| std_bind_portable.test | 10 | 0.001220s | 0.001281s | 0.001247s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001244s | 0.001235s | 0.001218s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001198s | 0.001247s | 0.001236s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001359s | 0.001344s | 0.001382s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001272s | 0.001240s | 0.001328s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
