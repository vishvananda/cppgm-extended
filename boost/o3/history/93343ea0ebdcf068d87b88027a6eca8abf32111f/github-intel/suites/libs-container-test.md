# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 7.870016 | 41.912376 | 34.185883 | 0.007464 | 0.006903 | 0.009426 |
| vector_options_test.test | RUN | 0.404010 | 1.842400 | 1.463136 | 0.001835 | 0.001978 | 0.001923 |
| uses_allocator_test.test | RUN | 0.075871 | 0.151186 | 0.196037 | 0.000732 | 0.000729 | 0.001597 |
| uses_allocator_construction_test.test | RUN | 0.220716 | 0.582633 | 0.648515 | 0.001658 | 0.001631 | 0.001680 |
| unsynchronized_pool_resource_test.test | RUN | 0.333570 | 1.763107 | 0.863197 | 0.001701 | 0.001893 | 0.001799 |
| tree_test.test | RUN | 1.148828 | 6.224233 | 4.806165 | 0.001834 | 0.001676 | 0.001666 |
| throw_exception_test.test | RUN | 0.158282 | 0.352639 | 0.454282 | 0.001666 | 0.001765 | 0.001626 |
| synchronized_pool_resource_test.test | RUN | 0.339331 | 1.751825 | 0.872069 | 0.001804 | 0.001884 | 0.001751 |
| string_view_compat_test.test | RUN | 0.528307 | 3.478311 | 1.702468 | 0.001766 | 0.001768 | 0.001664 |
| string_test.test | RUN | 2.298651 | 13.213188 | 9.249320 | 0.020210 | 0.019026 | 0.018277 |
| string_options_test.test | RUN | 0.368277 | 2.861190 | 1.382550 | 0.001832 | 0.001825 | 0.001696 |
| static_vector_test.test | RUN | 2.636988 | 10.954600 | 9.213985 | 0.002653 | 0.002264 | 0.002274 |
| static_vector_options_test.test | RUN | 0.281260 | 0.748606 | 0.848390 | 0.001850 | 0.001945 | 0.001819 |
| stable_vector_test.test | RUN | 5.726326 | 22.809574 | 17.522641 | 0.037167 | 0.037394 | 0.039720 |
| small_vector_test.test | RUN | 3.120850 | 16.104792 | 12.703731 | 0.004625 | 0.003439 | 0.007032 |
| small_vector_options_test.test | RUN | 0.474121 | 1.951615 | 1.702373 | 0.001925 | 0.001896 | 0.001796 |
| slist_test.test | RUN | 2.465136 | 14.493097 | 11.404839 | 0.033874 | 0.032292 | 0.032086 |
| set_test.test | RUN | 4.234963 | 26.953692 | 21.031051 | 0.017822 | 0.015842 | 0.016760 |
| segtor_test.test | RUN | 6.261936 | 29.400012 | 23.020801 | 0.728030 | 0.703920 | 0.767793 |
| segtor_options_test.test | RUN | 0.319919 | 0.815355 | 1.022923 | 0.001886 | 0.001918 | 0.001814 |
| scoped_allocator_usage_test.test | RUN | 1.078404 | 4.320275 | 3.913559 | 0.001706 | 0.001720 | 0.001699 |
| scoped_allocator_adaptor_test.test | RUN | 0.516115 | 1.236578 | 1.750116 | 0.001757 | 0.001796 | 0.001768 |
| resource_adaptor_test.test | RUN | 0.269190 | 0.757287 | 0.758064 | 0.001718 | 0.001794 | 0.001755 |
| polymorphic_allocator_test.test | RUN | 0.235239 | 0.595865 | 0.628647 | 0.001596 | 0.001735 | 0.001376 |
| pmr_vector_test.test | RUN | 0.125557 | 0.232774 | 0.340027 | 0.000732 | 0.000768 | 0.001626 |
| pmr_string_test.test | RUN | 0.247008 | 0.486776 | 0.640896 | 0.000574 | 0.000760 | 0.001675 |
| pmr_stable_vector_test.test | RUN | 0.264222 | 0.768764 | 0.882579 | 0.001676 | 0.001666 | 0.001740 |
| pmr_small_vector_test.test | RUN | 0.146832 | 0.315244 | 0.406382 | 0.001780 | 0.001478 | 0.001697 |
| pmr_slist_test.test | RUN | 0.198809 | 1.015530 | 0.865466 | 0.001618 | 0.001608 | 0.001618 |
| pmr_set_test.test | RUN | 0.229870 | 0.529979 | 0.689452 | 0.001715 | 0.001664 | 0.001598 |
| pmr_nest_test.test | RUN | 0.182628 | 0.333315 | 0.449322 | 0.001870 | 0.001628 | 0.001792 |
| pmr_map_test.test | RUN | 0.236413 | 0.542699 | 0.650687 | 0.001699 | 0.001599 | 0.001688 |
| pmr_list_test.test | RUN | 0.156290 | 0.340065 | 0.458499 | 0.001538 | 0.001560 | 0.001643 |
| pmr_hub_test.test | RUN | 0.222911 | 0.474094 | 0.594351 | 0.001680 | 0.001660 | 0.001693 |
| pmr_flat_set_test.test | RUN | 0.175037 | 0.364502 | 0.516612 | 0.001404 | 0.001637 | 0.001699 |
| pmr_flat_map_test.test | RUN | 0.211369 | 0.426554 | 0.620460 | 0.001681 | 0.001708 | 0.001428 |
| pmr_devector_test.test | RUN | 0.134976 | 0.370019 | 0.423619 | 0.001767 | 0.001485 | 0.001628 |
| pmr_deque_test.test | RUN | 0.135487 | 0.297538 | 0.403943 | 0.001536 | 0.001626 | 0.001634 |
| pair_test.test | RUN | 0.341393 | 0.722479 | 0.961383 | 0.001818 | 0.001749 | 0.001622 |
| null_iterators_test.test | RUN | 0.564259 | 1.410388 | 1.560712 | 0.001688 | 0.001728 | 0.001721 |
| node_handle_test.test | RUN | 0.253449 | 1.808512 | 0.681832 | 0.001806 | 0.001680 | 0.001608 |
| node_allocator_test.test | RUN | 3.041069 | 17.662990 | 14.275057 | 0.012907 | 0.012511 | 0.013036 |
| new_delete_resource_test.test | RUN | 0.307547 | 1.055890 | 0.897964 | 0.002180 | 0.002467 | 0.002422 |
| nest_test.test | RUN | 0.767932 | 4.888673 | 3.077559 | 0.001597 | 0.001834 | 0.001714 |
| nest_api_test.test | RUN | 1.517619 | 9.342278 | 6.016333 | 0.003070 | 0.002827 | 0.002823 |
| monotonic_buffer_resource_test.test | RUN | 0.344763 | 1.907056 | 0.903554 | 0.001640 | 0.001726 | 0.001677 |
| memory_resource_test.test | RUN | 0.198932 | 0.544204 | 0.526452 | 0.001709 | 0.001732 | 0.001698 |
| map_test.test | RUN | 6.320071 | 29.825125 | 30.336001 | 0.012672 | 0.012957 | 0.012374 |
| list_test.test | RUN | 2.800335 | 15.447850 | 12.568594 | 0.033262 | 0.032111 | 0.030716 |
| is_constructible_test.test | RUN | 0.108714 | 0.200414 | 0.297803 | 0.000778 | 0.000766 | 0.001636 |
| insert_vs_emplace_test.test | RUN | 0.322910 | 1.251942 | 1.724585 | 0.001823 | 0.001792 | 0.001769 |
| hub_uses_allocator_test.test | RUN | 0.387175 | 1.052529 | 1.224221 | 0.001692 | 0.001795 | 0.001735 |
| hub_stability_test.test | RUN | 0.748474 | 4.984060 | 4.289177 | 0.002192 | 0.002318 | 0.001924 |
| hub_sort_test.test | RUN | 0.480259 | 2.264116 | 2.026686 | 0.695235 | 0.648072 | 0.643627 |
| hub_pocxx_test.test | RUN | 0.493860 | 2.635437 | 1.522630 | 0.001850 | 0.001775 | 0.001521 |
| hub_new_extended_alignment_test.test | RUN | 0.273453 | 0.796382 | 0.804182 | 0.001638 | 0.001677 | 0.001752 |
| hub_exception_safety_test.test | RUN | 1.009140 | 6.231211 | 4.241495 | 0.099316 | 0.096634 | 0.104766 |
| hub_empty_sort_test.test | RUN | 0.289968 | 0.740958 | 0.926925 | 0.001739 | 0.001769 | 0.001624 |
| hub_api_test.test | RUN | 3.032519 | 25.792033 | 13.997953 | 0.004972 | 0.004221 | 0.004038 |
| global_resource_test.test | RUN | 0.175792 | 0.505090 | 0.487050 | 0.001786 | 0.001788 | 0.001683 |
| flat_tree_test.test | RUN | 5.220074 | 38.278261 | 24.830766 | 0.001703 | 0.001679 | 0.001619 |
| flat_set_test.test | RUN | 6.359507 | 48.787516 | 33.058513 | 0.009687 | 0.008409 | 0.008812 |
| flat_set_adaptor_test.test | RUN | 5.026931 | 43.159848 | 26.034247 | 0.009515 | 0.008672 | 0.009123 |
| flat_map_test.test | RUN | 11.071384 | 72.083484 | 53.570779 | 0.010709 | 0.010398 | 0.009925 |
| flat_map_adaptor_test.test | RUN | 7.127042 | 52.660067 | 38.048403 | 0.010130 | 0.009580 | 0.009371 |
| explicit_inst_vector_test.test | RUN | 1.093598 | 7.207593 | 5.728229 | 0.001619 | 0.001714 | 0.001712 |
| explicit_inst_string_test.test | RUN | 1.464363 | 12.506864 | 7.869518 | 0.001508 | 0.001726 | 0.001689 |
| explicit_inst_static_vector_test.test | RUN | 0.150218 | 0.467055 | 0.577452 | 0.001670 | 0.001568 | 0.001742 |
| explicit_inst_stable_vector_test.test | RUN | 0.952652 | 4.522003 | 3.729152 | 0.001662 | 0.001742 | 0.001650 |
| explicit_inst_small_vector_test.test | RUN | 0.761197 | 4.114573 | 3.311306 | 0.001689 | 0.001698 | 0.001567 |
| explicit_inst_slist_test.test | RUN | 0.563755 | 3.693559 | 2.619357 | 0.001684 | 0.001652 | 0.001640 |
| explicit_inst_set_test.test | RUN | 0.843061 | 4.257890 | 3.013027 | 0.001732 | 0.001694 | 0.001632 |
| explicit_inst_map_test.test | RUN | 0.944627 | 4.492652 | 3.289064 | 0.001810 | 0.001801 | 0.001685 |
| explicit_inst_list_test.test | RUN | 0.205357 | 1.034717 | 0.859469 | 0.001686 | 0.001706 | 0.001672 |
| explicit_inst_flat_set_test.test | RUN | 4.653686 | 32.617485 | 19.765044 | 0.001725 | 0.001723 | 0.001592 |
| explicit_inst_flat_map_test.test | RUN | 5.219719 | 33.838576 | 22.002475 | 0.001608 | 0.001688 | 0.001360 |
| explicit_inst_devector_test.test | RUN | 0.612306 | 4.126948 | 2.714117 | 0.001494 | 0.001719 | 0.001612 |
| explicit_inst_deque_test.test | RUN | 0.644596 | 4.396934 | 2.888818 | 0.001679 | 0.001693 | 0.001577 |
| devector_test.test | RUN | 9.121442 | 51.821874 | 37.392617 | 0.004494 | 0.005447 | 0.005359 |
| devector_options_test.test | RUN | 0.401199 | 2.464822 | 1.618740 | 0.002160 | 0.001957 | 0.001954 |
| deque_test.test | RUN | 6.471992 | 26.267994 | 23.383248 | 0.371627 | 0.369078 | 0.444549 |
| deque_options_test.test | RUN | 0.327098 | 0.905422 | 1.092965 | 0.001821 | 0.001944 | 0.001786 |
| copy_move_algo_test.test | RUN | 0.315194 | 1.058682 | 1.011295 | 0.001997 | 0.001882 | 0.002010 |
| common_iterator_test.test | RUN | 0.022250 | 0.054467 | 0.046982 | 0.000719 | 0.000774 | 0.001683 |
| boost_iterator_comp_test.test | RUN | 0.430751 | 1.285894 | 1.535719 | 0.001694 | 0.001717 | 0.001732 |
| allocator_traits_test.test | RUN | 0.261319 | 0.582616 | 0.733553 | 0.001527 | 0.001753 | 0.001712 |
| alloc_full_test.test | RUN | 0.270392 | 1.189902 | 1.073047 | 0.004540 | 0.005602 | 0.005986 |
| alloc_basic_test.test | RUN | 0.239136 | 0.859025 | 0.778667 | 0.001823 | 0.001820 | 0.001932 |
| adaptive_pool_test.test | RUN | 2.136839 | 17.680609 | 9.123146 | 0.011881 | 0.010436 | 0.011725 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.953× | 0.955×, 0.951× | 0.996× |
| clang | 1.032× | 1.025×, 1.039× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.012756s | 0.010817s | 0.012589s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001050s | 0.001055s | 0.001021s | byte equality observed |
| alloc_full_test.test | 6 | 0.003630s | 0.003490s | 0.003807s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000966s | 0.000971s | 0.000970s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001025s | 0.001020s | 0.000971s | byte equality observed |
| common_iterator_test.test | 10 | 0.000462s | 0.000455s | 0.001034s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001427s | 0.001132s | 0.001225s | byte equality observed |
| deque_options_test.test | 10 | 0.001054s | 0.001054s | 0.001032s | byte equality observed |
| deque_test.test | 1 | 0.402161s | 0.389458s | 0.472287s | byte equality observed |
| devector_options_test.test | 10 | 0.001208s | 0.001039s | 0.001062s | byte equality observed |
| devector_test.test | 4 | 0.003948s | 0.003542s | 0.004873s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000914s | 0.000881s | 0.000921s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000914s | 0.000891s | 0.000907s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000930s | 0.000901s | 0.000917s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000927s | 0.000909s | 0.000917s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000917s | 0.000899s | 0.000906s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000933s | 0.000906s | 0.000908s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000931s | 0.000922s | 0.000915s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000926s | 0.000921s | 0.000915s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000929s | 0.000921s | 0.000963s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000928s | 0.000934s | 0.000944s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000912s | 0.000903s | 0.000933s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000927s | 0.000937s | 0.000926s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000923s | 0.000914s | 0.000911s | byte equality observed |
| flat_map_adaptor_test.test | 3 | 0.006948s | 0.006480s | 0.006542s | byte equality observed |
| flat_map_test.test | 3 | 0.007675s | 0.007092s | 0.006789s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.006781s | 0.006141s | 0.006465s | byte equality observed |
| flat_set_test.test | 3 | 0.006761s | 0.005903s | 0.006544s | byte equality observed |
| flat_tree_test.test | 10 | 0.000961s | 0.000942s | 0.000965s | byte equality observed |
| global_resource_test.test | 10 | 0.000959s | 0.000957s | 0.000956s | byte equality observed |
| hub_api_test.test | 6 | 0.003404s | 0.002539s | 0.002488s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000940s | 0.000938s | 0.000939s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.071987s | 0.073524s | 0.074872s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000982s | 0.000992s | 0.000992s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001004s | 0.000997s | 0.000971s | byte equality observed |
| hub_sort_test.test | 1 | 0.538598s | 0.530069s | 0.513877s | byte equality observed |
| hub_stability_test.test | 10 | 0.001203s | 0.001149s | 0.001204s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000948s | 0.000917s | 0.000952s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000977s | 0.000935s | 0.000971s | byte equality observed |
| is_constructible_test.test | 10 | 0.000439s | 0.000391s | 0.000897s | byte equality observed |
| list_test.test | 1 | 0.019910s | 0.020101s | 0.020131s | byte equality observed |
| map_test.test | 3 | 0.009334s | 0.009151s | 0.009100s | byte equality observed |
| memory_resource_test.test | 10 | 0.000978s | 0.000925s | 0.000957s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000970s | 0.000940s | 0.000945s | byte equality observed |
| nest_api_test.test | 10 | 0.001917s | 0.001850s | 0.001802s | byte equality observed |
| nest_test.test | 10 | 0.000994s | 0.000966s | 0.000961s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001397s | 0.001407s | 0.001400s | byte equality observed |
| node_allocator_test.test | 3 | 0.008378s | 0.008384s | 0.008460s | byte equality observed |
| node_handle_test.test | 10 | 0.000971s | 0.000954s | 0.000938s | byte equality observed |
| null_iterators_test.test | 10 | 0.000960s | 0.000917s | 0.000916s | byte equality observed |
| pair_test.test | 10 | 0.000946s | 0.000927s | 0.000926s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000939s | 0.000908s | 0.000905s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000929s | 0.000900s | 0.000918s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000928s | 0.000905s | 0.000934s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000925s | 0.000922s | 0.000936s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000924s | 0.000907s | 0.000931s | byte equality observed |
| pmr_list_test.test | 10 | 0.000931s | 0.000900s | 0.000931s | byte equality observed |
| pmr_map_test.test | 10 | 0.000928s | 0.000937s | 0.000929s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000925s | 0.000933s | 0.000927s | byte equality observed |
| pmr_set_test.test | 10 | 0.000927s | 0.000928s | 0.000942s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000931s | 0.000930s | 0.000925s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000925s | 0.000917s | 0.000931s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000938s | 0.000940s | 0.000930s | byte equality observed |
| pmr_string_test.test | 10 | 0.000441s | 0.000442s | 0.000954s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000452s | 0.000446s | 0.000934s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.000968s | 0.000982s | 0.000950s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000948s | 0.000941s | 0.000954s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000951s | 0.000948s | 0.000949s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000948s | 0.000941s | 0.000989s | byte equality observed |
| segtor_options_test.test | 10 | 0.001009s | 0.000966s | 0.001034s | byte equality observed |
| segtor_test.test | 1 | 0.418360s | 0.370083s | 0.414501s | byte equality observed |
| set_test.test | 2 | 0.013240s | 0.011575s | 0.012762s | byte equality observed |
| slist_test.test | 1 | 0.020827s | 0.020504s | 0.020567s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001091s | 0.001078s | 0.001074s | byte equality observed |
| small_vector_test.test | 5 | 0.002620s | 0.001971s | 0.004049s | byte equality observed |
| stable_vector_test.test | 1 | 0.031596s | 0.031774s | 0.033153s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001059s | 0.001004s | 0.001037s | byte equality observed |
| static_vector_test.test | 10 | 0.001417s | 0.001250s | 0.001309s | byte equality observed |
| string_options_test.test | 10 | 0.000997s | 0.000983s | 0.000981s | byte equality observed |
| string_test.test | 2 | 0.012082s | 0.012060s | 0.011722s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000982s | 0.000958s | 0.000962s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000999s | 0.000992s | 0.000985s | byte equality observed |
| throw_exception_test.test | 10 | 0.000926s | 0.000925s | 0.000934s | byte equality observed |
| tree_test.test | 10 | 0.000931s | 0.000950s | 0.000924s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000991s | 0.000987s | 0.000973s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000927s | 0.000933s | 0.000929s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000431s | 0.000430s | 0.000911s | byte equality observed |
| vector_options_test.test | 10 | 0.001092s | 0.001074s | 0.001085s | byte equality observed |
| vector_test.test | 3 | 0.004305s | 0.004032s | 0.005690s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
