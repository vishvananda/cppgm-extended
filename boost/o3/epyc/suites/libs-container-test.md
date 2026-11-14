# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 6.737296 | 32.582099 | 27.228415 | 0.005881 | 0.005472 | 0.007443 |
| vector_options_test.test | RUN | 0.317629 | 1.442804 | 1.242871 | 0.001848 | 0.002034 | 0.001988 |
| uses_allocator_test.test | RUN | 0.058330 | 0.123156 | 0.162776 | 0.000659 | 0.000789 | 0.001789 |
| uses_allocator_construction_test.test | RUN | 0.174168 | 0.460255 | 0.520142 | 0.001634 | 0.001986 | 0.001768 |
| unsynchronized_pool_resource_test.test | RUN | 0.216937 | 1.362791 | 0.700195 | 0.001666 | 0.002027 | 0.001860 |
| tree_test.test | RUN | 0.846814 | 4.910927 | 3.801147 | 0.001618 | 0.001937 | 0.001460 |
| throw_exception_test.test | RUN | 0.120396 | 0.291594 | 0.383474 | 0.001477 | 0.001910 | 0.001714 |
| synchronized_pool_resource_test.test | RUN | 0.221522 | 1.384111 | 0.695165 | 0.001810 | 0.001982 | 0.001798 |
| string_view_compat_test.test | RUN | 0.329094 | 2.698888 | 1.363293 | 0.001434 | 0.001912 | 0.001754 |
| string_test.test | RUN | 1.544355 | 10.315445 | 7.292520 | 0.015782 | 0.015468 | 0.015498 |
| string_options_test.test | RUN | 0.282978 | 2.144426 | 1.201415 | 0.001743 | 0.001976 | 0.001708 |
| static_vector_test.test | RUN | 1.698257 | 8.623069 | 7.307218 | 0.002246 | 0.002499 | 0.002232 |
| static_vector_options_test.test | RUN | 0.217181 | 0.528020 | 0.698460 | 0.001853 | 0.001890 | 0.001830 |
| stable_vector_test.test | RUN | 3.725575 | 17.718966 | 13.939351 | 0.031103 | 0.031610 | 0.033012 |
| small_vector_test.test | RUN | 2.570431 | 12.441133 | 10.104938 | 0.003704 | 0.003490 | 0.005841 |
| small_vector_options_test.test | RUN | 0.361136 | 1.542898 | 1.352735 | 0.001924 | 0.002066 | 0.001710 |
| slist_test.test | RUN | 2.361328 | 11.118190 | 9.030976 | 0.025183 | 0.024993 | 0.024827 |
| set_test.test | RUN | 3.572560 | 21.172846 | 16.382246 | 0.015216 | 0.013951 | 0.015233 |
| segtor_test.test | RUN | 4.553136 | 22.438376 | 18.254158 | 0.693371 | 0.642660 | 0.657104 |
| segtor_options_test.test | RUN | 0.250760 | 0.654240 | 0.809464 | 0.002163 | 0.001924 | 0.001784 |
| scoped_allocator_usage_test.test | RUN | 0.862237 | 3.441209 | 3.087721 | 0.001816 | 0.001982 | 0.001766 |
| scoped_allocator_adaptor_test.test | RUN | 0.381961 | 0.997615 | 1.425249 | 0.001805 | 0.001567 | 0.001834 |
| resource_adaptor_test.test | RUN | 0.191010 | 0.596534 | 0.664953 | 0.001631 | 0.001722 | 0.001738 |
| polymorphic_allocator_test.test | RUN | 0.168280 | 0.475361 | 0.512100 | 0.001590 | 0.001907 | 0.001820 |
| pmr_vector_test.test | RUN | 0.090317 | 0.187589 | 0.294351 | 0.000623 | 0.000811 | 0.001637 |
| pmr_string_test.test | RUN | 0.180227 | 0.382101 | 0.508278 | 0.000698 | 0.000830 | 0.001632 |
| pmr_stable_vector_test.test | RUN | 0.201663 | 0.600749 | 0.736021 | 0.001774 | 0.001795 | 0.001646 |
| pmr_small_vector_test.test | RUN | 0.105908 | 0.251831 | 0.329654 | 0.001778 | 0.001876 | 0.001742 |
| pmr_slist_test.test | RUN | 0.145628 | 0.768333 | 0.713102 | 0.001814 | 0.001867 | 0.001572 |
| pmr_set_test.test | RUN | 0.178136 | 0.418910 | 0.486487 | 0.001784 | 0.001771 | 0.001673 |
| pmr_nest_test.test | RUN | 0.134603 | 0.275734 | 0.356243 | 0.001592 | 0.001830 | 0.001786 |
| pmr_map_test.test | RUN | 0.182254 | 0.421754 | 0.475802 | 0.001773 | 0.001648 | 0.001708 |
| pmr_list_test.test | RUN | 0.118543 | 0.275431 | 0.391426 | 0.001690 | 0.001810 | 0.001774 |
| pmr_hub_test.test | RUN | 0.172113 | 0.368182 | 0.492951 | 0.001533 | 0.001846 | 0.001697 |
| pmr_flat_set_test.test | RUN | 0.133215 | 0.288429 | 0.373508 | 0.001773 | 0.001696 | 0.001652 |
| pmr_flat_map_test.test | RUN | 0.160590 | 0.323552 | 0.492039 | 0.001593 | 0.001612 | 0.001748 |
| pmr_devector_test.test | RUN | 0.103336 | 0.285972 | 0.342221 | 0.001520 | 0.001661 | 0.001776 |
| pmr_deque_test.test | RUN | 0.101246 | 0.235693 | 0.344804 | 0.001698 | 0.001888 | 0.001774 |
| pair_test.test | RUN | 0.255036 | 0.583639 | 0.792728 | 0.001797 | 0.001970 | 0.001784 |
| null_iterators_test.test | RUN | 0.392735 | 1.116023 | 1.305553 | 0.001645 | 0.001779 | 0.001830 |
| node_handle_test.test | RUN | 0.170932 | 1.387691 | 0.550993 | 0.001583 | 0.001946 | 0.001472 |
| node_allocator_test.test | RUN | 2.780807 | 13.682912 | 11.189452 | 0.010435 | 0.010320 | 0.010386 |
| new_delete_resource_test.test | RUN | 0.203888 | 0.841380 | 0.759995 | 0.002404 | 0.002713 | 0.002266 |
| nest_test.test | RUN | 0.545795 | 3.734917 | 2.506311 | 0.001862 | 0.001896 | 0.001810 |
| nest_api_test.test | RUN | 0.932499 | 7.102622 | 4.915239 | 0.003031 | 0.002759 | 0.002688 |
| monotonic_buffer_resource_test.test | RUN | 0.217394 | 1.428012 | 0.731875 | 0.001822 | 0.001816 | 0.001417 |
| memory_resource_test.test | RUN | 0.134675 | 0.426601 | 0.437965 | 0.001800 | 0.001682 | 0.001735 |
| map_test.test | RUN | 4.880959 | 23.346440 | 23.774844 | 0.011499 | 0.011538 | 0.010665 |
| list_test.test | RUN | 2.511565 | 12.005167 | 9.840541 | 0.023417 | 0.025684 | 0.025686 |
| is_constructible_test.test | RUN | 0.082797 | 0.158712 | 0.252603 | 0.000737 | 0.000757 | 0.001888 |
| insert_vs_emplace_test.test | RUN | 0.266731 | 0.995551 | 1.332808 | 0.001766 | 0.001903 | 0.001892 |
| hub_uses_allocator_test.test | RUN | 0.276057 | 0.808242 | 0.974443 | 0.001700 | 0.001957 | 0.001768 |
| hub_stability_test.test | RUN | 0.588760 | 3.887023 | 3.314774 | 0.002136 | 0.002212 | 0.002132 |
| hub_sort_test.test | RUN | 0.375282 | 1.741227 | 1.556740 | 0.537876 | 0.518522 | 0.504568 |
| hub_pocxx_test.test | RUN | 0.362582 | 1.996141 | 1.193293 | 0.001794 | 0.002048 | 0.001742 |
| hub_new_extended_alignment_test.test | RUN | 0.197164 | 0.620019 | 0.639034 | 0.001770 | 0.001726 | 0.001726 |
| hub_exception_safety_test.test | RUN | 0.701755 | 4.707193 | 3.381718 | 0.092548 | 0.090230 | 0.094729 |
| hub_empty_sort_test.test | RUN | 0.221553 | 0.575354 | 0.737119 | 0.001804 | 0.001841 | 0.001685 |
| hub_api_test.test | RUN | 2.453092 | 19.549717 | 10.874987 | 0.004298 | 0.003572 | 0.003658 |
| global_resource_test.test | RUN | 0.125034 | 0.386392 | 0.384791 | 0.001953 | 0.001849 | 0.001852 |
| flat_tree_test.test | RUN | 4.399332 | 29.501141 | 19.639404 | 0.001690 | 0.001846 | 0.001659 |
| flat_set_test.test | RUN | 5.526078 | 36.901643 | 25.895819 | 0.008043 | 0.007583 | 0.008077 |
| flat_set_adaptor_test.test | RUN | 4.202782 | 32.710293 | 20.343360 | 0.008210 | 0.007750 | 0.008577 |
| flat_map_test.test | RUN | 9.128882 | 54.520066 | 42.585639 | 0.009796 | 0.008784 | 0.008580 |
| flat_map_adaptor_test.test | RUN | 5.813034 | 39.622832 | 30.208105 | 0.008489 | 0.008018 | 0.007138 |
| explicit_inst_vector_test.test | RUN | 0.858519 | 5.470097 | 4.496401 | 0.001684 | 0.001790 | 0.001877 |
| explicit_inst_string_test.test | RUN | 1.115103 | 9.446734 | 6.048776 | 0.001604 | 0.001735 | 0.001562 |
| explicit_inst_static_vector_test.test | RUN | 0.114454 | 0.357191 | 0.453608 | 0.001393 | 0.001701 | 0.001750 |
| explicit_inst_stable_vector_test.test | RUN | 0.702465 | 3.407853 | 2.709046 | 0.001744 | 0.001776 | 0.001767 |
| explicit_inst_small_vector_test.test | RUN | 0.591814 | 3.012993 | 2.653578 | 0.001610 | 0.001744 | 0.001718 |
| explicit_inst_slist_test.test | RUN | 0.453973 | 2.803056 | 2.067066 | 0.001665 | 0.001761 | 0.001742 |
| explicit_inst_set_test.test | RUN | 0.639988 | 3.229073 | 2.441950 | 0.001703 | 0.001802 | 0.001854 |
| explicit_inst_map_test.test | RUN | 0.701132 | 3.476905 | 2.633917 | 0.001879 | 0.001831 | 0.001628 |
| explicit_inst_list_test.test | RUN | 0.161524 | 0.799674 | 0.679780 | 0.001786 | 0.001815 | 0.001690 |
| explicit_inst_flat_set_test.test | RUN | 3.713914 | 24.573569 | 15.461750 | 0.001786 | 0.001892 | 0.001800 |
| explicit_inst_flat_map_test.test | RUN | 3.949935 | 25.556085 | 17.027771 | 0.001683 | 0.001828 | 0.001752 |
| explicit_inst_devector_test.test | RUN | 0.469838 | 3.132859 | 2.213365 | 0.001732 | 0.001770 | 0.001728 |
| explicit_inst_deque_test.test | RUN | 0.506592 | 3.407677 | 2.320161 | 0.001810 | 0.001830 | 0.001668 |
| devector_test.test | RUN | 7.069671 | 38.157647 | 27.505883 | 0.003697 | 0.003534 | 0.004770 |
| devector_options_test.test | RUN | 0.322044 | 1.809877 | 1.242557 | 0.002112 | 0.001996 | 0.002016 |
| deque_test.test | RUN | 4.873580 | 25.154334 | 19.900099 | 0.513465 | 0.358482 | 0.422935 |
| deque_options_test.test | RUN | 0.231681 | 0.678970 | 0.943192 | 0.001802 | 0.001970 | 0.001931 |
| copy_move_algo_test.test | RUN | 0.225737 | 0.837925 | 0.855837 | 0.002028 | 0.001906 | 0.001723 |
| common_iterator_test.test | RUN | 0.018657 | 0.046470 | 0.050058 | 0.000739 | 0.000782 | 0.001502 |
| boost_iterator_comp_test.test | RUN | 0.337700 | 0.997042 | 1.045668 | 0.001757 | 0.001469 | 0.001674 |
| allocator_traits_test.test | RUN | 0.202296 | 0.458033 | 0.664353 | 0.001754 | 0.001762 | 0.001803 |
| alloc_full_test.test | RUN | 0.213497 | 0.908333 | 0.776986 | 0.004698 | 0.004992 | 0.004992 |
| alloc_basic_test.test | RUN | 0.176164 | 0.655901 | 0.642236 | 0.001704 | 0.001881 | 0.001872 |
| adaptive_pool_test.test | RUN | 2.070034 | 14.361159 | 11.539545 | 0.010255 | 0.013552 | 0.014740 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.884× | 0.889×, 0.879× | 1.016× |
| clang | 0.919× | 0.910×, 0.927× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.010325s | 0.009715s | 0.010461s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001085s | 0.001085s | 0.001069s | byte equality observed |
| alloc_full_test.test | 7 | 0.003002s | 0.002983s | 0.003083s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000992s | 0.000982s | 0.000980s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001043s | 0.001029s | 0.001015s | byte equality observed |
| common_iterator_test.test | 10 | 0.000464s | 0.000466s | 0.000971s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001250s | 0.001112s | 0.001130s | byte equality observed |
| deque_options_test.test | 10 | 0.001053s | 0.001040s | 0.001089s | byte equality observed |
| deque_test.test | 1 | 0.532898s | 0.389392s | 0.460755s | byte equality observed |
| devector_options_test.test | 10 | 0.001163s | 0.001102s | 0.001095s | byte equality observed |
| devector_test.test | 5 | 0.003206s | 0.003119s | 0.004292s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000954s | 0.000950s | 0.000944s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000947s | 0.000940s | 0.000941s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000966s | 0.000946s | 0.000954s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000955s | 0.000954s | 0.000940s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000942s | 0.000935s | 0.000933s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000965s | 0.000962s | 0.000946s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000960s | 0.000974s | 0.000940s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000959s | 0.000961s | 0.000931s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000961s | 0.000966s | 0.000942s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000971s | 0.000968s | 0.000948s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000943s | 0.000947s | 0.000945s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000960s | 0.000961s | 0.000989s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000974s | 0.000956s | 0.000952s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005925s | 0.005638s | 0.005753s | byte equality observed |
| flat_map_test.test | 3 | 0.006387s | 0.006073s | 0.005989s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.005946s | 0.005382s | 0.005958s | byte equality observed |
| flat_set_test.test | 4 | 0.005974s | 0.005234s | 0.005861s | byte equality observed |
| flat_tree_test.test | 10 | 0.000968s | 0.000970s | 0.000960s | byte equality observed |
| global_resource_test.test | 10 | 0.001002s | 0.000998s | 0.000989s | byte equality observed |
| hub_api_test.test | 7 | 0.002721s | 0.002217s | 0.002226s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000973s | 0.000980s | 0.000975s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.066343s | 0.064620s | 0.063309s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001033s | 0.001115s | 0.001031s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001005s | 0.001023s | 0.000993s | byte equality observed |
| hub_sort_test.test | 1 | 0.437231s | 0.440423s | 0.421602s | byte equality observed |
| hub_stability_test.test | 10 | 0.001265s | 0.001199s | 0.001220s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000976s | 0.000975s | 0.000970s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001000s | 0.000998s | 0.000995s | byte equality observed |
| is_constructible_test.test | 10 | 0.000451s | 0.000451s | 0.000945s | byte equality observed |
| list_test.test | 2 | 0.017307s | 0.017475s | 0.017730s | byte equality observed |
| map_test.test | 3 | 0.007882s | 0.007849s | 0.007829s | byte equality observed |
| memory_resource_test.test | 10 | 0.000998s | 0.001006s | 0.000982s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001003s | 0.000991s | 0.000982s | byte equality observed |
| nest_api_test.test | 10 | 0.001600s | 0.001618s | 0.001637s | byte equality observed |
| nest_test.test | 10 | 0.001005s | 0.001012s | 0.001010s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001422s | 0.001435s | 0.001424s | byte equality observed |
| node_allocator_test.test | 3 | 0.007156s | 0.007136s | 0.007163s | byte equality observed |
| node_handle_test.test | 10 | 0.000982s | 0.000995s | 0.000986s | byte equality observed |
| null_iterators_test.test | 10 | 0.000983s | 0.000984s | 0.000986s | byte equality observed |
| pair_test.test | 10 | 0.000974s | 0.000986s | 0.000969s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000948s | 0.000956s | 0.000965s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000957s | 0.000937s | 0.000962s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000949s | 0.000940s | 0.000963s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000976s | 0.000967s | 0.000967s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000944s | 0.000962s | 0.000962s | byte equality observed |
| pmr_list_test.test | 10 | 0.000944s | 0.000951s | 0.000965s | byte equality observed |
| pmr_map_test.test | 10 | 0.000965s | 0.000959s | 0.000959s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000980s | 0.000952s | 0.000969s | byte equality observed |
| pmr_set_test.test | 10 | 0.000977s | 0.000949s | 0.000989s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000961s | 0.000961s | 0.000963s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001022s | 0.000939s | 0.000955s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001006s | 0.000962s | 0.000984s | byte equality observed |
| pmr_string_test.test | 10 | 0.000466s | 0.000450s | 0.000983s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000472s | 0.000458s | 0.000977s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001022s | 0.001011s | 0.001001s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001002s | 0.001006s | 0.001053s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001003s | 0.001024s | 0.001029s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001008s | 0.001003s | 0.001025s | byte equality observed |
| segtor_options_test.test | 10 | 0.001029s | 0.001045s | 0.001020s | byte equality observed |
| segtor_test.test | 1 | 0.422786s | 0.374837s | 0.373743s | byte equality observed |
| set_test.test | 2 | 0.010985s | 0.009658s | 0.011096s | byte equality observed |
| slist_test.test | 1 | 0.017028s | 0.016987s | 0.016916s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001099s | 0.001044s | 0.001059s | byte equality observed |
| small_vector_test.test | 6 | 0.002101s | 0.001931s | 0.003559s | byte equality observed |
| stable_vector_test.test | 1 | 0.025261s | 0.025439s | 0.026459s | byte equality observed |
| static_vector_options_test.test | 9 | 0.001056s | 0.001019s | 0.001038s | byte equality observed |
| static_vector_test.test | 10 | 0.001313s | 0.001262s | 0.001276s | byte equality observed |
| string_options_test.test | 10 | 0.001029s | 0.001045s | 0.001003s | byte equality observed |
| string_test.test | 2 | 0.011075s | 0.011970s | 0.011498s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000987s | 0.001008s | 0.000974s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001022s | 0.001032s | 0.001007s | byte equality observed |
| throw_exception_test.test | 10 | 0.000960s | 0.000965s | 0.000961s | byte equality observed |
| tree_test.test | 10 | 0.000973s | 0.000970s | 0.000966s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001034s | 0.001042s | 0.001019s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000945s | 0.000961s | 0.000960s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000433s | 0.000450s | 0.000940s | byte equality observed |
| vector_options_test.test | 10 | 0.001109s | 0.001118s | 0.001097s | byte equality observed |
| vector_test.test | 4 | 0.003566s | 0.003440s | 0.004899s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
