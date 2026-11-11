# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.041577 | 0.071391 | 0.124886 | — | — | — |
| complex_h.test | COMPILE | 0.164246 | 0.291491 | 0.471728 | — | — | — |
| mp_size.test | RUN | 0.193720 | 0.947102 | 0.577082 | 0.001570 | 0.001737 | 0.001783 |
| mp_size_2.test | RUN | 0.017200 | 0.034889 | 0.053178 | 0.000691 | 0.000725 | 0.001723 |
| mp_empty.test | RUN | 0.185698 | 0.576088 | 0.551843 | 0.001648 | 0.001850 | 0.001724 |
| mp_empty_2.test | RUN | 0.018327 | 0.037104 | 0.058314 | 0.000640 | 0.000764 | 0.001617 |
| mp_assign.test | RUN | 0.204342 | 1.226096 | 0.700272 | 0.001611 | 0.001779 | 0.001498 |
| mp_assign_2.test | RUN | 0.017602 | 0.037590 | 0.057433 | 0.000715 | 0.000788 | 0.001068 |
| mp_clear.test | RUN | 0.179623 | 0.528476 | 0.585616 | 0.001682 | 0.001545 | 0.001676 |
| mp_clear_2.test | RUN | 0.017893 | 0.037920 | 0.058084 | 0.000682 | 0.000658 | 0.001697 |
| mp_front.test | RUN | 0.192724 | 0.845682 | 0.669875 | 0.001755 | 0.001878 | 0.001651 |
| mp_front_2.test | RUN | 0.017504 | 0.035086 | 0.053660 | 0.000637 | 0.000524 | 0.001499 |
| mp_pop_front.test | RUN | 0.185390 | 0.871443 | 0.618747 | 0.001524 | 0.001685 | 0.001415 |
| mp_pop_front_2.test | RUN | 0.017658 | 0.034635 | 0.049410 | 0.000614 | 0.000700 | 0.001615 |
| mp_second.test | RUN | 0.187226 | 0.558931 | 0.601792 | 0.001551 | 0.001632 | 0.001604 |
| mp_second_2.test | RUN | 0.016973 | 0.037278 | 0.058417 | 0.000687 | 0.000770 | 0.001717 |
| mp_third.test | RUN | 0.180261 | 0.524819 | 0.581730 | 0.001406 | 0.001711 | 0.001601 |
| mp_third_2.test | RUN | 0.017081 | 0.037303 | 0.051816 | 0.000645 | 0.000626 | 0.001706 |
| mp_push_front.test | RUN | 0.270298 | 1.875065 | 1.031874 | 0.001678 | 0.001628 | 0.001789 |
| mp_push_front_2.test | RUN | 0.017905 | 0.034754 | 0.057142 | 0.000605 | 0.000682 | 0.001493 |
| mp_push_back.test | RUN | 0.271637 | 1.873213 | 1.047255 | 0.001700 | 0.001775 | 0.001722 |
| mp_push_back_2.test | RUN | 0.016808 | 0.035930 | 0.051750 | 0.000659 | 0.000699 | 0.001646 |
| mp_rename.test | RUN | 0.220295 | 1.701149 | 0.815327 | 0.001354 | 0.001722 | 0.001445 |
| mp_rename_2.test | RUN | 0.018154 | 0.037619 | 0.058302 | 0.000640 | 0.000772 | 0.001757 |
| mp_append.test | RUN | 0.221517 | 1.931962 | 0.940844 | 0.001601 | 0.001496 | 0.001487 |
| mp_append_2.test | RUN | 0.436625 | 4.299881 | 1.226901 | 0.001568 | 0.001830 | 0.001699 |
| mp_append_3.test | RUN | 0.017380 | 0.037349 | 0.056001 | 0.000725 | 0.000806 | 0.001574 |
| mp_append_sf.test | RUN | 0.177783 | 0.481508 | 0.529872 | 0.001618 | 0.001659 | 0.001608 |
| mp_replace_front.test | RUN | 0.215010 | 1.390165 | 0.814215 | 0.001756 | 0.001668 | 0.001581 |
| mp_replace_front_2.test | RUN | 0.017556 | 0.035653 | 0.049728 | 0.000643 | 0.000739 | 0.001704 |
| mp_replace_second.test | RUN | 0.205341 | 0.895660 | 0.732615 | 0.001539 | 0.001568 | 0.001684 |
| mp_replace_second_2.test | RUN | 0.016648 | 0.038049 | 0.058983 | 0.000658 | 0.000783 | 0.001614 |
| mp_replace_third.test | RUN | 0.201777 | 0.734336 | 0.716623 | 0.001726 | 0.001685 | 0.001728 |
| mp_replace_third_2.test | RUN | 0.018298 | 0.036607 | 0.058645 | 0.000580 | 0.000653 | 0.001646 |
| mp_apply_q.test | RUN | 0.224385 | 1.696885 | 0.891901 | 0.001716 | 0.001678 | 0.001556 |
| mp_apply_q_sf.test | RUN | 0.179254 | 0.503898 | 0.536766 | 0.001669 | 0.001656 | 0.001698 |
| mp_is_list.test | RUN | 0.192264 | 0.600226 | 0.583008 | 0.001408 | 0.001769 | 0.001613 |
| mp_list_c.test | RUN | 0.187233 | 0.713332 | 0.630693 | 0.001714 | 0.001807 | 0.001356 |
| mp_transform_front.test | RUN | 0.203348 | 1.366747 | 0.767178 | 0.001707 | 0.001729 | 0.001529 |
| mp_transform_front_2.test | RUN | 0.018115 | 0.036244 | 0.051406 | 0.000634 | 0.000733 | 0.001678 |
| mp_transform_second.test | RUN | 0.202882 | 1.199910 | 0.661035 | 0.001352 | 0.001753 | 0.001436 |
| mp_transform_second_2.test | RUN | 0.018600 | 0.036744 | 0.055980 | 0.000585 | 0.000722 | 0.001712 |
| mp_transform_third.test | RUN | 0.189063 | 0.916906 | 0.611480 | 0.001510 | 0.001524 | 0.001714 |
| mp_transform_third_2.test | RUN | 0.018396 | 0.034877 | 0.056009 | 0.000621 | 0.000589 | 0.001692 |
| mp_list_v.test | RUN | 0.016654 | 0.035101 | 0.052466 | 0.000598 | 0.000686 | 0.001782 |
| mp_rename_v.test | RUN | 0.017948 | 0.035272 | 0.051491 | 0.000638 | 0.000887 | 0.001585 |
| mp_is_value_list.test | RUN | 0.173555 | 0.364243 | 0.529785 | 0.001666 | 0.001680 | 0.001681 |
| mp_transform.test | RUN | 0.360729 | 2.884842 | 1.582360 | 0.001750 | 0.001885 | 0.001713 |
| mp_transform_2.test | RUN | 0.172298 | 0.336412 | 0.488556 | 0.001479 | 0.001806 | 0.001704 |
| mp_transform_q.test | RUN | 0.353400 | 2.744045 | 1.515369 | 0.001743 | 0.001636 | 0.001767 |
| mp_transform_q_2.test | RUN | 0.404879 | 0.892135 | 1.965721 | 0.001822 | 0.001615 | 0.001435 |
| mp_transform_sf.test | RUN | 0.190115 | 0.519713 | 0.594160 | 0.001802 | 0.001746 | 0.001738 |
| mp_transform_if.test | RUN | 0.224757 | 1.385969 | 0.862054 | 0.001762 | 0.001661 | 0.001607 |
| mp_transform_if_q.test | RUN | 0.235674 | 1.404280 | 0.864834 | 0.001772 | 0.001325 | 0.001304 |
| mp_filter.test | RUN | 0.212351 | 0.840114 | 0.686145 | 0.001679 | 0.001944 | 0.001584 |
| mp_fill.test | RUN | 0.195478 | 0.645573 | 0.619582 | 0.001772 | 0.001640 | 0.001774 |
| mp_fill_2.test | RUN | 0.030120 | 0.055049 | 0.099118 | 0.000622 | 0.000708 | 0.001646 |
| mp_count.test | RUN | 0.366887 | 1.492356 | 0.975457 | 0.001652 | 0.001564 | 0.001747 |
| mp_count_if.test | RUN | 0.371510 | 1.522430 | 1.079469 | 0.001814 | 0.001660 | 0.001575 |
| mp_count_if_q.test | RUN | 0.207713 | 0.975939 | 0.626995 | 0.001623 | 0.001569 | 0.001776 |
| mp_contains.test | RUN | 0.199363 | 0.601294 | 0.568159 | 0.001316 | 0.001738 | 0.001737 |
| mp_starts_with.test | RUN | 0.193906 | 0.606552 | 0.594447 | 0.001734 | 0.001296 | 0.001207 |
| mp_starts_with_sf.test | RUN | 0.187597 | 0.504556 | 0.546219 | 0.001532 | 0.001822 | 0.001558 |
| mp_repeat.test | RUN | 0.289406 | 2.445194 | 1.098443 | 0.001894 | 0.001554 | 0.001513 |
| mp_repeat_2.test | RUN | 0.031529 | 0.058632 | 0.096232 | 0.000810 | 0.000768 | 0.001684 |
| mp_repeat_3.test | RUN | 0.228959 | 0.336926 | 0.540535 | 0.001615 | 0.001782 | 0.001570 |
| mp_product.test | RUN | 0.218250 | 1.467028 | 0.815089 | 0.001676 | 0.001876 | 0.001620 |
| mp_drop.test | RUN | 0.225787 | 1.818661 | 0.873266 | 0.001625 | 0.001536 | 0.001666 |
| mp_drop_2.test | RUN | 0.032708 | 0.056552 | 0.086082 | 0.000580 | 0.000700 | 0.001730 |
| mp_iota.test | RUN | 0.224275 | 1.563953 | 0.810171 | 0.001692 | 0.001721 | 0.001526 |
| mp_iota_2.test | RUN | 0.241294 | 1.881972 | 0.881369 | 0.001657 | 0.001790 | 0.001746 |
| mp_at.test | RUN | 0.201056 | 0.832506 | 0.671398 | 0.001524 | 0.001658 | 0.001726 |
| mp_at_2.test | RUN | 0.030922 | 0.053465 | 0.096052 | 0.000775 | 0.000641 | 0.001714 |
| mp_at_sf.test | RUN | 0.192144 | 0.514254 | 0.581310 | 0.001736 | 0.001835 | 0.001526 |
| mp_take.test | RUN | 0.289468 | 2.424219 | 1.083403 | 0.001715 | 0.001678 | 0.001607 |
| mp_take_2.test | RUN | 0.032275 | 0.056509 | 0.100013 | 0.000625 | 0.000744 | 0.001603 |
| mp_slice.test | RUN | 0.324546 | 2.459778 | 1.403748 | 0.001715 | 0.001742 | 0.001676 |
| mp_slice_2.test | RUN | 0.031729 | 0.058002 | 0.084351 | 0.000744 | 0.000755 | 0.001711 |
| mp_replace.test | RUN | 0.231537 | 1.272685 | 0.878330 | 0.001710 | 0.001685 | 0.001684 |
| mp_replace_if.test | RUN | 0.220152 | 1.097442 | 0.832525 | 0.001897 | 0.001367 | 0.001749 |
| mp_replace_if_q.test | RUN | 0.220827 | 1.121334 | 0.807920 | 0.001668 | 0.001752 | 0.001757 |
| mp_copy_if.test | RUN | 0.226823 | 1.555699 | 0.789184 | 0.001646 | 0.001753 | 0.001688 |
| mp_copy_if_q.test | RUN | 0.229681 | 1.567032 | 0.789465 | 0.001816 | 0.001686 | 0.001698 |
| mp_remove.test | RUN | 0.219202 | 1.038385 | 0.797153 | 0.001840 | 0.001728 | 0.001711 |
| mp_remove_if.test | RUN | 0.236144 | 1.643067 | 0.833125 | 0.001415 | 0.001899 | 0.001681 |
| mp_remove_if_q.test | RUN | 0.236340 | 1.625840 | 0.836704 | 0.001652 | 0.001678 | 0.001710 |
| mp_partition.test | RUN | 0.228182 | 1.540246 | 0.893775 | 0.001565 | 0.001547 | 0.001762 |
| mp_partition_q.test | RUN | 0.233267 | 1.541493 | 0.887826 | 0.001791 | 0.001691 | 0.001689 |
| mp_sort.test | RUN | 0.216422 | 1.122475 | 0.783216 | 0.001686 | 0.001755 | 0.001721 |
| mp_sort_q.test | RUN | 0.219494 | 1.122947 | 0.782994 | 0.001756 | 0.001720 | 0.001714 |
| mp_find.test | RUN | 0.218302 | 1.218529 | 0.685003 | 0.001810 | 0.001885 | 0.001559 |
| mp_find_if.test | RUN | 0.207254 | 1.124907 | 0.642479 | 0.001753 | 0.001592 | 0.001471 |
| mp_find_if_q.test | RUN | 0.224639 | 1.237978 | 0.811479 | 0.001464 | 0.001880 | 0.001688 |
| mp_reverse.test | RUN | 0.390036 | 3.202541 | 1.510946 | 0.001748 | 0.001862 | 0.001635 |
| mp_fold.test | RUN | 0.664086 | 2.516660 | 3.793051 | 0.001854 | 0.001780 | 0.001660 |
| mp_fold_q.test | RUN | 0.213977 | 1.105480 | 0.731412 | 0.001789 | 0.001813 | 0.001716 |
| mp_fold_q_sf.test | RUN | 0.200972 | 0.532247 | 0.567410 | 0.001836 | 0.001809 | 0.001785 |
| mp_reverse_fold.test | RUN | 0.333000 | 2.399264 | 1.055096 | 0.001675 | 0.001817 | 0.001682 |
| mp_reverse_fold_q.test | RUN | 0.222615 | 1.350021 | 0.792569 | 0.001824 | 0.001780 | 0.001716 |
| mp_unique.test | RUN | 0.213831 | 1.208301 | 0.774381 | 0.001623 | 0.001800 | 0.001741 |
| mp_unique_if.test | RUN | 0.350623 | 2.501332 | 1.196362 | 0.001817 | 0.001759 | 0.001558 |
| mp_unique_if_q.test | RUN | 0.306001 | 2.440063 | 1.008358 | 0.001594 | 0.001680 | 0.001640 |
| mp_all_of.test | RUN | 0.516432 | 1.356621 | 1.347803 | 0.001664 | 0.001856 | 0.001665 |
| mp_all_of_q.test | RUN | 0.193245 | 0.609775 | 0.556949 | 0.001630 | 0.001885 | 0.001807 |
| mp_any_of.test | RUN | 0.494034 | 1.437988 | 1.292750 | 0.001678 | 0.001885 | 0.001521 |
| mp_any_of_q.test | RUN | 0.195909 | 0.605984 | 0.595814 | 0.001749 | 0.001736 | 0.001800 |
| mp_none_of.test | RUN | 0.489146 | 1.422635 | 1.257806 | 0.001651 | 0.002146 | 0.001732 |
| mp_none_of_q.test | RUN | 0.195708 | 0.606262 | 0.599164 | 0.001576 | 0.001873 | 0.001536 |
| mp_replace_at.test | RUN | 0.223815 | 1.166498 | 0.893414 | 0.001833 | 0.001836 | 0.001572 |
| mp_replace_at_c.test | RUN | 0.229634 | 1.206916 | 0.879827 | 0.001644 | 0.001597 | 0.001726 |
| mp_for_each.test | RUN | 0.218349 | 0.899310 | 0.969757 | 0.001564 | 0.001397 | 0.001734 |
| mp_insert.test | RUN | 0.314287 | 2.414874 | 1.225528 | 0.001615 | 0.001671 | 0.001678 |
| mp_insert_2.test | RUN | 0.032534 | 0.059127 | 0.094042 | 0.000672 | 0.000812 | 0.001423 |
| mp_erase.test | RUN | 0.336651 | 2.590359 | 1.489820 | 0.001866 | 0.001784 | 0.001570 |
| mp_erase_2.test | RUN | 0.031407 | 0.058945 | 0.094724 | 0.000663 | 0.000781 | 0.001642 |
| mp_with_index.test | RUN | 0.481344 | 5.300003 | 3.192173 | 0.002223 | 0.002103 | 0.001992 |
| mp_with_index_cx.test | RUN | 0.003465 | 0.017577 | 0.026928 | 0.000728 | 0.000735 | 0.001579 |
| mp_from_sequence.test | RUN | 0.230490 | 1.386147 | 0.840776 | 0.001860 | 0.001572 | 0.001694 |
| mp_from_sequence_2.test | RUN | 0.237311 | 1.616508 | 0.913010 | 0.001882 | 0.001608 | 0.001713 |
| mp_from_sequence_3.test | RUN | 0.203176 | 0.739760 | 0.612874 | 0.001738 | 0.001730 | 0.001770 |
| mp_min_element.test | RUN | 0.198432 | 0.669902 | 0.618393 | 0.001800 | 0.001821 | 0.001608 |
| mp_min_element_2.test | RUN | 0.031088 | 0.057197 | 0.095912 | 0.000652 | 0.000698 | 0.001802 |
| mp_min_element_q.test | RUN | 0.204124 | 0.664122 | 0.602917 | 0.001585 | 0.001825 | 0.001714 |
| mp_min_element_q_2.test | RUN | 0.030654 | 0.057424 | 0.089375 | 0.000764 | 0.000666 | 0.001609 |
| mp_max_element.test | RUN | 0.202749 | 0.664788 | 0.615875 | 0.001808 | 0.001797 | 0.001343 |
| mp_max_element_2.test | RUN | 0.031512 | 0.053267 | 0.089184 | 0.000605 | 0.000700 | 0.001637 |
| mp_max_element_q.test | RUN | 0.201225 | 0.672816 | 0.616230 | 0.001504 | 0.001865 | 0.001738 |
| mp_max_element_q_2.test | RUN | 0.032691 | 0.057190 | 0.088911 | 0.000653 | 0.000733 | 0.001600 |
| mp_nth_element.test | RUN | 0.217597 | 0.924466 | 0.766762 | 0.001693 | 0.001671 | 0.001338 |
| mp_nth_element_q.test | RUN | 0.213668 | 0.915494 | 0.709054 | 0.001595 | 0.001768 | 0.001658 |
| mp_back.test | RUN | 0.208939 | 0.774523 | 0.725177 | 0.001680 | 0.001552 | 0.001589 |
| mp_back_2.test | RUN | 0.032296 | 0.054645 | 0.094702 | 0.000658 | 0.000724 | 0.001654 |
| mp_pop_back.test | RUN | 0.487596 | 4.943612 | 1.415075 | 0.001763 | 0.001590 | 0.001594 |
| mp_pop_back_2.test | RUN | 0.032300 | 0.054727 | 0.090531 | 0.000748 | 0.000613 | 0.001625 |
| mp_flatten.test | RUN | 0.238060 | 1.767088 | 0.998874 | 0.001783 | 0.001759 | 0.001700 |
| mp_rotate_left.test | RUN | 0.235871 | 1.897519 | 0.992404 | 0.001857 | 0.001626 | 0.001650 |
| mp_rotate_right.test | RUN | 0.237860 | 1.903351 | 0.994400 | 0.001539 | 0.001611 | 0.001526 |
| mp_power_set.test | RUN | 0.235274 | 1.815702 | 0.965725 | 0.001674 | 0.001763 | 0.001800 |
| mp_partial_sum.test | RUN | 0.290292 | 2.428885 | 1.092504 | 0.001865 | 0.001773 | 0.001650 |
| mp_iterate.test | RUN | 0.214998 | 0.744858 | 0.790043 | 0.001496 | 0.001726 | 0.001517 |
| mp_pairwise_fold.test | RUN | 0.241537 | 1.679337 | 0.990150 | 0.001536 | 0.001884 | 0.001636 |
| mp_pairwise_fold_q.test | RUN | 0.196058 | 0.502193 | 0.606672 | 0.001819 | 0.001548 | 0.001766 |
| mp_sliding_fold.test | RUN | 0.407724 | 3.218242 | 1.824089 | 0.001446 | 0.001751 | 0.001476 |
| mp_sliding_fold_q.test | RUN | 0.402543 | 3.185859 | 1.813087 | 0.001423 | 0.001676 | 0.001567 |
| mp_intersperse.test | RUN | 0.222877 | 1.219128 | 0.890076 | 0.001771 | 0.001776 | 0.001731 |
| mp_split.test | RUN | 0.295999 | 2.306857 | 1.173859 | 0.001804 | 0.001714 | 0.001716 |
| mp_join.test | RUN | 0.285224 | 1.918393 | 1.057852 | 0.001834 | 0.001834 | 0.001728 |
| integral.test | RUN | 0.199131 | 0.858517 | 0.626275 | 0.001802 | 0.001856 | 0.001747 |
| mp_value.test | RUN | 0.014092 | 0.030724 | 0.046389 | 0.000644 | 0.000651 | 0.001706 |
| mp_identity.test | RUN | 0.189127 | 0.798862 | 0.624465 | 0.001778 | 0.001634 | 0.001738 |
| mp_inherit.test | RUN | 0.186968 | 0.566315 | 0.580510 | 0.001562 | 0.001764 | 0.001711 |
| mp_if.test | RUN | 0.186946 | 0.657207 | 0.587922 | 0.001828 | 0.001740 | 0.001734 |
| mp_if_sf.test | RUN | 0.181442 | 0.477496 | 0.522413 | 0.001778 | 0.001580 | 0.001683 |
| mp_eval_if.test | RUN | 0.188636 | 0.696694 | 0.597898 | 0.001784 | 0.001356 | 0.001704 |
| mp_eval_if_sf.test | RUN | 0.179475 | 0.497586 | 0.539163 | 0.001604 | 0.001902 | 0.001695 |
| mp_valid.test | RUN | 0.182552 | 0.508600 | 0.523193 | 0.001737 | 0.001691 | 0.001684 |
| mp_defer.test | RUN | 0.195372 | 0.844691 | 0.675325 | 0.001655 | 0.001834 | 0.001763 |
| mp_quote.test | RUN | 0.197133 | 0.844472 | 0.682623 | 0.001854 | 0.001826 | 0.001710 |
| mp_invoke_q.test | RUN | 0.206818 | 1.085508 | 0.729121 | 0.001640 | 0.001768 | 0.001914 |
| mp_invoke_q_sf.test | RUN | 0.180925 | 0.495582 | 0.531351 | 0.001690 | 0.001808 | 0.001658 |
| mp_quote_trait.test | RUN | 0.194893 | 0.928476 | 0.646198 | 0.001842 | 0.001508 | 0.001498 |
| mp_cond.test | RUN | 0.188073 | 0.563942 | 0.562635 | 0.001560 | 0.001717 | 0.001726 |
| mp_cond_sf.test | RUN | 0.179733 | 0.506526 | 0.544990 | 0.001608 | 0.001754 | 0.001760 |
| mp_not_fn.test | RUN | 0.181848 | 0.491142 | 0.521395 | 0.001739 | 0.001744 | 0.001423 |
| mp_eval_if_not.test | RUN | 0.191893 | 0.698493 | 0.596635 | 0.001724 | 0.001840 | 0.001722 |
| mp_eval_or.test | RUN | 0.181059 | 0.392114 | 0.559015 | 0.001546 | 0.001857 | 0.001754 |
| mp_compose.test | RUN | 0.199327 | 1.132653 | 0.683849 | 0.001618 | 0.001639 | 0.001740 |
| mp_valid_and_true.test | RUN | 0.182214 | 0.587731 | 0.566263 | 0.001764 | 0.001804 | 0.001651 |
| mp_compose_sf.test | COMPILE | 0.043613 | 0.075064 | 0.141154 | — | — | — |
| integer_sequence.test | RUN | 0.214741 | 1.128876 | 0.797031 | 0.001567 | 0.001564 | 0.001588 |
| tuple_for_each.test | RUN | 0.190512 | 0.426188 | 0.610884 | 0.001653 | 0.001580 | 0.001716 |
| tuple_for_each_cx.test | COMPILE | 0.035135 | 0.062648 | 0.096178 | — | — | — |
| tuple_apply.test | RUN | 0.193485 | 0.423825 | 0.610053 | 0.004240 | 0.001767 | 0.001668 |
| tuple_apply_cx.test | COMPILE | 0.053634 | 0.090622 | 0.151729 | — | — | — |
| construct_from_tuple.test | RUN | 0.210108 | 0.461539 | 0.597567 | 0.001667 | 0.001880 | 0.001578 |
| construct_from_tuple_cx.test | COMPILE | 0.055355 | 0.091088 | 0.137356 | — | — | — |
| tuple_transform.test | RUN | 0.337737 | 0.928571 | 1.143017 | 0.001935 | 0.001630 | 0.001702 |
| tuple_transform_2.test | RUN | 0.259541 | 0.699073 | 1.044370 | 0.001772 | 0.001684 | 0.001584 |
| tuple_transform_cx.test | COMPILE | 0.045063 | 0.083364 | 0.142853 | — | — | — |
| mp_set_contains.test | RUN | 0.187775 | 0.602870 | 0.610548 | 0.001761 | 0.001600 | 0.001429 |
| mp_set_push_back.test | RUN | 0.236294 | 1.823007 | 0.981864 | 0.001768 | 0.001860 | 0.001727 |
| mp_set_push_front.test | RUN | 0.286008 | 2.183313 | 1.111824 | 0.001623 | 0.001798 | 0.001313 |
| mp_is_set.test | RUN | 0.187004 | 0.599987 | 0.617559 | 0.001850 | 0.001788 | 0.001636 |
| mp_set_union.test | RUN | 0.227672 | 1.548997 | 0.919889 | 0.001665 | 0.001854 | 0.001718 |
| mp_set_union_sf.test | RUN | 0.187644 | 0.489767 | 0.569555 | 0.001532 | 0.001741 | 0.001404 |
| mp_set_difference.test | RUN | 0.211259 | 1.220418 | 0.793322 | 0.001766 | 0.001714 | 0.001583 |
| mp_set_difference_sf.test | RUN | 0.179528 | 0.476185 | 0.554848 | 0.001564 | 0.001794 | 0.001304 |
| mp_set_intersection.test | RUN | 0.206279 | 1.201032 | 0.790320 | 0.001476 | 0.001532 | 0.001747 |
| mp_set_intersection_sf.test | RUN | 0.183617 | 0.479087 | 0.567387 | 0.001740 | 0.001808 | 0.001615 |
| mp_all.test | RUN | 0.187769 | 0.603001 | 0.591000 | 0.001853 | 0.001713 | 0.001549 |
| mp_all_2.test | RUN | 0.497671 | 1.301050 | 1.331450 | 0.001820 | 0.001868 | 0.001756 |
| mp_and.test | RUN | 0.184739 | 0.582122 | 0.578028 | 0.001771 | 0.001748 | 0.001646 |
| mp_any.test | RUN | 0.188082 | 0.594345 | 0.617772 | 0.001612 | 0.001706 | 0.001762 |
| mp_any_2.test | RUN | 0.494577 | 1.274292 | 1.308765 | 0.001544 | 0.001758 | 0.001457 |
| mp_or.test | RUN | 0.186855 | 0.604130 | 0.558621 | 0.001706 | 0.001860 | 0.001741 |
| mp_same.test | RUN | 0.343215 | 0.988690 | 0.888122 | 0.001757 | 0.001600 | 0.001446 |
| mp_plus.test | RUN | 0.192164 | 0.703692 | 0.671470 | 0.001556 | 0.001667 | 0.001335 |
| mp_less.test | RUN | 0.190581 | 0.597898 | 0.607702 | 0.001830 | 0.001594 | 0.001665 |
| mp_min.test | RUN | 0.187967 | 0.545312 | 0.573074 | 0.001670 | 0.001889 | 0.001595 |
| mp_max.test | RUN | 0.187722 | 0.608763 | 0.579038 | 0.001640 | 0.001488 | 0.001376 |
| mp_similar.test | RUN | 0.191366 | 0.586625 | 0.580924 | 0.001594 | 0.001774 | 0.001473 |
| mp_map_find.test | RUN | 0.225465 | 1.087786 | 0.768847 | 0.001840 | 0.001332 | 0.001726 |
| mp_map_find_2.test | RUN | 0.208354 | 0.428070 | 0.647429 | 0.001785 | 0.001621 | 0.001734 |
| mp_map_find_3.test | RUN | 0.204462 | 0.426259 | 0.667404 | 0.001686 | 0.001410 | 0.001640 |
| mp_map_find_4.test | RUN | 0.196971 | 0.404656 | 0.620586 | 0.001592 | 0.001739 | 0.001636 |
| mp_map_contains.test | RUN | 0.200032 | 0.619157 | 0.580120 | 0.001881 | 0.001834 | 0.001739 |
| mp_map_insert.test | RUN | 0.286328 | 2.020863 | 1.068377 | 0.001594 | 0.001797 | 0.001650 |
| mp_map_replace.test | RUN | 0.302002 | 2.305091 | 1.218344 | 0.001535 | 0.001785 | 0.001707 |
| mp_map_erase.test | RUN | 0.249494 | 1.713978 | 1.004937 | 0.001596 | 0.001803 | 0.001530 |
| mp_map_update.test | RUN | 0.223410 | 1.127883 | 0.803582 | 0.001802 | 0.001759 | 0.001686 |
| mp_map_update_q.test | RUN | 0.215586 | 1.134579 | 0.798062 | 0.001532 | 0.001733 | 0.001606 |
| mp_map_keys.test | RUN | 0.210578 | 0.987264 | 0.778471 | 0.001868 | 0.001720 | 0.001467 |
| mp_is_map.test | RUN | 0.198480 | 0.617290 | 0.650843 | 0.001573 | 0.001709 | 0.001661 |
| mp_bind.test | RUN | 0.238577 | 1.796908 | 0.919094 | 0.001826 | 0.001634 | 0.001752 |
| mp_bind_q.test | RUN | 0.212872 | 1.113369 | 0.768045 | 0.001616 | 0.001766 | 0.001570 |
| mp_bind_front.test | RUN | 0.207917 | 1.214506 | 0.748786 | 0.001353 | 0.001750 | 0.001237 |
| mp_bind_back.test | RUN | 0.207533 | 1.220544 | 0.707704 | 0.001714 | 0.001756 | 0.001365 |
| mp_lambda.test | RUN | 0.411972 | 3.660754 | 1.862307 | 0.001986 | 0.001335 | 0.001380 |
| mpl.test | RUN | 0.405733 | 0.966414 | 1.512513 | 0.001772 | 0.001724 | 0.001371 |
| mpl_list.test | RUN | 0.247251 | 0.735471 | 0.718019 | 0.001286 | 0.001750 | 0.001668 |
| mpl_tuple.test | RUN | 0.246268 | 0.750441 | 0.732448 | 0.001573 | 0.001605 | 0.001708 |
| version.test | RUN | 0.123890 | 0.272816 | 0.352760 | 0.001754 | 0.001826 | 0.001590 |
| mp11_version.test | RUN | 0.150172 | 0.308830 | 0.454426 | 0.001337 | 0.001726 | 0.001442 |
| list_version.test | RUN | 0.127925 | 0.274327 | 0.386475 | 0.001585 | 0.001750 | 0.001597 |
| algorithm_version.test | RUN | 0.146285 | 0.299390 | 0.406348 | 0.001741 | 0.001694 | 0.001716 |
| integral_version.test | RUN | 0.125173 | 0.267659 | 0.374427 | 0.001790 | 0.001757 | 0.001582 |
| utility_version.test | RUN | 0.128173 | 0.279597 | 0.393135 | 0.001500 | 0.001666 | 0.001702 |
| function_version.test | RUN | 0.128711 | 0.273979 | 0.393343 | 0.001660 | 0.001668 | 0.001472 |
| map_version.test | RUN | 0.144652 | 0.301776 | 0.409941 | 0.001815 | 0.001750 | 0.001753 |
| set_version.test | RUN | 0.130696 | 0.281105 | 0.387166 | 0.001886 | 0.001648 | 0.001528 |
| bind_version.test | RUN | 0.139097 | 0.291844 | 0.419235 | 0.001620 | 0.001671 | 0.001723 |
| integer_sequence_version.test | RUN | 0.124234 | 0.273406 | 0.316580 | 0.001397 | 0.001788 | 0.001306 |
| tuple_version.test | RUN | 0.135566 | 0.300496 | 0.379893 | 0.001357 | 0.001826 | 0.001300 |
| mpl_version.test | RUN | 0.101750 | 0.205568 | 0.388060 | 0.001321 | 0.001324 | 0.001213 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.008× | 1.009×, 1.007× | 1.005× |
| clang | 1.114×† | 1.077×, 1.152× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000988s | 0.001014s | 0.001018s | byte equality observed |
| bind_version.test | 10 | 0.000987s | 0.000986s | 0.001012s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001007s | 0.000995s | 0.001031s | byte equality observed |
| function_version.test | 10 | 0.000985s | 0.000985s | 0.001025s | byte equality observed |
| integer_sequence.test | 10 | 0.000993s | 0.001012s | 0.001070s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000992s | 0.001000s | 0.001029s | byte equality observed |
| integral.test | 10 | 0.001002s | 0.001020s | 0.001034s | byte equality observed |
| integral_version.test | 10 | 0.000996s | 0.000992s | 0.001077s | byte equality observed |
| list_version.test | 10 | 0.001022s | 0.000982s | 0.001031s | byte equality observed |
| map_version.test | 10 | 0.001015s | 0.000984s | 0.001021s | byte equality observed |
| mp11_version.test | 10 | 0.001010s | 0.000984s | 0.001037s | byte equality observed |
| mp_all.test | 10 | 0.001009s | 0.001006s | 0.001053s | byte equality observed |
| mp_all_2.test | 10 | 0.001019s | 0.000992s | 0.001083s | byte equality observed |
| mp_all_of.test | 10 | 0.001021s | 0.001038s | 0.001060s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001025s | 0.001041s | 0.001021s | byte equality observed |
| mp_and.test | 10 | 0.001039s | 0.001080s | 0.001109s | byte equality observed |
| mp_any.test | 10 | 0.001032s | 0.001047s | 0.001034s | byte equality observed |
| mp_any_2.test | 10 | 0.001025s | 0.001065s | 0.001051s | byte equality observed |
| mp_any_of.test | 10 | 0.001044s | 0.001031s | 0.001057s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001014s | 0.001056s | 0.001007s | byte equality observed |
| mp_append.test | 10 | 0.001014s | 0.001041s | 0.001034s | byte equality observed |
| mp_append_2.test | 10 | 0.001054s | 0.001072s | 0.001019s | byte equality observed |
| mp_append_3.test | 10 | 0.000479s | 0.000486s | 0.001028s | byte equality observed |
| mp_append_sf.test | 10 | 0.001036s | 0.001034s | 0.001024s | byte equality observed |
| mp_apply_q.test | 10 | 0.001026s | 0.001033s | 0.001040s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001018s | 0.001022s | 0.001011s | byte equality observed |
| mp_assign.test | 10 | 0.001039s | 0.001045s | 0.001039s | byte equality observed |
| mp_assign_2.test | 10 | 0.000483s | 0.000488s | 0.001028s | byte equality observed |
| mp_at.test | 10 | 0.001051s | 0.001071s | 0.001017s | byte equality observed |
| mp_at_2.test | 10 | 0.000478s | 0.000480s | 0.001024s | byte equality observed |
| mp_at_sf.test | 10 | 0.001036s | 0.001039s | 0.001022s | byte equality observed |
| mp_back.test | 10 | 0.001045s | 0.001037s | 0.001086s | byte equality observed |
| mp_back_2.test | 10 | 0.000481s | 0.000482s | 0.001032s | byte equality observed |
| mp_bind.test | 10 | 0.001036s | 0.001059s | 0.001060s | byte equality observed |
| mp_bind_back.test | 10 | 0.001015s | 0.001084s | 0.001063s | byte equality observed |
| mp_bind_front.test | 10 | 0.001019s | 0.001053s | 0.001044s | byte equality observed |
| mp_bind_q.test | 10 | 0.001016s | 0.001016s | 0.001033s | byte equality observed |
| mp_clear.test | 10 | 0.001043s | 0.001064s | 0.001030s | byte equality observed |
| mp_clear_2.test | 10 | 0.000476s | 0.000475s | 0.001046s | byte equality observed |
| mp_compose.test | 10 | 0.001022s | 0.001058s | 0.001054s | byte equality observed |
| mp_cond.test | 10 | 0.001058s | 0.001022s | 0.001070s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001044s | 0.001044s | 0.001024s | byte equality observed |
| mp_contains.test | 10 | 0.001019s | 0.001062s | 0.001062s | byte equality observed |
| mp_copy_if.test | 10 | 0.001043s | 0.001045s | 0.001063s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001036s | 0.001096s | 0.001037s | byte equality observed |
| mp_count.test | 10 | 0.001025s | 0.001029s | 0.001039s | byte equality observed |
| mp_count_if.test | 10 | 0.001022s | 0.001054s | 0.001035s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001031s | 0.001019s | 0.001082s | byte equality observed |
| mp_defer.test | 10 | 0.001030s | 0.001050s | 0.001057s | byte equality observed |
| mp_drop.test | 10 | 0.001047s | 0.001079s | 0.001023s | byte equality observed |
| mp_drop_2.test | 10 | 0.000483s | 0.000479s | 0.001032s | byte equality observed |
| mp_empty.test | 10 | 0.001044s | 0.001043s | 0.001033s | byte equality observed |
| mp_empty_2.test | 10 | 0.000479s | 0.000481s | 0.001037s | byte equality observed |
| mp_erase.test | 10 | 0.001069s | 0.001061s | 0.001037s | byte equality observed |
| mp_erase_2.test | 10 | 0.000480s | 0.000480s | 0.001010s | byte equality observed |
| mp_eval_if.test | 10 | 0.001028s | 0.001037s | 0.001022s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001039s | 0.001020s | 0.001052s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001028s | 0.001028s | 0.001058s | byte equality observed |
| mp_eval_or.test | 10 | 0.001046s | 0.001040s | 0.001062s | byte equality observed |
| mp_fill.test | 10 | 0.001030s | 0.001038s | 0.001015s | byte equality observed |
| mp_fill_2.test | 10 | 0.000481s | 0.000481s | 0.001026s | byte equality observed |
| mp_filter.test | 10 | 0.001021s | 0.001044s | 0.001050s | byte equality observed |
| mp_find.test | 10 | 0.001014s | 0.001050s | 0.001032s | byte equality observed |
| mp_find_if.test | 10 | 0.001019s | 0.001050s | 0.001007s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001014s | 0.001051s | 0.001053s | byte equality observed |
| mp_flatten.test | 10 | 0.001014s | 0.001034s | 0.001041s | byte equality observed |
| mp_fold.test | 10 | 0.001057s | 0.001062s | 0.001029s | byte equality observed |
| mp_fold_q.test | 10 | 0.001017s | 0.001042s | 0.001065s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001033s | 0.001026s | 0.001044s | byte equality observed |
| mp_for_each.test | 10 | 0.001030s | 0.001010s | 0.001033s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001020s | 0.001033s | 0.001035s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001011s | 0.001020s | 0.001037s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001021s | 0.001007s | 0.001036s | byte equality observed |
| mp_front.test | 10 | 0.001029s | 0.001031s | 0.001019s | byte equality observed |
| mp_front_2.test | 10 | 0.000484s | 0.000477s | 0.001021s | byte equality observed |
| mp_identity.test | 10 | 0.001039s | 0.001020s | 0.001038s | byte equality observed |
| mp_if.test | 10 | 0.001025s | 0.001052s | 0.001049s | byte equality observed |
| mp_if_sf.test | 10 | 0.001018s | 0.001030s | 0.001055s | byte equality observed |
| mp_inherit.test | 10 | 0.001037s | 0.001008s | 0.001050s | byte equality observed |
| mp_insert.test | 10 | 0.001047s | 0.001063s | 0.001059s | byte equality observed |
| mp_insert_2.test | 10 | 0.000475s | 0.000471s | 0.001054s | byte equality observed |
| mp_intersperse.test | 10 | 0.001018s | 0.001038s | 0.001022s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001026s | 0.001034s | 0.001034s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001021s | 0.001048s | 0.001022s | byte equality observed |
| mp_iota.test | 10 | 0.001037s | 0.001021s | 0.001054s | byte equality observed |
| mp_iota_2.test | 10 | 0.001030s | 0.001023s | 0.001063s | byte equality observed |
| mp_is_list.test | 10 | 0.001030s | 0.001018s | 0.001079s | byte equality observed |
| mp_is_map.test | 10 | 0.001017s | 0.001021s | 0.001038s | byte equality observed |
| mp_is_set.test | 10 | 0.001043s | 0.001064s | 0.001008s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001041s | 0.001029s | 0.001045s | byte equality observed |
| mp_iterate.test | 10 | 0.001040s | 0.001018s | 0.001005s | byte equality observed |
| mp_join.test | 10 | 0.001057s | 0.001014s | 0.001042s | byte equality observed |
| mp_lambda.test | 10 | 0.001063s | 0.001039s | 0.000997s | byte equality observed |
| mp_less.test | 10 | 0.001037s | 0.001022s | 0.001004s | byte equality observed |
| mp_list_c.test | 10 | 0.001032s | 0.001021s | 0.001051s | byte equality observed |
| mp_list_v.test | 10 | 0.000482s | 0.000483s | 0.001004s | byte equality observed |
| mp_map_contains.test | 10 | 0.001069s | 0.001036s | 0.001042s | byte equality observed |
| mp_map_erase.test | 10 | 0.001015s | 0.001025s | 0.001021s | byte equality observed |
| mp_map_find.test | 10 | 0.001011s | 0.001015s | 0.001046s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001010s | 0.001009s | 0.001080s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001006s | 0.001056s | 0.001080s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001010s | 0.001018s | 0.001058s | byte equality observed |
| mp_map_insert.test | 10 | 0.001039s | 0.001023s | 0.001069s | byte equality observed |
| mp_map_keys.test | 10 | 0.001010s | 0.001053s | 0.001017s | byte equality observed |
| mp_map_replace.test | 10 | 0.001027s | 0.001025s | 0.001054s | byte equality observed |
| mp_map_update.test | 10 | 0.001002s | 0.001058s | 0.001029s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001012s | 0.001053s | 0.001033s | byte equality observed |
| mp_max.test | 10 | 0.001034s | 0.001031s | 0.001053s | byte equality observed |
| mp_max_element.test | 10 | 0.001042s | 0.001048s | 0.001027s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000477s | 0.000485s | 0.001020s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001043s | 0.001057s | 0.001021s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000478s | 0.000482s | 0.001046s | byte equality observed |
| mp_min.test | 10 | 0.001020s | 0.001037s | 0.001030s | byte equality observed |
| mp_min_element.test | 10 | 0.001036s | 0.001045s | 0.001018s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000475s | 0.000468s | 0.001030s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001053s | 0.001069s | 0.001017s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000476s | 0.000484s | 0.001045s | byte equality observed |
| mp_none_of.test | 10 | 0.001028s | 0.001041s | 0.001029s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001038s | 0.001046s | 0.001043s | byte equality observed |
| mp_not_fn.test | 10 | 0.001058s | 0.001043s | 0.001061s | byte equality observed |
| mp_nth_element.test | 10 | 0.001019s | 0.001019s | 0.001089s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001025s | 0.001020s | 0.001030s | byte equality observed |
| mp_or.test | 10 | 0.001028s | 0.001027s | 0.001027s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001048s | 0.001030s | 0.001036s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001025s | 0.001010s | 0.001029s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001034s | 0.001045s | 0.001062s | byte equality observed |
| mp_partition.test | 10 | 0.001018s | 0.001013s | 0.001055s | byte equality observed |
| mp_partition_q.test | 10 | 0.001031s | 0.001036s | 0.001069s | byte equality observed |
| mp_plus.test | 10 | 0.001024s | 0.001008s | 0.001031s | byte equality observed |
| mp_pop_back.test | 10 | 0.001063s | 0.001067s | 0.001040s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000483s | 0.000481s | 0.001021s | byte equality observed |
| mp_pop_front.test | 10 | 0.001027s | 0.001053s | 0.001032s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000477s | 0.000480s | 0.001042s | byte equality observed |
| mp_power_set.test | 10 | 0.001021s | 0.001042s | 0.001059s | byte equality observed |
| mp_product.test | 10 | 0.001033s | 0.001056s | 0.001065s | byte equality observed |
| mp_push_back.test | 10 | 0.001070s | 0.001053s | 0.001067s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000472s | 0.000494s | 0.001021s | byte equality observed |
| mp_push_front.test | 10 | 0.001060s | 0.001055s | 0.001045s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000472s | 0.000480s | 0.001020s | byte equality observed |
| mp_quote.test | 10 | 0.001054s | 0.001012s | 0.001007s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001021s | 0.001050s | 0.001021s | byte equality observed |
| mp_remove.test | 10 | 0.001004s | 0.001027s | 0.001007s | byte equality observed |
| mp_remove_if.test | 10 | 0.001022s | 0.001036s | 0.001023s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001024s | 0.001041s | 0.001021s | byte equality observed |
| mp_rename.test | 10 | 0.001025s | 0.001042s | 0.001045s | byte equality observed |
| mp_rename_2.test | 10 | 0.000479s | 0.000496s | 0.001022s | byte equality observed |
| mp_rename_v.test | 10 | 0.000477s | 0.000501s | 0.001030s | byte equality observed |
| mp_repeat.test | 10 | 0.001073s | 0.001063s | 0.001006s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000479s | 0.000478s | 0.001046s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001022s | 0.001016s | 0.001032s | byte equality observed |
| mp_replace.test | 10 | 0.001031s | 0.001086s | 0.001013s | byte equality observed |
| mp_replace_at.test | 10 | 0.001025s | 0.001034s | 0.001041s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001012s | 0.001028s | 0.001043s | byte equality observed |
| mp_replace_front.test | 10 | 0.001037s | 0.001025s | 0.001031s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000480s | 0.000484s | 0.001026s | byte equality observed |
| mp_replace_if.test | 10 | 0.001013s | 0.001031s | 0.001039s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001018s | 0.001041s | 0.001045s | byte equality observed |
| mp_replace_second.test | 10 | 0.001037s | 0.001058s | 0.001028s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000459s | 0.000476s | 0.001037s | byte equality observed |
| mp_replace_third.test | 10 | 0.001032s | 0.001047s | 0.001027s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000482s | 0.000479s | 0.001004s | byte equality observed |
| mp_reverse.test | 10 | 0.001040s | 0.001051s | 0.001072s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001066s | 0.001030s | 0.001037s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001010s | 0.001041s | 0.001066s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001039s | 0.001029s | 0.001056s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001037s | 0.001036s | 0.001078s | byte equality observed |
| mp_same.test | 10 | 0.001019s | 0.001056s | 0.001037s | byte equality observed |
| mp_second.test | 10 | 0.001036s | 0.001048s | 0.001036s | byte equality observed |
| mp_second_2.test | 10 | 0.000475s | 0.000485s | 0.001031s | byte equality observed |
| mp_set_contains.test | 10 | 0.001024s | 0.001042s | 0.001021s | byte equality observed |
| mp_set_difference.test | 10 | 0.001023s | 0.001032s | 0.001061s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001035s | 0.001025s | 0.001059s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001026s | 0.001067s | 0.001070s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001033s | 0.001014s | 0.001041s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001019s | 0.001031s | 0.001099s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001049s | 0.001046s | 0.001031s | byte equality observed |
| mp_set_union.test | 10 | 0.001017s | 0.001018s | 0.001042s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001007s | 0.001039s | 0.001054s | byte equality observed |
| mp_similar.test | 10 | 0.001028s | 0.001023s | 0.001043s | byte equality observed |
| mp_size.test | 10 | 0.001036s | 0.001053s | 0.001039s | byte equality observed |
| mp_size_2.test | 10 | 0.000476s | 0.000474s | 0.001014s | byte equality observed |
| mp_slice.test | 10 | 0.001062s | 0.001041s | 0.001017s | byte equality observed |
| mp_slice_2.test | 10 | 0.000480s | 0.000476s | 0.001007s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001039s | 0.001054s | 0.001021s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001035s | 0.001043s | 0.001042s | byte equality observed |
| mp_sort.test | 10 | 0.001037s | 0.001057s | 0.001043s | byte equality observed |
| mp_sort_q.test | 10 | 0.001022s | 0.001027s | 0.001032s | byte equality observed |
| mp_split.test | 10 | 0.001032s | 0.001056s | 0.001026s | byte equality observed |
| mp_starts_with.test | 10 | 0.001010s | 0.001035s | 0.001033s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001017s | 0.001024s | 0.001037s | byte equality observed |
| mp_take.test | 10 | 0.001047s | 0.001071s | 0.001059s | byte equality observed |
| mp_take_2.test | 10 | 0.000479s | 0.000474s | 0.001004s | byte equality observed |
| mp_third.test | 10 | 0.001044s | 0.001055s | 0.001034s | byte equality observed |
| mp_third_2.test | 10 | 0.000482s | 0.000476s | 0.001033s | byte equality observed |
| mp_transform.test | 10 | 0.001045s | 0.001024s | 0.001024s | byte equality observed |
| mp_transform_2.test | 10 | 0.001023s | 0.001026s | 0.001046s | byte equality observed |
| mp_transform_front.test | 10 | 0.001043s | 0.001037s | 0.001063s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000484s | 0.000483s | 0.001021s | byte equality observed |
| mp_transform_if.test | 10 | 0.001017s | 0.001060s | 0.001067s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001044s | 0.001036s | 0.001022s | byte equality observed |
| mp_transform_q.test | 10 | 0.001051s | 0.001069s | 0.001019s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001044s | 0.001010s | 0.001028s | byte equality observed |
| mp_transform_second.test | 10 | 0.001031s | 0.001041s | 0.001047s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000477s | 0.000479s | 0.001041s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001010s | 0.001036s | 0.001043s | byte equality observed |
| mp_transform_third.test | 10 | 0.001033s | 0.001030s | 0.001085s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000482s | 0.000477s | 0.001003s | byte equality observed |
| mp_unique.test | 10 | 0.001043s | 0.001037s | 0.001060s | byte equality observed |
| mp_unique_if.test | 10 | 0.001057s | 0.001034s | 0.001025s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001035s | 0.001082s | 0.001030s | byte equality observed |
| mp_valid.test | 10 | 0.001032s | 0.001035s | 0.001018s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001044s | 0.001049s | 0.001041s | byte equality observed |
| mp_value.test | 10 | 0.000473s | 0.000472s | 0.001034s | byte equality observed |
| mp_with_index.test | 10 | 0.001404s | 0.001370s | 0.001302s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000474s | 0.000479s | 0.000998s | byte equality observed |
| mpl.test | 10 | 0.001034s | 0.001006s | 0.001029s | byte equality observed |
| mpl_list.test | 10 | 0.001018s | 0.001025s | 0.001086s | byte equality observed |
| mpl_tuple.test | 10 | 0.001017s | 0.001022s | 0.001040s | byte equality observed |
| mpl_version.test | 10 | 0.001010s | 0.001024s | 0.001085s | byte equality observed |
| set_version.test | 10 | 0.001039s | 0.001042s | 0.001072s | byte equality observed |
| tuple_apply.test | 10 | 0.001038s | 0.001020s | 0.001053s | byte equality observed |
| tuple_for_each.test | 10 | 0.001026s | 0.001006s | 0.001034s | byte equality observed |
| tuple_transform.test | 10 | 0.001018s | 0.001032s | 0.001049s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001052s | 0.001015s | 0.001053s | byte equality observed |
| tuple_version.test | 10 | 0.001021s | 0.001013s | 0.001033s | byte equality observed |
| utility_version.test | 10 | 0.001025s | 0.001029s | 0.001050s | byte equality observed |
| version.test | 10 | 0.001018s | 0.001037s | 0.001083s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
