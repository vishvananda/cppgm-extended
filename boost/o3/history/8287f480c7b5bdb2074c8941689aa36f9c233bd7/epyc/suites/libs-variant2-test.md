# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.127121 | 0.245356 | 0.406386 | 0.001519 | 0.000785 | 0.001798 |
| variant_size.test | RUN | 0.211040 | 0.621045 | 0.722869 | 0.001820 | 0.001886 | 0.001980 |
| variant_alternative.test | RUN | 0.268337 | 1.920139 | 1.197182 | 0.001780 | 0.001827 | 0.001824 |
| variant_holds_alternative.test | RUN | 0.204056 | 0.473074 | 0.717824 | 0.001692 | 0.001748 | 0.001958 |
| variant_holds_alternative_cx.test | COMPILE | 0.132629 | 0.269140 | 0.409871 | — | — | — |
| variant_get_by_index.test | RUN | 0.303952 | 2.080951 | 1.437428 | 0.001853 | 0.001964 | 0.002028 |
| variant_get_by_index_cx.test | COMPILE | 0.139735 | 0.283044 | 0.437216 | — | — | — |
| variant_get_by_type.test | RUN | 0.314580 | 2.226231 | 1.547519 | 0.001890 | 0.001772 | 0.001965 |
| variant_get_by_type_cx.test | COMPILE | 0.128231 | 0.248582 | 0.365629 | — | — | — |
| variant_default_construct.test | RUN | 0.297527 | 1.625760 | 1.248764 | 0.001448 | 0.001732 | 0.001491 |
| variant_default_construct_cx.test | COMPILE | 0.144848 | 0.270062 | 0.430019 | — | — | — |
| variant_copy_construct.test | RUN | 0.479844 | 2.747165 | 2.280176 | 0.001681 | 0.001678 | 0.001739 |
| variant_copy_construct_cx.test | COMPILE | 0.148632 | 0.306534 | 0.473583 | — | — | — |
| variant_move_construct.test | RUN | 0.458633 | 2.397835 | 2.118712 | 0.001620 | 0.001749 | 0.001992 |
| variant_move_construct_cx.test | COMPILE | 0.124048 | 0.250152 | 0.360747 | — | — | — |
| variant_value_construct.test | RUN | 0.235924 | 0.622126 | 0.917669 | 0.001652 | 0.001819 | 0.001982 |
| variant_value_construct_cx.test | COMPILE | 0.138907 | 0.289980 | 0.445646 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.247313 | 0.628208 | 0.866167 | 0.001700 | 0.001686 | 0.001826 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.135268 | 0.289742 | 0.407286 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.247317 | 0.612713 | 0.894895 | 0.001732 | 0.001642 | 0.001631 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.138356 | 0.288040 | 0.449501 | — | — | — |
| variant_copy_assign.test | RUN | 0.314033 | 1.892265 | 1.374789 | 0.001764 | 0.001667 | 0.001762 |
| variant_copy_assign_cx.test | COMPILE | 0.123539 | 0.241853 | 0.366490 | — | — | — |
| variant_move_assign.test | RUN | 0.310417 | 1.884736 | 1.336211 | 0.001710 | 0.001761 | 0.001432 |
| variant_move_assign_cx.test | COMPILE | 0.123743 | 0.251329 | 0.354409 | — | — | — |
| variant_value_assign.test | RUN | 0.255991 | 0.693088 | 0.980085 | 0.001490 | 0.001575 | 0.001493 |
| variant_value_assign_cx.test | COMPILE | 0.123836 | 0.245372 | 0.338278 | — | — | — |
| variant_emplace_index.test | RUN | 0.269650 | 0.788983 | 1.076576 | 0.001716 | 0.001713 | 0.001854 |
| variant_emplace_index_cx.test | COMPILE | 0.125768 | 0.254357 | 0.362839 | — | — | — |
| variant_emplace_type.test | RUN | 0.249373 | 0.659522 | 0.959479 | 0.001779 | 0.001902 | 0.001773 |
| variant_emplace_type_cx.test | COMPILE | 0.125046 | 0.235720 | 0.354555 | — | — | — |
| variant_swap.test | RUN | 0.264598 | 0.836171 | 1.006184 | 0.001780 | 0.001732 | 0.001502 |
| variant_eq_ne.test | RUN | 0.221229 | 0.525932 | 0.730461 | 0.001640 | 0.001608 | 0.001313 |
| variant_eq_ne_cx.test | COMPILE | 0.124578 | 0.236118 | 0.346078 | — | — | — |
| variant_lt_gt.test | RUN | 0.231426 | 0.581296 | 0.783330 | 0.001786 | 0.001862 | 0.001799 |
| variant_lt_gt_cx.test | COMPILE | 0.126084 | 0.252415 | 0.341881 | — | — | — |
| variant_destroy.test | RUN | 0.230283 | 0.576245 | 0.788974 | 0.001710 | 0.001899 | 0.001672 |
| variant_visit.test | RUN | 1.285204 | 4.500566 | 12.436911 | 0.001483 | 0.001777 | 0.001826 |
| variant_convert_construct.test | RUN | 0.278945 | 0.750755 | 1.130640 | 0.001740 | 0.001867 | 0.001842 |
| variant_subset.test | RUN | 0.306664 | 0.834756 | 1.193914 | 0.001831 | 0.001766 | 0.001890 |
| variant_valueless.test | RUN | 0.210279 | 0.525515 | 0.803242 | 0.002060 | 0.001864 | 0.001908 |
| variant_copy_construct_throw.test | RUN | 0.175974 | 0.397581 | 0.546847 | 0.001411 | 0.001958 | 0.001946 |
| variant_move_construct_throw.test | RUN | 0.175099 | 0.397563 | 0.585069 | 0.001776 | 0.001735 | 0.001797 |
| variant_convert_construct_throw.test | RUN | 0.177678 | 0.392483 | 0.578264 | 0.001779 | 0.001857 | 0.001789 |
| variant_copy_assign_throw.test | RUN | 0.171428 | 0.370182 | 0.572280 | 0.001922 | 0.001952 | 0.001710 |
| variant_move_assign_throw.test | RUN | 0.171642 | 0.370729 | 0.537989 | 0.001856 | 0.001916 | 0.001832 |
| variant_get_by_index_nx.test | RUN | 0.371287 | 1.826144 | 1.404314 | 0.001716 | 0.001928 | 0.001943 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.138287 | 0.261047 | 0.392167 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.376394 | 2.011132 | 1.479998 | 0.001864 | 0.001692 | 0.001904 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.121932 | 0.250109 | 0.354462 | — | — | — |
| variant_subset_nx.test | RUN | 0.304501 | 0.838385 | 1.194286 | 0.001574 | 0.001749 | 0.001774 |
| variant_hash.test | RUN | 0.298461 | 0.902888 | 1.065764 | 0.001798 | 0.001889 | 0.001684 |
| variant_trivial.test | RUN | 1.617269 | 5.166460 | 8.300349 | 0.001948 | 0.001756 | 0.001834 |
| variant_special.test | RUN | 1.585302 | 5.379959 | 8.285965 | 0.002110 | 0.001930 | 0.001817 |
| variant_visit_derived.test | RUN | 0.215569 | 0.501429 | 0.800319 | 0.001897 | 0.001873 | 0.001781 |
| variant_many_types.test | RUN | 1.673724 | 7.059624 | 11.470272 | 0.001740 | 0.001874 | 0.001591 |
| variant_visit_r.test | RUN | 0.651117 | 2.176266 | 4.254756 | 0.001619 | 0.001757 | 0.001670 |
| variant_derived_construct.test | COMPILE | 0.130484 | 0.243694 | 0.374262 | — | — | — |
| variant_visit_by_index.test | RUN | 0.287323 | 0.656983 | 1.039688 | 0.001580 | 0.001843 | 0.001815 |
| variant_ostream_insert.test | RUN | 0.213466 | 0.505444 | 0.716612 | 0.001767 | 0.001659 | 0.001628 |
| is_output_streamable.test | RUN | 0.250693 | 1.037312 | 0.934537 | 0.001373 | 0.001843 | 0.001824 |
| variant_json_value_from.test | RUN | 0.375016 | 0.860069 | 1.212744 | 0.001805 | 0.001824 | 0.001906 |
| variant_json_value_to.test | RUN | 0.459654 | 1.150843 | 1.648016 | 0.001678 | 0.001764 | 0.001572 |
| variant_uses_double_storage.test | COMPILE | 0.133751 | 0.254364 | 0.382409 | — | — | — |
| variant_derived_construct2.test | RUN | 0.177803 | 0.429662 | 0.601548 | 0.001612 | 0.001773 | 0.001849 |
| variant_visit_cx.test | COMPILE | 0.123561 | 0.247808 | 0.350576 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.125419 | 0.253668 | 0.333910 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.124394 | 0.240239 | 0.345955 | — | — | — |
| variant_index_type_126.test | RUN | 1.055121 | 2.403524 | 5.172089 | 0.001691 | 0.001782 | 0.001835 |
| variant_index_type_127.test | RUN | 1.114867 | 2.422409 | 5.208202 | 0.001761 | 0.001860 | 0.001935 |
| variant_index_type_128.test | RUN | 1.095776 | 2.449444 | 5.406264 | 0.001671 | 0.001776 | 0.001517 |
| variant_index_type_129.test | RUN | 1.111714 | 2.415975 | 5.340869 | 0.001755 | 0.001844 | 0.001615 |
| variant_index_type_254.test | RUN | 3.713584 | 8.973476 | 15.713851 | 0.001855 | 0.001870 | 0.001818 |
| variant_index_type_255.test | RUN | 3.862117 | 8.987911 | 15.935645 | 0.001645 | 0.001859 | 0.001772 |
| variant_index_type_256.test | RUN | 3.766085 | 9.143601 | 16.317350 | 0.001776 | 0.001617 | 0.001568 |
| variant_index_type_257.test | RUN | 3.895887 | 9.177388 | 16.437878 | 0.001760 | 0.001578 | 0.001802 |
| variant_default_construct_cx_2.test | COMPILE | 0.117013 | 0.267013 | 0.305775 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.121410 | 0.241461 | 0.345794 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.124344 | 0.243890 | 0.349349 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.124451 | 0.238713 | 0.343785 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.124236 | 0.235662 | 0.350366 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.123120 | 0.239859 | 0.311399 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.106492 | 0.205025 | 0.224937 | — | — | — |
| variant_issue_55.test | RUN | 0.163372 | 0.410549 | 0.550693 | 0.001362 | 0.001315 | 0.001252 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996× | 0.996×, 0.995× | 0.995× |
| clang | 0.998× | 0.999×, 0.997× | 1.015× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001179s | 0.001187s | 0.001196s | byte equality observed |
| quick.test | 10 | 0.001155s | 0.000541s | 0.001187s | byte equality observed |
| variant_alternative.test | 10 | 0.001180s | 0.001186s | 0.001168s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001187s | 0.001193s | 0.001201s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001218s | 0.001208s | 0.001234s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001194s | 0.001217s | 0.001184s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001202s | 0.001190s | 0.001217s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001202s | 0.001193s | 0.001179s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001214s | 0.001208s | 0.001220s | byte equality observed |
| variant_default_construct.test | 10 | 0.001184s | 0.001182s | 0.001211s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001182s | 0.001182s | 0.001163s | byte equality observed |
| variant_destroy.test | 10 | 0.001165s | 0.001165s | 0.001197s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001194s | 0.001219s | 0.001203s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001201s | 0.001195s | 0.001215s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001183s | 0.001197s | 0.001181s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001286s | 0.001275s | 0.001249s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001210s | 0.001184s | 0.001178s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001236s | 0.001252s | 0.001243s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001208s | 0.001213s | 0.001181s | byte equality observed |
| variant_hash.test | 10 | 0.001192s | 0.001190s | 0.001176s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001175s | 0.001197s | 0.001200s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001204s | 0.001200s | 0.001189s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001196s | 0.001196s | 0.001185s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001185s | 0.001182s | 0.001175s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001164s | 0.001189s | 0.001187s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001185s | 0.001199s | 0.001177s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001188s | 0.001193s | 0.001201s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001173s | 0.001177s | 0.001209s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001175s | 0.001163s | 0.001207s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001186s | 0.001203s | 0.001178s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001185s | 0.001179s | 0.001182s | byte equality observed |
| variant_issue_55.test | 10 | 0.001193s | 0.001199s | 0.001186s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001218s | 0.001233s | 0.001207s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001222s | 0.001228s | 0.001209s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001193s | 0.001182s | 0.001196s | byte equality observed |
| variant_many_types.test | 10 | 0.001213s | 0.001240s | 0.001216s | byte equality observed |
| variant_move_assign.test | 10 | 0.001189s | 0.001183s | 0.001173s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001230s | 0.001214s | 0.001211s | byte equality observed |
| variant_move_construct.test | 10 | 0.001187s | 0.001194s | 0.001198s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001215s | 0.001222s | 0.001233s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001219s | 0.001224s | 0.001226s | byte equality observed |
| variant_size.test | 10 | 0.001171s | 0.001193s | 0.001184s | byte equality observed |
| variant_special.test | 10 | 0.001271s | 0.001231s | 0.001236s | byte equality observed |
| variant_subset.test | 10 | 0.001210s | 0.001217s | 0.001226s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001180s | 0.001158s | 0.001193s | byte equality observed |
| variant_swap.test | 10 | 0.001195s | 0.001207s | 0.001194s | byte equality observed |
| variant_trivial.test | 10 | 0.001234s | 0.001239s | 0.001211s | byte equality observed |
| variant_value_assign.test | 10 | 0.001196s | 0.001195s | 0.001197s | byte equality observed |
| variant_value_construct.test | 10 | 0.001197s | 0.001173s | 0.001213s | byte equality observed |
| variant_valueless.test | 10 | 0.001248s | 0.001259s | 0.001228s | byte equality observed |
| variant_visit.test | 10 | 0.001187s | 0.001198s | 0.001202s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001196s | 0.001192s | 0.001195s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001182s | 0.001171s | 0.001189s | byte equality observed |
| variant_visit_r.test | 10 | 0.001210s | 0.001174s | 0.001186s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
