# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 1.152547 | 11.268997 | 6.171166 | 0.021057 | 0.021745 | 0.022165 |
| vector_options_test.test | RUN | 0.221935 | 0.881954 | 0.618519 | 0.002161 | 0.002210 | 0.002036 |
| uses_allocator_test.test | RUN | 0.053528 | 0.089758 | 0.114102 | 0.000695 | 0.000825 | 0.001630 |
| uses_allocator_construction_test.test | RUN | 0.145247 | 0.364220 | 0.389975 | 0.001678 | 0.002202 | 0.001678 |
| unsynchronized_pool_resource_test.test | RUN | 0.174342 | 0.450252 | 0.412107 | 0.002014 | 0.002139 | 0.001855 |
| tree_test.test | RUN | 0.476862 | 2.873544 | 1.896277 | 0.001597 | 0.002001 | 0.001869 |
| throw_exception_test.test | RUN | 0.111182 | 0.241089 | 0.258474 | 0.001710 | 0.001988 | 0.001734 |
| synchronized_pool_resource_test.test | RUN | 0.185056 | 0.428790 | 0.443205 | 0.001872 | 0.001870 | 0.001924 |
| string_view_compat_test.test | RUN | 0.272955 | 0.623121 | 0.697785 | 0.001705 | 0.001832 | 0.001728 |
| string_test.test | RUN | 0.554631 | 3.203050 | 2.177320 | 0.095007 | 0.105939 | 0.098124 |
| string_options_test.test | RUN | 0.195078 | 0.662133 | 0.536182 | 0.002360 | 0.002426 | 0.002301 |
| static_vector_test.test | RUN | 0.543835 | 4.297810 | 2.619742 | 0.002887 | 0.003159 | 0.002600 |
| static_vector_options_test.test | RUN | 0.187346 | 0.513633 | 0.458797 | 0.001847 | 0.002014 | 0.001930 |
| stable_vector_test.test | RUN | 0.845327 | 7.813562 | 4.114795 | 0.165438 | 0.179814 | 0.164439 |
| small_vector_test.test | RUN | 0.679537 | 5.190814 | 3.074896 | 0.013141 | 0.013516 | 0.013453 |
| small_vector_options_test.test | RUN | 0.269721 | 1.256892 | 0.773520 | 0.002112 | 0.002198 | 0.001880 |
| slist_test.test | RUN | 0.650372 | 5.290929 | 3.175236 | 0.116726 | 0.127535 | 0.118919 |
| set_test.test | RUN | 1.031212 | 8.746744 | 4.884140 | 0.064984 | 0.071095 | 0.069300 |
| segtor_test.test | RUN | 0.989831 | 8.417682 | 4.651119 | 5.860402 | 6.218550 | 6.451273 |
| segtor_options_test.test | RUN | 0.182620 | 0.529198 | 0.500142 | 0.001916 | 0.002076 | 0.001810 |
| scoped_allocator_usage_test.test | RUN | 0.475250 | 2.625149 | 1.706349 | 0.001687 | 0.001976 | 0.001978 |
| scoped_allocator_adaptor_test.test | RUN | 0.337992 | 1.242211 | 1.085078 | 0.001818 | 0.001824 | 0.001874 |
| resource_adaptor_test.test | RUN | 0.172143 | 0.417879 | 0.435564 | 0.001669 | 0.001764 | 0.001661 |
| polymorphic_allocator_test.test | RUN | 0.144563 | 0.348011 | 0.396790 | 0.001610 | 0.001936 | 0.001909 |
| pmr_vector_test.test | RUN | 0.085393 | 0.149163 | 0.188911 | 0.000833 | 0.000768 | 0.001774 |
| pmr_string_test.test | RUN | 0.209620 | 0.320465 | 0.376166 | 0.000756 | 0.000828 | 0.001664 |
| pmr_stable_vector_test.test | RUN | 0.158229 | 0.507525 | 0.412463 | 0.001429 | 0.001885 | 0.001388 |
| pmr_small_vector_test.test | RUN | 0.125475 | 0.223645 | 0.228254 | 0.001538 | 0.001894 | 0.001561 |
| pmr_slist_test.test | RUN | 0.108413 | 0.406485 | 0.357390 | 0.001898 | 0.001797 | 0.001679 |
| pmr_set_test.test | RUN | 0.136983 | 0.352063 | 0.369225 | 0.001749 | 0.001699 | 0.001733 |
| pmr_nest_test.test | RUN | 0.123939 | 0.224054 | 0.226220 | 0.001550 | 0.001858 | 0.001404 |
| pmr_map_test.test | RUN | 0.151703 | 0.390113 | 0.389890 | 0.001702 | 0.002082 | 0.001464 |
| pmr_list_test.test | RUN | 0.101963 | 0.236333 | 0.235260 | 0.001612 | 0.001905 | 0.001536 |
| pmr_hub_test.test | RUN | 0.158808 | 0.301112 | 0.324981 | 0.001653 | 0.001704 | 0.001802 |
| pmr_flat_set_test.test | RUN | 0.105430 | 0.257759 | 0.246276 | 0.001751 | 0.001966 | 0.001655 |
| pmr_flat_map_test.test | RUN | 0.137819 | 0.288155 | 0.316508 | 0.001445 | 0.001429 | 0.001552 |
| pmr_devector_test.test | RUN | 0.086649 | 0.204793 | 0.220740 | 0.001486 | 0.001657 | 0.001414 |
| pmr_deque_test.test | RUN | 0.086578 | 0.207644 | 0.225319 | 0.001855 | 0.001826 | 0.001779 |
| pair_test.test | RUN | 0.212900 | 0.478298 | 0.554061 | 0.001756 | 0.001691 | 0.001750 |
| null_iterators_test.test | RUN | 0.343565 | 1.067655 | 0.797808 | 0.001634 | 0.001563 | 0.001636 |
| node_handle_test.test | RUN | 0.142311 | 0.331088 | 0.343490 | 0.001570 | 0.001756 | 0.001670 |
| node_allocator_test.test | RUN | 0.605071 | 4.940713 | 2.920729 | 0.044241 | 0.045844 | 0.043826 |
| new_delete_resource_test.test | RUN | 0.190026 | 0.410875 | 0.480540 | 0.002488 | 0.002316 | 0.002235 |
| nest_test.test | RUN | 0.281735 | 1.329803 | 0.962908 | 0.001917 | 0.002044 | 0.001842 |
| nest_api_test.test | RUN | 0.394163 | 2.311614 | 1.441681 | 0.004552 | 0.004728 | 0.004377 |
| monotonic_buffer_resource_test.test | RUN | 0.180128 | 0.385290 | 0.360983 | 0.001594 | 0.001758 | 0.001663 |
| memory_resource_test.test | RUN | 0.110519 | 0.213182 | 0.244125 | 0.001612 | 0.001562 | 0.001432 |
| map_test.test | RUN | 1.250415 | 10.065119 | 5.934999 | 0.057291 | 0.062452 | 0.055931 |
| list_test.test | RUN | 0.673853 | 5.452830 | 2.863572 | 0.105228 | 0.111996 | 0.106982 |
| is_constructible_test.test | RUN | 0.083024 | 0.122073 | 0.166181 | 0.000794 | 0.000818 | 0.001689 |
| insert_vs_emplace_test.test | RUN | 0.185096 | 0.486127 | 0.468782 | 0.001839 | 0.002142 | 0.001794 |
| hub_uses_allocator_test.test | RUN | 0.245587 | 0.679002 | 0.610413 | 0.001984 | 0.001930 | 0.001748 |
| hub_stability_test.test | RUN | 0.339762 | 1.797479 | 1.295685 | 0.003301 | 0.002949 | 0.003241 |
| hub_sort_test.test | RUN | 0.222926 | 0.843049 | 0.665611 | 2.251233 | 2.297034 | 2.225781 |
| hub_pocxx_test.test | RUN | 0.222132 | 0.718466 | 0.617704 | 0.001944 | 0.002004 | 0.001792 |
| hub_new_extended_alignment_test.test | RUN | 0.165376 | 0.371981 | 0.352065 | 0.001450 | 0.002000 | 0.001329 |
| hub_exception_safety_test.test | RUN | 0.309850 | 1.811989 | 1.243589 | 0.145273 | 0.167010 | 0.228261 |
| hub_empty_sort_test.test | RUN | 0.171268 | 0.422701 | 0.348173 | 0.001984 | 0.001901 | 0.001461 |
| hub_api_test.test | RUN | 0.828089 | 7.665418 | 4.079114 | 0.010142 | 0.009900 | 0.010386 |
| global_resource_test.test | RUN | 0.114351 | 0.221810 | 0.258601 | 0.002060 | 0.001939 | 0.001448 |
| flat_tree_test.test | RUN | 0.904722 | 7.983278 | 4.309580 | 0.001850 | 0.001932 | 0.001727 |
| flat_set_test.test | RUN | 0.961217 | 9.503623 | 4.930932 | 0.020339 | 0.021605 | 0.021578 |
| flat_set_adaptor_test.test | RUN | 0.718531 | 6.411031 | 3.391252 | 0.022360 | 0.027890 | 0.023744 |
| flat_map_test.test | RUN | 1.493777 | 15.412235 | 7.754015 | 0.025886 | 0.029348 | 0.028262 |
| flat_map_adaptor_test.test | RUN | 0.930263 | 9.246718 | 4.818263 | 0.026884 | 0.031572 | 0.029083 |
| explicit_inst_vector_test.test | RUN | 0.384150 | 1.999887 | 1.257190 | 0.001802 | 0.001866 | 0.001584 |
| explicit_inst_string_test.test | RUN | 0.408129 | 2.263417 | 1.491625 | 0.001450 | 0.004803 | 0.001596 |
| explicit_inst_static_vector_test.test | RUN | 0.097223 | 0.279792 | 0.276992 | 0.001550 | 0.001880 | 0.001626 |
| explicit_inst_stable_vector_test.test | RUN | 0.365787 | 1.802110 | 1.203605 | 0.001480 | 0.001860 | 0.001711 |
| explicit_inst_small_vector_test.test | RUN | 0.338134 | 1.597700 | 1.121264 | 0.001206 | 0.001779 | 0.001645 |
| explicit_inst_slist_test.test | RUN | 0.308050 | 1.333164 | 0.923935 | 0.001486 | 0.001773 | 0.001766 |
| explicit_inst_set_test.test | RUN | 0.409169 | 1.712567 | 1.246153 | 0.001942 | 0.001783 | 0.001444 |
| explicit_inst_map_test.test | RUN | 0.433116 | 1.949464 | 1.366470 | 0.001660 | 0.001838 | 0.001734 |
| explicit_inst_list_test.test | RUN | 0.110432 | 0.389150 | 0.342940 | 0.001878 | 0.001734 | 0.001612 |
| explicit_inst_flat_set_test.test | RUN | 0.690761 | 5.593399 | 2.987282 | 0.001836 | 0.001804 | 0.001571 |
| explicit_inst_flat_map_test.test | RUN | 0.810046 | 6.197687 | 3.325804 | 0.001894 | 0.001872 | 0.001644 |
| explicit_inst_devector_test.test | RUN | 0.212918 | 0.979372 | 0.724857 | 0.001942 | 0.001913 | 0.001601 |
| explicit_inst_deque_test.test | RUN | 0.304041 | 1.111752 | 0.923133 | 0.001798 | 0.001785 | 0.001661 |
| devector_test.test | RUN | 1.266505 | 10.806393 | 5.988056 | 0.016611 | 0.011850 | 0.018069 |
| devector_options_test.test | RUN | 0.221450 | 0.763700 | 0.578391 | 0.003527 | 0.003972 | 0.003516 |
| deque_test.test | RUN | 1.040430 | 7.667640 | 4.956085 | 5.705227 | 5.678628 | 5.884020 |
| deque_options_test.test | RUN | 0.212497 | 0.531409 | 0.511060 | 0.002104 | 0.001881 | 0.001991 |
| copy_move_algo_test.test | RUN | 0.183738 | 0.398744 | 0.440626 | 0.003193 | 0.003075 | 0.002958 |
| common_iterator_test.test | RUN | 0.018332 | 0.029598 | 0.042672 | 0.000829 | 0.000545 | 0.001692 |
| boost_iterator_comp_test.test | RUN | 0.287033 | 0.789103 | 0.732508 | 0.002036 | 0.001890 | 0.001763 |
| allocator_traits_test.test | RUN | 0.193073 | 0.374204 | 0.429660 | 0.001886 | 0.001906 | 0.001763 |
| alloc_full_test.test | RUN | 0.161899 | 0.467561 | 0.421684 | 0.006784 | 0.006506 | 0.006195 |
| alloc_basic_test.test | RUN | 0.147994 | 0.430337 | 0.395070 | 0.002133 | 0.002369 | 0.002360 |
| adaptive_pool_test.test | RUN | 0.528233 | 4.645679 | 2.730430 | 0.077502 | 0.095219 | 0.075136 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
