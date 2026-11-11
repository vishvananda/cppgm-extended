# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.815617 | 31.460880 | 27.467737 | 0.006435 | 0.005360 | 0.007620 |
| vector_options_test.test | RUN | 0.343862 | 1.408710 | 1.163268 | 0.002082 | 0.002243 | 0.002105 |
| uses_allocator_test.test | RUN | 0.068152 | 0.123285 | 0.165965 | 0.000814 | 0.000846 | 0.001860 |
| uses_allocator_construction_test.test | RUN | 0.197080 | 0.453837 | 0.576539 | 0.001824 | 0.001900 | 0.001800 |
| unsynchronized_pool_resource_test.test | RUN | 0.274571 | 1.284411 | 0.762069 | 0.002010 | 0.001942 | 0.001975 |
| tree_test.test | RUN | 0.971462 | 4.822924 | 3.873194 | 0.001968 | 0.001844 | 0.001750 |
| throw_exception_test.test | RUN | 0.140085 | 0.292510 | 0.397091 | 0.001862 | 0.002538 | 0.001854 |
| synchronized_pool_resource_test.test | RUN | 0.273836 | 1.340408 | 0.746486 | 0.001906 | 0.001942 | 0.001935 |
| string_view_compat_test.test | RUN | 0.440832 | 2.641561 | 1.389429 | 0.001867 | 0.001882 | 0.001903 |
| string_test.test | RUN | 1.793083 | 10.148960 | 7.324876 | 0.015642 | 0.015357 | 0.015357 |
| string_options_test.test | RUN | 0.296719 | 2.074594 | 1.205106 | 0.002012 | 0.002364 | 0.001590 |
| static_vector_test.test | RUN | 1.998445 | 8.420427 | 7.331599 | 0.002517 | 0.002368 | 0.002362 |
| static_vector_options_test.test | RUN | 0.241625 | 0.540585 | 0.698560 | 0.001927 | 0.001945 | 0.002021 |
| stable_vector_test.test | RUN | 4.156643 | 17.191291 | 14.170380 | 0.032879 | 0.032138 | 0.032784 |
| small_vector_test.test | RUN | 2.315300 | 12.010738 | 10.142807 | 0.004241 | 0.003064 | 0.005525 |
| small_vector_options_test.test | RUN | 0.370549 | 1.464209 | 1.366830 | 0.001712 | 0.002048 | 0.002042 |
| slist_test.test | RUN | 1.869338 | 10.766932 | 9.045927 | 0.025831 | 0.024893 | 0.024317 |
| set_test.test | RUN | 3.196914 | 19.977890 | 16.502835 | 0.015025 | 0.013908 | 0.015060 |
| segtor_test.test | RUN | 4.586252 | 21.657656 | 17.925958 | 0.661660 | 0.646427 | 0.649096 |
| segtor_options_test.test | RUN | 0.260511 | 0.662364 | 0.872503 | 0.001842 | 0.001797 | 0.001976 |
| scoped_allocator_usage_test.test | RUN | 0.811856 | 3.366150 | 3.118203 | 0.001825 | 0.001705 | 0.001465 |
| scoped_allocator_adaptor_test.test | RUN | 0.421924 | 0.886095 | 1.457555 | 0.001761 | 0.001992 | 0.001642 |
| resource_adaptor_test.test | RUN | 0.224000 | 0.574977 | 0.620097 | 0.001856 | 0.001685 | 0.001688 |
| polymorphic_allocator_test.test | RUN | 0.192944 | 0.454877 | 0.502161 | 0.001625 | 0.001784 | 0.001835 |
| pmr_vector_test.test | RUN | 0.101426 | 0.182071 | 0.282275 | 0.000574 | 0.000784 | 0.001459 |
| pmr_string_test.test | RUN | 0.200116 | 0.374353 | 0.531162 | 0.000718 | 0.000760 | 0.001793 |
| pmr_stable_vector_test.test | RUN | 0.211720 | 0.590704 | 0.713102 | 0.001628 | 0.001714 | 0.001785 |
| pmr_small_vector_test.test | RUN | 0.122399 | 0.238738 | 0.317530 | 0.001510 | 0.001910 | 0.001764 |
| pmr_slist_test.test | RUN | 0.160198 | 0.753056 | 0.692209 | 0.001871 | 0.001819 | 0.001605 |
| pmr_set_test.test | RUN | 0.189436 | 0.414303 | 0.551015 | 0.001586 | 0.001834 | 0.001868 |
| pmr_nest_test.test | RUN | 0.149057 | 0.273760 | 0.391793 | 0.001746 | 0.001792 | 0.001813 |
| pmr_map_test.test | RUN | 0.197353 | 0.403147 | 0.554213 | 0.001682 | 0.001736 | 0.001769 |
| pmr_list_test.test | RUN | 0.132551 | 0.247551 | 0.341490 | 0.001564 | 0.001534 | 0.001806 |
| pmr_hub_test.test | RUN | 0.191757 | 0.345134 | 0.448699 | 0.001627 | 0.001402 | 0.001828 |
| pmr_flat_set_test.test | RUN | 0.144347 | 0.277972 | 0.380445 | 0.001808 | 0.001717 | 0.001912 |
| pmr_flat_map_test.test | RUN | 0.173408 | 0.320318 | 0.461330 | 0.001580 | 0.001524 | 0.001687 |
| pmr_devector_test.test | RUN | 0.112533 | 0.277264 | 0.348019 | 0.001512 | 0.001462 | 0.001940 |
| pmr_deque_test.test | RUN | 0.113162 | 0.229708 | 0.351758 | 0.001828 | 0.001787 | 0.001836 |
| pair_test.test | RUN | 0.282322 | 0.544739 | 0.811063 | 0.001772 | 0.001929 | 0.002101 |
| null_iterators_test.test | RUN | 0.426978 | 1.061927 | 1.355762 | 0.001790 | 0.001894 | 0.001956 |
| node_handle_test.test | RUN | 0.210679 | 1.313311 | 0.560961 | 0.001765 | 0.001991 | 0.001586 |
| node_allocator_test.test | RUN | 2.341636 | 13.334850 | 11.427599 | 0.010384 | 0.010242 | 0.010784 |
| new_delete_resource_test.test | RUN | 0.262816 | 0.825952 | 0.760674 | 0.002682 | 0.002719 | 0.002634 |
| nest_test.test | RUN | 0.607115 | 3.679997 | 2.383106 | 0.002005 | 0.002004 | 0.001824 |
| nest_api_test.test | RUN | 1.159608 | 7.003123 | 4.881126 | 0.002789 | 0.002752 | 0.002651 |
| monotonic_buffer_resource_test.test | RUN | 0.269917 | 1.425931 | 0.750139 | 0.001804 | 0.001842 | 0.001910 |
| memory_resource_test.test | RUN | 0.165471 | 0.391659 | 0.429429 | 0.001828 | 0.001642 | 0.001820 |
| map_test.test | RUN | 4.802797 | 22.800071 | 23.828235 | 0.011006 | 0.011246 | 0.010935 |
| list_test.test | RUN | 2.175610 | 11.586106 | 9.883921 | 0.023857 | 0.025374 | 0.025477 |
| is_constructible_test.test | RUN | 0.097755 | 0.152152 | 0.259172 | 0.000791 | 0.000779 | 0.001796 |
| insert_vs_emplace_test.test | RUN | 0.263209 | 0.958234 | 1.396000 | 0.001806 | 0.001921 | 0.001843 |
| hub_uses_allocator_test.test | RUN | 0.318342 | 0.817235 | 0.955313 | 0.001771 | 0.001861 | 0.001624 |
| hub_stability_test.test | RUN | 0.582920 | 3.699363 | 3.290500 | 0.002185 | 0.002268 | 0.002114 |
| hub_sort_test.test | RUN | 0.384424 | 1.710219 | 1.585905 | 0.569262 | 0.528704 | 0.500648 |
| hub_pocxx_test.test | RUN | 0.380250 | 1.954707 | 1.181359 | 0.001696 | 0.001828 | 0.001448 |
| hub_new_extended_alignment_test.test | RUN | 0.230455 | 0.605938 | 0.647731 | 0.002299 | 0.001727 | 0.001561 |
| hub_exception_safety_test.test | RUN | 0.802614 | 4.713209 | 3.408453 | 0.093465 | 0.089746 | 0.097224 |
| hub_empty_sort_test.test | RUN | 0.253837 | 0.568174 | 0.735074 | 0.001913 | 0.001933 | 0.001744 |
| hub_api_test.test | RUN | 2.232416 | 19.269475 | 10.444047 | 0.004449 | 0.003598 | 0.003601 |
| global_resource_test.test | RUN | 0.148126 | 0.399438 | 0.383924 | 0.001686 | 0.001935 | 0.001838 |
| flat_tree_test.test | RUN | 3.932572 | 28.830826 | 19.939264 | 0.001880 | 0.001706 | 0.001833 |
| flat_set_test.test | RUN | 4.770810 | 36.023587 | 25.958740 | 0.008224 | 0.007322 | 0.008004 |
| flat_set_adaptor_test.test | RUN | 3.759128 | 32.133102 | 20.407757 | 0.008397 | 0.007796 | 0.008676 |
| flat_map_test.test | RUN | 8.231075 | 54.077504 | 42.129671 | 0.008975 | 0.009047 | 0.008405 |
| flat_map_adaptor_test.test | RUN | 5.291208 | 39.546755 | 30.299157 | 0.008717 | 0.008348 | 0.008378 |
| explicit_inst_vector_test.test | RUN | 0.845199 | 5.275651 | 4.561620 | 0.001841 | 0.001884 | 0.001797 |
| explicit_inst_string_test.test | RUN | 1.129906 | 9.496363 | 6.163383 | 0.001695 | 0.001940 | 0.001671 |
| explicit_inst_static_vector_test.test | RUN | 0.127335 | 0.354943 | 0.442670 | 0.001545 | 0.001797 | 0.001684 |
| explicit_inst_stable_vector_test.test | RUN | 0.727215 | 3.450143 | 2.687724 | 0.001796 | 0.001918 | 0.001762 |
| explicit_inst_small_vector_test.test | RUN | 0.598442 | 3.095419 | 2.541643 | 0.001789 | 0.001859 | 0.001856 |
| explicit_inst_slist_test.test | RUN | 0.448818 | 2.796596 | 2.055963 | 0.001771 | 0.001830 | 0.001619 |
| explicit_inst_set_test.test | RUN | 0.664864 | 3.286958 | 2.432500 | 0.001893 | 0.001851 | 0.001746 |
| explicit_inst_map_test.test | RUN | 0.737259 | 3.429079 | 2.617107 | 0.001796 | 0.001869 | 0.001743 |
| explicit_inst_list_test.test | RUN | 0.166264 | 0.777687 | 0.671747 | 0.001880 | 0.001824 | 0.001835 |
| explicit_inst_flat_set_test.test | RUN | 3.497352 | 24.542182 | 15.151387 | 0.001838 | 0.001875 | 0.001429 |
| explicit_inst_flat_map_test.test | RUN | 3.919321 | 25.603790 | 17.302872 | 0.001798 | 0.001867 | 0.001823 |
| explicit_inst_devector_test.test | RUN | 0.450609 | 3.079097 | 2.169238 | 0.001451 | 0.001808 | 0.001764 |
| explicit_inst_deque_test.test | RUN | 0.516614 | 3.319624 | 2.367615 | 0.002033 | 0.001843 | 0.001704 |
| devector_test.test | RUN | 7.040765 | 37.579238 | 30.153810 | 0.004265 | 0.003574 | 0.004673 |
| devector_options_test.test | RUN | 0.322984 | 1.847953 | 1.226776 | 0.002233 | 0.002062 | 0.001976 |
| deque_test.test | RUN | 5.017327 | 25.061890 | 18.859115 | 0.366745 | 0.363699 | 0.426855 |
| deque_options_test.test | RUN | 0.262063 | 0.637195 | 0.946176 | 0.002019 | 0.001887 | 0.001873 |
| copy_move_algo_test.test | RUN | 0.259871 | 0.814778 | 0.784386 | 0.002085 | 0.001976 | 0.001766 |
| common_iterator_test.test | RUN | 0.021401 | 0.039529 | 0.038409 | 0.000769 | 0.000642 | 0.001513 |
| boost_iterator_comp_test.test | RUN | 0.354460 | 1.023802 | 1.231399 | 0.001579 | 0.001937 | 0.001900 |
| allocator_traits_test.test | RUN | 0.224429 | 0.454597 | 0.705986 | 0.001866 | 0.001924 | 0.002814 |
| alloc_full_test.test | RUN | 0.221074 | 0.879180 | 0.890870 | 0.004470 | 0.005027 | 0.005095 |
| alloc_basic_test.test | RUN | 0.198606 | 0.661017 | 0.578079 | 0.002022 | 0.001972 | 0.001992 |
| adaptive_pool_test.test | RUN | 1.671777 | 14.324985 | 7.765341 | 0.010597 | 0.013860 | 0.010425 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.930× | 0.931×, 0.928× | 0.965× |
| clang | 0.987× | 0.982×, 0.991× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.010763s | 0.009940s | 0.010615s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001116s | 0.001087s | 0.001104s | byte equality observed |
| alloc_full_test.test | 7 | 0.003107s | 0.003091s | 0.003121s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000998s | 0.001007s | 0.000998s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001062s | 0.001058s | 0.001015s | byte equality observed |
| common_iterator_test.test | 10 | 0.000485s | 0.000490s | 0.001028s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001274s | 0.001105s | 0.001172s | byte equality observed |
| deque_options_test.test | 10 | 0.001084s | 0.001058s | 0.001077s | byte equality observed |
| deque_test.test | 1 | 0.396051s | 0.386528s | 0.460547s | byte equality observed |
| devector_options_test.test | 10 | 0.001209s | 0.001113s | 0.001143s | byte equality observed |
| devector_test.test | 5 | 0.003723s | 0.003180s | 0.004359s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.001008s | 0.000990s | 0.000974s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000976s | 0.000969s | 0.000976s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000976s | 0.000978s | 0.000983s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000968s | 0.000976s | 0.000974s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000956s | 0.000954s | 0.000962s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000976s | 0.000969s | 0.000969s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000990s | 0.000967s | 0.000960s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000983s | 0.000963s | 0.000970s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000974s | 0.000971s | 0.000979s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000985s | 0.000985s | 0.000981s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000954s | 0.000965s | 0.000962s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000976s | 0.000981s | 0.000987s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000979s | 0.000994s | 0.000966s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.006018s | 0.005686s | 0.005782s | byte equality observed |
| flat_map_test.test | 3 | 0.006600s | 0.006169s | 0.006007s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.006003s | 0.005476s | 0.005955s | byte equality observed |
| flat_set_test.test | 4 | 0.006119s | 0.005251s | 0.005882s | byte equality observed |
| flat_tree_test.test | 10 | 0.001021s | 0.001031s | 0.000998s | byte equality observed |
| global_resource_test.test | 10 | 0.001027s | 0.001038s | 0.001002s | byte equality observed |
| hub_api_test.test | 6 | 0.003212s | 0.002252s | 0.002198s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001042s | 0.000998s | 0.000995s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.071665s | 0.065815s | 0.069973s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001143s | 0.001156s | 0.001105s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001064s | 0.001054s | 0.001068s | byte equality observed |
| hub_sort_test.test | 1 | 0.471294s | 0.431146s | 0.418536s | byte equality observed |
| hub_stability_test.test | 10 | 0.001243s | 0.001235s | 0.001243s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000990s | 0.000972s | 0.000978s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001020s | 0.001033s | 0.001021s | byte equality observed |
| is_constructible_test.test | 10 | 0.000453s | 0.000473s | 0.000965s | byte equality observed |
| list_test.test | 2 | 0.017682s | 0.017476s | 0.017681s | byte equality observed |
| map_test.test | 3 | 0.007916s | 0.007921s | 0.007808s | byte equality observed |
| memory_resource_test.test | 10 | 0.001046s | 0.001049s | 0.001034s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001022s | 0.001020s | 0.001036s | byte equality observed |
| nest_api_test.test | 10 | 0.001722s | 0.001661s | 0.001688s | byte equality observed |
| nest_test.test | 10 | 0.001045s | 0.001098s | 0.001066s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001510s | 0.001514s | 0.001490s | byte equality observed |
| node_allocator_test.test | 3 | 0.007513s | 0.007291s | 0.007425s | byte equality observed |
| node_handle_test.test | 10 | 0.001079s | 0.001032s | 0.001085s | byte equality observed |
| null_iterators_test.test | 10 | 0.001018s | 0.001058s | 0.001015s | byte equality observed |
| pair_test.test | 10 | 0.001005s | 0.001014s | 0.001056s | byte equality observed |
| pmr_deque_test.test | 10 | 0.001005s | 0.000986s | 0.001024s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000993s | 0.000988s | 0.000990s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000983s | 0.000996s | 0.000999s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000990s | 0.001026s | 0.000996s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000981s | 0.001005s | 0.000999s | byte equality observed |
| pmr_list_test.test | 10 | 0.001002s | 0.001037s | 0.001005s | byte equality observed |
| pmr_map_test.test | 10 | 0.001000s | 0.001054s | 0.001013s | byte equality observed |
| pmr_nest_test.test | 10 | 0.001008s | 0.001041s | 0.001012s | byte equality observed |
| pmr_set_test.test | 10 | 0.000987s | 0.001074s | 0.001055s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000996s | 0.001011s | 0.001015s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.001001s | 0.001032s | 0.001034s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001049s | 0.001041s | 0.001044s | byte equality observed |
| pmr_string_test.test | 10 | 0.000494s | 0.000509s | 0.001019s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000487s | 0.000504s | 0.001023s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001057s | 0.001083s | 0.001016s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001041s | 0.001078s | 0.001021s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001054s | 0.001036s | 0.001036s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001057s | 0.001061s | 0.001024s | byte equality observed |
| segtor_options_test.test | 10 | 0.001102s | 0.001030s | 0.001056s | byte equality observed |
| segtor_test.test | 1 | 0.409992s | 0.373661s | 0.375738s | byte equality observed |
| set_test.test | 2 | 0.011196s | 0.009767s | 0.011095s | byte equality observed |
| slist_test.test | 2 | 0.017344s | 0.017335s | 0.017021s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001184s | 0.001166s | 0.001089s | byte equality observed |
| small_vector_test.test | 6 | 0.002609s | 0.002051s | 0.003614s | byte equality observed |
| stable_vector_test.test | 1 | 0.025842s | 0.025537s | 0.026545s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001145s | 0.001072s | 0.001073s | byte equality observed |
| static_vector_test.test | 10 | 0.001434s | 0.001280s | 0.001324s | byte equality observed |
| string_options_test.test | 10 | 0.001068s | 0.001054s | 0.001049s | byte equality observed |
| string_test.test | 2 | 0.011034s | 0.011965s | 0.011627s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001051s | 0.001033s | 0.000998s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001058s | 0.001061s | 0.001038s | byte equality observed |
| throw_exception_test.test | 10 | 0.000998s | 0.000993s | 0.000989s | byte equality observed |
| tree_test.test | 10 | 0.000997s | 0.001015s | 0.000966s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001076s | 0.001065s | 0.001046s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000995s | 0.001010s | 0.000984s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000470s | 0.000473s | 0.000974s | byte equality observed |
| vector_options_test.test | 10 | 0.001166s | 0.001169s | 0.001132s | byte equality observed |
| vector_test.test | 4 | 0.004078s | 0.003666s | 0.005394s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
