# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 4.515282 | 24.077213 | 20.587213 | 0.004632 | 0.004461 | 0.005573 |
| vector_options_test.test | RUN | 0.246228 | 1.069502 | 0.775069 | 0.001772 | 0.001678 | 0.001704 |
| uses_allocator_test.test | RUN | 0.048917 | 0.090022 | 0.107408 | 0.000566 | 0.000727 | 0.001458 |
| uses_allocator_construction_test.test | RUN | 0.140765 | 0.322197 | 0.373810 | 0.001453 | 0.001508 | 0.001554 |
| unsynchronized_pool_resource_test.test | RUN | 0.200274 | 0.962242 | 0.493786 | 0.001543 | 0.001434 | 0.001534 |
| tree_test.test | RUN | 0.701686 | 3.648644 | 2.801549 | 0.001423 | 0.001541 | 0.001558 |
| throw_exception_test.test | RUN | 0.098934 | 0.209330 | 0.246055 | 0.001561 | 0.001541 | 0.001475 |
| synchronized_pool_resource_test.test | RUN | 0.230046 | 0.944381 | 0.481206 | 0.001491 | 0.001625 | 0.001573 |
| string_view_compat_test.test | RUN | 0.328573 | 1.944701 | 0.937708 | 0.001287 | 0.001610 | 0.001561 |
| string_test.test | RUN | 1.328753 | 7.551453 | 5.421742 | 0.011649 | 0.012029 | 0.011507 |
| string_options_test.test | RUN | 0.221756 | 1.584617 | 0.726146 | 0.001678 | 0.001734 | 0.001540 |
| static_vector_test.test | RUN | 1.479041 | 6.322955 | 5.577416 | 0.002024 | 0.001988 | 0.002077 |
| static_vector_options_test.test | RUN | 0.169982 | 0.424040 | 0.471633 | 0.001527 | 0.001629 | 0.001574 |
| stable_vector_test.test | RUN | 3.303114 | 13.203238 | 10.590214 | 0.023722 | 0.023904 | 0.024546 |
| small_vector_test.test | RUN | 1.789961 | 9.181953 | 7.597216 | 0.003367 | 0.002652 | 0.004139 |
| small_vector_options_test.test | RUN | 0.280483 | 1.075914 | 0.968927 | 0.001830 | 0.001738 | 0.001660 |
| slist_test.test | RUN | 1.514791 | 8.412829 | 6.635886 | 0.018299 | 0.018440 | 0.018063 |
| set_test.test | RUN | 2.558445 | 15.499379 | 12.039241 | 0.012176 | 0.010761 | 0.011598 |
| segtor_test.test | RUN | 3.664714 | 16.880882 | 13.732736 | 0.452410 | 0.415989 | 0.392903 |
| segtor_options_test.test | RUN | 0.196296 | 0.493212 | 0.575760 | 0.001492 | 0.001692 | 0.001675 |
| scoped_allocator_usage_test.test | RUN | 0.624685 | 2.524417 | 2.214398 | 0.001378 | 0.001440 | 0.001578 |
| scoped_allocator_adaptor_test.test | RUN | 0.304902 | 0.718759 | 0.929761 | 0.001504 | 0.001660 | 0.001564 |
| resource_adaptor_test.test | RUN | 0.163026 | 0.467902 | 0.428749 | 0.001247 | 0.001277 | 0.001570 |
| polymorphic_allocator_test.test | RUN | 0.137750 | 0.401072 | 0.355078 | 0.001570 | 0.001447 | 0.001540 |
| pmr_vector_test.test | RUN | 0.073732 | 0.123795 | 0.184892 | 0.000624 | 0.000669 | 0.001482 |
| pmr_string_test.test | RUN | 0.165656 | 0.264593 | 0.351816 | 0.000592 | 0.000658 | 0.001526 |
| pmr_stable_vector_test.test | RUN | 0.152164 | 0.440692 | 0.501747 | 0.001375 | 0.001577 | 0.001667 |
| pmr_small_vector_test.test | RUN | 0.089999 | 0.176249 | 0.228271 | 0.001211 | 0.001475 | 0.001424 |
| pmr_slist_test.test | RUN | 0.119176 | 0.571374 | 0.476270 | 0.001405 | 0.001598 | 0.001399 |
| pmr_set_test.test | RUN | 0.135520 | 0.298014 | 0.360753 | 0.001276 | 0.001473 | 0.001442 |
| pmr_nest_test.test | RUN | 0.111697 | 0.209265 | 0.243157 | 0.001288 | 0.001586 | 0.001409 |
| pmr_map_test.test | RUN | 0.141927 | 0.314507 | 0.348811 | 0.001468 | 0.001559 | 0.001494 |
| pmr_list_test.test | RUN | 0.094340 | 0.208197 | 0.250432 | 0.001367 | 0.001609 | 0.001479 |
| pmr_hub_test.test | RUN | 0.138613 | 0.270251 | 0.319163 | 0.001475 | 0.001574 | 0.001544 |
| pmr_flat_set_test.test | RUN | 0.104679 | 0.205681 | 0.262973 | 0.001576 | 0.001489 | 0.001446 |
| pmr_flat_map_test.test | RUN | 0.124855 | 0.240605 | 0.338152 | 0.001322 | 0.001548 | 0.001570 |
| pmr_devector_test.test | RUN | 0.080578 | 0.213819 | 0.222002 | 0.001371 | 0.001333 | 0.001521 |
| pmr_deque_test.test | RUN | 0.080822 | 0.178955 | 0.221386 | 0.001251 | 0.001648 | 0.001595 |
| pair_test.test | RUN | 0.198453 | 0.402726 | 0.516149 | 0.001471 | 0.001599 | 0.001559 |
| null_iterators_test.test | RUN | 0.322421 | 0.792279 | 0.825619 | 0.001365 | 0.001586 | 0.001486 |
| node_handle_test.test | RUN | 0.148806 | 0.962276 | 0.362588 | 0.001412 | 0.001638 | 0.001488 |
| node_allocator_test.test | RUN | 1.804750 | 10.163909 | 8.367254 | 0.007746 | 0.007884 | 0.008062 |
| new_delete_resource_test.test | RUN | 0.185520 | 0.606445 | 0.485916 | 0.002272 | 0.002182 | 0.002072 |
| nest_test.test | RUN | 0.445955 | 2.799376 | 1.726040 | 0.001356 | 0.001764 | 0.001604 |
| nest_api_test.test | RUN | 0.903026 | 5.212860 | 3.385320 | 0.002320 | 0.002281 | 0.002144 |
| monotonic_buffer_resource_test.test | RUN | 0.211267 | 1.055706 | 0.505303 | 0.001555 | 0.001656 | 0.001445 |
| memory_resource_test.test | RUN | 0.117566 | 0.309797 | 0.289484 | 0.001493 | 0.001458 | 0.001541 |
| map_test.test | RUN | 3.736282 | 17.929577 | 17.146380 | 0.009034 | 0.008941 | 0.008705 |
| list_test.test | RUN | 1.719660 | 8.983088 | 7.156186 | 0.018559 | 0.018473 | 0.018579 |
| is_constructible_test.test | RUN | 0.069403 | 0.114868 | 0.159234 | 0.000678 | 0.000718 | 0.001454 |
| insert_vs_emplace_test.test | RUN | 0.193351 | 0.684460 | 0.917674 | 0.001487 | 0.001588 | 0.001619 |
| hub_uses_allocator_test.test | RUN | 0.223410 | 0.599429 | 0.628600 | 0.001604 | 0.001606 | 0.001442 |
| hub_stability_test.test | RUN | 0.438536 | 2.783898 | 2.383776 | 0.001934 | 0.001826 | 0.001809 |
| hub_sort_test.test | RUN | 0.290257 | 1.288897 | 1.164299 | 0.479163 | 0.441418 | 0.422978 |
| hub_pocxx_test.test | RUN | 0.289113 | 1.454580 | 0.824538 | 0.001804 | 0.001746 | 0.001632 |
| hub_new_extended_alignment_test.test | RUN | 0.184378 | 0.475782 | 0.447375 | 0.001700 | 0.001650 | 0.001516 |
| hub_exception_safety_test.test | RUN | 0.601023 | 3.660377 | 2.440592 | 0.068859 | 0.072757 | 0.066998 |
| hub_empty_sort_test.test | RUN | 0.181119 | 0.435143 | 0.524969 | 0.001598 | 0.001562 | 0.001485 |
| hub_api_test.test | RUN | 1.736154 | 14.323556 | 7.716134 | 0.003688 | 0.003246 | 0.002937 |
| global_resource_test.test | RUN | 0.109859 | 0.303138 | 0.261830 | 0.001690 | 0.001703 | 0.001439 |
| flat_tree_test.test | RUN | 2.990831 | 22.354121 | 14.850636 | 0.001446 | 0.001548 | 0.001610 |
| flat_set_test.test | RUN | 3.627511 | 27.677316 | 19.535152 | 0.007001 | 0.006219 | 0.006616 |
| flat_set_adaptor_test.test | RUN | 2.898647 | 24.223555 | 15.159771 | 0.006900 | 0.006141 | 0.006394 |
| flat_map_test.test | RUN | 6.328665 | 41.480261 | 32.291210 | 0.008430 | 0.007213 | 0.006850 |
| flat_map_adaptor_test.test | RUN | 4.087388 | 30.085359 | 22.484936 | 0.007977 | 0.006792 | 0.006776 |
| explicit_inst_vector_test.test | RUN | 0.632757 | 4.125424 | 3.414952 | 0.001544 | 0.001558 | 0.001460 |
| explicit_inst_string_test.test | RUN | 0.874180 | 7.403365 | 4.636399 | 0.001269 | 0.001310 | 0.001531 |
| explicit_inst_static_vector_test.test | RUN | 0.091912 | 0.273696 | 0.296913 | 0.001623 | 0.001592 | 0.001498 |
| explicit_inst_stable_vector_test.test | RUN | 0.569787 | 2.671478 | 2.002616 | 0.001366 | 0.001559 | 0.001583 |
| explicit_inst_small_vector_test.test | RUN | 0.458911 | 2.409888 | 1.860449 | 0.001595 | 0.001542 | 0.001491 |
| explicit_inst_slist_test.test | RUN | 0.339269 | 2.185176 | 1.470443 | 0.001506 | 0.001563 | 0.001522 |
| explicit_inst_set_test.test | RUN | 0.497875 | 2.527731 | 1.741696 | 0.001251 | 0.001658 | 0.001531 |
| explicit_inst_map_test.test | RUN | 0.557425 | 2.635071 | 1.873796 | 0.001609 | 0.001616 | 0.001494 |
| explicit_inst_list_test.test | RUN | 0.123935 | 0.605118 | 0.479049 | 0.001526 | 0.001584 | 0.001523 |
| explicit_inst_flat_set_test.test | RUN | 2.737902 | 18.391127 | 11.805756 | 0.001523 | 0.001632 | 0.001548 |
| explicit_inst_flat_map_test.test | RUN | 3.174262 | 19.357779 | 13.105320 | 0.001395 | 0.001438 | 0.001329 |
| explicit_inst_devector_test.test | RUN | 0.384048 | 2.329367 | 1.762305 | 0.001640 | 0.001535 | 0.001656 |
| explicit_inst_deque_test.test | RUN | 0.399589 | 2.518927 | 1.740693 | 0.001621 | 0.001564 | 0.001504 |
| devector_test.test | RUN | 5.438002 | 28.516993 | 20.512986 | 0.003298 | 0.002946 | 0.003633 |
| devector_options_test.test | RUN | 0.247933 | 1.353261 | 0.869307 | 0.001767 | 0.001766 | 0.001708 |
| deque_test.test | RUN | 3.852214 | 19.042067 | 14.941042 | 0.227440 | 0.296148 | 0.300744 |
| deque_options_test.test | RUN | 0.244808 | 0.520759 | 0.636247 | 0.001552 | 0.001722 | 0.001832 |
| copy_move_algo_test.test | RUN | 0.186533 | 0.609409 | 0.589068 | 0.001444 | 0.001629 | 0.001538 |
| common_iterator_test.test | RUN | 0.015997 | 0.034012 | 0.036583 | 0.000545 | 0.000579 | 0.001666 |
| boost_iterator_comp_test.test | RUN | 0.251694 | 0.722918 | 0.777626 | 0.001279 | 0.001540 | 0.001600 |
| allocator_traits_test.test | RUN | 0.156076 | 0.311196 | 0.343179 | 0.001486 | 0.001502 | 0.001556 |
| alloc_full_test.test | RUN | 0.162258 | 0.668923 | 0.588759 | 0.003460 | 0.003736 | 0.003565 |
| alloc_basic_test.test | RUN | 0.142234 | 0.478976 | 0.426805 | 0.001392 | 0.001748 | 0.001599 |
| adaptive_pool_test.test | RUN | 1.287133 | 10.828038 | 8.419850 | 0.007604 | 0.009963 | 0.010750 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.050× | 1.046×, 1.053× | 0.966× |
| clang | 1.014× | 1.008×, 1.020× | 0.980× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adaptive_pool_test.test | 3 | 0.008397s | 0.007170s | 0.008482s | byte equality observed |
| alloc_basic_test.test | 10 | 0.001002s | 0.001001s | 0.001106s | byte equality observed |
| alloc_full_test.test | 8 | 0.002720s | 0.002661s | 0.002892s | byte equality observed |
| allocator_traits_test.test | 10 | 0.000933s | 0.000922s | 0.000992s | byte equality observed |
| boost_iterator_comp_test.test | 10 | 0.000975s | 0.000961s | 0.001013s | byte equality observed |
| common_iterator_test.test | 10 | 0.000440s | 0.000427s | 0.000998s | byte equality observed |
| copy_move_algo_test.test | 10 | 0.001155s | 0.001045s | 0.001100s | byte equality observed |
| deque_options_test.test | 10 | 0.001061s | 0.000998s | 0.001090s | byte equality observed |
| deque_test.test | 1 | 0.256830s | 0.346584s | 0.349570s | byte equality observed |
| devector_options_test.test | 10 | 0.001151s | 0.001035s | 0.001122s | byte equality observed |
| devector_test.test | 6 | 0.003197s | 0.002871s | 0.003977s | byte equality observed |
| explicit_inst_deque_test.test | 10 | 0.000931s | 0.000891s | 0.001050s | byte equality observed |
| explicit_inst_devector_test.test | 10 | 0.000908s | 0.000867s | 0.000951s | byte equality observed |
| explicit_inst_flat_map_test.test | 10 | 0.000911s | 0.000881s | 0.000937s | byte equality observed |
| explicit_inst_flat_set_test.test | 10 | 0.000902s | 0.000880s | 0.000947s | byte equality observed |
| explicit_inst_list_test.test | 10 | 0.000924s | 0.000872s | 0.000940s | byte equality observed |
| explicit_inst_map_test.test | 10 | 0.000909s | 0.000881s | 0.000963s | byte equality observed |
| explicit_inst_set_test.test | 10 | 0.000919s | 0.000880s | 0.000956s | byte equality observed |
| explicit_inst_slist_test.test | 10 | 0.000918s | 0.000872s | 0.000935s | byte equality observed |
| explicit_inst_small_vector_test.test | 10 | 0.000917s | 0.000889s | 0.000935s | byte equality observed |
| explicit_inst_stable_vector_test.test | 10 | 0.000924s | 0.000885s | 0.000931s | byte equality observed |
| explicit_inst_static_vector_test.test | 10 | 0.000916s | 0.000874s | 0.000920s | byte equality observed |
| explicit_inst_string_test.test | 10 | 0.000905s | 0.000872s | 0.000939s | byte equality observed |
| explicit_inst_vector_test.test | 10 | 0.000899s | 0.000873s | 0.000931s | byte equality observed |
| flat_map_adaptor_test.test | 4 | 0.005117s | 0.004767s | 0.005174s | byte equality observed |
| flat_map_test.test | 4 | 0.005620s | 0.005103s | 0.005685s | byte equality observed |
| flat_set_adaptor_test.test | 4 | 0.004952s | 0.004379s | 0.005012s | byte equality observed |
| flat_set_test.test | 4 | 0.005341s | 0.004292s | 0.004964s | byte equality observed |
| flat_tree_test.test | 10 | 0.000943s | 0.000901s | 0.001049s | byte equality observed |
| global_resource_test.test | 10 | 0.000967s | 0.000950s | 0.000990s | byte equality observed |
| hub_api_test.test | 7 | 0.002608s | 0.002088s | 0.002208s | byte equality observed |
| hub_empty_sort_test.test | 10 | 0.000957s | 0.000901s | 0.001012s | byte equality observed |
| hub_exception_safety_test.test | 1 | 0.059543s | 0.055660s | 0.061139s | byte equality observed |
| hub_new_extended_alignment_test.test | 10 | 0.000923s | 0.001015s | 0.001007s | byte equality observed |
| hub_pocxx_test.test | 10 | 0.000997s | 0.000946s | 0.001046s | byte equality observed |
| hub_sort_test.test | 1 | 0.440507s | 0.417940s | 0.411816s | byte equality observed |
| hub_stability_test.test | 7 | 0.001354s | 0.001152s | 0.001327s | byte equality observed |
| hub_uses_allocator_test.test | 10 | 0.000948s | 0.000911s | 0.001038s | byte equality observed |
| insert_vs_emplace_test.test | 10 | 0.000956s | 0.000939s | 0.001027s | byte equality observed |
| is_constructible_test.test | 10 | 0.000432s | 0.000412s | 0.000955s | byte equality observed |
| list_test.test | 2 | 0.013206s | 0.013249s | 0.014028s | byte equality observed |
| map_test.test | 3 | 0.006488s | 0.006389s | 0.006883s | byte equality observed |
| memory_resource_test.test | 10 | 0.000980s | 0.000905s | 0.001090s | byte equality observed |
| monotonic_buffer_resource_test.test | 10 | 0.000950s | 0.000931s | 0.001029s | byte equality observed |
| nest_api_test.test | 10 | 0.001534s | 0.001479s | 0.001666s | byte equality observed |
| nest_test.test | 10 | 0.000985s | 0.000945s | 0.001091s | byte equality observed |
| new_delete_resource_test.test | 10 | 0.001356s | 0.001346s | 0.001556s | byte equality observed |
| node_allocator_test.test | 3 | 0.006051s | 0.005893s | 0.006579s | byte equality observed |
| node_handle_test.test | 10 | 0.000951s | 0.000950s | 0.001123s | byte equality observed |
| null_iterators_test.test | 10 | 0.000944s | 0.000909s | 0.001041s | byte equality observed |
| pair_test.test | 10 | 0.000925s | 0.000904s | 0.001005s | byte equality observed |
| pmr_deque_test.test | 10 | 0.000924s | 0.000887s | 0.000970s | byte equality observed |
| pmr_devector_test.test | 10 | 0.000936s | 0.000887s | 0.000962s | byte equality observed |
| pmr_flat_map_test.test | 10 | 0.000916s | 0.000876s | 0.000943s | byte equality observed |
| pmr_flat_set_test.test | 10 | 0.000918s | 0.000888s | 0.000945s | byte equality observed |
| pmr_hub_test.test | 10 | 0.000909s | 0.000893s | 0.000947s | byte equality observed |
| pmr_list_test.test | 10 | 0.000919s | 0.000904s | 0.000964s | byte equality observed |
| pmr_map_test.test | 10 | 0.000907s | 0.000895s | 0.000955s | byte equality observed |
| pmr_nest_test.test | 10 | 0.000911s | 0.000924s | 0.000982s | byte equality observed |
| pmr_set_test.test | 10 | 0.000918s | 0.000907s | 0.000959s | byte equality observed |
| pmr_slist_test.test | 10 | 0.000924s | 0.000923s | 0.000965s | byte equality observed |
| pmr_small_vector_test.test | 10 | 0.000921s | 0.000932s | 0.000976s | byte equality observed |
| pmr_stable_vector_test.test | 10 | 0.000945s | 0.000945s | 0.001002s | byte equality observed |
| pmr_string_test.test | 10 | 0.000448s | 0.000447s | 0.000967s | byte equality observed |
| pmr_vector_test.test | 10 | 0.000429s | 0.000455s | 0.000951s | byte equality observed |
| polymorphic_allocator_test.test | 10 | 0.001007s | 0.000983s | 0.001061s | byte equality observed |
| resource_adaptor_test.test | 10 | 0.000967s | 0.000941s | 0.001049s | byte equality observed |
| scoped_allocator_adaptor_test.test | 10 | 0.000995s | 0.000945s | 0.001025s | byte equality observed |
| scoped_allocator_usage_test.test | 10 | 0.000981s | 0.000943s | 0.000995s | byte equality observed |
| segtor_options_test.test | 10 | 0.001003s | 0.001002s | 0.001014s | byte equality observed |
| segtor_test.test | 1 | 0.306193s | 0.280181s | 0.278575s | byte equality observed |
| set_test.test | 2 | 0.008949s | 0.007849s | 0.009005s | byte equality observed |
| slist_test.test | 1 | 0.013345s | 0.013188s | 0.013706s | byte equality observed |
| small_vector_options_test.test | 9 | 0.001057s | 0.000985s | 0.001146s | byte equality observed |
| small_vector_test.test | 6 | 0.002056s | 0.001668s | 0.003002s | byte equality observed |
| stable_vector_test.test | 1 | 0.021262s | 0.021894s | 0.022625s | byte equality observed |
| static_vector_options_test.test | 10 | 0.001033s | 0.000954s | 0.001101s | byte equality observed |
| static_vector_test.test | 10 | 0.001283s | 0.001148s | 0.001264s | byte equality observed |
| string_options_test.test | 10 | 0.000986s | 0.000966s | 0.001006s | byte equality observed |
| string_test.test | 2 | 0.008381s | 0.008759s | 0.008983s | byte equality observed |
| string_view_compat_test.test | 10 | 0.000950s | 0.000947s | 0.001028s | byte equality observed |
| synchronized_pool_resource_test.test | 10 | 0.000980s | 0.000993s | 0.001029s | byte equality observed |
| throw_exception_test.test | 10 | 0.000917s | 0.000933s | 0.000939s | byte equality observed |
| tree_test.test | 10 | 0.000911s | 0.000905s | 0.000939s | byte equality observed |
| unsynchronized_pool_resource_test.test | 10 | 0.000962s | 0.000980s | 0.001007s | byte equality observed |
| uses_allocator_construction_test.test | 10 | 0.000904s | 0.000911s | 0.000961s | byte equality observed |
| uses_allocator_test.test | 10 | 0.000445s | 0.000430s | 0.000927s | byte equality observed |
| vector_options_test.test | 10 | 0.001091s | 0.001059s | 0.001169s | byte equality observed |
| vector_test.test | 4 | 0.003220s | 0.002950s | 0.004219s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
