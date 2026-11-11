# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.199060 | 1.062141 | 0.670836 | 0.001337 | 0.001678 | 0.001651 |
| optional_test_assign.test | RUN | 0.106259 | 0.227158 | 0.300204 | 0.001475 | 0.001582 | 0.001435 |
| optional_test_swap.test | RUN | 0.115738 | 0.282522 | 0.329661 | 0.001196 | 0.001495 | 0.001355 |
| optional_test_constexpr.test | COMPILE | 0.076140 | 0.154308 | 0.186498 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.115741 | 0.384749 | 0.336000 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.072311 | 0.150240 | 0.188907 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.119386 | 0.293948 | 0.380205 | 0.001478 | 0.001624 | 0.001604 |
| optional_test_constructors.test | RUN | 0.273657 | 1.053395 | 0.985789 | 0.001462 | 0.001445 | 0.001477 |
| optional_test_convert_from_T.test | RUN | 0.108891 | 0.244913 | 0.302165 | 0.001364 | 0.001474 | 0.001637 |
| optional_test_convert_assign.test | RUN | 0.113503 | 0.244948 | 0.330691 | 0.001510 | 0.001582 | 0.001640 |
| optional_test_empty_braces.test | RUN | 0.110718 | 0.229679 | 0.288427 | 0.001329 | 0.001628 | 0.001694 |
| optional_test_make_optional.test | RUN | 0.149089 | 0.356287 | 0.501485 | 0.001344 | 0.001472 | 0.001629 |
| optional_test_flat_map.test | RUN | 0.164318 | 0.417761 | 0.549088 | 0.001319 | 0.001497 | 0.001331 |
| optional_test_hash.test | RUN | 0.135506 | 0.339028 | 0.425169 | 0.001365 | 0.001504 | 0.001458 |
| optional_test_map.test | RUN | 0.167512 | 0.402945 | 0.562049 | 0.001394 | 0.001364 | 0.001386 |
| optional_test_tie.test | RUN | 0.115655 | 0.263564 | 0.348537 | 0.001423 | 0.001430 | 0.001469 |
| optional_test_ranges_find.test | RUN | 0.109493 | 0.229064 | 0.292602 | 0.001458 | 0.001359 | 0.001361 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.176663 | 0.664456 | 0.430848 | 0.001594 | 0.001341 | 0.001602 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.120566 | 0.279778 | 0.321624 | 0.001369 | 0.001218 | 0.001565 |
| optional_test_ref_assign_const_int.test | RUN | 0.119502 | 0.293360 | 0.335072 | 0.001391 | 0.001417 | 0.001646 |
| optional_test_ref_converting_ctor.test | RUN | 0.157013 | 0.400320 | 0.367273 | 0.001524 | 0.001528 | 0.001478 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.119829 | 0.268906 | 0.304161 | 0.001364 | 0.001459 | 0.001631 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.111293 | 0.249009 | 0.302597 | 0.001398 | 0.001532 | 0.001439 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.110961 | 0.248676 | 0.283859 | 0.001416 | 0.001356 | 0.001539 |
| optional_test_ref_portable_minimum.test | RUN | 0.390443 | 1.286560 | 1.074755 | 0.001600 | 0.001655 | 0.001742 |
| optional_test_ref_move.test | RUN | 0.123458 | 0.261567 | 0.330669 | 0.001533 | 0.001541 | 0.001564 |
| optional_test_ref_to_val.test | RUN | 0.161755 | 0.514383 | 0.442960 | 0.001407 | 0.001249 | 0.001508 |
| optional_test_inplace_factory.test | RUN | 0.145075 | 0.472604 | 0.418387 | 0.001327 | 0.001512 | 0.001537 |
| optional_test_io.test | RUN | 0.135541 | 0.536825 | 0.426707 | 0.001496 | 0.001470 | 0.001367 |
| optional_test_move.test | RUN | 0.130242 | 0.350806 | 0.363496 | 0.001356 | 0.001437 | 0.001579 |
| optional_test_noexcept_move.test | RUN | 0.078454 | 0.153654 | 0.187696 | 0.000530 | 0.000577 | 0.001492 |
| optional_test_cmp_none.test | RUN | 0.110336 | 0.248907 | 0.302690 | 0.001305 | 0.001657 | 0.001568 |
| optional_test_value_access.test | RUN | 0.131631 | 0.423454 | 0.400959 | 0.001487 | 0.001527 | 0.002395 |
| optional_test_emplace.test | RUN | 0.131922 | 0.421859 | 0.409925 | 0.001390 | 0.001345 | 0.001739 |
| optional_test_minimum_requirements.test | RUN | 0.121085 | 0.325619 | 0.372640 | 0.001331 | 0.001382 | 0.002476 |
| optional_test_msvc_bug_workaround.test | RUN | 0.109688 | 0.231078 | 0.404357 | 0.001500 | 0.001588 | 0.002820 |
| optional_test_member_T.test | RUN | 0.109137 | 0.221931 | 0.397396 | 0.001392 | 0.001440 | 0.001497 |
| optional_test_tc_base.test | RUN | 0.110194 | 0.228405 | 0.381996 | 0.001358 | 0.001244 | 0.001380 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.085575 | 0.161913 | 0.267091 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.080073 | 0.160542 | 0.208136 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.073320 | 0.142621 | 0.178258 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.096432 | 0.194969 | 0.277942 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.070504 | 0.143490 | 0.177870 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.072762 | 0.142303 | 0.177151 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.074000 | 0.146238 | 0.180808 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.074523 | 0.139216 | 0.178950 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.073620 | 0.139373 | 0.174097 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.074410 | 0.139208 | 0.174391 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.074420 | 0.145435 | 0.177304 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.120740 | 0.216829 | 0.298874 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.119543 | 0.216767 | 0.297527 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.071433 | 0.138309 | 0.177750 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.074379 | 0.146071 | 0.180873 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.072092 | 0.139679 | 0.166218 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.073250 | 0.141291 | 0.179723 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.072397 | 0.141059 | 0.178615 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.073490 | 0.141809 | 0.173140 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.073315 | 0.139845 | 0.174482 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.071865 | 0.141215 | 0.172284 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.071976 | 0.144420 | 0.174614 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.072753 | 0.142738 | 0.169591 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.097459 | 0.198687 | 0.255930 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.077968 | 0.195217 | 0.215077 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.074311 | 0.179125 | 0.173028 | — | — | — |
| optional_test_static_properties.test | RUN | 0.166296 | 0.397020 | 0.444898 | 0.001365 | 0.001382 | 0.001560 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.076793 | 0.156383 | 0.184974 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.078373 | 0.162319 | 0.192558 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.075363 | 0.152275 | 0.186494 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.087929 | 0.197357 | 0.250304 | 0.001423 | 0.001470 | 0.001343 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.087569 | 0.199907 | 0.243161 | 0.001179 | 0.001313 | 0.001356 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.086453 | 0.192957 | 0.252796 | 0.001231 | 0.001187 | 0.001302 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.092296 | 0.204105 | 0.244774 | 0.001352 | 0.001178 | 0.001239 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.069024 | 0.135946 | 0.241369 | 0.001061 | 0.001088 | 0.001136 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.044380 | 0.125334 | 0.218322 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.127×† | 1.089×, 1.164× | 0.891× |
| clang | 1.021× | 1.027×, 1.015× | 0.956× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001022s | 0.001025s | 0.001066s | byte equality observed |
| optional_test_assign.test | 10 | 0.000954s | 0.000944s | 0.000993s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000978s | 0.000970s | 0.001019s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000969s | 0.000972s | 0.001005s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000988s | 0.000968s | 0.001010s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000964s | 0.001012s | 0.000983s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000952s | 0.000976s | 0.000981s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000988s | 0.000980s | 0.001003s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000966s | 0.000963s | 0.000989s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000952s | 0.000972s | 0.001002s | byte equality observed |
| optional_test_hash.test | 10 | 0.000966s | 0.000992s | 0.001010s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000974s | 0.000971s | 0.001013s | byte equality observed |
| optional_test_io.test | 10 | 0.000973s | 0.000977s | 0.001022s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000947s | 0.000953s | 0.000993s | byte equality observed |
| optional_test_map.test | 10 | 0.000950s | 0.000953s | 0.000985s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000934s | 0.000940s | 0.000997s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000969s | 0.000978s | 0.001025s | byte equality observed |
| optional_test_move.test | 10 | 0.000957s | 0.000985s | 0.000991s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000962s | 0.000955s | 0.001016s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000441s | 0.000451s | 0.000981s | byte equality observed |
| optional_test_ranges_find.test | 9 | 0.000952s | 0.000958s | 0.000992s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000936s | 0.000979s | 0.001017s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000947s | 0.000960s | 0.000984s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000946s | 0.000973s | 0.001021s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000948s | 0.000965s | 0.001011s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000962s | 0.000981s | 0.000995s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000953s | 0.000956s | 0.000991s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000974s | 0.000983s | 0.000997s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000969s | 0.000993s | 0.001026s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001019s | 0.001012s | 0.001029s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000960s | 0.000957s | 0.001025s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000948s | 0.000951s | 0.001005s | byte equality observed |
| optional_test_swap.test | 10 | 0.000958s | 0.000949s | 0.001023s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000960s | 0.000990s | 0.001025s | byte equality observed |
| optional_test_tie.test | 10 | 0.000980s | 0.001210s | 0.000997s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001029s | 0.001265s | 0.001035s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000945s | 0.001196s | 0.001001s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000960s | 0.001252s | 0.001012s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000970s | 0.001119s | 0.001044s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
