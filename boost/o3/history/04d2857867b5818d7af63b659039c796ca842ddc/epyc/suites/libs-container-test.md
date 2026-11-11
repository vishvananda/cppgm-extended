# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 4.688796 | 30.847198 | 23.962226 | 0.004970 | 0.005233 | 0.006208 |
| vector_options_test.test | RUN | 0.297787 | 1.262364 | 0.912217 | 0.002018 | 0.002052 | 0.002114 |
| uses_allocator_test.test | RUN | 0.053751 | 0.104922 | 0.118037 | 0.000932 | 0.000922 | 0.001857 |
| uses_allocator_construction_test.test | RUN | 0.171824 | 0.390716 | 0.419422 | 0.002122 | 0.001894 | 0.001794 |
| unsynchronized_pool_resource_test.test | RUN | 0.232424 | 1.234274 | 0.597568 | 0.002106 | 0.002032 | 0.001795 |
| tree_test.test | RUN | 0.814553 | 4.620663 | 3.230837 | 0.001861 | 0.001902 | 0.001812 |
| throw_exception_test.test | RUN | 0.126808 | 0.254780 | 0.289521 | 0.002215 | 0.001919 | 0.001801 |
| synchronized_pool_resource_test.test | RUN | 0.244762 | 1.323392 | 0.554826 | 0.002061 | 0.002021 | 0.001822 |
| string_view_compat_test.test | RUN | 0.385660 | 2.445763 | 1.084286 | 0.001812 | 0.001944 | 0.001865 |
| string_test.test | RUN | 1.520205 | 9.721473 | 6.216336 | 0.015189 | 0.012824 | 0.012723 |
| string_options_test.test | RUN | 0.280257 | 2.001358 | 0.952865 | 0.001941 | 0.002120 | 0.002156 |
| static_vector_test.test | RUN | 1.701659 | 8.158240 | 6.427244 | 0.002538 | 0.002356 | 0.002350 |
| static_vector_options_test.test | RUN | 0.223944 | 0.555161 | 0.527651 | 0.001886 | 0.002132 | 0.002020 |
| stable_vector_test.test | RUN | 3.359779 | 16.986691 | 12.363343 | 0.025603 | 0.025493 | 0.025874 |
| small_vector_test.test | RUN | 1.987981 | 11.836410 | 8.861315 | 0.003300 | 0.003190 | 0.004121 |
| small_vector_options_test.test | RUN | 0.334112 | 1.419383 | 1.146385 | 0.001740 | 0.002131 | 0.001852 |
| slist_test.test | RUN | 1.947267 | 10.609092 | 7.859657 | 0.020153 | 0.020225 | 0.019780 |
| set_test.test | RUN | 2.902513 | 21.184105 | 14.634666 | 0.013588 | 0.012172 | 0.012421 |
| segtor_test.test | RUN | 3.364205 | 21.821346 | 15.960309 | 0.503309 | 0.467767 | 0.400391 |
| segtor_options_test.test | RUN | 0.226799 | 0.617936 | 0.658855 | 0.001902 | 0.002148 | 0.001946 |
| scoped_allocator_usage_test.test | RUN | 0.757535 | 3.180083 | 2.599233 | 0.001498 | 0.002270 | 0.001798 |
| scoped_allocator_adaptor_test.test | RUN | 0.389761 | 0.935084 | 1.064676 | 0.001950 | 0.001876 | 0.001755 |
| resource_adaptor_test.test | RUN | 0.202148 | 0.516968 | 0.457943 | 0.001723 | 0.002013 | 0.001676 |
| polymorphic_allocator_test.test | RUN | 0.170652 | 0.447022 | 0.382365 | 0.001770 | 0.001898 | 0.001382 |
| pmr_vector_test.test | RUN | 0.088537 | 0.186419 | 0.204866 | 0.000524 | 0.000667 | 0.001472 |
| pmr_string_test.test | RUN | 0.184114 | 0.344006 | 0.388518 | 0.000891 | 0.000862 | 0.001864 |
| pmr_stable_vector_test.test | RUN | 0.191668 | 0.549965 | 0.627245 | 0.001666 | 0.001937 | 0.001752 |
| pmr_small_vector_test.test | RUN | 0.107237 | 0.220131 | 0.276840 | 0.001848 | 0.001952 | 0.001725 |
| pmr_slist_test.test | RUN | 0.144386 | 0.736255 | 0.549312 | 0.001762 | 0.001941 | 0.001718 |
| pmr_set_test.test | RUN | 0.167436 | 0.373564 | 0.422510 | 0.001538 | 0.001851 | 0.001612 |
| pmr_nest_test.test | RUN | 0.131955 | 0.252186 | 0.237494 | 0.001540 | 0.002050 | 0.001739 |
| pmr_map_test.test | RUN | 0.170441 | 0.355141 | 0.394596 | 0.001783 | 0.001857 | 0.001720 |
| pmr_list_test.test | RUN | 0.109415 | 0.238987 | 0.289145 | 0.001652 | 0.002018 | 0.001775 |
| pmr_hub_test.test | RUN | 0.165062 | 0.306812 | 0.378073 | 0.001799 | 0.001772 | 0.001776 |
| pmr_flat_set_test.test | RUN | 0.125127 | 0.242668 | 0.327120 | 0.001724 | 0.001898 | 0.001910 |
| pmr_flat_map_test.test | RUN | 0.149134 | 0.279625 | 0.351523 | 0.001521 | 0.001737 | 0.001962 |
| pmr_devector_test.test | RUN | 0.098985 | 0.239571 | 0.278111 | 0.001650 | 0.001856 | 0.001786 |
| pmr_deque_test.test | RUN | 0.101173 | 0.207431 | 0.246740 | 0.002017 | 0.001976 | 0.001713 |
| pair_test.test | RUN | 0.258392 | 0.503476 | 0.602742 | 0.001650 | 0.001955 | 0.001849 |
| null_iterators_test.test | RUN | 0.410584 | 1.009976 | 0.931572 | 0.002257 | 0.002018 | 0.001650 |
| node_handle_test.test | RUN | 0.178332 | 1.323652 | 0.406887 | 0.001952 | 0.001984 | 0.001746 |
| node_allocator_test.test | RUN | 2.066641 | 12.913884 | 10.064615 | 0.009238 | 0.009075 | 0.008893 |
| new_delete_resource_test.test | RUN | 0.238781 | 0.756168 | 0.596357 | 0.002842 | 0.002599 | 0.002324 |
| nest_test.test | RUN | 0.522440 | 3.321316 | 2.018264 | 0.002114 | 0.001824 | 0.001762 |
| nest_api_test.test | RUN | 0.958459 | 6.275679 | 4.341882 | 0.002802 | 0.002707 | 0.002589 |
| monotonic_buffer_resource_test.test | RUN | 0.261055 | 1.284716 | 0.554704 | 0.001898 | 0.001880 | 0.001878 |
| memory_resource_test.test | RUN | 0.150419 | 0.380697 | 0.339368 | 0.001966 | 0.002001 | 0.002117 |
| map_test.test | RUN | 3.856369 | 21.792127 | 20.683553 | 0.009823 | 0.009302 | 0.009498 |
| list_test.test | RUN | 2.174385 | 11.581993 | 8.623523 | 0.019570 | 0.021171 | 0.020642 |
| is_constructible_test.test | RUN | 0.090384 | 0.150589 | 0.185323 | 0.000841 | 0.000883 | 0.001661 |
| insert_vs_emplace_test.test | RUN | 0.238386 | 0.910650 | 1.108634 | 0.001872 | 0.001977 | 0.002014 |
| hub_uses_allocator_test.test | RUN | 0.288854 | 0.816134 | 0.750848 | 0.001989 | 0.002036 | 0.001880 |
| hub_stability_test.test | RUN | 0.516767 | 3.580358 | 2.901159 | 0.002080 | 0.002508 | 0.002184 |
| hub_sort_test.test | RUN | 0.327600 | 1.558057 | 1.392280 | 0.507762 | 0.490960 | 0.470996 |
| hub_pocxx_test.test | RUN | 0.302335 | 1.850660 | 1.008777 | 0.001760 | 0.002184 | 0.002052 |
| hub_new_extended_alignment_test.test | RUN | 0.196787 | 0.550112 | 0.541565 | 0.001696 | 0.001970 | 0.001500 |
| hub_exception_safety_test.test | RUN | 0.671184 | 4.458804 | 2.929451 | 0.080130 | 0.084001 | 0.080063 |
| hub_empty_sort_test.test | RUN | 0.210087 | 0.545257 | 0.611542 | 0.001866 | 0.001851 | 0.001858 |
| hub_api_test.test | RUN | 2.035471 | 19.914291 | 9.708587 | 0.003821 | 0.003694 | 0.003552 |
| global_resource_test.test | RUN | 0.129413 | 0.329334 | 0.314560 | 0.001825 | 0.002306 | 0.001850 |
| flat_tree_test.test | RUN | 2.774006 | 26.818918 | 17.969928 | 0.001771 | 0.001648 | 0.001874 |
| flat_set_test.test | RUN | 3.738283 | 34.312601 | 23.093008 | 0.008199 | 0.006726 | 0.007424 |
| flat_set_adaptor_test.test | RUN | 2.737448 | 29.967053 | 18.052195 | 0.007809 | 0.006994 | 0.007063 |
| flat_map_test.test | RUN | 5.763805 | 51.342908 | 37.679094 | 0.008812 | 0.008034 | 0.007638 |
| flat_map_adaptor_test.test | RUN | 3.621551 | 37.463100 | 26.542833 | 0.007806 | 0.007636 | 0.007717 |
| explicit_inst_vector_test.test | RUN | 0.753232 | 5.024437 | 3.902446 | 0.001816 | 0.002045 | 0.001638 |
| explicit_inst_string_test.test | RUN | 1.017372 | 9.117837 | 5.487471 | 0.001884 | 0.002062 | 0.001806 |
| explicit_inst_static_vector_test.test | RUN | 0.109136 | 0.321212 | 0.292081 | 0.002041 | 0.001773 | 0.001624 |
| explicit_inst_stable_vector_test.test | RUN | 0.626345 | 3.217625 | 2.580056 | 0.001794 | 0.001905 | 0.001830 |
| explicit_inst_small_vector_test.test | RUN | 0.556956 | 2.956161 | 2.270119 | 0.002057 | 0.002565 | 0.001760 |
| explicit_inst_slist_test.test | RUN | 0.431641 | 2.843711 | 1.702871 | 0.001752 | 0.001814 | 0.001871 |
| explicit_inst_set_test.test | RUN | 0.557893 | 3.128859 | 2.071188 | 0.001788 | 0.001966 | 0.001805 |
| explicit_inst_map_test.test | RUN | 0.659715 | 3.256691 | 2.236824 | 0.001860 | 0.001886 | 0.001822 |
| explicit_inst_list_test.test | RUN | 0.144744 | 0.740955 | 0.570030 | 0.001674 | 0.001890 | 0.001466 |
| explicit_inst_flat_set_test.test | RUN | 2.296038 | 22.882544 | 13.220720 | 0.002007 | 0.002098 | 0.001780 |
| explicit_inst_flat_map_test.test | RUN | 2.538700 | 24.018655 | 14.639572 | 0.001636 | 0.002016 | 0.001701 |
| explicit_inst_devector_test.test | RUN | 0.364056 | 2.769368 | 1.898842 | 0.001928 | 0.001789 | 0.002110 |
| explicit_inst_deque_test.test | RUN | 0.432770 | 2.992766 | 1.991888 | 0.001793 | 0.001794 | 0.001641 |
| devector_test.test | RUN | 5.808239 | 35.649577 | 24.630062 | 0.004042 | 0.003724 | 0.004319 |
| devector_options_test.test | RUN | 0.293642 | 1.598363 | 0.959741 | 0.002198 | 0.002080 | 0.001906 |
| deque_test.test | RUN | 3.995350 | 23.297327 | 17.146166 | 0.246837 | 0.326524 | 0.316092 |
| deque_options_test.test | RUN | 0.235550 | 0.608178 | 0.707786 | 0.001643 | 0.001808 | 0.001854 |
| copy_move_algo_test.test | RUN | 0.220099 | 0.736354 | 0.650693 | 0.002136 | 0.002454 | 0.001926 |
| common_iterator_test.test | RUN | 0.018812 | 0.042674 | 0.037601 | 0.000898 | 0.000756 | 0.001624 |
| boost_iterator_comp_test.test | RUN | 0.352482 | 0.945950 | 0.961234 | 0.002136 | 0.001952 | 0.001723 |
| allocator_traits_test.test | RUN | 0.218956 | 0.371443 | 0.404794 | 0.001958 | 0.002047 | 0.001782 |
| alloc_full_test.test | RUN | 0.201995 | 0.803542 | 0.675828 | 0.004168 | 0.004250 | 0.004108 |
| alloc_basic_test.test | RUN | 0.175196 | 0.595127 | 0.469251 | 0.001718 | 0.002319 | 0.001780 |
| adaptive_pool_test.test | RUN | 1.478214 | 13.590375 | 9.652560 | 0.012317 | 0.011212 | 0.012410 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.035× | 1.056×, 1.014× | 0.987× |
| clang | 0.992× | 1.008×, 0.976× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.008138s | 0.007444s | 0.008290s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001019s | 0.001017s | 0.001031s | byte equality observed |
| alloc_full_test.test | 7 | 0.002695s | 0.002745s | 0.002855s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000924s | 0.000965s | 0.000968s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000993s | 0.001001s | 0.000976s | byte equality observed |
| common_iterator_test.test | 10 | 0.000451s | 0.000459s | 0.000937s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001123s | 0.001119s | 0.001096s | byte equality observed |
| deque_options_test.test | 10 | 0.001020s | 0.001058s | 0.001047s | byte equality observed |
| deque_test.test | 1 | 0.257067s | 0.347677s | 0.325657s | byte equality observed |
| devector_options_test.test | 10 | 0.001093s | 0.001087s | 0.001077s | byte equality observed |
| devector_test.test | 6 | 0.003224s | 0.003056s | 0.003971s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000949s | 0.000938s | 0.001041s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000918s | 0.000926s | 0.000956s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000923s | 0.000938s | 0.000933s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000907s | 0.000936s | 0.000913s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000931s | 0.000912s | 0.000911s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000934s | 0.000951s | 0.000924s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000907s | 0.000930s | 0.000910s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000911s | 0.000947s | 0.000895s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000924s | 0.000942s | 0.000917s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000927s | 0.000933s | 0.000930s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000905s | 0.000930s | 0.000893s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000901s | 0.000951s | 0.000926s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000913s | 0.000946s | 0.000947s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005348s | 0.004854s | 0.005177s | byte equality observed |
| flat_map_test.test | 4 | 0.005882s | 0.005307s | 0.005409s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.005361s | 0.004447s | 0.005052s | byte equality observed |
| flat_set_test.test | 4 | 0.005377s | 0.004473s | 0.004916s | byte equality observed |
| flat_tree_test.test | 10 | 0.000968s | 0.000991s | 0.001018s | byte equality observed |
| global_resource_test.test | 10 | 0.000967s | 0.000983s | 0.001045s | byte equality observed |
| hub_api_test.test | 8 | 0.002502s | 0.002097s | 0.002144s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000947s | 0.000975s | 0.000985s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.061691s | 0.061595s | 0.059215s | byte equality observed |
| hub_new_extended_alignment_test.test | 9 | 0.000923s | 0.001017s | 0.000942s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000951s | 0.001039s | 0.000961s | byte equality observed |
| hub_sort_test.test | 1 | 0.442074s | 0.432068s | 0.419686s | byte equality observed |
| hub_stability_test.test | 10 | 0.001306s | 0.001221s | 0.001352s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000916s | 0.000937s | 0.000969s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000931s | 0.000946s | 0.001041s | byte equality observed |
| is_constructible_test.test | 10 | 0.000414s | 0.000431s | 0.000997s | byte equality observed |
| list_test.test | 2 | 0.013734s | 0.013573s | 0.013650s | byte equality observed |
| map_test.test | 3 | 0.006828s | 0.006791s | 0.006986s | byte equality observed |
| memory_resource_test.test | 10 | 0.001062s | 0.000956s | 0.001087s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000932s | 0.000956s | 0.001131s | byte equality observed |
| nest_api_test.test | 10 | 0.001506s | 0.001584s | 0.001606s | byte equality observed |
| nest_test.test | 10 | 0.000981s | 0.000972s | 0.001015s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001400s | 0.001373s | 0.001381s | byte equality observed |
| node_allocator_test.test | 4 | 0.006581s | 0.006121s | 0.006630s | byte equality observed |
| node_handle_test.test | 10 | 0.001053s | 0.001054s | 0.001104s | byte equality observed |
| null_iterators_test.test | 10 | 0.000940s | 0.001012s | 0.000976s | byte equality observed |
| pair_test.test | 10 | 0.000915s | 0.000959s | 0.001065s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000915s | 0.000928s | 0.000915s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000884s | 0.000927s | 0.000969s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000877s | 0.000907s | 0.000951s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000901s | 0.000904s | 0.000964s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000915s | 0.000930s | 0.000932s | byte equality observed |
| pmr_list_test.test | 10 | 0.000893s | 0.000932s | 0.000948s | byte equality observed |
| pmr_map_test.test | 10 | 0.000892s | 0.000952s | 0.000935s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000892s | 0.000946s | 0.000941s | byte equality observed |
| pmr_set_test.test | 10 | 0.000896s | 0.000937s | 0.000948s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000906s | 0.000939s | 0.000973s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000914s | 0.000959s | 0.000985s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000947s | 0.000954s | 0.000984s | byte equality observed |
| pmr_string_test.test | 10 | 0.000446s | 0.000446s | 0.000962s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000447s | 0.000446s | 0.000950s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001000s | 0.000987s | 0.000957s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000962s | 0.000988s | 0.000978s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000972s | 0.000965s | 0.000961s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000966s | 0.000967s | 0.000965s | byte equality observed |
| segtor_options_test.test | 10 | 0.000984s | 0.000940s | 0.001013s | byte equality observed |
| segtor_test.test | 1 | 0.323661s | 0.282567s | 0.269849s | byte equality observed |
| set_test.test | 2 | 0.008912s | 0.008072s | 0.008897s | byte equality observed |
| slist_test.test | 2 | 0.013823s | 0.013503s | 0.013779s | byte equality observed |
| small_vector_options_test.test | 9 | 0.001167s | 0.001122s | 0.001193s | byte equality observed |
| small_vector_test.test | 7 | 0.001955s | 0.001671s | 0.002945s | byte equality observed |
| stable_vector_test.test | 1 | 0.021282s | 0.021586s | 0.022686s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001072s | 0.001000s | 0.001111s | byte equality observed |
| static_vector_test.test | 10 | 0.001300s | 0.001157s | 0.001228s | byte equality observed |
| string_options_test.test | 10 | 0.000992s | 0.001000s | 0.001030s | byte equality observed |
| string_test.test | 3 | 0.008903s | 0.008862s | 0.009141s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001007s | 0.000994s | 0.001046s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001010s | 0.001003s | 0.000987s | byte equality observed |
| throw_exception_test.test | 10 | 0.000939s | 0.000932s | 0.000912s | byte equality observed |
| tree_test.test | 10 | 0.000935s | 0.000931s | 0.000920s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000980s | 0.000999s | 0.000984s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000913s | 0.000939s | 0.000987s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000427s | 0.000455s | 0.000947s | byte equality observed |
| vector_options_test.test | 10 | 0.001123s | 0.001118s | 0.001070s | byte equality observed |
| vector_test.test | 5 | 0.003093s | 0.003178s | 0.004018s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
