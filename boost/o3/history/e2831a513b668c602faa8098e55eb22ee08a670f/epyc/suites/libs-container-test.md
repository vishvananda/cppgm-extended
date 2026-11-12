# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 8.849187 | 42.590707 | 38.447389 | 0.007295 | 0.006596 | 0.009906 |
| vector_options_test.test | RUN | 0.439876 | 1.964395 | 1.831946 | 0.002426 | 0.002438 | 0.002288 |
| uses_allocator_test.test | RUN | 0.082319 | 0.171374 | 0.243282 | 0.000779 | 0.000879 | 0.002022 |
| uses_allocator_construction_test.test | RUN | 0.247047 | 0.655017 | 0.777878 | 0.001940 | 0.001987 | 0.002035 |
| unsynchronized_pool_resource_test.test | RUN | 0.308252 | 1.827277 | 1.000371 | 0.001776 | 0.002446 | 0.002254 |
| tree_test.test | RUN | 1.279205 | 6.348374 | 5.715754 | 0.001788 | 0.002002 | 0.002114 |
| throw_exception_test.test | RUN | 0.163086 | 0.375083 | 0.538722 | 0.001945 | 0.002149 | 0.001998 |
| synchronized_pool_resource_test.test | RUN | 0.287091 | 1.871714 | 1.119686 | 0.002254 | 0.002308 | 0.002365 |
| string_view_compat_test.test | RUN | 0.503933 | 3.698315 | 2.139447 | 0.001819 | 0.002342 | 0.002052 |
| string_test.test | RUN | 2.101479 | 13.682649 | 10.306921 | 0.023393 | 0.021754 | 0.022808 |
| string_options_test.test | RUN | 0.383291 | 2.988903 | 1.776530 | 0.002102 | 0.002202 | 0.001831 |
| static_vector_test.test | RUN | 2.250441 | 11.258278 | 10.450938 | 0.002736 | 0.002842 | 0.003037 |
| static_vector_options_test.test | RUN | 0.287937 | 0.787987 | 1.070957 | 0.002306 | 0.002227 | 0.002731 |
| stable_vector_test.test | RUN | 4.923454 | 22.877812 | 20.076636 | 0.036902 | 0.037093 | 0.039287 |
| small_vector_test.test | RUN | 3.442070 | 16.276440 | 14.304258 | 0.004404 | 0.003909 | 0.007063 |
| small_vector_options_test.test | RUN | 0.483104 | 2.002997 | 2.044964 | 0.002224 | 0.002494 | 0.002300 |
| slist_test.test | RUN | 3.041684 | 14.596479 | 12.921326 | 0.033976 | 0.033468 | 0.034541 |
| set_test.test | RUN | 4.645437 | 27.241327 | 22.716348 | 0.021217 | 0.017964 | 0.020588 |
| segtor_test.test | RUN | 6.244036 | 29.519386 | 25.825224 | 0.977642 | 0.937965 | 0.920178 |
| segtor_options_test.test | RUN | 0.349900 | 0.924596 | 1.337889 | 0.002364 | 0.002148 | 0.002229 |
| scoped_allocator_usage_test.test | RUN | 1.120017 | 4.593402 | 4.637903 | 0.001635 | 0.002386 | 0.002164 |
| scoped_allocator_adaptor_test.test | RUN | 0.528420 | 1.352671 | 2.172894 | 0.002281 | 0.002150 | 0.002102 |
| resource_adaptor_test.test | RUN | 0.252020 | 0.771358 | 0.946652 | 0.001982 | 0.002097 | 0.002116 |
| polymorphic_allocator_test.test | RUN | 0.225152 | 0.625423 | 0.794663 | 0.002403 | 0.003013 | 0.002148 |
| pmr_vector_test.test | RUN | 0.121475 | 0.252910 | 0.397718 | 0.000799 | 0.000839 | 0.002092 |
| pmr_string_test.test | RUN | 0.239566 | 0.519193 | 0.811343 | 0.000814 | 0.001033 | 0.001856 |
| pmr_stable_vector_test.test | RUN | 0.278123 | 0.821307 | 1.039170 | 0.001886 | 0.001975 | 0.001870 |
| pmr_small_vector_test.test | RUN | 0.150348 | 0.344331 | 0.540349 | 0.002102 | 0.002056 | 0.002066 |
| pmr_slist_test.test | RUN | 0.196505 | 1.000277 | 0.921228 | 0.001932 | 0.002145 | 0.002068 |
| pmr_set_test.test | RUN | 0.238795 | 0.569020 | 0.853018 | 0.002181 | 0.002161 | 0.002011 |
| pmr_nest_test.test | RUN | 0.182927 | 0.406369 | 0.577184 | 0.002132 | 0.002064 | 0.002308 |
| pmr_map_test.test | RUN | 0.254029 | 0.606440 | 0.862689 | 0.002112 | 0.001984 | 0.002058 |
| pmr_list_test.test | RUN | 0.163621 | 0.367033 | 0.593280 | 0.001918 | 0.001944 | 0.002026 |
| pmr_hub_test.test | RUN | 0.223880 | 0.538803 | 0.655619 | 0.001965 | 0.001905 | 0.002047 |
| pmr_flat_set_test.test | RUN | 0.180995 | 0.407185 | 0.651141 | 0.001963 | 0.002072 | 0.002024 |
| pmr_flat_map_test.test | RUN | 0.245082 | 0.464881 | 0.631187 | 0.002065 | 0.001816 | 0.001928 |
| pmr_devector_test.test | RUN | 0.143024 | 0.382407 | 0.560455 | 0.001773 | 0.002228 | 0.002202 |
| pmr_deque_test.test | RUN | 0.139261 | 0.323425 | 0.474432 | 0.002037 | 0.002380 | 0.001890 |
| pair_test.test | RUN | 0.356678 | 0.771658 | 1.176582 | 0.002066 | 0.002058 | 0.002245 |
| null_iterators_test.test | RUN | 0.564565 | 1.525615 | 1.950901 | 0.001818 | 0.002106 | 0.001998 |
| node_handle_test.test | RUN | 0.249119 | 1.812219 | 0.793445 | 0.002037 | 0.002224 | 0.001793 |
| node_allocator_test.test | RUN | 3.660790 | 17.989812 | 15.815678 | 0.013139 | 0.013290 | 0.013111 |
| new_delete_resource_test.test | RUN | 0.295397 | 1.147069 | 1.053986 | 0.003087 | 0.003078 | 0.002856 |
| nest_test.test | RUN | 0.691756 | 4.938847 | 3.630250 | 0.002022 | 0.002366 | 0.002084 |
| nest_api_test.test | RUN | 1.281443 | 9.500878 | 6.911904 | 0.003598 | 0.003280 | 0.003253 |
| monotonic_buffer_resource_test.test | RUN | 0.249227 | 1.991721 | 1.104041 | 0.002166 | 0.002470 | 0.002128 |
| memory_resource_test.test | RUN | 0.179393 | 0.573463 | 0.677751 | 0.002105 | 0.002076 | 0.002098 |
| map_test.test | RUN | 6.499301 | 30.901542 | 33.417183 | 0.014879 | 0.015046 | 0.014091 |
| list_test.test | RUN | 3.348414 | 15.629103 | 14.066423 | 0.033647 | 0.029392 | 0.035493 |
| is_constructible_test.test | RUN | 0.108743 | 0.217674 | 0.397826 | 0.000957 | 0.000797 | 0.001990 |
| insert_vs_emplace_test.test | RUN | 0.354781 | 1.365199 | 2.082595 | 0.002175 | 0.002230 | 0.002140 |
| hub_uses_allocator_test.test | RUN | 0.376200 | 1.130331 | 1.476618 | 0.002106 | 0.002317 | 0.001923 |
| hub_stability_test.test | RUN | 0.778745 | 4.956075 | 4.773927 | 0.002374 | 0.003289 | 0.002323 |
| hub_sort_test.test | RUN | 0.506559 | 2.353222 | 2.272141 | 0.675066 | 0.644219 | 0.612428 |
| hub_pocxx_test.test | RUN | 0.449205 | 2.778566 | 1.779990 | 0.002469 | 0.002104 | 0.001900 |
| hub_new_extended_alignment_test.test | RUN | 0.276966 | 0.863283 | 0.957901 | 0.002118 | 0.003042 | 0.002052 |
| hub_exception_safety_test.test | RUN | 0.977556 | 6.267291 | 4.517354 | 0.104002 | 0.145035 | 0.112006 |
| hub_empty_sort_test.test | RUN | 0.318312 | 0.767521 | 1.126478 | 0.002290 | 0.002110 | 0.002104 |
| hub_api_test.test | RUN | 3.062758 | 25.729264 | 14.713423 | 0.005397 | 0.004612 | 0.004209 |
| global_resource_test.test | RUN | 0.163638 | 0.541866 | 0.644644 | 0.001994 | 0.002422 | 0.002210 |
| flat_tree_test.test | RUN | 5.538251 | 38.603318 | 27.597670 | 0.001970 | 0.001853 | 0.001861 |
| flat_set_test.test | RUN | 7.111216 | 48.991415 | 36.640905 | 0.011155 | 0.009885 | 0.010947 |
| flat_set_adaptor_test.test | RUN | 5.303377 | 43.550677 | 28.686141 | 0.011532 | 0.009820 | 0.010808 |
| flat_map_test.test | RUN | 11.614399 | 72.925087 | 59.690190 | 0.012094 | 0.011958 | 0.011562 |
| flat_map_adaptor_test.test | RUN | 7.356876 | 53.744207 | 43.049991 | 0.011055 | 0.011289 | 0.010943 |
| explicit_inst_vector_test.test | RUN | 1.137960 | 7.398972 | 6.317894 | 0.002462 | 0.002086 | 0.001582 |
| explicit_inst_string_test.test | RUN | 1.470238 | 13.082639 | 8.448153 | 0.001992 | 0.002299 | 0.001856 |
| explicit_inst_static_vector_test.test | RUN | 0.156388 | 0.493882 | 0.669175 | 0.001835 | 0.002695 | 0.001959 |
| explicit_inst_stable_vector_test.test | RUN | 0.921992 | 4.741502 | 3.927157 | 0.002018 | 0.002262 | 0.002092 |
| explicit_inst_small_vector_test.test | RUN | 0.781523 | 4.239186 | 3.801848 | 0.001813 | 0.002630 | 0.001984 |
| explicit_inst_slist_test.test | RUN | 0.566138 | 3.854919 | 2.886908 | 0.002098 | 0.002092 | 0.002014 |
| explicit_inst_set_test.test | RUN | 0.818548 | 4.473397 | 3.589518 | 0.002048 | 0.002080 | 0.001999 |
| explicit_inst_map_test.test | RUN | 0.927780 | 4.588757 | 3.890488 | 0.002020 | 0.002121 | 0.001864 |
| explicit_inst_list_test.test | RUN | 0.207821 | 1.054521 | 0.980358 | 0.001916 | 0.002064 | 0.001934 |
| explicit_inst_flat_set_test.test | RUN | 4.684002 | 33.312440 | 21.255606 | 0.002263 | 0.002106 | 0.002010 |
| explicit_inst_flat_map_test.test | RUN | 5.078243 | 34.586707 | 24.561015 | 0.002112 | 0.002308 | 0.002137 |
| explicit_inst_devector_test.test | RUN | 0.632527 | 4.254292 | 3.196553 | 0.002328 | 0.002101 | 0.002155 |
| explicit_inst_deque_test.test | RUN | 0.681370 | 4.544703 | 3.416690 | 0.002261 | 0.001948 | 0.001931 |
| devector_test.test | RUN | 8.941833 | 49.513286 | 40.157916 | 0.004582 | 0.004126 | 0.005123 |
| devector_options_test.test | RUN | 0.400394 | 2.581219 | 1.819278 | 0.002379 | 0.002467 | 0.002362 |
| deque_test.test | RUN | 5.583413 | 34.198955 | 24.599007 | 1.110547 | 0.430574 | 0.538657 |
| deque_options_test.test | RUN | 0.323935 | 0.945776 | 1.434020 | 0.002035 | 0.002322 | 0.002230 |
| copy_move_algo_test.test | RUN | 0.308068 | 1.193689 | 1.356991 | 0.002289 | 0.002292 | 0.002591 |
| common_iterator_test.test | RUN | 0.023984 | 0.063542 | 0.066068 | 0.000749 | 0.000880 | 0.001783 |
| boost_iterator_comp_test.test | RUN | 0.444197 | 1.425801 | 2.002829 | 0.002184 | 0.002404 | 0.002084 |
| allocator_traits_test.test | RUN | 0.263318 | 0.576871 | 0.896124 | 0.001884 | 0.002387 | 0.002060 |
| alloc_full_test.test | RUN | 0.294166 | 1.286249 | 1.296417 | 0.005989 | 0.006294 | 0.006601 |
| alloc_basic_test.test | RUN | 0.239916 | 0.935692 | 0.929030 | 0.002087 | 0.002392 | 0.002246 |
| adaptive_pool_test.test | RUN | 3.365931 | 19.553489 | 16.221061 | 0.012410 | 0.018581 | 0.019768 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.921× | 0.908×, 0.934× | 0.977× |
| clang | 0.980× | 0.977×, 0.983× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.013429s | 0.012572s | 0.013673s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001290s | 0.001304s | 0.001319s | byte equality observed |
| alloc_full_test.test | 6 | 0.003847s | 0.003850s | 0.003907s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001228s | 0.001240s | 0.001210s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001290s | 0.001304s | 0.001241s | byte equality observed |
| common_iterator_test.test | 10 | 0.000619s | 0.000624s | 0.001276s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001491s | 0.001421s | 0.001398s | byte equality observed |
| deque_options_test.test | 10 | 0.001381s | 0.001316s | 0.001381s | byte equality observed |
| deque_test.test | 1 | 0.563435s | 0.474241s | 0.590225s | byte equality observed |
| devector_options_test.test | 10 | 0.001500s | 0.001345s | 0.001332s | byte equality observed |
| devector_test.test | 4 | 0.003726s | 0.003697s | 0.004893s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.001195s | 0.001199s | 0.001140s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.001170s | 0.001185s | 0.001138s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.001186s | 0.001210s | 0.001158s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.001182s | 0.001198s | 0.001194s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.001173s | 0.001170s | 0.001155s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.001191s | 0.001182s | 0.001151s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.001189s | 0.001181s | 0.001151s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.001177s | 0.001188s | 0.001144s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.001187s | 0.001201s | 0.001156s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.001177s | 0.001195s | 0.001163s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.001154s | 0.001174s | 0.001142s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.001182s | 0.001197s | 0.001175s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.001174s | 0.001192s | 0.001149s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.007240s | 0.006874s | 0.006916s | byte equality observed |
| flat_map_test.test | 3 | 0.007797s | 0.007419s | 0.007154s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.007305s | 0.006623s | 0.007130s | byte equality observed |
| flat_set_test.test | 3 | 0.007354s | 0.006378s | 0.007004s | byte equality observed |
| flat_tree_test.test | 10 | 0.001184s | 0.001226s | 0.001179s | byte equality observed |
| global_resource_test.test | 10 | 0.001208s | 0.001270s | 0.001223s | byte equality observed |
| hub_api_test.test | 6 | 0.003380s | 0.002801s | 0.002717s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001179s | 0.001213s | 0.001172s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.081614s | 0.085378s | 0.075413s | byte equality observed |
| hub_new_extended_alignment_test.test | 9 | 0.001287s | 0.001361s | 0.001245s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001203s | 0.001309s | 0.001226s | byte equality observed |
| hub_sort_test.test | 1 | 0.530582s | 0.522354s | 0.498673s | byte equality observed |
| hub_stability_test.test | 9 | 0.001504s | 0.001485s | 0.001490s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.001205s | 0.001229s | 0.001184s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001235s | 0.001258s | 0.001219s | byte equality observed |
| is_constructible_test.test | 10 | 0.000581s | 0.000581s | 0.001171s | byte equality observed |
| list_test.test | 1 | 0.021604s | 0.021388s | 0.021436s | byte equality observed |
| map_test.test | 2 | 0.009575s | 0.009749s | 0.009328s | byte equality observed |
| memory_resource_test.test | 10 | 0.001195s | 0.001297s | 0.001210s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001218s | 0.001252s | 0.001189s | byte equality observed |
| nest_api_test.test | 8 | 0.002006s | 0.002039s | 0.002030s | byte equality observed |
| nest_test.test | 10 | 0.001241s | 0.001265s | 0.001228s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001760s | 0.001808s | 0.001772s | byte equality observed |
| node_allocator_test.test | 3 | 0.009026s | 0.008934s | 0.008787s | byte equality observed |
| node_handle_test.test | 10 | 0.001212s | 0.001236s | 0.001205s | byte equality observed |
| null_iterators_test.test | 9 | 0.001218s | 0.001207s | 0.001191s | byte equality observed |
| pair_test.test | 10 | 0.001217s | 0.001226s | 0.001205s | byte equality observed |
| pmr_deque_test.test | 10 | 0.001199s | 0.001219s | 0.001180s | byte equality observed |
| pmr_devector_test.test | 10 | 0.001206s | 0.001211s | 0.001174s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.001205s | 0.001187s | 0.001181s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.001194s | 0.001196s | 0.001170s | byte equality observed |
| pmr_hub_test.test | 10 | 0.001182s | 0.001214s | 0.001176s | byte equality observed |
| pmr_list_test.test | 10 | 0.001183s | 0.001246s | 0.001163s | byte equality observed |
| pmr_map_test.test | 10 | 0.001198s | 0.001220s | 0.001175s | byte equality observed |
| pmr_nest_test.test | 10 | 0.001188s | 0.001201s | 0.001179s | byte equality observed |
| pmr_set_test.test | 10 | 0.001193s | 0.001236s | 0.001201s | byte equality observed |
| pmr_slist_test.test | 10 | 0.001193s | 0.001228s | 0.001214s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001191s | 0.001216s | 0.001250s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001212s | 0.001236s | 0.001287s | byte equality observed |
| pmr_string_test.test | 10 | 0.000591s | 0.000592s | 0.001249s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000589s | 0.000583s | 0.001237s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001275s | 0.001278s | 0.001253s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001274s | 0.001277s | 0.001315s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001289s | 0.001297s | 0.001269s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001343s | 0.001299s | 0.001285s | byte equality observed |
| segtor_options_test.test | 10 | 0.001289s | 0.001304s | 0.001269s | byte equality observed |
| segtor_test.test | 1 | 0.485900s | 0.446215s | 0.448764s | byte equality observed |
| set_test.test | 2 | 0.013560s | 0.011881s | 0.013249s | byte equality observed |
| slist_test.test | 1 | 0.020445s | 0.020380s | 0.020304s | byte equality observed |
| small_vector_options_test.test | 8 | 0.001334s | 0.001311s | 0.001283s | byte equality observed |
| small_vector_test.test | 5 | 0.002506s | 0.002270s | 0.004159s | byte equality observed |
| stable_vector_test.test | 1 | 0.029736s | 0.029761s | 0.031063s | byte equality observed |
| static_vector_options_test.test | 8 | 0.001285s | 0.001260s | 0.001284s | byte equality observed |
| static_vector_test.test | 9 | 0.001619s | 0.001507s | 0.001574s | byte equality observed |
| string_options_test.test | 10 | 0.001266s | 0.001252s | 0.001249s | byte equality observed |
| string_test.test | 2 | 0.015011s | 0.015588s | 0.015810s | byte equality observed |
| string_view_compat_test.test | 9 | 0.001190s | 0.001204s | 0.001199s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001281s | 0.001242s | 0.001264s | byte equality observed |
| throw_exception_test.test | 10 | 0.001185s | 0.001167s | 0.001198s | byte equality observed |
| tree_test.test | 10 | 0.001197s | 0.001182s | 0.001182s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001260s | 0.001250s | 0.001252s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.001183s | 0.001193s | 0.001170s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000572s | 0.000566s | 0.001147s | byte equality observed |
| vector_options_test.test | 10 | 0.001378s | 0.001352s | 0.001334s | byte equality observed |
| vector_test.test | 4 | 0.004054s | 0.004008s | 0.005589s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
