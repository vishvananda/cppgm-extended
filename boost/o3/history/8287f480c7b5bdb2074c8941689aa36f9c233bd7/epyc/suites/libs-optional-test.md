# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.276596 | 1.452126 | 0.924601 | 0.001891 | 0.001889 | 0.001853 |
| optional_test_assign.test | RUN | 0.144356 | 0.318517 | 0.441802 | 0.001641 | 0.001780 | 0.001706 |
| optional_test_swap.test | RUN | 0.158668 | 0.392011 | 0.476853 | 0.001650 | 0.001852 | 0.001594 |
| optional_test_constexpr.test | COMPILE | 0.104729 | 0.215432 | 0.283490 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.164235 | 0.508825 | 0.431264 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.108969 | 0.213559 | 0.274037 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.167238 | 0.405764 | 0.510130 | 0.001865 | 0.001790 | 0.001742 |
| optional_test_constructors.test | RUN | 0.384211 | 1.487482 | 1.324060 | 0.001644 | 0.001612 | 0.001703 |
| optional_test_convert_from_T.test | RUN | 0.152129 | 0.334577 | 0.443571 | 0.001830 | 0.001773 | 0.001726 |
| optional_test_convert_assign.test | RUN | 0.155748 | 0.314999 | 0.470662 | 0.001658 | 0.001678 | 0.001758 |
| optional_test_empty_braces.test | RUN | 0.149600 | 0.327255 | 0.419506 | 0.001778 | 0.001851 | 0.001762 |
| optional_test_make_optional.test | RUN | 0.209011 | 0.477502 | 0.649028 | 0.001768 | 0.001824 | 0.001776 |
| optional_test_flat_map.test | RUN | 0.225454 | 0.558541 | 0.789604 | 0.001815 | 0.001522 | 0.001654 |
| optional_test_hash.test | RUN | 0.185195 | 0.457217 | 0.593932 | 0.001770 | 0.001837 | 0.001563 |
| optional_test_map.test | RUN | 0.226537 | 0.545315 | 0.781241 | 0.001702 | 0.001570 | 0.001470 |
| optional_test_tie.test | RUN | 0.163604 | 0.355509 | 0.497056 | 0.001769 | 0.001484 | 0.001329 |
| optional_test_ranges_find.test | RUN | 0.145297 | 0.313749 | 0.430647 | 0.001613 | 0.001783 | 0.001764 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.240867 | 0.897712 | 0.608086 | 0.001856 | 0.001623 | 0.001724 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.164505 | 0.384159 | 0.476785 | 0.001804 | 0.001624 | 0.001394 |
| optional_test_ref_assign_const_int.test | RUN | 0.168292 | 0.396617 | 0.481671 | 0.001792 | 0.001532 | 0.001569 |
| optional_test_ref_converting_ctor.test | RUN | 0.219852 | 0.544172 | 0.508356 | 0.001776 | 0.001798 | 0.001723 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.167813 | 0.369393 | 0.475027 | 0.001768 | 0.001484 | 0.001687 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.155407 | 0.333149 | 0.421687 | 0.001647 | 0.001713 | 0.001476 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.155127 | 0.342642 | 0.406913 | 0.001596 | 0.001652 | 0.001514 |
| optional_test_ref_portable_minimum.test | RUN | 0.546033 | 1.776115 | 1.576530 | 0.001786 | 0.001417 | 0.001485 |
| optional_test_ref_move.test | RUN | 0.156464 | 0.349876 | 0.487419 | 0.001627 | 0.001797 | 0.001710 |
| optional_test_ref_to_val.test | RUN | 0.221856 | 0.718730 | 0.646468 | 0.001773 | 0.001425 | 0.001743 |
| optional_test_inplace_factory.test | RUN | 0.189356 | 0.650583 | 0.601689 | 0.001676 | 0.001759 | 0.001458 |
| optional_test_io.test | RUN | 0.187673 | 0.712312 | 0.584078 | 0.001598 | 0.001644 | 0.001488 |
| optional_test_move.test | RUN | 0.183916 | 0.490076 | 0.529182 | 0.001594 | 0.001536 | 0.001716 |
| optional_test_noexcept_move.test | RUN | 0.106976 | 0.215270 | 0.298462 | 0.000813 | 0.000671 | 0.001393 |
| optional_test_cmp_none.test | RUN | 0.155686 | 0.348903 | 0.453322 | 0.001645 | 0.001888 | 0.001755 |
| optional_test_value_access.test | RUN | 0.173808 | 0.570738 | 0.564674 | 0.001701 | 0.001718 | 0.001838 |
| optional_test_emplace.test | RUN | 0.183005 | 0.564565 | 0.580078 | 0.001858 | 0.001712 | 0.001660 |
| optional_test_minimum_requirements.test | RUN | 0.169991 | 0.446439 | 0.518640 | 0.001644 | 0.001561 | 0.001744 |
| optional_test_msvc_bug_workaround.test | RUN | 0.154827 | 0.311337 | 0.426881 | 0.001649 | 0.001599 | 0.001710 |
| optional_test_member_T.test | RUN | 0.152572 | 0.310091 | 0.422735 | 0.001572 | 0.001582 | 0.001812 |
| optional_test_tc_base.test | RUN | 0.147752 | 0.313740 | 0.419218 | 0.001316 | 0.001531 | 0.001607 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.114933 | 0.224976 | 0.302451 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.108462 | 0.221901 | 0.259969 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.102399 | 0.197945 | 0.247368 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.140452 | 0.273125 | 0.404178 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.099342 | 0.198278 | 0.249647 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.099618 | 0.206502 | 0.260243 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.102579 | 0.198001 | 0.266658 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.101662 | 0.195859 | 0.254262 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.099248 | 0.197637 | 0.256603 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.100525 | 0.198356 | 0.251186 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.101573 | 0.197474 | 0.258654 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.168308 | 0.313091 | 0.449503 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.165950 | 0.316124 | 0.444183 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.096561 | 0.195956 | 0.259137 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.103213 | 0.197394 | 0.266454 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.100361 | 0.195600 | 0.260856 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.101139 | 0.197316 | 0.262943 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.103329 | 0.198138 | 0.247727 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.101851 | 0.194544 | 0.257845 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.100315 | 0.197051 | 0.264807 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.099990 | 0.200993 | 0.251845 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.099950 | 0.190277 | 0.259263 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.100877 | 0.204877 | 0.255437 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.133781 | 0.262403 | 0.383393 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.105866 | 0.213067 | 0.328882 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.100971 | 0.206905 | 0.257577 | — | — | — |
| optional_test_static_properties.test | RUN | 0.233243 | 0.452015 | 0.662063 | 0.001784 | 0.001818 | 0.001764 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.103717 | 0.218023 | 0.290599 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.105071 | 0.222958 | 0.296419 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.103156 | 0.210795 | 0.280501 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.120158 | 0.269159 | 0.379012 | 0.001666 | 0.001535 | 0.001653 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.118436 | 0.272597 | 0.333249 | 0.001257 | 0.001402 | 0.001639 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.120916 | 0.259306 | 0.380489 | 0.001280 | 0.001540 | 0.001627 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.123622 | 0.270295 | 0.345917 | 0.001381 | 0.001384 | 0.001697 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.102788 | 0.195563 | 0.248569 | 0.001373 | 0.001256 | 0.001194 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.076116 | 0.182970 | 0.241473 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.996×, 0.996× | 1.005× |
| clang | 1.014×† | 0.947×, 1.081× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001125s | 0.001110s | 0.001106s | byte equality observed |
| optional_test_assign.test | 10 | 0.001033s | 0.001018s | 0.001028s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001023s | 0.001010s | 0.001029s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001068s | 0.001034s | 0.001046s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001037s | 0.001020s | 0.001033s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001036s | 0.001009s | 0.001060s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001014s | 0.001015s | 0.001046s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001065s | 0.001049s | 0.001077s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001000s | 0.001055s | 0.001062s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001019s | 0.001008s | 0.001031s | byte equality observed |
| optional_test_hash.test | 10 | 0.001057s | 0.001064s | 0.001038s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001028s | 0.001011s | 0.001062s | byte equality observed |
| optional_test_io.test | 10 | 0.001069s | 0.001047s | 0.001058s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001044s | 0.001006s | 0.001020s | byte equality observed |
| optional_test_map.test | 10 | 0.001030s | 0.001026s | 0.001050s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001051s | 0.001015s | 0.001031s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001062s | 0.001049s | 0.001056s | byte equality observed |
| optional_test_move.test | 10 | 0.001063s | 0.001014s | 0.001018s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001046s | 0.001035s | 0.001012s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000484s | 0.000473s | 0.001030s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001038s | 0.001016s | 0.001023s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001027s | 0.001046s | 0.001054s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001016s | 0.001022s | 0.001062s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001029s | 0.001007s | 0.001058s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001012s | 0.001061s | 0.001032s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001025s | 0.001011s | 0.001069s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001038s | 0.001011s | 0.001080s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001067s | 0.001018s | 0.001070s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001024s | 0.001058s | 0.001040s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001068s | 0.001041s | 0.001028s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001049s | 0.001025s | 0.001018s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001048s | 0.001050s | 0.001039s | byte equality observed |
| optional_test_swap.test | 10 | 0.001024s | 0.001021s | 0.001062s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001033s | 0.001053s | 0.001081s | byte equality observed |
| optional_test_tie.test | 10 | 0.001027s | 0.001038s | 0.001047s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001077s | 0.001058s | 0.001076s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001022s | 0.001006s | 0.001043s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001024s | 0.001024s | 0.001060s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001010s | 0.001009s | 0.001039s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
