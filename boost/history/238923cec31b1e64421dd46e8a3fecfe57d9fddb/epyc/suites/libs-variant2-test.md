# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.104998 | 0.198483 | 0.263391 | 0.001534 | 0.001858 | 0.001670 |
| variant_size.test | RUN | 0.180309 | 0.388037 | 0.458943 | 0.001824 | 0.002004 | 0.001716 |
| variant_alternative.test | RUN | 0.189132 | 0.657491 | 0.632221 | 0.001553 | 0.001936 | 0.001405 |
| variant_holds_alternative.test | RUN | 0.153943 | 0.383621 | 0.473795 | 0.001673 | 0.001743 | 0.001737 |
| variant_holds_alternative_cx.test | COMPILE | 0.113390 | 0.222246 | 0.258302 | — | — | — |
| variant_get_by_index.test | RUN | 0.207561 | 0.821660 | 0.760065 | 0.001930 | 0.002132 | 0.001490 |
| variant_get_by_index_cx.test | COMPILE | 0.111155 | 0.219525 | 0.288005 | — | — | — |
| variant_get_by_type.test | RUN | 0.211557 | 0.884963 | 0.785388 | 0.001890 | 0.001654 | 0.001863 |
| variant_get_by_type_cx.test | COMPILE | 0.105595 | 0.200046 | 0.239207 | — | — | — |
| variant_default_construct.test | RUN | 0.224224 | 0.758153 | 0.721193 | 0.001625 | 0.001833 | 0.001734 |
| variant_default_construct_cx.test | COMPILE | 0.115272 | 0.273013 | 0.277854 | — | — | — |
| variant_copy_construct.test | RUN | 0.289979 | 1.567459 | 1.141702 | 0.001757 | 0.001752 | 0.001531 |
| variant_copy_construct_cx.test | COMPILE | 0.131577 | 0.237304 | 0.328118 | — | — | — |
| variant_move_construct.test | RUN | 0.281639 | 1.423006 | 1.134678 | 0.001970 | 0.001800 | 0.001346 |
| variant_move_construct_cx.test | COMPILE | 0.114964 | 0.183822 | 0.230309 | — | — | — |
| variant_value_construct.test | RUN | 0.199977 | 0.485303 | 0.543632 | 0.001830 | 0.002033 | 0.001597 |
| variant_value_construct_cx.test | COMPILE | 0.123807 | 0.289603 | 0.293207 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.197605 | 0.516641 | 0.543116 | 0.001870 | 0.002084 | 0.001726 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.118880 | 0.227303 | 0.259394 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.201183 | 0.510144 | 0.531791 | 0.001866 | 0.001900 | 0.001637 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.128602 | 0.242210 | 0.278700 | — | — | — |
| variant_copy_assign.test | RUN | 0.230416 | 0.849100 | 0.772983 | 0.001788 | 0.001914 | 0.001811 |
| variant_copy_assign_cx.test | COMPILE | 0.107576 | 0.205318 | 0.220492 | — | — | — |
| variant_move_assign.test | RUN | 0.230431 | 0.832559 | 0.768845 | 0.001458 | 0.002075 | 0.001669 |
| variant_move_assign_cx.test | COMPILE | 0.106661 | 0.203248 | 0.229129 | — | — | — |
| variant_value_assign.test | RUN | 0.203778 | 0.584962 | 0.605556 | 0.001514 | 0.001852 | 0.001765 |
| variant_value_assign_cx.test | COMPILE | 0.108264 | 0.188951 | 0.240488 | — | — | — |
| variant_emplace_index.test | RUN | 0.220867 | 0.648608 | 0.622365 | 0.001646 | 0.001885 | 0.001681 |
| variant_emplace_index_cx.test | COMPILE | 0.113531 | 0.191966 | 0.220810 | — | — | — |
| variant_emplace_type.test | RUN | 0.205288 | 0.537066 | 0.579218 | 0.003442 | 0.001722 | 0.001669 |
| variant_emplace_type_cx.test | COMPILE | 0.107907 | 0.185267 | 0.227891 | — | — | — |
| variant_swap.test | RUN | 0.208640 | 0.568042 | 0.591055 | 0.001976 | 0.001485 | 0.001412 |
| variant_eq_ne.test | RUN | 0.221818 | 0.399683 | 0.455721 | 0.001635 | 0.001836 | 0.001848 |
| variant_eq_ne_cx.test | COMPILE | 0.121420 | 0.177453 | 0.224006 | — | — | — |
| variant_lt_gt.test | RUN | 0.186196 | 0.423910 | 0.525158 | 0.001784 | 0.001990 | 0.001587 |
| variant_lt_gt_cx.test | COMPILE | 0.111519 | 0.187963 | 0.245767 | — | — | — |
| variant_destroy.test | RUN | 0.186565 | 0.460797 | 0.477220 | 0.001813 | 0.002032 | 0.001487 |
| variant_visit.test | RUN | 0.753688 | 5.914123 | 4.778455 | 0.001730 | 0.001778 | 0.001792 |
| variant_convert_construct.test | RUN | 0.216879 | 0.728378 | 0.650588 | 0.001874 | 0.001949 | 0.001790 |
| variant_subset.test | RUN | 0.223260 | 0.799502 | 0.692817 | 0.002049 | 0.001706 | 0.001769 |
| variant_valueless.test | RUN | 0.176892 | 0.509127 | 0.524177 | 0.001942 | 0.001918 | 0.001450 |
| variant_copy_construct_throw.test | RUN | 0.148726 | 0.283476 | 0.364580 | 0.001881 | 0.001724 | 0.001780 |
| variant_move_construct_throw.test | RUN | 0.153800 | 0.315382 | 0.355240 | 0.001542 | 0.002032 | 0.001752 |
| variant_convert_construct_throw.test | RUN | 0.158058 | 0.340909 | 0.396326 | 0.001866 | 0.001920 | 0.001474 |
| variant_copy_assign_throw.test | RUN | 0.146119 | 0.309292 | 0.345919 | 0.001800 | 0.001746 | 0.001720 |
| variant_move_assign_throw.test | RUN | 0.152081 | 0.293783 | 0.370336 | 0.001856 | 0.001806 | 0.001718 |
| variant_get_by_index_nx.test | RUN | 0.239542 | 0.756053 | 0.732237 | 0.001620 | 0.002068 | 0.001654 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.117880 | 0.205234 | 0.284526 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.242649 | 0.785846 | 0.785550 | 0.001904 | 0.001932 | 0.001514 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.108640 | 0.186216 | 0.225403 | — | — | — |
| variant_subset_nx.test | RUN | 0.246987 | 0.764531 | 0.728400 | 0.002254 | 0.001496 | 0.001650 |
| variant_hash.test | RUN | 0.243354 | 0.635755 | 0.688387 | 0.001965 | 0.001931 | 0.001730 |
| variant_trivial.test | RUN | 1.316396 | 4.160337 | 5.325804 | 0.001970 | 0.001928 | 0.001786 |
| variant_special.test | RUN | 1.251140 | 4.298869 | 5.335754 | 0.001938 | 0.001944 | 0.001820 |
| variant_visit_derived.test | RUN | 0.171218 | 0.417473 | 0.503899 | 0.001923 | 0.001795 | 0.001649 |
| variant_many_types.test | RUN | 1.087357 | 12.505356 | 7.759477 | 0.002290 | 0.002104 | 0.002046 |
| variant_visit_r.test | RUN | 0.418913 | 2.895298 | 2.356617 | 0.001777 | 0.002046 | 0.001766 |
| variant_derived_construct.test | COMPILE | 0.120799 | 0.195353 | 0.255472 | — | — | — |
| variant_visit_by_index.test | RUN | 0.215393 | 0.648736 | 0.713279 | 0.001550 | 0.002138 | 0.001680 |
| variant_ostream_insert.test | RUN | 0.157263 | 0.391206 | 0.441167 | 0.001739 | 0.001875 | 0.001723 |
| is_output_streamable.test | RUN | 0.191792 | 0.573292 | 0.551193 | 0.001804 | 0.001676 | 0.001670 |
| variant_json_value_from.test | RUN | 0.356076 | 0.711981 | 0.800795 | 0.001924 | 0.001657 | 0.001752 |
| variant_json_value_to.test | RUN | 0.344994 | 0.909089 | 1.039064 | 0.001896 | 0.001872 | 0.001456 |
| variant_uses_double_storage.test | COMPILE | 0.110518 | 0.201825 | 0.278876 | — | — | — |
| variant_derived_construct2.test | RUN | 0.147219 | 0.292389 | 0.383762 | 0.001544 | 0.001830 | 0.001702 |
| variant_visit_cx.test | COMPILE | 0.104642 | 0.191791 | 0.229317 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.106868 | 0.198005 | 0.252177 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.105408 | 0.185318 | 0.223603 | — | — | — |
| variant_index_type_126.test | RUN | 0.981231 | 1.897365 | 3.336259 | 0.001896 | 0.001823 | 0.001770 |
| variant_index_type_127.test | RUN | 0.984007 | 2.025389 | 3.457977 | 0.001782 | 0.001795 | 0.001632 |
| variant_index_type_128.test | RUN | 1.011825 | 1.901002 | 3.546846 | 0.001558 | 0.001540 | 0.001705 |
| variant_index_type_129.test | RUN | 1.006707 | 2.074174 | 3.533058 | 0.001855 | 0.001595 | 0.001651 |
| variant_index_type_254.test | RUN | 3.301336 | 7.359507 | 11.289127 | 0.001818 | 0.001697 | 0.002001 |
| variant_index_type_255.test | RUN | 3.263889 | 7.368860 | 11.477820 | 0.001432 | 0.001472 | 0.001681 |
| variant_index_type_256.test | RUN | 3.278272 | 6.957207 | 11.265663 | 0.001445 | 0.001286 | 0.001632 |
| variant_index_type_257.test | RUN | 3.326023 | 5.797568 | 10.978609 | 0.001894 | 0.001145 | 0.001204 |
| variant_default_construct_cx_2.test | COMPILE | 0.104570 | 0.171475 | 0.228926 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.103125 | 0.173699 | 0.235676 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.104613 | 0.176698 | 0.235506 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.105906 | 0.176569 | 0.233071 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.107474 | 0.177984 | 0.241774 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.103221 | 0.178667 | 0.240936 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.103267 | 0.173003 | 0.223361 | — | — | — |
| variant_issue_55.test | RUN | 0.157476 | 0.395720 | 0.344739 | 0.001371 | 0.001866 | 0.001406 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
