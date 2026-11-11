# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.156023 | 0.583732 | 0.516711 | 0.001612 | 0.002012 | 0.001651 |
| optional_test_assign.test | RUN | 0.118365 | 0.235209 | 0.304268 | 0.001498 | 0.001778 | 0.001723 |
| optional_test_swap.test | RUN | 0.123192 | 0.267412 | 0.322081 | 0.001757 | 0.001558 | 0.001494 |
| optional_test_constexpr.test | COMPILE | 0.086769 | 0.170500 | 0.205132 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.127528 | 0.253657 | 0.301242 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.089285 | 0.170262 | 0.196328 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.137361 | 0.337444 | 0.361592 | 0.001822 | 0.001896 | 0.001717 |
| optional_test_constructors.test | RUN | 0.220102 | 0.940050 | 0.792932 | 0.001986 | 0.001986 | 0.001859 |
| optional_test_convert_from_T.test | RUN | 0.129050 | 0.255330 | 0.309728 | 0.001631 | 0.001807 | 0.001629 |
| optional_test_convert_assign.test | RUN | 0.129589 | 0.246270 | 0.304105 | 0.001637 | 0.001689 | 0.001590 |
| optional_test_empty_braces.test | RUN | 0.129726 | 0.259174 | 0.316041 | 0.001962 | 0.001694 | 0.001724 |
| optional_test_make_optional.test | RUN | 0.165153 | 0.439329 | 0.431532 | 0.001840 | 0.002330 | 0.001756 |
| optional_test_flat_map.test | RUN | 0.173794 | 0.503422 | 0.467899 | 0.001865 | 0.002122 | 0.001788 |
| optional_test_hash.test | RUN | 0.153544 | 0.382672 | 0.359238 | 0.001697 | 0.002019 | 0.001780 |
| optional_test_map.test | RUN | 0.170417 | 0.549426 | 0.506309 | 0.001636 | 0.001584 | 0.001533 |
| optional_test_tie.test | RUN | 0.138953 | 0.271046 | 0.336891 | 0.001644 | 0.001722 | 0.001530 |
| optional_test_ranges_find.test | RUN | 0.125357 | 0.226047 | 0.286549 | 0.001609 | 0.001715 | 0.001480 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.140269 | 0.352115 | 0.364012 | 0.001679 | 0.001748 | 0.001605 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.132650 | 0.255041 | 0.310602 | 0.001676 | 0.001760 | 0.001530 |
| optional_test_ref_assign_const_int.test | RUN | 0.131948 | 0.260118 | 0.311938 | 0.001688 | 0.001782 | 0.001632 |
| optional_test_ref_converting_ctor.test | RUN | 0.140438 | 0.322812 | 0.337551 | 0.001834 | 0.001936 | 0.001544 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.131763 | 0.264544 | 0.309659 | 0.001622 | 0.001744 | 0.001666 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.127134 | 0.245398 | 0.302673 | 0.001882 | 0.001737 | 0.001730 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.126241 | 0.239847 | 0.294114 | 0.001534 | 0.001953 | 0.001603 |
| optional_test_ref_portable_minimum.test | RUN | 0.187147 | 0.885878 | 0.733430 | 0.001979 | 0.001862 | 0.001801 |
| optional_test_ref_move.test | RUN | 0.133216 | 0.280876 | 0.307411 | 0.001812 | 0.001870 | 0.001786 |
| optional_test_ref_to_val.test | RUN | 0.146370 | 0.409639 | 0.426580 | 0.001684 | 0.001984 | 0.001333 |
| optional_test_inplace_factory.test | RUN | 0.152843 | 0.355464 | 0.383652 | 0.001618 | 0.001666 | 0.001263 |
| optional_test_io.test | RUN | 0.147554 | 0.342210 | 0.358854 | 0.001888 | 0.001370 | 0.001860 |
| optional_test_move.test | RUN | 0.145608 | 0.338430 | 0.359600 | 0.001935 | 0.002096 | 0.001488 |
| optional_test_noexcept_move.test | RUN | 0.096230 | 0.178586 | 0.208612 | 0.000839 | 0.000728 | 0.001317 |
| optional_test_cmp_none.test | RUN | 0.134535 | 0.244824 | 0.289178 | 0.001864 | 0.001867 | 0.001590 |
| optional_test_value_access.test | RUN | 0.143921 | 0.362264 | 0.372150 | 0.001730 | 0.001689 | 0.001294 |
| optional_test_emplace.test | RUN | 0.141539 | 0.395439 | 0.391604 | 0.001744 | 0.001916 | 0.001453 |
| optional_test_minimum_requirements.test | RUN | 0.138679 | 0.326388 | 0.332531 | 0.002008 | 0.001811 | 0.001490 |
| optional_test_msvc_bug_workaround.test | RUN | 0.135385 | 0.255299 | 0.304401 | 0.001816 | 0.001642 | 0.001646 |
| optional_test_member_T.test | RUN | 0.131702 | 0.242056 | 0.301294 | 0.001588 | 0.001767 | 0.001692 |
| optional_test_tc_base.test | RUN | 0.133427 | 0.248364 | 0.299133 | 0.001635 | 0.001840 | 0.001713 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.101535 | 0.184447 | 0.208344 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.094689 | 0.178190 | 0.197553 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.086322 | 0.149874 | 0.180557 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.117170 | 0.209122 | 0.262658 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.086012 | 0.151604 | 0.184016 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.087322 | 0.151481 | 0.185548 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.088414 | 0.154517 | 0.178685 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.086205 | 0.145844 | 0.183333 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.085193 | 0.146801 | 0.184665 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.087902 | 0.150177 | 0.180113 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.085883 | 0.151101 | 0.176068 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.141348 | 0.242639 | 0.317188 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.138682 | 0.244061 | 0.303840 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.082992 | 0.149611 | 0.179150 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.088556 | 0.154422 | 0.180868 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.085736 | 0.140720 | 0.174980 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.089767 | 0.141519 | 0.177732 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.087121 | 0.144484 | 0.176039 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.091156 | 0.142995 | 0.180994 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.089236 | 0.144649 | 0.181583 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.087045 | 0.143176 | 0.181318 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.086269 | 0.146823 | 0.183203 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.086103 | 0.147580 | 0.186053 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.117835 | 0.196099 | 0.272461 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.092382 | 0.169320 | 0.227992 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.088522 | 0.151758 | 0.195172 | — | — | — |
| optional_test_static_properties.test | RUN | 0.205449 | 0.358663 | 0.462641 | 0.001906 | 0.001904 | 0.001786 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.090017 | 0.179028 | 0.200916 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.092650 | 0.185356 | 0.214809 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.090289 | 0.173968 | 0.197833 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.106650 | 0.204081 | 0.260583 | 0.001806 | 0.001914 | 0.001639 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.105770 | 0.205610 | 0.255869 | 0.001688 | 0.001646 | 0.001399 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.106412 | 0.202759 | 0.247657 | 0.001265 | 0.001714 | 0.001327 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.088559 | 0.207985 | 0.247771 | 0.001517 | 0.001992 | 0.001635 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.088010 | 0.155117 | 0.178062 | 0.001367 | 0.001393 | 0.001120 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.059238 | 0.136623 | 0.163804 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
