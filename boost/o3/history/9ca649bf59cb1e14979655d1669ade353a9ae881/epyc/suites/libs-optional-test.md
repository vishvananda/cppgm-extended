# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.272878 | 1.467090 | 0.925719 | 0.001920 | 0.001974 | 0.001904 |
| optional_test_assign.test | RUN | 0.145474 | 0.319945 | 0.435861 | 0.001663 | 0.001878 | 0.001832 |
| optional_test_swap.test | RUN | 0.159554 | 0.395025 | 0.488860 | 0.001603 | 0.001843 | 0.001622 |
| optional_test_constexpr.test | COMPILE | 0.104596 | 0.217198 | 0.288568 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.159081 | 0.532622 | 0.429756 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.104366 | 0.209098 | 0.269962 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.165835 | 0.409066 | 0.540223 | 0.001719 | 0.001756 | 0.001720 |
| optional_test_constructors.test | RUN | 0.379766 | 1.434379 | 1.285030 | 0.001801 | 0.001782 | 0.001830 |
| optional_test_convert_from_T.test | RUN | 0.158058 | 0.330824 | 0.460403 | 0.001686 | 0.001789 | 0.001698 |
| optional_test_convert_assign.test | RUN | 0.154336 | 0.326198 | 0.451609 | 0.001553 | 0.001686 | 0.001832 |
| optional_test_empty_braces.test | RUN | 0.152914 | 0.312394 | 0.419028 | 0.001758 | 0.001858 | 0.001671 |
| optional_test_make_optional.test | RUN | 0.210119 | 0.486553 | 0.717610 | 0.001751 | 0.001706 | 0.001724 |
| optional_test_flat_map.test | RUN | 0.225966 | 0.556345 | 0.783386 | 0.001614 | 0.001676 | 0.001816 |
| optional_test_hash.test | RUN | 0.181745 | 0.451105 | 0.543483 | 0.001523 | 0.001566 | 0.001838 |
| optional_test_map.test | RUN | 0.225829 | 0.556225 | 0.741111 | 0.001665 | 0.001470 | 0.001431 |
| optional_test_tie.test | RUN | 0.162198 | 0.372467 | 0.473252 | 0.001610 | 0.001750 | 0.001542 |
| optional_test_ranges_find.test | RUN | 0.141272 | 0.320549 | 0.418411 | 0.001587 | 0.001788 | 0.001617 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.233142 | 0.919021 | 0.588891 | 0.001693 | 0.001518 | 0.001791 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.157990 | 0.396234 | 0.467401 | 0.001591 | 0.001763 | 0.001376 |
| optional_test_ref_assign_const_int.test | RUN | 0.168990 | 0.401203 | 0.443092 | 0.001880 | 0.001729 | 0.001764 |
| optional_test_ref_converting_ctor.test | RUN | 0.216099 | 0.552758 | 0.513198 | 0.001741 | 0.001778 | 0.001790 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.167639 | 0.378691 | 0.445829 | 0.001748 | 0.001857 | 0.001768 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.149860 | 0.350131 | 0.442884 | 0.001710 | 0.001528 | 0.001623 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.146154 | 0.331408 | 0.447327 | 0.002172 | 0.001612 | 0.001197 |
| optional_test_ref_portable_minimum.test | RUN | 0.552346 | 1.840593 | 1.556887 | 0.001734 | 0.001634 | 0.001423 |
| optional_test_ref_move.test | RUN | 0.160495 | 0.355918 | 0.488754 | 0.001753 | 0.001780 | 0.001662 |
| optional_test_ref_to_val.test | RUN | 0.227788 | 0.726684 | 0.654647 | 0.001873 | 0.001652 | 0.001515 |
| optional_test_inplace_factory.test | RUN | 0.196296 | 0.664423 | 0.601845 | 0.001872 | 0.002040 | 0.001629 |
| optional_test_io.test | RUN | 0.188925 | 0.743452 | 0.581991 | 0.001661 | 0.001984 | 0.001650 |
| optional_test_move.test | RUN | 0.182967 | 0.520747 | 0.543662 | 0.001760 | 0.001790 | 0.001699 |
| optional_test_noexcept_move.test | RUN | 0.110796 | 0.225755 | 0.304583 | 0.000709 | 0.000690 | 0.001499 |
| optional_test_cmp_none.test | RUN | 0.147680 | 0.358639 | 0.455030 | 0.001622 | 0.001867 | 0.001679 |
| optional_test_value_access.test | RUN | 0.178296 | 0.581213 | 0.554418 | 0.001851 | 0.001598 | 0.001773 |
| optional_test_emplace.test | RUN | 0.176376 | 0.582134 | 0.578839 | 0.001412 | 0.001804 | 0.001703 |
| optional_test_minimum_requirements.test | RUN | 0.168246 | 0.442468 | 0.504312 | 0.001785 | 0.001823 | 0.001761 |
| optional_test_msvc_bug_workaround.test | RUN | 0.153891 | 0.319912 | 0.399585 | 0.001656 | 0.001680 | 0.001800 |
| optional_test_member_T.test | RUN | 0.146580 | 0.309382 | 0.430467 | 0.001634 | 0.001784 | 0.001675 |
| optional_test_tc_base.test | RUN | 0.148158 | 0.316564 | 0.436902 | 0.001822 | 0.001537 | 0.001812 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.112442 | 0.224449 | 0.267899 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.108494 | 0.222732 | 0.263819 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.099041 | 0.197408 | 0.266268 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.136333 | 0.274626 | 0.405901 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.100499 | 0.172840 | 0.267049 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.100387 | 0.205394 | 0.271838 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.101497 | 0.198886 | 0.261837 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.101445 | 0.200674 | 0.266896 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.100172 | 0.190598 | 0.263401 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.099811 | 0.193287 | 0.249659 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.098549 | 0.212245 | 0.261426 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.169813 | 0.335205 | 0.442598 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.171043 | 0.338870 | 0.430671 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.098879 | 0.218740 | 0.261930 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.101228 | 0.208457 | 0.266705 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.100405 | 0.199215 | 0.252291 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.102418 | 0.199597 | 0.253561 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.102405 | 0.195542 | 0.270459 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.103380 | 0.201415 | 0.266468 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.102606 | 0.194330 | 0.251204 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.101117 | 0.193549 | 0.251973 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.100408 | 0.197535 | 0.261265 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.098735 | 0.204447 | 0.261572 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.135278 | 0.264597 | 0.376897 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.108858 | 0.225101 | 0.315095 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.100110 | 0.194300 | 0.268968 | — | — | — |
| optional_test_static_properties.test | RUN | 0.241928 | 0.457418 | 0.672704 | 0.001670 | 0.001875 | 0.001722 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.107903 | 0.219269 | 0.278575 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.110247 | 0.232307 | 0.289550 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.105339 | 0.214533 | 0.277067 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.120944 | 0.266654 | 0.373711 | 0.001600 | 0.001804 | 0.001538 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.119274 | 0.266052 | 0.380504 | 0.001512 | 0.001757 | 0.001502 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.123341 | 0.270783 | 0.343152 | 0.001523 | 0.001593 | 0.001551 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.125377 | 0.268935 | 0.359482 | 0.001705 | 0.001954 | 0.001760 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.090411 | 0.204534 | 0.233241 | 0.001313 | 0.001326 | 0.001224 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.061688 | 0.163796 | 0.252104 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.054×† | 1.069×, 1.038× | 1.060× |
| clang | 1.023×† | 1.006×, 1.040× | 1.068× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001140s | 0.001083s | 0.001122s | byte equality observed |
| optional_test_assign.test | 10 | 0.001033s | 0.001086s | 0.001070s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001019s | 0.001062s | 0.001050s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001040s | 0.001069s | 0.001048s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001007s | 0.001060s | 0.001043s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001007s | 0.001036s | 0.001035s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001051s | 0.001021s | 0.001064s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001066s | 0.001069s | 0.001081s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001037s | 0.001031s | 0.001054s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001056s | 0.001037s | 0.001034s | byte equality observed |
| optional_test_hash.test | 10 | 0.001050s | 0.001051s | 0.001039s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001049s | 0.001073s | 0.001083s | byte equality observed |
| optional_test_io.test | 10 | 0.001060s | 0.001083s | 0.001073s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001065s | 0.001036s | 0.001029s | byte equality observed |
| optional_test_map.test | 10 | 0.001048s | 0.001082s | 0.001051s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001007s | 0.001141s | 0.001041s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001054s | 0.001110s | 0.001101s | byte equality observed |
| optional_test_move.test | 10 | 0.001058s | 0.001114s | 0.001036s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001048s | 0.001132s | 0.001051s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000486s | 0.000508s | 0.001032s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001056s | 0.001085s | 0.001025s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001047s | 0.001129s | 0.001060s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001053s | 0.001112s | 0.001021s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001060s | 0.001088s | 0.001033s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001041s | 0.001086s | 0.001078s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001064s | 0.001168s | 0.001053s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001052s | 0.001080s | 0.001048s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001039s | 0.001114s | 0.001075s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001057s | 0.001084s | 0.001073s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001099s | 0.001144s | 0.001091s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001051s | 0.001116s | 0.001024s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001032s | 0.001113s | 0.001030s | byte equality observed |
| optional_test_swap.test | 10 | 0.001045s | 0.001109s | 0.001036s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001042s | 0.001065s | 0.001072s | byte equality observed |
| optional_test_tie.test | 10 | 0.001026s | 0.001126s | 0.001098s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001090s | 0.001173s | 0.001057s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001008s | 0.001100s | 0.001012s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001037s | 0.001112s | 0.001033s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001009s | 0.001138s | 0.001037s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
