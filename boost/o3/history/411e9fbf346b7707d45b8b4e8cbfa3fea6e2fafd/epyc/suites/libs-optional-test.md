# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.202298 | 1.098301 | 0.685927 | 0.001660 | 0.001747 | 0.001608 |
| optional_test_assign.test | RUN | 0.109421 | 0.234489 | 0.314399 | 0.001226 | 0.001636 | 0.001573 |
| optional_test_swap.test | RUN | 0.117628 | 0.292934 | 0.332405 | 0.001651 | 0.001500 | 0.001516 |
| optional_test_constexpr.test | COMPILE | 0.079438 | 0.159171 | 0.191636 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.119583 | 0.384958 | 0.324574 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.078816 | 0.163859 | 0.208238 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.124486 | 0.304046 | 0.411446 | 0.001612 | 0.001582 | 0.001752 |
| optional_test_constructors.test | RUN | 0.251162 | 1.060917 | 1.036522 | 0.001617 | 0.001612 | 0.001732 |
| optional_test_convert_from_T.test | RUN | 0.118031 | 0.252375 | 0.350142 | 0.001784 | 0.001627 | 0.001734 |
| optional_test_convert_assign.test | RUN | 0.116234 | 0.249624 | 0.341120 | 0.001417 | 0.001456 | 0.001729 |
| optional_test_empty_braces.test | RUN | 0.113844 | 0.232849 | 0.344379 | 0.001595 | 0.001537 | 0.001581 |
| optional_test_make_optional.test | RUN | 0.149620 | 0.364690 | 0.517935 | 0.001817 | 0.001717 | 0.001586 |
| optional_test_flat_map.test | RUN | 0.160901 | 0.432164 | 0.596475 | 0.001459 | 0.001595 | 0.001656 |
| optional_test_hash.test | RUN | 0.136589 | 0.347422 | 0.440242 | 0.001489 | 0.001402 | 0.001762 |
| optional_test_map.test | RUN | 0.165470 | 0.425568 | 0.622540 | 0.001732 | 0.001621 | 0.001746 |
| optional_test_tie.test | RUN | 0.123001 | 0.276611 | 0.328526 | 0.001688 | 0.001488 | 0.001690 |
| optional_test_ranges_find.test | RUN | 0.108883 | 0.236800 | 0.317963 | 0.001451 | 0.001270 | 0.001551 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.132690 | 0.703289 | 0.443492 | 0.001589 | 0.001459 | 0.001680 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.115735 | 0.297742 | 0.332186 | 0.001710 | 0.001442 | 0.001651 |
| optional_test_ref_assign_const_int.test | RUN | 0.121300 | 0.308349 | 0.353202 | 0.001659 | 0.001343 | 0.001488 |
| optional_test_ref_converting_ctor.test | RUN | 0.129727 | 0.418460 | 0.386315 | 0.001311 | 0.001692 | 0.001734 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.119409 | 0.283488 | 0.334265 | 0.001475 | 0.001718 | 0.001800 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.115933 | 0.262858 | 0.331347 | 0.001796 | 0.001470 | 0.001857 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.107818 | 0.258085 | 0.321282 | 0.001746 | 0.001290 | 0.001748 |
| optional_test_ref_portable_minimum.test | RUN | 0.447008 | 1.356520 | 1.160345 | 0.001793 | 0.001472 | 0.001823 |
| optional_test_ref_move.test | RUN | 0.123592 | 0.272930 | 0.349893 | 0.001399 | 0.001608 | 0.001746 |
| optional_test_ref_to_val.test | RUN | 0.145330 | 0.533082 | 0.468829 | 0.001528 | 0.001379 | 0.004009 |
| optional_test_inplace_factory.test | RUN | 0.142507 | 0.490127 | 0.421936 | 0.001610 | 0.001628 | 0.001451 |
| optional_test_io.test | RUN | 0.141410 | 0.567520 | 0.499922 | 0.001380 | 0.001630 | 0.001710 |
| optional_test_move.test | RUN | 0.127333 | 0.365562 | 0.370720 | 0.001562 | 0.001284 | 0.001742 |
| optional_test_noexcept_move.test | RUN | 0.080732 | 0.163414 | 0.218527 | 0.000529 | 0.000584 | 0.001428 |
| optional_test_cmp_none.test | RUN | 0.113705 | 0.257454 | 0.326002 | 0.001884 | 0.001645 | 0.001750 |
| optional_test_value_access.test | RUN | 0.136004 | 0.436543 | 0.421302 | 0.001805 | 0.001530 | 0.001612 |
| optional_test_emplace.test | RUN | 0.135526 | 0.433128 | 0.433768 | 0.001766 | 0.001405 | 0.001438 |
| optional_test_minimum_requirements.test | RUN | 0.130066 | 0.335130 | 0.401836 | 0.001601 | 0.001511 | 0.001732 |
| optional_test_msvc_bug_workaround.test | RUN | 0.111666 | 0.233056 | 0.319545 | 0.001489 | 0.001563 | 0.001744 |
| optional_test_member_T.test | RUN | 0.116627 | 0.235310 | 0.322340 | 0.001824 | 0.001370 | 0.001749 |
| optional_test_tc_base.test | RUN | 0.115436 | 0.243344 | 0.319962 | 0.001529 | 0.001474 | 0.001398 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.087231 | 0.163567 | 0.209868 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.085553 | 0.165303 | 0.201484 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.074708 | 0.152340 | 0.186289 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.095662 | 0.204447 | 0.295155 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.076719 | 0.148757 | 0.188643 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.074403 | 0.145868 | 0.183051 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.075546 | 0.152917 | 0.195059 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.074531 | 0.147094 | 0.179745 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.071871 | 0.149181 | 0.182360 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.073474 | 0.145507 | 0.191450 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.073955 | 0.152439 | 0.189189 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.116783 | 0.234168 | 0.327457 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.115580 | 0.231143 | 0.319977 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.070154 | 0.145699 | 0.185844 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.073251 | 0.151062 | 0.192338 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.072407 | 0.145429 | 0.177606 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.071068 | 0.149374 | 0.188710 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.074846 | 0.145196 | 0.185919 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.074023 | 0.144634 | 0.184931 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.070263 | 0.139718 | 0.188746 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.073713 | 0.140157 | 0.183862 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.076553 | 0.137969 | 0.183867 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.072683 | 0.142294 | 0.188013 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.100695 | 0.193545 | 0.273467 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.082925 | 0.163135 | 0.231121 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.075331 | 0.152409 | 0.190120 | — | — | — |
| optional_test_static_properties.test | RUN | 0.172479 | 0.337387 | 0.468135 | 0.001518 | 0.001500 | 0.001785 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.074814 | 0.162550 | 0.200810 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.078173 | 0.167542 | 0.204950 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.075533 | 0.157259 | 0.190257 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.088832 | 0.208365 | 0.255502 | 0.001733 | 0.001541 | 0.001652 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.085928 | 0.216826 | 0.263959 | 0.001438 | 0.001131 | 0.001348 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.088392 | 0.206913 | 0.259753 | 0.001383 | 0.001444 | 0.001262 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.091296 | 0.221545 | 0.261490 | 0.001790 | 0.001254 | 0.001789 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.065536 | 0.149429 | 0.187279 | 0.001192 | 0.001128 | 0.001204 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.051460 | 0.134705 | 0.171439 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996×† | 0.965×, 1.026× | 1.039× |
| clang | 1.020×† | 1.056×, 0.983× | 0.974× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.000967s | 0.000965s | 0.000980s | byte equality observed |
| optional_test_assign.test | 10 | 0.000896s | 0.000900s | 0.000918s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000913s | 0.000889s | 0.000937s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000919s | 0.000880s | 0.000936s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000910s | 0.000901s | 0.000945s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000899s | 0.000881s | 0.000899s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000904s | 0.000902s | 0.000911s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000927s | 0.000914s | 0.000946s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000886s | 0.000896s | 0.000903s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000891s | 0.000891s | 0.000914s | byte equality observed |
| optional_test_hash.test | 10 | 0.000894s | 0.000889s | 0.000916s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000897s | 0.000919s | 0.000921s | byte equality observed |
| optional_test_io.test | 10 | 0.000909s | 0.000918s | 0.000941s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000896s | 0.000907s | 0.000902s | byte equality observed |
| optional_test_map.test | 10 | 0.000906s | 0.000896s | 0.000918s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000884s | 0.000892s | 0.000901s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000939s | 0.000950s | 0.000944s | byte equality observed |
| optional_test_move.test | 10 | 0.000893s | 0.000915s | 0.000878s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000906s | 0.000920s | 0.000889s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000415s | 0.000412s | 0.000878s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000879s | 0.000911s | 0.000903s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000904s | 0.000894s | 0.000886s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000895s | 0.000903s | 0.000900s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000903s | 0.000903s | 0.000886s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000895s | 0.000904s | 0.000892s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000902s | 0.000895s | 0.000890s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000911s | 0.000892s | 0.000897s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000901s | 0.000937s | 0.000918s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000917s | 0.000886s | 0.000884s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.000973s | 0.000915s | 0.000914s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000907s | 0.000878s | 0.000882s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000900s | 0.000886s | 0.000886s | byte equality observed |
| optional_test_swap.test | 10 | 0.000916s | 0.000884s | 0.000884s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000903s | 0.000884s | 0.000891s | byte equality observed |
| optional_test_tie.test | 10 | 0.000944s | 0.000894s | 0.000893s | byte equality observed |
| optional_test_value_access.test | 10 | 0.000944s | 0.000958s | 0.000945s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000867s | 0.000876s | 0.000895s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000915s | 0.000888s | 0.000880s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000893s | 0.000869s | 0.000894s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
