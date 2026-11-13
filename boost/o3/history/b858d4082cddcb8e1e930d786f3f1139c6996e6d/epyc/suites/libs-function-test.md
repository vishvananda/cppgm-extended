# libs/function/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| function_test.test | RUN | 0.276427 | 1.494005 | 1.337412 | 0.001774 | 0.001759 | 0.001824 |
| function_test_no_rtti.test | RUN | 0.272489 | 1.482069 | 1.361857 | 0.001441 | 0.001875 | 0.001699 |
| function_n_test.test | RUN | 0.259743 | 1.231496 | 1.158881 | 0.001868 | 0.001955 | 0.001756 |
| allocator_test.test | RUN | 0.212011 | 0.694731 | 0.788194 | 0.001956 | 0.001883 | 0.001880 |
| stateless_test.test | RUN | 0.167153 | 0.393385 | 0.505983 | 0.001794 | 0.001639 | 0.001925 |
| lambda_test.test | RUN | 0.347831 | 0.934309 | 1.108276 | 0.001844 | 0.001858 | 0.001962 |
| function_test_fail1.test | COMPILE_FAIL | 0.123333 | 0.259277 | 0.370681 | — | — | — |
| function_test_fail2.test | COMPILE_FAIL | 0.127022 | 0.265805 | 0.345726 | — | — | — |
| function_30.test | COMPILE | 0.129913 | 0.285257 | 0.376231 | — | — | — |
| function_30_repeat.test | COMPILE | 0.132286 | 0.277489 | 0.383428 | — | — | — |
| function_arith_cxx98.test | RUN | 0.175153 | 0.460039 | 0.556339 | 0.001808 | 0.001728 | 0.001772 |
| function_arith_portable.test | RUN | 0.164886 | 0.433736 | 0.520595 | 0.001792 | 0.001492 | 0.001967 |
| sum_avg_cxx98.test | RUN | 0.163525 | 0.404951 | 0.478327 | 0.001541 | 0.001956 | 0.001721 |
| sum_avg_portable.test | RUN | 0.162909 | 0.366731 | 0.486746 | 0.001737 | 0.001866 | 0.001763 |
| mem_fun_cxx98.test | RUN | 0.181103 | 0.542404 | 0.592625 | 0.001925 | 0.002084 | 0.001722 |
| mem_fun_portable.test | RUN | 0.182152 | 0.493430 | 0.585523 | 0.001655 | 0.001789 | 0.001704 |
| std_bind_cxx98.test | RUN | 0.171256 | 0.452274 | 0.514492 | 0.001577 | 0.001669 | 0.001708 |
| std_bind_portable.test | RUN | 0.171326 | 0.420412 | 0.509912 | 0.001769 | 0.001856 | 0.001718 |
| function_ref_cxx98.test | RUN | 0.163418 | 0.388642 | 0.466042 | 0.001749 | 0.001600 | 0.001486 |
| function_ref_portable.test | RUN | 0.157523 | 0.362935 | 0.451986 | 0.001712 | 0.001487 | 0.001827 |
| contains_test.test | RUN | 0.195104 | 0.714458 | 0.764250 | 0.001570 | 0.001704 | 0.001819 |
| contains2_test.test | RUN | 0.182188 | 0.519204 | 0.598959 | 0.001860 | 0.001723 | 0.001574 |
| nothrow_swap.test | RUN | 0.171224 | 0.485780 | 0.561738 | 0.001692 | 0.001612 | 0.001506 |
| rvalues_test.test | RUN | 0.201108 | 0.592700 | 0.663739 | 0.001677 | 0.001736 | 0.001463 |
| function_typeof_test.test | COMPILE | 0.133702 | 0.263309 | 0.356871 | — | — | — |
| result_arg_types_test.test | RUN | 0.191787 | 0.424387 | 0.619108 | 0.001614 | 0.001785 | 0.001928 |
| result_arg_n_types_test.test | RUN | 0.199023 | 0.458806 | 0.706986 | 0.002194 | 0.001727 | 0.001839 |
| test_bad_function_call_shared.test | RUN | 0.167116 | 0.401547 | 0.514427 | 0.001784 | 0.001450 | 0.002129 |
| test_bad_function_call_static.test | RUN | 0.186792 | 0.382152 | 0.530921 | 0.001628 | 0.001830 | 0.001747 |
| mixed_cxxstd_shared.test | RUN | 0.175689 | 0.469479 | 0.581163 | 0.001805 | 0.002042 | 0.002096 |
| mixed_cxxstd_static.test | RUN | 0.169217 | 0.455912 | 0.583532 | 0.001856 | 0.001581 | 0.001623 |
| mixed_cxxstd_shared_0x.test | RUN | 0.170783 | 0.439635 | 0.514100 | 0.002045 | 0.002058 | 0.001894 |
| mixed_cxxstd_static_0x.test | RUN | 0.178328 | 0.453866 | 0.560689 | 0.001746 | 0.001931 | 0.001883 |
| mixed_cxxstd_shared_14.test | RUN | 0.175318 | 0.447693 | 0.580428 | 0.001978 | 0.002102 | 0.001633 |
| mixed_cxxstd_static_14.test | RUN | 0.170800 | 0.459953 | 0.538282 | 0.001817 | 0.002162 | 0.001689 |
| return_function_shared.test | RUN | 0.169618 | 0.463157 | 0.544115 | 0.001675 | 0.002070 | 0.001875 |
| return_function_static.test | RUN | 0.168154 | 0.470331 | 0.562486 | 0.001412 | 0.001802 | 0.001599 |
| return_function_shared_0x.test | RUN | 0.165381 | 0.439664 | 0.531158 | 0.001843 | 0.002143 | 0.002060 |
| return_function_static_0x.test | RUN | 0.171523 | 0.444288 | 0.583277 | 0.001892 | 0.001767 | 0.001876 |
| return_function_shared_14.test | RUN | 0.175308 | 0.445713 | 0.524660 | 0.001830 | 0.001724 | 0.001928 |
| return_function_static_14.test | RUN | 0.168702 | 0.469579 | 0.542952 | 0.001580 | 0.001770 | 0.001859 |
| quick.test | RUN | 0.171801 | 0.484699 | 0.532538 | 0.001908 | 0.001828 | 0.001805 |
| issue_42.test | COMPILE | 0.139691 | 0.296284 | 0.389939 | — | — | — |
| fn_eq_bind_test.test | RUN | 0.226085 | 0.627504 | 0.818980 | 0.001592 | 0.001772 | 0.001593 |
| contains_test_no_rtti.test | RUN | 0.198552 | 0.719864 | 0.737806 | 0.001402 | 0.001677 | 0.001626 |
| contains2_test_no_rtti.test | RUN | 0.182902 | 0.533141 | 0.595800 | 0.001594 | 0.001607 | 0.001432 |
| contains3_test.test | RUN | 0.171724 | 0.414059 | 0.543952 | 0.001350 | 0.001658 | 0.001884 |
| contains3_test_no_rtti.test | RUN | 0.131688 | 0.336819 | 0.365991 | 0.001419 | 0.001396 | 0.001301 |
| issue_53.test | COMPILE | 0.118052 | 0.228536 | 0.268070 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.030×† | 1.073×, 0.987× | 0.997× |
| clang | 1.022× | 1.034×, 1.010× | 0.987× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| allocator_test.test | 10 | 0.001012s | 0.001058s | 0.000996s | byte equality observed |
| contains2_test.test | 10 | 0.000997s | 0.001035s | 0.000991s | byte equality observed |
| contains2_test_no_rtti.test | 10 | 0.000991s | 0.001035s | 0.001009s | byte equality observed |
| contains3_test.test | 10 | 0.001000s | 0.001058s | 0.001014s | byte equality observed |
| contains3_test_no_rtti.test | 10 | 0.001009s | 0.001027s | 0.001019s | byte equality observed |
| contains_test.test | 10 | 0.001039s | 0.001016s | 0.001031s | byte equality observed |
| contains_test_no_rtti.test | 10 | 0.001042s | 0.001026s | 0.001028s | byte equality observed |
| fn_eq_bind_test.test | 10 | 0.001038s | 0.001029s | 0.001024s | byte equality observed |
| function_arith_cxx98.test | 10 | 0.001048s | 0.001070s | 0.001063s | byte equality observed |
| function_arith_portable.test | 10 | 0.001070s | 0.001036s | 0.001028s | byte equality observed |
| function_n_test.test | 10 | 0.001013s | 0.001034s | 0.001038s | byte equality observed |
| function_ref_cxx98.test | 10 | 0.000991s | 0.000942s | 0.001006s | byte equality observed |
| function_ref_portable.test | 10 | 0.000990s | 0.000953s | 0.001001s | byte equality observed |
| function_test.test | 10 | 0.001062s | 0.001050s | 0.001088s | byte equality observed |
| function_test_no_rtti.test | 10 | 0.001050s | 0.001102s | 0.001100s | byte equality observed |
| lambda_test.test | 10 | 0.001015s | 0.001027s | 0.001038s | byte equality observed |
| mem_fun_cxx98.test | 10 | 0.001020s | 0.001003s | 0.001070s | byte equality observed |
| mem_fun_portable.test | 10 | 0.001008s | 0.001028s | 0.001031s | byte equality observed |
| mixed_cxxstd_shared.test | 10 | 0.001092s | 0.001102s | 0.001161s | byte equality observed |
| mixed_cxxstd_shared_0x.test | 10 | 0.001079s | 0.001085s | 0.001127s | byte equality observed |
| mixed_cxxstd_shared_14.test | 10 | 0.001079s | 0.001113s | 0.001105s | byte equality observed |
| mixed_cxxstd_static.test | 10 | 0.001018s | 0.000991s | 0.001020s | byte equality observed |
| mixed_cxxstd_static_0x.test | 10 | 0.001005s | 0.001019s | 0.001045s | byte equality observed |
| mixed_cxxstd_static_14.test | 10 | 0.001028s | 0.000992s | 0.001067s | byte equality observed |
| nothrow_swap.test | 10 | 0.001034s | 0.001014s | 0.001054s | byte equality observed |
| quick.test | 10 | 0.001080s | 0.001035s | 0.001056s | byte equality observed |
| result_arg_n_types_test.test | 10 | 0.001032s | 0.000995s | 0.001038s | byte equality observed |
| result_arg_types_test.test | 10 | 0.001015s | 0.001027s | 0.001084s | byte equality observed |
| return_function_shared.test | 10 | 0.001117s | 0.001102s | 0.001152s | byte equality observed |
| return_function_shared_0x.test | 10 | 0.001106s | 0.001138s | 0.001109s | byte equality observed |
| return_function_shared_14.test | 10 | 0.001087s | 0.001105s | 0.001166s | byte equality observed |
| return_function_static.test | 10 | 0.001031s | 0.001026s | 0.001034s | byte equality observed |
| return_function_static_0x.test | 10 | 0.001034s | 0.001021s | 0.001016s | byte equality observed |
| return_function_static_14.test | 10 | 0.001013s | 0.000999s | 0.001006s | byte equality observed |
| rvalues_test.test | 10 | 0.001024s | 0.001044s | 0.001047s | byte equality observed |
| stateless_test.test | 10 | 0.001001s | 0.001004s | 0.001011s | byte equality observed |
| std_bind_cxx98.test | 10 | 0.000993s | 0.001002s | 0.000994s | byte equality observed |
| std_bind_portable.test | 10 | 0.001012s | 0.001007s | 0.000983s | byte equality observed |
| sum_avg_cxx98.test | 10 | 0.000980s | 0.001010s | 0.001020s | byte equality observed |
| sum_avg_portable.test | 10 | 0.001000s | 0.001005s | 0.000980s | byte equality observed |
| test_bad_function_call_shared.test | 10 | 0.001133s | 0.001166s | 0.001142s | byte equality observed |
| test_bad_function_call_static.test | 10 | 0.001052s | 0.001059s | 0.001058s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
