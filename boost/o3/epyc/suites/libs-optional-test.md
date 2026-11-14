# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.182651 | 1.059901 | 0.609787 | 0.001279 | 0.001692 | 0.001356 |
| optional_test_assign.test | RUN | 0.098438 | 0.230844 | 0.285714 | 0.001482 | 0.001592 | 0.001373 |
| optional_test_swap.test | RUN | 0.105532 | 0.290045 | 0.299625 | 0.001192 | 0.001493 | 0.001327 |
| optional_test_constexpr.test | COMPILE | 0.069005 | 0.153892 | 0.180529 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.105290 | 0.373275 | 0.291292 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.067614 | 0.156863 | 0.180078 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.108178 | 0.296782 | 0.345033 | 0.001590 | 0.001502 | 0.001472 |
| optional_test_constructors.test | RUN | 0.214527 | 1.059976 | 0.878926 | 0.001106 | 0.001384 | 0.001461 |
| optional_test_convert_from_T.test | RUN | 0.102300 | 0.245987 | 0.302365 | 0.001494 | 0.001553 | 0.001386 |
| optional_test_convert_assign.test | RUN | 0.102306 | 0.249107 | 0.290159 | 0.001406 | 0.001417 | 0.001460 |
| optional_test_empty_braces.test | RUN | 0.100297 | 0.230777 | 0.297306 | 0.001441 | 0.001500 | 0.001449 |
| optional_test_make_optional.test | RUN | 0.139288 | 0.363323 | 0.468955 | 0.001573 | 0.001348 | 0.001427 |
| optional_test_flat_map.test | RUN | 0.148340 | 0.424992 | 0.522418 | 0.001395 | 0.001321 | 0.001436 |
| optional_test_hash.test | RUN | 0.124985 | 0.349752 | 0.372995 | 0.001348 | 0.001439 | 0.001498 |
| optional_test_map.test | RUN | 0.153518 | 0.409448 | 0.534090 | 0.001611 | 0.001445 | 0.001394 |
| optional_test_tie.test | RUN | 0.110731 | 0.266730 | 0.311527 | 0.001662 | 0.001488 | 0.001438 |
| optional_test_ranges_find.test | RUN | 0.104223 | 0.235376 | 0.273004 | 0.001320 | 0.001287 | 0.001340 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.126094 | 0.660574 | 0.404928 | 0.001434 | 0.001408 | 0.001476 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.105501 | 0.290984 | 0.313028 | 0.001400 | 0.001556 | 0.001355 |
| optional_test_ref_assign_const_int.test | RUN | 0.109091 | 0.292691 | 0.300266 | 0.001661 | 0.001213 | 0.001357 |
| optional_test_ref_converting_ctor.test | RUN | 0.119787 | 0.396317 | 0.338757 | 0.001366 | 0.001484 | 0.001470 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.102458 | 0.270219 | 0.297669 | 0.001320 | 0.001426 | 0.001484 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.103777 | 0.254194 | 0.275345 | 0.001606 | 0.001375 | 0.001380 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.103882 | 0.240902 | 0.293636 | 0.001473 | 0.001291 | 0.001463 |
| optional_test_ref_portable_minimum.test | RUN | 0.414614 | 1.300687 | 1.009388 | 0.001472 | 0.001558 | 0.001388 |
| optional_test_ref_move.test | RUN | 0.110946 | 0.256925 | 0.303859 | 0.001364 | 0.001586 | 0.001541 |
| optional_test_ref_to_val.test | RUN | 0.130952 | 0.523959 | 0.410327 | 0.001589 | 0.001424 | 0.001506 |
| optional_test_inplace_factory.test | RUN | 0.131468 | 0.471576 | 0.390138 | 0.001252 | 0.001674 | 0.001476 |
| optional_test_io.test | RUN | 0.130060 | 0.531052 | 0.398241 | 0.001498 | 0.001460 | 0.001514 |
| optional_test_move.test | RUN | 0.121517 | 0.355752 | 0.352585 | 0.001451 | 0.001366 | 0.001505 |
| optional_test_noexcept_move.test | RUN | 0.072115 | 0.160503 | 0.189308 | 0.000522 | 0.000530 | 0.001354 |
| optional_test_cmp_none.test | RUN | 0.103945 | 0.251157 | 0.292587 | 0.001260 | 0.001598 | 0.001455 |
| optional_test_value_access.test | RUN | 0.125110 | 0.429243 | 0.382184 | 0.001469 | 0.001406 | 0.001614 |
| optional_test_emplace.test | RUN | 0.124134 | 0.422574 | 0.399762 | 0.001424 | 0.001446 | 0.001590 |
| optional_test_minimum_requirements.test | RUN | 0.115006 | 0.327650 | 0.353368 | 0.001574 | 0.001453 | 0.001417 |
| optional_test_msvc_bug_workaround.test | RUN | 0.100425 | 0.232844 | 0.272233 | 0.001374 | 0.001561 | 0.001462 |
| optional_test_member_T.test | RUN | 0.102235 | 0.224068 | 0.271436 | 0.001225 | 0.001383 | 0.001340 |
| optional_test_tc_base.test | RUN | 0.101033 | 0.228301 | 0.278601 | 0.001668 | 0.001367 | 0.001369 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.075654 | 0.160417 | 0.188392 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.071928 | 0.159227 | 0.175720 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.067166 | 0.143786 | 0.169332 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.091888 | 0.194876 | 0.252557 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.066960 | 0.141921 | 0.162838 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.066769 | 0.143532 | 0.166594 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.068777 | 0.148280 | 0.170636 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.066725 | 0.143531 | 0.161522 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.066213 | 0.144551 | 0.157945 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.066301 | 0.139964 | 0.165954 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.065591 | 0.145969 | 0.164673 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.108380 | 0.225954 | 0.281645 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.106284 | 0.224572 | 0.282276 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.064853 | 0.145179 | 0.163571 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.068516 | 0.149118 | 0.168560 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.065951 | 0.139946 | 0.152221 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.068297 | 0.144486 | 0.170578 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.067538 | 0.145421 | 0.165575 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.069427 | 0.144072 | 0.163388 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.070104 | 0.140883 | 0.165977 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.068956 | 0.143854 | 0.161483 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.068317 | 0.145501 | 0.160830 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.068812 | 0.145447 | 0.159574 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.098045 | 0.185376 | 0.245031 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.071017 | 0.157092 | 0.201677 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.069669 | 0.143269 | 0.170482 | — | — | — |
| optional_test_static_properties.test | RUN | 0.157349 | 0.328248 | 0.415011 | 0.001503 | 0.001494 | 0.001452 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.068122 | 0.156503 | 0.178693 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.071027 | 0.161395 | 0.185119 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.069386 | 0.151599 | 0.171953 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.084933 | 0.193484 | 0.243046 | 0.001173 | 0.001318 | 0.001010 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.084168 | 0.198629 | 0.244384 | 0.001480 | 0.001387 | 0.001165 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.088311 | 0.190691 | 0.225484 | 0.001335 | 0.001177 | 0.001402 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.080944 | 0.204021 | 0.228068 | 0.003052 | 0.001170 | 0.001492 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.061470 | 0.167955 | 0.171933 | 0.001149 | 0.001087 | 0.001000 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.047953 | 0.187344 | 0.144691 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002×† | 1.005×, 1.000× | 0.910× |
| clang | 1.007× | 0.997×, 1.017× | 1.011× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.000949s | 0.000916s | 0.000918s | byte equality observed |
| optional_test_assign.test | 10 | 0.000855s | 0.000847s | 0.000854s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000865s | 0.000862s | 0.000860s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000871s | 0.000854s | 0.000867s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000858s | 0.000857s | 0.000851s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000854s | 0.000862s | 0.000870s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000859s | 0.000875s | 0.000856s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000873s | 0.000881s | 0.000879s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000854s | 0.000866s | 0.000859s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000851s | 0.000861s | 0.000868s | byte equality observed |
| optional_test_hash.test | 10 | 0.000853s | 0.000886s | 0.000865s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000866s | 0.000877s | 0.000865s | byte equality observed |
| optional_test_io.test | 10 | 0.000873s | 0.000896s | 0.000871s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000860s | 0.000848s | 0.000868s | byte equality observed |
| optional_test_map.test | 10 | 0.000860s | 0.000838s | 0.000857s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000862s | 0.000879s | 0.000900s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000899s | 0.000882s | 0.000885s | byte equality observed |
| optional_test_move.test | 10 | 0.000862s | 0.000854s | 0.000874s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000858s | 0.000862s | 0.000866s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000397s | 0.000386s | 0.000870s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000850s | 0.000851s | 0.000865s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000858s | 0.000876s | 0.000848s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000858s | 0.000850s | 0.000856s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000863s | 0.000862s | 0.000847s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000859s | 0.000850s | 0.000841s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000865s | 0.000846s | 0.000867s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000868s | 0.000844s | 0.000846s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000856s | 0.000845s | 0.000858s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000872s | 0.000871s | 0.000856s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.000905s | 0.000888s | 0.000909s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000855s | 0.000862s | 0.000865s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000862s | 0.000849s | 0.000839s | byte equality observed |
| optional_test_swap.test | 10 | 0.000858s | 0.000864s | 0.000839s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000862s | 0.000862s | 0.000842s | byte equality observed |
| optional_test_tie.test | 10 | 0.000857s | 0.000875s | 0.000834s | byte equality observed |
| optional_test_value_access.test | 10 | 0.000892s | 0.000902s | 0.000901s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000837s | 0.000838s | 0.000836s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000856s | 0.000855s | 0.000844s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000861s | 0.000869s | 0.000839s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
