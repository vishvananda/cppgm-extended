# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.130111 | 0.257620 | 0.378806 | 0.001602 | 0.000871 | 0.001735 |
| variant_size.test | RUN | 0.214173 | 0.631803 | 0.702774 | 0.001829 | 0.002046 | 0.001821 |
| variant_alternative.test | RUN | 0.271518 | 1.993765 | 1.158823 | 0.001774 | 0.001998 | 0.001381 |
| variant_holds_alternative.test | RUN | 0.205509 | 0.475751 | 0.706254 | 0.001838 | 0.002066 | 0.001684 |
| variant_holds_alternative_cx.test | COMPILE | 0.132599 | 0.282963 | 0.400271 | — | — | — |
| variant_get_by_index.test | RUN | 0.311172 | 2.228397 | 1.394333 | 0.001536 | 0.002038 | 0.001899 |
| variant_get_by_index_cx.test | COMPILE | 0.141019 | 0.318165 | 0.423975 | — | — | — |
| variant_get_by_type.test | RUN | 0.320784 | 2.280335 | 1.505561 | 0.001907 | 0.001692 | 0.001820 |
| variant_get_by_type_cx.test | COMPILE | 0.127852 | 0.242427 | 0.347893 | — | — | — |
| variant_default_construct.test | RUN | 0.325560 | 1.699327 | 1.219000 | 0.001710 | 0.001641 | 0.001632 |
| variant_default_construct_cx.test | COMPILE | 0.136305 | 0.271765 | 0.409121 | — | — | — |
| variant_copy_construct.test | RUN | 0.483939 | 2.852591 | 2.216311 | 0.001858 | 0.001804 | 0.001823 |
| variant_copy_construct_cx.test | COMPILE | 0.158514 | 0.319613 | 0.484041 | — | — | — |
| variant_move_construct.test | RUN | 0.465283 | 2.550946 | 2.031070 | 0.001385 | 0.001834 | 0.001689 |
| variant_move_construct_cx.test | COMPILE | 0.131052 | 0.254819 | 0.342542 | — | — | — |
| variant_value_construct.test | RUN | 0.248104 | 0.665795 | 0.850164 | 0.001598 | 0.002027 | 0.001803 |
| variant_value_construct_cx.test | COMPILE | 0.146857 | 0.294415 | 0.403743 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.250013 | 0.644783 | 0.864724 | 0.001565 | 0.002070 | 0.001609 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.137387 | 0.298974 | 0.392397 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.256034 | 0.646227 | 0.867462 | 0.001669 | 0.001788 | 0.001543 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.139751 | 0.293193 | 0.425150 | — | — | — |
| variant_copy_assign.test | RUN | 0.321862 | 2.028940 | 1.392509 | 0.001804 | 0.002040 | 0.001683 |
| variant_copy_assign_cx.test | COMPILE | 0.124107 | 0.257033 | 0.330343 | — | — | — |
| variant_move_assign.test | RUN | 0.311603 | 1.999192 | 1.342561 | 0.001623 | 0.001964 | 0.001242 |
| variant_move_assign_cx.test | COMPILE | 0.124256 | 0.261981 | 0.347027 | — | — | — |
| variant_value_assign.test | RUN | 0.249788 | 0.745880 | 0.958104 | 0.001471 | 0.001968 | 0.001622 |
| variant_value_assign_cx.test | COMPILE | 0.129577 | 0.256530 | 0.356815 | — | — | — |
| variant_emplace_index.test | RUN | 0.273292 | 0.823901 | 1.020030 | 0.001661 | 0.001864 | 0.001573 |
| variant_emplace_index_cx.test | COMPILE | 0.120708 | 0.262155 | 0.315904 | — | — | — |
| variant_emplace_type.test | RUN | 0.256965 | 0.694980 | 0.949314 | 0.001801 | 0.001894 | 0.001320 |
| variant_emplace_type_cx.test | COMPILE | 0.127046 | 0.260916 | 0.351682 | — | — | — |
| variant_swap.test | RUN | 0.267176 | 0.871767 | 1.036133 | 0.001876 | 0.001908 | 0.001473 |
| variant_eq_ne.test | RUN | 0.219989 | 0.516675 | 0.713206 | 0.001820 | 0.001951 | 0.001518 |
| variant_eq_ne_cx.test | COMPILE | 0.124444 | 0.263486 | 0.323399 | — | — | — |
| variant_lt_gt.test | RUN | 0.231600 | 0.599437 | 0.779812 | 0.001822 | 0.001926 | 0.001550 |
| variant_lt_gt_cx.test | COMPILE | 0.125962 | 0.253500 | 0.334341 | — | — | — |
| variant_destroy.test | RUN | 0.229810 | 0.617230 | 0.786890 | 0.001864 | 0.002082 | 0.001685 |
| variant_visit.test | RUN | 1.279345 | 4.778336 | 12.459311 | 0.001830 | 0.001827 | 0.001844 |
| variant_convert_construct.test | RUN | 0.273378 | 0.796005 | 1.097510 | 0.001790 | 0.002012 | 0.001914 |
| variant_subset.test | RUN | 0.300012 | 0.862624 | 1.132434 | 0.001828 | 0.001994 | 0.001792 |
| variant_valueless.test | RUN | 0.210918 | 0.586930 | 0.807815 | 0.001785 | 0.001992 | 0.001804 |
| variant_copy_construct_throw.test | RUN | 0.178577 | 0.397469 | 0.562555 | 0.001906 | 0.001985 | 0.001520 |
| variant_move_construct_throw.test | RUN | 0.190620 | 0.391623 | 0.535371 | 0.001770 | 0.001961 | 0.001788 |
| variant_convert_construct_throw.test | RUN | 0.195287 | 0.456205 | 0.571224 | 0.001761 | 0.001970 | 0.001788 |
| variant_copy_assign_throw.test | RUN | 0.181547 | 0.406646 | 0.570782 | 0.001677 | 0.001860 | 0.001812 |
| variant_move_assign_throw.test | RUN | 0.169036 | 0.411387 | 0.540947 | 0.001606 | 0.002168 | 0.001844 |
| variant_get_by_index_nx.test | RUN | 0.379005 | 1.852514 | 1.407403 | 0.001744 | 0.001896 | 0.001528 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.140505 | 0.294683 | 0.391265 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.382708 | 2.101615 | 1.459456 | 0.001790 | 0.001996 | 0.001764 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.119720 | 0.258118 | 0.358546 | — | — | — |
| variant_subset_nx.test | RUN | 0.311060 | 0.840007 | 1.183597 | 0.001631 | 0.001914 | 0.001778 |
| variant_hash.test | RUN | 0.302850 | 0.970191 | 1.025066 | 0.001912 | 0.002021 | 0.001727 |
| variant_trivial.test | RUN | 1.605476 | 5.359605 | 8.115719 | 0.001614 | 0.002024 | 0.001826 |
| variant_special.test | RUN | 1.550111 | 5.958161 | 8.321934 | 0.001938 | 0.002052 | 0.001943 |
| variant_visit_derived.test | RUN | 0.205991 | 0.502015 | 0.770244 | 0.001943 | 0.001980 | 0.001655 |
| variant_many_types.test | RUN | 1.629975 | 7.452179 | 11.027124 | 0.001581 | 0.002052 | 0.001467 |
| variant_visit_r.test | RUN | 0.657095 | 2.294687 | 4.265283 | 0.001910 | 0.001777 | 0.001704 |
| variant_derived_construct.test | COMPILE | 0.127520 | 0.255240 | 0.360317 | — | — | — |
| variant_visit_by_index.test | RUN | 0.284343 | 0.657931 | 0.935454 | 0.001652 | 0.001942 | 0.001778 |
| variant_ostream_insert.test | RUN | 0.197658 | 0.550329 | 0.693115 | 0.001816 | 0.002018 | 0.001790 |
| is_output_streamable.test | RUN | 0.252901 | 1.127465 | 0.976530 | 0.001437 | 0.002005 | 0.001892 |
| variant_json_value_from.test | RUN | 0.371536 | 0.871129 | 1.245473 | 0.001724 | 0.001956 | 0.001778 |
| variant_json_value_to.test | RUN | 0.447323 | 1.196273 | 1.610630 | 0.001886 | 0.002080 | 0.001920 |
| variant_uses_double_storage.test | COMPILE | 0.131151 | 0.265052 | 0.412229 | — | — | — |
| variant_derived_construct2.test | RUN | 0.178392 | 0.440655 | 0.558248 | 0.001557 | 0.002134 | 0.001565 |
| variant_visit_cx.test | COMPILE | 0.127877 | 0.251711 | 0.356975 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.124950 | 0.269675 | 0.355799 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.123594 | 0.247601 | 0.347417 | — | — | — |
| variant_index_type_126.test | RUN | 1.051303 | 2.563959 | 5.149542 | 0.001649 | 0.002063 | 0.001797 |
| variant_index_type_127.test | RUN | 1.054609 | 2.540124 | 5.164016 | 0.001507 | 0.002109 | 0.001833 |
| variant_index_type_128.test | RUN | 1.070832 | 2.627833 | 5.301312 | 0.001716 | 0.001819 | 0.001734 |
| variant_index_type_129.test | RUN | 1.079788 | 2.566453 | 5.251089 | 0.001528 | 0.001982 | 0.001658 |
| variant_index_type_254.test | RUN | 3.834091 | 9.451631 | 15.503385 | 0.001714 | 0.002040 | 0.001888 |
| variant_index_type_255.test | RUN | 3.738855 | 9.562706 | 15.726121 | 0.001290 | 0.002047 | 0.001714 |
| variant_index_type_256.test | RUN | 3.760928 | 9.695622 | 15.813868 | 0.001693 | 0.001678 | 0.001763 |
| variant_index_type_257.test | RUN | 3.886059 | 9.818414 | 15.794201 | 0.001703 | 0.001948 | 0.001644 |
| variant_default_construct_cx_2.test | COMPILE | 0.125880 | 0.286089 | 0.381214 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.114727 | 0.244853 | 0.301047 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.124009 | 0.271772 | 0.292918 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.123387 | 0.248429 | 0.345544 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.122353 | 0.242602 | 0.343774 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.102240 | 0.251120 | 0.340828 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.114464 | 0.188559 | 0.328551 | — | — | — |
| variant_issue_55.test | RUN | 0.163669 | 0.420487 | 0.471641 | 0.001345 | 0.001451 | 0.001269 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.996×† | 0.991×, 1.000× | 0.811× |
| clang | 1.015× | 1.030×, 0.999× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001187s | 0.001205s | 0.001190s | byte equality observed |
| quick.test | 10 | 0.001186s | 0.000532s | 0.001184s | byte equality observed |
| variant_alternative.test | 10 | 0.001184s | 0.001203s | 0.001209s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001218s | 0.001212s | 0.001209s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001246s | 0.001227s | 0.001228s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001202s | 0.001205s | 0.001177s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001224s | 0.001223s | 0.001219s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001199s | 0.001222s | 0.001173s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001230s | 0.001238s | 0.001223s | byte equality observed |
| variant_default_construct.test | 10 | 0.001196s | 0.001204s | 0.001195s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001208s | 0.001179s | 0.001195s | byte equality observed |
| variant_destroy.test | 10 | 0.001178s | 0.001193s | 0.001198s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001187s | 0.001228s | 0.001177s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001221s | 0.001186s | 0.001175s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001174s | 0.001145s | 0.001222s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001280s | 0.001297s | 0.001237s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001178s | 0.001222s | 0.001182s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001254s | 0.001278s | 0.001241s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001214s | 0.001184s | 0.001233s | byte equality observed |
| variant_hash.test | 10 | 0.001176s | 0.001202s | 0.001183s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001203s | 0.001208s | 0.001178s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001217s | 0.001204s | 0.001177s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001205s | 0.001214s | 0.001206s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001180s | 0.001185s | 0.001191s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001192s | 0.001202s | 0.001202s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001184s | 0.001184s | 0.001194s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001188s | 0.001190s | 0.001166s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001196s | 0.001202s | 0.001217s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001190s | 0.001213s | 0.001200s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001185s | 0.001187s | 0.001193s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001180s | 0.001171s | 0.001205s | byte equality observed |
| variant_issue_55.test | 10 | 0.001201s | 0.001209s | 0.001199s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001231s | 0.001253s | 0.001230s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001238s | 0.001244s | 0.001211s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001193s | 0.001179s | 0.001209s | byte equality observed |
| variant_many_types.test | 10 | 0.001224s | 0.001233s | 0.001212s | byte equality observed |
| variant_move_assign.test | 10 | 0.001194s | 0.001222s | 0.001180s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001208s | 0.001245s | 0.001234s | byte equality observed |
| variant_move_construct.test | 10 | 0.001209s | 0.001215s | 0.001197s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001215s | 0.001237s | 0.001213s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001226s | 0.001238s | 0.001238s | byte equality observed |
| variant_size.test | 10 | 0.001188s | 0.001208s | 0.001189s | byte equality observed |
| variant_special.test | 10 | 0.001265s | 0.001263s | 0.001229s | byte equality observed |
| variant_subset.test | 10 | 0.001219s | 0.001246s | 0.001246s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001190s | 0.001198s | 0.001199s | byte equality observed |
| variant_swap.test | 10 | 0.001201s | 0.001229s | 0.001215s | byte equality observed |
| variant_trivial.test | 10 | 0.001242s | 0.001242s | 0.001239s | byte equality observed |
| variant_value_assign.test | 10 | 0.001180s | 0.001209s | 0.001191s | byte equality observed |
| variant_value_construct.test | 10 | 0.001167s | 0.001225s | 0.001184s | byte equality observed |
| variant_valueless.test | 10 | 0.001244s | 0.001280s | 0.001235s | byte equality observed |
| variant_visit.test | 10 | 0.001216s | 0.001195s | 0.001180s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001204s | 0.001186s | 0.001190s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001175s | 0.001182s | 0.001200s | byte equality observed |
| variant_visit_r.test | 10 | 0.001222s | 0.001183s | 0.001206s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
