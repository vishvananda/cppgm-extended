# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.246771 | 1.185393 | 1.122506 | 0.001615 | 0.001605 | 0.001646 |
| function_test_no_rtti.test | RUN | 0.239448 | 1.185588 | 1.160629 | 0.001588 | 0.001574 | 0.001768 |
| function_n_test.test | RUN | 0.230720 | 0.965447 | 0.910687 | 0.001557 | 0.001767 | 0.001839 |
| allocator_test.test | RUN | 0.181995 | 0.552419 | 0.620364 | 0.001846 | 0.001833 | 0.001918 |
| stateless_test.test | RUN | 0.150315 | 0.305963 | 0.417755 | 0.001840 | 0.001536 | 0.001876 |
| lambda_test.test | RUN | 0.333287 | 0.715602 | 0.832517 | 0.001804 | 0.001643 | 0.001909 |
| function_test_fail1.test | COMPILE_FAIL | 0.117189 | 0.202142 | 0.266437 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.118272 | 0.190398 | 0.265415 | — | — | — |
| function_30.test | COMPILE | 0.125865 | 0.199636 | 0.269098 | — | — | — |
| function_30_repeat.test | COMPILE | 0.125053 | 0.221090 | 0.292873 | — | — | — |
| function_arith_cxx98.test | RUN | 0.159153 | 0.355354 | 0.420760 | 0.001742 | 0.001462 | 0.002064 |
| function_arith_portable.test | RUN | 0.152394 | 0.331619 | 0.400331 | 0.002215 | 0.001519 | 0.002098 |
| sum_avg_cxx98.test | RUN | 0.146511 | 0.288050 | 0.367756 | 0.001694 | 0.003271 | 0.001831 |
| sum_avg_portable.test | RUN | 0.160503 | 0.313893 | 0.359222 | 0.001856 | 0.001718 | 0.001930 |
| mem_fun_cxx98.test | RUN | 0.185415 | 0.456184 | 0.477138 | 0.001849 | 0.001467 | 0.002012 |
| mem_fun_portable.test | RUN | 0.171299 | 0.400227 | 0.441188 | 0.001404 | 0.001487 | 0.001963 |
| std_bind_cxx98.test | RUN | 0.163710 | 0.348760 | 0.408783 | 0.001897 | 0.001551 | 0.001798 |
| std_bind_portable.test | RUN | 0.165157 | 0.326335 | 0.387714 | 0.001734 | 0.001744 | 0.001822 |
| function_ref_cxx98.test | RUN | 0.146960 | 0.292438 | 0.355095 | 0.001727 | 0.001627 | 0.001968 |
| function_ref_portable.test | RUN | 0.144660 | 0.261946 | 0.367184 | 0.001830 | 0.001318 | 0.001856 |
| contains_test.test | RUN | 0.173218 | 0.557710 | 0.581131 | 0.001575 | 0.001577 | 0.001934 |
| contains2_test.test | RUN | 0.164350 | 0.405173 | 0.454583 | 0.001772 | 0.001490 | 0.001870 |
| nothrow_swap.test | RUN | 0.161155 | 0.375318 | 0.446264 | 0.001399 | 0.001687 | 0.002018 |
| rvalues_test.test | RUN | 0.186008 | 0.454680 | 0.517120 | 0.001862 | 0.001601 | 0.001945 |
| function_typeof_test.test | COMPILE | 0.121366 | 0.213071 | 0.280537 | — | — | — |
| result_arg_types_test.test | RUN | 0.174192 | 0.326366 | 0.453179 | 0.003285 | 0.001603 | 0.001440 |
| result_arg_n_types_test.test | RUN | 0.189083 | 0.343639 | 0.533509 | 0.001808 | 0.001668 | 0.001796 |
| test_bad_function_call_shared.test | RUN | 0.193208 | 0.293387 | 0.356000 | 0.002808 | 0.002124 | 0.001764 |
| test_bad_function_call_static.test | RUN | 0.148493 | 0.324072 | 0.350349 | 0.002902 | 0.001820 | 0.001572 |
| mixed_cxxstd_shared.test | RUN | 0.180764 | 0.368489 | 0.421594 | 0.001847 | 0.001678 | 0.002002 |
| mixed_cxxstd_static.test | RUN | 0.171254 | 0.357709 | 0.421497 | 0.001600 | 0.001554 | 0.001459 |
| mixed_cxxstd_shared_0x.test | RUN | 0.171740 | 0.356919 | 0.418763 | 0.001872 | 0.001959 | 0.002137 |
| mixed_cxxstd_static_0x.test | RUN | 0.155563 | 0.348973 | 0.392012 | 0.001500 | 0.001698 | 0.002006 |
| mixed_cxxstd_shared_14.test | RUN | 0.158491 | 0.352338 | 0.440495 | 0.001603 | 0.001655 | 0.001967 |
| mixed_cxxstd_static_14.test | RUN | 0.160318 | 0.347291 | 0.415217 | 0.001591 | 0.001730 | 0.001904 |
| return_function_shared.test | RUN | 0.152050 | 0.355044 | 0.404849 | 0.001680 | 0.001826 | 0.001688 |
| return_function_static.test | RUN | 0.155235 | 0.359594 | 0.393967 | 0.001498 | 0.001606 | 0.001579 |
| return_function_shared_0x.test | RUN | 0.159193 | 0.370023 | 0.417877 | 0.001724 | 0.002053 | 0.001934 |
| return_function_static_0x.test | RUN | 0.159298 | 0.367675 | 0.425994 | 0.001823 | 0.001675 | 0.001799 |
| return_function_shared_14.test | RUN | 0.162135 | 0.408153 | 0.387662 | 0.001642 | 0.001656 | 0.001751 |
| return_function_static_14.test | RUN | 0.155102 | 0.411121 | 0.402810 | 0.001530 | 0.001757 | 0.001886 |
| quick.test | RUN | 0.157233 | 0.405565 | 0.412658 | 0.001674 | 0.001476 | 0.001845 |
| issue_42.test | COMPILE | 0.122365 | 0.242110 | 0.264083 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.195880 | 0.503461 | 0.625931 | 0.002908 | 0.001826 | 0.001764 |
| contains_test_no_rtti.test | RUN | 0.172041 | 0.571387 | 0.587618 | 0.002431 | 0.001537 | 0.001283 |
| contains2_test_no_rtti.test | RUN | 0.162201 | 0.415084 | 0.458394 | 0.002793 | 0.001553 | 0.001620 |
| contains3_test.test | RUN | 0.161029 | 0.325114 | 0.420173 | 0.001978 | 0.001704 | 0.001603 |
| contains3_test_no_rtti.test | RUN | 0.113002 | 0.268940 | 0.277062 | 0.001374 | 0.001262 | 0.001396 |
| issue_53.test | COMPILE | 0.106055 | 0.181066 | 0.212351 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.016× | 1.017×, 1.015× | 0.951× |
| clang | 0.993× | 0.971×, 1.014× | 1.038× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.000961s | 0.000991s | 0.000946s | byte equality observed |
| contains2_test.test | 10 | 0.000994s | 0.000964s | 0.000953s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000979s | 0.000982s | 0.000960s | byte equality observed |
| contains3_test.test | 10 | 0.000959s | 0.000966s | 0.000932s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000966s | 0.000966s | 0.000935s | byte equality observed |
| contains_test.test | 10 | 0.000965s | 0.000991s | 0.000959s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.000968s | 0.000979s | 0.000962s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.000943s | 0.000977s | 0.000958s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.000981s | 0.001003s | 0.000958s | byte equality observed |
| function_arith_portable.test | 10 | 0.000996s | 0.000997s | 0.000989s | byte equality observed |
| function_n_test.test | 10 | 0.000990s | 0.001017s | 0.000978s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000934s | 0.000962s | 0.000932s | byte equality observed |
| function_ref_portable.test | 10 | 0.000935s | 0.000947s | 0.000966s | byte equality observed |
| function_test.test | 10 | 0.001010s | 0.001029s | 0.001012s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001037s | 0.001017s | 0.001019s | byte equality observed |
| lambda_test.test | 10 | 0.000956s | 0.000963s | 0.000951s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.000982s | 0.000972s | 0.000967s | byte equality observed |
| mem_fun_portable.test | 10 | 0.000974s | 0.000979s | 0.000947s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001035s | 0.001076s | 0.001077s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001041s | 0.001084s | 0.001063s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001048s | 0.001084s | 0.001073s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.000975s | 0.000973s | 0.000973s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.000983s | 0.000987s | 0.000963s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.000979s | 0.000987s | 0.000965s | byte equality observed |
| nothrow_swap.test | 10 | 0.000978s | 0.000975s | 0.000983s | byte equality observed |
| quick.test | 10 | 0.000965s | 0.000983s | 0.000982s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.000956s | 0.001019s | 0.000977s | byte equality observed |
| result_arg_types_test.test | 10 | 0.000943s | 0.000998s | 0.000954s | byte equality observed |
| return_function_shared.test | 10 | 0.001015s | 0.001073s | 0.001052s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001050s | 0.001061s | 0.001039s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001036s | 0.001055s | 0.001045s | byte equality observed |
| return_function_static.test | 10 | 0.000960s | 0.000983s | 0.000973s | byte equality observed |
| return_function_static_0x.test | 10 | 0.000944s | 0.000966s | 0.000963s | byte equality observed |
| return_function_static_14.test | 10 | 0.000938s | 0.000978s | 0.000956s | byte equality observed |
| rvalues_test.test | 10 | 0.000957s | 0.000996s | 0.000977s | byte equality observed |
| stateless_test.test | 10 | 0.000958s | 0.000973s | 0.000965s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000945s | 0.000968s | 0.000933s | byte equality observed |
| std_bind_portable.test | 10 | 0.000939s | 0.000955s | 0.000967s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000952s | 0.000946s | 0.000934s | byte equality observed |
| sum_avg_portable.test | 10 | 0.000917s | 0.000950s | 0.000951s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001061s | 0.001091s | 0.001086s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.000983s | 0.000991s | 0.000976s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
