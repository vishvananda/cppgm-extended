# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.157276 | 0.320962 | 0.433343 | 0.001761 | 0.000765 | 0.001628 |
| variant_size.test | RUN | 0.264291 | 0.796044 | 0.837923 | 0.001697 | 0.001683 | 0.001630 |
| variant_alternative.test | RUN | 0.334930 | 2.479314 | 1.403179 | 0.001773 | 0.001684 | 0.001566 |
| variant_holds_alternative.test | RUN | 0.245427 | 0.600489 | 0.802716 | 0.001684 | 0.001717 | 0.001657 |
| variant_holds_alternative_cx.test | COMPILE | 0.161039 | 0.349656 | 0.454669 | — | — | — |
| variant_get_by_index.test | RUN | 0.378645 | 2.741930 | 1.727797 | 0.001858 | 0.001834 | 0.001730 |
| variant_get_by_index_cx.test | COMPILE | 0.165201 | 0.362345 | 0.480598 | — | — | — |
| variant_get_by_type.test | RUN | 0.394182 | 2.963054 | 1.856186 | 0.001811 | 0.001735 | 0.001541 |
| variant_get_by_type_cx.test | COMPILE | 0.151438 | 0.309487 | 0.396335 | — | — | — |
| variant_default_construct.test | RUN | 0.381077 | 2.141089 | 1.540357 | 0.001736 | 0.001530 | 0.001730 |
| variant_default_construct_cx.test | COMPILE | 0.163398 | 0.348251 | 0.457918 | — | — | — |
| variant_copy_construct.test | RUN | 0.592099 | 3.654853 | 2.666719 | 0.001690 | 0.001750 | 0.001688 |
| variant_copy_construct_cx.test | COMPILE | 0.184932 | 0.378665 | 0.569658 | — | — | — |
| variant_move_construct.test | RUN | 0.565776 | 3.122223 | 2.447406 | 0.001729 | 0.001766 | 0.001608 |
| variant_move_construct_cx.test | COMPILE | 0.149012 | 0.312117 | 0.423620 | — | — | — |
| variant_value_construct.test | RUN | 0.293938 | 0.825121 | 1.057819 | 0.001648 | 0.001684 | 0.001663 |
| variant_value_construct_cx.test | COMPILE | 0.170753 | 0.358827 | 0.501758 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.308191 | 0.804967 | 1.062081 | 0.001853 | 0.001712 | 0.001514 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.171987 | 0.365115 | 0.481433 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.308619 | 0.806271 | 1.054864 | 0.001692 | 0.001590 | 0.001607 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.171466 | 0.368522 | 0.476169 | — | — | — |
| variant_copy_assign.test | RUN | 0.397818 | 2.510770 | 1.652770 | 0.001710 | 0.001725 | 0.001551 |
| variant_copy_assign_cx.test | COMPILE | 0.152727 | 0.308525 | 0.408131 | — | — | — |
| variant_move_assign.test | RUN | 0.396056 | 2.525741 | 1.622635 | 0.001722 | 0.001868 | 0.001532 |
| variant_move_assign_cx.test | COMPILE | 0.150960 | 0.317392 | 0.406157 | — | — | — |
| variant_value_assign.test | RUN | 0.317170 | 0.909402 | 1.153987 | 0.001499 | 0.001708 | 0.001411 |
| variant_value_assign_cx.test | COMPILE | 0.150253 | 0.316397 | 0.415400 | — | — | — |
| variant_emplace_index.test | RUN | 0.335958 | 1.034580 | 1.266357 | 0.001690 | 0.001728 | 0.001642 |
| variant_emplace_index_cx.test | COMPILE | 0.149981 | 0.314829 | 0.405044 | — | — | — |
| variant_emplace_type.test | RUN | 0.321923 | 0.889162 | 1.127584 | 0.001701 | 0.001522 | 0.001623 |
| variant_emplace_type_cx.test | COMPILE | 0.154359 | 0.318310 | 0.402285 | — | — | — |
| variant_swap.test | RUN | 0.331362 | 1.109273 | 1.218347 | 0.001722 | 0.001724 | 0.001646 |
| variant_eq_ne.test | RUN | 0.272433 | 0.658724 | 0.870443 | 0.001445 | 0.001543 | 0.001496 |
| variant_eq_ne_cx.test | COMPILE | 0.156248 | 0.311468 | 0.415156 | — | — | — |
| variant_lt_gt.test | RUN | 0.280287 | 0.777828 | 0.913539 | 0.001495 | 0.001626 | 0.001687 |
| variant_lt_gt_cx.test | COMPILE | 0.152159 | 0.304542 | 0.388176 | — | — | — |
| variant_destroy.test | RUN | 0.295399 | 0.749611 | 0.933453 | 0.001710 | 0.001710 | 0.001738 |
| variant_visit.test | RUN | 1.580777 | 5.742312 | 15.320994 | 0.001695 | 0.001689 | 0.001621 |
| variant_convert_construct.test | RUN | 0.348309 | 0.971877 | 1.294553 | 0.001839 | 0.001732 | 0.001672 |
| variant_subset.test | RUN | 0.364609 | 1.074336 | 1.350546 | 0.001793 | 0.001787 | 0.001761 |
| variant_valueless.test | RUN | 0.267541 | 0.683944 | 0.932617 | 0.001929 | 0.001713 | 0.001524 |
| variant_copy_construct_throw.test | RUN | 0.213088 | 0.501863 | 0.642579 | 0.001702 | 0.001739 | 0.001722 |
| variant_move_construct_throw.test | RUN | 0.213772 | 0.510605 | 0.650635 | 0.001796 | 0.001758 | 0.001717 |
| variant_convert_construct_throw.test | RUN | 0.225710 | 0.531193 | 0.694259 | 0.001808 | 0.001767 | 0.001683 |
| variant_copy_assign_throw.test | RUN | 0.213118 | 0.485291 | 0.645868 | 0.001695 | 0.001771 | 0.001670 |
| variant_move_assign_throw.test | RUN | 0.209493 | 0.491877 | 0.648250 | 0.001591 | 0.001784 | 0.001686 |
| variant_get_by_index_nx.test | RUN | 0.475593 | 2.339909 | 1.672792 | 0.001830 | 0.001755 | 0.001626 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.173133 | 0.357779 | 0.472015 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.470153 | 2.611198 | 1.787347 | 0.001712 | 0.001533 | 0.001656 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.150794 | 0.309392 | 0.387995 | — | — | — |
| variant_subset_nx.test | RUN | 0.382093 | 1.030610 | 1.391420 | 0.001539 | 0.001670 | 0.001642 |
| variant_hash.test | RUN | 0.372553 | 1.183440 | 1.277173 | 0.001788 | 0.001822 | 0.001641 |
| variant_trivial.test | RUN | 1.940046 | 6.361441 | 9.496379 | 0.001761 | 0.001811 | 0.001426 |
| variant_special.test | RUN | 1.830593 | 6.848743 | 9.932057 | 0.001838 | 0.001877 | 0.001708 |
| variant_visit_derived.test | RUN | 0.250686 | 0.633370 | 0.929008 | 0.001721 | 0.001723 | 0.001714 |
| variant_many_types.test | RUN | 2.024510 | 9.150638 | 13.413272 | 0.001857 | 0.001732 | 0.001535 |
| variant_visit_r.test | RUN | 0.782070 | 2.884970 | 5.112360 | 0.001658 | 0.001723 | 0.001730 |
| variant_derived_construct.test | COMPILE | 0.163236 | 0.304771 | 0.430172 | — | — | — |
| variant_visit_by_index.test | RUN | 0.338564 | 0.771028 | 1.204788 | 0.001632 | 0.001610 | 0.001641 |
| variant_ostream_insert.test | RUN | 0.258597 | 0.669954 | 0.812969 | 0.001838 | 0.001768 | 0.001374 |
| is_output_streamable.test | RUN | 0.300644 | 1.349549 | 1.104632 | 0.001668 | 0.001744 | 0.001513 |
| variant_json_value_from.test | RUN | 0.466763 | 1.115374 | 1.487184 | 0.001797 | 0.001724 | 0.001759 |
| variant_json_value_to.test | RUN | 0.552202 | 1.518501 | 1.984826 | 0.001622 | 0.001812 | 0.001670 |
| variant_uses_double_storage.test | COMPILE | 0.166062 | 0.332738 | 0.499589 | — | — | — |
| variant_derived_construct2.test | RUN | 0.221239 | 0.548470 | 0.695466 | 0.001554 | 0.001704 | 0.001464 |
| variant_visit_cx.test | COMPILE | 0.151221 | 0.314888 | 0.419998 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.150017 | 0.325504 | 0.404825 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.152783 | 0.319792 | 0.404064 | — | — | — |
| variant_index_type_126.test | RUN | 1.305554 | 3.038804 | 6.069162 | 0.001682 | 0.001693 | 0.001768 |
| variant_index_type_127.test | RUN | 1.285033 | 3.051884 | 6.178155 | 0.001675 | 0.001725 | 0.001614 |
| variant_index_type_128.test | RUN | 1.330848 | 3.055046 | 6.252420 | 0.001754 | 0.001679 | 0.001506 |
| variant_index_type_129.test | RUN | 1.306469 | 3.029308 | 6.168040 | 0.001486 | 0.001711 | 0.001512 |
| variant_index_type_254.test | RUN | 4.482272 | 11.649146 | 19.150249 | 0.001648 | 0.001742 | 0.001761 |
| variant_index_type_255.test | RUN | 4.437273 | 11.652689 | 19.363241 | 0.001506 | 0.001760 | 0.001528 |
| variant_index_type_256.test | RUN | 4.478291 | 11.561146 | 19.538818 | 0.001738 | 0.001610 | 0.001663 |
| variant_index_type_257.test | RUN | 4.544536 | 11.729727 | 19.629066 | 0.001707 | 0.001739 | 0.001678 |
| variant_default_construct_cx_2.test | COMPILE | 0.147555 | 0.311652 | 0.440068 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.149978 | 0.311235 | 0.388947 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.153540 | 0.308451 | 0.382459 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.151381 | 0.309841 | 0.408071 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.152266 | 0.309995 | 0.406237 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.150671 | 0.257432 | 0.410327 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.096501 | 0.281453 | 0.408892 | — | — | — |
| variant_issue_55.test | RUN | 0.197757 | 0.529689 | 0.608324 | 0.001234 | 0.001153 | 0.001208 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.998×, 0.988× | 0.999× |
| clang | 1.002× | 0.996×, 1.009× | 0.984× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001174s | 0.001219s | 0.001191s | byte equality observed |
| quick.test | 10 | 0.001183s | 0.000511s | 0.001174s | byte equality observed |
| variant_alternative.test | 10 | 0.001182s | 0.001216s | 0.001224s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001209s | 0.001228s | 0.001211s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001247s | 0.001237s | 0.001261s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001232s | 0.001221s | 0.001223s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001234s | 0.001228s | 0.001232s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001208s | 0.001251s | 0.001216s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001244s | 0.001246s | 0.001235s | byte equality observed |
| variant_default_construct.test | 10 | 0.001221s | 0.001180s | 0.001224s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001204s | 0.001222s | 0.001224s | byte equality observed |
| variant_destroy.test | 10 | 0.001217s | 0.001223s | 0.001187s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001208s | 0.001209s | 0.001228s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001225s | 0.001215s | 0.001223s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001200s | 0.001188s | 0.001213s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001295s | 0.001326s | 0.001284s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001217s | 0.001200s | 0.001213s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001284s | 0.001300s | 0.001287s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001230s | 0.001220s | 0.001204s | byte equality observed |
| variant_hash.test | 9 | 0.001195s | 0.001203s | 0.001215s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001212s | 0.001211s | 0.001198s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001214s | 0.001228s | 0.001206s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001207s | 0.001217s | 0.001234s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001195s | 0.001204s | 0.001206s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001203s | 0.001165s | 0.001209s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001199s | 0.001199s | 0.001186s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001211s | 0.001201s | 0.001212s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001202s | 0.001195s | 0.001197s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001199s | 0.001187s | 0.001184s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001196s | 0.001179s | 0.001182s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001195s | 0.001189s | 0.001220s | byte equality observed |
| variant_issue_55.test | 10 | 0.001196s | 0.001218s | 0.001195s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001247s | 0.001261s | 0.001237s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001220s | 0.001246s | 0.001240s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001179s | 0.001212s | 0.001197s | byte equality observed |
| variant_many_types.test | 10 | 0.001273s | 0.001209s | 0.001238s | byte equality observed |
| variant_move_assign.test | 10 | 0.001218s | 0.001215s | 0.001212s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001235s | 0.001228s | 0.001234s | byte equality observed |
| variant_move_construct.test | 10 | 0.001201s | 0.001237s | 0.001229s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001210s | 0.001252s | 0.001232s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001231s | 0.001236s | 0.001229s | byte equality observed |
| variant_size.test | 10 | 0.001208s | 0.001206s | 0.001198s | byte equality observed |
| variant_special.test | 10 | 0.001271s | 0.001231s | 0.001253s | byte equality observed |
| variant_subset.test | 10 | 0.001234s | 0.001236s | 0.001251s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001204s | 0.001189s | 0.001209s | byte equality observed |
| variant_swap.test | 10 | 0.001220s | 0.001180s | 0.001229s | byte equality observed |
| variant_trivial.test | 10 | 0.001240s | 0.001225s | 0.001224s | byte equality observed |
| variant_value_assign.test | 10 | 0.001215s | 0.001215s | 0.001197s | byte equality observed |
| variant_value_construct.test | 10 | 0.001198s | 0.001188s | 0.001170s | byte equality observed |
| variant_valueless.test | 10 | 0.001250s | 0.001250s | 0.001250s | byte equality observed |
| variant_visit.test | 10 | 0.001211s | 0.001217s | 0.001223s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001180s | 0.001195s | 0.001201s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001223s | 0.001165s | 0.001183s | byte equality observed |
| variant_visit_r.test | 10 | 0.001243s | 0.001168s | 0.001210s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
