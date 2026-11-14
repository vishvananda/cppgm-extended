# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.463997 | 0.958533 | 1.637780 | 0.001773 | 0.001760 | 0.001834 |
| any.test | RUN | 0.450189 | 0.907772 | 1.517363 | 0.001752 | 0.001765 | 0.001693 |
| clear.test | RUN | 0.319093 | 0.858007 | 1.205215 | 0.001756 | 0.001910 | 0.002055 |
| copy.test | RUN | 0.272892 | 0.655209 | 0.970366 | 0.001916 | 0.001878 | 0.001989 |
| count.test | RUN | 0.315044 | 0.727975 | 1.114367 | 0.001953 | 0.002196 | 0.001988 |
| count_if.test | RUN | 0.300536 | 0.680050 | 0.996781 | 0.001994 | 0.002136 | 0.002032 |
| erase.test | RUN | 0.423470 | 1.173089 | 1.580562 | 0.001896 | 0.002259 | 0.001960 |
| erase_key.test | RUN | 0.409454 | 1.192840 | 1.509708 | 0.001871 | 0.001900 | 0.001681 |
| filter.test | RUN | 0.369141 | 1.017282 | 1.372478 | 0.002086 | 0.002419 | 0.002128 |
| filter_if.test | RUN | 0.425280 | 1.203267 | 1.624508 | 0.001994 | 0.001854 | 0.001970 |
| find.test | RUN | 0.375543 | 0.820502 | 1.208615 | 0.001838 | 0.001747 | 0.001843 |
| find_if.test | RUN | 0.332754 | 0.743481 | 1.172053 | 0.001928 | 0.002154 | 0.002040 |
| fold.test | RUN | 0.385367 | 0.937119 | 1.370163 | 0.002104 | 0.002232 | 0.001852 |
| for_each.test | RUN | 0.337855 | 0.840679 | 1.203200 | 0.001955 | 0.002276 | 0.002096 |
| insert.test | RUN | 0.481295 | 1.484327 | 1.905789 | 0.002024 | 0.002046 | 0.002247 |
| insert_range.test | RUN | 0.483991 | 1.475812 | 1.965327 | 0.001739 | 0.001636 | 0.001859 |
| iter_fold.test | RUN | 0.573481 | 1.422619 | 2.252778 | 0.002208 | 0.002221 | 0.001970 |
| move.test | RUN | 0.288280 | 0.697294 | 0.991989 | 0.001928 | 0.001917 | 0.001704 |
| none.test | RUN | 0.468183 | 0.982537 | 1.615154 | 0.001723 | 0.002112 | 0.002022 |
| pop_back.test | RUN | 0.460230 | 1.284722 | 1.756761 | 0.001860 | 0.001818 | 0.002068 |
| pop_front.test | RUN | 0.385542 | 1.053605 | 1.430109 | 0.001954 | 0.002033 | 0.002048 |
| push_back.test | RUN | 0.473302 | 1.497933 | 1.868301 | 0.002072 | 0.002142 | 0.002218 |
| push_front.test | RUN | 0.422411 | 1.262591 | 1.623440 | 0.002019 | 0.001941 | 0.002040 |
| remove.test | RUN | 0.482211 | 1.382914 | 1.897043 | 0.001851 | 0.002024 | 0.001870 |
| remove_if.test | RUN | 0.431417 | 1.167344 | 1.620681 | 0.001954 | 0.002110 | 0.002128 |
| replace.test | RUN | 0.410424 | 1.119913 | 1.477947 | 0.001952 | 0.002193 | 0.001944 |
| replace_if.test | RUN | 0.396984 | 1.104460 | 1.482749 | 0.002140 | 0.002285 | 0.002042 |
| reverse_fold.test | RUN | 0.640082 | 1.636787 | 2.515571 | 0.002034 | 0.002175 | 0.001876 |
| reverse_iter_fold.test | RUN | 0.667470 | 1.696032 | 2.699649 | 0.002071 | 0.002053 | 0.002104 |
| reverse.test | RUN | 0.410766 | 1.192726 | 1.561890 | 0.001840 | 0.002230 | 0.001987 |
| segmented_for_each.test | RUN | 0.682831 | 2.220717 | 3.081797 | 0.002014 | 0.001892 | 0.001686 |
| segmented_find.test | RUN | 0.428527 | 1.168620 | 1.758284 | 0.001838 | 0.002156 | 0.001843 |
| segmented_find_if.test | RUN | 0.437358 | 1.218553 | 1.778867 | 0.001880 | 0.001905 | 0.001920 |
| segmented_fold.test | RUN | 0.548389 | 1.549952 | 2.134885 | 0.001978 | 0.002314 | 0.002059 |
| transform.test | RUN | 0.419940 | 1.277276 | 1.576984 | 0.001884 | 0.002325 | 0.002066 |
| join.test | RUN | 0.313731 | 0.751333 | 1.169414 | 0.002128 | 0.001960 | 0.002134 |
| zip.test | RUN | 0.569254 | 1.415146 | 2.136217 | 0.002029 | 0.002102 | 0.002092 |
| zip2.test | RUN | 0.538720 | 1.371640 | 2.045886 | 0.001815 | 0.001952 | 0.001896 |
| zip_ignore.test | RUN | 0.418208 | 0.945133 | 1.375895 | 0.002049 | 0.001884 | 0.001730 |
| flatten.test | RUN | 0.393685 | 1.072015 | 1.376027 | 0.001741 | 0.002172 | 0.002074 |
| ticket-5490.test | COMPILE | 0.202036 | 0.483231 | 0.614297 | — | — | — |
| as_deque.test | RUN | 0.389210 | 0.972800 | 1.512233 | 0.002056 | 0.002187 | 0.002057 |
| as_list.test | RUN | 0.439269 | 1.196728 | 1.598254 | 0.001699 | 0.001972 | 0.001708 |
| as_map.test | RUN | 0.418810 | 1.170538 | 1.621068 | 0.001513 | 0.001994 | 0.002115 |
| as_map_assoc.test | RUN | 0.409054 | 1.114454 | 1.525797 | 0.001776 | 0.001974 | 0.002031 |
| as_set.test | RUN | 0.472356 | 1.375685 | 1.879065 | 0.001770 | 0.001891 | 0.001700 |
| as_vector.test | RUN | 0.417428 | 1.270688 | 1.609877 | 0.001807 | 0.001876 | 0.002090 |
| boost_tuple.test | RUN | 0.418945 | 1.182703 | 1.626419 | 0.001927 | 0.001994 | 0.001646 |
| boost_tuple_iterator.test | RUN | 0.306345 | 0.746395 | 0.999030 | 0.001798 | 0.001856 | 0.001972 |
| cons.test | RUN | 0.536725 | 1.366346 | 1.957908 | 0.001859 | 0.001991 | 0.001891 |
| convert_boost_tuple.test | RUN | 0.341490 | 1.049046 | 1.251485 | 0.001953 | 0.002004 | 0.002050 |
| convert_deque.test | RUN | 0.343956 | 1.046376 | 1.280453 | 0.002115 | 0.001945 | 0.001936 |
| convert_list.test | RUN | 0.348047 | 1.076556 | 1.283436 | 0.001842 | 0.002076 | 0.001978 |
| convert_std_pair.test | RUN | 0.357723 | 1.058797 | 1.264156 | 0.002086 | 0.001890 | 0.001908 |
| convert_std_tuple.test | RUN | 0.339280 | 1.031005 | 1.265652 | 0.001898 | 0.001932 | 0.001818 |
| convert_vector.test | RUN | 0.348423 | 1.061112 | 1.270101 | 0.001717 | 0.001863 | 0.001934 |
| filter_view.test | RUN | 0.419925 | 1.117389 | 1.508929 | 0.001716 | 0.001971 | 0.001908 |
| hash.test | RUN | 0.406455 | 0.993796 | 1.470469 | 0.001761 | 0.001943 | 0.001976 |
| io.test | RUN | 0.435399 | 1.377984 | 1.730633 | 0.001894 | 0.002666 | 0.002205 |
| iterator_range.test | RUN | 0.378749 | 1.054025 | 1.446786 | 0.001854 | 0.002240 | 0.001754 |
| joint_view.test | RUN | 0.530927 | 1.771182 | 2.146285 | 0.002146 | 0.002217 | 0.001704 |
| list_comparison.test | RUN | 0.303929 | 0.733227 | 1.110145 | 0.001906 | 0.002322 | 0.001966 |
| list_construction.test | RUN | 0.300887 | 0.727600 | 1.015322 | 0.001733 | 0.001956 | 0.001873 |
| list_copy.test | RUN | 0.451526 | 1.285993 | 1.769328 | 0.001580 | 0.002086 | 0.002000 |
| list_iterator.test | RUN | 0.302024 | 0.707953 | 1.007151 | 0.001928 | 0.002228 | 0.002032 |
| list_hash.test | RUN | 0.392995 | 1.394255 | 1.426203 | 0.001819 | 0.002160 | 0.001776 |
| list_make.test | RUN | 0.315844 | 0.806027 | 1.124838 | 0.002037 | 0.001887 | 0.001942 |
| list_misc.test | RUN | 0.418456 | 0.915433 | 1.373436 | 0.001761 | 0.001966 | 0.001980 |
| list_mutate.test | RUN | 0.279258 | 0.625436 | 0.938310 | 0.001886 | 0.002188 | 0.001855 |
| list_nest.test | RUN | 0.571348 | 1.761110 | 2.413907 | 0.001843 | 0.002196 | 0.001799 |
| list_tie.test | RUN | 0.299860 | 0.749451 | 1.072732 | 0.002012 | 0.002094 | 0.001969 |
| list_value_at.test | RUN | 0.286055 | 0.658506 | 0.963146 | 0.002034 | 0.002017 | 0.002031 |
| deque_comparison.test | RUN | 0.308334 | 0.761737 | 1.108182 | 0.001716 | 0.001820 | 0.001746 |
| deque_construction.test | RUN | 0.298881 | 0.703721 | 1.045767 | 0.002042 | 0.002194 | 0.002034 |
| deque_copy.test | RUN | 0.440002 | 1.193872 | 1.652090 | 0.001948 | 0.001799 | 0.002154 |
| deque_iterator.test | RUN | 0.299559 | 0.801011 | 1.024255 | 0.001900 | 0.001835 | 0.002061 |
| deque_hash.test | RUN | 0.408709 | 1.302642 | 1.503078 | 0.001770 | 0.002176 | 0.002072 |
| deque_is_constructible.test | COMPILE | 0.160144 | 0.356431 | 0.478834 | — | — | — |
| deque_make.test | RUN | 0.303663 | 0.764400 | 1.064122 | 0.001880 | 0.001996 | 0.002070 |
| deque_misc.test | RUN | 0.406595 | 0.900603 | 1.365502 | 0.001678 | 0.001919 | 0.001819 |
| deque_move.test | RUN | 0.308226 | 0.789794 | 1.082950 | 0.002022 | 0.001924 | 0.001662 |
| deque_mutate.test | RUN | 0.286201 | 0.637667 | 0.919612 | 0.001515 | 0.001907 | 0.001621 |
| deque_nest.test | RUN | 0.827893 | 2.760877 | 3.899197 | 0.001922 | 0.002166 | 0.001901 |
| deque_tie.test | RUN | 0.283352 | 0.692492 | 1.036138 | 0.001954 | 0.002045 | 0.002010 |
| deque_value_at.test | RUN | 0.289313 | 0.637720 | 0.967490 | 0.001746 | 0.001970 | 0.001724 |
| front_extended_deque.test | RUN | 0.401709 | 0.955123 | 1.409197 | 0.001905 | 0.002195 | 0.002040 |
| back_extended_deque.test | RUN | 0.401073 | 0.968050 | 1.400090 | 0.001805 | 0.001897 | 0.001858 |
| make_list.test | RUN | 0.313050 | 0.824640 | 1.155547 | 0.001888 | 0.002128 | 0.002006 |
| make_vector.test | RUN | 0.315699 | 0.775930 | 1.064785 | 0.001758 | 0.002158 | 0.002071 |
| map.test | RUN | 0.418303 | 1.173478 | 1.585285 | 0.002073 | 0.001944 | 0.001733 |
| map_comparison.test | RUN | 0.308391 | 0.756591 | 1.042895 | 0.001760 | 0.002041 | 0.002011 |
| map_construction.test | RUN | 0.314145 | 0.750337 | 1.083102 | 0.001892 | 0.001965 | 0.001921 |
| map_copy.test | RUN | 0.374976 | 0.955316 | 1.273689 | 0.002048 | 0.002240 | 0.001930 |
| map_misc.test | RUN | 0.388383 | 0.944479 | 1.286016 | 0.001818 | 0.001843 | 0.002012 |
| map_move.test | RUN | 0.306591 | 0.795505 | 1.091750 | 0.001786 | 0.001892 | 0.001794 |
| map_mutate.test | RUN | 0.281155 | 0.663018 | 0.939752 | 0.001758 | 0.001773 | 0.002029 |
| map_tie.test | RUN | 0.288382 | 0.632645 | 0.961695 | 0.001752 | 0.001974 | 0.002080 |
| nil.test | RUN | 0.200120 | 0.449163 | 0.668648 | 0.001914 | 0.001938 | 0.001974 |
| nview.test | RUN | 0.393475 | 0.953528 | 1.357151 | 0.002008 | 0.001972 | 0.002146 |
| reverse_view.test | RUN | 0.489006 | 1.485015 | 1.995425 | 0.001810 | 0.001958 | 0.002209 |
| segmented_iterator_range.test | RUN | 0.589218 | 1.802739 | 2.474462 | 0.001945 | 0.002128 | 0.002072 |
| set.test | RUN | 0.383867 | 1.027634 | 1.430949 | 0.001921 | 0.002034 | 0.002249 |
| single_view.test | RUN | 0.337497 | 0.953983 | 1.256730 | 0.002054 | 0.002114 | 0.001992 |
| std_pair.test | RUN | 0.436438 | 1.160980 | 1.590851 | 0.001905 | 0.001992 | 0.002202 |
| boost_array.test | RUN | 0.378819 | 0.841283 | 1.236837 | 0.002070 | 0.002204 | 0.001823 |
| array.test | RUN | 0.369698 | 0.852330 | 1.220825 | 0.001826 | 0.002232 | 0.002181 |
| std_array.test | RUN | 0.365143 | 0.868886 | 1.211568 | 0.001990 | 0.002019 | 0.001964 |
| tuple_comparison.test | RUN | 0.329661 | 0.779626 | 1.103571 | 0.001907 | 0.001931 | 0.002234 |
| tuple_construction.test | RUN | 0.338861 | 0.790528 | 1.151115 | 0.002022 | 0.001950 | 0.002076 |
| tuple_conversion.test | RUN | 0.739520 | 2.405139 | 3.575320 | 0.001728 | 0.001910 | 0.002136 |
| tuple_copy.test | RUN | 0.481624 | 1.419310 | 1.956678 | 0.002122 | 0.001788 | 0.001849 |
| tuple_element.test | RUN | 0.310753 | 0.690752 | 1.102368 | 0.001878 | 0.002228 | 0.001711 |
| tuple_make.test | RUN | 0.341900 | 0.830498 | 1.191163 | 0.001951 | 0.002018 | 0.001932 |
| tuple_misc.test | RUN | 0.422734 | 0.988371 | 1.428249 | 0.001770 | 0.002067 | 0.002027 |
| tuple_mutate.test | RUN | 0.293163 | 0.708885 | 1.033409 | 0.002108 | 0.001795 | 0.001949 |
| tuple_nest.test | RUN | 1.226595 | 4.672858 | 6.716420 | 0.001751 | 0.002043 | 0.002054 |
| tuple_hash.test | RUN | 0.430428 | 1.400691 | 1.536513 | 0.001974 | 0.001922 | 0.001739 |
| tuple_tie.test | RUN | 0.329092 | 0.736087 | 1.154787 | 0.001838 | 0.001819 | 0.002014 |
| tuple_traits__maybe_variadic.test | RUN | 0.340499 | 0.899137 | 1.201390 | 0.001876 | 0.001918 | 0.001733 |
| tuple_traits__no_variadic.test | RUN | 0.385327 | 0.958314 | 1.329914 | 0.001857 | 0.002070 | 0.001754 |
| transform_view.test | RUN | 0.503369 | 1.500549 | 1.965148 | 0.001782 | 0.002230 | 0.001738 |
| identity_view.test | RUN | 0.479357 | 1.467778 | 1.877897 | 0.001745 | 0.002136 | 0.001944 |
| vector_comparison.test | RUN | 0.308640 | 0.755030 | 1.126082 | 0.001841 | 0.002163 | 0.002060 |
| vector_construction.test | RUN | 0.310283 | 0.703778 | 1.065504 | 0.001793 | 0.002164 | 0.002286 |
| vector_conversion.test | RUN | 0.883661 | 3.069817 | 4.560574 | 0.002076 | 0.001930 | 0.002058 |
| vector_copy.test | RUN | 0.460919 | 1.344057 | 1.938607 | 0.001890 | 0.001809 | 0.002153 |
| vector_iterator.test | RUN | 0.315918 | 0.791611 | 1.064397 | 0.002004 | 0.001892 | 0.001621 |
| vector_make.test | RUN | 0.308283 | 0.812025 | 1.176519 | 0.001778 | 0.001938 | 0.001804 |
| vector_misc.test | RUN | 0.409316 | 1.030939 | 1.353796 | 0.001765 | 0.001904 | 0.001944 |
| vector_move.test | RUN | 0.304091 | 0.773072 | 1.066552 | 0.002006 | 0.001822 | 0.001953 |
| vector_mutate.test | RUN | 0.282421 | 0.647276 | 1.010309 | 0.001898 | 0.001906 | 0.001932 |
| vector_n.test | RUN | 0.346676 | 0.814767 | 1.292109 | 0.002110 | 0.002330 | 0.002098 |
| vector_nest.test | RUN | 1.246061 | 4.872064 | 6.871755 | 0.001968 | 0.001906 | 0.001924 |
| vector_hash.test | RUN | 0.415165 | 1.323391 | 1.496165 | 0.002102 | 0.002246 | 0.002179 |
| vector_tie.test | RUN | 0.293433 | 0.721926 | 1.102161 | 0.002415 | 0.002166 | 0.002034 |
| vector_traits__maybe_variadic.test | RUN | 0.328442 | 0.857347 | 1.040820 | 0.002068 | 0.002228 | 0.002197 |
| vector_traits__no_variadic.test | RUN | 0.343517 | 0.892610 | 1.166053 | 0.001810 | 0.002081 | 0.001683 |
| vector_value_at.test | RUN | 0.286935 | 0.651357 | 0.985151 | 0.001765 | 0.002092 | 0.001401 |
| zip_view.test | RUN | 0.597883 | 1.574709 | 2.511396 | 0.001890 | 0.002070 | 0.002144 |
| zip_view2.test | RUN | 0.490074 | 1.311631 | 1.868601 | 0.001919 | 0.001818 | 0.001907 |
| zip_view_ignore.test | RUN | 0.452012 | 1.159663 | 1.694550 | 0.001770 | 0.002050 | 0.002038 |
| repetitive_view.test | RUN | 0.777117 | 2.039977 | 3.162514 | 0.001873 | 0.001930 | 0.001950 |
| deduce_sequence.test | RUN | 0.356164 | 0.875963 | 1.298024 | 0.001838 | 0.001901 | 0.002023 |
| adapt_adt_named.test | RUN | 0.442650 | 1.209682 | 1.683202 | 0.001897 | 0.002220 | 0.002195 |
| adapt_adt_named_empty.test | RUN | 0.367729 | 0.964686 | 1.350908 | 0.002102 | 0.002129 | 0.002148 |
| adapt_adt.test | RUN | 0.484174 | 1.569462 | 1.898270 | 0.001952 | 0.002224 | 0.001994 |
| adapt_adt_empty.test | RUN | 0.379504 | 0.967831 | 1.348028 | 0.002202 | 0.002054 | 0.002044 |
| adapt_assoc_adt_named.test | RUN | 0.479627 | 1.253490 | 1.760079 | 0.001792 | 0.002194 | 0.002025 |
| adapt_assoc_adt_named_empty.test | RUN | 0.371545 | 0.969027 | 1.331521 | 0.001682 | 0.002193 | 0.002208 |
| adapt_assoc_adt.test | RUN | 0.497684 | 1.309592 | 1.850357 | 0.001789 | 0.002260 | 0.002132 |
| adapt_assoc_adt_empty.test | RUN | 0.369468 | 0.973688 | 1.318990 | 0.001820 | 0.002102 | 0.002272 |
| adapt_assoc_struct_named.test | RUN | 0.475289 | 1.251128 | 1.713381 | 0.001898 | 0.002010 | 0.002314 |
| adapt_assoc_struct_named_empty.test | RUN | 0.383659 | 0.950846 | 1.354930 | 0.001888 | 0.002028 | 0.002110 |
| adapt_assoc_struct.test | RUN | 0.436748 | 1.168364 | 1.619837 | 0.001656 | 0.002069 | 0.002290 |
| adapt_assoc_struct_empty.test | RUN | 0.365118 | 0.945462 | 1.333158 | 0.002104 | 0.002182 | 0.002314 |
| adapt_assoc_tpl_adt.test | RUN | 0.498252 | 1.354936 | 1.837661 | 0.001860 | 0.002281 | 0.001770 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.386113 | 0.975708 | 1.369931 | 0.002068 | 0.002244 | 0.002216 |
| adapt_assoc_tpl_struct.test | RUN | 0.406154 | 1.163021 | 1.550347 | 0.001754 | 0.001921 | 0.002047 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.372990 | 0.959912 | 1.314460 | 0.001954 | 0.001870 | 0.002085 |
| adapt_struct_named.test | RUN | 0.424432 | 1.221199 | 1.586296 | 0.001993 | 0.001992 | 0.002344 |
| adapt_struct_named_empty.test | RUN | 0.372302 | 1.006122 | 1.346359 | 0.001722 | 0.001962 | 0.002249 |
| adapt_struct.test | RUN | 0.452851 | 1.314015 | 1.784563 | 0.001780 | 0.001961 | 0.002300 |
| adapt_struct_empty.test | RUN | 0.365852 | 0.982827 | 1.313815 | 0.002074 | 0.002066 | 0.002174 |
| adapt_tpl_adt.test | RUN | 0.461829 | 1.531205 | 1.869055 | 0.001880 | 0.002105 | 0.002032 |
| adapt_tpl_adt_empty.test | RUN | 0.379700 | 0.985708 | 1.321964 | 0.001786 | 0.002156 | 0.002206 |
| adapt_tpl_struct.test | RUN | 0.392279 | 1.037314 | 1.508060 | 0.002144 | 0.001957 | 0.002026 |
| adapt_tpl_struct_empty.test | RUN | 0.374700 | 0.998448 | 1.311609 | 0.001761 | 0.001992 | 0.002225 |
| adt_attribute_proxy.test | RUN | 0.422164 | 0.960155 | 1.429445 | 0.001984 | 0.002178 | 0.002162 |
| define_struct.test | RUN | 0.518580 | 1.338365 | 1.839658 | 0.002108 | 0.001939 | 0.002304 |
| define_struct_empty.test | RUN | 0.450638 | 1.090538 | 1.520042 | 0.002018 | 0.001837 | 0.002359 |
| define_struct_move.test | RUN | 0.287160 | 0.632015 | 0.954101 | 0.001896 | 0.002158 | 0.001924 |
| define_struct_inline.test | RUN | 0.538964 | 1.610647 | 2.107754 | 0.001914 | 0.001958 | 0.002202 |
| define_struct_inline_empty.test | RUN | 0.448878 | 1.147211 | 1.521198 | 0.001880 | 0.001931 | 0.002306 |
| define_struct_inline_move.test | RUN | 0.298657 | 0.603546 | 0.972607 | 0.001770 | 0.002166 | 0.002110 |
| define_assoc_struct.test | RUN | 0.499864 | 1.267525 | 1.806098 | 0.002019 | 0.002224 | 0.002116 |
| define_assoc_struct_empty.test | RUN | 0.455735 | 1.105088 | 1.552564 | 0.001819 | 0.002262 | 0.002247 |
| define_assoc_struct_move.test | RUN | 0.277734 | 0.598739 | 0.962018 | 0.002062 | 0.002214 | 0.002222 |
| define_tpl_struct.test | RUN | 0.496142 | 1.276845 | 1.750021 | 0.002099 | 0.001961 | 0.001767 |
| define_tpl_struct_empty.test | RUN | 0.444961 | 1.104897 | 1.514917 | 0.001776 | 0.001883 | 0.002100 |
| define_tpl_struct_move.test | RUN | 0.292339 | 0.657911 | 0.951847 | 0.002024 | 0.001984 | 0.002061 |
| define_tpl_struct_inline.test | RUN | 0.508338 | 1.495898 | 1.949642 | 0.001892 | 0.001991 | 0.002150 |
| define_tpl_struct_inline_empty.test | RUN | 0.453932 | 1.148680 | 1.519810 | 0.002049 | 0.002238 | 0.002200 |
| define_tpl_struct_inline_move.test | RUN | 0.297284 | 0.604053 | 0.980642 | 0.001736 | 0.002146 | 0.001888 |
| define_assoc_tpl_struct.test | RUN | 0.485700 | 1.291202 | 1.749293 | 0.002002 | 0.002232 | 0.002174 |
| define_assoc_tpl_struct_empty.test | RUN | 0.440292 | 1.086746 | 1.524063 | 0.001895 | 0.001964 | 0.002028 |
| define_assoc_tpl_struct_move.test | RUN | 0.284614 | 0.628103 | 0.961549 | 0.001696 | 0.002134 | 0.002244 |
| std_tuple.test | RUN | 0.303673 | 0.719152 | 1.031128 | 0.001945 | 0.001870 | 0.001891 |
| std_tuple_iterator.test | RUN | 0.299204 | 0.753932 | 1.032099 | 0.001968 | 0.002214 | 0.002058 |
| ref_vector.test | RUN | 0.335694 | 0.754789 | 1.098343 | 0.001838 | 0.002202 | 0.002090 |
| flatten_view.test | RUN | 0.355085 | 0.938238 | 1.313540 | 0.001792 | 0.002004 | 0.001749 |
| github-159.test | COMPILE | 0.164376 | 0.363718 | 0.473727 | — | — | — |
| github-176.test | RUN | 0.337836 | 0.829710 | 1.128229 | 0.001881 | 0.001925 | 0.002030 |
| size.test | COMPILE | 0.282458 | 0.606066 | 0.896113 | — | — | — |
| fused.test | RUN | 0.391552 | 0.874441 | 1.338321 | 0.001927 | 0.002157 | 0.002126 |
| fused_function_object.test | RUN | 0.296980 | 0.664007 | 1.016406 | 0.002152 | 0.002208 | 0.002242 |
| fused_procedure.test | RUN | 0.388956 | 0.872574 | 1.272178 | 0.001960 | 0.002068 | 0.002006 |
| unfused.test | RUN | 0.306562 | 0.709096 | 1.020850 | 0.001879 | 0.001900 | 0.001988 |
| unfused_typed.test | RUN | 0.314169 | 0.751842 | 1.037460 | 0.001928 | 0.002154 | 0.002122 |
| make_fused.test | RUN | 0.395595 | 0.872689 | 1.287268 | 0.001945 | 0.001918 | 0.001866 |
| make_fused_function_object.test | RUN | 0.299173 | 0.698992 | 1.030537 | 0.001852 | 0.001974 | 0.001640 |
| make_fused_procedure.test | RUN | 0.378192 | 0.891911 | 1.282267 | 0.001952 | 0.002244 | 0.002178 |
| make_unfused.test | RUN | 0.306709 | 0.697205 | 1.005537 | 0.002011 | 0.002157 | 0.002168 |
| invoke.test | RUN | 0.791540 | 2.431358 | 3.348646 | 0.001844 | 0.001941 | 0.002148 |
| invoke_function_object.test | RUN | 0.363338 | 0.865512 | 1.303585 | 0.002065 | 0.002156 | 0.002112 |
| invoke_procedure.test | RUN | 0.653939 | 1.834319 | 2.547915 | 0.001710 | 0.001977 | 0.001948 |
| swap.test | RUN | 0.448713 | 1.162344 | 1.699685 | 0.001920 | 0.002072 | 0.001891 |
| is_sequence.test | COMPILE | 0.080020 | 0.168786 | 0.206735 | — | — | — |
| is_view.test | COMPILE | 0.079617 | 0.164127 | 0.205643 | — | — | — |
| pair_deque.test | COMPILE | 0.169591 | 0.377889 | 0.535388 | — | — | — |
| pair_list.test | COMPILE | 0.163097 | 0.353885 | 0.529390 | — | — | — |
| pair_map.test | COMPILE | 0.169011 | 0.367010 | 0.484985 | — | — | — |
| pair_set.test | COMPILE | 0.174088 | 0.383669 | 0.512780 | — | — | — |
| pair_vector.test | COMPILE | 0.169286 | 0.362459 | 0.460845 | — | — | — |
| pair_nest.test | COMPILE | 0.070594 | 0.160690 | 0.209895 | — | — | — |
| index_sequence.test | COMPILE | 0.059031 | 0.104717 | 0.147437 | — | — | — |
| and.test | COMPILE | 0.044943 | 0.080178 | 0.102127 | — | — | — |
| tag_of.test | COMPILE | 0.076968 | 0.153288 | 0.184582 | — | — | — |
| unused.test | COMPILE | 0.040623 | 0.101402 | 0.114522 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.069882 | 0.135249 | 0.144787 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.058511 | 0.129126 | 0.151260 | — | — | — |
| quick.test | RUN | 0.202601 | 0.470560 | 0.601794 | 0.000668 | 0.001389 | 0.001628 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.007× | 1.010×, 1.005× | 1.003× |
| clang | 0.997× | 1.001×, 0.993× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001265s | 0.001271s | 0.001296s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001246s | 0.001312s | 0.001313s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001270s | 0.001376s | 0.001267s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001270s | 0.001301s | 0.001277s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001299s | 0.001288s | 0.001282s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001317s | 0.001292s | 0.001290s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001289s | 0.001284s | 0.001273s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001270s | 0.001334s | 0.001256s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001303s | 0.001247s | 0.001301s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001307s | 0.001295s | 0.001281s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001291s | 0.001233s | 0.001339s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001304s | 0.001332s | 0.001304s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001314s | 0.001289s | 0.001291s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001291s | 0.001304s | 0.001249s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001324s | 0.001307s | 0.001306s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001277s | 0.001326s | 0.001262s | byte equality observed |
| adapt_struct.test | 10 | 0.001301s | 0.001345s | 0.001264s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001261s | 0.001267s | 0.001251s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001277s | 0.001288s | 0.001263s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001313s | 0.001359s | 0.001263s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001306s | 0.001299s | 0.001244s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001284s | 0.001323s | 0.001237s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001259s | 0.001298s | 0.001313s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001296s | 0.001275s | 0.001304s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001247s | 0.001235s | 0.001270s | byte equality observed |
| all.test | 10 | 0.001228s | 0.001243s | 0.001274s | byte equality observed |
| any.test | 10 | 0.001238s | 0.001247s | 0.001273s | byte equality observed |
| array.test | 10 | 0.001252s | 0.001271s | 0.001252s | byte equality observed |
| as_deque.test | 10 | 0.001239s | 0.001252s | 0.001274s | byte equality observed |
| as_list.test | 10 | 0.001290s | 0.001306s | 0.001258s | byte equality observed |
| as_map.test | 10 | 0.001259s | 0.001343s | 0.001289s | byte equality observed |
| as_map_assoc.test | 10 | 0.001309s | 0.001256s | 0.001278s | byte equality observed |
| as_set.test | 10 | 0.001291s | 0.001306s | 0.001344s | byte equality observed |
| as_vector.test | 10 | 0.001339s | 0.001304s | 0.001311s | byte equality observed |
| back_extended_deque.test | 10 | 0.001293s | 0.001286s | 0.001275s | byte equality observed |
| boost_array.test | 10 | 0.001252s | 0.001249s | 0.001240s | byte equality observed |
| boost_tuple.test | 10 | 0.001268s | 0.001266s | 0.001282s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001280s | 0.001267s | 0.001236s | byte equality observed |
| clear.test | 10 | 0.001248s | 0.001290s | 0.001307s | byte equality observed |
| cons.test | 10 | 0.001252s | 0.001281s | 0.001285s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001264s | 0.001264s | 0.001243s | byte equality observed |
| convert_deque.test | 10 | 0.001244s | 0.001306s | 0.001233s | byte equality observed |
| convert_list.test | 10 | 0.001296s | 0.001290s | 0.001261s | byte equality observed |
| convert_std_pair.test | 10 | 0.001278s | 0.001244s | 0.001227s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001270s | 0.001298s | 0.001212s | byte equality observed |
| convert_vector.test | 10 | 0.001280s | 0.001229s | 0.001224s | byte equality observed |
| copy.test | 10 | 0.001240s | 0.001237s | 0.001255s | byte equality observed |
| count.test | 10 | 0.001242s | 0.001277s | 0.001239s | byte equality observed |
| count_if.test | 10 | 0.001230s | 0.001304s | 0.001224s | byte equality observed |
| deduce_sequence.test | 9 | 0.001265s | 0.001237s | 0.001298s | byte equality observed |
| define_assoc_struct.test | 5 | 0.001293s | 0.001219s | 0.001344s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001301s | 0.001271s | 0.001285s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001237s | 0.001205s | 0.001284s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001283s | 0.001255s | 0.001341s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001300s | 0.001303s | 0.001284s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001254s | 0.001335s | 0.001228s | byte equality observed |
| define_struct.test | 10 | 0.001266s | 0.001271s | 0.001291s | byte equality observed |
| define_struct_empty.test | 10 | 0.001269s | 0.001264s | 0.001308s | byte equality observed |
| define_struct_inline.test | 10 | 0.001286s | 0.001243s | 0.001279s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001289s | 0.001271s | 0.001285s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001279s | 0.001228s | 0.001233s | byte equality observed |
| define_struct_move.test | 10 | 0.001233s | 0.001278s | 0.001238s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001285s | 0.001241s | 0.001271s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001234s | 0.001285s | 0.001244s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001289s | 0.001247s | 0.001284s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001262s | 0.001270s | 0.001281s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001214s | 0.001260s | 0.001245s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001286s | 0.001272s | 0.001262s | byte equality observed |
| deque_comparison.test | 10 | 0.001251s | 0.001267s | 0.001257s | byte equality observed |
| deque_construction.test | 10 | 0.001229s | 0.001306s | 0.001215s | byte equality observed |
| deque_copy.test | 10 | 0.001245s | 0.001230s | 0.001269s | byte equality observed |
| deque_hash.test | 10 | 0.001222s | 0.001235s | 0.001239s | byte equality observed |
| deque_iterator.test | 10 | 0.001239s | 0.001271s | 0.001266s | byte equality observed |
| deque_make.test | 10 | 0.001278s | 0.001278s | 0.001237s | byte equality observed |
| deque_misc.test | 10 | 0.001220s | 0.001277s | 0.001274s | byte equality observed |
| deque_move.test | 10 | 0.001259s | 0.001270s | 0.001288s | byte equality observed |
| deque_mutate.test | 10 | 0.001305s | 0.001224s | 0.001234s | byte equality observed |
| deque_nest.test | 10 | 0.001273s | 0.001239s | 0.001240s | byte equality observed |
| deque_tie.test | 10 | 0.001226s | 0.001240s | 0.001237s | byte equality observed |
| deque_value_at.test | 10 | 0.001299s | 0.001237s | 0.001327s | byte equality observed |
| erase.test | 10 | 0.001278s | 0.001276s | 0.001300s | byte equality observed |
| erase_key.test | 10 | 0.001325s | 0.001307s | 0.001303s | byte equality observed |
| filter.test | 10 | 0.001286s | 0.001266s | 0.001276s | byte equality observed |
| filter_if.test | 9 | 0.001278s | 0.001259s | 0.001278s | byte equality observed |
| filter_view.test | 10 | 0.001309s | 0.001303s | 0.001278s | byte equality observed |
| find.test | 10 | 0.001253s | 0.001314s | 0.001361s | byte equality observed |
| find_if.test | 10 | 0.001288s | 0.001283s | 0.001263s | byte equality observed |
| flatten.test | 10 | 0.001245s | 0.001310s | 0.001254s | byte equality observed |
| flatten_view.test | 10 | 0.001284s | 0.001334s | 0.001268s | byte equality observed |
| fold.test | 10 | 0.001299s | 0.001277s | 0.001264s | byte equality observed |
| for_each.test | 10 | 0.001311s | 0.001256s | 0.001295s | byte equality observed |
| front_extended_deque.test | 10 | 0.001269s | 0.001226s | 0.001244s | byte equality observed |
| fused.test | 10 | 0.001216s | 0.001272s | 0.001286s | byte equality observed |
| fused_function_object.test | 10 | 0.001266s | 0.001264s | 0.001237s | byte equality observed |
| fused_procedure.test | 10 | 0.001292s | 0.001201s | 0.001308s | byte equality observed |
| github-176.test | 10 | 0.001272s | 0.001249s | 0.001290s | byte equality observed |
| hash.test | 10 | 0.001250s | 0.001243s | 0.001236s | byte equality observed |
| identity_view.test | 10 | 0.001293s | 0.001298s | 0.001286s | byte equality observed |
| insert.test | 10 | 0.001283s | 0.001270s | 0.001352s | byte equality observed |
| insert_range.test | 10 | 0.001291s | 0.001273s | 0.001317s | byte equality observed |
| invoke.test | 10 | 0.001245s | 0.001315s | 0.001252s | byte equality observed |
| invoke_function_object.test | 10 | 0.001273s | 0.001270s | 0.001253s | byte equality observed |
| invoke_procedure.test | 10 | 0.001293s | 0.001263s | 0.001264s | byte equality observed |
| io.test | 10 | 0.001381s | 0.001377s | 0.001405s | byte equality observed |
| iter_fold.test | 10 | 0.001289s | 0.001292s | 0.001290s | byte equality observed |
| iterator_range.test | 10 | 0.001296s | 0.001251s | 0.001281s | byte equality observed |
| join.test | 10 | 0.001235s | 0.001219s | 0.001261s | byte equality observed |
| joint_view.test | 10 | 0.001304s | 0.001321s | 0.001289s | byte equality observed |
| list_comparison.test | 10 | 0.001233s | 0.001267s | 0.001245s | byte equality observed |
| list_construction.test | 10 | 0.001221s | 0.001271s | 0.001257s | byte equality observed |
| list_copy.test | 10 | 0.001244s | 0.001218s | 0.001285s | byte equality observed |
| list_hash.test | 10 | 0.001239s | 0.001262s | 0.001238s | byte equality observed |
| list_iterator.test | 8 | 0.001249s | 0.001295s | 0.001233s | byte equality observed |
| list_make.test | 10 | 0.001266s | 0.001272s | 0.001283s | byte equality observed |
| list_misc.test | 10 | 0.001240s | 0.001237s | 0.001273s | byte equality observed |
| list_mutate.test | 10 | 0.001225s | 0.001200s | 0.001224s | byte equality observed |
| list_nest.test | 10 | 0.001208s | 0.001217s | 0.001308s | byte equality observed |
| list_tie.test | 10 | 0.001232s | 0.001212s | 0.001268s | byte equality observed |
| list_value_at.test | 10 | 0.001276s | 0.001220s | 0.001242s | byte equality observed |
| make_fused.test | 10 | 0.001236s | 0.001223s | 0.001244s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001253s | 0.001272s | 0.001220s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001281s | 0.001219s | 0.001282s | byte equality observed |
| make_list.test | 10 | 0.001251s | 0.001252s | 0.001249s | byte equality observed |
| make_unfused.test | 10 | 0.001220s | 0.001197s | 0.001299s | byte equality observed |
| make_vector.test | 10 | 0.001236s | 0.001233s | 0.001273s | byte equality observed |
| map.test | 10 | 0.001269s | 0.001267s | 0.001262s | byte equality observed |
| map_comparison.test | 10 | 0.001240s | 0.001248s | 0.001246s | byte equality observed |
| map_construction.test | 10 | 0.001226s | 0.001274s | 0.001255s | byte equality observed |
| map_copy.test | 9 | 0.001230s | 0.001237s | 0.001225s | byte equality observed |
| map_misc.test | 10 | 0.001219s | 0.001236s | 0.001256s | byte equality observed |
| map_move.test | 10 | 0.001249s | 0.001282s | 0.001274s | byte equality observed |
| map_mutate.test | 10 | 0.001258s | 0.001258s | 0.001255s | byte equality observed |
| map_tie.test | 10 | 0.001243s | 0.001261s | 0.001269s | byte equality observed |
| move.test | 10 | 0.001265s | 0.001282s | 0.001274s | byte equality observed |
| nil.test | 10 | 0.001245s | 0.001223s | 0.001207s | byte equality observed |
| none.test | 10 | 0.001262s | 0.001247s | 0.001239s | byte equality observed |
| nview.test | 10 | 0.001282s | 0.001258s | 0.001290s | byte equality observed |
| pop_back.test | 10 | 0.001292s | 0.001303s | 0.001321s | byte equality observed |
| pop_front.test | 10 | 0.001312s | 0.001278s | 0.001311s | byte equality observed |
| push_back.test | 10 | 0.001250s | 0.001282s | 0.001293s | byte equality observed |
| push_front.test | 10 | 0.001298s | 0.001283s | 0.001281s | byte equality observed |
| quick.test | 10 | 0.000576s | 0.001212s | 0.001303s | byte equality observed |
| ref_vector.test | 10 | 0.001258s | 0.001249s | 0.001261s | byte equality observed |
| remove.test | 10 | 0.001285s | 0.001298s | 0.001343s | byte equality observed |
| remove_if.test | 10 | 0.001274s | 0.001348s | 0.001275s | byte equality observed |
| repetitive_view.test | 10 | 0.001238s | 0.001215s | 0.001251s | byte equality observed |
| replace.test | 10 | 0.001257s | 0.001253s | 0.001250s | byte equality observed |
| replace_if.test | 10 | 0.001267s | 0.001256s | 0.001311s | byte equality observed |
| reverse.test | 10 | 0.001293s | 0.001273s | 0.001364s | byte equality observed |
| reverse_fold.test | 10 | 0.001290s | 0.001298s | 0.001280s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001295s | 0.001232s | 0.001318s | byte equality observed |
| reverse_view.test | 10 | 0.001290s | 0.001253s | 0.001287s | byte equality observed |
| segmented_find.test | 10 | 0.001285s | 0.001221s | 0.001254s | byte equality observed |
| segmented_find_if.test | 10 | 0.001240s | 0.001271s | 0.001239s | byte equality observed |
| segmented_fold.test | 10 | 0.001282s | 0.001285s | 0.001229s | byte equality observed |
| segmented_for_each.test | 10 | 0.001307s | 0.001294s | 0.001351s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001290s | 0.001345s | 0.001297s | byte equality observed |
| set.test | 10 | 0.001257s | 0.001301s | 0.001293s | byte equality observed |
| single_view.test | 10 | 0.001322s | 0.001276s | 0.001369s | byte equality observed |
| std_array.test | 10 | 0.001250s | 0.001279s | 0.001228s | byte equality observed |
| std_pair.test | 10 | 0.001296s | 0.001283s | 0.001300s | byte equality observed |
| std_tuple.test | 10 | 0.001276s | 0.001251s | 0.001250s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001232s | 0.001195s | 0.001226s | byte equality observed |
| swap.test | 10 | 0.001303s | 0.001194s | 0.001256s | byte equality observed |
| transform.test | 10 | 0.001282s | 0.001276s | 0.001338s | byte equality observed |
| transform_view.test | 10 | 0.001328s | 0.001279s | 0.001270s | byte equality observed |
| tuple_comparison.test | 10 | 0.001242s | 0.001223s | 0.001233s | byte equality observed |
| tuple_construction.test | 10 | 0.001253s | 0.001223s | 0.001235s | byte equality observed |
| tuple_conversion.test | 10 | 0.001267s | 0.001230s | 0.001264s | byte equality observed |
| tuple_copy.test | 10 | 0.001251s | 0.001250s | 0.001233s | byte equality observed |
| tuple_element.test | 10 | 0.001256s | 0.001230s | 0.001236s | byte equality observed |
| tuple_hash.test | 10 | 0.001261s | 0.001237s | 0.001279s | byte equality observed |
| tuple_make.test | 10 | 0.001243s | 0.001286s | 0.001275s | byte equality observed |
| tuple_misc.test | 10 | 0.001241s | 0.001230s | 0.001234s | byte equality observed |
| tuple_mutate.test | 10 | 0.001277s | 0.001230s | 0.001275s | byte equality observed |
| tuple_nest.test | 10 | 0.001241s | 0.001299s | 0.001248s | byte equality observed |
| tuple_tie.test | 10 | 0.001249s | 0.001240s | 0.001245s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001235s | 0.001264s | 0.001276s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001282s | 0.001201s | 0.001237s | byte equality observed |
| unfused.test | 10 | 0.001280s | 0.001206s | 0.001253s | byte equality observed |
| unfused_typed.test | 10 | 0.001283s | 0.001240s | 0.001241s | byte equality observed |
| vector_comparison.test | 10 | 0.001235s | 0.001247s | 0.001260s | byte equality observed |
| vector_construction.test | 10 | 0.001255s | 0.001256s | 0.001265s | byte equality observed |
| vector_conversion.test | 10 | 0.001224s | 0.001226s | 0.001201s | byte equality observed |
| vector_copy.test | 9 | 0.001291s | 0.001289s | 0.001221s | byte equality observed |
| vector_hash.test | 10 | 0.001235s | 0.001308s | 0.001220s | byte equality observed |
| vector_iterator.test | 10 | 0.001249s | 0.001297s | 0.001253s | byte equality observed |
| vector_make.test | 10 | 0.001268s | 0.001251s | 0.001271s | byte equality observed |
| vector_misc.test | 10 | 0.001273s | 0.001260s | 0.001279s | byte equality observed |
| vector_move.test | 10 | 0.001240s | 0.001271s | 0.001277s | byte equality observed |
| vector_mutate.test | 10 | 0.001248s | 0.001255s | 0.001223s | byte equality observed |
| vector_n.test | 10 | 0.001263s | 0.001273s | 0.001280s | byte equality observed |
| vector_nest.test | 10 | 0.001233s | 0.001300s | 0.001234s | byte equality observed |
| vector_tie.test | 10 | 0.001209s | 0.001250s | 0.001216s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001254s | 0.001293s | 0.001211s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001252s | 0.001233s | 0.001233s | byte equality observed |
| vector_value_at.test | 10 | 0.001235s | 0.001255s | 0.001224s | byte equality observed |
| zip.test | 10 | 0.001232s | 0.001212s | 0.001256s | byte equality observed |
| zip2.test | 10 | 0.001251s | 0.001251s | 0.001235s | byte equality observed |
| zip_ignore.test | 10 | 0.001230s | 0.001250s | 0.001281s | byte equality observed |
| zip_view.test | 10 | 0.001265s | 0.001248s | 0.001237s | byte equality observed |
| zip_view2.test | 9 | 0.001258s | 0.001259s | 0.001266s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001225s | 0.001255s | 0.001219s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
