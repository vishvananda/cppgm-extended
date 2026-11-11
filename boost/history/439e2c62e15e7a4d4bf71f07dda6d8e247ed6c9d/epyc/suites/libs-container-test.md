# libs/container/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| vector_test.test | RUN | 0.973016 | 9.426652 | 5.654769 | 0.019595 | 0.019830 | 0.021299 |
| vector_options_test.test | RUN | 0.189221 | 0.686636 | 0.607371 | 0.001777 | 0.001727 | 0.001625 |
| uses_allocator_test.test | RUN | 0.046385 | 0.080768 | 0.101320 | 0.000674 | 0.000701 | 0.001410 |
| uses_allocator_construction_test.test | RUN | 0.128676 | 0.315360 | 0.334838 | 0.001448 | 0.001673 | 0.001460 |
| unsynchronized_pool_resource_test.test | RUN | 0.156971 | 0.353744 | 0.380446 | 0.001556 | 0.001510 | 0.001598 |
| tree_test.test | RUN | 0.419760 | 2.354292 | 1.653069 | 0.001517 | 0.001611 | 0.001434 |
| throw_exception_test.test | RUN | 0.094514 | 0.172875 | 0.236410 | 0.001671 | 0.001660 | 0.001492 |
| synchronized_pool_resource_test.test | RUN | 0.152766 | 0.347340 | 0.405166 | 0.001712 | 0.001745 | 0.001592 |
| string_view_compat_test.test | RUN | 0.254043 | 0.554812 | 0.617319 | 0.001529 | 0.001656 | 0.001545 |
| string_test.test | RUN | 0.487592 | 2.741408 | 1.986066 | 0.086007 | 0.099248 | 0.095451 |
| string_options_test.test | RUN | 0.166748 | 0.555559 | 0.472547 | 0.002094 | 0.002184 | 0.002248 |
| static_vector_test.test | RUN | 0.466767 | 3.749745 | 2.401562 | 0.002569 | 0.002528 | 0.002603 |
| static_vector_options_test.test | RUN | 0.163321 | 0.418424 | 0.432403 | 0.001508 | 0.001778 | 0.001639 |
| stable_vector_test.test | RUN | 0.722067 | 6.643400 | 3.876881 | 0.151050 | 0.159171 | 0.155356 |
| small_vector_test.test | RUN | 0.557730 | 4.280932 | 2.829744 | 0.012225 | 0.011837 | 0.013159 |
| small_vector_options_test.test | RUN | 0.233953 | 0.931854 | 0.770229 | 0.001824 | 0.001812 | 0.001669 |
| slist_test.test | RUN | 0.556421 | 4.552424 | 2.909780 | 0.109525 | 0.120810 | 0.113554 |
| set_test.test | RUN | 0.877445 | 7.139072 | 4.398458 | 0.061708 | 0.067580 | 0.066994 |
| segtor_test.test | RUN | 0.843874 | 7.138753 | 4.175144 | 5.782871 | 5.783673 | 6.143686 |
| segtor_options_test.test | RUN | 0.163936 | 0.445711 | 0.458467 | 0.001560 | 0.001727 | 0.001561 |
| scoped_allocator_usage_test.test | RUN | 0.395927 | 2.049583 | 1.564992 | 0.001350 | 0.001672 | 0.001573 |
| scoped_allocator_adaptor_test.test | RUN | 0.280403 | 0.935940 | 0.931608 | 0.001546 | 0.001466 | 0.001464 |
| resource_adaptor_test.test | RUN | 0.144260 | 0.336788 | 0.386809 | 0.001299 | 0.001559 | 0.001424 |
| polymorphic_allocator_test.test | RUN | 0.124537 | 0.275623 | 0.335233 | 0.001212 | 0.001302 | 0.001295 |
| pmr_vector_test.test | RUN | 0.073073 | 0.125901 | 0.176577 | 0.000555 | 0.000694 | 0.001568 |
| pmr_string_test.test | RUN | 0.145696 | 0.262856 | 0.328073 | 0.000503 | 0.000657 | 0.001556 |
| pmr_stable_vector_test.test | RUN | 0.111883 | 0.422631 | 0.337807 | 0.001352 | 0.001656 | 0.001336 |
| pmr_small_vector_test.test | RUN | 0.085394 | 0.192127 | 0.208746 | 0.001198 | 0.001490 | 0.001149 |
| pmr_slist_test.test | RUN | 0.091454 | 0.322448 | 0.313914 | 0.001406 | 0.001387 | 0.001328 |
| pmr_set_test.test | RUN | 0.115735 | 0.323619 | 0.286514 | 0.001199 | 0.001496 | 0.001430 |
| pmr_nest_test.test | RUN | 0.106304 | 0.192458 | 0.224938 | 0.001376 | 0.001197 | 0.001337 |
| pmr_map_test.test | RUN | 0.133954 | 0.313427 | 0.344787 | 0.001340 | 0.001379 | 0.001117 |
| pmr_list_test.test | RUN | 0.097495 | 0.184636 | 0.236878 | 0.001559 | 0.001430 | 0.001299 |
| pmr_hub_test.test | RUN | 0.132558 | 0.243018 | 0.252113 | 0.001282 | 0.001368 | 0.001302 |
| pmr_flat_set_test.test | RUN | 0.102605 | 0.215310 | 0.238914 | 0.001261 | 0.001518 | 0.001334 |
| pmr_flat_map_test.test | RUN | 0.122240 | 0.255893 | 0.277268 | 0.001239 | 0.001342 | 0.001267 |
| pmr_devector_test.test | RUN | 0.074098 | 0.178699 | 0.170719 | 0.001464 | 0.001618 | 0.001323 |
| pmr_deque_test.test | RUN | 0.081169 | 0.170640 | 0.176583 | 0.001353 | 0.001381 | 0.001295 |
| pair_test.test | RUN | 0.188644 | 0.413461 | 0.491614 | 0.001506 | 0.001471 | 0.001292 |
| null_iterators_test.test | RUN | 0.300137 | 0.827094 | 0.796241 | 0.001579 | 0.001290 | 0.001439 |
| node_handle_test.test | RUN | 0.124093 | 0.271700 | 0.266507 | 0.001464 | 0.001375 | 0.001419 |
| node_allocator_test.test | RUN | 0.508962 | 4.193868 | 2.188697 | 0.046651 | 0.041070 | 0.041090 |
| new_delete_resource_test.test | RUN | 0.163847 | 0.371915 | 0.400033 | 0.002201 | 0.002062 | 0.002034 |
| nest_test.test | RUN | 0.247542 | 1.125726 | 0.887477 | 0.001940 | 0.001692 | 0.001698 |
| nest_api_test.test | RUN | 0.346907 | 1.971028 | 1.382006 | 0.003860 | 0.004303 | 0.004357 |
| monotonic_buffer_resource_test.test | RUN | 0.162332 | 0.320233 | 0.386506 | 0.001552 | 0.001445 | 0.001463 |
| memory_resource_test.test | RUN | 0.100498 | 0.181139 | 0.234725 | 0.001230 | 0.001578 | 0.001182 |
| map_test.test | RUN | 1.089119 | 8.431369 | 4.742905 | 0.055761 | 0.058385 | 0.056262 |
| list_test.test | RUN | 0.567011 | 4.597010 | 3.002910 | 0.105584 | 0.108764 | 0.101692 |
| is_constructible_test.test | RUN | 0.068147 | 0.108024 | 0.149889 | 0.000711 | 0.000607 | 0.001414 |
| insert_vs_emplace_test.test | RUN | 0.163468 | 0.410480 | 0.445861 | 0.001707 | 0.001578 | 0.001530 |
| hub_uses_allocator_test.test | RUN | 0.217072 | 0.578546 | 0.549243 | 0.001509 | 0.001664 | 0.001460 |
| hub_stability_test.test | RUN | 0.291841 | 1.597730 | 1.221484 | 0.002771 | 0.002985 | 0.002744 |
| hub_sort_test.test | RUN | 0.187170 | 0.704499 | 0.612615 | 2.215327 | 2.107566 | 2.125792 |
| hub_pocxx_test.test | RUN | 0.195170 | 0.669365 | 0.579202 | 0.001474 | 0.001829 | 0.001472 |
| hub_new_extended_alignment_test.test | RUN | 0.142203 | 0.304286 | 0.366418 | 0.001464 | 0.001592 | 0.001341 |
| hub_exception_safety_test.test | RUN | 0.279126 | 1.544036 | 1.152125 | 0.141681 | 0.152910 | 0.178563 |
| hub_empty_sort_test.test | RUN | 0.152045 | 0.360171 | 0.391351 | 0.001428 | 0.001578 | 0.001544 |
| hub_api_test.test | RUN | 0.705121 | 6.532460 | 3.792315 | 0.010158 | 0.008977 | 0.009959 |
| global_resource_test.test | RUN | 0.102152 | 0.194676 | 0.220061 | 0.001498 | 0.001613 | 0.001572 |
| flat_tree_test.test | RUN | 0.773898 | 7.012714 | 3.936845 | 0.001678 | 0.001548 | 0.001482 |
| flat_set_test.test | RUN | 0.899422 | 8.210381 | 4.554282 | 0.020982 | 0.019922 | 0.020260 |
| flat_set_adaptor_test.test | RUN | 0.650436 | 5.535965 | 3.053208 | 0.022034 | 0.022876 | 0.022713 |
| flat_map_test.test | RUN | 1.317717 | 13.927324 | 7.474915 | 0.026248 | 0.031402 | 0.027253 |
| flat_map_adaptor_test.test | RUN | 0.868817 | 8.399552 | 4.644245 | 0.027614 | 0.031777 | 0.028245 |
| explicit_inst_vector_test.test | RUN | 0.347029 | 1.757731 | 1.198935 | 0.001672 | 0.001563 | 0.001449 |
| explicit_inst_string_test.test | RUN | 0.381446 | 2.036272 | 1.376365 | 0.001432 | 0.001672 | 0.001490 |
| explicit_inst_static_vector_test.test | RUN | 0.092383 | 0.272433 | 0.244046 | 0.001485 | 0.001656 | 0.001471 |
| explicit_inst_stable_vector_test.test | RUN | 0.321336 | 1.650675 | 1.163489 | 0.001622 | 0.001638 | 0.001530 |
| explicit_inst_small_vector_test.test | RUN | 0.304768 | 1.511135 | 1.069380 | 0.001448 | 0.001653 | 0.001499 |
| explicit_inst_slist_test.test | RUN | 0.283162 | 1.139723 | 0.897790 | 0.001384 | 0.001658 | 0.001599 |
| explicit_inst_set_test.test | RUN | 0.357039 | 1.574630 | 1.184755 | 0.001675 | 0.001652 | 0.001588 |
| explicit_inst_map_test.test | RUN | 0.380043 | 1.822961 | 1.300043 | 0.001704 | 0.001525 | 0.001481 |
| explicit_inst_list_test.test | RUN | 0.104898 | 0.387132 | 0.330191 | 0.001664 | 0.001528 | 0.001392 |
| explicit_inst_flat_set_test.test | RUN | 0.641845 | 5.209850 | 2.745553 | 0.001536 | 0.001741 | 0.001463 |
| explicit_inst_flat_map_test.test | RUN | 0.722050 | 5.864778 | 3.146943 | 0.001734 | 0.001834 | 0.001705 |
| explicit_inst_devector_test.test | RUN | 0.193594 | 0.895189 | 0.649545 | 0.001661 | 0.001613 | 0.001552 |
| explicit_inst_deque_test.test | RUN | 0.263570 | 1.171720 | 0.876516 | 0.001488 | 0.001689 | 0.001364 |
| devector_test.test | RUN | 1.140401 | 9.563065 | 5.072249 | 0.016289 | 0.015901 | 0.011460 |
| devector_options_test.test | RUN | 0.199054 | 0.716305 | 0.571613 | 0.004120 | 0.003538 | 0.003163 |
| deque_test.test | RUN | 0.907617 | 7.526087 | 4.579962 | 5.776147 | 5.771300 | 5.482817 |
| deque_options_test.test | RUN | 0.186409 | 0.492148 | 0.477177 | 0.001913 | 0.001944 | 0.001284 |
| copy_move_algo_test.test | RUN | 0.165254 | 0.398074 | 0.404488 | 0.002834 | 0.002868 | 0.002628 |
| common_iterator_test.test | RUN | 0.017219 | 0.030810 | 0.036716 | 0.000726 | 0.000665 | 0.001485 |
| boost_iterator_comp_test.test | RUN | 0.255245 | 0.748212 | 0.674959 | 0.001478 | 0.001685 | 0.001414 |
| allocator_traits_test.test | RUN | 0.170865 | 0.338924 | 0.393070 | 0.001456 | 0.001588 | 0.001467 |
| alloc_full_test.test | RUN | 0.138542 | 0.459321 | 0.389623 | 0.006180 | 0.007811 | 0.006182 |
| alloc_basic_test.test | RUN | 0.122411 | 0.424065 | 0.349074 | 0.001876 | 0.002152 | 0.001753 |
| adaptive_pool_test.test | RUN | 0.436235 | 4.495248 | 2.425458 | 0.078418 | 0.075261 | 0.071313 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
