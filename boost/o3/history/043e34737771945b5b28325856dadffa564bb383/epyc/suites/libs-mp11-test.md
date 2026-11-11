# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.035029 | 0.056355 | 0.088187 | — | — | — |
| complex_h.test | COMPILE | 0.143665 | 0.244285 | 0.325552 | — | — | — |
| mp_size.test | RUN | 0.164043 | 0.765757 | 0.432190 | 0.001684 | 0.001662 | 0.001532 |
| mp_size_2.test | RUN | 0.015332 | 0.029479 | 0.043792 | 0.000791 | 0.000636 | 0.002018 |
| mp_empty.test | RUN | 0.164023 | 0.477491 | 0.425644 | 0.001659 | 0.001789 | 0.002018 |
| mp_empty_2.test | RUN | 0.015635 | 0.030923 | 0.047438 | 0.000625 | 0.000741 | 0.001638 |
| mp_assign.test | RUN | 0.174021 | 0.991366 | 0.608254 | 0.001455 | 0.001598 | 0.001732 |
| mp_assign_2.test | RUN | 0.016192 | 0.030516 | 0.047971 | 0.000738 | 0.000758 | 0.001698 |
| mp_clear.test | RUN | 0.158735 | 0.437379 | 0.429310 | 0.001515 | 0.001570 | 0.001484 |
| mp_clear_2.test | RUN | 0.015937 | 0.031046 | 0.051491 | 0.000789 | 0.000606 | 0.001616 |
| mp_front.test | RUN | 0.159798 | 0.703619 | 0.493751 | 0.001792 | 0.001744 | 0.001615 |
| mp_front_2.test | RUN | 0.015969 | 0.030760 | 0.043521 | 0.000641 | 0.000661 | 0.002318 |
| mp_pop_front.test | RUN | 0.166584 | 0.732452 | 0.493155 | 0.001778 | 0.001645 | 0.001593 |
| mp_pop_front_2.test | RUN | 0.014208 | 0.030120 | 0.045371 | 0.000624 | 0.000605 | 0.001799 |
| mp_second.test | RUN | 0.156067 | 0.449433 | 0.489880 | 0.001510 | 0.001548 | 0.001409 |
| mp_second_2.test | RUN | 0.016200 | 0.030796 | 0.049206 | 0.000562 | 0.000767 | 0.001619 |
| mp_third.test | RUN | 0.153237 | 0.429998 | 0.457540 | 0.001584 | 0.001871 | 0.001616 |
| mp_third_2.test | RUN | 0.014709 | 0.030090 | 0.045721 | 0.000671 | 0.000578 | 0.001536 |
| mp_push_front.test | RUN | 0.226899 | 1.566376 | 0.776690 | 0.001890 | 0.001850 | 0.001840 |
| mp_push_front_2.test | RUN | 0.015936 | 0.029216 | 0.047068 | 0.000627 | 0.000678 | 0.001604 |
| mp_push_back.test | RUN | 0.230336 | 1.576263 | 0.805021 | 0.001317 | 0.001920 | 0.001594 |
| mp_push_back_2.test | RUN | 0.013745 | 0.031022 | 0.046368 | 0.000795 | 0.000739 | 0.001841 |
| mp_rename.test | RUN | 0.181748 | 1.452059 | 0.698791 | 0.001361 | 0.001715 | 0.001998 |
| mp_rename_2.test | RUN | 0.015640 | 0.031679 | 0.050953 | 0.000763 | 0.000783 | 0.001371 |
| mp_append.test | RUN | 0.192864 | 1.688714 | 0.830067 | 0.001603 | 0.001832 | 0.001723 |
| mp_append_2.test | RUN | 0.379441 | 3.629455 | 1.060586 | 0.002029 | 0.001688 | 0.001613 |
| mp_append_3.test | RUN | 0.015205 | 0.032220 | 0.044286 | 0.000780 | 0.000867 | 0.001819 |
| mp_append_sf.test | RUN | 0.154826 | 0.428739 | 0.425648 | 0.002852 | 0.001550 | 0.001666 |
| mp_replace_front.test | RUN | 0.248601 | 1.176429 | 0.601628 | 0.001862 | 0.001401 | 0.001465 |
| mp_replace_front_2.test | RUN | 0.015037 | 0.029180 | 0.048567 | 0.000770 | 0.000754 | 0.001565 |
| mp_replace_second.test | RUN | 0.186979 | 0.721909 | 0.581716 | 0.001688 | 0.001686 | 0.001858 |
| mp_replace_second_2.test | RUN | 0.017008 | 0.030962 | 0.048449 | 0.000638 | 0.000728 | 0.001581 |
| mp_replace_third.test | RUN | 0.166535 | 0.602249 | 0.521528 | 0.001748 | 0.001631 | 0.001920 |
| mp_replace_third_2.test | RUN | 0.015556 | 0.028246 | 0.044500 | 0.000604 | 0.000652 | 0.001844 |
| mp_apply_q.test | RUN | 0.179218 | 1.422645 | 0.682622 | 0.001734 | 0.001625 | 0.001810 |
| mp_apply_q_sf.test | RUN | 0.152291 | 0.415523 | 0.412292 | 0.001774 | 0.001642 | 0.001858 |
| mp_is_list.test | RUN | 0.153635 | 0.481752 | 0.440593 | 0.001508 | 0.001718 | 0.001753 |
| mp_list_c.test | RUN | 0.160633 | 0.599346 | 0.457825 | 0.001816 | 0.001369 | 0.001484 |
| mp_transform_front.test | RUN | 0.178632 | 1.170102 | 0.595605 | 0.001652 | 0.001572 | 0.001758 |
| mp_transform_front_2.test | RUN | 0.015905 | 0.029953 | 0.046708 | 0.000612 | 0.000785 | 0.001660 |
| mp_transform_second.test | RUN | 0.169347 | 0.984827 | 0.558204 | 0.001584 | 0.001784 | 0.001655 |
| mp_transform_second_2.test | RUN | 0.015481 | 0.031083 | 0.043963 | 0.000787 | 0.000766 | 0.001858 |
| mp_transform_third.test | RUN | 0.167144 | 0.762513 | 0.474981 | 0.001734 | 0.001632 | 0.001255 |
| mp_transform_third_2.test | RUN | 0.015050 | 0.029630 | 0.049770 | 0.000542 | 0.000640 | 0.001843 |
| mp_list_v.test | RUN | 0.015112 | 0.028756 | 0.050599 | 0.000535 | 0.000560 | 0.001752 |
| mp_rename_v.test | RUN | 0.014244 | 0.028481 | 0.046327 | 0.000605 | 0.000622 | 0.001774 |
| mp_is_value_list.test | RUN | 0.153718 | 0.294481 | 0.425694 | 0.001786 | 0.001791 | 0.001729 |
| mp_transform.test | RUN | 0.298051 | 2.449038 | 1.318753 | 0.001910 | 0.001834 | 0.001760 |
| mp_transform_2.test | RUN | 0.152059 | 0.265866 | 0.381920 | 0.001382 | 0.001792 | 0.001780 |
| mp_transform_q.test | RUN | 0.287064 | 2.356559 | 1.232728 | 0.001611 | 0.001633 | 0.001599 |
| mp_transform_q_2.test | RUN | 0.373978 | 0.716634 | 1.624135 | 0.001572 | 0.001316 | 0.001768 |
| mp_transform_sf.test | RUN | 0.199055 | 0.422220 | 0.437545 | 0.001822 | 0.001642 | 0.001742 |
| mp_transform_if.test | RUN | 0.183216 | 1.150579 | 0.673656 | 0.001746 | 0.001582 | 0.001776 |
| mp_transform_if_q.test | RUN | 0.189950 | 1.145701 | 0.665425 | 0.001497 | 0.001429 | 0.001848 |
| mp_filter.test | RUN | 0.176633 | 0.706926 | 0.497528 | 0.001611 | 0.001983 | 0.001584 |
| mp_fill.test | RUN | 0.170656 | 0.537480 | 0.518818 | 0.001938 | 0.001614 | 0.002030 |
| mp_fill_2.test | RUN | 0.027238 | 0.044772 | 0.089371 | 0.000590 | 0.000797 | 0.001832 |
| mp_count.test | RUN | 0.335341 | 1.302607 | 0.788847 | 0.001764 | 0.001745 | 0.001598 |
| mp_count_if.test | RUN | 0.338231 | 1.317238 | 0.839816 | 0.001526 | 0.001490 | 0.001855 |
| mp_count_if_q.test | RUN | 0.179503 | 0.827957 | 0.495916 | 0.001657 | 0.001634 | 0.001720 |
| mp_contains.test | RUN | 0.170552 | 0.505796 | 0.462625 | 0.001827 | 0.001784 | 0.001536 |
| mp_starts_with.test | RUN | 0.168264 | 0.508609 | 0.464171 | 0.001606 | 0.001890 | 0.001341 |
| mp_starts_with_sf.test | RUN | 0.158108 | 0.420976 | 0.416564 | 0.001485 | 0.001662 | 0.001776 |
| mp_repeat.test | RUN | 0.235454 | 2.097398 | 0.890197 | 0.001898 | 0.001879 | 0.001592 |
| mp_repeat_2.test | RUN | 0.025386 | 0.045473 | 0.073509 | 0.000736 | 0.000780 | 0.001523 |
| mp_repeat_3.test | RUN | 0.200761 | 0.274490 | 0.458745 | 0.001696 | 0.001669 | 0.001754 |
| mp_product.test | RUN | 0.188147 | 1.239688 | 0.635102 | 0.001482 | 0.001826 | 0.001788 |
| mp_drop.test | RUN | 0.193485 | 1.585321 | 0.739653 | 0.001502 | 0.001544 | 0.001838 |
| mp_drop_2.test | RUN | 0.027057 | 0.048177 | 0.081242 | 0.000677 | 0.000776 | 0.001658 |
| mp_iota.test | RUN | 0.199471 | 1.341445 | 0.631817 | 0.001398 | 0.001992 | 0.001862 |
| mp_iota_2.test | RUN | 0.210875 | 1.620072 | 0.760813 | 0.001486 | 0.001790 | 0.001676 |
| mp_at.test | RUN | 0.179990 | 0.720996 | 0.531759 | 0.001516 | 0.001735 | 0.001818 |
| mp_at_2.test | RUN | 0.027587 | 0.049593 | 0.082053 | 0.000841 | 0.000829 | 0.001780 |
| mp_at_sf.test | RUN | 0.161127 | 0.451862 | 0.427516 | 0.001625 | 0.001693 | 0.001643 |
| mp_take.test | RUN | 0.235262 | 2.046688 | 0.890423 | 0.001792 | 0.001677 | 0.001944 |
| mp_take_2.test | RUN | 0.024813 | 0.045150 | 0.067419 | 0.000682 | 0.000725 | 0.001644 |
| mp_slice.test | RUN | 0.261032 | 2.060769 | 1.168992 | 0.001696 | 0.001763 | 0.001636 |
| mp_slice_2.test | RUN | 0.026267 | 0.045820 | 0.072692 | 0.000829 | 0.000722 | 0.001932 |
| mp_replace.test | RUN | 0.200702 | 1.038113 | 0.742447 | 0.001822 | 0.001792 | 0.001869 |
| mp_replace_if.test | RUN | 0.194093 | 0.915735 | 0.646656 | 0.001822 | 0.001503 | 0.001986 |
| mp_replace_if_q.test | RUN | 0.188959 | 0.914287 | 0.661237 | 0.001572 | 0.001442 | 0.001824 |
| mp_copy_if.test | RUN | 0.194647 | 1.271569 | 0.618178 | 0.001644 | 0.001683 | 0.001862 |
| mp_copy_if_q.test | RUN | 0.200073 | 1.266201 | 0.620255 | 0.001770 | 0.001501 | 0.001892 |
| mp_remove.test | RUN | 0.187014 | 0.847862 | 0.632882 | 0.003641 | 0.001803 | 0.001647 |
| mp_remove_if.test | RUN | 0.196727 | 1.311695 | 0.659248 | 0.001692 | 0.001779 | 0.001821 |
| mp_remove_if_q.test | RUN | 0.214788 | 1.310303 | 0.657922 | 0.001664 | 0.001718 | 0.001806 |
| mp_partition.test | RUN | 0.251850 | 1.228981 | 0.692095 | 0.001879 | 0.001706 | 0.001845 |
| mp_partition_q.test | RUN | 0.198482 | 1.247011 | 0.695898 | 0.001669 | 0.001826 | 0.001758 |
| mp_sort.test | RUN | 0.185835 | 0.908862 | 0.612500 | 0.001578 | 0.001675 | 0.001778 |
| mp_sort_q.test | RUN | 0.188300 | 0.898489 | 0.569657 | 0.001619 | 0.001744 | 0.001810 |
| mp_find.test | RUN | 0.185137 | 0.972728 | 0.520090 | 0.001812 | 0.001767 | 0.001802 |
| mp_find_if.test | RUN | 0.176249 | 0.884385 | 0.504156 | 0.001703 | 0.001574 | 0.001584 |
| mp_find_if_q.test | RUN | 0.191867 | 1.003396 | 0.574922 | 0.001625 | 0.001671 | 0.001802 |
| mp_reverse.test | RUN | 0.326701 | 2.673617 | 1.227423 | 0.001918 | 0.001587 | 0.001732 |
| mp_fold.test | RUN | 0.549302 | 2.106827 | 3.106289 | 0.001663 | 0.001670 | 0.001720 |
| mp_fold_q.test | RUN | 0.181729 | 0.891183 | 0.536385 | 0.001898 | 0.001736 | 0.001750 |
| mp_fold_q_sf.test | RUN | 0.168729 | 0.407622 | 0.429830 | 0.001472 | 0.001782 | 0.001834 |
| mp_reverse_fold.test | RUN | 0.275605 | 1.986831 | 0.840057 | 0.001863 | 0.001782 | 0.001539 |
| mp_reverse_fold_q.test | RUN | 0.190873 | 1.094446 | 0.617483 | 0.001570 | 0.001840 | 0.001794 |
| mp_unique.test | RUN | 0.181414 | 0.981025 | 0.599280 | 0.001764 | 0.001882 | 0.001723 |
| mp_unique_if.test | RUN | 0.288938 | 2.110559 | 0.935482 | 0.001752 | 0.001760 | 0.001716 |
| mp_unique_if_q.test | RUN | 0.251945 | 2.074318 | 0.762204 | 0.001697 | 0.001543 | 0.001726 |
| mp_all_of.test | RUN | 0.414310 | 1.184317 | 0.973471 | 0.001555 | 0.001860 | 0.001749 |
| mp_all_of_q.test | RUN | 0.162050 | 0.504436 | 0.433950 | 0.001810 | 0.001898 | 0.001748 |
| mp_any_of.test | RUN | 0.410318 | 1.251521 | 0.967049 | 0.001618 | 0.001777 | 0.001749 |
| mp_any_of_q.test | RUN | 0.164042 | 0.504210 | 0.424719 | 0.001844 | 0.001697 | 0.001792 |
| mp_none_of.test | RUN | 0.411093 | 1.150668 | 0.942787 | 0.001468 | 0.001834 | 0.001767 |
| mp_none_of_q.test | RUN | 0.166609 | 0.499978 | 0.434659 | 0.001588 | 0.001539 | 0.001584 |
| mp_replace_at.test | RUN | 0.198712 | 0.986480 | 0.663934 | 0.001684 | 0.001541 | 0.001768 |
| mp_replace_at_c.test | RUN | 0.188040 | 0.968221 | 0.681939 | 0.001415 | 0.001615 | 0.001480 |
| mp_for_each.test | RUN | 0.185572 | 0.674199 | 0.701624 | 0.001616 | 0.001814 | 0.001521 |
| mp_insert.test | RUN | 0.250680 | 1.985370 | 0.971051 | 0.001745 | 0.001596 | 0.001821 |
| mp_insert_2.test | RUN | 0.026171 | 0.045402 | 0.073436 | 0.000523 | 0.000739 | 0.001489 |
| mp_erase.test | RUN | 0.281466 | 2.151321 | 1.165413 | 0.001865 | 0.001808 | 0.001580 |
| mp_erase_2.test | RUN | 0.026396 | 0.043830 | 0.068551 | 0.000695 | 0.000817 | 0.001782 |
| mp_with_index.test | RUN | 0.386114 | 4.380106 | 2.429584 | 0.002020 | 0.002252 | 0.002128 |
| mp_with_index_cx.test | RUN | 0.003319 | 0.016370 | 0.026128 | 0.000817 | 0.000764 | 0.001744 |
| mp_from_sequence.test | RUN | 0.193220 | 1.135390 | 0.657098 | 0.001803 | 0.001587 | 0.001530 |
| mp_from_sequence_2.test | RUN | 0.205836 | 1.351419 | 0.705628 | 0.001335 | 0.001734 | 0.001845 |
| mp_from_sequence_3.test | RUN | 0.174567 | 0.584249 | 0.467653 | 0.001568 | 0.001758 | 0.001752 |
| mp_min_element.test | RUN | 0.167741 | 0.526676 | 0.457181 | 0.001594 | 0.001460 | 0.001635 |
| mp_min_element_2.test | RUN | 0.026188 | 0.046460 | 0.071719 | 0.000563 | 0.000703 | 0.001788 |
| mp_min_element_q.test | RUN | 0.170814 | 0.562011 | 0.448060 | 0.001587 | 0.001735 | 0.001361 |
| mp_min_element_q_2.test | RUN | 0.026365 | 0.044361 | 0.070901 | 0.000761 | 0.000776 | 0.001822 |
| mp_max_element.test | RUN | 0.168988 | 0.572065 | 0.457775 | 0.001666 | 0.001506 | 0.001631 |
| mp_max_element_2.test | RUN | 0.025184 | 0.061988 | 0.067024 | 0.000771 | 0.000709 | 0.001728 |
| mp_max_element_q.test | RUN | 0.163589 | 0.540496 | 0.464417 | 0.001788 | 0.001764 | 0.001854 |
| mp_max_element_q_2.test | RUN | 0.026636 | 0.045616 | 0.067511 | 0.000753 | 0.000772 | 0.001763 |
| mp_nth_element.test | RUN | 0.181912 | 0.747562 | 0.558987 | 0.001723 | 0.001676 | 0.001441 |
| mp_nth_element_q.test | RUN | 0.182668 | 0.748839 | 0.573868 | 0.001514 | 0.001497 | 0.001633 |
| mp_back.test | RUN | 0.179713 | 0.602927 | 0.543299 | 0.001650 | 0.001600 | 0.001673 |
| mp_back_2.test | RUN | 0.026324 | 0.044652 | 0.069342 | 0.000558 | 0.000618 | 0.001371 |
| mp_pop_back.test | RUN | 0.388884 | 4.280558 | 1.159624 | 0.001740 | 0.001972 | 0.001808 |
| mp_pop_back_2.test | RUN | 0.026039 | 0.043230 | 0.066437 | 0.000809 | 0.000797 | 0.001740 |
| mp_flatten.test | RUN | 0.199527 | 1.441688 | 0.762622 | 0.001762 | 0.001724 | 0.001784 |
| mp_rotate_left.test | RUN | 0.198982 | 1.557735 | 0.771188 | 0.001744 | 0.001890 | 0.001793 |
| mp_rotate_right.test | RUN | 0.208258 | 1.568347 | 0.749116 | 0.001489 | 0.001559 | 0.001644 |
| mp_power_set.test | RUN | 0.197622 | 1.577906 | 0.748101 | 0.001770 | 0.002130 | 0.001625 |
| mp_partial_sum.test | RUN | 0.239283 | 2.156700 | 0.853928 | 0.001819 | 0.001967 | 0.001912 |
| mp_iterate.test | RUN | 0.183604 | 0.614909 | 0.592774 | 0.001540 | 0.001785 | 0.001637 |
| mp_pairwise_fold.test | RUN | 0.200621 | 1.577284 | 0.776278 | 0.001513 | 0.001888 | 0.001890 |
| mp_pairwise_fold_q.test | RUN | 0.162126 | 0.459262 | 0.441854 | 0.001800 | 0.001795 | 0.001858 |
| mp_sliding_fold.test | RUN | 0.328583 | 2.742168 | 1.527433 | 0.001791 | 0.001895 | 0.001735 |
| mp_sliding_fold_q.test | RUN | 0.321156 | 2.756926 | 1.529428 | 0.001655 | 0.001943 | 0.001587 |
| mp_intersperse.test | RUN | 0.202636 | 1.029250 | 0.707900 | 0.001764 | 0.001790 | 0.001844 |
| mp_split.test | RUN | 0.242519 | 1.948575 | 0.983559 | 0.001800 | 0.001820 | 0.001886 |
| mp_join.test | RUN | 0.234237 | 1.613484 | 0.850128 | 0.001462 | 0.001943 | 0.001732 |
| integral.test | RUN | 0.169751 | 0.750379 | 0.488981 | 0.001772 | 0.001840 | 0.001846 |
| mp_value.test | RUN | 0.012166 | 0.027171 | 0.041529 | 0.000642 | 0.000647 | 0.001779 |
| mp_identity.test | RUN | 0.160770 | 0.675099 | 0.503493 | 0.001804 | 0.001759 | 0.001874 |
| mp_inherit.test | RUN | 0.158233 | 0.481098 | 0.470270 | 0.001723 | 0.001728 | 0.001782 |
| mp_if.test | RUN | 0.157995 | 0.547308 | 0.452232 | 0.001823 | 0.001621 | 0.001852 |
| mp_if_sf.test | RUN | 0.149869 | 0.403334 | 0.395951 | 0.001723 | 0.001994 | 0.001836 |
| mp_eval_if.test | RUN | 0.161838 | 0.583879 | 0.467715 | 0.001799 | 0.001431 | 0.001772 |
| mp_eval_if_sf.test | RUN | 0.155423 | 0.409437 | 0.412410 | 0.001708 | 0.001746 | 0.001658 |
| mp_valid.test | RUN | 0.153360 | 0.419935 | 0.411177 | 0.001836 | 0.001798 | 0.001779 |
| mp_defer.test | RUN | 0.166137 | 0.709536 | 0.516909 | 0.001569 | 0.001719 | 0.001618 |
| mp_quote.test | RUN | 0.169785 | 0.714942 | 0.512956 | 0.001791 | 0.001956 | 0.001774 |
| mp_invoke_q.test | RUN | 0.179811 | 0.950828 | 0.568596 | 0.001803 | 0.001784 | 0.001532 |
| mp_invoke_q_sf.test | RUN | 0.150793 | 0.409410 | 0.412203 | 0.001686 | 0.001876 | 0.001756 |
| mp_quote_trait.test | RUN | 0.161645 | 0.823921 | 0.478163 | 0.001518 | 0.001834 | 0.001693 |
| mp_cond.test | RUN | 0.155838 | 0.513105 | 0.420870 | 0.001804 | 0.001965 | 0.001588 |
| mp_cond_sf.test | RUN | 0.153467 | 0.412067 | 0.398212 | 0.001651 | 0.001841 | 0.001561 |
| mp_not_fn.test | RUN | 0.157066 | 0.410608 | 0.383963 | 0.001534 | 0.001920 | 0.001499 |
| mp_eval_if_not.test | RUN | 0.158321 | 0.577911 | 0.439271 | 0.001843 | 0.001760 | 0.001808 |
| mp_eval_or.test | RUN | 0.155258 | 0.320933 | 0.389841 | 0.001572 | 0.001601 | 0.001401 |
| mp_compose.test | RUN | 0.169882 | 0.986886 | 0.546898 | 0.001716 | 0.001409 | 0.001803 |
| mp_valid_and_true.test | RUN | 0.158878 | 0.546726 | 0.426148 | 0.001640 | 0.001904 | 0.001757 |
| mp_compose_sf.test | COMPILE | 0.037468 | 0.080056 | 0.088812 | — | — | — |
| integer_sequence.test | RUN | 0.187329 | 0.958988 | 0.609838 | 0.001563 | 0.001790 | 0.001860 |
| tuple_for_each.test | RUN | 0.164068 | 0.357066 | 0.445243 | 0.001455 | 0.001711 | 0.001700 |
| tuple_for_each_cx.test | COMPILE | 0.031428 | 0.051402 | 0.082632 | — | — | — |
| tuple_apply.test | RUN | 0.166058 | 0.344130 | 0.459505 | 0.001736 | 0.001837 | 0.001764 |
| tuple_apply_cx.test | COMPILE | 0.043069 | 0.072442 | 0.111275 | — | — | — |
| construct_from_tuple.test | RUN | 0.161068 | 0.388708 | 0.439095 | 0.001561 | 0.001355 | 0.001394 |
| construct_from_tuple_cx.test | COMPILE | 0.044262 | 0.073334 | 0.103072 | — | — | — |
| tuple_transform.test | RUN | 0.254422 | 0.734485 | 0.892685 | 0.001871 | 0.001447 | 0.001317 |
| tuple_transform_2.test | RUN | 0.227506 | 0.551344 | 0.812316 | 0.001896 | 0.001416 | 0.001413 |
| tuple_transform_cx.test | COMPILE | 0.036496 | 0.067736 | 0.097000 | — | — | — |
| mp_set_contains.test | RUN | 0.160580 | 0.492490 | 0.464007 | 0.001908 | 0.001622 | 0.001576 |
| mp_set_push_back.test | RUN | 0.196716 | 1.558451 | 0.771952 | 0.001585 | 0.001911 | 0.001841 |
| mp_set_push_front.test | RUN | 0.244879 | 1.917668 | 0.857668 | 0.001636 | 0.001489 | 0.001772 |
| mp_is_set.test | RUN | 0.161728 | 0.514698 | 0.438010 | 0.001990 | 0.001870 | 0.001794 |
| mp_set_union.test | RUN | 0.194140 | 1.340623 | 0.719799 | 0.001810 | 0.001944 | 0.001718 |
| mp_set_union_sf.test | RUN | 0.155566 | 0.421685 | 0.426118 | 0.001698 | 0.001966 | 0.001369 |
| mp_set_difference.test | RUN | 0.186038 | 1.106552 | 0.614648 | 0.001684 | 0.001888 | 0.002012 |
| mp_set_difference_sf.test | RUN | 0.156066 | 0.466776 | 0.424676 | 0.001748 | 0.001803 | 0.001841 |
| mp_set_intersection.test | RUN | 0.178040 | 1.059375 | 0.613877 | 0.001632 | 0.001935 | 0.001634 |
| mp_set_intersection_sf.test | RUN | 0.157870 | 0.425076 | 0.412194 | 0.001645 | 0.001454 | 0.001854 |
| mp_all.test | RUN | 0.157420 | 0.524682 | 0.445764 | 0.001836 | 0.001637 | 0.001850 |
| mp_all_2.test | RUN | 0.426601 | 1.069186 | 0.976351 | 0.001749 | 0.002008 | 0.002436 |
| mp_and.test | RUN | 0.158013 | 0.469540 | 0.426680 | 0.001600 | 0.001799 | 0.001810 |
| mp_any.test | RUN | 0.154725 | 0.492073 | 0.419960 | 0.001630 | 0.001788 | 0.001738 |
| mp_any_2.test | RUN | 0.421387 | 1.062798 | 0.973153 | 0.001678 | 0.001837 | 0.001554 |
| mp_or.test | RUN | 0.157485 | 0.495958 | 0.412889 | 0.001676 | 0.001859 | 0.001750 |
| mp_same.test | RUN | 0.295485 | 0.814491 | 0.677076 | 0.001646 | 0.001821 | 0.001892 |
| mp_plus.test | RUN | 0.165918 | 0.587623 | 0.476619 | 0.001586 | 0.001703 | 0.001754 |
| mp_less.test | RUN | 0.185911 | 0.493196 | 0.432914 | 0.001551 | 0.001454 | 0.002018 |
| mp_min.test | RUN | 0.158831 | 0.448970 | 0.413506 | 0.001692 | 0.001790 | 0.001930 |
| mp_max.test | RUN | 0.158997 | 0.515392 | 0.463945 | 0.001503 | 0.001847 | 0.001660 |
| mp_similar.test | RUN | 0.157561 | 0.498625 | 0.456692 | 0.001497 | 0.001776 | 0.001567 |
| mp_map_find.test | RUN | 0.180597 | 0.913949 | 0.590346 | 0.001713 | 0.001674 | 0.001960 |
| mp_map_find_2.test | RUN | 0.173152 | 0.367124 | 0.503119 | 0.001814 | 0.001542 | 0.001912 |
| mp_map_find_3.test | RUN | 0.172346 | 0.339519 | 0.510960 | 0.001483 | 0.001629 | 0.001743 |
| mp_map_find_4.test | RUN | 0.164598 | 0.325648 | 0.475847 | 0.001481 | 0.001554 | 0.001709 |
| mp_map_contains.test | RUN | 0.165290 | 0.495851 | 0.456926 | 0.001518 | 0.001822 | 0.001871 |
| mp_map_insert.test | RUN | 0.238087 | 1.652625 | 0.875774 | 0.001625 | 0.001804 | 0.001720 |
| mp_map_replace.test | RUN | 0.254471 | 1.892125 | 1.009105 | 0.001514 | 0.001878 | 0.001621 |
| mp_map_erase.test | RUN | 0.199693 | 1.403275 | 0.818460 | 0.001708 | 0.001780 | 0.001612 |
| mp_map_update.test | RUN | 0.186961 | 0.907084 | 0.644744 | 0.001882 | 0.001730 | 0.001860 |
| mp_map_update_q.test | RUN | 0.180989 | 0.906257 | 0.621632 | 0.001808 | 0.001610 | 0.001974 |
| mp_map_keys.test | RUN | 0.178742 | 0.823337 | 0.614513 | 0.001656 | 0.001808 | 0.001647 |
| mp_is_map.test | RUN | 0.165917 | 0.501282 | 0.496726 | 0.001549 | 0.001582 | 0.001488 |
| mp_bind.test | RUN | 0.199051 | 1.510921 | 0.745857 | 0.001593 | 0.001585 | 0.001814 |
| mp_bind_q.test | RUN | 0.180085 | 0.907510 | 0.605667 | 0.001360 | 0.001710 | 0.001956 |
| mp_bind_front.test | RUN | 0.176516 | 0.965705 | 0.618315 | 0.001674 | 0.001569 | 0.001585 |
| mp_bind_back.test | RUN | 0.180944 | 0.994046 | 0.618559 | 0.001442 | 0.001524 | 0.001532 |
| mp_lambda.test | RUN | 0.340002 | 3.173481 | 1.633990 | 0.001571 | 0.001355 | 0.001670 |
| mpl.test | RUN | 0.332137 | 0.800781 | 1.247269 | 0.001712 | 0.001524 | 0.001555 |
| mpl_list.test | RUN | 0.211192 | 0.592893 | 0.578090 | 0.001606 | 0.001698 | 0.001539 |
| mpl_tuple.test | RUN | 0.217515 | 0.602056 | 0.572309 | 0.001614 | 0.001500 | 0.001870 |
| version.test | RUN | 0.112487 | 0.222920 | 0.277506 | 0.001606 | 0.001797 | 0.001835 |
| mp11_version.test | RUN | 0.131672 | 0.258659 | 0.349312 | 0.001391 | 0.001616 | 0.001902 |
| list_version.test | RUN | 0.112272 | 0.229823 | 0.319496 | 0.001199 | 0.001641 | 0.001703 |
| algorithm_version.test | RUN | 0.122145 | 0.238440 | 0.310079 | 0.001664 | 0.001712 | 0.001547 |
| integral_version.test | RUN | 0.104969 | 0.221311 | 0.287882 | 0.001883 | 0.001444 | 0.001233 |
| utility_version.test | RUN | 0.108179 | 0.235170 | 0.290798 | 0.001500 | 0.001518 | 0.001428 |
| function_version.test | RUN | 0.109094 | 0.231535 | 0.306120 | 0.001463 | 0.002032 | 0.001720 |
| map_version.test | RUN | 0.124605 | 0.259462 | 0.325013 | 0.001786 | 0.001603 | 0.001742 |
| set_version.test | RUN | 0.109575 | 0.241517 | 0.299125 | 0.001872 | 0.001478 | 0.001648 |
| bind_version.test | RUN | 0.117768 | 0.258855 | 0.299704 | 0.001512 | 0.001688 | 0.001329 |
| integer_sequence_version.test | RUN | 0.104686 | 0.228499 | 0.277784 | 0.001532 | 0.001656 | 0.001822 |
| tuple_version.test | RUN | 0.126250 | 0.250428 | 0.277753 | 0.001474 | 0.001317 | 0.001456 |
| mpl_version.test | RUN | 0.090458 | 0.180209 | 0.234576 | 0.001424 | 0.001409 | 0.001382 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.008× | 1.010×, 1.006× | 0.987× |
| clang | 1.078× | 1.059×, 1.096× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000933s | 0.000932s | 0.000987s | byte equality observed |
| bind_version.test | 10 | 0.000921s | 0.000950s | 0.000940s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000929s | 0.000965s | 0.000954s | byte equality observed |
| function_version.test | 10 | 0.000932s | 0.000975s | 0.000975s | byte equality observed |
| integer_sequence.test | 10 | 0.000933s | 0.001005s | 0.000962s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000956s | 0.000984s | 0.000967s | byte equality observed |
| integral.test | 10 | 0.000947s | 0.000996s | 0.000960s | byte equality observed |
| integral_version.test | 10 | 0.000929s | 0.001000s | 0.000948s | byte equality observed |
| list_version.test | 10 | 0.000940s | 0.000972s | 0.000978s | byte equality observed |
| map_version.test | 10 | 0.000955s | 0.000991s | 0.000962s | byte equality observed |
| mp11_version.test | 10 | 0.000953s | 0.000982s | 0.000961s | byte equality observed |
| mp_all.test | 10 | 0.000960s | 0.000981s | 0.000960s | byte equality observed |
| mp_all_2.test | 10 | 0.000958s | 0.000977s | 0.000964s | byte equality observed |
| mp_all_of.test | 10 | 0.000966s | 0.000970s | 0.000964s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000953s | 0.000955s | 0.000978s | byte equality observed |
| mp_and.test | 10 | 0.000963s | 0.000963s | 0.000970s | byte equality observed |
| mp_any.test | 10 | 0.000971s | 0.000980s | 0.000992s | byte equality observed |
| mp_any_2.test | 10 | 0.000952s | 0.000972s | 0.000972s | byte equality observed |
| mp_any_of.test | 10 | 0.000994s | 0.000966s | 0.001001s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000974s | 0.000970s | 0.000957s | byte equality observed |
| mp_append.test | 10 | 0.000986s | 0.001013s | 0.000996s | byte equality observed |
| mp_append_2.test | 10 | 0.000996s | 0.000981s | 0.000985s | byte equality observed |
| mp_append_3.test | 10 | 0.000445s | 0.000452s | 0.000973s | byte equality observed |
| mp_append_sf.test | 10 | 0.000983s | 0.000959s | 0.000986s | byte equality observed |
| mp_apply_q.test | 10 | 0.000995s | 0.000987s | 0.001002s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.000947s | 0.000950s | 0.000953s | byte equality observed |
| mp_assign.test | 10 | 0.000986s | 0.000957s | 0.000968s | byte equality observed |
| mp_assign_2.test | 10 | 0.000456s | 0.000452s | 0.000941s | byte equality observed |
| mp_at.test | 10 | 0.000988s | 0.000996s | 0.000964s | byte equality observed |
| mp_at_2.test | 10 | 0.000452s | 0.000459s | 0.000969s | byte equality observed |
| mp_at_sf.test | 10 | 0.000967s | 0.000983s | 0.000970s | byte equality observed |
| mp_back.test | 10 | 0.000950s | 0.000969s | 0.000974s | byte equality observed |
| mp_back_2.test | 10 | 0.000454s | 0.000452s | 0.000931s | byte equality observed |
| mp_bind.test | 10 | 0.000982s | 0.000965s | 0.000968s | byte equality observed |
| mp_bind_back.test | 10 | 0.000951s | 0.000970s | 0.000967s | byte equality observed |
| mp_bind_front.test | 10 | 0.000969s | 0.000991s | 0.000951s | byte equality observed |
| mp_bind_q.test | 10 | 0.000957s | 0.000960s | 0.000974s | byte equality observed |
| mp_clear.test | 10 | 0.000968s | 0.000990s | 0.000964s | byte equality observed |
| mp_clear_2.test | 10 | 0.000448s | 0.000454s | 0.000946s | byte equality observed |
| mp_compose.test | 10 | 0.000945s | 0.001009s | 0.000960s | byte equality observed |
| mp_cond.test | 10 | 0.000946s | 0.000973s | 0.000995s | byte equality observed |
| mp_cond_sf.test | 10 | 0.000952s | 0.000967s | 0.000974s | byte equality observed |
| mp_contains.test | 10 | 0.000936s | 0.000966s | 0.000956s | byte equality observed |
| mp_copy_if.test | 10 | 0.000939s | 0.000977s | 0.000972s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000962s | 0.001001s | 0.000958s | byte equality observed |
| mp_count.test | 10 | 0.000950s | 0.000972s | 0.000947s | byte equality observed |
| mp_count_if.test | 10 | 0.000956s | 0.000963s | 0.000969s | byte equality observed |
| mp_count_if_q.test | 10 | 0.000965s | 0.000983s | 0.000951s | byte equality observed |
| mp_defer.test | 10 | 0.000966s | 0.000986s | 0.000990s | byte equality observed |
| mp_drop.test | 10 | 0.000981s | 0.000976s | 0.000945s | byte equality observed |
| mp_drop_2.test | 10 | 0.000449s | 0.000452s | 0.000935s | byte equality observed |
| mp_empty.test | 10 | 0.000986s | 0.000978s | 0.000968s | byte equality observed |
| mp_empty_2.test | 10 | 0.000451s | 0.000452s | 0.000923s | byte equality observed |
| mp_erase.test | 10 | 0.001016s | 0.000990s | 0.000952s | byte equality observed |
| mp_erase_2.test | 10 | 0.000451s | 0.000448s | 0.000959s | byte equality observed |
| mp_eval_if.test | 10 | 0.000952s | 0.000968s | 0.000966s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.000965s | 0.000977s | 0.000957s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.000953s | 0.000964s | 0.000975s | byte equality observed |
| mp_eval_or.test | 10 | 0.000951s | 0.000952s | 0.000962s | byte equality observed |
| mp_fill.test | 10 | 0.000976s | 0.000975s | 0.000954s | byte equality observed |
| mp_fill_2.test | 10 | 0.000450s | 0.000445s | 0.000930s | byte equality observed |
| mp_filter.test | 10 | 0.000964s | 0.000972s | 0.000965s | byte equality observed |
| mp_find.test | 10 | 0.000982s | 0.000961s | 0.000989s | byte equality observed |
| mp_find_if.test | 10 | 0.000971s | 0.000957s | 0.000969s | byte equality observed |
| mp_find_if_q.test | 10 | 0.000952s | 0.001004s | 0.000968s | byte equality observed |
| mp_flatten.test | 10 | 0.000961s | 0.000975s | 0.001018s | byte equality observed |
| mp_fold.test | 10 | 0.000992s | 0.000954s | 0.000966s | byte equality observed |
| mp_fold_q.test | 10 | 0.000959s | 0.000983s | 0.001001s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.000939s | 0.000990s | 0.000965s | byte equality observed |
| mp_for_each.test | 10 | 0.000979s | 0.000948s | 0.000972s | byte equality observed |
| mp_from_sequence.test | 10 | 0.000971s | 0.000975s | 0.001007s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.000961s | 0.000975s | 0.000969s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.000954s | 0.000955s | 0.000978s | byte equality observed |
| mp_front.test | 10 | 0.000971s | 0.000994s | 0.000995s | byte equality observed |
| mp_front_2.test | 10 | 0.000450s | 0.000449s | 0.000959s | byte equality observed |
| mp_identity.test | 10 | 0.000971s | 0.000972s | 0.000983s | byte equality observed |
| mp_if.test | 10 | 0.000968s | 0.000950s | 0.000965s | byte equality observed |
| mp_if_sf.test | 10 | 0.000946s | 0.000950s | 0.000963s | byte equality observed |
| mp_inherit.test | 10 | 0.000955s | 0.000973s | 0.000964s | byte equality observed |
| mp_insert.test | 10 | 0.000991s | 0.000988s | 0.000955s | byte equality observed |
| mp_insert_2.test | 10 | 0.000446s | 0.000440s | 0.000960s | byte equality observed |
| mp_intersperse.test | 10 | 0.000970s | 0.000958s | 0.000952s | byte equality observed |
| mp_invoke_q.test | 10 | 0.000956s | 0.000978s | 0.000952s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.000948s | 0.000968s | 0.000954s | byte equality observed |
| mp_iota.test | 10 | 0.000947s | 0.000965s | 0.000963s | byte equality observed |
| mp_iota_2.test | 10 | 0.000937s | 0.001011s | 0.000948s | byte equality observed |
| mp_is_list.test | 10 | 0.000961s | 0.000982s | 0.000949s | byte equality observed |
| mp_is_map.test | 10 | 0.000955s | 0.000965s | 0.000950s | byte equality observed |
| mp_is_set.test | 10 | 0.000962s | 0.000978s | 0.000945s | byte equality observed |
| mp_is_value_list.test | 10 | 0.000944s | 0.000965s | 0.000956s | byte equality observed |
| mp_iterate.test | 10 | 0.000942s | 0.000964s | 0.000950s | byte equality observed |
| mp_join.test | 10 | 0.000963s | 0.000975s | 0.000965s | byte equality observed |
| mp_lambda.test | 10 | 0.000972s | 0.000979s | 0.000955s | byte equality observed |
| mp_less.test | 10 | 0.000947s | 0.000979s | 0.000959s | byte equality observed |
| mp_list_c.test | 10 | 0.000955s | 0.001003s | 0.000955s | byte equality observed |
| mp_list_v.test | 10 | 0.000443s | 0.000441s | 0.000935s | byte equality observed |
| mp_map_contains.test | 10 | 0.000953s | 0.000969s | 0.000985s | byte equality observed |
| mp_map_erase.test | 10 | 0.000955s | 0.000964s | 0.000966s | byte equality observed |
| mp_map_find.test | 10 | 0.000953s | 0.000950s | 0.000961s | byte equality observed |
| mp_map_find_2.test | 10 | 0.000960s | 0.000964s | 0.000985s | byte equality observed |
| mp_map_find_3.test | 10 | 0.000944s | 0.000983s | 0.000992s | byte equality observed |
| mp_map_find_4.test | 10 | 0.000960s | 0.000981s | 0.000971s | byte equality observed |
| mp_map_insert.test | 10 | 0.000971s | 0.000994s | 0.001001s | byte equality observed |
| mp_map_keys.test | 10 | 0.000942s | 0.000977s | 0.000976s | byte equality observed |
| mp_map_replace.test | 10 | 0.000971s | 0.000975s | 0.000974s | byte equality observed |
| mp_map_update.test | 10 | 0.000952s | 0.000972s | 0.000984s | byte equality observed |
| mp_map_update_q.test | 10 | 0.000962s | 0.000963s | 0.000984s | byte equality observed |
| mp_max.test | 10 | 0.000956s | 0.000959s | 0.000980s | byte equality observed |
| mp_max_element.test | 10 | 0.000963s | 0.001001s | 0.000965s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000449s | 0.000449s | 0.000962s | byte equality observed |
| mp_max_element_q.test | 10 | 0.000970s | 0.000988s | 0.000975s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000451s | 0.000460s | 0.000955s | byte equality observed |
| mp_min.test | 10 | 0.000985s | 0.000961s | 0.000986s | byte equality observed |
| mp_min_element.test | 10 | 0.000964s | 0.000994s | 0.000975s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000454s | 0.000445s | 0.000954s | byte equality observed |
| mp_min_element_q.test | 10 | 0.000990s | 0.000981s | 0.000975s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000461s | 0.000445s | 0.000964s | byte equality observed |
| mp_none_of.test | 10 | 0.000962s | 0.000986s | 0.000967s | byte equality observed |
| mp_none_of_q.test | 10 | 0.000970s | 0.000979s | 0.000983s | byte equality observed |
| mp_not_fn.test | 10 | 0.000953s | 0.000989s | 0.000978s | byte equality observed |
| mp_nth_element.test | 10 | 0.000973s | 0.000972s | 0.000979s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000965s | 0.000970s | 0.000967s | byte equality observed |
| mp_or.test | 10 | 0.000943s | 0.000970s | 0.000974s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.000942s | 0.000979s | 0.000963s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.000963s | 0.000976s | 0.000964s | byte equality observed |
| mp_partial_sum.test | 10 | 0.000970s | 0.000982s | 0.000952s | byte equality observed |
| mp_partition.test | 10 | 0.000965s | 0.000981s | 0.000975s | byte equality observed |
| mp_partition_q.test | 10 | 0.000959s | 0.000964s | 0.000974s | byte equality observed |
| mp_plus.test | 10 | 0.000962s | 0.000965s | 0.000963s | byte equality observed |
| mp_pop_back.test | 10 | 0.000990s | 0.001017s | 0.000956s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000439s | 0.000445s | 0.000939s | byte equality observed |
| mp_pop_front.test | 10 | 0.000986s | 0.000983s | 0.000939s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000449s | 0.000454s | 0.000943s | byte equality observed |
| mp_power_set.test | 10 | 0.000943s | 0.000971s | 0.000961s | byte equality observed |
| mp_product.test | 10 | 0.000957s | 0.000993s | 0.000933s | byte equality observed |
| mp_push_back.test | 10 | 0.000999s | 0.000993s | 0.000943s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000457s | 0.000448s | 0.000940s | byte equality observed |
| mp_push_front.test | 10 | 0.000997s | 0.000986s | 0.000979s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000446s | 0.000448s | 0.000960s | byte equality observed |
| mp_quote.test | 10 | 0.000977s | 0.000968s | 0.000971s | byte equality observed |
| mp_quote_trait.test | 10 | 0.000942s | 0.000982s | 0.000973s | byte equality observed |
| mp_remove.test | 10 | 0.000963s | 0.000992s | 0.000952s | byte equality observed |
| mp_remove_if.test | 10 | 0.000964s | 0.000968s | 0.000989s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.000960s | 0.000979s | 0.001012s | byte equality observed |
| mp_rename.test | 10 | 0.000951s | 0.000989s | 0.000970s | byte equality observed |
| mp_rename_2.test | 10 | 0.000448s | 0.000463s | 0.000941s | byte equality observed |
| mp_rename_v.test | 10 | 0.000447s | 0.000461s | 0.000968s | byte equality observed |
| mp_repeat.test | 10 | 0.001000s | 0.001026s | 0.000978s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000452s | 0.000449s | 0.000946s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000968s | 0.000977s | 0.001007s | byte equality observed |
| mp_replace.test | 10 | 0.000958s | 0.000981s | 0.000960s | byte equality observed |
| mp_replace_at.test | 10 | 0.000961s | 0.000977s | 0.000963s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000974s | 0.000976s | 0.000954s | byte equality observed |
| mp_replace_front.test | 10 | 0.000971s | 0.001000s | 0.000968s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000442s | 0.000446s | 0.000936s | byte equality observed |
| mp_replace_if.test | 10 | 0.000959s | 0.000980s | 0.000942s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.000954s | 0.000979s | 0.000944s | byte equality observed |
| mp_replace_second.test | 10 | 0.000959s | 0.000982s | 0.000948s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000441s | 0.000463s | 0.000940s | byte equality observed |
| mp_replace_third.test | 10 | 0.000974s | 0.000992s | 0.000974s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000451s | 0.000443s | 0.000938s | byte equality observed |
| mp_reverse.test | 10 | 0.000976s | 0.000979s | 0.000960s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.000974s | 0.000981s | 0.000969s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.000944s | 0.000972s | 0.000981s | byte equality observed |
| mp_rotate_left.test | 10 | 0.000950s | 0.000968s | 0.000953s | byte equality observed |
| mp_rotate_right.test | 10 | 0.000972s | 0.000990s | 0.000971s | byte equality observed |
| mp_same.test | 10 | 0.000965s | 0.000961s | 0.000956s | byte equality observed |
| mp_second.test | 10 | 0.000959s | 0.000990s | 0.000977s | byte equality observed |
| mp_second_2.test | 10 | 0.000457s | 0.000457s | 0.000926s | byte equality observed |
| mp_set_contains.test | 10 | 0.000961s | 0.000969s | 0.000942s | byte equality observed |
| mp_set_difference.test | 10 | 0.000959s | 0.000969s | 0.000948s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.000970s | 0.000978s | 0.000946s | byte equality observed |
| mp_set_intersection.test | 10 | 0.000967s | 0.000985s | 0.000975s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.000954s | 0.000987s | 0.000969s | byte equality observed |
| mp_set_push_back.test | 10 | 0.000954s | 0.000986s | 0.000940s | byte equality observed |
| mp_set_push_front.test | 10 | 0.000993s | 0.000965s | 0.000962s | byte equality observed |
| mp_set_union.test | 10 | 0.000948s | 0.000973s | 0.000969s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.000963s | 0.000974s | 0.000964s | byte equality observed |
| mp_similar.test | 10 | 0.000972s | 0.000964s | 0.000957s | byte equality observed |
| mp_size.test | 10 | 0.000963s | 0.000996s | 0.000972s | byte equality observed |
| mp_size_2.test | 10 | 0.000452s | 0.000444s | 0.000942s | byte equality observed |
| mp_slice.test | 10 | 0.000989s | 0.000995s | 0.000964s | byte equality observed |
| mp_slice_2.test | 10 | 0.000443s | 0.000449s | 0.000951s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.000993s | 0.000967s | 0.000966s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.000991s | 0.000989s | 0.000969s | byte equality observed |
| mp_sort.test | 10 | 0.000955s | 0.000984s | 0.000945s | byte equality observed |
| mp_sort_q.test | 10 | 0.000958s | 0.000995s | 0.000963s | byte equality observed |
| mp_split.test | 10 | 0.000976s | 0.000970s | 0.000947s | byte equality observed |
| mp_starts_with.test | 10 | 0.000957s | 0.000983s | 0.000953s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.000954s | 0.000983s | 0.000954s | byte equality observed |
| mp_take.test | 10 | 0.000984s | 0.000997s | 0.000951s | byte equality observed |
| mp_take_2.test | 10 | 0.000447s | 0.000452s | 0.000930s | byte equality observed |
| mp_third.test | 10 | 0.000969s | 0.000997s | 0.000957s | byte equality observed |
| mp_third_2.test | 10 | 0.000451s | 0.000450s | 0.000948s | byte equality observed |
| mp_transform.test | 10 | 0.000996s | 0.000981s | 0.000957s | byte equality observed |
| mp_transform_2.test | 10 | 0.000937s | 0.000961s | 0.000954s | byte equality observed |
| mp_transform_front.test | 10 | 0.000965s | 0.000986s | 0.000957s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000446s | 0.000453s | 0.000924s | byte equality observed |
| mp_transform_if.test | 10 | 0.000954s | 0.000999s | 0.000960s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.000963s | 0.000974s | 0.000951s | byte equality observed |
| mp_transform_q.test | 10 | 0.000974s | 0.000991s | 0.000948s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.000957s | 0.000972s | 0.000972s | byte equality observed |
| mp_transform_second.test | 10 | 0.000948s | 0.000993s | 0.000943s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000447s | 0.000456s | 0.000939s | byte equality observed |
| mp_transform_sf.test | 10 | 0.000943s | 0.000966s | 0.000946s | byte equality observed |
| mp_transform_third.test | 10 | 0.000945s | 0.000988s | 0.000970s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000448s | 0.000441s | 0.000940s | byte equality observed |
| mp_unique.test | 10 | 0.000956s | 0.000983s | 0.000958s | byte equality observed |
| mp_unique_if.test | 10 | 0.000991s | 0.000984s | 0.000963s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.000991s | 0.001015s | 0.000967s | byte equality observed |
| mp_valid.test | 10 | 0.000939s | 0.000965s | 0.000953s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.000968s | 0.000991s | 0.000959s | byte equality observed |
| mp_value.test | 10 | 0.000443s | 0.000451s | 0.000939s | byte equality observed |
| mp_with_index.test | 10 | 0.001396s | 0.001398s | 0.001294s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000449s | 0.000454s | 0.000944s | byte equality observed |
| mpl.test | 10 | 0.000978s | 0.000976s | 0.000964s | byte equality observed |
| mpl_list.test | 10 | 0.000944s | 0.000979s | 0.000971s | byte equality observed |
| mpl_tuple.test | 10 | 0.000969s | 0.000971s | 0.000990s | byte equality observed |
| mpl_version.test | 10 | 0.000946s | 0.000970s | 0.000926s | byte equality observed |
| set_version.test | 10 | 0.000968s | 0.000960s | 0.000964s | byte equality observed |
| tuple_apply.test | 10 | 0.000972s | 0.000970s | 0.000971s | byte equality observed |
| tuple_for_each.test | 10 | 0.000966s | 0.000967s | 0.000945s | byte equality observed |
| tuple_transform.test | 10 | 0.000959s | 0.000963s | 0.000946s | byte equality observed |
| tuple_transform_2.test | 10 | 0.000951s | 0.000948s | 0.000937s | byte equality observed |
| tuple_version.test | 10 | 0.000966s | 0.000948s | 0.000954s | byte equality observed |
| utility_version.test | 10 | 0.000952s | 0.000945s | 0.000950s | byte equality observed |
| version.test | 10 | 0.000961s | 0.000951s | 0.000953s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
