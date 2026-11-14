# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.031576 | 0.051498 | 0.086830 | — | — | — |
| complex_h.test | COMPILE | 0.127490 | 0.223484 | 0.341594 | — | — | — |
| mp_size.test | RUN | 0.148301 | 0.722052 | 0.448718 | 0.001832 | 0.001457 | 0.001674 |
| mp_size_2.test | RUN | 0.013355 | 0.027404 | 0.044514 | 0.000690 | 0.000687 | 0.001773 |
| mp_empty.test | RUN | 0.136903 | 0.450638 | 0.432240 | 0.001571 | 0.001571 | 0.001554 |
| mp_empty_2.test | RUN | 0.014397 | 0.028486 | 0.048731 | 0.000646 | 0.000648 | 0.001822 |
| mp_assign.test | RUN | 0.169376 | 0.959515 | 0.608764 | 0.001575 | 0.001572 | 0.001988 |
| mp_assign_2.test | RUN | 0.014214 | 0.029125 | 0.050081 | 0.000586 | 0.000792 | 0.001489 |
| mp_clear.test | RUN | 0.138228 | 0.414605 | 0.452206 | 0.001572 | 0.001565 | 0.001732 |
| mp_clear_2.test | RUN | 0.013988 | 0.027837 | 0.048358 | 0.000633 | 0.000466 | 0.001690 |
| mp_front.test | RUN | 0.147705 | 0.635451 | 0.497501 | 0.001531 | 0.001524 | 0.001707 |
| mp_front_2.test | RUN | 0.013478 | 0.027438 | 0.053993 | 0.000592 | 0.000657 | 0.001645 |
| mp_pop_front.test | RUN | 0.145361 | 0.666067 | 0.528433 | 0.001390 | 0.001445 | 0.001684 |
| mp_pop_front_2.test | RUN | 0.014240 | 0.028723 | 0.045617 | 0.000580 | 0.000677 | 0.001430 |
| mp_second.test | RUN | 0.149286 | 0.423036 | 0.476089 | 0.001421 | 0.001303 | 0.002412 |
| mp_second_2.test | RUN | 0.013470 | 0.028947 | 0.046467 | 0.000714 | 0.000557 | 0.001733 |
| mp_third.test | RUN | 0.141570 | 0.393732 | 0.416875 | 0.001606 | 0.001616 | 0.001972 |
| mp_third_2.test | RUN | 0.013862 | 0.028326 | 0.045927 | 0.000612 | 0.000581 | 0.001658 |
| mp_push_front.test | RUN | 0.183823 | 1.445921 | 0.856254 | 0.001689 | 0.001592 | 0.001613 |
| mp_push_front_2.test | RUN | 0.013736 | 0.027293 | 0.066709 | 0.000591 | 0.000707 | 0.001944 |
| mp_push_back.test | RUN | 0.186166 | 1.454669 | 0.794659 | 0.001297 | 0.001531 | 0.001629 |
| mp_push_back_2.test | RUN | 0.013735 | 0.028602 | 0.051435 | 0.000574 | 0.000651 | 0.001765 |
| mp_rename.test | RUN | 0.169355 | 1.332782 | 0.693300 | 0.001497 | 0.001666 | 0.001649 |
| mp_rename_2.test | RUN | 0.013777 | 0.028211 | 0.048536 | 0.000638 | 0.000655 | 0.001724 |
| mp_append.test | RUN | 0.184151 | 1.522931 | 0.715168 | 0.001556 | 0.001501 | 0.001782 |
| mp_append_2.test | RUN | 0.350637 | 3.412028 | 1.005754 | 0.001504 | 0.001618 | 0.001706 |
| mp_append_3.test | RUN | 0.014446 | 0.029410 | 0.043801 | 0.000681 | 0.000712 | 0.001495 |
| mp_append_sf.test | RUN | 0.142166 | 0.380234 | 0.416040 | 0.001718 | 0.001519 | 0.001359 |
| mp_replace_front.test | RUN | 0.168545 | 1.121426 | 0.577532 | 0.001739 | 0.001654 | 0.001745 |
| mp_replace_front_2.test | RUN | 0.014031 | 0.028649 | 0.048405 | 0.000617 | 0.000572 | 0.001272 |
| mp_replace_second.test | RUN | 0.162259 | 0.716268 | 0.533218 | 0.001738 | 0.001506 | 0.002220 |
| mp_replace_second_2.test | RUN | 0.013502 | 0.028989 | 0.044283 | 0.000595 | 0.000671 | 0.001488 |
| mp_replace_third.test | RUN | 0.161011 | 0.606323 | 0.517710 | 0.001726 | 0.001536 | 0.001708 |
| mp_replace_third_2.test | RUN | 0.014176 | 0.029461 | 0.047018 | 0.000572 | 0.000702 | 0.001712 |
| mp_apply_q.test | RUN | 0.173328 | 1.325954 | 0.687074 | 0.001562 | 0.001609 | 0.001551 |
| mp_apply_q_sf.test | RUN | 0.141243 | 0.396922 | 0.417271 | 0.001706 | 0.001562 | 0.001835 |
| mp_is_list.test | RUN | 0.145074 | 0.463719 | 0.433659 | 0.001459 | 0.001665 | 0.001662 |
| mp_list_c.test | RUN | 0.150204 | 0.547395 | 0.491254 | 0.001792 | 0.001518 | 0.001886 |
| mp_transform_front.test | RUN | 0.163888 | 1.077431 | 0.619983 | 0.001452 | 0.002154 | 0.001646 |
| mp_transform_front_2.test | RUN | 0.014464 | 0.029346 | 0.043849 | 0.000750 | 0.000687 | 0.001731 |
| mp_transform_second.test | RUN | 0.156924 | 0.943448 | 0.571217 | 0.001702 | 0.001516 | 0.001647 |
| mp_transform_second_2.test | RUN | 0.014439 | 0.028579 | 0.052036 | 0.000544 | 0.000707 | 0.001740 |
| mp_transform_third.test | RUN | 0.146487 | 0.731378 | 0.497326 | 0.001537 | 0.001349 | 0.001423 |
| mp_transform_third_2.test | RUN | 0.014096 | 0.028173 | 0.044614 | 0.000568 | 0.000650 | 0.001553 |
| mp_list_v.test | RUN | 0.013818 | 0.028245 | 0.050640 | 0.000559 | 0.000594 | 0.001678 |
| mp_rename_v.test | RUN | 0.012581 | 0.026793 | 0.045490 | 0.000603 | 0.000561 | 0.001574 |
| mp_is_value_list.test | RUN | 0.139210 | 0.283822 | 0.404579 | 0.001724 | 0.001491 | 0.001814 |
| mp_transform.test | RUN | 0.299703 | 2.284740 | 1.294847 | 0.001560 | 0.001600 | 0.001720 |
| mp_transform_2.test | RUN | 0.134591 | 0.269397 | 0.377763 | 0.001449 | 0.001648 | 0.001698 |
| mp_transform_q.test | RUN | 0.291288 | 2.173699 | 1.235812 | 0.001872 | 0.001728 | 0.001818 |
| mp_transform_q_2.test | RUN | 0.329812 | 0.689922 | 1.678099 | 0.001898 | 0.001541 | 0.001266 |
| mp_transform_sf.test | RUN | 0.156635 | 0.395502 | 0.444928 | 0.001499 | 0.001571 | 0.001798 |
| mp_transform_if.test | RUN | 0.184595 | 1.084095 | 0.683299 | 0.001602 | 0.001568 | 0.001460 |
| mp_transform_if_q.test | RUN | 0.187985 | 1.082627 | 0.665987 | 0.001457 | 0.001477 | 0.001315 |
| mp_filter.test | RUN | 0.166483 | 0.647610 | 0.509935 | 0.001491 | 0.001484 | 0.001684 |
| mp_fill.test | RUN | 0.155582 | 0.491154 | 0.456543 | 0.001792 | 0.001540 | 0.001812 |
| mp_fill_2.test | RUN | 0.023708 | 0.044092 | 0.072732 | 0.000551 | 0.000682 | 0.001676 |
| mp_count.test | RUN | 0.293136 | 1.164834 | 0.730464 | 0.001638 | 0.001570 | 0.001461 |
| mp_count_if.test | RUN | 0.300536 | 1.176513 | 0.799646 | 0.001543 | 0.001372 | 0.001432 |
| mp_count_if_q.test | RUN | 0.159895 | 0.765363 | 0.474917 | 0.001784 | 0.001602 | 0.001794 |
| mp_contains.test | RUN | 0.150623 | 0.471184 | 0.444735 | 0.001494 | 0.001606 | 0.001898 |
| mp_starts_with.test | RUN | 0.152647 | 0.462189 | 0.431461 | 0.001538 | 0.001593 | 0.001685 |
| mp_starts_with_sf.test | RUN | 0.148572 | 0.389087 | 0.426110 | 0.001410 | 0.001544 | 0.001218 |
| mp_repeat.test | RUN | 0.217313 | 1.914361 | 0.855241 | 0.001833 | 0.001718 | 0.001710 |
| mp_repeat_2.test | RUN | 0.023846 | 0.040552 | 0.072194 | 0.000728 | 0.000626 | 0.001610 |
| mp_repeat_3.test | RUN | 0.190240 | 0.254538 | 0.422886 | 0.001736 | 0.001620 | 0.001767 |
| mp_product.test | RUN | 0.167677 | 1.161975 | 0.655626 | 0.001454 | 0.001523 | 0.001806 |
| mp_drop.test | RUN | 0.176435 | 1.476234 | 0.722445 | 0.001418 | 0.001447 | 0.001524 |
| mp_drop_2.test | RUN | 0.024070 | 0.044706 | 0.074746 | 0.000692 | 0.000650 | 0.001449 |
| mp_iota.test | RUN | 0.174645 | 1.201470 | 0.628964 | 0.001427 | 0.001578 | 0.001723 |
| mp_iota_2.test | RUN | 0.184290 | 1.471642 | 0.684909 | 0.001417 | 0.001547 | 0.001626 |
| mp_at.test | RUN | 0.154045 | 0.651384 | 0.510904 | 0.001397 | 0.001510 | 0.001833 |
| mp_at_2.test | RUN | 0.023226 | 0.040311 | 0.074106 | 0.000798 | 0.000628 | 0.001716 |
| mp_at_sf.test | RUN | 0.144177 | 0.395411 | 0.450291 | 0.001548 | 0.001558 | 0.001576 |
| mp_take.test | RUN | 0.238025 | 1.918283 | 0.838282 | 0.001774 | 0.001346 | 0.001802 |
| mp_take_2.test | RUN | 0.023922 | 0.043371 | 0.076251 | 0.000597 | 0.000653 | 0.001699 |
| mp_slice.test | RUN | 0.271922 | 1.968413 | 1.067625 | 0.001581 | 0.001581 | 0.001600 |
| mp_slice_2.test | RUN | 0.023224 | 0.042987 | 0.069030 | 0.000740 | 0.000602 | 0.001648 |
| mp_replace.test | RUN | 0.180424 | 0.997254 | 0.701983 | 0.001808 | 0.001308 | 0.001705 |
| mp_replace_if.test | RUN | 0.173480 | 0.883015 | 0.629292 | 0.001645 | 0.001528 | 0.001730 |
| mp_replace_if_q.test | RUN | 0.170460 | 0.868524 | 0.618373 | 0.001459 | 0.001565 | 0.001696 |
| mp_copy_if.test | RUN | 0.172113 | 1.196393 | 0.612004 | 0.001733 | 0.001582 | 0.001630 |
| mp_copy_if_q.test | RUN | 0.174328 | 1.210997 | 0.620234 | 0.001600 | 0.001456 | 0.001618 |
| mp_remove.test | RUN | 0.168868 | 0.826197 | 0.613924 | 0.001754 | 0.001644 | 0.001664 |
| mp_remove_if.test | RUN | 0.175809 | 1.297447 | 0.638576 | 0.001561 | 0.001648 | 0.001676 |
| mp_remove_if_q.test | RUN | 0.172740 | 1.291836 | 0.622152 | 0.001576 | 0.001576 | 0.001696 |
| mp_partition.test | RUN | 0.176665 | 1.221815 | 0.668065 | 0.001653 | 0.001727 | 0.001743 |
| mp_partition_q.test | RUN | 0.177025 | 1.225699 | 0.684529 | 0.001699 | 0.001599 | 0.001588 |
| mp_sort.test | RUN | 0.166338 | 0.846065 | 0.588550 | 0.001441 | 0.001588 | 0.001803 |
| mp_sort_q.test | RUN | 0.167610 | 0.854144 | 0.579014 | 0.001583 | 0.001428 | 0.001796 |
| mp_find.test | RUN | 0.167293 | 0.905019 | 0.537231 | 0.001766 | 0.001473 | 0.001832 |
| mp_find_if.test | RUN | 0.162927 | 0.846046 | 0.521332 | 0.001839 | 0.001650 | 0.001768 |
| mp_find_if_q.test | RUN | 0.173110 | 0.950807 | 0.593763 | 0.001765 | 0.001788 | 0.001794 |
| mp_reverse.test | RUN | 0.316814 | 2.556052 | 1.222202 | 0.001674 | 0.001568 | 0.001736 |
| mp_fold.test | RUN | 0.538513 | 2.039890 | 3.189479 | 0.001860 | 0.001546 | 0.001901 |
| mp_fold_q.test | RUN | 0.166905 | 0.866142 | 0.594504 | 0.001835 | 0.001505 | 0.001829 |
| mp_fold_q_sf.test | RUN | 0.149334 | 0.407895 | 0.452010 | 0.001491 | 0.003444 | 0.001597 |
| mp_reverse_fold.test | RUN | 0.268255 | 1.903981 | 0.841530 | 0.001464 | 0.001329 | 0.001557 |
| mp_reverse_fold_q.test | RUN | 0.174541 | 1.032729 | 0.582314 | 0.001590 | 0.001496 | 0.001744 |
| mp_unique.test | RUN | 0.156917 | 0.948913 | 0.594121 | 0.001642 | 0.001578 | 0.001657 |
| mp_unique_if.test | RUN | 0.288332 | 1.977943 | 0.984920 | 0.001824 | 0.001659 | 0.001443 |
| mp_unique_if_q.test | RUN | 0.234858 | 1.947355 | 0.758750 | 0.001812 | 0.001608 | 0.001719 |
| mp_all_of.test | RUN | 0.402701 | 1.075381 | 0.994790 | 0.001514 | 0.001517 | 0.001523 |
| mp_all_of_q.test | RUN | 0.149132 | 0.481706 | 0.421167 | 0.001628 | 0.001679 | 0.001862 |
| mp_any_of.test | RUN | 0.396348 | 1.115655 | 0.966587 | 0.001711 | 0.001557 | 0.001598 |
| mp_any_of_q.test | RUN | 0.153882 | 0.456189 | 0.417805 | 0.001950 | 0.001532 | 0.001751 |
| mp_none_of.test | RUN | 0.426202 | 1.119601 | 0.977227 | 0.001808 | 0.001623 | 0.001692 |
| mp_none_of_q.test | RUN | 0.154552 | 0.473902 | 0.431732 | 0.001508 | 0.001737 | 0.001724 |
| mp_replace_at.test | RUN | 0.182265 | 0.965730 | 0.650781 | 0.001807 | 0.001374 | 0.001688 |
| mp_replace_at_c.test | RUN | 0.184513 | 0.945397 | 0.670183 | 0.001605 | 0.001576 | 0.001415 |
| mp_for_each.test | RUN | 0.179184 | 0.665342 | 0.709480 | 0.001456 | 0.001752 | 0.001412 |
| mp_insert.test | RUN | 0.255903 | 1.994983 | 0.958467 | 0.001766 | 0.001504 | 0.001773 |
| mp_insert_2.test | RUN | 0.024132 | 0.043987 | 0.070826 | 0.000748 | 0.000664 | 0.001669 |
| mp_erase.test | RUN | 0.278159 | 2.103394 | 1.095862 | 0.001646 | 0.001632 | 0.001512 |
| mp_erase_2.test | RUN | 0.024429 | 0.043624 | 0.070002 | 0.000741 | 0.000683 | 0.001681 |
| mp_with_index.test | RUN | 0.347901 | 4.220356 | 2.425591 | 0.002278 | 0.002158 | 0.001810 |
| mp_with_index_cx.test | RUN | 0.003699 | 0.015450 | 0.024312 | 0.000756 | 0.000690 | 0.001570 |
| mp_from_sequence.test | RUN | 0.183114 | 1.116294 | 0.624155 | 0.001756 | 0.001547 | 0.001737 |
| mp_from_sequence_2.test | RUN | 0.193107 | 1.274882 | 0.714099 | 0.001827 | 0.001571 | 0.001776 |
| mp_from_sequence_3.test | RUN | 0.162202 | 0.577503 | 0.512833 | 0.001604 | 0.001716 | 0.001419 |
| mp_min_element.test | RUN | 0.152411 | 0.517865 | 0.476338 | 0.001508 | 0.001594 | 0.001689 |
| mp_min_element_2.test | RUN | 0.024491 | 0.044624 | 0.074309 | 0.000739 | 0.000644 | 0.001764 |
| mp_min_element_q.test | RUN | 0.154237 | 0.525735 | 0.452959 | 0.001594 | 0.001363 | 0.001485 |
| mp_min_element_q_2.test | RUN | 0.023358 | 0.043985 | 0.073416 | 0.000590 | 0.000641 | 0.001696 |
| mp_max_element.test | RUN | 0.158776 | 0.527439 | 0.472079 | 0.001688 | 0.001670 | 0.001459 |
| mp_max_element_2.test | RUN | 0.024427 | 0.044214 | 0.072453 | 0.000581 | 0.000663 | 0.001458 |
| mp_max_element_q.test | RUN | 0.156192 | 0.513612 | 0.456333 | 0.001599 | 0.001611 | 0.001740 |
| mp_max_element_q_2.test | RUN | 0.024372 | 0.042267 | 0.065489 | 0.000719 | 0.000688 | 0.001754 |
| mp_nth_element.test | RUN | 0.173020 | 0.716366 | 0.570334 | 0.001450 | 0.001461 | 0.001484 |
| mp_nth_element_q.test | RUN | 0.171583 | 0.706516 | 0.548715 | 0.001692 | 0.001326 | 0.001485 |
| mp_back.test | RUN | 0.162594 | 0.600999 | 0.504354 | 0.001731 | 0.001353 | 0.001966 |
| mp_back_2.test | RUN | 0.023933 | 0.043760 | 0.072415 | 0.000515 | 0.000651 | 0.001371 |
| mp_pop_back.test | RUN | 0.396362 | 3.899309 | 1.125517 | 0.001480 | 0.001439 | 0.001484 |
| mp_pop_back_2.test | RUN | 0.023544 | 0.041913 | 0.070618 | 0.000597 | 0.000569 | 0.001680 |
| mp_flatten.test | RUN | 0.192223 | 1.391251 | 0.759275 | 0.001588 | 0.001645 | 0.001500 |
| mp_rotate_left.test | RUN | 0.198059 | 1.496172 | 0.741284 | 0.001682 | 0.001346 | 0.001389 |
| mp_rotate_right.test | RUN | 0.191733 | 1.481343 | 0.766951 | 0.001525 | 0.001443 | 0.001636 |
| mp_power_set.test | RUN | 0.187807 | 1.395638 | 0.757797 | 0.001569 | 0.001596 | 0.001406 |
| mp_partial_sum.test | RUN | 0.233996 | 1.872543 | 0.861682 | 0.001598 | 0.001714 | 0.001680 |
| mp_iterate.test | RUN | 0.166921 | 0.551090 | 0.593361 | 0.001675 | 0.001528 | 0.001456 |
| mp_pairwise_fold.test | RUN | 0.187029 | 1.326991 | 0.751101 | 0.001540 | 0.001737 | 0.001627 |
| mp_pairwise_fold_q.test | RUN | 0.146270 | 0.388550 | 0.429343 | 0.001828 | 0.001483 | 0.001788 |
| mp_sliding_fold.test | RUN | 0.331357 | 2.562967 | 1.428913 | 0.001327 | 0.001574 | 0.001535 |
| mp_sliding_fold_q.test | RUN | 0.330541 | 2.541803 | 1.425164 | 0.001637 | 0.001618 | 0.001530 |
| mp_intersperse.test | RUN | 0.179581 | 0.959350 | 0.659606 | 0.001742 | 0.001664 | 0.001722 |
| mp_split.test | RUN | 0.229413 | 1.834169 | 0.940681 | 0.001689 | 0.001572 | 0.001287 |
| mp_join.test | RUN | 0.202597 | 1.518456 | 0.828572 | 0.001749 | 0.001465 | 0.001733 |
| integral.test | RUN | 0.159328 | 0.656957 | 0.483374 | 0.001586 | 0.001642 | 0.001622 |
| mp_value.test | RUN | 0.010939 | 0.024423 | 0.037026 | 0.000588 | 0.000579 | 0.001686 |
| mp_identity.test | RUN | 0.150707 | 0.613985 | 0.464385 | 0.001686 | 0.001538 | 0.001504 |
| mp_inherit.test | RUN | 0.146458 | 0.436877 | 0.434281 | 0.001780 | 0.001541 | 0.001825 |
| mp_if.test | RUN | 0.146753 | 0.507941 | 0.457690 | 0.001512 | 0.001505 | 0.001670 |
| mp_if_sf.test | RUN | 0.139693 | 0.370385 | 0.407126 | 0.001440 | 0.001418 | 0.001532 |
| mp_eval_if.test | RUN | 0.151019 | 0.543292 | 0.483026 | 0.001756 | 0.001351 | 0.001566 |
| mp_eval_if_sf.test | RUN | 0.140896 | 0.381337 | 0.370114 | 0.001774 | 0.001603 | 0.001593 |
| mp_valid.test | RUN | 0.139811 | 0.381934 | 0.402166 | 0.001700 | 0.001414 | 0.001559 |
| mp_defer.test | RUN | 0.154195 | 0.654421 | 0.470376 | 0.001496 | 0.001648 | 0.001616 |
| mp_quote.test | RUN | 0.156836 | 0.695090 | 0.506429 | 0.001762 | 0.001597 | 0.001535 |
| mp_invoke_q.test | RUN | 0.164746 | 0.889472 | 0.554616 | 0.001699 | 0.001632 | 0.001541 |
| mp_invoke_q_sf.test | RUN | 0.141449 | 0.381524 | 0.385789 | 0.001479 | 0.001725 | 0.001699 |
| mp_quote_trait.test | RUN | 0.153833 | 0.777979 | 0.482394 | 0.001520 | 0.001496 | 0.001419 |
| mp_cond.test | RUN | 0.149104 | 0.455708 | 0.419419 | 0.001739 | 0.001606 | 0.001602 |
| mp_cond_sf.test | RUN | 0.139915 | 0.387978 | 0.400650 | 0.001618 | 0.001688 | 0.001572 |
| mp_not_fn.test | RUN | 0.137891 | 0.385872 | 0.391063 | 0.001554 | 0.001694 | 0.001705 |
| mp_eval_if_not.test | RUN | 0.153280 | 0.544102 | 0.447387 | 0.001778 | 0.001646 | 0.001682 |
| mp_eval_or.test | RUN | 0.143011 | 0.301014 | 0.413696 | 0.001456 | 0.001572 | 0.001344 |
| mp_compose.test | RUN | 0.155090 | 0.875291 | 0.544278 | 0.001857 | 0.001666 | 0.001646 |
| mp_valid_and_true.test | RUN | 0.141151 | 0.452651 | 0.425117 | 0.001593 | 0.001684 | 0.001369 |
| mp_compose_sf.test | COMPILE | 0.032937 | 0.054932 | 0.094598 | — | — | — |
| integer_sequence.test | RUN | 0.168925 | 0.912488 | 0.585620 | 0.001480 | 0.001567 | 0.001224 |
| tuple_for_each.test | RUN | 0.145329 | 0.319662 | 0.436386 | 0.001527 | 0.001351 | 0.001598 |
| tuple_for_each_cx.test | COMPILE | 0.027165 | 0.048976 | 0.076994 | — | — | — |
| tuple_apply.test | RUN | 0.145606 | 0.336982 | 0.434427 | 0.001574 | 0.001382 | 0.001744 |
| tuple_apply_cx.test | COMPILE | 0.038107 | 0.066304 | 0.113152 | — | — | — |
| construct_from_tuple.test | RUN | 0.149039 | 0.350382 | 0.417037 | 0.001656 | 0.001385 | 0.001648 |
| construct_from_tuple_cx.test | COMPILE | 0.036130 | 0.067656 | 0.102783 | — | — | — |
| tuple_transform.test | RUN | 0.232069 | 0.705545 | 0.905055 | 0.001860 | 0.001389 | 0.001630 |
| tuple_transform_2.test | RUN | 0.209229 | 0.501705 | 0.825671 | 0.001645 | 0.001319 | 0.001354 |
| tuple_transform_cx.test | COMPILE | 0.034058 | 0.062610 | 0.100407 | — | — | — |
| mp_set_contains.test | RUN | 0.144918 | 0.463768 | 0.457864 | 0.001734 | 0.001469 | 0.001626 |
| mp_set_push_back.test | RUN | 0.192680 | 1.404374 | 0.800381 | 0.001604 | 0.001594 | 0.001694 |
| mp_set_push_front.test | RUN | 0.225016 | 1.678225 | 0.871088 | 0.001493 | 0.001538 | 0.001419 |
| mp_is_set.test | RUN | 0.145977 | 0.462790 | 0.463939 | 0.001538 | 0.001493 | 0.001798 |
| mp_set_union.test | RUN | 0.174991 | 1.182989 | 0.737426 | 0.001556 | 0.001632 | 0.001331 |
| mp_set_union_sf.test | RUN | 0.140673 | 0.360647 | 0.426258 | 0.001850 | 0.001655 | 0.001468 |
| mp_set_difference.test | RUN | 0.170402 | 0.943562 | 0.621129 | 0.001447 | 0.001594 | 0.001482 |
| mp_set_difference_sf.test | RUN | 0.149065 | 0.363176 | 0.406489 | 0.001399 | 0.001473 | 0.001356 |
| mp_set_intersection.test | RUN | 0.168954 | 0.919722 | 0.578836 | 0.001780 | 0.001553 | 0.001681 |
| mp_set_intersection_sf.test | RUN | 0.146736 | 0.361612 | 0.398629 | 0.001845 | 0.001347 | 0.001498 |
| mp_all.test | RUN | 0.149117 | 0.446017 | 0.436983 | 0.001770 | 0.001636 | 0.001522 |
| mp_all_2.test | RUN | 0.413806 | 0.979160 | 1.048382 | 0.001510 | 0.001725 | 0.001842 |
| mp_and.test | RUN | 0.146561 | 0.455745 | 0.416690 | 0.001743 | 0.001733 | 0.001734 |
| mp_any.test | RUN | 0.152182 | 0.471244 | 0.432475 | 0.002362 | 0.001736 | 0.001728 |
| mp_any_2.test | RUN | 0.428830 | 1.009919 | 1.025435 | 0.001526 | 0.001615 | 0.001390 |
| mp_or.test | RUN | 0.147160 | 0.476823 | 0.465811 | 0.001706 | 0.001805 | 0.001456 |
| mp_same.test | RUN | 0.279612 | 0.827101 | 0.702621 | 0.001560 | 0.001524 | 0.001488 |
| mp_plus.test | RUN | 0.156593 | 0.596458 | 0.466927 | 0.001838 | 0.001315 | 0.001627 |
| mp_less.test | RUN | 0.145126 | 0.470514 | 0.414885 | 0.001790 | 0.001406 | 0.001365 |
| mp_min.test | RUN | 0.139863 | 0.425461 | 0.412902 | 0.001733 | 0.001676 | 0.001772 |
| mp_max.test | RUN | 0.141983 | 0.471581 | 0.425769 | 0.001613 | 0.001607 | 0.001543 |
| mp_similar.test | RUN | 0.145389 | 0.454932 | 0.414588 | 0.001588 | 0.001318 | 0.001622 |
| mp_map_find.test | RUN | 0.167410 | 0.862539 | 0.549592 | 0.001656 | 0.001458 | 0.001555 |
| mp_map_find_2.test | RUN | 0.159980 | 0.335313 | 0.492084 | 0.001624 | 0.001389 | 0.001378 |
| mp_map_find_3.test | RUN | 0.155043 | 0.324113 | 0.490579 | 0.001516 | 0.001288 | 0.001277 |
| mp_map_find_4.test | RUN | 0.145814 | 0.306072 | 0.433359 | 0.001686 | 0.001424 | 0.001629 |
| mp_map_contains.test | RUN | 0.151658 | 0.477856 | 0.437744 | 0.001718 | 0.001652 | 0.001689 |
| mp_map_insert.test | RUN | 0.190421 | 1.610415 | 0.791156 | 0.001842 | 0.001632 | 0.001586 |
| mp_map_replace.test | RUN | 0.249578 | 1.821103 | 0.940962 | 0.001426 | 0.001642 | 0.001364 |
| mp_map_erase.test | RUN | 0.195165 | 1.337238 | 0.782674 | 0.001532 | 0.001590 | 0.001510 |
| mp_map_update.test | RUN | 0.172024 | 0.886462 | 0.596345 | 0.001818 | 0.001596 | 0.001435 |
| mp_map_update_q.test | RUN | 0.165600 | 0.879928 | 0.587284 | 0.001732 | 0.001549 | 0.001359 |
| mp_map_keys.test | RUN | 0.168510 | 0.781608 | 0.574371 | 0.001514 | 0.001147 | 0.001330 |
| mp_is_map.test | RUN | 0.153443 | 0.477560 | 0.453590 | 0.001570 | 0.001621 | 0.001446 |
| mp_bind.test | RUN | 0.185471 | 1.414197 | 0.684501 | 0.001770 | 0.001238 | 0.001710 |
| mp_bind_q.test | RUN | 0.165100 | 0.875481 | 0.546560 | 0.001776 | 0.001568 | 0.001742 |
| mp_bind_front.test | RUN | 0.157025 | 0.927163 | 0.552005 | 0.001564 | 0.001614 | 0.001443 |
| mp_bind_back.test | RUN | 0.160800 | 0.932684 | 0.549492 | 0.001410 | 0.001410 | 0.001314 |
| mp_lambda.test | RUN | 0.339179 | 2.896138 | 1.455330 | 0.001736 | 0.001164 | 0.001476 |
| mpl.test | RUN | 0.309578 | 0.761942 | 1.153496 | 0.001723 | 0.001590 | 0.001547 |
| mpl_list.test | RUN | 0.179555 | 0.566886 | 0.543380 | 0.001530 | 0.001427 | 0.001670 |
| mpl_tuple.test | RUN | 0.180774 | 0.569686 | 0.520043 | 0.001427 | 0.001372 | 0.001397 |
| version.test | RUN | 0.093543 | 0.200332 | 0.286367 | 0.001810 | 0.001540 | 0.001587 |
| mp11_version.test | RUN | 0.115985 | 0.237443 | 0.305386 | 0.001456 | 0.001490 | 0.001440 |
| list_version.test | RUN | 0.098571 | 0.204859 | 0.284570 | 0.001462 | 0.001601 | 0.001520 |
| algorithm_version.test | RUN | 0.116735 | 0.217898 | 0.293013 | 0.001656 | 0.001601 | 0.001513 |
| integral_version.test | RUN | 0.092777 | 0.208214 | 0.270102 | 0.001726 | 0.001428 | 0.001514 |
| utility_version.test | RUN | 0.097119 | 0.202151 | 0.277483 | 0.001408 | 0.001635 | 0.001391 |
| function_version.test | RUN | 0.099789 | 0.208734 | 0.282822 | 0.001407 | 0.001572 | 0.001494 |
| map_version.test | RUN | 0.111609 | 0.226812 | 0.297776 | 0.001772 | 0.001454 | 0.001277 |
| set_version.test | RUN | 0.097990 | 0.207460 | 0.274115 | 0.001857 | 0.001570 | 0.001593 |
| bind_version.test | RUN | 0.108035 | 0.217606 | 0.308219 | 0.001552 | 0.001582 | 0.001783 |
| integer_sequence_version.test | RUN | 0.093860 | 0.204963 | 0.222159 | 0.001454 | 0.001411 | 0.001329 |
| tuple_version.test | RUN | 0.106568 | 0.219066 | 0.271544 | 0.001424 | 0.001586 | 0.001193 |
| mpl_version.test | RUN | 0.080964 | 0.179707 | 0.280512 | 0.001374 | 0.001186 | 0.001092 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.011× | 1.000×, 1.022× | 1.010× |
| clang | 1.093× | 1.096×, 1.090× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000858s | 0.000847s | 0.000892s | byte equality observed |
| bind_version.test | 10 | 0.000864s | 0.000849s | 0.000902s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000867s | 0.000856s | 0.000943s | byte equality observed |
| function_version.test | 10 | 0.000889s | 0.000845s | 0.000902s | byte equality observed |
| integer_sequence.test | 10 | 0.000877s | 0.000860s | 0.000903s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000903s | 0.000882s | 0.000896s | byte equality observed |
| integral.test | 10 | 0.000890s | 0.000906s | 0.000889s | byte equality observed |
| integral_version.test | 10 | 0.000904s | 0.000869s | 0.000918s | byte equality observed |
| list_version.test | 10 | 0.000881s | 0.000895s | 0.000894s | byte equality observed |
| map_version.test | 10 | 0.000888s | 0.000892s | 0.000904s | byte equality observed |
| mp11_version.test | 10 | 0.000905s | 0.000906s | 0.000887s | byte equality observed |
| mp_all.test | 10 | 0.000889s | 0.000888s | 0.000915s | byte equality observed |
| mp_all_2.test | 10 | 0.000895s | 0.000916s | 0.000911s | byte equality observed |
| mp_all_of.test | 10 | 0.000889s | 0.000906s | 0.000917s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000896s | 0.000909s | 0.000907s | byte equality observed |
| mp_and.test | 10 | 0.000911s | 0.000921s | 0.000914s | byte equality observed |
| mp_any.test | 10 | 0.000882s | 0.000919s | 0.000906s | byte equality observed |
| mp_any_2.test | 10 | 0.000902s | 0.000902s | 0.000899s | byte equality observed |
| mp_any_of.test | 10 | 0.000902s | 0.000895s | 0.000896s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000900s | 0.000893s | 0.000897s | byte equality observed |
| mp_append.test | 10 | 0.000899s | 0.000899s | 0.000927s | byte equality observed |
| mp_append_2.test | 10 | 0.000936s | 0.000910s | 0.000930s | byte equality observed |
| mp_append_3.test | 10 | 0.000412s | 0.000410s | 0.000889s | byte equality observed |
| mp_append_sf.test | 10 | 0.000890s | 0.000925s | 0.000919s | byte equality observed |
| mp_apply_q.test | 10 | 0.000885s | 0.000907s | 0.000919s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000903s | 0.000905s | 0.000910s | byte equality observed |
| mp_assign.test | 10 | 0.000899s | 0.000908s | 0.000893s | byte equality observed |
| mp_assign_2.test | 10 | 0.000417s | 0.000403s | 0.000878s | byte equality observed |
| mp_at.test | 10 | 0.000915s | 0.000913s | 0.000912s | byte equality observed |
| mp_at_2.test | 10 | 0.000423s | 0.000404s | 0.000894s | byte equality observed |
| mp_at_sf.test | 10 | 0.000892s | 0.000890s | 0.000909s | byte equality observed |
| mp_back.test | 10 | 0.000909s | 0.000913s | 0.000921s | byte equality observed |
| mp_back_2.test | 10 | 0.000413s | 0.000412s | 0.000893s | byte equality observed |
| mp_bind.test | 10 | 0.000904s | 0.000911s | 0.000898s | byte equality observed |
| mp_bind_back.test | 10 | 0.000902s | 0.000900s | 0.000939s | byte equality observed |
| mp_bind_front.test | 10 | 0.000925s | 0.000890s | 0.000899s | byte equality observed |
| mp_bind_q.test | 10 | 0.000921s | 0.000902s | 0.000912s | byte equality observed |
| mp_clear.test | 10 | 0.000916s | 0.000907s | 0.000901s | byte equality observed |
| mp_clear_2.test | 10 | 0.000413s | 0.000407s | 0.000883s | byte equality observed |
| mp_compose.test | 10 | 0.000905s | 0.000910s | 0.000912s | byte equality observed |
| mp_cond.test | 10 | 0.000924s | 0.000908s | 0.000908s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000906s | 0.000900s | 0.000913s | byte equality observed |
| mp_contains.test | 10 | 0.000898s | 0.000889s | 0.000920s | byte equality observed |
| mp_copy_if.test | 10 | 0.000913s | 0.000896s | 0.000887s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000906s | 0.000912s | 0.000911s | byte equality observed |
| mp_count.test | 10 | 0.000918s | 0.000911s | 0.000920s | byte equality observed |
| mp_count_if.test | 10 | 0.000914s | 0.000909s | 0.000928s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000898s | 0.000895s | 0.000920s | byte equality observed |
| mp_defer.test | 10 | 0.000902s | 0.000889s | 0.000908s | byte equality observed |
| mp_drop.test | 10 | 0.000939s | 0.000900s | 0.000914s | byte equality observed |
| mp_drop_2.test | 10 | 0.000424s | 0.000417s | 0.000891s | byte equality observed |
| mp_empty.test | 10 | 0.000915s | 0.000926s | 0.000922s | byte equality observed |
| mp_empty_2.test | 10 | 0.000417s | 0.000419s | 0.000882s | byte equality observed |
| mp_erase.test | 10 | 0.000959s | 0.000918s | 0.000908s | byte equality observed |
| mp_erase_2.test | 10 | 0.000416s | 0.000420s | 0.000898s | byte equality observed |
| mp_eval_if.test | 10 | 0.000917s | 0.000926s | 0.000913s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000915s | 0.000903s | 0.000916s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000928s | 0.000907s | 0.000911s | byte equality observed |
| mp_eval_or.test | 10 | 0.000916s | 0.000922s | 0.000922s | byte equality observed |
| mp_fill.test | 10 | 0.000908s | 0.000915s | 0.000896s | byte equality observed |
| mp_fill_2.test | 10 | 0.000418s | 0.000414s | 0.000885s | byte equality observed |
| mp_filter.test | 10 | 0.000904s | 0.000923s | 0.000910s | byte equality observed |
| mp_find.test | 10 | 0.000904s | 0.000904s | 0.000887s | byte equality observed |
| mp_find_if.test | 10 | 0.000889s | 0.000911s | 0.000935s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000889s | 0.000901s | 0.000934s | byte equality observed |
| mp_flatten.test | 10 | 0.000889s | 0.000900s | 0.000901s | byte equality observed |
| mp_fold.test | 10 | 0.000931s | 0.000904s | 0.000928s | byte equality observed |
| mp_fold_q.test | 10 | 0.000907s | 0.000900s | 0.000895s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000900s | 0.000897s | 0.000911s | byte equality observed |
| mp_for_each.test | 10 | 0.000891s | 0.000891s | 0.000904s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000890s | 0.000930s | 0.000918s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000907s | 0.000887s | 0.000901s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000907s | 0.000893s | 0.000909s | byte equality observed |
| mp_front.test | 10 | 0.000908s | 0.000893s | 0.000891s | byte equality observed |
| mp_front_2.test | 10 | 0.000417s | 0.000413s | 0.000902s | byte equality observed |
| mp_identity.test | 10 | 0.000917s | 0.000886s | 0.000896s | byte equality observed |
| mp_if.test | 10 | 0.000928s | 0.000896s | 0.000917s | byte equality observed |
| mp_if_sf.test | 10 | 0.000904s | 0.000899s | 0.000899s | byte equality observed |
| mp_inherit.test | 10 | 0.000916s | 0.000870s | 0.000893s | byte equality observed |
| mp_insert.test | 10 | 0.000939s | 0.000929s | 0.000926s | byte equality observed |
| mp_insert_2.test | 10 | 0.000417s | 0.000406s | 0.000861s | byte equality observed |
| mp_intersperse.test | 10 | 0.000906s | 0.000890s | 0.000928s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000923s | 0.000896s | 0.000937s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000919s | 0.000905s | 0.000944s | byte equality observed |
| mp_iota.test | 10 | 0.000918s | 0.000902s | 0.000917s | byte equality observed |
| mp_iota_2.test | 10 | 0.000911s | 0.000929s | 0.000894s | byte equality observed |
| mp_is_list.test | 10 | 0.000912s | 0.000903s | 0.000904s | byte equality observed |
| mp_is_map.test | 10 | 0.000911s | 0.000924s | 0.000889s | byte equality observed |
| mp_is_set.test | 10 | 0.000915s | 0.000904s | 0.000914s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000906s | 0.000906s | 0.000927s | byte equality observed |
| mp_iterate.test | 10 | 0.000911s | 0.000905s | 0.000953s | byte equality observed |
| mp_join.test | 10 | 0.000921s | 0.000909s | 0.000931s | byte equality observed |
| mp_lambda.test | 10 | 0.000938s | 0.000928s | 0.000945s | byte equality observed |
| mp_less.test | 10 | 0.000911s | 0.000917s | 0.000918s | byte equality observed |
| mp_list_c.test | 10 | 0.000927s | 0.000903s | 0.000913s | byte equality observed |
| mp_list_v.test | 10 | 0.000419s | 0.000405s | 0.000873s | byte equality observed |
| mp_map_contains.test | 10 | 0.000912s | 0.000917s | 0.000891s | byte equality observed |
| mp_map_erase.test | 10 | 0.000902s | 0.000898s | 0.000920s | byte equality observed |
| mp_map_find.test | 10 | 0.000889s | 0.000886s | 0.000915s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000912s | 0.000906s | 0.000907s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000896s | 0.000919s | 0.000916s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000907s | 0.000891s | 0.000919s | byte equality observed |
| mp_map_insert.test | 10 | 0.000903s | 0.000912s | 0.000897s | byte equality observed |
| mp_map_keys.test | 10 | 0.000894s | 0.000904s | 0.000922s | byte equality observed |
| mp_map_replace.test | 10 | 0.000931s | 0.000909s | 0.000910s | byte equality observed |
| mp_map_update.test | 10 | 0.000907s | 0.000911s | 0.000911s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000899s | 0.000905s | 0.000908s | byte equality observed |
| mp_max.test | 10 | 0.000909s | 0.000892s | 0.000904s | byte equality observed |
| mp_max_element.test | 10 | 0.000912s | 0.000903s | 0.000896s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000415s | 0.000420s | 0.000878s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000918s | 0.000894s | 0.000894s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000417s | 0.000410s | 0.000897s | byte equality observed |
| mp_min.test | 10 | 0.000893s | 0.000889s | 0.000890s | byte equality observed |
| mp_min_element.test | 10 | 0.000913s | 0.000902s | 0.000919s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000416s | 0.000408s | 0.000885s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000922s | 0.000921s | 0.000904s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000418s | 0.000402s | 0.000887s | byte equality observed |
| mp_none_of.test | 10 | 0.000910s | 0.000904s | 0.000893s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000910s | 0.000920s | 0.000891s | byte equality observed |
| mp_not_fn.test | 10 | 0.000912s | 0.000929s | 0.000916s | byte equality observed |
| mp_nth_element.test | 10 | 0.000907s | 0.000925s | 0.000906s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000912s | 0.000931s | 0.000898s | byte equality observed |
| mp_or.test | 10 | 0.000905s | 0.000927s | 0.000919s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000897s | 0.000927s | 0.000920s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000906s | 0.000932s | 0.000923s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000933s | 0.000900s | 0.000930s | byte equality observed |
| mp_partition.test | 10 | 0.000897s | 0.000909s | 0.000927s | byte equality observed |
| mp_partition_q.test | 10 | 0.000905s | 0.000903s | 0.000913s | byte equality observed |
| mp_plus.test | 10 | 0.000913s | 0.000900s | 0.000908s | byte equality observed |
| mp_pop_back.test | 10 | 0.000949s | 0.000931s | 0.000887s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000406s | 0.000414s | 0.000904s | byte equality observed |
| mp_pop_front.test | 10 | 0.000936s | 0.000941s | 0.000915s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000418s | 0.000420s | 0.000867s | byte equality observed |
| mp_power_set.test | 10 | 0.000925s | 0.000941s | 0.000906s | byte equality observed |
| mp_product.test | 10 | 0.000916s | 0.000919s | 0.000906s | byte equality observed |
| mp_push_back.test | 10 | 0.000925s | 0.000923s | 0.000899s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000419s | 0.000413s | 0.000884s | byte equality observed |
| mp_push_front.test | 10 | 0.000908s | 0.000922s | 0.000887s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000420s | 0.000424s | 0.000872s | byte equality observed |
| mp_quote.test | 10 | 0.000903s | 0.000926s | 0.000901s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000913s | 0.000907s | 0.000910s | byte equality observed |
| mp_remove.test | 10 | 0.000891s | 0.000894s | 0.000904s | byte equality observed |
| mp_remove_if.test | 10 | 0.000915s | 0.000925s | 0.000901s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000904s | 0.000921s | 0.000888s | byte equality observed |
| mp_rename.test | 10 | 0.000919s | 0.000910s | 0.000905s | byte equality observed |
| mp_rename_2.test | 10 | 0.000422s | 0.000414s | 0.000888s | byte equality observed |
| mp_rename_v.test | 10 | 0.000418s | 0.000420s | 0.000887s | byte equality observed |
| mp_repeat.test | 10 | 0.000936s | 0.000946s | 0.000909s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000418s | 0.000423s | 0.000872s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000877s | 0.000905s | 0.000891s | byte equality observed |
| mp_replace.test | 10 | 0.000906s | 0.000889s | 0.000878s | byte equality observed |
| mp_replace_at.test | 10 | 0.000919s | 0.000905s | 0.000886s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000902s | 0.000888s | 0.000924s | byte equality observed |
| mp_replace_front.test | 10 | 0.000916s | 0.000935s | 0.000885s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000412s | 0.000422s | 0.000904s | byte equality observed |
| mp_replace_if.test | 10 | 0.000901s | 0.000928s | 0.000927s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000903s | 0.000924s | 0.000901s | byte equality observed |
| mp_replace_second.test | 10 | 0.000894s | 0.000924s | 0.000904s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000415s | 0.000419s | 0.000874s | byte equality observed |
| mp_replace_third.test | 10 | 0.000904s | 0.000921s | 0.000873s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000413s | 0.000407s | 0.000880s | byte equality observed |
| mp_reverse.test | 10 | 0.000923s | 0.000942s | 0.000904s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000914s | 0.000893s | 0.000884s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000902s | 0.000929s | 0.000894s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000894s | 0.000904s | 0.000888s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000891s | 0.000920s | 0.000880s | byte equality observed |
| mp_same.test | 10 | 0.000890s | 0.000930s | 0.000899s | byte equality observed |
| mp_second.test | 10 | 0.000913s | 0.000905s | 0.000909s | byte equality observed |
| mp_second_2.test | 10 | 0.000415s | 0.000410s | 0.000908s | byte equality observed |
| mp_set_contains.test | 10 | 0.000898s | 0.000914s | 0.000916s | byte equality observed |
| mp_set_difference.test | 10 | 0.000902s | 0.000902s | 0.000903s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000903s | 0.000914s | 0.000911s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000895s | 0.000911s | 0.000922s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000908s | 0.000927s | 0.000910s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000889s | 0.000914s | 0.000916s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000922s | 0.000916s | 0.000911s | byte equality observed |
| mp_set_union.test | 10 | 0.000901s | 0.000923s | 0.000881s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000896s | 0.000886s | 0.000911s | byte equality observed |
| mp_similar.test | 10 | 0.000910s | 0.000897s | 0.000899s | byte equality observed |
| mp_size.test | 10 | 0.000904s | 0.000894s | 0.000889s | byte equality observed |
| mp_size_2.test | 10 | 0.000421s | 0.000405s | 0.000881s | byte equality observed |
| mp_slice.test | 10 | 0.000934s | 0.000914s | 0.000911s | byte equality observed |
| mp_slice_2.test | 10 | 0.000414s | 0.000413s | 0.000855s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000914s | 0.000929s | 0.000910s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000922s | 0.000887s | 0.000932s | byte equality observed |
| mp_sort.test | 10 | 0.000904s | 0.000881s | 0.000905s | byte equality observed |
| mp_sort_q.test | 10 | 0.000917s | 0.000891s | 0.000906s | byte equality observed |
| mp_split.test | 10 | 0.000916s | 0.000916s | 0.000927s | byte equality observed |
| mp_starts_with.test | 10 | 0.000912s | 0.000883s | 0.000913s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000909s | 0.000934s | 0.000945s | byte equality observed |
| mp_take.test | 10 | 0.000930s | 0.000923s | 0.000905s | byte equality observed |
| mp_take_2.test | 10 | 0.000417s | 0.000427s | 0.000905s | byte equality observed |
| mp_third.test | 10 | 0.000926s | 0.000931s | 0.000912s | byte equality observed |
| mp_third_2.test | 10 | 0.000414s | 0.000422s | 0.000886s | byte equality observed |
| mp_transform.test | 10 | 0.000935s | 0.000943s | 0.000941s | byte equality observed |
| mp_transform_2.test | 10 | 0.000889s | 0.000886s | 0.000884s | byte equality observed |
| mp_transform_front.test | 10 | 0.000927s | 0.000938s | 0.000900s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000421s | 0.000413s | 0.000874s | byte equality observed |
| mp_transform_if.test | 10 | 0.000886s | 0.000924s | 0.000890s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000903s | 0.000941s | 0.000901s | byte equality observed |
| mp_transform_q.test | 10 | 0.000919s | 0.000928s | 0.000898s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000891s | 0.000907s | 0.000892s | byte equality observed |
| mp_transform_second.test | 10 | 0.000912s | 0.000936s | 0.000917s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000420s | 0.000409s | 0.000876s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000919s | 0.000895s | 0.000908s | byte equality observed |
| mp_transform_third.test | 10 | 0.000907s | 0.000902s | 0.000903s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000411s | 0.000420s | 0.000878s | byte equality observed |
| mp_unique.test | 10 | 0.000903s | 0.000903s | 0.000900s | byte equality observed |
| mp_unique_if.test | 10 | 0.000914s | 0.000906s | 0.000916s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000907s | 0.000905s | 0.000918s | byte equality observed |
| mp_valid.test | 10 | 0.000890s | 0.000909s | 0.000924s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000922s | 0.000896s | 0.000907s | byte equality observed |
| mp_value.test | 10 | 0.000415s | 0.000421s | 0.000902s | byte equality observed |
| mp_with_index.test | 10 | 0.001318s | 0.001293s | 0.001238s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000409s | 0.000409s | 0.000899s | byte equality observed |
| mpl.test | 10 | 0.000917s | 0.000907s | 0.000920s | byte equality observed |
| mpl_list.test | 10 | 0.000901s | 0.000926s | 0.000917s | byte equality observed |
| mpl_tuple.test | 10 | 0.000890s | 0.000916s | 0.000904s | byte equality observed |
| mpl_version.test | 10 | 0.000923s | 0.000904s | 0.000911s | byte equality observed |
| set_version.test | 10 | 0.000907s | 0.000905s | 0.000930s | byte equality observed |
| tuple_apply.test | 10 | 0.000924s | 0.000908s | 0.000919s | byte equality observed |
| tuple_for_each.test | 10 | 0.000905s | 0.000920s | 0.000918s | byte equality observed |
| tuple_transform.test | 10 | 0.000892s | 0.000925s | 0.000913s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000889s | 0.000923s | 0.000924s | byte equality observed |
| tuple_version.test | 10 | 0.000884s | 0.000914s | 0.000921s | byte equality observed |
| utility_version.test | 10 | 0.000910s | 0.000900s | 0.000903s | byte equality observed |
| version.test | 10 | 0.000883s | 0.000917s | 0.000913s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
