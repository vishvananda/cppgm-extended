# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 7.778826 | 33.728284 | 27.295542 | 0.005436 | 0.004926 | 0.007118 |
| vector_options_test.test | RUN | 0.320500 | 1.480803 | 1.133154 | 0.001703 | 0.001564 | 0.001567 |
| uses_allocator_test.test | RUN | 0.054168 | 0.117654 | 0.140504 | 0.000587 | 0.000577 | 0.001339 |
| uses_allocator_construction_test.test | RUN | 0.166639 | 0.445478 | 0.508522 | 0.001485 | 0.001458 | 0.001349 |
| unsynchronized_pool_resource_test.test | RUN | 0.214749 | 1.376094 | 0.650400 | 0.001548 | 0.001434 | 0.001352 |
| tree_test.test | RUN | 1.016595 | 5.104603 | 3.801921 | 0.001345 | 0.001296 | 0.001241 |
| throw_exception_test.test | RUN | 0.114789 | 0.307097 | 0.330369 | 0.001495 | 0.001318 | 0.001366 |
| synchronized_pool_resource_test.test | RUN | 0.211640 | 1.372006 | 0.659743 | 0.001429 | 0.001424 | 0.001450 |
| string_view_compat_test.test | RUN | 0.358309 | 2.777792 | 1.291491 | 0.001388 | 0.001401 | 0.001336 |
| string_test.test | RUN | 1.767049 | 10.649140 | 7.189166 | 0.016993 | 0.014253 | 0.015160 |
| string_options_test.test | RUN | 0.293827 | 2.241871 | 1.133796 | 0.001533 | 0.001481 | 0.001404 |
| static_vector_test.test | RUN | 1.899464 | 8.872242 | 7.262184 | 0.002044 | 0.001870 | 0.001824 |
| static_vector_options_test.test | RUN | 0.205725 | 0.576093 | 0.628522 | 0.001686 | 0.001438 | 0.001434 |
| stable_vector_test.test | RUN | 4.388602 | 18.459819 | 13.956004 | 0.035295 | 0.034994 | 0.036593 |
| small_vector_test.test | RUN | 2.930376 | 12.846006 | 10.115180 | 0.003244 | 0.002863 | 0.005122 |
| small_vector_options_test.test | RUN | 0.373571 | 1.512112 | 1.278711 | 0.001616 | 0.001480 | 0.001391 |
| slist_test.test | RUN | 2.677655 | 11.598975 | 8.945680 | 0.026295 | 0.025854 | 0.025858 |
| set_test.test | RUN | 3.958647 | 21.531811 | 16.161326 | 0.015940 | 0.014437 | 0.015383 |
| segtor_test.test | RUN | 5.461507 | 23.722265 | 18.416815 | 0.602688 | 0.558926 | 0.611203 |
| segtor_options_test.test | RUN | 0.233446 | 0.653075 | 0.819066 | 0.001426 | 0.001450 | 0.001385 |
| scoped_allocator_usage_test.test | RUN | 0.906750 | 3.456204 | 3.047987 | 0.001254 | 0.001485 | 0.001481 |
| scoped_allocator_adaptor_test.test | RUN | 0.381373 | 0.949812 | 1.276312 | 0.001361 | 0.001320 | 0.001254 |
| resource_adaptor_test.test | RUN | 0.185034 | 0.579107 | 0.587723 | 0.001312 | 0.001357 | 0.001210 |
| polymorphic_allocator_test.test | RUN | 0.151073 | 0.462416 | 0.488909 | 0.001261 | 0.001389 | 0.001234 |
| pmr_vector_test.test | RUN | 0.085220 | 0.175715 | 0.235330 | 0.000525 | 0.000537 | 0.001113 |
| pmr_string_test.test | RUN | 0.169197 | 0.368845 | 0.476997 | 0.000506 | 0.000521 | 0.001324 |
| pmr_stable_vector_test.test | RUN | 0.213216 | 0.606003 | 0.671272 | 0.001318 | 0.001266 | 0.001199 |
| pmr_small_vector_test.test | RUN | 0.102561 | 0.238159 | 0.311078 | 0.001227 | 0.001288 | 0.001179 |
| pmr_slist_test.test | RUN | 0.149150 | 0.809779 | 0.667556 | 0.001336 | 0.001560 | 0.001177 |
| pmr_set_test.test | RUN | 0.163014 | 0.408382 | 0.499368 | 0.001338 | 0.001416 | 0.001222 |
| pmr_nest_test.test | RUN | 0.126266 | 0.267660 | 0.385700 | 0.001254 | 0.001308 | 0.001435 |
| pmr_map_test.test | RUN | 0.173137 | 0.418377 | 0.558626 | 0.001191 | 0.001304 | 0.001265 |
| pmr_list_test.test | RUN | 0.111696 | 0.259141 | 0.349819 | 0.001260 | 0.001386 | 0.001225 |
| pmr_hub_test.test | RUN | 0.152507 | 0.353231 | 0.412533 | 0.001310 | 0.001234 | 0.001146 |
| pmr_flat_set_test.test | RUN | 0.126323 | 0.277385 | 0.376605 | 0.001233 | 0.001401 | 0.001240 |
| pmr_flat_map_test.test | RUN | 0.149287 | 0.320805 | 0.393048 | 0.001266 | 0.001262 | 0.001224 |
| pmr_devector_test.test | RUN | 0.098116 | 0.281832 | 0.330677 | 0.001304 | 0.001365 | 0.001248 |
| pmr_deque_test.test | RUN | 0.095988 | 0.230324 | 0.300484 | 0.001292 | 0.001401 | 0.001102 |
| pair_test.test | RUN | 0.247164 | 0.553526 | 0.681933 | 0.001320 | 0.001319 | 0.001241 |
| null_iterators_test.test | RUN | 0.378635 | 1.096376 | 1.135616 | 0.001573 | 0.001479 | 0.001255 |
| node_handle_test.test | RUN | 0.173969 | 1.391351 | 0.497877 | 0.001208 | 0.001480 | 0.001220 |
| node_allocator_test.test | RUN | 3.229386 | 14.521049 | 11.104741 | 0.010372 | 0.010366 | 0.010399 |
| new_delete_resource_test.test | RUN | 0.216184 | 0.861834 | 0.663287 | 0.002534 | 0.002176 | 0.001824 |
| nest_test.test | RUN | 0.585901 | 3.960614 | 2.393339 | 0.001395 | 0.001482 | 0.001340 |
| nest_api_test.test | RUN | 1.100957 | 7.507491 | 4.826466 | 0.002358 | 0.002547 | 0.002412 |
| monotonic_buffer_resource_test.test | RUN | 0.209199 | 1.502768 | 0.694212 | 0.001549 | 0.001457 | 0.001383 |
| memory_resource_test.test | RUN | 0.111941 | 0.422203 | 0.385353 | 0.001221 | 0.001378 | 0.001264 |
| map_test.test | RUN | 5.659388 | 24.474946 | 23.517658 | 0.012198 | 0.011516 | 0.011269 |
| list_test.test | RUN | 2.836633 | 12.572679 | 9.773958 | 0.025358 | 0.024943 | 0.025898 |
| is_constructible_test.test | RUN | 0.077485 | 0.152581 | 0.213963 | 0.000535 | 0.000553 | 0.001258 |
| insert_vs_emplace_test.test | RUN | 0.267368 | 0.999693 | 1.293891 | 0.001425 | 0.001510 | 0.001373 |
| hub_uses_allocator_test.test | RUN | 0.283999 | 0.824470 | 0.911526 | 0.001336 | 0.001479 | 0.001393 |
| hub_stability_test.test | RUN | 0.622925 | 3.953450 | 3.335297 | 0.001764 | 0.001778 | 0.001673 |
| hub_sort_test.test | RUN | 0.392291 | 1.815399 | 1.532661 | 0.629345 | 0.612609 | 0.596719 |
| hub_pocxx_test.test | RUN | 0.388358 | 2.094405 | 1.164296 | 0.001384 | 0.001534 | 0.001372 |
| hub_new_extended_alignment_test.test | RUN | 0.192188 | 0.632423 | 0.592591 | 0.001364 | 0.001454 | 0.001258 |
| hub_exception_safety_test.test | RUN | 0.765037 | 5.045760 | 3.194817 | 0.078116 | 0.077424 | 0.074514 |
| hub_empty_sort_test.test | RUN | 0.216973 | 0.582353 | 0.710990 | 0.001333 | 0.001381 | 0.001366 |
| hub_api_test.test | RUN | 2.589230 | 19.945664 | 10.591249 | 0.004129 | 0.003593 | 0.003440 |
| global_resource_test.test | RUN | 0.117188 | 0.393988 | 0.366819 | 0.001401 | 0.001507 | 0.001386 |
| flat_tree_test.test | RUN | 5.131092 | 31.426897 | 20.363601 | 0.001298 | 0.001409 | 0.001242 |
| flat_set_test.test | RUN | 6.403999 | 39.329111 | 26.055133 | 0.008790 | 0.007401 | 0.007674 |
| flat_set_adaptor_test.test | RUN | 4.800619 | 34.747106 | 20.468334 | 0.008563 | 0.008020 | 0.008122 |
| flat_map_test.test | RUN | 10.479942 | 59.006824 | 42.690262 | 0.009982 | 0.009093 | 0.008559 |
| flat_map_adaptor_test.test | RUN | 6.731602 | 43.013920 | 30.339492 | 0.009012 | 0.008469 | 0.008363 |
| explicit_inst_vector_test.test | RUN | 0.942576 | 5.815292 | 4.451069 | 0.001310 | 0.001378 | 0.001264 |
| explicit_inst_string_test.test | RUN | 1.263970 | 10.289997 | 6.193547 | 0.001326 | 0.001346 | 0.001233 |
| explicit_inst_static_vector_test.test | RUN | 0.113812 | 0.371912 | 0.406000 | 0.001329 | 0.001412 | 0.001171 |
| explicit_inst_stable_vector_test.test | RUN | 0.769362 | 3.683489 | 2.805830 | 0.001359 | 0.001450 | 0.001170 |
| explicit_inst_small_vector_test.test | RUN | 0.627976 | 3.338953 | 2.630825 | 0.001325 | 0.001365 | 0.001237 |
| explicit_inst_slist_test.test | RUN | 0.484346 | 3.030591 | 1.993473 | 0.001448 | 0.001438 | 0.001164 |
| explicit_inst_set_test.test | RUN | 0.702828 | 3.502279 | 2.342563 | 0.001427 | 0.001337 | 0.001269 |
| explicit_inst_map_test.test | RUN | 0.735202 | 3.681834 | 2.583203 | 0.001293 | 0.001407 | 0.001257 |
| explicit_inst_list_test.test | RUN | 0.157282 | 0.828993 | 0.654145 | 0.001360 | 0.001365 | 0.001169 |
| explicit_inst_flat_set_test.test | RUN | 4.195815 | 26.437403 | 15.565167 | 0.001370 | 0.001342 | 0.001360 |
| explicit_inst_flat_map_test.test | RUN | 4.522847 | 27.717207 | 17.321981 | 0.001342 | 0.001398 | 0.001205 |
| explicit_inst_devector_test.test | RUN | 0.517013 | 3.310098 | 2.209511 | 0.001250 | 0.001349 | 0.001311 |
| explicit_inst_deque_test.test | RUN | 0.549352 | 3.546400 | 2.331902 | 0.001316 | 0.001425 | 0.001266 |
| devector_test.test | RUN | 7.880206 | 38.231324 | 27.833220 | 0.003532 | 0.003409 | 0.004933 |
| devector_options_test.test | RUN | 0.310267 | 1.933532 | 1.188537 | 0.001668 | 0.001591 | 0.001404 |
| deque_test.test | RUN | 5.545353 | 26.819413 | 19.506416 | 0.481992 | 0.348816 | 0.433679 |
| deque_options_test.test | RUN | 0.244734 | 0.705267 | 0.842540 | 0.001474 | 0.001436 | 0.001370 |
| copy_move_algo_test.test | RUN | 0.226985 | 0.823643 | 0.752789 | 0.001732 | 0.001530 | 0.001460 |
| common_iterator_test.test | RUN | 0.015493 | 0.041628 | 0.041017 | 0.000526 | 0.000570 | 0.001281 |
| boost_iterator_comp_test.test | RUN | 0.322632 | 1.009252 | 1.140255 | 0.001339 | 0.001303 | 0.001223 |
| allocator_traits_test.test | RUN | 0.183302 | 0.434472 | 0.536124 | 0.001311 | 0.001350 | 0.001236 |
| alloc_full_test.test | RUN | 0.219526 | 0.941850 | 0.793445 | 0.004550 | 0.004422 | 0.004350 |
| alloc_basic_test.test | RUN | 0.178234 | 0.651863 | 0.586452 | 0.001506 | 0.001574 | 0.001372 |
| adaptive_pool_test.test | RUN | 2.283874 | 15.546080 | 11.123858 | 0.009605 | 0.013181 | 0.014150 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.885× | 0.876×, 0.894× | 0.993× |
| clang | 0.974× | 0.968×, 0.980× | 0.987× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.010324s | 0.009576s | 0.010505s | byte equality observed |
| alloc_basic_test.test | 10 | 0.000910s | 0.000921s | 0.000913s | byte equality observed |
| alloc_full_test.test | 7 | 0.002906s | 0.003014s | 0.003068s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000873s | 0.000838s | 0.000846s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000909s | 0.000886s | 0.000910s | byte equality observed |
| common_iterator_test.test | 10 | 0.000402s | 0.000410s | 0.000893s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001317s | 0.001021s | 0.001126s | byte equality observed |
| deque_options_test.test | 10 | 0.000940s | 0.000930s | 0.000902s | byte equality observed |
| deque_test.test | 1 | 0.504140s | 0.373154s | 0.455952s | byte equality observed |
| devector_options_test.test | 10 | 0.000999s | 0.000965s | 0.000940s | byte equality observed |
| devector_test.test | 4 | 0.003281s | 0.003160s | 0.004685s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000820s | 0.000806s | 0.000805s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000816s | 0.000793s | 0.000819s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000823s | 0.000820s | 0.000795s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000826s | 0.000823s | 0.000795s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000810s | 0.000807s | 0.000796s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000814s | 0.000821s | 0.000784s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000820s | 0.000828s | 0.000799s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000819s | 0.000840s | 0.000803s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000818s | 0.000820s | 0.000824s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000816s | 0.000827s | 0.000806s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000805s | 0.000802s | 0.000825s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000834s | 0.000820s | 0.000809s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000822s | 0.000828s | 0.000811s | byte equality observed |
| flat_map_adaptor_test.test | 2 | 0.006490s | 0.006140s | 0.006059s | byte equality observed |
| flat_map_test.test | 2 | 0.007119s | 0.006576s | 0.006237s | byte equality observed |
| flat_set_adaptor_test.test | 2 | 0.006284s | 0.005789s | 0.006005s | byte equality observed |
| flat_set_test.test | 3 | 0.006303s | 0.005478s | 0.005836s | byte equality observed |
| flat_tree_test.test | 10 | 0.000832s | 0.000832s | 0.000841s | byte equality observed |
| global_resource_test.test | 10 | 0.000892s | 0.000862s | 0.000859s | byte equality observed |
| hub_api_test.test | 7 | 0.002780s | 0.002342s | 0.002319s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000840s | 0.000826s | 0.000839s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.057582s | 0.058894s | 0.058326s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000901s | 0.000889s | 0.000897s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000879s | 0.000887s | 0.000869s | byte equality observed |
| hub_sort_test.test | 1 | 0.502521s | 0.494232s | 0.481332s | byte equality observed |
| hub_stability_test.test | 10 | 0.001141s | 0.001071s | 0.001160s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000854s | 0.000842s | 0.000849s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000873s | 0.000869s | 0.000866s | byte equality observed |
| is_constructible_test.test | 10 | 0.000378s | 0.000376s | 0.000844s | byte equality observed |
| list_test.test | 2 | 0.017134s | 0.017186s | 0.017712s | byte equality observed |
| map_test.test | 3 | 0.008656s | 0.008313s | 0.008196s | byte equality observed |
| memory_resource_test.test | 10 | 0.000860s | 0.000856s | 0.000858s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000880s | 0.000864s | 0.000891s | byte equality observed |
| nest_api_test.test | 10 | 0.001739s | 0.001744s | 0.001730s | byte equality observed |
| nest_test.test | 10 | 0.000890s | 0.000885s | 0.000892s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001284s | 0.001254s | 0.001276s | byte equality observed |
| node_allocator_test.test | 3 | 0.007434s | 0.007302s | 0.007737s | byte equality observed |
| node_handle_test.test | 10 | 0.000827s | 0.000844s | 0.000851s | byte equality observed |
| null_iterators_test.test | 10 | 0.000833s | 0.000820s | 0.000852s | byte equality observed |
| pair_test.test | 10 | 0.000832s | 0.000813s | 0.000850s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000824s | 0.000800s | 0.000824s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000827s | 0.000788s | 0.000843s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000828s | 0.000793s | 0.000834s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000829s | 0.000779s | 0.000833s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000822s | 0.000795s | 0.000836s | byte equality observed |
| pmr_list_test.test | 10 | 0.000828s | 0.000793s | 0.000821s | byte equality observed |
| pmr_map_test.test | 10 | 0.000828s | 0.000784s | 0.000823s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000829s | 0.000795s | 0.000852s | byte equality observed |
| pmr_set_test.test | 10 | 0.000829s | 0.000797s | 0.000846s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000827s | 0.000788s | 0.000848s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000831s | 0.000788s | 0.000836s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000888s | 0.000800s | 0.000837s | byte equality observed |
| pmr_string_test.test | 10 | 0.000400s | 0.000352s | 0.000851s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000404s | 0.000351s | 0.000894s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000893s | 0.000824s | 0.000905s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000922s | 0.000829s | 0.000890s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000913s | 0.000818s | 0.000893s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000880s | 0.000825s | 0.000920s | byte equality observed |
| segtor_options_test.test | 10 | 0.000909s | 0.000880s | 0.000916s | byte equality observed |
| segtor_test.test | 1 | 0.370198s | 0.331963s | 0.383792s | byte equality observed |
| set_test.test | 2 | 0.012158s | 0.010622s | 0.011696s | byte equality observed |
| slist_test.test | 1 | 0.017911s | 0.017575s | 0.017633s | byte equality observed |
| small_vector_options_test.test | 10 | 0.000933s | 0.000912s | 0.000926s | byte equality observed |
| small_vector_test.test | 5 | 0.001994s | 0.001774s | 0.003820s | byte equality observed |
| stable_vector_test.test | 1 | 0.030336s | 0.030465s | 0.031845s | byte equality observed |
| static_vector_options_test.test | 10 | 0.000917s | 0.000866s | 0.000912s | byte equality observed |
| static_vector_test.test | 10 | 0.001195s | 0.001099s | 0.001159s | byte equality observed |
| string_options_test.test | 10 | 0.000877s | 0.000877s | 0.000888s | byte equality observed |
| string_test.test | 2 | 0.011421s | 0.010412s | 0.011123s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000858s | 0.000850s | 0.000851s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000905s | 0.000884s | 0.000883s | byte equality observed |
| throw_exception_test.test | 10 | 0.000823s | 0.000834s | 0.000850s | byte equality observed |
| tree_test.test | 10 | 0.000828s | 0.000820s | 0.000809s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000892s | 0.000865s | 0.000863s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000841s | 0.000819s | 0.000818s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000376s | 0.000380s | 0.000797s | byte equality observed |
| vector_options_test.test | 10 | 0.000974s | 0.000980s | 0.000947s | byte equality observed |
| vector_test.test | 4 | 0.003453s | 0.003376s | 0.005199s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
