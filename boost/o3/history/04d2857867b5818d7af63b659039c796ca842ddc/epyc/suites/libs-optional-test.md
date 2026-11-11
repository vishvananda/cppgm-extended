# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.250675 | 1.451787 | 0.732796 | 0.001970 | 0.001968 | 0.001766 |
| optional_test_assign.test | RUN | 0.136614 | 0.408924 | 0.334793 | 0.001758 | 0.002080 | 0.001689 |
| optional_test_swap.test | RUN | 0.150660 | 0.480307 | 0.373160 | 0.001884 | 0.001777 | 0.001421 |
| optional_test_constexpr.test | COMPILE | 0.097821 | 0.301043 | 0.218214 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.142894 | 0.475834 | 0.337384 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.093384 | 0.209825 | 0.204632 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.153612 | 0.383369 | 0.413459 | 0.001834 | 0.001663 | 0.001828 |
| optional_test_constructors.test | RUN | 0.300165 | 1.342160 | 1.051030 | 0.001460 | 0.001715 | 0.001844 |
| optional_test_convert_from_T.test | RUN | 0.138015 | 0.294832 | 0.358567 | 0.001776 | 0.002002 | 0.001713 |
| optional_test_convert_assign.test | RUN | 0.140623 | 0.290914 | 0.320066 | 0.001596 | 0.001826 | 0.001633 |
| optional_test_empty_braces.test | RUN | 0.133763 | 0.304233 | 0.321168 | 0.001772 | 0.001918 | 0.001842 |
| optional_test_make_optional.test | RUN | 0.181489 | 0.467425 | 0.543819 | 0.001708 | 0.001694 | 0.001782 |
| optional_test_flat_map.test | RUN | 0.196918 | 0.528746 | 0.571688 | 0.001568 | 0.001835 | 0.001844 |
| optional_test_hash.test | RUN | 0.165003 | 0.427450 | 0.442159 | 0.001784 | 0.001812 | 0.001776 |
| optional_test_map.test | RUN | 0.198210 | 0.494481 | 0.607664 | 0.001826 | 0.001517 | 0.001893 |
| optional_test_tie.test | RUN | 0.143140 | 0.326609 | 0.379912 | 0.001496 | 0.001802 | 0.001432 |
| optional_test_ranges_find.test | RUN | 0.129305 | 0.262628 | 0.331744 | 0.001686 | 0.001727 | 0.001276 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.190417 | 0.830825 | 0.483836 | 0.001972 | 0.002082 | 0.001775 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.142943 | 0.406312 | 0.369415 | 0.001916 | 0.001772 | 0.001686 |
| optional_test_ref_assign_const_int.test | RUN | 0.144092 | 0.407176 | 0.371561 | 0.001826 | 0.001696 | 0.001143 |
| optional_test_ref_converting_ctor.test | RUN | 0.183872 | 0.537132 | 0.390575 | 0.001956 | 0.001880 | 0.001481 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.149663 | 0.329501 | 0.343904 | 0.001468 | 0.001845 | 0.001835 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.139373 | 0.297073 | 0.327702 | 0.001912 | 0.001504 | 0.001600 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.140193 | 0.286271 | 0.324144 | 0.001810 | 0.001994 | 0.001658 |
| optional_test_ref_portable_minimum.test | RUN | 0.435790 | 1.591457 | 1.223067 | 0.001754 | 0.001555 | 0.001728 |
| optional_test_ref_move.test | RUN | 0.147780 | 0.342487 | 0.363980 | 0.001856 | 0.001904 | 0.001712 |
| optional_test_ref_to_val.test | RUN | 0.194042 | 0.640631 | 0.498923 | 0.001828 | 0.001467 | 0.001446 |
| optional_test_inplace_factory.test | RUN | 0.170079 | 0.596190 | 0.459473 | 0.001534 | 0.001836 | 0.001799 |
| optional_test_io.test | RUN | 0.170336 | 0.637279 | 0.482881 | 0.001913 | 0.001821 | 0.001695 |
| optional_test_move.test | RUN | 0.161805 | 0.424357 | 0.430290 | 0.001848 | 0.001947 | 0.001812 |
| optional_test_noexcept_move.test | RUN | 0.092618 | 0.184335 | 0.224565 | 0.000828 | 0.000548 | 0.001842 |
| optional_test_cmp_none.test | RUN | 0.131971 | 0.295223 | 0.359312 | 0.001578 | 0.001943 | 0.001748 |
| optional_test_value_access.test | RUN | 0.159686 | 0.502603 | 0.423639 | 0.001719 | 0.002113 | 0.001611 |
| optional_test_emplace.test | RUN | 0.159462 | 0.487533 | 0.462321 | 0.001867 | 0.001491 | 0.001570 |
| optional_test_minimum_requirements.test | RUN | 0.144404 | 0.378109 | 0.393981 | 0.001694 | 0.001540 | 0.001824 |
| optional_test_msvc_bug_workaround.test | RUN | 0.134962 | 0.279722 | 0.312320 | 0.001562 | 0.001998 | 0.001765 |
| optional_test_member_T.test | RUN | 0.133521 | 0.281814 | 0.317311 | 0.001864 | 0.002190 | 0.001426 |
| optional_test_tc_base.test | RUN | 0.132242 | 0.291052 | 0.327726 | 0.001845 | 0.001616 | 0.001697 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.102322 | 0.195549 | 0.227673 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.099299 | 0.203907 | 0.195018 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.091558 | 0.185265 | 0.184684 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.124113 | 0.251875 | 0.290209 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.088169 | 0.176617 | 0.195527 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.091104 | 0.177259 | 0.196803 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.091145 | 0.173544 | 0.199476 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.092335 | 0.166674 | 0.200647 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.090014 | 0.165515 | 0.194618 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.092278 | 0.166717 | 0.196985 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.090855 | 0.169037 | 0.196274 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.147405 | 0.258875 | 0.325636 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.145735 | 0.261536 | 0.320834 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.088059 | 0.162203 | 0.197093 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.093058 | 0.168737 | 0.189395 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.089032 | 0.169551 | 0.185182 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.089769 | 0.163927 | 0.199139 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.092540 | 0.168430 | 0.199082 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.091341 | 0.170732 | 0.196490 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.091080 | 0.173045 | 0.208946 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.089133 | 0.169828 | 0.208088 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.090372 | 0.171275 | 0.197710 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.088735 | 0.173089 | 0.190774 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.119530 | 0.214506 | 0.292424 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.093096 | 0.188664 | 0.244978 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.095191 | 0.173101 | 0.205455 | — | — | — |
| optional_test_static_properties.test | RUN | 0.212599 | 0.388410 | 0.504749 | 0.001711 | 0.001811 | 0.001836 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.093393 | 0.180469 | 0.219423 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.096677 | 0.185811 | 0.225414 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.094536 | 0.178154 | 0.210211 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.103072 | 0.227722 | 0.286987 | 0.002061 | 0.001528 | 0.001890 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.106679 | 0.221561 | 0.286278 | 0.001554 | 0.001553 | 0.001476 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.107424 | 0.231771 | 0.271132 | 0.001732 | 0.001461 | 0.001608 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.105293 | 0.241800 | 0.275559 | 0.001354 | 0.001901 | 0.001882 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.097413 | 0.173583 | 0.193542 | 0.001304 | 0.001261 | 0.001307 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.053433 | 0.165555 | 0.185527 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.987×† | 0.985×, 0.989× | 1.057× |
| clang | 1.032× | 1.012×, 1.051× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001053s | 0.001009s | 0.001121s | byte equality observed |
| optional_test_assign.test | 10 | 0.000948s | 0.000942s | 0.001036s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000948s | 0.000951s | 0.001006s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000990s | 0.000959s | 0.000975s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000949s | 0.000955s | 0.000992s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000960s | 0.000951s | 0.000983s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000984s | 0.000949s | 0.000962s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000982s | 0.000989s | 0.000996s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000952s | 0.000980s | 0.000959s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000944s | 0.000971s | 0.000983s | byte equality observed |
| optional_test_hash.test | 10 | 0.000968s | 0.000966s | 0.001038s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000969s | 0.000967s | 0.000964s | byte equality observed |
| optional_test_io.test | 10 | 0.000995s | 0.000987s | 0.001006s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000958s | 0.000945s | 0.000945s | byte equality observed |
| optional_test_map.test | 10 | 0.000943s | 0.000961s | 0.000961s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000948s | 0.000945s | 0.000963s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000984s | 0.000983s | 0.000996s | byte equality observed |
| optional_test_move.test | 10 | 0.000928s | 0.000947s | 0.000991s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000984s | 0.000964s | 0.000961s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000450s | 0.000432s | 0.000945s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000954s | 0.000950s | 0.000956s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000973s | 0.000954s | 0.000983s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000962s | 0.000938s | 0.000988s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000974s | 0.000943s | 0.000980s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000983s | 0.000951s | 0.000972s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000958s | 0.000968s | 0.000999s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000987s | 0.000952s | 0.001004s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000964s | 0.000966s | 0.000977s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000950s | 0.000974s | 0.000962s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001036s | 0.000990s | 0.001028s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000949s | 0.000975s | 0.000972s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000954s | 0.000935s | 0.001023s | byte equality observed |
| optional_test_swap.test | 10 | 0.000969s | 0.000954s | 0.001034s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000952s | 0.000962s | 0.000987s | byte equality observed |
| optional_test_tie.test | 10 | 0.000951s | 0.000959s | 0.000962s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001005s | 0.000995s | 0.001005s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000940s | 0.000915s | 0.000935s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000956s | 0.000940s | 0.000975s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000942s | 0.000910s | 0.000976s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
