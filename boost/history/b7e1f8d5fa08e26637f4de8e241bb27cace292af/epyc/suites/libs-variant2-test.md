# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.130984 | 0.243399 | 0.359048 | 0.001555 | 0.001744 | 0.001666 |
| variant_size.test | RUN | 0.199437 | 0.470809 | 0.680705 | 0.001601 | 0.001877 | 0.001564 |
| variant_alternative.test | RUN | 0.223565 | 0.789093 | 0.894506 | 0.001670 | 0.001930 | 0.001704 |
| variant_holds_alternative.test | RUN | 0.186435 | 0.470571 | 0.674044 | 0.001342 | 0.001745 | 0.001607 |
| variant_holds_alternative_cx.test | COMPILE | 0.140955 | 0.260356 | 0.415374 | — | — | — |
| variant_get_by_index.test | RUN | 0.250175 | 0.971622 | 1.126566 | 0.001765 | 0.001889 | 0.001832 |
| variant_get_by_index_cx.test | COMPILE | 0.135267 | 0.271228 | 0.425351 | — | — | — |
| variant_get_by_type.test | RUN | 0.254703 | 1.032815 | 1.136306 | 0.001828 | 0.001874 | 0.001555 |
| variant_get_by_type_cx.test | COMPILE | 0.124619 | 0.228919 | 0.357062 | — | — | — |
| variant_default_construct.test | RUN | 0.264764 | 0.881929 | 1.042913 | 0.001618 | 0.001874 | 0.001799 |
| variant_default_construct_cx.test | COMPILE | 0.139629 | 0.253583 | 0.402488 | — | — | — |
| variant_copy_construct.test | RUN | 0.344979 | 1.804717 | 1.755678 | 0.001879 | 0.001690 | 0.001768 |
| variant_copy_construct_cx.test | COMPILE | 0.146981 | 0.299858 | 0.500724 | — | — | — |
| variant_move_construct.test | RUN | 0.338915 | 1.683154 | 1.643977 | 0.001641 | 0.002138 | 0.001802 |
| variant_move_construct_cx.test | COMPILE | 0.123400 | 0.224811 | 0.356206 | — | — | — |
| variant_value_construct.test | RUN | 0.222032 | 0.536003 | 0.810887 | 0.001465 | 0.001696 | 0.001791 |
| variant_value_construct_cx.test | COMPILE | 0.141768 | 0.268286 | 0.437772 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.222137 | 0.600822 | 0.796362 | 0.001755 | 0.001744 | 0.001767 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.137643 | 0.289433 | 0.409310 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.228000 | 0.594390 | 0.805106 | 0.001620 | 0.002017 | 0.001763 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.139576 | 0.268598 | 0.415279 | — | — | — |
| variant_copy_assign.test | RUN | 0.268789 | 0.966272 | 1.129696 | 0.001686 | 0.001794 | 0.001756 |
| variant_copy_assign_cx.test | COMPILE | 0.121023 | 0.223897 | 0.323512 | — | — | — |
| variant_move_assign.test | RUN | 0.276152 | 0.943394 | 1.132545 | 0.001603 | 0.001692 | 0.001494 |
| variant_move_assign_cx.test | COMPILE | 0.124146 | 0.239794 | 0.350636 | — | — | — |
| variant_value_assign.test | RUN | 0.229705 | 0.656782 | 0.852939 | 0.001691 | 0.001816 | 0.001770 |
| variant_value_assign_cx.test | COMPILE | 0.126266 | 0.225644 | 0.356380 | — | — | — |
| variant_emplace_index.test | RUN | 0.244500 | 0.741259 | 0.949164 | 0.001739 | 0.001758 | 0.001743 |
| variant_emplace_index_cx.test | COMPILE | 0.125028 | 0.236872 | 0.308909 | — | — | — |
| variant_emplace_type.test | RUN | 0.229982 | 0.655139 | 0.857313 | 0.001972 | 0.001647 | 0.001622 |
| variant_emplace_type_cx.test | COMPILE | 0.125746 | 0.223493 | 0.348464 | — | — | — |
| variant_swap.test | RUN | 0.234033 | 0.692473 | 0.894818 | 0.001782 | 0.001931 | 0.001760 |
| variant_eq_ne.test | RUN | 0.213681 | 0.477379 | 0.715810 | 0.001792 | 0.001878 | 0.001678 |
| variant_eq_ne_cx.test | COMPILE | 0.123544 | 0.234960 | 0.311760 | — | — | — |
| variant_lt_gt.test | RUN | 0.213048 | 0.506549 | 0.735368 | 0.001779 | 0.001759 | 0.001648 |
| variant_lt_gt_cx.test | COMPILE | 0.126942 | 0.227968 | 0.355370 | — | — | — |
| variant_destroy.test | RUN | 0.215181 | 0.541003 | 0.726658 | 0.001704 | 0.001864 | 0.001738 |
| variant_visit.test | RUN | 0.902595 | 6.757047 | 6.990432 | 0.001639 | 0.001764 | 0.001880 |
| variant_convert_construct.test | RUN | 0.247965 | 0.805497 | 1.000197 | 0.001783 | 0.001968 | 0.001764 |
| variant_subset.test | RUN | 0.259650 | 0.936503 | 1.040554 | 0.001838 | 0.001695 | 0.001839 |
| variant_valueless.test | RUN | 0.202244 | 0.579454 | 0.767611 | 0.001953 | 0.001790 | 0.001571 |
| variant_copy_construct_throw.test | RUN | 0.175157 | 0.353672 | 0.510108 | 0.001787 | 0.001634 | 0.001786 |
| variant_move_construct_throw.test | RUN | 0.172212 | 0.356958 | 0.548435 | 0.001871 | 0.001716 | 0.001925 |
| variant_convert_construct_throw.test | RUN | 0.177888 | 0.375934 | 0.549248 | 0.001674 | 0.001680 | 0.001874 |
| variant_copy_assign_throw.test | RUN | 0.173727 | 0.359571 | 0.557317 | 0.001538 | 0.001655 | 0.001841 |
| variant_move_assign_throw.test | RUN | 0.172097 | 0.360002 | 0.528216 | 0.001641 | 0.001718 | 0.001808 |
| variant_get_by_index_nx.test | RUN | 0.284369 | 0.860603 | 1.048759 | 0.001827 | 0.001901 | 0.001731 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.136869 | 0.272197 | 0.420513 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.282583 | 0.998086 | 1.158174 | 0.001632 | 0.001872 | 0.001882 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.122962 | 0.224643 | 0.312345 | — | — | — |
| variant_subset_nx.test | RUN | 0.290239 | 0.892293 | 1.091182 | 0.001788 | 0.001698 | 0.001782 |
| variant_hash.test | RUN | 0.280816 | 0.759774 | 1.017782 | 0.001853 | 0.001894 | 0.001864 |
| variant_trivial.test | RUN | 1.561363 | 4.749428 | 7.860506 | 0.001683 | 0.001792 | 0.001484 |
| variant_special.test | RUN | 1.518990 | 4.984980 | 7.825431 | 0.001826 | 0.002044 | 0.001798 |
| variant_visit_derived.test | RUN | 0.200595 | 0.508988 | 0.722647 | 0.001866 | 0.001893 | 0.001757 |
| variant_many_types.test | RUN | 1.306571 | 14.479485 | 11.242275 | 0.001878 | 0.002088 | 0.002086 |
| variant_visit_r.test | RUN | 0.485992 | 3.383071 | 3.486684 | 0.001865 | 0.001870 | 0.001848 |
| variant_derived_construct.test | COMPILE | 0.126096 | 0.223083 | 0.363833 | — | — | — |
| variant_visit_by_index.test | RUN | 0.262825 | 0.759246 | 1.004586 | 0.001718 | 0.001884 | 0.001625 |
| variant_ostream_insert.test | RUN | 0.189037 | 0.432561 | 0.651457 | 0.001412 | 0.001824 | 0.001694 |
| is_output_streamable.test | RUN | 0.210068 | 0.630906 | 0.799287 | 0.001445 | 0.001712 | 0.001367 |
| variant_json_value_from.test | RUN | 0.367766 | 0.820301 | 1.198995 | 0.001864 | 0.002004 | 0.001814 |
| variant_json_value_to.test | RUN | 0.393652 | 1.106841 | 1.499276 | 0.001460 | 0.002038 | 0.001919 |
| variant_uses_double_storage.test | COMPILE | 0.135181 | 0.250399 | 0.419992 | — | — | — |
| variant_derived_construct2.test | RUN | 0.171734 | 0.378113 | 0.577656 | 0.001487 | 0.001797 | 0.001750 |
| variant_visit_cx.test | COMPILE | 0.122973 | 0.232887 | 0.345869 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.122635 | 0.232088 | 0.371706 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.119473 | 0.225740 | 0.321989 | — | — | — |
| variant_index_type_126.test | RUN | 1.067856 | 2.418170 | 5.139427 | 0.001850 | 0.001891 | 0.001698 |
| variant_index_type_127.test | RUN | 1.074706 | 2.623375 | 5.036789 | 0.001712 | 0.001846 | 0.001618 |
| variant_index_type_128.test | RUN | 1.077055 | 2.500536 | 5.250535 | 0.001606 | 0.001883 | 0.001758 |
| variant_index_type_129.test | RUN | 1.087475 | 2.505017 | 5.191013 | 0.001644 | 0.001668 | 0.001705 |
| variant_index_type_254.test | RUN | 3.684514 | 9.248469 | 15.521868 | 0.001716 | 0.001594 | 0.001847 |
| variant_index_type_255.test | RUN | 3.712630 | 8.845165 | 15.667758 | 0.001518 | 0.001834 | 0.001831 |
| variant_index_type_256.test | RUN | 3.893769 | 8.979792 | 15.795609 | 0.001493 | 0.001310 | 0.001687 |
| variant_index_type_257.test | RUN | 3.870675 | 7.677784 | 15.218604 | 0.001293 | 0.001290 | 0.001226 |
| variant_default_construct_cx_2.test | COMPILE | 0.123693 | 0.218354 | 0.333992 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.123951 | 0.225540 | 0.328671 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.122444 | 0.228535 | 0.300219 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.122329 | 0.224880 | 0.331378 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.125641 | 0.228933 | 0.325331 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.124001 | 0.224947 | 0.339970 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.115892 | 0.228321 | 0.242764 | — | — | — |
| variant_issue_55.test | RUN | 0.150623 | 0.413234 | 0.524116 | 0.001350 | 0.001396 | 0.001308 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
