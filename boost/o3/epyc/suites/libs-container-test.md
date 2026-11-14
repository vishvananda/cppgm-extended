# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 5.528316 | 27.260936 | 22.860822 | 0.004515 | 0.004752 | 0.005976 |
| vector_options_test.test | RUN | 0.262429 | 1.222718 | 0.989222 | 0.001930 | 0.001870 | 0.002029 |
| uses_allocator_test.test | RUN | 0.048599 | 0.098746 | 0.124192 | 0.000805 | 0.000740 | 0.001812 |
| uses_allocator_construction_test.test | RUN | 0.144855 | 0.364384 | 0.400904 | 0.001676 | 0.001424 | 0.001772 |
| unsynchronized_pool_resource_test.test | RUN | 0.178000 | 1.111323 | 0.554109 | 0.001636 | 0.001852 | 0.001788 |
| tree_test.test | RUN | 0.824996 | 4.062567 | 3.293756 | 0.001532 | 0.001671 | 0.001880 |
| throw_exception_test.test | RUN | 0.103932 | 0.238388 | 0.288009 | 0.001784 | 0.001608 | 0.001790 |
| synchronized_pool_resource_test.test | RUN | 0.182598 | 1.091730 | 0.560437 | 0.001594 | 0.001960 | 0.001918 |
| string_view_compat_test.test | RUN | 0.314287 | 2.196175 | 1.064292 | 0.001757 | 0.001779 | 0.001767 |
| string_test.test | RUN | 1.289767 | 8.439979 | 6.181620 | 0.013196 | 0.011970 | 0.012468 |
| string_options_test.test | RUN | 0.248041 | 1.729465 | 0.845967 | 0.001882 | 0.001882 | 0.001960 |
| static_vector_test.test | RUN | 1.395808 | 7.008880 | 6.271764 | 0.002314 | 0.002334 | 0.002132 |
| static_vector_options_test.test | RUN | 0.177315 | 0.462645 | 0.537878 | 0.001736 | 0.001788 | 0.001830 |
| stable_vector_test.test | RUN | 3.202390 | 15.239660 | 11.739041 | 0.025295 | 0.025818 | 0.026340 |
| small_vector_test.test | RUN | 2.148275 | 10.563231 | 8.463699 | 0.003001 | 0.002986 | 0.004465 |
| small_vector_options_test.test | RUN | 0.317703 | 1.247950 | 1.089518 | 0.001893 | 0.002725 | 0.001831 |
| slist_test.test | RUN | 2.007675 | 9.582898 | 7.439186 | 0.019738 | 0.019601 | 0.019213 |
| set_test.test | RUN | 3.025896 | 17.534969 | 13.779783 | 0.013008 | 0.011274 | 0.012867 |
| segtor_test.test | RUN | 3.831600 | 18.664917 | 15.397350 | 0.459039 | 0.440538 | 0.417036 |
| segtor_options_test.test | RUN | 0.198330 | 0.519649 | 0.631322 | 0.001596 | 0.001750 | 0.001766 |
| scoped_allocator_usage_test.test | RUN | 0.697396 | 2.810423 | 2.418614 | 0.001725 | 0.001746 | 0.001592 |
| scoped_allocator_adaptor_test.test | RUN | 0.310616 | 0.808848 | 1.008256 | 0.001536 | 0.001741 | 0.001496 |
| resource_adaptor_test.test | RUN | 0.164937 | 0.468347 | 0.457693 | 0.001711 | 0.001696 | 0.001371 |
| polymorphic_allocator_test.test | RUN | 0.142497 | 0.378315 | 0.339474 | 0.001570 | 0.001765 | 0.001379 |
| pmr_vector_test.test | RUN | 0.079094 | 0.139823 | 0.180767 | 0.000678 | 0.000778 | 0.001362 |
| pmr_string_test.test | RUN | 0.156475 | 0.303096 | 0.342767 | 0.000738 | 0.000775 | 0.001638 |
| pmr_stable_vector_test.test | RUN | 0.176206 | 0.496725 | 0.506103 | 0.001710 | 0.001704 | 0.001590 |
| pmr_small_vector_test.test | RUN | 0.087713 | 0.199562 | 0.224372 | 0.001686 | 0.001722 | 0.001674 |
| pmr_slist_test.test | RUN | 0.123996 | 0.648250 | 0.544317 | 0.001698 | 0.001483 | 0.001612 |
| pmr_set_test.test | RUN | 0.137097 | 0.328363 | 0.430144 | 0.001597 | 0.001648 | 0.001577 |
| pmr_nest_test.test | RUN | 0.110212 | 0.198232 | 0.276163 | 0.001706 | 0.001717 | 0.001810 |
| pmr_map_test.test | RUN | 0.152143 | 0.327610 | 0.412042 | 0.001732 | 0.001269 | 0.001714 |
| pmr_list_test.test | RUN | 0.099264 | 0.202147 | 0.259145 | 0.001480 | 0.001671 | 0.001759 |
| pmr_hub_test.test | RUN | 0.134982 | 0.270692 | 0.327120 | 0.001726 | 0.001473 | 0.001741 |
| pmr_flat_set_test.test | RUN | 0.107271 | 0.211094 | 0.284158 | 0.001699 | 0.001587 | 0.001683 |
| pmr_flat_map_test.test | RUN | 0.124936 | 0.246299 | 0.357635 | 0.001812 | 0.001504 | 0.001611 |
| pmr_devector_test.test | RUN | 0.086512 | 0.213541 | 0.279570 | 0.001701 | 0.001376 | 0.001673 |
| pmr_deque_test.test | RUN | 0.085291 | 0.180219 | 0.241077 | 0.001731 | 0.001733 | 0.001670 |
| pair_test.test | RUN | 0.217467 | 0.442047 | 0.610199 | 0.001709 | 0.001836 | 0.001811 |
| null_iterators_test.test | RUN | 0.324791 | 0.846057 | 0.926856 | 0.001755 | 0.001694 | 0.001807 |
| node_handle_test.test | RUN | 0.142372 | 1.000627 | 0.416105 | 0.001509 | 0.001776 | 0.001663 |
| node_allocator_test.test | RUN | 2.305175 | 11.447553 | 10.102166 | 0.008762 | 0.007635 | 0.009417 |
| new_delete_resource_test.test | RUN | 0.176297 | 0.671260 | 0.554935 | 0.002280 | 0.002355 | 0.002288 |
| nest_test.test | RUN | 0.441953 | 2.939672 | 2.168683 | 0.001724 | 0.001776 | 0.003903 |
| nest_api_test.test | RUN | 0.812774 | 5.843022 | 4.361156 | 0.002477 | 0.002416 | 0.002620 |
| monotonic_buffer_resource_test.test | RUN | 0.186967 | 1.191487 | 0.576869 | 0.001533 | 0.001764 | 0.001749 |
| memory_resource_test.test | RUN | 0.106813 | 0.326969 | 0.345175 | 0.001810 | 0.001526 | 0.001759 |
| map_test.test | RUN | 4.124820 | 19.291339 | 20.152930 | 0.009953 | 0.009166 | 0.008338 |
| list_test.test | RUN | 2.138056 | 9.909835 | 8.612077 | 0.019424 | 0.019546 | 0.019342 |
| is_constructible_test.test | RUN | 0.066288 | 0.123030 | 0.188990 | 0.000802 | 0.000798 | 0.001858 |
| insert_vs_emplace_test.test | RUN | 0.219975 | 0.755287 | 1.200822 | 0.001699 | 0.001712 | 0.001838 |
| hub_uses_allocator_test.test | RUN | 0.239503 | 0.644147 | 0.795498 | 0.001513 | 0.001728 | 0.001839 |
| hub_stability_test.test | RUN | 0.491468 | 3.062293 | 2.924838 | 0.001920 | 0.002166 | 0.002221 |
| hub_sort_test.test | RUN | 0.318586 | 1.389157 | 1.460738 | 0.479919 | 0.473990 | 0.468753 |
| hub_pocxx_test.test | RUN | 0.292831 | 1.674179 | 0.994514 | 0.001867 | 0.001826 | 0.001758 |
| hub_new_extended_alignment_test.test | RUN | 0.167046 | 0.518952 | 0.532962 | 0.001724 | 0.001868 | 0.001514 |
| hub_exception_safety_test.test | RUN | 0.613237 | 3.854980 | 3.110188 | 0.075743 | 0.075944 | 0.092874 |
| hub_empty_sort_test.test | RUN | 0.181640 | 0.485623 | 0.606600 | 0.001700 | 0.001800 | 0.001744 |
| hub_api_test.test | RUN | 2.027165 | 15.680295 | 9.001428 | 0.003741 | 0.003304 | 0.003061 |
| global_resource_test.test | RUN | 0.111260 | 0.305834 | 0.333410 | 0.001888 | 0.001588 | 0.002020 |
| flat_tree_test.test | RUN | 3.658109 | 23.771678 | 16.963233 | 0.001656 | 0.001765 | 0.001765 |
| flat_set_test.test | RUN | 4.606770 | 29.785843 | 21.841777 | 0.007683 | 0.006106 | 0.007211 |
| flat_set_adaptor_test.test | RUN | 3.432117 | 26.143165 | 16.779400 | 0.007190 | 0.006700 | 0.006897 |
| flat_map_test.test | RUN | 7.497495 | 45.286229 | 35.630075 | 0.008060 | 0.007596 | 0.007642 |
| flat_map_adaptor_test.test | RUN | 4.781238 | 32.872175 | 25.152089 | 0.007636 | 0.006966 | 0.007236 |
| explicit_inst_vector_test.test | RUN | 0.733473 | 4.438921 | 3.963192 | 0.001708 | 0.001730 | 0.001853 |
| explicit_inst_string_test.test | RUN | 0.972974 | 8.000554 | 5.529559 | 0.001718 | 0.001695 | 0.001623 |
| explicit_inst_static_vector_test.test | RUN | 0.097545 | 0.290043 | 0.360891 | 0.001650 | 0.001654 | 0.001739 |
| explicit_inst_stable_vector_test.test | RUN | 0.590732 | 2.825245 | 2.496129 | 0.001660 | 0.001622 | 0.001675 |
| explicit_inst_small_vector_test.test | RUN | 0.501580 | 2.659605 | 2.256456 | 0.001713 | 0.001765 | 0.001712 |
| explicit_inst_slist_test.test | RUN | 0.376742 | 2.423234 | 1.721549 | 0.001751 | 0.001735 | 0.001714 |
| explicit_inst_set_test.test | RUN | 0.538240 | 2.842403 | 1.997239 | 0.001796 | 0.001795 | 0.001760 |
| explicit_inst_map_test.test | RUN | 0.605441 | 2.950777 | 2.248811 | 0.001594 | 0.001905 | 0.001616 |
| explicit_inst_list_test.test | RUN | 0.131816 | 0.684800 | 0.576282 | 0.001492 | 0.001657 | 0.001726 |
| explicit_inst_flat_set_test.test | RUN | 3.091622 | 20.487579 | 13.099179 | 0.001783 | 0.001796 | 0.001713 |
| explicit_inst_flat_map_test.test | RUN | 3.293502 | 21.327494 | 14.540554 | 0.001751 | 0.001646 | 0.001712 |
| explicit_inst_devector_test.test | RUN | 0.389379 | 2.583507 | 1.770653 | 0.001616 | 0.001718 | 0.001695 |
| explicit_inst_deque_test.test | RUN | 0.416635 | 2.781084 | 1.872245 | 0.001827 | 0.001722 | 0.001723 |
| devector_test.test | RUN | 5.521720 | 34.455160 | 25.930251 | 0.003505 | 0.003274 | 0.004170 |
| devector_options_test.test | RUN | 0.247823 | 1.483067 | 1.030657 | 0.002122 | 0.001954 | 0.001938 |
| deque_test.test | RUN | 4.017312 | 20.399802 | 16.431867 | 0.487044 | 0.345047 | 0.343642 |
| deque_options_test.test | RUN | 0.218674 | 0.589250 | 0.634402 | 0.001938 | 0.001716 | 0.001733 |
| copy_move_algo_test.test | RUN | 0.204489 | 0.670653 | 0.558296 | 0.001743 | 0.001831 | 0.001625 |
| common_iterator_test.test | RUN | 0.016635 | 0.038132 | 0.037552 | 0.000694 | 0.000534 | 0.001447 |
| boost_iterator_comp_test.test | RUN | 0.285685 | 0.792012 | 0.945612 | 0.001557 | 0.001795 | 0.001479 |
| allocator_traits_test.test | RUN | 0.166904 | 0.347378 | 0.403409 | 0.001643 | 0.001810 | 0.001656 |
| alloc_full_test.test | RUN | 0.182562 | 0.751674 | 0.680090 | 0.004026 | 0.004046 | 0.004072 |
| alloc_basic_test.test | RUN | 0.149254 | 0.530765 | 0.483897 | 0.001599 | 0.001875 | 0.001783 |
| adaptive_pool_test.test | RUN | 2.315934 | 8.872536 | 6.749384 | 0.008110 | 0.009744 | 0.008133 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.027×† | 0.999×, 1.055× | 1.015× |
| clang | 0.932× | 0.934×, 0.930× | 0.952× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.007684s | 0.007167s | 0.007955s | byte equality observed |
| alloc_basic_test.test | 10 | 0.000955s | 0.000947s | 0.000967s | byte equality observed |
| alloc_full_test.test | 7 | 0.002647s | 0.002586s | 0.002798s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000888s | 0.000875s | 0.000902s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000918s | 0.000904s | 0.000919s | byte equality observed |
| common_iterator_test.test | 10 | 0.000426s | 0.000413s | 0.000901s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001085s | 0.000986s | 0.001032s | byte equality observed |
| deque_options_test.test | 10 | 0.000977s | 0.000952s | 0.000995s | byte equality observed |
| deque_test.test | 1 | 0.335962s | 0.350652s | 0.325877s | byte equality observed |
| devector_options_test.test | 10 | 0.001045s | 0.000984s | 0.001007s | byte equality observed |
| devector_test.test | 5 | 0.002781s | 0.002819s | 0.003595s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000892s | 0.000855s | 0.000906s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000877s | 0.000841s | 0.000896s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000875s | 0.000857s | 0.000878s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000875s | 0.000840s | 0.000873s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000863s | 0.000832s | 0.000859s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000871s | 0.000844s | 0.000889s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000875s | 0.000828s | 0.000870s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000870s | 0.000842s | 0.000862s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000869s | 0.000836s | 0.000859s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000870s | 0.000826s | 0.000865s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000846s | 0.000824s | 0.000855s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000856s | 0.000845s | 0.000854s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000855s | 0.000842s | 0.000864s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005075s | 0.004742s | 0.004771s | byte equality observed |
| flat_map_test.test | 4 | 0.005631s | 0.005092s | 0.004892s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.004703s | 0.004320s | 0.004631s | byte equality observed |
| flat_set_test.test | 4 | 0.004759s | 0.004230s | 0.004498s | byte equality observed |
| flat_tree_test.test | 10 | 0.000909s | 0.000875s | 0.000872s | byte equality observed |
| global_resource_test.test | 10 | 0.000896s | 0.000876s | 0.000896s | byte equality observed |
| hub_api_test.test | 8 | 0.002313s | 0.001949s | 0.001951s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000883s | 0.000858s | 0.000872s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.057949s | 0.056661s | 0.056643s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000918s | 0.001000s | 0.000909s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000940s | 0.000970s | 0.000913s | byte equality observed |
| hub_sort_test.test | 1 | 0.422079s | 0.425261s | 0.396198s | byte equality observed |
| hub_stability_test.test | 10 | 0.001243s | 0.001097s | 0.001186s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000889s | 0.000870s | 0.000888s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000941s | 0.000901s | 0.000902s | byte equality observed |
| is_constructible_test.test | 10 | 0.000406s | 0.000406s | 0.000860s | byte equality observed |
| list_test.test | 2 | 0.013421s | 0.013383s | 0.012876s | byte equality observed |
| map_test.test | 3 | 0.006567s | 0.006671s | 0.006192s | byte equality observed |
| memory_resource_test.test | 10 | 0.000921s | 0.000943s | 0.000890s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000890s | 0.000947s | 0.000896s | byte equality observed |
| nest_api_test.test | 10 | 0.001439s | 0.001527s | 0.001463s | byte equality observed |
| nest_test.test | 10 | 0.000929s | 0.000957s | 0.000922s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001280s | 0.001401s | 0.001326s | byte equality observed |
| node_allocator_test.test | 4 | 0.006031s | 0.005998s | 0.005844s | byte equality observed |
| node_handle_test.test | 10 | 0.000929s | 0.000974s | 0.000898s | byte equality observed |
| null_iterators_test.test | 10 | 0.000870s | 0.000906s | 0.000875s | byte equality observed |
| pair_test.test | 10 | 0.000852s | 0.000902s | 0.000884s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000859s | 0.000862s | 0.000873s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000853s | 0.000862s | 0.000874s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000859s | 0.000865s | 0.000878s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000859s | 0.000851s | 0.000888s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000858s | 0.000865s | 0.000861s | byte equality observed |
| pmr_list_test.test | 10 | 0.000865s | 0.000873s | 0.000868s | byte equality observed |
| pmr_map_test.test | 10 | 0.000857s | 0.000879s | 0.000875s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000862s | 0.000890s | 0.000883s | byte equality observed |
| pmr_set_test.test | 10 | 0.000876s | 0.000893s | 0.000872s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000868s | 0.000895s | 0.000878s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000869s | 0.000890s | 0.000866s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000883s | 0.000888s | 0.000869s | byte equality observed |
| pmr_string_test.test | 10 | 0.000417s | 0.000427s | 0.000872s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000415s | 0.000420s | 0.000895s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000931s | 0.000975s | 0.000905s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000928s | 0.000936s | 0.000899s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000922s | 0.000921s | 0.000903s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000925s | 0.000940s | 0.000898s | byte equality observed |
| segtor_options_test.test | 10 | 0.000924s | 0.000954s | 0.000914s | byte equality observed |
| segtor_test.test | 1 | 0.286434s | 0.275371s | 0.267995s | byte equality observed |
| set_test.test | 3 | 0.008663s | 0.007857s | 0.008494s | byte equality observed |
| slist_test.test | 2 | 0.012676s | 0.013256s | 0.012804s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001002s | 0.001034s | 0.001001s | byte equality observed |
| small_vector_test.test | 7 | 0.001658s | 0.001645s | 0.002730s | byte equality observed |
| stable_vector_test.test | 1 | 0.020805s | 0.021531s | 0.021628s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001038s | 0.000974s | 0.001002s | byte equality observed |
| static_vector_test.test | 10 | 0.001175s | 0.001137s | 0.001142s | byte equality observed |
| string_options_test.test | 10 | 0.000932s | 0.000963s | 0.000942s | byte equality observed |
| string_test.test | 3 | 0.008243s | 0.008566s | 0.008421s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000904s | 0.000917s | 0.000899s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000912s | 0.000967s | 0.000940s | byte equality observed |
| throw_exception_test.test | 10 | 0.000849s | 0.000882s | 0.000883s | byte equality observed |
| tree_test.test | 10 | 0.000851s | 0.000893s | 0.000865s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000925s | 0.000994s | 0.000923s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000868s | 0.000917s | 0.000885s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000398s | 0.000426s | 0.000875s | byte equality observed |
| vector_options_test.test | 10 | 0.001021s | 0.001055s | 0.001030s | byte equality observed |
| vector_test.test | 5 | 0.002977s | 0.003007s | 0.003930s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
