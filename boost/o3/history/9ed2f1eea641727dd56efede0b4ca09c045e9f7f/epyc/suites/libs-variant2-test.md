# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.115065 | 0.252755 | 0.355270 | 0.001733 | 0.000799 | 0.001658 |
| variant_size.test | RUN | 0.210979 | 0.611022 | 0.675279 | 0.001782 | 0.001878 | 0.001762 |
| variant_alternative.test | RUN | 0.265908 | 1.920609 | 1.142967 | 0.001660 | 0.001948 | 0.001436 |
| variant_holds_alternative.test | RUN | 0.196775 | 0.472291 | 0.700320 | 0.001734 | 0.001864 | 0.001515 |
| variant_holds_alternative_cx.test | COMPILE | 0.122508 | 0.276833 | 0.400073 | — | — | — |
| variant_get_by_index.test | RUN | 0.304547 | 2.135500 | 1.427141 | 0.001894 | 0.001836 | 0.001831 |
| variant_get_by_index_cx.test | COMPILE | 0.124234 | 0.285427 | 0.419549 | — | — | — |
| variant_get_by_type.test | RUN | 0.301778 | 2.283070 | 1.530907 | 0.001982 | 0.001827 | 0.001850 |
| variant_get_by_type_cx.test | COMPILE | 0.113352 | 0.258854 | 0.359523 | — | — | — |
| variant_default_construct.test | RUN | 0.284402 | 1.698613 | 1.224412 | 0.001382 | 0.001632 | 0.001810 |
| variant_default_construct_cx.test | COMPILE | 0.126538 | 0.256333 | 0.410742 | — | — | — |
| variant_copy_construct.test | RUN | 0.444205 | 2.850880 | 2.229206 | 0.001598 | 0.001821 | 0.001812 |
| variant_copy_construct_cx.test | COMPILE | 0.135227 | 0.310530 | 0.488969 | — | — | — |
| variant_move_construct.test | RUN | 0.418102 | 2.414738 | 2.057999 | 0.001513 | 0.001693 | 0.001770 |
| variant_move_construct_cx.test | COMPILE | 0.110675 | 0.258710 | 0.342077 | — | — | — |
| variant_value_construct.test | RUN | 0.220223 | 0.642455 | 0.898786 | 0.001568 | 0.001875 | 0.001836 |
| variant_value_construct_cx.test | COMPILE | 0.127513 | 0.289838 | 0.397203 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.225196 | 0.620982 | 0.906076 | 0.001709 | 0.001750 | 0.001391 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.125224 | 0.278286 | 0.439192 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.226948 | 0.635413 | 0.864400 | 0.001824 | 0.001881 | 0.001778 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.127410 | 0.289597 | 0.437792 | — | — | — |
| variant_copy_assign.test | RUN | 0.299838 | 1.938237 | 1.373804 | 0.001778 | 0.001912 | 0.001502 |
| variant_copy_assign_cx.test | COMPILE | 0.112518 | 0.251922 | 0.322638 | — | — | — |
| variant_move_assign.test | RUN | 0.304261 | 1.934036 | 1.355045 | 0.001593 | 0.001857 | 0.001622 |
| variant_move_assign_cx.test | COMPILE | 0.107184 | 0.248683 | 0.349509 | — | — | — |
| variant_value_assign.test | RUN | 0.243451 | 0.694007 | 0.955815 | 0.001675 | 0.001800 | 0.001736 |
| variant_value_assign_cx.test | COMPILE | 0.113696 | 0.238048 | 0.351791 | — | — | — |
| variant_emplace_index.test | RUN | 0.256845 | 0.828491 | 1.014632 | 0.001357 | 0.001606 | 0.001810 |
| variant_emplace_index_cx.test | COMPILE | 0.109910 | 0.247297 | 0.320545 | — | — | — |
| variant_emplace_type.test | RUN | 0.236117 | 0.714766 | 0.958187 | 0.001742 | 0.001718 | 0.001844 |
| variant_emplace_type_cx.test | COMPILE | 0.108426 | 0.239301 | 0.353534 | — | — | — |
| variant_swap.test | RUN | 0.245992 | 0.847589 | 1.013115 | 0.001733 | 0.001693 | 0.001572 |
| variant_eq_ne.test | RUN | 0.199144 | 0.533782 | 0.710308 | 0.001628 | 0.001938 | 0.001570 |
| variant_eq_ne_cx.test | COMPILE | 0.110074 | 0.241440 | 0.358288 | — | — | — |
| variant_lt_gt.test | RUN | 0.209142 | 0.599486 | 0.763606 | 0.001799 | 0.001944 | 0.001747 |
| variant_lt_gt_cx.test | COMPILE | 0.108237 | 0.259947 | 0.323054 | — | — | — |
| variant_destroy.test | RUN | 0.209301 | 0.592038 | 0.769134 | 0.001888 | 0.002092 | 0.001722 |
| variant_visit.test | RUN | 1.198454 | 4.537916 | 12.236698 | 0.001668 | 0.001644 | 0.001607 |
| variant_convert_construct.test | RUN | 0.268600 | 0.784022 | 1.093913 | 0.002067 | 0.001810 | 0.001750 |
| variant_subset.test | RUN | 0.268627 | 0.844765 | 1.126225 | 0.001796 | 0.001869 | 0.001567 |
| variant_valueless.test | RUN | 0.203654 | 0.574288 | 0.800071 | 0.001845 | 0.001860 | 0.001719 |
| variant_copy_construct_throw.test | RUN | 0.149690 | 0.397302 | 0.550988 | 0.001868 | 0.001946 | 0.001777 |
| variant_move_construct_throw.test | RUN | 0.155273 | 0.403216 | 0.548219 | 0.001774 | 0.001891 | 0.001660 |
| variant_convert_construct_throw.test | RUN | 0.160680 | 0.406364 | 0.590998 | 0.001732 | 0.001831 | 0.001538 |
| variant_copy_assign_throw.test | RUN | 0.153081 | 0.376321 | 0.540769 | 0.001637 | 0.001918 | 0.001744 |
| variant_move_assign_throw.test | RUN | 0.155329 | 0.398704 | 0.500180 | 0.001654 | 0.001944 | 0.001715 |
| variant_get_by_index_nx.test | RUN | 0.368160 | 1.804291 | 1.387111 | 0.001566 | 0.001628 | 0.001734 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.119752 | 0.292887 | 0.372131 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.357314 | 2.001152 | 1.426352 | 0.001839 | 0.001502 | 0.001716 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.106290 | 0.247462 | 0.350162 | — | — | — |
| variant_subset_nx.test | RUN | 0.278413 | 0.803872 | 1.193400 | 0.001707 | 0.001864 | 0.001718 |
| variant_hash.test | RUN | 0.275430 | 0.928354 | 1.048143 | 0.001780 | 0.001916 | 0.001785 |
| variant_trivial.test | RUN | 1.618114 | 5.178282 | 7.988348 | 0.001529 | 0.001748 | 0.001650 |
| variant_special.test | RUN | 1.510553 | 5.583925 | 8.012901 | 0.001874 | 0.002022 | 0.001625 |
| variant_visit_derived.test | RUN | 0.187449 | 0.486719 | 0.791123 | 0.001816 | 0.001873 | 0.001689 |
| variant_many_types.test | RUN | 1.564905 | 7.158490 | 10.934490 | 0.001805 | 0.001749 | 0.001714 |
| variant_visit_r.test | RUN | 0.644887 | 2.220741 | 4.132709 | 0.001780 | 0.001690 | 0.001390 |
| variant_derived_construct.test | COMPILE | 0.118680 | 0.257024 | 0.365682 | — | — | — |
| variant_visit_by_index.test | RUN | 0.257468 | 0.675417 | 1.057971 | 0.001351 | 0.001942 | 0.001712 |
| variant_ostream_insert.test | RUN | 0.180989 | 0.512151 | 0.654263 | 0.001345 | 0.001604 | 0.001641 |
| is_output_streamable.test | RUN | 0.226007 | 1.033363 | 0.874100 | 0.001680 | 0.001599 | 0.001726 |
| variant_json_value_from.test | RUN | 0.344400 | 0.869819 | 1.245589 | 0.001591 | 0.001849 | 0.001410 |
| variant_json_value_to.test | RUN | 0.416682 | 1.171609 | 1.590125 | 0.001866 | 0.001802 | 0.001746 |
| variant_uses_double_storage.test | COMPILE | 0.118660 | 0.253411 | 0.411825 | — | — | — |
| variant_derived_construct2.test | RUN | 0.159812 | 0.407659 | 0.607544 | 0.001509 | 0.001866 | 0.001730 |
| variant_visit_cx.test | COMPILE | 0.111388 | 0.252070 | 0.316332 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.108768 | 0.239154 | 0.352298 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.108777 | 0.242101 | 0.313597 | — | — | — |
| variant_index_type_126.test | RUN | 1.064788 | 2.412871 | 5.044150 | 0.001714 | 0.001927 | 0.001607 |
| variant_index_type_127.test | RUN | 1.041829 | 2.436801 | 5.093566 | 0.001658 | 0.001873 | 0.001611 |
| variant_index_type_128.test | RUN | 1.075272 | 2.479732 | 5.140509 | 0.001719 | 0.001735 | 0.001709 |
| variant_index_type_129.test | RUN | 1.086048 | 2.449934 | 5.105576 | 0.001272 | 0.001820 | 0.001660 |
| variant_index_type_254.test | RUN | 3.645806 | 9.003795 | 15.539117 | 0.001604 | 0.001853 | 0.001852 |
| variant_index_type_255.test | RUN | 3.553813 | 9.035040 | 15.776207 | 0.001706 | 0.001896 | 0.001815 |
| variant_index_type_256.test | RUN | 3.681678 | 9.131257 | 15.848754 | 0.001724 | 0.001656 | 0.001584 |
| variant_index_type_257.test | RUN | 3.729451 | 9.362616 | 15.817542 | 0.001545 | 0.001359 | 0.001550 |
| variant_default_construct_cx_2.test | COMPILE | 0.112875 | 0.270210 | 0.299122 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.109858 | 0.237559 | 0.349469 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.110216 | 0.272558 | 0.285381 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.110498 | 0.241597 | 0.348317 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.108350 | 0.249267 | 0.335869 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.105660 | 0.238209 | 0.341915 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.089779 | 0.231513 | 0.341374 | — | — | — |
| variant_issue_55.test | RUN | 0.159105 | 0.432594 | 0.587941 | 0.001326 | 0.001328 | 0.001292 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.999× | 1.005×, 0.992× | 1.006× |
| clang | 0.994× | 0.998×, 0.990× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001157s | 0.001190s | 0.001151s | byte equality observed |
| quick.test | 10 | 0.001154s | 0.000522s | 0.001162s | byte equality observed |
| variant_alternative.test | 10 | 0.001171s | 0.001183s | 0.001168s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001152s | 0.001168s | 0.001146s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001201s | 0.001213s | 0.001203s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001184s | 0.001185s | 0.001160s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001201s | 0.001192s | 0.001207s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001176s | 0.001204s | 0.001162s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001198s | 0.001205s | 0.001196s | byte equality observed |
| variant_default_construct.test | 10 | 0.001166s | 0.001183s | 0.001168s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001162s | 0.001168s | 0.001177s | byte equality observed |
| variant_destroy.test | 10 | 0.001170s | 0.001174s | 0.001168s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001166s | 0.001175s | 0.001181s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001193s | 0.001175s | 0.001164s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001170s | 0.001176s | 0.001196s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001247s | 0.001263s | 0.001236s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001210s | 0.001178s | 0.001177s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001240s | 0.001276s | 0.001222s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001178s | 0.001186s | 0.001162s | byte equality observed |
| variant_hash.test | 10 | 0.001174s | 0.001180s | 0.001140s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001173s | 0.001165s | 0.001155s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001175s | 0.001185s | 0.001164s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001192s | 0.001169s | 0.001182s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001173s | 0.001191s | 0.001154s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001169s | 0.001164s | 0.001181s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001166s | 0.001173s | 0.001176s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001180s | 0.001195s | 0.001167s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001158s | 0.001157s | 0.001171s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001160s | 0.001153s | 0.001176s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001164s | 0.001181s | 0.001174s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001159s | 0.001161s | 0.001175s | byte equality observed |
| variant_issue_55.test | 10 | 0.001175s | 0.001200s | 0.001164s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001234s | 0.001211s | 0.001201s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001200s | 0.001227s | 0.001179s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001165s | 0.001162s | 0.001152s | byte equality observed |
| variant_many_types.test | 10 | 0.001209s | 0.001197s | 0.001204s | byte equality observed |
| variant_move_assign.test | 10 | 0.001178s | 0.001184s | 0.001165s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001202s | 0.001222s | 0.001184s | byte equality observed |
| variant_move_construct.test | 10 | 0.001176s | 0.001179s | 0.001183s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001199s | 0.001219s | 0.001207s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001189s | 0.001197s | 0.001195s | byte equality observed |
| variant_size.test | 10 | 0.001164s | 0.001168s | 0.001160s | byte equality observed |
| variant_special.test | 10 | 0.001226s | 0.001222s | 0.001228s | byte equality observed |
| variant_subset.test | 10 | 0.001212s | 0.001213s | 0.001218s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001167s | 0.001169s | 0.001185s | byte equality observed |
| variant_swap.test | 10 | 0.001192s | 0.001197s | 0.001198s | byte equality observed |
| variant_trivial.test | 10 | 0.001218s | 0.001232s | 0.001179s | byte equality observed |
| variant_value_assign.test | 10 | 0.001171s | 0.001157s | 0.001173s | byte equality observed |
| variant_value_construct.test | 10 | 0.001163s | 0.001155s | 0.001171s | byte equality observed |
| variant_valueless.test | 10 | 0.001211s | 0.001231s | 0.001259s | byte equality observed |
| variant_visit.test | 10 | 0.001172s | 0.001193s | 0.001185s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001165s | 0.001156s | 0.001135s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001175s | 0.001167s | 0.001144s | byte equality observed |
| variant_visit_r.test | 10 | 0.001192s | 0.001158s | 0.001190s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
