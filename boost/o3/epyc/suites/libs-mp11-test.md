# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.048299 | 0.098355 | 0.197847 | — | — | — |
| complex_h.test | COMPILE | 0.200210 | 0.431792 | 0.720086 | — | — | — |
| mp_size.test | RUN | 0.228362 | 1.323454 | 0.952370 | 0.001868 | 0.001905 | 0.002058 |
| mp_size_2.test | RUN | 0.020313 | 0.047938 | 0.079307 | 0.000837 | 0.000874 | 0.002140 |
| mp_empty.test | RUN | 0.222696 | 0.849616 | 0.906141 | 0.001833 | 0.002106 | 0.001921 |
| mp_empty_2.test | RUN | 0.020939 | 0.051380 | 0.082622 | 0.000735 | 0.000864 | 0.002300 |
| mp_assign.test | RUN | 0.244661 | 1.734742 | 1.143333 | 0.001731 | 0.002136 | 0.002286 |
| mp_assign_2.test | RUN | 0.021787 | 0.047022 | 0.078282 | 0.000869 | 0.000970 | 0.001854 |
| mp_clear.test | RUN | 0.214355 | 0.772594 | 0.818872 | 0.001762 | 0.001836 | 0.002372 |
| mp_clear_2.test | RUN | 0.020325 | 0.047713 | 0.084579 | 0.000827 | 0.000737 | 0.002211 |
| mp_front.test | RUN | 0.233392 | 1.162783 | 0.972622 | 0.001515 | 0.001987 | 0.001926 |
| mp_front_2.test | RUN | 0.021145 | 0.047540 | 0.084714 | 0.000745 | 0.000787 | 0.002186 |
| mp_pop_front.test | RUN | 0.240228 | 1.252200 | 1.033359 | 0.001790 | 0.002045 | 0.002114 |
| mp_pop_front_2.test | RUN | 0.018859 | 0.047376 | 0.084858 | 0.000775 | 0.000953 | 0.001840 |
| mp_second.test | RUN | 0.236690 | 0.812651 | 0.961695 | 0.001852 | 0.001767 | 0.002268 |
| mp_second_2.test | RUN | 0.020522 | 0.050803 | 0.074980 | 0.000709 | 0.000827 | 0.002186 |
| mp_third.test | RUN | 0.228707 | 0.731722 | 0.838704 | 0.001835 | 0.001846 | 0.001796 |
| mp_third_2.test | RUN | 0.021441 | 0.051490 | 0.086719 | 0.000784 | 0.000840 | 0.002022 |
| mp_push_front.test | RUN | 0.288149 | 2.530985 | 1.524607 | 0.001831 | 0.001873 | 0.002372 |
| mp_push_front_2.test | RUN | 0.020958 | 0.050011 | 0.084950 | 0.000737 | 0.000906 | 0.002170 |
| mp_push_back.test | RUN | 0.296931 | 2.532850 | 1.588916 | 0.001931 | 0.001865 | 0.002250 |
| mp_push_back_2.test | RUN | 0.021178 | 0.046396 | 0.077726 | 0.000728 | 0.000886 | 0.002200 |
| mp_rename.test | RUN | 0.278902 | 2.368111 | 1.363860 | 0.001802 | 0.002186 | 0.002008 |
| mp_rename_2.test | RUN | 0.022584 | 0.047559 | 0.080134 | 0.000754 | 0.000968 | 0.002106 |
| mp_append.test | RUN | 0.280938 | 2.665092 | 1.390770 | 0.001758 | 0.002242 | 0.002250 |
| mp_append_2.test | RUN | 0.613197 | 5.918531 | 1.900211 | 0.001888 | 0.002068 | 0.001954 |
| mp_append_3.test | RUN | 0.020158 | 0.046763 | 0.085468 | 0.000767 | 0.000852 | 0.002096 |
| mp_append_sf.test | RUN | 0.226687 | 0.703752 | 0.883947 | 0.001754 | 0.002008 | 0.002031 |
| mp_replace_front.test | RUN | 0.269734 | 1.912190 | 1.126345 | 0.002022 | 0.002059 | 0.002090 |
| mp_replace_front_2.test | RUN | 0.020973 | 0.049291 | 0.082945 | 0.000743 | 0.000806 | 0.002096 |
| mp_replace_second.test | RUN | 0.243406 | 1.233886 | 1.163212 | 0.001850 | 0.001958 | 0.002304 |
| mp_replace_second_2.test | RUN | 0.019704 | 0.049105 | 0.083405 | 0.000712 | 0.000779 | 0.001906 |
| mp_replace_third.test | RUN | 0.246885 | 1.045616 | 1.026397 | 0.001919 | 0.001858 | 0.001886 |
| mp_replace_third_2.test | RUN | 0.019691 | 0.046560 | 0.083234 | 0.000762 | 0.000671 | 0.002170 |
| mp_apply_q.test | RUN | 0.274870 | 2.335901 | 1.365135 | 0.001636 | 0.001936 | 0.002200 |
| mp_apply_q_sf.test | RUN | 0.214890 | 0.693298 | 0.868203 | 0.001777 | 0.001946 | 0.002260 |
| mp_is_list.test | RUN | 0.224761 | 0.834190 | 0.899584 | 0.001465 | 0.002036 | 0.002276 |
| mp_list_c.test | RUN | 0.235827 | 0.974324 | 0.933148 | 0.001948 | 0.001968 | 0.001745 |
| mp_transform_front.test | RUN | 0.259019 | 1.902928 | 1.189726 | 0.001685 | 0.001726 | 0.001924 |
| mp_transform_front_2.test | RUN | 0.022005 | 0.049208 | 0.085690 | 0.000779 | 0.000863 | 0.001867 |
| mp_transform_second.test | RUN | 0.250180 | 1.634734 | 1.110020 | 0.001701 | 0.001799 | 0.001868 |
| mp_transform_second_2.test | RUN | 0.022377 | 0.048790 | 0.068807 | 0.000736 | 0.000827 | 0.001854 |
| mp_transform_third.test | RUN | 0.237485 | 1.230389 | 0.933841 | 0.001762 | 0.002022 | 0.001882 |
| mp_transform_third_2.test | RUN | 0.019361 | 0.047840 | 0.085792 | 0.000720 | 0.000835 | 0.002116 |
| mp_list_v.test | RUN | 0.022533 | 0.045791 | 0.086200 | 0.000721 | 0.000728 | 0.001943 |
| mp_rename_v.test | RUN | 0.020464 | 0.047107 | 0.081209 | 0.000749 | 0.000969 | 0.001831 |
| mp_is_value_list.test | RUN | 0.214196 | 0.511103 | 0.858988 | 0.001856 | 0.001974 | 0.001717 |
| mp_transform.test | RUN | 0.496866 | 3.934288 | 2.370561 | 0.002036 | 0.002032 | 0.001942 |
| mp_transform_2.test | RUN | 0.206931 | 0.477319 | 0.798295 | 0.001670 | 0.002204 | 0.002243 |
| mp_transform_q.test | RUN | 0.482397 | 3.704594 | 2.331945 | 0.001770 | 0.002174 | 0.001909 |
| mp_transform_q_2.test | RUN | 0.533973 | 1.263787 | 3.032798 | 0.001735 | 0.001832 | 0.001814 |
| mp_transform_sf.test | RUN | 0.238005 | 0.726099 | 0.940706 | 0.001904 | 0.001890 | 0.002168 |
| mp_transform_if.test | RUN | 0.284913 | 1.915258 | 1.312133 | 0.001949 | 0.001768 | 0.001957 |
| mp_transform_if_q.test | RUN | 0.277361 | 1.929839 | 1.368466 | 0.001726 | 0.001976 | 0.001786 |
| mp_filter.test | RUN | 0.254007 | 1.167110 | 1.059857 | 0.001814 | 0.002031 | 0.001855 |
| mp_fill.test | RUN | 0.236941 | 0.879767 | 0.968515 | 0.001756 | 0.002034 | 0.002276 |
| mp_fill_2.test | RUN | 0.037261 | 0.079502 | 0.156394 | 0.000692 | 0.000924 | 0.002188 |
| mp_count.test | RUN | 0.460186 | 2.062332 | 1.487271 | 0.001948 | 0.001768 | 0.002093 |
| mp_count_if.test | RUN | 0.450035 | 2.100626 | 1.651864 | 0.001661 | 0.002162 | 0.002196 |
| mp_count_if_q.test | RUN | 0.248781 | 1.369425 | 0.995536 | 0.001780 | 0.001932 | 0.001980 |
| mp_contains.test | RUN | 0.235664 | 0.828439 | 0.933978 | 0.001934 | 0.002067 | 0.002139 |
| mp_starts_with.test | RUN | 0.239750 | 0.823006 | 0.953997 | 0.001912 | 0.001857 | 0.002025 |
| mp_starts_with_sf.test | RUN | 0.232654 | 0.724948 | 0.904307 | 0.001768 | 0.001990 | 0.002074 |
| mp_repeat.test | RUN | 0.331910 | 3.370241 | 1.628544 | 0.001944 | 0.001685 | 0.001683 |
| mp_repeat_2.test | RUN | 0.039026 | 0.081965 | 0.152471 | 0.000680 | 0.000789 | 0.002026 |
| mp_repeat_3.test | RUN | 0.299515 | 0.455522 | 0.883337 | 0.001802 | 0.002038 | 0.001993 |
| mp_product.test | RUN | 0.267487 | 2.036462 | 1.248323 | 0.001959 | 0.001946 | 0.002013 |
| mp_drop.test | RUN | 0.292233 | 2.505190 | 1.423283 | 0.001704 | 0.001966 | 0.002184 |
| mp_drop_2.test | RUN | 0.038549 | 0.077776 | 0.137565 | 0.000693 | 0.000828 | 0.002344 |
| mp_iota.test | RUN | 0.278189 | 2.151050 | 1.270560 | 0.001854 | 0.002253 | 0.002110 |
| mp_iota_2.test | RUN | 0.296467 | 2.619357 | 1.429635 | 0.001886 | 0.002093 | 0.002149 |
| mp_at.test | RUN | 0.247403 | 1.139594 | 0.977877 | 0.001834 | 0.001998 | 0.002205 |
| mp_at_2.test | RUN | 0.037390 | 0.077110 | 0.143108 | 0.000798 | 0.000767 | 0.002162 |
| mp_at_sf.test | RUN | 0.230081 | 0.728728 | 0.837712 | 0.001831 | 0.001953 | 0.002064 |
| mp_take.test | RUN | 0.380256 | 3.315422 | 1.618871 | 0.001778 | 0.002027 | 0.002065 |
| mp_take_2.test | RUN | 0.038100 | 0.076372 | 0.147729 | 0.000716 | 0.000839 | 0.001914 |
| mp_slice.test | RUN | 0.435449 | 3.358253 | 2.096706 | 0.001629 | 0.002142 | 0.002197 |
| mp_slice_2.test | RUN | 0.038871 | 0.080930 | 0.127759 | 0.000832 | 0.000872 | 0.002010 |
| mp_replace.test | RUN | 0.282002 | 1.730942 | 1.388154 | 0.001986 | 0.001954 | 0.002132 |
| mp_replace_if.test | RUN | 0.275301 | 1.523034 | 1.249060 | 0.001924 | 0.002106 | 0.002145 |
| mp_replace_if_q.test | RUN | 0.261480 | 1.536333 | 1.245690 | 0.001748 | 0.002026 | 0.002026 |
| mp_copy_if.test | RUN | 0.280704 | 2.163355 | 1.234774 | 0.001806 | 0.002051 | 0.001825 |
| mp_copy_if_q.test | RUN | 0.276362 | 2.161587 | 1.263975 | 0.001948 | 0.002108 | 0.001890 |
| mp_remove.test | RUN | 0.265058 | 1.430521 | 1.253215 | 0.001736 | 0.001760 | 0.001980 |
| mp_remove_if.test | RUN | 0.277554 | 2.264314 | 1.288109 | 0.001869 | 0.002154 | 0.002220 |
| mp_remove_if_q.test | RUN | 0.283540 | 2.239488 | 1.323816 | 0.001898 | 0.001868 | 0.001868 |
| mp_partition.test | RUN | 0.288389 | 2.110860 | 1.391930 | 0.002042 | 0.001834 | 0.002187 |
| mp_partition_q.test | RUN | 0.286797 | 2.118691 | 1.407028 | 0.001928 | 0.001864 | 0.002252 |
| mp_sort.test | RUN | 0.260965 | 1.524396 | 1.256819 | 0.001560 | 0.001920 | 0.002108 |
| mp_sort_q.test | RUN | 0.279059 | 1.535578 | 1.192580 | 0.001942 | 0.001888 | 0.002302 |
| mp_find.test | RUN | 0.267589 | 1.644104 | 1.128933 | 0.002008 | 0.001837 | 0.002110 |
| mp_find_if.test | RUN | 0.253469 | 1.518173 | 1.025341 | 0.001966 | 0.001826 | 0.002305 |
| mp_find_if_q.test | RUN | 0.269633 | 1.714592 | 1.242463 | 0.001800 | 0.002023 | 0.002138 |
| mp_reverse.test | RUN | 0.544404 | 4.376472 | 2.254082 | 0.001940 | 0.002172 | 0.002208 |
| mp_fold.test | RUN | 0.902772 | 3.345497 | 5.584427 | 0.001942 | 0.001962 | 0.001659 |
| mp_fold_q.test | RUN | 0.259953 | 1.536652 | 1.192755 | 0.001748 | 0.002002 | 0.002137 |
| mp_fold_q_sf.test | RUN | 0.232945 | 0.713903 | 0.952975 | 0.001736 | 0.002122 | 0.002066 |
| mp_reverse_fold.test | RUN | 0.434996 | 3.286332 | 1.668647 | 0.001733 | 0.002168 | 0.001772 |
| mp_reverse_fold_q.test | RUN | 0.267907 | 1.856225 | 1.196654 | 0.001856 | 0.001943 | 0.001976 |
| mp_unique.test | RUN | 0.257720 | 1.699012 | 1.231011 | 0.001594 | 0.002250 | 0.002193 |
| mp_unique_if.test | RUN | 0.472559 | 3.454338 | 1.826506 | 0.001726 | 0.002282 | 0.001852 |
| mp_unique_if_q.test | RUN | 0.387545 | 3.372312 | 1.542364 | 0.001837 | 0.002042 | 0.002262 |
| mp_all_of.test | RUN | 0.599497 | 1.898071 | 2.054411 | 0.002018 | 0.002163 | 0.002134 |
| mp_all_of_q.test | RUN | 0.235685 | 0.843675 | 0.850302 | 0.001941 | 0.002154 | 0.002219 |
| mp_any_of.test | RUN | 0.612665 | 1.952705 | 2.047654 | 0.001678 | 0.002228 | 0.002037 |
| mp_any_of_q.test | RUN | 0.231197 | 0.859951 | 0.965432 | 0.001904 | 0.002029 | 0.001873 |
| mp_none_of.test | RUN | 0.598025 | 1.973296 | 2.022893 | 0.001888 | 0.002073 | 0.001986 |
| mp_none_of_q.test | RUN | 0.219444 | 0.839094 | 0.915247 | 0.001860 | 0.001942 | 0.001936 |
| mp_replace_at.test | RUN | 0.269948 | 1.677796 | 1.259025 | 0.001895 | 0.002542 | 0.002043 |
| mp_replace_at_c.test | RUN | 0.286734 | 1.706802 | 1.325266 | 0.001837 | 0.002010 | 0.002140 |
| mp_for_each.test | RUN | 0.276867 | 1.252971 | 1.508175 | 0.001849 | 0.002025 | 0.002102 |
| mp_insert.test | RUN | 0.408870 | 3.335957 | 1.852801 | 0.001923 | 0.001990 | 0.002308 |
| mp_insert_2.test | RUN | 0.038675 | 0.073705 | 0.142293 | 0.000562 | 0.000785 | 0.001867 |
| mp_erase.test | RUN | 0.456636 | 3.547456 | 2.227035 | 0.001908 | 0.002179 | 0.001994 |
| mp_erase_2.test | RUN | 0.036800 | 0.084094 | 0.148938 | 0.000820 | 0.000941 | 0.001768 |
| mp_with_index.test | RUN | 0.598449 | 7.274873 | 4.946626 | 0.002540 | 0.002600 | 0.002118 |
| mp_with_index_cx.test | RUN | 0.004097 | 0.022164 | 0.030710 | 0.000842 | 0.000898 | 0.002113 |
| mp_from_sequence.test | RUN | 0.278854 | 1.913604 | 1.248544 | 0.001657 | 0.001899 | 0.002072 |
| mp_from_sequence_2.test | RUN | 0.289422 | 2.270403 | 1.482916 | 0.001762 | 0.001964 | 0.002081 |
| mp_from_sequence_3.test | RUN | 0.250899 | 1.038591 | 0.995098 | 0.001990 | 0.002110 | 0.001624 |
| mp_min_element.test | RUN | 0.240156 | 0.915791 | 0.948407 | 0.001917 | 0.002332 | 0.001728 |
| mp_min_element_2.test | RUN | 0.036275 | 0.079782 | 0.123648 | 0.000721 | 0.000856 | 0.001775 |
| mp_min_element_q.test | RUN | 0.244216 | 0.915602 | 0.934136 | 0.001878 | 0.001890 | 0.001754 |
| mp_min_element_q_2.test | RUN | 0.037383 | 0.081344 | 0.134030 | 0.000749 | 0.000718 | 0.001505 |
| mp_max_element.test | RUN | 0.236857 | 0.921299 | 0.878474 | 0.002034 | 0.002173 | 0.001704 |
| mp_max_element_2.test | RUN | 0.036060 | 0.077873 | 0.132114 | 0.000794 | 0.000609 | 0.002094 |
| mp_max_element_q.test | RUN | 0.240513 | 0.918412 | 0.926962 | 0.001925 | 0.001990 | 0.001895 |
| mp_max_element_q_2.test | RUN | 0.037895 | 0.081096 | 0.130953 | 0.000726 | 0.000887 | 0.002116 |
| mp_nth_element.test | RUN | 0.263318 | 1.299678 | 1.184114 | 0.001795 | 0.002061 | 0.002155 |
| mp_nth_element_q.test | RUN | 0.269207 | 1.287402 | 1.169775 | 0.001821 | 0.002128 | 0.001731 |
| mp_back.test | RUN | 0.253982 | 1.044526 | 1.075918 | 0.002029 | 0.002014 | 0.002109 |
| mp_back_2.test | RUN | 0.039057 | 0.081654 | 0.148159 | 0.000681 | 0.000836 | 0.002048 |
| mp_pop_back.test | RUN | 0.668455 | 6.788514 | 2.187772 | 0.001883 | 0.002093 | 0.001967 |
| mp_pop_back_2.test | RUN | 0.039117 | 0.083405 | 0.125824 | 0.000763 | 0.000886 | 0.001857 |
| mp_flatten.test | RUN | 0.297359 | 2.425234 | 1.537878 | 0.001764 | 0.002124 | 0.001658 |
| mp_rotate_left.test | RUN | 0.308077 | 2.613595 | 1.522328 | 0.001956 | 0.002428 | 0.001890 |
| mp_rotate_right.test | RUN | 0.306525 | 2.645480 | 1.537418 | 0.001842 | 0.002128 | 0.002194 |
| mp_power_set.test | RUN | 0.297504 | 2.578399 | 1.501673 | 0.001851 | 0.002120 | 0.001670 |
| mp_partial_sum.test | RUN | 0.388269 | 3.341486 | 1.730643 | 0.001768 | 0.001762 | 0.001886 |
| mp_iterate.test | RUN | 0.267946 | 1.029346 | 1.242007 | 0.001934 | 0.002092 | 0.001841 |
| mp_pairwise_fold.test | RUN | 0.295146 | 2.307751 | 1.532046 | 0.001819 | 0.002236 | 0.002119 |
| mp_pairwise_fold_q.test | RUN | 0.234376 | 0.703780 | 0.960354 | 0.002127 | 0.001984 | 0.002140 |
| mp_sliding_fold.test | RUN | 0.555236 | 4.318181 | 2.793862 | 0.001778 | 0.002145 | 0.001896 |
| mp_sliding_fold_q.test | RUN | 0.546764 | 4.328836 | 2.744377 | 0.001790 | 0.002064 | 0.001986 |
| mp_intersperse.test | RUN | 0.272545 | 1.685132 | 1.349333 | 0.002409 | 0.002132 | 0.002178 |
| mp_split.test | RUN | 0.393514 | 3.197316 | 1.765477 | 0.001887 | 0.002032 | 0.002180 |
| mp_join.test | RUN | 0.322195 | 2.677897 | 1.603146 | 0.001772 | 0.002173 | 0.002184 |
| integral.test | RUN | 0.244241 | 1.210687 | 0.989285 | 0.001860 | 0.002154 | 0.002234 |
| mp_value.test | RUN | 0.015170 | 0.038084 | 0.067266 | 0.000757 | 0.000758 | 0.002245 |
| mp_identity.test | RUN | 0.224888 | 1.110071 | 0.968730 | 0.001807 | 0.001818 | 0.002128 |
| mp_inherit.test | RUN | 0.228872 | 0.841852 | 0.914414 | 0.002074 | 0.002090 | 0.002171 |
| mp_if.test | RUN | 0.215148 | 0.923111 | 0.901219 | 0.001904 | 0.001892 | 0.002108 |
| mp_if_sf.test | RUN | 0.218775 | 0.683165 | 0.836799 | 0.002002 | 0.001946 | 0.002048 |
| mp_eval_if.test | RUN | 0.229415 | 0.988438 | 0.899737 | 0.001794 | 0.001688 | 0.002126 |
| mp_eval_if_sf.test | RUN | 0.221575 | 0.724564 | 0.888906 | 0.002115 | 0.002062 | 0.002088 |
| mp_valid.test | RUN | 0.230857 | 0.723035 | 0.874593 | 0.001880 | 0.001930 | 0.002258 |
| mp_defer.test | RUN | 0.248357 | 1.214960 | 1.020678 | 0.001988 | 0.002226 | 0.002085 |
| mp_quote.test | RUN | 0.233605 | 1.179175 | 1.037964 | 0.001827 | 0.002110 | 0.002238 |
| mp_invoke_q.test | RUN | 0.252527 | 1.561664 | 1.150641 | 0.001894 | 0.002233 | 0.001718 |
| mp_invoke_q_sf.test | RUN | 0.222717 | 0.714122 | 0.870065 | 0.001742 | 0.002042 | 0.002044 |
| mp_quote_trait.test | RUN | 0.243913 | 1.297160 | 1.021715 | 0.001918 | 0.002242 | 0.002248 |
| mp_cond.test | RUN | 0.223216 | 0.788047 | 0.899805 | 0.001880 | 0.002076 | 0.002293 |
| mp_cond_sf.test | RUN | 0.222203 | 0.702289 | 0.877477 | 0.001923 | 0.002219 | 0.001816 |
| mp_not_fn.test | RUN | 0.220273 | 0.690525 | 0.872625 | 0.001772 | 0.002314 | 0.002126 |
| mp_eval_if_not.test | RUN | 0.232694 | 0.982215 | 0.950871 | 0.001839 | 0.002102 | 0.001919 |
| mp_eval_or.test | RUN | 0.215938 | 0.563755 | 0.897932 | 0.001838 | 0.002112 | 0.002334 |
| mp_compose.test | RUN | 0.249355 | 1.600784 | 1.127023 | 0.001701 | 0.001804 | 0.001549 |
| mp_valid_and_true.test | RUN | 0.223692 | 0.846653 | 0.927891 | 0.001844 | 0.002171 | 0.002018 |
| mp_compose_sf.test | COMPILE | 0.052513 | 0.101158 | 0.191608 | — | — | — |
| integer_sequence.test | RUN | 0.260397 | 1.560143 | 1.251361 | 0.001578 | 0.001782 | 0.002170 |
| tuple_for_each.test | RUN | 0.226255 | 0.606882 | 0.937552 | 0.001910 | 0.001898 | 0.001918 |
| tuple_for_each_cx.test | COMPILE | 0.043036 | 0.094430 | 0.160047 | — | — | — |
| tuple_apply.test | RUN | 0.228832 | 0.619944 | 0.886560 | 0.001766 | 0.002367 | 0.001991 |
| tuple_apply_cx.test | COMPILE | 0.061482 | 0.119021 | 0.232582 | — | — | — |
| construct_from_tuple.test | RUN | 0.233701 | 0.643931 | 0.938870 | 0.001901 | 0.001628 | 0.002223 |
| construct_from_tuple_cx.test | COMPILE | 0.063401 | 0.129881 | 0.195503 | — | — | — |
| tuple_transform.test | RUN | 0.371079 | 1.309140 | 1.842320 | 0.001783 | 0.001984 | 0.001785 |
| tuple_transform_2.test | RUN | 0.327867 | 0.964816 | 1.694198 | 0.002016 | 0.001913 | 0.001850 |
| tuple_transform_cx.test | COMPILE | 0.053896 | 0.118847 | 0.233489 | — | — | — |
| mp_set_contains.test | RUN | 0.228957 | 0.847365 | 0.933585 | 0.001866 | 0.001837 | 0.002236 |
| mp_set_push_back.test | RUN | 0.296338 | 2.540375 | 1.532507 | 0.002009 | 0.002164 | 0.002182 |
| mp_set_push_front.test | RUN | 0.380734 | 3.045359 | 1.731358 | 0.002008 | 0.002239 | 0.001762 |
| mp_is_set.test | RUN | 0.228876 | 0.849753 | 0.981843 | 0.001954 | 0.002126 | 0.002362 |
| mp_set_union.test | RUN | 0.289378 | 2.117999 | 1.452666 | 0.001880 | 0.002170 | 0.001826 |
| mp_set_union_sf.test | RUN | 0.215765 | 0.691992 | 0.887479 | 0.001854 | 0.002118 | 0.001726 |
| mp_set_difference.test | RUN | 0.265368 | 1.683129 | 1.243597 | 0.001900 | 0.002008 | 0.001841 |
| mp_set_difference_sf.test | RUN | 0.224309 | 0.682714 | 0.902984 | 0.001832 | 0.002220 | 0.001864 |
| mp_set_intersection.test | RUN | 0.260322 | 1.689290 | 1.246765 | 0.001765 | 0.002219 | 0.002267 |
| mp_set_intersection_sf.test | RUN | 0.219986 | 0.689169 | 0.927470 | 0.001731 | 0.002226 | 0.002229 |
| mp_all.test | RUN | 0.230920 | 0.822541 | 0.964317 | 0.001934 | 0.001897 | 0.001974 |
| mp_all_2.test | RUN | 0.611280 | 1.712420 | 2.093003 | 0.002040 | 0.002028 | 0.001876 |
| mp_and.test | RUN | 0.226192 | 0.844703 | 0.912479 | 0.001988 | 0.002191 | 0.002180 |
| mp_any.test | RUN | 0.223093 | 0.845213 | 0.934688 | 0.001732 | 0.002320 | 0.002199 |
| mp_any_2.test | RUN | 0.636534 | 1.678983 | 1.929284 | 0.001870 | 0.002358 | 0.001904 |
| mp_or.test | RUN | 0.226562 | 0.858156 | 0.865392 | 0.002514 | 0.001702 | 0.001847 |
| mp_same.test | RUN | 0.431075 | 1.390395 | 1.352593 | 0.001886 | 0.001844 | 0.001791 |
| mp_plus.test | RUN | 0.248665 | 0.994909 | 1.046383 | 0.001888 | 0.002011 | 0.001794 |
| mp_less.test | RUN | 0.231746 | 0.818677 | 0.927016 | 0.002013 | 0.002078 | 0.001879 |
| mp_min.test | RUN | 0.227479 | 0.772370 | 0.910790 | 0.001986 | 0.001859 | 0.001816 |
| mp_max.test | RUN | 0.229849 | 0.864340 | 0.908013 | 0.001951 | 0.001761 | 0.002036 |
| mp_similar.test | RUN | 0.226863 | 0.834449 | 0.938651 | 0.001848 | 0.001726 | 0.002078 |
| mp_map_find.test | RUN | 0.257580 | 1.532162 | 1.187030 | 0.002100 | 0.001842 | 0.002048 |
| mp_map_find_2.test | RUN | 0.247993 | 0.622227 | 0.976672 | 0.001908 | 0.001763 | 0.002232 |
| mp_map_find_3.test | RUN | 0.249061 | 0.605030 | 1.020941 | 0.001960 | 0.002163 | 0.002122 |
| mp_map_find_4.test | RUN | 0.231594 | 0.569318 | 0.966593 | 0.001759 | 0.001733 | 0.001836 |
| mp_map_contains.test | RUN | 0.236822 | 0.857587 | 0.922582 | 0.002083 | 0.002341 | 0.002016 |
| mp_map_insert.test | RUN | 0.302920 | 2.778277 | 1.594537 | 0.001924 | 0.002174 | 0.001893 |
| mp_map_replace.test | RUN | 0.388898 | 3.113876 | 1.830573 | 0.001676 | 0.002275 | 0.001952 |
| mp_map_erase.test | RUN | 0.293484 | 2.375978 | 1.539209 | 0.002094 | 0.002234 | 0.001919 |
| mp_map_update.test | RUN | 0.263253 | 1.576228 | 1.219956 | 0.001933 | 0.002042 | 0.001958 |
| mp_map_update_q.test | RUN | 0.264193 | 1.573858 | 1.213660 | 0.001956 | 0.001968 | 0.001935 |
| mp_map_keys.test | RUN | 0.268699 | 1.390939 | 1.208503 | 0.001901 | 0.002037 | 0.001872 |
| mp_is_map.test | RUN | 0.250431 | 0.873605 | 0.983375 | 0.001835 | 0.001916 | 0.001920 |
| mp_bind.test | RUN | 0.284227 | 2.477017 | 1.411938 | 0.002004 | 0.001561 | 0.002123 |
| mp_bind_q.test | RUN | 0.267834 | 1.534927 | 1.172445 | 0.001790 | 0.001998 | 0.001792 |
| mp_bind_front.test | RUN | 0.259401 | 1.704748 | 1.142609 | 0.002032 | 0.001799 | 0.001643 |
| mp_bind_back.test | RUN | 0.256322 | 1.715580 | 1.089617 | 0.002042 | 0.001913 | 0.001705 |
| mp_lambda.test | RUN | 0.567775 | 4.964407 | 2.791465 | 0.001783 | 0.001656 | 0.001934 |
| mpl.test | RUN | 0.506392 | 1.340548 | 2.261077 | 0.001750 | 0.001951 | 0.002027 |
| mpl_list.test | RUN | 0.293805 | 1.059405 | 1.125001 | 0.001838 | 0.002032 | 0.001784 |
| mpl_tuple.test | RUN | 0.301510 | 1.034010 | 1.125536 | 0.001888 | 0.001926 | 0.001653 |
| version.test | RUN | 0.149051 | 0.385373 | 0.592800 | 0.002024 | 0.001895 | 0.001838 |
| mp11_version.test | RUN | 0.183527 | 0.433258 | 0.696189 | 0.001765 | 0.002188 | 0.001706 |
| list_version.test | RUN | 0.153836 | 0.395486 | 0.568685 | 0.001918 | 0.002040 | 0.002039 |
| algorithm_version.test | RUN | 0.173009 | 0.412865 | 0.648132 | 0.001802 | 0.001938 | 0.001827 |
| integral_version.test | RUN | 0.150166 | 0.387138 | 0.559677 | 0.001990 | 0.002118 | 0.001790 |
| utility_version.test | RUN | 0.153514 | 0.380386 | 0.584518 | 0.001804 | 0.001892 | 0.001730 |
| function_version.test | RUN | 0.157287 | 0.383802 | 0.589342 | 0.001877 | 0.002018 | 0.002012 |
| map_version.test | RUN | 0.175538 | 0.424939 | 0.631853 | 0.001822 | 0.001827 | 0.001792 |
| set_version.test | RUN | 0.158564 | 0.396753 | 0.604604 | 0.001854 | 0.001960 | 0.001704 |
| bind_version.test | RUN | 0.169664 | 0.411914 | 0.661048 | 0.001890 | 0.001895 | 0.001720 |
| integer_sequence_version.test | RUN | 0.151161 | 0.378344 | 0.429490 | 0.001418 | 0.001916 | 0.001498 |
| tuple_version.test | RUN | 0.173053 | 0.417271 | 0.505661 | 0.001323 | 0.001580 | 0.001462 |
| mpl_version.test | RUN | 0.111126 | 0.318465 | 0.528457 | 0.001592 | 0.001465 | 0.001725 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 1.006×, 0.999× | 1.007× |
| clang | 1.090× | 1.099×, 1.081× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001140s | 0.001126s | 0.001159s | byte equality observed |
| bind_version.test | 10 | 0.001134s | 0.001149s | 0.001175s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001175s | 0.001158s | 0.001198s | byte equality observed |
| function_version.test | 10 | 0.001155s | 0.001150s | 0.001230s | byte equality observed |
| integer_sequence.test | 10 | 0.001149s | 0.001150s | 0.001253s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001162s | 0.001136s | 0.001247s | byte equality observed |
| integral.test | 10 | 0.001164s | 0.001136s | 0.001289s | byte equality observed |
| integral_version.test | 10 | 0.001163s | 0.001138s | 0.001294s | byte equality observed |
| list_version.test | 10 | 0.001169s | 0.001136s | 0.001272s | byte equality observed |
| map_version.test | 10 | 0.001163s | 0.001119s | 0.001219s | byte equality observed |
| mp11_version.test | 10 | 0.001196s | 0.001128s | 0.001251s | byte equality observed |
| mp_all.test | 10 | 0.001164s | 0.001156s | 0.001293s | byte equality observed |
| mp_all_2.test | 10 | 0.001163s | 0.001131s | 0.001248s | byte equality observed |
| mp_all_of.test | 10 | 0.001160s | 0.001137s | 0.001301s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001158s | 0.001128s | 0.001237s | byte equality observed |
| mp_and.test | 10 | 0.001156s | 0.001134s | 0.001212s | byte equality observed |
| mp_any.test | 10 | 0.001159s | 0.001131s | 0.001240s | byte equality observed |
| mp_any_2.test | 10 | 0.001175s | 0.001126s | 0.001227s | byte equality observed |
| mp_any_of.test | 10 | 0.001159s | 0.001136s | 0.001248s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001220s | 0.001128s | 0.001276s | byte equality observed |
| mp_append.test | 10 | 0.001205s | 0.001134s | 0.001260s | byte equality observed |
| mp_append_2.test | 10 | 0.001315s | 0.001142s | 0.001248s | byte equality observed |
| mp_append_3.test | 10 | 0.000582s | 0.000515s | 0.001259s | byte equality observed |
| mp_append_sf.test | 10 | 0.001244s | 0.001130s | 0.001231s | byte equality observed |
| mp_apply_q.test | 10 | 0.001263s | 0.001142s | 0.001295s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001225s | 0.001171s | 0.001228s | byte equality observed |
| mp_assign.test | 10 | 0.001245s | 0.001266s | 0.001219s | byte equality observed |
| mp_assign_2.test | 10 | 0.000584s | 0.000567s | 0.001226s | byte equality observed |
| mp_at.test | 10 | 0.001266s | 0.001252s | 0.001233s | byte equality observed |
| mp_at_2.test | 10 | 0.000573s | 0.000576s | 0.001221s | byte equality observed |
| mp_at_sf.test | 10 | 0.001246s | 0.001261s | 0.001283s | byte equality observed |
| mp_back.test | 10 | 0.001269s | 0.001316s | 0.001254s | byte equality observed |
| mp_back_2.test | 10 | 0.000566s | 0.000579s | 0.001241s | byte equality observed |
| mp_bind.test | 10 | 0.001214s | 0.001262s | 0.001230s | byte equality observed |
| mp_bind_back.test | 10 | 0.001200s | 0.001260s | 0.001276s | byte equality observed |
| mp_bind_front.test | 10 | 0.001223s | 0.001235s | 0.001283s | byte equality observed |
| mp_bind_q.test | 10 | 0.001264s | 0.001243s | 0.001302s | byte equality observed |
| mp_clear.test | 10 | 0.001254s | 0.001281s | 0.001274s | byte equality observed |
| mp_clear_2.test | 10 | 0.000556s | 0.000569s | 0.001244s | byte equality observed |
| mp_compose.test | 10 | 0.001270s | 0.001255s | 0.001228s | byte equality observed |
| mp_cond.test | 10 | 0.001214s | 0.001236s | 0.001270s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001223s | 0.001271s | 0.001264s | byte equality observed |
| mp_contains.test | 10 | 0.001264s | 0.001283s | 0.001225s | byte equality observed |
| mp_copy_if.test | 10 | 0.001252s | 0.001238s | 0.001299s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001286s | 0.001256s | 0.001223s | byte equality observed |
| mp_count.test | 10 | 0.001247s | 0.001319s | 0.001230s | byte equality observed |
| mp_count_if.test | 10 | 0.001249s | 0.001265s | 0.001255s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001262s | 0.001276s | 0.001234s | byte equality observed |
| mp_defer.test | 10 | 0.001252s | 0.001293s | 0.001211s | byte equality observed |
| mp_drop.test | 10 | 0.001273s | 0.001298s | 0.001280s | byte equality observed |
| mp_drop_2.test | 10 | 0.000570s | 0.000571s | 0.001240s | byte equality observed |
| mp_empty.test | 10 | 0.001264s | 0.001268s | 0.001215s | byte equality observed |
| mp_empty_2.test | 10 | 0.000562s | 0.000585s | 0.001221s | byte equality observed |
| mp_erase.test | 10 | 0.001302s | 0.001261s | 0.001216s | byte equality observed |
| mp_erase_2.test | 10 | 0.000574s | 0.000556s | 0.001230s | byte equality observed |
| mp_eval_if.test | 10 | 0.001260s | 0.001239s | 0.001235s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001212s | 0.001244s | 0.001297s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001251s | 0.001294s | 0.001276s | byte equality observed |
| mp_eval_or.test | 10 | 0.001222s | 0.001270s | 0.001262s | byte equality observed |
| mp_fill.test | 10 | 0.001248s | 0.001268s | 0.001221s | byte equality observed |
| mp_fill_2.test | 10 | 0.000572s | 0.000580s | 0.001199s | byte equality observed |
| mp_filter.test | 10 | 0.001222s | 0.001288s | 0.001229s | byte equality observed |
| mp_find.test | 10 | 0.001281s | 0.001340s | 0.001220s | byte equality observed |
| mp_find_if.test | 10 | 0.001214s | 0.001259s | 0.001228s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001235s | 0.001250s | 0.001242s | byte equality observed |
| mp_flatten.test | 10 | 0.001231s | 0.001263s | 0.001233s | byte equality observed |
| mp_fold.test | 10 | 0.001278s | 0.001283s | 0.001248s | byte equality observed |
| mp_fold_q.test | 10 | 0.001248s | 0.001290s | 0.001249s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001245s | 0.001299s | 0.001252s | byte equality observed |
| mp_for_each.test | 10 | 0.001261s | 0.001255s | 0.001298s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001259s | 0.001243s | 0.001232s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001264s | 0.001293s | 0.001231s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001262s | 0.001304s | 0.001312s | byte equality observed |
| mp_front.test | 10 | 0.001255s | 0.001275s | 0.001249s | byte equality observed |
| mp_front_2.test | 10 | 0.000587s | 0.000576s | 0.001224s | byte equality observed |
| mp_identity.test | 10 | 0.001216s | 0.001213s | 0.001215s | byte equality observed |
| mp_if.test | 10 | 0.001267s | 0.001245s | 0.001243s | byte equality observed |
| mp_if_sf.test | 10 | 0.001256s | 0.001249s | 0.001322s | byte equality observed |
| mp_inherit.test | 10 | 0.001239s | 0.001224s | 0.001262s | byte equality observed |
| mp_insert.test | 10 | 0.001291s | 0.001286s | 0.001227s | byte equality observed |
| mp_insert_2.test | 10 | 0.000571s | 0.000553s | 0.001253s | byte equality observed |
| mp_intersperse.test | 10 | 0.001275s | 0.001273s | 0.001257s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001270s | 0.001287s | 0.001254s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001243s | 0.001266s | 0.001293s | byte equality observed |
| mp_iota.test | 10 | 0.001268s | 0.001332s | 0.001296s | byte equality observed |
| mp_iota_2.test | 10 | 0.001240s | 0.001294s | 0.001266s | byte equality observed |
| mp_is_list.test | 10 | 0.001288s | 0.001291s | 0.001256s | byte equality observed |
| mp_is_map.test | 10 | 0.001272s | 0.001238s | 0.001221s | byte equality observed |
| mp_is_set.test | 10 | 0.001241s | 0.001234s | 0.001195s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001267s | 0.001253s | 0.001222s | byte equality observed |
| mp_iterate.test | 10 | 0.001259s | 0.001241s | 0.001277s | byte equality observed |
| mp_join.test | 10 | 0.001212s | 0.001231s | 0.001220s | byte equality observed |
| mp_lambda.test | 10 | 0.001262s | 0.001262s | 0.001225s | byte equality observed |
| mp_less.test | 10 | 0.001244s | 0.001250s | 0.001274s | byte equality observed |
| mp_list_c.test | 10 | 0.001260s | 0.001300s | 0.001312s | byte equality observed |
| mp_list_v.test | 10 | 0.000569s | 0.000567s | 0.001249s | byte equality observed |
| mp_map_contains.test | 10 | 0.001231s | 0.001248s | 0.001226s | byte equality observed |
| mp_map_erase.test | 10 | 0.001225s | 0.001266s | 0.001239s | byte equality observed |
| mp_map_find.test | 10 | 0.001262s | 0.001291s | 0.001220s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001253s | 0.001238s | 0.001281s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001231s | 0.001259s | 0.001244s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001231s | 0.001289s | 0.001250s | byte equality observed |
| mp_map_insert.test | 10 | 0.001246s | 0.001249s | 0.001350s | byte equality observed |
| mp_map_keys.test | 10 | 0.001244s | 0.001216s | 0.001290s | byte equality observed |
| mp_map_replace.test | 10 | 0.001288s | 0.001261s | 0.001254s | byte equality observed |
| mp_map_update.test | 10 | 0.001270s | 0.001272s | 0.001271s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001284s | 0.001257s | 0.001235s | byte equality observed |
| mp_max.test | 10 | 0.001267s | 0.001269s | 0.001230s | byte equality observed |
| mp_max_element.test | 10 | 0.001280s | 0.001268s | 0.001220s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000565s | 0.000583s | 0.001256s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001272s | 0.001282s | 0.001265s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000569s | 0.000570s | 0.001197s | byte equality observed |
| mp_min.test | 10 | 0.001269s | 0.001231s | 0.001266s | byte equality observed |
| mp_min_element.test | 10 | 0.001284s | 0.001324s | 0.001281s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000567s | 0.000586s | 0.001218s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001264s | 0.001286s | 0.001230s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000591s | 0.000588s | 0.001226s | byte equality observed |
| mp_none_of.test | 10 | 0.001249s | 0.001319s | 0.001228s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001266s | 0.001254s | 0.001273s | byte equality observed |
| mp_not_fn.test | 10 | 0.001254s | 0.001303s | 0.001285s | byte equality observed |
| mp_nth_element.test | 10 | 0.001294s | 0.001284s | 0.001266s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001296s | 0.001222s | 0.001212s | byte equality observed |
| mp_or.test | 10 | 0.001251s | 0.001274s | 0.001256s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001251s | 0.001308s | 0.001216s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001264s | 0.001268s | 0.001246s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001301s | 0.001260s | 0.001250s | byte equality observed |
| mp_partition.test | 10 | 0.001261s | 0.001277s | 0.001279s | byte equality observed |
| mp_partition_q.test | 10 | 0.001267s | 0.001291s | 0.001286s | byte equality observed |
| mp_plus.test | 10 | 0.001267s | 0.001240s | 0.001253s | byte equality observed |
| mp_pop_back.test | 10 | 0.001304s | 0.001290s | 0.001259s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000569s | 0.000580s | 0.001195s | byte equality observed |
| mp_pop_front.test | 10 | 0.001270s | 0.001278s | 0.001253s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000565s | 0.000571s | 0.001191s | byte equality observed |
| mp_power_set.test | 10 | 0.001249s | 0.001248s | 0.001243s | byte equality observed |
| mp_product.test | 5 | 0.001254s | 0.001263s | 0.001204s | byte equality observed |
| mp_push_back.test | 10 | 0.001275s | 0.001278s | 0.001230s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000570s | 0.000570s | 0.001266s | byte equality observed |
| mp_push_front.test | 10 | 0.001275s | 0.001283s | 0.001238s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000579s | 0.000578s | 0.001236s | byte equality observed |
| mp_quote.test | 10 | 0.001239s | 0.001276s | 0.001261s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001278s | 0.001242s | 0.001255s | byte equality observed |
| mp_remove.test | 10 | 0.001263s | 0.001241s | 0.001283s | byte equality observed |
| mp_remove_if.test | 10 | 0.001254s | 0.001289s | 0.001243s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001239s | 0.001226s | 0.001266s | byte equality observed |
| mp_rename.test | 10 | 0.001248s | 0.001247s | 0.001300s | byte equality observed |
| mp_rename_2.test | 10 | 0.000592s | 0.000579s | 0.001223s | byte equality observed |
| mp_rename_v.test | 10 | 0.000594s | 0.000591s | 0.001265s | byte equality observed |
| mp_repeat.test | 10 | 0.001303s | 0.001294s | 0.001228s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000567s | 0.000590s | 0.001266s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001260s | 0.001256s | 0.001275s | byte equality observed |
| mp_replace.test | 10 | 0.001255s | 0.001265s | 0.001269s | byte equality observed |
| mp_replace_at.test | 10 | 0.001268s | 0.001245s | 0.001252s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001284s | 0.001263s | 0.001271s | byte equality observed |
| mp_replace_front.test | 10 | 0.001275s | 0.001259s | 0.001210s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000584s | 0.000569s | 0.001304s | byte equality observed |
| mp_replace_if.test | 10 | 0.001234s | 0.001225s | 0.001244s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001265s | 0.001227s | 0.001325s | byte equality observed |
| mp_replace_second.test | 10 | 0.001289s | 0.001262s | 0.001210s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000581s | 0.000570s | 0.001232s | byte equality observed |
| mp_replace_third.test | 10 | 0.001272s | 0.001271s | 0.001240s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000576s | 0.000571s | 0.001191s | byte equality observed |
| mp_reverse.test | 10 | 0.001302s | 0.001260s | 0.001249s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001260s | 0.001244s | 0.001284s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001250s | 0.001233s | 0.001258s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001278s | 0.001327s | 0.001281s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001272s | 0.001313s | 0.001280s | byte equality observed |
| mp_same.test | 10 | 0.001240s | 0.001248s | 0.001247s | byte equality observed |
| mp_second.test | 10 | 0.001261s | 0.001274s | 0.001231s | byte equality observed |
| mp_second_2.test | 10 | 0.000575s | 0.000585s | 0.001186s | byte equality observed |
| mp_set_contains.test | 10 | 0.001212s | 0.001225s | 0.001240s | byte equality observed |
| mp_set_difference.test | 10 | 0.001258s | 0.001279s | 0.001225s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001260s | 0.001253s | 0.001216s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001256s | 0.001237s | 0.001221s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001223s | 0.001271s | 0.001230s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001237s | 0.001274s | 0.001243s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001235s | 0.001258s | 0.001249s | byte equality observed |
| mp_set_union.test | 10 | 0.001236s | 0.001300s | 0.001217s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001259s | 0.001323s | 0.001298s | byte equality observed |
| mp_similar.test | 10 | 0.001225s | 0.001302s | 0.001275s | byte equality observed |
| mp_size.test | 10 | 0.001265s | 0.001238s | 0.001212s | byte equality observed |
| mp_size_2.test | 10 | 0.000575s | 0.000585s | 0.001263s | byte equality observed |
| mp_slice.test | 10 | 0.001285s | 0.001276s | 0.001226s | byte equality observed |
| mp_slice_2.test | 10 | 0.000562s | 0.000585s | 0.001211s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001290s | 0.001275s | 0.001276s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001283s | 0.001310s | 0.001230s | byte equality observed |
| mp_sort.test | 10 | 0.001239s | 0.001268s | 0.001199s | byte equality observed |
| mp_sort_q.test | 10 | 0.001246s | 0.001217s | 0.001296s | byte equality observed |
| mp_split.test | 10 | 0.001256s | 0.001237s | 0.001266s | byte equality observed |
| mp_starts_with.test | 10 | 0.001253s | 0.001218s | 0.001281s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001256s | 0.001280s | 0.001238s | byte equality observed |
| mp_take.test | 10 | 0.001295s | 0.001275s | 0.001222s | byte equality observed |
| mp_take_2.test | 10 | 0.000575s | 0.000587s | 0.001257s | byte equality observed |
| mp_third.test | 10 | 0.001271s | 0.001269s | 0.001220s | byte equality observed |
| mp_third_2.test | 10 | 0.000574s | 0.000568s | 0.001298s | byte equality observed |
| mp_transform.test | 10 | 0.001228s | 0.001262s | 0.001211s | byte equality observed |
| mp_transform_2.test | 10 | 0.001235s | 0.001201s | 0.001220s | byte equality observed |
| mp_transform_front.test | 10 | 0.001260s | 0.001321s | 0.001212s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000562s | 0.000568s | 0.001259s | byte equality observed |
| mp_transform_if.test | 10 | 0.001232s | 0.001280s | 0.001249s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001267s | 0.001262s | 0.001260s | byte equality observed |
| mp_transform_q.test | 10 | 0.001295s | 0.001266s | 0.001244s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001259s | 0.001251s | 0.001231s | byte equality observed |
| mp_transform_second.test | 10 | 0.001266s | 0.001299s | 0.001270s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000580s | 0.000561s | 0.001182s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001270s | 0.001281s | 0.001224s | byte equality observed |
| mp_transform_third.test | 10 | 0.001280s | 0.001300s | 0.001242s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000577s | 0.000576s | 0.001231s | byte equality observed |
| mp_unique.test | 10 | 0.001241s | 0.001269s | 0.001230s | byte equality observed |
| mp_unique_if.test | 10 | 0.001243s | 0.001278s | 0.001270s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001264s | 0.001302s | 0.001217s | byte equality observed |
| mp_valid.test | 10 | 0.001265s | 0.001270s | 0.001281s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001268s | 0.001268s | 0.001296s | byte equality observed |
| mp_value.test | 10 | 0.000574s | 0.000579s | 0.001198s | byte equality observed |
| mp_with_index.test | 10 | 0.001645s | 0.001664s | 0.001540s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000565s | 0.000564s | 0.001232s | byte equality observed |
| mpl.test | 10 | 0.001269s | 0.001220s | 0.001218s | byte equality observed |
| mpl_list.test | 10 | 0.001268s | 0.001314s | 0.001215s | byte equality observed |
| mpl_tuple.test | 10 | 0.001265s | 0.001226s | 0.001275s | byte equality observed |
| mpl_version.test | 10 | 0.001275s | 0.001237s | 0.001230s | byte equality observed |
| set_version.test | 10 | 0.001265s | 0.001237s | 0.001274s | byte equality observed |
| tuple_apply.test | 10 | 0.001256s | 0.001231s | 0.001296s | byte equality observed |
| tuple_for_each.test | 10 | 0.001251s | 0.001223s | 0.001256s | byte equality observed |
| tuple_transform.test | 10 | 0.001293s | 0.001264s | 0.001277s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001259s | 0.001220s | 0.001251s | byte equality observed |
| tuple_version.test | 10 | 0.001265s | 0.001270s | 0.001205s | byte equality observed |
| utility_version.test | 10 | 0.001295s | 0.001252s | 0.001227s | byte equality observed |
| version.test | 10 | 0.001275s | 0.001254s | 0.001256s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
