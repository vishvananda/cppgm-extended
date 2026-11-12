# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.330428 | 2.053877 | 1.426283 | 0.001795 | 0.002358 | 0.002390 |
| optional_test_assign.test | RUN | 0.179781 | 0.445419 | 0.702804 | 0.001944 | 0.002152 | 0.002007 |
| optional_test_swap.test | RUN | 0.189257 | 0.542256 | 0.762077 | 0.001886 | 0.002220 | 0.002056 |
| optional_test_constexpr.test | COMPILE | 0.128032 | 0.300201 | 0.449616 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.185196 | 0.729860 | 0.707601 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.127862 | 0.305627 | 0.426110 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.210953 | 0.565166 | 0.857953 | 0.002132 | 0.002180 | 0.002127 |
| optional_test_constructors.test | RUN | 0.426776 | 2.085338 | 2.053652 | 0.001692 | 0.001496 | 0.002032 |
| optional_test_convert_from_T.test | RUN | 0.199854 | 0.460485 | 0.731944 | 0.001938 | 0.002260 | 0.001996 |
| optional_test_convert_assign.test | RUN | 0.197490 | 0.448365 | 0.688520 | 0.001762 | 0.002136 | 0.002132 |
| optional_test_empty_braces.test | RUN | 0.189114 | 0.447609 | 0.727364 | 0.002093 | 0.003076 | 0.002224 |
| optional_test_make_optional.test | RUN | 0.250012 | 0.693878 | 1.064030 | 0.002086 | 0.001812 | 0.001916 |
| optional_test_flat_map.test | RUN | 0.275063 | 0.791982 | 1.190470 | 0.002013 | 0.001924 | 0.002164 |
| optional_test_hash.test | RUN | 0.229420 | 0.656616 | 0.858458 | 0.002096 | 0.002048 | 0.002204 |
| optional_test_map.test | RUN | 0.276975 | 0.771694 | 1.231548 | 0.002144 | 0.002107 | 0.002055 |
| optional_test_tie.test | RUN | 0.203278 | 0.522612 | 0.733561 | 0.002260 | 0.002148 | 0.002123 |
| optional_test_ranges_find.test | RUN | 0.189965 | 0.432954 | 0.660681 | 0.001958 | 0.002084 | 0.002181 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.226200 | 1.268019 | 0.927577 | 0.001896 | 0.001878 | 0.002254 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.197178 | 0.545023 | 0.732577 | 0.001870 | 0.002214 | 0.001982 |
| optional_test_ref_assign_const_int.test | RUN | 0.211154 | 0.572200 | 0.723852 | 0.002058 | 0.001821 | 0.002237 |
| optional_test_ref_converting_ctor.test | RUN | 0.227741 | 0.786489 | 0.821311 | 0.001952 | 0.002172 | 0.002079 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.200087 | 0.513070 | 0.727774 | 0.001483 | 0.002283 | 0.002048 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.186363 | 0.481768 | 0.691257 | 0.001835 | 0.001866 | 0.002201 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.184167 | 0.477685 | 0.682644 | 0.002023 | 0.002048 | 0.002211 |
| optional_test_ref_portable_minimum.test | RUN | 0.815993 | 2.521208 | 2.425184 | 0.002170 | 0.001978 | 0.002232 |
| optional_test_ref_move.test | RUN | 0.185360 | 0.502132 | 0.761380 | 0.001865 | 0.002105 | 0.002002 |
| optional_test_ref_to_val.test | RUN | 0.239880 | 0.987635 | 1.001806 | 0.001874 | 0.001862 | 0.001857 |
| optional_test_inplace_factory.test | RUN | 0.239733 | 0.925939 | 0.922432 | 0.002070 | 0.002058 | 0.002201 |
| optional_test_io.test | RUN | 0.216996 | 0.987526 | 0.887237 | 0.001963 | 0.002128 | 0.002092 |
| optional_test_move.test | RUN | 0.218687 | 0.677712 | 0.819922 | 0.001936 | 0.001887 | 0.002041 |
| optional_test_noexcept_move.test | RUN | 0.128270 | 0.300408 | 0.476762 | 0.000844 | 0.001026 | 0.001751 |
| optional_test_cmp_none.test | RUN | 0.190565 | 0.488295 | 0.691486 | 0.001766 | 0.002191 | 0.002221 |
| optional_test_value_access.test | RUN | 0.214069 | 0.796187 | 0.876330 | 0.002061 | 0.002112 | 0.002217 |
| optional_test_emplace.test | RUN | 0.209618 | 0.792391 | 0.924683 | 0.001814 | 0.001896 | 0.002202 |
| optional_test_minimum_requirements.test | RUN | 0.195241 | 0.633866 | 0.843571 | 0.001897 | 0.001944 | 0.002490 |
| optional_test_msvc_bug_workaround.test | RUN | 0.183456 | 0.443466 | 0.669521 | 0.001743 | 0.001916 | 0.002131 |
| optional_test_member_T.test | RUN | 0.180434 | 0.441065 | 0.687945 | 0.002022 | 0.001968 | 0.001982 |
| optional_test_tc_base.test | RUN | 0.194276 | 0.446934 | 0.664150 | 0.002030 | 0.001855 | 0.001758 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.134580 | 0.320439 | 0.487231 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.140895 | 0.326279 | 0.429888 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.124607 | 0.278852 | 0.383016 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.168576 | 0.387709 | 0.622630 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.119612 | 0.279844 | 0.419970 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.125219 | 0.294030 | 0.424392 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.125573 | 0.286091 | 0.442557 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.130148 | 0.282483 | 0.409339 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.122002 | 0.277267 | 0.404422 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.123146 | 0.289135 | 0.426463 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.120997 | 0.278515 | 0.420548 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.207520 | 0.449647 | 0.708844 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.198599 | 0.452752 | 0.706099 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.127897 | 0.291406 | 0.421146 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.124432 | 0.306930 | 0.433510 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.119851 | 0.284870 | 0.419038 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.123534 | 0.275574 | 0.425754 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.123963 | 0.287452 | 0.421049 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.123660 | 0.287188 | 0.427457 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.122621 | 0.283632 | 0.424144 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.118932 | 0.281319 | 0.415694 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.119296 | 0.291877 | 0.401662 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.119038 | 0.290769 | 0.410566 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.166788 | 0.370253 | 0.609920 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.131531 | 0.308508 | 0.506104 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.122202 | 0.289266 | 0.434185 | — | — | — |
| optional_test_static_properties.test | RUN | 0.288851 | 0.657253 | 1.062341 | 0.001757 | 0.002146 | 0.002098 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.128843 | 0.299428 | 0.449707 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.135283 | 0.324750 | 0.458537 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.136514 | 0.291927 | 0.440026 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.156580 | 0.378261 | 0.540756 | 0.001500 | 0.001868 | 0.002164 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.152249 | 0.375547 | 0.596380 | 0.002010 | 0.002014 | 0.001912 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.150120 | 0.380669 | 0.599844 | 0.001791 | 0.001563 | 0.001786 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.147952 | 0.369893 | 0.548152 | 0.001719 | 0.003192 | 0.001502 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.103882 | 0.277049 | 0.388838 | 0.001571 | 0.001529 | 0.001493 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.063793 | 0.256289 | 0.378771 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 0.993×, 1.000× | 1.002× |
| clang | 1.019× | 1.020×, 1.019× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001361s | 0.001345s | 0.001366s | byte equality observed |
| optional_test_assign.test | 10 | 0.001246s | 0.001261s | 0.001324s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001267s | 0.001344s | 0.001304s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001282s | 0.001337s | 0.001314s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001260s | 0.001311s | 0.001300s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001336s | 0.001308s | 0.001320s | byte equality observed |
| optional_test_convert_from_T.test | 9 | 0.001278s | 0.001295s | 0.001312s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001284s | 0.001305s | 0.001279s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001296s | 0.001356s | 0.001214s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001288s | 0.001314s | 0.001282s | byte equality observed |
| optional_test_hash.test | 10 | 0.001317s | 0.001285s | 0.001309s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001283s | 0.001279s | 0.001297s | byte equality observed |
| optional_test_io.test | 10 | 0.001281s | 0.001359s | 0.001306s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001294s | 0.001263s | 0.001292s | byte equality observed |
| optional_test_map.test | 10 | 0.001276s | 0.001309s | 0.001309s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001303s | 0.001304s | 0.001278s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001349s | 0.001321s | 0.001296s | byte equality observed |
| optional_test_move.test | 10 | 0.001298s | 0.001341s | 0.001274s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001285s | 0.001292s | 0.001307s | byte equality observed |
| optional_test_noexcept_move.test | 8 | 0.000619s | 0.000613s | 0.001273s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001242s | 0.001275s | 0.001252s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001267s | 0.001235s | 0.001307s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001282s | 0.001298s | 0.001257s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001297s | 0.001268s | 0.001268s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001285s | 0.001290s | 0.001255s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001307s | 0.001327s | 0.001259s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001291s | 0.001331s | 0.001260s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001294s | 0.001308s | 0.001275s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001317s | 0.001259s | 0.001318s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001303s | 0.001293s | 0.001317s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001284s | 0.001321s | 0.001262s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001291s | 0.001256s | 0.001314s | byte equality observed |
| optional_test_swap.test | 10 | 0.001315s | 0.001271s | 0.001257s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001255s | 0.001240s | 0.001300s | byte equality observed |
| optional_test_tie.test | 10 | 0.001292s | 0.001312s | 0.001354s | byte equality observed |
| optional_test_value_access.test | 9 | 0.001386s | 0.001346s | 0.001307s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001291s | 0.001280s | 0.001315s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001262s | 0.001278s | 0.001288s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001274s | 0.001256s | 0.001326s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
