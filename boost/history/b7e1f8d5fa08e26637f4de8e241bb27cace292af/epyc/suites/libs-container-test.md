# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 1.302770 | 12.003417 | 8.145486 | 0.028060 | 0.029318 | 0.029707 |
| vector_options_test.test | RUN | 0.262195 | 0.916476 | 0.891215 | 0.002024 | 0.002148 | 0.001704 |
| uses_allocator_test.test | RUN | 0.062100 | 0.113477 | 0.168101 | 0.000782 | 0.000811 | 0.001642 |
| uses_allocator_construction_test.test | RUN | 0.176336 | 0.434176 | 0.563833 | 0.001573 | 0.001826 | 0.001838 |
| unsynchronized_pool_resource_test.test | RUN | 0.215113 | 0.491889 | 0.601243 | 0.001705 | 0.002028 | 0.002010 |
| tree_test.test | RUN | 0.562275 | 3.101095 | 2.621799 | 0.001624 | 0.001864 | 0.001824 |
| throw_exception_test.test | RUN | 0.127025 | 0.251880 | 0.360807 | 0.001850 | 0.001940 | 0.001714 |
| synchronized_pool_resource_test.test | RUN | 0.201122 | 0.485130 | 0.590781 | 0.001768 | 0.001833 | 0.001917 |
| string_view_compat_test.test | RUN | 0.336983 | 0.723749 | 0.970453 | 0.001725 | 0.001795 | 0.001826 |
| string_test.test | RUN | 0.647615 | 3.561082 | 2.999661 | 0.120341 | 0.141758 | 0.152223 |
| string_options_test.test | RUN | 0.230013 | 0.714137 | 0.826407 | 0.002400 | 0.002480 | 0.002479 |
| static_vector_test.test | RUN | 0.635716 | 4.605242 | 3.420384 | 0.002935 | 0.002997 | 0.002894 |
| static_vector_options_test.test | RUN | 0.211718 | 0.570894 | 0.652350 | 0.001704 | 0.001982 | 0.001883 |
| stable_vector_test.test | RUN | 0.979563 | 8.202130 | 5.705737 | 0.200177 | 0.246265 | 0.280343 |
| small_vector_test.test | RUN | 0.774781 | 5.656103 | 4.240157 | 0.017023 | 0.017724 | 0.019074 |
| small_vector_options_test.test | RUN | 0.303709 | 1.246964 | 1.052189 | 0.002018 | 0.001963 | 0.001998 |
| slist_test.test | RUN | 0.746713 | 5.600461 | 4.353800 | 0.155482 | 0.186991 | 0.176355 |
| set_test.test | RUN | 1.146623 | 9.192815 | 6.641060 | 0.081025 | 0.118624 | 0.094965 |
| segtor_test.test | RUN | 1.118892 | 8.841078 | 6.061612 | 7.209847 | 7.957753 | 8.629944 |
| segtor_options_test.test | RUN | 0.229451 | 0.634719 | 0.719509 | 0.001904 | 0.001540 | 0.001897 |
| scoped_allocator_usage_test.test | RUN | 0.518494 | 2.756276 | 2.307455 | 0.001578 | 0.001751 | 0.001861 |
| scoped_allocator_adaptor_test.test | RUN | 0.381338 | 1.322534 | 1.444274 | 0.001616 | 0.001741 | 0.001769 |
| resource_adaptor_test.test | RUN | 0.191064 | 0.450350 | 0.561210 | 0.001424 | 0.001545 | 0.001670 |
| polymorphic_allocator_test.test | RUN | 0.170246 | 0.374869 | 0.512048 | 0.001558 | 0.001738 | 0.001613 |
| pmr_vector_test.test | RUN | 0.099659 | 0.165229 | 0.238729 | 0.000640 | 0.000776 | 0.001436 |
| pmr_string_test.test | RUN | 0.200800 | 0.354208 | 0.497800 | 0.000600 | 0.000749 | 0.001650 |
| pmr_stable_vector_test.test | RUN | 0.154440 | 0.569761 | 0.579020 | 0.001388 | 0.001692 | 0.001592 |
| pmr_small_vector_test.test | RUN | 0.114380 | 0.263519 | 0.373646 | 0.001504 | 0.001588 | 0.001472 |
| pmr_slist_test.test | RUN | 0.126924 | 0.501079 | 0.419314 | 0.001619 | 0.001351 | 0.001528 |
| pmr_set_test.test | RUN | 0.156385 | 0.447637 | 0.439506 | 0.001601 | 0.001294 | 0.001463 |
| pmr_nest_test.test | RUN | 0.147175 | 0.253186 | 0.367292 | 0.001357 | 0.001692 | 0.001756 |
| pmr_map_test.test | RUN | 0.163098 | 0.394285 | 0.530329 | 0.001726 | 0.001503 | 0.001768 |
| pmr_list_test.test | RUN | 0.129475 | 0.283569 | 0.307330 | 0.001302 | 0.001582 | 0.001518 |
| pmr_hub_test.test | RUN | 0.182642 | 0.353836 | 0.441788 | 0.001309 | 0.001733 | 0.001656 |
| pmr_flat_set_test.test | RUN | 0.128414 | 0.299932 | 0.354905 | 0.001366 | 0.001645 | 0.001782 |
| pmr_flat_map_test.test | RUN | 0.156430 | 0.353480 | 0.376054 | 0.001632 | 0.001778 | 0.001616 |
| pmr_devector_test.test | RUN | 0.095292 | 0.243452 | 0.289037 | 0.001460 | 0.001700 | 0.001680 |
| pmr_deque_test.test | RUN | 0.103901 | 0.256447 | 0.301205 | 0.001704 | 0.001788 | 0.001732 |
| pair_test.test | RUN | 0.260358 | 0.571505 | 0.631056 | 0.001796 | 0.001724 | 0.001552 |
| null_iterators_test.test | RUN | 0.365403 | 1.251829 | 1.252748 | 0.001396 | 0.001556 | 0.001780 |
| node_handle_test.test | RUN | 0.149870 | 0.386703 | 0.512659 | 0.001729 | 0.001824 | 0.001882 |
| node_allocator_test.test | RUN | 0.634513 | 5.624099 | 3.482463 | 0.057147 | 0.062284 | 0.072291 |
| new_delete_resource_test.test | RUN | 0.209629 | 0.491315 | 0.613046 | 0.002382 | 0.002492 | 0.002570 |
| nest_test.test | RUN | 0.315305 | 1.602201 | 1.337184 | 0.001943 | 0.002132 | 0.001976 |
| nest_api_test.test | RUN | 0.434835 | 2.719737 | 1.832819 | 0.005140 | 0.005717 | 0.005477 |
| monotonic_buffer_resource_test.test | RUN | 0.187095 | 0.459450 | 0.592669 | 0.001592 | 0.001802 | 0.001635 |
| memory_resource_test.test | RUN | 0.133854 | 0.259186 | 0.404983 | 0.001602 | 0.001719 | 0.001598 |
| map_test.test | RUN | 1.265755 | 11.047879 | 8.256725 | 0.070858 | 0.085597 | 0.082683 |
| list_test.test | RUN | 0.802787 | 6.059267 | 4.417773 | 0.137005 | 0.157150 | 0.158281 |
| is_constructible_test.test | RUN | 0.089722 | 0.146366 | 0.188134 | 0.000746 | 0.000773 | 0.001528 |
| insert_vs_emplace_test.test | RUN | 0.211598 | 0.568755 | 0.541583 | 0.001898 | 0.001856 | 0.001720 |
| hub_uses_allocator_test.test | RUN | 0.265301 | 0.693576 | 0.683172 | 0.001581 | 0.001705 | 0.002002 |
| hub_stability_test.test | RUN | 0.361970 | 2.147412 | 1.697643 | 0.004030 | 0.004195 | 0.004252 |
| hub_sort_test.test | RUN | 0.240292 | 0.937690 | 0.980894 | 3.081494 | 3.116683 | 3.385439 |
| hub_pocxx_test.test | RUN | 0.267702 | 0.843647 | 0.817237 | 0.001865 | 0.002116 | 0.001866 |
| hub_new_extended_alignment_test.test | RUN | 0.197913 | 0.450146 | 0.565336 | 0.001627 | 0.001945 | 0.001773 |
| hub_exception_safety_test.test | RUN | 0.378254 | 2.099578 | 1.391643 | 0.186423 | 0.221797 | 0.256839 |
| hub_empty_sort_test.test | RUN | 0.211790 | 0.489573 | 0.630308 | 0.001622 | 0.001819 | 0.001613 |
| hub_api_test.test | RUN | 1.003559 | 8.550054 | 5.498095 | 0.013964 | 0.013612 | 0.016251 |
| global_resource_test.test | RUN | 0.137289 | 0.266211 | 0.343331 | 0.001779 | 0.001842 | 0.001854 |
| flat_tree_test.test | RUN | 1.023039 | 8.932535 | 5.798316 | 0.001698 | 0.001740 | 0.001499 |
| flat_set_test.test | RUN | 1.148919 | 10.508577 | 6.786470 | 0.024157 | 0.027401 | 0.027216 |
| flat_set_adaptor_test.test | RUN | 0.813091 | 7.296461 | 4.616593 | 0.025271 | 0.033012 | 0.029475 |
| flat_map_test.test | RUN | 1.680640 | 17.100050 | 10.762984 | 0.033318 | 0.039589 | 0.039689 |
| flat_map_adaptor_test.test | RUN | 1.104203 | 10.480002 | 6.868537 | 0.033762 | 0.041205 | 0.040933 |
| explicit_inst_vector_test.test | RUN | 0.426490 | 2.338842 | 1.969395 | 0.001924 | 0.001640 | 0.001729 |
| explicit_inst_string_test.test | RUN | 0.468504 | 2.575196 | 2.171335 | 0.001693 | 0.001761 | 0.001513 |
| explicit_inst_static_vector_test.test | RUN | 0.110699 | 0.315654 | 0.401279 | 0.001516 | 0.001885 | 0.001783 |
| explicit_inst_stable_vector_test.test | RUN | 0.415259 | 2.073011 | 1.702965 | 0.001620 | 0.001822 | 0.001796 |
| explicit_inst_small_vector_test.test | RUN | 0.373970 | 1.795417 | 1.617221 | 0.001517 | 0.001658 | 0.001660 |
| explicit_inst_slist_test.test | RUN | 0.352130 | 1.464999 | 1.345034 | 0.001735 | 0.001767 | 0.001718 |
| explicit_inst_set_test.test | RUN | 0.451283 | 1.978969 | 1.701344 | 0.001801 | 0.001833 | 0.001721 |
| explicit_inst_map_test.test | RUN | 0.487563 | 2.112682 | 1.798935 | 0.001800 | 0.001677 | 0.001730 |
| explicit_inst_list_test.test | RUN | 0.129030 | 0.487312 | 0.505143 | 0.001782 | 0.001697 | 0.001674 |
| explicit_inst_flat_set_test.test | RUN | 0.806044 | 6.284871 | 3.905350 | 0.002066 | 0.001771 | 0.001744 |
| explicit_inst_flat_map_test.test | RUN | 0.915147 | 7.269014 | 4.731737 | 0.001748 | 0.001850 | 0.001827 |
| explicit_inst_devector_test.test | RUN | 0.254109 | 1.119807 | 1.003855 | 0.001599 | 0.001879 | 0.001878 |
| explicit_inst_deque_test.test | RUN | 0.332445 | 1.427756 | 1.310632 | 0.001573 | 0.001857 | 0.001694 |
| devector_test.test | RUN | 1.427011 | 12.410383 | 8.678597 | 0.020125 | 0.015515 | 0.017280 |
| devector_options_test.test | RUN | 0.244443 | 0.891687 | 0.822586 | 0.003829 | 0.004470 | 0.004203 |
| deque_test.test | RUN | 1.140382 | 8.371746 | 6.567030 | 7.206791 | 8.512717 | 8.363522 |
| deque_options_test.test | RUN | 0.231145 | 0.599802 | 0.754706 | 0.001812 | 0.001918 | 0.001954 |
| copy_move_algo_test.test | RUN | 0.205174 | 0.478856 | 0.649640 | 0.003442 | 0.003738 | 0.003558 |
| common_iterator_test.test | RUN | 0.021767 | 0.033866 | 0.049356 | 0.000716 | 0.000757 | 0.001748 |
| boost_iterator_comp_test.test | RUN | 0.324191 | 0.905328 | 1.045671 | 0.001671 | 0.001768 | 0.001832 |
| allocator_traits_test.test | RUN | 0.212843 | 0.449818 | 0.627838 | 0.001729 | 0.001880 | 0.001805 |
| alloc_full_test.test | RUN | 0.169465 | 0.561348 | 0.582827 | 0.008605 | 0.008114 | 0.009167 |
| alloc_basic_test.test | RUN | 0.157110 | 0.508575 | 0.540146 | 0.002001 | 0.002404 | 0.002410 |
| adaptive_pool_test.test | RUN | 0.497827 | 5.172581 | 2.534483 | 0.112804 | 0.178664 | 0.133300 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
