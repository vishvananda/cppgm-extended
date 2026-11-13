# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 6.923821 | 32.658557 | 27.444330 | 0.005892 | 0.005618 | 0.007659 |
| vector_options_test.test | RUN | 0.346425 | 1.417562 | 1.226224 | 0.002094 | 0.001998 | 0.002026 |
| uses_allocator_test.test | RUN | 0.058486 | 0.120855 | 0.162048 | 0.000555 | 0.000765 | 0.001749 |
| uses_allocator_construction_test.test | RUN | 0.178602 | 0.458383 | 0.519924 | 0.001868 | 0.001760 | 0.001788 |
| unsynchronized_pool_resource_test.test | RUN | 0.236169 | 1.364185 | 0.688283 | 0.001650 | 0.002002 | 0.001828 |
| tree_test.test | RUN | 0.947721 | 4.847453 | 3.853083 | 0.001734 | 0.001808 | 0.001661 |
| throw_exception_test.test | RUN | 0.133935 | 0.266958 | 0.344665 | 0.001750 | 0.001495 | 0.001790 |
| synchronized_pool_resource_test.test | RUN | 0.236171 | 1.348828 | 0.691238 | 0.002007 | 0.001936 | 0.001896 |
| string_view_compat_test.test | RUN | 0.389983 | 2.702417 | 1.372724 | 0.001653 | 0.001858 | 0.001848 |
| string_test.test | RUN | 1.626721 | 10.280358 | 7.346798 | 0.016231 | 0.016075 | 0.015325 |
| string_options_test.test | RUN | 0.294557 | 2.229595 | 1.209506 | 0.001899 | 0.001966 | 0.001957 |
| static_vector_test.test | RUN | 1.719437 | 8.446556 | 7.461342 | 0.002448 | 0.002421 | 0.002507 |
| static_vector_options_test.test | RUN | 0.224602 | 0.598075 | 0.707101 | 0.001943 | 0.001921 | 0.001898 |
| stable_vector_test.test | RUN | 3.871386 | 17.778679 | 14.284099 | 0.031115 | 0.031117 | 0.033207 |
| small_vector_test.test | RUN | 2.668227 | 12.476009 | 10.250845 | 0.003679 | 0.003410 | 0.005154 |
| small_vector_options_test.test | RUN | 0.419430 | 1.540623 | 1.420098 | 0.002040 | 0.002072 | 0.001822 |
| slist_test.test | RUN | 2.411863 | 11.364871 | 8.978557 | 0.024511 | 0.024766 | 0.024620 |
| set_test.test | RUN | 3.708345 | 21.811900 | 16.578627 | 0.015268 | 0.013940 | 0.014972 |
| segtor_test.test | RUN | 4.838426 | 23.078868 | 18.441298 | 0.677193 | 0.641887 | 0.664890 |
| segtor_options_test.test | RUN | 0.260712 | 0.702451 | 0.814618 | 0.001998 | 0.001972 | 0.001859 |
| scoped_allocator_usage_test.test | RUN | 0.878490 | 3.388107 | 3.076275 | 0.001894 | 0.002097 | 0.002014 |
| scoped_allocator_adaptor_test.test | RUN | 0.391193 | 1.062530 | 1.440333 | 0.001930 | 0.001995 | 0.001976 |
| resource_adaptor_test.test | RUN | 0.203162 | 0.596738 | 0.659019 | 0.001734 | 0.001880 | 0.001893 |
| polymorphic_allocator_test.test | RUN | 0.174898 | 0.481595 | 0.520802 | 0.001920 | 0.001878 | 0.001832 |
| pmr_vector_test.test | RUN | 0.100813 | 0.189574 | 0.292307 | 0.000811 | 0.000772 | 0.001564 |
| pmr_string_test.test | RUN | 0.200735 | 0.402295 | 0.549589 | 0.000830 | 0.000789 | 0.001730 |
| pmr_stable_vector_test.test | RUN | 0.246330 | 0.625007 | 0.695646 | 0.001762 | 0.001845 | 0.001654 |
| pmr_small_vector_test.test | RUN | 0.117358 | 0.260027 | 0.371485 | 0.001665 | 0.001909 | 0.001764 |
| pmr_slist_test.test | RUN | 0.155466 | 0.822040 | 0.714243 | 0.001742 | 0.001881 | 0.001484 |
| pmr_set_test.test | RUN | 0.176288 | 0.398878 | 0.574195 | 0.001845 | 0.001997 | 0.001509 |
| pmr_nest_test.test | RUN | 0.138439 | 0.283145 | 0.349242 | 0.001728 | 0.001854 | 0.001676 |
| pmr_map_test.test | RUN | 0.182138 | 0.460789 | 0.529739 | 0.001642 | 0.001950 | 0.001730 |
| pmr_list_test.test | RUN | 0.126985 | 0.279343 | 0.375034 | 0.001761 | 0.001769 | 0.001721 |
| pmr_hub_test.test | RUN | 0.170663 | 0.379810 | 0.486425 | 0.001854 | 0.001739 | 0.001692 |
| pmr_flat_set_test.test | RUN | 0.133855 | 0.291248 | 0.393551 | 0.001720 | 0.001641 | 0.001520 |
| pmr_flat_map_test.test | RUN | 0.159235 | 0.331820 | 0.480943 | 0.001852 | 0.001842 | 0.001420 |
| pmr_devector_test.test | RUN | 0.105197 | 0.299953 | 0.369750 | 0.001834 | 0.001595 | 0.001583 |
| pmr_deque_test.test | RUN | 0.107353 | 0.252664 | 0.373232 | 0.001860 | 0.001843 | 0.001791 |
| pair_test.test | RUN | 0.275577 | 0.578448 | 0.809278 | 0.001872 | 0.001866 | 0.001884 |
| null_iterators_test.test | RUN | 0.416647 | 1.127380 | 1.320122 | 0.001848 | 0.001950 | 0.001763 |
| node_handle_test.test | RUN | 0.180435 | 1.382354 | 0.556940 | 0.001766 | 0.001823 | 0.001801 |
| node_allocator_test.test | RUN | 2.827084 | 13.988553 | 11.447705 | 0.010278 | 0.010297 | 0.010502 |
| new_delete_resource_test.test | RUN | 0.227893 | 0.818518 | 0.725263 | 0.002434 | 0.002522 | 0.002610 |
| nest_test.test | RUN | 0.512168 | 3.802167 | 2.462605 | 0.001763 | 0.001940 | 0.001825 |
| nest_api_test.test | RUN | 1.012170 | 7.098279 | 4.722541 | 0.002767 | 0.002736 | 0.002633 |
| monotonic_buffer_resource_test.test | RUN | 0.220998 | 1.487248 | 0.753786 | 0.001736 | 0.002000 | 0.001657 |
| memory_resource_test.test | RUN | 0.134117 | 0.435296 | 0.420097 | 0.001769 | 0.001948 | 0.001724 |
| map_test.test | RUN | 5.057516 | 23.769892 | 24.126763 | 0.011520 | 0.011243 | 0.011079 |
| list_test.test | RUN | 2.586083 | 12.256245 | 9.851318 | 0.024861 | 0.025619 | 0.025166 |
| is_constructible_test.test | RUN | 0.082485 | 0.152559 | 0.258552 | 0.000793 | 0.000858 | 0.001819 |
| insert_vs_emplace_test.test | RUN | 0.269559 | 0.975847 | 1.397232 | 0.001860 | 0.002074 | 0.001799 |
| hub_uses_allocator_test.test | RUN | 0.287938 | 0.877004 | 1.028048 | 0.001623 | 0.001950 | 0.001838 |
| hub_stability_test.test | RUN | 0.613548 | 4.023113 | 3.247214 | 0.002096 | 0.002144 | 0.001938 |
| hub_sort_test.test | RUN | 0.396035 | 1.784206 | 1.613171 | 0.545751 | 0.518735 | 0.506114 |
| hub_pocxx_test.test | RUN | 0.347861 | 2.085168 | 1.235140 | 0.001831 | 0.001878 | 0.001556 |
| hub_new_extended_alignment_test.test | RUN | 0.207697 | 0.645455 | 0.672123 | 0.001942 | 0.001897 | 0.001612 |
| hub_exception_safety_test.test | RUN | 0.722934 | 4.861928 | 3.275642 | 0.092161 | 0.090377 | 0.094857 |
| hub_empty_sort_test.test | RUN | 0.223381 | 0.577569 | 0.801041 | 0.001780 | 0.001884 | 0.001836 |
| hub_api_test.test | RUN | 2.441365 | 21.887064 | 10.832049 | 0.004484 | 0.003594 | 0.003734 |
| global_resource_test.test | RUN | 0.129905 | 0.382015 | 0.433074 | 0.001830 | 0.001970 | 0.001923 |
| flat_tree_test.test | RUN | 4.433471 | 30.177223 | 19.941592 | 0.001896 | 0.001838 | 0.001860 |
| flat_set_test.test | RUN | 5.629538 | 38.251201 | 25.849481 | 0.008650 | 0.007719 | 0.008126 |
| flat_set_adaptor_test.test | RUN | 4.172404 | 34.521088 | 20.384666 | 0.008589 | 0.007846 | 0.008394 |
| flat_map_test.test | RUN | 9.193332 | 57.308348 | 42.927668 | 0.009461 | 0.008992 | 0.008633 |
| flat_map_adaptor_test.test | RUN | 5.867187 | 41.803669 | 30.576306 | 0.008681 | 0.008156 | 0.008294 |
| explicit_inst_vector_test.test | RUN | 0.871568 | 5.638576 | 4.395060 | 0.001824 | 0.001851 | 0.001656 |
| explicit_inst_string_test.test | RUN | 1.173676 | 10.023115 | 5.934112 | 0.001662 | 0.001874 | 0.001821 |
| explicit_inst_static_vector_test.test | RUN | 0.119169 | 0.384383 | 0.461878 | 0.001855 | 0.001916 | 0.001786 |
| explicit_inst_stable_vector_test.test | RUN | 0.700053 | 3.534816 | 2.954766 | 0.001855 | 0.001849 | 0.001754 |
| explicit_inst_small_vector_test.test | RUN | 0.625561 | 3.222570 | 2.583587 | 0.001720 | 0.001869 | 0.001828 |
| explicit_inst_slist_test.test | RUN | 0.517913 | 2.943821 | 2.077847 | 0.001909 | 0.001789 | 0.001812 |
| explicit_inst_set_test.test | RUN | 0.729836 | 3.441691 | 2.461401 | 0.001787 | 0.001834 | 0.001800 |
| explicit_inst_map_test.test | RUN | 0.722284 | 3.567672 | 2.704400 | 0.001870 | 0.001866 | 0.001737 |
| explicit_inst_list_test.test | RUN | 0.163643 | 0.792653 | 0.677281 | 0.001848 | 0.001765 | 0.001782 |
| explicit_inst_flat_set_test.test | RUN | 3.702984 | 25.249375 | 15.269876 | 0.001820 | 0.002118 | 0.001614 |
| explicit_inst_flat_map_test.test | RUN | 3.989235 | 26.396185 | 17.142869 | 0.001842 | 0.001908 | 0.001774 |
| explicit_inst_devector_test.test | RUN | 0.492627 | 3.208212 | 2.141264 | 0.001604 | 0.001759 | 0.001726 |
| explicit_inst_deque_test.test | RUN | 0.524775 | 3.378482 | 2.302228 | 0.001794 | 0.001767 | 0.001768 |
| devector_test.test | RUN | 7.277786 | 42.944709 | 31.683343 | 0.003908 | 0.003838 | 0.004747 |
| devector_options_test.test | RUN | 0.324322 | 1.865099 | 1.366668 | 0.002242 | 0.002028 | 0.001910 |
| deque_test.test | RUN | 4.511939 | 21.588896 | 16.405598 | 0.431462 | 0.384807 | 0.425037 |
| deque_options_test.test | RUN | 0.271452 | 0.699147 | 0.948467 | 0.001894 | 0.001845 | 0.001962 |
| copy_move_algo_test.test | RUN | 0.248888 | 0.823447 | 0.899839 | 0.002196 | 0.001990 | 0.001876 |
| common_iterator_test.test | RUN | 0.019800 | 0.046307 | 0.051420 | 0.000824 | 0.000753 | 0.001826 |
| boost_iterator_comp_test.test | RUN | 0.345495 | 0.993732 | 1.320632 | 0.001669 | 0.002002 | 0.001604 |
| allocator_traits_test.test | RUN | 0.208217 | 0.465803 | 0.553500 | 0.001645 | 0.001865 | 0.001614 |
| alloc_full_test.test | RUN | 0.228706 | 0.913155 | 0.734543 | 0.004960 | 0.004906 | 0.004463 |
| alloc_basic_test.test | RUN | 0.185496 | 0.669852 | 0.575312 | 0.001889 | 0.001632 | 0.001944 |
| adaptive_pool_test.test | RUN | 2.730390 | 14.508594 | 11.754374 | 0.010562 | 0.013312 | 0.014775 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.932× | 0.938×, 0.926× | 1.009× |
| clang | 0.961× | 0.956×, 0.967× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 2 | 0.010485s | 0.009808s | 0.010576s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001093s | 0.001082s | 0.001079s | byte equality observed |
| alloc_full_test.test | 7 | 0.003083s | 0.002999s | 0.003142s | byte equality observed |
| allocator_traits_test.test | 10 | 0.001002s | 0.000991s | 0.001035s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.001069s | 0.001074s | 0.001017s | byte equality observed |
| common_iterator_test.test | 10 | 0.000477s | 0.000476s | 0.000998s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001254s | 0.001137s | 0.001143s | byte equality observed |
| deque_options_test.test | 10 | 0.001096s | 0.001046s | 0.001055s | byte equality observed |
| deque_test.test | 1 | 0.455724s | 0.392370s | 0.461867s | byte equality observed |
| devector_options_test.test | 10 | 0.001167s | 0.001128s | 0.001126s | byte equality observed |
| devector_test.test | 5 | 0.003314s | 0.003163s | 0.004337s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000958s | 0.000979s | 0.000963s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000951s | 0.000970s | 0.000945s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000960s | 0.000992s | 0.000949s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000958s | 0.000989s | 0.000945s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000955s | 0.000963s | 0.000941s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000974s | 0.000965s | 0.000939s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000966s | 0.000970s | 0.000959s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000964s | 0.000979s | 0.000954s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000962s | 0.000968s | 0.000980s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000970s | 0.000974s | 0.000987s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000955s | 0.000949s | 0.000956s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000968s | 0.000963s | 0.000970s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000976s | 0.000960s | 0.000959s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.006029s | 0.005618s | 0.005868s | byte equality observed |
| flat_map_test.test | 3 | 0.006493s | 0.006217s | 0.006169s | byte equality observed |
| flat_set_adaptor_test.test | 3 | 0.006001s | 0.005462s | 0.005935s | byte equality observed |
| flat_set_test.test | 4 | 0.006146s | 0.005260s | 0.005857s | byte equality observed |
| flat_tree_test.test | 10 | 0.001020s | 0.000974s | 0.000971s | byte equality observed |
| global_resource_test.test | 10 | 0.001014s | 0.001002s | 0.000995s | byte equality observed |
| hub_api_test.test | 7 | 0.002807s | 0.002224s | 0.002184s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000992s | 0.000962s | 0.000967s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.071244s | 0.066998s | 0.065968s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.001061s | 0.001118s | 0.001047s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.001054s | 0.001043s | 0.001049s | byte equality observed |
| hub_sort_test.test | 1 | 0.446240s | 0.431124s | 0.416357s | byte equality observed |
| hub_stability_test.test | 10 | 0.001240s | 0.001239s | 0.001232s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000973s | 0.001015s | 0.000980s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.001002s | 0.000998s | 0.001027s | byte equality observed |
| is_constructible_test.test | 10 | 0.000440s | 0.000458s | 0.000954s | byte equality observed |
| list_test.test | 2 | 0.017525s | 0.017693s | 0.017747s | byte equality observed |
| map_test.test | 3 | 0.008088s | 0.007882s | 0.008007s | byte equality observed |
| memory_resource_test.test | 10 | 0.001080s | 0.001027s | 0.001055s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.001020s | 0.001021s | 0.000986s | byte equality observed |
| nest_api_test.test | 10 | 0.001641s | 0.001654s | 0.001692s | byte equality observed |
| nest_test.test | 10 | 0.001032s | 0.001026s | 0.001029s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001481s | 0.001449s | 0.001474s | byte equality observed |
| node_allocator_test.test | 3 | 0.007445s | 0.007402s | 0.007550s | byte equality observed |
| node_handle_test.test | 10 | 0.001050s | 0.001038s | 0.000987s | byte equality observed |
| null_iterators_test.test | 10 | 0.000990s | 0.000999s | 0.001038s | byte equality observed |
| pair_test.test | 10 | 0.000992s | 0.000985s | 0.001000s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000966s | 0.000986s | 0.001006s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000950s | 0.001034s | 0.000995s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000975s | 0.000967s | 0.001053s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.001000s | 0.001002s | 0.001021s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000975s | 0.000996s | 0.001014s | byte equality observed |
| pmr_list_test.test | 10 | 0.000981s | 0.000996s | 0.001009s | byte equality observed |
| pmr_map_test.test | 10 | 0.000956s | 0.000988s | 0.000996s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000967s | 0.001016s | 0.000998s | byte equality observed |
| pmr_set_test.test | 10 | 0.000967s | 0.000981s | 0.001007s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000982s | 0.000969s | 0.001000s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000990s | 0.000974s | 0.001021s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.001029s | 0.000999s | 0.001019s | byte equality observed |
| pmr_string_test.test | 10 | 0.000477s | 0.000467s | 0.000987s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000465s | 0.000463s | 0.001023s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001040s | 0.001024s | 0.001031s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.001034s | 0.001002s | 0.000991s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.001022s | 0.001020s | 0.001011s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.001019s | 0.001015s | 0.001016s | byte equality observed |
| segtor_options_test.test | 10 | 0.001050s | 0.001033s | 0.001071s | byte equality observed |
| segtor_test.test | 1 | 0.414739s | 0.375550s | 0.366290s | byte equality observed |
| set_test.test | 2 | 0.011006s | 0.009680s | 0.011074s | byte equality observed |
| slist_test.test | 2 | 0.017143s | 0.016955s | 0.017589s | byte equality observed |
| small_vector_options_test.test | 10 | 0.001149s | 0.001134s | 0.001115s | byte equality observed |
| small_vector_test.test | 6 | 0.002136s | 0.001967s | 0.003699s | byte equality observed |
| stable_vector_test.test | 1 | 0.025456s | 0.026506s | 0.026654s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001113s | 0.001075s | 0.001110s | byte equality observed |
| static_vector_test.test | 10 | 0.001353s | 0.001236s | 0.001321s | byte equality observed |
| string_options_test.test | 10 | 0.001036s | 0.001026s | 0.001023s | byte equality observed |
| string_test.test | 2 | 0.011130s | 0.011902s | 0.011454s | byte equality observed |
| string_view_compat_test.test | 10 | 0.001034s | 0.001017s | 0.001013s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.001044s | 0.001056s | 0.001046s | byte equality observed |
| throw_exception_test.test | 10 | 0.000974s | 0.001002s | 0.000952s | byte equality observed |
| tree_test.test | 10 | 0.001000s | 0.000984s | 0.000963s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.001036s | 0.001062s | 0.001079s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000991s | 0.001019s | 0.001004s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000460s | 0.000472s | 0.000970s | byte equality observed |
| vector_options_test.test | 10 | 0.001179s | 0.001159s | 0.001145s | byte equality observed |
| vector_test.test | 4 | 0.003632s | 0.003584s | 0.005317s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
