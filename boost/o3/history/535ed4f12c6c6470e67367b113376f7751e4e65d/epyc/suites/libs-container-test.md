# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.313911 | 31.608979 | 27.540787 | 0.006338 | 0.005387 | 0.007596 |
| vector_options_test.test | RUN | 0.331740 | 1.401303 | 1.173864 | 0.002000 | 0.002098 | 0.002048 |
| uses_allocator_test.test | RUN | 0.063098 | 0.122715 | 0.168868 | 0.000773 | 0.000783 | 0.001822 |
| uses_allocator_construction_test.test | RUN | 0.198294 | 0.452486 | 0.578271 | 0.001706 | 0.001707 | 0.001743 |
| unsynchronized_pool_resource_test.test | RUN | 0.275383 | 1.319101 | 0.765619 | 0.001932 | 0.001915 | 0.001977 |
| tree_test.test | RUN | 0.965747 | 4.801179 | 3.788393 | 0.001845 | 0.001783 | 0.001872 |
| throw_exception_test.test | RUN | 0.137983 | 0.266678 | 0.396448 | 0.001778 | 0.001607 | 0.001910 |
| synchronized_pool_resource_test.test | RUN | 0.276974 | 1.303937 | 0.757447 | 0.001983 | 0.002024 | 0.001924 |
| string_view_compat_test.test | RUN | 0.450047 | 2.606184 | 1.451584 | 0.001761 | 0.001940 | 0.001933 |
| string_test.test | RUN | 1.670772 | 9.999448 | 7.560930 | 0.016081 | 0.015529 | 0.015648 |
| string_options_test.test | RUN | 0.289418 | 2.096102 | 1.242885 | 0.001982 | 0.001808 | 0.001982 |
| static_vector_test.test | RUN | 1.960265 | 8.362948 | 7.529915 | 0.002586 | 0.001765 | 0.002568 |
| static_vector_options_test.test | RUN | 0.232096 | 0.526133 | 0.702411 | 0.002086 | 0.001939 | 0.001961 |
| stable_vector_test.test | RUN | 3.881219 | 17.304716 | 14.223836 | 0.032256 | 0.031763 | 0.030286 |
| small_vector_test.test | RUN | 2.245744 | 12.014869 | 10.204904 | 0.004342 | 0.003240 | 0.005611 |
| small_vector_options_test.test | RUN | 0.379618 | 1.467958 | 1.330150 | 0.001978 | 0.002046 | 0.002102 |
| slist_test.test | RUN | 2.171582 | 10.820175 | 9.288028 | 0.025820 | 0.025150 | 0.025138 |
| set_test.test | RUN | 3.302614 | 20.482840 | 16.630297 | 0.014214 | 0.013606 | 0.014313 |
| segtor_test.test | RUN | 3.830552 | 21.817564 | 18.303535 | 0.696569 | 0.646666 | 0.656168 |
| segtor_options_test.test | RUN | 0.250997 | 0.646347 | 0.884214 | 0.001920 | 0.001768 | 0.001763 |
| scoped_allocator_usage_test.test | RUN | 0.825247 | 3.366176 | 3.312622 | 0.001653 | 0.001837 | 0.001844 |
| scoped_allocator_adaptor_test.test | RUN | 0.424331 | 0.938218 | 1.486351 | 0.001939 | 0.001974 | 0.001876 |
| resource_adaptor_test.test | RUN | 0.217788 | 0.561319 | 0.629400 | 0.001904 | 0.001896 | 0.001685 |
| polymorphic_allocator_test.test | RUN | 0.186430 | 0.467634 | 0.529690 | 0.001617 | 0.001459 | 0.001838 |
| pmr_vector_test.test | RUN | 0.102184 | 0.181107 | 0.269579 | 0.000761 | 0.000646 | 0.001840 |
| pmr_string_test.test | RUN | 0.206660 | 0.377681 | 0.513976 | 0.000790 | 0.000776 | 0.001749 |
| pmr_stable_vector_test.test | RUN | 0.203364 | 0.598865 | 0.717458 | 0.001786 | 0.001831 | 0.001400 |
| pmr_small_vector_test.test | RUN | 0.117181 | 0.235657 | 0.359572 | 0.001737 | 0.001792 | 0.001353 |
| pmr_slist_test.test | RUN | 0.161897 | 0.796169 | 0.719571 | 0.001551 | 0.001831 | 0.002038 |
| pmr_set_test.test | RUN | 0.179142 | 0.411619 | 0.568258 | 0.001570 | 0.001667 | 0.001644 |
| pmr_nest_test.test | RUN | 0.153115 | 0.286620 | 0.386059 | 0.001721 | 0.001731 | 0.001818 |
| pmr_map_test.test | RUN | 0.188017 | 0.430273 | 0.549437 | 0.001784 | 0.001699 | 0.001784 |
| pmr_list_test.test | RUN | 0.124810 | 0.267511 | 0.357221 | 0.001601 | 0.001894 | 0.001880 |
| pmr_hub_test.test | RUN | 0.187584 | 0.367380 | 0.453801 | 0.001730 | 0.001760 | 0.001870 |
| pmr_flat_set_test.test | RUN | 0.142182 | 0.282520 | 0.392406 | 0.001808 | 0.001674 | 0.001867 |
| pmr_flat_map_test.test | RUN | 0.167682 | 0.324863 | 0.456604 | 0.001758 | 0.001598 | 0.001653 |
| pmr_devector_test.test | RUN | 0.101980 | 0.276632 | 0.354263 | 0.001373 | 0.001622 | 0.001701 |
| pmr_deque_test.test | RUN | 0.112301 | 0.228786 | 0.348344 | 0.001748 | 0.001810 | 0.001679 |
| pair_test.test | RUN | 0.277702 | 0.577475 | 0.715506 | 0.001662 | 0.001923 | 0.001856 |
| null_iterators_test.test | RUN | 0.457774 | 1.085306 | 1.297927 | 0.001854 | 0.001861 | 0.001829 |
| node_handle_test.test | RUN | 0.204714 | 1.322950 | 0.537476 | 0.001575 | 0.001983 | 0.001864 |
| node_allocator_test.test | RUN | 2.278947 | 13.559034 | 11.340044 | 0.010107 | 0.010578 | 0.010846 |
| new_delete_resource_test.test | RUN | 0.247968 | 0.824897 | 0.702734 | 0.002560 | 0.002637 | 0.002444 |
| nest_test.test | RUN | 0.567149 | 3.702715 | 2.561107 | 0.001849 | 0.002017 | 0.001879 |
| nest_api_test.test | RUN | 1.068038 | 6.911687 | 4.870843 | 0.002762 | 0.002800 | 0.002680 |
| monotonic_buffer_resource_test.test | RUN | 0.269765 | 1.438674 | 0.734313 | 0.001844 | 0.001710 | 0.001772 |
| memory_resource_test.test | RUN | 0.162945 | 0.415812 | 0.444997 | 0.001778 | 0.001854 | 0.001770 |
| map_test.test | RUN | 4.925147 | 22.965123 | 24.343730 | 0.011005 | 0.010549 | 0.009639 |
| list_test.test | RUN | 2.310703 | 11.753121 | 10.051936 | 0.024551 | 0.025846 | 0.025527 |
| is_constructible_test.test | RUN | 0.095293 | 0.156584 | 0.203892 | 0.000848 | 0.000815 | 0.001892 |
| insert_vs_emplace_test.test | RUN | 0.266070 | 0.937773 | 1.387239 | 0.001687 | 0.002043 | 0.001822 |
| hub_uses_allocator_test.test | RUN | 0.310232 | 0.810229 | 0.955648 | 0.001891 | 0.001894 | 0.001874 |
| hub_stability_test.test | RUN | 0.568053 | 3.758041 | 3.403950 | 0.002154 | 0.002081 | 0.002211 |
| hub_sort_test.test | RUN | 0.366229 | 1.731574 | 1.650926 | 0.553495 | 0.528360 | 0.531659 |
| hub_pocxx_test.test | RUN | 0.377114 | 1.996300 | 1.152389 | 0.001811 | 0.002011 | 0.001869 |
| hub_new_extended_alignment_test.test | RUN | 0.211193 | 0.629152 | 0.670688 | 0.001822 | 0.001860 | 0.002004 |
| hub_exception_safety_test.test | RUN | 0.771500 | 4.737637 | 3.490025 | 0.091144 | 0.086856 | 0.098415 |
| hub_empty_sort_test.test | RUN | 0.238900 | 0.560879 | 0.816169 | 0.001830 | 0.001892 | 0.001938 |
| hub_api_test.test | RUN | 2.216791 | 19.332896 | 10.980376 | 0.004555 | 0.003721 | 0.003548 |
| global_resource_test.test | RUN | 0.143813 | 0.390723 | 0.386722 | 0.001728 | 0.001760 | 0.001822 |
| flat_tree_test.test | RUN | 3.459959 | 29.057002 | 19.665657 | 0.001802 | 0.001918 | 0.001774 |
| flat_set_test.test | RUN | 4.392218 | 36.387259 | 25.794818 | 0.008424 | 0.007715 | 0.008205 |
| flat_set_adaptor_test.test | RUN | 3.423608 | 32.638112 | 20.504378 | 0.008905 | 0.007355 | 0.008693 |
| flat_map_test.test | RUN | 7.523490 | 54.669972 | 43.070110 | 0.009137 | 0.008785 | 0.008742 |
| flat_map_adaptor_test.test | RUN | 4.749916 | 39.642775 | 30.635134 | 0.008249 | 0.008559 | 0.008449 |
| explicit_inst_vector_test.test | RUN | 0.765624 | 5.382083 | 4.506025 | 0.001734 | 0.001736 | 0.001865 |
| explicit_inst_string_test.test | RUN | 1.099214 | 9.624542 | 6.095969 | 0.001631 | 0.001793 | 0.001816 |
| explicit_inst_static_vector_test.test | RUN | 0.128796 | 0.362014 | 0.483945 | 0.001777 | 0.001882 | 0.001844 |
| explicit_inst_stable_vector_test.test | RUN | 0.605437 | 3.510108 | 2.731541 | 0.001926 | 0.001906 | 0.001648 |
| explicit_inst_small_vector_test.test | RUN | 0.523530 | 3.194719 | 2.673947 | 0.002000 | 0.001880 | 0.001822 |
| explicit_inst_slist_test.test | RUN | 0.476951 | 2.835088 | 2.081449 | 0.001859 | 0.001842 | 0.001809 |
| explicit_inst_set_test.test | RUN | 0.660383 | 3.270276 | 2.494396 | 0.001848 | 0.001844 | 0.001856 |
| explicit_inst_map_test.test | RUN | 0.747891 | 3.462124 | 2.690569 | 0.001610 | 0.001855 | 0.001773 |
| explicit_inst_list_test.test | RUN | 0.163840 | 0.779415 | 0.670278 | 0.001587 | 0.001785 | 0.001730 |
| explicit_inst_flat_set_test.test | RUN | 3.010503 | 24.247820 | 15.536667 | 0.001700 | 0.001934 | 0.001706 |
| explicit_inst_flat_map_test.test | RUN | 3.394174 | 25.342132 | 16.992980 | 0.001836 | 0.001896 | 0.001553 |
| explicit_inst_devector_test.test | RUN | 0.427937 | 3.041755 | 2.230448 | 0.001848 | 0.001543 | 0.001801 |
| explicit_inst_deque_test.test | RUN | 0.494664 | 3.336560 | 2.325811 | 0.001833 | 0.001933 | 0.001874 |
| devector_test.test | RUN | 6.614230 | 38.490269 | 30.190657 | 0.004262 | 0.003615 | 0.004755 |
| devector_options_test.test | RUN | 0.328755 | 1.875690 | 1.254662 | 0.002098 | 0.002100 | 0.001990 |
| deque_test.test | RUN | 4.135894 | 24.663315 | 18.641792 | 0.680924 | 0.353385 | 0.422866 |
| deque_options_test.test | RUN | 0.253231 | 0.681436 | 0.880547 | 0.001669 | 0.001982 | 0.001900 |
| copy_move_algo_test.test | RUN | 0.237817 | 0.803115 | 0.790415 | 0.002092 | 0.001714 | 0.001652 |
| common_iterator_test.test | RUN | 0.020691 | 0.045318 | 0.043790 | 0.000764 | 0.000748 | 0.001839 |
| boost_iterator_comp_test.test | RUN | 0.354537 | 0.982448 | 1.227412 | 0.001828 | 0.001926 | 0.001772 |
| allocator_traits_test.test | RUN | 0.225430 | 0.447034 | 0.623715 | 0.001574 | 0.001788 | 0.001876 |
| alloc_full_test.test | RUN | 0.213265 | 0.904642 | 0.856874 | 0.004877 | 0.004911 | 0.004828 |
| alloc_basic_test.test | RUN | 0.191851 | 0.646867 | 0.650897 | 0.001700 | 0.001924 | 0.001934 |
| adaptive_pool_test.test | RUN | 1.889053 | 13.825032 | 7.461661 | 0.012081 | 0.013344 | 0.010443 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.946× | 0.951×, 0.940× | 1.008× |
| clang | 0.972× | 0.958×, 0.986× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.010707s | 0.009802s | 0.010603s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001109s | 0.001087s | 0.001081s | byte equality observed |
| alloc_full_test.test | 7 | 0.003012s | 0.003041s | 0.003143s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001020s | 0.001017s | 0.001009s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001060s | 0.001052s | 0.001063s | byte equality observed |
| common_iterator_test.test | 10 | 0.000487s | 0.000493s | 0.001040s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001279s | 0.001143s | 0.001202s | byte equality observed |
| deque_options_test.test | 10 | 0.001092s | 0.001079s | 0.001099s | byte equality observed |
| deque_test.test | 1 | 0.398438s | 0.397420s | 0.459825s | byte equality observed |
| devector_options_test.test | 10 | 0.001169s | 0.001125s | 0.001117s | byte equality observed |
| devector_test.test | 5 | 0.003641s | 0.003153s | 0.004325s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000976s | 0.000998s | 0.000976s | byte equality observed |
| explicit_inst_devector_test.test | 5 | 0.000955s | 0.001014s | 0.000973s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000976s | 0.000990s | 0.000978s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000970s | 0.000992s | 0.000977s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000971s | 0.000975s | 0.000963s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000989s | 0.001003s | 0.000973s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000972s | 0.001000s | 0.000974s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000978s | 0.000983s | 0.000962s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000984s | 0.000994s | 0.000968s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000982s | 0.000992s | 0.000996s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000965s | 0.000976s | 0.000965s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000979s | 0.000984s | 0.000971s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000981s | 0.000991s | 0.000969s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.005944s | 0.005644s | 0.005791s | byte equality observed |
| flat_map_test.test | 3 | 0.006464s | 0.006060s | 0.005988s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.005935s | 0.005558s | 0.005940s | byte equality observed |
| flat_set_test.test | 4 | 0.006010s | 0.005200s | 0.005839s | byte equality observed |
| flat_tree_test.test | 10 | 0.000991s | 0.000992s | 0.000970s | byte equality observed |
| global_resource_test.test | 10 | 0.001028s | 0.001027s | 0.001026s | byte equality observed |
| hub_api_test.test | 6 | 0.003157s | 0.002244s | 0.002211s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001005s | 0.000994s | 0.000998s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.070381s | 0.064143s | 0.065681s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001055s | 0.001050s | 0.001037s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001044s | 0.001034s | 0.001030s | byte equality observed |
| hub_sort_test.test | 1 | 0.459588s | 0.433896s | 0.424441s | byte equality observed |
| hub_stability_test.test | 10 | 0.001254s | 0.001240s | 0.001261s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000993s | 0.001019s | 0.001006s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001026s | 0.001031s | 0.001028s | byte equality observed |
| is_constructible_test.test | 10 | 0.000473s | 0.000477s | 0.000969s | byte equality observed |
| list_test.test | 2 | 0.017671s | 0.019334s | 0.017614s | byte equality observed |
| map_test.test | 2 | 0.007722s | 0.007763s | 0.007826s | byte equality observed |
| memory_resource_test.test | 10 | 0.001026s | 0.001008s | 0.001014s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001020s | 0.001027s | 0.001013s | byte equality observed |
| nest_api_test.test | 10 | 0.001706s | 0.001647s | 0.001632s | byte equality observed |
| nest_test.test | 10 | 0.001041s | 0.001060s | 0.001019s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001468s | 0.001528s | 0.001452s | byte equality observed |
| node_allocator_test.test | 3 | 0.007187s | 0.007216s | 0.007259s | byte equality observed |
| node_handle_test.test | 10 | 0.001028s | 0.001043s | 0.001017s | byte equality observed |
| null_iterators_test.test | 10 | 0.001016s | 0.000993s | 0.001015s | byte equality observed |
| pair_test.test | 10 | 0.001004s | 0.001013s | 0.001030s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000996s | 0.000987s | 0.000992s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000992s | 0.000990s | 0.001027s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000985s | 0.000980s | 0.001001s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000986s | 0.000981s | 0.000976s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000985s | 0.000993s | 0.000975s | byte equality observed |
| pmr_list_test.test | 10 | 0.000987s | 0.000984s | 0.000988s | byte equality observed |
| pmr_map_test.test | 10 | 0.000985s | 0.000986s | 0.000986s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000995s | 0.000995s | 0.000991s | byte equality observed |
| pmr_set_test.test | 10 | 0.000982s | 0.001007s | 0.000981s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000991s | 0.000974s | 0.001002s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000990s | 0.000982s | 0.000986s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000995s | 0.000995s | 0.000997s | byte equality observed |
| pmr_string_test.test | 10 | 0.000483s | 0.000477s | 0.001014s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000481s | 0.000482s | 0.001022s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001052s | 0.001062s | 0.001024s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001027s | 0.001036s | 0.001004s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001049s | 0.001033s | 0.001012s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001047s | 0.001046s | 0.001047s | byte equality observed |
| segtor_options_test.test | 10 | 0.001060s | 0.001057s | 0.001048s | byte equality observed |
| segtor_test.test | 1 | 0.440365s | 0.377339s | 0.372008s | byte equality observed |
| set_test.test | 2 | 0.011038s | 0.009687s | 0.011046s | byte equality observed |
| slist_test.test | 2 | 0.017207s | 0.017040s | 0.016994s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001099s | 0.001100s | 0.001095s | byte equality observed |
| small_vector_test.test | 6 | 0.002490s | 0.001953s | 0.003604s | byte equality observed |
| stable_vector_test.test | 1 | 0.025699s | 0.025598s | 0.026474s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001062s | 0.001050s | 0.001071s | byte equality observed |
| static_vector_test.test | 10 | 0.001394s | 0.001275s | 0.001333s | byte equality observed |
| string_options_test.test | 10 | 0.001036s | 0.001057s | 0.001046s | byte equality observed |
| string_test.test | 2 | 0.011096s | 0.011993s | 0.011439s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001007s | 0.001021s | 0.000999s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001040s | 0.001060s | 0.001055s | byte equality observed |
| throw_exception_test.test | 10 | 0.000970s | 0.000980s | 0.000989s | byte equality observed |
| tree_test.test | 10 | 0.000974s | 0.000986s | 0.000984s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001023s | 0.001072s | 0.001063s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000968s | 0.000989s | 0.000997s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000456s | 0.000481s | 0.000965s | byte equality observed |
| vector_options_test.test | 10 | 0.001139s | 0.001140s | 0.001127s | byte equality observed |
| vector_test.test | 4 | 0.003992s | 0.003461s | 0.005130s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
