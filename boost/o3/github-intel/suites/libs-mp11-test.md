# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.048768 | 0.083981 | 0.120994 | — | — | — |
| complex_h.test | COMPILE | 0.195678 | 0.358788 | 0.512621 | — | — | — |
| mp_size.test | RUN | 0.240109 | 1.173500 | 0.655784 | 0.001519 | 0.001670 | 0.001810 |
| mp_size_2.test | RUN | 0.017760 | 0.041203 | 0.054879 | 0.000704 | 0.000794 | 0.001678 |
| mp_empty.test | RUN | 0.223680 | 0.735115 | 0.629697 | 0.001688 | 0.001756 | 0.001749 |
| mp_empty_2.test | RUN | 0.020023 | 0.043961 | 0.059580 | 0.000802 | 0.000699 | 0.001562 |
| mp_assign.test | RUN | 0.252261 | 1.545122 | 0.836992 | 0.001736 | 0.001827 | 0.001642 |
| mp_assign_2.test | RUN | 0.020951 | 0.042146 | 0.055410 | 0.000793 | 0.000794 | 0.001660 |
| mp_clear.test | RUN | 0.221430 | 0.668853 | 0.610633 | 0.001697 | 0.001609 | 0.001752 |
| mp_clear_2.test | RUN | 0.020809 | 0.041654 | 0.059720 | 0.000828 | 0.000565 | 0.001703 |
| mp_front.test | RUN | 0.230523 | 1.051741 | 0.716451 | 0.001695 | 0.001788 | 0.001762 |
| mp_front_2.test | RUN | 0.020355 | 0.040389 | 0.059577 | 0.000661 | 0.000641 | 0.001312 |
| mp_pop_front.test | RUN | 0.234232 | 1.127261 | 0.723392 | 0.001750 | 0.001638 | 0.001704 |
| mp_pop_front_2.test | RUN | 0.021004 | 0.041872 | 0.061940 | 0.000608 | 0.000808 | 0.001686 |
| mp_second.test | RUN | 0.227961 | 0.693924 | 0.665043 | 0.001724 | 0.001722 | 0.001487 |
| mp_second_2.test | RUN | 0.020303 | 0.041803 | 0.059053 | 0.000829 | 0.000789 | 0.001706 |
| mp_third.test | RUN | 0.220305 | 0.645931 | 0.632652 | 0.001700 | 0.001771 | 0.001807 |
| mp_third_2.test | RUN | 0.020710 | 0.042768 | 0.058641 | 0.000786 | 0.000609 | 0.001492 |
| mp_push_front.test | RUN | 0.343532 | 2.370188 | 1.201705 | 0.001716 | 0.001653 | 0.001804 |
| mp_push_front_2.test | RUN | 0.020996 | 0.038364 | 0.059038 | 0.000776 | 0.000735 | 0.001484 |
| mp_push_back.test | RUN | 0.341204 | 2.361680 | 1.204449 | 0.001611 | 0.001574 | 0.001632 |
| mp_push_back_2.test | RUN | 0.020204 | 0.043293 | 0.058533 | 0.000795 | 0.000799 | 0.001699 |
| mp_rename.test | RUN | 0.265573 | 2.178582 | 0.995604 | 0.001567 | 0.001629 | 0.001755 |
| mp_rename_2.test | RUN | 0.020337 | 0.042677 | 0.060125 | 0.000650 | 0.000804 | 0.001686 |
| mp_append.test | RUN | 0.280036 | 2.479006 | 1.112517 | 0.001663 | 0.001811 | 0.001446 |
| mp_append_2.test | RUN | 0.563121 | 5.538587 | 1.478646 | 0.001549 | 0.001766 | 0.001718 |
| mp_append_3.test | RUN | 0.019920 | 0.043023 | 0.060909 | 0.000781 | 0.000791 | 0.001700 |
| mp_append_sf.test | RUN | 0.211696 | 0.604406 | 0.603318 | 0.001747 | 0.001500 | 0.001791 |
| mp_replace_front.test | RUN | 0.254478 | 1.734813 | 0.917532 | 0.001708 | 0.001755 | 0.001635 |
| mp_replace_front_2.test | RUN | 0.020626 | 0.042228 | 0.058194 | 0.000676 | 0.000755 | 0.001733 |
| mp_replace_second.test | RUN | 0.253108 | 1.118747 | 0.813192 | 0.001784 | 0.001592 | 0.001773 |
| mp_replace_second_2.test | RUN | 0.020581 | 0.041716 | 0.059803 | 0.000768 | 0.000779 | 0.001731 |
| mp_replace_third.test | RUN | 0.240282 | 0.922016 | 0.761926 | 0.001680 | 0.001568 | 0.001824 |
| mp_replace_third_2.test | RUN | 0.020704 | 0.041766 | 0.058966 | 0.000773 | 0.000783 | 0.001705 |
| mp_apply_q.test | RUN | 0.266111 | 2.175652 | 0.990041 | 0.001807 | 0.001666 | 0.001830 |
| mp_apply_q_sf.test | RUN | 0.217557 | 0.618644 | 0.607627 | 0.001698 | 0.001802 | 0.001747 |
| mp_is_list.test | RUN | 0.218475 | 0.726258 | 0.648073 | 0.001760 | 0.001742 | 0.001704 |
| mp_list_c.test | RUN | 0.227476 | 0.882589 | 0.711855 | 0.001708 | 0.001810 | 0.001697 |
| mp_transform_front.test | RUN | 0.253830 | 1.770800 | 0.877220 | 0.001584 | 0.001771 | 0.001732 |
| mp_transform_front_2.test | RUN | 0.020136 | 0.042027 | 0.057401 | 0.000570 | 0.000766 | 0.001769 |
| mp_transform_second.test | RUN | 0.244174 | 1.513635 | 0.780278 | 0.001506 | 0.001598 | 0.001590 |
| mp_transform_second_2.test | RUN | 0.019877 | 0.041772 | 0.059843 | 0.000792 | 0.000847 | 0.001580 |
| mp_transform_third.test | RUN | 0.228770 | 1.131938 | 0.714763 | 0.001480 | 0.001785 | 0.001733 |
| mp_transform_third_2.test | RUN | 0.020666 | 0.042056 | 0.058169 | 0.000752 | 0.000688 | 0.001543 |
| mp_list_v.test | RUN | 0.020557 | 0.041566 | 0.059979 | 0.000778 | 0.000772 | 0.001545 |
| mp_rename_v.test | RUN | 0.020883 | 0.042529 | 0.057964 | 0.000652 | 0.000770 | 0.001511 |
| mp_is_value_list.test | RUN | 0.212432 | 0.452403 | 0.593668 | 0.001752 | 0.001598 | 0.001339 |
| mp_transform.test | RUN | 0.455590 | 3.704622 | 1.879327 | 0.001800 | 0.001510 | 0.001736 |
| mp_transform_2.test | RUN | 0.203154 | 0.401006 | 0.531792 | 0.001790 | 0.001679 | 0.001671 |
| mp_transform_q.test | RUN | 0.450151 | 3.546766 | 1.800858 | 0.001716 | 0.001619 | 0.001704 |
| mp_transform_q_2.test | RUN | 0.498978 | 1.107025 | 2.328176 | 0.001910 | 0.001452 | 0.001881 |
| mp_transform_sf.test | RUN | 0.230866 | 0.623629 | 0.642802 | 0.001770 | 0.001751 | 0.001727 |
| mp_transform_if.test | RUN | 0.275605 | 1.766889 | 0.971355 | 0.001756 | 0.001592 | 0.001716 |
| mp_transform_if_q.test | RUN | 0.274894 | 1.772735 | 0.983532 | 0.001728 | 0.001485 | 0.001280 |
| mp_filter.test | RUN | 0.252177 | 1.021694 | 0.745296 | 0.001738 | 0.001760 | 0.001709 |
| mp_fill.test | RUN | 0.238092 | 0.783027 | 0.674266 | 0.001647 | 0.001761 | 0.001721 |
| mp_fill_2.test | RUN | 0.036628 | 0.064491 | 0.100265 | 0.000759 | 0.000774 | 0.001683 |
| mp_count.test | RUN | 0.434066 | 1.900641 | 1.110128 | 0.002092 | 0.001816 | 0.001797 |
| mp_count_if.test | RUN | 0.433546 | 1.934898 | 1.192536 | 0.001585 | 0.001513 | 0.001760 |
| mp_count_if_q.test | RUN | 0.242597 | 1.257515 | 0.716424 | 0.001792 | 0.001638 | 0.001654 |
| mp_contains.test | RUN | 0.221435 | 0.752738 | 0.671175 | 0.001641 | 0.001726 | 0.001736 |
| mp_starts_with.test | RUN | 0.230082 | 0.754861 | 0.683815 | 0.001742 | 0.001773 | 0.001594 |
| mp_starts_with_sf.test | RUN | 0.227207 | 0.635565 | 0.644656 | 0.001601 | 0.001686 | 0.001434 |
| mp_repeat.test | RUN | 0.359455 | 3.177560 | 1.240623 | 0.001835 | 0.001801 | 0.001827 |
| mp_repeat_2.test | RUN | 0.035746 | 0.068415 | 0.095550 | 0.000849 | 0.000782 | 0.001721 |
| mp_repeat_3.test | RUN | 0.275419 | 0.396405 | 0.637193 | 0.001908 | 0.001690 | 0.001729 |
| mp_product.test | RUN | 0.263895 | 1.891386 | 0.948727 | 0.001620 | 0.001808 | 0.001728 |
| mp_drop.test | RUN | 0.278263 | 2.389598 | 1.065617 | 0.001640 | 0.001639 | 0.001764 |
| mp_drop_2.test | RUN | 0.036288 | 0.067593 | 0.092858 | 0.000596 | 0.000794 | 0.001672 |
| mp_iota.test | RUN | 0.271570 | 1.983940 | 0.945791 | 0.002058 | 0.001704 | 0.001732 |
| mp_iota_2.test | RUN | 0.300371 | 2.410157 | 1.073130 | 0.001689 | 0.001732 | 0.001593 |
| mp_at.test | RUN | 0.242797 | 1.051091 | 0.721322 | 0.001642 | 0.001770 | 0.001586 |
| mp_at_2.test | RUN | 0.036846 | 0.063168 | 0.098499 | 0.000785 | 0.000806 | 0.001694 |
| mp_at_sf.test | RUN | 0.206344 | 0.629241 | 0.623204 | 0.001853 | 0.001842 | 0.001778 |
| mp_take.test | RUN | 0.361512 | 3.114422 | 1.225381 | 0.001872 | 0.001826 | 0.001799 |
| mp_take_2.test | RUN | 0.036931 | 0.066416 | 0.098312 | 0.000796 | 0.000831 | 0.001682 |
| mp_slice.test | RUN | 0.409535 | 3.208891 | 1.617842 | 0.001905 | 0.001878 | 0.001834 |
| mp_slice_2.test | RUN | 0.035177 | 0.066186 | 0.099578 | 0.000726 | 0.000805 | 0.001732 |
| mp_replace.test | RUN | 0.281369 | 1.608052 | 1.023057 | 0.001794 | 0.001713 | 0.001794 |
| mp_replace_if.test | RUN | 0.264976 | 1.408129 | 0.931415 | 0.001825 | 0.001813 | 0.001463 |
| mp_replace_if_q.test | RUN | 0.263796 | 1.420754 | 0.966023 | 0.001962 | 0.001807 | 0.001816 |
| mp_copy_if.test | RUN | 0.274609 | 1.988303 | 0.893925 | 0.001554 | 0.001768 | 0.001768 |
| mp_copy_if_q.test | RUN | 0.277313 | 1.978208 | 0.903182 | 0.001806 | 0.001886 | 0.001826 |
| mp_remove.test | RUN | 0.266069 | 1.336913 | 0.923643 | 0.001926 | 0.001828 | 0.001575 |
| mp_remove_if.test | RUN | 0.278496 | 2.108107 | 0.957364 | 0.001767 | 0.001852 | 0.001790 |
| mp_remove_if_q.test | RUN | 0.283161 | 2.096365 | 0.960268 | 0.001784 | 0.001792 | 0.001846 |
| mp_partition.test | RUN | 0.281105 | 1.966928 | 1.024763 | 0.001853 | 0.001546 | 0.001826 |
| mp_partition_q.test | RUN | 0.279226 | 1.980160 | 1.035460 | 0.001788 | 0.001802 | 0.001661 |
| mp_sort.test | RUN | 0.256681 | 1.414438 | 0.890836 | 0.001584 | 0.001882 | 0.001710 |
| mp_sort_q.test | RUN | 0.266052 | 1.390361 | 0.885236 | 0.001754 | 0.001591 | 0.001494 |
| mp_find.test | RUN | 0.257604 | 1.499646 | 0.779538 | 0.001832 | 0.001657 | 0.001566 |
| mp_find_if.test | RUN | 0.250453 | 1.394678 | 0.745412 | 0.001880 | 0.001756 | 0.001466 |
| mp_find_if_q.test | RUN | 0.267655 | 1.556073 | 0.889056 | 0.001645 | 0.001823 | 0.001776 |
| mp_reverse.test | RUN | 0.500902 | 4.185290 | 1.787530 | 0.001823 | 0.001880 | 0.001768 |
| mp_fold.test | RUN | 0.866095 | 3.176289 | 4.585444 | 0.001686 | 0.001792 | 0.001570 |
| mp_fold_q.test | RUN | 0.255334 | 1.401836 | 0.853189 | 0.001637 | 0.001576 | 0.001693 |
| mp_fold_q_sf.test | RUN | 0.230095 | 0.633712 | 0.660390 | 0.001558 | 0.001674 | 0.001818 |
| mp_reverse_fold.test | RUN | 0.416580 | 3.125201 | 1.242482 | 0.001579 | 0.001777 | 0.001740 |
| mp_reverse_fold_q.test | RUN | 0.265526 | 1.711770 | 0.901265 | 0.001705 | 0.001854 | 0.001522 |
| mp_unique.test | RUN | 0.260530 | 1.538111 | 0.884963 | 0.001670 | 0.001716 | 0.001779 |
| mp_unique_if.test | RUN | 0.436422 | 3.218840 | 1.434658 | 0.001512 | 0.001813 | 0.001687 |
| mp_unique_if_q.test | RUN | 0.374759 | 3.186067 | 1.136335 | 0.001604 | 0.001878 | 0.001698 |
| mp_all_of.test | RUN | 0.568801 | 1.731880 | 1.504759 | 0.001774 | 0.001760 | 0.001594 |
| mp_all_of_q.test | RUN | 0.233359 | 0.750845 | 0.657019 | 0.001923 | 0.001826 | 0.001513 |
| mp_any_of.test | RUN | 0.577332 | 1.798725 | 1.483428 | 0.001517 | 0.001718 | 0.001778 |
| mp_any_of_q.test | RUN | 0.227371 | 0.743416 | 0.671206 | 0.001835 | 0.001830 | 0.001730 |
| mp_none_of.test | RUN | 0.592308 | 1.790458 | 1.454341 | 0.001849 | 0.001792 | 0.001800 |
| mp_none_of_q.test | RUN | 0.225963 | 0.762580 | 0.671271 | 0.001593 | 0.001892 | 0.001690 |
| mp_replace_at.test | RUN | 0.273424 | 1.554389 | 0.997553 | 0.001757 | 0.001682 | 0.001846 |
| mp_replace_at_c.test | RUN | 0.274827 | 1.520819 | 0.986916 | 0.001669 | 0.001800 | 0.001650 |
| mp_for_each.test | RUN | 0.254576 | 1.035572 | 1.033314 | 0.001574 | 0.001559 | 0.001506 |
| mp_insert.test | RUN | 0.390946 | 3.166953 | 1.465239 | 0.001866 | 0.001734 | 0.001896 |
| mp_insert_2.test | RUN | 0.035651 | 0.063161 | 0.096447 | 0.000770 | 0.000696 | 0.001656 |
| mp_erase.test | RUN | 0.434010 | 3.377656 | 1.726336 | 0.001805 | 0.001794 | 0.001602 |
| mp_erase_2.test | RUN | 0.036405 | 0.065064 | 0.097854 | 0.000696 | 0.000755 | 0.001726 |
| mp_with_index.test | RUN | 0.595695 | 6.745566 | 3.698129 | 0.002509 | 0.002265 | 0.002130 |
| mp_with_index_cx.test | RUN | 0.003832 | 0.020925 | 0.028580 | 0.000803 | 0.000787 | 0.001684 |
| mp_from_sequence.test | RUN | 0.272887 | 1.767455 | 0.960422 | 0.001835 | 0.001761 | 0.001650 |
| mp_from_sequence_2.test | RUN | 0.292209 | 2.086445 | 1.038145 | 0.001712 | 0.001790 | 0.001746 |
| mp_from_sequence_3.test | RUN | 0.246274 | 0.913340 | 0.729212 | 0.001808 | 0.001795 | 0.001820 |
| mp_min_element.test | RUN | 0.237756 | 0.820937 | 0.679674 | 0.001670 | 0.001758 | 0.001516 |
| mp_min_element_2.test | RUN | 0.036291 | 0.066548 | 0.101187 | 0.000786 | 0.000815 | 0.001728 |
| mp_min_element_q.test | RUN | 0.237449 | 0.820600 | 0.673866 | 0.001873 | 0.001515 | 0.001848 |
| mp_min_element_q_2.test | RUN | 0.036069 | 0.065298 | 0.092380 | 0.000798 | 0.000781 | 0.001723 |
| mp_max_element.test | RUN | 0.238261 | 0.816258 | 0.655199 | 0.001835 | 0.001683 | 0.001541 |
| mp_max_element_2.test | RUN | 0.035910 | 0.063808 | 0.098498 | 0.000691 | 0.000712 | 0.001746 |
| mp_max_element_q.test | RUN | 0.232315 | 0.824871 | 0.671325 | 0.001774 | 0.001804 | 0.001773 |
| mp_max_element_q_2.test | RUN | 0.035676 | 0.064106 | 0.094861 | 0.000753 | 0.000789 | 0.001621 |
| mp_nth_element.test | RUN | 0.259699 | 1.173449 | 0.842344 | 0.001581 | 0.001712 | 0.001750 |
| mp_nth_element_q.test | RUN | 0.252700 | 1.164085 | 0.822444 | 0.001598 | 0.001757 | 0.001616 |
| mp_back.test | RUN | 0.246255 | 0.955615 | 0.796140 | 0.001600 | 0.001682 | 0.001436 |
| mp_back_2.test | RUN | 0.036580 | 0.066608 | 0.101431 | 0.000766 | 0.000845 | 0.001634 |
| mp_pop_back.test | RUN | 0.631122 | 6.433787 | 1.712752 | 0.001609 | 0.001641 | 0.001773 |
| mp_pop_back_2.test | RUN | 0.035928 | 0.064446 | 0.093724 | 0.000798 | 0.000650 | 0.001664 |
| mp_flatten.test | RUN | 0.292524 | 2.271163 | 1.150443 | 0.001892 | 0.001812 | 0.001395 |
| mp_rotate_left.test | RUN | 0.284870 | 2.460318 | 1.120706 | 0.001700 | 0.001896 | 0.001527 |
| mp_rotate_right.test | RUN | 0.290395 | 2.430597 | 1.125729 | 0.001717 | 0.001747 | 0.001736 |
| mp_power_set.test | RUN | 0.283027 | 2.317329 | 1.125151 | 0.001737 | 0.001743 | 0.001538 |
| mp_partial_sum.test | RUN | 0.365836 | 3.140555 | 1.294075 | 0.001840 | 0.001628 | 0.001751 |
| mp_iterate.test | RUN | 0.259546 | 0.906013 | 0.877691 | 0.001585 | 0.001764 | 0.001684 |
| mp_pairwise_fold.test | RUN | 0.286416 | 2.160742 | 1.149860 | 0.001344 | 0.001628 | 0.001730 |
| mp_pairwise_fold_q.test | RUN | 0.230069 | 0.635072 | 0.637501 | 0.001836 | 0.001720 | 0.001746 |
| mp_sliding_fold.test | RUN | 0.520911 | 4.186817 | 2.160631 | 0.001724 | 0.001870 | 0.001675 |
| mp_sliding_fold_q.test | RUN | 0.509231 | 4.178823 | 2.146097 | 0.001670 | 0.001774 | 0.001744 |
| mp_intersperse.test | RUN | 0.275206 | 1.530057 | 0.989452 | 0.001844 | 0.001757 | 0.001708 |
| mp_split.test | RUN | 0.367874 | 2.959346 | 1.395642 | 0.001826 | 0.001758 | 0.001671 |
| mp_join.test | RUN | 0.352714 | 2.498069 | 1.231405 | 0.001782 | 0.001910 | 0.001728 |
| integral.test | RUN | 0.237425 | 1.100737 | 0.753032 | 0.001683 | 0.001659 | 0.001650 |
| mp_value.test | RUN | 0.016044 | 0.036196 | 0.049871 | 0.000746 | 0.000771 | 0.001663 |
| mp_identity.test | RUN | 0.229812 | 1.008855 | 0.710740 | 0.001711 | 0.001578 | 0.001628 |
| mp_inherit.test | RUN | 0.220230 | 0.727726 | 0.647544 | 0.001561 | 0.001842 | 0.001726 |
| mp_if.test | RUN | 0.226436 | 0.816804 | 0.657115 | 0.001796 | 0.001772 | 0.001750 |
| mp_if_sf.test | RUN | 0.218514 | 0.604669 | 0.597470 | 0.001688 | 0.001646 | 0.001512 |
| mp_eval_if.test | RUN | 0.233556 | 0.869897 | 0.677653 | 0.001681 | 0.001606 | 0.001583 |
| mp_eval_if_sf.test | RUN | 0.218623 | 0.614872 | 0.596543 | 0.001633 | 0.001720 | 0.001720 |
| mp_valid.test | RUN | 0.220786 | 0.619321 | 0.616628 | 0.001734 | 0.001638 | 0.001806 |
| mp_defer.test | RUN | 0.234256 | 1.066827 | 0.740363 | 0.001970 | 0.001786 | 0.001730 |
| mp_quote.test | RUN | 0.236463 | 1.065474 | 0.761191 | 0.001684 | 0.001810 | 0.001700 |
| mp_invoke_q.test | RUN | 0.253740 | 1.423809 | 0.844494 | 0.001626 | 0.001793 | 0.001668 |
| mp_invoke_q_sf.test | RUN | 0.214618 | 0.611796 | 0.610847 | 0.001748 | 0.001794 | 0.001732 |
| mp_quote_trait.test | RUN | 0.235602 | 1.187128 | 0.740625 | 0.001686 | 0.001519 | 0.001753 |
| mp_cond.test | RUN | 0.218683 | 0.708032 | 0.641930 | 0.001702 | 0.001658 | 0.001673 |
| mp_cond_sf.test | RUN | 0.215937 | 0.623777 | 0.604439 | 0.001624 | 0.001701 | 0.001750 |
| mp_not_fn.test | RUN | 0.213576 | 0.615794 | 0.590949 | 0.001734 | 0.001768 | 0.001733 |
| mp_eval_if_not.test | RUN | 0.223361 | 0.874900 | 0.674205 | 0.001692 | 0.001787 | 0.001676 |
| mp_eval_or.test | RUN | 0.214544 | 0.462921 | 0.622050 | 0.001906 | 0.001678 | 0.001651 |
| mp_compose.test | RUN | 0.238868 | 1.445885 | 0.814310 | 0.001787 | 0.001671 | 0.001556 |
| mp_valid_and_true.test | RUN | 0.215402 | 0.741046 | 0.633363 | 0.001824 | 0.001730 | 0.001725 |
| mp_compose_sf.test | COMPILE | 0.052042 | 0.081337 | 0.140875 | — | — | — |
| integer_sequence.test | RUN | 0.256901 | 1.425456 | 0.914769 | 0.001768 | 0.001583 | 0.001850 |
| tuple_for_each.test | RUN | 0.220030 | 0.522010 | 0.655756 | 0.001773 | 0.001701 | 0.001702 |
| tuple_for_each_cx.test | COMPILE | 0.041768 | 0.072871 | 0.111276 | — | — | — |
| tuple_apply.test | RUN | 0.207670 | 0.517298 | 0.659853 | 0.001789 | 0.001510 | 0.001715 |
| tuple_apply_cx.test | COMPILE | 0.064569 | 0.101495 | 0.157961 | — | — | — |
| construct_from_tuple.test | RUN | 0.225813 | 0.556445 | 0.646822 | 0.001875 | 0.001856 | 0.001651 |
| construct_from_tuple_cx.test | COMPILE | 0.061245 | 0.105893 | 0.151622 | — | — | — |
| tuple_transform.test | RUN | 0.351406 | 1.130588 | 1.279519 | 0.001886 | 0.001762 | 0.001482 |
| tuple_transform_2.test | RUN | 0.299247 | 0.824745 | 1.159890 | 0.001788 | 0.001670 | 0.001664 |
| tuple_transform_cx.test | COMPILE | 0.052466 | 0.103218 | 0.141578 | — | — | — |
| mp_set_contains.test | RUN | 0.227516 | 0.747961 | 0.655238 | 0.001684 | 0.001845 | 0.001640 |
| mp_set_push_back.test | RUN | 0.288164 | 2.339259 | 1.141119 | 0.001771 | 0.001894 | 0.001783 |
| mp_set_push_front.test | RUN | 0.355326 | 2.814094 | 1.303899 | 0.001724 | 0.001758 | 0.001556 |
| mp_is_set.test | RUN | 0.223696 | 0.748824 | 0.669408 | 0.001626 | 0.001792 | 0.001730 |
| mp_set_union.test | RUN | 0.282644 | 2.005244 | 1.085062 | 0.001569 | 0.001809 | 0.001669 |
| mp_set_union_sf.test | RUN | 0.214108 | 0.597719 | 0.617170 | 0.001593 | 0.001548 | 0.001750 |
| mp_set_difference.test | RUN | 0.258411 | 1.547934 | 0.908798 | 0.001494 | 0.001527 | 0.001591 |
| mp_set_difference_sf.test | RUN | 0.217993 | 0.597046 | 0.610293 | 0.001557 | 0.001840 | 0.001442 |
| mp_set_intersection.test | RUN | 0.255160 | 1.544775 | 0.893956 | 0.001815 | 0.001581 | 0.001700 |
| mp_set_intersection_sf.test | RUN | 0.224986 | 0.604666 | 0.604197 | 0.001603 | 0.001868 | 0.001625 |
| mp_all.test | RUN | 0.220095 | 0.749577 | 0.650999 | 0.001642 | 0.001908 | 0.001784 |
| mp_all_2.test | RUN | 0.574084 | 1.615627 | 1.499243 | 0.001779 | 0.001643 | 0.001612 |
| mp_and.test | RUN | 0.221615 | 0.738443 | 0.632080 | 0.001780 | 0.001836 | 0.001722 |
| mp_any.test | RUN | 0.223642 | 0.747197 | 0.666270 | 0.001615 | 0.001698 | 0.001714 |
| mp_any_2.test | RUN | 0.567478 | 1.628142 | 1.435711 | 0.001760 | 0.001803 | 0.001636 |
| mp_or.test | RUN | 0.221391 | 0.742086 | 0.643547 | 0.001765 | 0.001710 | 0.001700 |
| mp_same.test | RUN | 0.407325 | 1.237996 | 1.007097 | 0.001679 | 0.001536 | 0.001516 |
| mp_plus.test | RUN | 0.225293 | 0.890298 | 0.753084 | 0.001562 | 0.001696 | 0.001682 |
| mp_less.test | RUN | 0.223753 | 0.749766 | 0.653934 | 0.001870 | 0.001652 | 0.001446 |
| mp_min.test | RUN | 0.217094 | 0.682037 | 0.631317 | 0.001560 | 0.001840 | 0.001750 |
| mp_max.test | RUN | 0.225457 | 0.752279 | 0.654482 | 0.001650 | 0.001662 | 0.001435 |
| mp_similar.test | RUN | 0.219140 | 0.730469 | 0.647285 | 0.001518 | 0.001807 | 0.001753 |
| mp_map_find.test | RUN | 0.253847 | 1.375126 | 0.865545 | 0.001738 | 0.001632 | 0.001761 |
| mp_map_find_2.test | RUN | 0.244592 | 0.526876 | 0.742894 | 0.001680 | 0.001815 | 0.001688 |
| mp_map_find_3.test | RUN | 0.240539 | 0.515922 | 0.713626 | 0.001626 | 0.001527 | 0.001668 |
| mp_map_find_4.test | RUN | 0.226081 | 0.492770 | 0.672318 | 0.001832 | 0.001596 | 0.001703 |
| mp_map_contains.test | RUN | 0.234814 | 0.768671 | 0.668878 | 0.001886 | 0.001862 | 0.001750 |
| mp_map_insert.test | RUN | 0.352849 | 2.615720 | 1.231648 | 0.001803 | 0.001860 | 0.001623 |
| mp_map_replace.test | RUN | 0.380195 | 2.985673 | 1.432437 | 0.001707 | 0.001730 | 0.001641 |
| mp_map_erase.test | RUN | 0.302486 | 2.200127 | 1.191830 | 0.001830 | 0.001757 | 0.001692 |
| mp_map_update.test | RUN | 0.262499 | 1.419394 | 0.917132 | 0.001778 | 0.001848 | 0.001498 |
| mp_map_update_q.test | RUN | 0.257544 | 1.416545 | 0.912359 | 0.001609 | 0.001508 | 0.001752 |
| mp_map_keys.test | RUN | 0.253166 | 1.263784 | 0.892994 | 0.001692 | 0.001608 | 0.001718 |
| mp_is_map.test | RUN | 0.229633 | 0.766437 | 0.703155 | 0.001490 | 0.001650 | 0.001764 |
| mp_bind.test | RUN | 0.285585 | 2.312603 | 1.070537 | 0.001703 | 0.001838 | 0.001754 |
| mp_bind_q.test | RUN | 0.252819 | 1.420514 | 0.846834 | 0.001622 | 0.001486 | 0.001664 |
| mp_bind_front.test | RUN | 0.249029 | 1.541835 | 0.834332 | 0.001685 | 0.001773 | 0.001774 |
| mp_bind_back.test | RUN | 0.251257 | 1.555079 | 0.834336 | 0.001517 | 0.001714 | 0.001601 |
| mp_lambda.test | RUN | 0.523528 | 4.640162 | 2.222791 | 0.001699 | 0.001230 | 0.001582 |
| mpl.test | RUN | 0.491917 | 1.204295 | 1.759327 | 0.001750 | 0.002342 | 0.001442 |
| mpl_list.test | RUN | 0.287402 | 0.924845 | 0.800854 | 0.001704 | 0.001620 | 0.001835 |
| mpl_tuple.test | RUN | 0.288983 | 0.922925 | 0.805483 | 0.001591 | 0.001761 | 0.001574 |
| version.test | RUN | 0.144156 | 0.332898 | 0.406070 | 0.001957 | 0.001734 | 0.001564 |
| mp11_version.test | RUN | 0.175516 | 0.378177 | 0.472725 | 0.001495 | 0.001584 | 0.001786 |
| list_version.test | RUN | 0.151274 | 0.335510 | 0.410016 | 0.001544 | 0.001642 | 0.001729 |
| algorithm_version.test | RUN | 0.166817 | 0.353898 | 0.443955 | 0.001884 | 0.001668 | 0.001710 |
| integral_version.test | RUN | 0.144641 | 0.325548 | 0.396971 | 0.001648 | 0.001597 | 0.001697 |
| utility_version.test | RUN | 0.151738 | 0.331049 | 0.408988 | 0.001551 | 0.001608 | 0.001476 |
| function_version.test | RUN | 0.154722 | 0.331448 | 0.428220 | 0.001633 | 0.001616 | 0.001697 |
| map_version.test | RUN | 0.168152 | 0.355212 | 0.453740 | 0.001706 | 0.001624 | 0.001668 |
| set_version.test | RUN | 0.151804 | 0.331543 | 0.428349 | 0.001818 | 0.001722 | 0.001581 |
| bind_version.test | RUN | 0.163452 | 0.354944 | 0.451154 | 0.001606 | 0.001568 | 0.001706 |
| integer_sequence_version.test | RUN | 0.146362 | 0.325758 | 0.395143 | 0.001652 | 0.001529 | 0.001196 |
| tuple_version.test | RUN | 0.157777 | 0.350488 | 0.292611 | 0.001385 | 0.001221 | 0.001848 |
| mpl_version.test | RUN | 0.111910 | 0.249846 | 0.421796 | 0.001318 | 0.001244 | 0.001272 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.004×, 1.003× | 0.994× |
| clang | 1.084× | 1.089×, 1.080× | 0.988× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000994s | 0.000998s | 0.000989s | byte equality observed |
| bind_version.test | 10 | 0.000982s | 0.000992s | 0.000985s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001004s | 0.000997s | 0.001020s | byte equality observed |
| function_version.test | 10 | 0.000991s | 0.000986s | 0.001051s | byte equality observed |
| integer_sequence.test | 10 | 0.000989s | 0.001002s | 0.001083s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000982s | 0.001004s | 0.001074s | byte equality observed |
| integral.test | 10 | 0.000978s | 0.001005s | 0.001059s | byte equality observed |
| integral_version.test | 10 | 0.000991s | 0.000987s | 0.001057s | byte equality observed |
| list_version.test | 10 | 0.000997s | 0.001000s | 0.001086s | byte equality observed |
| map_version.test | 10 | 0.000988s | 0.000992s | 0.001081s | byte equality observed |
| mp11_version.test | 10 | 0.000986s | 0.000990s | 0.001040s | byte equality observed |
| mp_all.test | 10 | 0.000989s | 0.001008s | 0.001046s | byte equality observed |
| mp_all_2.test | 10 | 0.001001s | 0.000991s | 0.001061s | byte equality observed |
| mp_all_of.test | 10 | 0.001009s | 0.000998s | 0.001063s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001042s | 0.000994s | 0.001059s | byte equality observed |
| mp_and.test | 10 | 0.001050s | 0.000989s | 0.001076s | byte equality observed |
| mp_any.test | 10 | 0.001042s | 0.000996s | 0.001083s | byte equality observed |
| mp_any_2.test | 10 | 0.001033s | 0.000980s | 0.001033s | byte equality observed |
| mp_any_of.test | 10 | 0.001043s | 0.001019s | 0.001070s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001073s | 0.001016s | 0.001040s | byte equality observed |
| mp_append.test | 10 | 0.001057s | 0.001105s | 0.001073s | byte equality observed |
| mp_append_2.test | 10 | 0.001068s | 0.001061s | 0.001064s | byte equality observed |
| mp_append_3.test | 10 | 0.000490s | 0.000501s | 0.001034s | byte equality observed |
| mp_append_sf.test | 10 | 0.001030s | 0.001020s | 0.001044s | byte equality observed |
| mp_apply_q.test | 10 | 0.001042s | 0.001092s | 0.001068s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001051s | 0.001040s | 0.001058s | byte equality observed |
| mp_assign.test | 10 | 0.001046s | 0.001096s | 0.001044s | byte equality observed |
| mp_assign_2.test | 10 | 0.000484s | 0.000489s | 0.001038s | byte equality observed |
| mp_at.test | 10 | 0.001068s | 0.001078s | 0.001052s | byte equality observed |
| mp_at_2.test | 10 | 0.000490s | 0.000482s | 0.001050s | byte equality observed |
| mp_at_sf.test | 10 | 0.001077s | 0.001084s | 0.001046s | byte equality observed |
| mp_back.test | 10 | 0.001045s | 0.001067s | 0.001047s | byte equality observed |
| mp_back_2.test | 10 | 0.000482s | 0.000462s | 0.001016s | byte equality observed |
| mp_bind.test | 10 | 0.001056s | 0.001080s | 0.001065s | byte equality observed |
| mp_bind_back.test | 10 | 0.001043s | 0.001067s | 0.001021s | byte equality observed |
| mp_bind_front.test | 10 | 0.001052s | 0.001054s | 0.001090s | byte equality observed |
| mp_bind_q.test | 10 | 0.001039s | 0.001056s | 0.001079s | byte equality observed |
| mp_clear.test | 10 | 0.001058s | 0.001075s | 0.001053s | byte equality observed |
| mp_clear_2.test | 10 | 0.000492s | 0.000473s | 0.001001s | byte equality observed |
| mp_compose.test | 10 | 0.001071s | 0.001097s | 0.001030s | byte equality observed |
| mp_cond.test | 10 | 0.001093s | 0.001074s | 0.001044s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001052s | 0.001083s | 0.001063s | byte equality observed |
| mp_contains.test | 10 | 0.001043s | 0.001080s | 0.001037s | byte equality observed |
| mp_copy_if.test | 10 | 0.001063s | 0.001047s | 0.001039s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001081s | 0.001090s | 0.001021s | byte equality observed |
| mp_count.test | 10 | 0.001063s | 0.001060s | 0.001052s | byte equality observed |
| mp_count_if.test | 10 | 0.001067s | 0.001058s | 0.001078s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001074s | 0.001046s | 0.001042s | byte equality observed |
| mp_defer.test | 10 | 0.001051s | 0.001043s | 0.001043s | byte equality observed |
| mp_drop.test | 10 | 0.001059s | 0.001048s | 0.001046s | byte equality observed |
| mp_drop_2.test | 10 | 0.000480s | 0.000474s | 0.001060s | byte equality observed |
| mp_empty.test | 10 | 0.001052s | 0.001089s | 0.001013s | byte equality observed |
| mp_empty_2.test | 10 | 0.000477s | 0.000468s | 0.001034s | byte equality observed |
| mp_erase.test | 10 | 0.001085s | 0.001093s | 0.001023s | byte equality observed |
| mp_erase_2.test | 10 | 0.000469s | 0.000479s | 0.001037s | byte equality observed |
| mp_eval_if.test | 10 | 0.001034s | 0.001063s | 0.001032s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001053s | 0.001072s | 0.001036s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001061s | 0.001071s | 0.001032s | byte equality observed |
| mp_eval_or.test | 10 | 0.001049s | 0.001048s | 0.001054s | byte equality observed |
| mp_fill.test | 10 | 0.001058s | 0.001045s | 0.001031s | byte equality observed |
| mp_fill_2.test | 10 | 0.000492s | 0.000474s | 0.001021s | byte equality observed |
| mp_filter.test | 10 | 0.001045s | 0.001058s | 0.001060s | byte equality observed |
| mp_find.test | 10 | 0.001061s | 0.001060s | 0.001081s | byte equality observed |
| mp_find_if.test | 10 | 0.001050s | 0.001064s | 0.001072s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001056s | 0.001043s | 0.001063s | byte equality observed |
| mp_flatten.test | 10 | 0.001077s | 0.001079s | 0.001093s | byte equality observed |
| mp_fold.test | 10 | 0.001085s | 0.001087s | 0.001023s | byte equality observed |
| mp_fold_q.test | 10 | 0.001036s | 0.001042s | 0.001058s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001036s | 0.001071s | 0.001062s | byte equality observed |
| mp_for_each.test | 10 | 0.001027s | 0.001028s | 0.001026s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001053s | 0.001045s | 0.001020s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001042s | 0.001042s | 0.001073s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001052s | 0.001030s | 0.001011s | byte equality observed |
| mp_front.test | 10 | 0.001043s | 0.001075s | 0.001049s | byte equality observed |
| mp_front_2.test | 10 | 0.000477s | 0.000489s | 0.001044s | byte equality observed |
| mp_identity.test | 10 | 0.001058s | 0.001054s | 0.001064s | byte equality observed |
| mp_if.test | 10 | 0.001071s | 0.001042s | 0.001012s | byte equality observed |
| mp_if_sf.test | 10 | 0.001057s | 0.001032s | 0.001036s | byte equality observed |
| mp_inherit.test | 10 | 0.001043s | 0.001094s | 0.001074s | byte equality observed |
| mp_insert.test | 10 | 0.001068s | 0.001066s | 0.001056s | byte equality observed |
| mp_insert_2.test | 10 | 0.000468s | 0.000479s | 0.001012s | byte equality observed |
| mp_intersperse.test | 10 | 0.001064s | 0.001030s | 0.001028s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001058s | 0.001050s | 0.001044s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001046s | 0.001077s | 0.001027s | byte equality observed |
| mp_iota.test | 10 | 0.001065s | 0.001064s | 0.001077s | byte equality observed |
| mp_iota_2.test | 10 | 0.001044s | 0.001044s | 0.001041s | byte equality observed |
| mp_is_list.test | 10 | 0.001026s | 0.001059s | 0.001033s | byte equality observed |
| mp_is_map.test | 10 | 0.001030s | 0.001064s | 0.001063s | byte equality observed |
| mp_is_set.test | 10 | 0.001045s | 0.001079s | 0.001016s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001037s | 0.001067s | 0.001071s | byte equality observed |
| mp_iterate.test | 10 | 0.001080s | 0.001048s | 0.001043s | byte equality observed |
| mp_join.test | 10 | 0.001067s | 0.001058s | 0.001086s | byte equality observed |
| mp_lambda.test | 10 | 0.001071s | 0.001102s | 0.001027s | byte equality observed |
| mp_less.test | 10 | 0.001063s | 0.001059s | 0.001040s | byte equality observed |
| mp_list_c.test | 10 | 0.001082s | 0.001071s | 0.001062s | byte equality observed |
| mp_list_v.test | 10 | 0.000475s | 0.000480s | 0.001022s | byte equality observed |
| mp_map_contains.test | 10 | 0.001035s | 0.001047s | 0.001023s | byte equality observed |
| mp_map_erase.test | 10 | 0.001048s | 0.001064s | 0.001043s | byte equality observed |
| mp_map_find.test | 10 | 0.001037s | 0.001083s | 0.001037s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001062s | 0.001067s | 0.001042s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001064s | 0.001051s | 0.001057s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001057s | 0.001067s | 0.001061s | byte equality observed |
| mp_map_insert.test | 10 | 0.001079s | 0.001085s | 0.001037s | byte equality observed |
| mp_map_keys.test | 10 | 0.001048s | 0.001091s | 0.001055s | byte equality observed |
| mp_map_replace.test | 10 | 0.001085s | 0.001095s | 0.001031s | byte equality observed |
| mp_map_update.test | 10 | 0.001077s | 0.001113s | 0.001044s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001046s | 0.001075s | 0.001012s | byte equality observed |
| mp_max.test | 10 | 0.001062s | 0.001070s | 0.001044s | byte equality observed |
| mp_max_element.test | 10 | 0.001057s | 0.001079s | 0.001069s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000481s | 0.000489s | 0.001010s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001061s | 0.001086s | 0.001045s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000471s | 0.000482s | 0.001028s | byte equality observed |
| mp_min.test | 10 | 0.001045s | 0.001045s | 0.001044s | byte equality observed |
| mp_min_element.test | 10 | 0.001049s | 0.001043s | 0.001019s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000474s | 0.000489s | 0.000992s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001048s | 0.001071s | 0.001051s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000485s | 0.000483s | 0.000989s | byte equality observed |
| mp_none_of.test | 10 | 0.001054s | 0.001055s | 0.001036s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001037s | 0.001080s | 0.001060s | byte equality observed |
| mp_not_fn.test | 10 | 0.001068s | 0.001053s | 0.001027s | byte equality observed |
| mp_nth_element.test | 10 | 0.001051s | 0.001065s | 0.001009s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001055s | 0.001063s | 0.001007s | byte equality observed |
| mp_or.test | 10 | 0.001075s | 0.001030s | 0.001036s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001027s | 0.001069s | 0.001049s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001026s | 0.001045s | 0.001050s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001059s | 0.001055s | 0.001041s | byte equality observed |
| mp_partition.test | 10 | 0.001048s | 0.001071s | 0.001070s | byte equality observed |
| mp_partition_q.test | 10 | 0.001031s | 0.001069s | 0.001024s | byte equality observed |
| mp_plus.test | 10 | 0.001035s | 0.001079s | 0.001062s | byte equality observed |
| mp_pop_back.test | 10 | 0.001067s | 0.001077s | 0.001040s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000471s | 0.000478s | 0.001067s | byte equality observed |
| mp_pop_front.test | 10 | 0.001041s | 0.001053s | 0.001038s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000477s | 0.000481s | 0.001025s | byte equality observed |
| mp_power_set.test | 10 | 0.001034s | 0.001041s | 0.001048s | byte equality observed |
| mp_product.test | 10 | 0.001034s | 0.001063s | 0.000999s | byte equality observed |
| mp_push_back.test | 10 | 0.001074s | 0.001084s | 0.001051s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000470s | 0.000469s | 0.001025s | byte equality observed |
| mp_push_front.test | 10 | 0.001081s | 0.001083s | 0.001038s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000483s | 0.000467s | 0.001044s | byte equality observed |
| mp_quote.test | 10 | 0.001043s | 0.001070s | 0.001077s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001040s | 0.001040s | 0.001027s | byte equality observed |
| mp_remove.test | 10 | 0.001049s | 0.001060s | 0.001039s | byte equality observed |
| mp_remove_if.test | 10 | 0.001044s | 0.001092s | 0.001045s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001043s | 0.001071s | 0.001040s | byte equality observed |
| mp_rename.test | 10 | 0.001057s | 0.001071s | 0.001061s | byte equality observed |
| mp_rename_2.test | 10 | 0.000472s | 0.000490s | 0.001026s | byte equality observed |
| mp_rename_v.test | 10 | 0.000478s | 0.000477s | 0.000982s | byte equality observed |
| mp_repeat.test | 10 | 0.001100s | 0.001113s | 0.001016s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000473s | 0.000481s | 0.001023s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001056s | 0.001041s | 0.001034s | byte equality observed |
| mp_replace.test | 10 | 0.001047s | 0.001063s | 0.001060s | byte equality observed |
| mp_replace_at.test | 10 | 0.001037s | 0.001050s | 0.001042s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001028s | 0.001055s | 0.001041s | byte equality observed |
| mp_replace_front.test | 10 | 0.001049s | 0.001082s | 0.001048s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000477s | 0.000472s | 0.001006s | byte equality observed |
| mp_replace_if.test | 10 | 0.001016s | 0.001032s | 0.001038s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001069s | 0.001047s | 0.001036s | byte equality observed |
| mp_replace_second.test | 10 | 0.001049s | 0.001059s | 0.001032s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000476s | 0.000468s | 0.001042s | byte equality observed |
| mp_replace_third.test | 10 | 0.001051s | 0.001084s | 0.001012s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000476s | 0.000472s | 0.001013s | byte equality observed |
| mp_reverse.test | 10 | 0.001084s | 0.001067s | 0.001018s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001074s | 0.001070s | 0.001012s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001049s | 0.001039s | 0.001069s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001060s | 0.001087s | 0.001040s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001053s | 0.001038s | 0.001058s | byte equality observed |
| mp_same.test | 10 | 0.001050s | 0.001086s | 0.001043s | byte equality observed |
| mp_second.test | 10 | 0.001040s | 0.001105s | 0.001033s | byte equality observed |
| mp_second_2.test | 10 | 0.000487s | 0.000467s | 0.001026s | byte equality observed |
| mp_set_contains.test | 10 | 0.001066s | 0.001067s | 0.001070s | byte equality observed |
| mp_set_difference.test | 10 | 0.001039s | 0.001050s | 0.001043s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001043s | 0.001032s | 0.001013s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001067s | 0.001100s | 0.001058s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001028s | 0.001034s | 0.001042s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001083s | 0.001070s | 0.001019s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001072s | 0.001062s | 0.001008s | byte equality observed |
| mp_set_union.test | 10 | 0.001026s | 0.001068s | 0.001037s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001047s | 0.001032s | 0.001037s | byte equality observed |
| mp_similar.test | 10 | 0.001051s | 0.001018s | 0.001046s | byte equality observed |
| mp_size.test | 10 | 0.001026s | 0.001072s | 0.001022s | byte equality observed |
| mp_size_2.test | 10 | 0.000490s | 0.000479s | 0.001019s | byte equality observed |
| mp_slice.test | 10 | 0.001069s | 0.001091s | 0.001008s | byte equality observed |
| mp_slice_2.test | 10 | 0.000467s | 0.000473s | 0.001009s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001082s | 0.001082s | 0.001042s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001071s | 0.001083s | 0.001052s | byte equality observed |
| mp_sort.test | 10 | 0.001028s | 0.001084s | 0.001019s | byte equality observed |
| mp_sort_q.test | 10 | 0.001060s | 0.001075s | 0.001064s | byte equality observed |
| mp_split.test | 10 | 0.001077s | 0.001060s | 0.001048s | byte equality observed |
| mp_starts_with.test | 10 | 0.001046s | 0.001060s | 0.001032s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001045s | 0.001072s | 0.001068s | byte equality observed |
| mp_take.test | 10 | 0.001061s | 0.001056s | 0.001026s | byte equality observed |
| mp_take_2.test | 10 | 0.000477s | 0.000466s | 0.001032s | byte equality observed |
| mp_third.test | 10 | 0.001064s | 0.001064s | 0.001034s | byte equality observed |
| mp_third_2.test | 10 | 0.000476s | 0.000476s | 0.001026s | byte equality observed |
| mp_transform.test | 10 | 0.001061s | 0.001051s | 0.001020s | byte equality observed |
| mp_transform_2.test | 10 | 0.001036s | 0.001015s | 0.001007s | byte equality observed |
| mp_transform_front.test | 10 | 0.001044s | 0.001078s | 0.001026s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000469s | 0.000480s | 0.001005s | byte equality observed |
| mp_transform_if.test | 10 | 0.001049s | 0.001051s | 0.001033s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001066s | 0.001060s | 0.001033s | byte equality observed |
| mp_transform_q.test | 10 | 0.001089s | 0.001060s | 0.001014s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001053s | 0.001082s | 0.001054s | byte equality observed |
| mp_transform_second.test | 10 | 0.001053s | 0.001067s | 0.001023s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000470s | 0.000486s | 0.001010s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001029s | 0.001034s | 0.001031s | byte equality observed |
| mp_transform_third.test | 10 | 0.001040s | 0.001056s | 0.001086s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000463s | 0.000481s | 0.000968s | byte equality observed |
| mp_unique.test | 10 | 0.001046s | 0.001075s | 0.001063s | byte equality observed |
| mp_unique_if.test | 10 | 0.001063s | 0.001040s | 0.001057s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001080s | 0.001062s | 0.001046s | byte equality observed |
| mp_valid.test | 10 | 0.001032s | 0.001069s | 0.001041s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001050s | 0.001042s | 0.001035s | byte equality observed |
| mp_value.test | 10 | 0.000477s | 0.000478s | 0.001000s | byte equality observed |
| mp_with_index.test | 10 | 0.001455s | 0.001418s | 0.001318s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000459s | 0.000471s | 0.000992s | byte equality observed |
| mpl.test | 10 | 0.001071s | 0.001066s | 0.001053s | byte equality observed |
| mpl_list.test | 10 | 0.001041s | 0.001056s | 0.001029s | byte equality observed |
| mpl_tuple.test | 10 | 0.001063s | 0.001105s | 0.001028s | byte equality observed |
| mpl_version.test | 10 | 0.001039s | 0.001065s | 0.001039s | byte equality observed |
| set_version.test | 10 | 0.001036s | 0.001053s | 0.001041s | byte equality observed |
| tuple_apply.test | 10 | 0.001097s | 0.001080s | 0.001031s | byte equality observed |
| tuple_for_each.test | 10 | 0.001064s | 0.001066s | 0.001009s | byte equality observed |
| tuple_transform.test | 10 | 0.001077s | 0.001054s | 0.001040s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001043s | 0.001082s | 0.001011s | byte equality observed |
| tuple_version.test | 10 | 0.001027s | 0.001032s | 0.001016s | byte equality observed |
| utility_version.test | 10 | 0.001085s | 0.001058s | 0.001016s | byte equality observed |
| version.test | 10 | 0.001025s | 0.001057s | 0.001055s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
