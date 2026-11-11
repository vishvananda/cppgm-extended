# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.035776 | 0.059391 | 0.088721 | — | — | — |
| complex_h.test | COMPILE | 0.145825 | 0.290223 | 0.318515 | — | — | — |
| mp_size.test | RUN | 0.173705 | 0.871726 | 0.467836 | 0.001670 | 0.001562 | 0.001705 |
| mp_size_2.test | RUN | 0.015344 | 0.030741 | 0.041707 | 0.000833 | 0.000828 | 0.001694 |
| mp_empty.test | RUN | 0.164917 | 0.578773 | 0.430812 | 0.001643 | 0.001862 | 0.001506 |
| mp_empty_2.test | RUN | 0.016401 | 0.046414 | 0.043600 | 0.000631 | 0.000924 | 0.001688 |
| mp_assign.test | RUN | 0.178089 | 1.146181 | 0.565067 | 0.001575 | 0.001942 | 0.001712 |
| mp_assign_2.test | RUN | 0.016585 | 0.033378 | 0.045937 | 0.000607 | 0.000933 | 0.001432 |
| mp_clear.test | RUN | 0.162329 | 0.513893 | 0.451479 | 0.001713 | 0.002076 | 0.001724 |
| mp_clear_2.test | RUN | 0.015385 | 0.033253 | 0.049680 | 0.000844 | 0.000566 | 0.001708 |
| mp_front.test | RUN | 0.174071 | 0.770129 | 0.488371 | 0.001653 | 0.001539 | 0.001902 |
| mp_front_2.test | RUN | 0.015521 | 0.034472 | 0.044902 | 0.000595 | 0.000865 | 0.001763 |
| mp_pop_front.test | RUN | 0.163249 | 0.832043 | 0.475169 | 0.001445 | 0.002282 | 0.001595 |
| mp_pop_front_2.test | RUN | 0.016143 | 0.032504 | 0.044752 | 0.000561 | 0.000714 | 0.001705 |
| mp_second.test | RUN | 0.164921 | 0.523086 | 0.435130 | 0.001656 | 0.001924 | 0.001743 |
| mp_second_2.test | RUN | 0.015105 | 0.031038 | 0.048199 | 0.000742 | 0.000785 | 0.001416 |
| mp_third.test | RUN | 0.161866 | 0.477568 | 0.413393 | 0.001776 | 0.001604 | 0.001484 |
| mp_third_2.test | RUN | 0.016218 | 0.033622 | 0.046910 | 0.000748 | 0.000856 | 0.001706 |
| mp_push_front.test | RUN | 0.217385 | 1.748936 | 0.778663 | 0.001666 | 0.001920 | 0.001491 |
| mp_push_front_2.test | RUN | 0.016104 | 0.032625 | 0.046469 | 0.000628 | 0.000831 | 0.001751 |
| mp_push_back.test | RUN | 0.228460 | 1.745730 | 0.764223 | 0.001794 | 0.001798 | 0.001696 |
| mp_push_back_2.test | RUN | 0.015071 | 0.032840 | 0.047016 | 0.000788 | 0.000881 | 0.001793 |
| mp_rename.test | RUN | 0.185152 | 1.580519 | 0.684550 | 0.001340 | 0.001816 | 0.001635 |
| mp_rename_2.test | RUN | 0.016755 | 0.034110 | 0.046781 | 0.000587 | 0.000917 | 0.001750 |
| mp_append.test | RUN | 0.195430 | 1.776633 | 0.747489 | 0.001559 | 0.001926 | 0.001845 |
| mp_append_2.test | RUN | 0.368986 | 3.972074 | 1.009229 | 0.001824 | 0.001618 | 0.001753 |
| mp_append_3.test | RUN | 0.017017 | 0.033562 | 0.048952 | 0.000807 | 0.000858 | 0.001676 |
| mp_append_sf.test | RUN | 0.159237 | 0.459992 | 0.412864 | 0.001748 | 0.001642 | 0.001726 |
| mp_replace_front.test | RUN | 0.188190 | 1.216900 | 0.592272 | 0.001639 | 0.001959 | 0.001827 |
| mp_replace_front_2.test | RUN | 0.015727 | 0.035531 | 0.048563 | 0.000630 | 0.000798 | 0.001497 |
| mp_replace_second.test | RUN | 0.187040 | 0.813854 | 0.557881 | 0.001460 | 0.001758 | 0.001845 |
| mp_replace_second_2.test | RUN | 0.016388 | 0.032343 | 0.047158 | 0.000584 | 0.000818 | 0.001541 |
| mp_replace_third.test | RUN | 0.174897 | 0.648042 | 0.501346 | 0.001627 | 0.001935 | 0.001820 |
| mp_replace_third_2.test | RUN | 0.014436 | 0.034375 | 0.049974 | 0.000748 | 0.000834 | 0.001825 |
| mp_apply_q.test | RUN | 0.193987 | 1.563847 | 0.704296 | 0.001833 | 0.001821 | 0.001748 |
| mp_apply_q_sf.test | RUN | 0.162605 | 0.432251 | 0.432351 | 0.001670 | 0.001828 | 0.001640 |
| mp_is_list.test | RUN | 0.163553 | 0.519037 | 0.429378 | 0.001545 | 0.002992 | 0.001486 |
| mp_list_c.test | RUN | 0.165441 | 0.690924 | 0.506425 | 0.001613 | 0.001900 | 0.001644 |
| mp_transform_front.test | RUN | 0.183845 | 1.243224 | 0.589840 | 0.001922 | 0.001824 | 0.001734 |
| mp_transform_front_2.test | RUN | 0.016102 | 0.034237 | 0.045536 | 0.000564 | 0.000787 | 0.001509 |
| mp_transform_second.test | RUN | 0.178114 | 1.053225 | 0.548699 | 0.001854 | 0.001890 | 0.001745 |
| mp_transform_second_2.test | RUN | 0.015631 | 0.031248 | 0.047568 | 0.000650 | 0.000851 | 0.001752 |
| mp_transform_third.test | RUN | 0.172023 | 0.792346 | 0.513421 | 0.001538 | 0.001623 | 0.001647 |
| mp_transform_third_2.test | RUN | 0.014859 | 0.031263 | 0.049459 | 0.000589 | 0.000793 | 0.001635 |
| mp_list_v.test | RUN | 0.016371 | 0.033168 | 0.045506 | 0.000625 | 0.000690 | 0.001762 |
| mp_rename_v.test | RUN | 0.015575 | 0.031682 | 0.047888 | 0.000611 | 0.000806 | 0.001526 |
| mp_is_value_list.test | RUN | 0.155061 | 0.336060 | 0.405543 | 0.001594 | 0.001529 | 0.001553 |
| mp_transform.test | RUN | 0.306696 | 2.581335 | 1.348656 | 0.001762 | 0.001768 | 0.001466 |
| mp_transform_2.test | RUN | 0.150727 | 0.310115 | 0.365355 | 0.001768 | 0.001780 | 0.001812 |
| mp_transform_q.test | RUN | 0.303053 | 2.469307 | 1.281442 | 0.001576 | 0.002088 | 0.001683 |
| mp_transform_q_2.test | RUN | 0.338683 | 0.801502 | 1.724674 | 0.001684 | 0.001794 | 0.001704 |
| mp_transform_sf.test | RUN | 0.169520 | 0.475238 | 0.445275 | 0.001649 | 0.002040 | 0.001778 |
| mp_transform_if.test | RUN | 0.196816 | 1.210180 | 0.738327 | 0.002016 | 0.001664 | 0.001583 |
| mp_transform_if_q.test | RUN | 0.204949 | 1.235792 | 0.715935 | 0.002023 | 0.002060 | 0.001694 |
| mp_filter.test | RUN | 0.197515 | 0.756345 | 0.560282 | 0.001662 | 0.001626 | 0.001366 |
| mp_fill.test | RUN | 0.184754 | 0.589408 | 0.480188 | 0.001705 | 0.001770 | 0.001713 |
| mp_fill_2.test | RUN | 0.028929 | 0.049085 | 0.074941 | 0.000818 | 0.000865 | 0.001810 |
| mp_count.test | RUN | 0.333771 | 1.322323 | 0.764406 | 0.001754 | 0.001805 | 0.001353 |
| mp_count_if.test | RUN | 0.332218 | 1.308847 | 0.796756 | 0.001608 | 0.001467 | 0.001713 |
| mp_count_if_q.test | RUN | 0.185525 | 0.857398 | 0.475629 | 0.001515 | 0.001763 | 0.001571 |
| mp_contains.test | RUN | 0.175492 | 0.503373 | 0.449913 | 0.001560 | 0.001744 | 0.001762 |
| mp_starts_with.test | RUN | 0.182436 | 0.537238 | 0.463427 | 0.001828 | 0.001785 | 0.001540 |
| mp_starts_with_sf.test | RUN | 0.179204 | 0.473670 | 0.439046 | 0.001715 | 0.001948 | 0.001373 |
| mp_repeat.test | RUN | 0.253859 | 2.173361 | 0.861773 | 0.001804 | 0.002183 | 0.001630 |
| mp_repeat_2.test | RUN | 0.028471 | 0.045675 | 0.080067 | 0.000664 | 0.000940 | 0.001673 |
| mp_repeat_3.test | RUN | 0.207778 | 0.312136 | 0.435937 | 0.001666 | 0.001972 | 0.001820 |
| mp_product.test | RUN | 0.185791 | 1.330539 | 0.653027 | 0.001486 | 0.001856 | 0.001475 |
| mp_drop.test | RUN | 0.197793 | 1.606405 | 0.720315 | 0.001779 | 0.001870 | 0.001624 |
| mp_drop_2.test | RUN | 0.027234 | 0.049902 | 0.083754 | 0.000588 | 0.000802 | 0.002150 |
| mp_iota.test | RUN | 0.194925 | 1.412018 | 0.631877 | 0.001746 | 0.001450 | 0.001684 |
| mp_iota_2.test | RUN | 0.207139 | 1.674199 | 0.738680 | 0.001283 | 0.001438 | 0.001652 |
| mp_at.test | RUN | 0.178144 | 0.747368 | 0.471623 | 0.001299 | 0.001627 | 0.001820 |
| mp_at_2.test | RUN | 0.026401 | 0.047856 | 0.072933 | 0.000661 | 0.000742 | 0.002025 |
| mp_at_sf.test | RUN | 0.170796 | 0.456853 | 0.446829 | 0.001816 | 0.001900 | 0.001590 |
| mp_take.test | RUN | 0.241188 | 2.127597 | 0.871865 | 0.001780 | 0.001603 | 0.001915 |
| mp_take_2.test | RUN | 0.026631 | 0.048080 | 0.073454 | 0.000770 | 0.000791 | 0.001737 |
| mp_slice.test | RUN | 0.279836 | 2.170103 | 1.133695 | 0.001657 | 0.001836 | 0.001504 |
| mp_slice_2.test | RUN | 0.028322 | 0.047220 | 0.076038 | 0.000774 | 0.000755 | 0.001857 |
| mp_replace.test | RUN | 0.201249 | 1.079534 | 0.696185 | 0.001988 | 0.001386 | 0.001788 |
| mp_replace_if.test | RUN | 0.195926 | 0.959904 | 0.681592 | 0.001770 | 0.001837 | 0.001523 |
| mp_replace_if_q.test | RUN | 0.198358 | 0.964423 | 0.648077 | 0.001479 | 0.002108 | 0.001781 |
| mp_copy_if.test | RUN | 0.200188 | 1.370481 | 0.619691 | 0.001876 | 0.001878 | 0.001875 |
| mp_copy_if_q.test | RUN | 0.204022 | 1.372981 | 0.642432 | 0.001860 | 0.001767 | 0.001790 |
| mp_remove.test | RUN | 0.191141 | 0.913983 | 0.652373 | 0.001970 | 0.002004 | 0.001549 |
| mp_remove_if.test | RUN | 0.202496 | 1.490425 | 0.659294 | 0.001600 | 0.002060 | 0.001797 |
| mp_remove_if_q.test | RUN | 0.210403 | 1.637392 | 0.657011 | 0.002000 | 0.001778 | 0.001918 |
| mp_partition.test | RUN | 0.214834 | 1.408500 | 0.723075 | 0.001871 | 0.002012 | 0.001844 |
| mp_partition_q.test | RUN | 0.202148 | 1.492801 | 0.734088 | 0.001884 | 0.001540 | 0.001435 |
| mp_sort.test | RUN | 0.197620 | 1.077371 | 0.620629 | 0.001576 | 0.004040 | 0.001842 |
| mp_sort_q.test | RUN | 0.193628 | 1.016863 | 0.607745 | 0.001572 | 0.001872 | 0.001974 |
| mp_find.test | RUN | 0.188662 | 1.088020 | 0.529648 | 0.001858 | 0.001667 | 0.001819 |
| mp_find_if.test | RUN | 0.183330 | 1.009085 | 0.505831 | 0.001593 | 0.001983 | 0.001607 |
| mp_find_if_q.test | RUN | 0.201555 | 1.061322 | 0.572699 | 0.001547 | 0.001922 | 0.001787 |
| mp_reverse.test | RUN | 0.326189 | 2.945088 | 1.251055 | 0.001807 | 0.001696 | 0.001782 |
| mp_fold.test | RUN | 0.562915 | 2.322665 | 3.267558 | 0.001732 | 0.001861 | 0.001949 |
| mp_fold_q.test | RUN | 0.184174 | 1.008497 | 0.551545 | 0.001888 | 0.001754 | 0.001776 |
| mp_fold_q_sf.test | RUN | 0.169095 | 0.505139 | 0.425142 | 0.001491 | 0.001876 | 0.001757 |
| mp_reverse_fold.test | RUN | 0.285451 | 2.176261 | 0.892062 | 0.001616 | 0.002074 | 0.001628 |
| mp_reverse_fold_q.test | RUN | 0.189657 | 1.206481 | 0.613808 | 0.001212 | 0.001961 | 0.001916 |
| mp_unique.test | RUN | 0.193842 | 1.112803 | 0.646768 | 0.001705 | 0.002038 | 0.001816 |
| mp_unique_if.test | RUN | 0.291483 | 2.357620 | 1.025112 | 0.001552 | 0.002022 | 0.001814 |
| mp_unique_if_q.test | RUN | 0.261948 | 2.366482 | 0.788298 | 0.001691 | 0.001721 | 0.001855 |
| mp_all_of.test | RUN | 0.425917 | 1.285820 | 1.012193 | 0.001946 | 0.001924 | 0.001690 |
| mp_all_of_q.test | RUN | 0.175925 | 0.559244 | 0.438080 | 0.001760 | 0.002060 | 0.001776 |
| mp_any_of.test | RUN | 0.420561 | 1.312471 | 1.014522 | 0.001635 | 0.001805 | 0.001696 |
| mp_any_of_q.test | RUN | 0.166715 | 0.551082 | 0.453779 | 0.001792 | 0.001948 | 0.001889 |
| mp_none_of.test | RUN | 0.436764 | 1.297698 | 1.001768 | 0.001455 | 0.002222 | 0.001918 |
| mp_none_of_q.test | RUN | 0.173319 | 0.541984 | 0.458313 | 0.001831 | 0.001617 | 0.001768 |
| mp_replace_at.test | RUN | 0.206102 | 1.057958 | 0.736907 | 0.001770 | 0.001835 | 0.003024 |
| mp_replace_at_c.test | RUN | 0.198598 | 1.063936 | 0.757064 | 0.001686 | 0.001776 | 0.001770 |
| mp_for_each.test | RUN | 0.195330 | 0.755907 | 0.823307 | 0.001508 | 0.001886 | 0.001560 |
| mp_insert.test | RUN | 0.261121 | 2.224795 | 1.122602 | 0.001565 | 0.002029 | 0.001768 |
| mp_insert_2.test | RUN | 0.027729 | 0.050830 | 0.076078 | 0.000569 | 0.000861 | 0.001587 |
| mp_erase.test | RUN | 0.285634 | 2.385777 | 1.215227 | 0.001726 | 0.001977 | 0.001726 |
| mp_erase_2.test | RUN | 0.029332 | 0.050820 | 0.074704 | 0.000774 | 0.000859 | 0.001796 |
| mp_with_index.test | RUN | 0.349578 | 4.938002 | 2.520284 | 0.002136 | 0.002503 | 0.002116 |
| mp_with_index_cx.test | RUN | 0.004066 | 0.017776 | 0.028163 | 0.000799 | 0.000847 | 0.001801 |
| mp_from_sequence.test | RUN | 0.200560 | 1.276115 | 0.644772 | 0.001459 | 0.002272 | 0.001856 |
| mp_from_sequence_2.test | RUN | 0.203116 | 1.434577 | 0.733856 | 0.001746 | 0.001914 | 0.001738 |
| mp_from_sequence_3.test | RUN | 0.174735 | 0.669050 | 0.525647 | 0.001550 | 0.001884 | 0.002001 |
| mp_min_element.test | RUN | 0.171797 | 0.586692 | 0.497476 | 0.001640 | 0.001871 | 0.001878 |
| mp_min_element_2.test | RUN | 0.026155 | 0.050366 | 0.073355 | 0.000830 | 0.000865 | 0.001758 |
| mp_min_element_q.test | RUN | 0.175593 | 0.627730 | 0.471617 | 0.001698 | 0.001888 | 0.001660 |
| mp_min_element_q_2.test | RUN | 0.027404 | 0.048467 | 0.079991 | 0.000565 | 0.000730 | 0.001670 |
| mp_max_element.test | RUN | 0.174386 | 0.605611 | 0.464987 | 0.001930 | 0.001838 | 0.001710 |
| mp_max_element_2.test | RUN | 0.029810 | 0.055105 | 0.076410 | 0.001129 | 0.000861 | 0.001656 |
| mp_max_element_q.test | RUN | 0.213355 | 0.640902 | 0.465974 | 0.001916 | 0.001910 | 0.001697 |
| mp_max_element_q_2.test | RUN | 0.043279 | 0.050302 | 0.063822 | 0.000607 | 0.000841 | 0.001668 |
| mp_nth_element.test | RUN | 0.193125 | 0.855637 | 0.574608 | 0.001686 | 0.001526 | 0.001848 |
| mp_nth_element_q.test | RUN | 0.196146 | 0.880092 | 0.569790 | 0.001585 | 0.001465 | 0.001864 |
| mp_back.test | RUN | 0.186216 | 0.699931 | 0.516092 | 0.001936 | 0.001969 | 0.001649 |
| mp_back_2.test | RUN | 0.027536 | 0.049882 | 0.077711 | 0.000680 | 0.000842 | 0.001720 |
| mp_pop_back.test | RUN | 0.429178 | 4.478005 | 1.187752 | 0.002225 | 0.001657 | 0.001502 |
| mp_pop_back_2.test | RUN | 0.026601 | 0.049267 | 0.074847 | 0.000797 | 0.000648 | 0.001903 |
| mp_flatten.test | RUN | 0.215574 | 1.587843 | 0.787923 | 0.001696 | 0.001893 | 0.001700 |
| mp_rotate_left.test | RUN | 0.204951 | 1.719102 | 0.758677 | 0.001550 | 0.002140 | 0.001392 |
| mp_rotate_right.test | RUN | 0.255078 | 1.715056 | 0.781906 | 0.001502 | 0.001700 | 0.001560 |
| mp_power_set.test | RUN | 0.203790 | 1.586681 | 0.764535 | 0.001654 | 0.001831 | 0.001543 |
| mp_partial_sum.test | RUN | 0.249266 | 2.158409 | 0.911504 | 0.001558 | 0.001908 | 0.001910 |
| mp_iterate.test | RUN | 0.193685 | 0.651407 | 0.642460 | 0.001713 | 0.001918 | 0.001614 |
| mp_pairwise_fold.test | RUN | 0.206894 | 1.476465 | 0.804275 | 0.001728 | 0.002042 | 0.001765 |
| mp_pairwise_fold_q.test | RUN | 0.172259 | 0.441623 | 0.431539 | 0.002122 | 0.001896 | 0.001799 |
| mp_sliding_fold.test | RUN | 0.333951 | 2.842832 | 1.498924 | 0.001786 | 0.001922 | 0.001868 |
| mp_sliding_fold_q.test | RUN | 0.317524 | 2.839779 | 1.470391 | 0.001576 | 0.001804 | 0.001458 |
| mp_intersperse.test | RUN | 0.201061 | 1.047477 | 0.683589 | 0.001847 | 0.001927 | 0.002202 |
| mp_split.test | RUN | 0.255610 | 2.037400 | 0.997625 | 0.001972 | 0.001836 | 0.001589 |
| mp_join.test | RUN | 0.243630 | 1.704901 | 0.863267 | 0.001775 | 0.001767 | 0.001756 |
| integral.test | RUN | 0.174122 | 0.740183 | 0.483634 | 0.001841 | 0.001792 | 0.001794 |
| mp_value.test | RUN | 0.013446 | 0.028330 | 0.047632 | 0.000561 | 0.000781 | 0.001834 |
| mp_identity.test | RUN | 0.163810 | 0.836881 | 0.483033 | 0.001920 | 0.001970 | 0.001802 |
| mp_inherit.test | RUN | 0.160096 | 0.650607 | 0.451123 | 0.001530 | 0.001578 | 0.001794 |
| mp_if.test | RUN | 0.161264 | 0.716271 | 0.450225 | 0.001550 | 0.001553 | 0.002005 |
| mp_if_sf.test | RUN | 0.154051 | 0.518305 | 0.394498 | 0.002205 | 0.002505 | 0.001758 |
| mp_eval_if.test | RUN | 0.167588 | 0.755000 | 0.462977 | 0.001448 | 0.001822 | 0.001718 |
| mp_eval_if_sf.test | RUN | 0.165472 | 0.500963 | 0.408161 | 0.001552 | 0.002116 | 0.001582 |
| mp_valid.test | RUN | 0.165121 | 0.489549 | 0.408087 | 0.001788 | 0.002078 | 0.001742 |
| mp_defer.test | RUN | 0.171563 | 0.801222 | 0.508248 | 0.001595 | 0.002126 | 0.001822 |
| mp_quote.test | RUN | 0.174291 | 0.759724 | 0.499777 | 0.001628 | 0.001830 | 0.001829 |
| mp_invoke_q.test | RUN | 0.185934 | 0.969682 | 0.568480 | 0.001545 | 0.001898 | 0.001910 |
| mp_invoke_q_sf.test | RUN | 0.164857 | 0.439528 | 0.412809 | 0.001576 | 0.001832 | 0.001888 |
| mp_quote_trait.test | RUN | 0.175915 | 0.856135 | 0.515114 | 0.001548 | 0.001842 | 0.001848 |
| mp_cond.test | RUN | 0.167349 | 0.530188 | 0.450510 | 0.001950 | 0.001596 | 0.001523 |
| mp_cond_sf.test | RUN | 0.157082 | 0.440648 | 0.394850 | 0.001565 | 0.001810 | 0.001797 |
| mp_not_fn.test | RUN | 0.155957 | 0.449122 | 0.396478 | 0.001900 | 0.001922 | 0.001690 |
| mp_eval_if_not.test | RUN | 0.167877 | 0.629718 | 0.458232 | 0.001862 | 0.001960 | 0.001616 |
| mp_eval_or.test | RUN | 0.158369 | 0.352771 | 0.414795 | 0.001642 | 0.002106 | 0.001385 |
| mp_compose.test | RUN | 0.174491 | 1.042665 | 0.553394 | 0.001623 | 0.001900 | 0.001733 |
| mp_valid_and_true.test | RUN | 0.153943 | 0.570505 | 0.431591 | 0.001553 | 0.001865 | 0.001437 |
| mp_compose_sf.test | COMPILE | 0.039740 | 0.064103 | 0.102867 | — | — | — |
| integer_sequence.test | RUN | 0.183966 | 1.058382 | 0.612119 | 0.001941 | 0.001912 | 0.001624 |
| tuple_for_each.test | RUN | 0.171469 | 0.416163 | 0.437044 | 0.001252 | 0.001992 | 0.001604 |
| tuple_for_each_cx.test | COMPILE | 0.032776 | 0.063355 | 0.081602 | — | — | — |
| tuple_apply.test | RUN | 0.170622 | 0.397389 | 0.446449 | 0.001778 | 0.002002 | 0.001861 |
| tuple_apply_cx.test | COMPILE | 0.046145 | 0.081119 | 0.115612 | — | — | — |
| construct_from_tuple.test | RUN | 0.168840 | 0.433256 | 0.422114 | 0.001714 | 0.001879 | 0.001497 |
| construct_from_tuple_cx.test | COMPILE | 0.044748 | 0.082483 | 0.107788 | — | — | — |
| tuple_transform.test | RUN | 0.258407 | 0.862795 | 0.902673 | 0.002140 | 0.001536 | 0.001623 |
| tuple_transform_2.test | RUN | 0.227399 | 0.649606 | 0.782124 | 0.002092 | 0.001882 | 0.001496 |
| tuple_transform_cx.test | COMPILE | 0.038808 | 0.074856 | 0.110485 | — | — | — |
| mp_set_contains.test | RUN | 0.169531 | 0.538274 | 0.440374 | 0.001956 | 0.002059 | 0.001890 |
| mp_set_push_back.test | RUN | 0.213237 | 1.685243 | 0.783224 | 0.001938 | 0.002058 | 0.001889 |
| mp_set_push_front.test | RUN | 0.249832 | 1.993011 | 0.922779 | 0.001760 | 0.001584 | 0.001522 |
| mp_is_set.test | RUN | 0.171940 | 0.544397 | 0.447807 | 0.001835 | 0.002204 | 0.001744 |
| mp_set_union.test | RUN | 0.204532 | 1.444447 | 0.768585 | 0.001635 | 0.001857 | 0.001312 |
| mp_set_union_sf.test | RUN | 0.165585 | 0.446445 | 0.417034 | 0.001852 | 0.001875 | 0.001719 |
| mp_set_difference.test | RUN | 0.182466 | 1.095625 | 0.620022 | 0.001776 | 0.002111 | 0.001779 |
| mp_set_difference_sf.test | RUN | 0.157704 | 0.454084 | 0.424178 | 0.001456 | 0.001696 | 0.001452 |
| mp_set_intersection.test | RUN | 0.185424 | 1.081550 | 0.605195 | 0.001762 | 0.001903 | 0.001722 |
| mp_set_intersection_sf.test | RUN | 0.165509 | 0.416935 | 0.415483 | 0.001955 | 0.001990 | 0.001488 |
| mp_all.test | RUN | 0.167326 | 0.512406 | 0.445740 | 0.001793 | 0.002068 | 0.001673 |
| mp_all_2.test | RUN | 0.425840 | 1.186048 | 1.041167 | 0.001989 | 0.001844 | 0.001924 |
| mp_and.test | RUN | 0.159564 | 0.541066 | 0.423375 | 0.002090 | 0.001798 | 0.001742 |
| mp_any.test | RUN | 0.166069 | 0.554580 | 0.441043 | 0.001975 | 0.001662 | 0.001844 |
| mp_any_2.test | RUN | 0.471808 | 1.165383 | 1.024107 | 0.001689 | 0.001858 | 0.001346 |
| mp_or.test | RUN | 0.177355 | 0.530510 | 0.440632 | 0.001615 | 0.001886 | 0.001646 |
| mp_same.test | RUN | 0.316034 | 0.895799 | 0.695313 | 0.001802 | 0.001883 | 0.001763 |
| mp_plus.test | RUN | 0.173937 | 0.634724 | 0.493689 | 0.001560 | 0.001642 | 0.001597 |
| mp_less.test | RUN | 0.172291 | 0.533603 | 0.425084 | 0.001707 | 0.002308 | 0.001772 |
| mp_min.test | RUN | 0.173327 | 0.493364 | 0.446998 | 0.001782 | 0.001842 | 0.001648 |
| mp_max.test | RUN | 0.175195 | 0.612155 | 0.455851 | 0.001523 | 0.002114 | 0.001299 |
| mp_similar.test | RUN | 0.167387 | 0.597379 | 0.440505 | 0.001962 | 0.001764 | 0.001797 |
| mp_map_find.test | RUN | 0.187880 | 1.040491 | 0.634119 | 0.001850 | 0.002094 | 0.001748 |
| mp_map_find_2.test | RUN | 0.176326 | 0.452298 | 0.508171 | 0.001656 | 0.001897 | 0.001690 |
| mp_map_find_3.test | RUN | 0.172449 | 0.371829 | 0.489936 | 0.001600 | 0.001537 | 0.001486 |
| mp_map_find_4.test | RUN | 0.174723 | 0.365592 | 0.448725 | 0.001768 | 0.001653 | 0.001794 |
| mp_map_contains.test | RUN | 0.169586 | 0.557026 | 0.439522 | 0.001850 | 0.002008 | 0.002018 |
| mp_map_insert.test | RUN | 0.236293 | 1.863884 | 0.880924 | 0.001833 | 0.002070 | 0.001848 |
| mp_map_replace.test | RUN | 0.262905 | 2.150593 | 1.013474 | 0.001748 | 0.001878 | 0.001795 |
| mp_map_erase.test | RUN | 0.212362 | 1.582222 | 0.814329 | 0.001551 | 0.001902 | 0.001722 |
| mp_map_update.test | RUN | 0.188219 | 1.033521 | 0.630104 | 0.001796 | 0.002022 | 0.001758 |
| mp_map_update_q.test | RUN | 0.191036 | 1.043406 | 0.629263 | 0.001380 | 0.001751 | 0.001556 |
| mp_map_keys.test | RUN | 0.182566 | 0.896152 | 0.610185 | 0.001776 | 0.001821 | 0.001443 |
| mp_is_map.test | RUN | 0.180265 | 0.542313 | 0.481618 | 0.001831 | 0.001904 | 0.001675 |
| mp_bind.test | RUN | 0.210756 | 1.656034 | 0.716810 | 0.002128 | 0.002258 | 0.001862 |
| mp_bind_q.test | RUN | 0.188938 | 1.042398 | 0.581822 | 0.001742 | 0.002062 | 0.001632 |
| mp_bind_front.test | RUN | 0.184190 | 1.268175 | 0.580474 | 0.001858 | 0.001997 | 0.001778 |
| mp_bind_back.test | RUN | 0.188380 | 1.272233 | 0.601667 | 0.001655 | 0.002050 | 0.001429 |
| mp_lambda.test | RUN | 0.344789 | 3.603688 | 1.560656 | 0.001894 | 0.001492 | 0.001760 |
| mpl.test | RUN | 0.347902 | 0.899315 | 1.220192 | 0.001522 | 0.001516 | 0.001478 |
| mpl_list.test | RUN | 0.214292 | 0.689786 | 0.549402 | 0.001549 | 0.001756 | 0.001816 |
| mpl_tuple.test | RUN | 0.221348 | 0.682222 | 0.551201 | 0.001905 | 0.002024 | 0.001449 |
| version.test | RUN | 0.108657 | 0.277638 | 0.287948 | 0.001788 | 0.001932 | 0.001609 |
| mp11_version.test | RUN | 0.138193 | 0.315609 | 0.333935 | 0.001631 | 0.001586 | 0.001483 |
| list_version.test | RUN | 0.113699 | 0.269728 | 0.290390 | 0.001600 | 0.001937 | 0.003813 |
| algorithm_version.test | RUN | 0.123066 | 0.272283 | 0.311829 | 0.001600 | 0.001926 | 0.001928 |
| integral_version.test | RUN | 0.113230 | 0.240778 | 0.296660 | 0.001823 | 0.002636 | 0.001497 |
| utility_version.test | RUN | 0.108603 | 0.251636 | 0.344788 | 0.001790 | 0.001942 | 0.001852 |
| function_version.test | RUN | 0.112844 | 0.270975 | 0.296133 | 0.001615 | 0.003164 | 0.001728 |
| map_version.test | RUN | 0.125815 | 0.279002 | 0.331063 | 0.001568 | 0.001714 | 0.001581 |
| set_version.test | RUN | 0.120281 | 0.270454 | 0.303484 | 0.001865 | 0.001960 | 0.001509 |
| bind_version.test | RUN | 0.125981 | 0.271709 | 0.296154 | 0.001866 | 0.001894 | 0.002021 |
| integer_sequence_version.test | RUN | 0.108154 | 0.264125 | 0.300203 | 0.001833 | 0.001614 | 0.001706 |
| tuple_version.test | RUN | 0.125094 | 0.268814 | 0.269325 | 0.001474 | 0.001904 | 0.001352 |
| mpl_version.test | RUN | 0.103069 | 0.200849 | 0.253102 | 0.001437 | 0.001441 | 0.001264 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.015×† | 1.025×, 1.004× | 1.051× |
| clang | 1.080× | 1.084×, 1.075× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000943s | 0.000949s | 0.000957s | byte equality observed |
| bind_version.test | 10 | 0.000938s | 0.000995s | 0.000962s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000960s | 0.001004s | 0.000949s | byte equality observed |
| function_version.test | 10 | 0.000928s | 0.000973s | 0.000954s | byte equality observed |
| integer_sequence.test | 10 | 0.000901s | 0.000983s | 0.000971s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000898s | 0.000948s | 0.000948s | byte equality observed |
| integral.test | 10 | 0.000926s | 0.000971s | 0.000965s | byte equality observed |
| integral_version.test | 10 | 0.000922s | 0.000985s | 0.000997s | byte equality observed |
| list_version.test | 10 | 0.000933s | 0.000993s | 0.000958s | byte equality observed |
| map_version.test | 10 | 0.000960s | 0.000960s | 0.000982s | byte equality observed |
| mp11_version.test | 10 | 0.000971s | 0.000960s | 0.000963s | byte equality observed |
| mp_all.test | 10 | 0.000972s | 0.000950s | 0.000950s | byte equality observed |
| mp_all_2.test | 10 | 0.000985s | 0.000943s | 0.000991s | byte equality observed |
| mp_all_of.test | 10 | 0.000984s | 0.000954s | 0.000966s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000953s | 0.001010s | 0.000960s | byte equality observed |
| mp_and.test | 10 | 0.000949s | 0.000964s | 0.000959s | byte equality observed |
| mp_any.test | 10 | 0.000936s | 0.000951s | 0.000957s | byte equality observed |
| mp_any_2.test | 10 | 0.000957s | 0.000998s | 0.000947s | byte equality observed |
| mp_any_of.test | 10 | 0.000971s | 0.000954s | 0.000967s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000930s | 0.000964s | 0.000963s | byte equality observed |
| mp_append.test | 10 | 0.000942s | 0.000968s | 0.000968s | byte equality observed |
| mp_append_2.test | 10 | 0.000993s | 0.000987s | 0.000950s | byte equality observed |
| mp_append_3.test | 10 | 0.000442s | 0.000449s | 0.000960s | byte equality observed |
| mp_append_sf.test | 10 | 0.000932s | 0.000990s | 0.000958s | byte equality observed |
| mp_apply_q.test | 10 | 0.000937s | 0.000958s | 0.000951s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000957s | 0.000930s | 0.000984s | byte equality observed |
| mp_assign.test | 10 | 0.000940s | 0.000949s | 0.000961s | byte equality observed |
| mp_assign_2.test | 10 | 0.000442s | 0.000451s | 0.000940s | byte equality observed |
| mp_at.test | 10 | 0.000955s | 0.000955s | 0.000954s | byte equality observed |
| mp_at_2.test | 10 | 0.000443s | 0.000446s | 0.000922s | byte equality observed |
| mp_at_sf.test | 10 | 0.000946s | 0.000972s | 0.000942s | byte equality observed |
| mp_back.test | 10 | 0.000950s | 0.000966s | 0.000957s | byte equality observed |
| mp_back_2.test | 10 | 0.000442s | 0.000438s | 0.000933s | byte equality observed |
| mp_bind.test | 10 | 0.000960s | 0.000944s | 0.000953s | byte equality observed |
| mp_bind_back.test | 10 | 0.000943s | 0.000955s | 0.000971s | byte equality observed |
| mp_bind_front.test | 10 | 0.000946s | 0.000964s | 0.000956s | byte equality observed |
| mp_bind_q.test | 10 | 0.000929s | 0.000949s | 0.000940s | byte equality observed |
| mp_clear.test | 10 | 0.000978s | 0.000953s | 0.000961s | byte equality observed |
| mp_clear_2.test | 10 | 0.000458s | 0.000430s | 0.000926s | byte equality observed |
| mp_compose.test | 10 | 0.000978s | 0.000938s | 0.000931s | byte equality observed |
| mp_cond.test | 10 | 0.000970s | 0.000947s | 0.000946s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000954s | 0.000946s | 0.000957s | byte equality observed |
| mp_contains.test | 10 | 0.000966s | 0.000982s | 0.000979s | byte equality observed |
| mp_copy_if.test | 10 | 0.000964s | 0.000934s | 0.000966s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000945s | 0.000988s | 0.000972s | byte equality observed |
| mp_count.test | 10 | 0.000951s | 0.000952s | 0.000975s | byte equality observed |
| mp_count_if.test | 10 | 0.000938s | 0.000941s | 0.000953s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000950s | 0.000988s | 0.000951s | byte equality observed |
| mp_defer.test | 10 | 0.000988s | 0.000954s | 0.001018s | byte equality observed |
| mp_drop.test | 10 | 0.000987s | 0.000976s | 0.000965s | byte equality observed |
| mp_drop_2.test | 10 | 0.000451s | 0.000448s | 0.000954s | byte equality observed |
| mp_empty.test | 10 | 0.000978s | 0.000968s | 0.000975s | byte equality observed |
| mp_empty_2.test | 10 | 0.000447s | 0.000441s | 0.000946s | byte equality observed |
| mp_erase.test | 10 | 0.000994s | 0.000991s | 0.000956s | byte equality observed |
| mp_erase_2.test | 10 | 0.000446s | 0.000450s | 0.000937s | byte equality observed |
| mp_eval_if.test | 10 | 0.000952s | 0.000942s | 0.000972s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000951s | 0.000967s | 0.001013s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000948s | 0.001005s | 0.000992s | byte equality observed |
| mp_eval_or.test | 10 | 0.000947s | 0.000977s | 0.000946s | byte equality observed |
| mp_fill.test | 10 | 0.000981s | 0.000968s | 0.000979s | byte equality observed |
| mp_fill_2.test | 10 | 0.000443s | 0.000441s | 0.000950s | byte equality observed |
| mp_filter.test | 10 | 0.000958s | 0.001037s | 0.000951s | byte equality observed |
| mp_find.test | 10 | 0.000953s | 0.001020s | 0.000950s | byte equality observed |
| mp_find_if.test | 10 | 0.000978s | 0.000956s | 0.000961s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000972s | 0.000972s | 0.000963s | byte equality observed |
| mp_flatten.test | 10 | 0.000968s | 0.000933s | 0.000949s | byte equality observed |
| mp_fold.test | 10 | 0.000996s | 0.000986s | 0.001002s | byte equality observed |
| mp_fold_q.test | 10 | 0.000969s | 0.000996s | 0.000956s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000955s | 0.001011s | 0.000945s | byte equality observed |
| mp_for_each.test | 10 | 0.000971s | 0.000984s | 0.000995s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000981s | 0.001011s | 0.000970s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000948s | 0.000997s | 0.000962s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000952s | 0.000977s | 0.001001s | byte equality observed |
| mp_front.test | 10 | 0.000966s | 0.000972s | 0.000952s | byte equality observed |
| mp_front_2.test | 10 | 0.000451s | 0.000459s | 0.000959s | byte equality observed |
| mp_identity.test | 10 | 0.000950s | 0.000963s | 0.000975s | byte equality observed |
| mp_if.test | 10 | 0.000968s | 0.000962s | 0.000986s | byte equality observed |
| mp_if_sf.test | 10 | 0.000960s | 0.000937s | 0.000997s | byte equality observed |
| mp_inherit.test | 10 | 0.000954s | 0.000951s | 0.000960s | byte equality observed |
| mp_insert.test | 10 | 0.000995s | 0.000978s | 0.000954s | byte equality observed |
| mp_insert_2.test | 10 | 0.000444s | 0.000451s | 0.000957s | byte equality observed |
| mp_intersperse.test | 10 | 0.000955s | 0.000979s | 0.000968s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000952s | 0.000937s | 0.000968s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000979s | 0.001000s | 0.000967s | byte equality observed |
| mp_iota.test | 10 | 0.000952s | 0.000975s | 0.000963s | byte equality observed |
| mp_iota_2.test | 10 | 0.000974s | 0.000965s | 0.000986s | byte equality observed |
| mp_is_list.test | 10 | 0.000980s | 0.000954s | 0.000956s | byte equality observed |
| mp_is_map.test | 10 | 0.000945s | 0.000959s | 0.000951s | byte equality observed |
| mp_is_set.test | 10 | 0.000947s | 0.000982s | 0.000991s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000957s | 0.000958s | 0.000974s | byte equality observed |
| mp_iterate.test | 10 | 0.000933s | 0.000993s | 0.000956s | byte equality observed |
| mp_join.test | 10 | 0.000972s | 0.000967s | 0.000979s | byte equality observed |
| mp_lambda.test | 10 | 0.000969s | 0.000959s | 0.000973s | byte equality observed |
| mp_less.test | 10 | 0.000979s | 0.000952s | 0.000978s | byte equality observed |
| mp_list_c.test | 10 | 0.000955s | 0.000949s | 0.001009s | byte equality observed |
| mp_list_v.test | 10 | 0.000456s | 0.000451s | 0.000957s | byte equality observed |
| mp_map_contains.test | 10 | 0.000976s | 0.000969s | 0.000967s | byte equality observed |
| mp_map_erase.test | 10 | 0.000958s | 0.000940s | 0.000988s | byte equality observed |
| mp_map_find.test | 10 | 0.000961s | 0.000943s | 0.000948s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000948s | 0.000932s | 0.001022s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000955s | 0.000944s | 0.000996s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000938s | 0.000949s | 0.000992s | byte equality observed |
| mp_map_insert.test | 10 | 0.000971s | 0.000959s | 0.000978s | byte equality observed |
| mp_map_keys.test | 10 | 0.000964s | 0.000938s | 0.001002s | byte equality observed |
| mp_map_replace.test | 10 | 0.000995s | 0.000973s | 0.001010s | byte equality observed |
| mp_map_update.test | 10 | 0.000977s | 0.000966s | 0.000959s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000958s | 0.000953s | 0.000955s | byte equality observed |
| mp_max.test | 10 | 0.000939s | 0.000982s | 0.000953s | byte equality observed |
| mp_max_element.test | 10 | 0.000971s | 0.000996s | 0.000957s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000444s | 0.000454s | 0.000941s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000974s | 0.000982s | 0.000946s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000445s | 0.000458s | 0.000980s | byte equality observed |
| mp_min.test | 10 | 0.000962s | 0.000970s | 0.000967s | byte equality observed |
| mp_min_element.test | 10 | 0.000953s | 0.000963s | 0.000977s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000448s | 0.000446s | 0.000950s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000965s | 0.001011s | 0.000962s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000444s | 0.000435s | 0.000929s | byte equality observed |
| mp_none_of.test | 10 | 0.000963s | 0.000985s | 0.000961s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000947s | 0.000990s | 0.000962s | byte equality observed |
| mp_not_fn.test | 10 | 0.000957s | 0.000979s | 0.000965s | byte equality observed |
| mp_nth_element.test | 10 | 0.000986s | 0.000973s | 0.000962s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000958s | 0.000976s | 0.000973s | byte equality observed |
| mp_or.test | 10 | 0.000957s | 0.000986s | 0.000951s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000951s | 0.001002s | 0.000950s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000945s | 0.000971s | 0.000950s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000990s | 0.000962s | 0.000962s | byte equality observed |
| mp_partition.test | 10 | 0.000960s | 0.000982s | 0.000971s | byte equality observed |
| mp_partition_q.test | 10 | 0.000938s | 0.000959s | 0.000956s | byte equality observed |
| mp_plus.test | 10 | 0.000949s | 0.000973s | 0.000966s | byte equality observed |
| mp_pop_back.test | 10 | 0.000976s | 0.000958s | 0.000941s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000446s | 0.000443s | 0.000956s | byte equality observed |
| mp_pop_front.test | 10 | 0.000964s | 0.000946s | 0.000960s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000438s | 0.000444s | 0.000949s | byte equality observed |
| mp_power_set.test | 10 | 0.000970s | 0.000974s | 0.000958s | byte equality observed |
| mp_product.test | 10 | 0.000966s | 0.000941s | 0.000951s | byte equality observed |
| mp_push_back.test | 10 | 0.000984s | 0.000980s | 0.000944s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000453s | 0.000447s | 0.000944s | byte equality observed |
| mp_push_front.test | 10 | 0.000971s | 0.000961s | 0.000961s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000440s | 0.000435s | 0.000925s | byte equality observed |
| mp_quote.test | 10 | 0.000954s | 0.000947s | 0.000965s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000965s | 0.000983s | 0.000961s | byte equality observed |
| mp_remove.test | 10 | 0.000971s | 0.000943s | 0.000972s | byte equality observed |
| mp_remove_if.test | 10 | 0.000960s | 0.000970s | 0.000974s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000964s | 0.000956s | 0.000966s | byte equality observed |
| mp_rename.test | 10 | 0.000961s | 0.000981s | 0.000973s | byte equality observed |
| mp_rename_2.test | 10 | 0.000444s | 0.000443s | 0.000956s | byte equality observed |
| mp_rename_v.test | 10 | 0.000452s | 0.000445s | 0.000943s | byte equality observed |
| mp_repeat.test | 10 | 0.001015s | 0.000988s | 0.000953s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000439s | 0.000439s | 0.000945s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000964s | 0.000947s | 0.000955s | byte equality observed |
| mp_replace.test | 10 | 0.000959s | 0.000955s | 0.000973s | byte equality observed |
| mp_replace_at.test | 10 | 0.000971s | 0.000949s | 0.000963s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000958s | 0.000966s | 0.000978s | byte equality observed |
| mp_replace_front.test | 10 | 0.000989s | 0.000976s | 0.000978s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000446s | 0.000434s | 0.000933s | byte equality observed |
| mp_replace_if.test | 10 | 0.000971s | 0.000973s | 0.000934s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000988s | 0.000949s | 0.000944s | byte equality observed |
| mp_replace_second.test | 10 | 0.000980s | 0.000947s | 0.000962s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000453s | 0.000433s | 0.000944s | byte equality observed |
| mp_replace_third.test | 10 | 0.000975s | 0.000949s | 0.000962s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000446s | 0.000430s | 0.000939s | byte equality observed |
| mp_reverse.test | 10 | 0.000998s | 0.000947s | 0.000985s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000968s | 0.000972s | 0.000974s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000956s | 0.000960s | 0.000990s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000971s | 0.000989s | 0.000991s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000941s | 0.000977s | 0.000980s | byte equality observed |
| mp_same.test | 10 | 0.000952s | 0.000961s | 0.000998s | byte equality observed |
| mp_second.test | 10 | 0.000971s | 0.001015s | 0.000981s | byte equality observed |
| mp_second_2.test | 10 | 0.000445s | 0.000458s | 0.000957s | byte equality observed |
| mp_set_contains.test | 10 | 0.000953s | 0.001009s | 0.000983s | byte equality observed |
| mp_set_difference.test | 10 | 0.000942s | 0.000990s | 0.000972s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000945s | 0.000971s | 0.001009s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000958s | 0.000977s | 0.001011s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000942s | 0.000973s | 0.000968s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000963s | 0.000954s | 0.000967s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000973s | 0.000959s | 0.000977s | byte equality observed |
| mp_set_union.test | 10 | 0.000962s | 0.000953s | 0.000985s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000955s | 0.000982s | 0.000985s | byte equality observed |
| mp_similar.test | 10 | 0.000971s | 0.000964s | 0.000987s | byte equality observed |
| mp_size.test | 10 | 0.000991s | 0.000967s | 0.000973s | byte equality observed |
| mp_size_2.test | 10 | 0.000442s | 0.000445s | 0.000989s | byte equality observed |
| mp_slice.test | 10 | 0.000985s | 0.000997s | 0.000992s | byte equality observed |
| mp_slice_2.test | 10 | 0.000443s | 0.000446s | 0.000955s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000969s | 0.001013s | 0.000957s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000971s | 0.000993s | 0.000962s | byte equality observed |
| mp_sort.test | 10 | 0.000960s | 0.000972s | 0.000988s | byte equality observed |
| mp_sort_q.test | 10 | 0.000959s | 0.000985s | 0.000990s | byte equality observed |
| mp_split.test | 10 | 0.000971s | 0.000974s | 0.000962s | byte equality observed |
| mp_starts_with.test | 10 | 0.000960s | 0.000946s | 0.000948s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000968s | 0.000964s | 0.000960s | byte equality observed |
| mp_take.test | 10 | 0.000977s | 0.000976s | 0.000959s | byte equality observed |
| mp_take_2.test | 10 | 0.000440s | 0.000444s | 0.000955s | byte equality observed |
| mp_third.test | 10 | 0.000970s | 0.000974s | 0.000986s | byte equality observed |
| mp_third_2.test | 10 | 0.000448s | 0.000442s | 0.000939s | byte equality observed |
| mp_transform.test | 10 | 0.000973s | 0.000962s | 0.000971s | byte equality observed |
| mp_transform_2.test | 10 | 0.000942s | 0.000919s | 0.000938s | byte equality observed |
| mp_transform_front.test | 10 | 0.000983s | 0.000982s | 0.000982s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000449s | 0.000446s | 0.000938s | byte equality observed |
| mp_transform_if.test | 10 | 0.000950s | 0.000971s | 0.000979s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000949s | 0.000939s | 0.001001s | byte equality observed |
| mp_transform_q.test | 10 | 0.000971s | 0.000963s | 0.000980s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000942s | 0.000941s | 0.000977s | byte equality observed |
| mp_transform_second.test | 10 | 0.000949s | 0.000967s | 0.001000s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000441s | 0.000442s | 0.000950s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000966s | 0.000975s | 0.000985s | byte equality observed |
| mp_transform_third.test | 10 | 0.000941s | 0.000972s | 0.000970s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000442s | 0.000454s | 0.000957s | byte equality observed |
| mp_unique.test | 10 | 0.000951s | 0.000976s | 0.000984s | byte equality observed |
| mp_unique_if.test | 10 | 0.000966s | 0.000996s | 0.000957s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000969s | 0.000986s | 0.000981s | byte equality observed |
| mp_valid.test | 10 | 0.000950s | 0.000963s | 0.000955s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000970s | 0.000979s | 0.000989s | byte equality observed |
| mp_value.test | 10 | 0.000446s | 0.000449s | 0.000913s | byte equality observed |
| mp_with_index.test | 10 | 0.001355s | 0.001393s | 0.001295s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000445s | 0.000444s | 0.000954s | byte equality observed |
| mpl.test | 10 | 0.000983s | 0.000943s | 0.001007s | byte equality observed |
| mpl_list.test | 10 | 0.000972s | 0.000965s | 0.000966s | byte equality observed |
| mpl_tuple.test | 10 | 0.000952s | 0.000977s | 0.000955s | byte equality observed |
| mpl_version.test | 10 | 0.000956s | 0.000958s | 0.000980s | byte equality observed |
| set_version.test | 10 | 0.000951s | 0.000966s | 0.000956s | byte equality observed |
| tuple_apply.test | 10 | 0.000955s | 0.000978s | 0.000942s | byte equality observed |
| tuple_for_each.test | 10 | 0.000987s | 0.000973s | 0.000979s | byte equality observed |
| tuple_transform.test | 10 | 0.000954s | 0.000975s | 0.000943s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000949s | 0.000953s | 0.000962s | byte equality observed |
| tuple_version.test | 10 | 0.000968s | 0.000941s | 0.000973s | byte equality observed |
| utility_version.test | 10 | 0.000939s | 0.000948s | 0.000962s | byte equality observed |
| version.test | 10 | 0.000952s | 0.000927s | 0.000972s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
