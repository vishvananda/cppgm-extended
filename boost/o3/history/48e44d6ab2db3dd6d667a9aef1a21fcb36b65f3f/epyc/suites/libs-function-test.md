# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.392147 | 1.986861 | 1.969702 | 0.001979 | 0.002042 | 0.002286 |
| function_test_no_rtti.test | RUN | 0.388420 | 1.994642 | 2.055015 | 0.001709 | 0.001854 | 0.002228 |
| function_n_test.test | RUN | 0.368894 | 1.640943 | 1.711072 | 0.002006 | 0.002119 | 0.002100 |
| allocator_test.test | RUN | 0.299761 | 0.930165 | 1.216821 | 0.001996 | 0.002117 | 0.002122 |
| stateless_test.test | RUN | 0.241159 | 0.525339 | 0.772307 | 0.002467 | 0.002036 | 0.002074 |
| lambda_test.test | RUN | 0.517396 | 1.254260 | 1.674988 | 0.001838 | 0.002066 | 0.002269 |
| function_test_fail1.test | COMPILE_FAIL | 0.182644 | 0.359395 | 0.577822 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.190843 | 0.366239 | 0.540728 | — | — | — |
| function_30.test | COMPILE | 0.193804 | 0.378517 | 0.577614 | — | — | — |
| function_30_repeat.test | COMPILE | 0.202301 | 0.394596 | 0.597291 | — | — | — |
| function_arith_cxx98.test | RUN | 0.255467 | 0.628525 | 0.854279 | 0.001919 | 0.001898 | 0.002156 |
| function_arith_portable.test | RUN | 0.244675 | 0.595423 | 0.779674 | 0.002115 | 0.001779 | 0.002231 |
| sum_avg_cxx98.test | RUN | 0.238275 | 0.511897 | 0.745492 | 0.001621 | 0.001907 | 0.002078 |
| sum_avg_portable.test | RUN | 0.237795 | 0.473055 | 0.723159 | 0.002078 | 0.002136 | 0.002142 |
| mem_fun_cxx98.test | RUN | 0.266397 | 0.732422 | 0.929896 | 0.001858 | 0.002152 | 0.002095 |
| mem_fun_portable.test | RUN | 0.257992 | 0.662911 | 0.884086 | 0.001772 | 0.002006 | 0.001961 |
| std_bind_cxx98.test | RUN | 0.245858 | 0.626703 | 0.823174 | 0.001789 | 0.001746 | 0.002036 |
| std_bind_portable.test | RUN | 0.245100 | 0.554262 | 0.778545 | 0.002143 | 0.002096 | 0.002034 |
| function_ref_cxx98.test | RUN | 0.233849 | 0.523563 | 0.761667 | 0.001845 | 0.002074 | 0.001961 |
| function_ref_portable.test | RUN | 0.231842 | 0.484854 | 0.761559 | 0.001644 | 0.001903 | 0.001877 |
| contains_test.test | RUN | 0.273063 | 0.963676 | 1.111461 | 0.001821 | 0.002090 | 0.002035 |
| contains2_test.test | RUN | 0.262277 | 0.692121 | 0.907258 | 0.002238 | 0.002208 | 0.001924 |
| nothrow_swap.test | RUN | 0.253770 | 0.661420 | 0.849268 | 0.001794 | 0.002282 | 0.001828 |
| rvalues_test.test | RUN | 0.287426 | 0.821392 | 0.919604 | 0.002094 | 0.002247 | 0.002080 |
| function_typeof_test.test | COMPILE | 0.190369 | 0.382733 | 0.571956 | — | — | — |
| result_arg_types_test.test | RUN | 0.261761 | 0.561884 | 0.943341 | 0.001925 | 0.001852 | 0.001727 |
| result_arg_n_types_test.test | RUN | 0.279650 | 0.617242 | 1.101448 | 0.001806 | 0.002087 | 0.002122 |
| test_bad_function_call_shared.test | RUN | 0.247986 | 0.512991 | 0.799478 | 0.002157 | 0.002431 | 0.002312 |
| test_bad_function_call_static.test | RUN | 0.247980 | 0.530492 | 0.738347 | 0.001778 | 0.001904 | 0.001972 |
| mixed_cxxstd_shared.test | RUN | 0.261798 | 0.606539 | 0.865078 | 0.002015 | 0.002038 | 0.002326 |
| mixed_cxxstd_static.test | RUN | 0.246243 | 0.596935 | 0.848167 | 0.001944 | 0.001986 | 0.002152 |
| mixed_cxxstd_shared_0x.test | RUN | 0.246096 | 0.621230 | 0.882180 | 0.002234 | 0.002376 | 0.002353 |
| mixed_cxxstd_static_0x.test | RUN | 0.252828 | 0.602994 | 0.809212 | 0.002139 | 0.002131 | 0.002158 |
| mixed_cxxstd_shared_14.test | RUN | 0.256293 | 0.631341 | 0.893460 | 0.002233 | 0.002240 | 0.002116 |
| mixed_cxxstd_static_14.test | RUN | 0.250229 | 0.615084 | 0.821910 | 0.001945 | 0.002026 | 0.002127 |
| return_function_shared.test | RUN | 0.250789 | 0.604576 | 0.871111 | 0.001966 | 0.002436 | 0.002454 |
| return_function_static.test | RUN | 0.256545 | 0.599610 | 0.851228 | 0.001826 | 0.002188 | 0.002050 |
| return_function_shared_0x.test | RUN | 0.242073 | 0.620261 | 0.809356 | 0.001578 | 0.002436 | 0.002344 |
| return_function_static_0x.test | RUN | 0.242328 | 0.602569 | 0.860823 | 0.002069 | 0.002210 | 0.002080 |
| return_function_shared_14.test | RUN | 0.255735 | 0.600640 | 0.836729 | 0.002103 | 0.001862 | 0.002331 |
| return_function_static_14.test | RUN | 0.247711 | 0.606138 | 0.836268 | 0.001644 | 0.001934 | 0.002097 |
| quick.test | RUN | 0.253413 | 0.624173 | 0.879358 | 0.001828 | 0.002166 | 0.002137 |
| issue_42.test | COMPILE | 0.203556 | 0.391980 | 0.618398 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.324594 | 0.851493 | 1.219454 | 0.001762 | 0.001961 | 0.002069 |
| contains_test_no_rtti.test | RUN | 0.283193 | 0.961319 | 1.142557 | 0.001976 | 0.001681 | 0.001728 |
| contains2_test_no_rtti.test | RUN | 0.263936 | 0.679756 | 0.913990 | 0.001822 | 0.002184 | 0.002030 |
| contains3_test.test | RUN | 0.255169 | 0.541050 | 0.830144 | 0.001697 | 0.001879 | 0.001856 |
| contains3_test_no_rtti.test | RUN | 0.162903 | 0.421655 | 0.534747 | 0.001492 | 0.001548 | 0.001571 |
| issue_53.test | COMPILE | 0.146211 | 0.302861 | 0.384343 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.987× | 0.991×, 0.983× | 1.008× |
| clang | 1.018× | 1.021×, 1.014× | 1.024× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001190s | 0.001198s | 0.001209s | byte equality observed |
| contains2_test.test | 10 | 0.001184s | 0.001171s | 0.001192s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001176s | 0.001183s | 0.001226s | byte equality observed |
| contains3_test.test | 10 | 0.001167s | 0.001169s | 0.001263s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001168s | 0.001173s | 0.001264s | byte equality observed |
| contains_test.test | 10 | 0.001179s | 0.001185s | 0.001292s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001191s | 0.001249s | 0.001264s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001217s | 0.001276s | 0.001344s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001292s | 0.001308s | 0.001320s | byte equality observed |
| function_arith_portable.test | 10 | 0.001333s | 0.001323s | 0.001338s | byte equality observed |
| function_n_test.test | 10 | 0.001341s | 0.001296s | 0.001254s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001275s | 0.001207s | 0.001247s | byte equality observed |
| function_ref_portable.test | 10 | 0.001257s | 0.001225s | 0.001244s | byte equality observed |
| function_test.test | 10 | 0.001326s | 0.001302s | 0.001351s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001332s | 0.001329s | 0.001338s | byte equality observed |
| lambda_test.test | 10 | 0.001332s | 0.001287s | 0.001294s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001291s | 0.001274s | 0.001322s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001282s | 0.001243s | 0.001267s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001385s | 0.001377s | 0.001385s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001411s | 0.001390s | 0.001394s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001371s | 0.001360s | 0.001370s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001287s | 0.001296s | 0.001351s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001309s | 0.001286s | 0.001296s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001286s | 0.001319s | 0.001268s | byte equality observed |
| nothrow_swap.test | 10 | 0.001310s | 0.001285s | 0.001326s | byte equality observed |
| quick.test | 10 | 0.001322s | 0.001302s | 0.001277s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001275s | 0.001261s | 0.001287s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001285s | 0.001251s | 0.001331s | byte equality observed |
| return_function_shared.test | 10 | 0.001361s | 0.001362s | 0.001364s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001393s | 0.001394s | 0.001393s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001410s | 0.001409s | 0.001380s | byte equality observed |
| return_function_static.test | 10 | 0.001281s | 0.001265s | 0.001266s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001302s | 0.001259s | 0.001311s | byte equality observed |
| return_function_static_14.test | 10 | 0.001308s | 0.001262s | 0.001300s | byte equality observed |
| rvalues_test.test | 10 | 0.001299s | 0.001282s | 0.001302s | byte equality observed |
| stateless_test.test | 10 | 0.001261s | 0.001288s | 0.001284s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001250s | 0.001253s | 0.001213s | byte equality observed |
| std_bind_portable.test | 10 | 0.001263s | 0.001233s | 0.001232s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001260s | 0.001253s | 0.001231s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001269s | 0.001297s | 0.001266s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001412s | 0.001385s | 0.001406s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001286s | 0.001327s | 0.001311s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
