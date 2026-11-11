# libs/optional/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| optional_test.test | RUN | 0.184542 | 0.694258 | 0.720078 | 0.001872 | 0.001893 | 0.001978 |
| optional_test_assign.test | RUN | 0.140853 | 0.285706 | 0.437812 | 0.001689 | 0.001593 | 0.001650 |
| optional_test_swap.test | RUN | 0.146638 | 0.324949 | 0.455436 | 0.001543 | 0.001515 | 0.001598 |
| optional_test_constexpr.test | COMPILE | 0.103365 | 0.205581 | 0.292053 | — | — | — |
| optional_test_wuninitialized.test | COMPILE | 0.149690 | 0.314994 | 0.427447 | — | — | — |
| optional_test_fwd_header.test | COMPILE | 0.103672 | 0.205027 | 0.273440 | — | — | — |
| optional_test_conversions_from_U.test | RUN | 0.155194 | 0.405423 | 0.492206 | 0.001735 | 0.001694 | 0.001738 |
| optional_test_constructors.test | RUN | 0.263648 | 1.134786 | 1.151129 | 0.001651 | 0.001723 | 0.001840 |
| optional_test_convert_from_T.test | RUN | 0.148129 | 0.310736 | 0.427570 | 0.001652 | 0.001831 | 0.001706 |
| optional_test_convert_assign.test | RUN | 0.154025 | 0.290725 | 0.458431 | 0.001647 | 0.001683 | 0.001619 |
| optional_test_empty_braces.test | RUN | 0.148147 | 0.301848 | 0.422599 | 0.001780 | 0.001794 | 0.001786 |
| optional_test_make_optional.test | RUN | 0.186545 | 0.524893 | 0.614140 | 0.001540 | 0.001417 | 0.001401 |
| optional_test_flat_map.test | RUN | 0.194806 | 0.593605 | 0.644425 | 0.001368 | 0.001907 | 0.001742 |
| optional_test_hash.test | RUN | 0.176761 | 0.398874 | 0.549985 | 0.001830 | 0.001846 | 0.001721 |
| optional_test_map.test | RUN | 0.197153 | 0.605189 | 0.686786 | 0.001694 | 0.001938 | 0.001540 |
| optional_test_tie.test | RUN | 0.160243 | 0.313264 | 0.458960 | 0.001600 | 0.001572 | 0.001680 |
| optional_test_ranges_find.test | RUN | 0.142739 | 0.273244 | 0.421354 | 0.001618 | 0.001811 | 0.001371 |
| optional_test_ref_assign_portable_minimum.test | RUN | 0.155861 | 0.430585 | 0.503584 | 0.001551 | 0.001690 | 0.001367 |
| optional_test_ref_assign_mutable_int.test | RUN | 0.146888 | 0.302917 | 0.449691 | 0.001807 | 0.001728 | 0.001737 |
| optional_test_ref_assign_const_int.test | RUN | 0.147975 | 0.312418 | 0.450908 | 0.001576 | 0.001830 | 0.001570 |
| optional_test_ref_converting_ctor.test | RUN | 0.155223 | 0.392655 | 0.487668 | 0.001685 | 0.001394 | 0.001583 |
| optional_test_ref_convert_assign_non_int.test | RUN | 0.146588 | 0.320345 | 0.433871 | 0.001539 | 0.001627 | 0.001683 |
| optional_test_ref_convert_assign_mutable_int.test | RUN | 0.150027 | 0.300419 | 0.433527 | 0.001729 | 0.001738 | 0.001820 |
| optional_test_ref_convert_assign_const_int.test | RUN | 0.146423 | 0.289525 | 0.444239 | 0.001621 | 0.001886 | 0.001764 |
| optional_test_ref_portable_minimum.test | RUN | 0.225301 | 1.073246 | 1.028295 | 0.001780 | 0.001750 | 0.002018 |
| optional_test_ref_move.test | RUN | 0.147313 | 0.333796 | 0.443498 | 0.001746 | 0.001869 | 0.001754 |
| optional_test_ref_to_val.test | RUN | 0.166127 | 0.497857 | 0.572874 | 0.001829 | 0.001908 | 0.001676 |
| optional_test_inplace_factory.test | RUN | 0.178777 | 0.416518 | 0.510078 | 0.001707 | 0.001676 | 0.001352 |
| optional_test_io.test | RUN | 0.158971 | 0.404389 | 0.498947 | 0.001746 | 0.001797 | 0.001700 |
| optional_test_move.test | RUN | 0.156223 | 0.388464 | 0.490460 | 0.001634 | 0.001896 | 0.001398 |
| optional_test_noexcept_move.test | RUN | 0.109262 | 0.202392 | 0.299312 | 0.000667 | 0.000605 | 0.001564 |
| optional_test_cmp_none.test | RUN | 0.146308 | 0.292120 | 0.423185 | 0.001649 | 0.001806 | 0.001353 |
| optional_test_value_access.test | RUN | 0.158717 | 0.427302 | 0.525802 | 0.001778 | 0.001801 | 0.001541 |
| optional_test_emplace.test | RUN | 0.158045 | 0.453165 | 0.567320 | 0.001554 | 0.001710 | 0.001429 |
| optional_test_minimum_requirements.test | RUN | 0.153985 | 0.369978 | 0.488792 | 0.001620 | 0.001671 | 0.001331 |
| optional_test_msvc_bug_workaround.test | RUN | 0.147573 | 0.285418 | 0.420000 | 0.001684 | 0.001837 | 0.001663 |
| optional_test_member_T.test | RUN | 0.145668 | 0.273027 | 0.416148 | 0.001628 | 0.001670 | 0.001463 |
| optional_test_tc_base.test | RUN | 0.144177 | 0.284055 | 0.423005 | 0.001601 | 0.001558 | 0.001772 |
| optional_test_sfinae_friendly_ctor.test | COMPILE | 0.116697 | 0.216365 | 0.301210 | — | — | — |
| optional_test_path_assignment.test | COMPILE | 0.108325 | 0.206777 | 0.295669 | — | — | — |
| optional_test_fail_const_swap.test | COMPILE_FAIL | 0.103193 | 0.182759 | 0.269504 | — | — | — |
| optional_test_ref_convert_assign_const_int_prevented.test | COMPILE_FAIL | 0.134876 | 0.252392 | 0.410286 | — | — | — |
| optional_test_fail1.test | COMPILE_FAIL | 0.103328 | 0.179048 | 0.247783 | — | — | — |
| optional_test_fail3a.test | COMPILE_FAIL | 0.100126 | 0.181439 | 0.264217 | — | — | — |
| optional_test_fail3b.test | COMPILE_FAIL | 0.102847 | 0.186258 | 0.268246 | — | — | — |
| optional_test_ref_fail1.test | COMPILE_FAIL | 0.101308 | 0.176784 | 0.259921 | — | — | — |
| optional_test_ref_fail3.test | COMPILE_FAIL | 0.096602 | 0.175383 | 0.256208 | — | — | — |
| optional_test_ref_fail4.test | COMPILE_FAIL | 0.099058 | 0.180420 | 0.262431 | — | — | — |
| optional_test_ref_fail_convert_from_temporary_optional_T.test | COMPILE_FAIL | 0.099672 | 0.181019 | 0.261905 | — | — | — |
| optional_test_inplace_fail.test | COMPILE_FAIL | 0.166021 | 0.296310 | 0.446801 | — | — | — |
| optional_test_inplace_fail2.test | COMPILE_FAIL | 0.166136 | 0.296733 | 0.451487 | — | — | — |
| optional_test_fail_implicit_bool_convert.test | COMPILE_FAIL | 0.096160 | 0.178154 | 0.265005 | — | — | — |
| optional_test_fail_copying_a_moveable_type.test | COMPILE_FAIL | 0.101487 | 0.185855 | 0.273069 | — | — | — |
| optional_test_fail_optional_rvalue_ref.test | COMPILE_FAIL | 0.098188 | 0.174484 | 0.261910 | — | — | — |
| optional_test_ref_fail_init_from_Trefref.test | COMPILE_FAIL | 0.100109 | 0.176305 | 0.273521 | — | — | — |
| optional_test_ref_fail_init_from_Urefref.test | COMPILE_FAIL | 0.099608 | 0.177801 | 0.258347 | — | — | — |
| optional_test_ref_fail_assign_from_Trefref.test | COMPILE_FAIL | 0.097813 | 0.181113 | 0.261839 | — | — | — |
| optional_test_ref_fail_assign_from_Urefref.test | COMPILE_FAIL | 0.094979 | 0.175842 | 0.279330 | — | — | — |
| optional_test_fail_convert_from_null.test | COMPILE_FAIL | 0.098576 | 0.179410 | 0.271598 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or.test | COMPILE_FAIL | 0.099231 | 0.175704 | 0.257587 | — | — | — |
| optional_test_fail_explicit_convert_in_value_or_call.test | COMPILE_FAIL | 0.097391 | 0.184881 | 0.253300 | — | — | — |
| optional_test_fail_io_without_io.test | COMPILE_FAIL | 0.134421 | 0.240257 | 0.391104 | — | — | — |
| optional_test_fail_none_io_without_io.test | COMPILE_FAIL | 0.105414 | 0.206114 | 0.335396 | — | — | — |
| optional_test_fail_convert_assign_of_enums.test | COMPILE_FAIL | 0.101619 | 0.184895 | 0.279590 | — | — | — |
| optional_test_static_properties.test | RUN | 0.230765 | 0.433117 | 0.671314 | 0.001759 | 0.001878 | 0.001807 |
| optional_test_maybe_uninitialized_warning.test | COMPILE | 0.105093 | 0.210079 | 0.287799 | — | — | — |
| optional_test_deleted_default_ctor.test | COMPILE | 0.107116 | 0.220388 | 0.293128 | — | — | — |
| optional_test_constructible_from_other.test | COMPILE | 0.106086 | 0.201565 | 0.280100 | — | — | — |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_pass.test | RUN | 0.114667 | 0.236566 | 0.360158 | 0.001784 | 0.001698 | 0.001209 |
| optional_xconfig_NO_PROPER_ASSIGN_FROM_CONST_INT_fail.test | RUN_FAIL | 0.121543 | 0.238434 | 0.357426 | 0.001584 | 0.001228 | 0.001414 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_pass.test | RUN | 0.123391 | 0.239132 | 0.343383 | 0.001641 | 0.001448 | 0.001620 |
| optional_xconfig_NO_PROPER_CONVERT_FROM_CONST_INT_fail.test | RUN_FAIL | 0.122274 | 0.238010 | 0.351074 | 0.001668 | 0.001321 | 0.001813 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_pass.test | RUN | 0.106015 | 0.183740 | 0.220686 | 0.001242 | 0.001247 | 0.001228 |
| optional_xconfig_NO_LEGAL_CONVERT_FROM_REF_fail.test | COMPILE_FAIL | 0.060756 | 0.166205 | 0.238957 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
