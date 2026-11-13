# libs/variant2/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| quick.test | RUN | 0.112093 | 0.243695 | 0.381562 | 0.001578 | 0.000802 | 0.001752 |
| variant_size.test | RUN | 0.187909 | 0.595887 | 0.695568 | 0.001718 | 0.001774 | 0.001689 |
| variant_alternative.test | RUN | 0.250449 | 1.890982 | 1.192581 | 0.001747 | 0.001852 | 0.001720 |
| variant_holds_alternative.test | RUN | 0.182012 | 0.466518 | 0.675409 | 0.001609 | 0.001754 | 0.001650 |
| variant_holds_alternative_cx.test | COMPILE | 0.118167 | 0.271679 | 0.400261 | — | — | — |
| variant_get_by_index.test | RUN | 0.287500 | 2.108209 | 1.406808 | 0.001846 | 0.001918 | 0.001897 |
| variant_get_by_index_cx.test | COMPILE | 0.123559 | 0.276206 | 0.422142 | — | — | — |
| variant_get_by_type.test | RUN | 0.297875 | 2.220694 | 1.509962 | 0.001841 | 0.001650 | 0.001688 |
| variant_get_by_type_cx.test | COMPILE | 0.112850 | 0.242330 | 0.352384 | — | — | — |
| variant_default_construct.test | RUN | 0.288088 | 1.625824 | 1.216137 | 0.001637 | 0.001852 | 0.001792 |
| variant_default_construct_cx.test | COMPILE | 0.123375 | 0.261420 | 0.407547 | — | — | — |
| variant_copy_construct.test | RUN | 0.437075 | 2.795432 | 2.177795 | 0.001535 | 0.001780 | 0.001826 |
| variant_copy_construct_cx.test | COMPILE | 0.135675 | 0.300486 | 0.489169 | — | — | — |
| variant_move_construct.test | RUN | 0.405918 | 2.375001 | 1.999472 | 0.001824 | 0.001470 | 0.001596 |
| variant_move_construct_cx.test | COMPILE | 0.110427 | 0.249733 | 0.353611 | — | — | — |
| variant_value_construct.test | RUN | 0.222976 | 0.634753 | 0.873818 | 0.001610 | 0.001736 | 0.001654 |
| variant_value_construct_cx.test | COMPILE | 0.121484 | 0.278206 | 0.394084 | — | — | — |
| variant_in_place_index_construct.test | RUN | 0.231531 | 0.608209 | 0.881375 | 0.001852 | 0.001647 | 0.001538 |
| variant_in_place_index_construct_cx.test | COMPILE | 0.122744 | 0.287426 | 0.396824 | — | — | — |
| variant_in_place_type_construct.test | RUN | 0.229331 | 0.603679 | 0.873305 | 0.001688 | 0.001840 | 0.001402 |
| variant_in_place_type_construct_cx.test | COMPILE | 0.124357 | 0.288310 | 0.398812 | — | — | — |
| variant_copy_assign.test | RUN | 0.305855 | 1.901175 | 1.389122 | 0.001630 | 0.001842 | 0.001690 |
| variant_copy_assign_cx.test | COMPILE | 0.109419 | 0.245327 | 0.329910 | — | — | — |
| variant_move_assign.test | RUN | 0.302668 | 1.893751 | 1.340105 | 0.001788 | 0.001836 | 0.001823 |
| variant_move_assign_cx.test | COMPILE | 0.107604 | 0.244998 | 0.349071 | — | — | — |
| variant_value_assign.test | RUN | 0.233990 | 0.677936 | 0.964562 | 0.001634 | 0.001723 | 0.001613 |
| variant_value_assign_cx.test | COMPILE | 0.108462 | 0.236486 | 0.356638 | — | — | — |
| variant_emplace_index.test | RUN | 0.257442 | 0.798822 | 1.043645 | 0.001756 | 0.001693 | 0.001757 |
| variant_emplace_index_cx.test | COMPILE | 0.109235 | 0.248645 | 0.308309 | — | — | — |
| variant_emplace_type.test | RUN | 0.233350 | 0.672138 | 0.931202 | 0.001615 | 0.001848 | 0.001716 |
| variant_emplace_type_cx.test | COMPILE | 0.107758 | 0.238472 | 0.330778 | — | — | — |
| variant_swap.test | RUN | 0.241475 | 0.832094 | 1.030462 | 0.001428 | 0.001883 | 0.001696 |
| variant_eq_ne.test | RUN | 0.205635 | 0.504464 | 0.751691 | 0.001576 | 0.001761 | 0.001435 |
| variant_eq_ne_cx.test | COMPILE | 0.107369 | 0.249048 | 0.307913 | — | — | — |
| variant_lt_gt.test | RUN | 0.208844 | 0.590713 | 0.779175 | 0.001820 | 0.001892 | 0.001673 |
| variant_lt_gt_cx.test | COMPILE | 0.106849 | 0.235961 | 0.356710 | — | — | — |
| variant_destroy.test | RUN | 0.207375 | 0.573379 | 0.800649 | 0.001802 | 0.001858 | 0.002086 |
| variant_visit.test | RUN | 1.183197 | 4.512450 | 12.323467 | 0.001592 | 0.001848 | 0.001838 |
| variant_convert_construct.test | RUN | 0.258230 | 0.782217 | 1.094815 | 0.002048 | 0.001854 | 0.002148 |
| variant_subset.test | RUN | 0.263970 | 0.811510 | 1.142967 | 0.001730 | 0.001794 | 0.001794 |
| variant_valueless.test | RUN | 0.197514 | 0.551597 | 0.775746 | 0.001600 | 0.001912 | 0.001824 |
| variant_copy_construct_throw.test | RUN | 0.159926 | 0.393398 | 0.551859 | 0.001849 | 0.001732 | 0.001863 |
| variant_move_construct_throw.test | RUN | 0.153366 | 0.400995 | 0.568451 | 0.001652 | 0.001373 | 0.001740 |
| variant_convert_construct_throw.test | RUN | 0.160379 | 0.435660 | 0.584349 | 0.001594 | 0.001786 | 0.001848 |
| variant_copy_assign_throw.test | RUN | 0.156570 | 0.376770 | 0.574110 | 0.001834 | 0.001854 | 0.001771 |
| variant_move_assign_throw.test | RUN | 0.154429 | 0.372715 | 0.562096 | 0.001558 | 0.001757 | 0.001644 |
| variant_get_by_index_nx.test | RUN | 0.358467 | 1.797382 | 1.447991 | 0.001539 | 0.001780 | 0.001648 |
| variant_get_by_index_cx_nx.test | COMPILE | 0.120280 | 0.265824 | 0.419704 | — | — | — |
| variant_get_by_type_nx.test | RUN | 0.359172 | 2.005234 | 1.461800 | 0.001646 | 0.001698 | 0.001768 |
| variant_get_by_type_cx_nx.test | COMPILE | 0.107572 | 0.254870 | 0.357124 | — | — | — |
| variant_subset_nx.test | RUN | 0.277021 | 0.819914 | 1.177889 | 0.001681 | 0.001762 | 0.001711 |
| variant_hash.test | RUN | 0.265052 | 0.900801 | 1.088409 | 0.001586 | 0.001858 | 0.001760 |
| variant_trivial.test | RUN | 1.611437 | 5.157401 | 7.771491 | 0.001595 | 0.001874 | 0.001708 |
| variant_special.test | RUN | 1.520951 | 5.380207 | 8.529078 | 0.002004 | 0.001916 | 0.001866 |
| variant_visit_derived.test | RUN | 0.190031 | 0.494148 | 0.798310 | 0.001795 | 0.001877 | 0.001758 |
| variant_many_types.test | RUN | 1.596447 | 7.087471 | 11.134436 | 0.001685 | 0.001776 | 0.001808 |
| variant_visit_r.test | RUN | 0.635174 | 2.165378 | 4.072966 | 0.001770 | 0.001720 | 0.001768 |
| variant_derived_construct.test | COMPILE | 0.109635 | 0.239261 | 0.366693 | — | — | — |
| variant_visit_by_index.test | RUN | 0.245610 | 0.663252 | 0.988418 | 0.001538 | 0.001836 | 0.001802 |
| variant_ostream_insert.test | RUN | 0.179666 | 0.506323 | 0.680470 | 0.001631 | 0.001700 | 0.001837 |
| is_output_streamable.test | RUN | 0.219018 | 1.012247 | 0.922792 | 0.001439 | 0.001774 | 0.001740 |
| variant_json_value_from.test | RUN | 0.340968 | 0.895544 | 1.192357 | 0.001838 | 0.001956 | 0.001803 |
| variant_json_value_to.test | RUN | 0.422927 | 1.140065 | 1.648679 | 0.001801 | 0.001437 | 0.001790 |
| variant_uses_double_storage.test | COMPILE | 0.116250 | 0.249970 | 0.421127 | — | — | — |
| variant_derived_construct2.test | RUN | 0.157258 | 0.420831 | 0.598178 | 0.001492 | 0.001845 | 0.001678 |
| variant_visit_cx.test | COMPILE | 0.111728 | 0.248827 | 0.356114 | — | — | — |
| variant_visit_cx_2.test | COMPILE | 0.106066 | 0.249796 | 0.357353 | — | — | — |
| variant_visit_r_cx.test | COMPILE | 0.108138 | 0.239403 | 0.312136 | — | — | — |
| variant_index_type_126.test | RUN | 1.050048 | 2.397407 | 5.143684 | 0.001764 | 0.001631 | 0.001526 |
| variant_index_type_127.test | RUN | 1.051597 | 2.406802 | 5.174167 | 0.001700 | 0.001526 | 0.001787 |
| variant_index_type_128.test | RUN | 1.077291 | 2.435563 | 5.256865 | 0.001404 | 0.001504 | 0.001643 |
| variant_index_type_129.test | RUN | 1.079520 | 2.504399 | 5.232043 | 0.001734 | 0.001719 | 0.001814 |
| variant_index_type_254.test | RUN | 3.733595 | 8.966995 | 15.769304 | 0.001732 | 0.001857 | 0.001825 |
| variant_index_type_255.test | RUN | 3.710614 | 9.149685 | 15.700738 | 0.001541 | 0.001316 | 0.001837 |
| variant_index_type_256.test | RUN | 3.679802 | 9.238094 | 15.876327 | 0.001490 | 0.001899 | 0.001842 |
| variant_index_type_257.test | RUN | 3.719322 | 9.275223 | 15.913159 | 0.001482 | 0.001865 | 0.001498 |
| variant_default_construct_cx_2.test | COMPILE | 0.110186 | 0.267658 | 0.290658 | — | — | — |
| variant_default_construct_cx_3.test | COMPILE | 0.107664 | 0.242237 | 0.322232 | — | — | — |
| variant_default_construct_cx_4.test | COMPILE | 0.107688 | 0.239758 | 0.317423 | — | — | — |
| variant_default_construct_cx_5.test | COMPILE | 0.107323 | 0.250238 | 0.314777 | — | — | — |
| variant_value_construct_cx_2.test | COMPILE | 0.108724 | 0.245648 | 0.344390 | — | — | — |
| variant_value_construct_cx_3.test | COMPILE | 0.107026 | 0.240032 | 0.329047 | — | — | — |
| variant_value_construct_cx_4.test | COMPILE | 0.094702 | 0.203071 | 0.331324 | — | — | — |
| variant_issue_55.test | RUN | 0.172497 | 0.397172 | 0.550084 | 0.001294 | 0.001310 | 0.001200 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 0.999×, 0.992× | 0.996× |
| clang | 0.998× | 0.997×, 0.999× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| is_output_streamable.test | 10 | 0.001156s | 0.001193s | 0.001177s | byte equality observed |
| quick.test | 10 | 0.001127s | 0.000513s | 0.001138s | byte equality observed |
| variant_alternative.test | 10 | 0.001155s | 0.001162s | 0.001157s | byte equality observed |
| variant_convert_construct.test | 10 | 0.001143s | 0.001174s | 0.001104s | byte equality observed |
| variant_convert_construct_throw.test | 10 | 0.001168s | 0.001206s | 0.001195s | byte equality observed |
| variant_copy_assign.test | 10 | 0.001161s | 0.001179s | 0.001147s | byte equality observed |
| variant_copy_assign_throw.test | 10 | 0.001195s | 0.001186s | 0.001189s | byte equality observed |
| variant_copy_construct.test | 10 | 0.001143s | 0.001168s | 0.001163s | byte equality observed |
| variant_copy_construct_throw.test | 10 | 0.001181s | 0.001171s | 0.001192s | byte equality observed |
| variant_default_construct.test | 10 | 0.001152s | 0.001148s | 0.001150s | byte equality observed |
| variant_derived_construct2.test | 10 | 0.001149s | 0.001127s | 0.001155s | byte equality observed |
| variant_destroy.test | 10 | 0.001155s | 0.001153s | 0.001144s | byte equality observed |
| variant_emplace_index.test | 10 | 0.001161s | 0.001166s | 0.001147s | byte equality observed |
| variant_emplace_type.test | 10 | 0.001158s | 0.001148s | 0.001182s | byte equality observed |
| variant_eq_ne.test | 10 | 0.001148s | 0.001141s | 0.001177s | byte equality observed |
| variant_get_by_index.test | 10 | 0.001229s | 0.001251s | 0.001229s | byte equality observed |
| variant_get_by_index_nx.test | 10 | 0.001168s | 0.001159s | 0.001155s | byte equality observed |
| variant_get_by_type.test | 10 | 0.001209s | 0.001261s | 0.001236s | byte equality observed |
| variant_get_by_type_nx.test | 10 | 0.001166s | 0.001190s | 0.001155s | byte equality observed |
| variant_hash.test | 10 | 0.001158s | 0.001150s | 0.001149s | byte equality observed |
| variant_holds_alternative.test | 10 | 0.001138s | 0.001149s | 0.001118s | byte equality observed |
| variant_in_place_index_construct.test | 10 | 0.001152s | 0.001180s | 0.001167s | byte equality observed |
| variant_in_place_type_construct.test | 10 | 0.001157s | 0.001170s | 0.001164s | byte equality observed |
| variant_index_type_126.test | 10 | 0.001167s | 0.001134s | 0.001162s | byte equality observed |
| variant_index_type_127.test | 10 | 0.001137s | 0.001136s | 0.001150s | byte equality observed |
| variant_index_type_128.test | 10 | 0.001146s | 0.001145s | 0.001156s | byte equality observed |
| variant_index_type_129.test | 10 | 0.001154s | 0.001138s | 0.001142s | byte equality observed |
| variant_index_type_254.test | 10 | 0.001160s | 0.001151s | 0.001166s | byte equality observed |
| variant_index_type_255.test | 10 | 0.001161s | 0.001164s | 0.001153s | byte equality observed |
| variant_index_type_256.test | 10 | 0.001142s | 0.001142s | 0.001152s | byte equality observed |
| variant_index_type_257.test | 10 | 0.001145s | 0.001140s | 0.001157s | byte equality observed |
| variant_issue_55.test | 10 | 0.001156s | 0.001170s | 0.001167s | byte equality observed |
| variant_json_value_from.test | 10 | 0.001192s | 0.001213s | 0.001188s | byte equality observed |
| variant_json_value_to.test | 10 | 0.001200s | 0.001191s | 0.001156s | byte equality observed |
| variant_lt_gt.test | 10 | 0.001148s | 0.001163s | 0.001148s | byte equality observed |
| variant_many_types.test | 10 | 0.001204s | 0.001156s | 0.001199s | byte equality observed |
| variant_move_assign.test | 10 | 0.001172s | 0.001163s | 0.001131s | byte equality observed |
| variant_move_assign_throw.test | 10 | 0.001182s | 0.001168s | 0.001188s | byte equality observed |
| variant_move_construct.test | 10 | 0.001152s | 0.001178s | 0.001154s | byte equality observed |
| variant_move_construct_throw.test | 10 | 0.001185s | 0.001185s | 0.001174s | byte equality observed |
| variant_ostream_insert.test | 10 | 0.001192s | 0.001180s | 0.001165s | byte equality observed |
| variant_size.test | 10 | 0.001147s | 0.001159s | 0.001169s | byte equality observed |
| variant_special.test | 10 | 0.001216s | 0.001207s | 0.001210s | byte equality observed |
| variant_subset.test | 10 | 0.001182s | 0.001185s | 0.001153s | byte equality observed |
| variant_subset_nx.test | 10 | 0.001138s | 0.001158s | 0.001151s | byte equality observed |
| variant_swap.test | 10 | 0.001165s | 0.001185s | 0.001137s | byte equality observed |
| variant_trivial.test | 10 | 0.001186s | 0.001210s | 0.001172s | byte equality observed |
| variant_value_assign.test | 10 | 0.001151s | 0.001175s | 0.001141s | byte equality observed |
| variant_value_construct.test | 10 | 0.001141s | 0.001167s | 0.001149s | byte equality observed |
| variant_valueless.test | 10 | 0.001194s | 0.001224s | 0.001197s | byte equality observed |
| variant_visit.test | 10 | 0.001158s | 0.001182s | 0.001181s | byte equality observed |
| variant_visit_by_index.test | 10 | 0.001145s | 0.001143s | 0.001130s | byte equality observed |
| variant_visit_derived.test | 10 | 0.001138s | 0.001153s | 0.001153s | byte equality observed |
| variant_visit_r.test | 10 | 0.001146s | 0.001129s | 0.001179s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
