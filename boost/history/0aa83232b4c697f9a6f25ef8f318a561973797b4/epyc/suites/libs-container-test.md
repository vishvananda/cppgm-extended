# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 1.087290 | 10.614839 | 6.534432 | 0.020576 | 0.021225 | 0.022462 |
| vector_options_test.test | RUN | 0.211219 | 0.794766 | 0.668997 | 0.001911 | 0.002161 | 0.002208 |
| uses_allocator_test.test | RUN | 0.050400 | 0.088403 | 0.121316 | 0.000834 | 0.000854 | 0.001738 |
| uses_allocator_construction_test.test | RUN | 0.141648 | 0.352772 | 0.416557 | 0.001728 | 0.001954 | 0.001646 |
| unsynchronized_pool_resource_test.test | RUN | 0.167119 | 0.396312 | 0.483177 | 0.001840 | 0.001955 | 0.002002 |
| tree_test.test | RUN | 0.466588 | 2.624115 | 1.988368 | 0.001764 | 0.001843 | 0.001805 |
| throw_exception_test.test | RUN | 0.102474 | 0.198592 | 0.282384 | 0.001732 | 0.001822 | 0.001907 |
| synchronized_pool_resource_test.test | RUN | 0.166717 | 0.392357 | 0.468919 | 0.001719 | 0.001998 | 0.002016 |
| string_view_compat_test.test | RUN | 0.261195 | 0.625040 | 0.737643 | 0.001281 | 0.001987 | 0.002032 |
| string_test.test | RUN | 0.525125 | 3.045876 | 2.276480 | 0.093275 | 0.103221 | 0.100977 |
| string_options_test.test | RUN | 0.188131 | 0.608622 | 0.600232 | 0.002236 | 0.002240 | 0.002392 |
| static_vector_test.test | RUN | 0.502120 | 4.176007 | 2.821152 | 0.002841 | 0.002866 | 0.002772 |
| static_vector_options_test.test | RUN | 0.173753 | 0.493660 | 0.495896 | 0.001851 | 0.002008 | 0.002134 |
| stable_vector_test.test | RUN | 0.799113 | 7.442212 | 4.422357 | 0.182438 | 0.171778 | 0.163021 |
| small_vector_test.test | RUN | 0.621824 | 5.051574 | 3.278911 | 0.013191 | 0.012348 | 0.013989 |
| small_vector_options_test.test | RUN | 0.252591 | 1.085627 | 0.849082 | 0.002050 | 0.002205 | 0.002020 |
| slist_test.test | RUN | 0.617556 | 5.164047 | 3.381823 | 0.118706 | 0.127768 | 0.120640 |
| set_test.test | RUN | 1.018106 | 8.008903 | 5.274052 | 0.061969 | 0.071475 | 0.073893 |
| segtor_test.test | RUN | 0.984716 | 7.816074 | 4.898230 | 5.784350 | 6.112498 | 6.566516 |
| segtor_options_test.test | RUN | 0.176631 | 0.500201 | 0.548802 | 0.001870 | 0.001833 | 0.001996 |
| scoped_allocator_usage_test.test | RUN | 0.440552 | 2.350830 | 1.849692 | 0.001429 | 0.002038 | 0.002108 |
| scoped_allocator_adaptor_test.test | RUN | 0.306393 | 1.068880 | 1.151978 | 0.001658 | 0.001848 | 0.001866 |
| resource_adaptor_test.test | RUN | 0.153433 | 0.371013 | 0.482881 | 0.001399 | 0.001762 | 0.001908 |
| polymorphic_allocator_test.test | RUN | 0.126734 | 0.303214 | 0.364689 | 0.001255 | 0.001873 | 0.001632 |
| pmr_vector_test.test | RUN | 0.074809 | 0.138233 | 0.221467 | 0.000568 | 0.000772 | 0.001718 |
| pmr_string_test.test | RUN | 0.158174 | 0.296308 | 0.370382 | 0.000693 | 0.000767 | 0.001932 |
| pmr_stable_vector_test.test | RUN | 0.124613 | 0.465408 | 0.425738 | 0.001409 | 0.001356 | 0.001616 |
| pmr_small_vector_test.test | RUN | 0.090398 | 0.204790 | 0.245001 | 0.001451 | 0.001746 | 0.001643 |
| pmr_slist_test.test | RUN | 0.095459 | 0.397791 | 0.363839 | 0.001685 | 0.001608 | 0.001811 |
| pmr_set_test.test | RUN | 0.129690 | 0.309883 | 0.384647 | 0.001274 | 0.001682 | 0.001624 |
| pmr_nest_test.test | RUN | 0.108509 | 0.212540 | 0.235165 | 0.001548 | 0.001721 | 0.001361 |
| pmr_map_test.test | RUN | 0.137717 | 0.359072 | 0.396734 | 0.001460 | 0.001764 | 0.001576 |
| pmr_list_test.test | RUN | 0.094223 | 0.214233 | 0.265781 | 0.001338 | 0.001750 | 0.001497 |
| pmr_hub_test.test | RUN | 0.142067 | 0.280891 | 0.297270 | 0.001454 | 0.001445 | 0.001449 |
| pmr_flat_set_test.test | RUN | 0.102616 | 0.237618 | 0.263799 | 0.002136 | 0.001771 | 0.001606 |
| pmr_flat_map_test.test | RUN | 0.122369 | 0.276928 | 0.335064 | 0.001120 | 0.001533 | 0.001676 |
| pmr_devector_test.test | RUN | 0.074915 | 0.186019 | 0.230584 | 0.001296 | 0.001440 | 0.001737 |
| pmr_deque_test.test | RUN | 0.083596 | 0.200755 | 0.239604 | 0.001700 | 0.001445 | 0.001682 |
| pair_test.test | RUN | 0.199351 | 0.469631 | 0.542580 | 0.001512 | 0.001658 | 0.001810 |
| null_iterators_test.test | RUN | 0.318012 | 0.962748 | 0.835296 | 0.001746 | 0.001745 | 0.001599 |
| node_handle_test.test | RUN | 0.124112 | 0.301550 | 0.345934 | 0.001561 | 0.001661 | 0.001846 |
| node_allocator_test.test | RUN | 0.548491 | 4.494017 | 3.004391 | 0.043082 | 0.046202 | 0.045228 |
| new_delete_resource_test.test | RUN | 0.164234 | 0.419902 | 0.475253 | 0.002288 | 0.002283 | 0.002207 |
| nest_test.test | RUN | 0.255034 | 1.357924 | 0.861520 | 0.002116 | 0.002044 | 0.002074 |
| nest_api_test.test | RUN | 0.364566 | 2.253992 | 1.648158 | 0.004202 | 0.004144 | 0.004550 |
| monotonic_buffer_resource_test.test | RUN | 0.170969 | 0.391233 | 0.363794 | 0.001741 | 0.001616 | 0.001706 |
| memory_resource_test.test | RUN | 0.108626 | 0.206340 | 0.247132 | 0.001663 | 0.001483 | 0.001713 |
| map_test.test | RUN | 1.118803 | 9.849127 | 6.059989 | 0.056210 | 0.056733 | 0.059129 |
| list_test.test | RUN | 0.576459 | 5.146701 | 3.308634 | 0.106917 | 0.112672 | 0.113030 |
| is_constructible_test.test | RUN | 0.071194 | 0.107549 | 0.176508 | 0.000750 | 0.000781 | 0.001722 |
| insert_vs_emplace_test.test | RUN | 0.168238 | 0.432143 | 0.480784 | 0.001866 | 0.001787 | 0.002008 |
| hub_uses_allocator_test.test | RUN | 0.216776 | 0.618192 | 0.598037 | 0.001586 | 0.001797 | 0.001858 |
| hub_stability_test.test | RUN | 0.287808 | 1.755685 | 1.353497 | 0.002850 | 0.003478 | 0.003198 |
| hub_sort_test.test | RUN | 0.199189 | 0.816666 | 0.751835 | 2.287517 | 2.239555 | 2.217593 |
| hub_pocxx_test.test | RUN | 0.208565 | 0.728806 | 0.645922 | 0.001732 | 0.001727 | 0.001850 |
| hub_new_extended_alignment_test.test | RUN | 0.152085 | 0.334694 | 0.349273 | 0.001705 | 0.001672 | 0.001432 |
| hub_exception_safety_test.test | RUN | 0.297492 | 1.792063 | 1.095177 | 0.144527 | 0.155291 | 0.204940 |
| hub_empty_sort_test.test | RUN | 0.163609 | 0.369142 | 0.474364 | 0.001513 | 0.001797 | 0.001756 |
| hub_api_test.test | RUN | 0.797335 | 7.476350 | 4.269457 | 0.009937 | 0.009646 | 0.010940 |
| global_resource_test.test | RUN | 0.115822 | 0.199979 | 0.274373 | 0.001691 | 0.002911 | 0.001393 |
| flat_tree_test.test | RUN | 0.867050 | 7.768091 | 4.357653 | 0.001754 | 0.001838 | 0.001830 |
| flat_set_test.test | RUN | 0.889521 | 9.190597 | 5.199940 | 0.020088 | 0.021627 | 0.021788 |
| flat_set_adaptor_test.test | RUN | 0.669143 | 6.195424 | 3.368681 | 0.021311 | 0.023605 | 0.024004 |
| flat_map_test.test | RUN | 1.428851 | 14.961594 | 8.322871 | 0.027430 | 0.029718 | 0.031323 |
| flat_map_adaptor_test.test | RUN | 0.909977 | 9.019332 | 5.091884 | 0.029738 | 0.033328 | 0.030005 |
| explicit_inst_vector_test.test | RUN | 0.389098 | 1.991937 | 1.440997 | 0.001792 | 0.001736 | 0.001854 |
| explicit_inst_string_test.test | RUN | 0.430583 | 2.160527 | 1.576791 | 0.001661 | 0.001846 | 0.001798 |
| explicit_inst_static_vector_test.test | RUN | 0.091691 | 0.281875 | 0.284582 | 0.001715 | 0.001713 | 0.001709 |
| explicit_inst_stable_vector_test.test | RUN | 0.346540 | 1.697514 | 1.281138 | 0.001762 | 0.001489 | 0.001896 |
| explicit_inst_small_vector_test.test | RUN | 0.308353 | 1.512806 | 1.201530 | 0.001494 | 0.001366 | 0.001842 |
| explicit_inst_slist_test.test | RUN | 0.275741 | 1.241744 | 0.942328 | 0.001376 | 0.001828 | 0.001656 |
| explicit_inst_set_test.test | RUN | 0.368486 | 1.731962 | 1.373707 | 0.001717 | 0.002016 | 0.001640 |
| explicit_inst_map_test.test | RUN | 0.395297 | 1.950065 | 1.440903 | 0.001768 | 0.001833 | 0.001927 |
| explicit_inst_list_test.test | RUN | 0.105560 | 0.426258 | 0.375410 | 0.002112 | 0.001901 | 0.001834 |
| explicit_inst_flat_set_test.test | RUN | 0.648077 | 5.366401 | 3.176400 | 0.001603 | 0.001835 | 0.001772 |
| explicit_inst_flat_map_test.test | RUN | 0.740999 | 6.072077 | 3.675536 | 0.001629 | 0.001612 | 0.001552 |
| explicit_inst_devector_test.test | RUN | 0.199185 | 0.906817 | 0.660395 | 0.001748 | 0.001655 | 0.001716 |
| explicit_inst_deque_test.test | RUN | 0.272928 | 1.189821 | 0.918516 | 0.001786 | 0.001758 | 0.001752 |
| devector_test.test | RUN | 1.152003 | 10.503052 | 7.105461 | 0.016974 | 0.012224 | 0.017313 |
| devector_options_test.test | RUN | 0.201508 | 0.749589 | 0.707845 | 0.003317 | 0.003799 | 0.003792 |
| deque_test.test | RUN | 0.958482 | 7.710143 | 4.422680 | 5.671547 | 5.631816 | 5.915846 |
| deque_options_test.test | RUN | 0.185820 | 0.494686 | 0.490716 | 0.001897 | 0.001567 | 0.001939 |
| copy_move_algo_test.test | RUN | 0.166165 | 0.388474 | 0.421040 | 0.002806 | 0.003091 | 0.002484 |
| common_iterator_test.test | RUN | 0.017530 | 0.029094 | 0.039880 | 0.000735 | 0.000799 | 0.001575 |
| boost_iterator_comp_test.test | RUN | 0.273912 | 0.760771 | 0.742860 | 0.001731 | 0.001503 | 0.001940 |
| allocator_traits_test.test | RUN | 0.176786 | 0.368967 | 0.426362 | 0.001622 | 0.001572 | 0.001747 |
| alloc_full_test.test | RUN | 0.140779 | 0.456595 | 0.433334 | 0.006583 | 0.006735 | 0.006532 |
| alloc_basic_test.test | RUN | 0.135199 | 0.415748 | 0.399248 | 0.001936 | 0.002216 | 0.002224 |
| adaptive_pool_test.test | RUN | 0.461136 | 3.686061 | 3.109371 | 0.076294 | 0.074830 | 0.090023 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
