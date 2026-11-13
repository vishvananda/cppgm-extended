# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 6.661345 | 31.248997 | 27.145512 | 0.005655 | 0.005342 | 0.007497 |
| vector_options_test.test | RUN | 0.310025 | 1.399978 | 1.192692 | 0.002010 | 0.002098 | 0.002000 |
| uses_allocator_test.test | RUN | 0.059194 | 0.121482 | 0.164383 | 0.000759 | 0.000842 | 0.001844 |
| uses_allocator_construction_test.test | RUN | 0.172843 | 0.453281 | 0.601703 | 0.001734 | 0.001820 | 0.001622 |
| unsynchronized_pool_resource_test.test | RUN | 0.222075 | 1.266709 | 0.751242 | 0.001739 | 0.001788 | 0.001623 |
| tree_test.test | RUN | 0.904722 | 4.703688 | 3.911004 | 0.001727 | 0.001932 | 0.001702 |
| throw_exception_test.test | RUN | 0.122768 | 0.290482 | 0.393825 | 0.001514 | 0.001868 | 0.001751 |
| synchronized_pool_resource_test.test | RUN | 0.220317 | 1.310638 | 0.739171 | 0.001792 | 0.001846 | 0.001733 |
| string_view_compat_test.test | RUN | 0.370373 | 2.643475 | 1.431703 | 0.001425 | 0.001886 | 0.001760 |
| string_test.test | RUN | 1.561761 | 9.918596 | 7.450180 | 0.016265 | 0.014596 | 0.015437 |
| string_options_test.test | RUN | 0.287063 | 2.152597 | 1.069835 | 0.001735 | 0.001880 | 0.001795 |
| static_vector_test.test | RUN | 1.689981 | 8.275766 | 7.292167 | 0.002323 | 0.002377 | 0.002348 |
| static_vector_options_test.test | RUN | 0.209847 | 0.561285 | 0.698440 | 0.001907 | 0.001815 | 0.001865 |
| stable_vector_test.test | RUN | 3.666618 | 16.970509 | 13.904162 | 0.031693 | 0.028826 | 0.033319 |
| small_vector_test.test | RUN | 2.534752 | 11.880095 | 10.307319 | 0.003580 | 0.003228 | 0.005428 |
| small_vector_options_test.test | RUN | 0.356575 | 1.449770 | 1.363437 | 0.001852 | 0.002028 | 0.001926 |
| slist_test.test | RUN | 2.309343 | 10.674615 | 9.209849 | 0.024957 | 0.024653 | 0.025123 |
| set_test.test | RUN | 3.525412 | 19.998230 | 16.373098 | 0.015147 | 0.013849 | 0.014945 |
| segtor_test.test | RUN | 4.665698 | 21.908844 | 18.049747 | 0.730374 | 0.650019 | 0.664958 |
| segtor_options_test.test | RUN | 0.234167 | 0.663644 | 0.817200 | 0.001889 | 0.001802 | 0.001855 |
| scoped_allocator_usage_test.test | RUN | 0.821854 | 3.347563 | 3.087139 | 0.001736 | 0.001656 | 0.001684 |
| scoped_allocator_adaptor_test.test | RUN | 0.370798 | 0.945190 | 1.440676 | 0.001760 | 0.001780 | 0.001701 |
| resource_adaptor_test.test | RUN | 0.188721 | 0.536766 | 0.613840 | 0.001784 | 0.001846 | 0.001864 |
| polymorphic_allocator_test.test | RUN | 0.161084 | 0.451904 | 0.503519 | 0.001744 | 0.001718 | 0.001686 |
| pmr_vector_test.test | RUN | 0.089750 | 0.182932 | 0.255930 | 0.000724 | 0.000663 | 0.001759 |
| pmr_string_test.test | RUN | 0.178276 | 0.369761 | 0.515216 | 0.000691 | 0.000767 | 0.001686 |
| pmr_stable_vector_test.test | RUN | 0.209612 | 0.591177 | 0.703807 | 0.001701 | 0.001786 | 0.001809 |
| pmr_small_vector_test.test | RUN | 0.107118 | 0.240568 | 0.327342 | 0.001704 | 0.001719 | 0.001760 |
| pmr_slist_test.test | RUN | 0.148321 | 0.766331 | 0.681498 | 0.001612 | 0.001674 | 0.001373 |
| pmr_set_test.test | RUN | 0.163592 | 0.404294 | 0.479116 | 0.001683 | 0.001599 | 0.001590 |
| pmr_nest_test.test | RUN | 0.131383 | 0.271168 | 0.405431 | 0.001692 | 0.001832 | 0.001461 |
| pmr_map_test.test | RUN | 0.172409 | 0.407517 | 0.559044 | 0.001675 | 0.001754 | 0.001796 |
| pmr_list_test.test | RUN | 0.118374 | 0.250840 | 0.369517 | 0.001607 | 0.001842 | 0.001813 |
| pmr_hub_test.test | RUN | 0.164295 | 0.354748 | 0.453240 | 0.001770 | 0.001626 | 0.001744 |
| pmr_flat_set_test.test | RUN | 0.126672 | 0.283359 | 0.379620 | 0.001552 | 0.001743 | 0.001668 |
| pmr_flat_map_test.test | RUN | 0.153710 | 0.323985 | 0.466342 | 0.001588 | 0.001825 | 0.001855 |
| pmr_devector_test.test | RUN | 0.102000 | 0.279926 | 0.352975 | 0.001766 | 0.001619 | 0.001584 |
| pmr_deque_test.test | RUN | 0.099015 | 0.234694 | 0.345433 | 0.001740 | 0.001756 | 0.001656 |
| pair_test.test | RUN | 0.257593 | 0.564737 | 0.762750 | 0.001793 | 0.001762 | 0.001682 |
| null_iterators_test.test | RUN | 0.390337 | 1.059692 | 1.240093 | 0.001761 | 0.001831 | 0.001662 |
| node_handle_test.test | RUN | 0.173619 | 1.324591 | 0.517174 | 0.001810 | 0.001796 | 0.001526 |
| node_allocator_test.test | RUN | 2.745829 | 13.193970 | 11.262814 | 0.010298 | 0.010217 | 0.010687 |
| new_delete_resource_test.test | RUN | 0.216218 | 0.793015 | 0.717170 | 0.002485 | 0.002474 | 0.004014 |
| nest_test.test | RUN | 0.533671 | 3.600547 | 2.446019 | 0.001791 | 0.001856 | 0.001857 |
| nest_api_test.test | RUN | 0.884432 | 6.860048 | 4.739001 | 0.002623 | 0.002754 | 0.002639 |
| monotonic_buffer_resource_test.test | RUN | 0.209018 | 1.419967 | 0.740758 | 0.001780 | 0.001836 | 0.001744 |
| memory_resource_test.test | RUN | 0.130226 | 0.421316 | 0.434134 | 0.001758 | 0.001738 | 0.001747 |
| map_test.test | RUN | 4.913510 | 22.673957 | 23.761864 | 0.011269 | 0.011068 | 0.009670 |
| list_test.test | RUN | 2.522393 | 11.563577 | 9.737747 | 0.024849 | 0.025066 | 0.025659 |
| is_constructible_test.test | RUN | 0.079841 | 0.163780 | 0.256401 | 0.000762 | 0.000785 | 0.001758 |
| insert_vs_emplace_test.test | RUN | 0.264499 | 0.904389 | 1.357917 | 0.001853 | 0.001804 | 0.001719 |
| hub_uses_allocator_test.test | RUN | 0.268809 | 0.829770 | 0.946224 | 0.001519 | 0.001798 | 0.001777 |
| hub_stability_test.test | RUN | 0.578321 | 3.647371 | 3.303390 | 0.001952 | 0.002070 | 0.002056 |
| hub_sort_test.test | RUN | 0.373578 | 1.677641 | 1.630330 | 0.532552 | 0.525555 | 0.507496 |
| hub_pocxx_test.test | RUN | 0.338109 | 1.973068 | 1.116473 | 0.001789 | 0.001844 | 0.001657 |
| hub_new_extended_alignment_test.test | RUN | 0.193097 | 0.610916 | 0.661781 | 0.001707 | 0.001842 | 0.001801 |
| hub_exception_safety_test.test | RUN | 0.684107 | 4.569179 | 3.353365 | 0.091891 | 0.088776 | 0.093389 |
| hub_empty_sort_test.test | RUN | 0.216601 | 0.566616 | 0.806059 | 0.001827 | 0.001759 | 0.001788 |
| hub_api_test.test | RUN | 2.318336 | 18.672946 | 10.614755 | 0.004205 | 0.003618 | 0.003517 |
| global_resource_test.test | RUN | 0.129372 | 0.378915 | 0.414856 | 0.001854 | 0.001786 | 0.001765 |
| flat_tree_test.test | RUN | 4.269358 | 28.432767 | 19.563252 | 0.001691 | 0.001812 | 0.001605 |
| flat_set_test.test | RUN | 5.361302 | 35.946717 | 25.465544 | 0.008267 | 0.007287 | 0.008368 |
| flat_set_adaptor_test.test | RUN | 4.033809 | 31.853054 | 20.052230 | 0.007981 | 0.007738 | 0.008345 |
| flat_map_test.test | RUN | 8.803764 | 53.952270 | 42.402304 | 0.008922 | 0.008970 | 0.008640 |
| flat_map_adaptor_test.test | RUN | 5.609166 | 39.249154 | 30.002229 | 0.008477 | 0.007960 | 0.008259 |
| explicit_inst_vector_test.test | RUN | 0.868433 | 5.421579 | 4.396520 | 0.001659 | 0.001824 | 0.001592 |
| explicit_inst_string_test.test | RUN | 1.126747 | 9.378917 | 6.024618 | 0.001325 | 0.001762 | 0.001732 |
| explicit_inst_static_vector_test.test | RUN | 0.113137 | 0.357617 | 0.454236 | 0.001610 | 0.001733 | 0.001631 |
| explicit_inst_stable_vector_test.test | RUN | 0.671804 | 3.375220 | 2.686846 | 0.001494 | 0.001766 | 0.001569 |
| explicit_inst_small_vector_test.test | RUN | 0.573503 | 3.090115 | 2.637044 | 0.001658 | 0.001698 | 0.001691 |
| explicit_inst_slist_test.test | RUN | 0.427173 | 2.838060 | 1.948258 | 0.001767 | 0.001775 | 0.001736 |
| explicit_inst_set_test.test | RUN | 0.615284 | 3.217777 | 2.457815 | 0.001763 | 0.001709 | 0.001694 |
| explicit_inst_map_test.test | RUN | 0.693149 | 3.353990 | 2.633523 | 0.001701 | 0.001800 | 0.001697 |
| explicit_inst_list_test.test | RUN | 0.153824 | 0.776716 | 0.671560 | 0.001748 | 0.001899 | 0.001612 |
| explicit_inst_flat_set_test.test | RUN | 3.570415 | 24.056954 | 14.917908 | 0.001880 | 0.001915 | 0.001866 |
| explicit_inst_flat_map_test.test | RUN | 3.825590 | 25.037171 | 17.060941 | 0.001553 | 0.001820 | 0.001714 |
| explicit_inst_devector_test.test | RUN | 0.456062 | 3.108237 | 2.171508 | 0.001362 | 0.001800 | 0.001731 |
| explicit_inst_deque_test.test | RUN | 0.503539 | 3.359358 | 2.275406 | 0.001788 | 0.001872 | 0.001624 |
| devector_test.test | RUN | 6.660949 | 40.693235 | 27.031357 | 0.003652 | 0.003528 | 0.004701 |
| devector_options_test.test | RUN | 0.301971 | 1.819830 | 1.249827 | 0.002154 | 0.001824 | 0.002028 |
| deque_test.test | RUN | 4.540499 | 20.820515 | 19.617759 | 0.609844 | 0.559789 | 0.428707 |
| deque_options_test.test | RUN | 0.242649 | 0.683843 | 0.949578 | 0.001708 | 0.001758 | 0.001933 |
| copy_move_algo_test.test | RUN | 0.226036 | 0.776114 | 0.890199 | 0.001958 | 0.001986 | 0.001809 |
| common_iterator_test.test | RUN | 0.018140 | 0.044049 | 0.049267 | 0.000711 | 0.000770 | 0.001682 |
| boost_iterator_comp_test.test | RUN | 0.325300 | 0.998541 | 1.162700 | 0.001608 | 0.001679 | 0.001598 |
| allocator_traits_test.test | RUN | 0.192890 | 0.448643 | 0.635044 | 0.001605 | 0.001675 | 0.001740 |
| alloc_full_test.test | RUN | 0.213352 | 0.908607 | 0.769715 | 0.004833 | 0.004797 | 0.004809 |
| alloc_basic_test.test | RUN | 0.176809 | 0.655765 | 0.578531 | 0.001778 | 0.001923 | 0.001948 |
| adaptive_pool_test.test | RUN | 2.613868 | 13.541588 | 11.362408 | 0.010451 | 0.009637 | 0.014732 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.909× | 0.915×, 0.903× | 1.004× |
| clang | 0.944× | 0.943×, 0.944× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.010379s | 0.009725s | 0.010472s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001058s | 0.001031s | 0.001045s | byte equality observed |
| alloc_full_test.test | 7 | 0.002984s | 0.002999s | 0.003070s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000970s | 0.000970s | 0.000959s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001020s | 0.001012s | 0.000982s | byte equality observed |
| common_iterator_test.test | 10 | 0.000470s | 0.000456s | 0.000992s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001234s | 0.001146s | 0.001130s | byte equality observed |
| deque_options_test.test | 10 | 0.001069s | 0.001028s | 0.001051s | byte equality observed |
| deque_test.test | 1 | 0.451857s | 0.389521s | 0.448275s | byte equality observed |
| devector_options_test.test | 10 | 0.001173s | 0.001105s | 0.001114s | byte equality observed |
| devector_test.test | 5 | 0.003217s | 0.003116s | 0.004287s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000955s | 0.000946s | 0.000951s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000943s | 0.000943s | 0.000958s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000951s | 0.000959s | 0.000954s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000954s | 0.000946s | 0.000945s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000937s | 0.000942s | 0.000935s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000952s | 0.000942s | 0.000943s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000947s | 0.000941s | 0.000948s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000945s | 0.000948s | 0.000944s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000948s | 0.000953s | 0.000952s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000953s | 0.000951s | 0.000948s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000938s | 0.000952s | 0.000962s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000956s | 0.000945s | 0.000958s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000948s | 0.000938s | 0.000942s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005912s | 0.005603s | 0.005756s | byte equality observed |
| flat_map_test.test | 4 | 0.006355s | 0.005996s | 0.005965s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.005879s | 0.005355s | 0.005909s | byte equality observed |
| flat_set_test.test | 4 | 0.005882s | 0.005151s | 0.005831s | byte equality observed |
| flat_tree_test.test | 10 | 0.000958s | 0.000956s | 0.000977s | byte equality observed |
| global_resource_test.test | 10 | 0.000981s | 0.000995s | 0.001001s | byte equality observed |
| hub_api_test.test | 7 | 0.002695s | 0.002219s | 0.002178s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000956s | 0.000957s | 0.000958s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.065510s | 0.063154s | 0.064929s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001030s | 0.001011s | 0.000977s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000994s | 0.001000s | 0.000996s | byte equality observed |
| hub_sort_test.test | 1 | 0.445580s | 0.430781s | 0.419546s | byte equality observed |
| hub_stability_test.test | 10 | 0.001196s | 0.001171s | 0.001223s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000947s | 0.000942s | 0.000976s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000982s | 0.000995s | 0.001001s | byte equality observed |
| is_constructible_test.test | 10 | 0.000436s | 0.000442s | 0.000946s | byte equality observed |
| list_test.test | 2 | 0.017437s | 0.017305s | 0.017455s | byte equality observed |
| map_test.test | 3 | 0.007829s | 0.007696s | 0.007725s | byte equality observed |
| memory_resource_test.test | 10 | 0.000975s | 0.000960s | 0.000991s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000982s | 0.000989s | 0.000991s | byte equality observed |
| nest_api_test.test | 10 | 0.001594s | 0.001638s | 0.001628s | byte equality observed |
| nest_test.test | 10 | 0.001007s | 0.001017s | 0.001048s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001435s | 0.001442s | 0.001483s | byte equality observed |
| node_allocator_test.test | 3 | 0.007196s | 0.006982s | 0.007114s | byte equality observed |
| node_handle_test.test | 10 | 0.000968s | 0.001020s | 0.000971s | byte equality observed |
| null_iterators_test.test | 10 | 0.000976s | 0.000977s | 0.000976s | byte equality observed |
| pair_test.test | 10 | 0.000975s | 0.000977s | 0.000966s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000946s | 0.000952s | 0.000935s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000950s | 0.000953s | 0.000939s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000948s | 0.000944s | 0.000929s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000961s | 0.000955s | 0.000949s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000940s | 0.000971s | 0.000951s | byte equality observed |
| pmr_list_test.test | 10 | 0.000946s | 0.000948s | 0.000959s | byte equality observed |
| pmr_map_test.test | 10 | 0.000944s | 0.000947s | 0.000955s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000952s | 0.000969s | 0.000955s | byte equality observed |
| pmr_set_test.test | 10 | 0.000947s | 0.000970s | 0.000969s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000964s | 0.000961s | 0.000978s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000949s | 0.000962s | 0.000959s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000974s | 0.001005s | 0.000982s | byte equality observed |
| pmr_string_test.test | 10 | 0.000459s | 0.000460s | 0.000971s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000458s | 0.000455s | 0.001006s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001003s | 0.001038s | 0.001012s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001024s | 0.001004s | 0.001014s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001035s | 0.001013s | 0.000973s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001012s | 0.001038s | 0.000998s | byte equality observed |
| segtor_options_test.test | 10 | 0.001036s | 0.001015s | 0.001022s | byte equality observed |
| segtor_test.test | 1 | 0.437103s | 0.371002s | 0.372737s | byte equality observed |
| set_test.test | 2 | 0.010917s | 0.009650s | 0.010999s | byte equality observed |
| slist_test.test | 1 | 0.016723s | 0.016827s | 0.016737s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001076s | 0.001068s | 0.001051s | byte equality observed |
| small_vector_test.test | 6 | 0.002111s | 0.001927s | 0.003790s | byte equality observed |
| stable_vector_test.test | 1 | 0.025271s | 0.025385s | 0.026494s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001061s | 0.001005s | 0.001035s | byte equality observed |
| static_vector_test.test | 10 | 0.001363s | 0.001228s | 0.001278s | byte equality observed |
| string_options_test.test | 10 | 0.001020s | 0.001026s | 0.001013s | byte equality observed |
| string_test.test | 2 | 0.011134s | 0.011902s | 0.011413s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000969s | 0.000997s | 0.000965s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001024s | 0.001023s | 0.001012s | byte equality observed |
| throw_exception_test.test | 10 | 0.000961s | 0.000955s | 0.000947s | byte equality observed |
| tree_test.test | 10 | 0.000948s | 0.000947s | 0.000954s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001034s | 0.001022s | 0.001013s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000955s | 0.000961s | 0.000950s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000443s | 0.000452s | 0.000925s | byte equality observed |
| vector_options_test.test | 10 | 0.001114s | 0.001128s | 0.001096s | byte equality observed |
| vector_test.test | 4 | 0.003494s | 0.003567s | 0.004865s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
