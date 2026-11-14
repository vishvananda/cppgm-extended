# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.223124 | 1.153416 | 0.915515 | 0.001513 | 0.001324 | 0.001458 |
| function_test_no_rtti.test | RUN | 0.226900 | 1.143599 | 0.924141 | 0.001454 | 0.001687 | 0.001228 |
| function_n_test.test | RUN | 0.210848 | 0.958929 | 0.747491 | 0.001506 | 0.001686 | 0.001503 |
| allocator_test.test | RUN | 0.171810 | 0.525763 | 0.503587 | 0.001814 | 0.001626 | 0.001398 |
| stateless_test.test | RUN | 0.132734 | 0.306001 | 0.327434 | 0.001721 | 0.001371 | 0.001428 |
| lambda_test.test | RUN | 0.287189 | 0.694529 | 0.690109 | 0.001765 | 0.001531 | 0.001474 |
| function_test_fail1.test | COMPILE_FAIL | 0.098894 | 0.188507 | 0.218873 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.103539 | 0.198408 | 0.219133 | — | — | — |
| function_30.test | COMPILE | 0.104889 | 0.201330 | 0.229235 | — | — | — |
| function_30_repeat.test | COMPILE | 0.104043 | 0.202002 | 0.230671 | — | — | — |
| function_arith_cxx98.test | RUN | 0.136974 | 0.346588 | 0.346683 | 0.001755 | 0.001401 | 0.001371 |
| function_arith_portable.test | RUN | 0.137074 | 0.323448 | 0.325148 | 0.001524 | 0.001179 | 0.001524 |
| sum_avg_cxx98.test | RUN | 0.129569 | 0.289843 | 0.303476 | 0.001550 | 0.001478 | 0.001426 |
| sum_avg_portable.test | RUN | 0.131918 | 0.255378 | 0.305317 | 0.001671 | 0.001583 | 0.001375 |
| mem_fun_cxx98.test | RUN | 0.150217 | 0.396430 | 0.383045 | 0.001803 | 0.001398 | 0.001482 |
| mem_fun_portable.test | RUN | 0.145749 | 0.363593 | 0.386278 | 0.001440 | 0.001568 | 0.001274 |
| std_bind_cxx98.test | RUN | 0.138693 | 0.338683 | 0.348864 | 0.001566 | 0.001267 | 0.001117 |
| std_bind_portable.test | RUN | 0.141987 | 0.309794 | 0.320943 | 0.001710 | 0.001629 | 0.001426 |
| function_ref_cxx98.test | RUN | 0.129769 | 0.276812 | 0.324037 | 0.001708 | 0.001346 | 0.001281 |
| function_ref_portable.test | RUN | 0.127463 | 0.252073 | 0.291665 | 0.001384 | 0.001538 | 0.001388 |
| contains_test.test | RUN | 0.156674 | 0.518606 | 0.478703 | 0.001510 | 0.001664 | 0.001369 |
| contains2_test.test | RUN | 0.150080 | 0.382047 | 0.373819 | 0.001773 | 0.001684 | 0.001346 |
| nothrow_swap.test | RUN | 0.144475 | 0.358510 | 0.358512 | 0.001752 | 0.001472 | 0.001534 |
| rvalues_test.test | RUN | 0.161193 | 0.439988 | 0.422641 | 0.001626 | 0.001592 | 0.001315 |
| function_typeof_test.test | COMPILE | 0.102795 | 0.197382 | 0.227923 | — | — | — |
| result_arg_types_test.test | RUN | 0.150151 | 0.305209 | 0.379854 | 0.001575 | 0.001546 | 0.001265 |
| result_arg_n_types_test.test | RUN | 0.167763 | 0.331392 | 0.439079 | 0.001487 | 0.001582 | 0.001464 |
| test_bad_function_call_shared.test | RUN | 0.135767 | 0.286738 | 0.321895 | 0.001686 | 0.001706 | 0.001536 |
| test_bad_function_call_static.test | RUN | 0.130098 | 0.281413 | 0.309271 | 0.001637 | 0.001667 | 0.001350 |
| mixed_cxxstd_shared.test | RUN | 0.141214 | 0.336743 | 0.353169 | 0.001668 | 0.001578 | 0.001532 |
| mixed_cxxstd_static.test | RUN | 0.139908 | 0.339683 | 0.345655 | 0.001740 | 0.001258 | 0.001519 |
| mixed_cxxstd_shared_0x.test | RUN | 0.139166 | 0.333658 | 0.363535 | 0.002255 | 0.001837 | 0.001610 |
| mixed_cxxstd_static_0x.test | RUN | 0.142280 | 0.330526 | 0.363046 | 0.001809 | 0.001763 | 0.001392 |
| mixed_cxxstd_shared_14.test | RUN | 0.143663 | 0.337959 | 0.360257 | 0.001750 | 0.001532 | 0.001525 |
| mixed_cxxstd_static_14.test | RUN | 0.146404 | 0.330634 | 0.361360 | 0.001653 | 0.001700 | 0.001466 |
| return_function_shared.test | RUN | 0.138421 | 0.340406 | 0.358530 | 0.001766 | 0.001763 | 0.001513 |
| return_function_static.test | RUN | 0.145174 | 0.344159 | 0.350114 | 0.001596 | 0.001641 | 0.001489 |
| return_function_shared_0x.test | RUN | 0.144588 | 0.347796 | 0.353875 | 0.001876 | 0.001622 | 0.001596 |
| return_function_static_0x.test | RUN | 0.138292 | 0.342152 | 0.355380 | 0.001702 | 0.001483 | 0.001512 |
| return_function_shared_14.test | RUN | 0.138743 | 0.354934 | 0.348357 | 0.001646 | 0.001432 | 0.001470 |
| return_function_static_14.test | RUN | 0.140380 | 0.335481 | 0.350544 | 0.001806 | 0.001654 | 0.001380 |
| quick.test | RUN | 0.143132 | 0.337148 | 0.326594 | 0.001771 | 0.001378 | 0.001471 |
| issue_42.test | COMPILE | 0.109190 | 0.216164 | 0.240453 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.182521 | 0.457909 | 0.518424 | 0.001386 | 0.001476 | 0.001311 |
| contains_test_no_rtti.test | RUN | 0.163451 | 0.516056 | 0.494567 | 0.001576 | 0.001404 | 0.001068 |
| contains2_test_no_rtti.test | RUN | 0.147914 | 0.392015 | 0.387893 | 0.001336 | 0.001457 | 0.001245 |
| contains3_test.test | RUN | 0.140108 | 0.303859 | 0.341779 | 0.001415 | 0.001453 | 0.001055 |
| contains3_test_no_rtti.test | RUN | 0.113059 | 0.245273 | 0.218398 | 0.001324 | 0.001128 | 0.001024 |
| issue_53.test | COMPILE | 0.102494 | 0.165106 | 0.166191 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.002×, 1.006× | 0.998× |
| clang | 0.997× | 0.988×, 1.005× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000877s | 0.000862s | 0.000852s | byte equality observed |
| contains2_test.test | 10 | 0.000882s | 0.000901s | 0.000869s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000870s | 0.000869s | 0.000886s | byte equality observed |
| contains3_test.test | 10 | 0.000865s | 0.000864s | 0.000855s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000866s | 0.000893s | 0.000862s | byte equality observed |
| contains_test.test | 10 | 0.000872s | 0.000880s | 0.000868s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000878s | 0.000872s | 0.000889s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000868s | 0.000870s | 0.000856s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000879s | 0.000890s | 0.000887s | byte equality observed |
| function_arith_portable.test | 10 | 0.000868s | 0.000891s | 0.000880s | byte equality observed |
| function_n_test.test | 10 | 0.000886s | 0.000863s | 0.000876s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000848s | 0.000845s | 0.000856s | byte equality observed |
| function_ref_portable.test | 10 | 0.000842s | 0.000824s | 0.000836s | byte equality observed |
| function_test.test | 10 | 0.000916s | 0.000910s | 0.000913s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.000908s | 0.000906s | 0.000901s | byte equality observed |
| lambda_test.test | 10 | 0.000881s | 0.000854s | 0.000858s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000864s | 0.000861s | 0.000856s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000864s | 0.000854s | 0.000878s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.000931s | 0.000929s | 0.000936s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.000932s | 0.000938s | 0.000943s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.000933s | 0.000937s | 0.000937s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000860s | 0.000857s | 0.000881s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000858s | 0.000870s | 0.000877s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000863s | 0.000869s | 0.000856s | byte equality observed |
| nothrow_swap.test | 10 | 0.000868s | 0.000858s | 0.000852s | byte equality observed |
| quick.test | 10 | 0.000873s | 0.000880s | 0.000866s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000854s | 0.000891s | 0.000855s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000870s | 0.000869s | 0.000854s | byte equality observed |
| return_function_shared.test | 10 | 0.000921s | 0.000944s | 0.000937s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.000937s | 0.000956s | 0.000922s | byte equality observed |
| return_function_shared_14.test | 10 | 0.000928s | 0.000954s | 0.000940s | byte equality observed |
| return_function_static.test | 10 | 0.000860s | 0.000895s | 0.000861s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000872s | 0.000863s | 0.000850s | byte equality observed |
| return_function_static_14.test | 10 | 0.000856s | 0.000860s | 0.000860s | byte equality observed |
| rvalues_test.test | 10 | 0.000878s | 0.000877s | 0.000848s | byte equality observed |
| stateless_test.test | 10 | 0.000855s | 0.000851s | 0.000848s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000847s | 0.000846s | 0.000855s | byte equality observed |
| std_bind_portable.test | 10 | 0.000855s | 0.000843s | 0.000859s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000853s | 0.000845s | 0.000850s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000837s | 0.000849s | 0.000834s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.000962s | 0.000968s | 0.000972s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000883s | 0.000891s | 0.000888s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
