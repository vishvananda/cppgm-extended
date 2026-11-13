# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.173265 | 1.064935 | 0.612255 | 0.001369 | 0.001570 | 0.001586 |
| optional_test_assign.test | RUN | 0.093847 | 0.227611 | 0.279873 | 0.001234 | 0.001507 | 0.001502 |
| optional_test_swap.test | RUN | 0.098527 | 0.281886 | 0.302008 | 0.001498 | 0.001451 | 0.001304 |
| optional_test_constexpr.test | COMPILE | 0.066404 | 0.153225 | 0.173677 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.098308 | 0.380715 | 0.289519 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.065737 | 0.151455 | 0.174909 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.105826 | 0.295670 | 0.348820 | 0.001448 | 0.001570 | 0.001452 |
| optional_test_constructors.test | RUN | 0.209552 | 1.056771 | 0.891010 | 0.001180 | 0.001420 | 0.001417 |
| optional_test_convert_from_T.test | RUN | 0.099224 | 0.242826 | 0.300778 | 0.001552 | 0.001526 | 0.001282 |
| optional_test_convert_assign.test | RUN | 0.097247 | 0.246654 | 0.285829 | 0.001429 | 0.001615 | 0.001223 |
| optional_test_empty_braces.test | RUN | 0.095127 | 0.229835 | 0.304366 | 0.001535 | 0.001586 | 0.001453 |
| optional_test_make_optional.test | RUN | 0.135491 | 0.357909 | 0.431950 | 0.001528 | 0.001393 | 0.001435 |
| optional_test_flat_map.test | RUN | 0.140010 | 0.421704 | 0.507157 | 0.001258 | 0.001396 | 0.001374 |
| optional_test_hash.test | RUN | 0.115825 | 0.350489 | 0.379648 | 0.001386 | 0.001415 | 0.001441 |
| optional_test_map.test | RUN | 0.148288 | 0.403749 | 0.538990 | 0.001348 | 0.001282 | 0.001314 |
| optional_test_tie.test | RUN | 0.104043 | 0.272837 | 0.283731 | 0.001360 | 0.001511 | 0.001516 |
| optional_test_ranges_find.test | RUN | 0.094367 | 0.221509 | 0.283976 | 0.001311 | 0.001379 | 0.001159 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.117395 | 0.658773 | 0.405324 | 0.001354 | 0.001345 | 0.001302 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.103516 | 0.285220 | 0.306863 | 0.001412 | 0.001362 | 0.001364 |
| optional_test_ref_assign_const_int.test | RUN | 0.099849 | 0.291474 | 0.315171 | 0.001226 | 0.001292 | 0.001442 |
| optional_test_ref_converting_ctor.test | RUN | 0.109534 | 0.396614 | 0.341158 | 0.001330 | 0.001519 | 0.001216 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.104287 | 0.269467 | 0.312105 | 0.001346 | 0.001374 | 0.001429 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.095489 | 0.247516 | 0.289781 | 0.001413 | 0.001254 | 0.001218 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.094843 | 0.247654 | 0.283410 | 0.001280 | 0.001393 | 0.001440 |
| optional_test_ref_portable_minimum.test | RUN | 0.393364 | 1.300001 | 1.004287 | 0.001342 | 0.001485 | 0.001482 |
| optional_test_ref_move.test | RUN | 0.099748 | 0.257783 | 0.305341 | 0.001360 | 0.001570 | 0.001456 |
| optional_test_ref_to_val.test | RUN | 0.119972 | 0.515266 | 0.418317 | 0.001459 | 0.001360 | 0.001379 |
| optional_test_inplace_factory.test | RUN | 0.123080 | 0.470675 | 0.380543 | 0.001196 | 0.001228 | 0.001449 |
| optional_test_io.test | RUN | 0.123142 | 0.531819 | 0.395722 | 0.001402 | 0.001529 | 0.001419 |
| optional_test_move.test | RUN | 0.110858 | 0.353161 | 0.332189 | 0.001388 | 0.001320 | 0.001333 |
| optional_test_noexcept_move.test | RUN | 0.068473 | 0.159593 | 0.184263 | 0.000503 | 0.000559 | 0.001399 |
| optional_test_cmp_none.test | RUN | 0.097461 | 0.247553 | 0.276392 | 0.001355 | 0.001504 | 0.001455 |
| optional_test_value_access.test | RUN | 0.115896 | 0.425883 | 0.392177 | 0.001628 | 0.001350 | 0.001591 |
| optional_test_emplace.test | RUN | 0.112027 | 0.411725 | 0.387720 | 0.001135 | 0.001450 | 0.001088 |
| optional_test_minimum_requirements.test | RUN | 0.108989 | 0.339923 | 0.334081 | 0.001492 | 0.001521 | 0.001361 |
| optional_test_msvc_bug_workaround.test | RUN | 0.099284 | 0.233696 | 0.283051 | 0.001202 | 0.001532 | 0.001445 |
| optional_test_member_T.test | RUN | 0.094266 | 0.222356 | 0.285687 | 0.001469 | 0.001315 | 0.001398 |
| optional_test_tc_base.test | RUN | 0.096859 | 0.229543 | 0.289109 | 0.001234 | 0.001338 | 0.001201 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.072896 | 0.158454 | 0.172360 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.068809 | 0.158385 | 0.170017 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.062594 | 0.142894 | 0.158984 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.085249 | 0.195052 | 0.251115 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.062501 | 0.141351 | 0.155962 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.062809 | 0.144820 | 0.166281 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.064369 | 0.143499 | 0.173086 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.062650 | 0.140431 | 0.158427 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.061046 | 0.139087 | 0.160473 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.063453 | 0.140145 | 0.164432 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.062783 | 0.143434 | 0.161641 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.102170 | 0.216466 | 0.272469 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.100908 | 0.216985 | 0.282305 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.062990 | 0.140431 | 0.161664 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.064136 | 0.144824 | 0.166906 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.061449 | 0.140053 | 0.160092 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.063990 | 0.136418 | 0.165241 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.063021 | 0.135979 | 0.163161 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.063463 | 0.142107 | 0.163096 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.062752 | 0.142409 | 0.165038 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.062432 | 0.144572 | 0.162886 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.061821 | 0.141445 | 0.161195 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.062282 | 0.145121 | 0.163368 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.085427 | 0.183218 | 0.239551 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.068388 | 0.157331 | 0.199611 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.063536 | 0.142380 | 0.166381 | — | — | — |
| optional_test_static_properties.test | RUN | 0.149196 | 0.326260 | 0.416903 | 0.001335 | 0.001524 | 0.001458 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.066063 | 0.156710 | 0.178706 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.067284 | 0.162713 | 0.180648 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.068170 | 0.150244 | 0.168426 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.077708 | 0.190786 | 0.238056 | 0.001518 | 0.001383 | 0.001247 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.076052 | 0.195334 | 0.240477 | 0.001255 | 0.001252 | 0.001252 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.078272 | 0.189883 | 0.225709 | 0.001275 | 0.001197 | 0.001027 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.080904 | 0.199618 | 0.233900 | 0.001221 | 0.001177 | 0.001443 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.062767 | 0.142768 | 0.177057 | 0.001053 | 0.001063 | 0.000999 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.040906 | 0.133588 | 0.141462 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.982× | 0.991×, 0.973× | 1.003× |
| clang | 1.010× | 1.006×, 1.015× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.000901s | 0.000920s | 0.000894s | byte equality observed |
| optional_test_assign.test | 10 | 0.000839s | 0.000841s | 0.000839s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.000844s | 0.000845s | 0.000850s | byte equality observed |
| optional_test_constructors.test | 10 | 0.000862s | 0.000862s | 0.000858s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.000840s | 0.000840s | 0.000861s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.000863s | 0.000828s | 0.000848s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.000846s | 0.000869s | 0.000851s | byte equality observed |
| optional_test_emplace.test | 10 | 0.000870s | 0.000865s | 0.000869s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.000845s | 0.000838s | 0.000842s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.000855s | 0.000845s | 0.000848s | byte equality observed |
| optional_test_hash.test | 10 | 0.000861s | 0.000856s | 0.000854s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.000870s | 0.000862s | 0.000846s | byte equality observed |
| optional_test_io.test | 10 | 0.000881s | 0.000886s | 0.000871s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.000844s | 0.000848s | 0.000853s | byte equality observed |
| optional_test_map.test | 10 | 0.000857s | 0.000853s | 0.000849s | byte equality observed |
| optional_test_member_T.test | 10 | 0.000849s | 0.000833s | 0.000837s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.000885s | 0.000877s | 0.000863s | byte equality observed |
| optional_test_move.test | 10 | 0.000856s | 0.000846s | 0.000842s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.000853s | 0.000859s | 0.000837s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000392s | 0.000386s | 0.000824s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.000865s | 0.000839s | 0.000843s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.000863s | 0.000842s | 0.000856s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.000855s | 0.000853s | 0.000850s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.000851s | 0.000845s | 0.000849s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.000847s | 0.000838s | 0.000843s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.000866s | 0.000836s | 0.000843s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.000856s | 0.000857s | 0.000861s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.000856s | 0.000843s | 0.000856s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.000855s | 0.000851s | 0.000853s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.000892s | 0.000872s | 0.000881s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.000863s | 0.000858s | 0.000839s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.000848s | 0.000836s | 0.000850s | byte equality observed |
| optional_test_swap.test | 10 | 0.000874s | 0.000860s | 0.000860s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.000847s | 0.000855s | 0.000839s | byte equality observed |
| optional_test_tie.test | 10 | 0.000861s | 0.000854s | 0.000860s | byte equality observed |
| optional_test_value_access.test | 10 | 0.000901s | 0.000877s | 0.000881s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.000829s | 0.000842s | 0.000846s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.000851s | 0.000843s | 0.000847s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.000842s | 0.000832s | 0.000851s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
