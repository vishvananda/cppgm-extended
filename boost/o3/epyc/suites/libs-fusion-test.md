# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.504831 | 1.012147 | 1.574133 | 0.001951 | 0.001832 | 0.002078 |
| any.test | RUN | 0.495422 | 0.969874 | 1.494884 | 0.001942 | 0.002049 | 0.002092 |
| clear.test | RUN | 0.338863 | 0.885314 | 1.213085 | 0.001853 | 0.001876 | 0.002182 |
| copy.test | RUN | 0.295082 | 0.674009 | 1.008543 | 0.002188 | 0.002076 | 0.001980 |
| count.test | RUN | 0.327301 | 0.741585 | 1.119293 | 0.002071 | 0.002830 | 0.002145 |
| count_if.test | RUN | 0.309677 | 0.691877 | 0.993861 | 0.002584 | 0.002138 | 0.001862 |
| erase.test | RUN | 0.445228 | 1.235082 | 1.582063 | 0.002085 | 0.002046 | 0.001928 |
| erase_key.test | RUN | 0.413397 | 1.203520 | 1.478207 | 0.002123 | 0.002295 | 0.002373 |
| filter.test | RUN | 0.378133 | 1.008915 | 1.375084 | 0.001685 | 0.002349 | 0.002313 |
| filter_if.test | RUN | 0.425534 | 1.171455 | 1.574902 | 0.002256 | 0.003183 | 0.002327 |
| find.test | RUN | 0.371978 | 0.877387 | 1.244780 | 0.001814 | 0.001894 | 0.002028 |
| find_if.test | RUN | 0.357330 | 0.786024 | 1.171656 | 0.002072 | 0.001873 | 0.001864 |
| fold.test | RUN | 0.399726 | 0.957094 | 1.419970 | 0.002126 | 0.002160 | 0.002015 |
| for_each.test | RUN | 0.348154 | 0.892616 | 1.215066 | 0.002060 | 0.002131 | 0.001895 |
| insert.test | RUN | 0.502481 | 1.471008 | 1.918035 | 0.002181 | 0.002450 | 0.002218 |
| insert_range.test | RUN | 0.512454 | 1.485529 | 1.970419 | 0.001862 | 0.001914 | 0.001874 |
| iter_fold.test | RUN | 0.604086 | 1.431325 | 2.280614 | 0.002122 | 0.002508 | 0.002072 |
| move.test | RUN | 0.298906 | 0.684569 | 1.056792 | 0.002028 | 0.002083 | 0.001850 |
| none.test | RUN | 0.484592 | 1.036487 | 1.578245 | 0.001866 | 0.002140 | 0.001590 |
| pop_back.test | RUN | 0.483971 | 1.287053 | 1.765335 | 0.002019 | 0.002250 | 0.001944 |
| pop_front.test | RUN | 0.406605 | 1.075735 | 1.457459 | 0.002161 | 0.001989 | 0.001934 |
| push_back.test | RUN | 0.490757 | 1.510733 | 1.863517 | 0.002112 | 0.002294 | 0.002112 |
| push_front.test | RUN | 0.436162 | 1.252501 | 1.629983 | 0.002352 | 0.002135 | 0.002119 |
| remove.test | RUN | 0.482753 | 1.456683 | 1.914270 | 0.002104 | 0.002208 | 0.002444 |
| remove_if.test | RUN | 0.438243 | 1.165366 | 1.632217 | 0.002234 | 0.002294 | 0.002164 |
| replace.test | RUN | 0.424681 | 1.157296 | 1.491891 | 0.002146 | 0.002228 | 0.001898 |
| replace_if.test | RUN | 0.419387 | 1.129100 | 1.486400 | 0.002252 | 0.002415 | 0.002176 |
| reverse_fold.test | RUN | 0.669933 | 1.654706 | 2.531788 | 0.001870 | 0.002550 | 0.002075 |
| reverse_iter_fold.test | RUN | 0.676500 | 1.750112 | 2.686299 | 0.002092 | 0.002034 | 0.001752 |
| reverse.test | RUN | 0.422776 | 1.171949 | 1.563127 | 0.001977 | 0.002260 | 0.001740 |
| segmented_for_each.test | RUN | 0.761391 | 2.283933 | 3.021000 | 0.002087 | 0.002018 | 0.002076 |
| segmented_find.test | RUN | 0.462962 | 1.202180 | 1.768028 | 0.001991 | 0.002151 | 0.002156 |
| segmented_find_if.test | RUN | 0.453709 | 1.237770 | 1.735348 | 0.002182 | 0.002062 | 0.001944 |
| segmented_fold.test | RUN | 0.593863 | 1.586743 | 2.223277 | 0.002100 | 0.002258 | 0.002224 |
| transform.test | RUN | 0.429132 | 1.279640 | 1.587834 | 0.001880 | 0.002428 | 0.002287 |
| join.test | RUN | 0.328457 | 0.813878 | 1.150918 | 0.002130 | 0.002142 | 0.002586 |
| zip.test | RUN | 0.573055 | 1.403581 | 2.139471 | 0.003348 | 0.002264 | 0.001981 |
| zip2.test | RUN | 0.590589 | 1.412269 | 2.044794 | 0.002155 | 0.001880 | 0.001930 |
| zip_ignore.test | RUN | 0.434655 | 0.970971 | 1.354975 | 0.001761 | 0.002036 | 0.001825 |
| flatten.test | RUN | 0.399945 | 1.073795 | 1.380584 | 0.001803 | 0.002299 | 0.001992 |
| ticket-5490.test | COMPILE | 0.207825 | 0.495587 | 0.603933 | — | — | — |
| as_deque.test | RUN | 0.401471 | 0.989844 | 1.544411 | 0.001850 | 0.002181 | 0.001908 |
| as_list.test | RUN | 0.440931 | 1.214054 | 1.580344 | 0.001823 | 0.002482 | 0.002170 |
| as_map.test | RUN | 0.432868 | 1.222287 | 1.591978 | 0.001804 | 0.002169 | 0.002087 |
| as_map_assoc.test | RUN | 0.424975 | 1.146472 | 1.518886 | 0.001837 | 0.002030 | 0.002004 |
| as_set.test | RUN | 0.480581 | 1.420578 | 1.855682 | 0.001703 | 0.002352 | 0.002064 |
| as_vector.test | RUN | 0.433894 | 1.297411 | 1.578969 | 0.001940 | 0.001861 | 0.001893 |
| boost_tuple.test | RUN | 0.439747 | 1.212956 | 1.640714 | 0.001942 | 0.001933 | 0.001976 |
| boost_tuple_iterator.test | RUN | 0.309078 | 0.758675 | 1.026175 | 0.002082 | 0.001751 | 0.001736 |
| cons.test | RUN | 0.557920 | 1.404523 | 1.996840 | 0.002063 | 0.001896 | 0.002086 |
| convert_boost_tuple.test | RUN | 0.350247 | 1.073039 | 1.301110 | 0.002080 | 0.002527 | 0.001894 |
| convert_deque.test | RUN | 0.346653 | 1.083449 | 1.270114 | 0.001766 | 0.002028 | 0.001835 |
| convert_list.test | RUN | 0.362371 | 1.103787 | 1.295181 | 0.002061 | 0.001784 | 0.002044 |
| convert_std_pair.test | RUN | 0.369851 | 1.083779 | 1.287991 | 0.001682 | 0.002297 | 0.002120 |
| convert_std_tuple.test | RUN | 0.355402 | 1.036283 | 1.243027 | 0.001959 | 0.001831 | 0.001960 |
| convert_vector.test | RUN | 0.366964 | 1.039872 | 1.281803 | 0.001950 | 0.001451 | 0.002152 |
| filter_view.test | RUN | 0.441605 | 1.134874 | 1.514313 | 0.002123 | 0.002013 | 0.002031 |
| hash.test | RUN | 0.424020 | 1.001885 | 1.473434 | 0.002006 | 0.002078 | 0.002014 |
| io.test | RUN | 0.436375 | 1.446927 | 1.731788 | 0.001976 | 0.002432 | 0.002062 |
| iterator_range.test | RUN | 0.409851 | 1.060058 | 1.465563 | 0.001926 | 0.002182 | 0.002008 |
| joint_view.test | RUN | 0.542543 | 1.832026 | 2.093966 | 0.002076 | 0.002310 | 0.001825 |
| list_comparison.test | RUN | 0.311842 | 0.768299 | 1.046429 | 0.002136 | 0.001816 | 0.001965 |
| list_construction.test | RUN | 0.306537 | 0.704505 | 1.038848 | 0.001866 | 0.002066 | 0.001846 |
| list_copy.test | RUN | 0.487420 | 1.303920 | 1.762479 | 0.002088 | 0.002051 | 0.002098 |
| list_iterator.test | RUN | 0.300725 | 0.721109 | 1.007060 | 0.002167 | 0.002090 | 0.002106 |
| list_hash.test | RUN | 0.413167 | 1.373907 | 1.462797 | 0.001913 | 0.002241 | 0.002088 |
| list_make.test | RUN | 0.326749 | 0.821454 | 1.160191 | 0.002062 | 0.001843 | 0.002100 |
| list_misc.test | RUN | 0.414250 | 0.941667 | 1.308997 | 0.001880 | 0.002242 | 0.001960 |
| list_mutate.test | RUN | 0.284581 | 0.642323 | 0.925779 | 0.002008 | 0.002170 | 0.002134 |
| list_nest.test | RUN | 0.578976 | 1.769607 | 2.435251 | 0.001896 | 0.002031 | 0.002124 |
| list_tie.test | RUN | 0.310933 | 0.710710 | 1.005415 | 0.001966 | 0.002214 | 0.001882 |
| list_value_at.test | RUN | 0.300484 | 0.726390 | 0.952447 | 0.001932 | 0.002388 | 0.002418 |
| deque_comparison.test | RUN | 0.304428 | 0.760167 | 1.107646 | 0.001801 | 0.002182 | 0.002016 |
| deque_construction.test | RUN | 0.318376 | 0.735768 | 1.044407 | 0.002786 | 0.002230 | 0.001891 |
| deque_copy.test | RUN | 0.452026 | 1.234309 | 1.643027 | 0.002128 | 0.001864 | 0.002424 |
| deque_iterator.test | RUN | 0.326440 | 0.772453 | 1.018792 | 0.002349 | 0.002064 | 0.002100 |
| deque_hash.test | RUN | 0.432675 | 1.366528 | 1.486088 | 0.001938 | 0.002198 | 0.002108 |
| deque_is_constructible.test | COMPILE | 0.182123 | 0.338361 | 0.472402 | — | — | — |
| deque_make.test | RUN | 0.317981 | 0.766230 | 1.105159 | 0.001933 | 0.002272 | 0.002068 |
| deque_misc.test | RUN | 0.429060 | 0.971728 | 1.349124 | 0.001663 | 0.001987 | 0.002165 |
| deque_move.test | RUN | 0.338798 | 0.792331 | 1.009195 | 0.001912 | 0.001888 | 0.002042 |
| deque_mutate.test | RUN | 0.295783 | 0.669654 | 0.899963 | 0.001971 | 0.002241 | 0.002108 |
| deque_nest.test | RUN | 0.882383 | 2.811117 | 3.821333 | 0.002049 | 0.002262 | 0.002030 |
| deque_tie.test | RUN | 0.290175 | 0.679306 | 0.985973 | 0.002236 | 0.001902 | 0.002385 |
| deque_value_at.test | RUN | 0.302607 | 0.671440 | 0.981200 | 0.002212 | 0.001858 | 0.002521 |
| front_extended_deque.test | RUN | 0.406945 | 1.025116 | 1.359492 | 0.002007 | 0.002246 | 0.002183 |
| back_extended_deque.test | RUN | 0.420936 | 0.984408 | 1.436477 | 0.001970 | 0.002086 | 0.002180 |
| make_list.test | RUN | 0.327303 | 0.795360 | 1.092680 | 0.001748 | 0.001848 | 0.002124 |
| make_vector.test | RUN | 0.322026 | 0.832944 | 1.147880 | 0.001826 | 0.002212 | 0.002318 |
| map.test | RUN | 0.411806 | 1.167796 | 1.509029 | 0.002122 | 0.002322 | 0.002068 |
| map_comparison.test | RUN | 0.320089 | 0.768296 | 1.078696 | 0.002150 | 0.002063 | 0.002166 |
| map_construction.test | RUN | 0.326695 | 0.751106 | 1.052753 | 0.002162 | 0.002087 | 0.002066 |
| map_copy.test | RUN | 0.379824 | 0.926471 | 1.307132 | 0.002180 | 0.002198 | 0.002132 |
| map_misc.test | RUN | 0.398415 | 0.977013 | 1.330542 | 0.001578 | 0.002080 | 0.001926 |
| map_move.test | RUN | 0.316781 | 0.785514 | 1.067093 | 0.001826 | 0.002024 | 0.001914 |
| map_mutate.test | RUN | 0.287993 | 0.671681 | 0.929965 | 0.002073 | 0.002104 | 0.001822 |
| map_tie.test | RUN | 0.295511 | 0.645502 | 0.940292 | 0.002024 | 0.002022 | 0.002028 |
| nil.test | RUN | 0.199060 | 0.452982 | 0.673489 | 0.002194 | 0.001966 | 0.001903 |
| nview.test | RUN | 0.393833 | 0.968111 | 1.377392 | 0.002148 | 0.002326 | 0.002296 |
| reverse_view.test | RUN | 0.489008 | 1.550729 | 1.975730 | 0.002128 | 0.001968 | 0.001926 |
| segmented_iterator_range.test | RUN | 0.610664 | 1.781275 | 2.466409 | 0.002144 | 0.001920 | 0.002013 |
| set.test | RUN | 0.390749 | 1.053758 | 1.388054 | 0.001996 | 0.002298 | 0.001990 |
| single_view.test | RUN | 0.346420 | 0.933621 | 1.265410 | 0.002016 | 0.001929 | 0.002012 |
| std_pair.test | RUN | 0.443028 | 1.188979 | 1.609717 | 0.001782 | 0.002035 | 0.002024 |
| boost_array.test | RUN | 0.381106 | 0.846382 | 1.228388 | 0.001978 | 0.002064 | 0.001765 |
| array.test | RUN | 0.389455 | 0.880784 | 1.199007 | 0.001977 | 0.002012 | 0.002052 |
| std_array.test | RUN | 0.380267 | 0.864952 | 1.154165 | 0.001938 | 0.002060 | 0.002140 |
| tuple_comparison.test | RUN | 0.343158 | 0.790404 | 1.124297 | 0.001975 | 0.002508 | 0.002190 |
| tuple_construction.test | RUN | 0.349200 | 0.802127 | 1.106821 | 0.001918 | 0.002331 | 0.001954 |
| tuple_conversion.test | RUN | 0.754765 | 2.422802 | 3.547031 | 0.002016 | 0.002401 | 0.001681 |
| tuple_copy.test | RUN | 0.482988 | 1.430199 | 1.913980 | 0.001990 | 0.002356 | 0.001877 |
| tuple_element.test | RUN | 0.332332 | 0.747525 | 1.091805 | 0.001790 | 0.002280 | 0.001898 |
| tuple_make.test | RUN | 0.341117 | 0.842631 | 1.211724 | 0.001782 | 0.002024 | 0.001920 |
| tuple_misc.test | RUN | 0.449888 | 1.022297 | 1.459193 | 0.002424 | 0.002380 | 0.002104 |
| tuple_mutate.test | RUN | 0.304660 | 0.720599 | 0.989240 | 0.001932 | 0.001842 | 0.001922 |
| tuple_nest.test | RUN | 1.298058 | 4.881504 | 6.588828 | 0.002404 | 0.001930 | 0.002085 |
| tuple_hash.test | RUN | 0.462074 | 1.355438 | 1.563979 | 0.002255 | 0.002098 | 0.001884 |
| tuple_tie.test | RUN | 0.356441 | 0.739768 | 1.123847 | 0.002494 | 0.002192 | 0.002059 |
| tuple_traits__maybe_variadic.test | RUN | 0.353663 | 0.978212 | 1.201299 | 0.002107 | 0.002039 | 0.001656 |
| tuple_traits__no_variadic.test | RUN | 0.424859 | 0.956644 | 1.335437 | 0.002092 | 0.002412 | 0.001940 |
| transform_view.test | RUN | 0.524385 | 1.513212 | 1.921134 | 0.002038 | 0.002121 | 0.001812 |
| identity_view.test | RUN | 0.487083 | 1.427104 | 1.854749 | 0.001816 | 0.001850 | 0.002065 |
| vector_comparison.test | RUN | 0.317494 | 0.800775 | 1.126053 | 0.002120 | 0.001958 | 0.002060 |
| vector_construction.test | RUN | 0.319236 | 0.770373 | 1.061013 | 0.001958 | 0.001948 | 0.002148 |
| vector_conversion.test | RUN | 0.874336 | 3.191497 | 4.427910 | 0.002094 | 0.002094 | 0.002004 |
| vector_copy.test | RUN | 0.472658 | 1.420936 | 1.940100 | 0.001868 | 0.002294 | 0.002062 |
| vector_iterator.test | RUN | 0.318808 | 0.783431 | 1.046611 | 0.001961 | 0.002116 | 0.002234 |
| vector_make.test | RUN | 0.325671 | 0.819145 | 1.103051 | 0.001959 | 0.001938 | 0.002050 |
| vector_misc.test | RUN | 0.415527 | 1.046240 | 1.327104 | 0.001916 | 0.001783 | 0.001728 |
| vector_move.test | RUN | 0.314471 | 0.794805 | 1.077107 | 0.002186 | 0.001869 | 0.001625 |
| vector_mutate.test | RUN | 0.285454 | 0.661438 | 0.923103 | 0.001604 | 0.002104 | 0.001793 |
| vector_n.test | RUN | 0.352979 | 0.837808 | 1.235456 | 0.002358 | 0.001960 | 0.002080 |
| vector_nest.test | RUN | 1.276313 | 5.139703 | 6.682967 | 0.002222 | 0.002008 | 0.002010 |
| vector_hash.test | RUN | 0.417940 | 1.357264 | 1.465934 | 0.002226 | 0.002191 | 0.001700 |
| vector_tie.test | RUN | 0.306987 | 0.760084 | 1.051692 | 0.002053 | 0.002359 | 0.001716 |
| vector_traits__maybe_variadic.test | RUN | 0.327700 | 0.874371 | 1.100883 | 0.002210 | 0.002392 | 0.002071 |
| vector_traits__no_variadic.test | RUN | 0.347228 | 0.916087 | 1.124834 | 0.001684 | 0.002512 | 0.001564 |
| vector_value_at.test | RUN | 0.294618 | 0.706804 | 0.981716 | 0.002020 | 0.002694 | 0.001874 |
| zip_view.test | RUN | 0.619639 | 1.641754 | 2.412486 | 0.001851 | 0.002020 | 0.002092 |
| zip_view2.test | RUN | 0.488848 | 1.236760 | 1.807295 | 0.001983 | 0.001962 | 0.001698 |
| zip_view_ignore.test | RUN | 0.471355 | 1.120743 | 1.664770 | 0.001870 | 0.002145 | 0.001754 |
| repetitive_view.test | RUN | 0.800981 | 2.115429 | 3.029387 | 0.001990 | 0.002464 | 0.001762 |
| deduce_sequence.test | RUN | 0.375949 | 0.888429 | 1.255555 | 0.002024 | 0.002170 | 0.002138 |
| adapt_adt_named.test | RUN | 0.426477 | 1.237550 | 1.619188 | 0.001815 | 0.002558 | 0.002143 |
| adapt_adt_named_empty.test | RUN | 0.394445 | 1.042260 | 1.311031 | 0.002232 | 0.002866 | 0.002125 |
| adapt_adt.test | RUN | 0.487841 | 1.604417 | 1.920873 | 0.001798 | 0.002104 | 0.001884 |
| adapt_adt_empty.test | RUN | 0.376472 | 1.022346 | 1.308087 | 0.001946 | 0.002414 | 0.001792 |
| adapt_assoc_adt_named.test | RUN | 0.492529 | 1.293599 | 1.707792 | 0.002140 | 0.002152 | 0.002098 |
| adapt_assoc_adt_named_empty.test | RUN | 0.386245 | 1.014753 | 1.328484 | 0.001761 | 0.002130 | 0.002090 |
| adapt_assoc_adt.test | RUN | 0.515611 | 1.310133 | 1.793122 | 0.001938 | 0.002112 | 0.002167 |
| adapt_assoc_adt_empty.test | RUN | 0.384522 | 1.007499 | 1.308734 | 0.002224 | 0.002216 | 0.002144 |
| adapt_assoc_struct_named.test | RUN | 0.492817 | 1.285240 | 1.692077 | 0.001805 | 0.002284 | 0.001931 |
| adapt_assoc_struct_named_empty.test | RUN | 0.377641 | 0.985923 | 1.319184 | 0.001867 | 0.002536 | 0.002116 |
| adapt_assoc_struct.test | RUN | 0.434478 | 1.226200 | 1.522562 | 0.001670 | 0.003363 | 0.002144 |
| adapt_assoc_struct_empty.test | RUN | 0.387353 | 1.048362 | 1.294632 | 0.001816 | 0.001946 | 0.002226 |
| adapt_assoc_tpl_adt.test | RUN | 0.522319 | 1.358767 | 1.831230 | 0.001949 | 0.002633 | 0.002140 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.387689 | 1.016085 | 1.283354 | 0.001808 | 0.002350 | 0.002063 |
| adapt_assoc_tpl_struct.test | RUN | 0.425810 | 1.201548 | 1.499596 | 0.001926 | 0.002154 | 0.001792 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.379788 | 1.022167 | 1.293361 | 0.002010 | 0.002452 | 0.001621 |
| adapt_struct_named.test | RUN | 0.424601 | 1.195204 | 1.593385 | 0.002361 | 0.002357 | 0.001919 |
| adapt_struct_named_empty.test | RUN | 0.393959 | 0.969283 | 1.326757 | 0.002028 | 0.002266 | 0.002099 |
| adapt_struct.test | RUN | 0.488553 | 1.391639 | 1.692482 | 0.001753 | 0.002059 | 0.001969 |
| adapt_struct_empty.test | RUN | 0.388593 | 1.001694 | 1.295652 | 0.002721 | 0.002186 | 0.002171 |
| adapt_tpl_adt.test | RUN | 0.495701 | 1.559945 | 1.784435 | 0.001870 | 0.001979 | 0.002114 |
| adapt_tpl_adt_empty.test | RUN | 0.409985 | 1.023495 | 1.310610 | 0.001783 | 0.002382 | 0.002176 |
| adapt_tpl_struct.test | RUN | 0.406630 | 1.112168 | 1.453412 | 0.002465 | 0.001836 | 0.002101 |
| adapt_tpl_struct_empty.test | RUN | 0.409240 | 1.041879 | 1.283406 | 0.002142 | 0.002750 | 0.002077 |
| adt_attribute_proxy.test | RUN | 0.442768 | 0.984580 | 1.327269 | 0.001938 | 0.002237 | 0.001850 |
| define_struct.test | RUN | 0.528850 | 1.371258 | 1.822810 | 0.002168 | 0.002645 | 0.001946 |
| define_struct_empty.test | RUN | 0.458007 | 1.161644 | 1.522133 | 0.002380 | 0.002012 | 0.001948 |
| define_struct_move.test | RUN | 0.294728 | 0.675091 | 0.895418 | 0.002269 | 0.002128 | 0.002328 |
| define_struct_inline.test | RUN | 0.565092 | 1.630468 | 1.990602 | 0.002090 | 0.002116 | 0.002074 |
| define_struct_inline_empty.test | RUN | 0.458346 | 1.172931 | 1.501071 | 0.001762 | 0.002035 | 0.001908 |
| define_struct_inline_move.test | RUN | 0.300897 | 0.618303 | 0.943913 | 0.002150 | 0.002226 | 0.002085 |
| define_assoc_struct.test | RUN | 0.515457 | 1.292977 | 1.738777 | 0.002120 | 0.002180 | 0.002080 |
| define_assoc_struct_empty.test | RUN | 0.454162 | 1.125138 | 1.531444 | 0.002050 | 0.002613 | 0.002352 |
| define_assoc_struct_move.test | RUN | 0.297407 | 0.615659 | 0.926999 | 0.001937 | 0.002206 | 0.001835 |
| define_tpl_struct.test | RUN | 0.503226 | 1.304990 | 1.728276 | 0.001909 | 0.001972 | 0.002076 |
| define_tpl_struct_empty.test | RUN | 0.461776 | 1.115440 | 1.516078 | 0.001761 | 0.001926 | 0.001962 |
| define_tpl_struct_move.test | RUN | 0.301718 | 0.668234 | 0.915143 | 0.001902 | 0.002234 | 0.001748 |
| define_tpl_struct_inline.test | RUN | 0.536139 | 1.548524 | 1.937541 | 0.002442 | 0.002147 | 0.001927 |
| define_tpl_struct_inline_empty.test | RUN | 0.467594 | 1.104894 | 1.534007 | 0.002160 | 0.002110 | 0.002014 |
| define_tpl_struct_inline_move.test | RUN | 0.299093 | 0.640870 | 0.944193 | 0.001636 | 0.002143 | 0.002026 |
| define_assoc_tpl_struct.test | RUN | 0.513076 | 1.324174 | 1.762311 | 0.002114 | 0.002414 | 0.002050 |
| define_assoc_tpl_struct_empty.test | RUN | 0.460932 | 1.153207 | 1.508423 | 0.002016 | 0.002264 | 0.001669 |
| define_assoc_tpl_struct_move.test | RUN | 0.294811 | 0.668724 | 0.912495 | 0.002300 | 0.002059 | 0.002068 |
| std_tuple.test | RUN | 0.318234 | 0.711863 | 1.006930 | 0.001988 | 0.001965 | 0.001678 |
| std_tuple_iterator.test | RUN | 0.311064 | 0.794403 | 1.022813 | 0.002018 | 0.002472 | 0.001815 |
| ref_vector.test | RUN | 0.351932 | 0.780809 | 1.114084 | 0.002211 | 0.002282 | 0.001788 |
| flatten_view.test | RUN | 0.367199 | 0.956943 | 1.250345 | 0.001918 | 0.002084 | 0.001992 |
| github-159.test | COMPILE | 0.168288 | 0.345204 | 0.492251 | — | — | — |
| github-176.test | RUN | 0.350701 | 0.858419 | 1.075386 | 0.001928 | 0.002067 | 0.002063 |
| size.test | COMPILE | 0.297032 | 0.615532 | 0.880134 | — | — | — |
| fused.test | RUN | 0.409998 | 0.916890 | 1.303795 | 0.002134 | 0.002106 | 0.001540 |
| fused_function_object.test | RUN | 0.299951 | 0.724788 | 0.991685 | 0.002084 | 0.002070 | 0.001798 |
| fused_procedure.test | RUN | 0.391161 | 0.865231 | 1.266583 | 0.001894 | 0.002118 | 0.001926 |
| unfused.test | RUN | 0.317529 | 0.734435 | 0.996520 | 0.001982 | 0.002292 | 0.001984 |
| unfused_typed.test | RUN | 0.323372 | 0.769524 | 1.032220 | 0.001876 | 0.002491 | 0.001934 |
| make_fused.test | RUN | 0.406471 | 0.939387 | 1.289903 | 0.002025 | 0.001904 | 0.001932 |
| make_fused_function_object.test | RUN | 0.314072 | 0.684340 | 0.986009 | 0.002000 | 0.001845 | 0.002195 |
| make_fused_procedure.test | RUN | 0.414635 | 0.894058 | 1.270984 | 0.002139 | 0.002094 | 0.002756 |
| make_unfused.test | RUN | 0.319795 | 0.699143 | 0.996853 | 0.002608 | 0.002210 | 0.001850 |
| invoke.test | RUN | 0.845688 | 2.499384 | 3.343311 | 0.001959 | 0.001822 | 0.001728 |
| invoke_function_object.test | RUN | 0.380266 | 0.922132 | 1.285448 | 0.002456 | 0.002330 | 0.002087 |
| invoke_procedure.test | RUN | 0.672173 | 1.919685 | 2.514501 | 0.001773 | 0.002088 | 0.002081 |
| swap.test | RUN | 0.461948 | 1.135300 | 1.676684 | 0.001855 | 0.002452 | 0.002058 |
| is_sequence.test | COMPILE | 0.080024 | 0.170285 | 0.205698 | — | — | — |
| is_view.test | COMPILE | 0.082100 | 0.168248 | 0.205023 | — | — | — |
| pair_deque.test | COMPILE | 0.170184 | 0.366163 | 0.521101 | — | — | — |
| pair_list.test | COMPILE | 0.170320 | 0.356436 | 0.488820 | — | — | — |
| pair_map.test | COMPILE | 0.172531 | 0.387102 | 0.475643 | — | — | — |
| pair_set.test | COMPILE | 0.175826 | 0.368778 | 0.504872 | — | — | — |
| pair_vector.test | COMPILE | 0.174830 | 0.373694 | 0.479896 | — | — | — |
| pair_nest.test | COMPILE | 0.071661 | 0.148913 | 0.195063 | — | — | — |
| index_sequence.test | COMPILE | 0.058532 | 0.112182 | 0.145370 | — | — | — |
| and.test | COMPILE | 0.045204 | 0.079542 | 0.091983 | — | — | — |
| tag_of.test | COMPILE | 0.078535 | 0.158869 | 0.188306 | — | — | — |
| unused.test | COMPILE | 0.041142 | 0.101624 | 0.112539 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.077057 | 0.155756 | 0.154894 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.069152 | 0.103625 | 0.156967 | — | — | — |
| quick.test | RUN | 0.222185 | 0.473404 | 0.611250 | 0.000633 | 0.001655 | 0.001508 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.005× | 1.010×, 1.000× | 1.000× |
| clang | 1.005× | 1.004×, 1.006× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001351s | 0.001304s | 0.001324s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001372s | 0.001305s | 0.001288s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001321s | 0.001387s | 0.001372s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001316s | 0.001355s | 0.001324s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001383s | 0.001347s | 0.001337s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001365s | 0.001402s | 0.001313s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001349s | 0.001343s | 0.001296s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001347s | 0.001320s | 0.001374s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001322s | 0.001361s | 0.001328s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001308s | 0.001299s | 0.001319s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001304s | 0.001357s | 0.001312s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001345s | 0.001327s | 0.001367s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001356s | 0.001364s | 0.001329s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001336s | 0.001308s | 0.001313s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001332s | 0.001382s | 0.001327s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001333s | 0.001445s | 0.001297s | byte equality observed |
| adapt_struct.test | 10 | 0.001341s | 0.001346s | 0.001294s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001382s | 0.001358s | 0.001298s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001338s | 0.001324s | 0.001314s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001360s | 0.001387s | 0.001289s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001361s | 0.001356s | 0.001344s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001336s | 0.001334s | 0.001365s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001358s | 0.001415s | 0.001361s | byte equality observed |
| adapt_tpl_struct_empty.test | 9 | 0.001294s | 0.001311s | 0.001367s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001364s | 0.001285s | 0.001344s | byte equality observed |
| all.test | 10 | 0.001299s | 0.001242s | 0.001256s | byte equality observed |
| any.test | 10 | 0.001277s | 0.001253s | 0.001295s | byte equality observed |
| array.test | 8 | 0.001278s | 0.001260s | 0.001307s | byte equality observed |
| as_deque.test | 10 | 0.001277s | 0.001291s | 0.001316s | byte equality observed |
| as_list.test | 10 | 0.001328s | 0.001361s | 0.001314s | byte equality observed |
| as_map.test | 10 | 0.001315s | 0.001296s | 0.001320s | byte equality observed |
| as_map_assoc.test | 10 | 0.001348s | 0.001328s | 0.001277s | byte equality observed |
| as_set.test | 10 | 0.001324s | 0.001367s | 0.001360s | byte equality observed |
| as_vector.test | 10 | 0.001353s | 0.001384s | 0.001355s | byte equality observed |
| back_extended_deque.test | 10 | 0.001266s | 0.001307s | 0.001347s | byte equality observed |
| boost_array.test | 10 | 0.001291s | 0.001286s | 0.001276s | byte equality observed |
| boost_tuple.test | 10 | 0.001292s | 0.001309s | 0.001290s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001278s | 0.001268s | 0.001294s | byte equality observed |
| clear.test | 9 | 0.001325s | 0.001344s | 0.001329s | byte equality observed |
| cons.test | 10 | 0.001363s | 0.001389s | 0.001382s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001286s | 0.001299s | 0.001311s | byte equality observed |
| convert_deque.test | 10 | 0.001283s | 0.001305s | 0.001272s | byte equality observed |
| convert_list.test | 10 | 0.001290s | 0.001296s | 0.001308s | byte equality observed |
| convert_std_pair.test | 10 | 0.001296s | 0.001334s | 0.001292s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001309s | 0.001313s | 0.001293s | byte equality observed |
| convert_vector.test | 10 | 0.001309s | 0.001318s | 0.001305s | byte equality observed |
| copy.test | 10 | 0.001283s | 0.001302s | 0.001278s | byte equality observed |
| count.test | 10 | 0.001304s | 0.001312s | 0.001329s | byte equality observed |
| count_if.test | 10 | 0.001257s | 0.001371s | 0.001294s | byte equality observed |
| deduce_sequence.test | 10 | 0.001299s | 0.001276s | 0.001315s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001302s | 0.001324s | 0.001294s | byte equality observed |
| define_assoc_struct_empty.test | 8 | 0.001283s | 0.001336s | 0.001323s | byte equality observed |
| define_assoc_struct_move.test | 9 | 0.001249s | 0.001346s | 0.001266s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001294s | 0.001347s | 0.001327s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001333s | 0.001308s | 0.001314s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001305s | 0.001316s | 0.001362s | byte equality observed |
| define_struct.test | 10 | 0.001318s | 0.001331s | 0.001328s | byte equality observed |
| define_struct_empty.test | 10 | 0.001352s | 0.001318s | 0.001342s | byte equality observed |
| define_struct_inline.test | 10 | 0.001339s | 0.001311s | 0.001329s | byte equality observed |
| define_struct_inline_empty.test | 8 | 0.001329s | 0.001368s | 0.001348s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001295s | 0.001330s | 0.001252s | byte equality observed |
| define_struct_move.test | 10 | 0.001285s | 0.001287s | 0.001308s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001305s | 0.001316s | 0.001351s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001306s | 0.001340s | 0.001323s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001299s | 0.001363s | 0.001339s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001286s | 0.001352s | 0.001332s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001276s | 0.001295s | 0.001339s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001275s | 0.001314s | 0.001304s | byte equality observed |
| deque_comparison.test | 10 | 0.001271s | 0.001327s | 0.001285s | byte equality observed |
| deque_construction.test | 8 | 0.001296s | 0.001328s | 0.001310s | byte equality observed |
| deque_copy.test | 10 | 0.001309s | 0.001282s | 0.001259s | byte equality observed |
| deque_hash.test | 10 | 0.001305s | 0.001345s | 0.001310s | byte equality observed |
| deque_iterator.test | 10 | 0.001273s | 0.001331s | 0.001308s | byte equality observed |
| deque_make.test | 10 | 0.001283s | 0.001298s | 0.001287s | byte equality observed |
| deque_misc.test | 10 | 0.001244s | 0.001315s | 0.001316s | byte equality observed |
| deque_move.test | 10 | 0.001280s | 0.001320s | 0.001292s | byte equality observed |
| deque_mutate.test | 10 | 0.001266s | 0.001323s | 0.001289s | byte equality observed |
| deque_nest.test | 10 | 0.001269s | 0.001264s | 0.001281s | byte equality observed |
| deque_tie.test | 9 | 0.001285s | 0.001319s | 0.001340s | byte equality observed |
| deque_value_at.test | 10 | 0.001311s | 0.001295s | 0.001270s | byte equality observed |
| erase.test | 10 | 0.001334s | 0.001328s | 0.001327s | byte equality observed |
| erase_key.test | 10 | 0.001405s | 0.001328s | 0.001364s | byte equality observed |
| filter.test | 10 | 0.001346s | 0.001301s | 0.001374s | byte equality observed |
| filter_if.test | 10 | 0.001363s | 0.001350s | 0.001378s | byte equality observed |
| filter_view.test | 10 | 0.001371s | 0.001355s | 0.001350s | byte equality observed |
| find.test | 10 | 0.001355s | 0.001315s | 0.001334s | byte equality observed |
| find_if.test | 9 | 0.001335s | 0.001348s | 0.001349s | byte equality observed |
| flatten.test | 7 | 0.001355s | 0.001278s | 0.001325s | byte equality observed |
| flatten_view.test | 9 | 0.001328s | 0.001328s | 0.001287s | byte equality observed |
| fold.test | 9 | 0.001335s | 0.001392s | 0.001345s | byte equality observed |
| for_each.test | 10 | 0.001326s | 0.001339s | 0.001334s | byte equality observed |
| front_extended_deque.test | 10 | 0.001296s | 0.001267s | 0.001249s | byte equality observed |
| fused.test | 7 | 0.001242s | 0.001287s | 0.001285s | byte equality observed |
| fused_function_object.test | 7 | 0.001245s | 0.001249s | 0.001226s | byte equality observed |
| fused_procedure.test | 10 | 0.001280s | 0.001309s | 0.001262s | byte equality observed |
| github-176.test | 10 | 0.001304s | 0.001343s | 0.001300s | byte equality observed |
| hash.test | 10 | 0.001302s | 0.001267s | 0.001296s | byte equality observed |
| identity_view.test | 9 | 0.001334s | 0.001320s | 0.001288s | byte equality observed |
| insert.test | 10 | 0.001355s | 0.001322s | 0.001263s | byte equality observed |
| insert_range.test | 10 | 0.001383s | 0.001395s | 0.001315s | byte equality observed |
| invoke.test | 10 | 0.001330s | 0.001346s | 0.001293s | byte equality observed |
| invoke_function_object.test | 10 | 0.001286s | 0.001329s | 0.001273s | byte equality observed |
| invoke_procedure.test | 10 | 0.001296s | 0.001310s | 0.001262s | byte equality observed |
| io.test | 8 | 0.001440s | 0.001459s | 0.001477s | byte equality observed |
| iter_fold.test | 10 | 0.001328s | 0.001346s | 0.001347s | byte equality observed |
| iterator_range.test | 10 | 0.001327s | 0.001371s | 0.001366s | byte equality observed |
| join.test | 8 | 0.001256s | 0.001286s | 0.001249s | byte equality observed |
| joint_view.test | 10 | 0.001363s | 0.001363s | 0.001338s | byte equality observed |
| list_comparison.test | 10 | 0.001288s | 0.001254s | 0.001324s | byte equality observed |
| list_construction.test | 10 | 0.001264s | 0.001276s | 0.001267s | byte equality observed |
| list_copy.test | 10 | 0.001284s | 0.001267s | 0.001294s | byte equality observed |
| list_hash.test | 10 | 0.001339s | 0.001326s | 0.001364s | byte equality observed |
| list_iterator.test | 10 | 0.001273s | 0.001284s | 0.001344s | byte equality observed |
| list_make.test | 10 | 0.001300s | 0.001297s | 0.001259s | byte equality observed |
| list_misc.test | 10 | 0.001330s | 0.001328s | 0.001301s | byte equality observed |
| list_mutate.test | 10 | 0.001294s | 0.001291s | 0.001302s | byte equality observed |
| list_nest.test | 10 | 0.001301s | 0.001351s | 0.001287s | byte equality observed |
| list_tie.test | 10 | 0.001295s | 0.001297s | 0.001301s | byte equality observed |
| list_value_at.test | 10 | 0.001294s | 0.001320s | 0.001301s | byte equality observed |
| make_fused.test | 9 | 0.001324s | 0.001271s | 0.001247s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001248s | 0.001256s | 0.001253s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001264s | 0.001305s | 0.001351s | byte equality observed |
| make_list.test | 9 | 0.001313s | 0.001356s | 0.001297s | byte equality observed |
| make_unfused.test | 10 | 0.001286s | 0.001295s | 0.001303s | byte equality observed |
| make_vector.test | 8 | 0.001306s | 0.001300s | 0.001274s | byte equality observed |
| map.test | 9 | 0.001300s | 0.001353s | 0.001312s | byte equality observed |
| map_comparison.test | 10 | 0.001270s | 0.001284s | 0.001316s | byte equality observed |
| map_construction.test | 10 | 0.001267s | 0.001309s | 0.001250s | byte equality observed |
| map_copy.test | 10 | 0.001283s | 0.001344s | 0.001339s | byte equality observed |
| map_misc.test | 10 | 0.001350s | 0.001282s | 0.001272s | byte equality observed |
| map_move.test | 10 | 0.001308s | 0.001309s | 0.001275s | byte equality observed |
| map_mutate.test | 10 | 0.001317s | 0.001313s | 0.001272s | byte equality observed |
| map_tie.test | 10 | 0.001290s | 0.001310s | 0.001288s | byte equality observed |
| move.test | 10 | 0.001301s | 0.001264s | 0.001297s | byte equality observed |
| nil.test | 10 | 0.001283s | 0.001271s | 0.001284s | byte equality observed |
| none.test | 10 | 0.001272s | 0.001272s | 0.001329s | byte equality observed |
| nview.test | 10 | 0.001277s | 0.001289s | 0.001303s | byte equality observed |
| pop_back.test | 10 | 0.001347s | 0.001341s | 0.001298s | byte equality observed |
| pop_front.test | 8 | 0.001393s | 0.001413s | 0.001297s | byte equality observed |
| push_back.test | 10 | 0.001379s | 0.001342s | 0.001334s | byte equality observed |
| push_front.test | 10 | 0.001374s | 0.001328s | 0.001346s | byte equality observed |
| quick.test | 10 | 0.000616s | 0.001278s | 0.001245s | byte equality observed |
| ref_vector.test | 10 | 0.001309s | 0.001288s | 0.001309s | byte equality observed |
| remove.test | 10 | 0.001353s | 0.001298s | 0.001312s | byte equality observed |
| remove_if.test | 10 | 0.001319s | 0.001304s | 0.001363s | byte equality observed |
| repetitive_view.test | 10 | 0.001256s | 0.001295s | 0.001289s | byte equality observed |
| replace.test | 10 | 0.001355s | 0.001352s | 0.001310s | byte equality observed |
| replace_if.test | 10 | 0.001378s | 0.001425s | 0.001333s | byte equality observed |
| reverse.test | 10 | 0.001329s | 0.001346s | 0.001401s | byte equality observed |
| reverse_fold.test | 9 | 0.001348s | 0.001378s | 0.001362s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001342s | 0.001340s | 0.001358s | byte equality observed |
| reverse_view.test | 10 | 0.001352s | 0.001328s | 0.001362s | byte equality observed |
| segmented_find.test | 10 | 0.001303s | 0.001386s | 0.001264s | byte equality observed |
| segmented_find_if.test | 8 | 0.001320s | 0.001318s | 0.001257s | byte equality observed |
| segmented_fold.test | 10 | 0.001320s | 0.001378s | 0.001372s | byte equality observed |
| segmented_for_each.test | 7 | 0.001301s | 0.001296s | 0.001268s | byte equality observed |
| segmented_iterator_range.test | 8 | 0.001398s | 0.001389s | 0.001327s | byte equality observed |
| set.test | 10 | 0.001345s | 0.001370s | 0.001395s | byte equality observed |
| single_view.test | 9 | 0.001368s | 0.001307s | 0.001318s | byte equality observed |
| std_array.test | 10 | 0.001246s | 0.001293s | 0.001317s | byte equality observed |
| std_pair.test | 10 | 0.001296s | 0.001299s | 0.001297s | byte equality observed |
| std_tuple.test | 10 | 0.001286s | 0.001350s | 0.001255s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001287s | 0.001294s | 0.001299s | byte equality observed |
| swap.test | 8 | 0.001302s | 0.001374s | 0.001253s | byte equality observed |
| transform.test | 10 | 0.001310s | 0.001359s | 0.001334s | byte equality observed |
| transform_view.test | 10 | 0.001388s | 0.001356s | 0.001403s | byte equality observed |
| tuple_comparison.test | 9 | 0.001289s | 0.001311s | 0.001277s | byte equality observed |
| tuple_construction.test | 8 | 0.001252s | 0.001269s | 0.001315s | byte equality observed |
| tuple_conversion.test | 10 | 0.001281s | 0.001274s | 0.001291s | byte equality observed |
| tuple_copy.test | 10 | 0.001313s | 0.001310s | 0.001351s | byte equality observed |
| tuple_element.test | 9 | 0.001302s | 0.001266s | 0.001268s | byte equality observed |
| tuple_hash.test | 10 | 0.001275s | 0.001332s | 0.001303s | byte equality observed |
| tuple_make.test | 10 | 0.001298s | 0.001266s | 0.001340s | byte equality observed |
| tuple_misc.test | 10 | 0.001254s | 0.001347s | 0.001250s | byte equality observed |
| tuple_mutate.test | 10 | 0.001270s | 0.001246s | 0.001277s | byte equality observed |
| tuple_nest.test | 10 | 0.001307s | 0.001284s | 0.001300s | byte equality observed |
| tuple_tie.test | 10 | 0.001281s | 0.001268s | 0.001318s | byte equality observed |
| tuple_traits__maybe_variadic.test | 7 | 0.001284s | 0.001274s | 0.001323s | byte equality observed |
| tuple_traits__no_variadic.test | 8 | 0.001354s | 0.001336s | 0.001251s | byte equality observed |
| unfused.test | 10 | 0.001303s | 0.001293s | 0.001288s | byte equality observed |
| unfused_typed.test | 10 | 0.001326s | 0.001320s | 0.001290s | byte equality observed |
| vector_comparison.test | 10 | 0.001273s | 0.001273s | 0.001278s | byte equality observed |
| vector_construction.test | 10 | 0.001272s | 0.001303s | 0.001312s | byte equality observed |
| vector_conversion.test | 10 | 0.001278s | 0.001322s | 0.001284s | byte equality observed |
| vector_copy.test | 8 | 0.001274s | 0.001301s | 0.001317s | byte equality observed |
| vector_hash.test | 10 | 0.001292s | 0.001347s | 0.001285s | byte equality observed |
| vector_iterator.test | 10 | 0.001331s | 0.001290s | 0.001260s | byte equality observed |
| vector_make.test | 10 | 0.001295s | 0.001300s | 0.001293s | byte equality observed |
| vector_misc.test | 10 | 0.001324s | 0.001327s | 0.001263s | byte equality observed |
| vector_move.test | 10 | 0.001310s | 0.001331s | 0.001273s | byte equality observed |
| vector_mutate.test | 10 | 0.001275s | 0.001284s | 0.001274s | byte equality observed |
| vector_n.test | 10 | 0.001289s | 0.001295s | 0.001321s | byte equality observed |
| vector_nest.test | 10 | 0.001338s | 0.001287s | 0.001256s | byte equality observed |
| vector_tie.test | 10 | 0.001283s | 0.001280s | 0.001282s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001289s | 0.001322s | 0.001327s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001302s | 0.001277s | 0.001281s | byte equality observed |
| vector_value_at.test | 10 | 0.001279s | 0.001323s | 0.001288s | byte equality observed |
| zip.test | 10 | 0.001301s | 0.001321s | 0.001360s | byte equality observed |
| zip2.test | 10 | 0.001285s | 0.001304s | 0.001283s | byte equality observed |
| zip_ignore.test | 10 | 0.001281s | 0.001300s | 0.001335s | byte equality observed |
| zip_view.test | 10 | 0.001295s | 0.001281s | 0.001268s | byte equality observed |
| zip_view2.test | 10 | 0.001267s | 0.001325s | 0.001267s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001280s | 0.001280s | 0.001282s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
