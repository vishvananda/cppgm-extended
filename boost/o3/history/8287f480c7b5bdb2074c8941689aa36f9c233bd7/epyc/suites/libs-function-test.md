# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.292279 | 1.460039 | 1.332147 | 0.001360 | 0.001835 | 0.001732 |
| function_test_no_rtti.test | RUN | 0.285536 | 1.462220 | 1.334379 | 0.001547 | 0.001549 | 0.001603 |
| function_n_test.test | RUN | 0.272255 | 1.225469 | 1.141009 | 0.001669 | 0.001836 | 0.001750 |
| allocator_test.test | RUN | 0.221495 | 0.681571 | 0.775506 | 0.001762 | 0.001722 | 0.001736 |
| stateless_test.test | RUN | 0.175362 | 0.376604 | 0.490874 | 0.001750 | 0.001516 | 0.001710 |
| lambda_test.test | RUN | 0.370071 | 0.911877 | 1.076212 | 0.001786 | 0.001618 | 0.001672 |
| function_test_fail1.test | COMPILE_FAIL | 0.137159 | 0.252029 | 0.331616 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.136048 | 0.255921 | 0.363240 | — | — | — |
| function_30.test | COMPILE | 0.143960 | 0.277735 | 0.353967 | — | — | — |
| function_30_repeat.test | COMPILE | 0.140912 | 0.268117 | 0.374851 | — | — | — |
| function_arith_cxx98.test | RUN | 0.182907 | 0.451393 | 0.535773 | 0.001771 | 0.001432 | 0.001616 |
| function_arith_portable.test | RUN | 0.182138 | 0.421881 | 0.508848 | 0.001458 | 0.001352 | 0.001684 |
| sum_avg_cxx98.test | RUN | 0.178647 | 0.382450 | 0.461829 | 0.001596 | 0.001814 | 0.001699 |
| sum_avg_portable.test | RUN | 0.176226 | 0.336468 | 0.479922 | 0.001728 | 0.001825 | 0.001655 |
| mem_fun_cxx98.test | RUN | 0.196641 | 0.521440 | 0.592641 | 0.001563 | 0.001617 | 0.001746 |
| mem_fun_portable.test | RUN | 0.193140 | 0.469683 | 0.595867 | 0.001516 | 0.001776 | 0.001572 |
| std_bind_cxx98.test | RUN | 0.183594 | 0.432783 | 0.494408 | 0.001592 | 0.001508 | 0.001707 |
| std_bind_portable.test | RUN | 0.184324 | 0.402903 | 0.474418 | 0.001814 | 0.001743 | 0.001732 |
| function_ref_cxx98.test | RUN | 0.177386 | 0.372131 | 0.466951 | 0.001682 | 0.001652 | 0.001508 |
| function_ref_portable.test | RUN | 0.173136 | 0.330059 | 0.452321 | 0.001580 | 0.001583 | 0.001643 |
| contains_test.test | RUN | 0.205989 | 0.695863 | 0.730849 | 0.001630 | 0.001626 | 0.001690 |
| contains2_test.test | RUN | 0.189007 | 0.507358 | 0.568860 | 0.001779 | 0.001709 | 0.001624 |
| nothrow_swap.test | RUN | 0.187889 | 0.495950 | 0.541601 | 0.001712 | 0.001948 | 0.001883 |
| rvalues_test.test | RUN | 0.208786 | 0.621962 | 0.624050 | 0.001756 | 0.001922 | 0.001773 |
| function_typeof_test.test | COMPILE | 0.143229 | 0.297034 | 0.366064 | — | — | — |
| result_arg_types_test.test | RUN | 0.198043 | 0.408441 | 0.603863 | 0.001668 | 0.001728 | 0.001592 |
| result_arg_n_types_test.test | RUN | 0.200944 | 0.437664 | 0.676581 | 0.001822 | 0.001646 | 0.001734 |
| test_bad_function_call_shared.test | RUN | 0.180369 | 0.374803 | 0.481691 | 0.001650 | 0.002079 | 0.001756 |
| test_bad_function_call_static.test | RUN | 0.177898 | 0.374681 | 0.512590 | 0.001618 | 0.001804 | 0.001703 |
| mixed_cxxstd_shared.test | RUN | 0.188291 | 0.442562 | 0.516860 | 0.001731 | 0.001679 | 0.001895 |
| mixed_cxxstd_static.test | RUN | 0.192037 | 0.440570 | 0.567988 | 0.001583 | 0.001619 | 0.001563 |
| mixed_cxxstd_shared_0x.test | RUN | 0.188108 | 0.445097 | 0.519317 | 0.001765 | 0.002004 | 0.001958 |
| mixed_cxxstd_static_0x.test | RUN | 0.182849 | 0.439780 | 0.560182 | 0.001533 | 0.001630 | 0.001706 |
| mixed_cxxstd_shared_14.test | RUN | 0.186076 | 0.445886 | 0.555265 | 0.001535 | 0.001708 | 0.001714 |
| mixed_cxxstd_static_14.test | RUN | 0.183884 | 0.446315 | 0.563380 | 0.001682 | 0.002158 | 0.001724 |
| return_function_shared.test | RUN | 0.185096 | 0.448048 | 0.556789 | 0.001654 | 0.001732 | 0.001698 |
| return_function_static.test | RUN | 0.184266 | 0.433787 | 0.548568 | 0.001586 | 0.001816 | 0.001718 |
| return_function_shared_0x.test | RUN | 0.185091 | 0.454134 | 0.541443 | 0.001488 | 0.002062 | 0.001916 |
| return_function_static_0x.test | RUN | 0.180222 | 0.432681 | 0.547911 | 0.001545 | 0.001876 | 0.001654 |
| return_function_shared_14.test | RUN | 0.187994 | 0.446350 | 0.513683 | 0.001876 | 0.002066 | 0.001717 |
| return_function_static_14.test | RUN | 0.181691 | 0.432820 | 0.542556 | 0.001449 | 0.001664 | 0.001749 |
| quick.test | RUN | 0.187192 | 0.433910 | 0.555434 | 0.001528 | 0.001659 | 0.001745 |
| issue_42.test | COMPILE | 0.144876 | 0.289205 | 0.355304 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.235953 | 0.603947 | 0.792608 | 0.001486 | 0.001837 | 0.001431 |
| contains_test_no_rtti.test | RUN | 0.205586 | 0.688288 | 0.762203 | 0.001769 | 0.001873 | 0.001646 |
| contains2_test_no_rtti.test | RUN | 0.196500 | 0.520234 | 0.601508 | 0.001598 | 0.001818 | 0.001624 |
| contains3_test.test | RUN | 0.182535 | 0.387766 | 0.546513 | 0.001505 | 0.001384 | 0.001313 |
| contains3_test_no_rtti.test | RUN | 0.125355 | 0.340178 | 0.345147 | 0.001317 | 0.001328 | 0.001278 |
| issue_53.test | COMPILE | 0.110063 | 0.248968 | 0.251490 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.006×, 1.002× | 0.993× |
| clang | 1.008× | 1.002×, 1.014× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001004s | 0.000991s | 0.001011s | byte equality observed |
| contains2_test.test | 10 | 0.001006s | 0.001013s | 0.001025s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.001014s | 0.001004s | 0.001061s | byte equality observed |
| contains3_test.test | 10 | 0.001001s | 0.001007s | 0.001064s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001009s | 0.000995s | 0.001056s | byte equality observed |
| contains_test.test | 10 | 0.001004s | 0.001043s | 0.001045s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001026s | 0.001032s | 0.001031s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001034s | 0.001038s | 0.001048s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001055s | 0.001026s | 0.001048s | byte equality observed |
| function_arith_portable.test | 10 | 0.001059s | 0.001051s | 0.001073s | byte equality observed |
| function_n_test.test | 10 | 0.001041s | 0.001034s | 0.001038s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.001040s | 0.001018s | 0.001022s | byte equality observed |
| function_ref_portable.test | 10 | 0.001008s | 0.001003s | 0.001005s | byte equality observed |
| function_test.test | 10 | 0.001071s | 0.001072s | 0.001076s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001068s | 0.001081s | 0.001055s | byte equality observed |
| lambda_test.test | 10 | 0.001026s | 0.001014s | 0.001044s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001046s | 0.001007s | 0.001041s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001047s | 0.001044s | 0.001018s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001108s | 0.001100s | 0.001107s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001113s | 0.001154s | 0.001103s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001102s | 0.001109s | 0.001120s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001030s | 0.001019s | 0.001023s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001029s | 0.001011s | 0.001024s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001046s | 0.001024s | 0.001044s | byte equality observed |
| nothrow_swap.test | 10 | 0.001040s | 0.001065s | 0.001049s | byte equality observed |
| quick.test | 10 | 0.001032s | 0.001038s | 0.001053s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001011s | 0.001074s | 0.001038s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001032s | 0.001022s | 0.001019s | byte equality observed |
| return_function_shared.test | 10 | 0.001115s | 0.001105s | 0.001130s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001099s | 0.001129s | 0.001107s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001107s | 0.001104s | 0.001125s | byte equality observed |
| return_function_static.test | 10 | 0.001010s | 0.001046s | 0.001025s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001026s | 0.001041s | 0.001036s | byte equality observed |
| return_function_static_14.test | 10 | 0.001039s | 0.001017s | 0.001051s | byte equality observed |
| rvalues_test.test | 10 | 0.001034s | 0.001041s | 0.001016s | byte equality observed |
| stateless_test.test | 10 | 0.001016s | 0.001027s | 0.001057s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.001005s | 0.001019s | 0.001018s | byte equality observed |
| std_bind_portable.test | 10 | 0.001008s | 0.001005s | 0.001028s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.001015s | 0.000980s | 0.001022s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001005s | 0.001009s | 0.000997s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001118s | 0.001138s | 0.001118s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001044s | 0.001042s | 0.001056s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
