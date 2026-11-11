# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.154044 | 0.305497 | 0.387968 | 0.001696 | 0.000808 | 0.001639 |
| variant_size.test | RUN | 0.248407 | 0.732549 | 0.762402 | 0.001757 | 0.001821 | 0.001725 |
| variant_alternative.test | RUN | 0.330744 | 2.410588 | 1.325417 | 0.001683 | 0.001797 | 0.001746 |
| variant_holds_alternative.test | RUN | 0.241803 | 0.550326 | 0.737085 | 0.001588 | 0.001804 | 0.001672 |
| variant_holds_alternative_cx.test | COMPILE | 0.157098 | 0.318369 | 0.420106 | — | — | — |
| variant_get_by_index.test | RUN | 0.372253 | 2.605998 | 1.608949 | 0.001908 | 0.001935 | 0.001830 |
| variant_get_by_index_cx.test | COMPILE | 0.160566 | 0.329781 | 0.435473 | — | — | — |
| variant_get_by_type.test | RUN | 0.397301 | 2.777151 | 1.725919 | 0.001818 | 0.001610 | 0.001530 |
| variant_get_by_type_cx.test | COMPILE | 0.148817 | 0.282475 | 0.336324 | — | — | — |
| variant_default_construct.test | RUN | 0.357302 | 2.058982 | 1.420530 | 0.001771 | 0.001603 | 0.001766 |
| variant_default_construct_cx.test | COMPILE | 0.159435 | 0.313990 | 0.396947 | — | — | — |
| variant_copy_construct.test | RUN | 0.589399 | 3.424454 | 2.488786 | 0.001575 | 0.001804 | 0.001752 |
| variant_copy_construct_cx.test | COMPILE | 0.177017 | 0.358220 | 0.507388 | — | — | — |
| variant_move_construct.test | RUN | 0.550992 | 2.944479 | 2.288736 | 0.001756 | 0.001384 | 0.001758 |
| variant_move_construct_cx.test | COMPILE | 0.146235 | 0.288828 | 0.354531 | — | — | — |
| variant_value_construct.test | RUN | 0.289178 | 0.731335 | 0.939774 | 0.001770 | 0.001811 | 0.001690 |
| variant_value_construct_cx.test | COMPILE | 0.162074 | 0.330728 | 0.419866 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.291520 | 0.733958 | 0.963033 | 0.001553 | 0.001588 | 0.001739 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.163563 | 0.318715 | 0.438184 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.293532 | 0.739802 | 0.924109 | 0.001842 | 0.001758 | 0.001744 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.161041 | 0.324417 | 0.440951 | — | — | — |
| variant_copy_assign.test | RUN | 0.387753 | 2.359223 | 1.525452 | 0.001734 | 0.001542 | 0.001567 |
| variant_copy_assign_cx.test | COMPILE | 0.139639 | 0.291717 | 0.359164 | — | — | — |
| variant_move_assign.test | RUN | 0.381596 | 2.373326 | 1.512069 | 0.001520 | 0.001792 | 0.001820 |
| variant_move_assign_cx.test | COMPILE | 0.147363 | 0.282574 | 0.362184 | — | — | — |
| variant_value_assign.test | RUN | 0.297242 | 0.830800 | 1.042557 | 0.001758 | 0.001712 | 0.001727 |
| variant_value_assign_cx.test | COMPILE | 0.142973 | 0.283823 | 0.357530 | — | — | — |
| variant_emplace_index.test | RUN | 0.318938 | 0.975197 | 1.145616 | 0.001556 | 0.001714 | 0.001775 |
| variant_emplace_index_cx.test | COMPILE | 0.141243 | 0.287251 | 0.348985 | — | — | — |
| variant_emplace_type.test | RUN | 0.298148 | 0.818924 | 1.032842 | 0.001820 | 0.001554 | 0.001732 |
| variant_emplace_type_cx.test | COMPILE | 0.144156 | 0.285654 | 0.362646 | — | — | — |
| variant_swap.test | RUN | 0.322126 | 1.019827 | 1.108428 | 0.001818 | 0.001674 | 0.001788 |
| variant_eq_ne.test | RUN | 0.257362 | 0.605525 | 0.762161 | 0.001522 | 0.001705 | 0.001605 |
| variant_eq_ne_cx.test | COMPILE | 0.141289 | 0.290213 | 0.363835 | — | — | — |
| variant_lt_gt.test | RUN | 0.271311 | 0.696373 | 0.825868 | 0.001778 | 0.001604 | 0.001802 |
| variant_lt_gt_cx.test | COMPILE | 0.143555 | 0.280490 | 0.344313 | — | — | — |
| variant_destroy.test | RUN | 0.276293 | 0.679956 | 0.851038 | 0.001776 | 0.001838 | 0.002886 |
| variant_visit.test | RUN | 1.514149 | 5.277543 | 14.357315 | 0.001896 | 0.001819 | 0.001740 |
| variant_convert_construct.test | RUN | 0.332664 | 0.927174 | 1.191213 | 0.001825 | 0.001871 | 0.001758 |
| variant_subset.test | RUN | 0.343317 | 0.973317 | 1.271795 | 0.001813 | 0.001664 | 0.001579 |
| variant_valueless.test | RUN | 0.249219 | 0.628909 | 0.820517 | 0.001790 | 0.001815 | 0.001697 |
| variant_copy_construct_throw.test | RUN | 0.205324 | 0.474049 | 0.582298 | 0.001741 | 0.001698 | 0.001696 |
| variant_move_construct_throw.test | RUN | 0.204425 | 0.475138 | 0.581390 | 0.001778 | 0.001713 | 0.001799 |
| variant_convert_construct_throw.test | RUN | 0.208063 | 0.515734 | 0.628748 | 0.001946 | 0.001821 | 0.001670 |
| variant_copy_assign_throw.test | RUN | 0.199510 | 0.442533 | 0.571409 | 0.001735 | 0.001688 | 0.001553 |
| variant_move_assign_throw.test | RUN | 0.200224 | 0.447631 | 0.595162 | 0.001801 | 0.001881 | 0.001694 |
| variant_get_by_index_nx.test | RUN | 0.458671 | 2.241627 | 1.535439 | 0.001658 | 0.001632 | 0.001781 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.160365 | 0.340678 | 0.423202 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.473587 | 2.523371 | 1.660034 | 0.001777 | 0.001700 | 0.001663 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.141936 | 0.291949 | 0.368300 | — | — | — |
| variant_subset_nx.test | RUN | 0.366912 | 0.948221 | 1.242307 | 0.001648 | 0.001978 | 0.001792 |
| variant_hash.test | RUN | 0.352840 | 1.112754 | 1.154536 | 0.002176 | 0.001838 | 0.001794 |
| variant_trivial.test | RUN | 1.799084 | 6.145521 | 8.373157 | 0.001844 | 0.001852 | 0.001843 |
| variant_special.test | RUN | 1.690110 | 6.483205 | 9.163243 | 0.001938 | 0.001939 | 0.001806 |
| variant_visit_derived.test | RUN | 0.237734 | 0.592888 | 0.861215 | 0.001837 | 0.001851 | 0.001708 |
| variant_many_types.test | RUN | 1.939432 | 8.357661 | 12.513577 | 0.001754 | 0.001808 | 0.001719 |
| variant_visit_r.test | RUN | 0.755249 | 2.687409 | 4.658582 | 0.001757 | 0.001676 | 0.001720 |
| variant_derived_construct.test | COMPILE | 0.149761 | 0.300059 | 0.385357 | — | — | — |
| variant_visit_by_index.test | RUN | 0.324688 | 0.776928 | 1.120835 | 0.001474 | 0.001690 | 0.001585 |
| variant_ostream_insert.test | RUN | 0.231962 | 0.609892 | 0.758885 | 0.001613 | 0.001880 | 0.001644 |
| is_output_streamable.test | RUN | 0.291314 | 1.278403 | 1.027429 | 0.001700 | 0.001570 | 0.001736 |
| variant_json_value_from.test | RUN | 0.443197 | 1.028323 | 1.299247 | 0.001893 | 0.001856 | 0.001805 |
| variant_json_value_to.test | RUN | 0.567328 | 1.413389 | 1.762603 | 0.001858 | 0.001902 | 0.001976 |
| variant_uses_double_storage.test | COMPILE | 0.152454 | 0.307194 | 0.421881 | — | — | — |
| variant_derived_construct2.test | RUN | 0.207480 | 0.503968 | 0.635585 | 0.001586 | 0.001791 | 0.001738 |
| variant_visit_cx.test | COMPILE | 0.140887 | 0.288860 | 0.351511 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.142147 | 0.289932 | 0.347963 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.141741 | 0.287201 | 0.344413 | — | — | — |
| variant_index_type_126.test | RUN | 1.158844 | 2.669020 | 5.218252 | 0.001796 | 0.001556 | 0.001784 |
| variant_index_type_127.test | RUN | 1.178621 | 2.702097 | 5.386630 | 0.001669 | 0.001772 | 0.001630 |
| variant_index_type_128.test | RUN | 1.191878 | 2.745954 | 5.364568 | 0.001525 | 0.001794 | 0.001694 |
| variant_index_type_129.test | RUN | 1.201853 | 2.747262 | 5.287438 | 0.001702 | 0.001682 | 0.001686 |
| variant_index_type_254.test | RUN | 4.201142 | 10.125229 | 16.733310 | 0.001734 | 0.001814 | 0.001841 |
| variant_index_type_255.test | RUN | 4.178256 | 10.227306 | 16.738928 | 0.001740 | 0.001853 | 0.001778 |
| variant_index_type_256.test | RUN | 4.172421 | 10.425758 | 17.007088 | 0.001644 | 0.001601 | 0.001635 |
| variant_index_type_257.test | RUN | 4.167377 | 10.502378 | 17.159321 | 0.001681 | 0.001744 | 0.001754 |
| variant_default_construct_cx_2.test | COMPILE | 0.142857 | 0.300446 | 0.378518 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.140061 | 0.273908 | 0.321070 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.144966 | 0.296637 | 0.348627 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.145537 | 0.282200 | 0.347500 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.142913 | 0.282021 | 0.332306 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.129092 | 0.248227 | 0.306325 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.128931 | 0.196303 | 0.259578 | — | — | — |
| variant_issue_55.test | RUN | 0.202914 | 0.467459 | 0.564378 | 0.001352 | 0.001230 | 0.001209 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.987× | 0.989×, 0.984× | 1.010× |
| clang | 0.989× | 0.987×, 0.991× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001204s | 0.001229s | 0.001200s | byte equality observed |
| quick.test | 10 | 0.001203s | 0.000549s | 0.001174s | byte equality observed |
| variant_alternative.test | 10 | 0.001222s | 0.001225s | 0.001218s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001214s | 0.001210s | 0.001181s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001245s | 0.001256s | 0.001242s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001228s | 0.001233s | 0.001194s | byte equality observed |
| variant_copy_assign_throw.test | 8 | 0.001225s | 0.001257s | 0.001240s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001217s | 0.001216s | 0.001197s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001225s | 0.001264s | 0.001240s | byte equality observed |
| variant_default_construct.test | 10 | 0.001214s | 0.001220s | 0.001196s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001210s | 0.001204s | 0.001195s | byte equality observed |
| variant_destroy.test | 10 | 0.001213s | 0.001219s | 0.001203s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001222s | 0.001227s | 0.001224s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001212s | 0.001226s | 0.001227s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001215s | 0.001208s | 0.001187s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001296s | 0.001315s | 0.001277s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001234s | 0.001238s | 0.001217s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001274s | 0.001289s | 0.001243s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001209s | 0.001210s | 0.001208s | byte equality observed |
| variant_hash.test | 10 | 0.001207s | 0.001214s | 0.001191s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001216s | 0.001204s | 0.001215s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001228s | 0.001233s | 0.001205s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001234s | 0.001218s | 0.001203s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001223s | 0.001214s | 0.001222s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001209s | 0.001209s | 0.001184s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001202s | 0.001224s | 0.001208s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001226s | 0.001204s | 0.001196s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001216s | 0.001204s | 0.001209s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001210s | 0.001214s | 0.001200s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001213s | 0.001218s | 0.001218s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001208s | 0.001214s | 0.001208s | byte equality observed |
| variant_issue_55.test | 10 | 0.001246s | 0.001214s | 0.001199s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001267s | 0.001232s | 0.001233s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001252s | 0.001250s | 0.001222s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001216s | 0.001217s | 0.001219s | byte equality observed |
| variant_many_types.test | 10 | 0.001273s | 0.001231s | 0.001248s | byte equality observed |
| variant_move_assign.test | 10 | 0.001245s | 0.001211s | 0.001184s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001255s | 0.001250s | 0.001207s | byte equality observed |
| variant_move_construct.test | 10 | 0.001220s | 0.001212s | 0.001200s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001239s | 0.001274s | 0.001257s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001259s | 0.001245s | 0.001208s | byte equality observed |
| variant_size.test | 10 | 0.001190s | 0.001239s | 0.001205s | byte equality observed |
| variant_special.test | 10 | 0.001302s | 0.001295s | 0.001269s | byte equality observed |
| variant_subset.test | 10 | 0.001259s | 0.001245s | 0.001239s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001213s | 0.001223s | 0.001204s | byte equality observed |
| variant_swap.test | 10 | 0.001228s | 0.001241s | 0.001196s | byte equality observed |
| variant_trivial.test | 10 | 0.001276s | 0.001268s | 0.001202s | byte equality observed |
| variant_value_assign.test | 10 | 0.001224s | 0.001210s | 0.001205s | byte equality observed |
| variant_value_construct.test | 10 | 0.001234s | 0.001230s | 0.001207s | byte equality observed |
| variant_valueless.test | 10 | 0.001270s | 0.001280s | 0.001268s | byte equality observed |
| variant_visit.test | 10 | 0.001235s | 0.001253s | 0.001204s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001198s | 0.001234s | 0.001171s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001184s | 0.001232s | 0.001210s | byte equality observed |
| variant_visit_r.test | 10 | 0.001250s | 0.001186s | 0.001215s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
