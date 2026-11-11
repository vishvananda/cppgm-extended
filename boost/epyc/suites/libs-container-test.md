# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 1.370676 | 12.291045 | 8.665719 | 0.027295 | 0.019873 | 0.031453 |
| vector_options_test.test | RUN | 0.270659 | 0.939299 | 0.931360 | 0.002145 | 0.002274 | 0.002302 |
| uses_allocator_test.test | RUN | 0.066206 | 0.111458 | 0.171832 | 0.000811 | 0.000817 | 0.001882 |
| uses_allocator_construction_test.test | RUN | 0.176036 | 0.433298 | 0.587329 | 0.001874 | 0.001847 | 0.001996 |
| unsynchronized_pool_resource_test.test | RUN | 0.211801 | 0.483274 | 0.621727 | 0.002060 | 0.001584 | 0.002260 |
| tree_test.test | RUN | 0.595726 | 3.077498 | 2.824153 | 0.001746 | 0.002014 | 0.001852 |
| throw_exception_test.test | RUN | 0.132735 | 0.237455 | 0.367590 | 0.001832 | 0.001894 | 0.002064 |
| synchronized_pool_resource_test.test | RUN | 0.214317 | 0.492011 | 0.611610 | 0.002080 | 0.001898 | 0.002138 |
| string_view_compat_test.test | RUN | 0.336944 | 0.777091 | 1.024319 | 0.001600 | 0.001918 | 0.002238 |
| string_test.test | RUN | 0.653767 | 3.545832 | 3.187548 | 0.122414 | 0.147240 | 0.149518 |
| string_options_test.test | RUN | 0.241699 | 0.754653 | 0.859192 | 0.002336 | 0.002554 | 0.002634 |
| static_vector_test.test | RUN | 0.653485 | 4.810285 | 3.679197 | 0.002992 | 0.002906 | 0.003270 |
| static_vector_options_test.test | RUN | 0.223561 | 0.555307 | 0.689626 | 0.001881 | 0.001979 | 0.002190 |
| stable_vector_test.test | RUN | 1.026795 | 8.502971 | 5.761770 | 0.199120 | 0.238876 | 0.273031 |
| small_vector_test.test | RUN | 0.787666 | 5.843626 | 4.371063 | 0.018978 | 0.017336 | 0.019652 |
| small_vector_options_test.test | RUN | 0.321160 | 1.337465 | 1.183904 | 0.002134 | 0.002286 | 0.002380 |
| slist_test.test | RUN | 0.769936 | 5.974324 | 4.498798 | 0.159399 | 0.172099 | 0.175983 |
| set_test.test | RUN | 1.172923 | 9.709166 | 7.185339 | 0.080947 | 0.118711 | 0.097525 |
| segtor_test.test | RUN | 1.175451 | 9.279679 | 6.451605 | 6.940060 | 7.963273 | 8.465217 |
| segtor_options_test.test | RUN | 0.242217 | 0.625991 | 0.762331 | 0.001699 | 0.001972 | 0.002102 |
| scoped_allocator_usage_test.test | RUN | 0.543996 | 2.921190 | 2.393816 | 0.001683 | 0.002044 | 0.001844 |
| scoped_allocator_adaptor_test.test | RUN | 0.387285 | 1.371010 | 1.531068 | 0.001590 | 0.001968 | 0.002030 |
| resource_adaptor_test.test | RUN | 0.194949 | 0.481419 | 0.634373 | 0.001544 | 0.001890 | 0.002034 |
| polymorphic_allocator_test.test | RUN | 0.174364 | 0.388582 | 0.537502 | 0.001504 | 0.001954 | 0.001946 |
| pmr_vector_test.test | RUN | 0.104759 | 0.178340 | 0.249810 | 0.000646 | 0.000763 | 0.001627 |
| pmr_string_test.test | RUN | 0.190213 | 0.380393 | 0.507773 | 0.000588 | 0.000828 | 0.001712 |
| pmr_stable_vector_test.test | RUN | 0.168932 | 0.595301 | 0.536441 | 0.001537 | 0.001830 | 0.001743 |
| pmr_small_vector_test.test | RUN | 0.118079 | 0.273747 | 0.341083 | 0.001776 | 0.001950 | 0.001947 |
| pmr_slist_test.test | RUN | 0.131550 | 0.523446 | 0.483343 | 0.001694 | 0.001768 | 0.001671 |
| pmr_set_test.test | RUN | 0.159918 | 0.390612 | 0.463338 | 0.001546 | 0.001584 | 0.001667 |
| pmr_nest_test.test | RUN | 0.152805 | 0.282533 | 0.357248 | 0.001612 | 0.001806 | 0.001902 |
| pmr_map_test.test | RUN | 0.167086 | 0.472623 | 0.496686 | 0.001582 | 0.001925 | 0.001805 |
| pmr_list_test.test | RUN | 0.133873 | 0.286603 | 0.381144 | 0.001658 | 0.001834 | 0.002019 |
| pmr_hub_test.test | RUN | 0.189259 | 0.350792 | 0.476059 | 0.001552 | 0.001759 | 0.001974 |
| pmr_flat_set_test.test | RUN | 0.134074 | 0.296400 | 0.350143 | 0.001706 | 0.001800 | 0.001718 |
| pmr_flat_map_test.test | RUN | 0.152711 | 0.347041 | 0.456761 | 0.001525 | 0.001778 | 0.001727 |
| pmr_devector_test.test | RUN | 0.103458 | 0.242555 | 0.324065 | 0.001491 | 0.001794 | 0.001886 |
| pmr_deque_test.test | RUN | 0.107546 | 0.244714 | 0.287946 | 0.001854 | 0.001948 | 0.001753 |
| pair_test.test | RUN | 0.269585 | 0.609586 | 0.849217 | 0.001874 | 0.001891 | 0.001974 |
| null_iterators_test.test | RUN | 0.371292 | 1.189528 | 1.275534 | 0.001727 | 0.002000 | 0.001806 |
| node_handle_test.test | RUN | 0.160053 | 0.389495 | 0.414880 | 0.001802 | 0.001732 | 0.001769 |
| node_allocator_test.test | RUN | 0.732539 | 5.649203 | 4.173749 | 0.054500 | 0.077491 | 0.066301 |
| new_delete_resource_test.test | RUN | 0.199999 | 0.530723 | 0.684678 | 0.002434 | 0.002631 | 0.002712 |
| nest_test.test | RUN | 0.348929 | 1.617467 | 1.247274 | 0.002110 | 0.002148 | 0.002131 |
| nest_api_test.test | RUN | 0.449349 | 2.735127 | 2.289517 | 0.005438 | 0.005752 | 0.006102 |
| monotonic_buffer_resource_test.test | RUN | 0.194556 | 0.461136 | 0.525778 | 0.001586 | 0.001856 | 0.001844 |
| memory_resource_test.test | RUN | 0.142011 | 0.259356 | 0.336698 | 0.005115 | 0.001802 | 0.001766 |
| map_test.test | RUN | 1.311996 | 11.742212 | 8.464384 | 0.073852 | 0.087019 | 0.081607 |
| list_test.test | RUN | 0.838509 | 6.217223 | 4.493641 | 0.137279 | 0.178198 | 0.170597 |
| is_constructible_test.test | RUN | 0.094447 | 0.146234 | 0.269800 | 0.000820 | 0.000791 | 0.001823 |
| insert_vs_emplace_test.test | RUN | 0.214684 | 0.597753 | 0.736911 | 0.001866 | 0.002083 | 0.002039 |
| hub_uses_allocator_test.test | RUN | 0.263038 | 0.783829 | 0.917186 | 0.001808 | 0.002010 | 0.001949 |
| hub_stability_test.test | RUN | 0.401275 | 2.192982 | 1.912001 | 0.003852 | 0.004530 | 0.004264 |
| hub_sort_test.test | RUN | 0.276788 | 0.984575 | 0.977906 | 2.975221 | 3.103824 | 3.437894 |
| hub_pocxx_test.test | RUN | 0.253144 | 0.881317 | 0.943373 | 0.001966 | 0.002278 | 0.002281 |
| hub_new_extended_alignment_test.test | RUN | 0.187010 | 0.419193 | 0.471551 | 0.001707 | 0.001740 | 0.001794 |
| hub_exception_safety_test.test | RUN | 0.349134 | 2.223880 | 1.446903 | 0.187979 | 0.219926 | 0.264617 |
| hub_empty_sort_test.test | RUN | 0.199227 | 0.465604 | 0.645782 | 0.001770 | 0.001900 | 0.002008 |
| hub_api_test.test | RUN | 0.956376 | 8.794138 | 5.842344 | 0.014912 | 0.014333 | 0.016574 |
| global_resource_test.test | RUN | 0.126290 | 0.307468 | 0.344789 | 0.001766 | 0.002002 | 0.002060 |
| flat_tree_test.test | RUN | 1.086881 | 9.500391 | 6.023878 | 0.001801 | 0.001955 | 0.001635 |
| flat_set_test.test | RUN | 1.172468 | 11.093688 | 7.088422 | 0.023492 | 0.025268 | 0.028005 |
| flat_set_adaptor_test.test | RUN | 0.794845 | 7.518859 | 4.736734 | 0.027112 | 0.032958 | 0.032754 |
| flat_map_test.test | RUN | 1.736289 | 17.554054 | 11.206655 | 0.033785 | 0.040898 | 0.039367 |
| flat_map_adaptor_test.test | RUN | 1.131645 | 10.713434 | 7.081788 | 0.034106 | 0.042060 | 0.041710 |
| explicit_inst_vector_test.test | RUN | 0.458807 | 2.387945 | 2.080625 | 0.001826 | 0.001741 | 0.001969 |
| explicit_inst_string_test.test | RUN | 0.511310 | 2.660304 | 2.245981 | 0.001835 | 0.001897 | 0.001950 |
| explicit_inst_static_vector_test.test | RUN | 0.116586 | 0.322620 | 0.419015 | 0.001797 | 0.001895 | 0.001851 |
| explicit_inst_stable_vector_test.test | RUN | 0.441145 | 2.134228 | 1.768988 | 0.001786 | 0.001546 | 0.001912 |
| explicit_inst_small_vector_test.test | RUN | 0.403149 | 1.845569 | 1.673885 | 0.001820 | 0.001810 | 0.001790 |
| explicit_inst_slist_test.test | RUN | 0.347692 | 1.514614 | 1.352218 | 0.001676 | 0.001904 | 0.001684 |
| explicit_inst_set_test.test | RUN | 0.474487 | 1.975971 | 1.888449 | 0.001742 | 0.001814 | 0.001852 |
| explicit_inst_map_test.test | RUN | 0.489733 | 2.216871 | 1.937837 | 0.001901 | 0.001936 | 0.001958 |
| explicit_inst_list_test.test | RUN | 0.142855 | 0.477879 | 0.549932 | 0.001713 | 0.001700 | 0.001759 |
| explicit_inst_flat_set_test.test | RUN | 0.828547 | 6.549751 | 4.213048 | 0.001583 | 0.001843 | 0.001970 |
| explicit_inst_flat_map_test.test | RUN | 0.956166 | 6.973097 | 5.031773 | 0.001693 | 0.001824 | 0.001952 |
| explicit_inst_devector_test.test | RUN | 0.255301 | 1.086622 | 0.884137 | 0.001880 | 0.001776 | 0.001908 |
| explicit_inst_deque_test.test | RUN | 0.341641 | 1.451564 | 1.366759 | 0.001726 | 0.001751 | 0.001819 |
| devector_test.test | RUN | 1.443885 | 12.491276 | 8.223672 | 0.020476 | 0.015346 | 0.017613 |
| devector_options_test.test | RUN | 0.254161 | 0.887608 | 0.961978 | 0.003641 | 0.004533 | 0.004352 |
| deque_test.test | RUN | 1.164553 | 8.279555 | 6.872383 | 7.261468 | 8.424485 | 8.148287 |
| deque_options_test.test | RUN | 0.244463 | 0.678361 | 0.774457 | 0.001748 | 0.002121 | 0.002116 |
| copy_move_algo_test.test | RUN | 0.212559 | 0.483409 | 0.653192 | 0.003537 | 0.003841 | 0.003843 |
| common_iterator_test.test | RUN | 0.021495 | 0.037014 | 0.052105 | 0.000797 | 0.000854 | 0.001936 |
| boost_iterator_comp_test.test | RUN | 0.333878 | 0.963620 | 1.066981 | 0.001847 | 0.001902 | 0.002044 |
| allocator_traits_test.test | RUN | 0.210806 | 0.457435 | 0.648284 | 0.001354 | 0.001881 | 0.002006 |
| alloc_full_test.test | RUN | 0.172794 | 0.571669 | 0.634634 | 0.009024 | 0.008575 | 0.008526 |
| alloc_basic_test.test | RUN | 0.155359 | 0.499518 | 0.543401 | 0.002227 | 0.002422 | 0.002599 |
| adaptive_pool_test.test | RUN | 0.580232 | 5.453743 | 3.720705 | 0.112252 | 0.115559 | 0.141526 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
