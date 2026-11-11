# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.046680 | 0.096089 | 0.258989 | — | — | — |
| complex_h.test | COMPILE | 0.191528 | 0.401381 | 0.634674 | — | — | — |
| mp_size.test | RUN | 0.224809 | 1.320886 | 0.821935 | 0.001459 | 0.002306 | 0.001552 |
| mp_size_2.test | RUN | 0.019410 | 0.051596 | 0.068246 | 0.000603 | 0.000629 | 0.001831 |
| mp_empty.test | RUN | 0.216298 | 0.831201 | 0.778024 | 0.001623 | 0.001906 | 0.002156 |
| mp_empty_2.test | RUN | 0.020584 | 0.044217 | 0.104490 | 0.000951 | 0.000797 | 0.001912 |
| mp_assign.test | RUN | 0.241638 | 1.646305 | 1.498583 | 0.001419 | 0.001992 | 0.001580 |
| mp_assign_2.test | RUN | 0.019364 | 0.043560 | 0.065774 | 0.000646 | 0.000622 | 0.001660 |
| mp_clear.test | RUN | 0.224534 | 0.766331 | 1.009032 | 0.001442 | 0.002691 | 0.001865 |
| mp_clear_2.test | RUN | 0.019307 | 0.043126 | 0.106913 | 0.000592 | 0.000803 | 0.001729 |
| mp_front.test | RUN | 0.220617 | 1.124413 | 1.362678 | 0.001668 | 0.001798 | 0.001942 |
| mp_front_2.test | RUN | 0.022581 | 0.042794 | 0.105690 | 0.000653 | 0.001004 | 0.001864 |
| mp_pop_front.test | RUN | 0.243907 | 1.349461 | 0.882508 | 0.001789 | 0.002668 | 0.001556 |
| mp_pop_front_2.test | RUN | 0.019297 | 0.043490 | 0.103729 | 0.000650 | 0.000813 | 0.001388 |
| mp_second.test | RUN | 0.263777 | 0.831158 | 0.818707 | 0.001596 | 0.002228 | 0.001670 |
| mp_second_2.test | RUN | 0.019691 | 0.043837 | 0.070324 | 0.000682 | 0.000697 | 0.002046 |
| mp_third.test | RUN | 0.225462 | 0.716938 | 0.798220 | 0.001872 | 0.001762 | 0.001897 |
| mp_third_2.test | RUN | 0.019897 | 0.046198 | 0.080224 | 0.000585 | 0.000595 | 0.001656 |
| mp_push_front.test | RUN | 0.360565 | 2.650946 | 1.735444 | 0.001844 | 0.001940 | 0.001752 |
| mp_push_front_2.test | RUN | 0.020461 | 0.046427 | 0.079225 | 0.000721 | 0.000658 | 0.001492 |
| mp_push_back.test | RUN | 0.336847 | 2.539698 | 1.335450 | 0.001450 | 0.002386 | 0.002410 |
| mp_push_back_2.test | RUN | 0.020425 | 0.045121 | 0.073917 | 0.000626 | 0.000614 | 0.001851 |
| mp_rename.test | RUN | 0.271859 | 2.315581 | 1.558553 | 0.002009 | 0.002120 | 0.001664 |
| mp_rename_2.test | RUN | 0.019767 | 0.045874 | 0.073078 | 0.000620 | 0.000586 | 0.001547 |
| mp_append.test | RUN | 0.291159 | 2.572326 | 1.293165 | 0.001652 | 0.002016 | 0.002062 |
| mp_append_2.test | RUN | 0.587282 | 5.652002 | 1.708577 | 0.001505 | 0.001710 | 0.002193 |
| mp_append_3.test | RUN | 0.019009 | 0.047278 | 0.069340 | 0.000662 | 0.001142 | 0.001787 |
| mp_append_sf.test | RUN | 0.215558 | 0.647005 | 0.706564 | 0.001888 | 0.001780 | 0.001569 |
| mp_replace_front.test | RUN | 0.255506 | 1.883037 | 1.007970 | 0.013780 | 0.001806 | 0.001751 |
| mp_replace_front_2.test | RUN | 0.020320 | 0.046288 | 0.072054 | 0.000749 | 0.000681 | 0.001919 |
| mp_replace_second.test | RUN | 0.242537 | 1.253401 | 0.985155 | 0.001826 | 0.002164 | 0.001736 |
| mp_replace_second_2.test | RUN | 0.019395 | 0.045507 | 0.070967 | 0.000936 | 0.000900 | 0.001659 |
| mp_replace_third.test | RUN | 0.237045 | 0.988946 | 0.863568 | 0.001560 | 0.001726 | 0.001952 |
| mp_replace_third_2.test | RUN | 0.022191 | 0.043838 | 0.073560 | 0.000690 | 0.000638 | 0.002008 |
| mp_apply_q.test | RUN | 0.268774 | 2.349837 | 1.152806 | 0.001758 | 0.002830 | 0.001716 |
| mp_apply_q_sf.test | RUN | 0.210805 | 0.686887 | 0.738691 | 0.001390 | 0.002070 | 0.001495 |
| mp_is_list.test | RUN | 0.219776 | 0.796938 | 0.736102 | 0.001522 | 0.001776 | 0.001704 |
| mp_list_c.test | RUN | 0.228008 | 0.947222 | 0.855656 | 0.001794 | 0.001926 | 0.001936 |
| mp_transform_front.test | RUN | 0.261221 | 1.843459 | 1.045840 | 0.001786 | 0.001980 | 0.001617 |
| mp_transform_front_2.test | RUN | 0.020108 | 0.047050 | 0.069602 | 0.000611 | 0.000910 | 0.001579 |
| mp_transform_second.test | RUN | 0.246765 | 1.745933 | 0.951158 | 0.001625 | 0.002009 | 0.001804 |
| mp_transform_second_2.test | RUN | 0.020972 | 0.046275 | 0.072998 | 0.000679 | 0.001137 | 0.001728 |
| mp_transform_third.test | RUN | 0.231083 | 1.237182 | 0.855221 | 0.001710 | 0.002122 | 0.001846 |
| mp_transform_third_2.test | RUN | 0.020996 | 0.045512 | 0.074422 | 0.000657 | 0.000890 | 0.001605 |
| mp_list_v.test | RUN | 0.020872 | 0.045593 | 0.068373 | 0.000718 | 0.000730 | 0.001501 |
| mp_rename_v.test | RUN | 0.022202 | 0.050354 | 0.075024 | 0.000728 | 0.000668 | 0.001674 |
| mp_is_value_list.test | RUN | 0.207502 | 0.520343 | 0.729920 | 0.001840 | 0.001774 | 0.001870 |
| mp_transform.test | RUN | 0.445495 | 3.861502 | 2.077769 | 0.001955 | 0.002463 | 0.002050 |
| mp_transform_2.test | RUN | 0.197920 | 0.452173 | 0.687562 | 0.001710 | 0.001828 | 0.001714 |
| mp_transform_q.test | RUN | 0.436966 | 3.675068 | 2.059842 | 0.001727 | 0.002268 | 0.001700 |
| mp_transform_q_2.test | RUN | 0.489300 | 1.218067 | 2.499991 | 0.001686 | 0.001928 | 0.002026 |
| mp_transform_sf.test | RUN | 0.220333 | 0.671060 | 0.775477 | 0.001716 | 0.002044 | 0.001958 |
| mp_transform_if.test | RUN | 0.268875 | 1.896451 | 1.122951 | 0.001956 | 0.001871 | 0.001938 |
| mp_transform_if_q.test | RUN | 0.282419 | 1.893644 | 1.136574 | 0.001952 | 0.001984 | 0.001660 |
| mp_filter.test | RUN | 0.244093 | 1.116386 | 0.879054 | 0.002186 | 0.002292 | 0.001734 |
| mp_fill.test | RUN | 0.225735 | 0.835528 | 0.807831 | 0.001644 | 0.002028 | 0.001610 |
| mp_fill_2.test | RUN | 0.035838 | 0.069106 | 0.128588 | 0.000876 | 0.000959 | 0.001548 |
| mp_count.test | RUN | 0.426991 | 1.933358 | 1.210862 | 0.001414 | 0.002422 | 0.001634 |
| mp_count_if.test | RUN | 0.450499 | 1.994908 | 1.471725 | 0.001397 | 0.002194 | 0.001932 |
| mp_count_if_q.test | RUN | 0.243250 | 1.311436 | 0.867151 | 0.001649 | 0.002176 | 0.001960 |
| mp_contains.test | RUN | 0.234147 | 0.803362 | 0.803134 | 0.001339 | 0.001792 | 0.001770 |
| mp_starts_with.test | RUN | 0.242659 | 0.837387 | 0.785583 | 0.001686 | 0.001753 | 0.001763 |
| mp_starts_with_sf.test | RUN | 0.222118 | 0.685805 | 0.777782 | 0.001984 | 0.002378 | 0.001728 |
| mp_repeat.test | RUN | 0.358392 | 3.297857 | 1.427642 | 0.001878 | 0.002036 | 0.001856 |
| mp_repeat_2.test | RUN | 0.036516 | 0.073999 | 0.126457 | 0.000620 | 0.000756 | 0.001552 |
| mp_repeat_3.test | RUN | 0.270512 | 0.443100 | 0.749116 | 0.001818 | 0.001989 | 0.001749 |
| mp_product.test | RUN | 0.261773 | 1.975957 | 1.080676 | 0.001990 | 0.002298 | 0.002254 |
| mp_drop.test | RUN | 0.275701 | 2.419042 | 1.187876 | 0.001634 | 0.001696 | 0.001640 |
| mp_drop_2.test | RUN | 0.036022 | 0.071634 | 0.123239 | 0.000701 | 0.000646 | 0.001532 |
| mp_iota.test | RUN | 0.271243 | 2.083369 | 1.078522 | 0.001613 | 0.002152 | 0.001597 |
| mp_iota_2.test | RUN | 0.320775 | 2.469045 | 1.249464 | 0.001767 | 0.002602 | 0.006162 |
| mp_at.test | RUN | 0.235865 | 1.089073 | 0.930670 | 0.001986 | 0.002338 | 0.001623 |
| mp_at_2.test | RUN | 0.037877 | 0.071089 | 0.125510 | 0.000642 | 0.000667 | 0.001768 |
| mp_at_sf.test | RUN | 0.226493 | 0.674794 | 0.791697 | 0.001925 | 0.002236 | 0.001580 |
| mp_take.test | RUN | 0.363286 | 3.374884 | 1.396494 | 0.002082 | 0.002298 | 0.002566 |
| mp_take_2.test | RUN | 0.036885 | 0.072051 | 0.127523 | 0.000696 | 0.001296 | 0.001398 |
| mp_slice.test | RUN | 0.413912 | 3.177836 | 1.894936 | 0.001456 | 0.002788 | 0.001378 |
| mp_slice_2.test | RUN | 0.049360 | 0.077452 | 0.135766 | 0.000680 | 0.000799 | 0.001886 |
| mp_replace.test | RUN | 0.274264 | 1.674112 | 1.163477 | 0.002022 | 0.002364 | 0.002070 |
| mp_replace_if.test | RUN | 0.258932 | 1.349677 | 1.133886 | 0.002073 | 0.001692 | 0.001472 |
| mp_replace_if_q.test | RUN | 0.262235 | 1.443376 | 1.372247 | 0.001750 | 0.002158 | 0.002047 |
| mp_copy_if.test | RUN | 0.276520 | 2.075802 | 1.052603 | 0.001941 | 0.002800 | 0.001606 |
| mp_copy_if_q.test | RUN | 0.275380 | 2.014454 | 1.043632 | 0.002424 | 0.002647 | 0.002042 |
| mp_remove.test | RUN | 0.259787 | 1.378322 | 1.090867 | 0.002146 | 0.003685 | 0.002322 |
| mp_remove_if.test | RUN | 0.270456 | 2.193393 | 1.127863 | 0.001696 | 0.001710 | 0.001689 |
| mp_remove_if_q.test | RUN | 0.278735 | 2.139744 | 1.093400 | 0.001682 | 0.001646 | 0.001952 |
| mp_partition.test | RUN | 0.272520 | 2.098724 | 1.150237 | 0.001700 | 0.001676 | 0.001633 |
| mp_partition_q.test | RUN | 0.282500 | 2.008550 | 1.151030 | 0.001662 | 0.001687 | 0.001633 |
| mp_sort.test | RUN | 0.253619 | 1.437922 | 1.022689 | 0.001674 | 0.002188 | 0.001811 |
| mp_sort_q.test | RUN | 0.258195 | 1.457040 | 1.010772 | 0.001790 | 0.001983 | 0.001942 |
| mp_find.test | RUN | 0.256368 | 1.579341 | 1.005412 | 0.001644 | 0.002033 | 0.001632 |
| mp_find_if.test | RUN | 0.252635 | 1.474123 | 0.907705 | 0.001727 | 0.002012 | 0.001652 |
| mp_find_if_q.test | RUN | 0.271500 | 1.596813 | 1.053280 | 0.001721 | 0.002502 | 0.001512 |
| mp_reverse.test | RUN | 0.505612 | 4.188677 | 1.957316 | 0.001869 | 0.002415 | 0.001494 |
| mp_fold.test | RUN | 0.901114 | 3.379833 | 4.877349 | 0.002056 | 0.002158 | 0.001770 |
| mp_fold_q.test | RUN | 0.251571 | 1.401122 | 1.026601 | 0.001714 | 0.003646 | 0.001994 |
| mp_fold_q_sf.test | RUN | 0.233191 | 0.670205 | 0.778172 | 0.001673 | 0.001598 | 0.001872 |
| mp_reverse_fold.test | RUN | 0.419704 | 3.212715 | 1.424363 | 0.001723 | 0.002204 | 0.001756 |
| mp_reverse_fold_q.test | RUN | 0.274530 | 1.870294 | 1.057145 | 0.002324 | 0.002070 | 0.001693 |
| mp_unique.test | RUN | 0.257114 | 1.606928 | 1.010206 | 0.002243 | 0.001822 | 0.001550 |
| mp_unique_if.test | RUN | 0.432060 | 3.290213 | 1.587751 | 0.001646 | 0.002322 | 0.002188 |
| mp_unique_if_q.test | RUN | 0.397770 | 3.295296 | 1.327893 | 0.001451 | 0.001736 | 0.001678 |
| mp_all_of.test | RUN | 0.577548 | 1.739818 | 1.631632 | 0.001610 | 0.001942 | 0.001937 |
| mp_all_of_q.test | RUN | 0.237656 | 0.801442 | 0.802982 | 0.001662 | 0.002116 | 0.002395 |
| mp_any_of.test | RUN | 0.565009 | 1.780032 | 1.724868 | 0.001700 | 0.001672 | 0.001690 |
| mp_any_of_q.test | RUN | 0.241858 | 0.799225 | 0.807605 | 0.001692 | 0.002226 | 0.001878 |
| mp_none_of.test | RUN | 0.564506 | 1.769064 | 1.695256 | 0.001716 | 0.001643 | 0.001657 |
| mp_none_of_q.test | RUN | 0.225339 | 0.786226 | 0.813625 | 0.001516 | 0.002046 | 0.001552 |
| mp_replace_at.test | RUN | 0.276037 | 1.638852 | 1.148980 | 0.001712 | 0.002088 | 0.001974 |
| mp_replace_at_c.test | RUN | 0.286972 | 1.591903 | 1.166481 | 0.001657 | 0.001986 | 0.001715 |
| mp_for_each.test | RUN | 0.257322 | 1.139444 | 1.353164 | 0.001764 | 0.001755 | 0.001880 |
| mp_insert.test | RUN | 0.397125 | 3.179725 | 1.610156 | 0.001780 | 0.002140 | 0.001616 |
| mp_insert_2.test | RUN | 0.037444 | 0.072069 | 0.116454 | 0.000655 | 0.001456 | 0.001611 |
| mp_erase.test | RUN | 0.419955 | 3.397072 | 1.872837 | 0.001496 | 0.002362 | 0.002597 |
| mp_erase_2.test | RUN | 0.039110 | 0.073236 | 0.118046 | 0.000909 | 0.001000 | 0.001461 |
| mp_with_index.test | RUN | 0.601307 | 6.934184 | 4.452589 | 0.002214 | 0.002246 | 0.002056 |
| mp_with_index_cx.test | RUN | 0.003684 | 0.019956 | 0.037817 | 0.000910 | 0.000901 | 0.001670 |
| mp_from_sequence.test | RUN | 0.290100 | 1.884864 | 1.134673 | 0.001455 | 0.002098 | 0.001659 |
| mp_from_sequence_2.test | RUN | 0.286809 | 2.123054 | 1.254857 | 0.001730 | 0.001790 | 0.001968 |
| mp_from_sequence_3.test | RUN | 0.248240 | 0.970145 | 0.871726 | 0.001522 | 0.001661 | 0.001968 |
| mp_min_element.test | RUN | 0.235280 | 0.879315 | 0.871223 | 0.001737 | 0.002070 | 0.001974 |
| mp_min_element_2.test | RUN | 0.048114 | 0.073231 | 0.125055 | 0.000838 | 0.000937 | 0.001629 |
| mp_min_element_q.test | RUN | 0.234734 | 0.860829 | 0.859035 | 0.001758 | 0.002040 | 0.001726 |
| mp_min_element_q_2.test | RUN | 0.054326 | 0.072070 | 0.121535 | 0.000832 | 0.000686 | 0.001947 |
| mp_max_element.test | RUN | 0.242207 | 0.861701 | 0.848813 | 0.001956 | 0.001698 | 0.001935 |
| mp_max_element_2.test | RUN | 0.037060 | 0.070877 | 0.124143 | 0.000644 | 0.000923 | 0.001619 |
| mp_max_element_q.test | RUN | 0.242711 | 0.901658 | 0.805497 | 0.001750 | 0.001778 | 0.001974 |
| mp_max_element_q_2.test | RUN | 0.035404 | 0.071635 | 0.124205 | 0.000681 | 0.000989 | 0.001438 |
| mp_nth_element.test | RUN | 0.269247 | 1.240516 | 0.980918 | 0.001699 | 0.001668 | 0.002269 |
| mp_nth_element_q.test | RUN | 0.278334 | 1.224026 | 1.020959 | 0.001656 | 0.002218 | 0.001852 |
| mp_back.test | RUN | 0.262396 | 1.037841 | 0.924122 | 0.001866 | 0.001839 | 0.002370 |
| mp_back_2.test | RUN | 0.036261 | 0.071221 | 0.130203 | 0.000603 | 0.001101 | 0.001946 |
| mp_pop_back.test | RUN | 0.662976 | 6.431212 | 2.007856 | 0.002115 | 0.002178 | 0.002338 |
| mp_pop_back_2.test | RUN | 0.036023 | 0.070759 | 0.126604 | 0.000696 | 0.000623 | 0.001569 |
| mp_flatten.test | RUN | 0.306185 | 2.346705 | 1.275563 | 0.001972 | 0.003381 | 0.002070 |
| mp_rotate_left.test | RUN | 0.301404 | 2.506800 | 1.253811 | 0.001678 | 0.001678 | 0.001754 |
| mp_rotate_right.test | RUN | 0.303152 | 2.450874 | 1.303317 | 0.001768 | 0.002493 | 0.001912 |
| mp_power_set.test | RUN | 0.310160 | 2.358287 | 1.291913 | 0.001613 | 0.001708 | 0.002398 |
| mp_partial_sum.test | RUN | 0.377576 | 3.301316 | 1.461647 | 0.001608 | 0.001906 | 0.002821 |
| mp_iterate.test | RUN | 0.263900 | 0.976533 | 1.052993 | 0.002090 | 0.002270 | 0.002088 |
| mp_pairwise_fold.test | RUN | 0.298846 | 2.207934 | 1.371174 | 0.002030 | 0.001491 | 0.002809 |
| mp_pairwise_fold_q.test | RUN | 0.227497 | 0.665852 | 0.762775 | 0.001659 | 0.001902 | 0.002010 |
| mp_sliding_fold.test | RUN | 0.520565 | 4.181904 | 2.388348 | 0.001778 | 0.001650 | 0.001551 |
| mp_sliding_fold_q.test | RUN | 0.519028 | 4.178623 | 2.361695 | 0.002019 | 0.002190 | 0.002237 |
| mp_intersperse.test | RUN | 0.283489 | 1.583576 | 1.252480 | 0.001954 | 0.001724 | 0.002877 |
| mp_split.test | RUN | 0.375063 | 3.004868 | 1.601373 | 0.002683 | 0.001740 | 0.001477 |
| mp_join.test | RUN | 0.369290 | 2.540516 | 1.402604 | 0.001984 | 0.002020 | 0.001776 |
| integral.test | RUN | 0.239956 | 1.143831 | 0.884575 | 0.001783 | 0.002298 | 0.002010 |
| mp_value.test | RUN | 0.018624 | 0.037342 | 0.054072 | 0.000598 | 0.001386 | 0.001441 |
| mp_identity.test | RUN | 0.227770 | 1.023764 | 0.822967 | 0.001706 | 0.001812 | 0.001956 |
| mp_inherit.test | RUN | 0.225526 | 0.778236 | 0.802362 | 0.002082 | 0.001906 | 0.001597 |
| mp_if.test | RUN | 0.231764 | 0.868722 | 0.780594 | 0.001752 | 0.001814 | 0.001595 |
| mp_if_sf.test | RUN | 0.224924 | 0.637477 | 0.701590 | 0.001451 | 0.002361 | 0.001599 |
| mp_eval_if.test | RUN | 0.231185 | 0.901848 | 0.788958 | 0.001626 | 0.001935 | 0.001770 |
| mp_eval_if_sf.test | RUN | 0.226023 | 0.651359 | 0.717746 | 0.001581 | 0.001730 | 0.002130 |
| mp_valid.test | RUN | 0.227190 | 0.667327 | 0.787181 | 0.001716 | 0.001656 | 0.002018 |
| mp_defer.test | RUN | 0.249609 | 1.095294 | 0.919168 | 0.001828 | 0.002082 | 0.001622 |
| mp_quote.test | RUN | 0.245454 | 1.119544 | 0.894029 | 0.001666 | 0.001756 | 0.001716 |
| mp_invoke_q.test | RUN | 0.265032 | 1.480447 | 0.979078 | 0.001943 | 0.001967 | 0.001933 |
| mp_invoke_q_sf.test | RUN | 0.216510 | 0.659404 | 0.718875 | 0.001834 | 0.002068 | 0.001696 |
| mp_quote_trait.test | RUN | 0.242846 | 1.206757 | 0.857852 | 0.001956 | 0.002398 | 0.001544 |
| mp_cond.test | RUN | 0.236040 | 0.743056 | 0.778830 | 0.001828 | 0.001974 | 0.001760 |
| mp_cond_sf.test | RUN | 0.224577 | 0.683877 | 0.717370 | 0.002008 | 0.001938 | 0.001671 |
| mp_not_fn.test | RUN | 0.221984 | 0.648967 | 0.708723 | 0.001732 | 0.002030 | 0.001415 |
| mp_eval_if_not.test | RUN | 0.236491 | 0.939369 | 0.807379 | 0.001744 | 0.001898 | 0.001762 |
| mp_eval_or.test | RUN | 0.223119 | 0.520358 | 0.787423 | 0.002538 | 0.001902 | 0.001421 |
| mp_compose.test | RUN | 0.244540 | 1.490108 | 0.937391 | 0.001440 | 0.003033 | 0.001601 |
| mp_valid_and_true.test | RUN | 0.231418 | 0.787238 | 0.749163 | 0.002168 | 0.002006 | 0.001883 |
| mp_compose_sf.test | COMPILE | 0.057167 | 0.094445 | 0.189833 | — | — | — |
| integer_sequence.test | RUN | 0.275571 | 1.473585 | 1.139689 | 0.001669 | 0.002272 | 0.002034 |
| tuple_for_each.test | RUN | 0.232785 | 0.562908 | 0.789325 | 0.001628 | 0.001626 | 0.001796 |
| tuple_for_each_cx.test | COMPILE | 0.043590 | 0.084070 | 0.137576 | — | — | — |
| tuple_apply.test | RUN | 0.232269 | 0.598919 | 0.817486 | 0.001653 | 0.002406 | 0.002646 |
| tuple_apply_cx.test | COMPILE | 0.066377 | 0.114580 | 0.187122 | — | — | — |
| construct_from_tuple.test | RUN | 0.237965 | 0.618961 | 0.791414 | 0.001852 | 0.001829 | 0.002016 |
| construct_from_tuple_cx.test | COMPILE | 0.072090 | 0.113781 | 0.234480 | — | — | — |
| tuple_transform.test | RUN | 0.365389 | 1.278657 | 1.625101 | 0.001632 | 0.001922 | 0.001614 |
| tuple_transform_2.test | RUN | 0.317121 | 0.891122 | 1.345205 | 0.001777 | 0.002333 | 0.001708 |
| tuple_transform_cx.test | COMPILE | 0.057567 | 0.109757 | 0.217035 | — | — | — |
| mp_set_contains.test | RUN | 0.228530 | 0.808695 | 0.778545 | 0.002104 | 0.001761 | 0.001655 |
| mp_set_push_back.test | RUN | 0.305303 | 2.390517 | 1.430547 | 0.001940 | 0.002022 | 0.001688 |
| mp_set_push_front.test | RUN | 0.349525 | 2.887077 | 1.442460 | 0.001724 | 0.002214 | 0.001610 |
| mp_is_set.test | RUN | 0.231670 | 0.779198 | 0.794434 | 0.001670 | 0.001824 | 0.001894 |
| mp_set_union.test | RUN | 0.277709 | 2.016373 | 1.250122 | 0.001731 | 0.002010 | 0.002020 |
| mp_set_union_sf.test | RUN | 0.211538 | 0.643558 | 0.715314 | 0.001486 | 0.001827 | 0.001568 |
| mp_set_difference.test | RUN | 0.251597 | 1.601117 | 1.027323 | 0.001651 | 0.002348 | 0.002120 |
| mp_set_difference_sf.test | RUN | 0.216795 | 0.634047 | 0.741171 | 0.001888 | 0.002249 | 0.001838 |
| mp_set_intersection.test | RUN | 0.266060 | 1.603604 | 0.998888 | 0.001811 | 0.001690 | 0.002052 |
| mp_set_intersection_sf.test | RUN | 0.225630 | 0.650567 | 1.056359 | 0.001550 | 0.002323 | 0.001806 |
| mp_all.test | RUN | 0.239016 | 0.799287 | 0.753925 | 0.001646 | 0.001834 | 0.001762 |
| mp_all_2.test | RUN | 0.587717 | 1.619013 | 1.612013 | 0.001785 | 0.001732 | 0.001908 |
| mp_and.test | RUN | 0.235192 | 0.778025 | 0.745721 | 0.001904 | 0.002311 | 0.001666 |
| mp_any.test | RUN | 0.234112 | 0.806313 | 0.817797 | 0.001786 | 0.001856 | 0.002911 |
| mp_any_2.test | RUN | 0.565020 | 1.625440 | 1.632084 | 0.001668 | 0.001682 | 0.001836 |
| mp_or.test | RUN | 0.219912 | 0.781202 | 0.826937 | 0.002088 | 0.002428 | 0.001878 |
| mp_same.test | RUN | 0.404978 | 1.298671 | 1.192587 | 0.001529 | 0.001878 | 0.001590 |
| mp_plus.test | RUN | 0.227005 | 0.916361 | 0.879900 | 0.001618 | 0.001906 | 0.002320 |
| mp_less.test | RUN | 0.217821 | 0.772396 | 0.758555 | 0.001708 | 0.001718 | 0.002086 |
| mp_min.test | RUN | 0.215087 | 0.718493 | 0.734542 | 0.001728 | 0.001475 | 0.001720 |
| mp_max.test | RUN | 0.217505 | 0.820154 | 0.746935 | 0.001728 | 0.001915 | 0.001678 |
| mp_similar.test | RUN | 0.218134 | 0.792539 | 0.767392 | 0.001724 | 0.001928 | 0.001496 |
| mp_map_find.test | RUN | 0.251917 | 1.399865 | 1.032083 | 0.001837 | 0.001790 | 0.001595 |
| mp_map_find_2.test | RUN | 0.241254 | 0.570049 | 0.864597 | 0.001703 | 0.001908 | 0.002158 |
| mp_map_find_3.test | RUN | 0.232747 | 0.589276 | 0.870820 | 0.009509 | 0.001597 | 0.001642 |
| mp_map_find_4.test | RUN | 0.232097 | 0.529428 | 0.794102 | 0.001540 | 0.001817 | 0.001689 |
| mp_map_contains.test | RUN | 0.229226 | 0.789220 | 0.811092 | 0.002058 | 0.001950 | 0.001735 |
| mp_map_insert.test | RUN | 0.347626 | 2.647137 | 1.350635 | 0.002365 | 0.001985 | 0.001823 |
| mp_map_replace.test | RUN | 0.382374 | 2.968044 | 1.646975 | 0.001822 | 0.002027 | 0.001637 |
| mp_map_erase.test | RUN | 0.305188 | 2.263962 | 1.326593 | 0.002024 | 0.001909 | 0.001728 |
| mp_map_update.test | RUN | 0.277505 | 1.481434 | 1.008268 | 0.002184 | 0.001851 | 0.002034 |
| mp_map_update_q.test | RUN | 0.270336 | 1.449116 | 1.002157 | 0.001936 | 0.001951 | 0.001892 |
| mp_map_keys.test | RUN | 0.283300 | 1.312002 | 1.013455 | 0.001861 | 0.002394 | 0.001804 |
| mp_is_map.test | RUN | 0.248722 | 0.808579 | 0.787215 | 0.001488 | 0.002258 | 0.002635 |
| mp_bind.test | RUN | 0.324448 | 2.306215 | 1.154920 | 0.001873 | 0.001602 | 0.002399 |
| mp_bind_q.test | RUN | 0.266993 | 1.431152 | 0.951327 | 0.001730 | 0.002208 | 0.001625 |
| mp_bind_front.test | RUN | 0.257123 | 1.627852 | 0.924590 | 0.001766 | 0.002028 | 0.001690 |
| mp_bind_back.test | RUN | 0.260535 | 1.591922 | 0.964845 | 0.001668 | 0.001856 | 0.002127 |
| mp_lambda.test | RUN | 0.534962 | 4.618844 | 2.388617 | 0.001806 | 0.001809 | 0.002940 |
| mpl.test | RUN | 0.518444 | 1.259365 | 1.915177 | 0.002230 | 0.001619 | 0.002574 |
| mpl_list.test | RUN | 0.309122 | 0.933329 | 0.976751 | 0.002184 | 0.002187 | 0.001914 |
| mpl_tuple.test | RUN | 0.303852 | 0.942753 | 0.951707 | 0.001880 | 0.001678 | 0.001780 |
| version.test | RUN | 0.147837 | 0.356919 | 0.481658 | 0.001970 | 0.001914 | 0.001412 |
| mp11_version.test | RUN | 0.184988 | 0.411293 | 0.563706 | 0.001617 | 0.001871 | 0.002165 |
| list_version.test | RUN | 0.161789 | 0.363855 | 0.512775 | 0.002066 | 0.001706 | 0.001938 |
| algorithm_version.test | RUN | 0.166901 | 0.386459 | 0.524393 | 0.001834 | 0.002075 | 0.002098 |
| integral_version.test | RUN | 0.150636 | 0.359813 | 0.476653 | 0.001895 | 0.001934 | 0.001630 |
| utility_version.test | RUN | 0.142362 | 0.358057 | 0.489770 | 0.001855 | 0.001628 | 0.001974 |
| function_version.test | RUN | 0.146878 | 0.368563 | 0.504063 | 0.001484 | 0.002018 | 0.001894 |
| map_version.test | RUN | 0.159657 | 0.378318 | 0.544346 | 0.002058 | 0.001598 | 0.001594 |
| set_version.test | RUN | 0.158138 | 0.359997 | 0.492366 | 0.002165 | 0.002305 | 0.002592 |
| bind_version.test | RUN | 0.161704 | 0.381701 | 0.554893 | 0.001868 | 0.003196 | 0.002230 |
| integer_sequence_version.test | RUN | 0.146165 | 0.349180 | 0.492282 | 0.002346 | 0.001539 | 0.001788 |
| tuple_version.test | RUN | 0.165582 | 0.389579 | 0.523147 | 0.001985 | 0.002174 | 0.002166 |
| mpl_version.test | RUN | 0.175508 | 0.396998 | 0.589132 | 0.001868 | 0.001677 | 0.001946 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.999× | 0.976×, 1.022× | 0.998× |
| clang | 1.069× | 1.067×, 1.072× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 8 | 0.001733s | 0.001609s | 0.001586s | byte equality observed |
| bind_version.test | 8 | 0.001733s | 0.001588s | 0.001646s | byte equality observed |
| construct_from_tuple.test | 8 | 0.001727s | 0.001666s | 0.001640s | byte equality observed |
| function_version.test | 8 | 0.001760s | 0.001720s | 0.001592s | byte equality observed |
| integer_sequence.test | 7 | 0.001742s | 0.001606s | 0.001662s | byte equality observed |
| integer_sequence_version.test | 8 | 0.001677s | 0.001603s | 0.001681s | byte equality observed |
| integral.test | 8 | 0.001595s | 0.001643s | 0.001642s | byte equality observed |
| integral_version.test | 7 | 0.001711s | 0.001616s | 0.001622s | byte equality observed |
| list_version.test | 8 | 0.001747s | 0.001607s | 0.001625s | byte equality observed |
| map_version.test | 8 | 0.001561s | 0.001582s | 0.001609s | byte equality observed |
| mp11_version.test | 8 | 0.001645s | 0.001779s | 0.001579s | byte equality observed |
| mp_all.test | 9 | 0.001607s | 0.001739s | 0.001565s | byte equality observed |
| mp_all_2.test | 8 | 0.001582s | 0.001601s | 0.001650s | byte equality observed |
| mp_all_of.test | 5 | 0.001659s | 0.001679s | 0.001616s | byte equality observed |
| mp_all_of_q.test | 8 | 0.001742s | 0.001625s | 0.001539s | byte equality observed |
| mp_and.test | 7 | 0.001620s | 0.001659s | 0.001600s | byte equality observed |
| mp_any.test | 7 | 0.001632s | 0.001716s | 0.001561s | byte equality observed |
| mp_any_2.test | 8 | 0.001581s | 0.001658s | 0.001567s | byte equality observed |
| mp_any_of.test | 8 | 0.001603s | 0.001648s | 0.001571s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001564s | 0.001713s | 0.001597s | byte equality observed |
| mp_append.test | 10 | 0.001667s | 0.001604s | 0.001638s | byte equality observed |
| mp_append_2.test | 8 | 0.001764s | 0.001636s | 0.001651s | byte equality observed |
| mp_append_3.test | 9 | 0.000811s | 0.000854s | 0.001586s | byte equality observed |
| mp_append_sf.test | 5 | 0.001652s | 0.001623s | 0.001578s | byte equality observed |
| mp_apply_q.test | 9 | 0.001610s | 0.001626s | 0.001603s | byte equality observed |
| mp_apply_q_sf.test | 8 | 0.001586s | 0.001577s | 0.001619s | byte equality observed |
| mp_assign.test | 7 | 0.001618s | 0.001626s | 0.001610s | byte equality observed |
| mp_assign_2.test | 9 | 0.000823s | 0.000807s | 0.001572s | byte equality observed |
| mp_at.test | 7 | 0.001684s | 0.001594s | 0.001605s | byte equality observed |
| mp_at_2.test | 8 | 0.000832s | 0.000851s | 0.001593s | byte equality observed |
| mp_at_sf.test | 7 | 0.001624s | 0.001630s | 0.001589s | byte equality observed |
| mp_back.test | 7 | 0.001630s | 0.001600s | 0.001602s | byte equality observed |
| mp_back_2.test | 9 | 0.000805s | 0.000844s | 0.001576s | byte equality observed |
| mp_bind.test | 8 | 0.001585s | 0.001743s | 0.001653s | byte equality observed |
| mp_bind_back.test | 8 | 0.001691s | 0.001682s | 0.001599s | byte equality observed |
| mp_bind_front.test | 7 | 0.001635s | 0.001712s | 0.001615s | byte equality observed |
| mp_bind_q.test | 8 | 0.001582s | 0.001610s | 0.001607s | byte equality observed |
| mp_clear.test | 5 | 0.001598s | 0.001558s | 0.001608s | byte equality observed |
| mp_clear_2.test | 8 | 0.000805s | 0.000824s | 0.001575s | byte equality observed |
| mp_compose.test | 9 | 0.001566s | 0.001634s | 0.001615s | byte equality observed |
| mp_cond.test | 8 | 0.001583s | 0.001641s | 0.001628s | byte equality observed |
| mp_cond_sf.test | 8 | 0.001650s | 0.001624s | 0.001605s | byte equality observed |
| mp_contains.test | 6 | 0.001602s | 0.001924s | 0.001605s | byte equality observed |
| mp_copy_if.test | 6 | 0.001563s | 0.001632s | 0.001553s | byte equality observed |
| mp_copy_if_q.test | 7 | 0.001582s | 0.001631s | 0.001588s | byte equality observed |
| mp_count.test | 7 | 0.001564s | 0.001643s | 0.001594s | byte equality observed |
| mp_count_if.test | 7 | 0.001573s | 0.001749s | 0.001594s | byte equality observed |
| mp_count_if_q.test | 5 | 0.001584s | 0.001603s | 0.001552s | byte equality observed |
| mp_defer.test | 5 | 0.001578s | 0.001634s | 0.001616s | byte equality observed |
| mp_drop.test | 7 | 0.001594s | 0.001772s | 0.001585s | byte equality observed |
| mp_drop_2.test | 8 | 0.000805s | 0.000792s | 0.001545s | byte equality observed |
| mp_empty.test | 8 | 0.001577s | 0.001601s | 0.001534s | byte equality observed |
| mp_empty_2.test | 6 | 0.000814s | 0.000804s | 0.001530s | byte equality observed |
| mp_erase.test | 7 | 0.001610s | 0.001580s | 0.001592s | byte equality observed |
| mp_erase_2.test | 8 | 0.000835s | 0.000839s | 0.001557s | byte equality observed |
| mp_eval_if.test | 7 | 0.001568s | 0.001703s | 0.001596s | byte equality observed |
| mp_eval_if_not.test | 7 | 0.001634s | 0.001769s | 0.001608s | byte equality observed |
| mp_eval_if_sf.test | 8 | 0.001560s | 0.001686s | 0.001572s | byte equality observed |
| mp_eval_or.test | 8 | 0.001590s | 0.001586s | 0.001544s | byte equality observed |
| mp_fill.test | 8 | 0.001546s | 0.001613s | 0.001627s | byte equality observed |
| mp_fill_2.test | 8 | 0.000793s | 0.000802s | 0.001548s | byte equality observed |
| mp_filter.test | 7 | 0.001620s | 0.001618s | 0.001539s | byte equality observed |
| mp_find.test | 6 | 0.001623s | 0.001674s | 0.001573s | byte equality observed |
| mp_find_if.test | 9 | 0.001633s | 0.001715s | 0.001585s | byte equality observed |
| mp_find_if_q.test | 7 | 0.001622s | 0.001762s | 0.001591s | byte equality observed |
| mp_flatten.test | 8 | 0.001613s | 0.001669s | 0.001604s | byte equality observed |
| mp_fold.test | 8 | 0.001613s | 0.001612s | 0.001584s | byte equality observed |
| mp_fold_q.test | 8 | 0.001586s | 0.001641s | 0.001585s | byte equality observed |
| mp_fold_q_sf.test | 8 | 0.001615s | 0.001607s | 0.001577s | byte equality observed |
| mp_for_each.test | 8 | 0.001559s | 0.001678s | 0.001601s | byte equality observed |
| mp_from_sequence.test | 8 | 0.001593s | 0.001806s | 0.001569s | byte equality observed |
| mp_from_sequence_2.test | 8 | 0.001595s | 0.001701s | 0.001565s | byte equality observed |
| mp_from_sequence_3.test | 8 | 0.001524s | 0.001647s | 0.001599s | byte equality observed |
| mp_front.test | 8 | 0.001601s | 0.001671s | 0.001613s | byte equality observed |
| mp_front_2.test | 9 | 0.000787s | 0.000768s | 0.001535s | byte equality observed |
| mp_identity.test | 8 | 0.001586s | 0.001568s | 0.001554s | byte equality observed |
| mp_if.test | 8 | 0.001578s | 0.001667s | 0.001536s | byte equality observed |
| mp_if_sf.test | 9 | 0.001580s | 0.001607s | 0.001572s | byte equality observed |
| mp_inherit.test | 8 | 0.001623s | 0.001865s | 0.001591s | byte equality observed |
| mp_insert.test | 9 | 0.001587s | 0.001724s | 0.001610s | byte equality observed |
| mp_insert_2.test | 8 | 0.000852s | 0.000818s | 0.001544s | byte equality observed |
| mp_intersperse.test | 8 | 0.001623s | 0.001605s | 0.001578s | byte equality observed |
| mp_invoke_q.test | 8 | 0.001553s | 0.001644s | 0.001600s | byte equality observed |
| mp_invoke_q_sf.test | 7 | 0.001597s | 0.001683s | 0.001606s | byte equality observed |
| mp_iota.test | 7 | 0.001562s | 0.001671s | 0.001554s | byte equality observed |
| mp_iota_2.test | 7 | 0.001672s | 0.001553s | 0.001588s | byte equality observed |
| mp_is_list.test | 6 | 0.001590s | 0.001829s | 0.001673s | byte equality observed |
| mp_is_map.test | 8 | 0.001590s | 0.001614s | 0.001590s | byte equality observed |
| mp_is_set.test | 8 | 0.001558s | 0.001592s | 0.001528s | byte equality observed |
| mp_is_value_list.test | 7 | 0.001632s | 0.001596s | 0.001597s | byte equality observed |
| mp_iterate.test | 8 | 0.001586s | 0.001621s | 0.001572s | byte equality observed |
| mp_join.test | 8 | 0.001635s | 0.001571s | 0.001641s | byte equality observed |
| mp_lambda.test | 8 | 0.001629s | 0.001760s | 0.001710s | byte equality observed |
| mp_less.test | 7 | 0.001683s | 0.001687s | 0.001594s | byte equality observed |
| mp_list_c.test | 9 | 0.001590s | 0.001690s | 0.001587s | byte equality observed |
| mp_list_v.test | 7 | 0.000793s | 0.000851s | 0.001624s | byte equality observed |
| mp_map_contains.test | 8 | 0.001556s | 0.001620s | 0.001712s | byte equality observed |
| mp_map_erase.test | 7 | 0.001629s | 0.001682s | 0.001541s | byte equality observed |
| mp_map_find.test | 6 | 0.001607s | 0.001645s | 0.001599s | byte equality observed |
| mp_map_find_2.test | 7 | 0.001582s | 0.001622s | 0.001674s | byte equality observed |
| mp_map_find_3.test | 8 | 0.001686s | 0.001837s | 0.001646s | byte equality observed |
| mp_map_find_4.test | 7 | 0.001596s | 0.001661s | 0.001599s | byte equality observed |
| mp_map_insert.test | 8 | 0.001695s | 0.001644s | 0.001610s | byte equality observed |
| mp_map_keys.test | 8 | 0.001626s | 0.001676s | 0.001654s | byte equality observed |
| mp_map_replace.test | 7 | 0.001698s | 0.001673s | 0.001594s | byte equality observed |
| mp_map_update.test | 8 | 0.001595s | 0.001646s | 0.001656s | byte equality observed |
| mp_map_update_q.test | 8 | 0.001677s | 0.001560s | 0.001643s | byte equality observed |
| mp_max.test | 8 | 0.001612s | 0.001658s | 0.001583s | byte equality observed |
| mp_max_element.test | 10 | 0.001591s | 0.001827s | 0.001625s | byte equality observed |
| mp_max_element_2.test | 9 | 0.000812s | 0.000804s | 0.001608s | byte equality observed |
| mp_max_element_q.test | 8 | 0.001620s | 0.001596s | 0.001598s | byte equality observed |
| mp_max_element_q_2.test | 8 | 0.000805s | 0.000847s | 0.001626s | byte equality observed |
| mp_min.test | 7 | 0.001616s | 0.001605s | 0.001565s | byte equality observed |
| mp_min_element.test | 6 | 0.001579s | 0.001626s | 0.001579s | byte equality observed |
| mp_min_element_2.test | 7 | 0.000821s | 0.000815s | 0.001569s | byte equality observed |
| mp_min_element_q.test | 9 | 0.001593s | 0.001655s | 0.001594s | byte equality observed |
| mp_min_element_q_2.test | 9 | 0.000798s | 0.000829s | 0.001574s | byte equality observed |
| mp_none_of.test | 8 | 0.001689s | 0.001731s | 0.001556s | byte equality observed |
| mp_none_of_q.test | 8 | 0.001571s | 0.001636s | 0.001582s | byte equality observed |
| mp_not_fn.test | 8 | 0.001564s | 0.001713s | 0.001601s | byte equality observed |
| mp_nth_element.test | 10 | 0.001667s | 0.001615s | 0.001606s | byte equality observed |
| mp_nth_element_q.test | 9 | 0.001579s | 0.001657s | 0.001646s | byte equality observed |
| mp_or.test | 9 | 0.001696s | 0.001651s | 0.001577s | byte equality observed |
| mp_pairwise_fold.test | 5 | 0.001653s | 0.001552s | 0.001645s | byte equality observed |
| mp_pairwise_fold_q.test | 9 | 0.001597s | 0.001761s | 0.001591s | byte equality observed |
| mp_partial_sum.test | 9 | 0.001627s | 0.001625s | 0.001609s | byte equality observed |
| mp_partition.test | 8 | 0.001622s | 0.001725s | 0.001624s | byte equality observed |
| mp_partition_q.test | 9 | 0.001736s | 0.001609s | 0.001629s | byte equality observed |
| mp_plus.test | 8 | 0.001657s | 0.001677s | 0.001656s | byte equality observed |
| mp_pop_back.test | 7 | 0.001702s | 0.001652s | 0.001621s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000811s | 0.000827s | 0.001585s | byte equality observed |
| mp_pop_front.test | 7 | 0.001575s | 0.001689s | 0.001644s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000788s | 0.000850s | 0.001641s | byte equality observed |
| mp_power_set.test | 8 | 0.001600s | 0.001739s | 0.001668s | byte equality observed |
| mp_product.test | 7 | 0.001609s | 0.001589s | 0.001584s | byte equality observed |
| mp_push_back.test | 9 | 0.001717s | 0.001671s | 0.001661s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000810s | 0.000833s | 0.001558s | byte equality observed |
| mp_push_front.test | 7 | 0.001606s | 0.001652s | 0.001594s | byte equality observed |
| mp_push_front_2.test | 7 | 0.000794s | 0.000818s | 0.001606s | byte equality observed |
| mp_quote.test | 8 | 0.001571s | 0.001597s | 0.001645s | byte equality observed |
| mp_quote_trait.test | 7 | 0.001567s | 0.001764s | 0.001578s | byte equality observed |
| mp_remove.test | 7 | 0.001624s | 0.001587s | 0.001619s | byte equality observed |
| mp_remove_if.test | 10 | 0.001595s | 0.001631s | 0.001655s | byte equality observed |
| mp_remove_if_q.test | 8 | 0.001609s | 0.001697s | 0.001562s | byte equality observed |
| mp_rename.test | 7 | 0.001642s | 0.001637s | 0.001587s | byte equality observed |
| mp_rename_2.test | 10 | 0.000830s | 0.000859s | 0.001554s | byte equality observed |
| mp_rename_v.test | 10 | 0.000808s | 0.000811s | 0.001611s | byte equality observed |
| mp_repeat.test | 7 | 0.001641s | 0.001757s | 0.001594s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000812s | 0.000831s | 0.001646s | byte equality observed |
| mp_repeat_3.test | 8 | 0.001601s | 0.001590s | 0.001619s | byte equality observed |
| mp_replace.test | 8 | 0.001550s | 0.001705s | 0.001608s | byte equality observed |
| mp_replace_at.test | 8 | 0.001569s | 0.001771s | 0.001646s | byte equality observed |
| mp_replace_at_c.test | 8 | 0.001586s | 0.001669s | 0.001579s | byte equality observed |
| mp_replace_front.test | 6 | 0.001598s | 0.001636s | 0.001587s | byte equality observed |
| mp_replace_front_2.test | 9 | 0.000788s | 0.000811s | 0.001717s | byte equality observed |
| mp_replace_if.test | 7 | 0.001574s | 0.001683s | 0.001594s | byte equality observed |
| mp_replace_if_q.test | 8 | 0.001614s | 0.001609s | 0.001622s | byte equality observed |
| mp_replace_second.test | 6 | 0.001561s | 0.001600s | 0.001596s | byte equality observed |
| mp_replace_second_2.test | 8 | 0.000812s | 0.000840s | 0.001540s | byte equality observed |
| mp_replace_third.test | 7 | 0.001587s | 0.001609s | 0.001622s | byte equality observed |
| mp_replace_third_2.test | 7 | 0.000814s | 0.000859s | 0.001542s | byte equality observed |
| mp_reverse.test | 6 | 0.001665s | 0.001634s | 0.001557s | byte equality observed |
| mp_reverse_fold.test | 7 | 0.001615s | 0.001713s | 0.001576s | byte equality observed |
| mp_reverse_fold_q.test | 7 | 0.001613s | 0.001581s | 0.001621s | byte equality observed |
| mp_rotate_left.test | 6 | 0.001616s | 0.001612s | 0.001662s | byte equality observed |
| mp_rotate_right.test | 8 | 0.001620s | 0.001730s | 0.001555s | byte equality observed |
| mp_same.test | 8 | 0.001595s | 0.001629s | 0.001716s | byte equality observed |
| mp_second.test | 7 | 0.001617s | 0.001658s | 0.001595s | byte equality observed |
| mp_second_2.test | 8 | 0.000807s | 0.000847s | 0.001607s | byte equality observed |
| mp_set_contains.test | 7 | 0.001586s | 0.001640s | 0.001575s | byte equality observed |
| mp_set_difference.test | 7 | 0.001573s | 0.001670s | 0.001642s | byte equality observed |
| mp_set_difference_sf.test | 7 | 0.001620s | 0.001602s | 0.001578s | byte equality observed |
| mp_set_intersection.test | 7 | 0.001557s | 0.001607s | 0.001570s | byte equality observed |
| mp_set_intersection_sf.test | 7 | 0.001676s | 0.001575s | 0.001577s | byte equality observed |
| mp_set_push_back.test | 9 | 0.001630s | 0.001722s | 0.001588s | byte equality observed |
| mp_set_push_front.test | 7 | 0.001646s | 0.001618s | 0.001687s | byte equality observed |
| mp_set_union.test | 7 | 0.001628s | 0.001576s | 0.001579s | byte equality observed |
| mp_set_union_sf.test | 9 | 0.001588s | 0.001623s | 0.001607s | byte equality observed |
| mp_similar.test | 7 | 0.001668s | 0.001700s | 0.001633s | byte equality observed |
| mp_size.test | 8 | 0.001595s | 0.001601s | 0.001621s | byte equality observed |
| mp_size_2.test | 8 | 0.000822s | 0.000795s | 0.001592s | byte equality observed |
| mp_slice.test | 8 | 0.001618s | 0.001635s | 0.001681s | byte equality observed |
| mp_slice_2.test | 9 | 0.000785s | 0.000883s | 0.001638s | byte equality observed |
| mp_sliding_fold.test | 8 | 0.001671s | 0.001645s | 0.001604s | byte equality observed |
| mp_sliding_fold_q.test | 7 | 0.001640s | 0.001651s | 0.001578s | byte equality observed |
| mp_sort.test | 8 | 0.001552s | 0.001608s | 0.001578s | byte equality observed |
| mp_sort_q.test | 8 | 0.001645s | 0.001669s | 0.001570s | byte equality observed |
| mp_split.test | 8 | 0.001641s | 0.001620s | 0.001637s | byte equality observed |
| mp_starts_with.test | 6 | 0.001635s | 0.001623s | 0.001741s | byte equality observed |
| mp_starts_with_sf.test | 8 | 0.001593s | 0.001599s | 0.001595s | byte equality observed |
| mp_take.test | 9 | 0.001590s | 0.001763s | 0.001584s | byte equality observed |
| mp_take_2.test | 9 | 0.000826s | 0.000820s | 0.001626s | byte equality observed |
| mp_third.test | 7 | 0.001670s | 0.001617s | 0.001605s | byte equality observed |
| mp_third_2.test | 9 | 0.000825s | 0.000843s | 0.001624s | byte equality observed |
| mp_transform.test | 7 | 0.001663s | 0.001687s | 0.001595s | byte equality observed |
| mp_transform_2.test | 8 | 0.001522s | 0.001651s | 0.001699s | byte equality observed |
| mp_transform_front.test | 8 | 0.001665s | 0.001654s | 0.001585s | byte equality observed |
| mp_transform_front_2.test | 9 | 0.000796s | 0.000838s | 0.001545s | byte equality observed |
| mp_transform_if.test | 8 | 0.001598s | 0.001703s | 0.001599s | byte equality observed |
| mp_transform_if_q.test | 8 | 0.001630s | 0.001610s | 0.001687s | byte equality observed |
| mp_transform_q.test | 8 | 0.001674s | 0.001666s | 0.001564s | byte equality observed |
| mp_transform_q_2.test | 7 | 0.001653s | 0.001608s | 0.001627s | byte equality observed |
| mp_transform_second.test | 8 | 0.001622s | 0.001694s | 0.001684s | byte equality observed |
| mp_transform_second_2.test | 9 | 0.000830s | 0.000816s | 0.001555s | byte equality observed |
| mp_transform_sf.test | 7 | 0.001624s | 0.001601s | 0.001588s | byte equality observed |
| mp_transform_third.test | 7 | 0.001646s | 0.001651s | 0.001581s | byte equality observed |
| mp_transform_third_2.test | 9 | 0.000813s | 0.000879s | 0.001605s | byte equality observed |
| mp_unique.test | 7 | 0.001739s | 0.001603s | 0.001678s | byte equality observed |
| mp_unique_if.test | 6 | 0.001648s | 0.001692s | 0.001589s | byte equality observed |
| mp_unique_if_q.test | 7 | 0.001618s | 0.001662s | 0.001584s | byte equality observed |
| mp_valid.test | 8 | 0.001599s | 0.001622s | 0.001628s | byte equality observed |
| mp_valid_and_true.test | 6 | 0.001586s | 0.001612s | 0.001671s | byte equality observed |
| mp_value.test | 10 | 0.000810s | 0.000800s | 0.001653s | byte equality observed |
| mp_with_index.test | 6 | 0.002025s | 0.002033s | 0.001973s | byte equality observed |
| mp_with_index_cx.test | 7 | 0.000805s | 0.000805s | 0.001573s | byte equality observed |
| mpl.test | 5 | 0.001693s | 0.001592s | 0.001702s | byte equality observed |
| mpl_list.test | 9 | 0.001641s | 0.001653s | 0.001565s | byte equality observed |
| mpl_tuple.test | 8 | 0.001574s | 0.001702s | 0.001632s | byte equality observed |
| mpl_version.test | 7 | 0.001582s | 0.001639s | 0.001589s | byte equality observed |
| set_version.test | 7 | 0.001597s | 0.001588s | 0.001577s | byte equality observed |
| tuple_apply.test | 7 | 0.001581s | 0.001666s | 0.001603s | byte equality observed |
| tuple_for_each.test | 8 | 0.001651s | 0.001620s | 0.001569s | byte equality observed |
| tuple_transform.test | 7 | 0.001614s | 0.001590s | 0.001621s | byte equality observed |
| tuple_transform_2.test | 8 | 0.001618s | 0.001585s | 0.001623s | byte equality observed |
| tuple_version.test | 9 | 0.001633s | 0.001617s | 0.001558s | byte equality observed |
| utility_version.test | 7 | 0.001617s | 0.001654s | 0.001585s | byte equality observed |
| version.test | 6 | 0.001604s | 0.001609s | 0.001613s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
