# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.098717 | 0.180671 | 0.237893 | 0.001428 | 0.000690 | 0.001696 |
| variant_size.test | RUN | 0.166994 | 0.457537 | 0.456760 | 0.001584 | 0.001603 | 0.001655 |
| variant_alternative.test | RUN | 0.206181 | 1.436806 | 0.836390 | 0.001473 | 0.001661 | 0.001436 |
| variant_holds_alternative.test | RUN | 0.156827 | 0.341254 | 0.475294 | 0.001298 | 0.001550 | 0.001521 |
| variant_holds_alternative_cx.test | COMPILE | 0.103840 | 0.194539 | 0.262757 | — | — | — |
| variant_get_by_index.test | RUN | 0.239204 | 1.561966 | 0.999788 | 0.001551 | 0.001643 | 0.001715 |
| variant_get_by_index_cx.test | COMPILE | 0.102469 | 0.203469 | 0.281256 | — | — | — |
| variant_get_by_type.test | RUN | 0.252763 | 1.676826 | 1.084810 | 0.001447 | 0.001583 | 0.001702 |
| variant_get_by_type_cx.test | COMPILE | 0.092741 | 0.177630 | 0.238432 | — | — | — |
| variant_default_construct.test | RUN | 0.222464 | 1.193699 | 0.872336 | 0.001582 | 0.001534 | 0.001781 |
| variant_default_construct_cx.test | COMPILE | 0.102159 | 0.195443 | 0.268203 | — | — | — |
| variant_copy_construct.test | RUN | 0.341975 | 2.102768 | 1.649224 | 0.001391 | 0.001346 | 0.001656 |
| variant_copy_construct_cx.test | COMPILE | 0.109634 | 0.227683 | 0.317429 | — | — | — |
| variant_move_construct.test | RUN | 0.323017 | 1.804092 | 1.423194 | 0.001255 | 0.001402 | 0.001637 |
| variant_move_construct_cx.test | COMPILE | 0.093083 | 0.184720 | 0.231574 | — | — | — |
| variant_value_construct.test | RUN | 0.178214 | 0.454253 | 0.601659 | 0.001178 | 0.001506 | 0.001674 |
| variant_value_construct_cx.test | COMPILE | 0.101135 | 0.217505 | 0.283507 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.175938 | 0.456264 | 0.607719 | 0.001486 | 0.001517 | 0.001514 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.099036 | 0.198904 | 0.285146 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.180933 | 0.441736 | 0.593864 | 0.001464 | 0.001522 | 0.001564 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.100320 | 0.208099 | 0.265300 | — | — | — |
| variant_copy_assign.test | RUN | 0.229948 | 1.404182 | 0.982532 | 0.001402 | 0.001370 | 0.001441 |
| variant_copy_assign_cx.test | COMPILE | 0.092872 | 0.180995 | 0.230183 | — | — | — |
| variant_move_assign.test | RUN | 0.232205 | 1.417610 | 0.936514 | 0.001396 | 0.001672 | 0.001306 |
| variant_move_assign_cx.test | COMPILE | 0.090479 | 0.180298 | 0.228844 | — | — | — |
| variant_value_assign.test | RUN | 0.183948 | 0.489795 | 0.666332 | 0.001309 | 0.001590 | 0.001427 |
| variant_value_assign_cx.test | COMPILE | 0.091570 | 0.167796 | 0.233206 | — | — | — |
| variant_emplace_index.test | RUN | 0.203121 | 0.568792 | 0.702687 | 0.001426 | 0.001522 | 0.001578 |
| variant_emplace_index_cx.test | COMPILE | 0.090841 | 0.170007 | 0.227660 | — | — | — |
| variant_emplace_type.test | RUN | 0.185037 | 0.487911 | 0.667071 | 0.001465 | 0.001453 | 0.001572 |
| variant_emplace_type_cx.test | COMPILE | 0.093062 | 0.180506 | 0.232940 | — | — | — |
| variant_swap.test | RUN | 0.191260 | 0.599096 | 0.686992 | 0.001520 | 0.001602 | 0.001563 |
| variant_eq_ne.test | RUN | 0.158959 | 0.366595 | 0.477700 | 0.001415 | 0.001594 | 0.001500 |
| variant_eq_ne_cx.test | COMPILE | 0.089909 | 0.179400 | 0.232372 | — | — | — |
| variant_lt_gt.test | RUN | 0.167397 | 0.425954 | 0.511934 | 0.001462 | 0.001622 | 0.001600 |
| variant_lt_gt_cx.test | COMPILE | 0.091396 | 0.174251 | 0.206894 | — | — | — |
| variant_destroy.test | RUN | 0.167982 | 0.427287 | 0.545644 | 0.001494 | 0.001723 | 0.001634 |
| variant_visit.test | RUN | 0.896762 | 3.269514 | 9.039797 | 0.001378 | 0.001571 | 0.001492 |
| variant_convert_construct.test | RUN | 0.200321 | 0.565361 | 0.705389 | 0.001473 | 0.001632 | 0.001574 |
| variant_subset.test | RUN | 0.212979 | 0.610294 | 0.790378 | 0.001557 | 0.001567 | 0.001717 |
| variant_valueless.test | RUN | 0.157273 | 0.379522 | 0.504240 | 0.001503 | 0.001666 | 0.001598 |
| variant_copy_construct_throw.test | RUN | 0.138054 | 0.295265 | 0.367962 | 0.001473 | 0.001686 | 0.001512 |
| variant_move_construct_throw.test | RUN | 0.128908 | 0.294614 | 0.365140 | 0.001583 | 0.001572 | 0.001576 |
| variant_convert_construct_throw.test | RUN | 0.134956 | 0.295125 | 0.381947 | 0.001406 | 0.001714 | 0.001661 |
| variant_copy_assign_throw.test | RUN | 0.127434 | 0.272308 | 0.378136 | 0.001380 | 0.001650 | 0.001511 |
| variant_move_assign_throw.test | RUN | 0.127204 | 0.285185 | 0.366405 | 0.001636 | 0.001662 | 0.001811 |
| variant_get_by_index_nx.test | RUN | 0.274539 | 1.318753 | 0.976312 | 0.001456 | 0.001456 | 0.001695 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.098962 | 0.204630 | 0.265096 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.278852 | 1.488996 | 1.047314 | 0.001410 | 0.001589 | 0.001679 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.095574 | 0.177775 | 0.228489 | — | — | — |
| variant_subset_nx.test | RUN | 0.239407 | 0.575966 | 0.820355 | 0.001676 | 0.001604 | 0.001710 |
| variant_hash.test | RUN | 0.229596 | 0.669600 | 0.729316 | 0.001720 | 0.001794 | 0.001492 |
| variant_trivial.test | RUN | 1.206939 | 3.820775 | 5.498543 | 0.001646 | 0.001598 | 0.001618 |
| variant_special.test | RUN | 1.146355 | 4.178647 | 5.602170 | 0.001427 | 0.001779 | 0.001654 |
| variant_visit_derived.test | RUN | 0.163598 | 0.344801 | 0.540411 | 0.001738 | 0.001670 | 0.001581 |
| variant_many_types.test | RUN | 1.247920 | 5.180694 | 7.722886 | 0.001357 | 0.001510 | 0.001360 |
| variant_visit_r.test | RUN | 0.469361 | 1.552964 | 3.026656 | 0.001738 | 0.001492 | 0.001616 |
| variant_derived_construct.test | COMPILE | 0.098913 | 0.184975 | 0.247135 | — | — | — |
| variant_visit_by_index.test | RUN | 0.214047 | 0.481157 | 0.722503 | 0.001466 | 0.001674 | 0.001665 |
| variant_ostream_insert.test | RUN | 0.153668 | 0.394258 | 0.441927 | 0.001714 | 0.001514 | 0.001648 |
| is_output_streamable.test | RUN | 0.189071 | 0.790287 | 0.645098 | 0.001372 | 0.001604 | 0.001660 |
| variant_json_value_from.test | RUN | 0.284518 | 0.606248 | 0.831943 | 0.001536 | 0.001676 | 0.001704 |
| variant_json_value_to.test | RUN | 0.351563 | 0.883542 | 1.153459 | 0.001462 | 0.001392 | 0.001632 |
| variant_uses_double_storage.test | COMPILE | 0.101777 | 0.182228 | 0.277680 | — | — | — |
| variant_derived_construct2.test | RUN | 0.135671 | 0.290412 | 0.393165 | 0.001432 | 0.001609 | 0.001464 |
| variant_visit_cx.test | COMPILE | 0.096273 | 0.176849 | 0.240007 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.095216 | 0.172726 | 0.231321 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.095870 | 0.167994 | 0.213320 | — | — | — |
| variant_index_type_126.test | RUN | 0.812477 | 1.696447 | 3.378464 | 0.001542 | 0.001562 | 0.001649 |
| variant_index_type_127.test | RUN | 0.816587 | 1.727748 | 3.454251 | 0.001564 | 0.001612 | 0.001465 |
| variant_index_type_128.test | RUN | 0.829957 | 1.744839 | 3.449201 | 0.001558 | 0.001519 | 0.001558 |
| variant_index_type_129.test | RUN | 0.838038 | 1.721108 | 3.458397 | 0.001546 | 0.001293 | 0.001424 |
| variant_index_type_254.test | RUN | 3.062947 | 6.510936 | 10.693626 | 0.001481 | 0.001496 | 0.001780 |
| variant_index_type_255.test | RUN | 3.040215 | 6.538361 | 11.116336 | 0.001504 | 0.001595 | 0.001636 |
| variant_index_type_256.test | RUN | 3.000897 | 6.683346 | 10.922190 | 0.001299 | 0.001321 | 0.001706 |
| variant_index_type_257.test | RUN | 3.064032 | 6.708273 | 11.195843 | 0.001450 | 0.001324 | 0.001450 |
| variant_default_construct_cx_2.test | COMPILE | 0.091517 | 0.171041 | 0.231356 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.091491 | 0.179102 | 0.218559 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.090976 | 0.166752 | 0.216570 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.095635 | 0.168952 | 0.229563 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.095175 | 0.169588 | 0.229122 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.088137 | 0.168200 | 0.235579 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.091855 | 0.161810 | 0.229350 | — | — | — |
| variant_issue_55.test | RUN | 0.133771 | 0.291466 | 0.337561 | 0.001251 | 0.001129 | 0.001263 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993×† | 1.006×, 0.979× | 0.932× |
| clang | 1.010× | 1.013×, 1.007× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001116s | 0.001052s | 0.001138s | byte equality observed |
| quick.test | 10 | 0.001058s | 0.000494s | 0.001089s | byte equality observed |
| variant_alternative.test | 10 | 0.001104s | 0.001100s | 0.001140s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001108s | 0.001098s | 0.001140s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001149s | 0.001122s | 0.001177s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001115s | 0.001095s | 0.001119s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001161s | 0.001090s | 0.001178s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001135s | 0.001091s | 0.001133s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001115s | 0.001088s | 0.001151s | byte equality observed |
| variant_default_construct.test | 10 | 0.001117s | 0.001097s | 0.001138s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001089s | 0.001084s | 0.001099s | byte equality observed |
| variant_destroy.test | 10 | 0.001105s | 0.001067s | 0.001128s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001110s | 0.001082s | 0.001151s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001110s | 0.001077s | 0.001130s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001096s | 0.001069s | 0.001130s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001168s | 0.001149s | 0.001191s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001124s | 0.001081s | 0.001127s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001136s | 0.001138s | 0.001214s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001126s | 0.001083s | 0.001104s | byte equality observed |
| variant_hash.test | 10 | 0.001081s | 0.001085s | 0.001123s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001091s | 0.001061s | 0.001154s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001114s | 0.001071s | 0.001126s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001101s | 0.001088s | 0.001120s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001084s | 0.001058s | 0.001122s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001097s | 0.001075s | 0.001115s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001109s | 0.001039s | 0.001114s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001107s | 0.001050s | 0.001129s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001108s | 0.001046s | 0.001148s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001088s | 0.001074s | 0.001156s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001091s | 0.001050s | 0.001143s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001110s | 0.001074s | 0.001155s | byte equality observed |
| variant_issue_55.test | 10 | 0.001137s | 0.001092s | 0.001137s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001146s | 0.001112s | 0.001145s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001149s | 0.001124s | 0.001144s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001103s | 0.001080s | 0.001139s | byte equality observed |
| variant_many_types.test | 10 | 0.001147s | 0.001094s | 0.001195s | byte equality observed |
| variant_move_assign.test | 10 | 0.001114s | 0.001070s | 0.001096s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001167s | 0.001099s | 0.001155s | byte equality observed |
| variant_move_construct.test | 10 | 0.001131s | 0.001097s | 0.001112s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001130s | 0.001119s | 0.001180s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001167s | 0.001101s | 0.001177s | byte equality observed |
| variant_size.test | 10 | 0.001123s | 0.001075s | 0.001136s | byte equality observed |
| variant_special.test | 10 | 0.001217s | 0.001110s | 0.001245s | byte equality observed |
| variant_subset.test | 10 | 0.001162s | 0.001122s | 0.001173s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001120s | 0.001089s | 0.001125s | byte equality observed |
| variant_swap.test | 10 | 0.001150s | 0.001089s | 0.001147s | byte equality observed |
| variant_trivial.test | 10 | 0.001187s | 0.001167s | 0.001169s | byte equality observed |
| variant_value_assign.test | 10 | 0.001136s | 0.001091s | 0.001165s | byte equality observed |
| variant_value_construct.test | 10 | 0.001122s | 0.001085s | 0.001146s | byte equality observed |
| variant_valueless.test | 10 | 0.001155s | 0.001113s | 0.001171s | byte equality observed |
| variant_visit.test | 10 | 0.001142s | 0.001069s | 0.001127s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001105s | 0.001082s | 0.001108s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001104s | 0.001082s | 0.001142s | byte equality observed |
| variant_visit_r.test | 10 | 0.001128s | 0.001092s | 0.001162s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
