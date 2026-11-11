# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.057807 | 0.103291 | 0.194375 | — | — | — |
| complex_h.test | COMPILE | 0.232990 | 0.454618 | 0.716997 | — | — | — |
| mp_size.test | RUN | 0.268725 | 1.363944 | 0.957260 | 0.001827 | 0.002150 | 0.001830 |
| mp_size_2.test | RUN | 0.023687 | 0.048444 | 0.079932 | 0.000952 | 0.000951 | 0.002168 |
| mp_empty.test | RUN | 0.265608 | 0.865442 | 0.935299 | 0.002114 | 0.002360 | 0.001715 |
| mp_empty_2.test | RUN | 0.025682 | 0.049591 | 0.080868 | 0.000892 | 0.000852 | 0.001984 |
| mp_assign.test | RUN | 0.290177 | 1.765235 | 1.144690 | 0.001698 | 0.002292 | 0.002336 |
| mp_assign_2.test | RUN | 0.025439 | 0.050910 | 0.076148 | 0.000861 | 0.001054 | 0.001924 |
| mp_clear.test | RUN | 0.254419 | 0.774044 | 0.844425 | 0.002151 | 0.002165 | 0.001947 |
| mp_clear_2.test | RUN | 0.025731 | 0.050972 | 0.085293 | 0.000861 | 0.000800 | 0.002128 |
| mp_front.test | RUN | 0.270475 | 1.197684 | 0.965752 | 0.002008 | 0.002304 | 0.002161 |
| mp_front_2.test | RUN | 0.024465 | 0.050640 | 0.084191 | 0.000808 | 0.000873 | 0.002144 |
| mp_pop_front.test | RUN | 0.269938 | 1.274415 | 1.024781 | 0.002030 | 0.002181 | 0.001936 |
| mp_pop_front_2.test | RUN | 0.025923 | 0.052441 | 0.082118 | 0.000823 | 0.001011 | 0.001990 |
| mp_second.test | RUN | 0.260210 | 0.812382 | 0.969351 | 0.001834 | 0.002087 | 0.002240 |
| mp_second_2.test | RUN | 0.024363 | 0.051130 | 0.074569 | 0.000740 | 0.000900 | 0.002180 |
| mp_third.test | RUN | 0.259867 | 0.779551 | 0.865940 | 0.001892 | 0.002241 | 0.002093 |
| mp_third_2.test | RUN | 0.023755 | 0.050082 | 0.083551 | 0.000831 | 0.000708 | 0.001942 |
| mp_push_front.test | RUN | 0.372884 | 2.698900 | 1.481356 | 0.002158 | 0.001878 | 0.002246 |
| mp_push_front_2.test | RUN | 0.026377 | 0.048203 | 0.076477 | 0.000928 | 0.000912 | 0.002186 |
| mp_push_back.test | RUN | 0.383853 | 2.651041 | 1.553360 | 0.001845 | 0.002110 | 0.002048 |
| mp_push_back_2.test | RUN | 0.024784 | 0.051835 | 0.085786 | 0.000853 | 0.000827 | 0.002184 |
| mp_rename.test | RUN | 0.316204 | 2.415283 | 1.362269 | 0.002196 | 0.002024 | 0.002082 |
| mp_rename_2.test | RUN | 0.025370 | 0.053765 | 0.083316 | 0.000956 | 0.000999 | 0.002066 |
| mp_append.test | RUN | 0.321278 | 2.728784 | 1.508019 | 0.002152 | 0.002205 | 0.002169 |
| mp_append_2.test | RUN | 0.610981 | 6.093984 | 1.873213 | 0.001956 | 0.001746 | 0.001982 |
| mp_append_3.test | RUN | 0.025613 | 0.050281 | 0.068388 | 0.000865 | 0.000704 | 0.002166 |
| mp_append_sf.test | RUN | 0.259405 | 0.728255 | 0.879083 | 0.002070 | 0.001942 | 0.002250 |
| mp_replace_front.test | RUN | 0.308060 | 1.960991 | 1.218343 | 0.002018 | 0.002244 | 0.002059 |
| mp_replace_front_2.test | RUN | 0.024044 | 0.051954 | 0.083705 | 0.000874 | 0.000932 | 0.001976 |
| mp_replace_second.test | RUN | 0.297271 | 1.272841 | 1.097127 | 0.002204 | 0.002124 | 0.002157 |
| mp_replace_second_2.test | RUN | 0.024267 | 0.051488 | 0.080476 | 0.001077 | 0.000922 | 0.001886 |
| mp_replace_third.test | RUN | 0.277364 | 1.056298 | 1.079549 | 0.002110 | 0.002216 | 0.002262 |
| mp_replace_third_2.test | RUN | 0.025259 | 0.050747 | 0.086262 | 0.000886 | 0.000809 | 0.002147 |
| mp_apply_q.test | RUN | 0.311588 | 2.420549 | 1.332149 | 0.002138 | 0.001858 | 0.002282 |
| mp_apply_q_sf.test | RUN | 0.254360 | 0.733939 | 0.863945 | 0.002138 | 0.002106 | 0.002310 |
| mp_is_list.test | RUN | 0.265297 | 0.837371 | 0.891725 | 0.001868 | 0.002182 | 0.002071 |
| mp_list_c.test | RUN | 0.272617 | 1.037711 | 0.899010 | 0.001922 | 0.002306 | 0.002103 |
| mp_transform_front.test | RUN | 0.294064 | 1.936673 | 1.219484 | 0.002078 | 0.002142 | 0.002138 |
| mp_transform_front_2.test | RUN | 0.026182 | 0.050475 | 0.083513 | 0.000784 | 0.000730 | 0.001920 |
| mp_transform_second.test | RUN | 0.293539 | 1.716011 | 1.119924 | 0.001846 | 0.002008 | 0.002034 |
| mp_transform_second_2.test | RUN | 0.022900 | 0.048169 | 0.075113 | 0.000791 | 0.000894 | 0.001700 |
| mp_transform_third.test | RUN | 0.271424 | 1.316351 | 0.942495 | 0.001821 | 0.002152 | 0.002158 |
| mp_transform_third_2.test | RUN | 0.024482 | 0.045983 | 0.079886 | 0.000882 | 0.000722 | 0.002247 |
| mp_list_v.test | RUN | 0.026358 | 0.046614 | 0.081473 | 0.000883 | 0.000931 | 0.001830 |
| mp_rename_v.test | RUN | 0.025918 | 0.047546 | 0.084547 | 0.000927 | 0.001028 | 0.001774 |
| mp_is_value_list.test | RUN | 0.249885 | 0.532236 | 0.830208 | 0.002012 | 0.002091 | 0.002038 |
| mp_transform.test | RUN | 0.505457 | 4.080794 | 2.498518 | 0.002225 | 0.002096 | 0.001898 |
| mp_transform_2.test | RUN | 0.246759 | 0.490325 | 0.818768 | 0.002148 | 0.002132 | 0.002142 |
| mp_transform_q.test | RUN | 0.495360 | 3.861568 | 2.315903 | 0.002236 | 0.002088 | 0.001848 |
| mp_transform_q_2.test | RUN | 0.572811 | 1.296931 | 3.056682 | 0.002152 | 0.002037 | 0.001750 |
| mp_transform_sf.test | RUN | 0.269756 | 0.752416 | 0.933780 | 0.002080 | 0.002092 | 0.002134 |
| mp_transform_if.test | RUN | 0.329876 | 1.989397 | 1.335281 | 0.002044 | 0.001999 | 0.002038 |
| mp_transform_if_q.test | RUN | 0.326071 | 2.012909 | 1.365134 | 0.001894 | 0.002281 | 0.002190 |
| mp_filter.test | RUN | 0.294493 | 1.183445 | 1.082078 | 0.002076 | 0.002130 | 0.001851 |
| mp_fill.test | RUN | 0.277582 | 0.916594 | 0.972235 | 0.002103 | 0.002192 | 0.001593 |
| mp_fill_2.test | RUN | 0.043066 | 0.085097 | 0.138023 | 0.000685 | 0.000993 | 0.002226 |
| mp_count.test | RUN | 0.485338 | 2.157749 | 1.501033 | 0.002042 | 0.002174 | 0.002293 |
| mp_count_if.test | RUN | 0.491963 | 2.190680 | 1.702661 | 0.001835 | 0.002334 | 0.001876 |
| mp_count_if_q.test | RUN | 0.291809 | 1.400701 | 0.972087 | 0.002015 | 0.002331 | 0.001927 |
| mp_contains.test | RUN | 0.275469 | 0.860551 | 0.876975 | 0.002002 | 0.002160 | 0.001781 |
| mp_starts_with.test | RUN | 0.272717 | 0.881045 | 0.996837 | 0.002121 | 0.002304 | 0.001948 |
| mp_starts_with_sf.test | RUN | 0.266224 | 0.730070 | 0.956826 | 0.001721 | 0.002266 | 0.001762 |
| mp_repeat.test | RUN | 0.397557 | 3.473787 | 1.612281 | 0.002158 | 0.002127 | 0.002280 |
| mp_repeat_2.test | RUN | 0.042887 | 0.084831 | 0.148725 | 0.000770 | 0.000941 | 0.002126 |
| mp_repeat_3.test | RUN | 0.327788 | 0.477514 | 0.906157 | 0.001916 | 0.002318 | 0.002039 |
| mp_product.test | RUN | 0.300879 | 2.161292 | 1.318707 | 0.001942 | 0.002183 | 0.002181 |
| mp_drop.test | RUN | 0.317115 | 2.654453 | 1.469938 | 0.001872 | 0.002224 | 0.002243 |
| mp_drop_2.test | RUN | 0.045362 | 0.085115 | 0.131786 | 0.000751 | 0.000963 | 0.002172 |
| mp_iota.test | RUN | 0.316311 | 2.236123 | 1.256142 | 0.002207 | 0.002152 | 0.002252 |
| mp_iota_2.test | RUN | 0.334206 | 2.683814 | 1.366366 | 0.001704 | 0.002305 | 0.001798 |
| mp_at.test | RUN | 0.283163 | 1.183193 | 1.063150 | 0.002132 | 0.001932 | 0.001820 |
| mp_at_2.test | RUN | 0.042772 | 0.084200 | 0.148255 | 0.000733 | 0.000861 | 0.002112 |
| mp_at_sf.test | RUN | 0.268018 | 0.748911 | 0.914649 | 0.001951 | 0.002182 | 0.002208 |
| mp_take.test | RUN | 0.404904 | 3.459265 | 1.682949 | 0.001874 | 0.001784 | 0.002346 |
| mp_take_2.test | RUN | 0.042440 | 0.076468 | 0.137601 | 0.000914 | 0.000910 | 0.002102 |
| mp_slice.test | RUN | 0.451012 | 3.519541 | 2.106770 | 0.002102 | 0.002252 | 0.002230 |
| mp_slice_2.test | RUN | 0.044224 | 0.078762 | 0.152026 | 0.000856 | 0.000925 | 0.002050 |
| mp_replace.test | RUN | 0.319317 | 1.801862 | 1.408434 | 0.002188 | 0.002140 | 0.001791 |
| mp_replace_if.test | RUN | 0.312454 | 1.591646 | 1.270439 | 0.001886 | 0.002108 | 0.002083 |
| mp_replace_if_q.test | RUN | 0.305295 | 1.601011 | 1.312141 | 0.001993 | 0.002165 | 0.002281 |
| mp_copy_if.test | RUN | 0.320010 | 2.242620 | 1.257033 | 0.001956 | 0.002068 | 0.001955 |
| mp_copy_if_q.test | RUN | 0.311846 | 2.266917 | 1.253256 | 0.002619 | 0.001965 | 0.002108 |
| mp_remove.test | RUN | 0.307118 | 1.485538 | 1.258866 | 0.001930 | 0.002025 | 0.002189 |
| mp_remove_if.test | RUN | 0.327091 | 2.372718 | 1.312610 | 0.001866 | 0.002290 | 0.002256 |
| mp_remove_if_q.test | RUN | 0.322254 | 2.381339 | 1.331214 | 0.002154 | 0.002051 | 0.001896 |
| mp_partition.test | RUN | 0.320413 | 2.231313 | 1.373619 | 0.002047 | 0.001892 | 0.001972 |
| mp_partition_q.test | RUN | 0.326450 | 2.147806 | 1.400556 | 0.002173 | 0.002071 | 0.002173 |
| mp_sort.test | RUN | 0.307158 | 1.586944 | 1.299588 | 0.002002 | 0.002184 | 0.002215 |
| mp_sort_q.test | RUN | 0.308771 | 1.600470 | 1.251813 | 0.002105 | 0.002144 | 0.001843 |
| mp_find.test | RUN | 0.301909 | 1.700334 | 1.077428 | 0.001890 | 0.001932 | 0.001926 |
| mp_find_if.test | RUN | 0.293594 | 1.569610 | 1.064464 | 0.002152 | 0.002122 | 0.001800 |
| mp_find_if_q.test | RUN | 0.322577 | 1.798844 | 1.279700 | 0.002020 | 0.002174 | 0.002246 |
| mp_reverse.test | RUN | 0.542325 | 4.569156 | 2.307370 | 0.001885 | 0.002190 | 0.002264 |
| mp_fold.test | RUN | 0.927297 | 3.574688 | 5.728607 | 0.002266 | 0.002114 | 0.002227 |
| mp_fold_q.test | RUN | 0.301083 | 1.561832 | 1.232780 | 0.002176 | 0.002336 | 0.002272 |
| mp_fold_q_sf.test | RUN | 0.271635 | 0.731023 | 0.946368 | 0.001898 | 0.002412 | 0.002030 |
| mp_reverse_fold.test | RUN | 0.453348 | 3.389562 | 1.671211 | 0.001854 | 0.002376 | 0.002185 |
| mp_reverse_fold_q.test | RUN | 0.299497 | 1.927202 | 1.242480 | 0.001927 | 0.002240 | 0.002142 |
| mp_unique.test | RUN | 0.296346 | 1.751166 | 1.227457 | 0.001775 | 0.002153 | 0.002242 |
| mp_unique_if.test | RUN | 0.482257 | 3.616754 | 1.920797 | 0.001962 | 0.001790 | 0.001746 |
| mp_unique_if_q.test | RUN | 0.409046 | 3.448735 | 1.532498 | 0.002053 | 0.001934 | 0.002030 |
| mp_all_of.test | RUN | 0.673425 | 1.919823 | 2.044319 | 0.002014 | 0.002156 | 0.002212 |
| mp_all_of_q.test | RUN | 0.274478 | 0.882885 | 0.883588 | 0.001882 | 0.002173 | 0.002291 |
| mp_any_of.test | RUN | 0.655672 | 1.995652 | 2.072529 | 0.002211 | 0.002328 | 0.001858 |
| mp_any_of_q.test | RUN | 0.268747 | 0.858602 | 0.966958 | 0.002069 | 0.002318 | 0.002168 |
| mp_none_of.test | RUN | 0.645834 | 1.997797 | 2.037550 | 0.002116 | 0.002194 | 0.002177 |
| mp_none_of_q.test | RUN | 0.260808 | 0.859312 | 0.908216 | 0.001940 | 0.002394 | 0.001926 |
| mp_replace_at.test | RUN | 0.310600 | 1.642928 | 1.296238 | 0.002222 | 0.002254 | 0.002130 |
| mp_replace_at_c.test | RUN | 0.314647 | 1.760148 | 1.348578 | 0.002172 | 0.001918 | 0.002140 |
| mp_for_each.test | RUN | 0.312040 | 1.302130 | 1.544040 | 0.001968 | 0.001932 | 0.002326 |
| mp_insert.test | RUN | 0.433657 | 3.460264 | 1.876423 | 0.002176 | 0.002182 | 0.002266 |
| mp_insert_2.test | RUN | 0.044301 | 0.078109 | 0.152481 | 0.000915 | 0.000794 | 0.001932 |
| mp_erase.test | RUN | 0.476247 | 3.688820 | 2.309512 | 0.001910 | 0.002156 | 0.002342 |
| mp_erase_2.test | RUN | 0.043052 | 0.076944 | 0.150808 | 0.000842 | 0.001005 | 0.002060 |
| mp_with_index.test | RUN | 0.681300 | 7.575382 | 5.015232 | 0.002412 | 0.002854 | 0.002664 |
| mp_with_index_cx.test | RUN | 0.004397 | 0.024414 | 0.032207 | 0.000873 | 0.000978 | 0.002065 |
| mp_from_sequence.test | RUN | 0.312992 | 1.990656 | 1.290390 | 0.002104 | 0.002222 | 0.002158 |
| mp_from_sequence_2.test | RUN | 0.317967 | 2.343915 | 1.429198 | 0.002166 | 0.002228 | 0.002207 |
| mp_from_sequence_3.test | RUN | 0.290962 | 1.045823 | 1.062004 | 0.002070 | 0.002228 | 0.001764 |
| mp_min_element.test | RUN | 0.283034 | 0.937231 | 0.930558 | 0.002024 | 0.002318 | 0.002196 |
| mp_min_element_2.test | RUN | 0.045268 | 0.081440 | 0.148137 | 0.000801 | 0.000952 | 0.001944 |
| mp_min_element_q.test | RUN | 0.277219 | 0.960112 | 0.990070 | 0.002003 | 0.002194 | 0.001944 |
| mp_min_element_q_2.test | RUN | 0.043165 | 0.083454 | 0.138061 | 0.000934 | 0.000910 | 0.002066 |
| mp_max_element.test | RUN | 0.280133 | 0.944021 | 0.991687 | 0.002208 | 0.002201 | 0.002260 |
| mp_max_element_2.test | RUN | 0.043797 | 0.076294 | 0.136143 | 0.000774 | 0.001026 | 0.002021 |
| mp_max_element_q.test | RUN | 0.273336 | 0.954001 | 0.946394 | 0.002213 | 0.002474 | 0.002327 |
| mp_max_element_q_2.test | RUN | 0.044739 | 0.081946 | 0.136529 | 0.000890 | 0.000951 | 0.002148 |
| mp_nth_element.test | RUN | 0.298282 | 1.340895 | 1.117485 | 0.001941 | 0.002051 | 0.002143 |
| mp_nth_element_q.test | RUN | 0.300313 | 1.354259 | 1.122147 | 0.002012 | 0.001935 | 0.001851 |
| mp_back.test | RUN | 0.302770 | 1.075112 | 1.162658 | 0.002078 | 0.002218 | 0.002172 |
| mp_back_2.test | RUN | 0.044799 | 0.081724 | 0.145979 | 0.000946 | 0.000921 | 0.002082 |
| mp_pop_back.test | RUN | 0.668014 | 7.104749 | 2.202572 | 0.002262 | 0.002097 | 0.001938 |
| mp_pop_back_2.test | RUN | 0.042834 | 0.074009 | 0.123545 | 0.000925 | 0.000973 | 0.002253 |
| mp_flatten.test | RUN | 0.336854 | 2.525498 | 1.514727 | 0.002238 | 0.002196 | 0.001896 |
| mp_rotate_left.test | RUN | 0.333816 | 2.657189 | 1.534881 | 0.002143 | 0.002192 | 0.002256 |
| mp_rotate_right.test | RUN | 0.329600 | 2.668183 | 1.536065 | 0.001933 | 0.002162 | 0.001867 |
| mp_power_set.test | RUN | 0.336173 | 2.648654 | 1.506563 | 0.002188 | 0.002180 | 0.001804 |
| mp_partial_sum.test | RUN | 0.407785 | 3.450974 | 1.727333 | 0.001800 | 0.001814 | 0.001930 |
| mp_iterate.test | RUN | 0.296848 | 1.091136 | 1.225566 | 0.002125 | 0.002283 | 0.002148 |
| mp_pairwise_fold.test | RUN | 0.342448 | 2.361459 | 1.557293 | 0.002159 | 0.002100 | 0.001914 |
| mp_pairwise_fold_q.test | RUN | 0.266488 | 0.725141 | 0.984069 | 0.002134 | 0.002148 | 0.002174 |
| mp_sliding_fold.test | RUN | 0.547626 | 4.317667 | 2.841175 | 0.002160 | 0.002139 | 0.002150 |
| mp_sliding_fold_q.test | RUN | 0.550732 | 4.289826 | 2.745763 | 0.002000 | 0.002198 | 0.002139 |
| mp_intersperse.test | RUN | 0.319414 | 1.705461 | 1.382569 | 0.001930 | 0.002336 | 0.002190 |
| mp_split.test | RUN | 0.405013 | 3.302067 | 1.811531 | 0.002186 | 0.002294 | 0.002176 |
| mp_join.test | RUN | 0.399653 | 2.734820 | 1.579976 | 0.002056 | 0.002366 | 0.002152 |
| integral.test | RUN | 0.282121 | 1.235913 | 1.027272 | 0.001944 | 0.002248 | 0.002151 |
| mp_value.test | RUN | 0.017639 | 0.042988 | 0.064248 | 0.000924 | 0.001005 | 0.002063 |
| mp_identity.test | RUN | 0.274910 | 1.129142 | 0.986274 | 0.002108 | 0.001894 | 0.002270 |
| mp_inherit.test | RUN | 0.258220 | 0.814471 | 0.905400 | 0.002668 | 0.002186 | 0.002252 |
| mp_if.test | RUN | 0.263608 | 0.945808 | 0.956681 | 0.001950 | 0.002079 | 0.002130 |
| mp_if_sf.test | RUN | 0.253458 | 0.697516 | 0.820199 | 0.002030 | 0.002435 | 0.002242 |
| mp_eval_if.test | RUN | 0.262509 | 0.974729 | 0.947828 | 0.002154 | 0.002169 | 0.002273 |
| mp_eval_if_sf.test | RUN | 0.257944 | 0.709737 | 0.845100 | 0.002142 | 0.002184 | 0.002164 |
| mp_valid.test | RUN | 0.260047 | 0.750024 | 0.834966 | 0.002021 | 0.002168 | 0.002250 |
| mp_defer.test | RUN | 0.269929 | 1.185856 | 1.046545 | 0.002020 | 0.002273 | 0.002259 |
| mp_quote.test | RUN | 0.275208 | 1.225246 | 1.051553 | 0.002186 | 0.002274 | 0.002231 |
| mp_invoke_q.test | RUN | 0.296184 | 1.612069 | 1.171322 | 0.002170 | 0.002312 | 0.002335 |
| mp_invoke_q_sf.test | RUN | 0.252867 | 0.716046 | 0.856318 | 0.002212 | 0.002381 | 0.002099 |
| mp_quote_trait.test | RUN | 0.276059 | 1.341493 | 1.026859 | 0.001953 | 0.002036 | 0.002317 |
| mp_cond.test | RUN | 0.260041 | 0.816890 | 0.911772 | 0.002131 | 0.002160 | 0.002110 |
| mp_cond_sf.test | RUN | 0.253832 | 0.739471 | 0.858800 | 0.002154 | 0.002086 | 0.002087 |
| mp_not_fn.test | RUN | 0.251418 | 0.703289 | 0.854400 | 0.002190 | 0.002371 | 0.002065 |
| mp_eval_if_not.test | RUN | 0.264074 | 0.994222 | 0.932221 | 0.001911 | 0.002241 | 0.002206 |
| mp_eval_or.test | RUN | 0.250550 | 0.548921 | 0.897651 | 0.002036 | 0.002133 | 0.002208 |
| mp_compose.test | RUN | 0.277626 | 1.623394 | 1.119383 | 0.001794 | 0.002143 | 0.002239 |
| mp_valid_and_true.test | RUN | 0.256610 | 0.871676 | 0.892381 | 0.001898 | 0.002221 | 0.002084 |
| mp_compose_sf.test | COMPILE | 0.061961 | 0.101668 | 0.213668 | — | — | — |
| integer_sequence.test | RUN | 0.300635 | 1.628842 | 1.233597 | 0.002004 | 0.002221 | 0.002257 |
| tuple_for_each.test | RUN | 0.263726 | 0.650535 | 0.933943 | 0.001876 | 0.002116 | 0.001627 |
| tuple_for_each_cx.test | COMPILE | 0.038840 | 0.088270 | 0.172240 | — | — | — |
| tuple_apply.test | RUN | 0.271579 | 0.624442 | 0.982749 | 0.002144 | 0.002256 | 0.001896 |
| tuple_apply_cx.test | COMPILE | 0.073624 | 0.129024 | 0.237552 | — | — | — |
| construct_from_tuple.test | RUN | 0.268427 | 0.673581 | 0.872933 | 0.002007 | 0.002302 | 0.001729 |
| construct_from_tuple_cx.test | COMPILE | 0.075062 | 0.130852 | 0.208374 | — | — | — |
| tuple_transform.test | RUN | 0.413729 | 1.373522 | 1.880533 | 0.002174 | 0.002206 | 0.002121 |
| tuple_transform_2.test | RUN | 0.361452 | 1.034063 | 1.737410 | 0.002162 | 0.001821 | 0.001926 |
| tuple_transform_cx.test | COMPILE | 0.065466 | 0.120044 | 0.188830 | — | — | — |
| mp_set_contains.test | RUN | 0.260014 | 0.857347 | 0.948925 | 0.001922 | 0.002264 | 0.001896 |
| mp_set_push_back.test | RUN | 0.325260 | 2.603340 | 1.485957 | 0.002336 | 0.002190 | 0.001873 |
| mp_set_push_front.test | RUN | 0.392386 | 3.096842 | 1.730345 | 0.001989 | 0.002258 | 0.002244 |
| mp_is_set.test | RUN | 0.267435 | 0.884675 | 0.956215 | 0.002147 | 0.002174 | 0.001952 |
| mp_set_union.test | RUN | 0.329456 | 2.239868 | 1.452018 | 0.002072 | 0.002398 | 0.002376 |
| mp_set_union_sf.test | RUN | 0.262917 | 0.702226 | 0.898994 | 0.001961 | 0.002252 | 0.001875 |
| mp_set_difference.test | RUN | 0.294390 | 1.731342 | 1.258111 | 0.001844 | 0.002088 | 0.002180 |
| mp_set_difference_sf.test | RUN | 0.254984 | 0.705082 | 0.897466 | 0.002232 | 0.002200 | 0.002231 |
| mp_set_intersection.test | RUN | 0.298677 | 1.711413 | 1.196614 | 0.002178 | 0.002148 | 0.001832 |
| mp_set_intersection_sf.test | RUN | 0.256759 | 0.691668 | 0.905706 | 0.001879 | 0.002282 | 0.001877 |
| mp_all.test | RUN | 0.260495 | 0.857862 | 0.973381 | 0.002065 | 0.002036 | 0.002282 |
| mp_all_2.test | RUN | 0.648782 | 1.804983 | 2.024690 | 0.002179 | 0.002373 | 0.002250 |
| mp_and.test | RUN | 0.262318 | 0.819316 | 0.909399 | 0.001833 | 0.002086 | 0.002250 |
| mp_any.test | RUN | 0.264547 | 0.867116 | 0.933872 | 0.001850 | 0.002405 | 0.002088 |
| mp_any_2.test | RUN | 0.642802 | 1.838975 | 1.937303 | 0.002073 | 0.002304 | 0.002185 |
| mp_or.test | RUN | 0.268718 | 0.821374 | 0.986885 | 0.001838 | 0.002159 | 0.002050 |
| mp_same.test | RUN | 0.446022 | 1.422166 | 1.473298 | 0.001932 | 0.001898 | 0.002072 |
| mp_plus.test | RUN | 0.276316 | 0.958230 | 0.946299 | 0.001900 | 0.002137 | 0.002173 |
| mp_less.test | RUN | 0.265173 | 0.858115 | 0.918794 | 0.001984 | 0.002082 | 0.002288 |
| mp_min.test | RUN | 0.255486 | 0.818205 | 0.874517 | 0.002133 | 0.002344 | 0.002118 |
| mp_max.test | RUN | 0.265424 | 0.863750 | 0.910597 | 0.001930 | 0.002232 | 0.001939 |
| mp_similar.test | RUN | 0.259354 | 0.847162 | 0.911832 | 0.001894 | 0.002204 | 0.002266 |
| mp_map_find.test | RUN | 0.296658 | 1.545564 | 1.169701 | 0.002117 | 0.002107 | 0.002314 |
| mp_map_find_2.test | RUN | 0.280985 | 0.632938 | 1.068651 | 0.002138 | 0.002319 | 0.002138 |
| mp_map_find_3.test | RUN | 0.283567 | 0.615612 | 1.054088 | 0.001880 | 0.001826 | 0.001779 |
| mp_map_find_4.test | RUN | 0.265735 | 0.574451 | 0.915073 | 0.001917 | 0.002250 | 0.002088 |
| mp_map_contains.test | RUN | 0.276698 | 0.883274 | 0.965663 | 0.002076 | 0.002319 | 0.002240 |
| mp_map_insert.test | RUN | 0.393669 | 2.874109 | 1.668023 | 0.001926 | 0.002382 | 0.002083 |
| mp_map_replace.test | RUN | 0.420718 | 3.244645 | 1.893294 | 0.001877 | 0.002132 | 0.001945 |
| mp_map_erase.test | RUN | 0.337774 | 2.417351 | 1.609845 | 0.002297 | 0.002148 | 0.002028 |
| mp_map_update.test | RUN | 0.303411 | 1.595582 | 1.251991 | 0.002198 | 0.002158 | 0.002216 |
| mp_map_update_q.test | RUN | 0.301445 | 1.618821 | 1.268785 | 0.002178 | 0.001946 | 0.001918 |
| mp_map_keys.test | RUN | 0.299991 | 1.442518 | 1.242607 | 0.002020 | 0.002115 | 0.001951 |
| mp_is_map.test | RUN | 0.283125 | 0.902196 | 1.014150 | 0.002170 | 0.002086 | 0.001992 |
| mp_bind.test | RUN | 0.325490 | 2.529595 | 1.466125 | 0.002115 | 0.002110 | 0.002313 |
| mp_bind_q.test | RUN | 0.297519 | 1.593157 | 1.201276 | 0.002012 | 0.002217 | 0.001911 |
| mp_bind_front.test | RUN | 0.291527 | 1.798238 | 1.119604 | 0.001761 | 0.001925 | 0.001370 |
| mp_bind_back.test | RUN | 0.298560 | 1.739676 | 1.179281 | 0.002138 | 0.002180 | 0.002214 |
| mp_lambda.test | RUN | 0.580663 | 5.134072 | 2.874369 | 0.001906 | 0.001763 | 0.001719 |
| mpl.test | RUN | 0.563783 | 1.470898 | 2.333094 | 0.002254 | 0.002169 | 0.002131 |
| mpl_list.test | RUN | 0.340548 | 1.034259 | 1.132686 | 0.002063 | 0.002143 | 0.002196 |
| mpl_tuple.test | RUN | 0.352950 | 1.036856 | 1.162454 | 0.001719 | 0.002008 | 0.002255 |
| version.test | RUN | 0.170532 | 0.391077 | 0.593083 | 0.002038 | 0.002077 | 0.002070 |
| mp11_version.test | RUN | 0.211258 | 0.449588 | 0.700246 | 0.001947 | 0.002104 | 0.002176 |
| list_version.test | RUN | 0.176504 | 0.415691 | 0.579457 | 0.001941 | 0.002326 | 0.002168 |
| algorithm_version.test | RUN | 0.191635 | 0.422288 | 0.633704 | 0.002140 | 0.001986 | 0.001810 |
| integral_version.test | RUN | 0.170809 | 0.396168 | 0.569137 | 0.002085 | 0.001942 | 0.002188 |
| utility_version.test | RUN | 0.178259 | 0.405585 | 0.590131 | 0.002090 | 0.002294 | 0.001954 |
| function_version.test | RUN | 0.179572 | 0.390503 | 0.591717 | 0.001818 | 0.002311 | 0.002256 |
| map_version.test | RUN | 0.197615 | 0.435295 | 0.645908 | 0.002088 | 0.002169 | 0.001882 |
| set_version.test | RUN | 0.182011 | 0.399331 | 0.624469 | 0.002054 | 0.002065 | 0.001952 |
| bind_version.test | RUN | 0.193876 | 0.416268 | 0.637073 | 0.001863 | 0.002293 | 0.002186 |
| integer_sequence_version.test | RUN | 0.172148 | 0.395785 | 0.573886 | 0.002073 | 0.002230 | 0.001643 |
| tuple_version.test | RUN | 0.192549 | 0.434963 | 0.433515 | 0.001639 | 0.002431 | 0.001619 |
| mpl_version.test | RUN | 0.174142 | 0.339425 | 0.587320 | 0.001742 | 0.001714 | 0.001684 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 1.004×, 0.999× | 0.997× |
| clang | 1.087× | 1.090×, 1.085× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001216s | 0.001204s | 0.001228s | byte equality observed |
| bind_version.test | 10 | 0.001214s | 0.001208s | 0.001241s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001224s | 0.001224s | 0.001211s | byte equality observed |
| function_version.test | 10 | 0.001217s | 0.001225s | 0.001286s | byte equality observed |
| integer_sequence.test | 10 | 0.001222s | 0.001230s | 0.001309s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001214s | 0.001206s | 0.001310s | byte equality observed |
| integral.test | 10 | 0.001232s | 0.001227s | 0.001275s | byte equality observed |
| integral_version.test | 10 | 0.001217s | 0.001213s | 0.001276s | byte equality observed |
| list_version.test | 10 | 0.001219s | 0.001208s | 0.001314s | byte equality observed |
| map_version.test | 10 | 0.001210s | 0.001208s | 0.001304s | byte equality observed |
| mp11_version.test | 10 | 0.001228s | 0.001203s | 0.001292s | byte equality observed |
| mp_all.test | 10 | 0.001215s | 0.001211s | 0.001294s | byte equality observed |
| mp_all_2.test | 10 | 0.001219s | 0.001217s | 0.001308s | byte equality observed |
| mp_all_of.test | 10 | 0.001214s | 0.001213s | 0.001332s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001227s | 0.001212s | 0.001317s | byte equality observed |
| mp_and.test | 10 | 0.001237s | 0.001227s | 0.001288s | byte equality observed |
| mp_any.test | 10 | 0.001322s | 0.001223s | 0.001312s | byte equality observed |
| mp_any_2.test | 10 | 0.001323s | 0.001226s | 0.001296s | byte equality observed |
| mp_any_of.test | 10 | 0.001319s | 0.001230s | 0.001318s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001352s | 0.001219s | 0.001304s | byte equality observed |
| mp_append.test | 10 | 0.001286s | 0.001226s | 0.001297s | byte equality observed |
| mp_append_2.test | 10 | 0.001330s | 0.001288s | 0.001322s | byte equality observed |
| mp_append_3.test | 10 | 0.000612s | 0.000603s | 0.001305s | byte equality observed |
| mp_append_sf.test | 10 | 0.001302s | 0.001336s | 0.001262s | byte equality observed |
| mp_apply_q.test | 10 | 0.001281s | 0.001356s | 0.001311s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001323s | 0.001339s | 0.001298s | byte equality observed |
| mp_assign.test | 10 | 0.001302s | 0.001364s | 0.001311s | byte equality observed |
| mp_assign_2.test | 10 | 0.000610s | 0.000599s | 0.001266s | byte equality observed |
| mp_at.test | 10 | 0.001351s | 0.001312s | 0.001292s | byte equality observed |
| mp_at_2.test | 10 | 0.000611s | 0.000617s | 0.001320s | byte equality observed |
| mp_at_sf.test | 10 | 0.001332s | 0.001321s | 0.001288s | byte equality observed |
| mp_back.test | 10 | 0.001320s | 0.001336s | 0.001323s | byte equality observed |
| mp_back_2.test | 10 | 0.000611s | 0.000632s | 0.001269s | byte equality observed |
| mp_bind.test | 10 | 0.001295s | 0.001344s | 0.001299s | byte equality observed |
| mp_bind_back.test | 10 | 0.001262s | 0.001317s | 0.001274s | byte equality observed |
| mp_bind_front.test | 10 | 0.001308s | 0.001284s | 0.001302s | byte equality observed |
| mp_bind_q.test | 10 | 0.001309s | 0.001266s | 0.001338s | byte equality observed |
| mp_clear.test | 10 | 0.001293s | 0.001351s | 0.001334s | byte equality observed |
| mp_clear_2.test | 10 | 0.000616s | 0.000597s | 0.001267s | byte equality observed |
| mp_compose.test | 10 | 0.001289s | 0.001277s | 0.001328s | byte equality observed |
| mp_cond.test | 10 | 0.001329s | 0.001344s | 0.001314s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001306s | 0.001309s | 0.001302s | byte equality observed |
| mp_contains.test | 10 | 0.001331s | 0.001317s | 0.001293s | byte equality observed |
| mp_copy_if.test | 10 | 0.001331s | 0.001308s | 0.001331s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001283s | 0.001328s | 0.001302s | byte equality observed |
| mp_count.test | 10 | 0.001311s | 0.001296s | 0.001298s | byte equality observed |
| mp_count_if.test | 10 | 0.001329s | 0.001329s | 0.001303s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001336s | 0.001318s | 0.001311s | byte equality observed |
| mp_defer.test | 10 | 0.001295s | 0.001308s | 0.001340s | byte equality observed |
| mp_drop.test | 10 | 0.001311s | 0.001316s | 0.001285s | byte equality observed |
| mp_drop_2.test | 10 | 0.000608s | 0.000599s | 0.001285s | byte equality observed |
| mp_empty.test | 10 | 0.001292s | 0.001331s | 0.001238s | byte equality observed |
| mp_empty_2.test | 10 | 0.000608s | 0.000610s | 0.001237s | byte equality observed |
| mp_erase.test | 10 | 0.001345s | 0.001356s | 0.001290s | byte equality observed |
| mp_erase_2.test | 10 | 0.000603s | 0.000606s | 0.001276s | byte equality observed |
| mp_eval_if.test | 10 | 0.001295s | 0.001320s | 0.001269s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001281s | 0.001324s | 0.001284s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001313s | 0.001318s | 0.001313s | byte equality observed |
| mp_eval_or.test | 10 | 0.001320s | 0.001320s | 0.001329s | byte equality observed |
| mp_fill.test | 10 | 0.001308s | 0.001318s | 0.001285s | byte equality observed |
| mp_fill_2.test | 10 | 0.000613s | 0.000605s | 0.001276s | byte equality observed |
| mp_filter.test | 10 | 0.001335s | 0.001306s | 0.001277s | byte equality observed |
| mp_find.test | 10 | 0.001297s | 0.001316s | 0.001297s | byte equality observed |
| mp_find_if.test | 10 | 0.001304s | 0.001293s | 0.001296s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001335s | 0.001326s | 0.001367s | byte equality observed |
| mp_flatten.test | 10 | 0.001329s | 0.001356s | 0.001302s | byte equality observed |
| mp_fold.test | 10 | 0.001353s | 0.001328s | 0.001312s | byte equality observed |
| mp_fold_q.test | 10 | 0.001361s | 0.001328s | 0.001388s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001323s | 0.001337s | 0.001308s | byte equality observed |
| mp_for_each.test | 10 | 0.001322s | 0.001361s | 0.001306s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001353s | 0.001289s | 0.001292s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001330s | 0.001319s | 0.001296s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001300s | 0.001310s | 0.001326s | byte equality observed |
| mp_front.test | 10 | 0.001311s | 0.001332s | 0.001287s | byte equality observed |
| mp_front_2.test | 10 | 0.000608s | 0.000604s | 0.001244s | byte equality observed |
| mp_identity.test | 10 | 0.001319s | 0.001311s | 0.001291s | byte equality observed |
| mp_if.test | 10 | 0.001320s | 0.001290s | 0.001303s | byte equality observed |
| mp_if_sf.test | 10 | 0.001293s | 0.001303s | 0.001274s | byte equality observed |
| mp_inherit.test | 10 | 0.001339s | 0.001302s | 0.001309s | byte equality observed |
| mp_insert.test | 10 | 0.001346s | 0.001312s | 0.001286s | byte equality observed |
| mp_insert_2.test | 10 | 0.000616s | 0.000622s | 0.001275s | byte equality observed |
| mp_intersperse.test | 10 | 0.001275s | 0.001291s | 0.001265s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001297s | 0.001331s | 0.001255s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001276s | 0.001283s | 0.001268s | byte equality observed |
| mp_iota.test | 10 | 0.001273s | 0.001350s | 0.001314s | byte equality observed |
| mp_iota_2.test | 10 | 0.001327s | 0.001374s | 0.001303s | byte equality observed |
| mp_is_list.test | 10 | 0.001345s | 0.001316s | 0.001301s | byte equality observed |
| mp_is_map.test | 10 | 0.001327s | 0.001309s | 0.001268s | byte equality observed |
| mp_is_set.test | 10 | 0.001354s | 0.001350s | 0.001299s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001277s | 0.001270s | 0.001334s | byte equality observed |
| mp_iterate.test | 10 | 0.001322s | 0.001267s | 0.001289s | byte equality observed |
| mp_join.test | 10 | 0.001322s | 0.001328s | 0.001291s | byte equality observed |
| mp_lambda.test | 10 | 0.001334s | 0.001344s | 0.001273s | byte equality observed |
| mp_less.test | 10 | 0.001286s | 0.001318s | 0.001268s | byte equality observed |
| mp_list_c.test | 10 | 0.001282s | 0.001301s | 0.001298s | byte equality observed |
| mp_list_v.test | 10 | 0.000614s | 0.000608s | 0.001267s | byte equality observed |
| mp_map_contains.test | 10 | 0.001325s | 0.001292s | 0.001275s | byte equality observed |
| mp_map_erase.test | 10 | 0.001342s | 0.001329s | 0.001323s | byte equality observed |
| mp_map_find.test | 10 | 0.001345s | 0.001351s | 0.001297s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001276s | 0.001319s | 0.001342s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001334s | 0.001295s | 0.001337s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001303s | 0.001285s | 0.001315s | byte equality observed |
| mp_map_insert.test | 10 | 0.001307s | 0.001358s | 0.001307s | byte equality observed |
| mp_map_keys.test | 10 | 0.001319s | 0.001317s | 0.001363s | byte equality observed |
| mp_map_replace.test | 10 | 0.001333s | 0.001335s | 0.001312s | byte equality observed |
| mp_map_update.test | 10 | 0.001340s | 0.001336s | 0.001284s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001300s | 0.001319s | 0.001318s | byte equality observed |
| mp_max.test | 10 | 0.001321s | 0.001306s | 0.001322s | byte equality observed |
| mp_max_element.test | 10 | 0.001302s | 0.001292s | 0.001279s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000602s | 0.000619s | 0.001338s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001317s | 0.001362s | 0.001257s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000603s | 0.000613s | 0.001342s | byte equality observed |
| mp_min.test | 10 | 0.001338s | 0.001337s | 0.001282s | byte equality observed |
| mp_min_element.test | 10 | 0.001316s | 0.001369s | 0.001286s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000609s | 0.000597s | 0.001262s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001313s | 0.001351s | 0.001279s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000611s | 0.000621s | 0.001247s | byte equality observed |
| mp_none_of.test | 10 | 0.001311s | 0.001289s | 0.001283s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001348s | 0.001337s | 0.001288s | byte equality observed |
| mp_not_fn.test | 10 | 0.001294s | 0.001327s | 0.001330s | byte equality observed |
| mp_nth_element.test | 10 | 0.001291s | 0.001324s | 0.001358s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001308s | 0.001359s | 0.001321s | byte equality observed |
| mp_or.test | 10 | 0.001332s | 0.001305s | 0.001316s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001324s | 0.001334s | 0.001273s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001309s | 0.001308s | 0.001281s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001328s | 0.001324s | 0.001283s | byte equality observed |
| mp_partition.test | 10 | 0.001334s | 0.001278s | 0.001295s | byte equality observed |
| mp_partition_q.test | 10 | 0.001319s | 0.001286s | 0.001284s | byte equality observed |
| mp_plus.test | 10 | 0.001291s | 0.001302s | 0.001325s | byte equality observed |
| mp_pop_back.test | 10 | 0.001361s | 0.001326s | 0.001304s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000604s | 0.000601s | 0.001266s | byte equality observed |
| mp_pop_front.test | 10 | 0.001310s | 0.001320s | 0.001282s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000613s | 0.000609s | 0.001332s | byte equality observed |
| mp_power_set.test | 10 | 0.001292s | 0.001323s | 0.001353s | byte equality observed |
| mp_product.test | 10 | 0.001322s | 0.001305s | 0.001311s | byte equality observed |
| mp_push_back.test | 10 | 0.001356s | 0.001332s | 0.001276s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000591s | 0.000594s | 0.001265s | byte equality observed |
| mp_push_front.test | 10 | 0.001349s | 0.001330s | 0.001243s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000614s | 0.000606s | 0.001320s | byte equality observed |
| mp_quote.test | 10 | 0.001300s | 0.001352s | 0.001301s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001307s | 0.001345s | 0.001368s | byte equality observed |
| mp_remove.test | 10 | 0.001340s | 0.001272s | 0.001324s | byte equality observed |
| mp_remove_if.test | 10 | 0.001327s | 0.001318s | 0.001331s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001330s | 0.001292s | 0.001359s | byte equality observed |
| mp_rename.test | 10 | 0.001311s | 0.001264s | 0.001286s | byte equality observed |
| mp_rename_2.test | 10 | 0.000609s | 0.000619s | 0.001281s | byte equality observed |
| mp_rename_v.test | 10 | 0.000627s | 0.000622s | 0.001283s | byte equality observed |
| mp_repeat.test | 10 | 0.001347s | 0.001365s | 0.001268s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000616s | 0.000606s | 0.001306s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001270s | 0.001368s | 0.001278s | byte equality observed |
| mp_replace.test | 10 | 0.001284s | 0.001316s | 0.001369s | byte equality observed |
| mp_replace_at.test | 10 | 0.001325s | 0.001352s | 0.001347s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001292s | 0.001286s | 0.001272s | byte equality observed |
| mp_replace_front.test | 10 | 0.001330s | 0.001348s | 0.001265s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000600s | 0.000588s | 0.001282s | byte equality observed |
| mp_replace_if.test | 10 | 0.001304s | 0.001329s | 0.001312s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001294s | 0.001339s | 0.001322s | byte equality observed |
| mp_replace_second.test | 10 | 0.001297s | 0.001295s | 0.001251s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000590s | 0.000617s | 0.001320s | byte equality observed |
| mp_replace_third.test | 10 | 0.001319s | 0.001309s | 0.001283s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000615s | 0.000614s | 0.001305s | byte equality observed |
| mp_reverse.test | 10 | 0.001319s | 0.001348s | 0.001275s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001292s | 0.001334s | 0.001248s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001273s | 0.001335s | 0.001277s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001284s | 0.001353s | 0.001297s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001329s | 0.001338s | 0.001260s | byte equality observed |
| mp_same.test | 10 | 0.001304s | 0.001312s | 0.001243s | byte equality observed |
| mp_second.test | 10 | 0.001313s | 0.001334s | 0.001273s | byte equality observed |
| mp_second_2.test | 10 | 0.000606s | 0.000605s | 0.001330s | byte equality observed |
| mp_set_contains.test | 10 | 0.001315s | 0.001326s | 0.001384s | byte equality observed |
| mp_set_difference.test | 10 | 0.001323s | 0.001342s | 0.001324s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001297s | 0.001308s | 0.001329s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001285s | 0.001306s | 0.001331s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001329s | 0.001292s | 0.001292s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001285s | 0.001311s | 0.001323s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001325s | 0.001324s | 0.001338s | byte equality observed |
| mp_set_union.test | 10 | 0.001319s | 0.001329s | 0.001314s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001280s | 0.001306s | 0.001333s | byte equality observed |
| mp_similar.test | 10 | 0.001310s | 0.001340s | 0.001284s | byte equality observed |
| mp_size.test | 10 | 0.001305s | 0.001335s | 0.001288s | byte equality observed |
| mp_size_2.test | 10 | 0.000615s | 0.000614s | 0.001302s | byte equality observed |
| mp_slice.test | 10 | 0.001358s | 0.001325s | 0.001286s | byte equality observed |
| mp_slice_2.test | 10 | 0.000600s | 0.000624s | 0.001304s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001337s | 0.001347s | 0.001309s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001324s | 0.001383s | 0.001311s | byte equality observed |
| mp_sort.test | 10 | 0.001319s | 0.001314s | 0.001282s | byte equality observed |
| mp_sort_q.test | 10 | 0.001333s | 0.001308s | 0.001315s | byte equality observed |
| mp_split.test | 10 | 0.001314s | 0.001345s | 0.001279s | byte equality observed |
| mp_starts_with.test | 10 | 0.001330s | 0.001383s | 0.001273s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001332s | 0.001308s | 0.001298s | byte equality observed |
| mp_take.test | 10 | 0.001340s | 0.001309s | 0.001378s | byte equality observed |
| mp_take_2.test | 10 | 0.000606s | 0.000623s | 0.001256s | byte equality observed |
| mp_third.test | 10 | 0.001340s | 0.001334s | 0.001292s | byte equality observed |
| mp_third_2.test | 10 | 0.000610s | 0.000612s | 0.001259s | byte equality observed |
| mp_transform.test | 10 | 0.001309s | 0.001290s | 0.001270s | byte equality observed |
| mp_transform_2.test | 10 | 0.001292s | 0.001319s | 0.001316s | byte equality observed |
| mp_transform_front.test | 10 | 0.001322s | 0.001350s | 0.001265s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000607s | 0.000612s | 0.001294s | byte equality observed |
| mp_transform_if.test | 10 | 0.001297s | 0.001290s | 0.001272s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001292s | 0.001293s | 0.001263s | byte equality observed |
| mp_transform_q.test | 10 | 0.001325s | 0.001311s | 0.001311s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001272s | 0.001294s | 0.001322s | byte equality observed |
| mp_transform_second.test | 10 | 0.001319s | 0.001329s | 0.001270s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000606s | 0.000606s | 0.001368s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001325s | 0.001335s | 0.001260s | byte equality observed |
| mp_transform_third.test | 10 | 0.001298s | 0.001340s | 0.001267s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000601s | 0.000619s | 0.001283s | byte equality observed |
| mp_unique.test | 10 | 0.001279s | 0.001272s | 0.001289s | byte equality observed |
| mp_unique_if.test | 10 | 0.001307s | 0.001277s | 0.001350s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001344s | 0.001305s | 0.001297s | byte equality observed |
| mp_valid.test | 10 | 0.001350s | 0.001308s | 0.001322s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001329s | 0.001365s | 0.001282s | byte equality observed |
| mp_value.test | 10 | 0.000607s | 0.000612s | 0.001281s | byte equality observed |
| mp_with_index.test | 10 | 0.001735s | 0.001704s | 0.001589s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000586s | 0.000609s | 0.001296s | byte equality observed |
| mpl.test | 10 | 0.001327s | 0.001281s | 0.001324s | byte equality observed |
| mpl_list.test | 10 | 0.001292s | 0.001321s | 0.001333s | byte equality observed |
| mpl_tuple.test | 10 | 0.001303s | 0.001340s | 0.001315s | byte equality observed |
| mpl_version.test | 10 | 0.001326s | 0.001315s | 0.001326s | byte equality observed |
| set_version.test | 10 | 0.001266s | 0.001303s | 0.001293s | byte equality observed |
| tuple_apply.test | 10 | 0.001286s | 0.001317s | 0.001326s | byte equality observed |
| tuple_for_each.test | 10 | 0.001315s | 0.001324s | 0.001370s | byte equality observed |
| tuple_transform.test | 10 | 0.001293s | 0.001345s | 0.001302s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001285s | 0.001333s | 0.001296s | byte equality observed |
| tuple_version.test | 10 | 0.001273s | 0.001315s | 0.001337s | byte equality observed |
| utility_version.test | 10 | 0.001303s | 0.001309s | 0.001298s | byte equality observed |
| version.test | 10 | 0.001303s | 0.001331s | 0.001338s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
