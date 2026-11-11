# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.833480 | 31.237205 | 26.971620 | 0.006384 | 0.005517 | 0.007530 |
| vector_options_test.test | RUN | 0.332088 | 1.442195 | 1.217162 | 0.001978 | 0.002067 | 0.001952 |
| uses_allocator_test.test | RUN | 0.067153 | 0.124575 | 0.159505 | 0.000739 | 0.000736 | 0.001716 |
| uses_allocator_construction_test.test | RUN | 0.192799 | 0.444128 | 0.513447 | 0.001788 | 0.001690 | 0.001773 |
| unsynchronized_pool_resource_test.test | RUN | 0.265860 | 1.307671 | 0.694729 | 0.001958 | 0.001868 | 0.001722 |
| tree_test.test | RUN | 0.951422 | 4.693105 | 3.877312 | 0.001711 | 0.001739 | 0.001699 |
| throw_exception_test.test | RUN | 0.135870 | 0.281026 | 0.395503 | 0.001596 | 0.001576 | 0.001721 |
| synchronized_pool_resource_test.test | RUN | 0.268132 | 1.297788 | 0.733690 | 0.001805 | 0.001771 | 0.001842 |
| string_view_compat_test.test | RUN | 0.432610 | 2.607264 | 1.433361 | 0.001612 | 0.001860 | 0.001681 |
| string_test.test | RUN | 1.737121 | 9.936343 | 7.316622 | 0.015088 | 0.015574 | 0.015248 |
| string_options_test.test | RUN | 0.290951 | 2.162625 | 1.057411 | 0.001776 | 0.001985 | 0.001724 |
| static_vector_test.test | RUN | 2.000383 | 8.314318 | 7.249759 | 0.003072 | 0.002259 | 0.002362 |
| static_vector_options_test.test | RUN | 0.220696 | 0.571729 | 0.699015 | 0.001792 | 0.002422 | 0.001854 |
| stable_vector_test.test | RUN | 4.249355 | 16.805336 | 13.813345 | 0.031843 | 0.031887 | 0.033327 |
| small_vector_test.test | RUN | 2.384259 | 11.804196 | 9.908412 | 0.004144 | 0.003301 | 0.005250 |
| small_vector_options_test.test | RUN | 0.379801 | 1.434156 | 1.370403 | 0.001914 | 0.001945 | 0.001624 |
| slist_test.test | RUN | 1.848642 | 10.745596 | 8.832357 | 0.025346 | 0.024683 | 0.024693 |
| set_test.test | RUN | 3.182726 | 19.847291 | 16.442818 | 0.015274 | 0.013844 | 0.015325 |
| segtor_test.test | RUN | 4.545509 | 21.668664 | 18.233095 | 0.669239 | 0.649108 | 0.654728 |
| segtor_options_test.test | RUN | 0.257149 | 0.661041 | 0.802635 | 0.001692 | 0.001900 | 0.001858 |
| scoped_allocator_usage_test.test | RUN | 0.823865 | 3.271766 | 3.071079 | 0.001729 | 0.001588 | 0.001714 |
| scoped_allocator_adaptor_test.test | RUN | 0.415837 | 0.988369 | 1.400886 | 0.001721 | 0.001590 | 0.001434 |
| resource_adaptor_test.test | RUN | 0.216055 | 0.553955 | 0.611574 | 0.001698 | 0.001434 | 0.001605 |
| polymorphic_allocator_test.test | RUN | 0.186899 | 0.455911 | 0.511673 | 0.001622 | 0.001699 | 0.001467 |
| pmr_vector_test.test | RUN | 0.100934 | 0.171943 | 0.289202 | 0.000625 | 0.000802 | 0.001583 |
| pmr_string_test.test | RUN | 0.194095 | 0.357844 | 0.512388 | 0.000772 | 0.000741 | 0.001560 |
| pmr_stable_vector_test.test | RUN | 0.209097 | 0.569709 | 0.725060 | 0.001398 | 0.001712 | 0.001673 |
| pmr_small_vector_test.test | RUN | 0.121685 | 0.247181 | 0.349155 | 0.001620 | 0.001526 | 0.001553 |
| pmr_slist_test.test | RUN | 0.155847 | 0.750745 | 0.709244 | 0.001676 | 0.001826 | 0.001703 |
| pmr_set_test.test | RUN | 0.182508 | 0.414347 | 0.544314 | 0.001303 | 0.001763 | 0.001383 |
| pmr_nest_test.test | RUN | 0.145513 | 0.269422 | 0.310730 | 0.001386 | 0.001946 | 0.001725 |
| pmr_map_test.test | RUN | 0.183212 | 0.404644 | 0.490541 | 0.001329 | 0.001650 | 0.001788 |
| pmr_list_test.test | RUN | 0.126365 | 0.254918 | 0.350602 | 0.001335 | 0.001707 | 0.001752 |
| pmr_hub_test.test | RUN | 0.185099 | 0.362025 | 0.494745 | 0.001501 | 0.001804 | 0.001700 |
| pmr_flat_set_test.test | RUN | 0.141935 | 0.277577 | 0.406214 | 0.001692 | 0.001628 | 0.001616 |
| pmr_flat_map_test.test | RUN | 0.163422 | 0.319361 | 0.494083 | 0.001522 | 0.001370 | 0.001752 |
| pmr_devector_test.test | RUN | 0.111524 | 0.272076 | 0.322788 | 0.001532 | 0.001752 | 0.001569 |
| pmr_deque_test.test | RUN | 0.110243 | 0.226775 | 0.334622 | 0.001855 | 0.001756 | 0.001638 |
| pair_test.test | RUN | 0.279477 | 0.574686 | 0.712085 | 0.001699 | 0.001802 | 0.001790 |
| null_iterators_test.test | RUN | 0.416366 | 1.066606 | 1.235433 | 0.001760 | 0.001753 | 0.001587 |
| node_handle_test.test | RUN | 0.204603 | 1.326862 | 0.522280 | 0.001632 | 0.001827 | 0.001734 |
| node_allocator_test.test | RUN | 2.330847 | 12.993521 | 11.263440 | 0.010518 | 0.010440 | 0.010606 |
| new_delete_resource_test.test | RUN | 0.243150 | 0.814315 | 0.697076 | 0.002522 | 0.002600 | 0.002442 |
| nest_test.test | RUN | 0.595842 | 3.655053 | 2.354220 | 0.001837 | 0.001912 | 0.001776 |
| nest_api_test.test | RUN | 1.153425 | 6.847259 | 4.807955 | 0.002732 | 0.002513 | 0.002606 |
| monotonic_buffer_resource_test.test | RUN | 0.263491 | 1.404292 | 0.570474 | 0.001740 | 0.001912 | 0.001686 |
| memory_resource_test.test | RUN | 0.158926 | 0.422090 | 0.425303 | 0.001738 | 0.001803 | 0.001759 |
| map_test.test | RUN | 4.722957 | 22.553021 | 23.540384 | 0.010940 | 0.010884 | 0.010234 |
| list_test.test | RUN | 2.147052 | 11.573625 | 9.586789 | 0.024941 | 0.022987 | 0.025906 |
| is_constructible_test.test | RUN | 0.092440 | 0.145793 | 0.258172 | 0.000759 | 0.000738 | 0.001758 |
| insert_vs_emplace_test.test | RUN | 0.260318 | 0.960930 | 1.301366 | 0.001862 | 0.001808 | 0.001778 |
| hub_uses_allocator_test.test | RUN | 0.308616 | 0.765977 | 0.951629 | 0.001839 | 0.001860 | 0.001746 |
| hub_stability_test.test | RUN | 0.579250 | 3.633091 | 3.272835 | 0.002142 | 0.002106 | 0.002028 |
| hub_sort_test.test | RUN | 0.370973 | 1.694571 | 1.632552 | 0.564995 | 0.523098 | 0.501460 |
| hub_pocxx_test.test | RUN | 0.376324 | 1.955359 | 1.208112 | 0.001843 | 0.001624 | 0.001757 |
| hub_new_extended_alignment_test.test | RUN | 0.227120 | 0.619475 | 0.691469 | 0.001839 | 0.001658 | 0.001685 |
| hub_exception_safety_test.test | RUN | 0.786951 | 4.640885 | 3.142163 | 0.089579 | 0.086507 | 0.092476 |
| hub_empty_sort_test.test | RUN | 0.231344 | 0.545838 | 0.754768 | 0.001776 | 0.001803 | 0.001874 |
| hub_api_test.test | RUN | 2.244623 | 18.659672 | 10.621010 | 0.004522 | 0.003704 | 0.003445 |
| global_resource_test.test | RUN | 0.143500 | 0.394930 | 0.415929 | 0.001928 | 0.001881 | 0.001840 |
| flat_tree_test.test | RUN | 3.863317 | 28.479954 | 19.401486 | 0.001864 | 0.001863 | 0.001668 |
| flat_set_test.test | RUN | 4.808174 | 35.994078 | 25.203801 | 0.008209 | 0.007549 | 0.008094 |
| flat_set_adaptor_test.test | RUN | 3.747062 | 31.879529 | 20.086655 | 0.008090 | 0.007930 | 0.008488 |
| flat_map_test.test | RUN | 8.195502 | 53.928075 | 42.090893 | 0.009380 | 0.008562 | 0.008431 |
| flat_map_adaptor_test.test | RUN | 5.317423 | 39.079418 | 29.801498 | 0.008542 | 0.008065 | 0.008204 |
| explicit_inst_vector_test.test | RUN | 0.855065 | 5.388667 | 4.395702 | 0.001744 | 0.001748 | 0.001648 |
| explicit_inst_string_test.test | RUN | 1.132596 | 9.510742 | 6.027533 | 0.001655 | 0.001848 | 0.001555 |
| explicit_inst_static_vector_test.test | RUN | 0.123327 | 0.352269 | 0.433479 | 0.001437 | 0.001837 | 0.001594 |
| explicit_inst_stable_vector_test.test | RUN | 0.736199 | 3.406516 | 2.685587 | 0.001692 | 0.001644 | 0.001698 |
| explicit_inst_small_vector_test.test | RUN | 0.597115 | 3.096044 | 2.638886 | 0.001504 | 0.001808 | 0.001733 |
| explicit_inst_slist_test.test | RUN | 0.450848 | 2.787203 | 2.046377 | 0.001790 | 0.001780 | 0.001720 |
| explicit_inst_set_test.test | RUN | 0.671403 | 3.281967 | 2.384570 | 0.001796 | 0.001768 | 0.001653 |
| explicit_inst_map_test.test | RUN | 0.740169 | 3.381952 | 2.624593 | 0.001704 | 0.001763 | 0.001719 |
| explicit_inst_list_test.test | RUN | 0.167545 | 0.771854 | 0.662936 | 0.001772 | 0.001727 | 0.001616 |
| explicit_inst_flat_set_test.test | RUN | 3.407876 | 23.954541 | 15.082995 | 0.001850 | 0.001823 | 0.001777 |
| explicit_inst_flat_map_test.test | RUN | 3.844544 | 24.940207 | 17.004651 | 0.001699 | 0.001759 | 0.001563 |
| explicit_inst_devector_test.test | RUN | 0.451438 | 3.076443 | 2.095978 | 0.001580 | 0.001763 | 0.001635 |
| explicit_inst_deque_test.test | RUN | 0.499353 | 3.374914 | 2.229964 | 0.001689 | 0.001452 | 0.001716 |
| devector_test.test | RUN | 6.359278 | 40.733785 | 27.094460 | 0.004104 | 0.003526 | 0.004600 |
| devector_options_test.test | RUN | 0.315270 | 1.819348 | 1.215768 | 0.001997 | 0.002008 | 0.001912 |
| deque_test.test | RUN | 5.002452 | 20.307729 | 19.625377 | 0.390949 | 0.360072 | 0.430179 |
| deque_options_test.test | RUN | 0.259520 | 0.705338 | 0.934549 | 0.001785 | 0.001982 | 0.001907 |
| copy_move_algo_test.test | RUN | 0.257061 | 0.797421 | 0.877699 | 0.001882 | 0.001892 | 0.001884 |
| common_iterator_test.test | RUN | 0.020730 | 0.043378 | 0.043971 | 0.000749 | 0.000772 | 0.001876 |
| boost_iterator_comp_test.test | RUN | 0.340572 | 1.010596 | 1.226580 | 0.001654 | 0.001800 | 0.001794 |
| allocator_traits_test.test | RUN | 0.215082 | 0.449535 | 0.632688 | 0.001718 | 0.001806 | 0.001749 |
| alloc_full_test.test | RUN | 0.216582 | 0.875427 | 0.830656 | 0.004758 | 0.004734 | 0.004862 |
| alloc_basic_test.test | RUN | 0.189869 | 0.658731 | 0.629718 | 0.001744 | 0.001964 | 0.001883 |
| adaptive_pool_test.test | RUN | 2.332543 | 13.513427 | 11.305631 | 0.010421 | 0.009618 | 0.015036 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.954× | 0.961×, 0.946× | 0.982× |
| clang | 0.975× | 0.972×, 0.979× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.010759s | 0.009959s | 0.010508s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001087s | 0.001062s | 0.001072s | byte equality observed |
| alloc_full_test.test | 7 | 0.003041s | 0.003091s | 0.003081s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001005s | 0.000996s | 0.001004s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001036s | 0.001043s | 0.001040s | byte equality observed |
| common_iterator_test.test | 10 | 0.000480s | 0.000475s | 0.001022s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001252s | 0.001122s | 0.001161s | byte equality observed |
| deque_options_test.test | 10 | 0.001100s | 0.001078s | 0.001099s | byte equality observed |
| deque_test.test | 1 | 0.396853s | 0.390387s | 0.455480s | byte equality observed |
| devector_options_test.test | 10 | 0.001170s | 0.001125s | 0.001141s | byte equality observed |
| devector_test.test | 5 | 0.003705s | 0.003173s | 0.004323s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000983s | 0.000981s | 0.000958s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000963s | 0.000977s | 0.000946s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000985s | 0.000989s | 0.000947s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000988s | 0.000979s | 0.000944s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000969s | 0.000961s | 0.000943s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.001000s | 0.000965s | 0.000974s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000980s | 0.000968s | 0.000956s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000985s | 0.000961s | 0.000953s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000976s | 0.000994s | 0.000957s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000988s | 0.000982s | 0.000979s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000969s | 0.000961s | 0.000949s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000979s | 0.000990s | 0.000973s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000997s | 0.000975s | 0.000966s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005932s | 0.005638s | 0.005788s | byte equality observed |
| flat_map_test.test | 3 | 0.006477s | 0.006066s | 0.006122s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.005978s | 0.005381s | 0.006005s | byte equality observed |
| flat_set_test.test | 4 | 0.006050s | 0.005184s | 0.006021s | byte equality observed |
| flat_tree_test.test | 10 | 0.000989s | 0.000973s | 0.000992s | byte equality observed |
| global_resource_test.test | 10 | 0.001014s | 0.001005s | 0.001030s | byte equality observed |
| hub_api_test.test | 6 | 0.003191s | 0.002220s | 0.002227s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.001007s | 0.000984s | 0.000998s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.071835s | 0.065987s | 0.066909s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001052s | 0.001043s | 0.001046s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001032s | 0.001036s | 0.001033s | byte equality observed |
| hub_sort_test.test | 1 | 0.462445s | 0.438555s | 0.422127s | byte equality observed |
| hub_stability_test.test | 10 | 0.001263s | 0.001223s | 0.001211s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000995s | 0.000994s | 0.001000s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001028s | 0.001012s | 0.001045s | byte equality observed |
| is_constructible_test.test | 10 | 0.000471s | 0.000470s | 0.000959s | byte equality observed |
| list_test.test | 2 | 0.017689s | 0.017496s | 0.017944s | byte equality observed |
| map_test.test | 3 | 0.007939s | 0.007803s | 0.007880s | byte equality observed |
| memory_resource_test.test | 10 | 0.001038s | 0.000991s | 0.000996s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001021s | 0.001005s | 0.001018s | byte equality observed |
| nest_api_test.test | 10 | 0.001713s | 0.001625s | 0.001655s | byte equality observed |
| nest_test.test | 10 | 0.001036s | 0.001014s | 0.001043s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001507s | 0.001445s | 0.001501s | byte equality observed |
| node_allocator_test.test | 3 | 0.007313s | 0.007187s | 0.007248s | byte equality observed |
| node_handle_test.test | 10 | 0.001030s | 0.001025s | 0.001005s | byte equality observed |
| null_iterators_test.test | 10 | 0.001008s | 0.000989s | 0.001022s | byte equality observed |
| pair_test.test | 10 | 0.001002s | 0.000970s | 0.001010s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000985s | 0.000969s | 0.000987s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000976s | 0.000961s | 0.000976s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000992s | 0.000953s | 0.000956s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000986s | 0.000967s | 0.000957s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000982s | 0.000985s | 0.000964s | byte equality observed |
| pmr_list_test.test | 10 | 0.000989s | 0.000988s | 0.000959s | byte equality observed |
| pmr_map_test.test | 10 | 0.000992s | 0.000972s | 0.000977s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000988s | 0.000980s | 0.001003s | byte equality observed |
| pmr_set_test.test | 10 | 0.000983s | 0.000971s | 0.000992s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000996s | 0.000985s | 0.001017s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000981s | 0.000982s | 0.001005s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001000s | 0.000994s | 0.001009s | byte equality observed |
| pmr_string_test.test | 10 | 0.000483s | 0.000471s | 0.001023s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000476s | 0.000468s | 0.001031s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001054s | 0.001080s | 0.001027s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001034s | 0.001034s | 0.001013s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001044s | 0.001034s | 0.001026s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001053s | 0.001049s | 0.001029s | byte equality observed |
| segtor_options_test.test | 10 | 0.001042s | 0.001024s | 0.001027s | byte equality observed |
| segtor_test.test | 1 | 0.416841s | 0.381306s | 0.372640s | byte equality observed |
| set_test.test | 2 | 0.011123s | 0.009690s | 0.011111s | byte equality observed |
| slist_test.test | 2 | 0.017051s | 0.017062s | 0.017170s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001109s | 0.001068s | 0.001090s | byte equality observed |
| small_vector_test.test | 6 | 0.002505s | 0.001938s | 0.003583s | byte equality observed |
| stable_vector_test.test | 1 | 0.025701s | 0.025434s | 0.026560s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001087s | 0.001026s | 0.001060s | byte equality observed |
| static_vector_test.test | 10 | 0.001411s | 0.001234s | 0.001295s | byte equality observed |
| string_options_test.test | 10 | 0.001038s | 0.001023s | 0.001053s | byte equality observed |
| string_test.test | 2 | 0.010992s | 0.011992s | 0.011547s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001006s | 0.001003s | 0.000995s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001038s | 0.001073s | 0.001038s | byte equality observed |
| throw_exception_test.test | 10 | 0.000963s | 0.000972s | 0.000974s | byte equality observed |
| tree_test.test | 10 | 0.000964s | 0.000976s | 0.000969s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001037s | 0.001052s | 0.001052s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000975s | 0.000978s | 0.000978s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000458s | 0.000467s | 0.000957s | byte equality observed |
| vector_options_test.test | 10 | 0.001154s | 0.001125s | 0.001161s | byte equality observed |
| vector_test.test | 4 | 0.004134s | 0.003490s | 0.004981s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
