# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.551743 | 1.044940 | 1.648785 | 0.001980 | 0.001922 | 0.001880 |
| any.test | RUN | 0.533987 | 1.013016 | 1.563194 | 0.002167 | 0.002130 | 0.002233 |
| clear.test | RUN | 0.379802 | 0.902361 | 1.230730 | 0.001968 | 0.002038 | 0.002261 |
| copy.test | RUN | 0.335228 | 0.673171 | 1.017261 | 0.001954 | 0.002239 | 0.002283 |
| count.test | RUN | 0.374377 | 0.759614 | 1.113233 | 0.002193 | 0.002058 | 0.002204 |
| count_if.test | RUN | 0.352212 | 0.732075 | 1.033022 | 0.002087 | 0.002034 | 0.002185 |
| erase.test | RUN | 0.503340 | 1.215678 | 1.653641 | 0.001950 | 0.002402 | 0.001976 |
| erase_key.test | RUN | 0.480521 | 1.209695 | 1.554256 | 0.002082 | 0.002228 | 0.001677 |
| filter.test | RUN | 0.434664 | 1.008126 | 1.413320 | 0.002308 | 0.002372 | 0.002138 |
| filter_if.test | RUN | 0.474749 | 1.223633 | 1.699806 | 0.002059 | 0.002196 | 0.002346 |
| find.test | RUN | 0.436000 | 0.832318 | 1.306227 | 0.001937 | 0.001930 | 0.002244 |
| find_if.test | RUN | 0.408685 | 0.793848 | 1.163622 | 0.001693 | 0.002158 | 0.002056 |
| fold.test | RUN | 0.438671 | 0.983274 | 1.450333 | 0.002143 | 0.002330 | 0.002060 |
| for_each.test | RUN | 0.401570 | 0.866832 | 1.260956 | 0.002166 | 0.002246 | 0.002310 |
| insert.test | RUN | 0.553119 | 1.540204 | 1.988740 | 0.002078 | 0.001906 | 0.002218 |
| insert_range.test | RUN | 0.575666 | 1.550979 | 2.015168 | 0.001986 | 0.001975 | 0.002176 |
| iter_fold.test | RUN | 0.693407 | 1.483991 | 2.393786 | 0.002161 | 0.002368 | 0.002259 |
| move.test | RUN | 0.344277 | 0.719019 | 1.050487 | 0.002178 | 0.002373 | 0.002260 |
| none.test | RUN | 0.548944 | 0.994067 | 1.624283 | 0.001868 | 0.002176 | 0.001969 |
| pop_back.test | RUN | 0.514553 | 1.338804 | 1.814641 | 0.001972 | 0.002253 | 0.002286 |
| pop_front.test | RUN | 0.445669 | 1.097633 | 1.457367 | 0.002223 | 0.002061 | 0.002263 |
| push_back.test | RUN | 0.554557 | 1.577484 | 1.934411 | 0.002172 | 0.002506 | 0.002334 |
| push_front.test | RUN | 0.499370 | 1.315215 | 1.669223 | 0.002302 | 0.002162 | 0.002276 |
| remove.test | RUN | 0.551041 | 1.429121 | 1.898988 | 0.001927 | 0.002022 | 0.002360 |
| remove_if.test | RUN | 0.495058 | 1.205216 | 1.700456 | 0.002156 | 0.002113 | 0.002129 |
| replace.test | RUN | 0.461387 | 1.148907 | 1.551751 | 0.002144 | 0.002409 | 0.002098 |
| replace_if.test | RUN | 0.465885 | 1.161291 | 1.525861 | 0.002215 | 0.002428 | 0.002239 |
| reverse_fold.test | RUN | 0.734834 | 1.711351 | 2.672030 | 0.001757 | 0.002158 | 0.001892 |
| reverse_iter_fold.test | RUN | 0.751595 | 1.809668 | 2.775392 | 0.002232 | 0.002164 | 0.002000 |
| reverse.test | RUN | 0.461582 | 1.254883 | 1.604184 | 0.002226 | 0.002338 | 0.001924 |
| segmented_for_each.test | RUN | 0.800723 | 2.335923 | 3.169541 | 0.002101 | 0.002258 | 0.002187 |
| segmented_find.test | RUN | 0.510033 | 1.215300 | 1.811748 | 0.002068 | 0.002160 | 0.002190 |
| segmented_find_if.test | RUN | 0.515668 | 1.237919 | 1.819858 | 0.002084 | 0.002037 | 0.002120 |
| segmented_fold.test | RUN | 0.637842 | 1.627031 | 2.299995 | 0.002152 | 0.002142 | 0.002034 |
| transform.test | RUN | 0.487734 | 1.276506 | 1.652680 | 0.002078 | 0.002139 | 0.002282 |
| join.test | RUN | 0.384802 | 0.821759 | 1.186176 | 0.002064 | 0.002061 | 0.002256 |
| zip.test | RUN | 0.657454 | 1.490489 | 2.209491 | 0.002056 | 0.002300 | 0.002229 |
| zip2.test | RUN | 0.633299 | 1.477088 | 2.127669 | 0.001961 | 0.002352 | 0.001876 |
| zip_ignore.test | RUN | 0.479448 | 0.954642 | 1.446604 | 0.002050 | 0.002238 | 0.002245 |
| flatten.test | RUN | 0.465179 | 1.102783 | 1.441422 | 0.001852 | 0.002065 | 0.002330 |
| ticket-5490.test | COMPILE | 0.251012 | 0.473024 | 0.671998 | — | — | — |
| as_deque.test | RUN | 0.448885 | 1.018051 | 1.580118 | 0.002221 | 0.001994 | 0.002983 |
| as_list.test | RUN | 0.489903 | 1.227307 | 1.681692 | 0.001898 | 0.001987 | 0.002124 |
| as_map.test | RUN | 0.485243 | 1.236808 | 1.658918 | 0.002069 | 0.002190 | 0.001602 |
| as_map_assoc.test | RUN | 0.490112 | 1.195461 | 1.595857 | 0.001870 | 0.002378 | 0.002091 |
| as_set.test | RUN | 0.543206 | 1.441947 | 1.959540 | 0.002110 | 0.002111 | 0.002019 |
| as_vector.test | RUN | 0.499541 | 1.311988 | 1.660187 | 0.002080 | 0.001862 | 0.002114 |
| boost_tuple.test | RUN | 0.490204 | 1.231202 | 1.681530 | 0.002080 | 0.002028 | 0.001826 |
| boost_tuple_iterator.test | RUN | 0.359492 | 0.758778 | 1.030523 | 0.001922 | 0.002052 | 0.002376 |
| cons.test | RUN | 0.610708 | 1.451922 | 2.030521 | 0.001926 | 0.002221 | 0.002395 |
| convert_boost_tuple.test | RUN | 0.404714 | 1.125769 | 1.305992 | 0.002067 | 0.001814 | 0.002117 |
| convert_deque.test | RUN | 0.413482 | 1.062674 | 1.320673 | 0.002162 | 0.001962 | 0.001970 |
| convert_list.test | RUN | 0.406684 | 1.088089 | 1.339029 | 0.002076 | 0.001895 | 0.001816 |
| convert_std_pair.test | RUN | 0.437480 | 1.107303 | 1.364068 | 0.001742 | 0.001870 | 0.001902 |
| convert_std_tuple.test | RUN | 0.410279 | 1.061678 | 1.333077 | 0.001715 | 0.002067 | 0.001758 |
| convert_vector.test | RUN | 0.407028 | 1.073612 | 1.298207 | 0.001580 | 0.001888 | 0.001956 |
| filter_view.test | RUN | 0.465707 | 1.131555 | 1.556437 | 0.002184 | 0.002146 | 0.002245 |
| hash.test | RUN | 0.476989 | 1.020618 | 1.570861 | 0.001972 | 0.002266 | 0.002038 |
| io.test | RUN | 0.483834 | 1.425802 | 1.813113 | 0.001985 | 0.002722 | 0.002530 |
| iterator_range.test | RUN | 0.447869 | 1.099930 | 1.430434 | 0.002181 | 0.002562 | 0.001660 |
| joint_view.test | RUN | 0.572483 | 1.803602 | 2.194567 | 0.002208 | 0.002073 | 0.002223 |
| list_comparison.test | RUN | 0.361253 | 0.765414 | 1.100613 | 0.001871 | 0.002158 | 0.002107 |
| list_construction.test | RUN | 0.349284 | 0.739739 | 1.054336 | 0.001909 | 0.002018 | 0.001928 |
| list_copy.test | RUN | 0.526451 | 1.355204 | 1.830522 | 0.001938 | 0.001988 | 0.001998 |
| list_iterator.test | RUN | 0.347412 | 0.740679 | 1.014993 | 0.001735 | 0.002001 | 0.002242 |
| list_hash.test | RUN | 0.473703 | 1.426468 | 1.496315 | 0.002192 | 0.002288 | 0.002157 |
| list_make.test | RUN | 0.372362 | 0.817201 | 1.164803 | 0.001896 | 0.002076 | 0.002118 |
| list_misc.test | RUN | 0.486346 | 0.938878 | 1.408079 | 0.002124 | 0.002295 | 0.002238 |
| list_mutate.test | RUN | 0.325878 | 0.654950 | 0.922209 | 0.001850 | 0.002278 | 0.002070 |
| list_nest.test | RUN | 0.659213 | 1.821996 | 2.530460 | 0.001725 | 0.002040 | 0.002008 |
| list_tie.test | RUN | 0.359616 | 0.734184 | 1.052372 | 0.002097 | 0.002288 | 0.002382 |
| list_value_at.test | RUN | 0.346662 | 0.748342 | 0.996868 | 0.001816 | 0.002148 | 0.002214 |
| deque_comparison.test | RUN | 0.365052 | 0.757837 | 1.146985 | 0.002108 | 0.002051 | 0.002017 |
| deque_construction.test | RUN | 0.349315 | 0.744611 | 1.049005 | 0.001987 | 0.002392 | 0.002177 |
| deque_copy.test | RUN | 0.496847 | 1.257038 | 1.742289 | 0.001862 | 0.002164 | 0.001854 |
| deque_iterator.test | RUN | 0.366424 | 0.777668 | 1.107043 | 0.001935 | 0.002170 | 0.002186 |
| deque_hash.test | RUN | 0.486737 | 1.414023 | 1.475498 | 0.001818 | 0.002334 | 0.002047 |
| deque_is_constructible.test | COMPILE | 0.195668 | 0.347755 | 0.494659 | — | — | — |
| deque_make.test | RUN | 0.361269 | 0.777387 | 1.104421 | 0.002025 | 0.002314 | 0.002258 |
| deque_misc.test | RUN | 0.463665 | 0.929557 | 1.404496 | 0.001846 | 0.002150 | 0.001673 |
| deque_move.test | RUN | 0.369970 | 0.835507 | 1.071669 | 0.002051 | 0.002074 | 0.002185 |
| deque_mutate.test | RUN | 0.328992 | 0.647826 | 0.927903 | 0.002160 | 0.002267 | 0.002132 |
| deque_nest.test | RUN | 0.974936 | 2.857467 | 4.011473 | 0.002089 | 0.002204 | 0.002014 |
| deque_tie.test | RUN | 0.341119 | 0.692941 | 1.024147 | 0.001778 | 0.002135 | 0.002113 |
| deque_value_at.test | RUN | 0.329583 | 0.693374 | 1.022841 | 0.001812 | 0.002210 | 0.002280 |
| front_extended_deque.test | RUN | 0.498344 | 1.043695 | 1.411352 | 0.002054 | 0.002444 | 0.002192 |
| back_extended_deque.test | RUN | 0.479690 | 0.986866 | 1.499320 | 0.002008 | 0.002088 | 0.002145 |
| make_list.test | RUN | 0.379080 | 0.831134 | 1.135786 | 0.001888 | 0.002310 | 0.002256 |
| make_vector.test | RUN | 0.381064 | 0.820157 | 1.129412 | 0.001927 | 0.002264 | 0.002184 |
| map.test | RUN | 0.465255 | 1.210407 | 1.603730 | 0.002186 | 0.002380 | 0.001774 |
| map_comparison.test | RUN | 0.382744 | 0.793538 | 1.061006 | 0.002154 | 0.002316 | 0.002212 |
| map_construction.test | RUN | 0.368012 | 0.764089 | 1.118418 | 0.001934 | 0.002188 | 0.001914 |
| map_copy.test | RUN | 0.434812 | 0.970409 | 1.337619 | 0.002188 | 0.002083 | 0.002292 |
| map_misc.test | RUN | 0.477690 | 0.978201 | 1.376950 | 0.001859 | 0.002207 | 0.001909 |
| map_move.test | RUN | 0.367732 | 0.858158 | 1.089987 | 0.001871 | 0.002179 | 0.001837 |
| map_mutate.test | RUN | 0.339805 | 0.657690 | 0.940650 | 0.001852 | 0.001817 | 0.001774 |
| map_tie.test | RUN | 0.345655 | 0.657827 | 0.994053 | 0.002042 | 0.002092 | 0.002133 |
| nil.test | RUN | 0.239311 | 0.464499 | 0.651796 | 0.002244 | 0.002268 | 0.002096 |
| nview.test | RUN | 0.464856 | 0.966894 | 1.400464 | 0.001991 | 0.002307 | 0.002218 |
| reverse_view.test | RUN | 0.555502 | 1.548962 | 2.050967 | 0.002131 | 0.002321 | 0.002238 |
| segmented_iterator_range.test | RUN | 0.665210 | 1.864022 | 2.502349 | 0.002140 | 0.002137 | 0.002211 |
| set.test | RUN | 0.454612 | 1.109194 | 1.402827 | 0.002078 | 0.002202 | 0.002356 |
| single_view.test | RUN | 0.405173 | 0.958078 | 1.333788 | 0.002228 | 0.002376 | 0.002040 |
| std_pair.test | RUN | 0.515015 | 1.244229 | 1.666237 | 0.002182 | 0.002328 | 0.002112 |
| boost_array.test | RUN | 0.440215 | 0.855135 | 1.259528 | 0.002023 | 0.002247 | 0.001978 |
| array.test | RUN | 0.442928 | 0.914947 | 1.253816 | 0.002068 | 0.002274 | 0.001820 |
| std_array.test | RUN | 0.441621 | 0.871942 | 1.223482 | 0.002027 | 0.002016 | 0.002214 |
| tuple_comparison.test | RUN | 0.386539 | 0.817749 | 1.123343 | 0.002012 | 0.002364 | 0.001644 |
| tuple_construction.test | RUN | 0.398940 | 0.805042 | 1.176816 | 0.002249 | 0.002082 | 0.002180 |
| tuple_conversion.test | RUN | 0.866269 | 2.514914 | 3.744888 | 0.002045 | 0.002180 | 0.002188 |
| tuple_copy.test | RUN | 0.545957 | 1.463245 | 2.018667 | 0.002172 | 0.002127 | 0.002183 |
| tuple_element.test | RUN | 0.369980 | 0.749091 | 1.050328 | 0.001832 | 0.002138 | 0.002194 |
| tuple_make.test | RUN | 0.406811 | 0.885562 | 1.170723 | 0.002217 | 0.002322 | 0.002047 |
| tuple_misc.test | RUN | 0.487141 | 1.009817 | 1.462357 | 0.002128 | 0.002280 | 0.002300 |
| tuple_mutate.test | RUN | 0.344627 | 0.678762 | 1.082110 | 0.002180 | 0.001894 | 0.002147 |
| tuple_nest.test | RUN | 1.425600 | 4.954422 | 6.829801 | 0.001963 | 0.002150 | 0.002286 |
| tuple_hash.test | RUN | 0.518146 | 1.375725 | 1.537294 | 0.002324 | 0.002253 | 0.002097 |
| tuple_tie.test | RUN | 0.381091 | 0.758710 | 1.157870 | 0.002116 | 0.002266 | 0.001958 |
| tuple_traits__maybe_variadic.test | RUN | 0.417116 | 0.976262 | 1.245571 | 0.002226 | 0.002138 | 0.001776 |
| tuple_traits__no_variadic.test | RUN | 0.441428 | 0.949816 | 1.348032 | 0.002143 | 0.002350 | 0.002128 |
| transform_view.test | RUN | 0.579631 | 1.577690 | 2.009430 | 0.002232 | 0.002110 | 0.002196 |
| identity_view.test | RUN | 0.542084 | 1.427082 | 1.888928 | 0.002109 | 0.002076 | 0.001769 |
| vector_comparison.test | RUN | 0.367160 | 0.817336 | 1.171171 | 0.002128 | 0.001921 | 0.002204 |
| vector_construction.test | RUN | 0.370702 | 0.739268 | 1.093373 | 0.002040 | 0.002274 | 0.002252 |
| vector_conversion.test | RUN | 0.988167 | 3.267057 | 4.783157 | 0.002060 | 0.002331 | 0.002210 |
| vector_copy.test | RUN | 0.541359 | 1.406906 | 1.986673 | 0.002304 | 0.002313 | 0.002289 |
| vector_iterator.test | RUN | 0.362026 | 0.786006 | 1.019310 | 0.002054 | 0.001936 | 0.002238 |
| vector_make.test | RUN | 0.385858 | 0.825018 | 1.100668 | 0.002218 | 0.002160 | 0.002302 |
| vector_misc.test | RUN | 0.476225 | 1.049580 | 1.341822 | 0.001889 | 0.002227 | 0.002026 |
| vector_move.test | RUN | 0.370833 | 0.801356 | 1.109727 | 0.002028 | 0.002042 | 0.001654 |
| vector_mutate.test | RUN | 0.335300 | 0.675905 | 0.963623 | 0.001956 | 0.001880 | 0.001844 |
| vector_n.test | RUN | 0.408377 | 0.835491 | 1.304483 | 0.002143 | 0.002314 | 0.002240 |
| vector_nest.test | RUN | 1.451912 | 5.077560 | 6.971927 | 0.002052 | 0.002265 | 0.002075 |
| vector_hash.test | RUN | 0.494013 | 1.403788 | 1.483580 | 0.002130 | 0.002280 | 0.001919 |
| vector_tie.test | RUN | 0.357075 | 0.746085 | 1.097938 | 0.001931 | 0.002098 | 0.002245 |
| vector_traits__maybe_variadic.test | RUN | 0.403677 | 0.865443 | 1.083045 | 0.002055 | 0.002440 | 0.002111 |
| vector_traits__no_variadic.test | RUN | 0.409535 | 0.931163 | 1.134117 | 0.002066 | 0.001857 | 0.002170 |
| vector_value_at.test | RUN | 0.361915 | 0.683342 | 1.031187 | 0.002140 | 0.002302 | 0.002072 |
| zip_view.test | RUN | 0.670752 | 1.615796 | 2.451222 | 0.002390 | 0.002050 | 0.002110 |
| zip_view2.test | RUN | 0.563950 | 1.388123 | 1.933274 | 0.002046 | 0.001908 | 0.002019 |
| zip_view_ignore.test | RUN | 0.531071 | 1.220396 | 1.734643 | 0.002068 | 0.001868 | 0.001857 |
| repetitive_view.test | RUN | 0.876666 | 2.154519 | 3.252442 | 0.001860 | 0.002308 | 0.002130 |
| deduce_sequence.test | RUN | 0.431439 | 0.888974 | 1.329955 | 0.002058 | 0.002153 | 0.001762 |
| adapt_adt_named.test | RUN | 0.509467 | 1.246263 | 1.657388 | 0.002092 | 0.002157 | 0.002222 |
| adapt_adt_named_empty.test | RUN | 0.461619 | 0.990445 | 1.371137 | 0.001998 | 0.002178 | 0.002314 |
| adapt_adt.test | RUN | 0.561579 | 1.670078 | 1.935653 | 0.002224 | 0.002353 | 0.002332 |
| adapt_adt_empty.test | RUN | 0.442334 | 1.010391 | 1.364403 | 0.001827 | 0.002113 | 0.002286 |
| adapt_assoc_adt_named.test | RUN | 0.580212 | 1.317492 | 1.769166 | 0.001975 | 0.002380 | 0.002118 |
| adapt_assoc_adt_named_empty.test | RUN | 0.448242 | 0.988765 | 1.332548 | 0.002180 | 0.002294 | 0.002207 |
| adapt_assoc_adt.test | RUN | 0.574660 | 1.374233 | 1.820282 | 0.002055 | 0.002378 | 0.002180 |
| adapt_assoc_adt_empty.test | RUN | 0.439219 | 1.003759 | 1.351486 | 0.002208 | 0.002278 | 0.002047 |
| adapt_assoc_struct_named.test | RUN | 0.558149 | 1.301256 | 1.727560 | 0.001906 | 0.002142 | 0.002310 |
| adapt_assoc_struct_named_empty.test | RUN | 0.448588 | 0.984946 | 1.352483 | 0.001884 | 0.002344 | 0.002156 |
| adapt_assoc_struct.test | RUN | 0.493305 | 1.224443 | 1.607666 | 0.002156 | 0.002374 | 0.002258 |
| adapt_assoc_struct_empty.test | RUN | 0.431205 | 0.996403 | 1.322604 | 0.001868 | 0.002150 | 0.002080 |
| adapt_assoc_tpl_adt.test | RUN | 0.586918 | 1.414223 | 1.851968 | 0.002064 | 0.002433 | 0.001737 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.458821 | 0.992571 | 1.297886 | 0.002097 | 0.002269 | 0.002354 |
| adapt_assoc_tpl_struct.test | RUN | 0.484244 | 1.237593 | 1.546623 | 0.002016 | 0.001976 | 0.002246 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.444619 | 0.987731 | 1.325673 | 0.001950 | 0.002063 | 0.001914 |
| adapt_struct_named.test | RUN | 0.513839 | 1.263560 | 1.539205 | 0.002146 | 0.002262 | 0.002215 |
| adapt_struct_named_empty.test | RUN | 0.446193 | 1.043613 | 1.367426 | 0.001711 | 0.002412 | 0.001958 |
| adapt_struct.test | RUN | 0.530761 | 1.372446 | 1.827534 | 0.001942 | 0.002184 | 0.002123 |
| adapt_struct_empty.test | RUN | 0.443937 | 1.006265 | 1.317935 | 0.002145 | 0.002490 | 0.002007 |
| adapt_tpl_adt.test | RUN | 0.544258 | 1.645499 | 1.860197 | 0.002166 | 0.001990 | 0.002242 |
| adapt_tpl_adt_empty.test | RUN | 0.465338 | 1.023692 | 1.345088 | 0.002092 | 0.002351 | 0.002166 |
| adapt_tpl_struct.test | RUN | 0.468554 | 1.103649 | 1.460223 | 0.002096 | 0.001956 | 0.002056 |
| adapt_tpl_struct_empty.test | RUN | 0.440833 | 1.025394 | 1.382462 | 0.002018 | 0.002298 | 0.002305 |
| adt_attribute_proxy.test | RUN | 0.525854 | 0.989829 | 1.463990 | 0.002116 | 0.002170 | 0.002085 |
| define_struct.test | RUN | 0.595596 | 1.367281 | 1.823818 | 0.002190 | 0.002282 | 0.002328 |
| define_struct_empty.test | RUN | 0.529048 | 1.110552 | 1.516007 | 0.002048 | 0.002181 | 0.002298 |
| define_struct_move.test | RUN | 0.365985 | 0.657372 | 0.942880 | 0.001913 | 0.002074 | 0.001959 |
| define_struct_inline.test | RUN | 0.632638 | 1.612670 | 2.110598 | 0.001816 | 0.002161 | 0.002037 |
| define_struct_inline_empty.test | RUN | 0.528129 | 1.129391 | 1.497706 | 0.001930 | 0.002288 | 0.002080 |
| define_struct_inline_move.test | RUN | 0.362302 | 0.670749 | 0.957158 | 0.002023 | 0.002105 | 0.002085 |
| define_assoc_struct.test | RUN | 0.578644 | 1.335635 | 1.802823 | 0.002222 | 0.002320 | 0.002314 |
| define_assoc_struct_empty.test | RUN | 0.522529 | 1.104890 | 1.605833 | 0.002208 | 0.002137 | 0.002258 |
| define_assoc_struct_move.test | RUN | 0.346918 | 0.642966 | 0.945718 | 0.001994 | 0.002150 | 0.002287 |
| define_tpl_struct.test | RUN | 0.570235 | 1.341447 | 1.723379 | 0.002139 | 0.002326 | 0.002006 |
| define_tpl_struct_empty.test | RUN | 0.540040 | 1.173726 | 1.576601 | 0.002166 | 0.002110 | 0.001806 |
| define_tpl_struct_move.test | RUN | 0.359486 | 0.635914 | 0.975097 | 0.002062 | 0.002282 | 0.002090 |
| define_tpl_struct_inline.test | RUN | 0.603992 | 1.571563 | 1.940095 | 0.001745 | 0.002372 | 0.002371 |
| define_tpl_struct_inline_empty.test | RUN | 0.531740 | 1.187385 | 1.549842 | 0.001904 | 0.002110 | 0.002303 |
| define_tpl_struct_inline_move.test | RUN | 0.357513 | 0.626001 | 0.964054 | 0.001952 | 0.002266 | 0.002256 |
| define_assoc_tpl_struct.test | RUN | 0.580490 | 1.327664 | 1.799096 | 0.002074 | 0.002274 | 0.002256 |
| define_assoc_tpl_struct_empty.test | RUN | 0.524637 | 1.147489 | 1.582374 | 0.001956 | 0.002327 | 0.001866 |
| define_assoc_tpl_struct_move.test | RUN | 0.356216 | 0.655874 | 0.918857 | 0.002078 | 0.002290 | 0.002125 |
| std_tuple.test | RUN | 0.350778 | 0.740695 | 1.028839 | 0.002222 | 0.002116 | 0.002047 |
| std_tuple_iterator.test | RUN | 0.352305 | 0.773896 | 1.051753 | 0.002010 | 0.001854 | 0.002343 |
| ref_vector.test | RUN | 0.404895 | 0.763196 | 1.128721 | 0.002154 | 0.002137 | 0.002259 |
| flatten_view.test | RUN | 0.438715 | 0.982067 | 1.309472 | 0.002102 | 0.002354 | 0.002242 |
| github-159.test | COMPILE | 0.196884 | 0.379888 | 0.469417 | — | — | — |
| github-176.test | RUN | 0.395845 | 0.870432 | 1.164985 | 0.002084 | 0.001926 | 0.002204 |
| size.test | COMPILE | 0.334569 | 0.625880 | 0.943384 | — | — | — |
| fused.test | RUN | 0.474493 | 0.906277 | 1.306644 | 0.002143 | 0.002143 | 0.002310 |
| fused_function_object.test | RUN | 0.354680 | 0.697487 | 1.036522 | 0.002072 | 0.002310 | 0.002201 |
| fused_procedure.test | RUN | 0.453986 | 0.910786 | 1.279891 | 0.002086 | 0.002215 | 0.002079 |
| unfused.test | RUN | 0.365001 | 0.704903 | 1.011186 | 0.002149 | 0.002080 | 0.002230 |
| unfused_typed.test | RUN | 0.363980 | 0.795030 | 1.079836 | 0.001974 | 0.002304 | 0.002086 |
| make_fused.test | RUN | 0.472251 | 0.934235 | 1.326049 | 0.002098 | 0.002112 | 0.001803 |
| make_fused_function_object.test | RUN | 0.357948 | 0.704120 | 1.028412 | 0.001906 | 0.002102 | 0.002082 |
| make_fused_procedure.test | RUN | 0.465116 | 0.905402 | 1.290984 | 0.002217 | 0.002072 | 0.002171 |
| make_unfused.test | RUN | 0.358225 | 0.714250 | 1.034540 | 0.002186 | 0.002301 | 0.002179 |
| invoke.test | RUN | 0.907947 | 2.557916 | 3.612507 | 0.002067 | 0.002062 | 0.002104 |
| invoke_function_object.test | RUN | 0.458377 | 0.919683 | 1.370746 | 0.002300 | 0.002356 | 0.002440 |
| invoke_procedure.test | RUN | 0.744049 | 1.934993 | 2.621573 | 0.002060 | 0.002052 | 0.002292 |
| swap.test | RUN | 0.541600 | 1.156695 | 1.630607 | 0.001787 | 0.002332 | 0.002208 |
| is_sequence.test | COMPILE | 0.109308 | 0.170835 | 0.202139 | — | — | — |
| is_view.test | COMPILE | 0.101253 | 0.169057 | 0.207712 | — | — | — |
| pair_deque.test | COMPILE | 0.209309 | 0.395037 | 0.472961 | — | — | — |
| pair_list.test | COMPILE | 0.199204 | 0.364731 | 0.515358 | — | — | — |
| pair_map.test | COMPILE | 0.207602 | 0.375231 | 0.481501 | — | — | — |
| pair_set.test | COMPILE | 0.218256 | 0.378162 | 0.515467 | — | — | — |
| pair_vector.test | COMPILE | 0.214793 | 0.380472 | 0.505247 | — | — | — |
| pair_nest.test | COMPILE | 0.083728 | 0.151376 | 0.210400 | — | — | — |
| index_sequence.test | COMPILE | 0.078840 | 0.108053 | 0.138752 | — | — | — |
| and.test | COMPILE | 0.059810 | 0.081446 | 0.094304 | — | — | — |
| tag_of.test | COMPILE | 0.102890 | 0.160580 | 0.188082 | — | — | — |
| unused.test | COMPILE | 0.048877 | 0.106433 | 0.121470 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.101924 | 0.158790 | 0.177206 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.071608 | 0.107866 | 0.163011 | — | — | — |
| quick.test | RUN | 0.244811 | 0.506610 | 0.622125 | 0.000843 | 0.001526 | 0.001538 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.000× | 1.000×, 1.000× | 0.994× |
| clang | 0.999× | 0.999×, 0.999× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001337s | 0.001342s | 0.001285s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001326s | 0.001328s | 0.001344s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001321s | 0.001355s | 0.001295s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001295s | 0.001306s | 0.001369s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001350s | 0.001311s | 0.001326s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001336s | 0.001308s | 0.001324s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001380s | 0.001305s | 0.001407s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001369s | 0.001363s | 0.001365s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001338s | 0.001361s | 0.001350s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001350s | 0.001380s | 0.001393s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001360s | 0.001353s | 0.001363s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001345s | 0.001363s | 0.001382s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001326s | 0.001399s | 0.001422s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001303s | 0.001330s | 0.001365s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001344s | 0.001399s | 0.001366s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001298s | 0.001340s | 0.001410s | byte equality observed |
| adapt_struct.test | 10 | 0.001304s | 0.001381s | 0.001424s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001286s | 0.001373s | 0.001354s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001354s | 0.001289s | 0.001389s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001388s | 0.001288s | 0.001311s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001375s | 0.001311s | 0.001383s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001338s | 0.001291s | 0.001343s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001349s | 0.001317s | 0.001340s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001393s | 0.001336s | 0.001305s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001319s | 0.001343s | 0.001365s | byte equality observed |
| all.test | 10 | 0.001314s | 0.001253s | 0.001335s | byte equality observed |
| any.test | 10 | 0.001323s | 0.001290s | 0.001277s | byte equality observed |
| array.test | 10 | 0.001304s | 0.001327s | 0.001317s | byte equality observed |
| as_deque.test | 10 | 0.001335s | 0.001309s | 0.001298s | byte equality observed |
| as_list.test | 10 | 0.001356s | 0.001353s | 0.001344s | byte equality observed |
| as_map.test | 10 | 0.001402s | 0.001333s | 0.001338s | byte equality observed |
| as_map_assoc.test | 10 | 0.001338s | 0.001371s | 0.001340s | byte equality observed |
| as_set.test | 10 | 0.001370s | 0.001368s | 0.001360s | byte equality observed |
| as_vector.test | 10 | 0.001359s | 0.001414s | 0.001308s | byte equality observed |
| back_extended_deque.test | 7 | 0.001307s | 0.001338s | 0.001425s | byte equality observed |
| boost_array.test | 8 | 0.001296s | 0.001311s | 0.001270s | byte equality observed |
| boost_tuple.test | 10 | 0.001316s | 0.001314s | 0.001330s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001367s | 0.001333s | 0.001284s | byte equality observed |
| clear.test | 10 | 0.001356s | 0.001333s | 0.001290s | byte equality observed |
| cons.test | 10 | 0.001361s | 0.001329s | 0.001303s | byte equality observed |
| convert_boost_tuple.test | 8 | 0.001345s | 0.001413s | 0.001303s | byte equality observed |
| convert_deque.test | 10 | 0.001308s | 0.001273s | 0.001318s | byte equality observed |
| convert_list.test | 10 | 0.001313s | 0.001302s | 0.001344s | byte equality observed |
| convert_std_pair.test | 10 | 0.001318s | 0.001287s | 0.001295s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001336s | 0.001312s | 0.001298s | byte equality observed |
| convert_vector.test | 10 | 0.001298s | 0.001298s | 0.001281s | byte equality observed |
| copy.test | 10 | 0.001332s | 0.001320s | 0.001309s | byte equality observed |
| count.test | 10 | 0.001306s | 0.001325s | 0.001306s | byte equality observed |
| count_if.test | 10 | 0.001313s | 0.001284s | 0.001360s | byte equality observed |
| deduce_sequence.test | 10 | 0.001267s | 0.001334s | 0.001368s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001305s | 0.001380s | 0.001307s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001327s | 0.001315s | 0.001349s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001303s | 0.001294s | 0.001296s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001349s | 0.001310s | 0.001311s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001325s | 0.001349s | 0.001336s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001288s | 0.001257s | 0.001372s | byte equality observed |
| define_struct.test | 10 | 0.001341s | 0.001329s | 0.001318s | byte equality observed |
| define_struct_empty.test | 10 | 0.001367s | 0.001341s | 0.001275s | byte equality observed |
| define_struct_inline.test | 10 | 0.001399s | 0.001372s | 0.001402s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001376s | 0.001332s | 0.001305s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001340s | 0.001289s | 0.001368s | byte equality observed |
| define_struct_move.test | 10 | 0.001284s | 0.001277s | 0.001323s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001326s | 0.001371s | 0.001348s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001345s | 0.001331s | 0.001331s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001354s | 0.001348s | 0.001355s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001342s | 0.001322s | 0.001375s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001322s | 0.001309s | 0.001297s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001306s | 0.001346s | 0.001325s | byte equality observed |
| deque_comparison.test | 10 | 0.001289s | 0.001302s | 0.001299s | byte equality observed |
| deque_construction.test | 10 | 0.001329s | 0.001292s | 0.001331s | byte equality observed |
| deque_copy.test | 10 | 0.001343s | 0.001308s | 0.001293s | byte equality observed |
| deque_hash.test | 10 | 0.001325s | 0.001337s | 0.001339s | byte equality observed |
| deque_iterator.test | 10 | 0.001313s | 0.001280s | 0.001323s | byte equality observed |
| deque_make.test | 10 | 0.001318s | 0.001304s | 0.001348s | byte equality observed |
| deque_misc.test | 10 | 0.001296s | 0.001381s | 0.001285s | byte equality observed |
| deque_move.test | 9 | 0.001310s | 0.001310s | 0.001286s | byte equality observed |
| deque_mutate.test | 10 | 0.001289s | 0.001316s | 0.001283s | byte equality observed |
| deque_nest.test | 10 | 0.001319s | 0.001289s | 0.001281s | byte equality observed |
| deque_tie.test | 10 | 0.001290s | 0.001310s | 0.001363s | byte equality observed |
| deque_value_at.test | 10 | 0.001295s | 0.001311s | 0.001276s | byte equality observed |
| erase.test | 10 | 0.001302s | 0.001352s | 0.001318s | byte equality observed |
| erase_key.test | 10 | 0.001364s | 0.001418s | 0.001379s | byte equality observed |
| filter.test | 10 | 0.001312s | 0.001347s | 0.001373s | byte equality observed |
| filter_if.test | 10 | 0.001391s | 0.001350s | 0.001430s | byte equality observed |
| filter_view.test | 10 | 0.001384s | 0.001371s | 0.001337s | byte equality observed |
| find.test | 10 | 0.001394s | 0.001380s | 0.001324s | byte equality observed |
| find_if.test | 10 | 0.001381s | 0.001339s | 0.001401s | byte equality observed |
| flatten.test | 10 | 0.001341s | 0.001343s | 0.001331s | byte equality observed |
| flatten_view.test | 10 | 0.001345s | 0.001358s | 0.001396s | byte equality observed |
| fold.test | 10 | 0.001366s | 0.001396s | 0.001331s | byte equality observed |
| for_each.test | 10 | 0.001339s | 0.001350s | 0.001353s | byte equality observed |
| front_extended_deque.test | 10 | 0.001345s | 0.001342s | 0.001260s | byte equality observed |
| fused.test | 10 | 0.001277s | 0.001299s | 0.001291s | byte equality observed |
| fused_function_object.test | 10 | 0.001299s | 0.001291s | 0.001292s | byte equality observed |
| fused_procedure.test | 10 | 0.001299s | 0.001327s | 0.001314s | byte equality observed |
| github-176.test | 10 | 0.001273s | 0.001348s | 0.001320s | byte equality observed |
| hash.test | 10 | 0.001322s | 0.001338s | 0.001353s | byte equality observed |
| identity_view.test | 10 | 0.001350s | 0.001314s | 0.001341s | byte equality observed |
| insert.test | 10 | 0.001339s | 0.001330s | 0.001402s | byte equality observed |
| insert_range.test | 10 | 0.001375s | 0.001386s | 0.001327s | byte equality observed |
| invoke.test | 8 | 0.001275s | 0.001317s | 0.001347s | byte equality observed |
| invoke_function_object.test | 10 | 0.001305s | 0.001299s | 0.001320s | byte equality observed |
| invoke_procedure.test | 10 | 0.001281s | 0.001292s | 0.001353s | byte equality observed |
| io.test | 10 | 0.001470s | 0.001450s | 0.001460s | byte equality observed |
| iter_fold.test | 8 | 0.001338s | 0.001340s | 0.001346s | byte equality observed |
| iterator_range.test | 10 | 0.001331s | 0.001355s | 0.001341s | byte equality observed |
| join.test | 10 | 0.001350s | 0.001284s | 0.001288s | byte equality observed |
| joint_view.test | 10 | 0.001357s | 0.001388s | 0.001363s | byte equality observed |
| list_comparison.test | 10 | 0.001305s | 0.001313s | 0.001320s | byte equality observed |
| list_construction.test | 7 | 0.001311s | 0.001293s | 0.001283s | byte equality observed |
| list_copy.test | 8 | 0.001302s | 0.001326s | 0.001352s | byte equality observed |
| list_hash.test | 10 | 0.001305s | 0.001310s | 0.001287s | byte equality observed |
| list_iterator.test | 8 | 0.001308s | 0.001332s | 0.001281s | byte equality observed |
| list_make.test | 10 | 0.001322s | 0.001301s | 0.001306s | byte equality observed |
| list_misc.test | 9 | 0.001363s | 0.001309s | 0.001309s | byte equality observed |
| list_mutate.test | 10 | 0.001337s | 0.001321s | 0.001331s | byte equality observed |
| list_nest.test | 9 | 0.001315s | 0.001299s | 0.001296s | byte equality observed |
| list_tie.test | 10 | 0.001324s | 0.001294s | 0.001354s | byte equality observed |
| list_value_at.test | 10 | 0.001334s | 0.001289s | 0.001304s | byte equality observed |
| make_fused.test | 10 | 0.001312s | 0.001293s | 0.001320s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001338s | 0.001386s | 0.001324s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001346s | 0.001283s | 0.001290s | byte equality observed |
| make_list.test | 10 | 0.001325s | 0.001311s | 0.001332s | byte equality observed |
| make_unfused.test | 10 | 0.001322s | 0.001255s | 0.001354s | byte equality observed |
| make_vector.test | 10 | 0.001333s | 0.001264s | 0.001324s | byte equality observed |
| map.test | 10 | 0.001323s | 0.001345s | 0.001323s | byte equality observed |
| map_comparison.test | 10 | 0.001322s | 0.001272s | 0.001273s | byte equality observed |
| map_construction.test | 10 | 0.001322s | 0.001296s | 0.001302s | byte equality observed |
| map_copy.test | 10 | 0.001297s | 0.001343s | 0.001324s | byte equality observed |
| map_misc.test | 10 | 0.001330s | 0.001300s | 0.001348s | byte equality observed |
| map_move.test | 10 | 0.001318s | 0.001283s | 0.001275s | byte equality observed |
| map_mutate.test | 9 | 0.001336s | 0.001336s | 0.001286s | byte equality observed |
| map_tie.test | 10 | 0.001309s | 0.001277s | 0.001349s | byte equality observed |
| move.test | 10 | 0.001319s | 0.001335s | 0.001303s | byte equality observed |
| nil.test | 10 | 0.001329s | 0.001293s | 0.001305s | byte equality observed |
| none.test | 10 | 0.001319s | 0.001280s | 0.001281s | byte equality observed |
| nview.test | 10 | 0.001306s | 0.001310s | 0.001297s | byte equality observed |
| pop_back.test | 10 | 0.001355s | 0.001355s | 0.001368s | byte equality observed |
| pop_front.test | 10 | 0.001409s | 0.001352s | 0.001378s | byte equality observed |
| push_back.test | 10 | 0.001374s | 0.001425s | 0.001359s | byte equality observed |
| push_front.test | 10 | 0.001374s | 0.001338s | 0.001422s | byte equality observed |
| quick.test | 10 | 0.000621s | 0.001281s | 0.001261s | byte equality observed |
| ref_vector.test | 10 | 0.001337s | 0.001265s | 0.001256s | byte equality observed |
| remove.test | 10 | 0.001338s | 0.001359s | 0.001346s | byte equality observed |
| remove_if.test | 10 | 0.001349s | 0.001395s | 0.001413s | byte equality observed |
| repetitive_view.test | 10 | 0.001298s | 0.001304s | 0.001320s | byte equality observed |
| replace.test | 10 | 0.001297s | 0.001312s | 0.001329s | byte equality observed |
| replace_if.test | 10 | 0.001414s | 0.001333s | 0.001388s | byte equality observed |
| reverse.test | 10 | 0.001386s | 0.001340s | 0.001377s | byte equality observed |
| reverse_fold.test | 10 | 0.001345s | 0.001328s | 0.001385s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001350s | 0.001327s | 0.001346s | byte equality observed |
| reverse_view.test | 10 | 0.001372s | 0.001321s | 0.001366s | byte equality observed |
| segmented_find.test | 10 | 0.001362s | 0.001301s | 0.001296s | byte equality observed |
| segmented_find_if.test | 10 | 0.001269s | 0.001345s | 0.001352s | byte equality observed |
| segmented_fold.test | 10 | 0.001312s | 0.001302s | 0.001319s | byte equality observed |
| segmented_for_each.test | 9 | 0.001369s | 0.001360s | 0.001362s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001332s | 0.001376s | 0.001423s | byte equality observed |
| set.test | 10 | 0.001301s | 0.001370s | 0.001311s | byte equality observed |
| single_view.test | 10 | 0.001320s | 0.001387s | 0.001328s | byte equality observed |
| std_array.test | 10 | 0.001262s | 0.001333s | 0.001313s | byte equality observed |
| std_pair.test | 10 | 0.001344s | 0.001346s | 0.001306s | byte equality observed |
| std_tuple.test | 10 | 0.001300s | 0.001330s | 0.001261s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001301s | 0.001312s | 0.001246s | byte equality observed |
| swap.test | 10 | 0.001288s | 0.001299s | 0.001282s | byte equality observed |
| transform.test | 10 | 0.001325s | 0.001308s | 0.001326s | byte equality observed |
| transform_view.test | 10 | 0.001333s | 0.001344s | 0.001347s | byte equality observed |
| tuple_comparison.test | 10 | 0.001286s | 0.001323s | 0.001293s | byte equality observed |
| tuple_construction.test | 10 | 0.001290s | 0.001311s | 0.001270s | byte equality observed |
| tuple_conversion.test | 10 | 0.001314s | 0.001313s | 0.001280s | byte equality observed |
| tuple_copy.test | 10 | 0.001281s | 0.001312s | 0.001289s | byte equality observed |
| tuple_element.test | 10 | 0.001306s | 0.001273s | 0.001314s | byte equality observed |
| tuple_hash.test | 9 | 0.001300s | 0.001329s | 0.001310s | byte equality observed |
| tuple_make.test | 10 | 0.001334s | 0.001312s | 0.001331s | byte equality observed |
| tuple_misc.test | 10 | 0.001311s | 0.001290s | 0.001280s | byte equality observed |
| tuple_mutate.test | 10 | 0.001305s | 0.001317s | 0.001309s | byte equality observed |
| tuple_nest.test | 7 | 0.001293s | 0.001319s | 0.001399s | byte equality observed |
| tuple_tie.test | 10 | 0.001312s | 0.001351s | 0.001322s | byte equality observed |
| tuple_traits__maybe_variadic.test | 9 | 0.001288s | 0.001282s | 0.001265s | byte equality observed |
| tuple_traits__no_variadic.test | 9 | 0.001291s | 0.001310s | 0.001331s | byte equality observed |
| unfused.test | 10 | 0.001296s | 0.001257s | 0.001296s | byte equality observed |
| unfused_typed.test | 10 | 0.001292s | 0.001290s | 0.001294s | byte equality observed |
| vector_comparison.test | 10 | 0.001342s | 0.001346s | 0.001296s | byte equality observed |
| vector_construction.test | 9 | 0.001325s | 0.001305s | 0.001294s | byte equality observed |
| vector_conversion.test | 10 | 0.001334s | 0.001286s | 0.001350s | byte equality observed |
| vector_copy.test | 10 | 0.001295s | 0.001316s | 0.001274s | byte equality observed |
| vector_hash.test | 10 | 0.001279s | 0.001342s | 0.001307s | byte equality observed |
| vector_iterator.test | 10 | 0.001277s | 0.001318s | 0.001287s | byte equality observed |
| vector_make.test | 10 | 0.001304s | 0.001272s | 0.001306s | byte equality observed |
| vector_misc.test | 10 | 0.001306s | 0.001283s | 0.001295s | byte equality observed |
| vector_move.test | 10 | 0.001306s | 0.001302s | 0.001294s | byte equality observed |
| vector_mutate.test | 10 | 0.001330s | 0.001382s | 0.001333s | byte equality observed |
| vector_n.test | 10 | 0.001325s | 0.001314s | 0.001283s | byte equality observed |
| vector_nest.test | 10 | 0.001353s | 0.001269s | 0.001374s | byte equality observed |
| vector_tie.test | 10 | 0.001328s | 0.001263s | 0.001279s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001321s | 0.001306s | 0.001289s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001318s | 0.001336s | 0.001299s | byte equality observed |
| vector_value_at.test | 10 | 0.001306s | 0.001301s | 0.001318s | byte equality observed |
| zip.test | 10 | 0.001296s | 0.001324s | 0.001323s | byte equality observed |
| zip2.test | 10 | 0.001306s | 0.001286s | 0.001309s | byte equality observed |
| zip_ignore.test | 10 | 0.001356s | 0.001266s | 0.001310s | byte equality observed |
| zip_view.test | 10 | 0.001283s | 0.001260s | 0.001300s | byte equality observed |
| zip_view2.test | 10 | 0.001297s | 0.001282s | 0.001314s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001282s | 0.001306s | 0.001323s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
