# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.297767 | 1.481114 | 1.355401 | 0.001514 | 0.001976 | 0.001648 |
| function_test_no_rtti.test | RUN | 0.296267 | 1.483601 | 1.353761 | 0.001377 | 0.001850 | 0.001534 |
| function_n_test.test | RUN | 0.277349 | 1.225019 | 1.167265 | 0.001742 | 0.001860 | 0.001848 |
| allocator_test.test | RUN | 0.224953 | 0.699388 | 0.783569 | 0.001922 | 0.001891 | 0.001823 |
| stateless_test.test | RUN | 0.178734 | 0.387945 | 0.497883 | 0.001859 | 0.001570 | 0.001883 |
| lambda_test.test | RUN | 0.393084 | 0.917569 | 1.082911 | 0.001743 | 0.001731 | 0.001654 |
| function_test_fail1.test | COMPILE_FAIL | 0.135143 | 0.255080 | 0.363452 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.137577 | 0.267232 | 0.347160 | — | — | — |
| function_30.test | COMPILE | 0.144527 | 0.264859 | 0.367848 | — | — | — |
| function_30_repeat.test | COMPILE | 0.149247 | 0.268503 | 0.378834 | — | — | — |
| function_arith_cxx98.test | RUN | 0.181807 | 0.457904 | 0.555605 | 0.001772 | 0.001581 | 0.001782 |
| function_arith_portable.test | RUN | 0.182104 | 0.422481 | 0.525547 | 0.001552 | 0.001684 | 0.001742 |
| sum_avg_cxx98.test | RUN | 0.174873 | 0.382864 | 0.477428 | 0.001764 | 0.001685 | 0.001839 |
| sum_avg_portable.test | RUN | 0.179859 | 0.343837 | 0.466931 | 0.001775 | 0.001909 | 0.001824 |
| mem_fun_cxx98.test | RUN | 0.204322 | 0.531109 | 0.628783 | 0.001813 | 0.001503 | 0.001586 |
| mem_fun_portable.test | RUN | 0.195938 | 0.479682 | 0.604156 | 0.001326 | 0.001795 | 0.001637 |
| std_bind_cxx98.test | RUN | 0.182377 | 0.450527 | 0.526609 | 0.001738 | 0.001564 | 0.001849 |
| std_bind_portable.test | RUN | 0.184815 | 0.409777 | 0.528486 | 0.001810 | 0.001876 | 0.001776 |
| function_ref_cxx98.test | RUN | 0.174413 | 0.371330 | 0.469388 | 0.001745 | 0.001688 | 0.001655 |
| function_ref_portable.test | RUN | 0.174622 | 0.343416 | 0.453499 | 0.001762 | 0.001802 | 0.001730 |
| contains_test.test | RUN | 0.207816 | 0.696594 | 0.789462 | 0.001582 | 0.001741 | 0.001739 |
| contains2_test.test | RUN | 0.194037 | 0.504961 | 0.616291 | 0.001878 | 0.001884 | 0.001681 |
| nothrow_swap.test | RUN | 0.188276 | 0.485303 | 0.597576 | 0.001713 | 0.001760 | 0.001694 |
| rvalues_test.test | RUN | 0.211568 | 0.572235 | 0.686447 | 0.001865 | 0.001752 | 0.001761 |
| function_typeof_test.test | COMPILE | 0.143496 | 0.271069 | 0.331449 | — | — | — |
| result_arg_types_test.test | RUN | 0.196474 | 0.411274 | 0.579268 | 0.001650 | 0.001787 | 0.001602 |
| result_arg_n_types_test.test | RUN | 0.205778 | 0.445724 | 0.705736 | 0.001800 | 0.001926 | 0.001732 |
| test_bad_function_call_shared.test | RUN | 0.178180 | 0.368789 | 0.529284 | 0.001811 | 0.001816 | 0.001894 |
| test_bad_function_call_static.test | RUN | 0.177921 | 0.367704 | 0.530820 | 0.001649 | 0.001684 | 0.001911 |
| mixed_cxxstd_shared.test | RUN | 0.185024 | 0.440723 | 0.549127 | 0.001846 | 0.001818 | 0.002118 |
| mixed_cxxstd_static.test | RUN | 0.187466 | 0.447176 | 0.539890 | 0.001609 | 0.001572 | 0.001806 |
| mixed_cxxstd_shared_0x.test | RUN | 0.185711 | 0.441799 | 0.563936 | 0.001579 | 0.002098 | 0.002070 |
| mixed_cxxstd_static_0x.test | RUN | 0.186504 | 0.444216 | 0.563071 | 0.002216 | 0.001780 | 0.001578 |
| mixed_cxxstd_shared_14.test | RUN | 0.192444 | 0.452572 | 0.562192 | 0.001545 | 0.002048 | 0.002022 |
| mixed_cxxstd_static_14.test | RUN | 0.192132 | 0.432694 | 0.563945 | 0.001684 | 0.001925 | 0.001834 |
| return_function_shared.test | RUN | 0.183160 | 0.446656 | 0.548870 | 0.001723 | 0.002005 | 0.002064 |
| return_function_static.test | RUN | 0.189789 | 0.443485 | 0.544440 | 0.001620 | 0.001829 | 0.001475 |
| return_function_shared_0x.test | RUN | 0.185684 | 0.450766 | 0.555599 | 0.001912 | 0.002080 | 0.001974 |
| return_function_static_0x.test | RUN | 0.183747 | 0.438575 | 0.509220 | 0.001816 | 0.001878 | 0.001841 |
| return_function_shared_14.test | RUN | 0.182453 | 0.439808 | 0.540910 | 0.001599 | 0.001747 | 0.002018 |
| return_function_static_14.test | RUN | 0.184461 | 0.444457 | 0.523496 | 0.001802 | 0.001695 | 0.001776 |
| quick.test | RUN | 0.190956 | 0.433931 | 0.543878 | 0.001906 | 0.001924 | 0.001812 |
| issue_42.test | COMPILE | 0.152811 | 0.291594 | 0.397031 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.239998 | 0.619053 | 0.822203 | 0.001608 | 0.001529 | 0.001648 |
| contains_test_no_rtti.test | RUN | 0.210549 | 0.694562 | 0.753811 | 0.001640 | 0.001836 | 0.001492 |
| contains2_test_no_rtti.test | RUN | 0.198823 | 0.522897 | 0.615733 | 0.001624 | 0.001593 | 0.001864 |
| contains3_test.test | RUN | 0.197952 | 0.401334 | 0.526602 | 0.001714 | 0.001692 | 0.001543 |
| contains3_test_no_rtti.test | RUN | 0.146309 | 0.327558 | 0.363958 | 0.001343 | 0.001403 | 0.001272 |
| issue_53.test | COMPILE | 0.129248 | 0.234677 | 0.275262 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 1.001×, 1.003× | 0.998× |
| clang | 0.996× | 0.997×, 0.994× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001011s | 0.001002s | 0.000996s | byte equality observed |
| contains2_test.test | 10 | 0.001018s | 0.001016s | 0.001069s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001000s | 0.001012s | 0.001011s | byte equality observed |
| contains3_test.test | 10 | 0.001012s | 0.001016s | 0.001071s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.000996s | 0.001043s | 0.001050s | byte equality observed |
| contains_test.test | 10 | 0.001000s | 0.001035s | 0.001053s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001011s | 0.001078s | 0.001013s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001021s | 0.001051s | 0.001004s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001076s | 0.001052s | 0.001029s | byte equality observed |
| function_arith_portable.test | 10 | 0.001091s | 0.001071s | 0.001039s | byte equality observed |
| function_n_test.test | 10 | 0.001082s | 0.001055s | 0.001049s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001002s | 0.001031s | 0.001025s | byte equality observed |
| function_ref_portable.test | 10 | 0.001013s | 0.001023s | 0.001017s | byte equality observed |
| function_test.test | 10 | 0.001095s | 0.001082s | 0.001082s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001104s | 0.001103s | 0.001078s | byte equality observed |
| lambda_test.test | 10 | 0.001059s | 0.001055s | 0.001043s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001053s | 0.001035s | 0.001050s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001040s | 0.001045s | 0.001050s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001120s | 0.001107s | 0.001119s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001144s | 0.001118s | 0.001132s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001107s | 0.001135s | 0.001137s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001039s | 0.001084s | 0.001038s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001042s | 0.001052s | 0.001024s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001026s | 0.001032s | 0.001010s | byte equality observed |
| nothrow_swap.test | 10 | 0.001040s | 0.001032s | 0.001045s | byte equality observed |
| quick.test | 10 | 0.001029s | 0.001040s | 0.001036s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001033s | 0.001033s | 0.001065s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001050s | 0.001051s | 0.001024s | byte equality observed |
| return_function_shared.test | 10 | 0.001116s | 0.001142s | 0.001100s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001125s | 0.001108s | 0.001129s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001118s | 0.001112s | 0.001107s | byte equality observed |
| return_function_static.test | 10 | 0.001062s | 0.001036s | 0.001026s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001060s | 0.001063s | 0.001045s | byte equality observed |
| return_function_static_14.test | 10 | 0.001033s | 0.001045s | 0.001064s | byte equality observed |
| rvalues_test.test | 10 | 0.001042s | 0.001062s | 0.001016s | byte equality observed |
| stateless_test.test | 10 | 0.001031s | 0.001034s | 0.001020s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001017s | 0.001035s | 0.001029s | byte equality observed |
| std_bind_portable.test | 10 | 0.001024s | 0.001000s | 0.001014s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001020s | 0.001048s | 0.001005s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001011s | 0.000993s | 0.001009s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001155s | 0.001158s | 0.001138s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001091s | 0.001060s | 0.001039s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
