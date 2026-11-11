# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.414426 | 2.087447 | 2.072613 | 0.001972 | 0.002221 | 0.002194 |
| function_test_no_rtti.test | RUN | 0.409846 | 2.051987 | 2.087371 | 0.001662 | 0.002182 | 0.002076 |
| function_n_test.test | RUN | 0.394975 | 1.745819 | 1.758028 | 0.002202 | 0.002138 | 0.002318 |
| allocator_test.test | RUN | 0.319405 | 0.979610 | 1.202439 | 0.002160 | 0.002282 | 0.002022 |
| stateless_test.test | RUN | 0.258537 | 0.544907 | 0.741692 | 0.002120 | 0.002141 | 0.002224 |
| lambda_test.test | RUN | 0.545167 | 1.321699 | 1.692935 | 0.002100 | 0.002110 | 0.002130 |
| function_test_fail1.test | COMPILE_FAIL | 0.191252 | 0.365064 | 0.552815 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.196821 | 0.391555 | 0.594334 | — | — | — |
| function_30.test | COMPILE | 0.207601 | 0.389418 | 0.584918 | — | — | — |
| function_30_repeat.test | COMPILE | 0.203634 | 0.395739 | 0.571935 | — | — | — |
| function_arith_cxx98.test | RUN | 0.272063 | 0.659837 | 0.853836 | 0.001618 | 0.001813 | 0.002315 |
| function_arith_portable.test | RUN | 0.255650 | 0.609444 | 0.835929 | 0.002094 | 0.002218 | 0.002292 |
| sum_avg_cxx98.test | RUN | 0.253693 | 0.546477 | 0.747360 | 0.001536 | 0.002322 | 0.002098 |
| sum_avg_portable.test | RUN | 0.242702 | 0.484817 | 0.754063 | 0.002056 | 0.002182 | 0.002090 |
| mem_fun_cxx98.test | RUN | 0.283841 | 0.739019 | 0.986065 | 0.002171 | 0.001899 | 0.001684 |
| mem_fun_portable.test | RUN | 0.275188 | 0.680662 | 0.882161 | 0.001780 | 0.001967 | 0.002026 |
| std_bind_cxx98.test | RUN | 0.265746 | 0.632482 | 0.821575 | 0.001996 | 0.001881 | 0.002245 |
| std_bind_portable.test | RUN | 0.260031 | 0.572833 | 0.774786 | 0.002084 | 0.002290 | 0.002221 |
| function_ref_cxx98.test | RUN | 0.252901 | 0.539019 | 0.811255 | 0.001824 | 0.002294 | 0.001717 |
| function_ref_portable.test | RUN | 0.248057 | 0.493195 | 0.786228 | 0.002138 | 0.001914 | 0.002030 |
| contains_test.test | RUN | 0.296077 | 1.006863 | 1.159168 | 0.001923 | 0.002286 | 0.001958 |
| contains2_test.test | RUN | 0.276003 | 0.716499 | 0.918543 | 0.002190 | 0.002260 | 0.002164 |
| nothrow_swap.test | RUN | 0.260964 | 0.690528 | 0.932219 | 0.002125 | 0.002274 | 0.001922 |
| rvalues_test.test | RUN | 0.298502 | 0.817361 | 1.063758 | 0.002085 | 0.001935 | 0.002028 |
| function_typeof_test.test | COMPILE | 0.207778 | 0.393461 | 0.573282 | — | — | — |
| result_arg_types_test.test | RUN | 0.280907 | 0.580308 | 0.970636 | 0.001993 | 0.001881 | 0.002063 |
| result_arg_n_types_test.test | RUN | 0.283895 | 0.664077 | 1.100401 | 0.002194 | 0.002251 | 0.002244 |
| test_bad_function_call_shared.test | RUN | 0.251166 | 0.550190 | 0.820266 | 0.002288 | 0.002255 | 0.002276 |
| test_bad_function_call_static.test | RUN | 0.246974 | 0.533761 | 0.819283 | 0.002660 | 0.002256 | 0.002252 |
| mixed_cxxstd_shared.test | RUN | 0.274979 | 0.655325 | 0.864630 | 0.002090 | 0.002063 | 0.002274 |
| mixed_cxxstd_static.test | RUN | 0.259534 | 0.637914 | 0.866970 | 0.002232 | 0.002054 | 0.002040 |
| mixed_cxxstd_shared_0x.test | RUN | 0.262888 | 0.641060 | 0.887094 | 0.002382 | 0.002448 | 0.002490 |
| mixed_cxxstd_static_0x.test | RUN | 0.262248 | 0.624220 | 0.894540 | 0.002209 | 0.002320 | 0.001862 |
| mixed_cxxstd_shared_14.test | RUN | 0.263235 | 0.654108 | 0.890644 | 0.002404 | 0.002504 | 0.002546 |
| mixed_cxxstd_static_14.test | RUN | 0.259576 | 0.632509 | 0.909567 | 0.002184 | 0.002344 | 0.002287 |
| return_function_shared.test | RUN | 0.267067 | 0.625234 | 0.843151 | 0.002100 | 0.002450 | 0.002473 |
| return_function_static.test | RUN | 0.257069 | 0.630309 | 0.907717 | 0.001903 | 0.002351 | 0.002078 |
| return_function_shared_0x.test | RUN | 0.267010 | 0.622587 | 0.856286 | 0.002193 | 0.002554 | 0.002483 |
| return_function_static_0x.test | RUN | 0.248308 | 0.634354 | 0.833827 | 0.002176 | 0.002086 | 0.002251 |
| return_function_shared_14.test | RUN | 0.260811 | 0.619085 | 0.848718 | 0.002130 | 0.002334 | 0.002439 |
| return_function_static_14.test | RUN | 0.255106 | 0.659243 | 0.815552 | 0.001958 | 0.001996 | 0.002058 |
| quick.test | RUN | 0.259412 | 0.612155 | 0.841654 | 0.002115 | 0.002090 | 0.002288 |
| issue_42.test | COMPILE | 0.200879 | 0.417910 | 0.625410 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.342701 | 0.900162 | 1.294142 | 0.002127 | 0.002080 | 0.002178 |
| contains_test_no_rtti.test | RUN | 0.297701 | 0.984286 | 1.148413 | 0.002041 | 0.002187 | 0.001983 |
| contains2_test_no_rtti.test | RUN | 0.282861 | 0.745960 | 0.912791 | 0.001901 | 0.001998 | 0.002000 |
| contains3_test.test | RUN | 0.265234 | 0.598719 | 0.849623 | 0.001648 | 0.002056 | 0.001605 |
| contains3_test_no_rtti.test | RUN | 0.162462 | 0.435451 | 0.581138 | 0.001670 | 0.001719 | 0.001692 |
| issue_53.test | COMPILE | 0.146173 | 0.300971 | 0.414617 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.988×, 0.994× | 0.996× |
| clang | 1.005× | 0.999×, 1.011× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001256s | 0.001224s | 0.001256s | byte equality observed |
| contains2_test.test | 10 | 0.001237s | 0.001203s | 0.001224s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001224s | 0.001216s | 0.001237s | byte equality observed |
| contains3_test.test | 10 | 0.001233s | 0.001214s | 0.001274s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001228s | 0.001239s | 0.001309s | byte equality observed |
| contains_test.test | 10 | 0.001229s | 0.001253s | 0.001329s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001291s | 0.001299s | 0.001324s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001294s | 0.001297s | 0.001283s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001286s | 0.001323s | 0.001339s | byte equality observed |
| function_arith_portable.test | 10 | 0.001324s | 0.001324s | 0.001396s | byte equality observed |
| function_n_test.test | 10 | 0.001301s | 0.001289s | 0.001291s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001305s | 0.001292s | 0.001242s | byte equality observed |
| function_ref_portable.test | 10 | 0.001262s | 0.001293s | 0.001337s | byte equality observed |
| function_test.test | 10 | 0.001360s | 0.001348s | 0.001331s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001397s | 0.001384s | 0.001374s | byte equality observed |
| lambda_test.test | 10 | 0.001351s | 0.001376s | 0.001317s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001298s | 0.001324s | 0.001303s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001319s | 0.001262s | 0.001360s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001423s | 0.001453s | 0.001425s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001398s | 0.001384s | 0.001460s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001393s | 0.001382s | 0.001469s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001290s | 0.001273s | 0.001270s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001306s | 0.001324s | 0.001381s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001335s | 0.001292s | 0.001285s | byte equality observed |
| nothrow_swap.test | 10 | 0.001331s | 0.001356s | 0.001280s | byte equality observed |
| quick.test | 10 | 0.001351s | 0.001276s | 0.001319s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001340s | 0.001302s | 0.001307s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001320s | 0.001285s | 0.001317s | byte equality observed |
| return_function_shared.test | 10 | 0.001427s | 0.001418s | 0.001387s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001452s | 0.001403s | 0.001391s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001455s | 0.001438s | 0.001393s | byte equality observed |
| return_function_static.test | 10 | 0.001291s | 0.001289s | 0.001305s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001325s | 0.001332s | 0.001287s | byte equality observed |
| return_function_static_14.test | 10 | 0.001318s | 0.001335s | 0.001283s | byte equality observed |
| rvalues_test.test | 10 | 0.001335s | 0.001311s | 0.001341s | byte equality observed |
| stateless_test.test | 10 | 0.001293s | 0.001332s | 0.001295s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001317s | 0.001329s | 0.001308s | byte equality observed |
| std_bind_portable.test | 10 | 0.001269s | 0.001270s | 0.001275s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001273s | 0.001264s | 0.001246s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001271s | 0.001290s | 0.001302s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001435s | 0.001418s | 0.001446s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001314s | 0.001305s | 0.001340s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
