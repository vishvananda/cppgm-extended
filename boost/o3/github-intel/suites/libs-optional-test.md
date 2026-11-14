# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.209879 | 1.305425 | 0.756900 | 0.001135 | 0.001425 | 0.001424 |
| optional_test_assign.test | RUN | 0.102954 | 0.272008 | 0.344260 | 0.001208 | 0.001220 | 0.001249 |
| optional_test_swap.test | RUN | 0.111556 | 0.333545 | 0.375443 | 0.001134 | 0.001146 | 0.001175 |
| optional_test_constexpr.test | COMPILE | 0.073286 | 0.180903 | 0.213908 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.106174 | 0.443209 | 0.353292 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.069543 | 0.173518 | 0.214024 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.118781 | 0.343940 | 0.407506 | 0.001138 | 0.001252 | 0.001273 |
| optional_test_constructors.test | RUN | 0.249817 | 1.310489 | 1.075493 | 0.001102 | 0.001144 | 0.001323 |
| optional_test_convert_from_T.test | RUN | 0.107067 | 0.288978 | 0.365591 | 0.001194 | 0.001332 | 0.001096 |
| optional_test_convert_assign.test | RUN | 0.111782 | 0.276003 | 0.346789 | 0.001074 | 0.001132 | 0.001230 |
| optional_test_empty_braces.test | RUN | 0.104806 | 0.272690 | 0.354095 | 0.001020 | 0.001229 | 0.001248 |
| optional_test_make_optional.test | RUN | 0.142147 | 0.426168 | 0.554033 | 0.001123 | 0.001225 | 0.001361 |
| optional_test_flat_map.test | RUN | 0.157964 | 0.497942 | 0.631101 | 0.000967 | 0.001211 | 0.001380 |
| optional_test_hash.test | RUN | 0.127855 | 0.401258 | 0.448814 | 0.001105 | 0.001193 | 0.001223 |
| optional_test_map.test | RUN | 0.155575 | 0.477823 | 0.646947 | 0.001103 | 0.001134 | 0.001291 |
| optional_test_tie.test | RUN | 0.105208 | 0.312752 | 0.367735 | 0.001064 | 0.001177 | 0.001227 |
| optional_test_ranges_find.test | RUN | 0.103891 | 0.257925 | 0.328214 | 0.001057 | 0.001256 | 0.001177 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.133242 | 0.788469 | 0.476015 | 0.001080 | 0.001255 | 0.001242 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.104797 | 0.333467 | 0.368709 | 0.001064 | 0.001339 | 0.001147 |
| optional_test_ref_assign_const_int.test | RUN | 0.105133 | 0.342314 | 0.375933 | 0.001024 | 0.001183 | 0.001280 |
| optional_test_ref_converting_ctor.test | RUN | 0.123358 | 0.485208 | 0.412121 | 0.001162 | 0.001118 | 0.001218 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.107312 | 0.316803 | 0.364447 | 0.001058 | 0.001178 | 0.001261 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.103508 | 0.290308 | 0.353578 | 0.001256 | 0.001111 | 0.001273 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.099443 | 0.280388 | 0.345328 | 0.001072 | 0.001107 | 0.001312 |
| optional_test_ref_portable_minimum.test | RUN | 0.519464 | 1.588589 | 1.263365 | 0.001066 | 0.001150 | 0.001184 |
| optional_test_ref_move.test | RUN | 0.112817 | 0.303403 | 0.368417 | 0.001014 | 0.001227 | 0.001434 |
| optional_test_ref_to_val.test | RUN | 0.130445 | 0.624623 | 0.516234 | 0.001123 | 0.001108 | 0.001380 |
| optional_test_inplace_factory.test | RUN | 0.134545 | 0.575025 | 0.475117 | 0.001060 | 0.001246 | 0.001292 |
| optional_test_io.test | RUN | 0.127095 | 0.625921 | 0.481578 | 0.001022 | 0.001297 | 0.001239 |
| optional_test_move.test | RUN | 0.119831 | 0.415655 | 0.427715 | 0.000969 | 0.001271 | 0.001416 |
| optional_test_noexcept_move.test | RUN | 0.073177 | 0.176638 | 0.226438 | 0.000536 | 0.000394 | 0.001243 |
| optional_test_cmp_none.test | RUN | 0.106011 | 0.289957 | 0.356118 | 0.001021 | 0.001180 | 0.001242 |
| optional_test_value_access.test | RUN | 0.121473 | 0.502563 | 0.468296 | 0.001091 | 0.001082 | 0.001382 |
| optional_test_emplace.test | RUN | 0.124187 | 0.486244 | 0.478637 | 0.001148 | 0.001316 | 0.001386 |
| optional_test_minimum_requirements.test | RUN | 0.119379 | 0.388043 | 0.418493 | 0.001107 | 0.001195 | 0.001224 |
| optional_test_msvc_bug_workaround.test | RUN | 0.104959 | 0.267016 | 0.339055 | 0.001045 | 0.001189 | 0.001261 |
| optional_test_member_T.test | RUN | 0.100050 | 0.258896 | 0.333079 | 0.001090 | 0.001126 | 0.001246 |
| optional_test_tc_base.test | RUN | 0.103833 | 0.259639 | 0.345214 | 0.001038 | 0.001145 | 0.001233 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.073630 | 0.182445 | 0.227294 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.069970 | 0.177525 | 0.207743 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.067870 | 0.160844 | 0.195330 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.092511 | 0.221680 | 0.305112 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.064545 | 0.162962 | 0.198787 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.067123 | 0.169579 | 0.203506 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.068985 | 0.164708 | 0.207637 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.067219 | 0.163126 | 0.202905 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.067815 | 0.162886 | 0.196056 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.069409 | 0.163998 | 0.201800 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.069387 | 0.161523 | 0.199812 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.116646 | 0.259546 | 0.345724 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.116974 | 0.258247 | 0.343139 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.068394 | 0.166696 | 0.198640 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.072540 | 0.168320 | 0.203464 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.067978 | 0.160908 | 0.196190 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.068416 | 0.163519 | 0.204517 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.068524 | 0.166435 | 0.194100 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.068705 | 0.161711 | 0.200826 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.067343 | 0.159150 | 0.201807 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.066413 | 0.159158 | 0.196228 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.067098 | 0.163552 | 0.198486 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.067619 | 0.165490 | 0.193782 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.093499 | 0.216020 | 0.294593 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.073977 | 0.181541 | 0.239495 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.069515 | 0.163207 | 0.203156 | — | — | — |
| optional_test_static_properties.test | RUN | 0.162152 | 0.378601 | 0.511725 | 0.001063 | 0.001254 | 0.001081 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.071383 | 0.180723 | 0.218153 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.072788 | 0.183999 | 0.220008 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.070782 | 0.171809 | 0.206245 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.081983 | 0.224142 | 0.281923 | 0.001084 | 0.001161 | 0.001199 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.083477 | 0.228999 | 0.285301 | 0.000833 | 0.001249 | 0.001109 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.079435 | 0.220560 | 0.276123 | 0.000983 | 0.000975 | 0.001101 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.085569 | 0.228616 | 0.278298 | 0.000798 | 0.000790 | 0.001317 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.066238 | 0.145636 | 0.198265 | 0.000724 | 0.000790 | 0.000826 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.047412 | 0.140922 | 0.163109 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 1.000×, 0.999× | 0.994× |
| clang | 1.016× | 1.018×, 1.015× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.000887s | 0.000855s | 0.000876s | byte equality observed |
| optional_test_assign.test | 10 | 0.000804s | 0.000810s | 0.000785s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000829s | 0.000820s | 0.000791s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000820s | 0.000820s | 0.000804s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000805s | 0.000821s | 0.000812s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000816s | 0.000809s | 0.000831s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000786s | 0.000818s | 0.000819s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000836s | 0.000820s | 0.000834s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000796s | 0.000820s | 0.000805s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000808s | 0.000784s | 0.000825s | byte equality observed |
| optional_test_hash.test | 10 | 0.000818s | 0.000815s | 0.000822s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000811s | 0.000822s | 0.000821s | byte equality observed |
| optional_test_io.test | 10 | 0.000825s | 0.000858s | 0.000820s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000827s | 0.000801s | 0.000843s | byte equality observed |
| optional_test_map.test | 10 | 0.000808s | 0.000809s | 0.000806s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000821s | 0.000807s | 0.000838s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000833s | 0.000824s | 0.000831s | byte equality observed |
| optional_test_move.test | 10 | 0.000802s | 0.000807s | 0.000855s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000803s | 0.000810s | 0.000827s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000368s | 0.000366s | 0.000801s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000811s | 0.000798s | 0.000814s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000784s | 0.000808s | 0.000800s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000829s | 0.000801s | 0.000851s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000804s | 0.000837s | 0.000875s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000829s | 0.000821s | 0.000815s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000816s | 0.000811s | 0.000808s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000819s | 0.000825s | 0.000811s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000809s | 0.000837s | 0.000832s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000828s | 0.000816s | 0.000813s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.000832s | 0.000837s | 0.000820s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000838s | 0.000811s | 0.000789s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000819s | 0.000795s | 0.000809s | byte equality observed |
| optional_test_swap.test | 10 | 0.000819s | 0.000798s | 0.000793s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000818s | 0.000819s | 0.000806s | byte equality observed |
| optional_test_tie.test | 10 | 0.000817s | 0.000808s | 0.000797s | byte equality observed |
| optional_test_value_access.test | 10 | 0.000830s | 0.000846s | 0.000852s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000810s | 0.000808s | 0.000800s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000784s | 0.000812s | 0.000800s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000787s | 0.000821s | 0.000785s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
