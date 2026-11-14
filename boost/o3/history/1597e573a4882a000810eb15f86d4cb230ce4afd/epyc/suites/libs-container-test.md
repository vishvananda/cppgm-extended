# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.840600 | 24.730411 | 20.388539 | 0.004721 | 0.004348 | 0.005295 |
| vector_options_test.test | RUN | 0.261040 | 1.108242 | 0.829806 | 0.002030 | 0.001620 | 0.001678 |
| uses_allocator_test.test | RUN | 0.047358 | 0.093194 | 0.104350 | 0.000632 | 0.000650 | 0.001423 |
| uses_allocator_construction_test.test | RUN | 0.142539 | 0.334251 | 0.344492 | 0.001562 | 0.001456 | 0.001492 |
| unsynchronized_pool_resource_test.test | RUN | 0.179458 | 0.967171 | 0.467262 | 0.001722 | 0.001710 | 0.001544 |
| tree_test.test | RUN | 0.794408 | 3.777372 | 2.749722 | 0.001636 | 0.001702 | 0.001382 |
| throw_exception_test.test | RUN | 0.113597 | 0.213576 | 0.256379 | 0.001550 | 0.001349 | 0.001488 |
| synchronized_pool_resource_test.test | RUN | 0.189453 | 0.983044 | 0.496184 | 0.001762 | 0.001738 | 0.001564 |
| string_view_compat_test.test | RUN | 0.309697 | 2.032523 | 0.936533 | 0.001642 | 0.001687 | 0.001542 |
| string_test.test | RUN | 1.336388 | 7.892693 | 5.188756 | 0.013508 | 0.011512 | 0.011492 |
| string_options_test.test | RUN | 0.241373 | 1.589031 | 0.841729 | 0.001894 | 0.001738 | 0.001374 |
| static_vector_test.test | RUN | 1.456142 | 6.616142 | 5.359660 | 0.002509 | 0.001960 | 0.002056 |
| static_vector_options_test.test | RUN | 0.186329 | 0.406175 | 0.441757 | 0.001936 | 0.001660 | 0.001613 |
| stable_vector_test.test | RUN | 3.290801 | 13.728742 | 10.500510 | 0.025785 | 0.024129 | 0.024024 |
| small_vector_test.test | RUN | 2.235597 | 9.466651 | 7.562932 | 0.003180 | 0.002643 | 0.003768 |
| small_vector_options_test.test | RUN | 0.333980 | 1.125891 | 0.974142 | 0.001980 | 0.001791 | 0.001612 |
| slist_test.test | RUN | 2.054874 | 8.551953 | 6.596500 | 0.019990 | 0.018106 | 0.018306 |
| set_test.test | RUN | 3.171476 | 15.913545 | 11.955653 | 0.013910 | 0.010874 | 0.011721 |
| segtor_test.test | RUN | 3.952946 | 17.234640 | 13.520101 | 0.510919 | 0.421680 | 0.395405 |
| segtor_options_test.test | RUN | 0.198204 | 0.478806 | 0.584613 | 0.001840 | 0.001820 | 0.001636 |
| scoped_allocator_usage_test.test | RUN | 0.685633 | 2.553375 | 2.190800 | 0.001801 | 0.001658 | 0.001354 |
| scoped_allocator_adaptor_test.test | RUN | 0.303647 | 0.728883 | 0.914243 | 0.001742 | 0.001672 | 0.001284 |
| resource_adaptor_test.test | RUN | 0.166111 | 0.414907 | 0.406236 | 0.001591 | 0.001707 | 0.001531 |
| polymorphic_allocator_test.test | RUN | 0.136298 | 0.332757 | 0.342700 | 0.001447 | 0.001327 | 0.001361 |
| pmr_vector_test.test | RUN | 0.076815 | 0.134946 | 0.171657 | 0.000700 | 0.000697 | 0.001531 |
| pmr_string_test.test | RUN | 0.151660 | 0.262657 | 0.325827 | 0.000741 | 0.000711 | 0.001184 |
| pmr_stable_vector_test.test | RUN | 0.169883 | 0.437835 | 0.486427 | 0.001479 | 0.001645 | 0.001054 |
| pmr_small_vector_test.test | RUN | 0.091833 | 0.174306 | 0.240412 | 0.001710 | 0.001655 | 0.001385 |
| pmr_slist_test.test | RUN | 0.128047 | 0.588936 | 0.486375 | 0.001785 | 0.001665 | 0.001588 |
| pmr_set_test.test | RUN | 0.146294 | 0.310747 | 0.360207 | 0.001592 | 0.001482 | 0.001330 |
| pmr_nest_test.test | RUN | 0.117285 | 0.208405 | 0.242580 | 0.001368 | 0.001615 | 0.001539 |
| pmr_map_test.test | RUN | 0.158966 | 0.318490 | 0.358202 | 0.001740 | 0.001606 | 0.001467 |
| pmr_list_test.test | RUN | 0.102287 | 0.198893 | 0.249997 | 0.001627 | 0.001597 | 0.001502 |
| pmr_hub_test.test | RUN | 0.143300 | 0.270811 | 0.311023 | 0.001480 | 0.001490 | 0.001541 |
| pmr_flat_set_test.test | RUN | 0.118467 | 0.215078 | 0.274448 | 0.001756 | 0.001562 | 0.001517 |
| pmr_flat_map_test.test | RUN | 0.134926 | 0.247392 | 0.301205 | 0.001571 | 0.001558 | 0.001280 |
| pmr_devector_test.test | RUN | 0.084404 | 0.214613 | 0.225493 | 0.001582 | 0.001561 | 0.001541 |
| pmr_deque_test.test | RUN | 0.090463 | 0.174685 | 0.231351 | 0.001742 | 0.001626 | 0.001514 |
| pair_test.test | RUN | 0.233954 | 0.441609 | 0.512374 | 0.001766 | 0.001594 | 0.001604 |
| null_iterators_test.test | RUN | 0.346004 | 0.832502 | 0.846612 | 0.001616 | 0.001658 | 0.001457 |
| node_handle_test.test | RUN | 0.153302 | 0.977527 | 0.356816 | 0.001662 | 0.001682 | 0.001346 |
| node_allocator_test.test | RUN | 2.437098 | 10.613961 | 8.249474 | 0.009325 | 0.008006 | 0.008016 |
| new_delete_resource_test.test | RUN | 0.186739 | 0.608248 | 0.484518 | 0.002548 | 0.002236 | 0.002190 |
| nest_test.test | RUN | 0.456726 | 2.793281 | 1.752658 | 0.001870 | 0.001766 | 0.001620 |
| nest_api_test.test | RUN | 0.876776 | 5.351236 | 3.520935 | 0.002634 | 0.002396 | 0.002314 |
| monotonic_buffer_resource_test.test | RUN | 0.192890 | 1.077917 | 0.500596 | 0.001630 | 0.001572 | 0.001477 |
| memory_resource_test.test | RUN | 0.118473 | 0.314434 | 0.294543 | 0.001805 | 0.001498 | 0.001482 |
| map_test.test | RUN | 4.273431 | 18.785276 | 17.215378 | 0.010254 | 0.008998 | 0.008691 |
| list_test.test | RUN | 2.232155 | 9.740810 | 7.218515 | 0.020375 | 0.018874 | 0.018419 |
| is_constructible_test.test | RUN | 0.070756 | 0.117650 | 0.160406 | 0.000795 | 0.000737 | 0.001480 |
| insert_vs_emplace_test.test | RUN | 0.224926 | 0.747180 | 0.934370 | 0.001886 | 0.001604 | 0.001576 |
| hub_uses_allocator_test.test | RUN | 0.260563 | 0.626335 | 0.602906 | 0.001905 | 0.001574 | 0.001562 |
| hub_stability_test.test | RUN | 0.537856 | 2.928443 | 2.387247 | 0.002034 | 0.004760 | 0.001755 |
| hub_sort_test.test | RUN | 0.346598 | 1.336160 | 1.153868 | 0.494058 | 0.456303 | 0.424706 |
| hub_pocxx_test.test | RUN | 0.317234 | 1.867249 | 0.828337 | 0.001712 | 0.001774 | 0.001524 |
| hub_new_extended_alignment_test.test | RUN | 0.177746 | 0.722717 | 0.453872 | 0.001523 | 0.001797 | 0.001524 |
| hub_exception_safety_test.test | RUN | 0.606158 | 3.733684 | 2.423452 | 0.077774 | 0.071693 | 0.072558 |
| hub_empty_sort_test.test | RUN | 0.193291 | 0.440047 | 0.499674 | 0.001753 | 0.001627 | 0.001476 |
| hub_api_test.test | RUN | 2.131731 | 14.953920 | 7.748002 | 0.003698 | 0.003205 | 0.002977 |
| global_resource_test.test | RUN | 0.110419 | 0.299076 | 0.264390 | 0.001861 | 0.001727 | 0.001445 |
| flat_tree_test.test | RUN | 3.839875 | 23.127257 | 14.906384 | 0.001789 | 0.001602 | 0.001530 |
| flat_set_test.test | RUN | 4.785039 | 28.473137 | 19.387327 | 0.008067 | 0.006043 | 0.006358 |
| flat_set_adaptor_test.test | RUN | 3.565371 | 24.889824 | 14.871081 | 0.007601 | 0.006321 | 0.006465 |
| flat_map_test.test | RUN | 7.763356 | 42.703367 | 31.646949 | 0.008979 | 0.007450 | 0.006881 |
| flat_map_adaptor_test.test | RUN | 4.964123 | 30.735954 | 22.010294 | 0.007938 | 0.006739 | 0.006816 |
| explicit_inst_vector_test.test | RUN | 0.757725 | 4.244700 | 3.370634 | 0.001710 | 0.001652 | 0.001490 |
| explicit_inst_string_test.test | RUN | 1.003034 | 7.580827 | 4.763686 | 0.001732 | 0.001678 | 0.001582 |
| explicit_inst_static_vector_test.test | RUN | 0.101522 | 0.276455 | 0.324933 | 0.001417 | 0.001638 | 0.001565 |
| explicit_inst_stable_vector_test.test | RUN | 0.620111 | 2.740996 | 2.174671 | 0.001731 | 0.001614 | 0.001537 |
| explicit_inst_small_vector_test.test | RUN | 0.517156 | 2.471915 | 1.926357 | 0.001684 | 0.001630 | 0.001359 |
| explicit_inst_slist_test.test | RUN | 0.380788 | 2.266668 | 1.515543 | 0.001726 | 0.001677 | 0.001529 |
| explicit_inst_set_test.test | RUN | 0.552815 | 2.613299 | 1.726232 | 0.001770 | 0.001613 | 0.001461 |
| explicit_inst_map_test.test | RUN | 0.601946 | 2.746082 | 1.903263 | 0.001379 | 0.001656 | 0.001536 |
| explicit_inst_list_test.test | RUN | 0.137242 | 0.622435 | 0.477561 | 0.001650 | 0.001594 | 0.001538 |
| explicit_inst_flat_set_test.test | RUN | 3.240128 | 18.893150 | 11.563110 | 0.001780 | 0.001542 | 0.001376 |
| explicit_inst_flat_map_test.test | RUN | 3.501724 | 19.880990 | 12.960286 | 0.001558 | 0.001588 | 0.001373 |
| explicit_inst_devector_test.test | RUN | 0.401577 | 2.420708 | 1.567368 | 0.001755 | 0.001620 | 0.001547 |
| explicit_inst_deque_test.test | RUN | 0.439846 | 2.639804 | 1.772735 | 0.001767 | 0.001702 | 0.001482 |
| devector_test.test | RUN | 5.614462 | 32.078545 | 23.001431 | 0.003452 | 0.003058 | 0.003596 |
| devector_options_test.test | RUN | 0.276195 | 1.381384 | 0.873706 | 0.002082 | 0.001944 | 0.001724 |
| deque_test.test | RUN | 4.459318 | 16.332186 | 12.385517 | 0.447929 | 0.300184 | 0.352143 |
| deque_options_test.test | RUN | 0.206120 | 0.540282 | 0.578376 | 0.001555 | 0.001734 | 0.001417 |
| copy_move_algo_test.test | RUN | 0.191090 | 0.618985 | 0.504993 | 0.001970 | 0.001827 | 0.001582 |
| common_iterator_test.test | RUN | 0.015636 | 0.036119 | 0.035455 | 0.000742 | 0.000640 | 0.001547 |
| boost_iterator_comp_test.test | RUN | 0.283115 | 0.774983 | 0.858264 | 0.001660 | 0.001615 | 0.001477 |
| allocator_traits_test.test | RUN | 0.168194 | 0.325571 | 0.369445 | 0.001504 | 0.001656 | 0.001616 |
| alloc_full_test.test | RUN | 0.188516 | 0.681214 | 0.607986 | 0.004310 | 0.003865 | 0.003636 |
| alloc_basic_test.test | RUN | 0.154274 | 0.494861 | 0.432383 | 0.001746 | 0.001718 | 0.001617 |
| adaptive_pool_test.test | RUN | 2.382941 | 10.880486 | 8.279244 | 0.008075 | 0.007260 | 0.010401 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.970× | 0.973×, 0.967× | 1.020× |
| clang | 0.933× | 0.926×, 0.941× | 0.969× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.007248s | 0.006938s | 0.007501s | byte equality observed |
| alloc_basic_test.test | 10 | 0.000893s | 0.000922s | 0.000902s | byte equality observed |
| alloc_full_test.test | 8 | 0.002478s | 0.002489s | 0.002587s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000847s | 0.000840s | 0.000853s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000864s | 0.000865s | 0.000857s | byte equality observed |
| common_iterator_test.test | 10 | 0.000392s | 0.000399s | 0.000839s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001016s | 0.000964s | 0.000960s | byte equality observed |
| deque_options_test.test | 10 | 0.000910s | 0.000927s | 0.000898s | byte equality observed |
| deque_test.test | 1 | 0.348925s | 0.324847s | 0.318770s | byte equality observed |
| devector_options_test.test | 10 | 0.000969s | 0.000933s | 0.000941s | byte equality observed |
| devector_test.test | 6 | 0.002544s | 0.002591s | 0.003364s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000814s | 0.000798s | 0.000814s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000800s | 0.000790s | 0.000807s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000796s | 0.000807s | 0.000792s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000803s | 0.000802s | 0.000807s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000795s | 0.000783s | 0.000802s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000803s | 0.000806s | 0.000798s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000813s | 0.000799s | 0.000804s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000812s | 0.000804s | 0.000811s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000798s | 0.000807s | 0.000816s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000806s | 0.000805s | 0.000819s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000800s | 0.000789s | 0.000820s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000812s | 0.000804s | 0.000820s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000810s | 0.000800s | 0.000811s | byte equality observed |
| flat_map_adaptor_test.test | 5 | 0.004714s | 0.004371s | 0.004544s | byte equality observed |
| flat_map_test.test | 4 | 0.005069s | 0.004709s | 0.004611s | byte equality observed |
| flat_set_adaptor_test.test | 5 | 0.004442s | 0.004010s | 0.004302s | byte equality observed |
| flat_set_test.test | 4 | 0.004487s | 0.003944s | 0.004244s | byte equality observed |
| flat_tree_test.test | 10 | 0.000813s | 0.000809s | 0.000826s | byte equality observed |
| global_resource_test.test | 10 | 0.000852s | 0.000838s | 0.000855s | byte equality observed |
| hub_api_test.test | 9 | 0.002149s | 0.001851s | 0.001887s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000835s | 0.000821s | 0.000839s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.052167s | 0.052914s | 0.052615s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000917s | 0.000870s | 0.000828s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000889s | 0.000896s | 0.000862s | byte equality observed |
| hub_sort_test.test | 1 | 0.391436s | 0.396106s | 0.369711s | byte equality observed |
| hub_stability_test.test | 10 | 0.001029s | 0.001055s | 0.001065s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000837s | 0.000845s | 0.000848s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000864s | 0.000867s | 0.000875s | byte equality observed |
| is_constructible_test.test | 10 | 0.000388s | 0.000393s | 0.000816s | byte equality observed |
| list_test.test | 2 | 0.012125s | 0.013413s | 0.012259s | byte equality observed |
| map_test.test | 4 | 0.005965s | 0.005980s | 0.006047s | byte equality observed |
| memory_resource_test.test | 10 | 0.000845s | 0.000844s | 0.000834s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000847s | 0.000851s | 0.000842s | byte equality observed |
| nest_api_test.test | 10 | 0.001343s | 0.001359s | 0.001373s | byte equality observed |
| nest_test.test | 10 | 0.000872s | 0.000873s | 0.000894s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001225s | 0.001242s | 0.001259s | byte equality observed |
| node_allocator_test.test | 4 | 0.005480s | 0.005427s | 0.005491s | byte equality observed |
| node_handle_test.test | 10 | 0.000840s | 0.000830s | 0.000841s | byte equality observed |
| null_iterators_test.test | 10 | 0.000840s | 0.000828s | 0.000835s | byte equality observed |
| pair_test.test | 10 | 0.000824s | 0.000827s | 0.000823s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000816s | 0.000804s | 0.000815s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000806s | 0.000819s | 0.000822s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000823s | 0.000800s | 0.000810s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000826s | 0.000799s | 0.000811s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000823s | 0.000815s | 0.000816s | byte equality observed |
| pmr_list_test.test | 10 | 0.000822s | 0.000825s | 0.000824s | byte equality observed |
| pmr_map_test.test | 10 | 0.000798s | 0.000817s | 0.000832s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000815s | 0.000811s | 0.000822s | byte equality observed |
| pmr_set_test.test | 10 | 0.000822s | 0.000818s | 0.000834s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000814s | 0.000815s | 0.000827s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000816s | 0.000822s | 0.000833s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000839s | 0.000826s | 0.000865s | byte equality observed |
| pmr_string_test.test | 10 | 0.000393s | 0.000391s | 0.000859s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000391s | 0.000389s | 0.000852s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000880s | 0.000854s | 0.000850s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000871s | 0.000867s | 0.000865s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000865s | 0.000861s | 0.000859s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000874s | 0.000870s | 0.000856s | byte equality observed |
| segtor_options_test.test | 10 | 0.000904s | 0.000883s | 0.000896s | byte equality observed |
| segtor_test.test | 1 | 0.280769s | 0.259599s | 0.256155s | byte equality observed |
| set_test.test | 3 | 0.008161s | 0.007282s | 0.008070s | byte equality observed |
| slist_test.test | 2 | 0.012141s | 0.012166s | 0.012230s | byte equality observed |
| small_vector_options_test.test | 10 | 0.000918s | 0.000890s | 0.000894s | byte equality observed |
| small_vector_test.test | 8 | 0.001581s | 0.001517s | 0.002665s | byte equality observed |
| stable_vector_test.test | 1 | 0.019733s | 0.020195s | 0.020515s | byte equality observed |
| static_vector_options_test.test | 10 | 0.000893s | 0.000839s | 0.000896s | byte equality observed |
| static_vector_test.test | 10 | 0.001118s | 0.001029s | 0.001090s | byte equality observed |
| string_options_test.test | 10 | 0.000894s | 0.000862s | 0.000862s | byte equality observed |
| string_test.test | 3 | 0.007729s | 0.007991s | 0.007973s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000846s | 0.000821s | 0.000824s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000895s | 0.000860s | 0.000868s | byte equality observed |
| throw_exception_test.test | 10 | 0.000835s | 0.000798s | 0.000845s | byte equality observed |
| tree_test.test | 10 | 0.000828s | 0.000815s | 0.000809s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000883s | 0.000873s | 0.000868s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000851s | 0.000836s | 0.000815s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000390s | 0.000392s | 0.000808s | byte equality observed |
| vector_options_test.test | 10 | 0.000966s | 0.000960s | 0.000985s | byte equality observed |
| vector_test.test | 5 | 0.002676s | 0.002869s | 0.003748s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
