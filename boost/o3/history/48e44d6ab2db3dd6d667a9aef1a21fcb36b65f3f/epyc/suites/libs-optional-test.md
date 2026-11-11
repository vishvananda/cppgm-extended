# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.370168 | 2.039458 | 1.468679 | 0.002169 | 0.002257 | 0.002454 |
| optional_test_assign.test | RUN | 0.195867 | 0.443138 | 0.720267 | 0.001864 | 0.002160 | 0.001934 |
| optional_test_swap.test | RUN | 0.217050 | 0.537175 | 0.780296 | 0.001922 | 0.002160 | 0.001691 |
| optional_test_constexpr.test | COMPILE | 0.140436 | 0.296899 | 0.472602 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.217033 | 0.715029 | 0.755790 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.140588 | 0.302303 | 0.445729 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.231905 | 0.563252 | 0.861178 | 0.002076 | 0.002202 | 0.002274 |
| optional_test_constructors.test | RUN | 0.488695 | 2.080722 | 2.075152 | 0.001825 | 0.001944 | 0.001953 |
| optional_test_convert_from_T.test | RUN | 0.206474 | 0.459429 | 0.708143 | 0.002078 | 0.002166 | 0.002320 |
| optional_test_convert_assign.test | RUN | 0.215107 | 0.457752 | 0.745279 | 0.001907 | 0.002300 | 0.002362 |
| optional_test_empty_braces.test | RUN | 0.208672 | 0.442925 | 0.669040 | 0.002045 | 0.002259 | 0.002320 |
| optional_test_make_optional.test | RUN | 0.267515 | 0.662402 | 1.077099 | 0.001985 | 0.001987 | 0.001959 |
| optional_test_flat_map.test | RUN | 0.306411 | 0.807175 | 1.208928 | 0.001924 | 0.002026 | 0.001725 |
| optional_test_hash.test | RUN | 0.256137 | 0.667717 | 0.925496 | 0.001862 | 0.002204 | 0.001812 |
| optional_test_map.test | RUN | 0.304006 | 0.773506 | 1.217245 | 0.002000 | 0.002032 | 0.002132 |
| optional_test_tie.test | RUN | 0.231837 | 0.499189 | 0.785914 | 0.002038 | 0.001812 | 0.002108 |
| optional_test_ranges_find.test | RUN | 0.195921 | 0.433907 | 0.679429 | 0.001854 | 0.001902 | 0.001909 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.327092 | 1.270266 | 0.947555 | 0.002002 | 0.001910 | 0.002430 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.213894 | 0.541953 | 0.722444 | 0.002071 | 0.001816 | 0.002346 |
| optional_test_ref_assign_const_int.test | RUN | 0.222914 | 0.552802 | 0.727347 | 0.002042 | 0.001765 | 0.002073 |
| optional_test_ref_converting_ctor.test | RUN | 0.328434 | 0.781289 | 0.815674 | 0.002035 | 0.002295 | 0.002272 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.221595 | 0.517798 | 0.725879 | 0.002083 | 0.001674 | 0.002250 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.207838 | 0.464357 | 0.677213 | 0.001848 | 0.001960 | 0.002246 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.201745 | 0.478084 | 0.675523 | 0.002064 | 0.002108 | 0.002236 |
| optional_test_ref_portable_minimum.test | RUN | 0.749281 | 2.486475 | 2.507429 | 0.002245 | 0.002148 | 0.001924 |
| optional_test_ref_move.test | RUN | 0.216612 | 0.485361 | 0.711779 | 0.002020 | 0.002056 | 0.002368 |
| optional_test_ref_to_val.test | RUN | 0.312032 | 0.984148 | 1.045424 | 0.002100 | 0.001965 | 0.002200 |
| optional_test_inplace_factory.test | RUN | 0.260013 | 0.911367 | 0.912199 | 0.001945 | 0.001938 | 0.002136 |
| optional_test_io.test | RUN | 0.250110 | 0.988323 | 0.943241 | 0.001984 | 0.002022 | 0.002071 |
| optional_test_move.test | RUN | 0.250599 | 0.673381 | 0.786374 | 0.002026 | 0.002204 | 0.002044 |
| optional_test_noexcept_move.test | RUN | 0.145566 | 0.304418 | 0.428033 | 0.000806 | 0.000806 | 0.001687 |
| optional_test_cmp_none.test | RUN | 0.202497 | 0.475347 | 0.695828 | 0.002001 | 0.002192 | 0.002408 |
| optional_test_value_access.test | RUN | 0.244825 | 0.793085 | 0.880898 | 0.001818 | 0.002096 | 0.002431 |
| optional_test_emplace.test | RUN | 0.249145 | 0.803379 | 0.954982 | 0.001632 | 0.001934 | 0.001832 |
| optional_test_minimum_requirements.test | RUN | 0.220710 | 0.638544 | 0.856898 | 0.001992 | 0.002052 | 0.001748 |
| optional_test_msvc_bug_workaround.test | RUN | 0.208253 | 0.450150 | 0.646851 | 0.001997 | 0.002007 | 0.002238 |
| optional_test_member_T.test | RUN | 0.207127 | 0.425514 | 0.691231 | 0.001843 | 0.001880 | 0.001927 |
| optional_test_tc_base.test | RUN | 0.206569 | 0.440524 | 0.721859 | 0.002084 | 0.002232 | 0.002273 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.150073 | 0.317604 | 0.425623 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.147908 | 0.302641 | 0.448911 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.140137 | 0.280692 | 0.413691 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.189355 | 0.390505 | 0.633068 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.132534 | 0.283495 | 0.443243 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.136617 | 0.284525 | 0.423259 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.140589 | 0.296389 | 0.437137 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.138901 | 0.277401 | 0.423380 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.136089 | 0.290365 | 0.404268 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.138433 | 0.280457 | 0.413315 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.135004 | 0.290634 | 0.410339 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.226302 | 0.439721 | 0.696863 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.227386 | 0.453709 | 0.705794 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.132437 | 0.276789 | 0.433284 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.134518 | 0.291696 | 0.440815 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.136080 | 0.273967 | 0.427316 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.136605 | 0.271537 | 0.439460 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.136584 | 0.287970 | 0.410193 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.138150 | 0.283486 | 0.425365 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.136291 | 0.274480 | 0.422610 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.138671 | 0.271700 | 0.413338 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.132867 | 0.277007 | 0.427541 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.139712 | 0.283982 | 0.423862 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.182388 | 0.364023 | 0.610601 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.144190 | 0.307638 | 0.507998 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.137475 | 0.291031 | 0.450417 | — | — | — |
| optional_test_static_properties.test | RUN | 0.320498 | 0.644336 | 1.081712 | 0.002042 | 0.002050 | 0.002204 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.138988 | 0.305688 | 0.452157 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.141855 | 0.314217 | 0.491669 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.143581 | 0.302330 | 0.439669 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.155091 | 0.379449 | 0.592581 | 0.001901 | 0.002245 | 0.001976 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.154904 | 0.388448 | 0.549168 | 0.001885 | 0.001571 | 0.001880 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.168996 | 0.370421 | 0.583635 | 0.001772 | 0.001818 | 0.001746 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.164502 | 0.388797 | 0.561505 | 0.002078 | 0.002290 | 0.002390 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.129078 | 0.263717 | 0.390136 | 0.001541 | 0.001358 | 0.001606 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.073421 | 0.237886 | 0.343319 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.988×, 0.992× | 0.996× |
| clang | 1.020× | 1.013×, 1.027× | 0.983× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001357s | 0.001339s | 0.001357s | byte equality observed |
| optional_test_assign.test | 10 | 0.001286s | 0.001331s | 0.001339s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001273s | 0.001294s | 0.001273s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001283s | 0.001314s | 0.001305s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001268s | 0.001288s | 0.001302s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001259s | 0.001265s | 0.001269s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001311s | 0.001272s | 0.001283s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001332s | 0.001288s | 0.001310s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001271s | 0.001297s | 0.001305s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001303s | 0.001253s | 0.001261s | byte equality observed |
| optional_test_hash.test | 10 | 0.001278s | 0.001289s | 0.001268s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001322s | 0.001282s | 0.001294s | byte equality observed |
| optional_test_io.test | 10 | 0.001290s | 0.001333s | 0.001285s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001273s | 0.001281s | 0.001280s | byte equality observed |
| optional_test_map.test | 10 | 0.001279s | 0.001298s | 0.001277s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001280s | 0.001344s | 0.001263s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001299s | 0.001304s | 0.001315s | byte equality observed |
| optional_test_move.test | 8 | 0.001243s | 0.001284s | 0.001303s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001302s | 0.001298s | 0.001290s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000594s | 0.000599s | 0.001307s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001291s | 0.001227s | 0.001282s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001260s | 0.001233s | 0.001302s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001287s | 0.001240s | 0.001284s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001314s | 0.001226s | 0.001327s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001279s | 0.001233s | 0.001317s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001309s | 0.001204s | 0.001375s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001305s | 0.001264s | 0.001273s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001304s | 0.001293s | 0.001312s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001304s | 0.001292s | 0.001245s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001348s | 0.001316s | 0.001279s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001302s | 0.001267s | 0.001316s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001274s | 0.001307s | 0.001243s | byte equality observed |
| optional_test_swap.test | 10 | 0.001296s | 0.001262s | 0.001283s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001284s | 0.001248s | 0.001293s | byte equality observed |
| optional_test_tie.test | 10 | 0.001239s | 0.001277s | 0.001278s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001316s | 0.001303s | 0.001344s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001270s | 0.001324s | 0.001277s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001284s | 0.001229s | 0.001254s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001256s | 0.001267s | 0.001234s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
