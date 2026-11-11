# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.536644 | 0.947774 | 1.619334 | 0.001870 | 0.001774 | 0.001824 |
| any.test | RUN | 0.525295 | 0.895458 | 1.529222 | 0.001998 | 0.001788 | 0.001912 |
| clear.test | RUN | 0.365584 | 0.862940 | 1.209454 | 0.001856 | 0.002034 | 0.002164 |
| copy.test | RUN | 0.323117 | 0.656860 | 0.982717 | 0.001996 | 0.002239 | 0.002213 |
| count.test | RUN | 0.361872 | 0.734217 | 1.116784 | 0.002140 | 0.002064 | 0.002098 |
| count_if.test | RUN | 0.340716 | 0.677573 | 0.985082 | 0.001929 | 0.002034 | 0.002020 |
| erase.test | RUN | 0.472593 | 1.201149 | 1.605306 | 0.001764 | 0.002225 | 0.001848 |
| erase_key.test | RUN | 0.452438 | 1.166596 | 1.518279 | 0.002056 | 0.001984 | 0.002296 |
| filter.test | RUN | 0.406571 | 0.981420 | 1.394976 | 0.002022 | 0.002264 | 0.002088 |
| filter_if.test | RUN | 0.454524 | 1.168766 | 1.648596 | 0.001882 | 0.001907 | 0.002141 |
| find.test | RUN | 0.399397 | 0.819255 | 1.238364 | 0.001667 | 0.001856 | 0.002078 |
| find_if.test | RUN | 0.387601 | 0.780937 | 1.188776 | 0.002052 | 0.001864 | 0.002211 |
| fold.test | RUN | 0.416861 | 0.930342 | 1.423857 | 0.001973 | 0.002270 | 0.002082 |
| for_each.test | RUN | 0.381300 | 0.841801 | 1.233159 | 0.001882 | 0.002210 | 0.002172 |
| insert.test | RUN | 0.520804 | 1.493301 | 1.973404 | 0.001888 | 0.002126 | 0.002182 |
| insert_range.test | RUN | 0.518539 | 1.493251 | 1.956322 | 0.001841 | 0.002074 | 0.002056 |
| iter_fold.test | RUN | 0.643920 | 1.445015 | 2.314882 | 0.002134 | 0.002308 | 0.002173 |
| move.test | RUN | 0.326060 | 0.691369 | 1.061359 | 0.001950 | 0.001950 | 0.002019 |
| none.test | RUN | 0.531942 | 1.003905 | 1.574948 | 0.001838 | 0.002156 | 0.001894 |
| pop_back.test | RUN | 0.503315 | 1.290729 | 1.807003 | 0.001958 | 0.001929 | 0.002070 |
| pop_front.test | RUN | 0.434394 | 1.065167 | 1.443070 | 0.002102 | 0.001885 | 0.001751 |
| push_back.test | RUN | 0.527225 | 1.513496 | 1.887870 | 0.002165 | 0.002166 | 0.002133 |
| push_front.test | RUN | 0.467441 | 1.263795 | 1.639751 | 0.002066 | 0.001902 | 0.002071 |
| remove.test | RUN | 0.518064 | 1.384436 | 1.881964 | 0.002002 | 0.001842 | 0.002281 |
| remove_if.test | RUN | 0.463238 | 1.168411 | 1.675942 | 0.001918 | 0.001889 | 0.002125 |
| replace.test | RUN | 0.453349 | 1.141736 | 1.540988 | 0.001696 | 0.002108 | 0.001966 |
| replace_if.test | RUN | 0.441967 | 1.160049 | 1.474244 | 0.001844 | 0.002142 | 0.002242 |
| reverse_fold.test | RUN | 0.696899 | 1.597639 | 2.592852 | 0.001966 | 0.001902 | 0.001947 |
| reverse_iter_fold.test | RUN | 0.708744 | 1.672130 | 2.672013 | 0.002050 | 0.001993 | 0.001970 |
| reverse.test | RUN | 0.459897 | 1.177580 | 1.519572 | 0.002117 | 0.002304 | 0.001736 |
| segmented_for_each.test | RUN | 0.722550 | 2.257183 | 3.089146 | 0.001950 | 0.001984 | 0.002043 |
| segmented_find.test | RUN | 0.498048 | 1.166958 | 1.760100 | 0.001737 | 0.001693 | 0.001713 |
| segmented_find_if.test | RUN | 0.489309 | 1.189097 | 1.814848 | 0.002076 | 0.002302 | 0.002081 |
| segmented_fold.test | RUN | 0.590525 | 1.594521 | 2.235129 | 0.002170 | 0.002125 | 0.002028 |
| transform.test | RUN | 0.468557 | 1.234709 | 1.612988 | 0.001837 | 0.002252 | 0.002372 |
| join.test | RUN | 0.362866 | 0.741547 | 1.142521 | 0.001857 | 0.002182 | 0.001952 |
| zip.test | RUN | 0.601131 | 1.456302 | 2.117080 | 0.002184 | 0.002185 | 0.002167 |
| zip2.test | RUN | 0.607804 | 1.384897 | 2.034511 | 0.001598 | 0.002126 | 0.002007 |
| zip_ignore.test | RUN | 0.461166 | 0.983127 | 1.402302 | 0.001957 | 0.001982 | 0.002091 |
| flatten.test | RUN | 0.430698 | 1.084932 | 1.423581 | 0.001889 | 0.002205 | 0.002124 |
| ticket-5490.test | COMPILE | 0.244330 | 0.445561 | 0.658907 | — | — | — |
| as_deque.test | RUN | 0.426997 | 0.988372 | 1.517275 | 0.002052 | 0.002130 | 0.002080 |
| as_list.test | RUN | 0.463965 | 1.194198 | 1.645254 | 0.001920 | 0.002293 | 0.002298 |
| as_map.test | RUN | 0.462125 | 1.201772 | 1.617252 | 0.001758 | 0.002066 | 0.001664 |
| as_map_assoc.test | RUN | 0.458610 | 1.137234 | 1.545108 | 0.001775 | 0.002248 | 0.002102 |
| as_set.test | RUN | 0.508974 | 1.382212 | 1.909474 | 0.001918 | 0.001861 | 0.001962 |
| as_vector.test | RUN | 0.469617 | 1.239438 | 1.696693 | 0.001858 | 0.002000 | 0.001924 |
| boost_tuple.test | RUN | 0.469723 | 1.226119 | 1.566702 | 0.001738 | 0.002020 | 0.001897 |
| boost_tuple_iterator.test | RUN | 0.341640 | 0.757386 | 1.035041 | 0.001836 | 0.001960 | 0.002421 |
| cons.test | RUN | 0.588041 | 1.391409 | 2.001626 | 0.002006 | 0.002040 | 0.002208 |
| convert_boost_tuple.test | RUN | 0.390901 | 1.061581 | 1.293089 | 0.001930 | 0.002064 | 0.001872 |
| convert_deque.test | RUN | 0.393379 | 1.034549 | 1.299957 | 0.001924 | 0.001955 | 0.002036 |
| convert_list.test | RUN | 0.385563 | 1.059751 | 1.289160 | 0.001846 | 0.001734 | 0.001825 |
| convert_std_pair.test | RUN | 0.421949 | 1.067135 | 1.278114 | 0.001928 | 0.002113 | 0.002145 |
| convert_std_tuple.test | RUN | 0.383347 | 1.032842 | 1.294594 | 0.001838 | 0.002002 | 0.001732 |
| convert_vector.test | RUN | 0.394357 | 1.046679 | 1.254986 | 0.001901 | 0.002052 | 0.001835 |
| filter_view.test | RUN | 0.457236 | 1.118494 | 1.540882 | 0.001813 | 0.002206 | 0.002464 |
| hash.test | RUN | 0.462645 | 0.994110 | 1.517711 | 0.001980 | 0.002022 | 0.001743 |
| io.test | RUN | 0.468462 | 1.376869 | 1.723377 | 0.001948 | 0.002467 | 0.002363 |
| iterator_range.test | RUN | 0.429764 | 1.065294 | 1.467621 | 0.001906 | 0.002334 | 0.002089 |
| joint_view.test | RUN | 0.557072 | 1.762096 | 2.170864 | 0.002034 | 0.001816 | 0.001882 |
| list_comparison.test | RUN | 0.342825 | 0.738879 | 1.122162 | 0.002080 | 0.002222 | 0.001841 |
| list_construction.test | RUN | 0.345321 | 0.727932 | 1.037762 | 0.001926 | 0.001998 | 0.001673 |
| list_copy.test | RUN | 0.500300 | 1.289040 | 1.762533 | 0.001976 | 0.001829 | 0.002096 |
| list_iterator.test | RUN | 0.348673 | 0.770245 | 1.030532 | 0.001868 | 0.001857 | 0.002310 |
| list_hash.test | RUN | 0.458109 | 1.355495 | 1.483896 | 0.001932 | 0.002278 | 0.002050 |
| list_make.test | RUN | 0.359184 | 0.808631 | 1.119413 | 0.002166 | 0.001798 | 0.001998 |
| list_misc.test | RUN | 0.456095 | 0.929476 | 1.313565 | 0.001793 | 0.002071 | 0.002032 |
| list_mutate.test | RUN | 0.335238 | 0.633045 | 0.963275 | 0.001701 | 0.002198 | 0.001869 |
| list_nest.test | RUN | 0.608443 | 1.751402 | 2.441078 | 0.001767 | 0.001903 | 0.002072 |
| list_tie.test | RUN | 0.346207 | 0.719492 | 0.991102 | 0.001861 | 0.002200 | 0.002092 |
| list_value_at.test | RUN | 0.327948 | 0.693204 | 1.047636 | 0.001818 | 0.002227 | 0.002056 |
| deque_comparison.test | RUN | 0.350341 | 0.752040 | 1.019245 | 0.001792 | 0.001846 | 0.002102 |
| deque_construction.test | RUN | 0.343783 | 0.711751 | 1.059586 | 0.001928 | 0.002198 | 0.002080 |
| deque_copy.test | RUN | 0.474874 | 1.211005 | 1.687400 | 0.001843 | 0.001906 | 0.002142 |
| deque_iterator.test | RUN | 0.346261 | 0.760282 | 1.016686 | 0.002050 | 0.001928 | 0.002094 |
| deque_hash.test | RUN | 0.469239 | 1.338819 | 1.523306 | 0.001778 | 0.002119 | 0.002262 |
| deque_is_constructible.test | COMPILE | 0.192066 | 0.359447 | 0.484038 | — | — | — |
| deque_make.test | RUN | 0.341583 | 0.743315 | 1.101274 | 0.002032 | 0.001986 | 0.002092 |
| deque_misc.test | RUN | 0.451246 | 0.927014 | 1.300001 | 0.001795 | 0.001833 | 0.002092 |
| deque_move.test | RUN | 0.353971 | 0.781592 | 1.046281 | 0.001780 | 0.001854 | 0.002000 |
| deque_mutate.test | RUN | 0.316492 | 0.626330 | 0.907864 | 0.002045 | 0.002240 | 0.002066 |
| deque_nest.test | RUN | 0.890819 | 2.804170 | 3.844869 | 0.002077 | 0.001885 | 0.002172 |
| deque_tie.test | RUN | 0.326147 | 0.682666 | 0.976249 | 0.002030 | 0.001942 | 0.002184 |
| deque_value_at.test | RUN | 0.321541 | 0.665839 | 0.993339 | 0.001880 | 0.001728 | 0.002121 |
| front_extended_deque.test | RUN | 0.481985 | 1.007322 | 1.448847 | 0.002046 | 0.002241 | 0.002308 |
| back_extended_deque.test | RUN | 0.457152 | 0.975639 | 1.406636 | 0.001948 | 0.002033 | 0.001950 |
| make_list.test | RUN | 0.354579 | 0.774375 | 1.116604 | 0.002190 | 0.001815 | 0.002048 |
| make_vector.test | RUN | 0.368034 | 0.807488 | 1.149606 | 0.001889 | 0.001793 | 0.002035 |
| map.test | RUN | 0.450708 | 1.184591 | 1.549391 | 0.002044 | 0.002144 | 0.002062 |
| map_comparison.test | RUN | 0.364986 | 0.798598 | 1.084029 | 0.002094 | 0.001977 | 0.002120 |
| map_construction.test | RUN | 0.353350 | 0.735999 | 1.094011 | 0.001911 | 0.002211 | 0.002213 |
| map_copy.test | RUN | 0.420746 | 0.949462 | 1.290445 | 0.002052 | 0.002018 | 0.002139 |
| map_misc.test | RUN | 0.450454 | 0.957293 | 1.335010 | 0.001927 | 0.001995 | 0.002068 |
| map_move.test | RUN | 0.354462 | 0.808365 | 1.098989 | 0.002080 | 0.001840 | 0.001898 |
| map_mutate.test | RUN | 0.316656 | 0.638389 | 0.930122 | 0.001805 | 0.001942 | 0.001766 |
| map_tie.test | RUN | 0.338827 | 0.661731 | 0.956393 | 0.001902 | 0.002180 | 0.002030 |
| nil.test | RUN | 0.230136 | 0.462531 | 0.675183 | 0.001936 | 0.002054 | 0.001894 |
| nview.test | RUN | 0.445867 | 0.947066 | 1.365814 | 0.002184 | 0.002365 | 0.002048 |
| reverse_view.test | RUN | 0.533878 | 1.496012 | 2.015077 | 0.002073 | 0.002232 | 0.002042 |
| segmented_iterator_range.test | RUN | 0.637848 | 1.770138 | 2.446094 | 0.002140 | 0.001916 | 0.001909 |
| set.test | RUN | 0.413849 | 1.059168 | 1.413571 | 0.001980 | 0.001940 | 0.002094 |
| single_view.test | RUN | 0.384837 | 0.926733 | 1.278484 | 0.002134 | 0.001996 | 0.001774 |
| std_pair.test | RUN | 0.484864 | 1.163541 | 1.579054 | 0.001954 | 0.002032 | 0.001681 |
| boost_array.test | RUN | 0.429277 | 0.824864 | 1.272607 | 0.001958 | 0.002011 | 0.002032 |
| array.test | RUN | 0.412211 | 0.859403 | 1.208633 | 0.001913 | 0.001807 | 0.001970 |
| std_array.test | RUN | 0.420261 | 0.852680 | 1.226147 | 0.001960 | 0.002084 | 0.001981 |
| tuple_comparison.test | RUN | 0.370652 | 0.797137 | 1.173399 | 0.002172 | 0.002078 | 0.001886 |
| tuple_construction.test | RUN | 0.384245 | 0.776892 | 1.180305 | 0.002094 | 0.002212 | 0.001650 |
| tuple_conversion.test | RUN | 0.788999 | 2.378814 | 3.583999 | 0.001840 | 0.002120 | 0.002072 |
| tuple_copy.test | RUN | 0.543468 | 1.408667 | 1.965123 | 0.001813 | 0.001751 | 0.001956 |
| tuple_element.test | RUN | 0.354617 | 0.754441 | 1.067910 | 0.001902 | 0.001948 | 0.001775 |
| tuple_make.test | RUN | 0.386415 | 0.855263 | 1.162211 | 0.002063 | 0.002196 | 0.001754 |
| tuple_misc.test | RUN | 0.466574 | 0.998596 | 1.460845 | 0.002036 | 0.002025 | 0.002070 |
| tuple_mutate.test | RUN | 0.343005 | 0.660060 | 1.022137 | 0.002262 | 0.001816 | 0.002332 |
| tuple_nest.test | RUN | 1.331733 | 4.700794 | 6.620881 | 0.002076 | 0.001844 | 0.002185 |
| tuple_hash.test | RUN | 0.476457 | 1.338716 | 1.548322 | 0.002049 | 0.001970 | 0.002072 |
| tuple_tie.test | RUN | 0.378880 | 0.769189 | 1.093480 | 0.001950 | 0.002112 | 0.002200 |
| tuple_traits__maybe_variadic.test | RUN | 0.386538 | 0.941648 | 1.239929 | 0.001935 | 0.002137 | 0.001908 |
| tuple_traits__no_variadic.test | RUN | 0.426639 | 0.901985 | 1.301662 | 0.001860 | 0.002239 | 0.002246 |
| transform_view.test | RUN | 0.544539 | 1.489615 | 1.968940 | 0.002086 | 0.001907 | 0.002063 |
| identity_view.test | RUN | 0.519314 | 1.398572 | 1.832013 | 0.001840 | 0.001946 | 0.002259 |
| vector_comparison.test | RUN | 0.362497 | 0.821630 | 1.083786 | 0.001950 | 0.002012 | 0.002138 |
| vector_construction.test | RUN | 0.358194 | 0.708742 | 1.035173 | 0.001808 | 0.001948 | 0.002153 |
| vector_conversion.test | RUN | 0.941173 | 3.123024 | 4.579604 | 0.002088 | 0.002221 | 0.002117 |
| vector_copy.test | RUN | 0.518568 | 1.415226 | 1.904793 | 0.001784 | 0.001790 | 0.001820 |
| vector_iterator.test | RUN | 0.335412 | 0.739165 | 1.078537 | 0.001852 | 0.001548 | 0.001812 |
| vector_make.test | RUN | 0.361920 | 0.792001 | 1.132233 | 0.001937 | 0.001910 | 0.002137 |
| vector_misc.test | RUN | 0.438649 | 1.021943 | 1.396819 | 0.001721 | 0.002190 | 0.002144 |
| vector_move.test | RUN | 0.350218 | 0.768481 | 1.074620 | 0.001880 | 0.002184 | 0.001846 |
| vector_mutate.test | RUN | 0.327807 | 0.645816 | 0.949154 | 0.001747 | 0.001852 | 0.002075 |
| vector_n.test | RUN | 0.382308 | 0.812980 | 1.257637 | 0.002088 | 0.002086 | 0.002152 |
| vector_nest.test | RUN | 1.308555 | 4.997497 | 6.764415 | 0.002108 | 0.001996 | 0.001648 |
| vector_hash.test | RUN | 0.462180 | 1.338672 | 1.465100 | 0.001926 | 0.002226 | 0.002339 |
| vector_tie.test | RUN | 0.342249 | 0.747949 | 1.050689 | 0.002044 | 0.002217 | 0.002194 |
| vector_traits__maybe_variadic.test | RUN | 0.360557 | 0.807725 | 1.140580 | 0.002002 | 0.002192 | 0.002100 |
| vector_traits__no_variadic.test | RUN | 0.399675 | 0.906269 | 1.145302 | 0.001891 | 0.002146 | 0.001780 |
| vector_value_at.test | RUN | 0.341514 | 0.655974 | 0.992033 | 0.001905 | 0.001802 | 0.001742 |
| zip_view.test | RUN | 0.640381 | 1.690665 | 2.446575 | 0.002048 | 0.001897 | 0.002036 |
| zip_view2.test | RUN | 0.532627 | 1.203499 | 1.860384 | 0.001943 | 0.001952 | 0.002148 |
| zip_view_ignore.test | RUN | 0.505786 | 1.097521 | 1.693005 | 0.001666 | 0.001920 | 0.001857 |
| repetitive_view.test | RUN | 0.864459 | 2.049701 | 3.128422 | 0.002070 | 0.002122 | 0.001914 |
| deduce_sequence.test | RUN | 0.411607 | 0.898038 | 1.284277 | 0.002074 | 0.001801 | 0.001978 |
| adapt_adt_named.test | RUN | 0.502331 | 1.211327 | 1.655085 | 0.001938 | 0.002270 | 0.002146 |
| adapt_adt_named_empty.test | RUN | 0.438648 | 0.980630 | 1.352986 | 0.002114 | 0.002354 | 0.001961 |
| adapt_adt.test | RUN | 0.524914 | 1.577529 | 1.896250 | 0.001894 | 0.002220 | 0.002143 |
| adapt_adt_empty.test | RUN | 0.424892 | 0.975744 | 1.342638 | 0.002195 | 0.002056 | 0.001808 |
| adapt_assoc_adt_named.test | RUN | 0.543551 | 1.256499 | 1.738745 | 0.001820 | 0.002126 | 0.002254 |
| adapt_assoc_adt_named_empty.test | RUN | 0.434071 | 0.950010 | 1.277747 | 0.001866 | 0.002032 | 0.002022 |
| adapt_assoc_adt.test | RUN | 0.543724 | 1.335550 | 1.788892 | 0.001857 | 0.002154 | 0.002240 |
| adapt_assoc_adt_empty.test | RUN | 0.419714 | 0.966843 | 1.298292 | 0.002037 | 0.002161 | 0.002172 |
| adapt_assoc_struct_named.test | RUN | 0.524407 | 1.247865 | 1.701127 | 0.002046 | 0.002287 | 0.002228 |
| adapt_assoc_struct_named_empty.test | RUN | 0.413429 | 0.963522 | 1.314330 | 0.002043 | 0.002113 | 0.001961 |
| adapt_assoc_struct.test | RUN | 0.482269 | 1.195433 | 1.541083 | 0.002125 | 0.002294 | 0.001988 |
| adapt_assoc_struct_empty.test | RUN | 0.415100 | 0.962329 | 1.290050 | 0.001867 | 0.002110 | 0.002279 |
| adapt_assoc_tpl_adt.test | RUN | 0.562734 | 1.347949 | 1.817794 | 0.001842 | 0.002500 | 0.002050 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.441663 | 0.974750 | 1.335749 | 0.001914 | 0.002220 | 0.002225 |
| adapt_assoc_tpl_struct.test | RUN | 0.445313 | 1.161206 | 1.551389 | 0.001817 | 0.002014 | 0.001812 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.416047 | 0.989987 | 1.331629 | 0.002104 | 0.002188 | 0.001940 |
| adapt_struct_named.test | RUN | 0.471952 | 1.184626 | 1.527845 | 0.001850 | 0.001939 | 0.002134 |
| adapt_struct_named_empty.test | RUN | 0.419187 | 0.956612 | 1.305376 | 0.001858 | 0.002278 | 0.001810 |
| adapt_struct.test | RUN | 0.500032 | 1.355959 | 1.714828 | 0.001961 | 0.002256 | 0.002108 |
| adapt_struct_empty.test | RUN | 0.413219 | 1.005355 | 1.332923 | 0.002043 | 0.002197 | 0.002303 |
| adapt_tpl_adt.test | RUN | 0.513196 | 1.526992 | 1.818529 | 0.001913 | 0.002092 | 0.001829 |
| adapt_tpl_adt_empty.test | RUN | 0.425985 | 0.972411 | 1.281900 | 0.001946 | 0.002050 | 0.002225 |
| adapt_tpl_struct.test | RUN | 0.435531 | 1.144813 | 1.448587 | 0.001944 | 0.002160 | 0.001849 |
| adapt_tpl_struct_empty.test | RUN | 0.423112 | 1.026054 | 1.312986 | 0.001761 | 0.002058 | 0.002174 |
| adt_attribute_proxy.test | RUN | 0.485063 | 0.966647 | 1.436196 | 0.002016 | 0.002440 | 0.002041 |
| define_struct.test | RUN | 0.552052 | 1.361251 | 1.839556 | 0.002096 | 0.001955 | 0.002242 |
| define_struct_empty.test | RUN | 0.497710 | 1.078085 | 1.484976 | 0.001968 | 0.002214 | 0.002186 |
| define_struct_move.test | RUN | 0.335962 | 0.641381 | 0.931129 | 0.001933 | 0.002244 | 0.001922 |
| define_struct_inline.test | RUN | 0.597374 | 1.594649 | 2.035874 | 0.002023 | 0.001896 | 0.002336 |
| define_struct_inline_empty.test | RUN | 0.504302 | 1.089447 | 1.555115 | 0.001859 | 0.002048 | 0.002231 |
| define_struct_inline_move.test | RUN | 0.340714 | 0.680402 | 0.957635 | 0.001716 | 0.001982 | 0.002148 |
| define_assoc_struct.test | RUN | 0.543666 | 1.290004 | 1.760990 | 0.001937 | 0.002242 | 0.002122 |
| define_assoc_struct_empty.test | RUN | 0.498249 | 1.093188 | 1.496070 | 0.001799 | 0.002076 | 0.002144 |
| define_assoc_struct_move.test | RUN | 0.333732 | 0.640001 | 0.944232 | 0.001901 | 0.002040 | 0.002061 |
| define_tpl_struct.test | RUN | 0.544897 | 1.324786 | 1.702935 | 0.002160 | 0.002132 | 0.002013 |
| define_tpl_struct_empty.test | RUN | 0.505124 | 1.105331 | 1.518061 | 0.002016 | 0.002134 | 0.002190 |
| define_tpl_struct_move.test | RUN | 0.342965 | 0.615972 | 0.925902 | 0.002027 | 0.002162 | 0.001867 |
| define_tpl_struct_inline.test | RUN | 0.571479 | 1.484893 | 1.912350 | 0.002089 | 0.001901 | 0.001868 |
| define_tpl_struct_inline_empty.test | RUN | 0.490382 | 1.112664 | 1.488888 | 0.001870 | 0.002184 | 0.002111 |
| define_tpl_struct_inline_move.test | RUN | 0.342031 | 0.626121 | 0.974763 | 0.001840 | 0.002039 | 0.002078 |
| define_assoc_tpl_struct.test | RUN | 0.564281 | 1.304038 | 1.767771 | 0.002141 | 0.002276 | 0.001968 |
| define_assoc_tpl_struct_empty.test | RUN | 0.499276 | 1.096027 | 1.467026 | 0.002038 | 0.001828 | 0.001972 |
| define_assoc_tpl_struct_move.test | RUN | 0.333508 | 0.648916 | 0.958763 | 0.001920 | 0.002065 | 0.001976 |
| std_tuple.test | RUN | 0.340382 | 0.713862 | 1.031297 | 0.001918 | 0.002224 | 0.001950 |
| std_tuple_iterator.test | RUN | 0.346838 | 0.751268 | 1.053662 | 0.002004 | 0.002235 | 0.002107 |
| ref_vector.test | RUN | 0.396893 | 0.755908 | 1.083615 | 0.001942 | 0.002024 | 0.001970 |
| flatten_view.test | RUN | 0.418714 | 0.969110 | 1.278840 | 0.001908 | 0.002278 | 0.002012 |
| github-159.test | COMPILE | 0.191301 | 0.348067 | 0.480781 | — | — | — |
| github-176.test | RUN | 0.384685 | 0.850304 | 1.121538 | 0.001904 | 0.002184 | 0.002052 |
| size.test | COMPILE | 0.338914 | 0.589503 | 0.890765 | — | — | — |
| fused.test | RUN | 0.465177 | 0.912662 | 1.328643 | 0.002086 | 0.002271 | 0.002165 |
| fused_function_object.test | RUN | 0.346927 | 0.681941 | 0.957505 | 0.001840 | 0.002040 | 0.002248 |
| fused_procedure.test | RUN | 0.451976 | 0.877646 | 1.274555 | 0.001844 | 0.002103 | 0.002092 |
| unfused.test | RUN | 0.362199 | 0.686614 | 1.019937 | 0.001971 | 0.002106 | 0.002170 |
| unfused_typed.test | RUN | 0.354102 | 0.764768 | 1.044568 | 0.002106 | 0.002186 | 0.002076 |
| make_fused.test | RUN | 0.448327 | 0.903131 | 1.311712 | 0.002070 | 0.002060 | 0.002024 |
| make_fused_function_object.test | RUN | 0.356139 | 0.698080 | 1.000209 | 0.001953 | 0.001805 | 0.001995 |
| make_fused_procedure.test | RUN | 0.451276 | 0.865939 | 1.278715 | 0.002058 | 0.002228 | 0.002122 |
| make_unfused.test | RUN | 0.345429 | 0.716254 | 1.023504 | 0.002010 | 0.001964 | 0.002197 |
| invoke.test | RUN | 0.827359 | 2.472728 | 3.357070 | 0.002188 | 0.001833 | 0.002070 |
| invoke_function_object.test | RUN | 0.435870 | 0.890874 | 1.305636 | 0.002132 | 0.002256 | 0.002098 |
| invoke_procedure.test | RUN | 0.668019 | 1.815548 | 2.612125 | 0.001890 | 0.002106 | 0.001782 |
| swap.test | RUN | 0.524438 | 1.157417 | 1.674826 | 0.001844 | 0.002222 | 0.002048 |
| is_sequence.test | COMPILE | 0.097705 | 0.169234 | 0.204094 | — | — | — |
| is_view.test | COMPILE | 0.100259 | 0.161661 | 0.204907 | — | — | — |
| pair_deque.test | COMPILE | 0.199360 | 0.392990 | 0.507737 | — | — | — |
| pair_list.test | COMPILE | 0.197039 | 0.343335 | 0.486660 | — | — | — |
| pair_map.test | COMPILE | 0.205937 | 0.373071 | 0.530287 | — | — | — |
| pair_set.test | COMPILE | 0.210679 | 0.377160 | 0.508085 | — | — | — |
| pair_vector.test | COMPILE | 0.206547 | 0.361758 | 0.496481 | — | — | — |
| pair_nest.test | COMPILE | 0.080529 | 0.160597 | 0.191805 | — | — | — |
| index_sequence.test | COMPILE | 0.076753 | 0.112791 | 0.137162 | — | — | — |
| and.test | COMPILE | 0.060806 | 0.082384 | 0.093581 | — | — | — |
| tag_of.test | COMPILE | 0.095160 | 0.153521 | 0.186111 | — | — | — |
| unused.test | COMPILE | 0.047035 | 0.098795 | 0.116465 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.093258 | 0.149798 | 0.190554 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.057817 | 0.130683 | 0.162918 | — | — | — |
| quick.test | RUN | 0.233436 | 0.453291 | 0.596070 | 0.001396 | 0.001473 | 0.001450 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.983×, 0.994× | 0.997× |
| clang | 0.994× | 0.997×, 0.991× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001303s | 0.001336s | 0.001321s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001284s | 0.001350s | 0.001303s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001313s | 0.001289s | 0.001292s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001278s | 0.001266s | 0.001289s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001276s | 0.001308s | 0.001328s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001305s | 0.001268s | 0.001293s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001341s | 0.001324s | 0.001313s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001317s | 0.001339s | 0.001313s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001327s | 0.001369s | 0.001297s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001310s | 0.001304s | 0.001283s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001305s | 0.001309s | 0.001308s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001298s | 0.001362s | 0.001258s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001328s | 0.001345s | 0.001254s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001282s | 0.001285s | 0.001308s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001328s | 0.001345s | 0.001317s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001266s | 0.001278s | 0.001314s | byte equality observed |
| adapt_struct.test | 10 | 0.001285s | 0.001304s | 0.001310s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001316s | 0.001304s | 0.001239s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001340s | 0.001300s | 0.001258s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001323s | 0.001362s | 0.001264s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001363s | 0.001339s | 0.001250s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001350s | 0.001285s | 0.001256s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001301s | 0.001301s | 0.001300s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001303s | 0.001334s | 0.001282s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001332s | 0.001249s | 0.001261s | byte equality observed |
| all.test | 10 | 0.001279s | 0.001276s | 0.001314s | byte equality observed |
| any.test | 10 | 0.001301s | 0.001266s | 0.001318s | byte equality observed |
| array.test | 10 | 0.001278s | 0.001294s | 0.001297s | byte equality observed |
| as_deque.test | 10 | 0.001296s | 0.001266s | 0.001250s | byte equality observed |
| as_list.test | 10 | 0.001321s | 0.001299s | 0.001316s | byte equality observed |
| as_map.test | 10 | 0.001333s | 0.001343s | 0.001301s | byte equality observed |
| as_map_assoc.test | 10 | 0.001309s | 0.001304s | 0.001259s | byte equality observed |
| as_set.test | 10 | 0.001359s | 0.001353s | 0.001339s | byte equality observed |
| as_vector.test | 10 | 0.001356s | 0.001345s | 0.001311s | byte equality observed |
| back_extended_deque.test | 10 | 0.001263s | 0.001235s | 0.001303s | byte equality observed |
| boost_array.test | 10 | 0.001302s | 0.001254s | 0.001264s | byte equality observed |
| boost_tuple.test | 10 | 0.001316s | 0.001290s | 0.001309s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001259s | 0.001270s | 0.001229s | byte equality observed |
| clear.test | 10 | 0.001334s | 0.001315s | 0.001281s | byte equality observed |
| cons.test | 10 | 0.001361s | 0.001365s | 0.001348s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001293s | 0.001267s | 0.001254s | byte equality observed |
| convert_deque.test | 10 | 0.001266s | 0.001272s | 0.001283s | byte equality observed |
| convert_list.test | 10 | 0.001290s | 0.001315s | 0.001272s | byte equality observed |
| convert_std_pair.test | 10 | 0.001285s | 0.001295s | 0.001275s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001288s | 0.001329s | 0.001314s | byte equality observed |
| convert_vector.test | 10 | 0.001287s | 0.001269s | 0.001326s | byte equality observed |
| copy.test | 10 | 0.001272s | 0.001292s | 0.001293s | byte equality observed |
| count.test | 10 | 0.001354s | 0.001244s | 0.001272s | byte equality observed |
| count_if.test | 10 | 0.001342s | 0.001260s | 0.001255s | byte equality observed |
| deduce_sequence.test | 10 | 0.001270s | 0.001320s | 0.001322s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001311s | 0.001317s | 0.001324s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001329s | 0.001309s | 0.001345s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001264s | 0.001252s | 0.001268s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001295s | 0.001314s | 0.001311s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001313s | 0.001257s | 0.001280s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001273s | 0.001247s | 0.001259s | byte equality observed |
| define_struct.test | 10 | 0.001313s | 0.001310s | 0.001301s | byte equality observed |
| define_struct_empty.test | 10 | 0.001292s | 0.001317s | 0.001340s | byte equality observed |
| define_struct_inline.test | 10 | 0.001367s | 0.001267s | 0.001315s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001333s | 0.001311s | 0.001287s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001297s | 0.001251s | 0.001270s | byte equality observed |
| define_struct_move.test | 10 | 0.001261s | 0.001233s | 0.001264s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001315s | 0.001308s | 0.001275s | byte equality observed |
| define_tpl_struct_empty.test | 9 | 0.001352s | 0.001309s | 0.001349s | byte equality observed |
| define_tpl_struct_inline.test | 9 | 0.001372s | 0.001304s | 0.001378s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001306s | 0.001277s | 0.001290s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001283s | 0.001235s | 0.001271s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001273s | 0.001294s | 0.001264s | byte equality observed |
| deque_comparison.test | 10 | 0.001271s | 0.001308s | 0.001270s | byte equality observed |
| deque_construction.test | 10 | 0.001276s | 0.001258s | 0.001293s | byte equality observed |
| deque_copy.test | 10 | 0.001347s | 0.001264s | 0.001313s | byte equality observed |
| deque_hash.test | 10 | 0.001282s | 0.001215s | 0.001257s | byte equality observed |
| deque_iterator.test | 10 | 0.001285s | 0.001241s | 0.001252s | byte equality observed |
| deque_make.test | 10 | 0.001347s | 0.001279s | 0.001293s | byte equality observed |
| deque_misc.test | 10 | 0.001249s | 0.001264s | 0.001289s | byte equality observed |
| deque_move.test | 10 | 0.001284s | 0.001278s | 0.001324s | byte equality observed |
| deque_mutate.test | 10 | 0.001276s | 0.001277s | 0.001262s | byte equality observed |
| deque_nest.test | 10 | 0.001264s | 0.001241s | 0.001257s | byte equality observed |
| deque_tie.test | 10 | 0.001281s | 0.001317s | 0.001246s | byte equality observed |
| deque_value_at.test | 10 | 0.001323s | 0.001303s | 0.001255s | byte equality observed |
| erase.test | 10 | 0.001313s | 0.001320s | 0.001310s | byte equality observed |
| erase_key.test | 10 | 0.001344s | 0.001331s | 0.001295s | byte equality observed |
| filter.test | 10 | 0.001297s | 0.001281s | 0.001314s | byte equality observed |
| filter_if.test | 10 | 0.001367s | 0.001293s | 0.001323s | byte equality observed |
| filter_view.test | 10 | 0.001346s | 0.001343s | 0.001362s | byte equality observed |
| find.test | 10 | 0.001319s | 0.001302s | 0.001319s | byte equality observed |
| find_if.test | 10 | 0.001359s | 0.001270s | 0.001296s | byte equality observed |
| flatten.test | 10 | 0.001365s | 0.001289s | 0.001289s | byte equality observed |
| flatten_view.test | 10 | 0.001332s | 0.001232s | 0.001272s | byte equality observed |
| fold.test | 10 | 0.001334s | 0.001255s | 0.001294s | byte equality observed |
| for_each.test | 10 | 0.001335s | 0.001291s | 0.001291s | byte equality observed |
| front_extended_deque.test | 10 | 0.001299s | 0.001261s | 0.001287s | byte equality observed |
| fused.test | 10 | 0.001286s | 0.001247s | 0.001279s | byte equality observed |
| fused_function_object.test | 10 | 0.001303s | 0.001259s | 0.001286s | byte equality observed |
| fused_procedure.test | 10 | 0.001348s | 0.001236s | 0.001270s | byte equality observed |
| github-176.test | 10 | 0.001256s | 0.001268s | 0.001303s | byte equality observed |
| hash.test | 10 | 0.001299s | 0.001275s | 0.001304s | byte equality observed |
| identity_view.test | 7 | 0.001350s | 0.001288s | 0.001303s | byte equality observed |
| insert.test | 8 | 0.001365s | 0.001373s | 0.001320s | byte equality observed |
| insert_range.test | 9 | 0.001308s | 0.001300s | 0.001347s | byte equality observed |
| invoke.test | 10 | 0.001286s | 0.001308s | 0.001269s | byte equality observed |
| invoke_function_object.test | 10 | 0.001252s | 0.001293s | 0.001234s | byte equality observed |
| invoke_procedure.test | 10 | 0.001283s | 0.001269s | 0.001298s | byte equality observed |
| io.test | 9 | 0.001441s | 0.001468s | 0.001457s | byte equality observed |
| iter_fold.test | 10 | 0.001286s | 0.001328s | 0.001274s | byte equality observed |
| iterator_range.test | 10 | 0.001318s | 0.001304s | 0.001297s | byte equality observed |
| join.test | 10 | 0.001313s | 0.001245s | 0.001299s | byte equality observed |
| joint_view.test | 10 | 0.001356s | 0.001339s | 0.001378s | byte equality observed |
| list_comparison.test | 10 | 0.001271s | 0.001246s | 0.001321s | byte equality observed |
| list_construction.test | 10 | 0.001280s | 0.001282s | 0.001284s | byte equality observed |
| list_copy.test | 10 | 0.001308s | 0.001293s | 0.001295s | byte equality observed |
| list_hash.test | 9 | 0.001306s | 0.001287s | 0.001249s | byte equality observed |
| list_iterator.test | 9 | 0.001278s | 0.001300s | 0.001259s | byte equality observed |
| list_make.test | 10 | 0.001287s | 0.001241s | 0.001278s | byte equality observed |
| list_misc.test | 10 | 0.001287s | 0.001272s | 0.001272s | byte equality observed |
| list_mutate.test | 10 | 0.001275s | 0.001266s | 0.001283s | byte equality observed |
| list_nest.test | 10 | 0.001295s | 0.001234s | 0.001277s | byte equality observed |
| list_tie.test | 10 | 0.001274s | 0.001234s | 0.001307s | byte equality observed |
| list_value_at.test | 10 | 0.001284s | 0.001296s | 0.001309s | byte equality observed |
| make_fused.test | 10 | 0.001279s | 0.001285s | 0.001309s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001267s | 0.001275s | 0.001278s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001232s | 0.001249s | 0.001267s | byte equality observed |
| make_list.test | 10 | 0.001267s | 0.001274s | 0.001272s | byte equality observed |
| make_unfused.test | 10 | 0.001299s | 0.001285s | 0.001324s | byte equality observed |
| make_vector.test | 10 | 0.001257s | 0.001288s | 0.001280s | byte equality observed |
| map.test | 10 | 0.001327s | 0.001286s | 0.001333s | byte equality observed |
| map_comparison.test | 10 | 0.001259s | 0.001276s | 0.001243s | byte equality observed |
| map_construction.test | 10 | 0.001260s | 0.001253s | 0.001292s | byte equality observed |
| map_copy.test | 10 | 0.001280s | 0.001273s | 0.001319s | byte equality observed |
| map_misc.test | 10 | 0.001276s | 0.001268s | 0.001250s | byte equality observed |
| map_move.test | 10 | 0.001292s | 0.001259s | 0.001301s | byte equality observed |
| map_mutate.test | 10 | 0.001310s | 0.001223s | 0.001223s | byte equality observed |
| map_tie.test | 10 | 0.001259s | 0.001312s | 0.001217s | byte equality observed |
| move.test | 10 | 0.001250s | 0.001252s | 0.001252s | byte equality observed |
| nil.test | 10 | 0.001288s | 0.001294s | 0.001288s | byte equality observed |
| none.test | 8 | 0.001312s | 0.001268s | 0.001380s | byte equality observed |
| nview.test | 10 | 0.001284s | 0.001262s | 0.001292s | byte equality observed |
| pop_back.test | 10 | 0.001304s | 0.001326s | 0.001311s | byte equality observed |
| pop_front.test | 9 | 0.001353s | 0.001344s | 0.001321s | byte equality observed |
| push_back.test | 10 | 0.001348s | 0.001380s | 0.001351s | byte equality observed |
| push_front.test | 9 | 0.001362s | 0.001317s | 0.001326s | byte equality observed |
| quick.test | 10 | 0.001273s | 0.001194s | 0.001266s | byte equality observed |
| ref_vector.test | 10 | 0.001314s | 0.001291s | 0.001280s | byte equality observed |
| remove.test | 9 | 0.001311s | 0.001282s | 0.001310s | byte equality observed |
| remove_if.test | 10 | 0.001341s | 0.001321s | 0.001301s | byte equality observed |
| repetitive_view.test | 10 | 0.001267s | 0.001289s | 0.001332s | byte equality observed |
| replace.test | 10 | 0.001322s | 0.001364s | 0.001296s | byte equality observed |
| replace_if.test | 10 | 0.001350s | 0.001376s | 0.001297s | byte equality observed |
| reverse.test | 10 | 0.001352s | 0.001329s | 0.001350s | byte equality observed |
| reverse_fold.test | 10 | 0.001328s | 0.001352s | 0.001343s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001305s | 0.001315s | 0.001296s | byte equality observed |
| reverse_view.test | 10 | 0.001326s | 0.001319s | 0.001299s | byte equality observed |
| segmented_find.test | 10 | 0.001292s | 0.001280s | 0.001259s | byte equality observed |
| segmented_find_if.test | 10 | 0.001305s | 0.001278s | 0.001291s | byte equality observed |
| segmented_fold.test | 10 | 0.001322s | 0.001291s | 0.001300s | byte equality observed |
| segmented_for_each.test | 8 | 0.001327s | 0.001326s | 0.001371s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001375s | 0.001345s | 0.001346s | byte equality observed |
| set.test | 8 | 0.001312s | 0.001316s | 0.001296s | byte equality observed |
| single_view.test | 10 | 0.001323s | 0.001300s | 0.001279s | byte equality observed |
| std_array.test | 10 | 0.001340s | 0.001244s | 0.001256s | byte equality observed |
| std_pair.test | 10 | 0.001310s | 0.001260s | 0.001306s | byte equality observed |
| std_tuple.test | 10 | 0.001273s | 0.001398s | 0.001272s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001311s | 0.001253s | 0.001254s | byte equality observed |
| swap.test | 10 | 0.001272s | 0.001292s | 0.001306s | byte equality observed |
| transform.test | 10 | 0.001381s | 0.001252s | 0.001288s | byte equality observed |
| transform_view.test | 10 | 0.001342s | 0.001283s | 0.001299s | byte equality observed |
| tuple_comparison.test | 10 | 0.001282s | 0.001258s | 0.001294s | byte equality observed |
| tuple_construction.test | 10 | 0.001293s | 0.001278s | 0.001309s | byte equality observed |
| tuple_conversion.test | 10 | 0.001311s | 0.001300s | 0.001286s | byte equality observed |
| tuple_copy.test | 10 | 0.001299s | 0.001283s | 0.001265s | byte equality observed |
| tuple_element.test | 10 | 0.001265s | 0.001265s | 0.001259s | byte equality observed |
| tuple_hash.test | 10 | 0.001314s | 0.001300s | 0.001277s | byte equality observed |
| tuple_make.test | 8 | 0.001308s | 0.001273s | 0.001323s | byte equality observed |
| tuple_misc.test | 10 | 0.001271s | 0.001282s | 0.001255s | byte equality observed |
| tuple_mutate.test | 10 | 0.001267s | 0.001284s | 0.001278s | byte equality observed |
| tuple_nest.test | 10 | 0.001250s | 0.001332s | 0.001327s | byte equality observed |
| tuple_tie.test | 10 | 0.001275s | 0.001273s | 0.001313s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001241s | 0.001321s | 0.001294s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001282s | 0.001284s | 0.001288s | byte equality observed |
| unfused.test | 10 | 0.001286s | 0.001276s | 0.001327s | byte equality observed |
| unfused_typed.test | 10 | 0.001266s | 0.001276s | 0.001308s | byte equality observed |
| vector_comparison.test | 10 | 0.001267s | 0.001347s | 0.001294s | byte equality observed |
| vector_construction.test | 10 | 0.001299s | 0.001253s | 0.001312s | byte equality observed |
| vector_conversion.test | 10 | 0.001309s | 0.001315s | 0.001281s | byte equality observed |
| vector_copy.test | 10 | 0.001299s | 0.001289s | 0.001261s | byte equality observed |
| vector_hash.test | 10 | 0.001328s | 0.001332s | 0.001262s | byte equality observed |
| vector_iterator.test | 10 | 0.001273s | 0.001244s | 0.001273s | byte equality observed |
| vector_make.test | 10 | 0.001257s | 0.001292s | 0.001245s | byte equality observed |
| vector_misc.test | 10 | 0.001285s | 0.001252s | 0.001256s | byte equality observed |
| vector_move.test | 10 | 0.001313s | 0.001288s | 0.001314s | byte equality observed |
| vector_mutate.test | 8 | 0.001258s | 0.001304s | 0.001272s | byte equality observed |
| vector_n.test | 10 | 0.001310s | 0.001324s | 0.001315s | byte equality observed |
| vector_nest.test | 10 | 0.001295s | 0.001246s | 0.001263s | byte equality observed |
| vector_tie.test | 10 | 0.001269s | 0.001255s | 0.001315s | byte equality observed |
| vector_traits__maybe_variadic.test | 9 | 0.001285s | 0.001280s | 0.001288s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001277s | 0.001264s | 0.001266s | byte equality observed |
| vector_value_at.test | 10 | 0.001265s | 0.001311s | 0.001255s | byte equality observed |
| zip.test | 10 | 0.001282s | 0.001292s | 0.001270s | byte equality observed |
| zip2.test | 10 | 0.001284s | 0.001247s | 0.001267s | byte equality observed |
| zip_ignore.test | 10 | 0.001301s | 0.001287s | 0.001288s | byte equality observed |
| zip_view.test | 10 | 0.001283s | 0.001280s | 0.001283s | byte equality observed |
| zip_view2.test | 10 | 0.001276s | 0.001309s | 0.001289s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001292s | 0.001240s | 0.001276s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
