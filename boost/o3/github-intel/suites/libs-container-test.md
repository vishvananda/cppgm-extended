# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 7.985684 | 40.657962 | 32.743094 | 0.007319 | 0.006165 | 0.008515 |
| vector_options_test.test | RUN | 0.402897 | 1.807833 | 1.315066 | 0.002043 | 0.002014 | 0.001968 |
| uses_allocator_test.test | RUN | 0.070452 | 0.140429 | 0.167178 | 0.000580 | 0.000648 | 0.001641 |
| uses_allocator_construction_test.test | RUN | 0.206351 | 0.528626 | 0.600330 | 0.001646 | 0.001830 | 0.001591 |
| unsynchronized_pool_resource_test.test | RUN | 0.328195 | 1.638470 | 0.782543 | 0.001493 | 0.001851 | 0.001776 |
| tree_test.test | RUN | 1.145457 | 6.121616 | 4.598375 | 0.001621 | 0.001837 | 0.001678 |
| throw_exception_test.test | RUN | 0.153835 | 0.349150 | 0.411735 | 0.001706 | 0.001823 | 0.001633 |
| synchronized_pool_resource_test.test | RUN | 0.328596 | 1.638587 | 0.791368 | 0.001360 | 0.001979 | 0.001787 |
| string_view_compat_test.test | RUN | 0.520543 | 3.353011 | 1.547302 | 0.001662 | 0.001763 | 0.001695 |
| string_test.test | RUN | 2.325894 | 12.877330 | 8.674633 | 0.017632 | 0.017595 | 0.018337 |
| string_options_test.test | RUN | 0.348148 | 2.710571 | 1.245832 | 0.001882 | 0.002056 | 0.001856 |
| static_vector_test.test | RUN | 2.650415 | 10.672495 | 8.722484 | 0.002576 | 0.002403 | 0.002430 |
| static_vector_options_test.test | RUN | 0.254270 | 0.697084 | 0.736117 | 0.001873 | 0.001940 | 0.001740 |
| stable_vector_test.test | RUN | 5.884562 | 22.318471 | 16.804131 | 0.042081 | 0.042382 | 0.043016 |
| small_vector_test.test | RUN | 3.151683 | 15.598766 | 12.126954 | 0.004987 | 0.003729 | 0.006246 |
| small_vector_options_test.test | RUN | 0.446365 | 1.842464 | 1.575074 | 0.001958 | 0.001926 | 0.001913 |
| slist_test.test | RUN | 2.536382 | 13.989315 | 10.804946 | 0.031521 | 0.030698 | 0.030383 |
| set_test.test | RUN | 4.250849 | 26.059294 | 19.607638 | 0.019259 | 0.017570 | 0.018338 |
| segtor_test.test | RUN | 6.399682 | 28.455661 | 21.740655 | 0.718019 | 0.661763 | 0.712512 |
| segtor_options_test.test | RUN | 0.295329 | 0.821756 | 0.926113 | 0.001780 | 0.001827 | 0.001836 |
| scoped_allocator_usage_test.test | RUN | 1.048478 | 4.221756 | 3.569266 | 0.001778 | 0.001807 | 0.001373 |
| scoped_allocator_adaptor_test.test | RUN | 0.478851 | 1.177827 | 1.549628 | 0.001887 | 0.001822 | 0.001441 |
| resource_adaptor_test.test | RUN | 0.249163 | 0.699404 | 0.679234 | 0.001811 | 0.001744 | 0.001727 |
| polymorphic_allocator_test.test | RUN | 0.220196 | 0.569114 | 0.567534 | 0.001592 | 0.001846 | 0.001676 |
| pmr_vector_test.test | RUN | 0.114424 | 0.203931 | 0.291100 | 0.000727 | 0.000775 | 0.001677 |
| pmr_string_test.test | RUN | 0.222838 | 0.432520 | 0.549916 | 0.000702 | 0.000790 | 0.001749 |
| pmr_stable_vector_test.test | RUN | 0.252959 | 0.723784 | 0.810127 | 0.001837 | 0.001623 | 0.001608 |
| pmr_small_vector_test.test | RUN | 0.134624 | 0.284614 | 0.381469 | 0.001742 | 0.001117 | 0.001668 |
| pmr_slist_test.test | RUN | 0.192658 | 0.978012 | 0.811698 | 0.001670 | 0.001776 | 0.001567 |
| pmr_set_test.test | RUN | 0.213287 | 0.487227 | 0.595944 | 0.001448 | 0.001741 | 0.001672 |
| pmr_nest_test.test | RUN | 0.174924 | 0.340258 | 0.400721 | 0.001616 | 0.001752 | 0.001613 |
| pmr_map_test.test | RUN | 0.221815 | 0.512642 | 0.598480 | 0.001867 | 0.001763 | 0.001693 |
| pmr_list_test.test | RUN | 0.150486 | 0.312360 | 0.381627 | 0.001579 | 0.001849 | 0.001716 |
| pmr_hub_test.test | RUN | 0.205360 | 0.424103 | 0.483931 | 0.001736 | 0.001506 | 0.001696 |
| pmr_flat_set_test.test | RUN | 0.164011 | 0.340580 | 0.422838 | 0.001698 | 0.001739 | 0.001676 |
| pmr_flat_map_test.test | RUN | 0.197156 | 0.389270 | 0.494353 | 0.001475 | 0.001506 | 0.001622 |
| pmr_devector_test.test | RUN | 0.125568 | 0.341001 | 0.376816 | 0.001729 | 0.001821 | 0.001681 |
| pmr_deque_test.test | RUN | 0.129010 | 0.282833 | 0.373494 | 0.001780 | 0.001847 | 0.001640 |
| pair_test.test | RUN | 0.309642 | 0.676273 | 0.811292 | 0.001718 | 0.001854 | 0.001933 |
| null_iterators_test.test | RUN | 0.533534 | 1.325313 | 1.409232 | 0.002775 | 0.002023 | 0.001640 |
| node_handle_test.test | RUN | 0.240730 | 1.678136 | 0.547419 | 0.001890 | 0.001929 | 0.001643 |
| node_allocator_test.test | RUN | 3.157721 | 17.562811 | 13.576983 | 0.012534 | 0.012007 | 0.012832 |
| new_delete_resource_test.test | RUN | 0.289329 | 1.025168 | 0.780467 | 0.002426 | 0.002677 | 0.002392 |
| nest_test.test | RUN | 0.763722 | 4.776824 | 2.825285 | 0.001805 | 0.001953 | 0.001742 |
| nest_api_test.test | RUN | 1.508416 | 8.998591 | 5.713654 | 0.003197 | 0.003141 | 0.003004 |
| monotonic_buffer_resource_test.test | RUN | 0.317878 | 1.804158 | 0.759411 | 0.001615 | 0.001906 | 0.001606 |
| memory_resource_test.test | RUN | 0.183648 | 0.496409 | 0.374519 | 0.001567 | 0.001788 | 0.001753 |
| map_test.test | RUN | 6.526657 | 29.233241 | 28.173262 | 0.014300 | 0.013833 | 0.013053 |
| list_test.test | RUN | 2.896423 | 15.041214 | 11.657867 | 0.030260 | 0.029170 | 0.030078 |
| is_constructible_test.test | RUN | 0.102482 | 0.181791 | 0.258303 | 0.000754 | 0.000804 | 0.001679 |
| insert_vs_emplace_test.test | RUN | 0.302652 | 1.192227 | 1.509210 | 0.001674 | 0.001803 | 0.001775 |
| hub_uses_allocator_test.test | RUN | 0.363294 | 1.017802 | 1.057733 | 0.001776 | 0.001817 | 0.001770 |
| hub_stability_test.test | RUN | 0.733045 | 4.818352 | 3.930743 | 0.002175 | 0.002116 | 0.002126 |
| hub_sort_test.test | RUN | 0.473053 | 2.194216 | 1.938409 | 0.778738 | 0.744099 | 0.706120 |
| hub_pocxx_test.test | RUN | 0.465296 | 2.572356 | 1.405170 | 0.001793 | 0.001806 | 0.001687 |
| hub_new_extended_alignment_test.test | RUN | 0.255462 | 0.753527 | 0.745092 | 0.001730 | 0.001804 | 0.001796 |
| hub_exception_safety_test.test | RUN | 1.016590 | 6.024809 | 4.001654 | 0.103401 | 0.100253 | 0.100273 |
| hub_empty_sort_test.test | RUN | 0.277320 | 0.693804 | 0.822020 | 0.001789 | 0.001838 | 0.001774 |
| hub_api_test.test | RUN | 2.970823 | 24.394751 | 12.665400 | 0.005671 | 0.004336 | 0.003968 |
| global_resource_test.test | RUN | 0.162005 | 0.477400 | 0.427075 | 0.001713 | 0.001830 | 0.001750 |
| flat_tree_test.test | RUN | 5.492708 | 37.379859 | 23.984306 | 0.001731 | 0.001802 | 0.001648 |
| flat_set_test.test | RUN | 6.672292 | 47.051792 | 31.567449 | 0.010858 | 0.008623 | 0.009722 |
| flat_set_adaptor_test.test | RUN | 5.322207 | 41.555348 | 24.474869 | 0.010768 | 0.009640 | 0.009762 |
| flat_map_test.test | RUN | 11.638594 | 70.000520 | 51.735565 | 0.011945 | 0.011191 | 0.010556 |
| flat_map_adaptor_test.test | RUN | 7.576572 | 50.946988 | 36.530902 | 0.011072 | 0.010101 | 0.010486 |
| explicit_inst_vector_test.test | RUN | 1.114259 | 6.990403 | 5.451659 | 0.001734 | 0.001798 | 0.001588 |
| explicit_inst_string_test.test | RUN | 1.517135 | 12.254334 | 7.489554 | 0.001864 | 0.001742 | 0.001698 |
| explicit_inst_static_vector_test.test | RUN | 0.145387 | 0.444368 | 0.496148 | 0.001700 | 0.001716 | 0.001733 |
| explicit_inst_stable_vector_test.test | RUN | 0.972167 | 4.395585 | 3.410989 | 0.001708 | 0.001726 | 0.001825 |
| explicit_inst_small_vector_test.test | RUN | 0.760096 | 3.997863 | 3.188290 | 0.001806 | 0.001729 | 0.001737 |
| explicit_inst_slist_test.test | RUN | 0.564608 | 3.620058 | 2.454236 | 0.001728 | 0.001832 | 0.001762 |
| explicit_inst_set_test.test | RUN | 0.858129 | 4.145428 | 2.853620 | 0.003336 | 0.001764 | 0.001650 |
| explicit_inst_map_test.test | RUN | 0.977300 | 4.339351 | 3.135385 | 0.001728 | 0.001670 | 0.001748 |
| explicit_inst_list_test.test | RUN | 0.198025 | 1.004512 | 0.796863 | 0.001670 | 0.001706 | 0.001752 |
| explicit_inst_flat_set_test.test | RUN | 4.886948 | 31.723375 | 18.907651 | 0.001768 | 0.001846 | 0.001685 |
| explicit_inst_flat_map_test.test | RUN | 5.465753 | 33.025800 | 21.097395 | 0.001674 | 0.001732 | 0.001736 |
| explicit_inst_devector_test.test | RUN | 0.620117 | 3.963243 | 2.640798 | 0.001609 | 0.001716 | 0.001756 |
| explicit_inst_deque_test.test | RUN | 0.654770 | 4.276432 | 2.840864 | 0.001864 | 0.001835 | 0.001714 |
| devector_test.test | RUN | 9.612804 | 50.578771 | 38.039923 | 0.004940 | 0.004097 | 0.005748 |
| devector_options_test.test | RUN | 0.395058 | 2.341351 | 1.430171 | 0.002192 | 0.002175 | 0.001960 |
| deque_test.test | RUN | 6.804962 | 29.779529 | 19.517401 | 0.417386 | 0.419017 | 0.562430 |
| deque_options_test.test | RUN | 0.311247 | 0.863724 | 1.040968 | 0.001854 | 0.001696 | 0.001819 |
| copy_move_algo_test.test | RUN | 0.331878 | 1.045999 | 0.870796 | 0.002303 | 0.001918 | 0.001945 |
| common_iterator_test.test | RUN | 0.021937 | 0.047443 | 0.045519 | 0.000845 | 0.000753 | 0.001724 |
| boost_iterator_comp_test.test | RUN | 0.417641 | 1.234100 | 1.324379 | 0.001792 | 0.001838 | 0.001705 |
| allocator_traits_test.test | RUN | 0.245969 | 0.535820 | 0.640146 | 0.001763 | 0.001833 | 0.001735 |
| alloc_full_test.test | RUN | 0.270590 | 1.126634 | 0.970982 | 0.005218 | 0.005357 | 0.005396 |
| alloc_basic_test.test | RUN | 0.236828 | 0.807639 | 0.722388 | 0.001658 | 0.001852 | 0.001884 |
| adaptive_pool_test.test | RUN | 2.306202 | 12.354890 | 13.908253 | 0.012156 | 0.010807 | 0.016501 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.958× | 0.951×, 0.964× | 0.999× |
| clang | 1.038× | 1.034×, 1.042× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.012382s | 0.010974s | 0.011956s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001108s | 0.001101s | 0.001079s | byte equality observed |
| alloc_full_test.test | 6 | 0.003616s | 0.003554s | 0.003683s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001000s | 0.001003s | 0.000987s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001054s | 0.001055s | 0.001022s | byte equality observed |
| common_iterator_test.test | 10 | 0.000476s | 0.000496s | 0.001026s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001546s | 0.001221s | 0.001325s | byte equality observed |
| deque_options_test.test | 10 | 0.001099s | 0.001143s | 0.001066s | byte equality observed |
| deque_test.test | 1 | 0.430386s | 0.436088s | 0.527637s | byte equality observed |
| devector_options_test.test | 10 | 0.001198s | 0.001118s | 0.001122s | byte equality observed |
| devector_test.test | 4 | 0.004289s | 0.003789s | 0.005419s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000972s | 0.000965s | 0.000960s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000957s | 0.000957s | 0.000943s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000962s | 0.000992s | 0.000955s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000966s | 0.000973s | 0.000953s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000961s | 0.000955s | 0.000950s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000969s | 0.000968s | 0.000956s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000973s | 0.000965s | 0.000953s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000963s | 0.000971s | 0.000950s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000965s | 0.000978s | 0.000962s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000969s | 0.000958s | 0.000964s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000957s | 0.000965s | 0.000950s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000970s | 0.000972s | 0.000957s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000969s | 0.000956s | 0.000965s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.007581s | 0.007139s | 0.007089s | byte equality observed |
| flat_map_test.test | 3 | 0.008405s | 0.007798s | 0.007380s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.007497s | 0.006742s | 0.007047s | byte equality observed |
| flat_set_test.test | 3 | 0.007507s | 0.006512s | 0.006889s | byte equality observed |
| flat_tree_test.test | 10 | 0.000983s | 0.000976s | 0.000971s | byte equality observed |
| global_resource_test.test | 10 | 0.001017s | 0.001024s | 0.000990s | byte equality observed |
| hub_api_test.test | 5 | 0.003729s | 0.002854s | 0.002717s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000992s | 0.000996s | 0.000973s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.071524s | 0.072984s | 0.073917s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001030s | 0.001098s | 0.001053s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001058s | 0.001115s | 0.000996s | byte equality observed |
| hub_sort_test.test | 1 | 0.612415s | 0.588143s | 0.565062s | byte equality observed |
| hub_stability_test.test | 10 | 0.001267s | 0.001259s | 0.001358s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000989s | 0.000997s | 0.000997s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001033s | 0.001045s | 0.001033s | byte equality observed |
| is_constructible_test.test | 10 | 0.000462s | 0.000460s | 0.000960s | byte equality observed |
| list_test.test | 1 | 0.020217s | 0.020307s | 0.020713s | byte equality observed |
| map_test.test | 2 | 0.010038s | 0.009959s | 0.009702s | byte equality observed |
| memory_resource_test.test | 10 | 0.001043s | 0.001036s | 0.001024s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001025s | 0.001036s | 0.001023s | byte equality observed |
| nest_api_test.test | 8 | 0.002066s | 0.002050s | 0.002063s | byte equality observed |
| nest_test.test | 10 | 0.001049s | 0.001050s | 0.001047s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001508s | 0.001493s | 0.001512s | byte equality observed |
| node_allocator_test.test | 3 | 0.008492s | 0.008640s | 0.008883s | byte equality observed |
| node_handle_test.test | 10 | 0.001024s | 0.001024s | 0.001030s | byte equality observed |
| null_iterators_test.test | 10 | 0.001011s | 0.000999s | 0.001003s | byte equality observed |
| pair_test.test | 10 | 0.001000s | 0.000998s | 0.000993s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000984s | 0.000973s | 0.000977s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000979s | 0.000975s | 0.000964s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000969s | 0.000968s | 0.000961s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000975s | 0.000968s | 0.000970s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000993s | 0.000980s | 0.000954s | byte equality observed |
| pmr_list_test.test | 10 | 0.000976s | 0.000973s | 0.000952s | byte equality observed |
| pmr_map_test.test | 10 | 0.000965s | 0.000978s | 0.000959s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000965s | 0.000971s | 0.000969s | byte equality observed |
| pmr_set_test.test | 10 | 0.000984s | 0.000960s | 0.000975s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000976s | 0.000974s | 0.000961s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000958s | 0.000961s | 0.000962s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000973s | 0.000972s | 0.000964s | byte equality observed |
| pmr_string_test.test | 10 | 0.000455s | 0.000460s | 0.000951s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000461s | 0.000462s | 0.000948s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001009s | 0.001027s | 0.001008s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001002s | 0.000995s | 0.001016s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001008s | 0.000968s | 0.001040s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000988s | 0.000963s | 0.001066s | byte equality observed |
| segtor_options_test.test | 10 | 0.001052s | 0.001018s | 0.001083s | byte equality observed |
| segtor_test.test | 1 | 0.428808s | 0.384569s | 0.436103s | byte equality observed |
| set_test.test | 2 | 0.014318s | 0.012723s | 0.013718s | byte equality observed |
| slist_test.test | 1 | 0.021130s | 0.021086s | 0.020867s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001132s | 0.001135s | 0.001137s | byte equality observed |
| small_vector_test.test | 5 | 0.002786s | 0.002120s | 0.004531s | byte equality observed |
| stable_vector_test.test | 1 | 0.035580s | 0.035829s | 0.037090s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001111s | 0.001080s | 0.001127s | byte equality observed |
| static_vector_test.test | 10 | 0.001505s | 0.001325s | 0.001404s | byte equality observed |
| string_options_test.test | 10 | 0.001054s | 0.001058s | 0.001069s | byte equality observed |
| string_test.test | 2 | 0.012821s | 0.012260s | 0.013045s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000999s | 0.001043s | 0.001021s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001052s | 0.001062s | 0.001061s | byte equality observed |
| throw_exception_test.test | 10 | 0.000978s | 0.000995s | 0.000986s | byte equality observed |
| tree_test.test | 10 | 0.000966s | 0.000980s | 0.000970s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001050s | 0.001054s | 0.001038s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000975s | 0.000990s | 0.000992s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000444s | 0.000454s | 0.000956s | byte equality observed |
| vector_options_test.test | 10 | 0.001151s | 0.001176s | 0.001161s | byte equality observed |
| vector_test.test | 3 | 0.004682s | 0.004221s | 0.006105s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
