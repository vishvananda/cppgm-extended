# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.129296 | 0.255238 | 0.368017 | 0.002072 | 0.000849 | 0.001771 |
| variant_size.test | RUN | 0.230561 | 0.625802 | 0.719263 | 0.001887 | 0.002026 | 0.001799 |
| variant_alternative.test | RUN | 0.283448 | 1.932719 | 1.157554 | 0.001848 | 0.001981 | 0.001735 |
| variant_holds_alternative.test | RUN | 0.204927 | 0.475316 | 0.710113 | 0.001768 | 0.001902 | 0.001744 |
| variant_holds_alternative_cx.test | COMPILE | 0.135266 | 0.284742 | 0.402724 | — | — | — |
| variant_get_by_index.test | RUN | 0.325075 | 2.105704 | 1.431800 | 0.001776 | 0.001972 | 0.001876 |
| variant_get_by_index_cx.test | COMPILE | 0.138464 | 0.289298 | 0.427450 | — | — | — |
| variant_get_by_type.test | RUN | 0.339494 | 2.223040 | 1.530135 | 0.001208 | 0.001544 | 0.001614 |
| variant_get_by_type_cx.test | COMPILE | 0.124276 | 0.245840 | 0.326648 | — | — | — |
| variant_default_construct.test | RUN | 0.301362 | 1.660054 | 1.293366 | 0.001903 | 0.001976 | 0.001801 |
| variant_default_construct_cx.test | COMPILE | 0.138868 | 0.269337 | 0.379151 | — | — | — |
| variant_copy_construct.test | RUN | 0.471877 | 2.769366 | 2.207651 | 0.001788 | 0.001678 | 0.001863 |
| variant_copy_construct_cx.test | COMPILE | 0.151172 | 0.310409 | 0.505688 | — | — | — |
| variant_move_construct.test | RUN | 0.454142 | 2.430134 | 2.052556 | 0.001728 | 0.001975 | 0.001730 |
| variant_move_construct_cx.test | COMPILE | 0.129826 | 0.256540 | 0.362597 | — | — | — |
| variant_value_construct.test | RUN | 0.247374 | 0.612878 | 0.888271 | 0.001704 | 0.001874 | 0.001532 |
| variant_value_construct_cx.test | COMPILE | 0.139414 | 0.281655 | 0.430311 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.253054 | 0.628384 | 0.913295 | 0.001640 | 0.001593 | 0.001870 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.142733 | 0.287649 | 0.439991 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.250376 | 0.624195 | 0.841406 | 0.001613 | 0.001900 | 0.001662 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.142084 | 0.268244 | 0.435571 | — | — | — |
| variant_copy_assign.test | RUN | 0.327159 | 1.910466 | 1.375457 | 0.001580 | 0.001844 | 0.001873 |
| variant_copy_assign_cx.test | COMPILE | 0.123892 | 0.246296 | 0.341375 | — | — | — |
| variant_move_assign.test | RUN | 0.320555 | 1.931058 | 1.353654 | 0.001595 | 0.001857 | 0.001669 |
| variant_move_assign_cx.test | COMPILE | 0.127984 | 0.251640 | 0.360941 | — | — | — |
| variant_value_assign.test | RUN | 0.264624 | 0.685121 | 0.954210 | 0.001630 | 0.001860 | 0.001749 |
| variant_value_assign_cx.test | COMPILE | 0.127440 | 0.237379 | 0.361711 | — | — | — |
| variant_emplace_index.test | RUN | 0.277391 | 0.800606 | 1.057773 | 0.001650 | 0.001954 | 0.001870 |
| variant_emplace_index_cx.test | COMPILE | 0.124993 | 0.241551 | 0.332084 | — | — | — |
| variant_emplace_type.test | RUN | 0.255058 | 0.684138 | 0.965987 | 0.001854 | 0.001875 | 0.001772 |
| variant_emplace_type_cx.test | COMPILE | 0.126554 | 0.253287 | 0.365107 | — | — | — |
| variant_swap.test | RUN | 0.275344 | 0.833358 | 1.011479 | 0.001824 | 0.001607 | 0.001540 |
| variant_eq_ne.test | RUN | 0.223282 | 0.503707 | 0.721857 | 0.001563 | 0.001924 | 0.001684 |
| variant_eq_ne_cx.test | COMPILE | 0.127773 | 0.251710 | 0.360214 | — | — | — |
| variant_lt_gt.test | RUN | 0.237413 | 0.599563 | 0.766632 | 0.001835 | 0.001888 | 0.001861 |
| variant_lt_gt_cx.test | COMPILE | 0.125944 | 0.258722 | 0.360648 | — | — | — |
| variant_destroy.test | RUN | 0.234935 | 0.581863 | 0.807958 | 0.001790 | 0.001928 | 0.001828 |
| variant_visit.test | RUN | 1.315596 | 4.564317 | 12.552922 | 0.001729 | 0.001946 | 0.001722 |
| variant_convert_construct.test | RUN | 0.280339 | 0.758433 | 1.092578 | 0.001497 | 0.001980 | 0.001822 |
| variant_subset.test | RUN | 0.290439 | 0.852636 | 1.161256 | 0.001929 | 0.001912 | 0.001835 |
| variant_valueless.test | RUN | 0.224314 | 0.547960 | 0.792679 | 0.001644 | 0.001667 | 0.001716 |
| variant_copy_construct_throw.test | RUN | 0.175500 | 0.415132 | 0.537548 | 0.001754 | 0.001722 | 0.001874 |
| variant_move_construct_throw.test | RUN | 0.175540 | 0.405450 | 0.573036 | 0.001933 | 0.001884 | 0.001785 |
| variant_convert_construct_throw.test | RUN | 0.179847 | 0.436911 | 0.554001 | 0.001790 | 0.001810 | 0.001766 |
| variant_copy_assign_throw.test | RUN | 0.174444 | 0.368262 | 0.577728 | 0.001596 | 0.001786 | 0.001909 |
| variant_move_assign_throw.test | RUN | 0.180659 | 0.380502 | 0.514160 | 0.001826 | 0.001857 | 0.001756 |
| variant_get_by_index_nx.test | RUN | 0.362843 | 1.814939 | 1.410120 | 0.001619 | 0.001802 | 0.001883 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.138826 | 0.300165 | 0.388474 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.372185 | 2.024610 | 1.465020 | 0.001810 | 0.001856 | 0.001886 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.128051 | 0.261021 | 0.353434 | — | — | — |
| variant_subset_nx.test | RUN | 0.311872 | 0.785921 | 1.200330 | 0.001529 | 0.001910 | 0.001842 |
| variant_hash.test | RUN | 0.311384 | 0.920860 | 1.050124 | 0.001897 | 0.001991 | 0.001928 |
| variant_trivial.test | RUN | 1.561144 | 4.996295 | 8.298685 | 0.001733 | 0.002007 | 0.001844 |
| variant_special.test | RUN | 1.585888 | 5.420336 | 8.262638 | 0.002056 | 0.001993 | 0.001921 |
| variant_visit_derived.test | RUN | 0.209696 | 0.474610 | 0.802058 | 0.001871 | 0.001942 | 0.001819 |
| variant_many_types.test | RUN | 1.619129 | 7.414066 | 11.317578 | 0.001972 | 0.001965 | 0.001880 |
| variant_visit_r.test | RUN | 0.674606 | 2.277021 | 4.320111 | 0.001936 | 0.001793 | 0.001876 |
| variant_derived_construct.test | COMPILE | 0.127422 | 0.236782 | 0.372316 | — | — | — |
| variant_visit_by_index.test | RUN | 0.272676 | 0.596332 | 0.966812 | 0.001599 | 0.001832 | 0.001904 |
| variant_ostream_insert.test | RUN | 0.197520 | 0.519533 | 0.700176 | 0.001834 | 0.002014 | 0.001773 |
| is_output_streamable.test | RUN | 0.246030 | 1.031506 | 0.943753 | 0.001580 | 0.001942 | 0.001868 |
| variant_json_value_from.test | RUN | 0.389015 | 0.916524 | 1.266588 | 0.001916 | 0.002052 | 0.001793 |
| variant_json_value_to.test | RUN | 0.474313 | 1.151977 | 1.617697 | 0.001872 | 0.001862 | 0.001924 |
| variant_uses_double_storage.test | COMPILE | 0.133390 | 0.279000 | 0.391621 | — | — | — |
| variant_derived_construct2.test | RUN | 0.180143 | 0.426054 | 0.613344 | 0.001588 | 0.001912 | 0.001837 |
| variant_visit_cx.test | COMPILE | 0.120985 | 0.240430 | 0.358801 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.130825 | 0.255689 | 0.347307 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.123066 | 0.242829 | 0.353361 | — | — | — |
| variant_index_type_126.test | RUN | 1.055875 | 2.452673 | 5.125270 | 0.001740 | 0.001953 | 0.001858 |
| variant_index_type_127.test | RUN | 1.068060 | 2.467097 | 5.248449 | 0.001816 | 0.001762 | 0.001865 |
| variant_index_type_128.test | RUN | 1.090836 | 2.511122 | 5.278423 | 0.001607 | 0.001582 | 0.001807 |
| variant_index_type_129.test | RUN | 1.078648 | 2.554987 | 5.257917 | 0.001848 | 0.001862 | 0.001768 |
| variant_index_type_254.test | RUN | 3.698461 | 9.075599 | 15.585219 | 0.001871 | 0.001867 | 0.001898 |
| variant_index_type_255.test | RUN | 3.866365 | 9.135597 | 15.979952 | 0.001799 | 0.001852 | 0.001617 |
| variant_index_type_256.test | RUN | 3.866346 | 9.278831 | 16.079289 | 0.001660 | 0.001898 | 0.001798 |
| variant_index_type_257.test | RUN | 3.813570 | 9.283211 | 16.039458 | 0.001400 | 0.001355 | 0.001846 |
| variant_default_construct_cx_2.test | COMPILE | 0.129694 | 0.242668 | 0.363181 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.126129 | 0.267299 | 0.322793 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.123094 | 0.236568 | 0.303198 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.126192 | 0.241071 | 0.337971 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.124032 | 0.246425 | 0.327480 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.118966 | 0.268506 | 0.341512 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.086599 | 0.236130 | 0.280950 | — | — | — |
| variant_issue_55.test | RUN | 0.163432 | 0.467971 | 0.482817 | 0.001390 | 0.001313 | 0.001267 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.990×, 0.995× | 0.987× |
| clang | 1.002× | 0.998×, 1.006× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001186s | 0.001206s | 0.001185s | byte equality observed |
| quick.test | 10 | 0.001181s | 0.000542s | 0.001163s | byte equality observed |
| variant_alternative.test | 10 | 0.001184s | 0.001226s | 0.001191s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001208s | 0.001197s | 0.001208s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001213s | 0.001246s | 0.001234s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001193s | 0.001218s | 0.001185s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001217s | 0.001221s | 0.001237s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001192s | 0.001194s | 0.001186s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001229s | 0.001239s | 0.001236s | byte equality observed |
| variant_default_construct.test | 10 | 0.001199s | 0.001200s | 0.001185s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001180s | 0.001189s | 0.001192s | byte equality observed |
| variant_destroy.test | 10 | 0.001192s | 0.001176s | 0.001207s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001188s | 0.001223s | 0.001191s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001199s | 0.001189s | 0.001205s | byte equality observed |
| variant_eq_ne.test | 6 | 0.001187s | 0.001192s | 0.001211s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001280s | 0.001295s | 0.001269s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001211s | 0.001225s | 0.001193s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001255s | 0.001295s | 0.001257s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001213s | 0.001219s | 0.001195s | byte equality observed |
| variant_hash.test | 10 | 0.001208s | 0.001211s | 0.001189s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001190s | 0.001217s | 0.001184s | byte equality observed |
| variant_in_place_index_construct.test | 8 | 0.001221s | 0.001204s | 0.001196s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001193s | 0.001209s | 0.001214s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001174s | 0.001193s | 0.001204s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001198s | 0.001207s | 0.001183s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001185s | 0.001170s | 0.001192s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001170s | 0.001197s | 0.001210s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001191s | 0.001212s | 0.001204s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001188s | 0.001210s | 0.001183s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001195s | 0.001185s | 0.001199s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001200s | 0.001190s | 0.001186s | byte equality observed |
| variant_issue_55.test | 10 | 0.001206s | 0.001237s | 0.001197s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001229s | 0.001231s | 0.001227s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001245s | 0.001218s | 0.001239s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001180s | 0.001193s | 0.001172s | byte equality observed |
| variant_many_types.test | 10 | 0.001209s | 0.001206s | 0.001229s | byte equality observed |
| variant_move_assign.test | 10 | 0.001211s | 0.001210s | 0.001188s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001219s | 0.001234s | 0.001258s | byte equality observed |
| variant_move_construct.test | 10 | 0.001205s | 0.001219s | 0.001183s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001216s | 0.001229s | 0.001245s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001223s | 0.001232s | 0.001233s | byte equality observed |
| variant_size.test | 10 | 0.001183s | 0.001185s | 0.001195s | byte equality observed |
| variant_special.test | 10 | 0.001264s | 0.001251s | 0.001237s | byte equality observed |
| variant_subset.test | 10 | 0.001252s | 0.001217s | 0.001242s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001201s | 0.001197s | 0.001209s | byte equality observed |
| variant_swap.test | 10 | 0.001209s | 0.001213s | 0.001198s | byte equality observed |
| variant_trivial.test | 10 | 0.001252s | 0.001229s | 0.001212s | byte equality observed |
| variant_value_assign.test | 10 | 0.001199s | 0.001203s | 0.001199s | byte equality observed |
| variant_value_construct.test | 10 | 0.001216s | 0.001213s | 0.001189s | byte equality observed |
| variant_valueless.test | 10 | 0.001253s | 0.001266s | 0.001236s | byte equality observed |
| variant_visit.test | 10 | 0.001211s | 0.001195s | 0.001182s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001178s | 0.001157s | 0.001204s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001203s | 0.001170s | 0.001206s | byte equality observed |
| variant_visit_r.test | 10 | 0.001197s | 0.001179s | 0.001190s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
