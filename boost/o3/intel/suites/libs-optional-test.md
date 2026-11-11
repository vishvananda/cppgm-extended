# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.346835 | 1.944844 | 1.187820 | 0.001916 | 0.001961 | 0.002304 |
| optional_test_assign.test | RUN | 0.179424 | 0.416866 | 0.582566 | 0.001685 | 0.002164 | 0.001624 |
| optional_test_swap.test | RUN | 0.189262 | 0.520557 | 0.608072 | 0.002256 | 0.001890 | 0.001975 |
| optional_test_constexpr.test | COMPILE | 0.120720 | 0.286234 | 0.361643 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.183082 | 0.689929 | 0.609193 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.117454 | 0.279054 | 0.380188 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.202421 | 0.553009 | 0.677843 | 0.001726 | 0.001651 | 0.001890 |
| optional_test_constructors.test | RUN | 0.501356 | 2.019959 | 1.794888 | 0.001680 | 0.001678 | 0.001674 |
| optional_test_convert_from_T.test | RUN | 0.170514 | 0.440495 | 0.599687 | 0.001982 | 0.001384 | 0.001892 |
| optional_test_convert_assign.test | RUN | 0.168523 | 0.429950 | 0.601758 | 0.001688 | 0.001643 | 0.002302 |
| optional_test_empty_braces.test | RUN | 0.168795 | 0.440754 | 0.562931 | 0.001956 | 0.001666 | 0.001461 |
| optional_test_make_optional.test | RUN | 0.232368 | 0.682659 | 0.854400 | 0.001461 | 0.001674 | 0.001828 |
| optional_test_flat_map.test | RUN | 0.263898 | 0.787275 | 0.978923 | 0.001738 | 0.001786 | 0.002024 |
| optional_test_hash.test | RUN | 0.223229 | 0.612577 | 0.726944 | 0.002184 | 0.001697 | 0.001512 |
| optional_test_map.test | RUN | 0.258320 | 0.747375 | 1.051056 | 0.001636 | 0.002008 | 0.002314 |
| optional_test_tie.test | RUN | 0.182822 | 0.571888 | 0.618809 | 0.001821 | 0.001682 | 0.001683 |
| optional_test_ranges_find.test | RUN | 0.175118 | 0.412652 | 0.574366 | 0.001846 | 0.001343 | 0.001906 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.309049 | 1.232018 | 0.783285 | 0.001699 | 0.001819 | 0.001952 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.200658 | 0.549301 | 0.612789 | 0.001796 | 0.001748 | 0.001834 |
| optional_test_ref_assign_const_int.test | RUN | 0.199411 | 0.526275 | 0.619912 | 0.002054 | 0.001674 | 0.001875 |
| optional_test_ref_converting_ctor.test | RUN | 0.268260 | 0.736017 | 0.686757 | 0.001674 | 0.002024 | 0.001720 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.195037 | 0.492245 | 0.614173 | 0.001449 | 0.002014 | 0.001690 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.176937 | 0.468450 | 0.565568 | 0.002006 | 0.001572 | 0.001672 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.175269 | 0.450046 | 0.663465 | 0.001733 | 0.001648 | 0.001638 |
| optional_test_ref_portable_minimum.test | RUN | 0.758671 | 2.405645 | 2.133527 | 0.001706 | 0.001665 | 0.002152 |
| optional_test_ref_move.test | RUN | 0.189343 | 0.467554 | 0.609630 | 0.001764 | 0.001916 | 0.001569 |
| optional_test_ref_to_val.test | RUN | 0.286436 | 0.947455 | 0.832174 | 0.002068 | 0.001770 | 0.001752 |
| optional_test_inplace_factory.test | RUN | 0.237043 | 0.865592 | 0.834189 | 0.001631 | 0.001944 | 0.001988 |
| optional_test_io.test | RUN | 0.224551 | 0.943159 | 0.788127 | 0.001656 | 0.001650 | 0.001502 |
| optional_test_move.test | RUN | 0.231100 | 0.655673 | 0.732048 | 0.001652 | 0.002212 | 0.001737 |
| optional_test_noexcept_move.test | RUN | 0.143610 | 0.301001 | 0.401621 | 0.000664 | 0.000668 | 0.001840 |
| optional_test_cmp_none.test | RUN | 0.178162 | 0.442859 | 0.573044 | 0.001586 | 0.002058 | 0.001491 |
| optional_test_value_access.test | RUN | 0.221132 | 0.742668 | 0.760428 | 0.001702 | 0.001686 | 0.001776 |
| optional_test_emplace.test | RUN | 0.214411 | 0.740794 | 0.787122 | 0.001785 | 0.001763 | 0.001544 |
| optional_test_minimum_requirements.test | RUN | 0.200804 | 0.612774 | 0.705141 | 0.001595 | 0.001846 | 0.001768 |
| optional_test_msvc_bug_workaround.test | RUN | 0.171566 | 0.423699 | 0.602152 | 0.001393 | 0.001684 | 0.001880 |
| optional_test_member_T.test | RUN | 0.165236 | 0.407751 | 0.573209 | 0.001537 | 0.001532 | 0.001369 |
| optional_test_tc_base.test | RUN | 0.165974 | 0.468103 | 0.581539 | 0.001620 | 0.001958 | 0.001557 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.131402 | 0.293487 | 0.380620 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.123500 | 0.286701 | 0.375323 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.121298 | 0.267048 | 0.360480 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.155878 | 0.367373 | 0.524864 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.112616 | 0.263616 | 0.382597 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.115080 | 0.266261 | 0.360023 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.118936 | 0.261775 | 0.374246 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.123949 | 0.268993 | 0.340914 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.122008 | 0.260226 | 0.341474 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.116728 | 0.254777 | 0.346515 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.114342 | 0.257843 | 0.382124 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.193111 | 0.400965 | 0.572923 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.192139 | 0.408892 | 0.576943 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.115670 | 0.263904 | 0.347815 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.114567 | 0.263083 | 0.338314 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.116248 | 0.254239 | 0.325041 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.121902 | 0.287319 | 0.320981 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.120250 | 0.292836 | 0.342194 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.113840 | 0.258062 | 0.342782 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.114548 | 0.263425 | 0.340468 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.124134 | 0.255415 | 0.344128 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.112572 | 0.264286 | 0.336402 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.115315 | 0.265185 | 0.341871 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.151084 | 0.347968 | 0.487057 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.121077 | 0.295939 | 0.403812 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.121825 | 0.262146 | 0.351549 | — | — | — |
| optional_test_static_properties.test | RUN | 0.256367 | 0.594218 | 0.854449 | 0.003255 | 0.001879 | 0.002432 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.120938 | 0.282731 | 0.353271 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.119698 | 0.301927 | 0.385354 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.119337 | 0.270110 | 0.344827 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.141837 | 0.342631 | 0.481718 | 0.001904 | 0.001790 | 0.001846 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.140459 | 0.350384 | 0.494130 | 0.002240 | 0.001440 | 0.002152 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.134589 | 0.337640 | 0.460339 | 0.002564 | 0.001766 | 0.001471 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.138588 | 0.352355 | 0.462307 | 0.002111 | 0.001762 | 0.001378 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.130544 | 0.307563 | 0.472031 | 0.001806 | 0.002104 | 0.002033 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.090346 | 0.298893 | 0.433777 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 1.008×, 0.985× | 0.993× |
| clang | 0.980× | 0.963×, 0.997× | 0.985× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 8 | 0.001755s | 0.001707s | 0.001716s | byte equality observed |
| optional_test_assign.test | 7 | 0.001600s | 0.001579s | 0.001559s | byte equality observed |
| optional_test_cmp_none.test | 8 | 0.001625s | 0.001600s | 0.001610s | byte equality observed |
| optional_test_constructors.test | 7 | 0.001686s | 0.001594s | 0.001653s | byte equality observed |
| optional_test_conversions_from_U.test | 8 | 0.001583s | 0.001620s | 0.001579s | byte equality observed |
| optional_test_convert_assign.test | 8 | 0.001741s | 0.001573s | 0.001617s | byte equality observed |
| optional_test_convert_from_T.test | 9 | 0.001584s | 0.001615s | 0.001581s | byte equality observed |
| optional_test_emplace.test | 7 | 0.001638s | 0.001671s | 0.001603s | byte equality observed |
| optional_test_empty_braces.test | 8 | 0.001612s | 0.001600s | 0.001647s | byte equality observed |
| optional_test_flat_map.test | 8 | 0.001609s | 0.001711s | 0.001667s | byte equality observed |
| optional_test_hash.test | 7 | 0.001610s | 0.001642s | 0.001613s | byte equality observed |
| optional_test_inplace_factory.test | 7 | 0.001656s | 0.001618s | 0.001597s | byte equality observed |
| optional_test_io.test | 7 | 0.001716s | 0.001653s | 0.001597s | byte equality observed |
| optional_test_make_optional.test | 7 | 0.001600s | 0.001623s | 0.001541s | byte equality observed |
| optional_test_map.test | 7 | 0.001611s | 0.001695s | 0.001569s | byte equality observed |
| optional_test_member_T.test | 6 | 0.001585s | 0.001647s | 0.001631s | byte equality observed |
| optional_test_minimum_requirements.test | 8 | 0.001614s | 0.001662s | 0.001629s | byte equality observed |
| optional_test_move.test | 8 | 0.001591s | 0.001591s | 0.001580s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 7 | 0.001615s | 0.001635s | 0.001673s | byte equality observed |
| optional_test_noexcept_move.test | 8 | 0.000828s | 0.000803s | 0.001568s | byte equality observed |
| optional_test_ranges_find.test | 7 | 0.001583s | 0.001664s | 0.001599s | byte equality observed |
| optional_test_ref_assign_const_int.test | 5 | 0.001596s | 0.001635s | 0.001714s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 7 | 0.001613s | 0.001597s | 0.001580s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 7 | 0.001599s | 0.001673s | 0.001641s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 8 | 0.001561s | 0.001678s | 0.001569s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 8 | 0.001622s | 0.001613s | 0.001656s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 6 | 0.001757s | 0.001605s | 0.001586s | byte equality observed |
| optional_test_ref_converting_ctor.test | 8 | 0.001610s | 0.001694s | 0.001609s | byte equality observed |
| optional_test_ref_move.test | 8 | 0.001597s | 0.001633s | 0.001597s | byte equality observed |
| optional_test_ref_portable_minimum.test | 8 | 0.001655s | 0.001661s | 0.001614s | byte equality observed |
| optional_test_ref_to_val.test | 7 | 0.001560s | 0.001659s | 0.001580s | byte equality observed |
| optional_test_static_properties.test | 6 | 0.001648s | 0.001590s | 0.001599s | byte equality observed |
| optional_test_swap.test | 7 | 0.001585s | 0.001693s | 0.001570s | byte equality observed |
| optional_test_tc_base.test | 7 | 0.001655s | 0.001623s | 0.001595s | byte equality observed |
| optional_test_tie.test | 7 | 0.001626s | 0.001675s | 0.001571s | byte equality observed |
| optional_test_value_access.test | 7 | 0.001698s | 0.001725s | 0.001701s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 8 | 0.001613s | 0.001561s | 0.001555s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001625s | 0.001588s | 0.001624s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 7 | 0.001651s | 0.001731s | 0.001562s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
