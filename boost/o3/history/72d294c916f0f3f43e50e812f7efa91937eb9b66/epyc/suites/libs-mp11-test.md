# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.048116 | 0.100876 | 0.169638 | — | — | — |
| complex_h.test | COMPILE | 0.200901 | 0.434753 | 0.717842 | — | — | — |
| mp_size.test | RUN | 0.236670 | 1.326953 | 0.957331 | 0.002010 | 0.001926 | 0.001854 |
| mp_size_2.test | RUN | 0.018720 | 0.047321 | 0.073285 | 0.000790 | 0.000845 | 0.001860 |
| mp_empty.test | RUN | 0.226199 | 0.805176 | 0.880021 | 0.001818 | 0.002130 | 0.001723 |
| mp_empty_2.test | RUN | 0.020946 | 0.046833 | 0.081711 | 0.000811 | 0.000795 | 0.002414 |
| mp_assign.test | RUN | 0.239780 | 1.690172 | 1.093541 | 0.001839 | 0.002034 | 0.002040 |
| mp_assign_2.test | RUN | 0.020697 | 0.051013 | 0.081070 | 0.000771 | 0.000870 | 0.001858 |
| mp_clear.test | RUN | 0.219316 | 0.736538 | 0.940345 | 0.001964 | 0.001968 | 0.001786 |
| mp_clear_2.test | RUN | 0.020414 | 0.051629 | 0.070253 | 0.000777 | 0.000872 | 0.001940 |
| mp_front.test | RUN | 0.238449 | 1.151391 | 0.992794 | 0.001878 | 0.002099 | 0.001837 |
| mp_front_2.test | RUN | 0.020413 | 0.048215 | 0.078531 | 0.000719 | 0.000659 | 0.001763 |
| mp_pop_front.test | RUN | 0.230012 | 1.238525 | 0.951275 | 0.001806 | 0.001772 | 0.001473 |
| mp_pop_front_2.test | RUN | 0.020637 | 0.046159 | 0.081851 | 0.000755 | 0.000903 | 0.001754 |
| mp_second.test | RUN | 0.228305 | 0.794828 | 0.949908 | 0.001829 | 0.001778 | 0.001920 |
| mp_second_2.test | RUN | 0.019951 | 0.049862 | 0.068491 | 0.000735 | 0.000839 | 0.002560 |
| mp_third.test | RUN | 0.216149 | 0.725915 | 0.914006 | 0.001944 | 0.001748 | 0.001702 |
| mp_third_2.test | RUN | 0.021583 | 0.049369 | 0.076869 | 0.000756 | 0.000840 | 0.001702 |
| mp_push_front.test | RUN | 0.293354 | 2.540330 | 1.554026 | 0.001958 | 0.001934 | 0.002079 |
| mp_push_front_2.test | RUN | 0.022053 | 0.047031 | 0.078061 | 0.000731 | 0.000753 | 0.001678 |
| mp_push_back.test | RUN | 0.289854 | 2.536445 | 1.508835 | 0.001834 | 0.001760 | 0.001753 |
| mp_push_back_2.test | RUN | 0.020247 | 0.049258 | 0.074700 | 0.000815 | 0.000799 | 0.001954 |
| mp_rename.test | RUN | 0.283443 | 2.342324 | 1.291707 | 0.001906 | 0.002042 | 0.001551 |
| mp_rename_2.test | RUN | 0.020381 | 0.051638 | 0.082360 | 0.000704 | 0.000900 | 0.002015 |
| mp_append.test | RUN | 0.292645 | 2.643401 | 1.506592 | 0.001814 | 0.002120 | 0.001876 |
| mp_append_2.test | RUN | 0.606985 | 5.923258 | 1.918100 | 0.001810 | 0.001748 | 0.001581 |
| mp_append_3.test | RUN | 0.021672 | 0.048539 | 0.070781 | 0.000903 | 0.000803 | 0.002089 |
| mp_append_sf.test | RUN | 0.211273 | 0.693075 | 0.785183 | 0.001850 | 0.001954 | 0.001778 |
| mp_replace_front.test | RUN | 0.261809 | 1.867648 | 1.233083 | 0.001979 | 0.002208 | 0.001967 |
| mp_replace_front_2.test | RUN | 0.019504 | 0.050747 | 0.068269 | 0.000763 | 0.000758 | 0.001561 |
| mp_replace_second.test | RUN | 0.252231 | 1.232264 | 1.101325 | 0.001908 | 0.001944 | 0.002041 |
| mp_replace_second_2.test | RUN | 0.020257 | 0.048073 | 0.078134 | 0.000684 | 0.000804 | 0.002027 |
| mp_replace_third.test | RUN | 0.246627 | 1.002822 | 1.018097 | 0.001980 | 0.002210 | 0.002004 |
| mp_replace_third_2.test | RUN | 0.020861 | 0.048039 | 0.074744 | 0.000640 | 0.000748 | 0.002052 |
| mp_apply_q.test | RUN | 0.271216 | 2.322804 | 1.293585 | 0.001820 | 0.001940 | 0.001657 |
| mp_apply_q_sf.test | RUN | 0.216797 | 0.692772 | 0.860647 | 0.001842 | 0.002033 | 0.001823 |
| mp_is_list.test | RUN | 0.221117 | 0.806367 | 0.837911 | 0.001735 | 0.002008 | 0.001846 |
| mp_list_c.test | RUN | 0.230672 | 0.973877 | 0.960128 | 0.001816 | 0.001972 | 0.001680 |
| mp_transform_front.test | RUN | 0.255109 | 1.878476 | 1.086616 | 0.001900 | 0.001905 | 0.002086 |
| mp_transform_front_2.test | RUN | 0.020871 | 0.046182 | 0.081489 | 0.000733 | 0.000790 | 0.001923 |
| mp_transform_second.test | RUN | 0.249603 | 1.632033 | 1.053895 | 0.001805 | 0.001756 | 0.001753 |
| mp_transform_second_2.test | RUN | 0.020832 | 0.046092 | 0.082348 | 0.000782 | 0.000878 | 0.001923 |
| mp_transform_third.test | RUN | 0.232043 | 1.249987 | 0.961391 | 0.001928 | 0.001900 | 0.002142 |
| mp_transform_third_2.test | RUN | 0.020737 | 0.048506 | 0.079472 | 0.000843 | 0.000808 | 0.001818 |
| mp_list_v.test | RUN | 0.020065 | 0.048290 | 0.072587 | 0.000728 | 0.000686 | 0.001928 |
| mp_rename_v.test | RUN | 0.020722 | 0.048717 | 0.066761 | 0.000776 | 0.000953 | 0.001540 |
| mp_is_value_list.test | RUN | 0.210167 | 0.523380 | 0.828347 | 0.001938 | 0.002058 | 0.001857 |
| mp_transform.test | RUN | 0.492008 | 3.937049 | 2.264177 | 0.001908 | 0.002042 | 0.002026 |
| mp_transform_2.test | RUN | 0.208288 | 0.467764 | 0.814781 | 0.001906 | 0.002135 | 0.001548 |
| mp_transform_q.test | RUN | 0.463377 | 3.709851 | 2.214563 | 0.001788 | 0.002084 | 0.002072 |
| mp_transform_q_2.test | RUN | 0.545513 | 1.243599 | 2.948302 | 0.001902 | 0.002040 | 0.001583 |
| mp_transform_sf.test | RUN | 0.236006 | 0.708177 | 0.929517 | 0.001939 | 0.001932 | 0.002033 |
| mp_transform_if.test | RUN | 0.276548 | 1.898354 | 1.292496 | 0.001711 | 0.001841 | 0.001970 |
| mp_transform_if_q.test | RUN | 0.282118 | 1.909537 | 1.323351 | 0.001843 | 0.001810 | 0.001445 |
| mp_filter.test | RUN | 0.251977 | 1.149332 | 0.988488 | 0.001992 | 0.002342 | 0.002032 |
| mp_fill.test | RUN | 0.239183 | 0.879166 | 0.964277 | 0.001849 | 0.002049 | 0.001970 |
| mp_fill_2.test | RUN | 0.037506 | 0.074449 | 0.137058 | 0.000800 | 0.000869 | 0.002100 |
| mp_count.test | RUN | 0.455787 | 1.994461 | 1.441661 | 0.001977 | 0.002004 | 0.001676 |
| mp_count_if.test | RUN | 0.449752 | 2.023541 | 1.676684 | 0.001782 | 0.001799 | 0.001695 |
| mp_count_if_q.test | RUN | 0.252341 | 1.387443 | 0.936415 | 0.001829 | 0.002092 | 0.002174 |
| mp_contains.test | RUN | 0.236887 | 0.848740 | 0.832951 | 0.001956 | 0.001844 | 0.001815 |
| mp_starts_with.test | RUN | 0.231894 | 0.829271 | 0.968171 | 0.001810 | 0.002042 | 0.001902 |
| mp_starts_with_sf.test | RUN | 0.225126 | 0.722759 | 0.926484 | 0.001722 | 0.001952 | 0.001598 |
| mp_repeat.test | RUN | 0.327622 | 3.385500 | 1.563589 | 0.002013 | 0.001852 | 0.001853 |
| mp_repeat_2.test | RUN | 0.034888 | 0.081614 | 0.146870 | 0.000754 | 0.000808 | 0.001968 |
| mp_repeat_3.test | RUN | 0.284323 | 0.457311 | 0.865902 | 0.001726 | 0.002142 | 0.001616 |
| mp_product.test | RUN | 0.260991 | 2.030803 | 1.269506 | 0.001817 | 0.002043 | 0.001960 |
| mp_drop.test | RUN | 0.288315 | 2.530114 | 1.388256 | 0.001752 | 0.001787 | 0.002142 |
| mp_drop_2.test | RUN | 0.038274 | 0.081308 | 0.128339 | 0.000686 | 0.000897 | 0.002020 |
| mp_iota.test | RUN | 0.272281 | 2.140722 | 1.216904 | 0.001842 | 0.001900 | 0.001991 |
| mp_iota_2.test | RUN | 0.292701 | 2.635522 | 1.401367 | 0.001635 | 0.002069 | 0.001868 |
| mp_at.test | RUN | 0.243526 | 1.140913 | 0.998158 | 0.001816 | 0.001902 | 0.002152 |
| mp_at_2.test | RUN | 0.036820 | 0.075342 | 0.142828 | 0.000761 | 0.000691 | 0.001950 |
| mp_at_sf.test | RUN | 0.227570 | 0.719927 | 0.883806 | 0.001664 | 0.002026 | 0.001637 |
| mp_take.test | RUN | 0.374013 | 3.314693 | 1.645250 | 0.002163 | 0.001480 | 0.002062 |
| mp_take_2.test | RUN | 0.037165 | 0.076752 | 0.144588 | 0.000790 | 0.000843 | 0.001916 |
| mp_slice.test | RUN | 0.430112 | 3.380630 | 2.085762 | 0.002332 | 0.002074 | 0.001926 |
| mp_slice_2.test | RUN | 0.037957 | 0.077013 | 0.139804 | 0.000820 | 0.000836 | 0.002004 |
| mp_replace.test | RUN | 0.282643 | 1.708586 | 1.345837 | 0.002007 | 0.001968 | 0.001949 |
| mp_replace_if.test | RUN | 0.272902 | 1.529100 | 1.235782 | 0.001933 | 0.002104 | 0.002253 |
| mp_replace_if_q.test | RUN | 0.273627 | 1.514755 | 1.231315 | 0.001995 | 0.002112 | 0.002041 |
| mp_copy_if.test | RUN | 0.277011 | 2.117504 | 1.178999 | 0.001633 | 0.002132 | 0.001972 |
| mp_copy_if_q.test | RUN | 0.277234 | 2.152493 | 1.215665 | 0.001980 | 0.002001 | 0.001594 |
| mp_remove.test | RUN | 0.273587 | 1.426335 | 1.177241 | 0.001800 | 0.001938 | 0.001665 |
| mp_remove_if.test | RUN | 0.281571 | 2.241034 | 1.315972 | 0.001904 | 0.001538 | 0.002088 |
| mp_remove_if_q.test | RUN | 0.282804 | 2.229347 | 1.251189 | 0.001658 | 0.002086 | 0.002034 |
| mp_partition.test | RUN | 0.280857 | 2.100003 | 1.369909 | 0.001814 | 0.001958 | 0.001852 |
| mp_partition_q.test | RUN | 0.284264 | 2.115388 | 1.364341 | 0.001942 | 0.002144 | 0.001707 |
| mp_sort.test | RUN | 0.266710 | 1.523665 | 1.197699 | 0.001691 | 0.001977 | 0.001966 |
| mp_sort_q.test | RUN | 0.268887 | 1.537104 | 1.180096 | 0.001922 | 0.002038 | 0.002106 |
| mp_find.test | RUN | 0.260512 | 1.660035 | 1.081869 | 0.001617 | 0.001766 | 0.001895 |
| mp_find_if.test | RUN | 0.253365 | 1.523502 | 1.036158 | 0.001780 | 0.001981 | 0.001511 |
| mp_find_if_q.test | RUN | 0.279144 | 1.714785 | 1.209129 | 0.001768 | 0.002031 | 0.002156 |
| mp_reverse.test | RUN | 0.541061 | 4.402801 | 2.234766 | 0.001906 | 0.002028 | 0.002082 |
| mp_fold.test | RUN | 0.899465 | 3.374962 | 5.473618 | 0.002034 | 0.001935 | 0.002023 |
| mp_fold_q.test | RUN | 0.259189 | 1.528164 | 1.157334 | 0.001850 | 0.002150 | 0.002184 |
| mp_fold_q_sf.test | RUN | 0.235601 | 0.755832 | 0.904062 | 0.001994 | 0.002060 | 0.001930 |
| mp_reverse_fold.test | RUN | 0.421437 | 3.341428 | 1.606745 | 0.001779 | 0.002122 | 0.001909 |
| mp_reverse_fold_q.test | RUN | 0.266163 | 1.793894 | 1.211776 | 0.001886 | 0.001964 | 0.001891 |
| mp_unique.test | RUN | 0.262454 | 1.694787 | 1.185935 | 0.002271 | 0.002104 | 0.002094 |
| mp_unique_if.test | RUN | 0.461074 | 3.382810 | 1.842539 | 0.001817 | 0.001974 | 0.001998 |
| mp_unique_if_q.test | RUN | 0.390776 | 3.374875 | 1.489644 | 0.001792 | 0.002106 | 0.002134 |
| mp_all_of.test | RUN | 0.617851 | 1.846545 | 2.000685 | 0.001835 | 0.001957 | 0.001971 |
| mp_all_of_q.test | RUN | 0.227249 | 0.795356 | 0.840614 | 0.002016 | 0.001903 | 0.001970 |
| mp_any_of.test | RUN | 0.619975 | 1.882254 | 1.965967 | 0.001665 | 0.002114 | 0.002146 |
| mp_any_of_q.test | RUN | 0.230658 | 0.822277 | 0.908466 | 0.001888 | 0.002166 | 0.001829 |
| mp_none_of.test | RUN | 0.592986 | 1.847611 | 1.966551 | 0.001704 | 0.002114 | 0.002043 |
| mp_none_of_q.test | RUN | 0.227361 | 0.833412 | 0.904568 | 0.001874 | 0.002154 | 0.001844 |
| mp_replace_at.test | RUN | 0.268005 | 1.621904 | 1.283245 | 0.001724 | 0.002046 | 0.001871 |
| mp_replace_at_c.test | RUN | 0.272108 | 1.668084 | 1.231749 | 0.001512 | 0.001827 | 0.001695 |
| mp_for_each.test | RUN | 0.266386 | 1.261385 | 1.559195 | 0.001966 | 0.002052 | 0.002146 |
| mp_insert.test | RUN | 0.413633 | 3.357589 | 1.936824 | 0.001994 | 0.002078 | 0.002148 |
| mp_insert_2.test | RUN | 0.036949 | 0.077780 | 0.136722 | 0.000831 | 0.000733 | 0.001674 |
| mp_erase.test | RUN | 0.452931 | 3.534987 | 2.080961 | 0.002026 | 0.001912 | 0.001769 |
| mp_erase_2.test | RUN | 0.035885 | 0.073077 | 0.139215 | 0.001187 | 0.000890 | 0.002061 |
| mp_with_index.test | RUN | 0.586119 | 7.259589 | 4.846451 | 0.002426 | 0.002438 | 0.002042 |
| mp_with_index_cx.test | RUN | 0.003765 | 0.022803 | 0.030812 | 0.000793 | 0.000864 | 0.001973 |
| mp_from_sequence.test | RUN | 0.278157 | 1.895213 | 1.224451 | 0.001907 | 0.002116 | 0.002096 |
| mp_from_sequence_2.test | RUN | 0.289224 | 2.223706 | 1.419901 | 0.001867 | 0.002052 | 0.002034 |
| mp_from_sequence_3.test | RUN | 0.245969 | 1.015229 | 1.017526 | 0.001574 | 0.001967 | 0.001779 |
| mp_min_element.test | RUN | 0.240773 | 0.897827 | 0.962691 | 0.001857 | 0.001935 | 0.001776 |
| mp_min_element_2.test | RUN | 0.038084 | 0.077418 | 0.135679 | 0.000695 | 0.000906 | 0.001780 |
| mp_min_element_q.test | RUN | 0.236583 | 0.923221 | 0.984486 | 0.001806 | 0.001765 | 0.001873 |
| mp_min_element_q_2.test | RUN | 0.036771 | 0.076736 | 0.131475 | 0.000881 | 0.000719 | 0.001828 |
| mp_max_element.test | RUN | 0.238893 | 0.911256 | 0.987913 | 0.001642 | 0.002097 | 0.002221 |
| mp_max_element_2.test | RUN | 0.035373 | 0.072863 | 0.130635 | 0.000701 | 0.000802 | 0.001854 |
| mp_max_element_q.test | RUN | 0.234317 | 0.924841 | 0.913206 | 0.002096 | 0.001928 | 0.001742 |
| mp_max_element_q_2.test | RUN | 0.035813 | 0.072236 | 0.128277 | 0.000690 | 0.000912 | 0.002014 |
| mp_nth_element.test | RUN | 0.263822 | 1.289348 | 1.157914 | 0.001956 | 0.002052 | 0.002042 |
| mp_nth_element_q.test | RUN | 0.265544 | 1.254951 | 1.122269 | 0.001863 | 0.001982 | 0.001742 |
| mp_back.test | RUN | 0.260529 | 1.026185 | 1.063573 | 0.001846 | 0.002042 | 0.001768 |
| mp_back_2.test | RUN | 0.036482 | 0.078498 | 0.145070 | 0.000658 | 0.000738 | 0.001954 |
| mp_pop_back.test | RUN | 0.661361 | 6.734948 | 2.140729 | 0.001728 | 0.001708 | 0.001571 |
| mp_pop_back_2.test | RUN | 0.036309 | 0.079640 | 0.125514 | 0.000889 | 0.000733 | 0.001897 |
| mp_flatten.test | RUN | 0.303830 | 2.425625 | 1.497934 | 0.001889 | 0.002156 | 0.001844 |
| mp_rotate_left.test | RUN | 0.302720 | 2.604285 | 1.459878 | 0.001809 | 0.002169 | 0.001849 |
| mp_rotate_right.test | RUN | 0.300306 | 2.598620 | 1.483070 | 0.001912 | 0.001917 | 0.002030 |
| mp_power_set.test | RUN | 0.293808 | 2.517349 | 1.485013 | 0.002194 | 0.001952 | 0.001976 |
| mp_partial_sum.test | RUN | 0.380499 | 3.341017 | 1.674221 | 0.001883 | 0.002194 | 0.001950 |
| mp_iterate.test | RUN | 0.265124 | 1.008934 | 1.210772 | 0.001706 | 0.002365 | 0.001861 |
| mp_pairwise_fold.test | RUN | 0.298141 | 2.285962 | 1.509872 | 0.001814 | 0.002124 | 0.001908 |
| mp_pairwise_fold_q.test | RUN | 0.226619 | 0.678577 | 0.899266 | 0.002020 | 0.002028 | 0.001946 |
| mp_sliding_fold.test | RUN | 0.533898 | 4.341879 | 2.679127 | 0.001852 | 0.001926 | 0.001776 |
| mp_sliding_fold_q.test | RUN | 0.551127 | 4.370056 | 2.660214 | 0.001940 | 0.001872 | 0.001934 |
| mp_intersperse.test | RUN | 0.276819 | 1.661081 | 1.320046 | 0.001983 | 0.002001 | 0.002004 |
| mp_split.test | RUN | 0.384670 | 3.173443 | 1.783930 | 0.001776 | 0.001979 | 0.002068 |
| mp_join.test | RUN | 0.312044 | 2.640017 | 1.553831 | 0.001910 | 0.001968 | 0.002024 |
| integral.test | RUN | 0.237020 | 1.190836 | 0.982227 | 0.002024 | 0.002040 | 0.002078 |
| mp_value.test | RUN | 0.016212 | 0.041755 | 0.063957 | 0.001174 | 0.000824 | 0.002030 |
| mp_identity.test | RUN | 0.232345 | 1.095760 | 0.946590 | 0.001774 | 0.001630 | 0.002133 |
| mp_inherit.test | RUN | 0.218204 | 0.809440 | 0.897878 | 0.001932 | 0.002061 | 0.001646 |
| mp_if.test | RUN | 0.218866 | 0.899363 | 0.864086 | 0.001841 | 0.001988 | 0.002020 |
| mp_if_sf.test | RUN | 0.211136 | 0.674083 | 0.836074 | 0.001528 | 0.002029 | 0.002030 |
| mp_eval_if.test | RUN | 0.223819 | 0.963854 | 0.935060 | 0.002179 | 0.002132 | 0.001934 |
| mp_eval_if_sf.test | RUN | 0.220196 | 0.701642 | 0.850757 | 0.001928 | 0.001830 | 0.001920 |
| mp_valid.test | RUN | 0.220391 | 0.701858 | 0.867379 | 0.001874 | 0.001985 | 0.002046 |
| mp_defer.test | RUN | 0.235889 | 1.178632 | 0.975599 | 0.002020 | 0.002159 | 0.002126 |
| mp_quote.test | RUN | 0.236782 | 1.180383 | 1.019457 | 0.002083 | 0.001972 | 0.001997 |
| mp_invoke_q.test | RUN | 0.255853 | 1.520396 | 1.117590 | 0.001851 | 0.002128 | 0.001895 |
| mp_invoke_q_sf.test | RUN | 0.214393 | 0.688950 | 0.842412 | 0.001741 | 0.001954 | 0.001914 |
| mp_quote_trait.test | RUN | 0.240708 | 1.268038 | 0.998859 | 0.001843 | 0.001979 | 0.002008 |
| mp_cond.test | RUN | 0.224862 | 0.794567 | 0.877324 | 0.002111 | 0.001912 | 0.002052 |
| mp_cond_sf.test | RUN | 0.217605 | 0.686387 | 0.885785 | 0.002024 | 0.002108 | 0.002039 |
| mp_not_fn.test | RUN | 0.213122 | 0.692982 | 0.822677 | 0.001737 | 0.001888 | 0.001905 |
| mp_eval_if_not.test | RUN | 0.229617 | 0.983990 | 0.882908 | 0.001881 | 0.002085 | 0.002134 |
| mp_eval_or.test | RUN | 0.217300 | 0.537084 | 0.887133 | 0.001930 | 0.001899 | 0.001730 |
| mp_compose.test | RUN | 0.243053 | 1.597717 | 1.116391 | 0.001648 | 0.001806 | 0.001516 |
| mp_valid_and_true.test | RUN | 0.216781 | 0.827254 | 0.848139 | 0.001730 | 0.001990 | 0.002038 |
| mp_compose_sf.test | COMPILE | 0.052315 | 0.104569 | 0.216492 | — | — | — |
| integer_sequence.test | RUN | 0.267646 | 1.533177 | 1.213839 | 0.001758 | 0.001578 | 0.001933 |
| tuple_for_each.test | RUN | 0.225987 | 0.596870 | 0.950303 | 0.001700 | 0.001874 | 0.001878 |
| tuple_for_each_cx.test | COMPILE | 0.039456 | 0.088714 | 0.135902 | — | — | — |
| tuple_apply.test | RUN | 0.224830 | 0.595514 | 0.951777 | 0.001772 | 0.001632 | 0.001945 |
| tuple_apply_cx.test | COMPILE | 0.063187 | 0.124147 | 0.225860 | — | — | — |
| construct_from_tuple.test | RUN | 0.230197 | 0.656400 | 0.921343 | 0.002332 | 0.001738 | 0.001659 |
| construct_from_tuple_cx.test | COMPILE | 0.061186 | 0.122397 | 0.197927 | — | — | — |
| tuple_transform.test | RUN | 0.364195 | 1.296867 | 1.799606 | 0.002021 | 0.001725 | 0.001767 |
| tuple_transform_2.test | RUN | 0.324276 | 0.979838 | 1.664719 | 0.002081 | 0.001894 | 0.001552 |
| tuple_transform_cx.test | COMPILE | 0.053997 | 0.111191 | 0.188052 | — | — | — |
| mp_set_contains.test | RUN | 0.225608 | 0.830063 | 0.937052 | 0.001815 | 0.001800 | 0.002002 |
| mp_set_push_back.test | RUN | 0.304708 | 2.495033 | 1.484918 | 0.002026 | 0.001988 | 0.002070 |
| mp_set_push_front.test | RUN | 0.370967 | 2.987705 | 1.663880 | 0.001824 | 0.002081 | 0.002072 |
| mp_is_set.test | RUN | 0.235652 | 0.818854 | 0.930249 | 0.001932 | 0.001917 | 0.001956 |
| mp_set_union.test | RUN | 0.283883 | 2.121775 | 1.414619 | 0.001620 | 0.001960 | 0.001978 |
| mp_set_union_sf.test | RUN | 0.219469 | 0.660779 | 0.872395 | 0.002011 | 0.002170 | 0.001736 |
| mp_set_difference.test | RUN | 0.262007 | 1.659485 | 1.209041 | 0.002018 | 0.002144 | 0.002176 |
| mp_set_difference_sf.test | RUN | 0.219124 | 0.686743 | 0.852608 | 0.001914 | 0.002140 | 0.001967 |
| mp_set_intersection.test | RUN | 0.266664 | 1.651260 | 1.176347 | 0.001691 | 0.001868 | 0.002053 |
| mp_set_intersection_sf.test | RUN | 0.218955 | 0.676078 | 0.863703 | 0.002018 | 0.002078 | 0.001950 |
| mp_all.test | RUN | 0.223937 | 0.823122 | 0.889355 | 0.002045 | 0.001986 | 0.002152 |
| mp_all_2.test | RUN | 0.597521 | 1.693962 | 1.985498 | 0.002074 | 0.001896 | 0.001969 |
| mp_and.test | RUN | 0.218820 | 0.837310 | 0.885569 | 0.001764 | 0.001887 | 0.002110 |
| mp_any.test | RUN | 0.231312 | 0.829406 | 0.890817 | 0.001841 | 0.002066 | 0.002150 |
| mp_any_2.test | RUN | 0.637460 | 1.667912 | 1.890377 | 0.001946 | 0.001969 | 0.002056 |
| mp_or.test | RUN | 0.234790 | 0.783903 | 0.847837 | 0.002030 | 0.001745 | 0.001966 |
| mp_same.test | RUN | 0.418860 | 1.374308 | 1.271649 | 0.001776 | 0.001871 | 0.001865 |
| mp_plus.test | RUN | 0.250548 | 0.967471 | 1.026224 | 0.001624 | 0.002010 | 0.001775 |
| mp_less.test | RUN | 0.229799 | 0.823582 | 0.930861 | 0.001927 | 0.001840 | 0.001616 |
| mp_min.test | RUN | 0.218066 | 0.756524 | 0.870076 | 0.002040 | 0.002510 | 0.001713 |
| mp_max.test | RUN | 0.226646 | 0.851513 | 0.902362 | 0.001615 | 0.001992 | 0.001978 |
| mp_similar.test | RUN | 0.220546 | 0.824457 | 0.916396 | 0.001714 | 0.001832 | 0.002020 |
| mp_map_find.test | RUN | 0.263748 | 1.497383 | 1.168498 | 0.002160 | 0.001738 | 0.002070 |
| mp_map_find_2.test | RUN | 0.238430 | 0.589770 | 1.011218 | 0.001747 | 0.001719 | 0.002077 |
| mp_map_find_3.test | RUN | 0.246945 | 0.601423 | 1.057426 | 0.002155 | 0.001974 | 0.002050 |
| mp_map_find_4.test | RUN | 0.226661 | 0.556205 | 0.926724 | 0.001774 | 0.001742 | 0.002024 |
| mp_map_contains.test | RUN | 0.237515 | 0.858201 | 0.884629 | 0.002330 | 0.002202 | 0.002080 |
| mp_map_insert.test | RUN | 0.293379 | 2.765796 | 1.609601 | 0.001938 | 0.002110 | 0.002090 |
| mp_map_replace.test | RUN | 0.382364 | 3.163643 | 1.807723 | 0.001672 | 0.002021 | 0.001756 |
| mp_map_erase.test | RUN | 0.296815 | 2.347575 | 1.545543 | 0.001670 | 0.002000 | 0.001846 |
| mp_map_update.test | RUN | 0.269512 | 1.554341 | 1.185847 | 0.001961 | 0.001968 | 0.001926 |
| mp_map_update_q.test | RUN | 0.265357 | 1.551165 | 1.178804 | 0.002071 | 0.002060 | 0.002032 |
| mp_map_keys.test | RUN | 0.261874 | 1.365540 | 1.188087 | 0.001565 | 0.001921 | 0.001708 |
| mp_is_map.test | RUN | 0.242811 | 0.858384 | 0.995603 | 0.001786 | 0.001599 | 0.001944 |
| mp_bind.test | RUN | 0.301569 | 2.458978 | 1.331483 | 0.002064 | 0.002048 | 0.002065 |
| mp_bind_q.test | RUN | 0.263048 | 1.540592 | 1.154776 | 0.001968 | 0.002154 | 0.002052 |
| mp_bind_front.test | RUN | 0.259005 | 1.670681 | 1.153831 | 0.001885 | 0.002132 | 0.002036 |
| mp_bind_back.test | RUN | 0.258029 | 1.670995 | 1.110432 | 0.001894 | 0.001928 | 0.002244 |
| mp_lambda.test | RUN | 0.556033 | 4.898668 | 2.762005 | 0.001745 | 0.001560 | 0.002016 |
| mpl.test | RUN | 0.501050 | 1.303915 | 2.199898 | 0.001996 | 0.001907 | 0.002128 |
| mpl_list.test | RUN | 0.282661 | 1.036519 | 1.120750 | 0.002002 | 0.001854 | 0.002071 |
| mpl_tuple.test | RUN | 0.287174 | 1.018461 | 1.099109 | 0.001830 | 0.001804 | 0.001876 |
| version.test | RUN | 0.149299 | 0.381069 | 0.578566 | 0.001844 | 0.002012 | 0.001882 |
| mp11_version.test | RUN | 0.175051 | 0.432306 | 0.681529 | 0.001783 | 0.001948 | 0.002010 |
| list_version.test | RUN | 0.153114 | 0.395884 | 0.550338 | 0.001740 | 0.001902 | 0.002100 |
| algorithm_version.test | RUN | 0.172037 | 0.408941 | 0.633976 | 0.002058 | 0.001912 | 0.001746 |
| integral_version.test | RUN | 0.147983 | 0.372961 | 0.545062 | 0.001850 | 0.001767 | 0.001850 |
| utility_version.test | RUN | 0.147242 | 0.385797 | 0.582631 | 0.001878 | 0.001891 | 0.002004 |
| function_version.test | RUN | 0.153766 | 0.379089 | 0.561521 | 0.001752 | 0.001990 | 0.002066 |
| map_version.test | RUN | 0.169047 | 0.409358 | 0.606831 | 0.002074 | 0.001955 | 0.001872 |
| set_version.test | RUN | 0.157957 | 0.392734 | 0.588564 | 0.001966 | 0.001711 | 0.001942 |
| bind_version.test | RUN | 0.169877 | 0.404278 | 0.624592 | 0.001860 | 0.001952 | 0.002069 |
| integer_sequence_version.test | RUN | 0.146733 | 0.372398 | 0.540436 | 0.001864 | 0.001739 | 0.002055 |
| tuple_version.test | RUN | 0.170416 | 0.412084 | 0.459832 | 0.001590 | 0.001884 | 0.001464 |
| mpl_version.test | RUN | 0.113190 | 0.289524 | 0.457451 | 0.001432 | 0.001564 | 0.001486 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.998× | 0.995×, 1.001× | 1.001× |
| clang | 1.085× | 1.089×, 1.080× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001145s | 0.001149s | 0.001122s | byte equality observed |
| bind_version.test | 10 | 0.001147s | 0.001167s | 0.001139s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001140s | 0.001150s | 0.001153s | byte equality observed |
| function_version.test | 10 | 0.001143s | 0.001156s | 0.001177s | byte equality observed |
| integer_sequence.test | 10 | 0.001135s | 0.001151s | 0.001204s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001139s | 0.001135s | 0.001272s | byte equality observed |
| integral.test | 10 | 0.001140s | 0.001134s | 0.001244s | byte equality observed |
| integral_version.test | 10 | 0.001125s | 0.001141s | 0.001270s | byte equality observed |
| list_version.test | 10 | 0.001133s | 0.001133s | 0.001194s | byte equality observed |
| map_version.test | 10 | 0.001119s | 0.001128s | 0.001251s | byte equality observed |
| mp11_version.test | 10 | 0.001116s | 0.001141s | 0.001210s | byte equality observed |
| mp_all.test | 10 | 0.001125s | 0.001178s | 0.001229s | byte equality observed |
| mp_all_2.test | 10 | 0.001121s | 0.001133s | 0.001218s | byte equality observed |
| mp_all_of.test | 10 | 0.001115s | 0.001147s | 0.001235s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001131s | 0.001116s | 0.001247s | byte equality observed |
| mp_and.test | 10 | 0.001145s | 0.001142s | 0.001242s | byte equality observed |
| mp_any.test | 10 | 0.001130s | 0.001148s | 0.001246s | byte equality observed |
| mp_any_2.test | 10 | 0.001145s | 0.001133s | 0.001221s | byte equality observed |
| mp_any_of.test | 10 | 0.001132s | 0.001150s | 0.001213s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001190s | 0.001139s | 0.001251s | byte equality observed |
| mp_append.test | 10 | 0.001240s | 0.001146s | 0.001317s | byte equality observed |
| mp_append_2.test | 10 | 0.001260s | 0.001137s | 0.001241s | byte equality observed |
| mp_append_3.test | 10 | 0.000581s | 0.000519s | 0.001245s | byte equality observed |
| mp_append_sf.test | 10 | 0.001233s | 0.001122s | 0.001194s | byte equality observed |
| mp_apply_q.test | 10 | 0.001217s | 0.001160s | 0.001211s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001237s | 0.001244s | 0.001250s | byte equality observed |
| mp_assign.test | 10 | 0.001230s | 0.001272s | 0.001222s | byte equality observed |
| mp_assign_2.test | 10 | 0.000577s | 0.000581s | 0.001269s | byte equality observed |
| mp_at.test | 10 | 0.001262s | 0.001270s | 0.001182s | byte equality observed |
| mp_at_2.test | 10 | 0.000575s | 0.000560s | 0.001271s | byte equality observed |
| mp_at_sf.test | 10 | 0.001260s | 0.001219s | 0.001237s | byte equality observed |
| mp_back.test | 10 | 0.001303s | 0.001271s | 0.001257s | byte equality observed |
| mp_back_2.test | 10 | 0.000562s | 0.000559s | 0.001219s | byte equality observed |
| mp_bind.test | 10 | 0.001285s | 0.001227s | 0.001196s | byte equality observed |
| mp_bind_back.test | 10 | 0.001230s | 0.001245s | 0.001223s | byte equality observed |
| mp_bind_front.test | 10 | 0.001227s | 0.001269s | 0.001276s | byte equality observed |
| mp_bind_q.test | 10 | 0.001224s | 0.001227s | 0.001231s | byte equality observed |
| mp_clear.test | 10 | 0.001258s | 0.001260s | 0.001244s | byte equality observed |
| mp_clear_2.test | 10 | 0.000579s | 0.000565s | 0.001202s | byte equality observed |
| mp_compose.test | 10 | 0.001234s | 0.001220s | 0.001255s | byte equality observed |
| mp_cond.test | 10 | 0.001228s | 0.001271s | 0.001194s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001287s | 0.001307s | 0.001182s | byte equality observed |
| mp_contains.test | 10 | 0.001243s | 0.001232s | 0.001205s | byte equality observed |
| mp_copy_if.test | 10 | 0.001271s | 0.001252s | 0.001256s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001274s | 0.001211s | 0.001221s | byte equality observed |
| mp_count.test | 10 | 0.001257s | 0.001216s | 0.001229s | byte equality observed |
| mp_count_if.test | 10 | 0.001286s | 0.001265s | 0.001266s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001229s | 0.001282s | 0.001288s | byte equality observed |
| mp_defer.test | 10 | 0.001225s | 0.001239s | 0.001267s | byte equality observed |
| mp_drop.test | 10 | 0.001262s | 0.001252s | 0.001285s | byte equality observed |
| mp_drop_2.test | 10 | 0.000584s | 0.000579s | 0.001230s | byte equality observed |
| mp_empty.test | 10 | 0.001264s | 0.001249s | 0.001216s | byte equality observed |
| mp_empty_2.test | 10 | 0.000565s | 0.000590s | 0.001291s | byte equality observed |
| mp_erase.test | 10 | 0.001276s | 0.001258s | 0.001220s | byte equality observed |
| mp_erase_2.test | 10 | 0.000577s | 0.000569s | 0.001193s | byte equality observed |
| mp_eval_if.test | 10 | 0.001270s | 0.001252s | 0.001249s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001235s | 0.001233s | 0.001278s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001246s | 0.001243s | 0.001262s | byte equality observed |
| mp_eval_or.test | 10 | 0.001230s | 0.001324s | 0.001260s | byte equality observed |
| mp_fill.test | 10 | 0.001256s | 0.001248s | 0.001227s | byte equality observed |
| mp_fill_2.test | 10 | 0.000555s | 0.000591s | 0.001171s | byte equality observed |
| mp_filter.test | 10 | 0.001243s | 0.001259s | 0.001245s | byte equality observed |
| mp_find.test | 10 | 0.001277s | 0.001322s | 0.001217s | byte equality observed |
| mp_find_if.test | 10 | 0.001266s | 0.001283s | 0.001220s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001297s | 0.001248s | 0.001208s | byte equality observed |
| mp_flatten.test | 10 | 0.001311s | 0.001243s | 0.001223s | byte equality observed |
| mp_fold.test | 10 | 0.001321s | 0.001274s | 0.001300s | byte equality observed |
| mp_fold_q.test | 10 | 0.001291s | 0.001280s | 0.001235s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001252s | 0.001288s | 0.001249s | byte equality observed |
| mp_for_each.test | 10 | 0.001288s | 0.001215s | 0.001224s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001283s | 0.001261s | 0.001243s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001280s | 0.001250s | 0.001274s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001278s | 0.001276s | 0.001335s | byte equality observed |
| mp_front.test | 10 | 0.001249s | 0.001256s | 0.001225s | byte equality observed |
| mp_front_2.test | 10 | 0.000572s | 0.000579s | 0.001221s | byte equality observed |
| mp_identity.test | 10 | 0.001232s | 0.001200s | 0.001222s | byte equality observed |
| mp_if.test | 10 | 0.001255s | 0.001236s | 0.001260s | byte equality observed |
| mp_if_sf.test | 10 | 0.001272s | 0.001211s | 0.001269s | byte equality observed |
| mp_inherit.test | 10 | 0.001267s | 0.001204s | 0.001217s | byte equality observed |
| mp_insert.test | 10 | 0.001278s | 0.001279s | 0.001244s | byte equality observed |
| mp_insert_2.test | 10 | 0.000587s | 0.000571s | 0.001315s | byte equality observed |
| mp_intersperse.test | 10 | 0.001231s | 0.001253s | 0.001200s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001253s | 0.001209s | 0.001236s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001240s | 0.001247s | 0.001230s | byte equality observed |
| mp_iota.test | 10 | 0.001233s | 0.001239s | 0.001209s | byte equality observed |
| mp_iota_2.test | 10 | 0.001272s | 0.001224s | 0.001219s | byte equality observed |
| mp_is_list.test | 10 | 0.001224s | 0.001229s | 0.001262s | byte equality observed |
| mp_is_map.test | 10 | 0.001256s | 0.001240s | 0.001238s | byte equality observed |
| mp_is_set.test | 10 | 0.001233s | 0.001249s | 0.001242s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001241s | 0.001191s | 0.001249s | byte equality observed |
| mp_iterate.test | 10 | 0.001276s | 0.001209s | 0.001219s | byte equality observed |
| mp_join.test | 10 | 0.001258s | 0.001266s | 0.001204s | byte equality observed |
| mp_lambda.test | 10 | 0.001262s | 0.001295s | 0.001258s | byte equality observed |
| mp_less.test | 10 | 0.001234s | 0.001270s | 0.001208s | byte equality observed |
| mp_list_c.test | 10 | 0.001270s | 0.001242s | 0.001242s | byte equality observed |
| mp_list_v.test | 10 | 0.000584s | 0.000589s | 0.001208s | byte equality observed |
| mp_map_contains.test | 10 | 0.001232s | 0.001257s | 0.001234s | byte equality observed |
| mp_map_erase.test | 10 | 0.001212s | 0.001225s | 0.001288s | byte equality observed |
| mp_map_find.test | 10 | 0.001217s | 0.001219s | 0.001266s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001288s | 0.001194s | 0.001277s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001278s | 0.001201s | 0.001262s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001222s | 0.001191s | 0.001221s | byte equality observed |
| mp_map_insert.test | 10 | 0.001248s | 0.001288s | 0.001242s | byte equality observed |
| mp_map_keys.test | 10 | 0.001237s | 0.001253s | 0.001224s | byte equality observed |
| mp_map_replace.test | 10 | 0.001274s | 0.001248s | 0.001221s | byte equality observed |
| mp_map_update.test | 10 | 0.001245s | 0.001259s | 0.001232s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001214s | 0.001249s | 0.001281s | byte equality observed |
| mp_max.test | 10 | 0.001261s | 0.001258s | 0.001275s | byte equality observed |
| mp_max_element.test | 10 | 0.001266s | 0.001265s | 0.001282s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000562s | 0.000579s | 0.001240s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001265s | 0.001261s | 0.001204s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000568s | 0.000570s | 0.001197s | byte equality observed |
| mp_min.test | 10 | 0.001253s | 0.001232s | 0.001228s | byte equality observed |
| mp_min_element.test | 10 | 0.001278s | 0.001272s | 0.001244s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000564s | 0.000568s | 0.001255s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001278s | 0.001247s | 0.001198s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000572s | 0.000573s | 0.001233s | byte equality observed |
| mp_none_of.test | 10 | 0.001220s | 0.001190s | 0.001201s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001291s | 0.001207s | 0.001219s | byte equality observed |
| mp_not_fn.test | 10 | 0.001271s | 0.001226s | 0.001270s | byte equality observed |
| mp_nth_element.test | 10 | 0.001270s | 0.001218s | 0.001262s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001266s | 0.001230s | 0.001216s | byte equality observed |
| mp_or.test | 10 | 0.001286s | 0.001317s | 0.001285s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001304s | 0.001275s | 0.001278s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001260s | 0.001247s | 0.001230s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001251s | 0.001238s | 0.001234s | byte equality observed |
| mp_partition.test | 10 | 0.001282s | 0.001289s | 0.001217s | byte equality observed |
| mp_partition_q.test | 10 | 0.001251s | 0.001280s | 0.001214s | byte equality observed |
| mp_plus.test | 10 | 0.001256s | 0.001253s | 0.001219s | byte equality observed |
| mp_pop_back.test | 9 | 0.001291s | 0.001298s | 0.001230s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000573s | 0.000585s | 0.001202s | byte equality observed |
| mp_pop_front.test | 10 | 0.001268s | 0.001259s | 0.001246s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000575s | 0.000585s | 0.001252s | byte equality observed |
| mp_power_set.test | 10 | 0.001229s | 0.001242s | 0.001248s | byte equality observed |
| mp_product.test | 10 | 0.001248s | 0.001258s | 0.001254s | byte equality observed |
| mp_push_back.test | 10 | 0.001269s | 0.001285s | 0.001248s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000569s | 0.000579s | 0.001216s | byte equality observed |
| mp_push_front.test | 10 | 0.001246s | 0.001280s | 0.001232s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000566s | 0.000573s | 0.001209s | byte equality observed |
| mp_quote.test | 10 | 0.001239s | 0.001244s | 0.001282s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001256s | 0.001270s | 0.001246s | byte equality observed |
| mp_remove.test | 10 | 0.001250s | 0.001210s | 0.001254s | byte equality observed |
| mp_remove_if.test | 10 | 0.001233s | 0.001252s | 0.001214s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001276s | 0.001282s | 0.001221s | byte equality observed |
| mp_rename.test | 10 | 0.001268s | 0.001266s | 0.001258s | byte equality observed |
| mp_rename_2.test | 10 | 0.000580s | 0.000583s | 0.001270s | byte equality observed |
| mp_rename_v.test | 10 | 0.000579s | 0.000566s | 0.001179s | byte equality observed |
| mp_repeat.test | 10 | 0.001282s | 0.001317s | 0.001222s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000577s | 0.000554s | 0.001190s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001206s | 0.001259s | 0.001230s | byte equality observed |
| mp_replace.test | 10 | 0.001240s | 0.001276s | 0.001252s | byte equality observed |
| mp_replace_at.test | 10 | 0.001232s | 0.001270s | 0.001189s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001274s | 0.001285s | 0.001206s | byte equality observed |
| mp_replace_front.test | 10 | 0.001243s | 0.001277s | 0.001191s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000587s | 0.000553s | 0.001161s | byte equality observed |
| mp_replace_if.test | 10 | 0.001208s | 0.001258s | 0.001276s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001214s | 0.001273s | 0.001261s | byte equality observed |
| mp_replace_second.test | 10 | 0.001271s | 0.001266s | 0.001216s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000578s | 0.000562s | 0.001230s | byte equality observed |
| mp_replace_third.test | 10 | 0.001283s | 0.001259s | 0.001239s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000577s | 0.000574s | 0.001203s | byte equality observed |
| mp_reverse.test | 10 | 0.001267s | 0.001262s | 0.001247s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001302s | 0.001301s | 0.001244s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001303s | 0.001255s | 0.001231s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001231s | 0.001248s | 0.001254s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001244s | 0.001264s | 0.001217s | byte equality observed |
| mp_same.test | 10 | 0.001268s | 0.001269s | 0.001248s | byte equality observed |
| mp_second.test | 10 | 0.001267s | 0.001239s | 0.001225s | byte equality observed |
| mp_second_2.test | 10 | 0.000575s | 0.000558s | 0.001221s | byte equality observed |
| mp_set_contains.test | 10 | 0.001212s | 0.001247s | 0.001253s | byte equality observed |
| mp_set_difference.test | 10 | 0.001238s | 0.001251s | 0.001216s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001254s | 0.001255s | 0.001238s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001253s | 0.001283s | 0.001218s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001249s | 0.001257s | 0.001229s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001231s | 0.001303s | 0.001265s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001288s | 0.001316s | 0.001207s | byte equality observed |
| mp_set_union.test | 10 | 0.001243s | 0.001250s | 0.001246s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001263s | 0.001238s | 0.001219s | byte equality observed |
| mp_similar.test | 10 | 0.001276s | 0.001235s | 0.001206s | byte equality observed |
| mp_size.test | 10 | 0.001250s | 0.001243s | 0.001216s | byte equality observed |
| mp_size_2.test | 10 | 0.000575s | 0.000576s | 0.001194s | byte equality observed |
| mp_slice.test | 10 | 0.001278s | 0.001281s | 0.001195s | byte equality observed |
| mp_slice_2.test | 10 | 0.000573s | 0.000574s | 0.001276s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001266s | 0.001280s | 0.001220s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001252s | 0.001327s | 0.001288s | byte equality observed |
| mp_sort.test | 10 | 0.001260s | 0.001268s | 0.001249s | byte equality observed |
| mp_sort_q.test | 10 | 0.001203s | 0.001256s | 0.001288s | byte equality observed |
| mp_split.test | 10 | 0.001254s | 0.001256s | 0.001242s | byte equality observed |
| mp_starts_with.test | 10 | 0.001275s | 0.001260s | 0.001192s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001244s | 0.001258s | 0.001215s | byte equality observed |
| mp_take.test | 10 | 0.001285s | 0.001289s | 0.001214s | byte equality observed |
| mp_take_2.test | 10 | 0.000582s | 0.000575s | 0.001343s | byte equality observed |
| mp_third.test | 10 | 0.001269s | 0.001289s | 0.001222s | byte equality observed |
| mp_third_2.test | 10 | 0.000564s | 0.000556s | 0.001289s | byte equality observed |
| mp_transform.test | 10 | 0.001230s | 0.001239s | 0.001193s | byte equality observed |
| mp_transform_2.test | 10 | 0.001192s | 0.001228s | 0.001209s | byte equality observed |
| mp_transform_front.test | 10 | 0.001240s | 0.001259s | 0.001208s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000578s | 0.000561s | 0.001196s | byte equality observed |
| mp_transform_if.test | 10 | 0.001256s | 0.001266s | 0.001204s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001221s | 0.001228s | 0.001176s | byte equality observed |
| mp_transform_q.test | 10 | 0.001258s | 0.001243s | 0.001204s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001230s | 0.001283s | 0.001273s | byte equality observed |
| mp_transform_second.test | 10 | 0.001243s | 0.001245s | 0.001217s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000577s | 0.000556s | 0.001304s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001241s | 0.001233s | 0.001209s | byte equality observed |
| mp_transform_third.test | 10 | 0.001265s | 0.001241s | 0.001217s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000582s | 0.000579s | 0.001192s | byte equality observed |
| mp_unique.test | 10 | 0.001244s | 0.001261s | 0.001264s | byte equality observed |
| mp_unique_if.test | 10 | 0.001272s | 0.001249s | 0.001265s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001258s | 0.001247s | 0.001247s | byte equality observed |
| mp_valid.test | 10 | 0.001274s | 0.001232s | 0.001249s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001263s | 0.001288s | 0.001201s | byte equality observed |
| mp_value.test | 10 | 0.000575s | 0.000564s | 0.001215s | byte equality observed |
| mp_with_index.test | 10 | 0.001633s | 0.001634s | 0.001539s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000574s | 0.000589s | 0.001197s | byte equality observed |
| mpl.test | 10 | 0.001266s | 0.001219s | 0.001279s | byte equality observed |
| mpl_list.test | 10 | 0.001238s | 0.001213s | 0.001239s | byte equality observed |
| mpl_tuple.test | 10 | 0.001257s | 0.001215s | 0.001252s | byte equality observed |
| mpl_version.test | 10 | 0.001251s | 0.001285s | 0.001245s | byte equality observed |
| set_version.test | 10 | 0.001226s | 0.001297s | 0.001234s | byte equality observed |
| tuple_apply.test | 10 | 0.001220s | 0.001265s | 0.001253s | byte equality observed |
| tuple_for_each.test | 10 | 0.001241s | 0.001299s | 0.001201s | byte equality observed |
| tuple_transform.test | 10 | 0.001225s | 0.001230s | 0.001247s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001241s | 0.001233s | 0.001190s | byte equality observed |
| tuple_version.test | 10 | 0.001238s | 0.001279s | 0.001251s | byte equality observed |
| utility_version.test | 10 | 0.001229s | 0.001300s | 0.001227s | byte equality observed |
| version.test | 10 | 0.001250s | 0.001317s | 0.001213s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
