# libs/mp11/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| mp11.test | COMPILE | 0.050900 | 0.102674 | 0.170054 | — | — | — |
| complex_h.test | COMPILE | 0.209475 | 0.435505 | 0.781557 | — | — | — |
| mp_size.test | RUN | 0.238016 | 1.321436 | 0.900299 | 0.001952 | 0.002120 | 0.001763 |
| mp_size_2.test | RUN | 0.020317 | 0.047282 | 0.072834 | 0.000854 | 0.000859 | 0.001783 |
| mp_empty.test | RUN | 0.227790 | 0.824743 | 0.904359 | 0.001889 | 0.002434 | 0.002066 |
| mp_empty_2.test | RUN | 0.021539 | 0.052570 | 0.079965 | 0.000703 | 0.000710 | 0.001986 |
| mp_assign.test | RUN | 0.261481 | 1.702359 | 1.111905 | 0.002020 | 0.002177 | 0.002205 |
| mp_assign_2.test | RUN | 0.022836 | 0.047275 | 0.074173 | 0.000876 | 0.000922 | 0.001668 |
| mp_clear.test | RUN | 0.237560 | 0.787466 | 0.863401 | 0.001826 | 0.002003 | 0.002763 |
| mp_clear_2.test | RUN | 0.020431 | 0.054163 | 0.081382 | 0.000859 | 0.000799 | 0.001912 |
| mp_front.test | RUN | 0.237353 | 1.143861 | 0.965391 | 0.001899 | 0.002185 | 0.002005 |
| mp_front_2.test | RUN | 0.021260 | 0.050010 | 0.084504 | 0.000766 | 0.000805 | 0.001988 |
| mp_pop_front.test | RUN | 0.249390 | 1.246978 | 0.950007 | 0.002084 | 0.002050 | 0.002136 |
| mp_pop_front_2.test | RUN | 0.020275 | 0.050822 | 0.090384 | 0.000787 | 0.000906 | 0.002588 |
| mp_second.test | RUN | 0.236216 | 0.799290 | 0.856389 | 0.001857 | 0.001868 | 0.002141 |
| mp_second_2.test | RUN | 0.022845 | 0.050215 | 0.082527 | 0.000887 | 0.000836 | 0.002097 |
| mp_third.test | RUN | 0.227205 | 0.735290 | 0.873238 | 0.002082 | 0.002171 | 0.002046 |
| mp_third_2.test | RUN | 0.019520 | 0.049795 | 0.080703 | 0.000767 | 0.000845 | 0.001612 |
| mp_push_front.test | RUN | 0.298013 | 2.568494 | 1.538352 | 0.002004 | 0.002547 | 0.002152 |
| mp_push_front_2.test | RUN | 0.020909 | 0.047956 | 0.071998 | 0.000854 | 0.000813 | 0.001944 |
| mp_push_back.test | RUN | 0.295390 | 2.581722 | 1.482560 | 0.002017 | 0.002048 | 0.002075 |
| mp_push_back_2.test | RUN | 0.021337 | 0.051920 | 0.083145 | 0.000843 | 0.000957 | 0.002044 |
| mp_rename.test | RUN | 0.271955 | 2.341752 | 1.324395 | 0.002236 | 0.002272 | 0.002035 |
| mp_rename_2.test | RUN | 0.022891 | 0.053615 | 0.085724 | 0.000728 | 0.000937 | 0.001908 |
| mp_append.test | RUN | 0.302873 | 2.664038 | 1.366371 | 0.002056 | 0.002118 | 0.002043 |
| mp_append_2.test | RUN | 0.629460 | 5.940791 | 1.819679 | 0.001636 | 0.002149 | 0.001976 |
| mp_append_3.test | RUN | 0.022758 | 0.049381 | 0.079140 | 0.000904 | 0.000954 | 0.001974 |
| mp_append_sf.test | RUN | 0.235062 | 0.703522 | 0.878160 | 0.001936 | 0.001790 | 0.002060 |
| mp_replace_front.test | RUN | 0.277266 | 1.920717 | 1.115692 | 0.001927 | 0.002173 | 0.001730 |
| mp_replace_front_2.test | RUN | 0.025068 | 0.051665 | 0.080634 | 0.000799 | 0.000835 | 0.001949 |
| mp_replace_second.test | RUN | 0.266601 | 1.253590 | 1.123253 | 0.002050 | 0.001934 | 0.002116 |
| mp_replace_second_2.test | RUN | 0.021528 | 0.050567 | 0.079668 | 0.000866 | 0.001157 | 0.001688 |
| mp_replace_third.test | RUN | 0.250301 | 1.021841 | 1.020132 | 0.002078 | 0.001765 | 0.001991 |
| mp_replace_third_2.test | RUN | 0.020905 | 0.049746 | 0.074725 | 0.000859 | 0.000864 | 0.002020 |
| mp_apply_q.test | RUN | 0.286569 | 2.340069 | 1.283315 | 0.001908 | 0.002081 | 0.002062 |
| mp_apply_q_sf.test | RUN | 0.227908 | 0.697540 | 0.861021 | 0.002122 | 0.001862 | 0.001619 |
| mp_is_list.test | RUN | 0.228706 | 0.818865 | 0.847136 | 0.002021 | 0.001979 | 0.002006 |
| mp_list_c.test | RUN | 0.244253 | 1.008423 | 0.970601 | 0.001924 | 0.002232 | 0.001950 |
| mp_transform_front.test | RUN | 0.262939 | 1.912198 | 1.080191 | 0.001846 | 0.001830 | 0.002503 |
| mp_transform_front_2.test | RUN | 0.021803 | 0.049608 | 0.082751 | 0.000889 | 0.000731 | 0.001828 |
| mp_transform_second.test | RUN | 0.262981 | 1.651503 | 1.101974 | 0.002304 | 0.001495 | 0.001985 |
| mp_transform_second_2.test | RUN | 0.020401 | 0.046756 | 0.079776 | 0.000885 | 0.001020 | 0.001954 |
| mp_transform_third.test | RUN | 0.275229 | 1.260901 | 1.000160 | 0.001733 | 0.002348 | 0.002085 |
| mp_transform_third_2.test | RUN | 0.022918 | 0.052559 | 0.072092 | 0.000708 | 0.000858 | 0.001994 |
| mp_list_v.test | RUN | 0.022581 | 0.048489 | 0.071872 | 0.000723 | 0.000790 | 0.001728 |
| mp_rename_v.test | RUN | 0.019689 | 0.049992 | 0.085384 | 0.000747 | 0.001024 | 0.001789 |
| mp_is_value_list.test | RUN | 0.231281 | 0.520399 | 0.853231 | 0.002012 | 0.001919 | 0.001930 |
| mp_transform.test | RUN | 0.508482 | 3.965858 | 2.399542 | 0.002066 | 0.002130 | 0.001936 |
| mp_transform_2.test | RUN | 0.215299 | 0.480757 | 0.770545 | 0.001948 | 0.002152 | 0.001695 |
| mp_transform_q.test | RUN | 0.491068 | 3.770023 | 2.270553 | 0.001907 | 0.001888 | 0.001988 |
| mp_transform_q_2.test | RUN | 0.543346 | 1.250025 | 2.910732 | 0.002048 | 0.002563 | 0.001737 |
| mp_transform_sf.test | RUN | 0.235906 | 0.730049 | 0.905890 | 0.001972 | 0.002697 | 0.002052 |
| mp_transform_if.test | RUN | 0.282425 | 1.942000 | 1.270223 | 0.001857 | 0.001771 | 0.002050 |
| mp_transform_if_q.test | RUN | 0.301093 | 1.946613 | 1.309236 | 0.001896 | 0.001850 | 0.001734 |
| mp_filter.test | RUN | 0.257479 | 1.171028 | 1.009113 | 0.002013 | 0.002031 | 0.001385 |
| mp_fill.test | RUN | 0.244540 | 0.897566 | 0.959443 | 0.002131 | 0.001973 | 0.002084 |
| mp_fill_2.test | RUN | 0.039178 | 0.077667 | 0.130565 | 0.000839 | 0.000934 | 0.002461 |
| mp_count.test | RUN | 0.450666 | 2.034926 | 1.483857 | 0.001871 | 0.002403 | 0.001946 |
| mp_count_if.test | RUN | 0.465200 | 2.061544 | 1.619524 | 0.001787 | 0.002223 | 0.002116 |
| mp_count_if_q.test | RUN | 0.251870 | 1.399095 | 0.966639 | 0.002010 | 0.001897 | 0.001932 |
| mp_contains.test | RUN | 0.230843 | 0.857856 | 0.917918 | 0.001780 | 0.002126 | 0.001962 |
| mp_starts_with.test | RUN | 0.249955 | 0.855672 | 0.896668 | 0.001903 | 0.001874 | 0.002032 |
| mp_starts_with_sf.test | RUN | 0.235182 | 0.709133 | 0.915101 | 0.001811 | 0.002194 | 0.002054 |
| mp_repeat.test | RUN | 0.343434 | 3.387931 | 1.675959 | 0.001948 | 0.001926 | 0.002088 |
| mp_repeat_2.test | RUN | 0.038621 | 0.079632 | 0.138447 | 0.000752 | 0.000890 | 0.001884 |
| mp_repeat_3.test | RUN | 0.281734 | 0.469607 | 0.810197 | 0.001884 | 0.002147 | 0.002062 |
| mp_product.test | RUN | 0.283327 | 2.055521 | 1.172754 | 0.001961 | 0.002224 | 0.002261 |
| mp_drop.test | RUN | 0.298662 | 2.555426 | 1.395552 | 0.002175 | 0.002264 | 0.002038 |
| mp_drop_2.test | RUN | 0.040294 | 0.082224 | 0.143566 | 0.000692 | 0.000857 | 0.001871 |
| mp_iota.test | RUN | 0.300944 | 2.167256 | 1.194779 | 0.002102 | 0.002098 | 0.002076 |
| mp_iota_2.test | RUN | 0.313439 | 2.642521 | 1.410473 | 0.001762 | 0.002076 | 0.002005 |
| mp_at.test | RUN | 0.257019 | 1.147348 | 1.077722 | 0.001692 | 0.002244 | 0.002304 |
| mp_at_2.test | RUN | 0.041900 | 0.080173 | 0.144484 | 0.000818 | 0.001075 | 0.002567 |
| mp_at_sf.test | RUN | 0.239830 | 0.747020 | 0.858649 | 0.001929 | 0.002010 | 0.002004 |
| mp_take.test | RUN | 0.378274 | 3.348333 | 1.600303 | 0.001835 | 0.002188 | 0.002050 |
| mp_take_2.test | RUN | 0.041236 | 0.077719 | 0.144130 | 0.000940 | 0.000913 | 0.002108 |
| mp_slice.test | RUN | 0.440310 | 3.384757 | 2.043919 | 0.001818 | 0.002089 | 0.001720 |
| mp_slice_2.test | RUN | 0.038310 | 0.081431 | 0.142198 | 0.000836 | 0.000972 | 0.001963 |
| mp_replace.test | RUN | 0.299281 | 1.765008 | 1.357551 | 0.002160 | 0.002110 | 0.001936 |
| mp_replace_if.test | RUN | 0.283837 | 1.532256 | 1.233211 | 0.002032 | 0.002122 | 0.002134 |
| mp_replace_if_q.test | RUN | 0.283455 | 1.541738 | 1.313720 | 0.001715 | 0.002224 | 0.002119 |
| mp_copy_if.test | RUN | 0.294937 | 2.156472 | 1.202004 | 0.001962 | 0.002098 | 0.001872 |
| mp_copy_if_q.test | RUN | 0.296447 | 2.197208 | 1.236492 | 0.001984 | 0.002248 | 0.002204 |
| mp_remove.test | RUN | 0.290405 | 1.448072 | 1.240707 | 0.002562 | 0.002318 | 0.002003 |
| mp_remove_if.test | RUN | 0.290739 | 2.281435 | 1.241847 | 0.001852 | 0.001994 | 0.002134 |
| mp_remove_if_q.test | RUN | 0.307207 | 2.290572 | 1.241483 | 0.002018 | 0.002052 | 0.001948 |
| mp_partition.test | RUN | 0.301520 | 2.121373 | 1.360313 | 0.001999 | 0.002194 | 0.001804 |
| mp_partition_q.test | RUN | 0.294668 | 2.130953 | 1.416111 | 0.001717 | 0.002026 | 0.001921 |
| mp_sort.test | RUN | 0.272376 | 1.564894 | 1.187467 | 0.001868 | 0.002292 | 0.001828 |
| mp_sort_q.test | RUN | 0.284705 | 1.556357 | 1.159260 | 0.002108 | 0.002130 | 0.002059 |
| mp_find.test | RUN | 0.268819 | 1.676479 | 1.089392 | 0.001924 | 0.001696 | 0.001920 |
| mp_find_if.test | RUN | 0.257540 | 1.528404 | 1.053751 | 0.001920 | 0.002176 | 0.001857 |
| mp_find_if_q.test | RUN | 0.281747 | 1.747166 | 1.236501 | 0.001840 | 0.002266 | 0.002035 |
| mp_reverse.test | RUN | 0.538076 | 4.408592 | 2.215941 | 0.002636 | 0.002154 | 0.002100 |
| mp_fold.test | RUN | 0.919105 | 3.431905 | 5.426938 | 0.001926 | 0.002119 | 0.002079 |
| mp_fold_q.test | RUN | 0.287230 | 1.544982 | 1.183596 | 0.001777 | 0.002224 | 0.001983 |
| mp_fold_q_sf.test | RUN | 0.248303 | 0.728897 | 0.924018 | 0.002056 | 0.002200 | 0.002061 |
| mp_reverse_fold.test | RUN | 0.466680 | 3.343243 | 1.599838 | 0.001950 | 0.002174 | 0.001988 |
| mp_reverse_fold_q.test | RUN | 0.273151 | 1.878728 | 1.225344 | 0.002143 | 0.002735 | 0.001983 |
| mp_unique.test | RUN | 0.279905 | 1.696828 | 1.190265 | 0.001795 | 0.002048 | 0.002149 |
| mp_unique_if.test | RUN | 0.465144 | 3.400825 | 1.809433 | 0.003024 | 0.002187 | 0.002114 |
| mp_unique_if_q.test | RUN | 0.407578 | 3.445025 | 1.496325 | 0.001887 | 0.001976 | 0.001851 |
| mp_all_of.test | RUN | 0.635962 | 1.942574 | 2.014503 | 0.002152 | 0.002070 | 0.002033 |
| mp_all_of_q.test | RUN | 0.244356 | 0.846138 | 0.916389 | 0.001982 | 0.002252 | 0.001949 |
| mp_any_of.test | RUN | 0.617509 | 1.962003 | 1.942998 | 0.002038 | 0.001968 | 0.001753 |
| mp_any_of_q.test | RUN | 0.243051 | 0.852143 | 0.900379 | 0.001979 | 0.001985 | 0.001795 |
| mp_none_of.test | RUN | 0.651942 | 1.936064 | 1.962347 | 0.001964 | 0.002633 | 0.001469 |
| mp_none_of_q.test | RUN | 0.235664 | 0.833963 | 0.944648 | 0.002130 | 0.002136 | 0.001861 |
| mp_replace_at.test | RUN | 0.285525 | 1.644853 | 1.361499 | 0.002134 | 0.002136 | 0.001983 |
| mp_replace_at_c.test | RUN | 0.288039 | 1.610530 | 1.311636 | 0.001734 | 0.002162 | 0.001980 |
| mp_for_each.test | RUN | 0.286908 | 1.237063 | 1.536929 | 0.001905 | 0.002160 | 0.002430 |
| mp_insert.test | RUN | 0.418774 | 3.365210 | 1.895203 | 0.002068 | 0.002102 | 0.002136 |
| mp_insert_2.test | RUN | 0.039278 | 0.082679 | 0.141914 | 0.000690 | 0.000726 | 0.002025 |
| mp_erase.test | RUN | 0.459149 | 3.560298 | 2.096697 | 0.002239 | 0.002326 | 0.002006 |
| mp_erase_2.test | RUN | 0.037688 | 0.077644 | 0.150769 | 0.000780 | 0.000933 | 0.002027 |
| mp_with_index.test | RUN | 0.603473 | 7.279009 | 4.759648 | 0.002500 | 0.002446 | 0.002336 |
| mp_with_index_cx.test | RUN | 0.004144 | 0.022992 | 0.035698 | 0.000817 | 0.000868 | 0.002021 |
| mp_from_sequence.test | RUN | 0.284631 | 1.923326 | 1.339721 | 0.002006 | 0.002028 | 0.002060 |
| mp_from_sequence_2.test | RUN | 0.303384 | 2.292117 | 1.359528 | 0.001788 | 0.002322 | 0.002260 |
| mp_from_sequence_3.test | RUN | 0.245717 | 1.026084 | 0.985551 | 0.001859 | 0.002150 | 0.001928 |
| mp_min_element.test | RUN | 0.248089 | 0.939530 | 0.952404 | 0.002153 | 0.002096 | 0.001855 |
| mp_min_element_2.test | RUN | 0.038049 | 0.080586 | 0.146667 | 0.000849 | 0.000904 | 0.001593 |
| mp_min_element_q.test | RUN | 0.246693 | 0.938931 | 0.897958 | 0.002318 | 0.002040 | 0.001815 |
| mp_min_element_q_2.test | RUN | 0.039324 | 0.079336 | 0.150066 | 0.000794 | 0.000880 | 0.002074 |
| mp_max_element.test | RUN | 0.254994 | 0.925424 | 0.922585 | 0.001898 | 0.002084 | 0.001634 |
| mp_max_element_2.test | RUN | 0.040383 | 0.080092 | 0.150213 | 0.000947 | 0.000919 | 0.001688 |
| mp_max_element_q.test | RUN | 0.250198 | 0.916892 | 0.939749 | 0.001822 | 0.002341 | 0.002092 |
| mp_max_element_q_2.test | RUN | 0.039044 | 0.078940 | 0.148802 | 0.000707 | 0.000863 | 0.001848 |
| mp_nth_element.test | RUN | 0.275569 | 1.261792 | 1.127088 | 0.002003 | 0.002258 | 0.001818 |
| mp_nth_element_q.test | RUN | 0.273661 | 1.287937 | 1.101222 | 0.001782 | 0.002396 | 0.002111 |
| mp_back.test | RUN | 0.262343 | 1.082803 | 1.068670 | 0.002112 | 0.002265 | 0.001938 |
| mp_back_2.test | RUN | 0.037604 | 0.076705 | 0.143092 | 0.000753 | 0.000812 | 0.001961 |
| mp_pop_back.test | RUN | 0.665394 | 6.809124 | 2.102054 | 0.002225 | 0.002204 | 0.001697 |
| mp_pop_back_2.test | RUN | 0.039076 | 0.078456 | 0.139638 | 0.000878 | 0.000740 | 0.002070 |
| mp_flatten.test | RUN | 0.304744 | 2.454007 | 1.505130 | 0.002286 | 0.002326 | 0.001948 |
| mp_rotate_left.test | RUN | 0.318240 | 2.637243 | 1.494466 | 0.001801 | 0.002782 | 0.002174 |
| mp_rotate_right.test | RUN | 0.317515 | 2.634897 | 1.459798 | 0.002168 | 0.002204 | 0.002083 |
| mp_power_set.test | RUN | 0.307553 | 2.545050 | 1.450363 | 0.002127 | 0.002145 | 0.001994 |
| mp_partial_sum.test | RUN | 0.396656 | 3.366004 | 1.664784 | 0.001742 | 0.002016 | 0.001944 |
| mp_iterate.test | RUN | 0.278473 | 1.040625 | 1.180772 | 0.002103 | 0.002240 | 0.001928 |
| mp_pairwise_fold.test | RUN | 0.304618 | 2.289031 | 1.458617 | 0.002038 | 0.002260 | 0.002038 |
| mp_pairwise_fold_q.test | RUN | 0.247668 | 0.719891 | 0.876606 | 0.002169 | 0.002078 | 0.002043 |
| mp_sliding_fold.test | RUN | 0.557020 | 4.383648 | 2.677920 | 0.002070 | 0.002230 | 0.002082 |
| mp_sliding_fold_q.test | RUN | 0.546514 | 4.375874 | 2.662712 | 0.001994 | 0.001987 | 0.001718 |
| mp_intersperse.test | RUN | 0.293623 | 1.676088 | 1.322994 | 0.002176 | 0.002156 | 0.002270 |
| mp_split.test | RUN | 0.388986 | 3.184034 | 1.787324 | 0.001889 | 0.002514 | 0.001746 |
| mp_join.test | RUN | 0.337398 | 2.646612 | 1.568409 | 0.002099 | 0.002168 | 0.001708 |
| integral.test | RUN | 0.254144 | 1.225465 | 0.976671 | 0.001964 | 0.002125 | 0.002255 |
| mp_value.test | RUN | 0.016858 | 0.041553 | 0.054480 | 0.000948 | 0.001021 | 0.002293 |
| mp_identity.test | RUN | 0.244790 | 1.130270 | 1.003400 | 0.001905 | 0.002191 | 0.002009 |
| mp_inherit.test | RUN | 0.236578 | 0.801214 | 0.935622 | 0.002056 | 0.002082 | 0.001796 |
| mp_if.test | RUN | 0.235692 | 0.919129 | 0.866520 | 0.002097 | 0.002094 | 0.001974 |
| mp_if_sf.test | RUN | 0.226476 | 0.676374 | 0.787173 | 0.001931 | 0.002262 | 0.002194 |
| mp_eval_if.test | RUN | 0.244480 | 0.974572 | 0.947770 | 0.001838 | 0.001872 | 0.001954 |
| mp_eval_if_sf.test | RUN | 0.236103 | 0.719518 | 0.890539 | 0.002359 | 0.002620 | 0.002066 |
| mp_valid.test | RUN | 0.244032 | 0.704778 | 0.801878 | 0.001797 | 0.002126 | 0.002088 |
| mp_defer.test | RUN | 0.250177 | 1.204775 | 0.995118 | 0.001986 | 0.002181 | 0.001966 |
| mp_quote.test | RUN | 0.249792 | 1.184898 | 1.079741 | 0.001860 | 0.002356 | 0.001594 |
| mp_invoke_q.test | RUN | 0.273223 | 1.558987 | 1.066256 | 0.001978 | 0.002048 | 0.002028 |
| mp_invoke_q_sf.test | RUN | 0.219023 | 0.721326 | 0.846029 | 0.002170 | 0.002220 | 0.001918 |
| mp_quote_trait.test | RUN | 0.247504 | 1.306246 | 0.979404 | 0.002039 | 0.001924 | 0.002105 |
| mp_cond.test | RUN | 0.234165 | 0.786221 | 0.894808 | 0.001946 | 0.002365 | 0.001668 |
| mp_cond_sf.test | RUN | 0.218804 | 0.712166 | 0.873609 | 0.002078 | 0.001729 | 0.001880 |
| mp_not_fn.test | RUN | 0.226637 | 0.697447 | 0.810854 | 0.002168 | 0.002257 | 0.001672 |
| mp_eval_if_not.test | RUN | 0.236566 | 0.991780 | 0.924950 | 0.002634 | 0.002092 | 0.002144 |
| mp_eval_or.test | RUN | 0.219409 | 0.544671 | 0.847912 | 0.002124 | 0.002246 | 0.002187 |
| mp_compose.test | RUN | 0.272482 | 1.570050 | 1.105838 | 0.001763 | 0.002006 | 0.002077 |
| mp_valid_and_true.test | RUN | 0.229217 | 0.815361 | 0.893217 | 0.001961 | 0.002192 | 0.001675 |
| mp_compose_sf.test | COMPILE | 0.056269 | 0.102809 | 0.186628 | — | — | — |
| integer_sequence.test | RUN | 0.277141 | 1.565277 | 1.233219 | 0.001944 | 0.002118 | 0.001556 |
| tuple_for_each.test | RUN | 0.231159 | 0.625048 | 0.913672 | 0.001834 | 0.002181 | 0.002083 |
| tuple_for_each_cx.test | COMPILE | 0.044806 | 0.084544 | 0.157738 | — | — | — |
| tuple_apply.test | RUN | 0.240393 | 0.615174 | 0.966738 | 0.002026 | 0.002648 | 0.001897 |
| tuple_apply_cx.test | COMPILE | 0.065760 | 0.123559 | 0.222331 | — | — | — |
| construct_from_tuple.test | RUN | 0.233691 | 0.650753 | 0.916835 | 0.001973 | 0.001942 | 0.001916 |
| construct_from_tuple_cx.test | COMPILE | 0.064415 | 0.129350 | 0.230509 | — | — | — |
| tuple_transform.test | RUN | 0.385377 | 1.307005 | 1.802401 | 0.002126 | 0.002161 | 0.001942 |
| tuple_transform_2.test | RUN | 0.326146 | 0.960984 | 1.674632 | 0.002110 | 0.002610 | 0.001838 |
| tuple_transform_cx.test | COMPILE | 0.058615 | 0.120643 | 0.192909 | — | — | — |
| mp_set_contains.test | RUN | 0.241335 | 0.845593 | 0.916797 | 0.002305 | 0.002114 | 0.001955 |
| mp_set_push_back.test | RUN | 0.306108 | 2.536358 | 1.462922 | 0.002152 | 0.002247 | 0.002057 |
| mp_set_push_front.test | RUN | 0.390852 | 2.995797 | 1.656016 | 0.001973 | 0.002533 | 0.002189 |
| mp_is_set.test | RUN | 0.242696 | 0.837730 | 0.945175 | 0.001960 | 0.002058 | 0.001937 |
| mp_set_union.test | RUN | 0.308334 | 2.108190 | 1.430842 | 0.002077 | 0.002126 | 0.002444 |
| mp_set_union_sf.test | RUN | 0.233583 | 0.683549 | 0.865939 | 0.001746 | 0.002382 | 0.001871 |
| mp_set_difference.test | RUN | 0.275108 | 1.678390 | 1.214576 | 0.001896 | 0.002072 | 0.001892 |
| mp_set_difference_sf.test | RUN | 0.231687 | 0.706615 | 0.813803 | 0.001859 | 0.002067 | 0.002032 |
| mp_set_intersection.test | RUN | 0.273448 | 1.695073 | 1.198713 | 0.001587 | 0.002054 | 0.002033 |
| mp_set_intersection_sf.test | RUN | 0.236782 | 0.689155 | 0.835493 | 0.002075 | 0.002105 | 0.001882 |
| mp_all.test | RUN | 0.242283 | 0.818013 | 0.904895 | 0.001958 | 0.002022 | 0.002192 |
| mp_all_2.test | RUN | 0.632945 | 1.721687 | 2.003887 | 0.002127 | 0.001944 | 0.002130 |
| mp_and.test | RUN | 0.238798 | 0.843346 | 0.893303 | 0.002392 | 0.002768 | 0.002128 |
| mp_any.test | RUN | 0.239879 | 0.850038 | 0.879875 | 0.001720 | 0.002247 | 0.002000 |
| mp_any_2.test | RUN | 0.608036 | 1.715038 | 2.012102 | 0.002032 | 0.002368 | 0.001963 |
| mp_or.test | RUN | 0.234905 | 0.851884 | 0.861593 | 0.002068 | 0.001944 | 0.002482 |
| mp_same.test | RUN | 0.413911 | 1.409818 | 1.369742 | 0.001868 | 0.001906 | 0.002100 |
| mp_plus.test | RUN | 0.242865 | 0.988628 | 0.936166 | 0.002186 | 0.002068 | 0.002230 |
| mp_less.test | RUN | 0.247833 | 0.808832 | 0.879113 | 0.001726 | 0.002268 | 0.002016 |
| mp_min.test | RUN | 0.236954 | 0.762793 | 0.908316 | 0.002186 | 0.002084 | 0.002150 |
| mp_max.test | RUN | 0.235047 | 0.847882 | 0.863533 | 0.001954 | 0.002202 | 0.001891 |
| mp_similar.test | RUN | 0.234054 | 0.835690 | 0.876676 | 0.001903 | 0.001904 | 0.002139 |
| mp_map_find.test | RUN | 0.274516 | 1.513093 | 1.172489 | 0.001994 | 0.002020 | 0.002052 |
| mp_map_find_2.test | RUN | 0.255148 | 0.617576 | 0.996062 | 0.002216 | 0.001956 | 0.001820 |
| mp_map_find_3.test | RUN | 0.252868 | 0.594568 | 0.983433 | 0.002306 | 0.001825 | 0.002028 |
| mp_map_find_4.test | RUN | 0.247012 | 0.574306 | 0.944421 | 0.001855 | 0.001795 | 0.001954 |
| mp_map_contains.test | RUN | 0.252008 | 0.845080 | 0.929529 | 0.001991 | 0.002365 | 0.002012 |
| mp_map_insert.test | RUN | 0.338158 | 2.771261 | 1.578252 | 0.001947 | 0.002750 | 0.002068 |
| mp_map_replace.test | RUN | 0.413566 | 3.200101 | 1.842230 | 0.001995 | 0.002088 | 0.001850 |
| mp_map_erase.test | RUN | 0.318355 | 2.349665 | 1.559826 | 0.002170 | 0.002872 | 0.002036 |
| mp_map_update.test | RUN | 0.282844 | 1.564177 | 1.158692 | 0.002055 | 0.002106 | 0.001940 |
| mp_map_update_q.test | RUN | 0.272011 | 1.579504 | 1.200012 | 0.001932 | 0.002586 | 0.001654 |
| mp_map_keys.test | RUN | 0.267658 | 1.409305 | 1.200582 | 0.001758 | 0.002030 | 0.001912 |
| mp_is_map.test | RUN | 0.246282 | 0.884481 | 0.981264 | 0.001829 | 0.002102 | 0.002109 |
| mp_bind.test | RUN | 0.305569 | 2.467288 | 1.371989 | 0.001826 | 0.002132 | 0.002140 |
| mp_bind_q.test | RUN | 0.267902 | 1.547252 | 1.179436 | 0.001375 | 0.002152 | 0.001744 |
| mp_bind_front.test | RUN | 0.261429 | 1.678410 | 1.142069 | 0.001890 | 0.002287 | 0.002068 |
| mp_bind_back.test | RUN | 0.265799 | 1.678142 | 1.086439 | 0.001875 | 0.002002 | 0.001924 |
| mp_lambda.test | RUN | 0.568191 | 5.010507 | 2.753856 | 0.001940 | 0.001442 | 0.002015 |
| mpl.test | RUN | 0.521022 | 1.330301 | 2.262769 | 0.001987 | 0.002235 | 0.001939 |
| mpl_list.test | RUN | 0.300439 | 1.050306 | 1.091799 | 0.001842 | 0.002288 | 0.001935 |
| mpl_tuple.test | RUN | 0.303323 | 1.023400 | 1.116919 | 0.002015 | 0.001871 | 0.001949 |
| version.test | RUN | 0.154659 | 0.370602 | 0.581646 | 0.001925 | 0.001966 | 0.001808 |
| mp11_version.test | RUN | 0.188598 | 0.441427 | 0.637476 | 0.001993 | 0.002004 | 0.001889 |
| list_version.test | RUN | 0.159625 | 0.380679 | 0.562434 | 0.001824 | 0.002010 | 0.001934 |
| algorithm_version.test | RUN | 0.167473 | 0.418501 | 0.638100 | 0.002047 | 0.002111 | 0.001818 |
| integral_version.test | RUN | 0.151992 | 0.395606 | 0.562834 | 0.002155 | 0.001940 | 0.001726 |
| utility_version.test | RUN | 0.157082 | 0.392068 | 0.557484 | 0.002127 | 0.002104 | 0.001810 |
| function_version.test | RUN | 0.153549 | 0.392676 | 0.578677 | 0.002000 | 0.001726 | 0.002180 |
| map_version.test | RUN | 0.184815 | 0.412136 | 0.602418 | 0.001971 | 0.001972 | 0.002055 |
| set_version.test | RUN | 0.158793 | 0.404234 | 0.623449 | 0.002066 | 0.002090 | 0.001615 |
| bind_version.test | RUN | 0.172469 | 0.417454 | 0.667916 | 0.001904 | 0.002003 | 0.002202 |
| integer_sequence_version.test | RUN | 0.151542 | 0.384769 | 0.564528 | 0.002388 | 0.002057 | 0.001682 |
| tuple_version.test | RUN | 0.175070 | 0.423807 | 0.409811 | 0.001578 | 0.002275 | 0.001493 |
| mpl_version.test | RUN | 0.168401 | 0.276002 | 0.538440 | 0.001422 | 0.001665 | 0.001645 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.994× | 0.989×, 0.999× | 0.998× |
| clang | 1.093× | 1.093×, 1.093× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| algorithm_version.test | 10 | 0.001178s | 0.001171s | 0.001187s | byte equality observed |
| bind_version.test | 10 | 0.001186s | 0.001164s | 0.001175s | byte equality observed |
| construct_from_tuple.test | 10 | 0.001192s | 0.001183s | 0.001212s | byte equality observed |
| function_version.test | 10 | 0.001191s | 0.001181s | 0.001197s | byte equality observed |
| integer_sequence.test | 10 | 0.001186s | 0.001183s | 0.001220s | byte equality observed |
| integer_sequence_version.test | 10 | 0.001187s | 0.001180s | 0.001267s | byte equality observed |
| integral.test | 10 | 0.001202s | 0.001185s | 0.001315s | byte equality observed |
| integral_version.test | 10 | 0.001182s | 0.001169s | 0.001282s | byte equality observed |
| list_version.test | 10 | 0.001195s | 0.001197s | 0.001321s | byte equality observed |
| map_version.test | 10 | 0.001183s | 0.001189s | 0.001266s | byte equality observed |
| mp11_version.test | 10 | 0.001176s | 0.001183s | 0.001337s | byte equality observed |
| mp_all.test | 10 | 0.001172s | 0.001208s | 0.001286s | byte equality observed |
| mp_all_2.test | 10 | 0.001179s | 0.001195s | 0.001254s | byte equality observed |
| mp_all_of.test | 10 | 0.001162s | 0.001208s | 0.001292s | byte equality observed |
| mp_all_of_q.test | 10 | 0.001184s | 0.001210s | 0.001282s | byte equality observed |
| mp_and.test | 10 | 0.001193s | 0.001202s | 0.001301s | byte equality observed |
| mp_any.test | 10 | 0.001194s | 0.001184s | 0.001276s | byte equality observed |
| mp_any_2.test | 10 | 0.001200s | 0.001201s | 0.001283s | byte equality observed |
| mp_any_of.test | 10 | 0.001243s | 0.001205s | 0.001325s | byte equality observed |
| mp_any_of_q.test | 10 | 0.001237s | 0.001196s | 0.001268s | byte equality observed |
| mp_append.test | 10 | 0.001233s | 0.001193s | 0.001312s | byte equality observed |
| mp_append_2.test | 10 | 0.001305s | 0.001183s | 0.001315s | byte equality observed |
| mp_append_3.test | 10 | 0.000615s | 0.000570s | 0.001274s | byte equality observed |
| mp_append_sf.test | 10 | 0.001305s | 0.001235s | 0.001248s | byte equality observed |
| mp_apply_q.test | 10 | 0.001319s | 0.001217s | 0.001347s | byte equality observed |
| mp_apply_q_sf.test | 10 | 0.001309s | 0.001273s | 0.001259s | byte equality observed |
| mp_assign.test | 10 | 0.001287s | 0.001286s | 0.001262s | byte equality observed |
| mp_assign_2.test | 10 | 0.000613s | 0.000610s | 0.001290s | byte equality observed |
| mp_at.test | 10 | 0.001303s | 0.001325s | 0.001252s | byte equality observed |
| mp_at_2.test | 10 | 0.000599s | 0.000605s | 0.001257s | byte equality observed |
| mp_at_sf.test | 10 | 0.001299s | 0.001261s | 0.001257s | byte equality observed |
| mp_back.test | 10 | 0.001306s | 0.001273s | 0.001262s | byte equality observed |
| mp_back_2.test | 10 | 0.000608s | 0.000597s | 0.001256s | byte equality observed |
| mp_bind.test | 10 | 0.001283s | 0.001293s | 0.001283s | byte equality observed |
| mp_bind_back.test | 10 | 0.001258s | 0.001349s | 0.001348s | byte equality observed |
| mp_bind_front.test | 10 | 0.001257s | 0.001320s | 0.001325s | byte equality observed |
| mp_bind_q.test | 10 | 0.001290s | 0.001325s | 0.001275s | byte equality observed |
| mp_clear.test | 10 | 0.001306s | 0.001328s | 0.001243s | byte equality observed |
| mp_clear_2.test | 10 | 0.000601s | 0.000608s | 0.001298s | byte equality observed |
| mp_compose.test | 10 | 0.001267s | 0.001301s | 0.001314s | byte equality observed |
| mp_cond.test | 10 | 0.001346s | 0.001322s | 0.001238s | byte equality observed |
| mp_cond_sf.test | 10 | 0.001336s | 0.001297s | 0.001281s | byte equality observed |
| mp_contains.test | 10 | 0.001306s | 0.001317s | 0.001300s | byte equality observed |
| mp_copy_if.test | 10 | 0.001308s | 0.001307s | 0.001307s | byte equality observed |
| mp_copy_if_q.test | 10 | 0.001325s | 0.001326s | 0.001346s | byte equality observed |
| mp_count.test | 10 | 0.001338s | 0.001289s | 0.001289s | byte equality observed |
| mp_count_if.test | 6 | 0.001284s | 0.001291s | 0.001256s | byte equality observed |
| mp_count_if_q.test | 10 | 0.001276s | 0.001303s | 0.001251s | byte equality observed |
| mp_defer.test | 10 | 0.001332s | 0.001307s | 0.001289s | byte equality observed |
| mp_drop.test | 10 | 0.001312s | 0.001309s | 0.001283s | byte equality observed |
| mp_drop_2.test | 10 | 0.000602s | 0.000620s | 0.001266s | byte equality observed |
| mp_empty.test | 10 | 0.001288s | 0.001304s | 0.001246s | byte equality observed |
| mp_empty_2.test | 10 | 0.000612s | 0.000597s | 0.001294s | byte equality observed |
| mp_erase.test | 10 | 0.001337s | 0.001346s | 0.001272s | byte equality observed |
| mp_erase_2.test | 10 | 0.000616s | 0.000607s | 0.001265s | byte equality observed |
| mp_eval_if.test | 10 | 0.001288s | 0.001288s | 0.001262s | byte equality observed |
| mp_eval_if_not.test | 10 | 0.001302s | 0.001312s | 0.001316s | byte equality observed |
| mp_eval_if_sf.test | 10 | 0.001261s | 0.001329s | 0.001252s | byte equality observed |
| mp_eval_or.test | 10 | 0.001324s | 0.001248s | 0.001313s | byte equality observed |
| mp_fill.test | 10 | 0.001294s | 0.001286s | 0.001252s | byte equality observed |
| mp_fill_2.test | 10 | 0.000612s | 0.000602s | 0.001257s | byte equality observed |
| mp_filter.test | 10 | 0.001318s | 0.001261s | 0.001237s | byte equality observed |
| mp_find.test | 10 | 0.001274s | 0.001289s | 0.001295s | byte equality observed |
| mp_find_if.test | 10 | 0.001274s | 0.001326s | 0.001266s | byte equality observed |
| mp_find_if_q.test | 10 | 0.001315s | 0.001313s | 0.001326s | byte equality observed |
| mp_flatten.test | 10 | 0.001285s | 0.001317s | 0.001313s | byte equality observed |
| mp_fold.test | 10 | 0.001285s | 0.001298s | 0.001277s | byte equality observed |
| mp_fold_q.test | 10 | 0.001278s | 0.001314s | 0.001325s | byte equality observed |
| mp_fold_q_sf.test | 10 | 0.001324s | 0.001294s | 0.001280s | byte equality observed |
| mp_for_each.test | 10 | 0.001293s | 0.001283s | 0.001317s | byte equality observed |
| mp_from_sequence.test | 10 | 0.001301s | 0.001298s | 0.001283s | byte equality observed |
| mp_from_sequence_2.test | 10 | 0.001310s | 0.001301s | 0.001309s | byte equality observed |
| mp_from_sequence_3.test | 10 | 0.001283s | 0.001290s | 0.001336s | byte equality observed |
| mp_front.test | 10 | 0.001320s | 0.001314s | 0.001263s | byte equality observed |
| mp_front_2.test | 10 | 0.000599s | 0.000603s | 0.001223s | byte equality observed |
| mp_identity.test | 10 | 0.001289s | 0.001333s | 0.001287s | byte equality observed |
| mp_if.test | 10 | 0.001281s | 0.001302s | 0.001305s | byte equality observed |
| mp_if_sf.test | 10 | 0.001322s | 0.001263s | 0.001287s | byte equality observed |
| mp_inherit.test | 10 | 0.001311s | 0.001325s | 0.001290s | byte equality observed |
| mp_insert.test | 10 | 0.001316s | 0.001329s | 0.001262s | byte equality observed |
| mp_insert_2.test | 10 | 0.000598s | 0.000597s | 0.001317s | byte equality observed |
| mp_intersperse.test | 10 | 0.001280s | 0.001289s | 0.001262s | byte equality observed |
| mp_invoke_q.test | 10 | 0.001328s | 0.001269s | 0.001328s | byte equality observed |
| mp_invoke_q_sf.test | 10 | 0.001347s | 0.001314s | 0.001309s | byte equality observed |
| mp_iota.test | 10 | 0.001291s | 0.001247s | 0.001340s | byte equality observed |
| mp_iota_2.test | 10 | 0.001297s | 0.001319s | 0.001298s | byte equality observed |
| mp_is_list.test | 10 | 0.001293s | 0.001361s | 0.001289s | byte equality observed |
| mp_is_map.test | 10 | 0.001318s | 0.001327s | 0.001277s | byte equality observed |
| mp_is_set.test | 10 | 0.001295s | 0.001283s | 0.001261s | byte equality observed |
| mp_is_value_list.test | 10 | 0.001316s | 0.001209s | 0.001258s | byte equality observed |
| mp_iterate.test | 10 | 0.001316s | 0.001305s | 0.001285s | byte equality observed |
| mp_join.test | 10 | 0.001290s | 0.001350s | 0.001326s | byte equality observed |
| mp_lambda.test | 10 | 0.001323s | 0.001281s | 0.001249s | byte equality observed |
| mp_less.test | 10 | 0.001322s | 0.001324s | 0.001298s | byte equality observed |
| mp_list_c.test | 10 | 0.001305s | 0.001326s | 0.001271s | byte equality observed |
| mp_list_v.test | 10 | 0.000603s | 0.000615s | 0.001217s | byte equality observed |
| mp_map_contains.test | 10 | 0.001301s | 0.001290s | 0.001255s | byte equality observed |
| mp_map_erase.test | 10 | 0.001291s | 0.001299s | 0.001313s | byte equality observed |
| mp_map_find.test | 10 | 0.001327s | 0.001296s | 0.001293s | byte equality observed |
| mp_map_find_2.test | 10 | 0.001303s | 0.001277s | 0.001333s | byte equality observed |
| mp_map_find_3.test | 10 | 0.001310s | 0.001316s | 0.001312s | byte equality observed |
| mp_map_find_4.test | 10 | 0.001291s | 0.001301s | 0.001306s | byte equality observed |
| mp_map_insert.test | 10 | 0.001314s | 0.001315s | 0.001256s | byte equality observed |
| mp_map_keys.test | 10 | 0.001303s | 0.001278s | 0.001288s | byte equality observed |
| mp_map_replace.test | 10 | 0.001301s | 0.001267s | 0.001303s | byte equality observed |
| mp_map_update.test | 10 | 0.001299s | 0.001282s | 0.001283s | byte equality observed |
| mp_map_update_q.test | 10 | 0.001308s | 0.001364s | 0.001314s | byte equality observed |
| mp_max.test | 10 | 0.001310s | 0.001266s | 0.001304s | byte equality observed |
| mp_max_element.test | 10 | 0.001264s | 0.001285s | 0.001276s | byte equality observed |
| mp_max_element_2.test | 10 | 0.000616s | 0.000592s | 0.001260s | byte equality observed |
| mp_max_element_q.test | 10 | 0.001304s | 0.001318s | 0.001242s | byte equality observed |
| mp_max_element_q_2.test | 10 | 0.000587s | 0.000596s | 0.001292s | byte equality observed |
| mp_min.test | 10 | 0.001269s | 0.001327s | 0.001254s | byte equality observed |
| mp_min_element.test | 10 | 0.001280s | 0.001265s | 0.001311s | byte equality observed |
| mp_min_element_2.test | 10 | 0.000606s | 0.000617s | 0.001227s | byte equality observed |
| mp_min_element_q.test | 10 | 0.001301s | 0.001321s | 0.001235s | byte equality observed |
| mp_min_element_q_2.test | 10 | 0.000601s | 0.000597s | 0.001242s | byte equality observed |
| mp_none_of.test | 10 | 0.001252s | 0.001288s | 0.001274s | byte equality observed |
| mp_none_of_q.test | 10 | 0.001253s | 0.001294s | 0.001305s | byte equality observed |
| mp_not_fn.test | 10 | 0.001290s | 0.001274s | 0.001319s | byte equality observed |
| mp_nth_element.test | 10 | 0.001291s | 0.001260s | 0.001277s | byte equality observed |
| mp_nth_element_q.test | 10 | 0.001333s | 0.001344s | 0.001276s | byte equality observed |
| mp_or.test | 10 | 0.001270s | 0.001305s | 0.001293s | byte equality observed |
| mp_pairwise_fold.test | 10 | 0.001311s | 0.001327s | 0.001284s | byte equality observed |
| mp_pairwise_fold_q.test | 10 | 0.001290s | 0.001298s | 0.001335s | byte equality observed |
| mp_partial_sum.test | 10 | 0.001285s | 0.001286s | 0.001262s | byte equality observed |
| mp_partition.test | 10 | 0.001281s | 0.001316s | 0.001284s | byte equality observed |
| mp_partition_q.test | 10 | 0.001303s | 0.001306s | 0.001322s | byte equality observed |
| mp_plus.test | 10 | 0.001296s | 0.001342s | 0.001273s | byte equality observed |
| mp_pop_back.test | 10 | 0.001321s | 0.001329s | 0.001244s | byte equality observed |
| mp_pop_back_2.test | 10 | 0.000594s | 0.000597s | 0.001322s | byte equality observed |
| mp_pop_front.test | 10 | 0.001292s | 0.001335s | 0.001256s | byte equality observed |
| mp_pop_front_2.test | 10 | 0.000612s | 0.000615s | 0.001244s | byte equality observed |
| mp_power_set.test | 10 | 0.001275s | 0.001327s | 0.001254s | byte equality observed |
| mp_product.test | 10 | 0.001250s | 0.001317s | 0.001328s | byte equality observed |
| mp_push_back.test | 10 | 0.001285s | 0.001327s | 0.001227s | byte equality observed |
| mp_push_back_2.test | 10 | 0.000585s | 0.000598s | 0.001329s | byte equality observed |
| mp_push_front.test | 10 | 0.001307s | 0.001342s | 0.001248s | byte equality observed |
| mp_push_front_2.test | 10 | 0.000595s | 0.000600s | 0.001269s | byte equality observed |
| mp_quote.test | 10 | 0.001292s | 0.001297s | 0.001268s | byte equality observed |
| mp_quote_trait.test | 10 | 0.001292s | 0.001298s | 0.001357s | byte equality observed |
| mp_remove.test | 10 | 0.001311s | 0.001306s | 0.001306s | byte equality observed |
| mp_remove_if.test | 10 | 0.001268s | 0.001341s | 0.001277s | byte equality observed |
| mp_remove_if_q.test | 10 | 0.001317s | 0.001295s | 0.001323s | byte equality observed |
| mp_rename.test | 10 | 0.001304s | 0.001313s | 0.001287s | byte equality observed |
| mp_rename_2.test | 10 | 0.000620s | 0.000612s | 0.001275s | byte equality observed |
| mp_rename_v.test | 10 | 0.000616s | 0.000624s | 0.001285s | byte equality observed |
| mp_repeat.test | 10 | 0.001336s | 0.001313s | 0.001239s | byte equality observed |
| mp_repeat_2.test | 10 | 0.000592s | 0.000599s | 0.001308s | byte equality observed |
| mp_repeat_3.test | 10 | 0.001270s | 0.001297s | 0.001261s | byte equality observed |
| mp_replace.test | 10 | 0.001298s | 0.001288s | 0.001358s | byte equality observed |
| mp_replace_at.test | 10 | 0.001316s | 0.001305s | 0.001294s | byte equality observed |
| mp_replace_at_c.test | 10 | 0.001313s | 0.001281s | 0.001312s | byte equality observed |
| mp_replace_front.test | 10 | 0.001308s | 0.001301s | 0.001246s | byte equality observed |
| mp_replace_front_2.test | 10 | 0.000590s | 0.000594s | 0.001260s | byte equality observed |
| mp_replace_if.test | 10 | 0.001325s | 0.001266s | 0.001306s | byte equality observed |
| mp_replace_if_q.test | 10 | 0.001267s | 0.001269s | 0.001285s | byte equality observed |
| mp_replace_second.test | 10 | 0.001332s | 0.001309s | 0.001269s | byte equality observed |
| mp_replace_second_2.test | 10 | 0.000598s | 0.000605s | 0.001251s | byte equality observed |
| mp_replace_third.test | 10 | 0.001290s | 0.001308s | 0.001225s | byte equality observed |
| mp_replace_third_2.test | 10 | 0.000605s | 0.000615s | 0.001288s | byte equality observed |
| mp_reverse.test | 10 | 0.001269s | 0.001303s | 0.001259s | byte equality observed |
| mp_reverse_fold.test | 10 | 0.001314s | 0.001283s | 0.001292s | byte equality observed |
| mp_reverse_fold_q.test | 10 | 0.001275s | 0.001294s | 0.001295s | byte equality observed |
| mp_rotate_left.test | 10 | 0.001273s | 0.001318s | 0.001311s | byte equality observed |
| mp_rotate_right.test | 10 | 0.001292s | 0.001281s | 0.001272s | byte equality observed |
| mp_same.test | 10 | 0.001310s | 0.001324s | 0.001318s | byte equality observed |
| mp_second.test | 10 | 0.001283s | 0.001334s | 0.001259s | byte equality observed |
| mp_second_2.test | 10 | 0.000602s | 0.000598s | 0.001222s | byte equality observed |
| mp_set_contains.test | 10 | 0.001299s | 0.001301s | 0.001261s | byte equality observed |
| mp_set_difference.test | 10 | 0.001289s | 0.001381s | 0.001324s | byte equality observed |
| mp_set_difference_sf.test | 10 | 0.001315s | 0.001340s | 0.001336s | byte equality observed |
| mp_set_intersection.test | 10 | 0.001307s | 0.001279s | 0.001274s | byte equality observed |
| mp_set_intersection_sf.test | 10 | 0.001285s | 0.001340s | 0.001273s | byte equality observed |
| mp_set_push_back.test | 10 | 0.001310s | 0.001326s | 0.001286s | byte equality observed |
| mp_set_push_front.test | 10 | 0.001319s | 0.001304s | 0.001245s | byte equality observed |
| mp_set_union.test | 10 | 0.001259s | 0.001317s | 0.001280s | byte equality observed |
| mp_set_union_sf.test | 10 | 0.001320s | 0.001321s | 0.001254s | byte equality observed |
| mp_similar.test | 10 | 0.001328s | 0.001306s | 0.001263s | byte equality observed |
| mp_size.test | 10 | 0.001276s | 0.001324s | 0.001270s | byte equality observed |
| mp_size_2.test | 10 | 0.000597s | 0.000596s | 0.001272s | byte equality observed |
| mp_slice.test | 10 | 0.001316s | 0.001336s | 0.001234s | byte equality observed |
| mp_slice_2.test | 10 | 0.000602s | 0.000599s | 0.001248s | byte equality observed |
| mp_sliding_fold.test | 10 | 0.001347s | 0.001287s | 0.001304s | byte equality observed |
| mp_sliding_fold_q.test | 10 | 0.001321s | 0.001334s | 0.001290s | byte equality observed |
| mp_sort.test | 10 | 0.001295s | 0.001287s | 0.001306s | byte equality observed |
| mp_sort_q.test | 10 | 0.001280s | 0.001305s | 0.001289s | byte equality observed |
| mp_split.test | 10 | 0.001320s | 0.001292s | 0.001270s | byte equality observed |
| mp_starts_with.test | 10 | 0.001277s | 0.001323s | 0.001300s | byte equality observed |
| mp_starts_with_sf.test | 10 | 0.001277s | 0.001343s | 0.001260s | byte equality observed |
| mp_take.test | 10 | 0.001310s | 0.001291s | 0.001349s | byte equality observed |
| mp_take_2.test | 10 | 0.000600s | 0.000611s | 0.001254s | byte equality observed |
| mp_third.test | 10 | 0.001341s | 0.001344s | 0.001229s | byte equality observed |
| mp_third_2.test | 10 | 0.000600s | 0.000602s | 0.001237s | byte equality observed |
| mp_transform.test | 10 | 0.001266s | 0.001271s | 0.001234s | byte equality observed |
| mp_transform_2.test | 10 | 0.001260s | 0.001247s | 0.001238s | byte equality observed |
| mp_transform_front.test | 10 | 0.001311s | 0.001324s | 0.001256s | byte equality observed |
| mp_transform_front_2.test | 10 | 0.000619s | 0.000598s | 0.001226s | byte equality observed |
| mp_transform_if.test | 10 | 0.001273s | 0.001293s | 0.001250s | byte equality observed |
| mp_transform_if_q.test | 10 | 0.001308s | 0.001239s | 0.001257s | byte equality observed |
| mp_transform_q.test | 10 | 0.001317s | 0.001300s | 0.001331s | byte equality observed |
| mp_transform_q_2.test | 10 | 0.001297s | 0.001300s | 0.001238s | byte equality observed |
| mp_transform_second.test | 10 | 0.001277s | 0.001309s | 0.001237s | byte equality observed |
| mp_transform_second_2.test | 10 | 0.000615s | 0.000601s | 0.001308s | byte equality observed |
| mp_transform_sf.test | 10 | 0.001253s | 0.001295s | 0.001242s | byte equality observed |
| mp_transform_third.test | 10 | 0.001315s | 0.001286s | 0.001338s | byte equality observed |
| mp_transform_third_2.test | 10 | 0.000597s | 0.000608s | 0.001250s | byte equality observed |
| mp_unique.test | 10 | 0.001332s | 0.001280s | 0.001258s | byte equality observed |
| mp_unique_if.test | 10 | 0.001289s | 0.001319s | 0.001283s | byte equality observed |
| mp_unique_if_q.test | 10 | 0.001286s | 0.001336s | 0.001269s | byte equality observed |
| mp_valid.test | 10 | 0.001320s | 0.001283s | 0.001249s | byte equality observed |
| mp_valid_and_true.test | 10 | 0.001310s | 0.001291s | 0.001268s | byte equality observed |
| mp_value.test | 10 | 0.000602s | 0.000597s | 0.001238s | byte equality observed |
| mp_with_index.test | 10 | 0.001688s | 0.001695s | 0.001583s | byte equality observed |
| mp_with_index_cx.test | 10 | 0.000593s | 0.000591s | 0.001243s | byte equality observed |
| mpl.test | 10 | 0.001273s | 0.001275s | 0.001293s | byte equality observed |
| mpl_list.test | 10 | 0.001293s | 0.001310s | 0.001273s | byte equality observed |
| mpl_tuple.test | 10 | 0.001259s | 0.001306s | 0.001255s | byte equality observed |
| mpl_version.test | 10 | 0.001253s | 0.001296s | 0.001311s | byte equality observed |
| set_version.test | 10 | 0.001272s | 0.001256s | 0.001302s | byte equality observed |
| tuple_apply.test | 10 | 0.001336s | 0.001322s | 0.001298s | byte equality observed |
| tuple_for_each.test | 10 | 0.001290s | 0.001348s | 0.001263s | byte equality observed |
| tuple_transform.test | 10 | 0.001329s | 0.001295s | 0.001274s | byte equality observed |
| tuple_transform_2.test | 10 | 0.001298s | 0.001330s | 0.001294s | byte equality observed |
| tuple_version.test | 10 | 0.001302s | 0.001317s | 0.001284s | byte equality observed |
| utility_version.test | 10 | 0.001317s | 0.001303s | 0.001308s | byte equality observed |
| version.test | 10 | 0.001289s | 0.001291s | 0.001315s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
