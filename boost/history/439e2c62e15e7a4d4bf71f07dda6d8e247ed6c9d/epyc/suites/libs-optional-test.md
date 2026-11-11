# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.133918 | 0.540343 | 0.455315 | 0.001447 | 0.001832 | 0.001506 |
| optional_test_assign.test | RUN | 0.103635 | 0.213829 | 0.275822 | 0.001542 | 0.001694 | 0.001455 |
| optional_test_swap.test | RUN | 0.107018 | 0.242936 | 0.289248 | 0.001332 | 0.001515 | 0.001288 |
| optional_test_constexpr.test | COMPILE | 0.075726 | 0.153862 | 0.178078 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.107926 | 0.234937 | 0.276575 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.075068 | 0.156993 | 0.183902 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.113720 | 0.304328 | 0.335160 | 0.001529 | 0.001438 | 0.001327 |
| optional_test_constructors.test | RUN | 0.182228 | 0.872612 | 0.743775 | 0.001521 | 0.001616 | 0.001428 |
| optional_test_convert_from_T.test | RUN | 0.107887 | 0.233753 | 0.276616 | 0.001504 | 0.001466 | 0.001433 |
| optional_test_convert_assign.test | RUN | 0.109512 | 0.227129 | 0.294400 | 0.001265 | 0.001642 | 0.001524 |
| optional_test_empty_braces.test | RUN | 0.108906 | 0.228590 | 0.274686 | 0.001580 | 0.001576 | 0.001568 |
| optional_test_make_optional.test | RUN | 0.137552 | 0.400152 | 0.396165 | 0.001374 | 0.001476 | 0.001423 |
| optional_test_flat_map.test | RUN | 0.145541 | 0.449871 | 0.421715 | 0.001431 | 0.001729 | 0.001446 |
| optional_test_hash.test | RUN | 0.128860 | 0.312481 | 0.355449 | 0.001566 | 0.001542 | 0.001361 |
| optional_test_map.test | RUN | 0.147096 | 0.475576 | 0.456982 | 0.001553 | 0.001576 | 0.001290 |
| optional_test_tie.test | RUN | 0.116594 | 0.242008 | 0.271253 | 0.001374 | 0.001686 | 0.001342 |
| optional_test_ranges_find.test | RUN | 0.106422 | 0.208380 | 0.265354 | 0.001375 | 0.001584 | 0.001501 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.119253 | 0.326311 | 0.332223 | 0.001316 | 0.001578 | 0.001378 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.110038 | 0.234772 | 0.284343 | 0.001460 | 0.001700 | 0.001354 |
| optional_test_ref_assign_const_int.test | RUN | 0.110311 | 0.234902 | 0.285226 | 0.001340 | 0.001542 | 0.001247 |
| optional_test_ref_converting_ctor.test | RUN | 0.114137 | 0.296420 | 0.311713 | 0.001401 | 0.001593 | 0.001483 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.110256 | 0.246416 | 0.299489 | 0.001371 | 0.001382 | 0.001546 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.104857 | 0.229560 | 0.275676 | 0.001511 | 0.001566 | 0.001208 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.106525 | 0.218878 | 0.264739 | 0.001335 | 0.001672 | 0.001397 |
| optional_test_ref_portable_minimum.test | RUN | 0.159697 | 0.801672 | 0.680560 | 0.001575 | 0.001604 | 0.001305 |
| optional_test_ref_move.test | RUN | 0.111373 | 0.259802 | 0.294464 | 0.001571 | 0.001568 | 0.001254 |
| optional_test_ref_to_val.test | RUN | 0.121657 | 0.376086 | 0.367409 | 0.001597 | 0.001556 | 0.001207 |
| optional_test_inplace_factory.test | RUN | 0.130167 | 0.324530 | 0.331457 | 0.001497 | 0.001647 | 0.001539 |
| optional_test_io.test | RUN | 0.118195 | 0.310317 | 0.340895 | 0.001495 | 0.001738 | 0.001314 |
| optional_test_move.test | RUN | 0.117923 | 0.300687 | 0.333959 | 0.001405 | 0.001582 | 0.001476 |
| optional_test_noexcept_move.test | RUN | 0.080122 | 0.156395 | 0.179215 | 0.000518 | 0.000626 | 0.001367 |
| optional_test_cmp_none.test | RUN | 0.107219 | 0.220643 | 0.261352 | 0.001517 | 0.001672 | 0.001352 |
| optional_test_value_access.test | RUN | 0.117341 | 0.332331 | 0.345562 | 0.001573 | 0.001576 | 0.001457 |
| optional_test_emplace.test | RUN | 0.117603 | 0.344334 | 0.356337 | 0.001397 | 0.001508 | 0.001358 |
| optional_test_minimum_requirements.test | RUN | 0.112918 | 0.285428 | 0.306495 | 0.001448 | 0.001537 | 0.001351 |
| optional_test_msvc_bug_workaround.test | RUN | 0.107623 | 0.220968 | 0.282132 | 0.001490 | 0.001671 | 0.001022 |
| optional_test_member_T.test | RUN | 0.106234 | 0.212020 | 0.267359 | 0.001468 | 0.001512 | 0.001565 |
| optional_test_tc_base.test | RUN | 0.109582 | 0.216350 | 0.273769 | 0.001300 | 0.001461 | 0.001520 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.083249 | 0.159232 | 0.190475 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.079424 | 0.157683 | 0.178659 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.071964 | 0.135669 | 0.158063 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.098038 | 0.190954 | 0.246824 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.071587 | 0.133451 | 0.158983 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.072947 | 0.139459 | 0.174224 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.074331 | 0.140954 | 0.175693 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.072928 | 0.134175 | 0.162959 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.071688 | 0.133074 | 0.162113 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.071574 | 0.134982 | 0.164414 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.072759 | 0.139208 | 0.161223 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.120422 | 0.224067 | 0.289276 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.119654 | 0.223749 | 0.281858 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.070527 | 0.132344 | 0.165767 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.073872 | 0.140646 | 0.167939 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.071758 | 0.132292 | 0.161301 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.071061 | 0.135311 | 0.165703 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.070411 | 0.132969 | 0.165490 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.073499 | 0.133220 | 0.163720 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.073026 | 0.127442 | 0.169909 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.070039 | 0.128821 | 0.161441 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.070028 | 0.130823 | 0.167540 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.072727 | 0.128078 | 0.162307 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.097658 | 0.173423 | 0.244613 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.076753 | 0.150139 | 0.208406 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.072715 | 0.130549 | 0.168682 | — | — | — |
| optional_test_static_properties.test | RUN | 0.166946 | 0.306663 | 0.412445 | 0.001472 | 0.001412 | 0.001448 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.075938 | 0.150485 | 0.182111 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.078013 | 0.161647 | 0.185961 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.074947 | 0.143635 | 0.178054 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.088218 | 0.171890 | 0.229499 | 0.001359 | 0.001419 | 0.001352 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.085023 | 0.168662 | 0.230182 | 0.001314 | 0.001414 | 0.001267 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.084977 | 0.168420 | 0.233129 | 0.001238 | 0.001210 | 0.001308 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.088761 | 0.173141 | 0.224995 | 0.001398 | 0.001452 | 0.001504 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.063039 | 0.117629 | 0.159815 | 0.001070 | 0.001081 | 0.001052 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.053902 | 0.111912 | 0.144070 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
