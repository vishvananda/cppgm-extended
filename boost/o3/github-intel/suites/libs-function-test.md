# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.286510 | 1.538402 | 1.291712 | 0.001260 | 0.001475 | 0.001352 |
| function_test_no_rtti.test | RUN | 0.280522 | 1.545285 | 1.297072 | 0.001332 | 0.001467 | 0.001235 |
| function_n_test.test | RUN | 0.255814 | 1.262548 | 1.092159 | 0.001380 | 0.001426 | 0.001383 |
| allocator_test.test | RUN | 0.207517 | 0.715732 | 0.713082 | 0.001317 | 0.001355 | 0.001391 |
| stateless_test.test | RUN | 0.158379 | 0.380125 | 0.448955 | 0.001297 | 0.001338 | 0.001270 |
| lambda_test.test | RUN | 0.326170 | 0.926561 | 0.980657 | 0.001305 | 0.001378 | 0.001346 |
| function_test_fail1.test | COMPILE_FAIL | 0.117741 | 0.250786 | 0.302372 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.118363 | 0.258109 | 0.307410 | — | — | — |
| function_30.test | COMPILE | 0.119310 | 0.275854 | 0.325158 | — | — | — |
| function_30_repeat.test | COMPILE | 0.119848 | 0.268415 | 0.325433 | — | — | — |
| function_arith_cxx98.test | RUN | 0.160115 | 0.464417 | 0.493962 | 0.001328 | 0.001140 | 0.001267 |
| function_arith_portable.test | RUN | 0.158328 | 0.433202 | 0.466121 | 0.001455 | 0.001360 | 0.001460 |
| sum_avg_cxx98.test | RUN | 0.152655 | 0.377972 | 0.443692 | 0.001443 | 0.001302 | 0.001354 |
| sum_avg_portable.test | RUN | 0.150485 | 0.343398 | 0.422591 | 0.001325 | 0.001329 | 0.001383 |
| mem_fun_cxx98.test | RUN | 0.176147 | 0.535480 | 0.563117 | 0.001692 | 0.001253 | 0.001412 |
| mem_fun_portable.test | RUN | 0.171114 | 0.486506 | 0.532960 | 0.001236 | 0.001205 | 0.001219 |
| std_bind_cxx98.test | RUN | 0.162400 | 0.449366 | 0.486550 | 0.001226 | 0.001334 | 0.001357 |
| std_bind_portable.test | RUN | 0.164138 | 0.412180 | 0.460647 | 0.001197 | 0.001421 | 0.001379 |
| function_ref_cxx98.test | RUN | 0.153126 | 0.376615 | 0.451026 | 0.001240 | 0.001322 | 0.001250 |
| function_ref_portable.test | RUN | 0.153088 | 0.339259 | 0.431824 | 0.001332 | 0.001205 | 0.001162 |
| contains_test.test | RUN | 0.194310 | 0.716820 | 0.688316 | 0.001217 | 0.001404 | 0.001375 |
| contains2_test.test | RUN | 0.165049 | 0.516223 | 0.535145 | 0.001290 | 0.001318 | 0.001198 |
| nothrow_swap.test | RUN | 0.162110 | 0.489141 | 0.519933 | 0.001335 | 0.001282 | 0.001197 |
| rvalues_test.test | RUN | 0.184218 | 0.586490 | 0.598630 | 0.001291 | 0.001306 | 0.001222 |
| function_typeof_test.test | COMPILE | 0.119719 | 0.265929 | 0.313920 | — | — | — |
| result_arg_types_test.test | RUN | 0.169979 | 0.404508 | 0.542406 | 0.001168 | 0.001303 | 0.001296 |
| result_arg_n_types_test.test | RUN | 0.198120 | 0.449780 | 0.634521 | 0.001337 | 0.001392 | 0.001293 |
| test_bad_function_call_shared.test | RUN | 0.155869 | 0.370697 | 0.442601 | 0.001290 | 0.001399 | 0.001441 |
| test_bad_function_call_static.test | RUN | 0.157389 | 0.371380 | 0.462358 | 0.001260 | 0.001393 | 0.001278 |
| mixed_cxxstd_shared.test | RUN | 0.161243 | 0.446457 | 0.520876 | 0.001412 | 0.001627 | 0.001457 |
| mixed_cxxstd_static.test | RUN | 0.159487 | 0.453055 | 0.527367 | 0.001258 | 0.001289 | 0.001201 |
| mixed_cxxstd_shared_0x.test | RUN | 0.157996 | 0.446677 | 0.494004 | 0.001368 | 0.001512 | 0.001453 |
| mixed_cxxstd_static_0x.test | RUN | 0.158488 | 0.444751 | 0.513164 | 0.001196 | 0.001392 | 0.001249 |
| mixed_cxxstd_shared_14.test | RUN | 0.160314 | 0.449386 | 0.501006 | 0.001419 | 0.001500 | 0.001423 |
| mixed_cxxstd_static_14.test | RUN | 0.162355 | 0.442153 | 0.498734 | 0.001256 | 0.001327 | 0.001392 |
| return_function_shared.test | RUN | 0.156699 | 0.453398 | 0.493336 | 0.001378 | 0.001350 | 0.001426 |
| return_function_static.test | RUN | 0.155555 | 0.443730 | 0.490659 | 0.001268 | 0.001301 | 0.001286 |
| return_function_shared_0x.test | RUN | 0.156800 | 0.457749 | 0.490565 | 0.001383 | 0.001760 | 0.001543 |
| return_function_static_0x.test | RUN | 0.156472 | 0.475553 | 0.483553 | 0.001331 | 0.001428 | 0.001356 |
| return_function_shared_14.test | RUN | 0.154065 | 0.480487 | 0.498713 | 0.001451 | 0.001421 | 0.001272 |
| return_function_static_14.test | RUN | 0.162061 | 0.451270 | 0.491112 | 0.001309 | 0.001416 | 0.001305 |
| quick.test | RUN | 0.169315 | 0.452369 | 0.486270 | 0.001256 | 0.001418 | 0.001237 |
| issue_42.test | COMPILE | 0.126371 | 0.284692 | 0.327647 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.209778 | 0.599744 | 0.721849 | 0.001294 | 0.001378 | 0.001387 |
| contains_test_no_rtti.test | RUN | 0.187769 | 0.707103 | 0.698933 | 0.001413 | 0.000940 | 0.001399 |
| contains2_test_no_rtti.test | RUN | 0.169130 | 0.530554 | 0.539610 | 0.001164 | 0.001345 | 0.000960 |
| contains3_test.test | RUN | 0.155431 | 0.403713 | 0.473024 | 0.000913 | 0.001439 | 0.001253 |
| contains3_test_no_rtti.test | RUN | 0.100974 | 0.350821 | 0.320793 | 0.001017 | 0.000968 | 0.000940 |
| issue_53.test | COMPILE | 0.088284 | 0.235958 | 0.226209 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.982× | 0.994×, 0.970× | 1.009× |
| clang | 0.992× | 0.989×, 0.996× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000844s | 0.000838s | 0.000834s | byte equality observed |
| contains2_test.test | 10 | 0.000847s | 0.000856s | 0.000840s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000855s | 0.000879s | 0.000832s | byte equality observed |
| contains3_test.test | 10 | 0.000841s | 0.000868s | 0.000829s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000861s | 0.000882s | 0.000869s | byte equality observed |
| contains_test.test | 10 | 0.000877s | 0.000898s | 0.000902s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000883s | 0.000902s | 0.000881s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000912s | 0.000877s | 0.000897s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000927s | 0.000884s | 0.000912s | byte equality observed |
| function_arith_portable.test | 10 | 0.000928s | 0.000915s | 0.000897s | byte equality observed |
| function_n_test.test | 10 | 0.000910s | 0.000901s | 0.000913s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000893s | 0.000869s | 0.000888s | byte equality observed |
| function_ref_portable.test | 10 | 0.000896s | 0.000864s | 0.000884s | byte equality observed |
| function_test.test | 10 | 0.000947s | 0.000943s | 0.000940s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.000954s | 0.000978s | 0.000959s | byte equality observed |
| lambda_test.test | 10 | 0.000912s | 0.000877s | 0.000927s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000924s | 0.000884s | 0.000886s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000925s | 0.000881s | 0.000934s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.000982s | 0.000941s | 0.000962s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.000967s | 0.000922s | 0.000998s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.000968s | 0.000964s | 0.000966s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000915s | 0.000911s | 0.000893s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000909s | 0.000891s | 0.000872s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000904s | 0.000915s | 0.000890s | byte equality observed |
| nothrow_swap.test | 10 | 0.000900s | 0.000897s | 0.000891s | byte equality observed |
| quick.test | 10 | 0.000924s | 0.000908s | 0.000893s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000888s | 0.000908s | 0.000888s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000916s | 0.000932s | 0.000874s | byte equality observed |
| return_function_shared.test | 10 | 0.000945s | 0.000952s | 0.000957s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.000973s | 0.000949s | 0.000945s | byte equality observed |
| return_function_shared_14.test | 10 | 0.000959s | 0.000943s | 0.000957s | byte equality observed |
| return_function_static.test | 10 | 0.000906s | 0.000888s | 0.000859s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000907s | 0.000873s | 0.000903s | byte equality observed |
| return_function_static_14.test | 10 | 0.000939s | 0.000880s | 0.000908s | byte equality observed |
| rvalues_test.test | 10 | 0.000937s | 0.000915s | 0.000907s | byte equality observed |
| stateless_test.test | 10 | 0.000920s | 0.000909s | 0.000885s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000868s | 0.000862s | 0.000891s | byte equality observed |
| std_bind_portable.test | 10 | 0.000901s | 0.000880s | 0.000882s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000871s | 0.000891s | 0.000898s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000888s | 0.000873s | 0.000869s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.000988s | 0.000990s | 0.000989s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000973s | 0.000896s | 0.000951s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
