# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 7.800435 | 44.284321 | 40.275208 | 0.008180 | 0.006703 | 0.009926 |
| vector_options_test.test | RUN | 0.475614 | 2.076143 | 1.916865 | 0.002402 | 0.002446 | 0.002700 |
| uses_allocator_test.test | RUN | 0.094894 | 0.171061 | 0.251490 | 0.000906 | 0.000897 | 0.002178 |
| uses_allocator_construction_test.test | RUN | 0.276606 | 0.648568 | 0.810843 | 0.002188 | 0.002061 | 0.002246 |
| unsynchronized_pool_resource_test.test | RUN | 0.390217 | 1.878951 | 1.102603 | 0.002125 | 0.002492 | 0.002274 |
| tree_test.test | RUN | 1.329699 | 6.638484 | 5.958212 | 0.002178 | 0.002294 | 0.001960 |
| throw_exception_test.test | RUN | 0.186102 | 0.405767 | 0.632499 | 0.001908 | 0.002264 | 0.002250 |
| synchronized_pool_resource_test.test | RUN | 0.372009 | 1.938487 | 1.155845 | 0.002282 | 0.002357 | 0.002242 |
| string_view_compat_test.test | RUN | 0.590926 | 3.790212 | 2.302815 | 0.001944 | 0.002353 | 0.002290 |
| string_test.test | RUN | 2.421755 | 14.101375 | 11.309490 | 0.022800 | 0.022159 | 0.023032 |
| string_options_test.test | RUN | 0.407024 | 3.133442 | 1.718955 | 0.002302 | 0.002430 | 0.002018 |
| static_vector_test.test | RUN | 2.717252 | 11.704186 | 11.123377 | 0.003046 | 0.003099 | 0.003061 |
| static_vector_options_test.test | RUN | 0.314716 | 0.850497 | 1.082044 | 0.002361 | 0.002472 | 0.002270 |
| stable_vector_test.test | RUN | 5.616073 | 23.892968 | 20.898062 | 0.038723 | 0.038163 | 0.039232 |
| small_vector_test.test | RUN | 3.100920 | 16.701897 | 15.169811 | 0.005393 | 0.004225 | 0.007169 |
| small_vector_options_test.test | RUN | 0.506608 | 2.052740 | 2.103285 | 0.002337 | 0.002548 | 0.002354 |
| slist_test.test | RUN | 2.549508 | 15.329670 | 13.628794 | 0.034596 | 0.033663 | 0.034255 |
| set_test.test | RUN | 4.370275 | 28.500555 | 24.184898 | 0.020588 | 0.018531 | 0.020017 |
| segtor_test.test | RUN | 6.099872 | 30.491444 | 26.981495 | 0.938634 | 0.911376 | 0.899508 |
| segtor_options_test.test | RUN | 0.363095 | 0.926646 | 1.322383 | 0.002232 | 0.002316 | 0.002250 |
| scoped_allocator_usage_test.test | RUN | 1.141997 | 4.468826 | 4.895135 | 0.002218 | 0.002248 | 0.002233 |
| scoped_allocator_adaptor_test.test | RUN | 0.592873 | 1.363951 | 2.207220 | 0.002047 | 0.002304 | 0.002107 |
| resource_adaptor_test.test | RUN | 0.304934 | 0.822405 | 0.968219 | 0.002196 | 0.002222 | 0.001936 |
| polymorphic_allocator_test.test | RUN | 0.264800 | 0.680060 | 0.818599 | 0.001787 | 0.001959 | 0.001979 |
| pmr_vector_test.test | RUN | 0.145076 | 0.266696 | 0.457364 | 0.000724 | 0.000963 | 0.002179 |
| pmr_string_test.test | RUN | 0.278471 | 0.553300 | 0.809084 | 0.000937 | 0.000947 | 0.001998 |
| pmr_stable_vector_test.test | RUN | 0.285067 | 0.838482 | 1.123716 | 0.001928 | 0.002190 | 0.002090 |
| pmr_small_vector_test.test | RUN | 0.167859 | 0.348464 | 0.531457 | 0.002012 | 0.002216 | 0.002206 |
| pmr_slist_test.test | RUN | 0.223000 | 1.026851 | 0.967921 | 0.002048 | 0.002158 | 0.001838 |
| pmr_set_test.test | RUN | 0.261446 | 0.585960 | 0.884687 | 0.001993 | 0.002234 | 0.002007 |
| pmr_nest_test.test | RUN | 0.206269 | 0.407308 | 0.610312 | 0.001824 | 0.002113 | 0.002253 |
| pmr_map_test.test | RUN | 0.269803 | 0.617637 | 0.845267 | 0.001679 | 0.002238 | 0.002255 |
| pmr_list_test.test | RUN | 0.178434 | 0.373860 | 0.559082 | 0.002176 | 0.002178 | 0.002272 |
| pmr_hub_test.test | RUN | 0.260868 | 0.537243 | 0.725580 | 0.002080 | 0.002218 | 0.001965 |
| pmr_flat_set_test.test | RUN | 0.190235 | 0.397019 | 0.626123 | 0.002023 | 0.002029 | 0.002151 |
| pmr_flat_map_test.test | RUN | 0.238857 | 0.449313 | 0.657716 | 0.002030 | 0.001881 | 0.001892 |
| pmr_devector_test.test | RUN | 0.155559 | 0.396841 | 0.574048 | 0.001823 | 0.002032 | 0.002028 |
| pmr_deque_test.test | RUN | 0.156853 | 0.329299 | 0.498183 | 0.002150 | 0.002140 | 0.001880 |
| pair_test.test | RUN | 0.337098 | 0.802108 | 1.215221 | 0.002034 | 0.002263 | 0.002158 |
| null_iterators_test.test | RUN | 0.649731 | 1.541436 | 1.975691 | 0.002026 | 0.002193 | 0.002016 |
| node_handle_test.test | RUN | 0.291222 | 1.953777 | 0.823641 | 0.002046 | 0.002280 | 0.002070 |
| node_allocator_test.test | RUN | 3.104498 | 18.992606 | 16.961751 | 0.013137 | 0.013055 | 0.014127 |
| new_delete_resource_test.test | RUN | 0.352333 | 1.186257 | 1.102089 | 0.003135 | 0.003386 | 0.003056 |
| nest_test.test | RUN | 0.827502 | 5.210306 | 3.580164 | 0.002135 | 0.002428 | 0.001974 |
| nest_api_test.test | RUN | 1.570615 | 9.922357 | 7.352751 | 0.003686 | 0.003483 | 0.003330 |
| monotonic_buffer_resource_test.test | RUN | 0.381356 | 2.002391 | 0.959835 | 0.002265 | 0.001972 | 0.002274 |
| memory_resource_test.test | RUN | 0.225151 | 0.511402 | 0.641370 | 0.002130 | 0.002010 | 0.002222 |
| map_test.test | RUN | 6.479880 | 31.756794 | 35.639188 | 0.014600 | 0.015233 | 0.014541 |
| list_test.test | RUN | 2.957162 | 16.264815 | 14.709677 | 0.032566 | 0.034830 | 0.035479 |
| is_constructible_test.test | RUN | 0.123966 | 0.215819 | 0.403397 | 0.000910 | 0.000958 | 0.001860 |
| insert_vs_emplace_test.test | RUN | 0.352357 | 1.364659 | 2.150259 | 0.002054 | 0.002325 | 0.002118 |
| hub_uses_allocator_test.test | RUN | 0.429000 | 1.162299 | 1.458650 | 0.002130 | 0.002345 | 0.002150 |
| hub_stability_test.test | RUN | 0.804494 | 5.286622 | 4.863289 | 0.002586 | 0.002731 | 0.002638 |
| hub_sort_test.test | RUN | 0.509607 | 2.435627 | 2.431224 | 0.695743 | 0.641532 | 0.626369 |
| hub_pocxx_test.test | RUN | 0.497005 | 2.872788 | 1.842595 | 0.001848 | 0.002230 | 0.002223 |
| hub_new_extended_alignment_test.test | RUN | 0.297719 | 0.888216 | 1.010376 | 0.002212 | 0.002353 | 0.001850 |
| hub_exception_safety_test.test | RUN | 1.053716 | 6.601755 | 5.163632 | 0.114909 | 0.116984 | 0.118134 |
| hub_empty_sort_test.test | RUN | 0.337008 | 0.799546 | 1.143102 | 0.002146 | 0.002151 | 0.002214 |
| hub_api_test.test | RUN | 3.008579 | 27.418382 | 16.365324 | 0.005826 | 0.004568 | 0.004473 |
| global_resource_test.test | RUN | 0.197553 | 0.564832 | 0.611781 | 0.002072 | 0.002207 | 0.002294 |
| flat_tree_test.test | RUN | 5.253543 | 40.166479 | 28.442157 | 0.002058 | 0.002137 | 0.001960 |
| flat_set_test.test | RUN | 6.376849 | 50.678516 | 37.466851 | 0.010981 | 0.009994 | 0.010874 |
| flat_set_adaptor_test.test | RUN | 5.009774 | 45.909722 | 29.942317 | 0.011313 | 0.010773 | 0.011046 |
| flat_map_test.test | RUN | 10.995408 | 76.026840 | 61.735664 | 0.012080 | 0.012102 | 0.011317 |
| flat_map_adaptor_test.test | RUN | 7.139934 | 55.232565 | 43.553745 | 0.011221 | 0.010732 | 0.010604 |
| explicit_inst_vector_test.test | RUN | 1.131613 | 7.511717 | 6.599565 | 0.002150 | 0.002228 | 0.002130 |
| explicit_inst_string_test.test | RUN | 1.487767 | 13.038686 | 8.775991 | 0.002118 | 0.002234 | 0.002269 |
| explicit_inst_static_vector_test.test | RUN | 0.170827 | 0.512573 | 0.688181 | 0.002100 | 0.001986 | 0.002132 |
| explicit_inst_stable_vector_test.test | RUN | 0.999936 | 4.704904 | 4.274099 | 0.002022 | 0.002196 | 0.002106 |
| explicit_inst_small_vector_test.test | RUN | 0.786878 | 4.284916 | 3.997293 | 0.001949 | 0.002205 | 0.002352 |
| explicit_inst_slist_test.test | RUN | 0.613104 | 3.885404 | 3.204483 | 0.002072 | 0.002089 | 0.002253 |
| explicit_inst_set_test.test | RUN | 0.914063 | 4.506485 | 3.796238 | 0.002158 | 0.002298 | 0.002236 |
| explicit_inst_map_test.test | RUN | 0.989822 | 4.681171 | 4.105036 | 0.002058 | 0.002345 | 0.002286 |
| explicit_inst_list_test.test | RUN | 0.225709 | 1.095363 | 1.044484 | 0.002134 | 0.002336 | 0.002206 |
| explicit_inst_flat_set_test.test | RUN | 4.579880 | 34.637529 | 22.788515 | 0.002228 | 0.002186 | 0.002190 |
| explicit_inst_flat_map_test.test | RUN | 5.212613 | 35.400063 | 25.557977 | 0.002104 | 0.001894 | 0.001965 |
| explicit_inst_devector_test.test | RUN | 0.640781 | 4.318198 | 3.177776 | 0.001942 | 0.001992 | 0.002007 |
| explicit_inst_deque_test.test | RUN | 0.701525 | 4.607260 | 3.431876 | 0.002082 | 0.002134 | 0.001962 |
| devector_test.test | RUN | 9.294828 | 54.787249 | 40.042577 | 0.004897 | 0.004350 | 0.005570 |
| devector_options_test.test | RUN | 0.430149 | 2.583999 | 2.074567 | 0.002595 | 0.002569 | 0.002556 |
| deque_test.test | RUN | 5.786108 | 32.146201 | 29.902767 | 0.436793 | 0.432173 | 0.540196 |
| deque_options_test.test | RUN | 0.357264 | 0.996556 | 1.341375 | 0.002220 | 0.002144 | 0.002198 |
| copy_move_algo_test.test | RUN | 0.353391 | 1.159029 | 1.245737 | 0.002486 | 0.002476 | 0.002250 |
| common_iterator_test.test | RUN | 0.027197 | 0.063396 | 0.058507 | 0.000817 | 0.000986 | 0.002110 |
| boost_iterator_comp_test.test | RUN | 0.486679 | 1.461358 | 1.978507 | 0.002144 | 0.002376 | 0.002134 |
| allocator_traits_test.test | RUN | 0.294749 | 0.632636 | 0.987367 | 0.002119 | 0.002258 | 0.002244 |
| alloc_full_test.test | RUN | 0.305633 | 1.273209 | 1.330870 | 0.006291 | 0.005884 | 0.006487 |
| alloc_basic_test.test | RUN | 0.265780 | 0.953437 | 1.003330 | 0.002276 | 0.002128 | 0.002350 |
| adaptive_pool_test.test | RUN | 3.028109 | 12.897736 | 16.935674 | 0.013060 | 0.011754 | 0.019775 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.949× | 0.952×, 0.946× | 1.013× |
| clang | 0.998× | 0.999×, 0.997× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.014056s | 0.012287s | 0.013766s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001341s | 0.001359s | 0.001360s | byte equality observed |
| alloc_full_test.test | 6 | 0.003973s | 0.004002s | 0.004098s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001261s | 0.001231s | 0.001209s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001306s | 0.001277s | 0.001255s | byte equality observed |
| common_iterator_test.test | 10 | 0.000609s | 0.000626s | 0.001297s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001483s | 0.001380s | 0.001442s | byte equality observed |
| deque_options_test.test | 10 | 0.001360s | 0.001317s | 0.001309s | byte equality observed |
| deque_test.test | 1 | 0.495437s | 0.474302s | 0.587908s | byte equality observed |
| devector_options_test.test | 10 | 0.001509s | 0.001381s | 0.001364s | byte equality observed |
| devector_test.test | 4 | 0.004372s | 0.003895s | 0.005030s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.001213s | 0.001220s | 0.001204s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.001176s | 0.001197s | 0.001185s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.001200s | 0.001225s | 0.001205s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.001191s | 0.001208s | 0.001215s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.001187s | 0.001180s | 0.001202s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.001204s | 0.001198s | 0.001217s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.001189s | 0.001205s | 0.001219s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.001197s | 0.001208s | 0.001214s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.001205s | 0.001205s | 0.001217s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.001203s | 0.001210s | 0.001210s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.001176s | 0.001185s | 0.001217s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.001201s | 0.001206s | 0.001213s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.001190s | 0.001195s | 0.001211s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.007448s | 0.007027s | 0.007049s | byte equality observed |
| flat_map_test.test | 3 | 0.007992s | 0.007522s | 0.007396s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.007519s | 0.006777s | 0.007489s | byte equality observed |
| flat_set_test.test | 3 | 0.007535s | 0.006527s | 0.007282s | byte equality observed |
| flat_tree_test.test | 10 | 0.001212s | 0.001233s | 0.001238s | byte equality observed |
| global_resource_test.test | 10 | 0.001228s | 0.001249s | 0.001251s | byte equality observed |
| hub_api_test.test | 5 | 0.004132s | 0.002814s | 0.002822s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001188s | 0.001214s | 0.001207s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.081202s | 0.086402s | 0.079079s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001321s | 0.001320s | 0.001239s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001311s | 0.001276s | 0.001280s | byte equality observed |
| hub_sort_test.test | 1 | 0.566660s | 0.530388s | 0.507426s | byte equality observed |
| hub_stability_test.test | 9 | 0.001514s | 0.001496s | 0.001534s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.001203s | 0.001207s | 0.001214s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001237s | 0.001265s | 0.001234s | byte equality observed |
| is_constructible_test.test | 10 | 0.000561s | 0.000570s | 0.001180s | byte equality observed |
| list_test.test | 1 | 0.021753s | 0.021652s | 0.021788s | byte equality observed |
| map_test.test | 2 | 0.009777s | 0.009767s | 0.009671s | byte equality observed |
| memory_resource_test.test | 10 | 0.001255s | 0.001249s | 0.001253s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001230s | 0.001229s | 0.001235s | byte equality observed |
| nest_api_test.test | 8 | 0.002351s | 0.002044s | 0.002081s | byte equality observed |
| nest_test.test | 10 | 0.001280s | 0.001283s | 0.001242s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001818s | 0.001796s | 0.001802s | byte equality observed |
| node_allocator_test.test | 3 | 0.009309s | 0.009212s | 0.009145s | byte equality observed |
| node_handle_test.test | 10 | 0.001272s | 0.001259s | 0.001249s | byte equality observed |
| null_iterators_test.test | 10 | 0.001238s | 0.001185s | 0.001194s | byte equality observed |
| pair_test.test | 10 | 0.001220s | 0.001187s | 0.001213s | byte equality observed |
| pmr_deque_test.test | 10 | 0.001206s | 0.001172s | 0.001197s | byte equality observed |
| pmr_devector_test.test | 10 | 0.001206s | 0.001174s | 0.001194s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.001203s | 0.001165s | 0.001200s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.001205s | 0.001173s | 0.001196s | byte equality observed |
| pmr_hub_test.test | 10 | 0.001212s | 0.001175s | 0.001184s | byte equality observed |
| pmr_list_test.test | 10 | 0.001203s | 0.001188s | 0.001194s | byte equality observed |
| pmr_map_test.test | 10 | 0.001215s | 0.001194s | 0.001191s | byte equality observed |
| pmr_nest_test.test | 10 | 0.001237s | 0.001191s | 0.001191s | byte equality observed |
| pmr_set_test.test | 10 | 0.001200s | 0.001192s | 0.001187s | byte equality observed |
| pmr_slist_test.test | 10 | 0.001208s | 0.001190s | 0.001186s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001214s | 0.001206s | 0.001282s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001205s | 0.001218s | 0.001293s | byte equality observed |
| pmr_string_test.test | 10 | 0.000596s | 0.000573s | 0.001299s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000588s | 0.000561s | 0.001243s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001275s | 0.001251s | 0.001252s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001237s | 0.001293s | 0.001313s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001223s | 0.001309s | 0.001333s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001285s | 0.001311s | 0.001284s | byte equality observed |
| segtor_options_test.test | 10 | 0.001363s | 0.001292s | 0.001313s | byte equality observed |
| segtor_test.test | 1 | 0.479363s | 0.456873s | 0.435824s | byte equality observed |
| set_test.test | 2 | 0.014040s | 0.012135s | 0.013472s | byte equality observed |
| slist_test.test | 1 | 0.021355s | 0.021201s | 0.020864s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001393s | 0.001338s | 0.001377s | byte equality observed |
| small_vector_test.test | 5 | 0.003060s | 0.002338s | 0.004267s | byte equality observed |
| stable_vector_test.test | 1 | 0.031068s | 0.030624s | 0.031552s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001349s | 0.001286s | 0.001331s | byte equality observed |
| static_vector_test.test | 10 | 0.001751s | 0.001528s | 0.001603s | byte equality observed |
| string_options_test.test | 10 | 0.001304s | 0.001275s | 0.001256s | byte equality observed |
| string_test.test | 2 | 0.015193s | 0.015723s | 0.015181s | byte equality observed |
| string_view_compat_test.test | 8 | 0.001290s | 0.001254s | 0.001246s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001301s | 0.001267s | 0.001274s | byte equality observed |
| throw_exception_test.test | 10 | 0.001208s | 0.001183s | 0.001198s | byte equality observed |
| tree_test.test | 10 | 0.001234s | 0.001188s | 0.001197s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001314s | 0.001266s | 0.001273s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.001214s | 0.001206s | 0.001198s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000589s | 0.000593s | 0.001191s | byte equality observed |
| vector_options_test.test | 10 | 0.001446s | 0.001405s | 0.001377s | byte equality observed |
| vector_test.test | 4 | 0.004795s | 0.004214s | 0.005774s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
