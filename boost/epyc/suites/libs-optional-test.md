# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.190199 | 0.707356 | 0.742660 | 0.001890 | 0.002078 | 0.002054 |
| optional_test_assign.test | RUN | 0.151032 | 0.285732 | 0.429727 | 0.001552 | 0.001881 | 0.001846 |
| optional_test_swap.test | RUN | 0.153410 | 0.330183 | 0.482076 | 0.001704 | 0.001771 | 0.002019 |
| optional_test_constexpr.test | COMPILE | 0.113761 | 0.211239 | 0.300689 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.159614 | 0.311687 | 0.422669 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.107737 | 0.208697 | 0.290890 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.164955 | 0.395206 | 0.537091 | 0.001563 | 0.001778 | 0.001812 |
| optional_test_constructors.test | RUN | 0.277946 | 1.143481 | 1.195612 | 0.001838 | 0.001617 | 0.001920 |
| optional_test_convert_from_T.test | RUN | 0.158371 | 0.310035 | 0.475346 | 0.001593 | 0.001699 | 0.001638 |
| optional_test_convert_assign.test | RUN | 0.161383 | 0.303229 | 0.431457 | 0.002050 | 0.001788 | 0.001688 |
| optional_test_empty_braces.test | RUN | 0.157137 | 0.306807 | 0.460281 | 0.001864 | 0.001867 | 0.001723 |
| optional_test_make_optional.test | RUN | 0.198018 | 0.531846 | 0.654597 | 0.001789 | 0.001807 | 0.001765 |
| optional_test_flat_map.test | RUN | 0.202588 | 0.615192 | 0.705316 | 0.001444 | 0.001703 | 0.001532 |
| optional_test_hash.test | RUN | 0.182945 | 0.409982 | 0.560869 | 0.001865 | 0.001572 | 0.001820 |
| optional_test_map.test | RUN | 0.201355 | 0.594768 | 0.698234 | 0.002080 | 0.001730 | 0.001458 |
| optional_test_tie.test | RUN | 0.165664 | 0.317284 | 0.478163 | 0.001713 | 0.001835 | 0.001515 |
| optional_test_ranges_find.test | RUN | 0.149917 | 0.274553 | 0.437559 | 0.001736 | 0.001733 | 0.001303 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.167759 | 0.426539 | 0.529935 | 0.001658 | 0.001645 | 0.001554 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.153306 | 0.304258 | 0.434373 | 0.002050 | 0.002022 | 0.001376 |
| optional_test_ref_assign_const_int.test | RUN | 0.163316 | 0.319316 | 0.461880 | 0.001733 | 0.001882 | 0.001492 |
| optional_test_ref_converting_ctor.test | RUN | 0.159067 | 0.411099 | 0.520610 | 0.001805 | 0.001688 | 0.001772 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.165556 | 0.310844 | 0.432312 | 0.001660 | 0.001709 | 0.001716 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.158064 | 0.301699 | 0.446557 | 0.002004 | 0.001836 | 0.001609 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.157532 | 0.286296 | 0.442698 | 0.001891 | 0.001908 | 0.001423 |
| optional_test_ref_portable_minimum.test | RUN | 0.234062 | 1.072287 | 1.068575 | 0.001740 | 0.001872 | 0.001718 |
| optional_test_ref_move.test | RUN | 0.155734 | 0.332850 | 0.474073 | 0.001964 | 0.001927 | 0.001698 |
| optional_test_ref_to_val.test | RUN | 0.178053 | 0.495292 | 0.599515 | 0.001882 | 0.001639 | 0.001518 |
| optional_test_inplace_factory.test | RUN | 0.194635 | 0.412405 | 0.547592 | 0.001494 | 0.001723 | 0.001506 |
| optional_test_io.test | RUN | 0.171088 | 0.403565 | 0.511286 | 0.001834 | 0.001908 | 0.001800 |
| optional_test_move.test | RUN | 0.167014 | 0.399765 | 0.501555 | 0.001696 | 0.001927 | 0.001597 |
| optional_test_noexcept_move.test | RUN | 0.116179 | 0.207458 | 0.297842 | 0.000825 | 0.000658 | 0.001489 |
| optional_test_cmp_none.test | RUN | 0.162147 | 0.295164 | 0.422948 | 0.001880 | 0.001914 | 0.001628 |
| optional_test_value_access.test | RUN | 0.171392 | 0.434420 | 0.518785 | 0.001819 | 0.001766 | 0.001628 |
| optional_test_emplace.test | RUN | 0.170913 | 0.459386 | 0.555545 | 0.001733 | 0.001373 | 0.001624 |
| optional_test_minimum_requirements.test | RUN | 0.167625 | 0.364554 | 0.491690 | 0.001906 | 0.001884 | 0.001624 |
| optional_test_msvc_bug_workaround.test | RUN | 0.157299 | 0.295754 | 0.428078 | 0.001826 | 0.001654 | 0.001554 |
| optional_test_member_T.test | RUN | 0.151453 | 0.287970 | 0.428081 | 0.001867 | 0.001581 | 0.001480 |
| optional_test_tc_base.test | RUN | 0.155582 | 0.294719 | 0.440569 | 0.001866 | 0.001431 | 0.001624 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.117902 | 0.214209 | 0.301685 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.113166 | 0.213717 | 0.290122 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.103690 | 0.178128 | 0.257559 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.140946 | 0.253480 | 0.401818 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.106340 | 0.183958 | 0.256605 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.103792 | 0.184903 | 0.265737 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.110525 | 0.181110 | 0.268682 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.109391 | 0.177251 | 0.259350 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.106977 | 0.175571 | 0.251566 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.105888 | 0.181527 | 0.261456 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.109122 | 0.175346 | 0.265235 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.180246 | 0.301190 | 0.440497 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.178462 | 0.302954 | 0.448099 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.104623 | 0.177677 | 0.264439 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.109277 | 0.130775 | 0.274797 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.108653 | 0.175264 | 0.261016 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.109157 | 0.173689 | 0.274669 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.108907 | 0.175267 | 0.273988 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.108626 | 0.180673 | 0.260394 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.108044 | 0.177759 | 0.264468 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.105979 | 0.175485 | 0.263215 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.106437 | 0.181399 | 0.262558 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.107740 | 0.183296 | 0.263021 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.146660 | 0.240016 | 0.392772 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.115290 | 0.209043 | 0.329306 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.109847 | 0.186793 | 0.278082 | — | — | — |
| optional_test_static_properties.test | RUN | 0.256339 | 0.435219 | 0.681556 | 0.001838 | 0.001888 | 0.001854 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.111730 | 0.211947 | 0.292805 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.114181 | 0.222283 | 0.293023 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.115835 | 0.202649 | 0.274168 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.127447 | 0.234489 | 0.367311 | 0.001780 | 0.001361 | 0.001400 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.122827 | 0.235832 | 0.338042 | 0.001479 | 0.001742 | 0.001676 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.128061 | 0.235488 | 0.369558 | 0.001565 | 0.001355 | 0.001504 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.127313 | 0.235628 | 0.348496 | 0.001537 | 0.001951 | 0.001813 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.111973 | 0.167224 | 0.236982 | 0.001393 | 0.001265 | 0.001253 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.083969 | 0.158281 | 0.226568 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
