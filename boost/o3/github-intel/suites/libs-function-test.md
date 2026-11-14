# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.233475 | 1.343946 | 1.129938 | 0.001070 | 0.001184 | 0.001278 |
| function_test_no_rtti.test | RUN | 0.230934 | 1.304157 | 1.132094 | 0.001140 | 0.001340 | 0.000949 |
| function_n_test.test | RUN | 0.216539 | 1.089748 | 0.954112 | 0.001192 | 0.001183 | 0.001207 |
| allocator_test.test | RUN | 0.175027 | 0.607516 | 0.625317 | 0.001318 | 0.001228 | 0.001248 |
| stateless_test.test | RUN | 0.126804 | 0.318279 | 0.393135 | 0.001111 | 0.001255 | 0.001201 |
| lambda_test.test | RUN | 0.273181 | 0.792149 | 0.855742 | 0.001150 | 0.001299 | 0.001206 |
| function_test_fail1.test | COMPILE_FAIL | 0.095663 | 0.208447 | 0.260340 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.097393 | 0.211845 | 0.267566 | — | — | — |
| function_30.test | COMPILE | 0.101867 | 0.221808 | 0.273442 | — | — | — |
| function_30_repeat.test | COMPILE | 0.098212 | 0.224364 | 0.273103 | — | — | — |
| function_arith_cxx98.test | RUN | 0.130756 | 0.393471 | 0.433093 | 0.001329 | 0.001112 | 0.001123 |
| function_arith_portable.test | RUN | 0.129316 | 0.364194 | 0.397396 | 0.001082 | 0.001226 | 0.001169 |
| sum_avg_cxx98.test | RUN | 0.125490 | 0.309342 | 0.382621 | 0.001107 | 0.000962 | 0.001227 |
| sum_avg_portable.test | RUN | 0.122871 | 0.286144 | 0.365964 | 0.001092 | 0.001154 | 0.001226 |
| mem_fun_cxx98.test | RUN | 0.142393 | 0.455691 | 0.485932 | 0.001259 | 0.001140 | 0.001149 |
| mem_fun_portable.test | RUN | 0.137553 | 0.410282 | 0.456719 | 0.001178 | 0.001093 | 0.001112 |
| std_bind_cxx98.test | RUN | 0.130542 | 0.374072 | 0.411778 | 0.001170 | 0.001180 | 0.001104 |
| std_bind_portable.test | RUN | 0.129762 | 0.343952 | 0.388554 | 0.001091 | 0.001146 | 0.001214 |
| function_ref_cxx98.test | RUN | 0.123765 | 0.306207 | 0.382113 | 0.001223 | 0.001190 | 0.001086 |
| function_ref_portable.test | RUN | 0.121416 | 0.286769 | 0.367972 | 0.001122 | 0.001067 | 0.001106 |
| contains_test.test | RUN | 0.157465 | 0.610558 | 0.603000 | 0.001095 | 0.001203 | 0.001324 |
| contains2_test.test | RUN | 0.141411 | 0.436151 | 0.469690 | 0.001119 | 0.001120 | 0.001119 |
| nothrow_swap.test | RUN | 0.132440 | 0.408566 | 0.453992 | 0.001207 | 0.001188 | 0.001168 |
| rvalues_test.test | RUN | 0.152824 | 0.502509 | 0.522172 | 0.001085 | 0.001130 | 0.001103 |
| function_typeof_test.test | COMPILE | 0.098452 | 0.215716 | 0.276394 | — | — | — |
| result_arg_types_test.test | RUN | 0.138254 | 0.342888 | 0.472003 | 0.001024 | 0.001091 | 0.001176 |
| result_arg_n_types_test.test | RUN | 0.153759 | 0.375081 | 0.537923 | 0.001109 | 0.001144 | 0.001183 |
| test_bad_function_call_shared.test | RUN | 0.117720 | 0.313235 | 0.378455 | 0.001185 | 0.001335 | 0.001266 |
| test_bad_function_call_static.test | RUN | 0.121586 | 0.312017 | 0.383955 | 0.001044 | 0.001149 | 0.001301 |
| mixed_cxxstd_shared.test | RUN | 0.135481 | 0.370381 | 0.428911 | 0.001401 | 0.001574 | 0.001449 |
| mixed_cxxstd_static.test | RUN | 0.134018 | 0.381326 | 0.436531 | 0.001115 | 0.001370 | 0.001179 |
| mixed_cxxstd_shared_0x.test | RUN | 0.131472 | 0.369718 | 0.438529 | 0.001170 | 0.001302 | 0.001323 |
| mixed_cxxstd_static_0x.test | RUN | 0.135514 | 0.370131 | 0.439378 | 0.001245 | 0.001189 | 0.001180 |
| mixed_cxxstd_shared_14.test | RUN | 0.132840 | 0.377383 | 0.439006 | 0.001143 | 0.001272 | 0.001412 |
| mixed_cxxstd_static_14.test | RUN | 0.133438 | 0.376658 | 0.434642 | 0.000921 | 0.001124 | 0.001288 |
| return_function_shared.test | RUN | 0.129112 | 0.378885 | 0.433796 | 0.001256 | 0.001196 | 0.001105 |
| return_function_static.test | RUN | 0.129478 | 0.379467 | 0.439008 | 0.001090 | 0.001146 | 0.001003 |
| return_function_shared_0x.test | RUN | 0.136362 | 0.379752 | 0.414244 | 0.001160 | 0.001329 | 0.001336 |
| return_function_static_0x.test | RUN | 0.133039 | 0.371389 | 0.423082 | 0.001019 | 0.001124 | 0.001125 |
| return_function_shared_14.test | RUN | 0.130481 | 0.376494 | 0.424728 | 0.001131 | 0.001201 | 0.001232 |
| return_function_static_14.test | RUN | 0.133268 | 0.372585 | 0.426777 | 0.001057 | 0.001088 | 0.001156 |
| quick.test | RUN | 0.134591 | 0.375936 | 0.421444 | 0.001085 | 0.001075 | 0.001293 |
| issue_42.test | COMPILE | 0.102608 | 0.243090 | 0.289527 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.173388 | 0.518668 | 0.621967 | 0.001200 | 0.001247 | 0.001195 |
| contains_test_no_rtti.test | RUN | 0.156049 | 0.608951 | 0.605696 | 0.001108 | 0.001259 | 0.001162 |
| contains2_test_no_rtti.test | RUN | 0.140484 | 0.430789 | 0.464107 | 0.000984 | 0.001223 | 0.001111 |
| contains3_test.test | RUN | 0.130777 | 0.334719 | 0.412229 | 0.001057 | 0.001285 | 0.001359 |
| contains3_test_no_rtti.test | RUN | 0.098821 | 0.278092 | 0.299143 | 0.000772 | 0.000843 | 0.000914 |
| issue_53.test | COMPILE | 0.076981 | 0.189479 | 0.202611 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 1.003×, 0.992× | 1.005× |
| clang | 0.973× | 0.975×, 0.970× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000733s | 0.000747s | 0.000724s | byte equality observed |
| contains2_test.test | 10 | 0.000738s | 0.000742s | 0.000717s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000737s | 0.000738s | 0.000713s | byte equality observed |
| contains3_test.test | 10 | 0.000738s | 0.000744s | 0.000730s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000743s | 0.000747s | 0.000720s | byte equality observed |
| contains_test.test | 10 | 0.000732s | 0.000749s | 0.000716s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000745s | 0.000724s | 0.000718s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000740s | 0.000740s | 0.000721s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000741s | 0.000749s | 0.000738s | byte equality observed |
| function_arith_portable.test | 10 | 0.000742s | 0.000748s | 0.000728s | byte equality observed |
| function_n_test.test | 10 | 0.000752s | 0.000744s | 0.000719s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000718s | 0.000720s | 0.000712s | byte equality observed |
| function_ref_portable.test | 10 | 0.000716s | 0.000746s | 0.000702s | byte equality observed |
| function_test.test | 10 | 0.000774s | 0.000798s | 0.000749s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.000770s | 0.000786s | 0.000761s | byte equality observed |
| lambda_test.test | 10 | 0.000737s | 0.000750s | 0.000724s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000753s | 0.000749s | 0.000722s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000758s | 0.000770s | 0.000720s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.000841s | 0.000858s | 0.000770s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.000848s | 0.000839s | 0.000774s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.000865s | 0.000863s | 0.000766s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000849s | 0.000803s | 0.000711s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000830s | 0.000808s | 0.000713s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000812s | 0.000830s | 0.000770s | byte equality observed |
| nothrow_swap.test | 10 | 0.000818s | 0.000781s | 0.000793s | byte equality observed |
| quick.test | 10 | 0.000832s | 0.000807s | 0.000806s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000796s | 0.000796s | 0.000809s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000814s | 0.000808s | 0.000781s | byte equality observed |
| return_function_shared.test | 10 | 0.000862s | 0.000857s | 0.000845s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.000876s | 0.000873s | 0.000854s | byte equality observed |
| return_function_shared_14.test | 10 | 0.000861s | 0.000852s | 0.000866s | byte equality observed |
| return_function_static.test | 10 | 0.000834s | 0.000839s | 0.000816s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000809s | 0.000843s | 0.000818s | byte equality observed |
| return_function_static_14.test | 10 | 0.000802s | 0.000802s | 0.000790s | byte equality observed |
| rvalues_test.test | 10 | 0.000805s | 0.000819s | 0.000794s | byte equality observed |
| stateless_test.test | 10 | 0.000821s | 0.000803s | 0.000777s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000817s | 0.000776s | 0.000789s | byte equality observed |
| std_bind_portable.test | 10 | 0.000795s | 0.000787s | 0.000772s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000812s | 0.000797s | 0.000790s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000785s | 0.000783s | 0.000784s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.000861s | 0.000851s | 0.000855s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000852s | 0.000820s | 0.000845s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
