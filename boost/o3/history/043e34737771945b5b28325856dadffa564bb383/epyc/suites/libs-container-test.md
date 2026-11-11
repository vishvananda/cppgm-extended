# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.112774 | 25.928663 | 23.216460 | 0.005652 | 0.004620 | 0.006079 |
| vector_options_test.test | RUN | 0.290655 | 1.173923 | 0.937239 | 0.001966 | 0.001944 | 0.002005 |
| uses_allocator_test.test | RUN | 0.054699 | 0.094093 | 0.124383 | 0.000838 | 0.000745 | 0.001710 |
| uses_allocator_construction_test.test | RUN | 0.164709 | 0.347453 | 0.433785 | 0.001741 | 0.001682 | 0.001847 |
| unsynchronized_pool_resource_test.test | RUN | 0.233468 | 1.026516 | 0.532843 | 0.001931 | 0.001649 | 0.001918 |
| tree_test.test | RUN | 0.798438 | 3.989588 | 3.130615 | 0.001890 | 0.001744 | 0.001793 |
| throw_exception_test.test | RUN | 0.120393 | 0.220255 | 0.265289 | 0.001697 | 0.001602 | 0.001835 |
| synchronized_pool_resource_test.test | RUN | 0.234279 | 1.032726 | 0.536508 | 0.001860 | 0.001839 | 0.002384 |
| string_view_compat_test.test | RUN | 0.351501 | 2.111121 | 1.047682 | 0.001528 | 0.001816 | 0.001857 |
| string_test.test | RUN | 1.555371 | 8.068629 | 6.141013 | 0.012975 | 0.012232 | 0.012733 |
| string_options_test.test | RUN | 0.248633 | 1.679764 | 0.946928 | 0.002048 | 0.001838 | 0.001888 |
| static_vector_test.test | RUN | 1.682654 | 6.718260 | 6.297852 | 0.002507 | 0.002138 | 0.002086 |
| static_vector_options_test.test | RUN | 0.200407 | 0.443633 | 0.577979 | 0.002054 | 0.001838 | 0.001960 |
| stable_vector_test.test | RUN | 3.642238 | 14.253727 | 11.929317 | 0.025943 | 0.025021 | 0.026997 |
| small_vector_test.test | RUN | 2.079934 | 9.933818 | 8.706364 | 0.003580 | 0.002796 | 0.004791 |
| small_vector_options_test.test | RUN | 0.358823 | 1.177745 | 1.052237 | 0.002371 | 0.001971 | 0.001992 |
| slist_test.test | RUN | 1.730417 | 9.027311 | 7.944064 | 0.019884 | 0.018154 | 0.019751 |
| set_test.test | RUN | 2.844422 | 17.143102 | 13.944698 | 0.013102 | 0.011207 | 0.012881 |
| segtor_test.test | RUN | 4.032515 | 18.781211 | 15.731250 | 0.491399 | 0.432868 | 0.429867 |
| segtor_options_test.test | RUN | 0.220217 | 0.510492 | 0.623157 | 0.001997 | 0.001918 | 0.001892 |
| scoped_allocator_usage_test.test | RUN | 0.739779 | 2.751196 | 2.615281 | 0.001857 | 0.001649 | 0.001867 |
| scoped_allocator_adaptor_test.test | RUN | 0.375257 | 0.790882 | 1.120839 | 0.001904 | 0.001509 | 0.001779 |
| resource_adaptor_test.test | RUN | 0.190942 | 0.455890 | 0.513295 | 0.001878 | 0.001584 | 0.001761 |
| polymorphic_allocator_test.test | RUN | 0.167927 | 0.365558 | 0.411748 | 0.002012 | 0.001824 | 0.001609 |
| pmr_vector_test.test | RUN | 0.118838 | 0.140727 | 0.216836 | 0.000664 | 0.000764 | 0.001830 |
| pmr_string_test.test | RUN | 0.174491 | 0.280229 | 0.399040 | 0.000691 | 0.000779 | 0.001837 |
| pmr_stable_vector_test.test | RUN | 0.175808 | 0.454308 | 0.579622 | 0.001750 | 0.001617 | 0.001675 |
| pmr_small_vector_test.test | RUN | 0.102808 | 0.191420 | 0.267000 | 0.001658 | 0.001579 | 0.001750 |
| pmr_slist_test.test | RUN | 0.131233 | 0.625956 | 0.582591 | 0.001931 | 0.001784 | 0.001406 |
| pmr_set_test.test | RUN | 0.160091 | 0.320230 | 0.341885 | 0.001320 | 0.001783 | 0.001341 |
| pmr_nest_test.test | RUN | 0.129493 | 0.214382 | 0.251767 | 0.001480 | 0.001774 | 0.001790 |
| pmr_map_test.test | RUN | 0.164213 | 0.324062 | 0.407316 | 0.001582 | 0.001643 | 0.001762 |
| pmr_list_test.test | RUN | 0.112814 | 0.207692 | 0.296741 | 0.001596 | 0.001495 | 0.001774 |
| pmr_hub_test.test | RUN | 0.160803 | 0.279164 | 0.373359 | 0.001617 | 0.001629 | 0.001835 |
| pmr_flat_set_test.test | RUN | 0.117688 | 0.219069 | 0.295121 | 0.003063 | 0.001479 | 0.001750 |
| pmr_flat_map_test.test | RUN | 0.150859 | 0.253645 | 0.372918 | 0.001620 | 0.001450 | 0.001678 |
| pmr_devector_test.test | RUN | 0.098706 | 0.220279 | 0.256991 | 0.001530 | 0.001518 | 0.001797 |
| pmr_deque_test.test | RUN | 0.093381 | 0.187972 | 0.269373 | 0.001827 | 0.001762 | 0.001850 |
| pair_test.test | RUN | 0.236566 | 0.449505 | 0.602403 | 0.002023 | 0.001816 | 0.001784 |
| null_iterators_test.test | RUN | 0.389778 | 0.879555 | 0.998460 | 0.001793 | 0.001783 | 0.001724 |
| node_handle_test.test | RUN | 0.177340 | 1.071208 | 0.411918 | 0.001585 | 0.001926 | 0.001766 |
| node_allocator_test.test | RUN | 2.024371 | 11.652528 | 9.726347 | 0.008708 | 0.008062 | 0.008942 |
| new_delete_resource_test.test | RUN | 0.224358 | 0.700601 | 0.529481 | 0.002619 | 0.002533 | 0.002452 |
| nest_test.test | RUN | 0.531543 | 3.022958 | 2.074335 | 0.001966 | 0.001964 | 0.001744 |
| nest_api_test.test | RUN | 0.999241 | 5.874250 | 4.166347 | 0.002671 | 0.002509 | 0.002591 |
| monotonic_buffer_resource_test.test | RUN | 0.247311 | 1.215189 | 0.491852 | 0.001510 | 0.002056 | 0.001881 |
| memory_resource_test.test | RUN | 0.139355 | 0.344674 | 0.332658 | 0.001745 | 0.001720 | 0.001828 |
| map_test.test | RUN | 4.108226 | 19.502043 | 20.477276 | 0.010136 | 0.009426 | 0.009581 |
| list_test.test | RUN | 1.862149 | 10.069691 | 8.379910 | 0.019209 | 0.019149 | 0.020741 |
| is_constructible_test.test | RUN | 0.083117 | 0.119587 | 0.182094 | 0.000840 | 0.000798 | 0.001824 |
| insert_vs_emplace_test.test | RUN | 0.222410 | 0.744310 | 1.059432 | 0.001833 | 0.001791 | 0.001850 |
| hub_uses_allocator_test.test | RUN | 0.275911 | 0.694002 | 0.780725 | 0.001806 | 0.001856 | 0.001892 |
| hub_stability_test.test | RUN | 0.506348 | 3.072545 | 2.795999 | 0.002217 | 0.002190 | 0.002030 |
| hub_sort_test.test | RUN | 0.334285 | 1.394178 | 1.309403 | 0.502902 | 0.466967 | 0.469911 |
| hub_pocxx_test.test | RUN | 0.337065 | 1.681708 | 0.943699 | 0.001552 | 0.002038 | 0.001512 |
| hub_new_extended_alignment_test.test | RUN | 0.188450 | 0.622860 | 0.477281 | 0.001803 | 0.001860 | 0.001607 |
| hub_exception_safety_test.test | RUN | 0.666711 | 3.894911 | 2.845585 | 0.072236 | 0.075260 | 0.073591 |
| hub_empty_sort_test.test | RUN | 0.212658 | 0.478576 | 0.576238 | 0.001831 | 0.001827 | 0.001737 |
| hub_api_test.test | RUN | 2.040816 | 15.689588 | 9.579565 | 0.004124 | 0.003397 | 0.003807 |
| global_resource_test.test | RUN | 0.131009 | 0.305512 | 0.304113 | 0.001556 | 0.001832 | 0.001884 |
| flat_tree_test.test | RUN | 3.516601 | 24.132466 | 17.851958 | 0.001904 | 0.001717 | 0.001954 |
| flat_set_test.test | RUN | 4.279586 | 29.811231 | 23.267010 | 0.008081 | 0.006518 | 0.007536 |
| flat_set_adaptor_test.test | RUN | 3.317989 | 26.621605 | 18.037444 | 0.009713 | 0.006936 | 0.007462 |
| flat_map_test.test | RUN | 7.247034 | 47.080740 | 38.373559 | 0.008561 | 0.008004 | 0.007531 |
| flat_map_adaptor_test.test | RUN | 4.649746 | 34.430445 | 26.427383 | 0.008322 | 0.007439 | 0.007756 |
| explicit_inst_vector_test.test | RUN | 0.758656 | 4.657786 | 4.052650 | 0.001804 | 0.001588 | 0.001844 |
| explicit_inst_string_test.test | RUN | 1.017508 | 8.601297 | 5.704695 | 0.001565 | 0.001748 | 0.001668 |
| explicit_inst_static_vector_test.test | RUN | 0.110827 | 0.305630 | 0.364249 | 0.001624 | 0.001458 | 0.001855 |
| explicit_inst_stable_vector_test.test | RUN | 0.650590 | 3.090813 | 2.671004 | 0.001847 | 0.001975 | 0.001692 |
| explicit_inst_small_vector_test.test | RUN | 0.508515 | 2.885352 | 2.327907 | 0.001605 | 0.001924 | 0.001826 |
| explicit_inst_slist_test.test | RUN | 0.396265 | 2.521963 | 1.789066 | 0.001797 | 0.001933 | 0.001860 |
| explicit_inst_set_test.test | RUN | 0.594431 | 2.877461 | 2.097148 | 0.001830 | 0.002028 | 0.001941 |
| explicit_inst_map_test.test | RUN | 0.654154 | 3.100615 | 2.373056 | 0.001950 | 0.001922 | 0.001529 |
| explicit_inst_list_test.test | RUN | 0.149800 | 0.732702 | 0.595273 | 0.001870 | 0.001792 | 0.001848 |
| explicit_inst_flat_set_test.test | RUN | 3.051105 | 20.925193 | 14.315699 | 0.001881 | 0.001805 | 0.001924 |
| explicit_inst_flat_map_test.test | RUN | 3.419157 | 21.740824 | 15.692944 | 0.001803 | 0.001822 | 0.001808 |
| explicit_inst_devector_test.test | RUN | 0.390487 | 2.648121 | 1.947109 | 0.001780 | 0.001743 | 0.001750 |
| explicit_inst_deque_test.test | RUN | 0.442831 | 2.823936 | 2.065183 | 0.001941 | 0.001771 | 0.001805 |
| devector_test.test | RUN | 6.035661 | 33.406443 | 27.555781 | 0.004064 | 0.003534 | 0.004448 |
| devector_options_test.test | RUN | 0.292396 | 1.535478 | 1.139318 | 0.002214 | 0.001994 | 0.002168 |
| deque_test.test | RUN | 3.825233 | 19.842573 | 18.093512 | 0.245738 | 0.313379 | 0.345551 |
| deque_options_test.test | RUN | 0.221449 | 0.572320 | 0.775323 | 0.001746 | 0.001843 | 0.002046 |
| copy_move_algo_test.test | RUN | 0.221983 | 0.642717 | 0.653893 | 0.001823 | 0.001878 | 0.001947 |
| common_iterator_test.test | RUN | 0.017788 | 0.038773 | 0.045615 | 0.000730 | 0.000762 | 0.001760 |
| boost_iterator_comp_test.test | RUN | 0.289005 | 0.826688 | 1.087860 | 0.001602 | 0.001848 | 0.001940 |
| allocator_traits_test.test | RUN | 0.178586 | 0.350356 | 0.470742 | 0.001724 | 0.001543 | 0.002002 |
| alloc_full_test.test | RUN | 0.190246 | 0.745091 | 0.771998 | 0.003624 | 0.004197 | 0.004666 |
| alloc_basic_test.test | RUN | 0.166395 | 0.518793 | 0.551293 | 0.001884 | 0.001773 | 0.002050 |
| adaptive_pool_test.test | RUN | 1.960883 | 8.837665 | 7.926956 | 0.008270 | 0.007229 | 0.008515 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.038× | 1.039×, 1.038× | 1.001× |
| clang | 1.017× | 1.010×, 1.025× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.008273s | 0.007365s | 0.007975s | byte equality observed |
| alloc_basic_test.test | 10 | 0.000996s | 0.001027s | 0.001028s | byte equality observed |
| alloc_full_test.test | 8 | 0.002678s | 0.002719s | 0.002805s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000944s | 0.000940s | 0.000941s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000972s | 0.001010s | 0.000930s | byte equality observed |
| common_iterator_test.test | 10 | 0.000450s | 0.000449s | 0.000926s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001122s | 0.001041s | 0.001056s | byte equality observed |
| deque_options_test.test | 10 | 0.001011s | 0.000991s | 0.001009s | byte equality observed |
| deque_test.test | 1 | 0.253594s | 0.345005s | 0.331017s | byte equality observed |
| devector_options_test.test | 10 | 0.001095s | 0.001072s | 0.001046s | byte equality observed |
| devector_test.test | 6 | 0.003136s | 0.002881s | 0.003560s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000935s | 0.000911s | 0.000904s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000914s | 0.000930s | 0.000882s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000912s | 0.000936s | 0.000917s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000919s | 0.000915s | 0.000888s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000893s | 0.000904s | 0.000881s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000904s | 0.000917s | 0.000899s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000895s | 0.000904s | 0.000915s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000901s | 0.000888s | 0.000896s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000890s | 0.000912s | 0.000880s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000904s | 0.000914s | 0.000894s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000895s | 0.000914s | 0.000896s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000908s | 0.000926s | 0.000913s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000926s | 0.000921s | 0.000917s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005076s | 0.004830s | 0.004900s | byte equality observed |
| flat_map_test.test | 4 | 0.005498s | 0.005220s | 0.005060s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.004891s | 0.004501s | 0.004661s | byte equality observed |
| flat_set_test.test | 4 | 0.005313s | 0.004355s | 0.004639s | byte equality observed |
| flat_tree_test.test | 10 | 0.000935s | 0.000947s | 0.000935s | byte equality observed |
| global_resource_test.test | 10 | 0.000970s | 0.000969s | 0.000965s | byte equality observed |
| hub_api_test.test | 8 | 0.002604s | 0.002046s | 0.002040s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000944s | 0.000942s | 0.000933s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.058821s | 0.067803s | 0.057160s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000923s | 0.001131s | 0.000942s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000969s | 0.001002s | 0.000963s | byte equality observed |
| hub_sort_test.test | 1 | 0.438610s | 0.438905s | 0.407520s | byte equality observed |
| hub_stability_test.test | 10 | 0.001243s | 0.001309s | 0.001232s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000921s | 0.001001s | 0.000917s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000945s | 0.000977s | 0.000933s | byte equality observed |
| is_constructible_test.test | 10 | 0.000435s | 0.000434s | 0.000886s | byte equality observed |
| list_test.test | 2 | 0.012879s | 0.013632s | 0.013164s | byte equality observed |
| map_test.test | 3 | 0.006452s | 0.006749s | 0.006301s | byte equality observed |
| memory_resource_test.test | 10 | 0.000969s | 0.001021s | 0.000917s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000969s | 0.000967s | 0.000921s | byte equality observed |
| nest_api_test.test | 10 | 0.001521s | 0.001502s | 0.001494s | byte equality observed |
| nest_test.test | 10 | 0.000972s | 0.000979s | 0.000951s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001363s | 0.001395s | 0.001353s | byte equality observed |
| node_allocator_test.test | 4 | 0.005882s | 0.006229s | 0.006023s | byte equality observed |
| node_handle_test.test | 10 | 0.000949s | 0.001007s | 0.001012s | byte equality observed |
| null_iterators_test.test | 10 | 0.000929s | 0.000948s | 0.000944s | byte equality observed |
| pair_test.test | 10 | 0.000937s | 0.000928s | 0.000907s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000901s | 0.000925s | 0.000909s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000922s | 0.000944s | 0.000898s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000915s | 0.000917s | 0.000893s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000914s | 0.000931s | 0.000892s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000906s | 0.000926s | 0.000902s | byte equality observed |
| pmr_list_test.test | 10 | 0.000924s | 0.000922s | 0.000925s | byte equality observed |
| pmr_map_test.test | 10 | 0.000917s | 0.000929s | 0.000912s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000918s | 0.000924s | 0.000913s | byte equality observed |
| pmr_set_test.test | 10 | 0.000930s | 0.000928s | 0.000908s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000919s | 0.000936s | 0.000904s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000922s | 0.000911s | 0.000927s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000941s | 0.000939s | 0.000943s | byte equality observed |
| pmr_string_test.test | 10 | 0.000446s | 0.000447s | 0.000937s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000448s | 0.000438s | 0.000942s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000992s | 0.001022s | 0.000988s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000968s | 0.000998s | 0.000962s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000972s | 0.000974s | 0.000964s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000974s | 0.000957s | 0.000975s | byte equality observed |
| segtor_options_test.test | 10 | 0.001003s | 0.001007s | 0.000974s | byte equality observed |
| segtor_test.test | 1 | 0.304585s | 0.282338s | 0.279977s | byte equality observed |
| set_test.test | 3 | 0.008824s | 0.007988s | 0.008700s | byte equality observed |
| slist_test.test | 2 | 0.013034s | 0.013427s | 0.013125s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001061s | 0.001092s | 0.000979s | byte equality observed |
| small_vector_test.test | 7 | 0.002089s | 0.001715s | 0.002791s | byte equality observed |
| stable_vector_test.test | 1 | 0.021465s | 0.022113s | 0.022122s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001024s | 0.001072s | 0.000976s | byte equality observed |
| static_vector_test.test | 10 | 0.001289s | 0.001184s | 0.001187s | byte equality observed |
| string_options_test.test | 10 | 0.000996s | 0.001001s | 0.000960s | byte equality observed |
| string_test.test | 3 | 0.008370s | 0.008971s | 0.008682s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000957s | 0.001001s | 0.000906s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000979s | 0.001000s | 0.000952s | byte equality observed |
| throw_exception_test.test | 10 | 0.000919s | 0.000932s | 0.000900s | byte equality observed |
| tree_test.test | 10 | 0.000921s | 0.000926s | 0.000880s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000968s | 0.000986s | 0.000951s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000930s | 0.000926s | 0.000908s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000444s | 0.000459s | 0.000925s | byte equality observed |
| vector_options_test.test | 10 | 0.001067s | 0.001127s | 0.001071s | byte equality observed |
| vector_test.test | 5 | 0.003323s | 0.003237s | 0.004071s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
