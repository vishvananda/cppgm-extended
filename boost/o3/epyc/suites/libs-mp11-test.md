# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.037982 | 0.069349 | 0.112352 | — | — | — |
| complex_h.test | COMPILE | 0.147824 | 0.308579 | 0.499704 | — | — | — |
| mp_size.test | RUN | 0.171262 | 0.946006 | 0.589290 | 0.001600 | 0.001629 | 0.001643 |
| mp_size_2.test | RUN | 0.013690 | 0.033953 | 0.052576 | 0.000640 | 0.000810 | 0.001355 |
| mp_empty.test | RUN | 0.163130 | 0.603641 | 0.576889 | 0.001584 | 0.001720 | 0.001618 |
| mp_empty_2.test | RUN | 0.016246 | 0.038509 | 0.059132 | 0.000611 | 0.000693 | 0.001688 |
| mp_assign.test | RUN | 0.186085 | 1.239369 | 0.757613 | 0.001482 | 0.001607 | 0.001415 |
| mp_assign_2.test | RUN | 0.016181 | 0.039168 | 0.052152 | 0.000678 | 0.000828 | 0.001440 |
| mp_clear.test | RUN | 0.160466 | 0.558857 | 0.513950 | 0.001747 | 0.001826 | 0.001653 |
| mp_clear_2.test | RUN | 0.017008 | 0.034763 | 0.058843 | 0.000691 | 0.000628 | 0.001648 |
| mp_front.test | RUN | 0.177974 | 0.861833 | 0.637105 | 0.001624 | 0.001802 | 0.001443 |
| mp_front_2.test | RUN | 0.015043 | 0.035171 | 0.057813 | 0.000620 | 0.000699 | 0.001570 |
| mp_pop_front.test | RUN | 0.168049 | 0.898920 | 0.620953 | 0.001482 | 0.001599 | 0.001629 |
| mp_pop_front_2.test | RUN | 0.016313 | 0.037201 | 0.057172 | 0.000666 | 0.000671 | 0.001682 |
| mp_second.test | RUN | 0.167868 | 0.572391 | 0.560835 | 0.001576 | 0.001704 | 0.001566 |
| mp_second_2.test | RUN | 0.015321 | 0.036761 | 0.058889 | 0.000750 | 0.000802 | 0.001599 |
| mp_third.test | RUN | 0.162208 | 0.536754 | 0.555484 | 0.001559 | 0.001748 | 0.001585 |
| mp_third_2.test | RUN | 0.015632 | 0.036857 | 0.050316 | 0.000744 | 0.000634 | 0.001461 |
| mp_push_front.test | RUN | 0.212001 | 1.880264 | 1.014299 | 0.001594 | 0.001531 | 0.001524 |
| mp_push_front_2.test | RUN | 0.016122 | 0.037630 | 0.056549 | 0.000642 | 0.000787 | 0.001552 |
| mp_push_back.test | RUN | 0.212812 | 1.883496 | 0.951788 | 0.001583 | 0.001545 | 0.001604 |
| mp_push_back_2.test | RUN | 0.015651 | 0.038513 | 0.058783 | 0.000754 | 0.000782 | 0.001808 |
| mp_rename.test | RUN | 0.206122 | 1.744234 | 0.893994 | 0.001598 | 0.001807 | 0.001706 |
| mp_rename_2.test | RUN | 0.016369 | 0.038107 | 0.057177 | 0.000633 | 0.000800 | 0.001703 |
| mp_append.test | RUN | 0.215165 | 1.961193 | 0.954300 | 0.001596 | 0.001858 | 0.001768 |
| mp_append_2.test | RUN | 0.444008 | 4.366949 | 1.272820 | 0.001670 | 0.001864 | 0.001538 |
| mp_append_3.test | RUN | 0.016382 | 0.036557 | 0.050765 | 0.000686 | 0.000711 | 0.001708 |
| mp_append_sf.test | RUN | 0.154835 | 0.486387 | 0.562053 | 0.001729 | 0.001789 | 0.001664 |
| mp_replace_front.test | RUN | 0.189195 | 1.413618 | 0.756999 | 0.001576 | 0.001503 | 0.001653 |
| mp_replace_front_2.test | RUN | 0.016225 | 0.036859 | 0.059054 | 0.000582 | 0.000757 | 0.001734 |
| mp_replace_second.test | RUN | 0.180024 | 0.904899 | 0.679633 | 0.001708 | 0.001614 | 0.001700 |
| mp_replace_second_2.test | RUN | 0.016169 | 0.037028 | 0.050825 | 0.000560 | 0.000746 | 0.001527 |
| mp_replace_third.test | RUN | 0.177159 | 0.752231 | 0.680442 | 0.001685 | 0.001771 | 0.001692 |
| mp_replace_third_2.test | RUN | 0.015573 | 0.034926 | 0.058692 | 0.000618 | 0.000642 | 0.001705 |
| mp_apply_q.test | RUN | 0.197846 | 1.706093 | 0.872599 | 0.001570 | 0.001621 | 0.001378 |
| mp_apply_q_sf.test | RUN | 0.162986 | 0.515425 | 0.544839 | 0.001976 | 0.001716 | 0.001595 |
| mp_is_list.test | RUN | 0.163173 | 0.594395 | 0.569975 | 0.001728 | 0.001843 | 0.001664 |
| mp_list_c.test | RUN | 0.173547 | 0.722802 | 0.584503 | 0.001677 | 0.001594 | 0.001438 |
| mp_transform_front.test | RUN | 0.193327 | 1.398785 | 0.779876 | 0.001573 | 0.001744 | 0.001732 |
| mp_transform_front_2.test | RUN | 0.015553 | 0.035056 | 0.057765 | 0.000641 | 0.000660 | 0.001667 |
| mp_transform_second.test | RUN | 0.178449 | 1.184938 | 0.725204 | 0.001756 | 0.001484 | 0.001515 |
| mp_transform_second_2.test | RUN | 0.015886 | 0.036715 | 0.049468 | 0.000572 | 0.000739 | 0.001378 |
| mp_transform_third.test | RUN | 0.178273 | 0.906106 | 0.603483 | 0.001664 | 0.001876 | 0.001595 |
| mp_transform_third_2.test | RUN | 0.016394 | 0.037804 | 0.058550 | 0.000586 | 0.000681 | 0.001769 |
| mp_list_v.test | RUN | 0.016525 | 0.037032 | 0.060496 | 0.000664 | 0.000680 | 0.001456 |
| mp_rename_v.test | RUN | 0.016185 | 0.036503 | 0.050073 | 0.000744 | 0.000656 | 0.001484 |
| mp_is_value_list.test | RUN | 0.165891 | 0.375857 | 0.534925 | 0.001861 | 0.001726 | 0.001697 |
| mp_transform.test | RUN | 0.365854 | 2.931953 | 1.568648 | 0.001628 | 0.001848 | 0.001874 |
| mp_transform_2.test | RUN | 0.155823 | 0.334905 | 0.528045 | 0.001751 | 0.001800 | 0.001729 |
| mp_transform_q.test | RUN | 0.345365 | 2.805293 | 1.528305 | 0.001648 | 0.001544 | 0.001790 |
| mp_transform_q_2.test | RUN | 0.402715 | 0.904637 | 2.026417 | 0.001701 | 0.001770 | 0.001530 |
| mp_transform_sf.test | RUN | 0.169187 | 0.536911 | 0.589237 | 0.001717 | 0.001536 | 0.001624 |
| mp_transform_if.test | RUN | 0.201282 | 1.423783 | 0.883633 | 0.001826 | 0.001648 | 0.001665 |
| mp_transform_if_q.test | RUN | 0.213313 | 1.432269 | 0.890799 | 0.001548 | 0.001640 | 0.001721 |
| mp_filter.test | RUN | 0.186853 | 0.850186 | 0.684320 | 0.001724 | 0.001512 | 0.001349 |
| mp_fill.test | RUN | 0.175650 | 0.656707 | 0.635064 | 0.001769 | 0.001836 | 0.001685 |
| mp_fill_2.test | RUN | 0.027322 | 0.056207 | 0.080372 | 0.000585 | 0.000768 | 0.001622 |
| mp_count.test | RUN | 0.344546 | 1.497377 | 0.981434 | 0.001698 | 0.001842 | 0.001805 |
| mp_count_if.test | RUN | 0.363280 | 1.512706 | 1.151963 | 0.001499 | 0.001926 | 0.001684 |
| mp_count_if_q.test | RUN | 0.188410 | 1.006783 | 0.619546 | 0.001779 | 0.001700 | 0.002175 |
| mp_contains.test | RUN | 0.176889 | 0.610576 | 0.598847 | 0.001429 | 0.001707 | 0.001682 |
| mp_starts_with.test | RUN | 0.171642 | 0.604322 | 0.595993 | 0.001794 | 0.001806 | 0.001469 |
| mp_starts_with_sf.test | RUN | 0.166798 | 0.516137 | 0.568542 | 0.001395 | 0.001465 | 0.001708 |
| mp_repeat.test | RUN | 0.252650 | 2.498557 | 1.060112 | 0.001687 | 0.001932 | 0.001502 |
| mp_repeat_2.test | RUN | 0.027591 | 0.060850 | 0.098752 | 0.000826 | 0.000742 | 0.001562 |
| mp_repeat_3.test | RUN | 0.228976 | 0.358020 | 0.572412 | 0.001608 | 0.001783 | 0.001621 |
| mp_product.test | RUN | 0.210947 | 1.522946 | 0.830246 | 0.001592 | 0.001820 | 0.001878 |
| mp_drop.test | RUN | 0.218316 | 1.868525 | 0.937966 | 0.001368 | 0.001957 | 0.001826 |
| mp_drop_2.test | RUN | 0.028136 | 0.057069 | 0.089414 | 0.000743 | 0.000742 | 0.001425 |
| mp_iota.test | RUN | 0.207884 | 1.614327 | 0.825843 | 0.001492 | 0.001918 | 0.001447 |
| mp_iota_2.test | RUN | 0.220801 | 1.937329 | 0.875015 | 0.001602 | 0.001843 | 0.001714 |
| mp_at.test | RUN | 0.183758 | 0.870733 | 0.676448 | 0.001582 | 0.001757 | 0.001724 |
| mp_at_2.test | RUN | 0.027590 | 0.060088 | 0.095382 | 0.000777 | 0.000614 | 0.001706 |
| mp_at_sf.test | RUN | 0.173245 | 0.528600 | 0.573109 | 0.001968 | 0.001828 | 0.001613 |
| mp_take.test | RUN | 0.286297 | 2.469374 | 1.088907 | 0.001693 | 0.001458 | 0.001808 |
| mp_take_2.test | RUN | 0.028244 | 0.055295 | 0.099448 | 0.000611 | 0.000739 | 0.001590 |
| mp_slice.test | RUN | 0.321138 | 2.485591 | 1.401260 | 0.001611 | 0.001775 | 0.001716 |
| mp_slice_2.test | RUN | 0.027934 | 0.058318 | 0.085648 | 0.000622 | 0.000779 | 0.001658 |
| mp_replace.test | RUN | 0.213836 | 1.275146 | 0.881271 | 0.001574 | 0.001912 | 0.001638 |
| mp_replace_if.test | RUN | 0.203042 | 1.131296 | 0.830641 | 0.001750 | 0.001713 | 0.001683 |
| mp_replace_if_q.test | RUN | 0.205402 | 1.138253 | 0.826387 | 0.001630 | 0.001810 | 0.001678 |
| mp_copy_if.test | RUN | 0.194322 | 1.552415 | 0.787281 | 0.001496 | 0.001824 | 0.002517 |
| mp_copy_if_q.test | RUN | 0.199772 | 1.568146 | 0.784132 | 0.001517 | 0.001491 | 0.001430 |
| mp_remove.test | RUN | 0.203322 | 1.073555 | 0.816249 | 0.001686 | 0.001838 | 0.001743 |
| mp_remove_if.test | RUN | 0.204938 | 1.668831 | 0.875124 | 0.001755 | 0.001801 | 0.001828 |
| mp_remove_if_q.test | RUN | 0.210385 | 1.642650 | 0.872512 | 0.001478 | 0.001767 | 0.001768 |
| mp_partition.test | RUN | 0.210918 | 1.539754 | 0.903341 | 0.001617 | 0.001622 | 0.001722 |
| mp_partition_q.test | RUN | 0.208567 | 1.559179 | 0.907727 | 0.001809 | 0.001926 | 0.001640 |
| mp_sort.test | RUN | 0.204515 | 1.134289 | 0.784245 | 0.001371 | 0.001798 | 0.001786 |
| mp_sort_q.test | RUN | 0.202723 | 1.125423 | 0.783508 | 0.001540 | 0.001771 | 0.001729 |
| mp_find.test | RUN | 0.192244 | 1.208511 | 0.676497 | 0.001711 | 0.001663 | 0.001571 |
| mp_find_if.test | RUN | 0.188710 | 1.109560 | 0.666144 | 0.001783 | 0.001614 | 0.001301 |
| mp_find_if_q.test | RUN | 0.205497 | 1.256267 | 0.820272 | 0.001796 | 0.001961 | 0.001670 |
| mp_reverse.test | RUN | 0.394758 | 3.246616 | 1.528528 | 0.001766 | 0.001836 | 0.001765 |
| mp_fold.test | RUN | 0.660881 | 2.571359 | 3.845622 | 0.001772 | 0.001799 | 0.001738 |
| mp_fold_q.test | RUN | 0.193766 | 1.136088 | 0.782323 | 0.001480 | 0.001870 | 0.001857 |
| mp_fold_q_sf.test | RUN | 0.180988 | 0.531920 | 0.624679 | 0.001542 | 0.001889 | 0.001696 |
| mp_reverse_fold.test | RUN | 0.327277 | 2.423872 | 1.103212 | 0.001597 | 0.001708 | 0.001678 |
| mp_reverse_fold_q.test | RUN | 0.200869 | 1.358630 | 0.819165 | 0.001616 | 0.001668 | 0.001772 |
| mp_unique.test | RUN | 0.192760 | 1.227432 | 0.788164 | 0.001451 | 0.001808 | 0.001600 |
| mp_unique_if.test | RUN | 0.341430 | 2.522362 | 1.258752 | 0.001816 | 0.001807 | 0.001526 |
| mp_unique_if_q.test | RUN | 0.294398 | 2.523654 | 1.055261 | 0.001674 | 0.001743 | 0.001728 |
| mp_all_of.test | RUN | 0.465530 | 1.390856 | 1.330942 | 0.001818 | 0.001782 | 0.001787 |
| mp_all_of_q.test | RUN | 0.169049 | 0.609142 | 0.580518 | 0.001789 | 0.001892 | 0.001806 |
| mp_any_of.test | RUN | 0.458319 | 1.455071 | 1.321893 | 0.001582 | 0.001724 | 0.001849 |
| mp_any_of_q.test | RUN | 0.173486 | 0.608390 | 0.615233 | 0.001783 | 0.001748 | 0.001854 |
| mp_none_of.test | RUN | 0.466149 | 1.456564 | 1.290569 | 0.001530 | 0.001756 | 0.001741 |
| mp_none_of_q.test | RUN | 0.179562 | 0.611969 | 0.618598 | 0.001714 | 0.001894 | 0.001623 |
| mp_replace_at.test | RUN | 0.211793 | 1.213107 | 0.874762 | 0.001586 | 0.001537 | 0.001743 |
| mp_replace_at_c.test | RUN | 0.210073 | 1.184249 | 0.909108 | 0.001766 | 0.001692 | 0.001738 |
| mp_for_each.test | RUN | 0.208856 | 0.890143 | 1.016877 | 0.001794 | 0.001691 | 0.001702 |
| mp_insert.test | RUN | 0.310554 | 2.454256 | 1.247694 | 0.001574 | 0.001806 | 0.001537 |
| mp_insert_2.test | RUN | 0.028215 | 0.058435 | 0.096154 | 0.000729 | 0.000732 | 0.001724 |
| mp_erase.test | RUN | 0.338158 | 2.598793 | 1.473870 | 0.001846 | 0.001849 | 0.001874 |
| mp_erase_2.test | RUN | 0.026741 | 0.056606 | 0.080893 | 0.000714 | 0.000750 | 0.001690 |
| mp_with_index.test | RUN | 0.428751 | 5.332073 | 3.373514 | 0.002302 | 0.002072 | 0.001892 |
| mp_with_index_cx.test | RUN | 0.003571 | 0.018271 | 0.026034 | 0.000781 | 0.000799 | 0.001597 |
| mp_from_sequence.test | RUN | 0.201051 | 1.402032 | 0.895968 | 0.001818 | 0.001785 | 0.001968 |
| mp_from_sequence_2.test | RUN | 0.213825 | 1.635970 | 0.899925 | 0.001812 | 0.001838 | 0.001618 |
| mp_from_sequence_3.test | RUN | 0.186697 | 0.719948 | 0.677835 | 0.001557 | 0.001780 | 0.001856 |
| mp_min_element.test | RUN | 0.181272 | 0.664757 | 0.642276 | 0.001537 | 0.001748 | 0.001757 |
| mp_min_element_2.test | RUN | 0.029298 | 0.057401 | 0.098990 | 0.000741 | 0.000665 | 0.001869 |
| mp_min_element_q.test | RUN | 0.182853 | 0.669689 | 0.620056 | 0.001679 | 0.001758 | 0.001693 |
| mp_min_element_q_2.test | RUN | 0.028683 | 0.057167 | 0.093242 | 0.000771 | 0.000800 | 0.001466 |
| mp_max_element.test | RUN | 0.182927 | 0.655135 | 0.601189 | 0.001646 | 0.001841 | 0.001796 |
| mp_max_element_2.test | RUN | 0.027800 | 0.057798 | 0.098321 | 0.000612 | 0.000669 | 0.001502 |
| mp_max_element_q.test | RUN | 0.178852 | 0.672397 | 0.619695 | 0.001692 | 0.001776 | 0.001454 |
| mp_max_element_q_2.test | RUN | 0.027846 | 0.057482 | 0.100138 | 0.000678 | 0.000728 | 0.001676 |
| mp_nth_element.test | RUN | 0.193103 | 0.930919 | 0.785790 | 0.001450 | 0.001780 | 0.001500 |
| mp_nth_element_q.test | RUN | 0.199470 | 0.935846 | 0.775603 | 0.001562 | 0.001580 | 0.001624 |
| mp_back.test | RUN | 0.194054 | 0.750013 | 0.702914 | 0.001584 | 0.001742 | 0.001544 |
| mp_back_2.test | RUN | 0.028352 | 0.056419 | 0.099458 | 0.000717 | 0.000858 | 0.001763 |
| mp_pop_back.test | RUN | 0.494278 | 4.976299 | 1.470648 | 0.001810 | 0.001556 | 0.001604 |
| mp_pop_back_2.test | RUN | 0.027461 | 0.058975 | 0.085625 | 0.000754 | 0.000607 | 0.001365 |
| mp_flatten.test | RUN | 0.225322 | 1.777044 | 0.999134 | 0.001794 | 0.001876 | 0.001680 |
| mp_rotate_left.test | RUN | 0.223287 | 1.925050 | 0.993767 | 0.001680 | 0.001620 | 0.001364 |
| mp_rotate_right.test | RUN | 0.223706 | 1.909770 | 0.983758 | 0.001572 | 0.001890 | 0.001580 |
| mp_power_set.test | RUN | 0.215485 | 1.826073 | 0.976940 | 0.001535 | 0.001865 | 0.001742 |
| mp_partial_sum.test | RUN | 0.280793 | 2.442364 | 1.118975 | 0.001668 | 0.001514 | 0.001596 |
| mp_iterate.test | RUN | 0.194869 | 0.745196 | 0.787096 | 0.001775 | 0.001902 | 0.001584 |
| mp_pairwise_fold.test | RUN | 0.220717 | 1.692800 | 0.999427 | 0.001716 | 0.001746 | 0.001661 |
| mp_pairwise_fold_q.test | RUN | 0.177263 | 0.504676 | 0.579738 | 0.001912 | 0.001839 | 0.001776 |
| mp_sliding_fold.test | RUN | 0.430465 | 3.206877 | 1.827933 | 0.001389 | 0.001842 | 0.001784 |
| mp_sliding_fold_q.test | RUN | 0.422959 | 3.232068 | 1.846932 | 0.001678 | 0.001516 | 0.001538 |
| mp_intersperse.test | RUN | 0.231266 | 1.218590 | 0.861532 | 0.001634 | 0.001794 | 0.001804 |
| mp_split.test | RUN | 0.292182 | 2.327575 | 1.210053 | 0.001590 | 0.001720 | 0.001717 |
| mp_join.test | RUN | 0.239281 | 1.940829 | 1.032855 | 0.001994 | 0.001926 | 0.001589 |
| integral.test | RUN | 0.179599 | 0.874829 | 0.624530 | 0.001423 | 0.001885 | 0.001772 |
| mp_value.test | RUN | 0.012066 | 0.032073 | 0.045602 | 0.000655 | 0.000719 | 0.001784 |
| mp_identity.test | RUN | 0.174278 | 0.798981 | 0.618247 | 0.001771 | 0.001708 | 0.001827 |
| mp_inherit.test | RUN | 0.165726 | 0.579260 | 0.578973 | 0.001673 | 0.001778 | 0.001810 |
| mp_if.test | RUN | 0.167725 | 0.662718 | 0.583051 | 0.001744 | 0.001594 | 0.001638 |
| mp_if_sf.test | RUN | 0.158514 | 0.489220 | 0.534720 | 0.001739 | 0.001877 | 0.001707 |
| mp_eval_if.test | RUN | 0.169402 | 0.697041 | 0.592383 | 0.001613 | 0.001712 | 0.001836 |
| mp_eval_if_sf.test | RUN | 0.160781 | 0.502847 | 0.535959 | 0.001563 | 0.001451 | 0.001612 |
| mp_valid.test | RUN | 0.167962 | 0.511298 | 0.573734 | 0.001791 | 0.001522 | 0.001650 |
| mp_defer.test | RUN | 0.176887 | 0.846730 | 0.625397 | 0.001800 | 0.001782 | 0.001732 |
| mp_quote.test | RUN | 0.173277 | 0.859657 | 0.669286 | 0.001778 | 0.001756 | 0.001519 |
| mp_invoke_q.test | RUN | 0.188799 | 1.108787 | 0.732341 | 0.001632 | 0.001713 | 0.001608 |
| mp_invoke_q_sf.test | RUN | 0.163980 | 0.500621 | 0.548614 | 0.001728 | 0.001685 | 0.001624 |
| mp_quote_trait.test | RUN | 0.173366 | 0.932850 | 0.646830 | 0.001804 | 0.001761 | 0.001825 |
| mp_cond.test | RUN | 0.169749 | 0.570254 | 0.583738 | 0.001722 | 0.001792 | 0.001547 |
| mp_cond_sf.test | RUN | 0.163662 | 0.505917 | 0.584913 | 0.001610 | 0.001818 | 0.001535 |
| mp_not_fn.test | RUN | 0.163356 | 0.484783 | 0.527060 | 0.001655 | 0.001834 | 0.001419 |
| mp_eval_if_not.test | RUN | 0.172773 | 0.709324 | 0.608505 | 0.001383 | 0.001808 | 0.001630 |
| mp_eval_or.test | RUN | 0.160694 | 0.381649 | 0.567178 | 0.001550 | 0.001774 | 0.001751 |
| mp_compose.test | RUN | 0.186233 | 1.154028 | 0.737875 | 0.001691 | 0.001630 | 0.001752 |
| mp_valid_and_true.test | RUN | 0.165443 | 0.590567 | 0.592966 | 0.001668 | 0.001904 | 0.001589 |
| mp_compose_sf.test | COMPILE | 0.037975 | 0.075837 | 0.123672 | — | — | — |
| integer_sequence.test | RUN | 0.200829 | 1.137703 | 0.800899 | 0.001490 | 0.001782 | 0.001568 |
| tuple_for_each.test | RUN | 0.169792 | 0.426467 | 0.586227 | 0.001400 | 0.001748 | 0.001690 |
| tuple_for_each_cx.test | COMPILE | 0.031064 | 0.066774 | 0.095330 | — | — | — |
| tuple_apply.test | RUN | 0.169748 | 0.398640 | 0.595423 | 0.001558 | 0.001520 | 0.001630 |
| tuple_apply_cx.test | COMPILE | 0.045000 | 0.092145 | 0.153005 | — | — | — |
| construct_from_tuple.test | RUN | 0.169591 | 0.468403 | 0.604330 | 0.001559 | 0.001689 | 0.001627 |
| construct_from_tuple_cx.test | COMPILE | 0.047598 | 0.086248 | 0.150262 | — | — | — |
| tuple_transform.test | RUN | 0.275255 | 0.923721 | 1.181917 | 0.001848 | 0.001651 | 0.001783 |
| tuple_transform_2.test | RUN | 0.239400 | 0.693719 | 1.041803 | 0.001691 | 0.001860 | 0.001702 |
| tuple_transform_cx.test | COMPILE | 0.039481 | 0.083628 | 0.146349 | — | — | — |
| mp_set_contains.test | RUN | 0.169817 | 0.600458 | 0.610555 | 0.001455 | 0.001944 | 0.001623 |
| mp_set_push_back.test | RUN | 0.219600 | 1.829785 | 0.994732 | 0.001662 | 0.001792 | 0.001763 |
| mp_set_push_front.test | RUN | 0.281095 | 2.189478 | 1.118294 | 0.001671 | 0.001748 | 0.001737 |
| mp_is_set.test | RUN | 0.176160 | 0.620072 | 0.624972 | 0.001741 | 0.001844 | 0.001787 |
| mp_set_union.test | RUN | 0.217171 | 1.539722 | 0.920763 | 0.001687 | 0.001860 | 0.001424 |
| mp_set_union_sf.test | RUN | 0.169289 | 0.493928 | 0.574209 | 0.001538 | 0.001728 | 0.001633 |
| mp_set_difference.test | RUN | 0.193107 | 1.222267 | 0.798657 | 0.001685 | 0.001763 | 0.001504 |
| mp_set_difference_sf.test | RUN | 0.167569 | 0.489289 | 0.557837 | 0.001542 | 0.001690 | 0.001571 |
| mp_set_intersection.test | RUN | 0.190349 | 1.204837 | 0.785521 | 0.001704 | 0.001726 | 0.001736 |
| mp_set_intersection_sf.test | RUN | 0.165982 | 0.489982 | 0.551864 | 0.001543 | 0.001728 | 0.001451 |
| mp_all.test | RUN | 0.170318 | 0.606393 | 0.570214 | 0.001589 | 0.001859 | 0.001754 |
| mp_all_2.test | RUN | 0.494317 | 1.290211 | 1.331094 | 0.001586 | 0.001905 | 0.001838 |
| mp_and.test | RUN | 0.169837 | 0.610855 | 0.590370 | 0.001620 | 0.001748 | 0.001802 |
| mp_any.test | RUN | 0.174017 | 0.599212 | 0.575661 | 0.001584 | 0.001717 | 0.001791 |
| mp_any_2.test | RUN | 0.482459 | 1.275633 | 1.341271 | 0.001620 | 0.001687 | 0.001600 |
| mp_or.test | RUN | 0.169049 | 0.597600 | 0.587171 | 0.001668 | 0.001692 | 0.001654 |
| mp_same.test | RUN | 0.322319 | 1.001611 | 0.896268 | 0.001648 | 0.001543 | 0.001573 |
| mp_plus.test | RUN | 0.180781 | 0.684053 | 0.604163 | 0.001601 | 0.001481 | 0.001374 |
| mp_less.test | RUN | 0.170830 | 0.604085 | 0.543975 | 0.001767 | 0.001716 | 0.001738 |
| mp_min.test | RUN | 0.169284 | 0.561419 | 0.592153 | 0.001410 | 0.001656 | 0.001374 |
| mp_max.test | RUN | 0.170865 | 0.613743 | 0.582791 | 0.001608 | 0.001670 | 0.001627 |
| mp_similar.test | RUN | 0.164740 | 0.603504 | 0.610402 | 0.001577 | 0.001492 | 0.001601 |
| mp_map_find.test | RUN | 0.196215 | 1.091096 | 0.767705 | 0.001792 | 0.001655 | 0.001413 |
| mp_map_find_2.test | RUN | 0.182567 | 0.440890 | 0.682221 | 0.001674 | 0.001724 | 0.001666 |
| mp_map_find_3.test | RUN | 0.183677 | 0.432190 | 0.681331 | 0.001546 | 0.001576 | 0.001452 |
| mp_map_find_4.test | RUN | 0.172510 | 0.405129 | 0.583957 | 0.001464 | 0.001830 | 0.001500 |
| mp_map_contains.test | RUN | 0.177207 | 0.619398 | 0.609127 | 0.001770 | 0.001918 | 0.001776 |
| mp_map_insert.test | RUN | 0.226521 | 2.028630 | 1.051999 | 0.001661 | 0.001843 | 0.001626 |
| mp_map_replace.test | RUN | 0.301370 | 2.303512 | 1.241412 | 0.001566 | 0.001830 | 0.001775 |
| mp_map_erase.test | RUN | 0.226356 | 1.731337 | 1.057782 | 0.001562 | 0.001911 | 0.001485 |
| mp_map_update.test | RUN | 0.195575 | 1.121621 | 0.808146 | 0.001341 | 0.001725 | 0.001627 |
| mp_map_update_q.test | RUN | 0.200360 | 1.145686 | 0.813820 | 0.001671 | 0.001666 | 0.001381 |
| mp_map_keys.test | RUN | 0.198236 | 1.013523 | 0.794515 | 0.001595 | 0.001911 | 0.001532 |
| mp_is_map.test | RUN | 0.180821 | 0.603148 | 0.638575 | 0.001407 | 0.001906 | 0.001536 |
| mp_bind.test | RUN | 0.219432 | 1.815021 | 0.921177 | 0.001778 | 0.001653 | 0.001774 |
| mp_bind_q.test | RUN | 0.192834 | 1.130693 | 0.774876 | 0.001752 | 0.001778 | 0.001422 |
| mp_bind_front.test | RUN | 0.187312 | 1.233305 | 0.724062 | 0.001838 | 0.001631 | 0.001613 |
| mp_bind_back.test | RUN | 0.186527 | 1.238260 | 0.754978 | 0.001391 | 0.001687 | 0.001442 |
| mp_lambda.test | RUN | 0.409977 | 3.711946 | 1.859961 | 0.001917 | 0.001295 | 0.001784 |
| mpl.test | RUN | 0.381545 | 1.014960 | 1.512889 | 0.001870 | 0.001620 | 0.001672 |
| mpl_list.test | RUN | 0.216606 | 0.747769 | 0.731758 | 0.001627 | 0.001858 | 0.001780 |
| mpl_tuple.test | RUN | 0.221786 | 0.759667 | 0.733370 | 0.001504 | 0.001664 | 0.001433 |
| version.test | RUN | 0.109694 | 0.267121 | 0.388691 | 0.001710 | 0.001782 | 0.001433 |
| mp11_version.test | RUN | 0.136508 | 0.311418 | 0.408066 | 0.001640 | 0.001589 | 0.001596 |
| list_version.test | RUN | 0.115692 | 0.274405 | 0.366105 | 0.001391 | 0.001819 | 0.001777 |
| algorithm_version.test | RUN | 0.123246 | 0.279401 | 0.406293 | 0.001806 | 0.001833 | 0.001838 |
| integral_version.test | RUN | 0.111335 | 0.278030 | 0.358079 | 0.001698 | 0.001784 | 0.001360 |
| utility_version.test | RUN | 0.110926 | 0.271196 | 0.394927 | 0.001439 | 0.001679 | 0.001431 |
| function_version.test | RUN | 0.112147 | 0.277955 | 0.370478 | 0.001420 | 0.001783 | 0.001640 |
| map_version.test | RUN | 0.122810 | 0.291220 | 0.398764 | 0.001669 | 0.001453 | 0.001884 |
| set_version.test | RUN | 0.115487 | 0.286439 | 0.404267 | 0.001693 | 0.001563 | 0.001710 |
| bind_version.test | RUN | 0.126531 | 0.293158 | 0.430091 | 0.001590 | 0.001788 | 0.001574 |
| integer_sequence_version.test | RUN | 0.107087 | 0.273170 | 0.348477 | 0.001461 | 0.001742 | 0.001369 |
| tuple_version.test | RUN | 0.130235 | 0.299720 | 0.320250 | 0.001365 | 0.001337 | 0.001252 |
| mpl_version.test | RUN | 0.095931 | 0.300857 | 0.332554 | 0.001289 | 0.001319 | 0.001292 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.009× | 1.008×, 1.010× | 0.996× |
| clang | 1.075× | 1.070×, 1.079× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.000949s | 0.000959s | 0.000959s | byte equality observed |
| bind_version.test | 10 | 0.000954s | 0.000958s | 0.000980s | byte equality observed |
| construct_from_tuple.test | 10 | 0.000969s | 0.000964s | 0.001007s | byte equality observed |
| function_version.test | 10 | 0.000954s | 0.000953s | 0.001002s | byte equality observed |
| integer_sequence.test | 10 | 0.000953s | 0.000969s | 0.001049s | byte equality observed |
| integer_sequence_version.test | 10 | 0.000955s | 0.000951s | 0.000980s | byte equality observed |
| integral.test | 10 | 0.000971s | 0.000964s | 0.001019s | byte equality observed |
| integral_version.test | 10 | 0.000965s | 0.000951s | 0.001005s | byte equality observed |
| list_version.test | 10 | 0.000968s | 0.000975s | 0.001000s | byte equality observed |
| map_version.test | 10 | 0.000960s | 0.000965s | 0.000993s | byte equality observed |
| mp11_version.test | 10 | 0.000957s | 0.000955s | 0.001015s | byte equality observed |
| mp_all.test | 10 | 0.000971s | 0.000987s | 0.001004s | byte equality observed |
| mp_all_2.test | 10 | 0.001001s | 0.001012s | 0.001011s | byte equality observed |
| mp_all_of.test | 10 | 0.001002s | 0.000993s | 0.001003s | byte equality observed |
| mp_all_of_q.test | 10 | 0.000989s | 0.000998s | 0.001005s | byte equality observed |
| mp_and.test | 10 | 0.001001s | 0.001054s | 0.001035s | byte equality observed |
| mp_any.test | 10 | 0.000990s | 0.000999s | 0.001049s | byte equality observed |
| mp_any_2.test | 10 | 0.001019s | 0.001014s | 0.001020s | byte equality observed |
| mp_any_of.test | 10 | 0.001013s | 0.001026s | 0.001028s | byte equality observed |
| mp_any_of_q.test | 10 | 0.000997s | 0.001017s | 0.001004s | byte equality observed |
| mp_append.test | 10 | 0.000988s | 0.001028s | 0.001016s | byte equality observed |
| mp_append_2.test | 10 | 0.001018s | 0.001034s | 0.001003s | byte equality observed |
| mp_append_3.test | 10 | 0.000458s | 0.000463s | 0.000983s | byte equality observed |
| mp_append_sf.test | 10 | 0.001001s | 0.001068s | 0.001002s | byte equality observed |
| mp_apply_q.test | 10 | 0.001003s | 0.001014s | 0.000999s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001010s | 0.001037s | 0.001003s | byte equality observed |
| mp_assign.test | 10 | 0.001014s | 0.001015s | 0.001000s | byte equality observed |
| mp_assign_2.test | 10 | 0.000461s | 0.000468s | 0.000969s | byte equality observed |
| mp_at.test | 10 | 0.001019s | 0.001037s | 0.001008s | byte equality observed |
| mp_at_2.test | 10 | 0.000462s | 0.000458s | 0.000993s | byte equality observed |
| mp_at_sf.test | 10 | 0.001014s | 0.001030s | 0.000991s | byte equality observed |
| mp_back.test | 10 | 0.001022s | 0.001028s | 0.001011s | byte equality observed |
| mp_back_2.test | 10 | 0.000466s | 0.000464s | 0.000982s | byte equality observed |
| mp_bind.test | 10 | 0.001019s | 0.001010s | 0.001003s | byte equality observed |
| mp_bind_back.test | 10 | 0.001018s | 0.001004s | 0.000998s | byte equality observed |
| mp_bind_front.test | 10 | 0.001007s | 0.001004s | 0.001038s | byte equality observed |
| mp_bind_q.test | 10 | 0.000988s | 0.001007s | 0.000993s | byte equality observed |
| mp_clear.test | 10 | 0.001017s | 0.001031s | 0.001000s | byte equality observed |
| mp_clear_2.test | 10 | 0.000463s | 0.000466s | 0.000995s | byte equality observed |
| mp_compose.test | 10 | 0.000992s | 0.001027s | 0.001031s | byte equality observed |
| mp_cond.test | 10 | 0.000998s | 0.001019s | 0.001007s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001004s | 0.001029s | 0.000998s | byte equality observed |
| mp_contains.test | 10 | 0.001031s | 0.001023s | 0.001003s | byte equality observed |
| mp_copy_if.test | 10 | 0.001025s | 0.001036s | 0.001047s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.000998s | 0.001024s | 0.000986s | byte equality observed |
| mp_count.test | 10 | 0.001040s | 0.001004s | 0.001005s | byte equality observed |
| mp_count_if.test | 10 | 0.001013s | 0.001012s | 0.001007s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001030s | 0.001007s | 0.001004s | byte equality observed |
| mp_defer.test | 10 | 0.001017s | 0.001027s | 0.001006s | byte equality observed |
| mp_drop.test | 10 | 0.001008s | 0.001042s | 0.000991s | byte equality observed |
| mp_drop_2.test | 10 | 0.000462s | 0.000462s | 0.001017s | byte equality observed |
| mp_empty.test | 10 | 0.001030s | 0.001053s | 0.001015s | byte equality observed |
| mp_empty_2.test | 10 | 0.000465s | 0.000461s | 0.000996s | byte equality observed |
| mp_erase.test | 10 | 0.001048s | 0.001050s | 0.001004s | byte equality observed |
| mp_erase_2.test | 10 | 0.000462s | 0.000463s | 0.001002s | byte equality observed |
| mp_eval_if.test | 10 | 0.001018s | 0.000991s | 0.001058s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001006s | 0.001023s | 0.001018s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001038s | 0.000997s | 0.000996s | byte equality observed |
| mp_eval_or.test | 10 | 0.001041s | 0.000987s | 0.001002s | byte equality observed |
| mp_fill.test | 10 | 0.001041s | 0.001027s | 0.001005s | byte equality observed |
| mp_fill_2.test | 10 | 0.000466s | 0.000470s | 0.000992s | byte equality observed |
| mp_filter.test | 10 | 0.000997s | 0.001019s | 0.001016s | byte equality observed |
| mp_find.test | 10 | 0.001015s | 0.000999s | 0.001004s | byte equality observed |
| mp_find_if.test | 10 | 0.001025s | 0.001012s | 0.001004s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001007s | 0.001008s | 0.001000s | byte equality observed |
| mp_flatten.test | 10 | 0.001022s | 0.001012s | 0.000992s | byte equality observed |
| mp_fold.test | 10 | 0.001017s | 0.001033s | 0.001003s | byte equality observed |
| mp_fold_q.test | 10 | 0.001049s | 0.001022s | 0.000990s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001018s | 0.000988s | 0.000985s | byte equality observed |
| mp_for_each.test | 10 | 0.001028s | 0.001007s | 0.000999s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001013s | 0.001026s | 0.001003s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001015s | 0.001019s | 0.001007s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001037s | 0.000990s | 0.001019s | byte equality observed |
| mp_front.test | 10 | 0.001021s | 0.001015s | 0.001005s | byte equality observed |
| mp_front_2.test | 10 | 0.000466s | 0.000466s | 0.001009s | byte equality observed |
| mp_identity.test | 10 | 0.001002s | 0.000999s | 0.001044s | byte equality observed |
| mp_if.test | 10 | 0.001003s | 0.000998s | 0.001013s | byte equality observed |
| mp_if_sf.test | 10 | 0.001001s | 0.001039s | 0.001020s | byte equality observed |
| mp_inherit.test | 10 | 0.001003s | 0.000995s | 0.001029s | byte equality observed |
| mp_insert.test | 10 | 0.001035s | 0.001048s | 0.000991s | byte equality observed |
| mp_insert_2.test | 10 | 0.000467s | 0.000466s | 0.001005s | byte equality observed |
| mp_intersperse.test | 10 | 0.001007s | 0.001017s | 0.001036s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001001s | 0.001006s | 0.001013s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001035s | 0.001009s | 0.000994s | byte equality observed |
| mp_iota.test | 10 | 0.000998s | 0.001068s | 0.000991s | byte equality observed |
| mp_iota_2.test | 10 | 0.001018s | 0.001019s | 0.001005s | byte equality observed |
| mp_is_list.test | 10 | 0.001012s | 0.001029s | 0.000991s | byte equality observed |
| mp_is_map.test | 10 | 0.001010s | 0.001024s | 0.000997s | byte equality observed |
| mp_is_set.test | 10 | 0.001026s | 0.001017s | 0.001007s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001021s | 0.001012s | 0.001035s | byte equality observed |
| mp_iterate.test | 10 | 0.001006s | 0.001018s | 0.000987s | byte equality observed |
| mp_join.test | 10 | 0.001027s | 0.001057s | 0.001005s | byte equality observed |
| mp_lambda.test | 10 | 0.001032s | 0.001032s | 0.001039s | byte equality observed |
| mp_less.test | 10 | 0.000995s | 0.001029s | 0.001013s | byte equality observed |
| mp_list_c.test | 10 | 0.001002s | 0.001037s | 0.001034s | byte equality observed |
| mp_list_v.test | 10 | 0.000462s | 0.000460s | 0.000979s | byte equality observed |
| mp_map_contains.test | 10 | 0.001008s | 0.001039s | 0.001000s | byte equality observed |
| mp_map_erase.test | 10 | 0.001021s | 0.001024s | 0.001013s | byte equality observed |
| mp_map_find.test | 10 | 0.001019s | 0.001028s | 0.001000s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001006s | 0.001030s | 0.000995s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001013s | 0.001001s | 0.001028s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001016s | 0.001022s | 0.001016s | byte equality observed |
| mp_map_insert.test | 10 | 0.001019s | 0.001045s | 0.001021s | byte equality observed |
| mp_map_keys.test | 10 | 0.001021s | 0.001006s | 0.001006s | byte equality observed |
| mp_map_replace.test | 10 | 0.001029s | 0.001006s | 0.001054s | byte equality observed |
| mp_map_update.test | 10 | 0.001012s | 0.000992s | 0.001002s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001024s | 0.000997s | 0.001002s | byte equality observed |
| mp_max.test | 10 | 0.001016s | 0.001005s | 0.001005s | byte equality observed |
| mp_max_element.test | 10 | 0.001024s | 0.001028s | 0.001041s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000463s | 0.000473s | 0.000980s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001039s | 0.001051s | 0.001015s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000466s | 0.000469s | 0.001005s | byte equality observed |
| mp_min.test | 10 | 0.001020s | 0.001030s | 0.000989s | byte equality observed |
| mp_min_element.test | 10 | 0.001019s | 0.001041s | 0.000993s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000467s | 0.000473s | 0.000979s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001017s | 0.001028s | 0.001007s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000471s | 0.000472s | 0.001009s | byte equality observed |
| mp_none_of.test | 10 | 0.001005s | 0.001029s | 0.001000s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001015s | 0.001069s | 0.001035s | byte equality observed |
| mp_not_fn.test | 10 | 0.001005s | 0.001003s | 0.001005s | byte equality observed |
| mp_nth_element.test | 10 | 0.000998s | 0.001012s | 0.001007s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.000990s | 0.001042s | 0.001001s | byte equality observed |
| mp_or.test | 10 | 0.001000s | 0.001042s | 0.001010s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001004s | 0.001000s | 0.001001s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001006s | 0.001027s | 0.001053s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001019s | 0.001056s | 0.001001s | byte equality observed |
| mp_partition.test | 10 | 0.001030s | 0.001007s | 0.001031s | byte equality observed |
| mp_partition_q.test | 10 | 0.001015s | 0.001015s | 0.001030s | byte equality observed |
| mp_plus.test | 10 | 0.001003s | 0.001000s | 0.001010s | byte equality observed |
| mp_pop_back.test | 10 | 0.001034s | 0.001033s | 0.001008s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000469s | 0.000461s | 0.000994s | byte equality observed |
| mp_pop_front.test | 10 | 0.001021s | 0.001031s | 0.001008s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000460s | 0.000469s | 0.000990s | byte equality observed |
| mp_power_set.test | 10 | 0.001008s | 0.001034s | 0.001004s | byte equality observed |
| mp_product.test | 10 | 0.001014s | 0.001015s | 0.001012s | byte equality observed |
| mp_push_back.test | 10 | 0.001017s | 0.001026s | 0.000988s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000466s | 0.000465s | 0.000988s | byte equality observed |
| mp_push_front.test | 10 | 0.001049s | 0.001037s | 0.001013s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000467s | 0.000454s | 0.000999s | byte equality observed |
| mp_quote.test | 10 | 0.000998s | 0.001001s | 0.000987s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001003s | 0.001042s | 0.001034s | byte equality observed |
| mp_remove.test | 5 | 0.001041s | 0.001027s | 0.001020s | byte equality observed |
| mp_remove_if.test | 10 | 0.001017s | 0.001028s | 0.001020s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001033s | 0.001033s | 0.000995s | byte equality observed |
| mp_rename.test | 10 | 0.001029s | 0.001036s | 0.000992s | byte equality observed |
| mp_rename_2.test | 10 | 0.000469s | 0.000464s | 0.000996s | byte equality observed |
| mp_rename_v.test | 10 | 0.000463s | 0.000466s | 0.001025s | byte equality observed |
| mp_repeat.test | 10 | 0.001034s | 0.001045s | 0.000980s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000465s | 0.000467s | 0.000998s | byte equality observed |
| mp_repeat_3.test | 10 | 0.000998s | 0.001037s | 0.000990s | byte equality observed |
| mp_replace.test | 10 | 0.000994s | 0.001008s | 0.001003s | byte equality observed |
| mp_replace_at.test | 10 | 0.001020s | 0.001005s | 0.001005s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.000990s | 0.001001s | 0.001019s | byte equality observed |
| mp_replace_front.test | 10 | 0.001017s | 0.001024s | 0.001008s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000467s | 0.000473s | 0.001005s | byte equality observed |
| mp_replace_if.test | 10 | 0.001012s | 0.001038s | 0.001006s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001027s | 0.001019s | 0.000993s | byte equality observed |
| mp_replace_second.test | 10 | 0.001032s | 0.001036s | 0.001024s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000465s | 0.000466s | 0.000999s | byte equality observed |
| mp_replace_third.test | 10 | 0.001016s | 0.001031s | 0.001022s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000466s | 0.000460s | 0.001015s | byte equality observed |
| mp_reverse.test | 10 | 0.001011s | 0.001027s | 0.001019s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001043s | 0.001012s | 0.001014s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001004s | 0.001016s | 0.001038s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001000s | 0.001015s | 0.001004s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001008s | 0.001016s | 0.001004s | byte equality observed |
| mp_same.test | 10 | 0.001015s | 0.001001s | 0.001016s | byte equality observed |
| mp_second.test | 10 | 0.001018s | 0.001039s | 0.000990s | byte equality observed |
| mp_second_2.test | 10 | 0.000457s | 0.000465s | 0.000975s | byte equality observed |
| mp_set_contains.test | 10 | 0.001015s | 0.001004s | 0.001004s | byte equality observed |
| mp_set_difference.test | 10 | 0.001030s | 0.001027s | 0.001023s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001015s | 0.001036s | 0.001018s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001012s | 0.001031s | 0.000973s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001017s | 0.001041s | 0.001000s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001017s | 0.001051s | 0.001014s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001012s | 0.001038s | 0.001005s | byte equality observed |
| mp_set_union.test | 10 | 0.001035s | 0.001017s | 0.001018s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001000s | 0.001012s | 0.001031s | byte equality observed |
| mp_similar.test | 10 | 0.001016s | 0.001012s | 0.001020s | byte equality observed |
| mp_size.test | 10 | 0.001023s | 0.001034s | 0.000996s | byte equality observed |
| mp_size_2.test | 10 | 0.000466s | 0.000482s | 0.000983s | byte equality observed |
| mp_slice.test | 10 | 0.001040s | 0.001048s | 0.001004s | byte equality observed |
| mp_slice_2.test | 10 | 0.000466s | 0.000466s | 0.000984s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001030s | 0.001027s | 0.001000s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001041s | 0.001001s | 0.001002s | byte equality observed |
| mp_sort.test | 10 | 0.001026s | 0.001029s | 0.001008s | byte equality observed |
| mp_sort_q.test | 10 | 0.001000s | 0.001023s | 0.001006s | byte equality observed |
| mp_split.test | 10 | 0.001033s | 0.001055s | 0.001002s | byte equality observed |
| mp_starts_with.test | 10 | 0.001020s | 0.001022s | 0.001016s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001023s | 0.001044s | 0.001027s | byte equality observed |
| mp_take.test | 10 | 0.001026s | 0.001043s | 0.001012s | byte equality observed |
| mp_take_2.test | 10 | 0.000469s | 0.000465s | 0.000987s | byte equality observed |
| mp_third.test | 10 | 0.001043s | 0.001036s | 0.000989s | byte equality observed |
| mp_third_2.test | 10 | 0.000471s | 0.000472s | 0.000996s | byte equality observed |
| mp_transform.test | 10 | 0.001016s | 0.001022s | 0.001011s | byte equality observed |
| mp_transform_2.test | 10 | 0.000997s | 0.000998s | 0.000989s | byte equality observed |
| mp_transform_front.test | 10 | 0.001038s | 0.001026s | 0.000991s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000466s | 0.000466s | 0.001006s | byte equality observed |
| mp_transform_if.test | 10 | 0.001018s | 0.001066s | 0.000994s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001005s | 0.001041s | 0.001001s | byte equality observed |
| mp_transform_q.test | 10 | 0.001089s | 0.001020s | 0.000978s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001000s | 0.001045s | 0.001014s | byte equality observed |
| mp_transform_second.test | 10 | 0.001027s | 0.001098s | 0.000980s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000472s | 0.000456s | 0.001006s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001005s | 0.001066s | 0.000994s | byte equality observed |
| mp_transform_third.test | 10 | 0.001009s | 0.001049s | 0.000996s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000473s | 0.000465s | 0.001013s | byte equality observed |
| mp_unique.test | 10 | 0.001015s | 0.001038s | 0.000992s | byte equality observed |
| mp_unique_if.test | 10 | 0.001028s | 0.001058s | 0.000994s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001061s | 0.001041s | 0.001008s | byte equality observed |
| mp_valid.test | 10 | 0.000986s | 0.001039s | 0.000995s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001016s | 0.001040s | 0.001010s | byte equality observed |
| mp_value.test | 10 | 0.000466s | 0.000461s | 0.000991s | byte equality observed |
| mp_with_index.test | 10 | 0.001370s | 0.001360s | 0.001262s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000460s | 0.000467s | 0.000989s | byte equality observed |
| mpl.test | 10 | 0.001026s | 0.000992s | 0.000983s | byte equality observed |
| mpl_list.test | 10 | 0.001014s | 0.001011s | 0.000963s | byte equality observed |
| mpl_tuple.test | 10 | 0.001007s | 0.001038s | 0.001018s | byte equality observed |
| mpl_version.test | 10 | 0.001001s | 0.000983s | 0.001016s | byte equality observed |
| set_version.test | 10 | 0.001023s | 0.001007s | 0.001006s | byte equality observed |
| tuple_apply.test | 10 | 0.001013s | 0.001010s | 0.001037s | byte equality observed |
| tuple_for_each.test | 10 | 0.001022s | 0.001014s | 0.001012s | byte equality observed |
| tuple_transform.test | 10 | 0.000984s | 0.001055s | 0.001029s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001034s | 0.001057s | 0.001018s | byte equality observed |
| tuple_version.test | 10 | 0.001025s | 0.001037s | 0.000985s | byte equality observed |
| utility_version.test | 10 | 0.001046s | 0.001008s | 0.001001s | byte equality observed |
| version.test | 10 | 0.001011s | 0.001000s | 0.001007s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
