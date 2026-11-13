# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.461974 | 1.016673 | 1.601624 | 0.001869 | 0.001738 | 0.002154 |
| any.test | RUN | 0.456113 | 0.977978 | 1.495830 | 0.001618 | 0.001623 | 0.001859 |
| clear.test | RUN | 0.323248 | 0.865046 | 1.224217 | 0.001771 | 0.001784 | 0.002112 |
| copy.test | RUN | 0.280247 | 0.648374 | 0.976817 | 0.001820 | 0.002056 | 0.002278 |
| count.test | RUN | 0.316780 | 0.717192 | 1.088320 | 0.002102 | 0.002445 | 0.001717 |
| count_if.test | RUN | 0.298541 | 0.668536 | 1.002776 | 0.002024 | 0.001956 | 0.001778 |
| erase.test | RUN | 0.428170 | 1.194020 | 1.608806 | 0.001824 | 0.002114 | 0.001531 |
| erase_key.test | RUN | 0.401482 | 1.167460 | 1.485486 | 0.001820 | 0.001916 | 0.001756 |
| filter.test | RUN | 0.366151 | 0.990984 | 1.389157 | 0.002682 | 0.002194 | 0.002040 |
| filter_if.test | RUN | 0.415575 | 1.167097 | 1.591877 | 0.001951 | 0.001881 | 0.002118 |
| find.test | RUN | 0.362301 | 0.804653 | 1.227211 | 0.001962 | 0.001815 | 0.001702 |
| find_if.test | RUN | 0.347667 | 0.765089 | 1.169844 | 0.001800 | 0.001812 | 0.001811 |
| fold.test | RUN | 0.374463 | 0.928615 | 1.398731 | 0.002081 | 0.002146 | 0.001811 |
| for_each.test | RUN | 0.327870 | 0.831911 | 1.221124 | 0.001926 | 0.002216 | 0.002109 |
| insert.test | RUN | 0.485917 | 1.477957 | 1.909542 | 0.002112 | 0.002044 | 0.001902 |
| insert_range.test | RUN | 0.479024 | 1.494084 | 1.959064 | 0.001762 | 0.001845 | 0.002080 |
| iter_fold.test | RUN | 0.573364 | 1.445590 | 2.290316 | 0.001938 | 0.002179 | 0.002102 |
| move.test | RUN | 0.288505 | 0.694906 | 1.021986 | 0.002047 | 0.001606 | 0.001858 |
| none.test | RUN | 0.462930 | 0.979075 | 1.611095 | 0.001695 | 0.001855 | 0.001718 |
| pop_back.test | RUN | 0.443923 | 1.300310 | 1.734977 | 0.001849 | 0.001990 | 0.002050 |
| pop_front.test | RUN | 0.375876 | 1.032067 | 1.397020 | 0.002030 | 0.001982 | 0.002097 |
| push_back.test | RUN | 0.474490 | 1.555786 | 1.861212 | 0.001816 | 0.002127 | 0.002068 |
| push_front.test | RUN | 0.420901 | 1.273136 | 1.599005 | 0.001920 | 0.002060 | 0.002192 |
| remove.test | RUN | 0.470491 | 1.433970 | 1.864932 | 0.001762 | 0.001884 | 0.002076 |
| remove_if.test | RUN | 0.420310 | 1.223406 | 1.637908 | 0.002134 | 0.002020 | 0.002271 |
| replace.test | RUN | 0.403353 | 1.118432 | 1.494512 | 0.001838 | 0.002010 | 0.002139 |
| replace_if.test | RUN | 0.395690 | 1.145710 | 1.516280 | 0.002220 | 0.002006 | 0.002149 |
| reverse_fold.test | RUN | 0.626983 | 1.574512 | 2.574356 | 0.002034 | 0.001854 | 0.001822 |
| reverse_iter_fold.test | RUN | 0.650726 | 1.619791 | 2.645288 | 0.001897 | 0.001866 | 0.001881 |
| reverse.test | RUN | 0.403502 | 1.154287 | 1.548106 | 0.001698 | 0.002214 | 0.001929 |
| segmented_for_each.test | RUN | 0.699707 | 2.213654 | 3.063187 | 0.001974 | 0.001909 | 0.001825 |
| segmented_find.test | RUN | 0.440045 | 1.164617 | 1.754981 | 0.001754 | 0.001963 | 0.001684 |
| segmented_find_if.test | RUN | 0.431804 | 1.162168 | 1.738236 | 0.001928 | 0.001692 | 0.001528 |
| segmented_fold.test | RUN | 0.539546 | 1.548087 | 2.192672 | 0.002171 | 0.001960 | 0.002008 |
| transform.test | RUN | 0.413064 | 1.189478 | 1.613760 | 0.001538 | 0.001938 | 0.002022 |
| join.test | RUN | 0.318844 | 0.764971 | 1.153237 | 0.001926 | 0.001974 | 0.002024 |
| zip.test | RUN | 0.562807 | 1.436752 | 2.059636 | 0.002001 | 0.002160 | 0.002000 |
| zip2.test | RUN | 0.532937 | 1.409933 | 2.061146 | 0.002038 | 0.002137 | 0.002091 |
| zip_ignore.test | RUN | 0.414162 | 0.924320 | 1.379707 | 0.001931 | 0.001876 | 0.001704 |
| flatten.test | RUN | 0.391750 | 1.046554 | 1.420766 | 0.001848 | 0.001954 | 0.002042 |
| ticket-5490.test | COMPILE | 0.200478 | 0.448919 | 0.601172 | — | — | — |
| as_deque.test | RUN | 0.383562 | 0.998629 | 1.469854 | 0.001814 | 0.002182 | 0.002076 |
| as_list.test | RUN | 0.415558 | 1.208636 | 1.619415 | 0.002103 | 0.002120 | 0.002150 |
| as_map.test | RUN | 0.406896 | 1.206355 | 1.615093 | 0.001701 | 0.001743 | 0.001970 |
| as_map_assoc.test | RUN | 0.419035 | 1.136993 | 1.517968 | 0.002082 | 0.001901 | 0.002041 |
| as_set.test | RUN | 0.471559 | 1.353579 | 1.861780 | 0.001892 | 0.001813 | 0.001776 |
| as_vector.test | RUN | 0.430203 | 1.227009 | 1.611877 | 0.001748 | 0.001976 | 0.001465 |
| boost_tuple.test | RUN | 0.420716 | 1.216408 | 1.630844 | 0.001741 | 0.001766 | 0.001735 |
| boost_tuple_iterator.test | RUN | 0.291974 | 0.749167 | 1.000673 | 0.001716 | 0.001731 | 0.001758 |
| cons.test | RUN | 0.529036 | 1.368752 | 1.936588 | 0.001788 | 0.002081 | 0.002106 |
| convert_boost_tuple.test | RUN | 0.340241 | 1.054158 | 1.267797 | 0.001775 | 0.001948 | 0.001692 |
| convert_deque.test | RUN | 0.337382 | 1.025392 | 1.261096 | 0.001831 | 0.001912 | 0.001744 |
| convert_list.test | RUN | 0.340198 | 1.056380 | 1.265896 | 0.001766 | 0.001642 | 0.001895 |
| convert_std_pair.test | RUN | 0.352092 | 1.051075 | 1.298914 | 0.001776 | 0.001852 | 0.001668 |
| convert_std_tuple.test | RUN | 0.328645 | 1.032493 | 1.275739 | 0.001838 | 0.001883 | 0.001823 |
| convert_vector.test | RUN | 0.329646 | 1.051232 | 1.254806 | 0.001692 | 0.001728 | 0.001898 |
| filter_view.test | RUN | 0.408109 | 1.091687 | 1.539907 | 0.002036 | 0.001840 | 0.002403 |
| hash.test | RUN | 0.396182 | 0.987437 | 1.523429 | 0.001757 | 0.002136 | 0.001942 |
| io.test | RUN | 0.420453 | 1.367538 | 1.743756 | 0.001898 | 0.002275 | 0.002538 |
| iterator_range.test | RUN | 0.375803 | 1.085719 | 1.424334 | 0.001547 | 0.002373 | 0.001902 |
| joint_view.test | RUN | 0.518953 | 1.753535 | 2.131638 | 0.002062 | 0.001977 | 0.001754 |
| list_comparison.test | RUN | 0.289021 | 0.730900 | 1.097113 | 0.001767 | 0.001896 | 0.001700 |
| list_construction.test | RUN | 0.285432 | 0.715958 | 0.997452 | 0.001835 | 0.001915 | 0.001839 |
| list_copy.test | RUN | 0.450567 | 1.291521 | 1.765238 | 0.001914 | 0.001853 | 0.002050 |
| list_iterator.test | RUN | 0.299741 | 0.712080 | 1.012953 | 0.002013 | 0.002012 | 0.002125 |
| list_hash.test | RUN | 0.411678 | 1.371622 | 1.486165 | 0.001625 | 0.002144 | 0.001896 |
| list_make.test | RUN | 0.311769 | 0.792550 | 1.129081 | 0.001875 | 0.002062 | 0.002032 |
| list_misc.test | RUN | 0.395295 | 0.927478 | 1.330461 | 0.001802 | 0.001930 | 0.002222 |
| list_mutate.test | RUN | 0.273695 | 0.620892 | 0.902631 | 0.001856 | 0.001894 | 0.002168 |
| list_nest.test | RUN | 0.576951 | 1.748584 | 2.450110 | 0.001578 | 0.001916 | 0.001966 |
| list_tie.test | RUN | 0.303922 | 0.727804 | 0.997324 | 0.001980 | 0.002160 | 0.002068 |
| list_value_at.test | RUN | 0.300045 | 0.668414 | 0.955157 | 0.001868 | 0.002123 | 0.002058 |
| deque_comparison.test | RUN | 0.306352 | 0.740088 | 1.108681 | 0.001974 | 0.002122 | 0.001820 |
| deque_construction.test | RUN | 0.301945 | 0.700913 | 1.038525 | 0.001846 | 0.002126 | 0.002138 |
| deque_copy.test | RUN | 0.442013 | 1.213213 | 1.629763 | 0.001696 | 0.001911 | 0.001972 |
| deque_iterator.test | RUN | 0.294889 | 0.744775 | 1.028001 | 0.001918 | 0.001701 | 0.001934 |
| deque_hash.test | RUN | 0.396773 | 1.365693 | 1.528451 | 0.001778 | 0.001892 | 0.002124 |
| deque_is_constructible.test | COMPILE | 0.161456 | 0.329718 | 0.472682 | — | — | — |
| deque_make.test | RUN | 0.304500 | 0.712467 | 1.056117 | 0.002009 | 0.002016 | 0.002066 |
| deque_misc.test | RUN | 0.392946 | 0.911985 | 1.344615 | 0.001777 | 0.001774 | 0.001970 |
| deque_move.test | RUN | 0.305482 | 0.788179 | 0.980653 | 0.001831 | 0.001942 | 0.002166 |
| deque_mutate.test | RUN | 0.274143 | 0.628743 | 0.912778 | 0.001823 | 0.002116 | 0.002151 |
| deque_nest.test | RUN | 0.833585 | 2.758141 | 3.861138 | 0.001875 | 0.001966 | 0.002079 |
| deque_tie.test | RUN | 0.280719 | 0.658691 | 0.967839 | 0.002073 | 0.001802 | 0.002072 |
| deque_value_at.test | RUN | 0.275356 | 0.667299 | 0.999975 | 0.001749 | 0.001922 | 0.001827 |
| front_extended_deque.test | RUN | 0.424361 | 1.006352 | 1.375197 | 0.001950 | 0.002075 | 0.002372 |
| back_extended_deque.test | RUN | 0.399605 | 0.955395 | 1.439596 | 0.001769 | 0.001778 | 0.002150 |
| make_list.test | RUN | 0.306000 | 0.783015 | 1.104070 | 0.002000 | 0.001774 | 0.002051 |
| make_vector.test | RUN | 0.315477 | 0.785856 | 1.052038 | 0.001936 | 0.002181 | 0.001892 |
| map.test | RUN | 0.420270 | 1.151351 | 1.564527 | 0.001794 | 0.001961 | 0.002042 |
| map_comparison.test | RUN | 0.304789 | 0.755405 | 1.062309 | 0.001868 | 0.002084 | 0.002140 |
| map_construction.test | RUN | 0.313820 | 0.753797 | 1.063917 | 0.001858 | 0.002090 | 0.001800 |
| map_copy.test | RUN | 0.377950 | 0.941883 | 1.293906 | 0.001909 | 0.002207 | 0.002104 |
| map_misc.test | RUN | 0.399886 | 0.937050 | 1.323512 | 0.001868 | 0.001920 | 0.001632 |
| map_move.test | RUN | 0.306858 | 0.822270 | 1.059991 | 0.001838 | 0.002152 | 0.001939 |
| map_mutate.test | RUN | 0.280401 | 0.639176 | 0.924167 | 0.001888 | 0.002000 | 0.001542 |
| map_tie.test | RUN | 0.288113 | 0.629781 | 0.929662 | 0.001903 | 0.001895 | 0.002066 |
| nil.test | RUN | 0.197376 | 0.454043 | 0.663930 | 0.002124 | 0.001724 | 0.002062 |
| nview.test | RUN | 0.383542 | 0.935938 | 1.385384 | 0.001998 | 0.002170 | 0.002110 |
| reverse_view.test | RUN | 0.482705 | 1.472216 | 1.921494 | 0.002176 | 0.001846 | 0.002135 |
| segmented_iterator_range.test | RUN | 0.596828 | 1.773553 | 2.433889 | 0.002072 | 0.002042 | 0.002145 |
| set.test | RUN | 0.374288 | 1.018848 | 1.389768 | 0.001740 | 0.002043 | 0.002141 |
| single_view.test | RUN | 0.345036 | 0.938272 | 1.295480 | 0.001977 | 0.002257 | 0.001926 |
| std_pair.test | RUN | 0.430980 | 1.160659 | 1.539258 | 0.002069 | 0.002162 | 0.001855 |
| boost_array.test | RUN | 0.369988 | 0.823698 | 1.195857 | 0.001858 | 0.002030 | 0.001657 |
| array.test | RUN | 0.356778 | 0.804040 | 1.227409 | 0.002075 | 0.001682 | 0.001885 |
| std_array.test | RUN | 0.365766 | 0.803287 | 1.196949 | 0.001898 | 0.002184 | 0.001863 |
| tuple_comparison.test | RUN | 0.323807 | 0.786984 | 1.141481 | 0.001967 | 0.002001 | 0.001804 |
| tuple_construction.test | RUN | 0.328921 | 0.758676 | 1.135523 | 0.002036 | 0.001910 | 0.001932 |
| tuple_conversion.test | RUN | 0.740952 | 2.380485 | 3.571643 | 0.001801 | 0.001822 | 0.001804 |
| tuple_copy.test | RUN | 0.483663 | 1.423868 | 1.971807 | 0.001994 | 0.002178 | 0.002032 |
| tuple_element.test | RUN | 0.315713 | 0.681775 | 1.018943 | 0.001723 | 0.002202 | 0.001684 |
| tuple_make.test | RUN | 0.324136 | 0.824739 | 1.129950 | 0.001989 | 0.001745 | 0.001656 |
| tuple_misc.test | RUN | 0.415905 | 0.975059 | 1.427189 | 0.001857 | 0.001741 | 0.001905 |
| tuple_mutate.test | RUN | 0.294190 | 0.704051 | 1.053101 | 0.001949 | 0.001644 | 0.001895 |
| tuple_nest.test | RUN | 1.208365 | 4.598919 | 6.530860 | 0.001968 | 0.001885 | 0.001624 |
| tuple_hash.test | RUN | 0.418336 | 1.342568 | 1.525871 | 0.001958 | 0.002123 | 0.001918 |
| tuple_tie.test | RUN | 0.312191 | 0.765655 | 1.112465 | 0.001766 | 0.002125 | 0.002052 |
| tuple_traits__maybe_variadic.test | RUN | 0.349856 | 0.897842 | 1.222456 | 0.002014 | 0.001708 | 0.002125 |
| tuple_traits__no_variadic.test | RUN | 0.364524 | 0.934514 | 1.217798 | 0.001622 | 0.002206 | 0.002084 |
| transform_view.test | RUN | 0.500375 | 1.476784 | 2.008297 | 0.001844 | 0.002082 | 0.001974 |
| identity_view.test | RUN | 0.471703 | 1.411609 | 1.818939 | 0.001694 | 0.002190 | 0.002132 |
| vector_comparison.test | RUN | 0.298513 | 0.807631 | 1.041861 | 0.001887 | 0.001873 | 0.002086 |
| vector_construction.test | RUN | 0.307073 | 0.699672 | 1.112079 | 0.001620 | 0.002006 | 0.001844 |
| vector_conversion.test | RUN | 0.877259 | 3.070056 | 4.560481 | 0.002062 | 0.002178 | 0.001746 |
| vector_copy.test | RUN | 0.461351 | 1.353318 | 1.900668 | 0.001614 | 0.001765 | 0.001796 |
| vector_iterator.test | RUN | 0.297231 | 0.784261 | 1.014031 | 0.001889 | 0.001824 | 0.001790 |
| vector_make.test | RUN | 0.320552 | 0.820738 | 1.104846 | 0.001867 | 0.001758 | 0.002070 |
| vector_misc.test | RUN | 0.395286 | 0.949083 | 1.352231 | 0.001923 | 0.001910 | 0.002045 |
| vector_move.test | RUN | 0.303564 | 0.759567 | 1.073690 | 0.001752 | 0.001926 | 0.001760 |
| vector_mutate.test | RUN | 0.274011 | 0.683656 | 0.918322 | 0.001747 | 0.001984 | 0.001884 |
| vector_n.test | RUN | 0.343109 | 0.792057 | 1.290175 | 0.002009 | 0.002141 | 0.002168 |
| vector_nest.test | RUN | 1.222824 | 4.721937 | 6.613147 | 0.002077 | 0.002086 | 0.001946 |
| vector_hash.test | RUN | 0.402826 | 1.378223 | 1.468444 | 0.001794 | 0.002238 | 0.002139 |
| vector_tie.test | RUN | 0.291053 | 0.683779 | 1.040229 | 0.001934 | 0.002190 | 0.002106 |
| vector_traits__maybe_variadic.test | RUN | 0.313593 | 0.839966 | 1.115772 | 0.001935 | 0.002140 | 0.002090 |
| vector_traits__no_variadic.test | RUN | 0.353156 | 0.867473 | 1.127744 | 0.001821 | 0.001952 | 0.002104 |
| vector_value_at.test | RUN | 0.282846 | 0.621046 | 0.975536 | 0.001917 | 0.002046 | 0.002002 |
| zip_view.test | RUN | 0.592522 | 1.568810 | 2.359275 | 0.001798 | 0.002026 | 0.002226 |
| zip_view2.test | RUN | 0.462337 | 1.262859 | 1.872280 | 0.001788 | 0.001910 | 0.001835 |
| zip_view_ignore.test | RUN | 0.445461 | 1.173897 | 1.624597 | 0.001939 | 0.001856 | 0.002031 |
| repetitive_view.test | RUN | 0.764222 | 2.001971 | 3.125193 | 0.001742 | 0.002034 | 0.002274 |
| deduce_sequence.test | RUN | 0.356445 | 0.863545 | 1.278649 | 0.001792 | 0.002030 | 0.001814 |
| adapt_adt_named.test | RUN | 0.426277 | 1.169324 | 1.550032 | 0.001774 | 0.002118 | 0.002109 |
| adapt_adt_named_empty.test | RUN | 0.377880 | 0.972748 | 1.300724 | 0.001926 | 0.002140 | 0.002037 |
| adapt_adt.test | RUN | 0.468981 | 1.575977 | 1.909280 | 0.001827 | 0.002004 | 0.002299 |
| adapt_adt_empty.test | RUN | 0.366559 | 0.943949 | 1.295363 | 0.002130 | 0.002178 | 0.001876 |
| adapt_assoc_adt_named.test | RUN | 0.469939 | 1.259496 | 1.705102 | 0.001820 | 0.002152 | 0.002184 |
| adapt_assoc_adt_named_empty.test | RUN | 0.368701 | 0.986868 | 1.273523 | 0.001688 | 0.001847 | 0.001747 |
| adapt_assoc_adt.test | RUN | 0.490703 | 1.277933 | 1.777464 | 0.001803 | 0.001989 | 0.002066 |
| adapt_assoc_adt_empty.test | RUN | 0.373220 | 0.945901 | 1.289203 | 0.001841 | 0.001940 | 0.002035 |
| adapt_assoc_struct_named.test | RUN | 0.471252 | 1.266074 | 1.678630 | 0.001910 | 0.002146 | 0.002103 |
| adapt_assoc_struct_named_empty.test | RUN | 0.378498 | 0.983186 | 1.290678 | 0.001664 | 0.002050 | 0.002066 |
| adapt_assoc_struct.test | RUN | 0.426591 | 1.130248 | 1.514977 | 0.001920 | 0.002050 | 0.001895 |
| adapt_assoc_struct_empty.test | RUN | 0.361135 | 0.927032 | 1.324174 | 0.001752 | 0.001982 | 0.001897 |
| adapt_assoc_tpl_adt.test | RUN | 0.511435 | 1.342556 | 1.840086 | 0.002014 | 0.001951 | 0.001823 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.371896 | 0.975804 | 1.281623 | 0.001912 | 0.002018 | 0.002229 |
| adapt_assoc_tpl_struct.test | RUN | 0.415797 | 1.170213 | 1.510757 | 0.002176 | 0.002018 | 0.002272 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.363369 | 0.985185 | 1.304936 | 0.001918 | 0.002220 | 0.002241 |
| adapt_struct_named.test | RUN | 0.412052 | 1.139862 | 1.540734 | 0.002076 | 0.001936 | 0.001959 |
| adapt_struct_named_empty.test | RUN | 0.382777 | 0.958472 | 1.292147 | 0.001948 | 0.001748 | 0.001960 |
| adapt_struct.test | RUN | 0.450312 | 1.347490 | 1.745301 | 0.001844 | 0.001962 | 0.002014 |
| adapt_struct_empty.test | RUN | 0.361842 | 0.997216 | 1.284216 | 0.001990 | 0.002139 | 0.002236 |
| adapt_tpl_adt.test | RUN | 0.463687 | 1.515919 | 1.826127 | 0.001858 | 0.002218 | 0.001719 |
| adapt_tpl_adt_empty.test | RUN | 0.377578 | 0.976754 | 1.305261 | 0.001717 | 0.002094 | 0.001735 |
| adapt_tpl_struct.test | RUN | 0.386487 | 1.063160 | 1.413910 | 0.002005 | 0.002012 | 0.001850 |
| adapt_tpl_struct_empty.test | RUN | 0.365878 | 1.012664 | 1.324876 | 0.001844 | 0.002004 | 0.002060 |
| adt_attribute_proxy.test | RUN | 0.425602 | 0.942049 | 1.364176 | 0.001870 | 0.002100 | 0.002110 |
| define_struct.test | RUN | 0.511508 | 1.328404 | 1.813102 | 0.001904 | 0.001942 | 0.002032 |
| define_struct_empty.test | RUN | 0.440975 | 1.085560 | 1.524308 | 0.002106 | 0.001716 | 0.001818 |
| define_struct_move.test | RUN | 0.288277 | 0.634002 | 0.897807 | 0.001892 | 0.002022 | 0.002156 |
| define_struct_inline.test | RUN | 0.540034 | 1.600097 | 2.006481 | 0.002140 | 0.001798 | 0.002182 |
| define_struct_inline_empty.test | RUN | 0.440124 | 1.101322 | 1.495717 | 0.001844 | 0.001777 | 0.002090 |
| define_struct_inline_move.test | RUN | 0.294395 | 0.656023 | 0.906752 | 0.001968 | 0.002088 | 0.001785 |
| define_assoc_struct.test | RUN | 0.493295 | 1.278375 | 1.738010 | 0.002106 | 0.002184 | 0.002093 |
| define_assoc_struct_empty.test | RUN | 0.435334 | 1.076045 | 1.481573 | 0.002005 | 0.002144 | 0.002161 |
| define_assoc_struct_move.test | RUN | 0.287021 | 0.593981 | 0.935962 | 0.001948 | 0.002037 | 0.002040 |
| define_tpl_struct.test | RUN | 0.477391 | 1.270941 | 1.737706 | 0.002009 | 0.001928 | 0.001673 |
| define_tpl_struct_empty.test | RUN | 0.438221 | 1.100069 | 1.470966 | 0.001738 | 0.001992 | 0.002122 |
| define_tpl_struct_move.test | RUN | 0.285382 | 0.646414 | 0.911316 | 0.001731 | 0.001893 | 0.001873 |
| define_tpl_struct_inline.test | RUN | 0.516493 | 1.488168 | 1.902654 | 0.001760 | 0.002038 | 0.001807 |
| define_tpl_struct_inline_empty.test | RUN | 0.444175 | 1.081328 | 1.511254 | 0.001829 | 0.001948 | 0.002092 |
| define_tpl_struct_inline_move.test | RUN | 0.283419 | 0.621230 | 0.939098 | 0.002138 | 0.002010 | 0.001806 |
| define_assoc_tpl_struct.test | RUN | 0.487233 | 1.308554 | 1.707285 | 0.002080 | 0.002175 | 0.002055 |
| define_assoc_tpl_struct_empty.test | RUN | 0.443845 | 1.088051 | 1.518746 | 0.001740 | 0.001688 | 0.002077 |
| define_assoc_tpl_struct_move.test | RUN | 0.284967 | 0.654678 | 0.924080 | 0.001960 | 0.001938 | 0.001903 |
| std_tuple.test | RUN | 0.295424 | 0.689926 | 1.003676 | 0.001996 | 0.002094 | 0.002038 |
| std_tuple_iterator.test | RUN | 0.294702 | 0.760041 | 0.999583 | 0.002021 | 0.001750 | 0.002086 |
| ref_vector.test | RUN | 0.339638 | 0.758842 | 1.102012 | 0.001956 | 0.002064 | 0.002050 |
| flatten_view.test | RUN | 0.352417 | 0.929300 | 1.288084 | 0.002077 | 0.001785 | 0.002060 |
| github-159.test | COMPILE | 0.157848 | 0.348803 | 0.446484 | — | — | — |
| github-176.test | RUN | 0.340036 | 0.822794 | 1.130190 | 0.001966 | 0.002102 | 0.001968 |
| size.test | COMPILE | 0.283970 | 0.590026 | 0.912631 | — | — | — |
| fused.test | RUN | 0.395472 | 0.881279 | 1.281128 | 0.001837 | 0.002160 | 0.002114 |
| fused_function_object.test | RUN | 0.289564 | 0.664961 | 0.995324 | 0.001870 | 0.001842 | 0.002144 |
| fused_procedure.test | RUN | 0.388850 | 0.852293 | 1.207686 | 0.001611 | 0.001928 | 0.001882 |
| unfused.test | RUN | 0.312065 | 0.696358 | 0.997806 | 0.001848 | 0.002143 | 0.001530 |
| unfused_typed.test | RUN | 0.310052 | 0.737888 | 1.062785 | 0.001898 | 0.002144 | 0.002114 |
| make_fused.test | RUN | 0.402818 | 0.893041 | 1.360351 | 0.001940 | 0.001728 | 0.001780 |
| make_fused_function_object.test | RUN | 0.295886 | 0.688467 | 1.003161 | 0.001994 | 0.001868 | 0.001782 |
| make_fused_procedure.test | RUN | 0.394102 | 0.850014 | 1.220377 | 0.001968 | 0.002102 | 0.002184 |
| make_unfused.test | RUN | 0.300476 | 0.697295 | 0.995481 | 0.001920 | 0.001950 | 0.002056 |
| invoke.test | RUN | 0.771105 | 2.441852 | 3.440219 | 0.001671 | 0.001764 | 0.001666 |
| invoke_function_object.test | RUN | 0.366538 | 0.882846 | 1.262598 | 0.001889 | 0.002160 | 0.001919 |
| invoke_procedure.test | RUN | 0.607497 | 1.807685 | 2.603575 | 0.001741 | 0.002120 | 0.002136 |
| swap.test | RUN | 0.460487 | 1.156006 | 1.546462 | 0.001935 | 0.001977 | 0.002114 |
| is_sequence.test | COMPILE | 0.077708 | 0.166029 | 0.202782 | — | — | — |
| is_view.test | COMPILE | 0.075875 | 0.157608 | 0.205951 | — | — | — |
| pair_deque.test | COMPILE | 0.163590 | 0.382583 | 0.466702 | — | — | — |
| pair_list.test | COMPILE | 0.162886 | 0.355034 | 0.476011 | — | — | — |
| pair_map.test | COMPILE | 0.170089 | 0.366275 | 0.501925 | — | — | — |
| pair_set.test | COMPILE | 0.170686 | 0.366706 | 0.507152 | — | — | — |
| pair_vector.test | COMPILE | 0.165149 | 0.366979 | 0.481032 | — | — | — |
| pair_nest.test | COMPILE | 0.068884 | 0.144057 | 0.184684 | — | — | — |
| index_sequence.test | COMPILE | 0.057991 | 0.110472 | 0.137628 | — | — | — |
| and.test | COMPILE | 0.046606 | 0.080530 | 0.096790 | — | — | — |
| tag_of.test | COMPILE | 0.077940 | 0.155241 | 0.187712 | — | — | — |
| unused.test | COMPILE | 0.041013 | 0.099850 | 0.112759 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.067991 | 0.148435 | 0.143557 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.051796 | 0.097763 | 0.142051 | — | — | — |
| quick.test | RUN | 0.205560 | 0.477492 | 0.581459 | 0.000714 | 0.001513 | 0.001460 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.001× | 1.002×, 1.000× | 0.999× |
| clang | 0.997× | 0.998×, 0.996× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001282s | 0.001278s | 0.001284s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001289s | 0.001296s | 0.001337s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001264s | 0.001265s | 0.001274s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001280s | 0.001285s | 0.001316s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001249s | 0.001301s | 0.001313s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001244s | 0.001311s | 0.001236s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001278s | 0.001301s | 0.001207s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001282s | 0.001354s | 0.001221s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001259s | 0.001276s | 0.001247s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001274s | 0.001298s | 0.001244s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001294s | 0.001306s | 0.001277s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001338s | 0.001267s | 0.001276s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001293s | 0.001236s | 0.001331s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001294s | 0.001325s | 0.001225s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001289s | 0.001311s | 0.001263s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001312s | 0.001253s | 0.001257s | byte equality observed |
| adapt_struct.test | 10 | 0.001313s | 0.001254s | 0.001295s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001284s | 0.001243s | 0.001287s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001320s | 0.001306s | 0.001289s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001295s | 0.001280s | 0.001338s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001268s | 0.001324s | 0.001323s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001247s | 0.001324s | 0.001307s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001293s | 0.001279s | 0.001289s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001270s | 0.001290s | 0.001276s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001281s | 0.001331s | 0.001287s | byte equality observed |
| all.test | 10 | 0.001269s | 0.001239s | 0.001212s | byte equality observed |
| any.test | 10 | 0.001246s | 0.001287s | 0.001188s | byte equality observed |
| array.test | 10 | 0.001262s | 0.001293s | 0.001236s | byte equality observed |
| as_deque.test | 10 | 0.001267s | 0.001288s | 0.001249s | byte equality observed |
| as_list.test | 10 | 0.001252s | 0.001304s | 0.001276s | byte equality observed |
| as_map.test | 10 | 0.001270s | 0.001304s | 0.001294s | byte equality observed |
| as_map_assoc.test | 10 | 0.001243s | 0.001278s | 0.001275s | byte equality observed |
| as_set.test | 10 | 0.001274s | 0.001307s | 0.001340s | byte equality observed |
| as_vector.test | 10 | 0.001287s | 0.001270s | 0.001301s | byte equality observed |
| back_extended_deque.test | 10 | 0.001248s | 0.001247s | 0.001280s | byte equality observed |
| boost_array.test | 10 | 0.001254s | 0.001226s | 0.001227s | byte equality observed |
| boost_tuple.test | 10 | 0.001270s | 0.001293s | 0.001259s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001260s | 0.001292s | 0.001273s | byte equality observed |
| clear.test | 10 | 0.001262s | 0.001273s | 0.001235s | byte equality observed |
| cons.test | 10 | 0.001321s | 0.001272s | 0.001234s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001252s | 0.001251s | 0.001231s | byte equality observed |
| convert_deque.test | 10 | 0.001264s | 0.001223s | 0.001304s | byte equality observed |
| convert_list.test | 10 | 0.001259s | 0.001202s | 0.001250s | byte equality observed |
| convert_std_pair.test | 10 | 0.001215s | 0.001215s | 0.001240s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001228s | 0.001257s | 0.001266s | byte equality observed |
| convert_vector.test | 10 | 0.001232s | 0.001284s | 0.001278s | byte equality observed |
| copy.test | 10 | 0.001279s | 0.001250s | 0.001311s | byte equality observed |
| count.test | 10 | 0.001252s | 0.001230s | 0.001254s | byte equality observed |
| count_if.test | 10 | 0.001267s | 0.001236s | 0.001250s | byte equality observed |
| deduce_sequence.test | 10 | 0.001294s | 0.001233s | 0.001223s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001317s | 0.001233s | 0.001323s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001310s | 0.001258s | 0.001262s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001235s | 0.001225s | 0.001249s | byte equality observed |
| define_assoc_tpl_struct.test | 9 | 0.001266s | 0.001220s | 0.001244s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001253s | 0.001234s | 0.001256s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001241s | 0.001312s | 0.001230s | byte equality observed |
| define_struct.test | 10 | 0.001277s | 0.001273s | 0.001271s | byte equality observed |
| define_struct_empty.test | 10 | 0.001258s | 0.001321s | 0.001294s | byte equality observed |
| define_struct_inline.test | 10 | 0.001292s | 0.001283s | 0.001296s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001276s | 0.001282s | 0.001289s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001221s | 0.001223s | 0.001261s | byte equality observed |
| define_struct_move.test | 10 | 0.001264s | 0.001257s | 0.001315s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001256s | 0.001320s | 0.001237s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001334s | 0.001269s | 0.001223s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001295s | 0.001325s | 0.001308s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001269s | 0.001318s | 0.001250s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001253s | 0.001268s | 0.001253s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001219s | 0.001268s | 0.001263s | byte equality observed |
| deque_comparison.test | 10 | 0.001274s | 0.001236s | 0.001226s | byte equality observed |
| deque_construction.test | 10 | 0.001266s | 0.001244s | 0.001241s | byte equality observed |
| deque_copy.test | 10 | 0.001249s | 0.001287s | 0.001238s | byte equality observed |
| deque_hash.test | 10 | 0.001273s | 0.001244s | 0.001260s | byte equality observed |
| deque_iterator.test | 10 | 0.001280s | 0.001288s | 0.001233s | byte equality observed |
| deque_make.test | 10 | 0.001256s | 0.001285s | 0.001297s | byte equality observed |
| deque_misc.test | 10 | 0.001252s | 0.001259s | 0.001248s | byte equality observed |
| deque_move.test | 10 | 0.001241s | 0.001258s | 0.001262s | byte equality observed |
| deque_mutate.test | 10 | 0.001229s | 0.001257s | 0.001243s | byte equality observed |
| deque_nest.test | 10 | 0.001234s | 0.001217s | 0.001230s | byte equality observed |
| deque_tie.test | 10 | 0.001245s | 0.001224s | 0.001286s | byte equality observed |
| deque_value_at.test | 10 | 0.001236s | 0.001226s | 0.001274s | byte equality observed |
| erase.test | 10 | 0.001259s | 0.001246s | 0.001292s | byte equality observed |
| erase_key.test | 10 | 0.001328s | 0.001306s | 0.001321s | byte equality observed |
| filter.test | 10 | 0.001264s | 0.001313s | 0.001267s | byte equality observed |
| filter_if.test | 10 | 0.001317s | 0.001341s | 0.001323s | byte equality observed |
| filter_view.test | 10 | 0.001351s | 0.001326s | 0.001267s | byte equality observed |
| find.test | 10 | 0.001319s | 0.001286s | 0.001263s | byte equality observed |
| find_if.test | 10 | 0.001260s | 0.001344s | 0.001237s | byte equality observed |
| flatten.test | 10 | 0.001302s | 0.001261s | 0.001258s | byte equality observed |
| flatten_view.test | 10 | 0.001265s | 0.001265s | 0.001312s | byte equality observed |
| fold.test | 10 | 0.001284s | 0.001291s | 0.001284s | byte equality observed |
| for_each.test | 10 | 0.001285s | 0.001273s | 0.001294s | byte equality observed |
| front_extended_deque.test | 10 | 0.001280s | 0.001240s | 0.001240s | byte equality observed |
| fused.test | 10 | 0.001300s | 0.001252s | 0.001280s | byte equality observed |
| fused_function_object.test | 10 | 0.001247s | 0.001190s | 0.001216s | byte equality observed |
| fused_procedure.test | 10 | 0.001239s | 0.001234s | 0.001210s | byte equality observed |
| github-176.test | 10 | 0.001227s | 0.001256s | 0.001240s | byte equality observed |
| hash.test | 10 | 0.001274s | 0.001316s | 0.001235s | byte equality observed |
| identity_view.test | 10 | 0.001293s | 0.001308s | 0.001283s | byte equality observed |
| insert.test | 10 | 0.001328s | 0.001335s | 0.001283s | byte equality observed |
| insert_range.test | 10 | 0.001288s | 0.001297s | 0.001300s | byte equality observed |
| invoke.test | 6 | 0.001247s | 0.001273s | 0.001250s | byte equality observed |
| invoke_function_object.test | 6 | 0.001248s | 0.001309s | 0.001232s | byte equality observed |
| invoke_procedure.test | 10 | 0.001254s | 0.001234s | 0.001264s | byte equality observed |
| io.test | 10 | 0.001415s | 0.001386s | 0.001450s | byte equality observed |
| iter_fold.test | 10 | 0.001253s | 0.001273s | 0.001280s | byte equality observed |
| iterator_range.test | 10 | 0.001269s | 0.001297s | 0.001282s | byte equality observed |
| join.test | 10 | 0.001263s | 0.001239s | 0.001271s | byte equality observed |
| joint_view.test | 10 | 0.001309s | 0.001291s | 0.001305s | byte equality observed |
| list_comparison.test | 10 | 0.001229s | 0.001228s | 0.001263s | byte equality observed |
| list_construction.test | 10 | 0.001220s | 0.001272s | 0.001241s | byte equality observed |
| list_copy.test | 10 | 0.001244s | 0.001263s | 0.001237s | byte equality observed |
| list_hash.test | 10 | 0.001263s | 0.001264s | 0.001270s | byte equality observed |
| list_iterator.test | 10 | 0.001228s | 0.001243s | 0.001236s | byte equality observed |
| list_make.test | 10 | 0.001245s | 0.001289s | 0.001314s | byte equality observed |
| list_misc.test | 10 | 0.001273s | 0.001255s | 0.001271s | byte equality observed |
| list_mutate.test | 10 | 0.001252s | 0.001266s | 0.001243s | byte equality observed |
| list_nest.test | 10 | 0.001219s | 0.001326s | 0.001247s | byte equality observed |
| list_tie.test | 10 | 0.001262s | 0.001288s | 0.001241s | byte equality observed |
| list_value_at.test | 10 | 0.001288s | 0.001246s | 0.001260s | byte equality observed |
| make_fused.test | 10 | 0.001217s | 0.001242s | 0.001214s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001246s | 0.001266s | 0.001232s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001224s | 0.001230s | 0.001314s | byte equality observed |
| make_list.test | 10 | 0.001257s | 0.001276s | 0.001274s | byte equality observed |
| make_unfused.test | 10 | 0.001229s | 0.001198s | 0.001277s | byte equality observed |
| make_vector.test | 10 | 0.001270s | 0.001202s | 0.001239s | byte equality observed |
| map.test | 10 | 0.001269s | 0.001269s | 0.001267s | byte equality observed |
| map_comparison.test | 10 | 0.001251s | 0.001244s | 0.001263s | byte equality observed |
| map_construction.test | 10 | 0.001266s | 0.001273s | 0.001236s | byte equality observed |
| map_copy.test | 10 | 0.001275s | 0.001253s | 0.001293s | byte equality observed |
| map_misc.test | 10 | 0.001292s | 0.001267s | 0.001279s | byte equality observed |
| map_move.test | 10 | 0.001269s | 0.001301s | 0.001257s | byte equality observed |
| map_mutate.test | 10 | 0.001225s | 0.001236s | 0.001264s | byte equality observed |
| map_tie.test | 10 | 0.001269s | 0.001254s | 0.001249s | byte equality observed |
| move.test | 10 | 0.001244s | 0.001202s | 0.001311s | byte equality observed |
| nil.test | 10 | 0.001235s | 0.001276s | 0.001219s | byte equality observed |
| none.test | 10 | 0.001246s | 0.001248s | 0.001317s | byte equality observed |
| nview.test | 10 | 0.001268s | 0.001235s | 0.001249s | byte equality observed |
| pop_back.test | 10 | 0.001276s | 0.001283s | 0.001306s | byte equality observed |
| pop_front.test | 10 | 0.001282s | 0.001270s | 0.001270s | byte equality observed |
| push_back.test | 10 | 0.001337s | 0.001228s | 0.001255s | byte equality observed |
| push_front.test | 10 | 0.001318s | 0.001258s | 0.001227s | byte equality observed |
| quick.test | 10 | 0.000603s | 0.001228s | 0.001218s | byte equality observed |
| ref_vector.test | 10 | 0.001277s | 0.001241s | 0.001200s | byte equality observed |
| remove.test | 10 | 0.001294s | 0.001336s | 0.001293s | byte equality observed |
| remove_if.test | 10 | 0.001299s | 0.001279s | 0.001266s | byte equality observed |
| repetitive_view.test | 10 | 0.001306s | 0.001312s | 0.001247s | byte equality observed |
| replace.test | 10 | 0.001272s | 0.001279s | 0.001236s | byte equality observed |
| replace_if.test | 10 | 0.001275s | 0.001350s | 0.001246s | byte equality observed |
| reverse.test | 10 | 0.001307s | 0.001306s | 0.001267s | byte equality observed |
| reverse_fold.test | 10 | 0.001255s | 0.001235s | 0.001312s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001271s | 0.001269s | 0.001275s | byte equality observed |
| reverse_view.test | 10 | 0.001342s | 0.001269s | 0.001246s | byte equality observed |
| segmented_find.test | 10 | 0.001314s | 0.001275s | 0.001235s | byte equality observed |
| segmented_find_if.test | 10 | 0.001245s | 0.001232s | 0.001227s | byte equality observed |
| segmented_fold.test | 10 | 0.001257s | 0.001271s | 0.001244s | byte equality observed |
| segmented_for_each.test | 10 | 0.001303s | 0.001312s | 0.001256s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001290s | 0.001300s | 0.001305s | byte equality observed |
| set.test | 9 | 0.001263s | 0.001275s | 0.001246s | byte equality observed |
| single_view.test | 10 | 0.001273s | 0.001297s | 0.001231s | byte equality observed |
| std_array.test | 10 | 0.001237s | 0.001246s | 0.001292s | byte equality observed |
| std_pair.test | 10 | 0.001265s | 0.001265s | 0.001265s | byte equality observed |
| std_tuple.test | 10 | 0.001260s | 0.001253s | 0.001265s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001287s | 0.001213s | 0.001263s | byte equality observed |
| swap.test | 8 | 0.001241s | 0.001253s | 0.001244s | byte equality observed |
| transform.test | 10 | 0.001350s | 0.001295s | 0.001296s | byte equality observed |
| transform_view.test | 10 | 0.001291s | 0.001351s | 0.001263s | byte equality observed |
| tuple_comparison.test | 10 | 0.001246s | 0.001211s | 0.001239s | byte equality observed |
| tuple_construction.test | 10 | 0.001258s | 0.001221s | 0.001234s | byte equality observed |
| tuple_conversion.test | 10 | 0.001279s | 0.001237s | 0.001269s | byte equality observed |
| tuple_copy.test | 10 | 0.001257s | 0.001271s | 0.001240s | byte equality observed |
| tuple_element.test | 10 | 0.001260s | 0.001248s | 0.001286s | byte equality observed |
| tuple_hash.test | 10 | 0.001249s | 0.001288s | 0.001266s | byte equality observed |
| tuple_make.test | 10 | 0.001274s | 0.001287s | 0.001214s | byte equality observed |
| tuple_misc.test | 10 | 0.001314s | 0.001235s | 0.001230s | byte equality observed |
| tuple_mutate.test | 10 | 0.001260s | 0.001227s | 0.001217s | byte equality observed |
| tuple_nest.test | 10 | 0.001286s | 0.001234s | 0.001246s | byte equality observed |
| tuple_tie.test | 8 | 0.001252s | 0.001216s | 0.001268s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001257s | 0.001297s | 0.001240s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001234s | 0.001320s | 0.001329s | byte equality observed |
| unfused.test | 10 | 0.001234s | 0.001219s | 0.001277s | byte equality observed |
| unfused_typed.test | 8 | 0.001231s | 0.001280s | 0.001188s | byte equality observed |
| vector_comparison.test | 10 | 0.001240s | 0.001239s | 0.001246s | byte equality observed |
| vector_construction.test | 10 | 0.001228s | 0.001310s | 0.001222s | byte equality observed |
| vector_conversion.test | 10 | 0.001276s | 0.001243s | 0.001262s | byte equality observed |
| vector_copy.test | 10 | 0.001228s | 0.001275s | 0.001258s | byte equality observed |
| vector_hash.test | 10 | 0.001248s | 0.001238s | 0.001271s | byte equality observed |
| vector_iterator.test | 10 | 0.001276s | 0.001246s | 0.001243s | byte equality observed |
| vector_make.test | 10 | 0.001226s | 0.001240s | 0.001235s | byte equality observed |
| vector_misc.test | 10 | 0.001239s | 0.001286s | 0.001218s | byte equality observed |
| vector_move.test | 10 | 0.001258s | 0.001246s | 0.001232s | byte equality observed |
| vector_mutate.test | 10 | 0.001269s | 0.001226s | 0.001188s | byte equality observed |
| vector_n.test | 10 | 0.001261s | 0.001228s | 0.001264s | byte equality observed |
| vector_nest.test | 10 | 0.001218s | 0.001271s | 0.001206s | byte equality observed |
| vector_tie.test | 10 | 0.001210s | 0.001239s | 0.001260s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001279s | 0.001279s | 0.001270s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001270s | 0.001248s | 0.001271s | byte equality observed |
| vector_value_at.test | 10 | 0.001255s | 0.001232s | 0.001295s | byte equality observed |
| zip.test | 7 | 0.001257s | 0.001240s | 0.001238s | byte equality observed |
| zip2.test | 10 | 0.001277s | 0.001255s | 0.001244s | byte equality observed |
| zip_ignore.test | 10 | 0.001266s | 0.001238s | 0.001209s | byte equality observed |
| zip_view.test | 10 | 0.001219s | 0.001204s | 0.001226s | byte equality observed |
| zip_view2.test | 10 | 0.001255s | 0.001203s | 0.001253s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001231s | 0.001250s | 0.001186s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
