# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.240949 | 1.459837 | 0.979167 | 0.001667 | 0.001832 | 0.002046 |
| optional_test_assign.test | RUN | 0.130936 | 0.317003 | 0.478459 | 0.001772 | 0.001718 | 0.001800 |
| optional_test_swap.test | RUN | 0.135097 | 0.390704 | 0.497083 | 0.001344 | 0.001689 | 0.001722 |
| optional_test_constexpr.test | COMPILE | 0.091624 | 0.213091 | 0.307579 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.133281 | 0.527349 | 0.457661 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.089482 | 0.208460 | 0.281613 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.143046 | 0.411499 | 0.510394 | 0.001676 | 0.001850 | 0.001910 |
| optional_test_constructors.test | RUN | 0.307653 | 1.497815 | 1.390033 | 0.001496 | 0.001734 | 0.001915 |
| optional_test_convert_from_T.test | RUN | 0.137032 | 0.337789 | 0.483744 | 0.001611 | 0.001826 | 0.001566 |
| optional_test_convert_assign.test | RUN | 0.147246 | 0.338933 | 0.448621 | 0.001490 | 0.001826 | 0.001960 |
| optional_test_empty_braces.test | RUN | 0.132725 | 0.312473 | 0.422619 | 0.001884 | 0.001823 | 0.001828 |
| optional_test_make_optional.test | RUN | 0.209520 | 0.490170 | 0.674363 | 0.001533 | 0.001639 | 0.001890 |
| optional_test_flat_map.test | RUN | 0.200621 | 0.581316 | 0.793231 | 0.001434 | 0.001600 | 0.001681 |
| optional_test_hash.test | RUN | 0.164025 | 0.479438 | 0.614100 | 0.001353 | 0.001798 | 0.001647 |
| optional_test_map.test | RUN | 0.198738 | 0.552299 | 0.760457 | 0.001720 | 0.001382 | 0.001664 |
| optional_test_tie.test | RUN | 0.143145 | 0.373206 | 0.467360 | 0.001621 | 0.001668 | 0.001699 |
| optional_test_ranges_find.test | RUN | 0.129402 | 0.310332 | 0.447411 | 0.001729 | 0.001545 | 0.001799 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.160701 | 0.908233 | 0.621838 | 0.001583 | 0.001581 | 0.001778 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.134245 | 0.391154 | 0.483414 | 0.001756 | 0.001578 | 0.001699 |
| optional_test_ref_assign_const_int.test | RUN | 0.139669 | 0.400643 | 0.461442 | 0.001616 | 0.001600 | 0.001522 |
| optional_test_ref_converting_ctor.test | RUN | 0.149175 | 0.551033 | 0.531638 | 0.001623 | 0.001733 | 0.001708 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.139283 | 0.373213 | 0.456783 | 0.001486 | 0.001583 | 0.001896 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.135860 | 0.346648 | 0.476444 | 0.001766 | 0.001557 | 0.001516 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.129176 | 0.335216 | 0.484981 | 0.001625 | 0.001667 | 0.001431 |
| optional_test_ref_portable_minimum.test | RUN | 0.585428 | 1.821404 | 1.635840 | 0.001700 | 0.001794 | 0.001765 |
| optional_test_ref_move.test | RUN | 0.139319 | 0.348545 | 0.530137 | 0.001827 | 0.001835 | 0.001740 |
| optional_test_ref_to_val.test | RUN | 0.167636 | 0.712923 | 0.666044 | 0.001528 | 0.001689 | 0.001647 |
| optional_test_inplace_factory.test | RUN | 0.168602 | 0.650834 | 0.618130 | 0.001675 | 0.001702 | 0.001796 |
| optional_test_io.test | RUN | 0.165730 | 0.740055 | 0.599560 | 0.001812 | 0.001866 | 0.001557 |
| optional_test_move.test | RUN | 0.155782 | 0.497952 | 0.543491 | 0.001793 | 0.001530 | 0.001794 |
| optional_test_noexcept_move.test | RUN | 0.095829 | 0.224999 | 0.310633 | 0.000674 | 0.000783 | 0.001601 |
| optional_test_cmp_none.test | RUN | 0.136091 | 0.354384 | 0.461140 | 0.001836 | 0.001924 | 0.001852 |
| optional_test_value_access.test | RUN | 0.165816 | 0.583090 | 0.576254 | 0.001852 | 0.001607 | 0.001563 |
| optional_test_emplace.test | RUN | 0.161427 | 0.586966 | 0.585661 | 0.001888 | 0.001765 | 0.001680 |
| optional_test_minimum_requirements.test | RUN | 0.149460 | 0.461631 | 0.524661 | 0.001907 | 0.001670 | 0.001906 |
| optional_test_msvc_bug_workaround.test | RUN | 0.132450 | 0.312147 | 0.440858 | 0.001762 | 0.001584 | 0.001830 |
| optional_test_member_T.test | RUN | 0.133785 | 0.319781 | 0.426995 | 0.001707 | 0.001648 | 0.001802 |
| optional_test_tc_base.test | RUN | 0.134152 | 0.324473 | 0.445626 | 0.001506 | 0.001572 | 0.001582 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.101327 | 0.230194 | 0.312535 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.098446 | 0.224565 | 0.265395 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.086389 | 0.203156 | 0.246914 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.123201 | 0.282853 | 0.439019 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.086244 | 0.198817 | 0.274846 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.086501 | 0.207142 | 0.315965 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.088663 | 0.198435 | 0.303749 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.088101 | 0.199904 | 0.291819 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.085530 | 0.196037 | 0.297381 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.087156 | 0.195766 | 0.302924 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.086807 | 0.200137 | 0.313294 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.143492 | 0.317678 | 0.508612 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.146116 | 0.316556 | 0.485545 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.086214 | 0.197785 | 0.292298 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.088987 | 0.202624 | 0.285843 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.088913 | 0.197597 | 0.266403 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.092767 | 0.197383 | 0.283920 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.093661 | 0.194383 | 0.282928 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.089439 | 0.201091 | 0.264163 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.086380 | 0.197653 | 0.262812 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.084183 | 0.193531 | 0.273862 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.088650 | 0.193673 | 0.273701 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.089093 | 0.199990 | 0.261970 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.123597 | 0.270232 | 0.393042 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.094950 | 0.228716 | 0.315921 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.087707 | 0.196359 | 0.280030 | — | — | — |
| optional_test_static_properties.test | RUN | 0.210874 | 0.452088 | 0.701737 | 0.001710 | 0.001672 | 0.001820 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.093681 | 0.223669 | 0.290995 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.096314 | 0.225095 | 0.304407 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.093978 | 0.213722 | 0.297056 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.104495 | 0.272062 | 0.386292 | 0.001343 | 0.001570 | 0.001728 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.104290 | 0.273286 | 0.385417 | 0.001648 | 0.001367 | 0.001449 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.105886 | 0.264775 | 0.344596 | 0.001516 | 0.001245 | 0.001360 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.105915 | 0.277636 | 0.349118 | 0.001362 | 0.001876 | 0.001416 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.091558 | 0.168519 | 0.290187 | 0.001315 | 0.001277 | 0.001302 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.066979 | 0.189352 | 0.285313 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 0.997×, 1.007× | 1.014× |
| clang | 1.069× | 1.095×, 1.044× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001100s | 0.001113s | 0.001098s | byte equality observed |
| optional_test_assign.test | 10 | 0.001029s | 0.001009s | 0.001060s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001020s | 0.001041s | 0.000994s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001029s | 0.001024s | 0.001064s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001027s | 0.001000s | 0.000998s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001024s | 0.001106s | 0.000999s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001055s | 0.001075s | 0.001063s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001041s | 0.001040s | 0.001053s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001009s | 0.001045s | 0.001019s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001022s | 0.001039s | 0.001026s | byte equality observed |
| optional_test_hash.test | 10 | 0.001064s | 0.001096s | 0.000990s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001055s | 0.001084s | 0.001029s | byte equality observed |
| optional_test_io.test | 10 | 0.001066s | 0.001131s | 0.001065s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001060s | 0.001121s | 0.001048s | byte equality observed |
| optional_test_map.test | 10 | 0.001004s | 0.001100s | 0.001047s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001001s | 0.001031s | 0.001067s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001057s | 0.001130s | 0.001077s | byte equality observed |
| optional_test_move.test | 10 | 0.001036s | 0.001042s | 0.001025s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001044s | 0.001094s | 0.001035s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000470s | 0.000482s | 0.001008s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001046s | 0.001089s | 0.001091s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001028s | 0.001086s | 0.001082s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001049s | 0.001067s | 0.001044s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001047s | 0.001152s | 0.001053s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001024s | 0.001109s | 0.001011s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001008s | 0.001077s | 0.001036s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001026s | 0.001080s | 0.001008s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001037s | 0.001095s | 0.000993s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001043s | 0.001115s | 0.001000s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001056s | 0.001156s | 0.001051s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001008s | 0.001088s | 0.001091s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001016s | 0.001151s | 0.001129s | byte equality observed |
| optional_test_swap.test | 10 | 0.001021s | 0.001195s | 0.001107s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001024s | 0.001128s | 0.001134s | byte equality observed |
| optional_test_tie.test | 10 | 0.001052s | 0.001103s | 0.001111s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001058s | 0.001093s | 0.001143s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000982s | 0.001042s | 0.001096s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001026s | 0.001096s | 0.001094s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001031s | 0.001078s | 0.001098s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
