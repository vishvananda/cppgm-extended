# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.228050 | 1.222541 | 0.738820 | 0.001867 | 0.002108 | 0.001954 |
| optional_test_assign.test | RUN | 0.121143 | 0.265046 | 0.323090 | 0.001664 | 0.001894 | 0.001780 |
| optional_test_swap.test | RUN | 0.133708 | 0.332497 | 0.363471 | 0.001540 | 0.001971 | 0.001548 |
| optional_test_constexpr.test | COMPILE | 0.090453 | 0.178697 | 0.207195 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.136260 | 0.428352 | 0.346952 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.092840 | 0.179491 | 0.218084 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.142777 | 0.352460 | 0.453829 | 0.001876 | 0.001724 | 0.001953 |
| optional_test_constructors.test | RUN | 0.320009 | 1.212760 | 1.125159 | 0.001950 | 0.001650 | 0.001752 |
| optional_test_convert_from_T.test | RUN | 0.135040 | 0.281305 | 0.388712 | 0.001852 | 0.001947 | 0.001605 |
| optional_test_convert_assign.test | RUN | 0.133246 | 0.274670 | 0.381288 | 0.001568 | 0.001872 | 0.001944 |
| optional_test_empty_braces.test | RUN | 0.131198 | 0.280928 | 0.354077 | 0.001770 | 0.001862 | 0.001815 |
| optional_test_make_optional.test | RUN | 0.173425 | 0.423531 | 0.524781 | 0.001705 | 0.001948 | 0.001787 |
| optional_test_flat_map.test | RUN | 0.179526 | 0.497644 | 0.608159 | 0.001589 | 0.001702 | 0.001652 |
| optional_test_hash.test | RUN | 0.151898 | 0.396030 | 0.448400 | 0.003352 | 0.001680 | 0.002068 |
| optional_test_map.test | RUN | 0.213095 | 0.476650 | 0.640172 | 0.001547 | 0.001760 | 0.001581 |
| optional_test_tie.test | RUN | 0.172579 | 0.302415 | 0.378197 | 0.001941 | 0.001446 | 0.001752 |
| optional_test_ranges_find.test | RUN | 0.122188 | 0.272323 | 0.319216 | 0.001704 | 0.001539 | 0.001674 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.196645 | 0.769472 | 0.464323 | 0.001986 | 0.001248 | 0.001793 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.149157 | 0.326641 | 0.374093 | 0.001970 | 0.001703 | 0.001815 |
| optional_test_ref_assign_const_int.test | RUN | 0.148024 | 0.338476 | 0.369238 | 0.001792 | 0.001608 | 0.001924 |
| optional_test_ref_converting_ctor.test | RUN | 0.219845 | 0.463335 | 0.383859 | 0.001934 | 0.001860 | 0.001882 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.156708 | 0.304123 | 0.354334 | 0.001912 | 0.001762 | 0.001799 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.135189 | 0.274255 | 0.352216 | 0.002164 | 0.001854 | 0.002002 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.132648 | 0.273154 | 0.344770 | 0.001766 | 0.001307 | 0.001678 |
| optional_test_ref_portable_minimum.test | RUN | 0.434220 | 1.487698 | 1.232670 | 0.001668 | 0.001778 | 0.003538 |
| optional_test_ref_move.test | RUN | 0.134774 | 0.294252 | 0.348787 | 0.001920 | 0.001822 | 0.001947 |
| optional_test_ref_to_val.test | RUN | 0.190816 | 0.593897 | 0.502588 | 0.001775 | 0.001726 | 0.001964 |
| optional_test_inplace_factory.test | RUN | 0.162296 | 0.536201 | 0.453795 | 0.001524 | 0.001601 | 0.001781 |
| optional_test_io.test | RUN | 0.154153 | 0.621803 | 0.514176 | 0.001882 | 0.001910 | 0.002144 |
| optional_test_move.test | RUN | 0.149072 | 0.433085 | 0.415387 | 0.001777 | 0.001892 | 0.001968 |
| optional_test_noexcept_move.test | RUN | 0.088170 | 0.196436 | 0.205927 | 0.000876 | 0.000818 | 0.001684 |
| optional_test_cmp_none.test | RUN | 0.127588 | 0.310601 | 0.353110 | 0.001586 | 0.002086 | 0.001946 |
| optional_test_value_access.test | RUN | 0.153402 | 0.523389 | 0.469304 | 0.001912 | 0.002050 | 0.001580 |
| optional_test_emplace.test | RUN | 0.153650 | 0.517308 | 0.440065 | 0.001776 | 0.001596 | 0.001959 |
| optional_test_minimum_requirements.test | RUN | 0.139584 | 0.420490 | 0.416617 | 0.001430 | 0.001728 | 0.001855 |
| optional_test_msvc_bug_workaround.test | RUN | 0.128099 | 0.290296 | 0.332822 | 0.001395 | 0.001930 | 0.001791 |
| optional_test_member_T.test | RUN | 0.122896 | 0.274519 | 0.335503 | 0.001576 | 0.002005 | 0.001597 |
| optional_test_tc_base.test | RUN | 0.125573 | 0.281785 | 0.332631 | 0.001843 | 0.001599 | 0.001367 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.094634 | 0.196073 | 0.220011 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.091431 | 0.190723 | 0.214111 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.081106 | 0.177414 | 0.200448 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.119138 | 0.230573 | 0.294833 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.083799 | 0.168378 | 0.183996 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.090273 | 0.171935 | 0.194936 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.091109 | 0.169266 | 0.202585 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.090441 | 0.162036 | 0.199673 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.090383 | 0.167836 | 0.193028 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.085785 | 0.160531 | 0.201581 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.084432 | 0.158296 | 0.200688 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.139936 | 0.258900 | 0.337616 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.141008 | 0.261095 | 0.330753 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.082203 | 0.158667 | 0.202158 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.085486 | 0.162855 | 0.202696 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.089678 | 0.161794 | 0.191113 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.090157 | 0.161874 | 0.194813 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.083607 | 0.165326 | 0.193162 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.085026 | 0.165788 | 0.190782 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.089189 | 0.164369 | 0.192065 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.085294 | 0.164332 | 0.194387 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.091453 | 0.169809 | 0.194138 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.092387 | 0.168281 | 0.194480 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.119113 | 0.225738 | 0.291572 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.095270 | 0.185255 | 0.242910 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.093123 | 0.175449 | 0.201354 | — | — | — |
| optional_test_static_properties.test | RUN | 0.262023 | 0.380195 | 0.491226 | 0.001979 | 0.001786 | 0.001945 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.120261 | 0.183134 | 0.211698 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.122147 | 0.187615 | 0.222956 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.093644 | 0.178009 | 0.204118 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.098489 | 0.226888 | 0.292098 | 0.001758 | 0.001816 | 0.001659 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.101904 | 0.230916 | 0.280820 | 0.001645 | 0.001484 | 0.001484 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.101870 | 0.221047 | 0.272764 | 0.001526 | 0.001458 | 0.001564 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.103914 | 0.233360 | 0.278517 | 0.001465 | 0.001513 | 0.001506 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.084464 | 0.167559 | 0.207052 | 0.001303 | 0.001275 | 0.001355 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.065383 | 0.154917 | 0.200438 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.021× | 1.029×, 1.014× | 0.974× |
| clang | 1.000× | 0.982×, 1.018× | 1.018× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001040s | 0.001031s | 0.001041s | byte equality observed |
| optional_test_assign.test | 10 | 0.000955s | 0.000963s | 0.000939s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000977s | 0.000987s | 0.000938s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001001s | 0.000982s | 0.000989s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000960s | 0.000974s | 0.000945s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000956s | 0.000978s | 0.000950s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000961s | 0.000986s | 0.000954s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000991s | 0.001044s | 0.000991s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000980s | 0.000986s | 0.000966s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000984s | 0.000996s | 0.000967s | byte equality observed |
| optional_test_hash.test | 10 | 0.000969s | 0.000973s | 0.000963s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000977s | 0.001012s | 0.000971s | byte equality observed |
| optional_test_io.test | 10 | 0.000990s | 0.001001s | 0.001015s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000959s | 0.000995s | 0.000955s | byte equality observed |
| optional_test_map.test | 10 | 0.000970s | 0.000993s | 0.000966s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000951s | 0.000984s | 0.000967s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001022s | 0.000999s | 0.000990s | byte equality observed |
| optional_test_move.test | 10 | 0.000959s | 0.000974s | 0.000972s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000969s | 0.000985s | 0.000950s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000451s | 0.000457s | 0.000931s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000991s | 0.000978s | 0.000968s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000981s | 0.000962s | 0.000984s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000980s | 0.000977s | 0.000968s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000998s | 0.000960s | 0.000991s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000970s | 0.000968s | 0.000990s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000976s | 0.000949s | 0.000960s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000968s | 0.000960s | 0.001008s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000963s | 0.000954s | 0.000965s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000977s | 0.000964s | 0.001003s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001034s | 0.000988s | 0.001023s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000980s | 0.000983s | 0.000984s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000961s | 0.000961s | 0.000970s | byte equality observed |
| optional_test_swap.test | 10 | 0.000979s | 0.000969s | 0.000968s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000952s | 0.000976s | 0.000970s | byte equality observed |
| optional_test_tie.test | 10 | 0.000953s | 0.000964s | 0.000958s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001011s | 0.001014s | 0.001008s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000921s | 0.000955s | 0.000927s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000949s | 0.000968s | 0.000969s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000956s | 0.000963s | 0.000976s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
