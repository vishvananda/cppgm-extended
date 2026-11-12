# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.378702 | 2.064553 | 1.979250 | 0.001886 | 0.002302 | 0.002002 |
| function_test_no_rtti.test | RUN | 0.368773 | 2.029645 | 2.017844 | 0.002065 | 0.002088 | 0.001704 |
| function_n_test.test | RUN | 0.348506 | 1.707416 | 1.696791 | 0.002071 | 0.002005 | 0.002113 |
| allocator_test.test | RUN | 0.291479 | 0.977686 | 1.173262 | 0.002224 | 0.002232 | 0.002000 |
| stateless_test.test | RUN | 0.224832 | 0.540199 | 0.785272 | 0.002116 | 0.001968 | 0.002000 |
| lambda_test.test | RUN | 0.488036 | 1.289287 | 1.682234 | 0.002094 | 0.002104 | 0.002148 |
| function_test_fail1.test | COMPILE_FAIL | 0.166556 | 0.359544 | 0.567459 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.177430 | 0.373364 | 0.575295 | — | — | — |
| function_30.test | COMPILE | 0.183336 | 0.395126 | 0.569701 | — | — | — |
| function_30_repeat.test | COMPILE | 0.183852 | 0.403997 | 0.585383 | — | — | — |
| function_arith_cxx98.test | RUN | 0.231589 | 0.639394 | 0.831556 | 0.002084 | 0.002171 | 0.002144 |
| function_arith_portable.test | RUN | 0.222561 | 0.605360 | 0.776701 | 0.001808 | 0.001926 | 0.002106 |
| sum_avg_cxx98.test | RUN | 0.218738 | 0.521183 | 0.734900 | 0.001737 | 0.002073 | 0.002458 |
| sum_avg_portable.test | RUN | 0.218396 | 0.485002 | 0.720937 | 0.002077 | 0.002203 | 0.002006 |
| mem_fun_cxx98.test | RUN | 0.251772 | 0.738170 | 0.895898 | 0.001954 | 0.002348 | 0.002034 |
| mem_fun_portable.test | RUN | 0.249753 | 0.677837 | 0.901218 | 0.001842 | 0.002919 | 0.002486 |
| std_bind_cxx98.test | RUN | 0.226322 | 0.620064 | 0.807737 | 0.002014 | 0.001823 | 0.002103 |
| std_bind_portable.test | RUN | 0.219207 | 0.589896 | 0.783909 | 0.002142 | 0.002148 | 0.002026 |
| function_ref_cxx98.test | RUN | 0.219350 | 0.524832 | 0.743105 | 0.001794 | 0.001918 | 0.001939 |
| function_ref_portable.test | RUN | 0.213177 | 0.483036 | 0.687585 | 0.002126 | 0.002323 | 0.002446 |
| contains_test.test | RUN | 0.267862 | 0.990080 | 1.173702 | 0.001894 | 0.002200 | 0.002092 |
| contains2_test.test | RUN | 0.243514 | 0.716988 | 0.906280 | 0.001944 | 0.002090 | 0.002074 |
| nothrow_swap.test | RUN | 0.236489 | 0.673282 | 0.843588 | 0.001903 | 0.002490 | 0.002082 |
| rvalues_test.test | RUN | 0.270692 | 0.819587 | 0.987908 | 0.002118 | 0.002008 | 0.002044 |
| function_typeof_test.test | COMPILE | 0.183195 | 0.371734 | 0.545180 | — | — | — |
| result_arg_types_test.test | RUN | 0.250857 | 0.583569 | 0.896458 | 0.001898 | 0.002371 | 0.001902 |
| result_arg_n_types_test.test | RUN | 0.274425 | 0.655527 | 1.086632 | 0.001890 | 0.002364 | 0.002041 |
| test_bad_function_call_shared.test | RUN | 0.222286 | 0.566750 | 0.791233 | 0.002370 | 0.002449 | 0.002019 |
| test_bad_function_call_static.test | RUN | 0.225149 | 0.532524 | 0.791478 | 0.001922 | 0.001878 | 0.002126 |
| mixed_cxxstd_shared.test | RUN | 0.236161 | 0.637141 | 0.841449 | 0.001858 | 0.002604 | 0.001954 |
| mixed_cxxstd_static.test | RUN | 0.227652 | 0.633193 | 0.859379 | 0.002191 | 0.002016 | 0.002100 |
| mixed_cxxstd_shared_0x.test | RUN | 0.226910 | 0.630151 | 0.874174 | 0.002081 | 0.002418 | 0.002291 |
| mixed_cxxstd_static_0x.test | RUN | 0.238365 | 0.615863 | 0.870995 | 0.002170 | 0.002130 | 0.001910 |
| mixed_cxxstd_shared_14.test | RUN | 0.232973 | 0.649423 | 0.864303 | 0.002087 | 0.002078 | 0.002322 |
| mixed_cxxstd_static_14.test | RUN | 0.242174 | 0.623041 | 0.892824 | 0.002526 | 0.002240 | 0.002612 |
| return_function_shared.test | RUN | 0.229316 | 0.615066 | 0.828209 | 0.001975 | 0.002253 | 0.002294 |
| return_function_static.test | RUN | 0.231039 | 0.620943 | 0.851020 | 0.001641 | 0.002579 | 0.001760 |
| return_function_shared_0x.test | RUN | 0.223125 | 0.629704 | 0.867207 | 0.001890 | 0.002646 | 0.002260 |
| return_function_static_0x.test | RUN | 0.235552 | 0.618320 | 0.856854 | 0.002090 | 0.002836 | 0.002042 |
| return_function_shared_14.test | RUN | 0.239708 | 0.627464 | 0.870184 | 0.002122 | 0.002048 | 0.002228 |
| return_function_static_14.test | RUN | 0.224075 | 0.640716 | 0.834520 | 0.001828 | 0.002184 | 0.002100 |
| quick.test | RUN | 0.237584 | 0.617509 | 0.849630 | 0.002202 | 0.001916 | 0.002232 |
| issue_42.test | COMPILE | 0.182025 | 0.424312 | 0.589623 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.300941 | 0.860904 | 1.162493 | 0.002340 | 0.002102 | 0.002128 |
| contains_test_no_rtti.test | RUN | 0.271009 | 0.975957 | 1.163414 | 0.002384 | 0.001994 | 0.002072 |
| contains2_test_no_rtti.test | RUN | 0.252054 | 0.732661 | 0.905316 | 0.002040 | 0.001577 | 0.001683 |
| contains3_test.test | RUN | 0.236455 | 0.575161 | 0.820328 | 0.001998 | 0.001952 | 0.001906 |
| contains3_test_no_rtti.test | RUN | 0.166571 | 0.451557 | 0.552008 | 0.001404 | 0.001468 | 0.001449 |
| issue_53.test | COMPILE | 0.132934 | 0.317376 | 0.368361 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.018× | 1.034×, 1.003× | 1.005× |
| clang | 0.997× | 1.003×, 0.991× | 1.015× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001203s | 0.001200s | 0.001173s | byte equality observed |
| contains2_test.test | 10 | 0.001209s | 0.001196s | 0.001180s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001199s | 0.001179s | 0.001189s | byte equality observed |
| contains3_test.test | 10 | 0.001190s | 0.001196s | 0.001191s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001187s | 0.001194s | 0.001259s | byte equality observed |
| contains_test.test | 10 | 0.001178s | 0.001225s | 0.001303s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001178s | 0.001230s | 0.001303s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001197s | 0.001302s | 0.001345s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001266s | 0.001338s | 0.001303s | byte equality observed |
| function_arith_portable.test | 10 | 0.001327s | 0.001334s | 0.001284s | byte equality observed |
| function_n_test.test | 10 | 0.001291s | 0.001404s | 0.001246s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001287s | 0.001255s | 0.001265s | byte equality observed |
| function_ref_portable.test | 10 | 0.001263s | 0.001294s | 0.001222s | byte equality observed |
| function_test.test | 10 | 0.001319s | 0.001359s | 0.001313s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001370s | 0.001439s | 0.001376s | byte equality observed |
| lambda_test.test | 10 | 0.001299s | 0.001295s | 0.001266s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001308s | 0.001337s | 0.001287s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001259s | 0.001277s | 0.001255s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001379s | 0.001384s | 0.001334s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001372s | 0.001396s | 0.001361s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001391s | 0.001388s | 0.001374s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001275s | 0.001364s | 0.001330s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001294s | 0.001313s | 0.001335s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001318s | 0.001328s | 0.001273s | byte equality observed |
| nothrow_swap.test | 10 | 0.001289s | 0.001275s | 0.001249s | byte equality observed |
| quick.test | 10 | 0.001306s | 0.001352s | 0.001307s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001301s | 0.001259s | 0.001299s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001281s | 0.001320s | 0.001302s | byte equality observed |
| return_function_shared.test | 10 | 0.001388s | 0.001425s | 0.001386s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001422s | 0.001442s | 0.001396s | byte equality observed |
| return_function_shared_14.test | 9 | 0.001420s | 0.001426s | 0.001420s | byte equality observed |
| return_function_static.test | 10 | 0.001332s | 0.001297s | 0.001302s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001290s | 0.001342s | 0.001284s | byte equality observed |
| return_function_static_14.test | 10 | 0.001300s | 0.001346s | 0.001228s | byte equality observed |
| rvalues_test.test | 10 | 0.001316s | 0.001341s | 0.001290s | byte equality observed |
| stateless_test.test | 10 | 0.001282s | 0.001313s | 0.001281s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001253s | 0.001301s | 0.001253s | byte equality observed |
| std_bind_portable.test | 10 | 0.001265s | 0.001277s | 0.001234s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001241s | 0.001277s | 0.001272s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001299s | 0.001311s | 0.001244s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001380s | 0.001442s | 0.001392s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001319s | 0.001352s | 0.001344s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
