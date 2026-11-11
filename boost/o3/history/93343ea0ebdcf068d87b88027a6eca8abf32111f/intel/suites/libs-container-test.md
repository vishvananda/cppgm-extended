# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 8.715466 | 41.289083 | 32.961592 | 0.007816 | 0.008134 | 0.011085 |
| vector_options_test.test | RUN | 0.600504 | 1.813931 | 1.615847 | 0.002518 | 0.002679 | 0.003884 |
| uses_allocator_test.test | RUN | 0.071469 | 0.144152 | 0.231935 | 0.000886 | 0.000953 | 0.002925 |
| uses_allocator_construction_test.test | RUN | 0.217697 | 0.577841 | 0.683167 | 0.002160 | 0.002306 | 0.001924 |
| unsynchronized_pool_resource_test.test | RUN | 0.526620 | 1.696479 | 0.916719 | 0.002356 | 0.002092 | 0.001769 |
| tree_test.test | RUN | 1.177398 | 6.259278 | 5.428150 | 0.002042 | 0.002272 | 0.002081 |
| throw_exception_test.test | RUN | 0.160575 | 0.379836 | 0.480694 | 0.001672 | 0.002344 | 0.002304 |
| synchronized_pool_resource_test.test | RUN | 0.334659 | 1.698978 | 1.111281 | 0.002839 | 0.002175 | 0.002014 |
| string_view_compat_test.test | RUN | 0.528235 | 3.496537 | 1.874693 | 0.001847 | 0.002184 | 0.001683 |
| string_test.test | RUN | 2.583425 | 13.119205 | 9.507761 | 0.019080 | 0.020255 | 0.019573 |
| string_options_test.test | RUN | 0.410066 | 2.798809 | 1.577345 | 0.002042 | 0.002272 | 0.001970 |
| static_vector_test.test | RUN | 2.981822 | 11.198973 | 9.078135 | 0.003422 | 0.003006 | 0.003579 |
| static_vector_options_test.test | RUN | 0.273292 | 0.745876 | 0.914474 | 0.002442 | 0.002533 | 0.001782 |
| stable_vector_test.test | RUN | 6.234796 | 22.833787 | 17.416425 | 0.038179 | 0.036348 | 0.041094 |
| small_vector_test.test | RUN | 3.494700 | 16.524032 | 12.914980 | 0.004539 | 0.004567 | 0.007537 |
| small_vector_options_test.test | RUN | 0.466092 | 1.989673 | 1.738630 | 0.003034 | 0.001881 | 0.002871 |
| slist_test.test | RUN | 2.712666 | 14.303099 | 11.517927 | 0.039449 | 0.031686 | 0.039301 |
| set_test.test | RUN | 4.885310 | 26.560299 | 20.065374 | 0.018465 | 0.016234 | 0.021772 |
| segtor_test.test | RUN | 6.978784 | 47.427415 | 22.502320 | 0.637564 | 0.538608 | 0.632799 |
| segtor_options_test.test | RUN | 0.306185 | 0.908503 | 1.093764 | 0.003664 | 0.002146 | 0.002997 |
| scoped_allocator_usage_test.test | RUN | 1.105780 | 4.502707 | 4.066483 | 0.002922 | 0.001992 | 0.002733 |
| scoped_allocator_adaptor_test.test | RUN | 0.495858 | 1.283472 | 1.756120 | 0.002942 | 0.001672 | 0.002790 |
| resource_adaptor_test.test | RUN | 0.252981 | 0.761495 | 0.792088 | 0.002740 | 0.001732 | 0.001648 |
| polymorphic_allocator_test.test | RUN | 0.239062 | 0.606594 | 0.681217 | 0.002748 | 0.002178 | 0.002133 |
| pmr_vector_test.test | RUN | 0.117491 | 0.219898 | 0.341374 | 0.000978 | 0.000664 | 0.002252 |
| pmr_string_test.test | RUN | 0.226948 | 0.479054 | 0.698534 | 0.000686 | 0.001005 | 0.002042 |
| pmr_stable_vector_test.test | RUN | 0.248485 | 0.796575 | 0.903274 | 0.002084 | 0.002454 | 0.002484 |
| pmr_small_vector_test.test | RUN | 0.144164 | 0.310434 | 0.464136 | 0.001649 | 0.001885 | 0.002223 |
| pmr_slist_test.test | RUN | 0.205012 | 1.021325 | 0.879365 | 0.002036 | 0.001637 | 0.002659 |
| pmr_set_test.test | RUN | 0.231437 | 0.534120 | 0.697267 | 0.001824 | 0.002240 | 0.002010 |
| pmr_nest_test.test | RUN | 0.193990 | 0.391655 | 0.508891 | 0.001728 | 0.001715 | 0.001911 |
| pmr_map_test.test | RUN | 0.240514 | 0.540468 | 0.725608 | 0.001361 | 0.002059 | 0.001925 |
| pmr_list_test.test | RUN | 0.157886 | 0.329698 | 0.486181 | 0.002502 | 0.002162 | 0.002072 |
| pmr_hub_test.test | RUN | 0.227123 | 0.462781 | 0.623537 | 0.001760 | 0.002292 | 0.002994 |
| pmr_flat_set_test.test | RUN | 0.178315 | 0.363786 | 0.526864 | 0.001580 | 0.002068 | 0.001580 |
| pmr_flat_map_test.test | RUN | 0.302196 | 0.417471 | 0.594736 | 0.001930 | 0.002344 | 0.001842 |
| pmr_devector_test.test | RUN | 0.174561 | 0.359089 | 0.452351 | 0.002240 | 0.002326 | 0.001812 |
| pmr_deque_test.test | RUN | 0.179326 | 0.312635 | 0.447226 | 0.001690 | 0.002094 | 0.002234 |
| pair_test.test | RUN | 0.533172 | 0.755335 | 1.001408 | 0.002229 | 0.002696 | 0.002020 |
| null_iterators_test.test | RUN | 0.721769 | 1.399956 | 1.696752 | 0.002042 | 0.001982 | 0.002264 |
| node_handle_test.test | RUN | 0.384358 | 1.754294 | 0.718100 | 0.002160 | 0.002162 | 0.003128 |
| node_allocator_test.test | RUN | 3.547523 | 17.473789 | 13.849504 | 0.018717 | 0.013886 | 0.014019 |
| new_delete_resource_test.test | RUN | 0.366324 | 1.051037 | 0.912547 | 0.003073 | 0.003121 | 0.003486 |
| nest_test.test | RUN | 0.916683 | 4.879181 | 3.150383 | 0.001846 | 0.003640 | 0.002980 |
| nest_api_test.test | RUN | 1.669959 | 9.393179 | 6.098618 | 0.003041 | 0.003534 | 0.004697 |
| monotonic_buffer_resource_test.test | RUN | 0.323250 | 1.886491 | 0.903993 | 0.001739 | 0.002538 | 0.002032 |
| memory_resource_test.test | RUN | 0.187749 | 0.548999 | 0.590135 | 0.001723 | 0.002192 | 0.001936 |
| map_test.test | RUN | 7.162114 | 29.260073 | 29.183582 | 0.013936 | 0.017340 | 0.012718 |
| list_test.test | RUN | 2.975391 | 15.199213 | 12.230443 | 0.036823 | 0.049551 | 0.030656 |
| is_constructible_test.test | RUN | 0.101553 | 0.196585 | 0.304174 | 0.000666 | 0.003162 | 0.002628 |
| insert_vs_emplace_test.test | RUN | 0.310013 | 1.266484 | 1.688326 | 0.001790 | 0.002375 | 0.003489 |
| hub_uses_allocator_test.test | RUN | 0.376150 | 1.071653 | 1.261583 | 0.001464 | 0.002428 | 0.002536 |
| hub_stability_test.test | RUN | 0.822179 | 5.154829 | 4.347154 | 0.002266 | 0.002628 | 0.002709 |
| hub_sort_test.test | RUN | 0.455066 | 2.226554 | 2.002283 | 0.702639 | 0.659476 | 0.615662 |
| hub_pocxx_test.test | RUN | 0.473633 | 2.742347 | 1.530251 | 0.002066 | 0.001978 | 0.002418 |
| hub_new_extended_alignment_test.test | RUN | 0.253633 | 0.816834 | 0.830442 | 0.001616 | 0.002767 | 0.001862 |
| hub_exception_safety_test.test | RUN | 1.023989 | 6.260844 | 4.336540 | 0.131207 | 0.127768 | 0.122492 |
| hub_empty_sort_test.test | RUN | 0.269466 | 0.729013 | 0.987733 | 0.001852 | 0.002298 | 0.002159 |
| hub_api_test.test | RUN | 3.242245 | 25.110716 | 12.976023 | 0.007362 | 0.005872 | 0.004183 |
| global_resource_test.test | RUN | 0.163244 | 0.504618 | 0.514361 | 0.001810 | 0.002166 | 0.002538 |
| flat_tree_test.test | RUN | 5.896016 | 38.345661 | 24.550105 | 0.001736 | 0.003361 | 0.003072 |
| flat_set_test.test | RUN | 7.008689 | 54.438767 | 31.280808 | 0.010252 | 0.008677 | 0.011840 |
| flat_set_adaptor_test.test | RUN | 5.737380 | 47.868678 | 24.648600 | 0.009457 | 0.014197 | 0.009249 |
| flat_map_test.test | RUN | 11.696893 | 75.800064 | 50.337506 | 0.014565 | 0.009914 | 0.012931 |
| flat_map_adaptor_test.test | RUN | 7.770225 | 58.409397 | 36.821618 | 0.009794 | 0.009148 | 0.012358 |
| explicit_inst_vector_test.test | RUN | 1.185398 | 7.094590 | 5.629576 | 0.001914 | 0.002257 | 0.002624 |
| explicit_inst_string_test.test | RUN | 1.643181 | 12.361572 | 7.757348 | 0.001996 | 0.002256 | 0.002264 |
| explicit_inst_static_vector_test.test | RUN | 0.234837 | 0.514450 | 0.657070 | 0.002006 | 0.003153 | 0.002553 |
| explicit_inst_stable_vector_test.test | RUN | 1.057928 | 4.474730 | 3.624604 | 0.001676 | 0.003578 | 0.001994 |
| explicit_inst_small_vector_test.test | RUN | 0.964929 | 4.096947 | 3.392290 | 0.003153 | 0.002230 | 0.002746 |
| explicit_inst_slist_test.test | RUN | 0.691930 | 3.691520 | 2.535056 | 0.001415 | 0.002216 | 0.001666 |
| explicit_inst_set_test.test | RUN | 0.986101 | 4.198569 | 3.008380 | 0.001646 | 0.002026 | 0.001927 |
| explicit_inst_map_test.test | RUN | 0.925672 | 4.384008 | 3.292310 | 0.002928 | 0.002321 | 0.003405 |
| explicit_inst_list_test.test | RUN | 0.206371 | 1.037584 | 0.865838 | 0.002502 | 0.002206 | 0.002746 |
| explicit_inst_flat_set_test.test | RUN | 4.984785 | 38.501825 | 19.161398 | 0.002247 | 0.002778 | 0.002162 |
| explicit_inst_flat_map_test.test | RUN | 5.537338 | 41.996696 | 21.030103 | 0.002130 | 0.003196 | 0.002374 |
| explicit_inst_devector_test.test | RUN | 0.605957 | 4.023687 | 2.949818 | 0.001865 | 0.002277 | 0.003175 |
| explicit_inst_deque_test.test | RUN | 0.722182 | 4.400268 | 3.133667 | 0.001752 | 0.001928 | 0.002547 |
| devector_test.test | RUN | 10.533126 | 69.985420 | 46.971783 | 0.008672 | 0.007488 | 0.007089 |
| devector_options_test.test | RUN | 0.460074 | 2.386319 | 1.669463 | 0.003794 | 0.003300 | 0.003234 |
| deque_test.test | RUN | 7.177016 | 41.015087 | 26.443856 | 0.648379 | 0.649788 | 0.688435 |
| deque_options_test.test | RUN | 0.323015 | 0.894091 | 1.181072 | 0.002738 | 0.002439 | 0.002866 |
| copy_move_algo_test.test | RUN | 0.310856 | 1.076476 | 1.148509 | 0.003388 | 0.002166 | 0.003056 |
| common_iterator_test.test | RUN | 0.022445 | 0.053086 | 0.057721 | 0.000876 | 0.000701 | 0.001633 |
| boost_iterator_comp_test.test | RUN | 0.516580 | 1.268561 | 1.593291 | 0.001928 | 0.002364 | 0.002505 |
| allocator_traits_test.test | RUN | 0.301574 | 0.588199 | 0.754422 | 0.002046 | 0.002641 | 0.002426 |
| alloc_full_test.test | RUN | 0.346645 | 1.188289 | 1.060749 | 0.005755 | 0.006402 | 0.008100 |
| alloc_basic_test.test | RUN | 0.234585 | 0.883782 | 0.859864 | 0.002446 | 0.002178 | 0.002258 |
| adaptive_pool_test.test | RUN | 3.398087 | 20.877352 | 13.802238 | 0.018624 | 0.026263 | 0.023240 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.938×† | 0.909×, 0.967× | 0.971× |
| clang | 0.977×† | 0.950×, 1.004× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 1 | 0.018967s | 0.020070s | 0.018605s | byte equality observed |
| alloc_basic_test.test | 7 | 0.001738s | 0.001875s | 0.001795s | byte equality observed |
| alloc_full_test.test | 2 | 0.005560s | 0.005954s | 0.005929s | byte equality observed |
| allocator_traits_test.test | 7 | 0.001622s | 0.001677s | 0.001815s | byte equality observed |
| boost_iterator_comp_test.test | 6 | 0.001647s | 0.001697s | 0.001727s | byte equality observed |
| common_iterator_test.test | 8 | 0.000805s | 0.000828s | 0.001839s | byte equality observed |
| copy_move_algo_test.test | 5 | 0.002049s | 0.001775s | 0.002031s | byte equality observed |
| deque_options_test.test | 7 | 0.001733s | 0.001578s | 0.001759s | byte equality observed |
| deque_test.test | 1 | 0.650907s | 0.646222s | 0.703645s | byte equality observed |
| devector_options_test.test | 6 | 0.002081s | 0.001877s | 0.001823s | byte equality observed |
| devector_test.test | 2 | 0.006720s | 0.005769s | 0.007742s | byte equality observed |
| explicit_inst_deque_test.test | 6 | 0.001676s | 0.001755s | 0.001702s | byte equality observed |
| explicit_inst_devector_test.test | 7 | 0.001554s | 0.001639s | 0.001503s | byte equality observed |
| explicit_inst_flat_map_test.test | 7 | 0.001586s | 0.001507s | 0.001483s | byte equality observed |
| explicit_inst_flat_set_test.test | 8 | 0.001577s | 0.001574s | 0.001574s | byte equality observed |
| explicit_inst_list_test.test | 7 | 0.001492s | 0.001618s | 0.001543s | byte equality observed |
| explicit_inst_map_test.test | 7 | 0.001498s | 0.001592s | 0.001497s | byte equality observed |
| explicit_inst_set_test.test | 7 | 0.001573s | 0.001601s | 0.001636s | byte equality observed |
| explicit_inst_slist_test.test | 6 | 0.001559s | 0.001565s | 0.001495s | byte equality observed |
| explicit_inst_small_vector_test.test | 6 | 0.001592s | 0.001636s | 0.001612s | byte equality observed |
| explicit_inst_stable_vector_test.test | 7 | 0.001761s | 0.001670s | 0.001583s | byte equality observed |
| explicit_inst_static_vector_test.test | 8 | 0.001628s | 0.001689s | 0.001546s | byte equality observed |
| explicit_inst_string_test.test | 6 | 0.001837s | 0.001695s | 0.001657s | byte equality observed |
| explicit_inst_vector_test.test | 7 | 0.001731s | 0.001700s | 0.001563s | byte equality observed |
| flat_map_adaptor_test.test | 1 | 0.009888s | 0.010114s | 0.009319s | byte equality observed |
| flat_map_test.test | 2 | 0.010985s | 0.009893s | 0.010179s | byte equality observed |
| flat_set_adaptor_test.test | 1 | 0.009541s | 0.008226s | 0.008792s | byte equality observed |
| flat_set_test.test | 2 | 0.009513s | 0.008475s | 0.010672s | byte equality observed |
| flat_tree_test.test | 7 | 0.001706s | 0.001727s | 0.001681s | byte equality observed |
| global_resource_test.test | 8 | 0.001855s | 0.001822s | 0.001735s | byte equality observed |
| hub_api_test.test | 4 | 0.005343s | 0.004016s | 0.004074s | byte equality observed |
| hub_empty_sort_test.test | 8 | 0.001688s | 0.001734s | 0.001671s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.113493s | 0.105159s | 0.115312s | byte equality observed |
| hub_new_extended_alignment_test.test | 7 | 0.001899s | 0.001762s | 0.001697s | byte equality observed |
| hub_pocxx_test.test | 9 | 0.001934s | 0.001783s | 0.001945s | byte equality observed |
| hub_sort_test.test | 1 | 0.694155s | 0.637333s | 0.609639s | byte equality observed |
| hub_stability_test.test | 5 | 0.002216s | 0.002051s | 0.002375s | byte equality observed |
| hub_uses_allocator_test.test | 5 | 0.001828s | 0.001929s | 0.001636s | byte equality observed |
| insert_vs_emplace_test.test | 6 | 0.001727s | 0.001763s | 0.001714s | byte equality observed |
| is_constructible_test.test | 7 | 0.000884s | 0.000801s | 0.001598s | byte equality observed |
| list_test.test | 1 | 0.030655s | 0.033550s | 0.030395s | byte equality observed |
| map_test.test | 2 | 0.012889s | 0.012399s | 0.012640s | byte equality observed |
| memory_resource_test.test | 7 | 0.001829s | 0.001608s | 0.001633s | byte equality observed |
| monotonic_buffer_resource_test.test | 7 | 0.001904s | 0.001690s | 0.001690s | byte equality observed |
| nest_api_test.test | 5 | 0.003277s | 0.003001s | 0.003179s | byte equality observed |
| nest_test.test | 6 | 0.001818s | 0.001890s | 0.001733s | byte equality observed |
| new_delete_resource_test.test | 5 | 0.002462s | 0.002488s | 0.002388s | byte equality observed |
| node_allocator_test.test | 1 | 0.014856s | 0.013041s | 0.013444s | byte equality observed |
| node_handle_test.test | 6 | 0.001755s | 0.001681s | 0.001789s | byte equality observed |
| null_iterators_test.test | 6 | 0.001733s | 0.001715s | 0.001633s | byte equality observed |
| pair_test.test | 6 | 0.001706s | 0.001626s | 0.001625s | byte equality observed |
| pmr_deque_test.test | 7 | 0.001727s | 0.001723s | 0.001637s | byte equality observed |
| pmr_devector_test.test | 7 | 0.001736s | 0.001706s | 0.001605s | byte equality observed |
| pmr_flat_map_test.test | 6 | 0.001689s | 0.001795s | 0.001576s | byte equality observed |
| pmr_flat_set_test.test | 6 | 0.001681s | 0.001642s | 0.001651s | byte equality observed |
| pmr_hub_test.test | 5 | 0.001721s | 0.001622s | 0.001555s | byte equality observed |
| pmr_list_test.test | 5 | 0.001699s | 0.001767s | 0.001586s | byte equality observed |
| pmr_map_test.test | 7 | 0.001616s | 0.001916s | 0.001571s | byte equality observed |
| pmr_nest_test.test | 6 | 0.001702s | 0.001641s | 0.001628s | byte equality observed |
| pmr_set_test.test | 7 | 0.001691s | 0.001623s | 0.001577s | byte equality observed |
| pmr_slist_test.test | 7 | 0.001651s | 0.001655s | 0.001631s | byte equality observed |
| pmr_small_vector_test.test | 6 | 0.001574s | 0.001588s | 0.001553s | byte equality observed |
| pmr_stable_vector_test.test | 4 | 0.001640s | 0.001556s | 0.001645s | byte equality observed |
| pmr_string_test.test | 7 | 0.000823s | 0.000841s | 0.001582s | byte equality observed |
| pmr_vector_test.test | 8 | 0.000832s | 0.000783s | 0.001598s | byte equality observed |
| polymorphic_allocator_test.test | 6 | 0.001776s | 0.001634s | 0.001613s | byte equality observed |
| resource_adaptor_test.test | 6 | 0.001662s | 0.001700s | 0.001601s | byte equality observed |
| scoped_allocator_adaptor_test.test | 5 | 0.001827s | 0.001726s | 0.001621s | byte equality observed |
| scoped_allocator_usage_test.test | 5 | 0.001660s | 0.001717s | 0.001635s | byte equality observed |
| segtor_options_test.test | 6 | 0.001713s | 0.001619s | 0.001659s | byte equality observed |
| segtor_test.test | 1 | 0.556680s | 0.540770s | 0.584113s | byte equality observed |
| set_test.test | 1 | 0.017305s | 0.015484s | 0.016801s | byte equality observed |
| slist_test.test | 1 | 0.031316s | 0.030403s | 0.030283s | byte equality observed |
| small_vector_options_test.test | 6 | 0.001753s | 0.001701s | 0.001711s | byte equality observed |
| small_vector_test.test | 2 | 0.004340s | 0.003394s | 0.006335s | byte equality observed |
| stable_vector_test.test | 1 | 0.037833s | 0.037264s | 0.044031s | byte equality observed |
| static_vector_options_test.test | 7 | 0.001747s | 0.001678s | 0.001796s | byte equality observed |
| static_vector_test.test | 5 | 0.002602s | 0.002402s | 0.002251s | byte equality observed |
| string_options_test.test | 7 | 0.001773s | 0.001840s | 0.001664s | byte equality observed |
| string_test.test | 1 | 0.018689s | 0.018832s | 0.018269s | byte equality observed |
| string_view_compat_test.test | 7 | 0.001683s | 0.001676s | 0.001579s | byte equality observed |
| synchronized_pool_resource_test.test | 8 | 0.001750s | 0.001824s | 0.001713s | byte equality observed |
| throw_exception_test.test | 8 | 0.001716s | 0.001632s | 0.001565s | byte equality observed |
| tree_test.test | 9 | 0.001655s | 0.001715s | 0.001626s | byte equality observed |
| unsynchronized_pool_resource_test.test | 9 | 0.001760s | 0.001671s | 0.001653s | byte equality observed |
| uses_allocator_construction_test.test | 8 | 0.001661s | 0.001675s | 0.001563s | byte equality observed |
| uses_allocator_test.test | 8 | 0.000851s | 0.000842s | 0.001604s | byte equality observed |
| vector_options_test.test | 6 | 0.001832s | 0.001844s | 0.001861s | byte equality observed |
| vector_test.test | 2 | 0.007664s | 0.006534s | 0.008719s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
