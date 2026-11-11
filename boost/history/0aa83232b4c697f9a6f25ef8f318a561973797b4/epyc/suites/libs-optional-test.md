# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.148517 | 0.575707 | 0.519352 | 0.001670 | 0.001872 | 0.001962 |
| optional_test_assign.test | RUN | 0.114415 | 0.223943 | 0.291939 | 0.001826 | 0.001846 | 0.001765 |
| optional_test_swap.test | RUN | 0.120598 | 0.259567 | 0.328558 | 0.001547 | 0.001580 | 0.001540 |
| optional_test_constexpr.test | COMPILE | 0.085947 | 0.165027 | 0.197699 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.125125 | 0.251265 | 0.317679 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.085200 | 0.169002 | 0.202898 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.129623 | 0.327926 | 0.372440 | 0.001624 | 0.001884 | 0.001668 |
| optional_test_constructors.test | RUN | 0.208124 | 0.933684 | 0.859462 | 0.001818 | 0.003880 | 0.001810 |
| optional_test_convert_from_T.test | RUN | 0.122167 | 0.251770 | 0.317399 | 0.001453 | 0.001458 | 0.001934 |
| optional_test_convert_assign.test | RUN | 0.125314 | 0.244050 | 0.331044 | 0.001387 | 0.001764 | 0.001820 |
| optional_test_empty_braces.test | RUN | 0.119411 | 0.255733 | 0.322596 | 0.001915 | 0.001756 | 0.001802 |
| optional_test_make_optional.test | RUN | 0.154279 | 0.409071 | 0.448630 | 0.001628 | 0.003723 | 0.001703 |
| optional_test_flat_map.test | RUN | 0.160166 | 0.522737 | 0.493148 | 0.001518 | 0.001420 | 0.001839 |
| optional_test_hash.test | RUN | 0.144034 | 0.381590 | 0.379869 | 0.001649 | 0.001552 | 0.001762 |
| optional_test_map.test | RUN | 0.162224 | 0.507431 | 0.503280 | 0.001338 | 0.001546 | 0.001636 |
| optional_test_tie.test | RUN | 0.130392 | 0.264429 | 0.343620 | 0.001452 | 0.001905 | 0.001496 |
| optional_test_ranges_find.test | RUN | 0.120176 | 0.224809 | 0.289762 | 0.001643 | 0.001274 | 0.001547 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.130945 | 0.348266 | 0.374852 | 0.001514 | 0.001801 | 0.001710 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.124958 | 0.251719 | 0.334067 | 0.001580 | 0.001459 | 0.001411 |
| optional_test_ref_assign_const_int.test | RUN | 0.120610 | 0.252008 | 0.330235 | 0.001502 | 0.001914 | 0.001548 |
| optional_test_ref_converting_ctor.test | RUN | 0.130534 | 0.318265 | 0.345926 | 0.001624 | 0.001713 | 0.001742 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.118424 | 0.261504 | 0.313243 | 0.001790 | 0.001653 | 0.001839 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.120270 | 0.244116 | 0.304464 | 0.001811 | 0.001444 | 0.001646 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.120573 | 0.243682 | 0.298261 | 0.001253 | 0.001836 | 0.001661 |
| optional_test_ref_portable_minimum.test | RUN | 0.176102 | 0.865646 | 0.744423 | 0.002160 | 0.001884 | 0.001673 |
| optional_test_ref_move.test | RUN | 0.124573 | 0.279897 | 0.331477 | 0.001740 | 0.001924 | 0.001809 |
| optional_test_ref_to_val.test | RUN | 0.125960 | 0.390236 | 0.419094 | 0.001813 | 0.001929 | 0.001827 |
| optional_test_inplace_factory.test | RUN | 0.133898 | 0.335824 | 0.368253 | 0.001565 | 0.001996 | 0.001512 |
| optional_test_io.test | RUN | 0.130031 | 0.343938 | 0.369813 | 0.001770 | 0.001880 | 0.001730 |
| optional_test_move.test | RUN | 0.128168 | 0.332352 | 0.359655 | 0.001443 | 0.001892 | 0.001421 |
| optional_test_noexcept_move.test | RUN | 0.086724 | 0.178494 | 0.212383 | 0.000648 | 0.000638 | 0.001260 |
| optional_test_cmp_none.test | RUN | 0.118387 | 0.245897 | 0.297280 | 0.001780 | 0.001766 | 0.001344 |
| optional_test_value_access.test | RUN | 0.134296 | 0.371558 | 0.390231 | 0.001661 | 0.001881 | 0.001662 |
| optional_test_emplace.test | RUN | 0.134846 | 0.388837 | 0.394949 | 0.001564 | 0.001571 | 0.001514 |
| optional_test_minimum_requirements.test | RUN | 0.130707 | 0.302862 | 0.346734 | 0.001798 | 0.001928 | 0.001365 |
| optional_test_msvc_bug_workaround.test | RUN | 0.121014 | 0.243897 | 0.313362 | 0.001476 | 0.001941 | 0.001309 |
| optional_test_member_T.test | RUN | 0.116569 | 0.235019 | 0.296251 | 0.001375 | 0.001660 | 0.001741 |
| optional_test_tc_base.test | RUN | 0.119391 | 0.242418 | 0.300468 | 0.001686 | 0.001697 | 0.001515 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.093522 | 0.180549 | 0.211506 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.088089 | 0.171879 | 0.200414 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.078942 | 0.148455 | 0.171479 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.108543 | 0.213584 | 0.274101 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.079156 | 0.143640 | 0.183797 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.082498 | 0.144512 | 0.188120 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.083797 | 0.157033 | 0.193304 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.081564 | 0.148893 | 0.184811 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.080042 | 0.144276 | 0.180710 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.083802 | 0.142318 | 0.185124 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.082381 | 0.154192 | 0.184267 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.139344 | 0.240153 | 0.316155 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.133363 | 0.233573 | 0.318325 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.081126 | 0.141110 | 0.185399 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.085964 | 0.147037 | 0.188378 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.085662 | 0.143960 | 0.179267 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.088227 | 0.140946 | 0.179609 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.086786 | 0.136177 | 0.176550 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.128056 | 0.133192 | 0.185263 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.088424 | 0.116982 | 0.191236 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.088173 | 0.139310 | 0.181177 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.089165 | 0.141539 | 0.184525 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.082384 | 0.144746 | 0.188247 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.105686 | 0.201935 | 0.274371 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.084145 | 0.166466 | 0.228731 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.081423 | 0.150609 | 0.185597 | — | — | — |
| optional_test_static_properties.test | RUN | 0.189383 | 0.348924 | 0.459886 | 0.001537 | 0.002000 | 0.001858 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.084458 | 0.172265 | 0.203170 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.085267 | 0.182772 | 0.214607 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.083783 | 0.162921 | 0.192846 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.094424 | 0.197919 | 0.247871 | 0.001549 | 0.001793 | 0.001883 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.094893 | 0.193473 | 0.267715 | 0.001514 | 0.001760 | 0.001605 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.095985 | 0.196809 | 0.266644 | 0.001402 | 0.001650 | 0.001582 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.096763 | 0.194417 | 0.250691 | 0.001784 | 0.001472 | 0.001859 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.085511 | 0.170833 | 0.207611 | 0.001250 | 0.001382 | 0.001301 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.064372 | 0.148280 | 0.179352 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
