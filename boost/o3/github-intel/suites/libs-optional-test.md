# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.252878 | 1.534084 | 0.867207 | 0.001331 | 0.001536 | 0.001381 |
| optional_test_assign.test | RUN | 0.130271 | 0.318683 | 0.382349 | 0.001309 | 0.001412 | 0.001344 |
| optional_test_swap.test | RUN | 0.136942 | 0.397698 | 0.431093 | 0.001293 | 0.001326 | 0.001219 |
| optional_test_constexpr.test | COMPILE | 0.089450 | 0.213894 | 0.245745 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.132097 | 0.534000 | 0.399717 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.087993 | 0.210099 | 0.236474 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.145335 | 0.414339 | 0.460437 | 0.001162 | 0.001388 | 0.001307 |
| optional_test_constructors.test | RUN | 0.304152 | 1.529742 | 1.229411 | 0.001308 | 0.001301 | 0.001203 |
| optional_test_convert_from_T.test | RUN | 0.131787 | 0.336281 | 0.422323 | 0.001515 | 0.001347 | 0.001220 |
| optional_test_convert_assign.test | RUN | 0.133233 | 0.330417 | 0.382914 | 0.001276 | 0.001354 | 0.001224 |
| optional_test_empty_braces.test | RUN | 0.128679 | 0.319395 | 0.389006 | 0.001411 | 0.001434 | 0.001258 |
| optional_test_make_optional.test | RUN | 0.180604 | 0.503304 | 0.621631 | 0.001431 | 0.001385 | 0.001140 |
| optional_test_flat_map.test | RUN | 0.192945 | 0.587148 | 0.696543 | 0.001328 | 0.001445 | 0.001150 |
| optional_test_hash.test | RUN | 0.160363 | 0.479553 | 0.524153 | 0.001266 | 0.001387 | 0.001194 |
| optional_test_map.test | RUN | 0.198392 | 0.568281 | 0.719423 | 0.001352 | 0.001370 | 0.001098 |
| optional_test_tie.test | RUN | 0.137010 | 0.366125 | 0.410562 | 0.001278 | 0.001263 | 0.001191 |
| optional_test_ranges_find.test | RUN | 0.124937 | 0.315648 | 0.361395 | 0.001450 | 0.001259 | 0.001276 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.163337 | 0.943852 | 0.557397 | 0.001209 | 0.001392 | 0.001191 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.133998 | 0.395639 | 0.408295 | 0.001277 | 0.001446 | 0.001176 |
| optional_test_ref_assign_const_int.test | RUN | 0.135694 | 0.411732 | 0.419971 | 0.001430 | 0.001304 | 0.001283 |
| optional_test_ref_converting_ctor.test | RUN | 0.146955 | 0.574969 | 0.478911 | 0.001332 | 0.001423 | 0.001198 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.132086 | 0.376901 | 0.429052 | 0.001211 | 0.001291 | 0.000906 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.130820 | 0.347818 | 0.391130 | 0.001285 | 0.001447 | 0.001171 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.131073 | 0.347005 | 0.388169 | 0.001332 | 0.001222 | 0.001151 |
| optional_test_ref_portable_minimum.test | RUN | 0.612999 | 1.895196 | 1.444124 | 0.001304 | 0.001422 | 0.001163 |
| optional_test_ref_move.test | RUN | 0.136567 | 0.359252 | 0.401960 | 0.001311 | 0.001423 | 0.001271 |
| optional_test_ref_to_val.test | RUN | 0.159535 | 0.763630 | 0.573340 | 0.001294 | 0.001194 | 0.001227 |
| optional_test_inplace_factory.test | RUN | 0.168970 | 0.695139 | 0.534821 | 0.001316 | 0.001354 | 0.001199 |
| optional_test_io.test | RUN | 0.163159 | 0.748503 | 0.557903 | 0.001363 | 0.001419 | 0.001285 |
| optional_test_move.test | RUN | 0.153412 | 0.497505 | 0.482033 | 0.001290 | 0.001344 | 0.001175 |
| optional_test_noexcept_move.test | RUN | 0.090780 | 0.216332 | 0.234378 | 0.000513 | 0.000526 | 0.001125 |
| optional_test_cmp_none.test | RUN | 0.131533 | 0.353833 | 0.389412 | 0.001350 | 0.001393 | 0.001254 |
| optional_test_value_access.test | RUN | 0.157395 | 0.591410 | 0.529762 | 0.001414 | 0.001383 | 0.001258 |
| optional_test_emplace.test | RUN | 0.154321 | 0.580751 | 0.537082 | 0.000951 | 0.001512 | 0.001232 |
| optional_test_minimum_requirements.test | RUN | 0.145258 | 0.463716 | 0.464804 | 0.001364 | 0.001320 | 0.001216 |
| optional_test_msvc_bug_workaround.test | RUN | 0.129233 | 0.317644 | 0.389036 | 0.001265 | 0.001247 | 0.001230 |
| optional_test_member_T.test | RUN | 0.128065 | 0.316761 | 0.378605 | 0.001547 | 0.001250 | 0.001160 |
| optional_test_tc_base.test | RUN | 0.128993 | 0.320649 | 0.378813 | 0.001247 | 0.001388 | 0.001190 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.096548 | 0.217517 | 0.250607 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.096288 | 0.214181 | 0.236871 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.087957 | 0.196236 | 0.212875 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.115803 | 0.275220 | 0.349006 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.083768 | 0.202511 | 0.220797 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.085783 | 0.200582 | 0.225362 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.088159 | 0.203651 | 0.227277 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.089143 | 0.197547 | 0.221467 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.088442 | 0.197778 | 0.222086 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.085593 | 0.195757 | 0.226420 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.086400 | 0.200467 | 0.216938 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.141325 | 0.318893 | 0.390560 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.141109 | 0.317003 | 0.386487 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.085455 | 0.199144 | 0.220963 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.085819 | 0.205787 | 0.225850 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.083087 | 0.202157 | 0.216957 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.085377 | 0.204706 | 0.220661 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.084823 | 0.198477 | 0.221016 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.084363 | 0.203691 | 0.210380 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.085374 | 0.197268 | 0.223444 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.084197 | 0.197873 | 0.217525 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.083119 | 0.199018 | 0.218704 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.083045 | 0.200535 | 0.215365 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.115287 | 0.256875 | 0.329684 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.092084 | 0.216396 | 0.272254 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.085477 | 0.195518 | 0.229478 | — | — | — |
| optional_test_static_properties.test | RUN | 0.196663 | 0.454014 | 0.577683 | 0.001347 | 0.001381 | 0.001280 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.088436 | 0.215758 | 0.246675 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.089795 | 0.218933 | 0.249755 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.087001 | 0.211850 | 0.233816 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.105288 | 0.268789 | 0.327932 | 0.001273 | 0.001309 | 0.001221 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.103802 | 0.272406 | 0.325798 | 0.001416 | 0.001255 | 0.001286 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.104069 | 0.266763 | 0.316601 | 0.001242 | 0.001064 | 0.001140 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.107598 | 0.274483 | 0.321445 | 0.000958 | 0.001523 | 0.000893 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.082956 | 0.177617 | 0.230290 | 0.000995 | 0.000968 | 0.000869 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.059118 | 0.168048 | 0.222176 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.994×, 0.988× | 1.015× |
| clang | 1.016×† | 1.027×, 1.006× | 0.948× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.000942s | 0.000916s | 0.000951s | byte equality observed |
| optional_test_assign.test | 10 | 0.000877s | 0.000888s | 0.000888s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000856s | 0.000884s | 0.000893s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000897s | 0.000885s | 0.000872s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000859s | 0.000878s | 0.000877s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000866s | 0.000898s | 0.000882s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000917s | 0.000867s | 0.000905s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000897s | 0.000927s | 0.000941s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000879s | 0.000919s | 0.000891s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000888s | 0.000870s | 0.000916s | byte equality observed |
| optional_test_hash.test | 10 | 0.000863s | 0.000869s | 0.000917s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000885s | 0.000893s | 0.000883s | byte equality observed |
| optional_test_io.test | 10 | 0.000901s | 0.000906s | 0.000876s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000900s | 0.000860s | 0.000862s | byte equality observed |
| optional_test_map.test | 10 | 0.000894s | 0.000866s | 0.000889s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000859s | 0.000899s | 0.000874s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000885s | 0.000903s | 0.000935s | byte equality observed |
| optional_test_move.test | 10 | 0.000886s | 0.000858s | 0.000896s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000920s | 0.000898s | 0.000852s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000392s | 0.000380s | 0.000870s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000869s | 0.000864s | 0.000869s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000860s | 0.000878s | 0.000918s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000871s | 0.000877s | 0.000886s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000894s | 0.000907s | 0.000873s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000881s | 0.000907s | 0.000919s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000871s | 0.000862s | 0.000906s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000912s | 0.000872s | 0.000920s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000889s | 0.000853s | 0.000900s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000924s | 0.000872s | 0.000892s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.000904s | 0.000913s | 0.000903s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000908s | 0.000897s | 0.000870s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000884s | 0.000871s | 0.000892s | byte equality observed |
| optional_test_swap.test | 10 | 0.000862s | 0.000867s | 0.000904s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000927s | 0.000922s | 0.000856s | byte equality observed |
| optional_test_tie.test | 10 | 0.000953s | 0.000877s | 0.000878s | byte equality observed |
| optional_test_value_access.test | 10 | 0.000925s | 0.000893s | 0.000901s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000886s | 0.000863s | 0.000897s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000888s | 0.000850s | 0.000875s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000896s | 0.000887s | 0.000889s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
