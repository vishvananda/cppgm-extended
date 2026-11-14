# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 6.619782 | 29.570352 | 24.175253 | 0.004419 | 0.004383 | 0.005770 |
| vector_options_test.test | RUN | 0.269971 | 1.275313 | 0.960966 | 0.001532 | 0.001454 | 0.001214 |
| uses_allocator_test.test | RUN | 0.042451 | 0.096144 | 0.117778 | 0.000447 | 0.000482 | 0.001086 |
| uses_allocator_construction_test.test | RUN | 0.132849 | 0.371098 | 0.435513 | 0.001342 | 0.001230 | 0.001170 |
| unsynchronized_pool_resource_test.test | RUN | 0.178670 | 1.200529 | 0.554076 | 0.000938 | 0.001243 | 0.001133 |
| tree_test.test | RUN | 0.835108 | 4.315723 | 3.317007 | 0.001183 | 0.001297 | 0.001156 |
| throw_exception_test.test | RUN | 0.095556 | 0.238865 | 0.278643 | 0.001010 | 0.001157 | 0.001103 |
| synchronized_pool_resource_test.test | RUN | 0.175966 | 1.182181 | 0.560562 | 0.001282 | 0.001338 | 0.001171 |
| string_view_compat_test.test | RUN | 0.300958 | 2.399435 | 1.099020 | 0.001029 | 0.001221 | 0.001162 |
| string_test.test | RUN | 1.464577 | 9.237539 | 6.232022 | 0.014348 | 0.012509 | 0.013103 |
| string_options_test.test | RUN | 0.236886 | 1.917860 | 0.981067 | 0.001180 | 0.001255 | 0.001291 |
| static_vector_test.test | RUN | 1.605856 | 7.627655 | 6.319985 | 0.001754 | 0.001590 | 0.001597 |
| static_vector_options_test.test | RUN | 0.170786 | 0.485676 | 0.544564 | 0.001596 | 0.001314 | 0.001236 |
| stable_vector_test.test | RUN | 3.742392 | 16.214191 | 12.383625 | 0.031476 | 0.032064 | 0.033521 |
| small_vector_test.test | RUN | 2.477628 | 11.247520 | 8.859615 | 0.002713 | 0.002553 | 0.004361 |
| small_vector_options_test.test | RUN | 0.307555 | 1.317662 | 1.113824 | 0.001367 | 0.001340 | 0.001304 |
| slist_test.test | RUN | 2.293393 | 10.197888 | 7.845569 | 0.022070 | 0.022567 | 0.022623 |
| set_test.test | RUN | 3.363738 | 19.022219 | 14.595569 | 0.014012 | 0.012559 | 0.013669 |
| segtor_test.test | RUN | 4.489112 | 20.712554 | 16.266163 | 0.700339 | 0.739942 | 0.727691 |
| segtor_options_test.test | RUN | 0.192292 | 0.582766 | 0.716310 | 0.001347 | 0.001344 | 0.001265 |
| scoped_allocator_usage_test.test | RUN | 0.743546 | 2.971892 | 2.656332 | 0.001110 | 0.001164 | 0.001394 |
| scoped_allocator_adaptor_test.test | RUN | 0.312329 | 0.829118 | 1.128396 | 0.001239 | 0.001308 | 0.001290 |
| resource_adaptor_test.test | RUN | 0.152507 | 0.500018 | 0.510142 | 0.001217 | 0.001316 | 0.001154 |
| polymorphic_allocator_test.test | RUN | 0.126371 | 0.388303 | 0.430688 | 0.001236 | 0.001212 | 0.001245 |
| pmr_vector_test.test | RUN | 0.070218 | 0.141141 | 0.199441 | 0.000463 | 0.000462 | 0.001295 |
| pmr_string_test.test | RUN | 0.142733 | 0.306934 | 0.425148 | 0.000467 | 0.000535 | 0.001294 |
| pmr_stable_vector_test.test | RUN | 0.176485 | 0.507595 | 0.582853 | 0.001163 | 0.001298 | 0.001130 |
| pmr_small_vector_test.test | RUN | 0.085641 | 0.196104 | 0.270099 | 0.001108 | 0.001184 | 0.001150 |
| pmr_slist_test.test | RUN | 0.127010 | 0.698355 | 0.593759 | 0.001119 | 0.001195 | 0.001153 |
| pmr_set_test.test | RUN | 0.137012 | 0.339250 | 0.432842 | 0.001124 | 0.001250 | 0.001121 |
| pmr_nest_test.test | RUN | 0.106648 | 0.227351 | 0.278427 | 0.001164 | 0.001304 | 0.001208 |
| pmr_map_test.test | RUN | 0.143208 | 0.350283 | 0.433544 | 0.001208 | 0.001365 | 0.001125 |
| pmr_list_test.test | RUN | 0.091330 | 0.212580 | 0.282900 | 0.001154 | 0.001178 | 0.001219 |
| pmr_hub_test.test | RUN | 0.133047 | 0.295678 | 0.376103 | 0.001284 | 0.001109 | 0.001140 |
| pmr_flat_set_test.test | RUN | 0.102170 | 0.236796 | 0.334043 | 0.001128 | 0.001287 | 0.001134 |
| pmr_flat_map_test.test | RUN | 0.126166 | 0.269003 | 0.382814 | 0.001141 | 0.001151 | 0.001160 |
| pmr_devector_test.test | RUN | 0.084733 | 0.241293 | 0.268065 | 0.001299 | 0.001266 | 0.001146 |
| pmr_deque_test.test | RUN | 0.081113 | 0.196021 | 0.268871 | 0.001139 | 0.001397 | 0.001197 |
| pair_test.test | RUN | 0.206752 | 0.462059 | 0.603044 | 0.001232 | 0.001327 | 0.001193 |
| null_iterators_test.test | RUN | 0.319544 | 0.920094 | 1.023432 | 0.001280 | 0.001352 | 0.001093 |
| node_handle_test.test | RUN | 0.148280 | 1.201255 | 0.405481 | 0.001245 | 0.001326 | 0.001208 |
| node_allocator_test.test | RUN | 2.729851 | 12.407468 | 9.885002 | 0.009175 | 0.008751 | 0.008469 |
| new_delete_resource_test.test | RUN | 0.178154 | 0.712332 | 0.567568 | 0.001888 | 0.001768 | 0.001674 |
| nest_test.test | RUN | 0.495848 | 3.337661 | 2.057433 | 0.001231 | 0.001279 | 0.001281 |
| nest_api_test.test | RUN | 0.946972 | 6.521197 | 4.127282 | 0.002030 | 0.002423 | 0.002317 |
| monotonic_buffer_resource_test.test | RUN | 0.179486 | 1.311385 | 0.574196 | 0.001097 | 0.001355 | 0.001155 |
| memory_resource_test.test | RUN | 0.088998 | 0.365754 | 0.262366 | 0.001132 | 0.001370 | 0.001023 |
| map_test.test | RUN | 4.753338 | 21.321375 | 20.303183 | 0.010647 | 0.009983 | 0.009770 |
| list_test.test | RUN | 2.397261 | 10.949525 | 8.401144 | 0.021900 | 0.022071 | 0.022556 |
| is_constructible_test.test | RUN | 0.065550 | 0.129003 | 0.193311 | 0.000480 | 0.000518 | 0.001268 |
| insert_vs_emplace_test.test | RUN | 0.221310 | 0.850686 | 1.131491 | 0.001310 | 0.001367 | 0.001159 |
| hub_uses_allocator_test.test | RUN | 0.226586 | 0.704058 | 0.780809 | 0.001108 | 0.001238 | 0.001187 |
| hub_stability_test.test | RUN | 0.522303 | 3.444325 | 2.891257 | 0.001513 | 0.001607 | 0.001500 |
| hub_sort_test.test | RUN | 0.330544 | 1.540786 | 1.400988 | 0.538873 | 0.540035 | 0.505931 |
| hub_pocxx_test.test | RUN | 0.325324 | 1.809326 | 1.019902 | 0.001391 | 0.001250 | 0.001186 |
| hub_new_extended_alignment_test.test | RUN | 0.156337 | 0.523491 | 0.546730 | 0.001221 | 0.001220 | 0.001188 |
| hub_exception_safety_test.test | RUN | 0.651895 | 4.318887 | 2.923605 | 0.059837 | 0.063331 | 0.065278 |
| hub_empty_sort_test.test | RUN | 0.183462 | 0.489419 | 0.599432 | 0.001274 | 0.001371 | 0.001152 |
| hub_api_test.test | RUN | 2.183732 | 16.833409 | 9.432004 | 0.003486 | 0.003144 | 0.003120 |
| global_resource_test.test | RUN | 0.096535 | 0.330121 | 0.308188 | 0.001200 | 0.001336 | 0.001291 |
| flat_tree_test.test | RUN | 4.297202 | 26.727640 | 17.848392 | 0.001230 | 0.001241 | 0.001162 |
| flat_set_test.test | RUN | 5.401933 | 33.864612 | 23.269343 | 0.007357 | 0.006565 | 0.007015 |
| flat_set_adaptor_test.test | RUN | 4.090038 | 29.570909 | 17.953537 | 0.007632 | 0.006897 | 0.007270 |
| flat_map_test.test | RUN | 9.007101 | 50.237853 | 37.734325 | 0.008698 | 0.007828 | 0.007932 |
| flat_map_adaptor_test.test | RUN | 5.725639 | 36.126902 | 26.230770 | 0.007933 | 0.007426 | 0.007379 |
| explicit_inst_vector_test.test | RUN | 0.797026 | 4.974281 | 4.105046 | 0.001105 | 0.001152 | 0.001288 |
| explicit_inst_string_test.test | RUN | 1.064887 | 8.760856 | 5.757309 | 0.001292 | 0.001263 | 0.001185 |
| explicit_inst_static_vector_test.test | RUN | 0.091309 | 0.312789 | 0.381146 | 0.001188 | 0.001169 | 0.001377 |
| explicit_inst_stable_vector_test.test | RUN | 0.648292 | 3.148632 | 2.602541 | 0.001182 | 0.001157 | 0.001265 |
| explicit_inst_small_vector_test.test | RUN | 0.523312 | 2.828233 | 2.306074 | 0.001094 | 0.001301 | 0.001146 |
| explicit_inst_slist_test.test | RUN | 0.384411 | 2.566454 | 1.799792 | 0.001177 | 0.001316 | 0.001256 |
| explicit_inst_set_test.test | RUN | 0.563224 | 2.920778 | 2.052242 | 0.001176 | 0.001240 | 0.001079 |
| explicit_inst_map_test.test | RUN | 0.618087 | 3.072752 | 2.229393 | 0.001174 | 0.001216 | 0.001260 |
| explicit_inst_list_test.test | RUN | 0.130866 | 0.708454 | 0.569894 | 0.001091 | 0.001333 | 0.001113 |
| explicit_inst_flat_set_test.test | RUN | 3.612494 | 22.416878 | 13.668017 | 0.001119 | 0.001235 | 0.001220 |
| explicit_inst_flat_map_test.test | RUN | 3.857532 | 23.749146 | 15.127106 | 0.001063 | 0.001193 | 0.001127 |
| explicit_inst_devector_test.test | RUN | 0.441879 | 2.790095 | 1.893766 | 0.001115 | 0.001234 | 0.001112 |
| explicit_inst_deque_test.test | RUN | 0.445727 | 2.996329 | 2.067637 | 0.001158 | 0.001266 | 0.001102 |
| devector_test.test | RUN | 6.543456 | 36.980098 | 28.562091 | 0.003082 | 0.002902 | 0.003865 |
| devector_options_test.test | RUN | 0.267249 | 1.660705 | 1.047719 | 0.001416 | 0.001425 | 0.001261 |
| deque_test.test | RUN | 4.797710 | 19.345637 | 14.911869 | 0.364113 | 0.359754 | 0.437126 |
| deque_options_test.test | RUN | 0.202210 | 0.571427 | 0.710258 | 0.001210 | 0.001257 | 0.001295 |
| copy_move_algo_test.test | RUN | 0.192006 | 0.726602 | 0.634913 | 0.001514 | 0.001274 | 0.001392 |
| common_iterator_test.test | RUN | 0.012590 | 0.033255 | 0.036829 | 0.000448 | 0.000450 | 0.001130 |
| boost_iterator_comp_test.test | RUN | 0.268405 | 0.844819 | 0.980524 | 0.001195 | 0.001286 | 0.001190 |
| allocator_traits_test.test | RUN | 0.151712 | 0.367865 | 0.472526 | 0.001109 | 0.001246 | 0.001161 |
| alloc_full_test.test | RUN | 0.180961 | 0.802883 | 0.704947 | 0.003752 | 0.003904 | 0.004028 |
| alloc_basic_test.test | RUN | 0.148512 | 0.555909 | 0.519102 | 0.001222 | 0.001213 | 0.001187 |
| adaptive_pool_test.test | RUN | 2.302612 | 12.115349 | 9.533459 | 0.012560 | 0.007765 | 0.012708 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.007× | 1.010×, 1.004× | 0.994× |
| clang | 1.061× | 1.061×, 1.062× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.009025s | 0.008252s | 0.009052s | byte equality observed |
| alloc_basic_test.test | 10 | 0.000806s | 0.000804s | 0.000804s | byte equality observed |
| alloc_full_test.test | 7 | 0.002708s | 0.002695s | 0.002847s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000774s | 0.000780s | 0.000781s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000803s | 0.000793s | 0.000801s | byte equality observed |
| common_iterator_test.test | 10 | 0.000360s | 0.000360s | 0.000792s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001174s | 0.000920s | 0.001003s | byte equality observed |
| deque_options_test.test | 10 | 0.000851s | 0.000836s | 0.000843s | byte equality observed |
| deque_test.test | 1 | 0.377527s | 0.372052s | 0.447496s | byte equality observed |
| devector_options_test.test | 10 | 0.000890s | 0.000843s | 0.000835s | byte equality observed |
| devector_test.test | 5 | 0.002902s | 0.002815s | 0.003626s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000714s | 0.000722s | 0.000715s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000708s | 0.000724s | 0.000720s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000719s | 0.000733s | 0.000719s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000715s | 0.000738s | 0.000723s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000704s | 0.000715s | 0.000700s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000707s | 0.000735s | 0.000717s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000719s | 0.000730s | 0.000712s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000732s | 0.000753s | 0.000718s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000715s | 0.000730s | 0.000710s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000724s | 0.000720s | 0.000724s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000710s | 0.000736s | 0.000716s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000720s | 0.000750s | 0.000714s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000714s | 0.000724s | 0.000722s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005775s | 0.005393s | 0.005438s | byte equality observed |
| flat_map_test.test | 4 | 0.006240s | 0.005769s | 0.005526s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.005551s | 0.005036s | 0.005340s | byte equality observed |
| flat_set_test.test | 4 | 0.005508s | 0.004847s | 0.005187s | byte equality observed |
| flat_tree_test.test | 10 | 0.000713s | 0.000741s | 0.000725s | byte equality observed |
| global_resource_test.test | 10 | 0.000759s | 0.000753s | 0.000753s | byte equality observed |
| hub_api_test.test | 8 | 0.002592s | 0.002250s | 0.002177s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000735s | 0.000723s | 0.000724s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.049346s | 0.054919s | 0.050277s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000741s | 0.000773s | 0.000737s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000759s | 0.000781s | 0.000753s | byte equality observed |
| hub_sort_test.test | 1 | 0.433881s | 0.442647s | 0.415568s | byte equality observed |
| hub_stability_test.test | 10 | 0.000982s | 0.000955s | 0.000987s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000766s | 0.000731s | 0.000769s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000778s | 0.000754s | 0.000775s | byte equality observed |
| is_constructible_test.test | 10 | 0.000353s | 0.000320s | 0.000730s | byte equality observed |
| list_test.test | 2 | 0.015484s | 0.015580s | 0.016030s | byte equality observed |
| map_test.test | 3 | 0.007654s | 0.007332s | 0.007264s | byte equality observed |
| memory_resource_test.test | 10 | 0.000775s | 0.000743s | 0.000761s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000775s | 0.000772s | 0.000775s | byte equality observed |
| nest_api_test.test | 10 | 0.001658s | 0.001629s | 0.001657s | byte equality observed |
| nest_test.test | 10 | 0.000788s | 0.000798s | 0.000790s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001132s | 0.001132s | 0.001125s | byte equality observed |
| node_allocator_test.test | 3 | 0.006638s | 0.006673s | 0.006731s | byte equality observed |
| node_handle_test.test | 10 | 0.000769s | 0.000751s | 0.000756s | byte equality observed |
| null_iterators_test.test | 10 | 0.000763s | 0.000762s | 0.000744s | byte equality observed |
| pair_test.test | 10 | 0.000756s | 0.000741s | 0.000752s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000740s | 0.000736s | 0.000743s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000747s | 0.000741s | 0.000747s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000751s | 0.000724s | 0.000757s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000742s | 0.000730s | 0.000744s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000750s | 0.000753s | 0.000754s | byte equality observed |
| pmr_list_test.test | 10 | 0.000757s | 0.000745s | 0.000748s | byte equality observed |
| pmr_map_test.test | 10 | 0.000750s | 0.000740s | 0.000756s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000751s | 0.000739s | 0.000726s | byte equality observed |
| pmr_set_test.test | 10 | 0.000759s | 0.000724s | 0.000719s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000750s | 0.000726s | 0.000714s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000754s | 0.000736s | 0.000730s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000753s | 0.000731s | 0.000775s | byte equality observed |
| pmr_string_test.test | 10 | 0.000356s | 0.000343s | 0.000788s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000349s | 0.000349s | 0.000779s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000766s | 0.000766s | 0.000797s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000775s | 0.000750s | 0.000833s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000762s | 0.000757s | 0.000828s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000781s | 0.000762s | 0.000847s | byte equality observed |
| segtor_options_test.test | 10 | 0.000815s | 0.000804s | 0.000786s | byte equality observed |
| segtor_test.test | 1 | 0.463828s | 0.468357s | 0.505937s | byte equality observed |
| set_test.test | 2 | 0.010768s | 0.009531s | 0.010318s | byte equality observed |
| slist_test.test | 2 | 0.016360s | 0.016094s | 0.016118s | byte equality observed |
| small_vector_options_test.test | 10 | 0.000838s | 0.000818s | 0.000800s | byte equality observed |
| small_vector_test.test | 7 | 0.001698s | 0.001592s | 0.002726s | byte equality observed |
| stable_vector_test.test | 1 | 0.027759s | 0.028031s | 0.028580s | byte equality observed |
| static_vector_options_test.test | 10 | 0.000789s | 0.000760s | 0.000773s | byte equality observed |
| static_vector_test.test | 10 | 0.001069s | 0.000980s | 0.001007s | byte equality observed |
| string_options_test.test | 10 | 0.000790s | 0.000783s | 0.000754s | byte equality observed |
| string_test.test | 3 | 0.009260s | 0.008970s | 0.009309s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000744s | 0.000739s | 0.000722s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000771s | 0.000787s | 0.000757s | byte equality observed |
| throw_exception_test.test | 10 | 0.000717s | 0.000720s | 0.000711s | byte equality observed |
| tree_test.test | 10 | 0.000730s | 0.000740s | 0.000690s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000768s | 0.000783s | 0.000758s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000732s | 0.000736s | 0.000713s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000322s | 0.000351s | 0.000701s | byte equality observed |
| vector_options_test.test | 10 | 0.000840s | 0.000841s | 0.000830s | byte equality observed |
| vector_test.test | 5 | 0.002975s | 0.002986s | 0.003920s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
