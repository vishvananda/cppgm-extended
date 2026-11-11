# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.355906 | 1.965657 | 1.132612 | 0.001810 | 0.001798 | 0.001775 |
| optional_test_assign.test | RUN | 0.182990 | 0.411413 | 0.513978 | 0.001773 | 0.001730 | 0.001596 |
| optional_test_swap.test | RUN | 0.202307 | 0.510945 | 0.570009 | 0.001496 | 0.001668 | 0.001598 |
| optional_test_constexpr.test | COMPILE | 0.133549 | 0.277453 | 0.330345 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.199881 | 0.687138 | 0.537130 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.125997 | 0.273089 | 0.320628 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.213988 | 0.527605 | 0.629795 | 0.001679 | 0.001791 | 0.001743 |
| optional_test_constructors.test | RUN | 0.482304 | 2.002136 | 1.634698 | 0.001625 | 0.001578 | 0.001659 |
| optional_test_convert_from_T.test | RUN | 0.190448 | 0.437595 | 0.549071 | 0.001554 | 0.001730 | 0.001762 |
| optional_test_convert_assign.test | RUN | 0.181381 | 0.430729 | 0.524313 | 0.001490 | 0.001499 | 0.001681 |
| optional_test_empty_braces.test | RUN | 0.181551 | 0.410300 | 0.562867 | 0.001700 | 0.001658 | 0.001695 |
| optional_test_make_optional.test | RUN | 0.248996 | 0.638771 | 0.842146 | 0.001683 | 0.001557 | 0.001655 |
| optional_test_flat_map.test | RUN | 0.274102 | 0.744071 | 0.920274 | 0.001563 | 0.001721 | 0.001574 |
| optional_test_hash.test | RUN | 0.224167 | 0.597964 | 0.690401 | 0.001536 | 0.001544 | 0.001582 |
| optional_test_map.test | RUN | 0.278517 | 0.705164 | 0.958488 | 0.001720 | 0.001601 | 0.001442 |
| optional_test_tie.test | RUN | 0.196316 | 0.469588 | 0.566156 | 0.001506 | 0.001429 | 0.001686 |
| optional_test_ranges_find.test | RUN | 0.183440 | 0.395315 | 0.514458 | 0.001528 | 0.001556 | 0.001390 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.294797 | 1.189106 | 0.743560 | 0.001768 | 0.001513 | 0.001711 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.192805 | 0.500414 | 0.564940 | 0.001656 | 0.001558 | 0.001663 |
| optional_test_ref_assign_const_int.test | RUN | 0.201318 | 0.520155 | 0.570447 | 0.001720 | 0.001616 | 0.001645 |
| optional_test_ref_converting_ctor.test | RUN | 0.276306 | 0.725238 | 0.630465 | 0.001782 | 0.001767 | 0.001658 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.201422 | 0.478454 | 0.544901 | 0.001740 | 0.001645 | 0.001665 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.188565 | 0.442740 | 0.516880 | 0.001612 | 0.001581 | 0.001623 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.183578 | 0.426633 | 0.518439 | 0.001755 | 0.001433 | 0.001661 |
| optional_test_ref_portable_minimum.test | RUN | 0.708860 | 2.420705 | 1.962942 | 0.001866 | 0.001632 | 0.001671 |
| optional_test_ref_move.test | RUN | 0.195569 | 0.449991 | 0.561907 | 0.001784 | 0.001701 | 0.001738 |
| optional_test_ref_to_val.test | RUN | 0.284169 | 0.942329 | 0.780544 | 0.001839 | 0.001608 | 0.001557 |
| optional_test_inplace_factory.test | RUN | 0.235901 | 0.855544 | 0.702307 | 0.001577 | 0.001789 | 0.001597 |
| optional_test_io.test | RUN | 0.231367 | 0.937141 | 0.738083 | 0.001560 | 0.001547 | 0.001563 |
| optional_test_move.test | RUN | 0.221278 | 0.644120 | 0.652669 | 0.001603 | 0.001730 | 0.001682 |
| optional_test_noexcept_move.test | RUN | 0.133021 | 0.281392 | 0.320732 | 0.000763 | 0.000756 | 0.001463 |
| optional_test_cmp_none.test | RUN | 0.188032 | 0.445316 | 0.525516 | 0.001662 | 0.001742 | 0.001691 |
| optional_test_value_access.test | RUN | 0.216424 | 0.743065 | 0.699809 | 0.001738 | 0.001670 | 0.001568 |
| optional_test_emplace.test | RUN | 0.228467 | 0.735178 | 0.697029 | 0.001755 | 0.001573 | 0.001508 |
| optional_test_minimum_requirements.test | RUN | 0.204461 | 0.580504 | 0.632799 | 0.001474 | 0.001711 | 0.001632 |
| optional_test_msvc_bug_workaround.test | RUN | 0.178270 | 0.399642 | 0.518245 | 0.001772 | 0.001436 | 0.001684 |
| optional_test_member_T.test | RUN | 0.178400 | 0.400606 | 0.503210 | 0.001526 | 0.001482 | 0.001660 |
| optional_test_tc_base.test | RUN | 0.182659 | 0.402857 | 0.519225 | 0.001478 | 0.001696 | 0.001437 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.137165 | 0.276928 | 0.350485 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.132909 | 0.278348 | 0.324633 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.122354 | 0.252354 | 0.303346 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.165910 | 0.354099 | 0.474408 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.122942 | 0.257620 | 0.310299 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.122490 | 0.261996 | 0.305382 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.124357 | 0.260030 | 0.312965 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.123554 | 0.256507 | 0.308172 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.123589 | 0.250623 | 0.298896 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.124201 | 0.256890 | 0.306347 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.120970 | 0.250753 | 0.302976 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.206495 | 0.408029 | 0.525870 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.208287 | 0.407731 | 0.525961 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.123381 | 0.257394 | 0.302823 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.125530 | 0.262675 | 0.311139 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.122930 | 0.253142 | 0.296110 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.125838 | 0.254440 | 0.309887 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.124321 | 0.256645 | 0.308387 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.123805 | 0.259885 | 0.300269 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.124515 | 0.254042 | 0.302531 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.123302 | 0.253890 | 0.309302 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.118322 | 0.259296 | 0.310696 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.126419 | 0.259042 | 0.301053 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.166587 | 0.334802 | 0.449210 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.133122 | 0.283831 | 0.375363 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.125904 | 0.259427 | 0.318893 | — | — | — |
| optional_test_static_properties.test | RUN | 0.287175 | 0.586196 | 0.791098 | 0.001668 | 0.001652 | 0.001650 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.127919 | 0.277653 | 0.331444 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.132725 | 0.281371 | 0.341029 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.127109 | 0.266975 | 0.317741 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.149138 | 0.344619 | 0.419792 | 0.001802 | 0.001399 | 0.001630 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.147846 | 0.349177 | 0.439899 | 0.001414 | 0.001415 | 0.001548 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.146912 | 0.338678 | 0.439370 | 0.001407 | 0.001429 | 0.001405 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.148145 | 0.351839 | 0.400513 | 0.001282 | 0.001804 | 0.001667 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.114123 | 0.237016 | 0.287106 | 0.001144 | 0.001156 | 0.001110 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.086733 | 0.185383 | 0.282343 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 1.001×, 0.979× | 1.008× |
| clang | 1.015× | 1.015×, 1.016× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001085s | 0.001031s | 0.001049s | byte equality observed |
| optional_test_assign.test | 10 | 0.000981s | 0.000969s | 0.001000s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000994s | 0.001012s | 0.000994s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001017s | 0.001009s | 0.000977s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000992s | 0.001041s | 0.000989s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000965s | 0.000996s | 0.001003s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000989s | 0.001000s | 0.000990s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000992s | 0.001017s | 0.001024s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000994s | 0.000978s | 0.000987s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000984s | 0.001042s | 0.000991s | byte equality observed |
| optional_test_hash.test | 10 | 0.001027s | 0.001014s | 0.001008s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001006s | 0.000995s | 0.000973s | byte equality observed |
| optional_test_io.test | 10 | 0.001007s | 0.001038s | 0.001031s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000973s | 0.000982s | 0.000955s | byte equality observed |
| optional_test_map.test | 10 | 0.000997s | 0.000966s | 0.001005s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000991s | 0.001002s | 0.001011s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001018s | 0.000995s | 0.001024s | byte equality observed |
| optional_test_move.test | 10 | 0.001009s | 0.000988s | 0.001000s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000993s | 0.001013s | 0.000995s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000440s | 0.000430s | 0.000982s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000999s | 0.000981s | 0.000958s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001013s | 0.000988s | 0.001013s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000960s | 0.000963s | 0.001014s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001050s | 0.001002s | 0.001003s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001011s | 0.000969s | 0.001035s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000966s | 0.000982s | 0.000992s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000985s | 0.000966s | 0.000984s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001030s | 0.000970s | 0.000998s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001018s | 0.000973s | 0.000995s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001042s | 0.001046s | 0.001028s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001015s | 0.001044s | 0.000972s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000988s | 0.000984s | 0.000981s | byte equality observed |
| optional_test_swap.test | 10 | 0.000985s | 0.000982s | 0.000967s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001037s | 0.001008s | 0.001007s | byte equality observed |
| optional_test_tie.test | 10 | 0.001032s | 0.000998s | 0.000984s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001024s | 0.001014s | 0.001026s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001007s | 0.001000s | 0.000952s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000967s | 0.000979s | 0.000985s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000996s | 0.000976s | 0.001012s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
