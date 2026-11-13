# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.238633 | 1.444638 | 0.926858 | 0.001748 | 0.001753 | 0.001827 |
| optional_test_assign.test | RUN | 0.128009 | 0.314681 | 0.434323 | 0.001727 | 0.001829 | 0.001794 |
| optional_test_swap.test | RUN | 0.134014 | 0.382384 | 0.492415 | 0.001434 | 0.001725 | 0.001587 |
| optional_test_constexpr.test | COMPILE | 0.090823 | 0.216077 | 0.288008 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.136366 | 0.507373 | 0.436043 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.089433 | 0.214445 | 0.269355 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.147081 | 0.402204 | 0.518208 | 0.001565 | 0.001729 | 0.001706 |
| optional_test_constructors.test | RUN | 0.297322 | 1.484813 | 1.332024 | 0.001459 | 0.001215 | 0.001726 |
| optional_test_convert_from_T.test | RUN | 0.137062 | 0.342903 | 0.448916 | 0.001678 | 0.001812 | 0.001667 |
| optional_test_convert_assign.test | RUN | 0.136056 | 0.316742 | 0.469528 | 0.001578 | 0.001653 | 0.001747 |
| optional_test_empty_braces.test | RUN | 0.131776 | 0.332319 | 0.398154 | 0.001730 | 0.001833 | 0.001727 |
| optional_test_make_optional.test | RUN | 0.177500 | 0.492367 | 0.673565 | 0.001764 | 0.001658 | 0.001752 |
| optional_test_flat_map.test | RUN | 0.195379 | 0.578345 | 0.762794 | 0.001704 | 0.001696 | 0.001771 |
| optional_test_hash.test | RUN | 0.162882 | 0.456069 | 0.591336 | 0.001510 | 0.001691 | 0.001590 |
| optional_test_map.test | RUN | 0.195443 | 0.550715 | 0.760460 | 0.001856 | 0.001609 | 0.001456 |
| optional_test_tie.test | RUN | 0.141443 | 0.355302 | 0.452240 | 0.001341 | 0.001528 | 0.001418 |
| optional_test_ranges_find.test | RUN | 0.126408 | 0.320877 | 0.425910 | 0.001629 | 0.001772 | 0.001614 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.163630 | 0.899402 | 0.598055 | 0.001540 | 0.001567 | 0.001810 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.138308 | 0.384813 | 0.484281 | 0.001706 | 0.001658 | 0.001726 |
| optional_test_ref_assign_const_int.test | RUN | 0.140079 | 0.396150 | 0.436164 | 0.001602 | 0.001594 | 0.001535 |
| optional_test_ref_converting_ctor.test | RUN | 0.147721 | 0.557073 | 0.518555 | 0.001617 | 0.001806 | 0.001700 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.134095 | 0.365865 | 0.431190 | 0.001461 | 0.001400 | 0.001699 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.133278 | 0.328325 | 0.457807 | 0.001706 | 0.001630 | 0.001498 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.135587 | 0.342381 | 0.454638 | 0.001420 | 0.001609 | 0.001407 |
| optional_test_ref_portable_minimum.test | RUN | 0.581990 | 1.788764 | 1.561807 | 0.001596 | 0.001645 | 0.001661 |
| optional_test_ref_move.test | RUN | 0.136781 | 0.355358 | 0.491154 | 0.001694 | 0.001810 | 0.001620 |
| optional_test_ref_to_val.test | RUN | 0.168631 | 0.713340 | 0.651074 | 0.001579 | 0.001668 | 0.001420 |
| optional_test_inplace_factory.test | RUN | 0.170065 | 0.655889 | 0.606982 | 0.001580 | 0.001476 | 0.001531 |
| optional_test_io.test | RUN | 0.160819 | 0.716514 | 0.585230 | 0.001680 | 0.001529 | 0.001605 |
| optional_test_move.test | RUN | 0.155821 | 0.489271 | 0.529684 | 0.001774 | 0.001687 | 0.001734 |
| optional_test_noexcept_move.test | RUN | 0.096092 | 0.216736 | 0.298963 | 0.000630 | 0.000763 | 0.001739 |
| optional_test_cmp_none.test | RUN | 0.133828 | 0.345782 | 0.452508 | 0.001764 | 0.001926 | 0.001747 |
| optional_test_value_access.test | RUN | 0.154914 | 0.582104 | 0.572231 | 0.001517 | 0.001521 | 0.001642 |
| optional_test_emplace.test | RUN | 0.155642 | 0.569331 | 0.584374 | 0.001507 | 0.001670 | 0.001608 |
| optional_test_minimum_requirements.test | RUN | 0.142594 | 0.435036 | 0.519061 | 0.001778 | 0.001591 | 0.001751 |
| optional_test_msvc_bug_workaround.test | RUN | 0.127600 | 0.319185 | 0.404545 | 0.001757 | 0.001682 | 0.001734 |
| optional_test_member_T.test | RUN | 0.128297 | 0.309546 | 0.407687 | 0.001586 | 0.001657 | 0.001568 |
| optional_test_tc_base.test | RUN | 0.128575 | 0.311477 | 0.408981 | 0.001565 | 0.001516 | 0.001616 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.097524 | 0.227356 | 0.296802 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.091655 | 0.219519 | 0.276307 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.085070 | 0.196502 | 0.243372 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.116627 | 0.276405 | 0.402395 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.084297 | 0.199806 | 0.270044 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.084617 | 0.205943 | 0.260958 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.087666 | 0.196634 | 0.268338 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.087517 | 0.198083 | 0.254958 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.085290 | 0.196587 | 0.266242 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.087784 | 0.199258 | 0.268777 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.084500 | 0.196732 | 0.253375 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.140894 | 0.316836 | 0.455124 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.141166 | 0.315427 | 0.452458 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.084693 | 0.197555 | 0.250096 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.086305 | 0.203708 | 0.260120 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.084070 | 0.193346 | 0.244077 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.085228 | 0.198291 | 0.267859 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.086352 | 0.194896 | 0.271337 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.087582 | 0.200182 | 0.248373 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.086056 | 0.194777 | 0.255407 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.084577 | 0.197349 | 0.269087 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.083260 | 0.201203 | 0.267078 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.086435 | 0.198676 | 0.254914 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.116751 | 0.264411 | 0.383125 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.091517 | 0.220856 | 0.320836 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.085458 | 0.193367 | 0.275964 | — | — | — |
| optional_test_static_properties.test | RUN | 0.202431 | 0.452160 | 0.662294 | 0.001756 | 0.001840 | 0.001739 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.089103 | 0.217441 | 0.291951 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.091868 | 0.220952 | 0.295502 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.086989 | 0.211941 | 0.280187 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.103772 | 0.261768 | 0.369034 | 0.001494 | 0.001596 | 0.001593 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.103647 | 0.270266 | 0.352721 | 0.001414 | 0.001371 | 0.001496 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.102445 | 0.263829 | 0.367909 | 0.001370 | 0.001504 | 0.001347 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.106851 | 0.272267 | 0.359733 | 0.001376 | 0.001360 | 0.001279 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.091236 | 0.194731 | 0.268879 | 0.001242 | 0.001272 | 0.001179 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.067583 | 0.183537 | 0.244672 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.994× | 0.994×, 0.994× | 0.952× |
| clang | 1.009× | 1.008×, 1.010× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001058s | 0.001053s | 0.001069s | byte equality observed |
| optional_test_assign.test | 10 | 0.000972s | 0.000990s | 0.001001s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001009s | 0.000978s | 0.000990s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001014s | 0.001004s | 0.001020s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001018s | 0.001010s | 0.001002s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000978s | 0.000998s | 0.000996s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001002s | 0.000968s | 0.001031s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001034s | 0.001051s | 0.001032s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000993s | 0.000972s | 0.001038s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000992s | 0.000981s | 0.001019s | byte equality observed |
| optional_test_hash.test | 10 | 0.000997s | 0.001004s | 0.001019s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001012s | 0.001019s | 0.001009s | byte equality observed |
| optional_test_io.test | 10 | 0.001044s | 0.000995s | 0.001012s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000995s | 0.000979s | 0.000978s | byte equality observed |
| optional_test_map.test | 10 | 0.000979s | 0.000984s | 0.000989s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000969s | 0.000980s | 0.001012s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001017s | 0.001007s | 0.001027s | byte equality observed |
| optional_test_move.test | 10 | 0.000986s | 0.000989s | 0.000987s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001008s | 0.001014s | 0.000987s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000464s | 0.000461s | 0.000986s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001006s | 0.000994s | 0.000974s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000994s | 0.000977s | 0.000989s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001004s | 0.000986s | 0.001012s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001028s | 0.001011s | 0.001003s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001015s | 0.000992s | 0.001010s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001019s | 0.000976s | 0.001011s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000993s | 0.000989s | 0.000980s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001035s | 0.001008s | 0.000991s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000981s | 0.000999s | 0.000984s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001036s | 0.001003s | 0.001024s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001004s | 0.001003s | 0.000999s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000985s | 0.000988s | 0.000989s | byte equality observed |
| optional_test_swap.test | 10 | 0.000998s | 0.000991s | 0.000990s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000986s | 0.000971s | 0.000997s | byte equality observed |
| optional_test_tie.test | 10 | 0.000992s | 0.000990s | 0.000977s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001041s | 0.001021s | 0.001028s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000979s | 0.000985s | 0.000972s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000994s | 0.000992s | 0.000995s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000996s | 0.000998s | 0.000981s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
