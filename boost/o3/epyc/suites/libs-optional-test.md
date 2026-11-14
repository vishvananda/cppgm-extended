# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.239989 | 1.472780 | 0.930640 | 0.001760 | 0.001822 | 0.001944 |
| optional_test_assign.test | RUN | 0.130031 | 0.319168 | 0.437390 | 0.001689 | 0.001791 | 0.001800 |
| optional_test_swap.test | RUN | 0.134764 | 0.397783 | 0.496681 | 0.001509 | 0.001543 | 0.001658 |
| optional_test_constexpr.test | COMPILE | 0.093024 | 0.218803 | 0.291523 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.134463 | 0.518642 | 0.437043 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.091787 | 0.213148 | 0.270761 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.147686 | 0.396651 | 0.504091 | 0.001531 | 0.001864 | 0.001828 |
| optional_test_constructors.test | RUN | 0.303529 | 1.441653 | 1.351502 | 0.001558 | 0.001538 | 0.001814 |
| optional_test_convert_from_T.test | RUN | 0.143419 | 0.334847 | 0.483790 | 0.001574 | 0.001787 | 0.001566 |
| optional_test_convert_assign.test | RUN | 0.131642 | 0.320418 | 0.418117 | 0.001573 | 0.001700 | 0.001379 |
| optional_test_empty_braces.test | RUN | 0.136356 | 0.334426 | 0.435308 | 0.001642 | 0.001817 | 0.001829 |
| optional_test_make_optional.test | RUN | 0.185932 | 0.497760 | 0.699505 | 0.001750 | 0.001870 | 0.001364 |
| optional_test_flat_map.test | RUN | 0.194104 | 0.590518 | 0.736434 | 0.001660 | 0.001563 | 0.001742 |
| optional_test_hash.test | RUN | 0.161657 | 0.465718 | 0.561065 | 0.001386 | 0.001988 | 0.001825 |
| optional_test_map.test | RUN | 0.198099 | 0.572628 | 0.810295 | 0.001629 | 0.001448 | 0.001582 |
| optional_test_tie.test | RUN | 0.143886 | 0.356139 | 0.451788 | 0.001273 | 0.001868 | 0.001738 |
| optional_test_ranges_find.test | RUN | 0.128199 | 0.331321 | 0.439584 | 0.001554 | 0.001554 | 0.001574 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.167531 | 0.913582 | 0.602845 | 0.001583 | 0.001648 | 0.001722 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.141138 | 0.397601 | 0.448285 | 0.001642 | 0.001592 | 0.001724 |
| optional_test_ref_assign_const_int.test | RUN | 0.139307 | 0.406892 | 0.485858 | 0.001773 | 0.001571 | 0.001559 |
| optional_test_ref_converting_ctor.test | RUN | 0.150688 | 0.567130 | 0.534699 | 0.001630 | 0.001958 | 0.001759 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.134458 | 0.370899 | 0.455131 | 0.001510 | 0.001504 | 0.001754 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.136685 | 0.347538 | 0.467350 | 0.001653 | 0.001597 | 0.001806 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.134775 | 0.346617 | 0.443465 | 0.001744 | 0.001960 | 0.001797 |
| optional_test_ref_portable_minimum.test | RUN | 0.577340 | 1.829617 | 1.496800 | 0.001865 | 0.001863 | 0.001413 |
| optional_test_ref_move.test | RUN | 0.139849 | 0.365376 | 0.449563 | 0.001787 | 0.001792 | 0.001684 |
| optional_test_ref_to_val.test | RUN | 0.165571 | 0.732653 | 0.670134 | 0.001722 | 0.001828 | 0.001352 |
| optional_test_inplace_factory.test | RUN | 0.170556 | 0.658759 | 0.594108 | 0.001774 | 0.001684 | 0.001620 |
| optional_test_io.test | RUN | 0.164186 | 0.727638 | 0.585092 | 0.001763 | 0.001734 | 0.001679 |
| optional_test_move.test | RUN | 0.157432 | 0.497883 | 0.489849 | 0.001564 | 0.001382 | 0.001768 |
| optional_test_noexcept_move.test | RUN | 0.091022 | 0.221828 | 0.261092 | 0.000728 | 0.000588 | 0.001700 |
| optional_test_cmp_none.test | RUN | 0.132235 | 0.343141 | 0.466714 | 0.001741 | 0.001878 | 0.001740 |
| optional_test_value_access.test | RUN | 0.158104 | 0.602064 | 0.587805 | 0.001886 | 0.001676 | 0.001679 |
| optional_test_emplace.test | RUN | 0.163636 | 0.587196 | 0.579630 | 0.001666 | 0.001852 | 0.001614 |
| optional_test_minimum_requirements.test | RUN | 0.149451 | 0.442868 | 0.519426 | 0.001596 | 0.001718 | 0.001809 |
| optional_test_msvc_bug_workaround.test | RUN | 0.132205 | 0.333351 | 0.447009 | 0.001726 | 0.001798 | 0.001516 |
| optional_test_member_T.test | RUN | 0.131402 | 0.316576 | 0.434968 | 0.001571 | 0.001609 | 0.001715 |
| optional_test_tc_base.test | RUN | 0.133219 | 0.325757 | 0.435470 | 0.001685 | 0.001886 | 0.001700 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.100166 | 0.221003 | 0.304008 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.096445 | 0.227870 | 0.276410 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.088661 | 0.202227 | 0.258865 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.123376 | 0.279997 | 0.396143 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.086255 | 0.205291 | 0.267222 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.087080 | 0.209094 | 0.259533 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.087849 | 0.204282 | 0.266618 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.085666 | 0.209029 | 0.268077 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.086666 | 0.200949 | 0.261583 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.091110 | 0.192595 | 0.254512 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.086118 | 0.196294 | 0.258939 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.143076 | 0.325918 | 0.452307 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.141476 | 0.324059 | 0.449236 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.084753 | 0.198598 | 0.249922 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.087192 | 0.205192 | 0.260383 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.087250 | 0.224268 | 0.248828 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.087439 | 0.216823 | 0.266511 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.085497 | 0.211900 | 0.263313 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.085293 | 0.204977 | 0.250968 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.087628 | 0.201667 | 0.257868 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.084297 | 0.194448 | 0.253071 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.085856 | 0.197366 | 0.267615 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.085910 | 0.206005 | 0.265691 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.118838 | 0.265829 | 0.379639 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.093330 | 0.220819 | 0.320903 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.088059 | 0.210486 | 0.275758 | — | — | — |
| optional_test_static_properties.test | RUN | 0.213714 | 0.454218 | 0.672185 | 0.001660 | 0.001892 | 0.001827 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.092370 | 0.225267 | 0.293174 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.094189 | 0.224483 | 0.295398 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.089799 | 0.213419 | 0.276284 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.106880 | 0.264567 | 0.373210 | 0.001600 | 0.001798 | 0.001632 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.107443 | 0.257490 | 0.359058 | 0.001317 | 0.001522 | 0.001513 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.106303 | 0.252434 | 0.373797 | 0.001534 | 0.001527 | 0.001444 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.108279 | 0.278389 | 0.359941 | 0.001762 | 0.001397 | 0.001712 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.088292 | 0.207741 | 0.237548 | 0.001265 | 0.001388 | 0.001201 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.050291 | 0.197360 | 0.229584 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.984×, 1.003× | 1.009× |
| clang | 0.971×† | 0.938×, 1.004× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001095s | 0.001085s | 0.001075s | byte equality observed |
| optional_test_assign.test | 10 | 0.001024s | 0.000994s | 0.000992s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001027s | 0.001005s | 0.001009s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001035s | 0.001010s | 0.001004s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001025s | 0.001015s | 0.001002s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001044s | 0.000995s | 0.001011s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001054s | 0.000996s | 0.000988s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001039s | 0.001059s | 0.001016s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001060s | 0.001030s | 0.001012s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001009s | 0.000999s | 0.001010s | byte equality observed |
| optional_test_hash.test | 10 | 0.001022s | 0.001050s | 0.001004s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001048s | 0.001008s | 0.001042s | byte equality observed |
| optional_test_io.test | 10 | 0.001020s | 0.001021s | 0.001037s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001023s | 0.001039s | 0.001028s | byte equality observed |
| optional_test_map.test | 10 | 0.001012s | 0.001017s | 0.000998s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001044s | 0.000996s | 0.000996s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001039s | 0.001042s | 0.001023s | byte equality observed |
| optional_test_move.test | 10 | 0.000993s | 0.001032s | 0.001008s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001014s | 0.001025s | 0.001018s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000473s | 0.000466s | 0.001006s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001002s | 0.001022s | 0.000988s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001010s | 0.001056s | 0.001033s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001012s | 0.000986s | 0.000989s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001027s | 0.001040s | 0.000987s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001015s | 0.001023s | 0.001032s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001042s | 0.001030s | 0.000994s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001048s | 0.001004s | 0.001024s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001028s | 0.001052s | 0.001053s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001008s | 0.001091s | 0.001022s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001064s | 0.001028s | 0.001051s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001030s | 0.001012s | 0.001008s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001057s | 0.001031s | 0.001014s | byte equality observed |
| optional_test_swap.test | 10 | 0.001036s | 0.001007s | 0.000995s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001012s | 0.001001s | 0.000989s | byte equality observed |
| optional_test_tie.test | 10 | 0.001010s | 0.001028s | 0.000999s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001082s | 0.001060s | 0.001045s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000995s | 0.001022s | 0.001008s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001027s | 0.001045s | 0.001029s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001024s | 0.001006s | 0.000991s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
