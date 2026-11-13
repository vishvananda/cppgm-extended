# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.036042 | 0.071447 | 0.123013 | — | — | — |
| complex_h.test | COMPILE | 0.146954 | 0.315351 | 0.521175 | — | — | — |
| mp_size.test | RUN | 0.169238 | 0.967158 | 0.619172 | 0.001572 | 0.001765 | 0.001534 |
| mp_size_2.test | RUN | 0.015744 | 0.036496 | 0.059465 | 0.000716 | 0.000630 | 0.001605 |
| mp_empty.test | RUN | 0.159198 | 0.612448 | 0.565570 | 0.001224 | 0.002016 | 0.001760 |
| mp_empty_2.test | RUN | 0.015371 | 0.038036 | 0.061749 | 0.000576 | 0.000774 | 0.001424 |
| mp_assign.test | RUN | 0.188307 | 1.228333 | 0.719317 | 0.001562 | 0.001700 | 0.000988 |
| mp_assign_2.test | RUN | 0.015670 | 0.035495 | 0.059160 | 0.000658 | 0.000801 | 0.001772 |
| mp_clear.test | RUN | 0.162545 | 0.567492 | 0.595739 | 0.001466 | 0.001628 | 0.001832 |
| mp_clear_2.test | RUN | 0.015574 | 0.035117 | 0.060152 | 0.000616 | 0.000592 | 0.001746 |
| mp_front.test | RUN | 0.167278 | 0.870622 | 0.669595 | 0.001623 | 0.001730 | 0.001827 |
| mp_front_2.test | RUN | 0.015502 | 0.035344 | 0.057343 | 0.000632 | 0.000735 | 0.001263 |
| mp_pop_front.test | RUN | 0.169703 | 0.896944 | 0.625811 | 0.001430 | 0.001617 | 0.001780 |
| mp_pop_front_2.test | RUN | 0.015431 | 0.038422 | 0.055117 | 0.000601 | 0.000754 | 0.001908 |
| mp_second.test | RUN | 0.168914 | 0.578081 | 0.606477 | 0.001339 | 0.001667 | 0.001810 |
| mp_second_2.test | RUN | 0.016102 | 0.037767 | 0.059567 | 0.000722 | 0.000820 | 0.001825 |
| mp_third.test | RUN | 0.166400 | 0.547272 | 0.570791 | 0.001672 | 0.001698 | 0.001779 |
| mp_third_2.test | RUN | 0.015560 | 0.037249 | 0.060949 | 0.000635 | 0.000772 | 0.001804 |
| mp_push_front.test | RUN | 0.221338 | 1.899317 | 0.955818 | 0.001620 | 0.001722 | 0.001944 |
| mp_push_front_2.test | RUN | 0.015772 | 0.037763 | 0.054812 | 0.000584 | 0.000786 | 0.001625 |
| mp_push_back.test | RUN | 0.219022 | 1.888658 | 1.045384 | 0.001632 | 0.001651 | 0.001746 |
| mp_push_back_2.test | RUN | 0.014895 | 0.037855 | 0.060046 | 0.000723 | 0.000756 | 0.001803 |
| mp_rename.test | RUN | 0.202758 | 1.734772 | 0.919659 | 0.001331 | 0.001834 | 0.001765 |
| mp_rename_2.test | RUN | 0.013881 | 0.037709 | 0.059504 | 0.000603 | 0.000767 | 0.001830 |
| mp_append.test | RUN | 0.216782 | 1.969566 | 0.983289 | 0.001742 | 0.001778 | 0.001839 |
| mp_append_2.test | RUN | 0.445090 | 4.401138 | 1.312384 | 0.001689 | 0.002070 | 0.001821 |
| mp_append_3.test | RUN | 0.013875 | 0.038716 | 0.054364 | 0.000726 | 0.000803 | 0.001854 |
| mp_append_sf.test | RUN | 0.162294 | 0.510807 | 0.545672 | 0.001706 | 0.001742 | 0.001888 |
| mp_replace_front.test | RUN | 0.190262 | 1.399348 | 0.802506 | 0.001583 | 0.001657 | 0.001336 |
| mp_replace_front_2.test | RUN | 0.016471 | 0.038266 | 0.054046 | 0.000567 | 0.000780 | 0.001736 |
| mp_replace_second.test | RUN | 0.179414 | 0.906760 | 0.704168 | 0.001726 | 0.001817 | 0.001896 |
| mp_replace_second_2.test | RUN | 0.016273 | 0.038286 | 0.058723 | 0.000612 | 0.000806 | 0.001562 |
| mp_replace_third.test | RUN | 0.178491 | 0.757058 | 0.684579 | 0.001710 | 0.001848 | 0.001707 |
| mp_replace_third_2.test | RUN | 0.016042 | 0.037976 | 0.056395 | 0.000685 | 0.000631 | 0.001737 |
| mp_apply_q.test | RUN | 0.206028 | 1.725239 | 0.847942 | 0.001802 | 0.001829 | 0.001792 |
| mp_apply_q_sf.test | RUN | 0.163277 | 0.506381 | 0.546968 | 0.001561 | 0.001878 | 0.001446 |
| mp_is_list.test | RUN | 0.159343 | 0.609667 | 0.581966 | 0.001721 | 0.001790 | 0.001681 |
| mp_list_c.test | RUN | 0.172242 | 0.731961 | 0.598194 | 0.001644 | 0.001795 | 0.001646 |
| mp_transform_front.test | RUN | 0.191363 | 1.421354 | 0.797377 | 0.001703 | 0.001832 | 0.001860 |
| mp_transform_front_2.test | RUN | 0.015590 | 0.037706 | 0.059761 | 0.000578 | 0.000748 | 0.001826 |
| mp_transform_second.test | RUN | 0.180621 | 1.227126 | 0.735611 | 0.001796 | 0.001708 | 0.001626 |
| mp_transform_second_2.test | RUN | 0.016559 | 0.037122 | 0.057145 | 0.000650 | 0.000781 | 0.001587 |
| mp_transform_third.test | RUN | 0.178136 | 0.927568 | 0.625614 | 0.001651 | 0.001816 | 0.001894 |
| mp_transform_third_2.test | RUN | 0.016558 | 0.040963 | 0.058036 | 0.000615 | 0.000794 | 0.001789 |
| mp_list_v.test | RUN | 0.016051 | 0.037721 | 0.055738 | 0.000625 | 0.000764 | 0.001489 |
| mp_rename_v.test | RUN | 0.015775 | 0.037548 | 0.061586 | 0.000597 | 0.000761 | 0.001826 |
| mp_is_value_list.test | RUN | 0.161124 | 0.378145 | 0.508139 | 0.001862 | 0.001944 | 0.001779 |
| mp_transform.test | RUN | 0.358059 | 2.993423 | 1.634534 | 0.001364 | 0.001639 | 0.001607 |
| mp_transform_2.test | RUN | 0.151169 | 0.366270 | 0.550063 | 0.001442 | 0.001916 | 0.001832 |
| mp_transform_q.test | RUN | 0.353703 | 2.847427 | 1.578521 | 0.001805 | 0.001728 | 0.001833 |
| mp_transform_q_2.test | RUN | 0.396417 | 0.936677 | 2.108714 | 0.001638 | 0.001876 | 0.001616 |
| mp_transform_sf.test | RUN | 0.169116 | 0.532424 | 0.609382 | 0.001712 | 0.001552 | 0.001829 |
| mp_transform_if.test | RUN | 0.204990 | 1.422799 | 0.890753 | 0.001354 | 0.001810 | 0.001604 |
| mp_transform_if_q.test | RUN | 0.213392 | 1.448227 | 0.876722 | 0.001302 | 0.001600 | 0.001578 |
| mp_filter.test | RUN | 0.188637 | 0.840872 | 0.695243 | 0.001744 | 0.001852 | 0.001650 |
| mp_fill.test | RUN | 0.178007 | 0.658624 | 0.672599 | 0.001750 | 0.001804 | 0.001843 |
| mp_fill_2.test | RUN | 0.027060 | 0.057049 | 0.091553 | 0.000639 | 0.000731 | 0.001854 |
| mp_count.test | RUN | 0.349581 | 1.522676 | 0.997961 | 0.001497 | 0.001792 | 0.001573 |
| mp_count_if.test | RUN | 0.345431 | 1.540853 | 1.158274 | 0.001554 | 0.001871 | 0.001948 |
| mp_count_if_q.test | RUN | 0.180579 | 1.014670 | 0.642621 | 0.001522 | 0.001752 | 0.001786 |
| mp_contains.test | RUN | 0.171231 | 0.625255 | 0.595595 | 0.001356 | 0.001795 | 0.001796 |
| mp_starts_with.test | RUN | 0.173264 | 0.617808 | 0.647442 | 0.001724 | 0.001878 | 0.001576 |
| mp_starts_with_sf.test | RUN | 0.173683 | 0.532576 | 0.624288 | 0.001602 | 0.001703 | 0.001461 |
| mp_repeat.test | RUN | 0.252802 | 2.512034 | 1.054865 | 0.001761 | 0.001950 | 0.001688 |
| mp_repeat_2.test | RUN | 0.027340 | 0.061025 | 0.096223 | 0.000728 | 0.000829 | 0.001445 |
| mp_repeat_3.test | RUN | 0.213245 | 0.333000 | 0.578196 | 0.001503 | 0.001929 | 0.001762 |
| mp_product.test | RUN | 0.193161 | 1.498524 | 0.861303 | 0.001571 | 0.001902 | 0.001687 |
| mp_drop.test | RUN | 0.209913 | 1.911809 | 0.964850 | 0.001636 | 0.001735 | 0.001805 |
| mp_drop_2.test | RUN | 0.028641 | 0.059887 | 0.091340 | 0.000579 | 0.000816 | 0.001930 |
| mp_iota.test | RUN | 0.211829 | 1.605492 | 0.845450 | 0.001292 | 0.001603 | 0.001674 |
| mp_iota_2.test | RUN | 0.219212 | 1.918304 | 0.946583 | 0.001633 | 0.001910 | 0.001751 |
| mp_at.test | RUN | 0.186924 | 0.847789 | 0.648878 | 0.001276 | 0.001810 | 0.001766 |
| mp_at_2.test | RUN | 0.028213 | 0.054451 | 0.101785 | 0.000622 | 0.000613 | 0.001853 |
| mp_at_sf.test | RUN | 0.169507 | 0.521906 | 0.573051 | 0.001602 | 0.001878 | 0.001792 |
| mp_take.test | RUN | 0.278375 | 2.470467 | 1.125073 | 0.001578 | 0.001825 | 0.001872 |
| mp_take_2.test | RUN | 0.027689 | 0.060417 | 0.102543 | 0.000740 | 0.000775 | 0.001501 |
| mp_slice.test | RUN | 0.315194 | 2.554246 | 1.441546 | 0.001675 | 0.001817 | 0.001873 |
| mp_slice_2.test | RUN | 0.028453 | 0.057649 | 0.090967 | 0.000708 | 0.000715 | 0.001860 |
| mp_replace.test | RUN | 0.215415 | 1.283172 | 0.902329 | 0.001786 | 0.001844 | 0.001764 |
| mp_replace_if.test | RUN | 0.201807 | 1.146968 | 0.828985 | 0.001750 | 0.001629 | 0.001914 |
| mp_replace_if_q.test | RUN | 0.199304 | 1.156485 | 0.817740 | 0.001660 | 0.001893 | 0.001813 |
| mp_copy_if.test | RUN | 0.204915 | 1.603880 | 0.806451 | 0.001871 | 0.001605 | 0.001807 |
| mp_copy_if_q.test | RUN | 0.210900 | 1.569560 | 0.810690 | 0.001568 | 0.001664 | 0.001860 |
| mp_remove.test | RUN | 0.198249 | 1.083782 | 0.807143 | 0.001793 | 0.001934 | 0.001792 |
| mp_remove_if.test | RUN | 0.205974 | 1.687094 | 0.872303 | 0.001595 | 0.001911 | 0.001825 |
| mp_remove_if_q.test | RUN | 0.209122 | 1.674483 | 0.835404 | 0.001709 | 0.001948 | 0.001906 |
| mp_partition.test | RUN | 0.213029 | 1.565222 | 0.879183 | 0.001618 | 0.001731 | 0.001854 |
| mp_partition_q.test | RUN | 0.202029 | 1.566102 | 0.956639 | 0.001441 | 0.001901 | 0.001842 |
| mp_sort.test | RUN | 0.198996 | 1.113929 | 0.806048 | 0.001559 | 0.001804 | 0.001829 |
| mp_sort_q.test | RUN | 0.201317 | 1.115716 | 0.806616 | 0.002098 | 0.001797 | 0.001828 |
| mp_find.test | RUN | 0.191067 | 1.210551 | 0.747494 | 0.001437 | 0.001698 | 0.001668 |
| mp_find_if.test | RUN | 0.185843 | 1.099984 | 0.647668 | 0.001960 | 0.001857 | 0.001808 |
| mp_find_if_q.test | RUN | 0.203812 | 1.239116 | 0.854058 | 0.001678 | 0.001832 | 0.001891 |
| mp_reverse.test | RUN | 0.403638 | 3.275704 | 1.614683 | 0.001523 | 0.001884 | 0.001645 |
| mp_fold.test | RUN | 0.663502 | 2.550060 | 3.954712 | 0.001725 | 0.002081 | 0.001537 |
| mp_fold_q.test | RUN | 0.200769 | 1.109498 | 0.828023 | 0.001676 | 0.001820 | 0.001871 |
| mp_fold_q_sf.test | RUN | 0.175468 | 0.548116 | 0.634892 | 0.001519 | 0.001810 | 0.001792 |
| mp_reverse_fold.test | RUN | 0.316865 | 2.454912 | 1.097379 | 0.001534 | 0.001832 | 0.001671 |
| mp_reverse_fold_q.test | RUN | 0.203378 | 1.396856 | 0.849063 | 0.001663 | 0.002117 | 0.001853 |
| mp_unique.test | RUN | 0.202428 | 1.236345 | 0.829806 | 0.001505 | 0.001772 | 0.001454 |
| mp_unique_if.test | RUN | 0.341935 | 2.489804 | 1.287125 | 0.001346 | 0.001922 | 0.001465 |
| mp_unique_if_q.test | RUN | 0.285716 | 2.481170 | 1.062389 | 0.001501 | 0.001669 | 0.001672 |
| mp_all_of.test | RUN | 0.468971 | 1.372460 | 1.349417 | 0.001801 | 0.001882 | 0.001824 |
| mp_all_of_q.test | RUN | 0.173401 | 0.610258 | 0.614712 | 0.001742 | 0.001799 | 0.001686 |
| mp_any_of.test | RUN | 0.461513 | 1.440415 | 1.311616 | 0.001432 | 0.001800 | 0.001802 |
| mp_any_of_q.test | RUN | 0.166825 | 0.609290 | 0.627731 | 0.001727 | 0.001750 | 0.001792 |
| mp_none_of.test | RUN | 0.458939 | 1.443444 | 1.297714 | 0.001486 | 0.001736 | 0.001866 |
| mp_none_of_q.test | RUN | 0.166380 | 0.595670 | 0.629313 | 0.001653 | 0.001712 | 0.001682 |
| mp_replace_at.test | RUN | 0.201461 | 1.178066 | 0.926238 | 0.001647 | 0.001701 | 0.001780 |
| mp_replace_at_c.test | RUN | 0.209738 | 1.232179 | 0.907239 | 0.001632 | 0.001596 | 0.001775 |
| mp_for_each.test | RUN | 0.198399 | 0.919852 | 0.959395 | 0.001611 | 0.001606 | 0.001765 |
| mp_insert.test | RUN | 0.303273 | 2.475311 | 1.282762 | 0.001724 | 0.001658 | 0.001560 |
| mp_insert_2.test | RUN | 0.027909 | 0.058380 | 0.101413 | 0.000657 | 0.000580 | 0.001785 |
| mp_erase.test | RUN | 0.338302 | 2.625201 | 1.522805 | 0.001782 | 0.001767 | 0.001746 |
| mp_erase_2.test | RUN | 0.027491 | 0.055909 | 0.098401 | 0.000594 | 0.000636 | 0.001780 |
| mp_with_index.test | RUN | 0.421610 | 5.310466 | 3.363331 | 0.002038 | 0.002084 | 0.002073 |
| mp_with_index_cx.test | RUN | 0.003534 | 0.018032 | 0.034084 | 0.000694 | 0.000776 | 0.001517 |
| mp_from_sequence.test | RUN | 0.203964 | 1.390978 | 0.855364 | 0.001741 | 0.001786 | 0.001723 |
| mp_from_sequence_2.test | RUN | 0.221792 | 1.614155 | 0.926329 | 0.001548 | 0.001738 | 0.001756 |
| mp_from_sequence_3.test | RUN | 0.185586 | 0.724607 | 0.692430 | 0.001654 | 0.001766 | 0.001866 |
| mp_min_element.test | RUN | 0.178227 | 0.669576 | 0.609893 | 0.001529 | 0.001714 | 0.001624 |
| mp_min_element_2.test | RUN | 0.027632 | 0.057502 | 0.100320 | 0.000590 | 0.000701 | 0.001822 |
| mp_min_element_q.test | RUN | 0.179361 | 0.677528 | 0.620574 | 0.001626 | 0.001583 | 0.001749 |
| mp_min_element_q_2.test | RUN | 0.027524 | 0.056328 | 0.088120 | 0.000716 | 0.000662 | 0.001580 |
| mp_max_element.test | RUN | 0.180151 | 0.654758 | 0.595946 | 0.001753 | 0.001832 | 0.001770 |
| mp_max_element_2.test | RUN | 0.027403 | 0.056533 | 0.100692 | 0.000593 | 0.000641 | 0.001834 |
| mp_max_element_q.test | RUN | 0.179556 | 0.660566 | 0.613981 | 0.001786 | 0.001836 | 0.001734 |
| mp_max_element_q_2.test | RUN | 0.028703 | 0.057873 | 0.084286 | 0.000788 | 0.000697 | 0.001813 |
| mp_nth_element.test | RUN | 0.198266 | 0.919606 | 0.796344 | 0.001600 | 0.001568 | 0.001634 |
| mp_nth_element_q.test | RUN | 0.200945 | 0.925210 | 0.792208 | 0.001600 | 0.001723 | 0.001790 |
| mp_back.test | RUN | 0.196390 | 0.770177 | 0.701174 | 0.001586 | 0.001656 | 0.001617 |
| mp_back_2.test | RUN | 0.027396 | 0.056041 | 0.100582 | 0.000803 | 0.000708 | 0.001724 |
| mp_pop_back.test | RUN | 0.495801 | 4.980677 | 1.489245 | 0.001798 | 0.001804 | 0.001649 |
| mp_pop_back_2.test | RUN | 0.032475 | 0.055866 | 0.089979 | 0.000795 | 0.000680 | 0.001426 |
| mp_flatten.test | RUN | 0.226632 | 1.776384 | 1.044424 | 0.001720 | 0.001764 | 0.001642 |
| mp_rotate_left.test | RUN | 0.230594 | 1.911852 | 1.005029 | 0.001822 | 0.001854 | 0.001273 |
| mp_rotate_right.test | RUN | 0.230793 | 1.908185 | 1.016633 | 0.001830 | 0.001496 | 0.001670 |
| mp_power_set.test | RUN | 0.228972 | 1.820028 | 0.999678 | 0.001656 | 0.001816 | 0.001622 |
| mp_partial_sum.test | RUN | 0.285425 | 2.454906 | 1.126791 | 0.001582 | 0.001595 | 0.001725 |
| mp_iterate.test | RUN | 0.199299 | 0.742908 | 0.793010 | 0.001756 | 0.001792 | 0.001722 |
| mp_pairwise_fold.test | RUN | 0.223824 | 1.695564 | 1.013431 | 0.001786 | 0.001838 | 0.001842 |
| mp_pairwise_fold_q.test | RUN | 0.174864 | 0.522198 | 0.585230 | 0.001870 | 0.001660 | 0.001849 |
| mp_sliding_fold.test | RUN | 0.417336 | 3.168565 | 1.880526 | 0.001426 | 0.001866 | 0.001720 |
| mp_sliding_fold_q.test | RUN | 0.414453 | 3.211343 | 1.896183 | 0.001606 | 0.001668 | 0.001874 |
| mp_intersperse.test | RUN | 0.211497 | 1.202792 | 0.881640 | 0.001836 | 0.001730 | 0.001911 |
| mp_split.test | RUN | 0.302985 | 2.310777 | 1.204821 | 0.002068 | 0.001739 | 0.001478 |
| mp_join.test | RUN | 0.291007 | 1.933910 | 1.096057 | 0.001760 | 0.001708 | 0.001889 |
| integral.test | RUN | 0.202894 | 0.860544 | 0.658056 | 0.001605 | 0.001893 | 0.001725 |
| mp_value.test | RUN | 0.012820 | 0.031436 | 0.048158 | 0.000628 | 0.000607 | 0.001804 |
| mp_identity.test | RUN | 0.174863 | 0.800672 | 0.610541 | 0.001856 | 0.001569 | 0.001861 |
| mp_inherit.test | RUN | 0.175477 | 0.591058 | 0.629468 | 0.001655 | 0.001692 | 0.001906 |
| mp_if.test | RUN | 0.167412 | 0.654389 | 0.638731 | 0.001869 | 0.001483 | 0.001832 |
| mp_if_sf.test | RUN | 0.167994 | 0.494836 | 0.503038 | 0.001597 | 0.001832 | 0.001885 |
| mp_eval_if.test | RUN | 0.173966 | 0.699886 | 0.641725 | 0.001878 | 0.001671 | 0.001662 |
| mp_eval_if_sf.test | RUN | 0.165894 | 0.508001 | 0.567001 | 0.001912 | 0.001572 | 0.001778 |
| mp_valid.test | RUN | 0.167876 | 0.504781 | 0.568843 | 0.001848 | 0.001630 | 0.001818 |
| mp_defer.test | RUN | 0.182840 | 0.847465 | 0.667119 | 0.001460 | 0.001712 | 0.001896 |
| mp_quote.test | RUN | 0.179602 | 0.857295 | 0.705274 | 0.001888 | 0.001712 | 0.001828 |
| mp_invoke_q.test | RUN | 0.194758 | 1.110838 | 0.772578 | 0.001708 | 0.001867 | 0.001633 |
| mp_invoke_q_sf.test | RUN | 0.156633 | 0.490500 | 0.536228 | 0.001834 | 0.001832 | 0.001666 |
| mp_quote_trait.test | RUN | 0.176360 | 0.931310 | 0.674780 | 0.001720 | 0.001750 | 0.001893 |
| mp_cond.test | RUN | 0.170908 | 0.589168 | 0.587269 | 0.001760 | 0.001724 | 0.001685 |
| mp_cond_sf.test | RUN | 0.162929 | 0.484847 | 0.546229 | 0.001549 | 0.001628 | 0.001672 |
| mp_not_fn.test | RUN | 0.170940 | 0.512028 | 0.548542 | 0.001785 | 0.001799 | 0.001657 |
| mp_eval_if_not.test | RUN | 0.177458 | 0.708261 | 0.599751 | 0.001702 | 0.001634 | 0.001832 |
| mp_eval_or.test | RUN | 0.165605 | 0.384197 | 0.574542 | 0.001837 | 0.001843 | 0.001836 |
| mp_compose.test | RUN | 0.187827 | 1.142466 | 0.748208 | 0.001518 | 0.001816 | 0.001858 |
| mp_valid_and_true.test | RUN | 0.163914 | 0.591823 | 0.576160 | 0.001834 | 0.001837 | 0.001748 |
| mp_compose_sf.test | COMPILE | 0.042055 | 0.069138 | 0.125015 | — | — | — |
| integer_sequence.test | RUN | 0.200833 | 1.122128 | 0.818029 | 0.001565 | 0.001766 | 0.001687 |
| tuple_for_each.test | RUN | 0.171460 | 0.442876 | 0.595268 | 0.001539 | 0.001607 | 0.001920 |
| tuple_for_each_cx.test | COMPILE | 0.032270 | 0.067038 | 0.111593 | — | — | — |
| tuple_apply.test | RUN | 0.166859 | 0.433663 | 0.616240 | 0.001574 | 0.001696 | 0.001832 |
| tuple_apply_cx.test | COMPILE | 0.047486 | 0.092265 | 0.156541 | — | — | — |
| construct_from_tuple.test | RUN | 0.175839 | 0.473773 | 0.598096 | 0.001633 | 0.001782 | 0.002268 |
| construct_from_tuple_cx.test | COMPILE | 0.044743 | 0.089378 | 0.153499 | — | — | — |
| tuple_transform.test | RUN | 0.278779 | 0.926212 | 1.217514 | 0.001960 | 0.001698 | 0.001793 |
| tuple_transform_2.test | RUN | 0.250989 | 0.688005 | 1.127594 | 0.001724 | 0.001394 | 0.001756 |
| tuple_transform_cx.test | COMPILE | 0.042370 | 0.087344 | 0.180421 | — | — | — |
| mp_set_contains.test | RUN | 0.181652 | 0.603095 | 0.632050 | 0.001451 | 0.001636 | 0.001758 |
| mp_set_push_back.test | RUN | 0.221853 | 1.827662 | 0.999608 | 0.001717 | 0.001644 | 0.001855 |
| mp_set_push_front.test | RUN | 0.282225 | 2.180405 | 1.152867 | 0.001873 | 0.001789 | 0.001621 |
| mp_is_set.test | RUN | 0.174068 | 0.607449 | 0.587294 | 0.001755 | 0.001807 | 0.001921 |
| mp_set_union.test | RUN | 0.214119 | 1.550956 | 0.972653 | 0.001824 | 0.001864 | 0.001892 |
| mp_set_union_sf.test | RUN | 0.165633 | 0.486979 | 0.538948 | 0.001554 | 0.001778 | 0.001901 |
| mp_set_difference.test | RUN | 0.198253 | 1.226023 | 0.788562 | 0.001698 | 0.001831 | 0.001817 |
| mp_set_difference_sf.test | RUN | 0.166469 | 0.495027 | 0.555881 | 0.001558 | 0.001610 | 0.001776 |
| mp_set_intersection.test | RUN | 0.192125 | 1.208643 | 0.780640 | 0.001551 | 0.001537 | 0.001802 |
| mp_set_intersection_sf.test | RUN | 0.164687 | 0.484235 | 0.550768 | 0.001602 | 0.001688 | 0.001861 |
| mp_all.test | RUN | 0.181763 | 0.598521 | 0.603750 | 0.001960 | 0.001845 | 0.001910 |
| mp_all_2.test | RUN | 0.508489 | 1.283970 | 1.322511 | 0.001881 | 0.001904 | 0.001920 |
| mp_and.test | RUN | 0.170288 | 0.606806 | 0.592222 | 0.001708 | 0.001795 | 0.001704 |
| mp_any.test | RUN | 0.174263 | 0.571729 | 0.603565 | 0.001656 | 0.001886 | 0.001785 |
| mp_any_2.test | RUN | 0.521969 | 1.289026 | 1.391978 | 0.001678 | 0.001872 | 0.001379 |
| mp_or.test | RUN | 0.182298 | 0.573679 | 0.596708 | 0.001818 | 0.001891 | 0.001684 |
| mp_same.test | RUN | 0.350660 | 0.999924 | 0.908930 | 0.001766 | 0.001656 | 0.001714 |
| mp_plus.test | RUN | 0.190308 | 0.678684 | 0.628685 | 0.001717 | 0.001550 | 0.001492 |
| mp_less.test | RUN | 0.176951 | 0.594976 | 0.555815 | 0.001739 | 0.001690 | 0.001876 |
| mp_min.test | RUN | 0.174754 | 0.559079 | 0.605362 | 0.001839 | 0.001843 | 0.001778 |
| mp_max.test | RUN | 0.176948 | 0.602654 | 0.589360 | 0.001540 | 0.001810 | 0.001604 |
| mp_similar.test | RUN | 0.176514 | 0.603429 | 0.616534 | 0.001660 | 0.001520 | 0.001604 |
| mp_map_find.test | RUN | 0.197206 | 1.092765 | 0.783517 | 0.001846 | 0.001586 | 0.001858 |
| mp_map_find_2.test | RUN | 0.183668 | 0.427195 | 0.695782 | 0.001868 | 0.001736 | 0.001729 |
| mp_map_find_3.test | RUN | 0.184494 | 0.427635 | 0.645164 | 0.001632 | 0.001348 | 0.001535 |
| mp_map_find_4.test | RUN | 0.174906 | 0.398078 | 0.639264 | 0.001684 | 0.001333 | 0.001589 |
| mp_map_contains.test | RUN | 0.177753 | 0.617605 | 0.620203 | 0.001789 | 0.001936 | 0.001816 |
| mp_map_insert.test | RUN | 0.235229 | 2.042797 | 1.085812 | 0.001696 | 0.001820 | 0.001465 |
| mp_map_replace.test | RUN | 0.294843 | 2.302058 | 1.238902 | 0.001906 | 0.001826 | 0.001328 |
| mp_map_erase.test | RUN | 0.226828 | 1.729890 | 1.050481 | 0.001498 | 0.001842 | 0.001792 |
| mp_map_update.test | RUN | 0.205907 | 1.133244 | 0.831636 | 0.001750 | 0.001720 | 0.001736 |
| mp_map_update_q.test | RUN | 0.200482 | 1.125338 | 0.829553 | 0.001802 | 0.001540 | 0.001478 |
| mp_map_keys.test | RUN | 0.194836 | 1.011986 | 0.810866 | 0.001557 | 0.001720 | 0.001535 |
| mp_is_map.test | RUN | 0.180466 | 0.615700 | 0.648083 | 0.001329 | 0.001472 | 0.001535 |
| mp_bind.test | RUN | 0.222001 | 1.803028 | 0.933077 | 0.001784 | 0.001565 | 0.001898 |
| mp_bind_q.test | RUN | 0.194407 | 1.119606 | 0.788126 | 0.001866 | 0.001664 | 0.001644 |
| mp_bind_front.test | RUN | 0.198277 | 1.221453 | 0.734127 | 0.001544 | 0.001630 | 0.001824 |
| mp_bind_back.test | RUN | 0.199510 | 1.211620 | 0.765688 | 0.001375 | 0.001730 | 0.001309 |
| mp_lambda.test | RUN | 0.411912 | 3.681837 | 1.936059 | 0.001839 | 0.001306 | 0.001673 |
| mpl.test | RUN | 0.390127 | 0.987749 | 1.571134 | 0.001537 | 0.001760 | 0.001574 |
| mpl_list.test | RUN | 0.222652 | 0.766596 | 0.742683 | 0.001656 | 0.001907 | 0.001823 |
| mpl_tuple.test | RUN | 0.224275 | 0.749119 | 0.739840 | 0.001589 | 0.001544 | 0.001734 |
| version.test | RUN | 0.112408 | 0.276212 | 0.350156 | 0.001826 | 0.001788 | 0.001805 |
| mp11_version.test | RUN | 0.136998 | 0.309196 | 0.465985 | 0.001778 | 0.001759 | 0.001648 |
| list_version.test | RUN | 0.114946 | 0.282777 | 0.389448 | 0.001640 | 0.001680 | 0.001856 |
| algorithm_version.test | RUN | 0.129482 | 0.300123 | 0.412867 | 0.001637 | 0.001664 | 0.001867 |
| integral_version.test | RUN | 0.113592 | 0.271801 | 0.387967 | 0.001779 | 0.001470 | 0.001623 |
| utility_version.test | RUN | 0.111532 | 0.289150 | 0.393518 | 0.001696 | 0.001776 | 0.001624 |
| function_version.test | RUN | 0.113375 | 0.280405 | 0.381248 | 0.001621 | 0.001764 | 0.001955 |
| map_version.test | RUN | 0.126224 | 0.296884 | 0.407753 | 0.001819 | 0.001493 | 0.001863 |
| set_version.test | RUN | 0.120842 | 0.290604 | 0.402508 | 0.001701 | 0.001800 | 0.001608 |
| bind_version.test | RUN | 0.125487 | 0.314349 | 0.433583 | 0.001642 | 0.001633 | 0.001677 |
| integer_sequence_version.test | RUN | 0.108025 | 0.281057 | 0.365998 | 0.001401 | 0.001660 | 0.001904 |
| tuple_version.test | RUN | 0.120700 | 0.295522 | 0.331892 | 0.001292 | 0.001788 | 0.001347 |
| mpl_version.test | RUN | 0.102425 | 0.236069 | 0.312093 | 0.001369 | 0.001340 | 0.001336 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 1.000×, 0.994× | 1.006× |
| clang | 1.099× | 1.112×, 1.086× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001011s | 0.000991s | 0.001003s | byte equality observed |
| bind_version.test | 10 | 0.001002s | 0.000969s | 0.001002s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001019s | 0.000992s | 0.001019s | byte equality observed |
| function_version.test | 10 | 0.000994s | 0.001043s | 0.001033s | byte equality observed |
| integer_sequence.test | 10 | 0.001012s | 0.001022s | 0.001020s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001032s | 0.000993s | 0.001054s | byte equality observed |
| integral.test | 10 | 0.001007s | 0.000991s | 0.001004s | byte equality observed |
| integral_version.test | 10 | 0.001029s | 0.001020s | 0.001051s | byte equality observed |
| list_version.test | 10 | 0.000994s | 0.001033s | 0.001021s | byte equality observed |
| map_version.test | 10 | 0.000983s | 0.001039s | 0.001010s | byte equality observed |
| mp11_version.test | 10 | 0.000991s | 0.001017s | 0.001014s | byte equality observed |
| mp_all.test | 10 | 0.001020s | 0.001054s | 0.000975s | byte equality observed |
| mp_all_2.test | 10 | 0.001001s | 0.001038s | 0.001045s | byte equality observed |
| mp_all_of.test | 10 | 0.001001s | 0.001048s | 0.001001s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001016s | 0.001036s | 0.001008s | byte equality observed |
| mp_and.test | 10 | 0.001011s | 0.001006s | 0.001031s | byte equality observed |
| mp_any.test | 10 | 0.001014s | 0.001069s | 0.000998s | byte equality observed |
| mp_any_2.test | 10 | 0.001014s | 0.001045s | 0.000996s | byte equality observed |
| mp_any_of.test | 10 | 0.001035s | 0.001031s | 0.001021s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001037s | 0.001034s | 0.001007s | byte equality observed |
| mp_append.test | 10 | 0.001029s | 0.001025s | 0.001031s | byte equality observed |
| mp_append_2.test | 10 | 0.001059s | 0.001036s | 0.000982s | byte equality observed |
| mp_append_3.test | 10 | 0.000469s | 0.000473s | 0.001019s | byte equality observed |
| mp_append_sf.test | 10 | 0.001021s | 0.001037s | 0.000986s | byte equality observed |
| mp_apply_q.test | 10 | 0.001044s | 0.001066s | 0.001020s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001031s | 0.001025s | 0.000985s | byte equality observed |
| mp_assign.test | 10 | 0.001021s | 0.001057s | 0.001001s | byte equality observed |
| mp_assign_2.test | 10 | 0.000466s | 0.000475s | 0.000997s | byte equality observed |
| mp_at.test | 10 | 0.001051s | 0.001075s | 0.000977s | byte equality observed |
| mp_at_2.test | 10 | 0.000465s | 0.000463s | 0.000985s | byte equality observed |
| mp_at_sf.test | 10 | 0.001012s | 0.001020s | 0.000989s | byte equality observed |
| mp_back.test | 10 | 0.001005s | 0.001024s | 0.000996s | byte equality observed |
| mp_back_2.test | 10 | 0.000464s | 0.000459s | 0.000954s | byte equality observed |
| mp_bind.test | 10 | 0.001033s | 0.001027s | 0.001002s | byte equality observed |
| mp_bind_back.test | 10 | 0.001010s | 0.001001s | 0.000968s | byte equality observed |
| mp_bind_front.test | 10 | 0.000998s | 0.001003s | 0.001020s | byte equality observed |
| mp_bind_q.test | 10 | 0.001013s | 0.001021s | 0.001003s | byte equality observed |
| mp_clear.test | 10 | 0.001005s | 0.001045s | 0.001007s | byte equality observed |
| mp_clear_2.test | 10 | 0.000461s | 0.000469s | 0.000998s | byte equality observed |
| mp_compose.test | 10 | 0.001027s | 0.001003s | 0.000989s | byte equality observed |
| mp_cond.test | 10 | 0.000994s | 0.000991s | 0.001031s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001005s | 0.001027s | 0.001001s | byte equality observed |
| mp_contains.test | 10 | 0.001026s | 0.001007s | 0.001005s | byte equality observed |
| mp_copy_if.test | 10 | 0.000997s | 0.001008s | 0.000984s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001026s | 0.001000s | 0.001001s | byte equality observed |
| mp_count.test | 10 | 0.000987s | 0.001018s | 0.001002s | byte equality observed |
| mp_count_if.test | 10 | 0.000996s | 0.000997s | 0.001004s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001020s | 0.001035s | 0.001050s | byte equality observed |
| mp_defer.test | 10 | 0.001007s | 0.001007s | 0.001023s | byte equality observed |
| mp_drop.test | 10 | 0.001039s | 0.001045s | 0.001035s | byte equality observed |
| mp_drop_2.test | 10 | 0.000465s | 0.000463s | 0.001001s | byte equality observed |
| mp_empty.test | 10 | 0.001017s | 0.001030s | 0.001050s | byte equality observed |
| mp_empty_2.test | 10 | 0.000460s | 0.000469s | 0.001013s | byte equality observed |
| mp_erase.test | 10 | 0.001062s | 0.001038s | 0.001035s | byte equality observed |
| mp_erase_2.test | 10 | 0.000451s | 0.000463s | 0.000995s | byte equality observed |
| mp_eval_if.test | 10 | 0.001021s | 0.001020s | 0.001022s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001012s | 0.001029s | 0.001031s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000997s | 0.001010s | 0.001020s | byte equality observed |
| mp_eval_or.test | 10 | 0.000997s | 0.001025s | 0.001018s | byte equality observed |
| mp_fill.test | 10 | 0.001026s | 0.001029s | 0.001015s | byte equality observed |
| mp_fill_2.test | 10 | 0.000466s | 0.000470s | 0.001010s | byte equality observed |
| mp_filter.test | 10 | 0.000982s | 0.001053s | 0.000997s | byte equality observed |
| mp_find.test | 10 | 0.001005s | 0.001023s | 0.001022s | byte equality observed |
| mp_find_if.test | 10 | 0.001015s | 0.001033s | 0.001047s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001003s | 0.001021s | 0.001018s | byte equality observed |
| mp_flatten.test | 10 | 0.001010s | 0.001041s | 0.001009s | byte equality observed |
| mp_fold.test | 10 | 0.001047s | 0.001012s | 0.001020s | byte equality observed |
| mp_fold_q.test | 10 | 0.001014s | 0.001025s | 0.001024s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001008s | 0.000999s | 0.001023s | byte equality observed |
| mp_for_each.test | 10 | 0.001002s | 0.001019s | 0.001008s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001009s | 0.001055s | 0.001021s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001027s | 0.001044s | 0.001048s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001022s | 0.001036s | 0.001007s | byte equality observed |
| mp_front.test | 10 | 0.001019s | 0.001036s | 0.001054s | byte equality observed |
| mp_front_2.test | 10 | 0.000471s | 0.000466s | 0.000973s | byte equality observed |
| mp_identity.test | 10 | 0.001016s | 0.001015s | 0.001005s | byte equality observed |
| mp_if.test | 10 | 0.001005s | 0.001013s | 0.001045s | byte equality observed |
| mp_if_sf.test | 10 | 0.001023s | 0.000994s | 0.001027s | byte equality observed |
| mp_inherit.test | 10 | 0.001019s | 0.001062s | 0.001036s | byte equality observed |
| mp_insert.test | 10 | 0.001055s | 0.001037s | 0.001017s | byte equality observed |
| mp_insert_2.test | 10 | 0.000479s | 0.000473s | 0.001011s | byte equality observed |
| mp_intersperse.test | 10 | 0.001022s | 0.000988s | 0.001015s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001014s | 0.001020s | 0.000988s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001034s | 0.000997s | 0.001002s | byte equality observed |
| mp_iota.test | 10 | 0.001033s | 0.001016s | 0.001008s | byte equality observed |
| mp_iota_2.test | 10 | 0.001014s | 0.001025s | 0.001016s | byte equality observed |
| mp_is_list.test | 10 | 0.001002s | 0.001063s | 0.001030s | byte equality observed |
| mp_is_map.test | 10 | 0.001024s | 0.001005s | 0.000984s | byte equality observed |
| mp_is_set.test | 10 | 0.001042s | 0.001012s | 0.000988s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001018s | 0.001016s | 0.001010s | byte equality observed |
| mp_iterate.test | 10 | 0.001007s | 0.001017s | 0.001039s | byte equality observed |
| mp_join.test | 10 | 0.001052s | 0.001022s | 0.001005s | byte equality observed |
| mp_lambda.test | 10 | 0.001037s | 0.001038s | 0.000995s | byte equality observed |
| mp_less.test | 10 | 0.001018s | 0.001026s | 0.000998s | byte equality observed |
| mp_list_c.test | 10 | 0.001026s | 0.001009s | 0.001011s | byte equality observed |
| mp_list_v.test | 10 | 0.000473s | 0.000467s | 0.000967s | byte equality observed |
| mp_map_contains.test | 10 | 0.001038s | 0.001023s | 0.000981s | byte equality observed |
| mp_map_erase.test | 10 | 0.001021s | 0.001016s | 0.001008s | byte equality observed |
| mp_map_find.test | 10 | 0.001020s | 0.001017s | 0.001009s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001008s | 0.001008s | 0.001000s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001002s | 0.001002s | 0.001004s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001021s | 0.000976s | 0.001021s | byte equality observed |
| mp_map_insert.test | 10 | 0.001021s | 0.001048s | 0.001007s | byte equality observed |
| mp_map_keys.test | 10 | 0.001014s | 0.001011s | 0.001017s | byte equality observed |
| mp_map_replace.test | 10 | 0.001054s | 0.001009s | 0.001004s | byte equality observed |
| mp_map_update.test | 10 | 0.001011s | 0.001017s | 0.001022s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001032s | 0.001006s | 0.001012s | byte equality observed |
| mp_max.test | 10 | 0.001029s | 0.001015s | 0.000990s | byte equality observed |
| mp_max_element.test | 10 | 0.001050s | 0.001030s | 0.000990s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000468s | 0.000481s | 0.000999s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001030s | 0.001026s | 0.000996s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000480s | 0.000452s | 0.000980s | byte equality observed |
| mp_min.test | 10 | 0.001027s | 0.001017s | 0.001002s | byte equality observed |
| mp_min_element.test | 10 | 0.001024s | 0.001032s | 0.000992s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000468s | 0.000481s | 0.000978s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001052s | 0.001064s | 0.001012s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000472s | 0.000481s | 0.000992s | byte equality observed |
| mp_none_of.test | 10 | 0.001025s | 0.001037s | 0.001010s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001018s | 0.001050s | 0.000992s | byte equality observed |
| mp_not_fn.test | 10 | 0.001056s | 0.001025s | 0.001023s | byte equality observed |
| mp_nth_element.test | 10 | 0.001019s | 0.001008s | 0.001006s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001030s | 0.001070s | 0.001016s | byte equality observed |
| mp_or.test | 10 | 0.001026s | 0.001028s | 0.001013s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001037s | 0.001045s | 0.001012s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001029s | 0.001048s | 0.000987s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001043s | 0.001044s | 0.001015s | byte equality observed |
| mp_partition.test | 10 | 0.001043s | 0.001065s | 0.001068s | byte equality observed |
| mp_partition_q.test | 10 | 0.001012s | 0.001030s | 0.001051s | byte equality observed |
| mp_plus.test | 10 | 0.001054s | 0.001032s | 0.001016s | byte equality observed |
| mp_pop_back.test | 10 | 0.001074s | 0.001079s | 0.000997s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000478s | 0.000471s | 0.000998s | byte equality observed |
| mp_pop_front.test | 10 | 0.001047s | 0.001057s | 0.001011s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000481s | 0.000482s | 0.000967s | byte equality observed |
| mp_power_set.test | 10 | 0.001037s | 0.001008s | 0.001018s | byte equality observed |
| mp_product.test | 10 | 0.001038s | 0.001041s | 0.000999s | byte equality observed |
| mp_push_back.test | 10 | 0.001055s | 0.001044s | 0.000996s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000487s | 0.000476s | 0.000981s | byte equality observed |
| mp_push_front.test | 10 | 0.001050s | 0.001045s | 0.000969s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000470s | 0.000472s | 0.000967s | byte equality observed |
| mp_quote.test | 10 | 0.001025s | 0.001008s | 0.001022s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001046s | 0.001023s | 0.001021s | byte equality observed |
| mp_remove.test | 10 | 0.001031s | 0.001012s | 0.001006s | byte equality observed |
| mp_remove_if.test | 10 | 0.001022s | 0.001042s | 0.001016s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001043s | 0.001033s | 0.001016s | byte equality observed |
| mp_rename.test | 10 | 0.001063s | 0.001040s | 0.001026s | byte equality observed |
| mp_rename_2.test | 10 | 0.000477s | 0.000469s | 0.000983s | byte equality observed |
| mp_rename_v.test | 10 | 0.000480s | 0.000480s | 0.001015s | byte equality observed |
| mp_repeat.test | 10 | 0.001079s | 0.001032s | 0.000987s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000478s | 0.000466s | 0.001020s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001036s | 0.000983s | 0.001029s | byte equality observed |
| mp_replace.test | 10 | 0.001045s | 0.001011s | 0.001050s | byte equality observed |
| mp_replace_at.test | 10 | 0.001002s | 0.001019s | 0.001027s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000996s | 0.001029s | 0.000985s | byte equality observed |
| mp_replace_front.test | 10 | 0.001018s | 0.001048s | 0.001012s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000463s | 0.000478s | 0.001004s | byte equality observed |
| mp_replace_if.test | 10 | 0.001001s | 0.001026s | 0.001030s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001010s | 0.001005s | 0.001022s | byte equality observed |
| mp_replace_second.test | 10 | 0.001060s | 0.001029s | 0.001057s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000482s | 0.000472s | 0.000983s | byte equality observed |
| mp_replace_third.test | 10 | 0.001034s | 0.001028s | 0.001027s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000475s | 0.000461s | 0.000986s | byte equality observed |
| mp_reverse.test | 10 | 0.001024s | 0.001068s | 0.001022s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001047s | 0.001045s | 0.001013s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001012s | 0.001009s | 0.001014s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001007s | 0.001069s | 0.001023s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001027s | 0.001039s | 0.001050s | byte equality observed |
| mp_same.test | 10 | 0.001020s | 0.001041s | 0.001017s | byte equality observed |
| mp_second.test | 10 | 0.001044s | 0.001029s | 0.001030s | byte equality observed |
| mp_second_2.test | 10 | 0.000467s | 0.000471s | 0.001006s | byte equality observed |
| mp_set_contains.test | 10 | 0.001019s | 0.001079s | 0.001001s | byte equality observed |
| mp_set_difference.test | 10 | 0.001023s | 0.001029s | 0.001029s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001006s | 0.001058s | 0.000996s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001035s | 0.001011s | 0.001032s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001008s | 0.000985s | 0.001050s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001032s | 0.001010s | 0.001026s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001011s | 0.001049s | 0.001063s | byte equality observed |
| mp_set_union.test | 10 | 0.001006s | 0.001022s | 0.001062s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001032s | 0.001083s | 0.001066s | byte equality observed |
| mp_similar.test | 10 | 0.001000s | 0.001024s | 0.001036s | byte equality observed |
| mp_size.test | 10 | 0.001016s | 0.001033s | 0.001035s | byte equality observed |
| mp_size_2.test | 10 | 0.000458s | 0.000470s | 0.000999s | byte equality observed |
| mp_slice.test | 10 | 0.001054s | 0.001046s | 0.001031s | byte equality observed |
| mp_slice_2.test | 10 | 0.000468s | 0.000465s | 0.000992s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001036s | 0.001033s | 0.001029s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001034s | 0.001010s | 0.000977s | byte equality observed |
| mp_sort.test | 10 | 0.001023s | 0.001064s | 0.000995s | byte equality observed |
| mp_sort_q.test | 10 | 0.001023s | 0.001011s | 0.001007s | byte equality observed |
| mp_split.test | 10 | 0.001026s | 0.001027s | 0.000992s | byte equality observed |
| mp_starts_with.test | 10 | 0.001036s | 0.001062s | 0.001024s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001026s | 0.001020s | 0.001034s | byte equality observed |
| mp_take.test | 10 | 0.001038s | 0.001045s | 0.001020s | byte equality observed |
| mp_take_2.test | 10 | 0.000474s | 0.000464s | 0.001009s | byte equality observed |
| mp_third.test | 10 | 0.001041s | 0.001073s | 0.001008s | byte equality observed |
| mp_third_2.test | 10 | 0.000464s | 0.000469s | 0.001020s | byte equality observed |
| mp_transform.test | 10 | 0.001033s | 0.001036s | 0.001069s | byte equality observed |
| mp_transform_2.test | 10 | 0.000962s | 0.001007s | 0.001006s | byte equality observed |
| mp_transform_front.test | 10 | 0.001042s | 0.001071s | 0.001021s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000467s | 0.000475s | 0.001026s | byte equality observed |
| mp_transform_if.test | 10 | 0.001039s | 0.001006s | 0.001043s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001017s | 0.000994s | 0.001081s | byte equality observed |
| mp_transform_q.test | 10 | 0.001060s | 0.001040s | 0.001007s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001010s | 0.001061s | 0.001052s | byte equality observed |
| mp_transform_second.test | 10 | 0.001038s | 0.001055s | 0.001028s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000471s | 0.000478s | 0.001031s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001056s | 0.001054s | 0.001051s | byte equality observed |
| mp_transform_third.test | 10 | 0.001025s | 0.001048s | 0.001045s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000471s | 0.000472s | 0.001023s | byte equality observed |
| mp_unique.test | 10 | 0.001038s | 0.001067s | 0.001053s | byte equality observed |
| mp_unique_if.test | 10 | 0.001030s | 0.001054s | 0.001039s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001065s | 0.001022s | 0.001049s | byte equality observed |
| mp_valid.test | 10 | 0.000992s | 0.001022s | 0.001064s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001009s | 0.001065s | 0.001048s | byte equality observed |
| mp_value.test | 10 | 0.000473s | 0.000478s | 0.001011s | byte equality observed |
| mp_with_index.test | 10 | 0.001404s | 0.001398s | 0.001293s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000471s | 0.000481s | 0.000991s | byte equality observed |
| mpl.test | 10 | 0.001056s | 0.001051s | 0.001048s | byte equality observed |
| mpl_list.test | 10 | 0.001019s | 0.001066s | 0.001033s | byte equality observed |
| mpl_tuple.test | 10 | 0.001009s | 0.001028s | 0.001060s | byte equality observed |
| mpl_version.test | 10 | 0.001013s | 0.001051s | 0.001033s | byte equality observed |
| set_version.test | 10 | 0.001008s | 0.001047s | 0.001029s | byte equality observed |
| tuple_apply.test | 10 | 0.001007s | 0.001020s | 0.001058s | byte equality observed |
| tuple_for_each.test | 10 | 0.001031s | 0.001057s | 0.001059s | byte equality observed |
| tuple_transform.test | 10 | 0.001007s | 0.001024s | 0.001073s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001005s | 0.001037s | 0.001044s | byte equality observed |
| tuple_version.test | 10 | 0.000999s | 0.001024s | 0.001068s | byte equality observed |
| utility_version.test | 10 | 0.001042s | 0.001048s | 0.001040s | byte equality observed |
| version.test | 10 | 0.001023s | 0.001010s | 0.001025s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
