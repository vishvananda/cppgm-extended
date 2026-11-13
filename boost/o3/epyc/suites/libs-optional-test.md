# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.322719 | 2.023799 | 1.393464 | 0.002160 | 0.002139 | 0.002128 |
| optional_test_assign.test | RUN | 0.169869 | 0.435923 | 0.681686 | 0.001738 | 0.002207 | 0.002175 |
| optional_test_swap.test | RUN | 0.182796 | 0.530991 | 0.753953 | 0.001814 | 0.002160 | 0.001852 |
| optional_test_constexpr.test | COMPILE | 0.123089 | 0.296346 | 0.450368 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.185617 | 0.728493 | 0.683787 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.125837 | 0.295407 | 0.411063 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.202675 | 0.558065 | 0.800171 | 0.001882 | 0.002081 | 0.002108 |
| optional_test_constructors.test | RUN | 0.413290 | 2.035961 | 2.030705 | 0.001990 | 0.001888 | 0.001704 |
| optional_test_convert_from_T.test | RUN | 0.188125 | 0.451194 | 0.697902 | 0.001852 | 0.002130 | 0.001966 |
| optional_test_convert_assign.test | RUN | 0.176434 | 0.436062 | 0.706862 | 0.001793 | 0.002012 | 0.002008 |
| optional_test_empty_braces.test | RUN | 0.184419 | 0.442367 | 0.638090 | 0.001940 | 0.002154 | 0.002116 |
| optional_test_make_optional.test | RUN | 0.249487 | 0.683753 | 1.038643 | 0.002011 | 0.002001 | 0.002030 |
| optional_test_flat_map.test | RUN | 0.264647 | 0.760271 | 1.131325 | 0.001761 | 0.001532 | 0.001977 |
| optional_test_hash.test | RUN | 0.220306 | 0.642427 | 0.872367 | 0.001796 | 0.001922 | 0.002097 |
| optional_test_map.test | RUN | 0.267425 | 0.763069 | 1.181162 | 0.001750 | 0.001880 | 0.001787 |
| optional_test_tie.test | RUN | 0.187816 | 0.513123 | 0.693235 | 0.001872 | 0.002082 | 0.002074 |
| optional_test_ranges_find.test | RUN | 0.175027 | 0.424167 | 0.669078 | 0.001731 | 0.001838 | 0.001733 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.220199 | 1.255833 | 0.931438 | 0.001850 | 0.001930 | 0.002094 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.184327 | 0.535077 | 0.699481 | 0.002043 | 0.001849 | 0.002052 |
| optional_test_ref_assign_const_int.test | RUN | 0.183215 | 0.552836 | 0.752342 | 0.001886 | 0.001946 | 0.001925 |
| optional_test_ref_converting_ctor.test | RUN | 0.206495 | 0.762868 | 0.824244 | 0.001784 | 0.002147 | 0.001896 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.189060 | 0.511709 | 0.672419 | 0.001797 | 0.001996 | 0.002164 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.177515 | 0.488666 | 0.680865 | 0.002006 | 0.001759 | 0.002074 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.178168 | 0.456505 | 0.670577 | 0.001605 | 0.001821 | 0.002064 |
| optional_test_ref_portable_minimum.test | RUN | 0.819482 | 2.477018 | 2.453392 | 0.001964 | 0.001901 | 0.001705 |
| optional_test_ref_move.test | RUN | 0.185094 | 0.489786 | 0.738137 | 0.002014 | 0.001991 | 0.002025 |
| optional_test_ref_to_val.test | RUN | 0.217630 | 0.984364 | 0.945909 | 0.001907 | 0.001732 | 0.001927 |
| optional_test_inplace_factory.test | RUN | 0.232316 | 0.916344 | 0.877848 | 0.002076 | 0.002054 | 0.002144 |
| optional_test_io.test | RUN | 0.225895 | 0.970010 | 0.876840 | 0.002042 | 0.001989 | 0.002107 |
| optional_test_move.test | RUN | 0.207061 | 0.667188 | 0.754607 | 0.002020 | 0.001718 | 0.001883 |
| optional_test_noexcept_move.test | RUN | 0.125187 | 0.295440 | 0.394413 | 0.000734 | 0.000811 | 0.001624 |
| optional_test_cmp_none.test | RUN | 0.176741 | 0.488669 | 0.671380 | 0.002117 | 0.001890 | 0.002038 |
| optional_test_value_access.test | RUN | 0.214171 | 0.787380 | 0.858113 | 0.001882 | 0.002028 | 0.001903 |
| optional_test_emplace.test | RUN | 0.216375 | 0.789496 | 0.912133 | 0.001767 | 0.001934 | 0.001998 |
| optional_test_minimum_requirements.test | RUN | 0.193716 | 0.624786 | 0.819192 | 0.001961 | 0.001893 | 0.001816 |
| optional_test_msvc_bug_workaround.test | RUN | 0.172914 | 0.430290 | 0.655443 | 0.001868 | 0.001709 | 0.002094 |
| optional_test_member_T.test | RUN | 0.175743 | 0.427136 | 0.668557 | 0.001697 | 0.001920 | 0.001920 |
| optional_test_tc_base.test | RUN | 0.175844 | 0.440253 | 0.637131 | 0.001772 | 0.002094 | 0.001859 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.131622 | 0.313175 | 0.481020 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.122112 | 0.307535 | 0.403271 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.120118 | 0.274432 | 0.356459 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.165382 | 0.385572 | 0.611741 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.116867 | 0.281924 | 0.396760 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.116091 | 0.283309 | 0.434358 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.120018 | 0.278638 | 0.417639 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.119205 | 0.279832 | 0.399825 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.114403 | 0.274818 | 0.395875 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.116709 | 0.272837 | 0.407308 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.118845 | 0.270921 | 0.422053 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.190079 | 0.437665 | 0.697953 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.188492 | 0.446880 | 0.691026 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.115904 | 0.283102 | 0.417607 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.118516 | 0.292046 | 0.430643 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.114724 | 0.274771 | 0.402323 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.117714 | 0.272469 | 0.410389 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.117383 | 0.282284 | 0.407755 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.116553 | 0.285570 | 0.410109 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.119811 | 0.281272 | 0.418350 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.116028 | 0.269083 | 0.419102 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.113928 | 0.277563 | 0.408149 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.115908 | 0.289987 | 0.412456 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.158588 | 0.368753 | 0.599544 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.127623 | 0.302422 | 0.511822 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.116668 | 0.284305 | 0.419829 | — | — | — |
| optional_test_static_properties.test | RUN | 0.273312 | 0.640343 | 1.063927 | 0.001793 | 0.002191 | 0.002037 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.121808 | 0.303269 | 0.446332 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.126950 | 0.316464 | 0.463072 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.118988 | 0.292599 | 0.434561 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.143991 | 0.367319 | 0.584892 | 0.001850 | 0.001885 | 0.001822 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.142592 | 0.378256 | 0.522131 | 0.001832 | 0.002090 | 0.001867 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.143358 | 0.372579 | 0.574150 | 0.001646 | 0.002024 | 0.001692 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.145009 | 0.373554 | 0.520874 | 0.001846 | 0.002593 | 0.002032 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.100602 | 0.251540 | 0.392175 | 0.001542 | 0.001460 | 0.001309 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.061722 | 0.236831 | 0.343128 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 0.990×, 1.012× | 1.007× |
| clang | 1.010× | 1.003×, 1.017× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001354s | 0.001317s | 0.001321s | byte equality observed |
| optional_test_assign.test | 10 | 0.001248s | 0.001253s | 0.001283s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001249s | 0.001226s | 0.001225s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001276s | 0.001270s | 0.001278s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001245s | 0.001246s | 0.001305s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001293s | 0.001228s | 0.001242s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001238s | 0.001221s | 0.001235s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001268s | 0.001311s | 0.001299s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001279s | 0.001262s | 0.001219s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001243s | 0.001289s | 0.001228s | byte equality observed |
| optional_test_hash.test | 10 | 0.001222s | 0.001234s | 0.001249s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001197s | 0.001225s | 0.001227s | byte equality observed |
| optional_test_io.test | 10 | 0.001268s | 0.001262s | 0.001288s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001244s | 0.001220s | 0.001258s | byte equality observed |
| optional_test_map.test | 10 | 0.001225s | 0.001232s | 0.001220s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001234s | 0.001226s | 0.001302s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001254s | 0.001316s | 0.001308s | byte equality observed |
| optional_test_move.test | 10 | 0.001226s | 0.001196s | 0.001286s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001246s | 0.001221s | 0.001254s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000571s | 0.000581s | 0.001203s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001220s | 0.001237s | 0.001244s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001237s | 0.001294s | 0.001215s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001265s | 0.001312s | 0.001232s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001263s | 0.001209s | 0.001262s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001227s | 0.001245s | 0.001246s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001249s | 0.001256s | 0.001259s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001290s | 0.001223s | 0.001187s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001267s | 0.001269s | 0.001270s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001268s | 0.001281s | 0.001282s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001280s | 0.001293s | 0.001266s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001247s | 0.001238s | 0.001310s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001276s | 0.001264s | 0.001236s | byte equality observed |
| optional_test_swap.test | 10 | 0.001245s | 0.001269s | 0.001263s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001292s | 0.001252s | 0.001255s | byte equality observed |
| optional_test_tie.test | 10 | 0.001216s | 0.001228s | 0.001193s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001284s | 0.001311s | 0.001290s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001251s | 0.001203s | 0.001213s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001230s | 0.001232s | 0.001228s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001241s | 0.001206s | 0.001244s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
