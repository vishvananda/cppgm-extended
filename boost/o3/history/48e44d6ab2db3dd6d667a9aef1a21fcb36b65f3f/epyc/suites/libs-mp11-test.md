# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.055548 | 0.097549 | 0.174979 | — | — | — |
| complex_h.test | COMPILE | 0.230219 | 0.441103 | 0.792830 | — | — | — |
| mp_size.test | RUN | 0.250848 | 1.331864 | 0.972374 | 0.001890 | 0.002001 | 0.001763 |
| mp_size_2.test | RUN | 0.023346 | 0.047611 | 0.075798 | 0.000940 | 0.001000 | 0.001950 |
| mp_empty.test | RUN | 0.256225 | 0.862275 | 0.835770 | 0.001866 | 0.002069 | 0.001808 |
| mp_empty_2.test | RUN | 0.021972 | 0.052869 | 0.084187 | 0.000783 | 0.000936 | 0.002052 |
| mp_assign.test | RUN | 0.270390 | 1.727645 | 1.106780 | 0.001880 | 0.002328 | 0.002120 |
| mp_assign_2.test | RUN | 0.024784 | 0.050901 | 0.078529 | 0.000833 | 0.000963 | 0.001936 |
| mp_clear.test | RUN | 0.253778 | 0.772111 | 0.884529 | 0.001837 | 0.002014 | 0.002194 |
| mp_clear_2.test | RUN | 0.025668 | 0.046885 | 0.084038 | 0.000877 | 0.000756 | 0.001924 |
| mp_front.test | RUN | 0.264709 | 1.203264 | 0.975643 | 0.001908 | 0.002274 | 0.001817 |
| mp_front_2.test | RUN | 0.022781 | 0.048925 | 0.078771 | 0.000926 | 0.000807 | 0.002350 |
| mp_pop_front.test | RUN | 0.264645 | 1.254290 | 1.024020 | 0.001827 | 0.002178 | 0.001814 |
| mp_pop_front_2.test | RUN | 0.022831 | 0.049183 | 0.088054 | 0.000832 | 0.000965 | 0.001868 |
| mp_second.test | RUN | 0.256832 | 0.790361 | 0.945487 | 0.001830 | 0.001869 | 0.001966 |
| mp_second_2.test | RUN | 0.023359 | 0.051544 | 0.072414 | 0.000748 | 0.000947 | 0.002240 |
| mp_third.test | RUN | 0.248811 | 0.760273 | 0.807453 | 0.001954 | 0.002238 | 0.002030 |
| mp_third_2.test | RUN | 0.022942 | 0.049781 | 0.084160 | 0.000833 | 0.000738 | 0.002171 |
| mp_push_front.test | RUN | 0.317852 | 2.625327 | 1.510096 | 0.001941 | 0.002204 | 0.001626 |
| mp_push_front_2.test | RUN | 0.023684 | 0.046932 | 0.075426 | 0.000802 | 0.000879 | 0.001772 |
| mp_push_back.test | RUN | 0.314352 | 2.607966 | 1.557552 | 0.001896 | 0.001972 | 0.002180 |
| mp_push_back_2.test | RUN | 0.023358 | 0.052431 | 0.084108 | 0.000883 | 0.000809 | 0.002156 |
| mp_rename.test | RUN | 0.292241 | 2.379976 | 1.316549 | 0.002176 | 0.002069 | 0.002058 |
| mp_rename_2.test | RUN | 0.024604 | 0.053256 | 0.081488 | 0.000708 | 0.000950 | 0.002098 |
| mp_append.test | RUN | 0.317375 | 2.715262 | 1.446463 | 0.001756 | 0.002340 | 0.002206 |
| mp_append_2.test | RUN | 0.599011 | 6.060080 | 1.848318 | 0.001838 | 0.001684 | 0.001920 |
| mp_append_3.test | RUN | 0.025196 | 0.051680 | 0.084326 | 0.000799 | 0.000767 | 0.002018 |
| mp_append_sf.test | RUN | 0.242054 | 0.724958 | 0.829628 | 0.001828 | 0.001951 | 0.001804 |
| mp_replace_front.test | RUN | 0.288407 | 1.943095 | 1.264268 | 0.001884 | 0.002136 | 0.001627 |
| mp_replace_front_2.test | RUN | 0.023642 | 0.052191 | 0.078877 | 0.000811 | 0.000794 | 0.001686 |
| mp_replace_second.test | RUN | 0.278799 | 1.269134 | 1.141217 | 0.001945 | 0.002082 | 0.002172 |
| mp_replace_second_2.test | RUN | 0.023462 | 0.051162 | 0.079695 | 0.000687 | 0.000947 | 0.001952 |
| mp_replace_third.test | RUN | 0.273411 | 1.046237 | 1.031440 | 0.001917 | 0.001808 | 0.002074 |
| mp_replace_third_2.test | RUN | 0.023754 | 0.050431 | 0.075330 | 0.000860 | 0.000805 | 0.002222 |
| mp_apply_q.test | RUN | 0.298671 | 2.383783 | 1.297684 | 0.001811 | 0.002124 | 0.001720 |
| mp_apply_q_sf.test | RUN | 0.249609 | 0.711888 | 0.901691 | 0.001879 | 0.001981 | 0.001797 |
| mp_is_list.test | RUN | 0.247580 | 0.840303 | 0.902656 | 0.001763 | 0.002026 | 0.001822 |
| mp_list_c.test | RUN | 0.262650 | 1.010482 | 1.000645 | 0.001932 | 0.002250 | 0.001906 |
| mp_transform_front.test | RUN | 0.280183 | 1.967445 | 1.114354 | 0.001846 | 0.002050 | 0.002218 |
| mp_transform_front_2.test | RUN | 0.023945 | 0.051644 | 0.081817 | 0.000776 | 0.000877 | 0.001754 |
| mp_transform_second.test | RUN | 0.271026 | 1.655977 | 1.086788 | 0.001906 | 0.001789 | 0.002002 |
| mp_transform_second_2.test | RUN | 0.025336 | 0.050656 | 0.083054 | 0.000780 | 0.000989 | 0.002056 |
| mp_transform_third.test | RUN | 0.262510 | 1.280806 | 0.997719 | 0.001890 | 0.002428 | 0.002088 |
| mp_transform_third_2.test | RUN | 0.024522 | 0.050875 | 0.077960 | 0.000770 | 0.000816 | 0.001818 |
| mp_list_v.test | RUN | 0.023966 | 0.051822 | 0.072698 | 0.000740 | 0.000787 | 0.001910 |
| mp_rename_v.test | RUN | 0.023341 | 0.052402 | 0.066449 | 0.000772 | 0.000978 | 0.002274 |
| mp_is_value_list.test | RUN | 0.244537 | 0.530671 | 0.829772 | 0.002038 | 0.001731 | 0.002032 |
| mp_transform.test | RUN | 0.484205 | 4.084879 | 2.396287 | 0.001994 | 0.001862 | 0.001947 |
| mp_transform_2.test | RUN | 0.236673 | 0.524883 | 0.769756 | 0.001945 | 0.002240 | 0.002246 |
| mp_transform_q.test | RUN | 0.471255 | 3.832806 | 2.252451 | 0.001830 | 0.002293 | 0.002090 |
| mp_transform_q_2.test | RUN | 0.539524 | 1.248780 | 2.960410 | 0.001914 | 0.002198 | 0.002238 |
| mp_transform_sf.test | RUN | 0.267352 | 0.732609 | 0.937236 | 0.002006 | 0.002124 | 0.002186 |
| mp_transform_if.test | RUN | 0.304184 | 1.946863 | 1.309377 | 0.001928 | 0.002211 | 0.001893 |
| mp_transform_if_q.test | RUN | 0.304916 | 1.954463 | 1.332444 | 0.001778 | 0.002343 | 0.001817 |
| mp_filter.test | RUN | 0.284685 | 1.165661 | 1.040960 | 0.002008 | 0.002006 | 0.001788 |
| mp_fill.test | RUN | 0.270948 | 0.905652 | 0.976757 | 0.001934 | 0.002000 | 0.002173 |
| mp_fill_2.test | RUN | 0.043754 | 0.082832 | 0.128686 | 0.000739 | 0.000935 | 0.002150 |
| mp_count.test | RUN | 0.475159 | 2.122951 | 1.478526 | 0.002010 | 0.002204 | 0.002108 |
| mp_count_if.test | RUN | 0.470816 | 2.146963 | 1.675688 | 0.001968 | 0.001830 | 0.002052 |
| mp_count_if_q.test | RUN | 0.272701 | 1.408378 | 0.930798 | 0.001549 | 0.001906 | 0.001723 |
| mp_contains.test | RUN | 0.263855 | 0.850427 | 0.883304 | 0.002089 | 0.002058 | 0.002162 |
| mp_starts_with.test | RUN | 0.263036 | 0.876298 | 0.931008 | 0.001589 | 0.002202 | 0.002094 |
| mp_starts_with_sf.test | RUN | 0.255180 | 0.737107 | 0.828092 | 0.001875 | 0.002107 | 0.001912 |
| mp_repeat.test | RUN | 0.386404 | 3.420862 | 1.648840 | 0.002182 | 0.001934 | 0.002024 |
| mp_repeat_2.test | RUN | 0.043997 | 0.077797 | 0.144548 | 0.000917 | 0.001012 | 0.002089 |
| mp_repeat_3.test | RUN | 0.304099 | 0.477196 | 0.827767 | 0.001730 | 0.002242 | 0.001947 |
| mp_product.test | RUN | 0.282845 | 2.099532 | 1.219986 | 0.001786 | 0.002210 | 0.001996 |
| mp_drop.test | RUN | 0.309824 | 2.621308 | 1.343198 | 0.001790 | 0.002284 | 0.002168 |
| mp_drop_2.test | RUN | 0.043498 | 0.082256 | 0.142336 | 0.000802 | 0.001050 | 0.001902 |
| mp_iota.test | RUN | 0.298597 | 2.217485 | 1.209966 | 0.001689 | 0.002060 | 0.002174 |
| mp_iota_2.test | RUN | 0.313873 | 2.651889 | 1.402745 | 0.001903 | 0.002314 | 0.002105 |
| mp_at.test | RUN | 0.266164 | 1.165886 | 1.072016 | 0.001896 | 0.002274 | 0.001786 |
| mp_at_2.test | RUN | 0.041017 | 0.072644 | 0.147592 | 0.000927 | 0.000908 | 0.001980 |
| mp_at_sf.test | RUN | 0.256724 | 0.741437 | 0.883689 | 0.002046 | 0.001972 | 0.001802 |
| mp_take.test | RUN | 0.394986 | 3.443378 | 1.721656 | 0.001943 | 0.001908 | 0.002090 |
| mp_take_2.test | RUN | 0.041120 | 0.079145 | 0.136641 | 0.000834 | 0.000879 | 0.001764 |
| mp_slice.test | RUN | 0.438464 | 3.462605 | 2.106895 | 0.001897 | 0.002151 | 0.002035 |
| mp_slice_2.test | RUN | 0.042141 | 0.081565 | 0.135251 | 0.000903 | 0.000942 | 0.002076 |
| mp_replace.test | RUN | 0.310724 | 1.788274 | 1.327467 | 0.001808 | 0.001992 | 0.002236 |
| mp_replace_if.test | RUN | 0.300587 | 1.575459 | 1.223389 | 0.002026 | 0.002055 | 0.001487 |
| mp_replace_if_q.test | RUN | 0.291213 | 1.583173 | 1.291235 | 0.001987 | 0.002177 | 0.002010 |
| mp_copy_if.test | RUN | 0.307061 | 2.218532 | 1.222571 | 0.001988 | 0.002090 | 0.002040 |
| mp_copy_if_q.test | RUN | 0.300637 | 2.243370 | 1.231556 | 0.002010 | 0.002270 | 0.001938 |
| mp_remove.test | RUN | 0.293279 | 1.469492 | 1.252853 | 0.002182 | 0.002394 | 0.002140 |
| mp_remove_if.test | RUN | 0.307963 | 2.339719 | 1.290496 | 0.001804 | 0.002153 | 0.002148 |
| mp_remove_if_q.test | RUN | 0.310778 | 2.289964 | 1.304789 | 0.002012 | 0.001785 | 0.002264 |
| mp_partition.test | RUN | 0.313177 | 2.160407 | 1.359650 | 0.002147 | 0.002001 | 0.002012 |
| mp_partition_q.test | RUN | 0.305710 | 2.147273 | 1.375107 | 0.002031 | 0.002214 | 0.002233 |
| mp_sort.test | RUN | 0.289723 | 1.582992 | 1.208815 | 0.001892 | 0.002426 | 0.002084 |
| mp_sort_q.test | RUN | 0.288446 | 1.567638 | 1.225409 | 0.001935 | 0.002055 | 0.001932 |
| mp_find.test | RUN | 0.289576 | 1.693790 | 1.069842 | 0.001659 | 0.002020 | 0.001726 |
| mp_find_if.test | RUN | 0.275451 | 1.558352 | 1.028003 | 0.001805 | 0.001866 | 0.002094 |
| mp_find_if_q.test | RUN | 0.307636 | 1.764987 | 1.232178 | 0.001884 | 0.002174 | 0.002033 |
| mp_reverse.test | RUN | 0.533261 | 4.530296 | 2.231734 | 0.002085 | 0.002074 | 0.001603 |
| mp_fold.test | RUN | 0.868373 | 3.566243 | 5.538909 | 0.002066 | 0.002028 | 0.002002 |
| mp_fold_q.test | RUN | 0.281716 | 1.548450 | 1.174124 | 0.001942 | 0.002328 | 0.002040 |
| mp_fold_q_sf.test | RUN | 0.256758 | 0.740213 | 0.941899 | 0.002005 | 0.002122 | 0.001761 |
| mp_reverse_fold.test | RUN | 0.438592 | 3.381826 | 1.635581 | 0.001778 | 0.002239 | 0.001952 |
| mp_reverse_fold_q.test | RUN | 0.295159 | 1.885835 | 1.228681 | 0.001874 | 0.002212 | 0.001920 |
| mp_unique.test | RUN | 0.284584 | 1.704607 | 1.205171 | 0.001862 | 0.002317 | 0.001702 |
| mp_unique_if.test | RUN | 0.468646 | 3.496044 | 1.817458 | 0.001969 | 0.002246 | 0.001810 |
| mp_unique_if_q.test | RUN | 0.407806 | 3.507282 | 1.480592 | 0.002061 | 0.002042 | 0.002025 |
| mp_all_of.test | RUN | 0.625477 | 1.940568 | 2.048621 | 0.001968 | 0.002154 | 0.002166 |
| mp_all_of_q.test | RUN | 0.257098 | 0.801402 | 0.910309 | 0.001828 | 0.002045 | 0.002146 |
| mp_any_of.test | RUN | 0.624581 | 1.966714 | 2.040586 | 0.001936 | 0.002083 | 0.001914 |
| mp_any_of_q.test | RUN | 0.255548 | 0.799091 | 0.945778 | 0.001905 | 0.002047 | 0.001879 |
| mp_none_of.test | RUN | 0.640929 | 1.973221 | 2.054942 | 0.001667 | 0.002110 | 0.001843 |
| mp_none_of_q.test | RUN | 0.245277 | 0.842374 | 0.890866 | 0.001763 | 0.002260 | 0.001936 |
| mp_replace_at.test | RUN | 0.296081 | 1.635320 | 1.257263 | 0.001751 | 0.002186 | 0.002042 |
| mp_replace_at_c.test | RUN | 0.304239 | 1.700074 | 1.311467 | 0.001839 | 0.002004 | 0.002234 |
| mp_for_each.test | RUN | 0.297259 | 1.267918 | 1.561289 | 0.001879 | 0.001884 | 0.002006 |
| mp_insert.test | RUN | 0.425135 | 3.404322 | 1.918388 | 0.001934 | 0.002238 | 0.002239 |
| mp_insert_2.test | RUN | 0.042968 | 0.079312 | 0.146118 | 0.000816 | 0.000853 | 0.002216 |
| mp_erase.test | RUN | 0.458318 | 3.627928 | 2.146116 | 0.001963 | 0.002182 | 0.002142 |
| mp_erase_2.test | RUN | 0.043736 | 0.074947 | 0.148411 | 0.000887 | 0.000917 | 0.002144 |
| mp_with_index.test | RUN | 0.642208 | 7.499364 | 4.882954 | 0.002360 | 0.002642 | 0.002304 |
| mp_with_index_cx.test | RUN | 0.004122 | 0.023076 | 0.032709 | 0.000764 | 0.000971 | 0.002080 |
| mp_from_sequence.test | RUN | 0.304008 | 1.960209 | 1.238418 | 0.002120 | 0.002134 | 0.002270 |
| mp_from_sequence_2.test | RUN | 0.316929 | 2.321991 | 1.463065 | 0.001907 | 0.001908 | 0.001721 |
| mp_from_sequence_3.test | RUN | 0.274394 | 1.057909 | 1.027016 | 0.001756 | 0.002139 | 0.002045 |
| mp_min_element.test | RUN | 0.265891 | 0.949459 | 0.954132 | 0.001822 | 0.002111 | 0.001923 |
| mp_min_element_2.test | RUN | 0.042676 | 0.082070 | 0.140848 | 0.000860 | 0.000899 | 0.002044 |
| mp_min_element_q.test | RUN | 0.265476 | 0.931868 | 0.924768 | 0.002207 | 0.002122 | 0.001938 |
| mp_min_element_q_2.test | RUN | 0.043181 | 0.079272 | 0.122779 | 0.000793 | 0.000912 | 0.002136 |
| mp_max_element.test | RUN | 0.265384 | 0.943851 | 0.888783 | 0.001940 | 0.002102 | 0.001900 |
| mp_max_element_2.test | RUN | 0.041982 | 0.078166 | 0.150438 | 0.000726 | 0.000843 | 0.001895 |
| mp_max_element_q.test | RUN | 0.266174 | 0.943239 | 0.925380 | 0.001870 | 0.002246 | 0.001572 |
| mp_max_element_q_2.test | RUN | 0.041027 | 0.081168 | 0.127114 | 0.000757 | 0.000967 | 0.002000 |
| mp_nth_element.test | RUN | 0.295082 | 1.324586 | 1.211028 | 0.002048 | 0.001970 | 0.001853 |
| mp_nth_element_q.test | RUN | 0.289573 | 1.288034 | 1.208408 | 0.001757 | 0.002012 | 0.001978 |
| mp_back.test | RUN | 0.283696 | 1.089045 | 1.083907 | 0.001733 | 0.002286 | 0.002116 |
| mp_back_2.test | RUN | 0.044187 | 0.077195 | 0.142815 | 0.000780 | 0.000933 | 0.002034 |
| mp_pop_back.test | RUN | 0.652385 | 6.942465 | 2.152296 | 0.002038 | 0.002296 | 0.001826 |
| mp_pop_back_2.test | RUN | 0.044787 | 0.081531 | 0.126643 | 0.000901 | 0.000943 | 0.002076 |
| mp_flatten.test | RUN | 0.322020 | 2.466763 | 1.507035 | 0.002022 | 0.002332 | 0.002097 |
| mp_rotate_left.test | RUN | 0.318828 | 2.682425 | 1.470031 | 0.001852 | 0.001841 | 0.001903 |
| mp_rotate_right.test | RUN | 0.321557 | 2.687367 | 1.490732 | 0.001932 | 0.002035 | 0.002120 |
| mp_power_set.test | RUN | 0.315965 | 2.570620 | 1.469131 | 0.001924 | 0.002305 | 0.001960 |
| mp_partial_sum.test | RUN | 0.386431 | 3.388007 | 1.690575 | 0.001796 | 0.001647 | 0.001910 |
| mp_iterate.test | RUN | 0.289853 | 1.074855 | 1.212957 | 0.001881 | 0.002181 | 0.001919 |
| mp_pairwise_fold.test | RUN | 0.319490 | 2.361839 | 1.499315 | 0.001798 | 0.002224 | 0.001955 |
| mp_pairwise_fold_q.test | RUN | 0.257770 | 0.722418 | 0.902916 | 0.002402 | 0.002048 | 0.001940 |
| mp_sliding_fold.test | RUN | 0.536792 | 4.469920 | 2.649516 | 0.001928 | 0.002395 | 0.002010 |
| mp_sliding_fold_q.test | RUN | 0.539496 | 4.497746 | 2.745767 | 0.001840 | 0.001993 | 0.001953 |
| mp_intersperse.test | RUN | 0.297189 | 1.729103 | 1.346559 | 0.001914 | 0.002168 | 0.002056 |
| mp_split.test | RUN | 0.392681 | 3.271406 | 1.780526 | 0.002008 | 0.002106 | 0.002248 |
| mp_join.test | RUN | 0.324920 | 2.723692 | 1.609518 | 0.001890 | 0.002350 | 0.002059 |
| integral.test | RUN | 0.268118 | 1.246785 | 0.954266 | 0.001935 | 0.002085 | 0.002194 |
| mp_value.test | RUN | 0.018498 | 0.043528 | 0.062697 | 0.000765 | 0.000833 | 0.002142 |
| mp_identity.test | RUN | 0.255653 | 1.133088 | 0.939236 | 0.001968 | 0.002156 | 0.001826 |
| mp_inherit.test | RUN | 0.247500 | 0.824862 | 0.902911 | 0.001805 | 0.002174 | 0.001898 |
| mp_if.test | RUN | 0.248831 | 0.933857 | 0.900456 | 0.001958 | 0.002370 | 0.001730 |
| mp_if_sf.test | RUN | 0.239715 | 0.681124 | 0.853755 | 0.002142 | 0.002348 | 0.001872 |
| mp_eval_if.test | RUN | 0.252045 | 1.014261 | 0.963203 | 0.001932 | 0.002131 | 0.001774 |
| mp_eval_if_sf.test | RUN | 0.247653 | 0.728478 | 0.895252 | 0.001821 | 0.002103 | 0.001749 |
| mp_valid.test | RUN | 0.246842 | 0.719515 | 0.888102 | 0.002014 | 0.001975 | 0.002011 |
| mp_defer.test | RUN | 0.262493 | 1.195734 | 1.018623 | 0.002004 | 0.002237 | 0.002122 |
| mp_quote.test | RUN | 0.263198 | 1.200566 | 1.056517 | 0.001889 | 0.002281 | 0.001982 |
| mp_invoke_q.test | RUN | 0.275526 | 1.583558 | 1.165902 | 0.001775 | 0.002301 | 0.002098 |
| mp_invoke_q_sf.test | RUN | 0.240976 | 0.719682 | 0.826753 | 0.001758 | 0.002372 | 0.002048 |
| mp_quote_trait.test | RUN | 0.264485 | 1.302985 | 1.015950 | 0.002048 | 0.002226 | 0.002010 |
| mp_cond.test | RUN | 0.249755 | 0.808792 | 0.917798 | 0.001977 | 0.001999 | 0.002072 |
| mp_cond_sf.test | RUN | 0.239370 | 0.707584 | 0.834114 | 0.001830 | 0.002278 | 0.002144 |
| mp_not_fn.test | RUN | 0.251950 | 0.704898 | 0.865800 | 0.001888 | 0.002290 | 0.001969 |
| mp_eval_if_not.test | RUN | 0.252654 | 1.011121 | 0.915739 | 0.002110 | 0.002096 | 0.002071 |
| mp_eval_or.test | RUN | 0.241128 | 0.551859 | 0.892340 | 0.001880 | 0.002273 | 0.001894 |
| mp_compose.test | RUN | 0.262632 | 1.627289 | 1.049880 | 0.001922 | 0.001635 | 0.001998 |
| mp_valid_and_true.test | RUN | 0.247308 | 0.831791 | 0.911639 | 0.001792 | 0.002195 | 0.002140 |
| mp_compose_sf.test | COMPILE | 0.062559 | 0.111910 | 0.214998 | — | — | — |
| integer_sequence.test | RUN | 0.287824 | 1.598024 | 1.247819 | 0.001972 | 0.002093 | 0.001723 |
| tuple_for_each.test | RUN | 0.256375 | 0.607668 | 0.891212 | 0.001438 | 0.001923 | 0.002128 |
| tuple_for_each_cx.test | COMPILE | 0.046822 | 0.093824 | 0.163175 | — | — | — |
| tuple_apply.test | RUN | 0.257418 | 0.635140 | 0.946983 | 0.001950 | 0.002156 | 0.001880 |
| tuple_apply_cx.test | COMPILE | 0.070222 | 0.117812 | 0.228821 | — | — | — |
| construct_from_tuple.test | RUN | 0.261099 | 0.655629 | 0.899549 | 0.001845 | 0.002084 | 0.001858 |
| construct_from_tuple_cx.test | COMPILE | 0.074127 | 0.129843 | 0.227060 | — | — | — |
| tuple_transform.test | RUN | 0.394849 | 1.307365 | 1.830809 | 0.001543 | 0.001876 | 0.001765 |
| tuple_transform_2.test | RUN | 0.354950 | 0.997171 | 1.694798 | 0.001925 | 0.001840 | 0.001692 |
| tuple_transform_cx.test | COMPILE | 0.056343 | 0.125119 | 0.203577 | — | — | — |
| mp_set_contains.test | RUN | 0.250808 | 0.862040 | 0.904296 | 0.001860 | 0.002004 | 0.002144 |
| mp_set_push_back.test | RUN | 0.305987 | 2.588611 | 1.510742 | 0.001986 | 0.001972 | 0.002028 |
| mp_set_push_front.test | RUN | 0.384035 | 3.052718 | 1.684476 | 0.001903 | 0.002197 | 0.002046 |
| mp_is_set.test | RUN | 0.259069 | 0.854404 | 0.913012 | 0.001952 | 0.002096 | 0.002240 |
| mp_set_union.test | RUN | 0.301720 | 2.166855 | 1.375181 | 0.001917 | 0.002306 | 0.001986 |
| mp_set_union_sf.test | RUN | 0.252277 | 0.691059 | 0.876611 | 0.001729 | 0.002086 | 0.001848 |
| mp_set_difference.test | RUN | 0.276586 | 1.709803 | 1.200145 | 0.001832 | 0.002196 | 0.001810 |
| mp_set_difference_sf.test | RUN | 0.240558 | 0.715760 | 0.880628 | 0.001998 | 0.002164 | 0.001628 |
| mp_set_intersection.test | RUN | 0.276752 | 1.701637 | 1.213222 | 0.001754 | 0.002104 | 0.002184 |
| mp_set_intersection_sf.test | RUN | 0.244185 | 0.702723 | 0.852094 | 0.001664 | 0.002107 | 0.001959 |
| mp_all.test | RUN | 0.251567 | 0.834031 | 0.940668 | 0.001842 | 0.002282 | 0.001840 |
| mp_all_2.test | RUN | 0.633847 | 1.772002 | 2.003945 | 0.002041 | 0.002122 | 0.002002 |
| mp_and.test | RUN | 0.247590 | 0.857293 | 0.918943 | 0.001615 | 0.001990 | 0.001701 |
| mp_any.test | RUN | 0.255656 | 0.784558 | 0.887079 | 0.001864 | 0.002006 | 0.001953 |
| mp_any_2.test | RUN | 0.626064 | 1.803823 | 1.956360 | 0.001932 | 0.002408 | 0.001980 |
| mp_or.test | RUN | 0.254678 | 0.799001 | 0.947118 | 0.002022 | 0.002073 | 0.002185 |
| mp_same.test | RUN | 0.437828 | 1.391347 | 1.310922 | 0.001886 | 0.001914 | 0.002192 |
| mp_plus.test | RUN | 0.265215 | 0.988019 | 1.009029 | 0.001620 | 0.002065 | 0.001818 |
| mp_less.test | RUN | 0.252652 | 0.851135 | 0.935473 | 0.002120 | 0.002206 | 0.002050 |
| mp_min.test | RUN | 0.245488 | 0.798199 | 0.880305 | 0.001951 | 0.002232 | 0.002084 |
| mp_max.test | RUN | 0.251036 | 0.861852 | 0.914394 | 0.002045 | 0.002177 | 0.002136 |
| mp_similar.test | RUN | 0.253405 | 0.857279 | 0.927632 | 0.001740 | 0.001885 | 0.002132 |
| mp_map_find.test | RUN | 0.289993 | 1.557906 | 1.180631 | 0.002080 | 0.002166 | 0.001963 |
| mp_map_find_2.test | RUN | 0.273332 | 0.609494 | 1.007478 | 0.002183 | 0.002205 | 0.002340 |
| mp_map_find_3.test | RUN | 0.270548 | 0.610663 | 0.985989 | 0.001874 | 0.001950 | 0.001742 |
| mp_map_find_4.test | RUN | 0.260487 | 0.580642 | 0.918015 | 0.001949 | 0.001974 | 0.001752 |
| mp_map_contains.test | RUN | 0.260674 | 0.880820 | 0.986254 | 0.002129 | 0.002385 | 0.002218 |
| mp_map_insert.test | RUN | 0.327788 | 2.878812 | 1.606865 | 0.002047 | 0.002311 | 0.002040 |
| mp_map_replace.test | RUN | 0.391925 | 3.244834 | 1.853078 | 0.001859 | 0.002317 | 0.001990 |
| mp_map_erase.test | RUN | 0.322117 | 2.455227 | 1.598257 | 0.001946 | 0.002130 | 0.001788 |
| mp_map_update.test | RUN | 0.291121 | 1.596692 | 1.179572 | 0.002076 | 0.002253 | 0.001730 |
| mp_map_update_q.test | RUN | 0.283995 | 1.593424 | 1.196934 | 0.002005 | 0.002128 | 0.002103 |
| mp_map_keys.test | RUN | 0.288096 | 1.420695 | 1.201119 | 0.001944 | 0.002098 | 0.002150 |
| mp_is_map.test | RUN | 0.265685 | 0.891176 | 1.001430 | 0.001804 | 0.001751 | 0.001812 |
| mp_bind.test | RUN | 0.313950 | 2.496260 | 1.408521 | 0.002086 | 0.002072 | 0.002076 |
| mp_bind_q.test | RUN | 0.287885 | 1.587288 | 1.210989 | 0.001911 | 0.002018 | 0.001978 |
| mp_bind_front.test | RUN | 0.276658 | 1.735693 | 1.135319 | 0.002064 | 0.002094 | 0.001980 |
| mp_bind_back.test | RUN | 0.277008 | 1.744299 | 1.103064 | 0.001644 | 0.002140 | 0.001965 |
| mp_lambda.test | RUN | 0.555555 | 5.068158 | 2.845779 | 0.001944 | 0.001612 | 0.002156 |
| mpl.test | RUN | 0.545212 | 1.425727 | 2.227406 | 0.001870 | 0.002228 | 0.001736 |
| mpl_list.test | RUN | 0.336871 | 1.046072 | 1.126487 | 0.001846 | 0.001934 | 0.001966 |
| mpl_tuple.test | RUN | 0.335939 | 1.062550 | 1.141389 | 0.001915 | 0.002102 | 0.001990 |
| version.test | RUN | 0.168111 | 0.384033 | 0.575330 | 0.001952 | 0.002490 | 0.001721 |
| mp11_version.test | RUN | 0.206500 | 0.449969 | 0.655929 | 0.001658 | 0.002296 | 0.002186 |
| list_version.test | RUN | 0.175314 | 0.404043 | 0.575302 | 0.002058 | 0.002142 | 0.001817 |
| algorithm_version.test | RUN | 0.192286 | 0.420582 | 0.660103 | 0.001771 | 0.002291 | 0.001754 |
| integral_version.test | RUN | 0.167767 | 0.393590 | 0.595829 | 0.001819 | 0.002075 | 0.001634 |
| utility_version.test | RUN | 0.175962 | 0.385789 | 0.573255 | 0.001910 | 0.002224 | 0.002055 |
| function_version.test | RUN | 0.174165 | 0.389830 | 0.581977 | 0.001660 | 0.002170 | 0.002004 |
| map_version.test | RUN | 0.188793 | 0.426189 | 0.658544 | 0.002178 | 0.002244 | 0.001650 |
| set_version.test | RUN | 0.180609 | 0.391277 | 0.632489 | 0.002032 | 0.002226 | 0.001782 |
| bind_version.test | RUN | 0.193597 | 0.407872 | 0.629272 | 0.001426 | 0.002212 | 0.002092 |
| integer_sequence_version.test | RUN | 0.170804 | 0.379339 | 0.408813 | 0.001953 | 0.001827 | 0.001420 |
| tuple_version.test | RUN | 0.189683 | 0.409467 | 0.557901 | 0.001483 | 0.001542 | 0.001482 |
| mpl_version.test | RUN | 0.142316 | 0.352145 | 0.580470 | 0.001399 | 0.001586 | 0.001417 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.994× | 0.988×, 1.000× | 1.003× |
| clang | 1.082× | 1.073×, 1.092× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001182s | 0.001195s | 0.001183s | byte equality observed |
| bind_version.test | 10 | 0.001199s | 0.001181s | 0.001217s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001189s | 0.001183s | 0.001223s | byte equality observed |
| function_version.test | 10 | 0.001195s | 0.001177s | 0.001243s | byte equality observed |
| integer_sequence.test | 10 | 0.001169s | 0.001198s | 0.001239s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001184s | 0.001161s | 0.001254s | byte equality observed |
| integral.test | 10 | 0.001188s | 0.001189s | 0.001257s | byte equality observed |
| integral_version.test | 10 | 0.001199s | 0.001198s | 0.001273s | byte equality observed |
| list_version.test | 10 | 0.001199s | 0.001173s | 0.001275s | byte equality observed |
| map_version.test | 10 | 0.001183s | 0.001174s | 0.001348s | byte equality observed |
| mp11_version.test | 10 | 0.001186s | 0.001185s | 0.001298s | byte equality observed |
| mp_all.test | 10 | 0.001198s | 0.001197s | 0.001308s | byte equality observed |
| mp_all_2.test | 10 | 0.001200s | 0.001188s | 0.001257s | byte equality observed |
| mp_all_of.test | 10 | 0.001209s | 0.001176s | 0.001286s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001204s | 0.001176s | 0.001255s | byte equality observed |
| mp_and.test | 10 | 0.001222s | 0.001182s | 0.001285s | byte equality observed |
| mp_any.test | 10 | 0.001258s | 0.001183s | 0.001264s | byte equality observed |
| mp_any_2.test | 10 | 0.001288s | 0.001193s | 0.001237s | byte equality observed |
| mp_any_of.test | 10 | 0.001284s | 0.001197s | 0.001305s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001254s | 0.001197s | 0.001271s | byte equality observed |
| mp_append.test | 10 | 0.001300s | 0.001206s | 0.001263s | byte equality observed |
| mp_append_2.test | 10 | 0.001340s | 0.001203s | 0.001260s | byte equality observed |
| mp_append_3.test | 10 | 0.000614s | 0.000562s | 0.001232s | byte equality observed |
| mp_append_sf.test | 10 | 0.001305s | 0.001227s | 0.001255s | byte equality observed |
| mp_apply_q.test | 10 | 0.001295s | 0.001246s | 0.001298s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001326s | 0.001317s | 0.001267s | byte equality observed |
| mp_assign.test | 10 | 0.001288s | 0.001332s | 0.001295s | byte equality observed |
| mp_assign_2.test | 10 | 0.000606s | 0.000604s | 0.001283s | byte equality observed |
| mp_at.test | 10 | 0.001318s | 0.001301s | 0.001279s | byte equality observed |
| mp_at_2.test | 10 | 0.000604s | 0.000595s | 0.001307s | byte equality observed |
| mp_at_sf.test | 10 | 0.001269s | 0.001218s | 0.001253s | byte equality observed |
| mp_back.test | 10 | 0.001305s | 0.001260s | 0.001260s | byte equality observed |
| mp_back_2.test | 10 | 0.000600s | 0.000591s | 0.001241s | byte equality observed |
| mp_bind.test | 10 | 0.001306s | 0.001294s | 0.001255s | byte equality observed |
| mp_bind_back.test | 10 | 0.001315s | 0.001272s | 0.001257s | byte equality observed |
| mp_bind_front.test | 10 | 0.001279s | 0.001302s | 0.001271s | byte equality observed |
| mp_bind_q.test | 10 | 0.001281s | 0.001294s | 0.001237s | byte equality observed |
| mp_clear.test | 10 | 0.001322s | 0.001315s | 0.001276s | byte equality observed |
| mp_clear_2.test | 10 | 0.000598s | 0.000605s | 0.001214s | byte equality observed |
| mp_compose.test | 10 | 0.001274s | 0.001342s | 0.001263s | byte equality observed |
| mp_cond.test | 10 | 0.001277s | 0.001306s | 0.001286s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001268s | 0.001316s | 0.001249s | byte equality observed |
| mp_contains.test | 10 | 0.001302s | 0.001284s | 0.001230s | byte equality observed |
| mp_copy_if.test | 10 | 0.001289s | 0.001313s | 0.001282s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001300s | 0.001283s | 0.001255s | byte equality observed |
| mp_count.test | 10 | 0.001309s | 0.001267s | 0.001261s | byte equality observed |
| mp_count_if.test | 10 | 0.001267s | 0.001295s | 0.001357s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001294s | 0.001292s | 0.001301s | byte equality observed |
| mp_defer.test | 10 | 0.001273s | 0.001353s | 0.001278s | byte equality observed |
| mp_drop.test | 10 | 0.001297s | 0.001327s | 0.001290s | byte equality observed |
| mp_drop_2.test | 10 | 0.000611s | 0.000616s | 0.001238s | byte equality observed |
| mp_empty.test | 10 | 0.001312s | 0.001332s | 0.001295s | byte equality observed |
| mp_empty_2.test | 10 | 0.000595s | 0.000592s | 0.001253s | byte equality observed |
| mp_erase.test | 10 | 0.001317s | 0.001339s | 0.001279s | byte equality observed |
| mp_erase_2.test | 10 | 0.000598s | 0.000601s | 0.001269s | byte equality observed |
| mp_eval_if.test | 10 | 0.001301s | 0.001292s | 0.001278s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001281s | 0.001311s | 0.001279s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001296s | 0.001266s | 0.001230s | byte equality observed |
| mp_eval_or.test | 10 | 0.001293s | 0.001261s | 0.001275s | byte equality observed |
| mp_fill.test | 10 | 0.001296s | 0.001298s | 0.001281s | byte equality observed |
| mp_fill_2.test | 10 | 0.000615s | 0.000604s | 0.001221s | byte equality observed |
| mp_filter.test | 10 | 0.001278s | 0.001374s | 0.001269s | byte equality observed |
| mp_find.test | 10 | 0.001302s | 0.001263s | 0.001305s | byte equality observed |
| mp_find_if.test | 10 | 0.001279s | 0.001294s | 0.001285s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001274s | 0.001295s | 0.001327s | byte equality observed |
| mp_flatten.test | 10 | 0.001304s | 0.001290s | 0.001253s | byte equality observed |
| mp_fold.test | 10 | 0.001362s | 0.001356s | 0.001332s | byte equality observed |
| mp_fold_q.test | 10 | 0.001310s | 0.001307s | 0.001263s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001303s | 0.001264s | 0.001230s | byte equality observed |
| mp_for_each.test | 10 | 0.001290s | 0.001235s | 0.001308s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001268s | 0.001290s | 0.001310s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001294s | 0.001279s | 0.001289s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001305s | 0.001234s | 0.001281s | byte equality observed |
| mp_front.test | 10 | 0.001310s | 0.001281s | 0.001293s | byte equality observed |
| mp_front_2.test | 10 | 0.000594s | 0.000575s | 0.001214s | byte equality observed |
| mp_identity.test | 10 | 0.001290s | 0.001318s | 0.001273s | byte equality observed |
| mp_if.test | 10 | 0.001273s | 0.001291s | 0.001237s | byte equality observed |
| mp_if_sf.test | 10 | 0.001283s | 0.001317s | 0.001253s | byte equality observed |
| mp_inherit.test | 10 | 0.001318s | 0.001324s | 0.001282s | byte equality observed |
| mp_insert.test | 10 | 0.001350s | 0.001362s | 0.001246s | byte equality observed |
| mp_insert_2.test | 10 | 0.000606s | 0.000604s | 0.001221s | byte equality observed |
| mp_intersperse.test | 10 | 0.001265s | 0.001276s | 0.001233s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001252s | 0.001298s | 0.001241s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001276s | 0.001281s | 0.001284s | byte equality observed |
| mp_iota.test | 10 | 0.001283s | 0.001273s | 0.001289s | byte equality observed |
| mp_iota_2.test | 10 | 0.001287s | 0.001262s | 0.001278s | byte equality observed |
| mp_is_list.test | 10 | 0.001287s | 0.001246s | 0.001322s | byte equality observed |
| mp_is_map.test | 10 | 0.001282s | 0.001313s | 0.001275s | byte equality observed |
| mp_is_set.test | 10 | 0.001286s | 0.001320s | 0.001309s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001277s | 0.001299s | 0.001274s | byte equality observed |
| mp_iterate.test | 10 | 0.001268s | 0.001290s | 0.001253s | byte equality observed |
| mp_join.test | 10 | 0.001278s | 0.001317s | 0.001236s | byte equality observed |
| mp_lambda.test | 10 | 0.001340s | 0.001331s | 0.001274s | byte equality observed |
| mp_less.test | 10 | 0.001285s | 0.001267s | 0.001257s | byte equality observed |
| mp_list_c.test | 10 | 0.001290s | 0.001299s | 0.001262s | byte equality observed |
| mp_list_v.test | 10 | 0.000598s | 0.000583s | 0.001301s | byte equality observed |
| mp_map_contains.test | 10 | 0.001303s | 0.001317s | 0.001243s | byte equality observed |
| mp_map_erase.test | 10 | 0.001315s | 0.001275s | 0.001266s | byte equality observed |
| mp_map_find.test | 10 | 0.001317s | 0.001309s | 0.001312s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001288s | 0.001300s | 0.001278s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001324s | 0.001249s | 0.001302s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001301s | 0.001328s | 0.001259s | byte equality observed |
| mp_map_insert.test | 10 | 0.001285s | 0.001343s | 0.001253s | byte equality observed |
| mp_map_keys.test | 10 | 0.001299s | 0.001283s | 0.001272s | byte equality observed |
| mp_map_replace.test | 10 | 0.001268s | 0.001304s | 0.001285s | byte equality observed |
| mp_map_update.test | 10 | 0.001240s | 0.001354s | 0.001275s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001264s | 0.001246s | 0.001272s | byte equality observed |
| mp_max.test | 10 | 0.001311s | 0.001289s | 0.001283s | byte equality observed |
| mp_max_element.test | 10 | 0.001280s | 0.001272s | 0.001281s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000592s | 0.000596s | 0.001337s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001286s | 0.001300s | 0.001235s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000598s | 0.000605s | 0.001310s | byte equality observed |
| mp_min.test | 10 | 0.001252s | 0.001268s | 0.001276s | byte equality observed |
| mp_min_element.test | 10 | 0.001265s | 0.001324s | 0.001257s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000598s | 0.000586s | 0.001279s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001310s | 0.001335s | 0.001234s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000600s | 0.000599s | 0.001235s | byte equality observed |
| mp_none_of.test | 10 | 0.001253s | 0.001273s | 0.001215s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001265s | 0.001296s | 0.001221s | byte equality observed |
| mp_not_fn.test | 10 | 0.001277s | 0.001250s | 0.001256s | byte equality observed |
| mp_nth_element.test | 10 | 0.001325s | 0.001283s | 0.001284s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001258s | 0.001229s | 0.001266s | byte equality observed |
| mp_or.test | 10 | 0.001276s | 0.001231s | 0.001249s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001301s | 0.001256s | 0.001221s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001260s | 0.001294s | 0.001248s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001286s | 0.001276s | 0.001288s | byte equality observed |
| mp_partition.test | 10 | 0.001248s | 0.001297s | 0.001272s | byte equality observed |
| mp_partition_q.test | 10 | 0.001277s | 0.001314s | 0.001342s | byte equality observed |
| mp_plus.test | 10 | 0.001306s | 0.001277s | 0.001278s | byte equality observed |
| mp_pop_back.test | 10 | 0.001340s | 0.001261s | 0.001278s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000603s | 0.000611s | 0.001245s | byte equality observed |
| mp_pop_front.test | 10 | 0.001301s | 0.001316s | 0.001245s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000584s | 0.000607s | 0.001238s | byte equality observed |
| mp_power_set.test | 10 | 0.001299s | 0.001322s | 0.001271s | byte equality observed |
| mp_product.test | 10 | 0.001347s | 0.001289s | 0.001252s | byte equality observed |
| mp_push_back.test | 10 | 0.001289s | 0.001322s | 0.001251s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000602s | 0.000599s | 0.001281s | byte equality observed |
| mp_push_front.test | 10 | 0.001295s | 0.001308s | 0.001236s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000598s | 0.000592s | 0.001314s | byte equality observed |
| mp_quote.test | 10 | 0.001245s | 0.001279s | 0.001256s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001284s | 0.001301s | 0.001271s | byte equality observed |
| mp_remove.test | 10 | 0.001249s | 0.001272s | 0.001281s | byte equality observed |
| mp_remove_if.test | 10 | 0.001271s | 0.001325s | 0.001298s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001303s | 0.001278s | 0.001268s | byte equality observed |
| mp_rename.test | 10 | 0.001275s | 0.001276s | 0.001233s | byte equality observed |
| mp_rename_2.test | 10 | 0.000611s | 0.000592s | 0.001275s | byte equality observed |
| mp_rename_v.test | 10 | 0.000603s | 0.000612s | 0.001250s | byte equality observed |
| mp_repeat.test | 10 | 0.001320s | 0.001311s | 0.001256s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000597s | 0.000614s | 0.001260s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001262s | 0.001262s | 0.001234s | byte equality observed |
| mp_replace.test | 10 | 0.001261s | 0.001321s | 0.001241s | byte equality observed |
| mp_replace_at.test | 10 | 0.001319s | 0.001300s | 0.001300s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001280s | 0.001340s | 0.001272s | byte equality observed |
| mp_replace_front.test | 10 | 0.001299s | 0.001318s | 0.001254s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000599s | 0.000582s | 0.001271s | byte equality observed |
| mp_replace_if.test | 10 | 0.001263s | 0.001243s | 0.001269s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001328s | 0.001251s | 0.001241s | byte equality observed |
| mp_replace_second.test | 10 | 0.001275s | 0.001301s | 0.001268s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000607s | 0.000599s | 0.001236s | byte equality observed |
| mp_replace_third.test | 10 | 0.001311s | 0.001304s | 0.001242s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000593s | 0.000595s | 0.001257s | byte equality observed |
| mp_reverse.test | 10 | 0.001304s | 0.001287s | 0.001216s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001322s | 0.001263s | 0.001250s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001270s | 0.001286s | 0.001245s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001251s | 0.001281s | 0.001254s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001309s | 0.001346s | 0.001278s | byte equality observed |
| mp_same.test | 10 | 0.001294s | 0.001294s | 0.001293s | byte equality observed |
| mp_second.test | 10 | 0.001310s | 0.001324s | 0.001301s | byte equality observed |
| mp_second_2.test | 10 | 0.000600s | 0.000601s | 0.001259s | byte equality observed |
| mp_set_contains.test | 10 | 0.001279s | 0.001281s | 0.001262s | byte equality observed |
| mp_set_difference.test | 10 | 0.001281s | 0.001291s | 0.001340s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001299s | 0.001328s | 0.001255s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001292s | 0.001306s | 0.001326s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001294s | 0.001298s | 0.001306s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001289s | 0.001300s | 0.001286s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001298s | 0.001319s | 0.001297s | byte equality observed |
| mp_set_union.test | 10 | 0.001265s | 0.001299s | 0.001268s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001293s | 0.001320s | 0.001286s | byte equality observed |
| mp_similar.test | 10 | 0.001333s | 0.001322s | 0.001298s | byte equality observed |
| mp_size.test | 10 | 0.001312s | 0.001300s | 0.001258s | byte equality observed |
| mp_size_2.test | 10 | 0.000596s | 0.000596s | 0.001251s | byte equality observed |
| mp_slice.test | 10 | 0.001309s | 0.001304s | 0.001232s | byte equality observed |
| mp_slice_2.test | 10 | 0.000592s | 0.000605s | 0.001313s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001310s | 0.001304s | 0.001246s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001346s | 0.001258s | 0.001275s | byte equality observed |
| mp_sort.test | 10 | 0.001306s | 0.001269s | 0.001226s | byte equality observed |
| mp_sort_q.test | 10 | 0.001289s | 0.001271s | 0.001231s | byte equality observed |
| mp_split.test | 10 | 0.001334s | 0.001284s | 0.001262s | byte equality observed |
| mp_starts_with.test | 10 | 0.001271s | 0.001336s | 0.001292s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001284s | 0.001302s | 0.001310s | byte equality observed |
| mp_take.test | 10 | 0.001297s | 0.001328s | 0.001242s | byte equality observed |
| mp_take_2.test | 10 | 0.000590s | 0.000585s | 0.001320s | byte equality observed |
| mp_third.test | 10 | 0.001328s | 0.001314s | 0.001242s | byte equality observed |
| mp_third_2.test | 10 | 0.000595s | 0.000600s | 0.001292s | byte equality observed |
| mp_transform.test | 10 | 0.001299s | 0.001249s | 0.001241s | byte equality observed |
| mp_transform_2.test | 10 | 0.001286s | 0.001257s | 0.001247s | byte equality observed |
| mp_transform_front.test | 10 | 0.001283s | 0.001290s | 0.001291s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000603s | 0.000594s | 0.001251s | byte equality observed |
| mp_transform_if.test | 10 | 0.001262s | 0.001258s | 0.001281s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001266s | 0.001239s | 0.001318s | byte equality observed |
| mp_transform_q.test | 10 | 0.001284s | 0.001255s | 0.001273s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001298s | 0.001267s | 0.001288s | byte equality observed |
| mp_transform_second.test | 10 | 0.001296s | 0.001277s | 0.001270s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000609s | 0.000590s | 0.001246s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001250s | 0.001303s | 0.001254s | byte equality observed |
| mp_transform_third.test | 10 | 0.001276s | 0.001291s | 0.001251s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000595s | 0.000590s | 0.001272s | byte equality observed |
| mp_unique.test | 10 | 0.001289s | 0.001272s | 0.001258s | byte equality observed |
| mp_unique_if.test | 10 | 0.001283s | 0.001266s | 0.001239s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001300s | 0.001262s | 0.001273s | byte equality observed |
| mp_valid.test | 10 | 0.001294s | 0.001284s | 0.001323s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001316s | 0.001304s | 0.001281s | byte equality observed |
| mp_value.test | 10 | 0.000597s | 0.000609s | 0.001256s | byte equality observed |
| mp_with_index.test | 9 | 0.001687s | 0.001676s | 0.001537s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000596s | 0.000579s | 0.001243s | byte equality observed |
| mpl.test | 10 | 0.001292s | 0.001235s | 0.001231s | byte equality observed |
| mpl_list.test | 10 | 0.001249s | 0.001289s | 0.001327s | byte equality observed |
| mpl_tuple.test | 10 | 0.001274s | 0.001312s | 0.001245s | byte equality observed |
| mpl_version.test | 10 | 0.001291s | 0.001281s | 0.001254s | byte equality observed |
| set_version.test | 10 | 0.001313s | 0.001304s | 0.001296s | byte equality observed |
| tuple_apply.test | 10 | 0.001262s | 0.001289s | 0.001325s | byte equality observed |
| tuple_for_each.test | 10 | 0.001269s | 0.001325s | 0.001271s | byte equality observed |
| tuple_transform.test | 10 | 0.001268s | 0.001283s | 0.001293s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001306s | 0.001280s | 0.001293s | byte equality observed |
| tuple_version.test | 10 | 0.001274s | 0.001331s | 0.001312s | byte equality observed |
| utility_version.test | 10 | 0.001271s | 0.001342s | 0.001263s | byte equality observed |
| version.test | 10 | 0.001297s | 0.001267s | 0.001308s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
