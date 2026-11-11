# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.272655 | 1.456327 | 0.939366 | 0.001904 | 0.001918 | 0.001851 |
| optional_test_assign.test | RUN | 0.147117 | 0.319428 | 0.459348 | 0.001678 | 0.001924 | 0.001746 |
| optional_test_swap.test | RUN | 0.158554 | 0.389276 | 0.481724 | 0.001681 | 0.001770 | 0.001536 |
| optional_test_constexpr.test | COMPILE | 0.106182 | 0.215078 | 0.290375 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.161689 | 0.520492 | 0.446393 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.104795 | 0.216307 | 0.270740 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.165541 | 0.410288 | 0.546623 | 0.001814 | 0.001868 | 0.001804 |
| optional_test_constructors.test | RUN | 0.354463 | 1.489623 | 1.338648 | 0.001579 | 0.001523 | 0.001949 |
| optional_test_convert_from_T.test | RUN | 0.161195 | 0.340722 | 0.472222 | 0.001542 | 0.001821 | 0.001596 |
| optional_test_convert_assign.test | RUN | 0.149901 | 0.318498 | 0.455463 | 0.001602 | 0.001820 | 0.001383 |
| optional_test_empty_braces.test | RUN | 0.147314 | 0.332288 | 0.474087 | 0.001800 | 0.001847 | 0.001717 |
| optional_test_make_optional.test | RUN | 0.209980 | 0.500798 | 0.701977 | 0.001758 | 0.001900 | 0.001809 |
| optional_test_flat_map.test | RUN | 0.226716 | 0.589489 | 0.816724 | 0.001626 | 0.001542 | 0.001760 |
| optional_test_hash.test | RUN | 0.185306 | 0.463196 | 0.571710 | 0.001656 | 0.001885 | 0.001840 |
| optional_test_map.test | RUN | 0.225914 | 0.554889 | 0.771070 | 0.001831 | 0.001596 | 0.001664 |
| optional_test_tie.test | RUN | 0.159236 | 0.358059 | 0.451271 | 0.001690 | 0.001596 | 0.001839 |
| optional_test_ranges_find.test | RUN | 0.146379 | 0.317016 | 0.430257 | 0.001590 | 0.001828 | 0.001629 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.230263 | 0.913407 | 0.600690 | 0.001664 | 0.001756 | 0.001930 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.158841 | 0.389913 | 0.455814 | 0.001726 | 0.001610 | 0.001615 |
| optional_test_ref_assign_const_int.test | RUN | 0.161703 | 0.405227 | 0.489974 | 0.001760 | 0.001686 | 0.001792 |
| optional_test_ref_converting_ctor.test | RUN | 0.217687 | 0.561613 | 0.554353 | 0.001859 | 0.001935 | 0.001849 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.164611 | 0.373820 | 0.477940 | 0.001650 | 0.001886 | 0.001753 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.156258 | 0.341234 | 0.444052 | 0.001831 | 0.001630 | 0.001668 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.151233 | 0.343793 | 0.424980 | 0.001801 | 0.001814 | 0.001762 |
| optional_test_ref_portable_minimum.test | RUN | 0.525785 | 1.806138 | 1.588438 | 0.001821 | 0.001628 | 0.001568 |
| optional_test_ref_move.test | RUN | 0.162732 | 0.353915 | 0.453044 | 0.001800 | 0.001853 | 0.001948 |
| optional_test_ref_to_val.test | RUN | 0.219037 | 0.721616 | 0.662944 | 0.001756 | 0.001844 | 0.001535 |
| optional_test_inplace_factory.test | RUN | 0.194314 | 0.665338 | 0.566458 | 0.001840 | 0.001532 | 0.001721 |
| optional_test_io.test | RUN | 0.187499 | 0.720730 | 0.601102 | 0.001776 | 0.001808 | 0.001933 |
| optional_test_move.test | RUN | 0.180661 | 0.486861 | 0.504949 | 0.001548 | 0.001484 | 0.001779 |
| optional_test_noexcept_move.test | RUN | 0.109094 | 0.217230 | 0.296202 | 0.000583 | 0.000811 | 0.001412 |
| optional_test_cmp_none.test | RUN | 0.154545 | 0.341187 | 0.433429 | 0.001407 | 0.001918 | 0.001930 |
| optional_test_value_access.test | RUN | 0.180010 | 0.572500 | 0.601756 | 0.001678 | 0.001636 | 0.001683 |
| optional_test_emplace.test | RUN | 0.182670 | 0.575121 | 0.605727 | 0.001200 | 0.001672 | 0.001621 |
| optional_test_minimum_requirements.test | RUN | 0.164810 | 0.453667 | 0.512271 | 0.001776 | 0.001489 | 0.001930 |
| optional_test_msvc_bug_workaround.test | RUN | 0.155191 | 0.315392 | 0.421628 | 0.001670 | 0.001614 | 0.001858 |
| optional_test_member_T.test | RUN | 0.152829 | 0.313278 | 0.450727 | 0.001858 | 0.001398 | 0.001671 |
| optional_test_tc_base.test | RUN | 0.150097 | 0.321558 | 0.454477 | 0.001736 | 0.001485 | 0.001357 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.110319 | 0.229289 | 0.275020 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.110549 | 0.224875 | 0.296291 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.102530 | 0.201640 | 0.277164 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.140687 | 0.282066 | 0.422093 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.100846 | 0.198792 | 0.269886 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.103061 | 0.200846 | 0.256326 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.101009 | 0.196539 | 0.276910 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.101305 | 0.199461 | 0.251530 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.100356 | 0.200797 | 0.264266 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.100604 | 0.198872 | 0.263979 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.102889 | 0.201555 | 0.255343 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.171083 | 0.317974 | 0.456800 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.166816 | 0.321122 | 0.443627 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.100441 | 0.200428 | 0.264569 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.100903 | 0.199669 | 0.275208 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.097008 | 0.200455 | 0.258337 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.098898 | 0.198398 | 0.256650 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.100753 | 0.202532 | 0.266049 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.100442 | 0.196996 | 0.255549 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.097998 | 0.196752 | 0.265547 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.097029 | 0.195660 | 0.255590 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.097357 | 0.200185 | 0.264538 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.097299 | 0.206251 | 0.264284 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.136293 | 0.265678 | 0.383025 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.104895 | 0.220441 | 0.324850 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.099904 | 0.208625 | 0.283856 | — | — | — |
| optional_test_static_properties.test | RUN | 0.236334 | 0.456580 | 0.677289 | 0.001386 | 0.001883 | 0.001785 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.106411 | 0.217399 | 0.291708 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.107500 | 0.230460 | 0.294711 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.105626 | 0.214881 | 0.284919 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.119743 | 0.271696 | 0.377628 | 0.001416 | 0.001688 | 0.001700 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.118746 | 0.271148 | 0.375110 | 0.001519 | 0.001669 | 0.001656 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.121687 | 0.277802 | 0.338335 | 0.001593 | 0.001596 | 0.001702 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.124604 | 0.277238 | 0.334947 | 0.001705 | 0.001928 | 0.001480 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.085881 | 0.200071 | 0.295872 | 0.001384 | 0.001293 | 0.001304 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.065831 | 0.181431 | 0.290116 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.998×† | 1.000×, 0.995× | 1.059× |
| clang | 1.007×† | 1.072×, 0.941× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001131s | 0.001079s | 0.001091s | byte equality observed |
| optional_test_assign.test | 10 | 0.001033s | 0.001006s | 0.001026s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001025s | 0.001014s | 0.001049s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001062s | 0.001037s | 0.001048s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001042s | 0.001029s | 0.001060s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001041s | 0.001024s | 0.001056s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001067s | 0.001046s | 0.001034s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001053s | 0.001054s | 0.001049s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001029s | 0.001019s | 0.001060s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001056s | 0.001046s | 0.001017s | byte equality observed |
| optional_test_hash.test | 10 | 0.001061s | 0.001059s | 0.001116s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001041s | 0.001060s | 0.001075s | byte equality observed |
| optional_test_io.test | 10 | 0.001068s | 0.001046s | 0.001043s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001027s | 0.001047s | 0.001024s | byte equality observed |
| optional_test_map.test | 10 | 0.001052s | 0.001024s | 0.001034s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001021s | 0.001017s | 0.001026s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001058s | 0.001063s | 0.001085s | byte equality observed |
| optional_test_move.test | 10 | 0.001044s | 0.001008s | 0.001035s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001036s | 0.001039s | 0.001032s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000483s | 0.000480s | 0.001018s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001027s | 0.001051s | 0.001057s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001029s | 0.001036s | 0.001053s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001047s | 0.001045s | 0.001083s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001039s | 0.001045s | 0.001046s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001034s | 0.001015s | 0.001026s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001039s | 0.001011s | 0.001048s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001058s | 0.001036s | 0.001027s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001042s | 0.001031s | 0.001036s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001044s | 0.001029s | 0.001036s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001103s | 0.001035s | 0.001081s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001018s | 0.001032s | 0.001048s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001004s | 0.001047s | 0.001017s | byte equality observed |
| optional_test_swap.test | 10 | 0.001029s | 0.001037s | 0.001029s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001030s | 0.001006s | 0.001053s | byte equality observed |
| optional_test_tie.test | 10 | 0.001029s | 0.001016s | 0.001016s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001057s | 0.001064s | 0.001083s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000994s | 0.001020s | 0.001014s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001035s | 0.001013s | 0.001024s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001025s | 0.001026s | 0.001055s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
