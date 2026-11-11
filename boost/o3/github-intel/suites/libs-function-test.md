# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.371350 | 1.860721 | 1.533629 | 0.001652 | 0.001814 | 0.001545 |
| function_test_no_rtti.test | RUN | 0.369623 | 1.845066 | 1.567786 | 0.001630 | 0.001880 | 0.001644 |
| function_n_test.test | RUN | 0.336884 | 1.518431 | 1.290301 | 0.001727 | 0.001750 | 0.001748 |
| allocator_test.test | RUN | 0.265862 | 0.852888 | 0.852915 | 0.001855 | 0.001858 | 0.001694 |
| stateless_test.test | RUN | 0.206093 | 0.456208 | 0.535085 | 0.001742 | 0.001552 | 0.001403 |
| lambda_test.test | RUN | 0.441321 | 1.121504 | 1.142823 | 0.001810 | 0.001646 | 0.001725 |
| function_test_fail1.test | COMPILE_FAIL | 0.159618 | 0.304350 | 0.368918 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.160211 | 0.308297 | 0.374890 | — | — | — |
| function_30.test | COMPILE | 0.163131 | 0.329130 | 0.378452 | — | — | — |
| function_30_repeat.test | COMPILE | 0.163485 | 0.327776 | 0.381007 | — | — | — |
| function_arith_cxx98.test | RUN | 0.217381 | 0.558012 | 0.592337 | 0.001648 | 0.001480 | 0.001718 |
| function_arith_portable.test | RUN | 0.200189 | 0.512012 | 0.544625 | 0.001730 | 0.001646 | 0.001637 |
| sum_avg_cxx98.test | RUN | 0.207165 | 0.448475 | 0.520233 | 0.001582 | 0.001308 | 0.001663 |
| sum_avg_portable.test | RUN | 0.196283 | 0.405867 | 0.501939 | 0.001790 | 0.001702 | 0.001724 |
| mem_fun_cxx98.test | RUN | 0.231407 | 0.634132 | 0.642693 | 0.001814 | 0.001540 | 0.001480 |
| mem_fun_portable.test | RUN | 0.228926 | 0.583899 | 0.619293 | 0.001524 | 0.001538 | 0.001572 |
| std_bind_cxx98.test | RUN | 0.213379 | 0.528674 | 0.569529 | 0.001562 | 0.001746 | 0.001540 |
| std_bind_portable.test | RUN | 0.217772 | 0.491228 | 0.527060 | 0.001860 | 0.001714 | 0.001699 |
| function_ref_cxx98.test | RUN | 0.206242 | 0.448389 | 0.523291 | 0.001661 | 0.001682 | 0.001525 |
| function_ref_portable.test | RUN | 0.199165 | 0.403617 | 0.470325 | 0.001751 | 0.001570 | 0.001498 |
| contains_test.test | RUN | 0.245170 | 0.850080 | 0.816100 | 0.001626 | 0.001675 | 0.001521 |
| contains2_test.test | RUN | 0.224684 | 0.608571 | 0.617522 | 0.001747 | 0.001788 | 0.001472 |
| nothrow_swap.test | RUN | 0.227171 | 0.569644 | 0.595987 | 0.001500 | 0.001796 | 0.001653 |
| rvalues_test.test | RUN | 0.253540 | 0.703522 | 0.692931 | 0.001827 | 0.001648 | 0.001647 |
| function_typeof_test.test | COMPILE | 0.163148 | 0.313073 | 0.356792 | — | — | — |
| result_arg_types_test.test | RUN | 0.235259 | 0.488895 | 0.626573 | 0.001646 | 0.001718 | 0.001746 |
| result_arg_n_types_test.test | RUN | 0.247245 | 0.534676 | 0.752848 | 0.001818 | 0.001743 | 0.001572 |
| test_bad_function_call_shared.test | RUN | 0.209624 | 0.457562 | 0.546110 | 0.001747 | 0.001965 | 0.001635 |
| test_bad_function_call_static.test | RUN | 0.206393 | 0.449771 | 0.534470 | 0.001461 | 0.001820 | 0.001678 |
| mixed_cxxstd_shared.test | RUN | 0.220660 | 0.528237 | 0.588065 | 0.001700 | 0.001676 | 0.001812 |
| mixed_cxxstd_static.test | RUN | 0.216022 | 0.535787 | 0.585675 | 0.001679 | 0.001633 | 0.001709 |
| mixed_cxxstd_shared_0x.test | RUN | 0.215373 | 0.532582 | 0.601427 | 0.001483 | 0.001969 | 0.001794 |
| mixed_cxxstd_static_0x.test | RUN | 0.215627 | 0.535464 | 0.596472 | 0.001511 | 0.001733 | 0.002152 |
| mixed_cxxstd_shared_14.test | RUN | 0.223552 | 0.534547 | 0.600716 | 0.001929 | 0.001680 | 0.001829 |
| mixed_cxxstd_static_14.test | RUN | 0.220659 | 0.530690 | 0.604969 | 0.001712 | 0.001667 | 0.001767 |
| return_function_shared.test | RUN | 0.213387 | 0.532406 | 0.583012 | 0.001587 | 0.001908 | 0.001789 |
| return_function_static.test | RUN | 0.215472 | 0.530883 | 0.574016 | 0.001693 | 0.001567 | 0.001712 |
| return_function_shared_0x.test | RUN | 0.214785 | 0.542007 | 0.581843 | 0.001610 | 0.001892 | 0.001852 |
| return_function_static_0x.test | RUN | 0.215469 | 0.547446 | 0.565718 | 0.001808 | 0.001564 | 0.001640 |
| return_function_shared_14.test | RUN | 0.217329 | 0.549192 | 0.579835 | 0.001765 | 0.001694 | 0.001554 |
| return_function_static_14.test | RUN | 0.213402 | 0.532353 | 0.587917 | 0.001504 | 0.001850 | 0.001716 |
| quick.test | RUN | 0.218100 | 0.534344 | 0.565558 | 0.001586 | 0.001714 | 0.001681 |
| issue_42.test | COMPILE | 0.164683 | 0.338901 | 0.405243 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.277978 | 0.723956 | 0.861160 | 0.001543 | 0.001748 | 0.001538 |
| contains_test_no_rtti.test | RUN | 0.247597 | 0.835065 | 0.818008 | 0.001709 | 0.001162 | 0.001342 |
| contains2_test_no_rtti.test | RUN | 0.227022 | 0.606379 | 0.635533 | 0.001522 | 0.001632 | 0.001760 |
| contains3_test.test | RUN | 0.216952 | 0.482043 | 0.566367 | 0.001690 | 0.001658 | 0.001498 |
| contains3_test_no_rtti.test | RUN | 0.141263 | 0.393451 | 0.384070 | 0.001350 | 0.001265 | 0.001218 |
| issue_53.test | COMPILE | 0.122235 | 0.265578 | 0.280256 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.996×, 0.997× | 1.012× |
| clang | 0.998× | 0.990×, 1.005× | 0.983× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001011s | 0.001046s | 0.001005s | byte equality observed |
| contains2_test.test | 10 | 0.001053s | 0.001070s | 0.000992s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001057s | 0.001074s | 0.000981s | byte equality observed |
| contains3_test.test | 10 | 0.001081s | 0.001078s | 0.001013s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001065s | 0.001058s | 0.001076s | byte equality observed |
| contains_test.test | 10 | 0.001050s | 0.001052s | 0.001083s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001072s | 0.001032s | 0.001079s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001101s | 0.001034s | 0.001087s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001087s | 0.001053s | 0.001077s | byte equality observed |
| function_arith_portable.test | 10 | 0.001119s | 0.001063s | 0.001054s | byte equality observed |
| function_n_test.test | 10 | 0.001089s | 0.001070s | 0.001061s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001044s | 0.001031s | 0.001047s | byte equality observed |
| function_ref_portable.test | 10 | 0.001054s | 0.001029s | 0.001034s | byte equality observed |
| function_test.test | 10 | 0.001120s | 0.001109s | 0.001098s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001101s | 0.001124s | 0.001115s | byte equality observed |
| lambda_test.test | 10 | 0.001086s | 0.001078s | 0.001078s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001077s | 0.001097s | 0.001076s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001079s | 0.001036s | 0.001073s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001133s | 0.001130s | 0.001151s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001142s | 0.001160s | 0.001121s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001101s | 0.001133s | 0.001141s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001083s | 0.001080s | 0.001038s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001079s | 0.001090s | 0.001030s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001071s | 0.001057s | 0.001079s | byte equality observed |
| nothrow_swap.test | 10 | 0.001062s | 0.001080s | 0.001089s | byte equality observed |
| quick.test | 10 | 0.001080s | 0.001077s | 0.001031s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001069s | 0.001083s | 0.001076s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001033s | 0.001046s | 0.001050s | byte equality observed |
| return_function_shared.test | 10 | 0.001120s | 0.001119s | 0.001139s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001121s | 0.001098s | 0.001153s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001111s | 0.001126s | 0.001112s | byte equality observed |
| return_function_static.test | 10 | 0.001072s | 0.001033s | 0.001044s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001031s | 0.001064s | 0.001061s | byte equality observed |
| return_function_static_14.test | 10 | 0.001075s | 0.001066s | 0.001061s | byte equality observed |
| rvalues_test.test | 10 | 0.001082s | 0.001060s | 0.001047s | byte equality observed |
| stateless_test.test | 10 | 0.001048s | 0.001016s | 0.001091s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001039s | 0.001043s | 0.001044s | byte equality observed |
| std_bind_portable.test | 10 | 0.001025s | 0.001015s | 0.001010s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001023s | 0.001003s | 0.001033s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001011s | 0.001021s | 0.001020s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001145s | 0.001146s | 0.001193s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001107s | 0.001071s | 0.001059s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
