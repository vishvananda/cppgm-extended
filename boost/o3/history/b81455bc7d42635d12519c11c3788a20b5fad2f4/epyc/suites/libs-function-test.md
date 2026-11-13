# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.200922 | 1.245417 | 0.887110 | 0.001095 | 0.001536 | 0.001430 |
| function_test_no_rtti.test | RUN | 0.196728 | 1.221440 | 0.898512 | 0.001379 | 0.001733 | 0.001325 |
| function_n_test.test | RUN | 0.181655 | 1.006981 | 0.754016 | 0.001322 | 0.001652 | 0.001396 |
| allocator_test.test | RUN | 0.150761 | 0.567064 | 0.503481 | 0.001501 | 0.001658 | 0.001481 |
| stateless_test.test | RUN | 0.110736 | 0.313856 | 0.336362 | 0.001312 | 0.001575 | 0.001402 |
| lambda_test.test | RUN | 0.244271 | 0.748280 | 0.706281 | 0.001277 | 0.001632 | 0.001360 |
| function_test_fail1.test | COMPILE_FAIL | 0.086486 | 0.207213 | 0.204198 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.088750 | 0.213412 | 0.221649 | — | — | — |
| function_30.test | COMPILE | 0.093718 | 0.220653 | 0.219637 | — | — | — |
| function_30_repeat.test | COMPILE | 0.090088 | 0.230998 | 0.223251 | — | — | — |
| function_arith_cxx98.test | RUN | 0.119769 | 0.378909 | 0.359347 | 0.001226 | 0.001541 | 0.001460 |
| function_arith_portable.test | RUN | 0.116349 | 0.357846 | 0.330180 | 0.001412 | 0.001404 | 0.001318 |
| sum_avg_cxx98.test | RUN | 0.112840 | 0.304367 | 0.315680 | 0.001097 | 0.001589 | 0.001415 |
| sum_avg_portable.test | RUN | 0.116686 | 0.269450 | 0.310220 | 0.001206 | 0.001706 | 0.001450 |
| mem_fun_cxx98.test | RUN | 0.130136 | 0.425287 | 0.389936 | 0.001452 | 0.001526 | 0.001473 |
| mem_fun_portable.test | RUN | 0.126259 | 0.387437 | 0.370131 | 0.001328 | 0.001622 | 0.001348 |
| std_bind_cxx98.test | RUN | 0.120083 | 0.357758 | 0.343915 | 0.001233 | 0.001540 | 0.001445 |
| std_bind_portable.test | RUN | 0.124254 | 0.341163 | 0.328147 | 0.001179 | 0.001721 | 0.001430 |
| function_ref_cxx98.test | RUN | 0.112555 | 0.307267 | 0.314665 | 0.001442 | 0.001562 | 0.001529 |
| function_ref_portable.test | RUN | 0.110286 | 0.284627 | 0.307047 | 0.001350 | 0.001569 | 0.001442 |
| contains_test.test | RUN | 0.141301 | 0.574130 | 0.473264 | 0.001330 | 0.001559 | 0.001465 |
| contains2_test.test | RUN | 0.129743 | 0.422150 | 0.372669 | 0.001226 | 0.001614 | 0.001436 |
| nothrow_swap.test | RUN | 0.122437 | 0.410850 | 0.349194 | 0.001337 | 0.001486 | 0.001431 |
| rvalues_test.test | RUN | 0.143445 | 0.478828 | 0.416479 | 0.001391 | 0.001608 | 0.001394 |
| function_typeof_test.test | COMPILE | 0.088416 | 0.215420 | 0.208671 | — | — | — |
| result_arg_types_test.test | RUN | 0.126181 | 0.332240 | 0.374391 | 0.001193 | 0.001667 | 0.001460 |
| result_arg_n_types_test.test | RUN | 0.134363 | 0.358977 | 0.445325 | 0.001358 | 0.001673 | 0.001318 |
| test_bad_function_call_shared.test | RUN | 0.114218 | 0.303059 | 0.312506 | 0.001379 | 0.001823 | 0.001503 |
| test_bad_function_call_static.test | RUN | 0.114812 | 0.305188 | 0.324732 | 0.001347 | 0.001609 | 0.001464 |
| mixed_cxxstd_shared.test | RUN | 0.123814 | 0.357966 | 0.348789 | 0.001280 | 0.001634 | 0.001591 |
| mixed_cxxstd_static.test | RUN | 0.119411 | 0.369168 | 0.361438 | 0.001496 | 0.001575 | 0.001207 |
| mixed_cxxstd_shared_0x.test | RUN | 0.118771 | 0.359998 | 0.339369 | 0.001505 | 0.001936 | 0.001546 |
| mixed_cxxstd_static_0x.test | RUN | 0.116880 | 0.364302 | 0.347594 | 0.001237 | 0.001661 | 0.001422 |
| mixed_cxxstd_shared_14.test | RUN | 0.121656 | 0.369329 | 0.363694 | 0.001629 | 0.001708 | 0.001182 |
| mixed_cxxstd_static_14.test | RUN | 0.120607 | 0.356442 | 0.347414 | 0.001357 | 0.001658 | 0.001439 |
| return_function_shared.test | RUN | 0.113283 | 0.364325 | 0.339943 | 0.001443 | 0.001798 | 0.001580 |
| return_function_static.test | RUN | 0.116926 | 0.364267 | 0.343861 | 0.001322 | 0.001615 | 0.001306 |
| return_function_shared_0x.test | RUN | 0.116793 | 0.377369 | 0.360164 | 0.001414 | 0.001910 | 0.001399 |
| return_function_static_0x.test | RUN | 0.117898 | 0.358605 | 0.351102 | 0.001217 | 0.001665 | 0.001457 |
| return_function_shared_14.test | RUN | 0.121949 | 0.368636 | 0.337278 | 0.001635 | 0.001271 | 0.001544 |
| return_function_static_14.test | RUN | 0.116968 | 0.364639 | 0.350007 | 0.001277 | 0.001634 | 0.001331 |
| quick.test | RUN | 0.120654 | 0.370177 | 0.356044 | 0.001275 | 0.001494 | 0.001230 |
| issue_42.test | COMPILE | 0.094523 | 0.236718 | 0.242108 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.154424 | 0.498354 | 0.505966 | 0.001394 | 0.001579 | 0.001414 |
| contains_test_no_rtti.test | RUN | 0.137119 | 0.572520 | 0.490627 | 0.001295 | 0.001599 | 0.001274 |
| contains2_test_no_rtti.test | RUN | 0.125333 | 0.415108 | 0.381697 | 0.001378 | 0.001584 | 0.001049 |
| contains3_test.test | RUN | 0.118371 | 0.324692 | 0.342399 | 0.001101 | 0.001546 | 0.001179 |
| contains3_test_no_rtti.test | RUN | 0.092721 | 0.269668 | 0.221906 | 0.001049 | 0.001213 | 0.001017 |
| issue_53.test | COMPILE | 0.080994 | 0.182328 | 0.166124 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.999× | 0.991×, 1.007× | 1.004× |
| clang | 0.995× | 0.996×, 0.995× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000859s | 0.000824s | 0.000845s | byte equality observed |
| contains2_test.test | 10 | 0.000862s | 0.000850s | 0.000835s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000853s | 0.000876s | 0.000862s | byte equality observed |
| contains3_test.test | 10 | 0.000854s | 0.000873s | 0.000859s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000851s | 0.000848s | 0.000878s | byte equality observed |
| contains_test.test | 10 | 0.000863s | 0.000855s | 0.000866s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000847s | 0.000862s | 0.000860s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000847s | 0.000853s | 0.000866s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000865s | 0.000879s | 0.000864s | byte equality observed |
| function_arith_portable.test | 10 | 0.000858s | 0.000864s | 0.000869s | byte equality observed |
| function_n_test.test | 10 | 0.000869s | 0.000900s | 0.000851s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000854s | 0.000834s | 0.000829s | byte equality observed |
| function_ref_portable.test | 10 | 0.000833s | 0.000834s | 0.000816s | byte equality observed |
| function_test.test | 10 | 0.000911s | 0.000885s | 0.000909s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.000910s | 0.000891s | 0.000908s | byte equality observed |
| lambda_test.test | 10 | 0.000858s | 0.000858s | 0.000855s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000861s | 0.000870s | 0.000846s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000852s | 0.000857s | 0.000846s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.000924s | 0.000935s | 0.000937s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.000926s | 0.000901s | 0.000924s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.000918s | 0.000915s | 0.000945s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000850s | 0.000879s | 0.000866s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000850s | 0.000876s | 0.000853s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000864s | 0.000861s | 0.000852s | byte equality observed |
| nothrow_swap.test | 10 | 0.000856s | 0.000863s | 0.000839s | byte equality observed |
| quick.test | 10 | 0.000855s | 0.000859s | 0.000846s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000855s | 0.000877s | 0.000856s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000846s | 0.000865s | 0.000858s | byte equality observed |
| return_function_shared.test | 10 | 0.000922s | 0.000916s | 0.000923s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.000920s | 0.000939s | 0.000921s | byte equality observed |
| return_function_shared_14.test | 10 | 0.000933s | 0.000924s | 0.000935s | byte equality observed |
| return_function_static.test | 10 | 0.000871s | 0.000866s | 0.000845s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000863s | 0.000866s | 0.000850s | byte equality observed |
| return_function_static_14.test | 10 | 0.000865s | 0.000865s | 0.000862s | byte equality observed |
| rvalues_test.test | 10 | 0.000872s | 0.000862s | 0.000845s | byte equality observed |
| stateless_test.test | 10 | 0.000851s | 0.000855s | 0.000845s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000836s | 0.000816s | 0.000831s | byte equality observed |
| std_bind_portable.test | 10 | 0.000839s | 0.000838s | 0.000824s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000834s | 0.000834s | 0.000840s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000825s | 0.000836s | 0.000825s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.000951s | 0.000964s | 0.000941s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000895s | 0.000867s | 0.000875s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
