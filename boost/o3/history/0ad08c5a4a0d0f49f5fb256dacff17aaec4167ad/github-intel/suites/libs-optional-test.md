# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.353172 | 1.847477 | 1.068714 | 0.001946 | 0.001895 | 0.001883 |
| optional_test_assign.test | RUN | 0.170418 | 0.382081 | 0.484390 | 0.001834 | 0.001890 | 0.001646 |
| optional_test_swap.test | RUN | 0.193874 | 0.481364 | 0.525181 | 0.001542 | 0.001609 | 0.001543 |
| optional_test_constexpr.test | COMPILE | 0.127744 | 0.255468 | 0.303244 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.188677 | 0.635072 | 0.490259 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.120604 | 0.257895 | 0.294339 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.201559 | 0.495181 | 0.585006 | 0.001927 | 0.001798 | 0.001814 |
| optional_test_constructors.test | RUN | 0.476543 | 1.846243 | 1.516051 | 0.001798 | 0.001747 | 0.001787 |
| optional_test_convert_from_T.test | RUN | 0.182100 | 0.407319 | 0.515033 | 0.001876 | 0.001811 | 0.001716 |
| optional_test_convert_assign.test | RUN | 0.178819 | 0.389152 | 0.487301 | 0.001564 | 0.001646 | 0.001745 |
| optional_test_empty_braces.test | RUN | 0.179975 | 0.392270 | 0.495631 | 0.001886 | 0.001810 | 0.001773 |
| optional_test_make_optional.test | RUN | 0.248955 | 0.605867 | 0.769554 | 0.001762 | 0.001638 | 0.001779 |
| optional_test_flat_map.test | RUN | 0.268623 | 0.717590 | 0.874163 | 0.001674 | 0.001845 | 0.001714 |
| optional_test_hash.test | RUN | 0.218805 | 0.572301 | 0.623772 | 0.001635 | 0.001735 | 0.001759 |
| optional_test_map.test | RUN | 0.268912 | 0.682482 | 0.896407 | 0.001759 | 0.001679 | 0.001584 |
| optional_test_tie.test | RUN | 0.192993 | 0.441322 | 0.508079 | 0.001706 | 0.001596 | 0.001722 |
| optional_test_ranges_find.test | RUN | 0.168825 | 0.385898 | 0.458477 | 0.001514 | 0.001574 | 0.001480 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.289451 | 1.132083 | 0.677697 | 0.001999 | 0.001756 | 0.001718 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.187305 | 0.475684 | 0.501907 | 0.001844 | 0.001692 | 0.001593 |
| optional_test_ref_assign_const_int.test | RUN | 0.194314 | 0.493727 | 0.524640 | 0.001740 | 0.001618 | 0.001828 |
| optional_test_ref_converting_ctor.test | RUN | 0.262167 | 0.683196 | 0.577329 | 0.001619 | 0.001783 | 0.001664 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.197861 | 0.453205 | 0.503819 | 0.001980 | 0.001728 | 0.001788 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.180601 | 0.423355 | 0.473872 | 0.001384 | 0.001451 | 0.001629 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.176147 | 0.409447 | 0.478621 | 0.001754 | 0.001680 | 0.001756 |
| optional_test_ref_portable_minimum.test | RUN | 0.698123 | 2.262452 | 1.795981 | 0.001940 | 0.001724 | 0.001770 |
| optional_test_ref_move.test | RUN | 0.189491 | 0.434251 | 0.493597 | 0.001692 | 0.001782 | 0.001764 |
| optional_test_ref_to_val.test | RUN | 0.274409 | 0.892832 | 0.732715 | 0.001957 | 0.001669 | 0.001635 |
| optional_test_inplace_factory.test | RUN | 0.229662 | 0.815095 | 0.664836 | 0.001887 | 0.001790 | 0.001671 |
| optional_test_io.test | RUN | 0.226260 | 0.910862 | 0.673807 | 0.001726 | 0.001683 | 0.001671 |
| optional_test_move.test | RUN | 0.212595 | 0.614775 | 0.577054 | 0.001529 | 0.001579 | 0.001739 |
| optional_test_noexcept_move.test | RUN | 0.125595 | 0.255730 | 0.290091 | 0.000681 | 0.000829 | 0.001673 |
| optional_test_cmp_none.test | RUN | 0.176292 | 0.418429 | 0.475388 | 0.001719 | 0.001886 | 0.001771 |
| optional_test_value_access.test | RUN | 0.216708 | 0.728033 | 0.660174 | 0.002048 | 0.001604 | 0.001618 |
| optional_test_emplace.test | RUN | 0.213413 | 0.709145 | 0.673201 | 0.001665 | 0.001584 | 0.002046 |
| optional_test_minimum_requirements.test | RUN | 0.192750 | 0.560252 | 0.574265 | 0.001617 | 0.001868 | 0.001823 |
| optional_test_msvc_bug_workaround.test | RUN | 0.172101 | 0.395364 | 0.484716 | 0.001790 | 0.001554 | 0.001798 |
| optional_test_member_T.test | RUN | 0.169659 | 0.383631 | 0.444748 | 0.001890 | 0.001624 | 0.001727 |
| optional_test_tc_base.test | RUN | 0.177313 | 0.386473 | 0.469115 | 0.001593 | 0.001767 | 0.001712 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.131756 | 0.269788 | 0.319000 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.124722 | 0.261509 | 0.292472 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.115366 | 0.239397 | 0.267553 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.159556 | 0.331579 | 0.432766 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.116249 | 0.246127 | 0.277572 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.115421 | 0.242639 | 0.281881 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.117497 | 0.245127 | 0.277500 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.117925 | 0.235632 | 0.279211 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.118415 | 0.244271 | 0.266544 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.118583 | 0.235464 | 0.277032 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.116652 | 0.243194 | 0.274629 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.194955 | 0.379416 | 0.480082 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.194524 | 0.385016 | 0.477577 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.115866 | 0.236356 | 0.274636 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.120229 | 0.240254 | 0.282491 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.114577 | 0.234431 | 0.269509 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.118939 | 0.236587 | 0.274884 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.115859 | 0.233987 | 0.276052 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.121336 | 0.236907 | 0.273038 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.115403 | 0.234967 | 0.273188 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.115944 | 0.237881 | 0.270333 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.116162 | 0.243154 | 0.276996 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.115158 | 0.241451 | 0.279521 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.157107 | 0.318422 | 0.409695 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.124512 | 0.264720 | 0.340519 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.117091 | 0.246851 | 0.287020 | — | — | — |
| optional_test_static_properties.test | RUN | 0.273056 | 0.548943 | 0.706142 | 0.001805 | 0.001763 | 0.001759 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.122988 | 0.264083 | 0.297819 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.126171 | 0.265562 | 0.309940 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.121654 | 0.252432 | 0.287336 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.135158 | 0.323675 | 0.399138 | 0.001882 | 0.001653 | 0.001651 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.142363 | 0.324705 | 0.399777 | 0.001630 | 0.001438 | 0.001420 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.141756 | 0.324456 | 0.390387 | 0.001510 | 0.001648 | 0.001619 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.141138 | 0.335744 | 0.392781 | 0.001347 | 0.001347 | 0.001802 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.115238 | 0.216111 | 0.288012 | 0.001591 | 0.001239 | 0.001231 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.093552 | 0.208659 | 0.230448 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.989×, 0.993× | 1.005× |
| clang | 1.010× | 1.006×, 1.014× | 0.984× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| optional_test.test | 10 | 0.001154s | 0.001124s | 0.001073s | byte equality observed |
| optional_test_assign.test | 10 | 0.001056s | 0.001020s | 0.001038s | byte equality observed |
| optional_test_cmp_none.test | 10 | 0.001086s | 0.001021s | 0.001050s | byte equality observed |
| optional_test_constructors.test | 10 | 0.001066s | 0.001066s | 0.001069s | byte equality observed |
| optional_test_conversions_from_U.test | 10 | 0.001054s | 0.001054s | 0.001042s | byte equality observed |
| optional_test_convert_assign.test | 10 | 0.001079s | 0.001071s | 0.001042s | byte equality observed |
| optional_test_convert_from_T.test | 10 | 0.001048s | 0.001059s | 0.001076s | byte equality observed |
| optional_test_emplace.test | 10 | 0.001096s | 0.001073s | 0.001071s | byte equality observed |
| optional_test_empty_braces.test | 10 | 0.001049s | 0.001053s | 0.001021s | byte equality observed |
| optional_test_flat_map.test | 10 | 0.001047s | 0.001036s | 0.001038s | byte equality observed |
| optional_test_hash.test | 10 | 0.001050s | 0.001044s | 0.001065s | byte equality observed |
| optional_test_inplace_factory.test | 10 | 0.001041s | 0.001041s | 0.001036s | byte equality observed |
| optional_test_io.test | 10 | 0.001088s | 0.001087s | 0.001121s | byte equality observed |
| optional_test_make_optional.test | 10 | 0.001070s | 0.001039s | 0.001087s | byte equality observed |
| optional_test_map.test | 10 | 0.001043s | 0.001066s | 0.001047s | byte equality observed |
| optional_test_member_T.test | 10 | 0.001075s | 0.001027s | 0.001075s | byte equality observed |
| optional_test_minimum_requirements.test | 10 | 0.001059s | 0.001045s | 0.001094s | byte equality observed |
| optional_test_move.test | 10 | 0.001053s | 0.001095s | 0.001093s | byte equality observed |
| optional_test_msvc_bug_workaround.test | 10 | 0.001033s | 0.001067s | 0.001035s | byte equality observed |
| optional_test_noexcept_move.test | 10 | 0.000477s | 0.000473s | 0.001023s | byte equality observed |
| optional_test_ranges_find.test | 10 | 0.001052s | 0.001065s | 0.001028s | byte equality observed |
| optional_test_ref_assign_const_int.test | 10 | 0.001039s | 0.001066s | 0.001045s | byte equality observed |
| optional_test_ref_assign_mutable_int.test | 10 | 0.001053s | 0.001046s | 0.001079s | byte equality observed |
| optional_test_ref_assign_portable_minimum.test | 10 | 0.001068s | 0.001044s | 0.001042s | byte equality observed |
| optional_test_ref_convert_assign_const_int.test | 10 | 0.001039s | 0.001073s | 0.001061s | byte equality observed |
| optional_test_ref_convert_assign_mutable_int.test | 10 | 0.001069s | 0.001041s | 0.001042s | byte equality observed |
| optional_test_ref_convert_assign_non_int.test | 10 | 0.001061s | 0.001047s | 0.001039s | byte equality observed |
| optional_test_ref_converting_ctor.test | 10 | 0.001050s | 0.001066s | 0.001044s | byte equality observed |
| optional_test_ref_move.test | 10 | 0.001050s | 0.001075s | 0.001050s | byte equality observed |
| optional_test_ref_portable_minimum.test | 10 | 0.001110s | 0.001055s | 0.001073s | byte equality observed |
| optional_test_ref_to_val.test | 10 | 0.001080s | 0.001037s | 0.001068s | byte equality observed |
| optional_test_static_properties.test | 10 | 0.001040s | 0.001028s | 0.001021s | byte equality observed |
| optional_test_swap.test | 10 | 0.001044s | 0.001025s | 0.001043s | byte equality observed |
| optional_test_tc_base.test | 10 | 0.001055s | 0.001032s | 0.001029s | byte equality observed |
| optional_test_tie.test | 10 | 0.001076s | 0.001059s | 0.001040s | byte equality observed |
| optional_test_value_access.test | 10 | 0.001105s | 0.001088s | 0.001113s | byte equality observed |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | 10 | 0.001016s | 0.000991s | 0.001025s | byte equality observed |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | 10 | 0.001038s | 0.001060s | 0.001035s | byte equality observed |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | 10 | 0.001059s | 0.001029s | 0.001077s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
