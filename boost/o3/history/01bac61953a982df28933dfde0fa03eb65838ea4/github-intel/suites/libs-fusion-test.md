# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.280003 | 0.622708 | 0.805246 | 0.001004 | 0.001132 | 0.001140 |
| any.test | RUN | 0.272050 | 0.590707 | 0.755080 | 0.000993 | 0.001320 | 0.001196 |
| clear.test | RUN | 0.196934 | 0.553049 | 0.627818 | 0.001196 | 0.001232 | 0.001246 |
| copy.test | RUN | 0.168248 | 0.409873 | 0.506415 | 0.001134 | 0.001239 | 0.001231 |
| count.test | RUN | 0.187708 | 0.457790 | 0.567946 | 0.001263 | 0.001184 | 0.001104 |
| count_if.test | RUN | 0.173434 | 0.424510 | 0.509830 | 0.001023 | 0.001081 | 0.001114 |
| erase.test | RUN | 0.252225 | 0.742718 | 0.849673 | 0.001099 | 0.001196 | 0.001190 |
| erase_key.test | RUN | 0.244127 | 0.729628 | 0.794103 | 0.001257 | 0.001255 | 0.001227 |
| filter.test | RUN | 0.222721 | 0.627919 | 0.727813 | 0.001302 | 0.001257 | 0.001228 |
| filter_if.test | RUN | 0.246506 | 0.734048 | 0.860336 | 0.001170 | 0.001234 | 0.001312 |
| find.test | RUN | 0.215249 | 0.498504 | 0.650676 | 0.001218 | 0.001176 | 0.001114 |
| find_if.test | RUN | 0.205490 | 0.457391 | 0.600329 | 0.001220 | 0.001298 | 0.001208 |
| fold.test | RUN | 0.222427 | 0.578242 | 0.727337 | 0.001355 | 0.001237 | 0.001229 |
| for_each.test | RUN | 0.201592 | 0.526847 | 0.630669 | 0.001310 | 0.001257 | 0.001249 |
| insert.test | RUN | 0.290803 | 0.913689 | 1.038575 | 0.001234 | 0.001292 | 0.001115 |
| insert_range.test | RUN | 0.289302 | 0.922561 | 1.040535 | 0.001194 | 0.001164 | 0.001198 |
| iter_fold.test | RUN | 0.348382 | 0.856719 | 1.239736 | 0.001378 | 0.001195 | 0.001183 |
| move.test | RUN | 0.178285 | 0.420817 | 0.526156 | 0.001405 | 0.001147 | 0.001030 |
| none.test | RUN | 0.288449 | 0.594637 | 0.785837 | 0.001133 | 0.001130 | 0.001312 |
| pop_back.test | RUN | 0.288090 | 0.808375 | 0.951605 | 0.001443 | 0.001273 | 0.001252 |
| pop_front.test | RUN | 0.243410 | 0.654165 | 0.749178 | 0.001365 | 0.001224 | 0.001209 |
| push_back.test | RUN | 0.295145 | 0.930166 | 1.028488 | 0.001313 | 0.001248 | 0.001398 |
| push_front.test | RUN | 0.264213 | 0.770148 | 0.851121 | 0.001215 | 0.001278 | 0.001219 |
| remove.test | RUN | 0.287533 | 0.871784 | 0.995163 | 0.001216 | 0.001298 | 0.001378 |
| remove_if.test | RUN | 0.266114 | 0.716213 | 0.871020 | 0.001296 | 0.001252 | 0.001179 |
| replace.test | RUN | 0.248940 | 0.712686 | 0.814669 | 0.001344 | 0.001235 | 0.001283 |
| replace_if.test | RUN | 0.249960 | 0.698783 | 0.791273 | 0.001319 | 0.001384 | 0.001294 |
| reverse_fold.test | RUN | 0.386343 | 0.970166 | 1.374408 | 0.001318 | 0.001146 | 0.001162 |
| reverse_iter_fold.test | RUN | 0.396503 | 1.001179 | 1.434096 | 0.001516 | 0.001181 | 0.001101 |
| reverse.test | RUN | 0.246178 | 0.738281 | 0.821111 | 0.001187 | 0.001232 | 0.001292 |
| segmented_for_each.test | RUN | 0.429352 | 1.357306 | 1.656694 | 0.001121 | 0.001187 | 0.001364 |
| segmented_find.test | RUN | 0.267359 | 0.716561 | 0.928453 | 0.001097 | 0.001406 | 0.001105 |
| segmented_find_if.test | RUN | 0.255532 | 0.731118 | 0.919989 | 0.001187 | 0.001275 | 0.001090 |
| segmented_fold.test | RUN | 0.323660 | 0.945426 | 1.194759 | 0.001220 | 0.001336 | 0.001217 |
| transform.test | RUN | 0.257128 | 0.768821 | 0.851180 | 0.001150 | 0.001272 | 0.001199 |
| join.test | RUN | 0.188694 | 0.457267 | 0.594067 | 0.001294 | 0.001198 | 0.001274 |
| zip.test | RUN | 0.318440 | 0.851911 | 1.142217 | 0.001195 | 0.001282 | 0.001321 |
| zip2.test | RUN | 0.326813 | 0.835798 | 1.099578 | 0.001085 | 0.001176 | 0.001217 |
| zip_ignore.test | RUN | 0.239820 | 0.569075 | 0.721584 | 0.001294 | 0.001127 | 0.001199 |
| flatten.test | RUN | 0.228033 | 0.649577 | 0.731971 | 0.001207 | 0.001177 | 0.001183 |
| ticket-5490.test | COMPILE | 0.124345 | 0.270869 | 0.333505 | — | — | — |
| as_deque.test | RUN | 0.227606 | 0.601238 | 0.778538 | 0.001213 | 0.001322 | 0.001153 |
| as_list.test | RUN | 0.253136 | 0.742041 | 0.852532 | 0.001339 | 0.001286 | 0.001146 |
| as_map.test | RUN | 0.254725 | 0.741217 | 0.846177 | 0.001068 | 0.001139 | 0.001062 |
| as_map_assoc.test | RUN | 0.249109 | 0.695571 | 0.791321 | 0.001093 | 0.001245 | 0.001057 |
| as_set.test | RUN | 0.291085 | 0.879083 | 1.004758 | 0.001111 | 0.001312 | 0.001236 |
| as_vector.test | RUN | 0.257731 | 0.772091 | 0.868880 | 0.001220 | 0.001313 | 0.001162 |
| boost_tuple.test | RUN | 0.263291 | 0.775702 | 0.868141 | 0.001185 | 0.001085 | 0.001007 |
| boost_tuple_iterator.test | RUN | 0.179600 | 0.447234 | 0.514501 | 0.001226 | 0.001267 | 0.001389 |
| cons.test | RUN | 0.326318 | 0.851319 | 1.010555 | 0.001134 | 0.001113 | 0.001058 |
| convert_boost_tuple.test | RUN | 0.205761 | 0.661077 | 0.681002 | 0.001315 | 0.001338 | 0.001087 |
| convert_deque.test | RUN | 0.205729 | 0.650519 | 0.668958 | 0.001152 | 0.001266 | 0.001101 |
| convert_list.test | RUN | 0.205159 | 0.672104 | 0.683675 | 0.001265 | 0.001159 | 0.001016 |
| convert_std_pair.test | RUN | 0.215425 | 0.671977 | 0.690486 | 0.001111 | 0.001272 | 0.001310 |
| convert_std_tuple.test | RUN | 0.203979 | 0.637066 | 0.676234 | 0.001386 | 0.001169 | 0.001297 |
| convert_vector.test | RUN | 0.203420 | 0.660299 | 0.669355 | 0.001117 | 0.001182 | 0.001116 |
| filter_view.test | RUN | 0.252447 | 0.684148 | 0.800816 | 0.001211 | 0.001168 | 0.001401 |
| hash.test | RUN | 0.243726 | 0.624176 | 0.766390 | 0.001292 | 0.001226 | 0.001244 |
| io.test | RUN | 0.260043 | 0.891885 | 0.927353 | 0.001230 | 0.001362 | 0.001552 |
| iterator_range.test | RUN | 0.232493 | 0.672775 | 0.759371 | 0.001306 | 0.001350 | 0.001268 |
| joint_view.test | RUN | 0.323031 | 1.096182 | 1.162919 | 0.001360 | 0.001282 | 0.001240 |
| list_comparison.test | RUN | 0.178447 | 0.457594 | 0.548989 | 0.001203 | 0.001076 | 0.001092 |
| list_construction.test | RUN | 0.174376 | 0.426666 | 0.543164 | 0.001256 | 0.001117 | 0.001212 |
| list_copy.test | RUN | 0.269343 | 0.810041 | 0.945710 | 0.001042 | 0.001249 | 0.001133 |
| list_iterator.test | RUN | 0.178107 | 0.434403 | 0.509997 | 0.001229 | 0.001278 | 0.001260 |
| list_hash.test | RUN | 0.239572 | 0.906662 | 0.761813 | 0.001262 | 0.001135 | 0.001145 |
| list_make.test | RUN | 0.192322 | 0.481464 | 0.597604 | 0.001334 | 0.001096 | 0.001279 |
| list_misc.test | RUN | 0.240605 | 0.575920 | 0.716708 | 0.001163 | 0.001204 | 0.001129 |
| list_mutate.test | RUN | 0.169523 | 0.366523 | 0.477627 | 0.001136 | 0.001070 | 0.001236 |
| list_nest.test | RUN | 0.337862 | 1.110161 | 1.325396 | 0.001041 | 0.001112 | 0.001216 |
| list_tie.test | RUN | 0.178697 | 0.430748 | 0.526408 | 0.001315 | 0.001297 | 0.001164 |
| list_value_at.test | RUN | 0.172547 | 0.404741 | 0.523629 | 0.001231 | 0.001194 | 0.001207 |
| deque_comparison.test | RUN | 0.181541 | 0.460592 | 0.545665 | 0.000999 | 0.001354 | 0.001153 |
| deque_construction.test | RUN | 0.180244 | 0.435191 | 0.546587 | 0.001162 | 0.001206 | 0.001361 |
| deque_copy.test | RUN | 0.255708 | 0.755418 | 0.871219 | 0.001234 | 0.001129 | 0.001120 |
| deque_iterator.test | RUN | 0.179589 | 0.464361 | 0.543197 | 0.001282 | 0.001346 | 0.001242 |
| deque_hash.test | RUN | 0.241858 | 0.884104 | 0.771254 | 0.001057 | 0.001254 | 0.001349 |
| deque_is_constructible.test | COMPILE | 0.094084 | 0.196439 | 0.248482 | — | — | — |
| deque_make.test | RUN | 0.178133 | 0.454311 | 0.566306 | 0.001215 | 0.001231 | 0.001140 |
| deque_misc.test | RUN | 0.238988 | 0.583668 | 0.696091 | 0.001003 | 0.001130 | 0.001101 |
| deque_move.test | RUN | 0.183430 | 0.471212 | 0.551058 | 0.001176 | 0.001214 | 0.001193 |
| deque_mutate.test | RUN | 0.162066 | 0.376486 | 0.473784 | 0.001182 | 0.001362 | 0.001208 |
| deque_nest.test | RUN | 0.506382 | 1.783603 | 2.100626 | 0.001250 | 0.001142 | 0.001202 |
| deque_tie.test | RUN | 0.174168 | 0.405618 | 0.517493 | 0.001054 | 0.001259 | 0.001259 |
| deque_value_at.test | RUN | 0.165814 | 0.401059 | 0.488663 | 0.001270 | 0.001200 | 0.001142 |
| front_extended_deque.test | RUN | 0.245184 | 0.600374 | 0.742437 | 0.001071 | 0.001350 | 0.001136 |
| back_extended_deque.test | RUN | 0.245606 | 0.590940 | 0.734382 | 0.001165 | 0.001100 | 0.001096 |
| make_list.test | RUN | 0.187663 | 0.482491 | 0.595465 | 0.001405 | 0.001262 | 0.001145 |
| make_vector.test | RUN | 0.188207 | 0.467298 | 0.549670 | 0.001115 | 0.001484 | 0.001215 |
| map.test | RUN | 0.248457 | 0.736611 | 0.834836 | 0.001358 | 0.001161 | 0.001156 |
| map_comparison.test | RUN | 0.187962 | 0.470793 | 0.558455 | 0.001109 | 0.001134 | 0.001251 |
| map_construction.test | RUN | 0.180088 | 0.448240 | 0.556977 | 0.001080 | 0.001132 | 0.001089 |
| map_copy.test | RUN | 0.223224 | 0.571333 | 0.678024 | 0.001069 | 0.001127 | 0.001246 |
| map_misc.test | RUN | 0.240213 | 0.579997 | 0.693043 | 0.001077 | 0.001141 | 0.001080 |
| map_move.test | RUN | 0.190342 | 0.484405 | 0.555109 | 0.001095 | 0.001049 | 0.001212 |
| map_mutate.test | RUN | 0.164937 | 0.391271 | 0.480779 | 0.001069 | 0.001133 | 0.001192 |
| map_tie.test | RUN | 0.176061 | 0.382851 | 0.487201 | 0.001109 | 0.001262 | 0.001126 |
| nil.test | RUN | 0.114558 | 0.277736 | 0.340707 | 0.001248 | 0.001163 | 0.001104 |
| nview.test | RUN | 0.237450 | 0.571631 | 0.711174 | 0.001153 | 0.001325 | 0.001170 |
| reverse_view.test | RUN | 0.299741 | 0.949743 | 1.052656 | 0.001208 | 0.001213 | 0.001249 |
| segmented_iterator_range.test | RUN | 0.360953 | 1.094469 | 1.294480 | 0.001196 | 0.001253 | 0.001116 |
| set.test | RUN | 0.225306 | 0.646187 | 0.735197 | 0.001286 | 0.001310 | 0.001116 |
| single_view.test | RUN | 0.211874 | 0.577205 | 0.670699 | 0.001124 | 0.001125 | 0.001077 |
| std_pair.test | RUN | 0.261431 | 0.740879 | 0.869251 | 0.001283 | 0.001159 | 0.001094 |
| boost_array.test | RUN | 0.218884 | 0.504851 | 0.630817 | 0.001269 | 0.001346 | 0.001244 |
| array.test | RUN | 0.220034 | 0.511541 | 0.615527 | 0.001198 | 0.001041 | 0.001083 |
| std_array.test | RUN | 0.221101 | 0.504688 | 0.637106 | 0.001095 | 0.001153 | 0.001175 |
| tuple_comparison.test | RUN | 0.194964 | 0.475203 | 0.584656 | 0.001122 | 0.001155 | 0.001136 |
| tuple_construction.test | RUN | 0.200323 | 0.474052 | 0.588309 | 0.001164 | 0.001277 | 0.001098 |
| tuple_conversion.test | RUN | 0.449458 | 1.469927 | 1.922826 | 0.001092 | 0.001145 | 0.001225 |
| tuple_copy.test | RUN | 0.284079 | 0.859643 | 1.023613 | 0.001119 | 0.001088 | 0.001137 |
| tuple_element.test | RUN | 0.182409 | 0.436704 | 0.546269 | 0.001274 | 0.001219 | 0.001148 |
| tuple_make.test | RUN | 0.198139 | 0.500566 | 0.612681 | 0.001091 | 0.001102 | 0.001158 |
| tuple_misc.test | RUN | 0.257133 | 0.611735 | 0.738570 | 0.001148 | 0.001120 | 0.000929 |
| tuple_mutate.test | RUN | 0.167927 | 0.405771 | 0.547464 | 0.001219 | 0.001223 | 0.001179 |
| tuple_nest.test | RUN | 0.747377 | 3.046999 | 3.607066 | 0.001099 | 0.001243 | 0.001098 |
| tuple_hash.test | RUN | 0.255280 | 0.918874 | 0.788490 | 0.001372 | 0.001187 | 0.001150 |
| tuple_tie.test | RUN | 0.187530 | 0.444559 | 0.559434 | 0.001084 | 0.001259 | 0.001286 |
| tuple_traits__maybe_variadic.test | RUN | 0.204712 | 0.543627 | 0.588058 | 0.001125 | 0.001110 | 0.001140 |
| tuple_traits__no_variadic.test | RUN | 0.223791 | 0.572886 | 0.638263 | 0.001317 | 0.001330 | 0.001432 |
| transform_view.test | RUN | 0.306201 | 0.947638 | 1.037734 | 0.001232 | 0.001380 | 0.001274 |
| identity_view.test | RUN | 0.290715 | 0.881299 | 0.991776 | 0.001314 | 0.001204 | 0.001166 |
| vector_comparison.test | RUN | 0.181189 | 0.461775 | 0.557371 | 0.001131 | 0.001182 | 0.001209 |
| vector_construction.test | RUN | 0.182562 | 0.425706 | 0.588497 | 0.001079 | 0.001313 | 0.001130 |
| vector_conversion.test | RUN | 0.531688 | 1.991280 | 2.491244 | 0.001105 | 0.001177 | 0.001110 |
| vector_copy.test | RUN | 0.276979 | 0.851966 | 1.009395 | 0.001026 | 0.001385 | 0.001105 |
| vector_iterator.test | RUN | 0.182113 | 0.459519 | 0.509315 | 0.001212 | 0.001129 | 0.001202 |
| vector_make.test | RUN | 0.186188 | 0.479072 | 0.552175 | 0.000963 | 0.001170 | 0.001165 |
| vector_misc.test | RUN | 0.242854 | 0.582115 | 0.693758 | 0.001178 | 0.001170 | 0.001189 |
| vector_move.test | RUN | 0.182462 | 0.483813 | 0.584998 | 0.001104 | 0.001232 | 0.001137 |
| vector_mutate.test | RUN | 0.163590 | 0.389664 | 0.469426 | 0.001290 | 0.001105 | 0.000992 |
| vector_n.test | RUN | 0.208740 | 0.490739 | 0.639018 | 0.001232 | 0.001200 | 0.001156 |
| vector_nest.test | RUN | 0.763779 | 3.242936 | 3.668570 | 0.001166 | 0.001001 | 0.001093 |
| vector_hash.test | RUN | 0.242567 | 0.897259 | 0.744031 | 0.001321 | 0.001270 | 0.001173 |
| vector_tie.test | RUN | 0.178027 | 0.437394 | 0.522079 | 0.001286 | 0.001264 | 0.001088 |
| vector_traits__maybe_variadic.test | RUN | 0.188887 | 0.494186 | 0.544882 | 0.001142 | 0.001167 | 0.001270 |
| vector_traits__no_variadic.test | RUN | 0.202555 | 0.547094 | 0.580493 | 0.001213 | 0.001179 | 0.001059 |
| vector_value_at.test | RUN | 0.172244 | 0.392936 | 0.486624 | 0.001156 | 0.001305 | 0.001190 |
| zip_view.test | RUN | 0.358141 | 0.949057 | 1.251010 | 0.001177 | 0.001226 | 0.001128 |
| zip_view2.test | RUN | 0.282014 | 0.764443 | 0.949516 | 0.001220 | 0.001116 | 0.001087 |
| zip_view_ignore.test | RUN | 0.265328 | 0.688466 | 0.882665 | 0.001080 | 0.001209 | 0.001213 |
| repetitive_view.test | RUN | 0.454197 | 1.240109 | 1.646779 | 0.001143 | 0.001216 | 0.001109 |
| deduce_sequence.test | RUN | 0.221547 | 0.531540 | 0.652599 | 0.001336 | 0.001224 | 0.001287 |
| adapt_adt_named.test | RUN | 0.264279 | 0.758126 | 0.877598 | 0.001088 | 0.001103 | 0.001202 |
| adapt_adt_named_empty.test | RUN | 0.233751 | 0.619511 | 0.703588 | 0.001196 | 0.001116 | 0.001248 |
| adapt_adt.test | RUN | 0.296988 | 0.995069 | 1.002227 | 0.001188 | 0.001256 | 0.001037 |
| adapt_adt_empty.test | RUN | 0.224584 | 0.606087 | 0.700202 | 0.001240 | 0.001280 | 0.001174 |
| adapt_assoc_adt_named.test | RUN | 0.289354 | 0.787240 | 0.906589 | 0.001167 | 0.001334 | 0.001154 |
| adapt_assoc_adt_named_empty.test | RUN | 0.223146 | 0.612578 | 0.695876 | 0.001214 | 0.001077 | 0.001147 |
| adapt_assoc_adt.test | RUN | 0.296633 | 0.809093 | 0.951575 | 0.001240 | 0.001128 | 0.001219 |
| adapt_assoc_adt_empty.test | RUN | 0.225114 | 0.621052 | 0.691600 | 0.001380 | 0.001069 | 0.001173 |
| adapt_assoc_struct_named.test | RUN | 0.286929 | 0.778256 | 0.882765 | 0.001110 | 0.001169 | 0.001158 |
| adapt_assoc_struct_named_empty.test | RUN | 0.223232 | 0.612776 | 0.691559 | 0.001151 | 0.001052 | 0.001275 |
| adapt_assoc_struct.test | RUN | 0.257430 | 0.722639 | 0.831237 | 0.001215 | 0.001202 | 0.001082 |
| adapt_assoc_struct_empty.test | RUN | 0.219914 | 0.607208 | 0.694735 | 0.000870 | 0.001338 | 0.001101 |
| adapt_assoc_tpl_adt.test | RUN | 0.303429 | 0.845666 | 0.983085 | 0.001110 | 0.001249 | 0.001167 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.226037 | 0.614987 | 0.692437 | 0.001252 | 0.001220 | 0.001309 |
| adapt_assoc_tpl_struct.test | RUN | 0.246498 | 0.710379 | 0.805324 | 0.001282 | 0.001195 | 0.001171 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.222404 | 0.612779 | 0.693288 | 0.001300 | 0.001350 | 0.001014 |
| adapt_struct_named.test | RUN | 0.248970 | 0.740626 | 0.820285 | 0.001180 | 0.001077 | 0.001305 |
| adapt_struct_named_empty.test | RUN | 0.223953 | 0.618730 | 0.697194 | 0.001087 | 0.001196 | 0.001104 |
| adapt_struct.test | RUN | 0.282870 | 0.838854 | 0.922909 | 0.001237 | 0.001200 | 0.001167 |
| adapt_struct_empty.test | RUN | 0.222897 | 0.613212 | 0.686998 | 0.001437 | 0.001180 | 0.001069 |
| adapt_tpl_adt.test | RUN | 0.286096 | 0.960280 | 0.973018 | 0.001097 | 0.001275 | 0.001161 |
| adapt_tpl_adt_empty.test | RUN | 0.226380 | 0.609028 | 0.695282 | 0.001082 | 0.001294 | 0.001224 |
| adapt_tpl_struct.test | RUN | 0.238140 | 0.669879 | 0.760624 | 0.001116 | 0.001402 | 0.001086 |
| adapt_tpl_struct_empty.test | RUN | 0.222366 | 0.618628 | 0.699066 | 0.001208 | 0.001172 | 0.001072 |
| adt_attribute_proxy.test | RUN | 0.257333 | 0.585889 | 0.731513 | 0.001148 | 0.001271 | 0.001239 |
| define_struct.test | RUN | 0.311415 | 0.829635 | 0.976701 | 0.001162 | 0.001184 | 0.001264 |
| define_struct_empty.test | RUN | 0.265494 | 0.687543 | 0.788802 | 0.001143 | 0.001210 | 0.001197 |
| define_struct_move.test | RUN | 0.172815 | 0.383188 | 0.466581 | 0.001316 | 0.001146 | 0.001197 |
| define_struct_inline.test | RUN | 0.320777 | 1.005763 | 1.121656 | 0.001144 | 0.001194 | 0.001296 |
| define_struct_inline_empty.test | RUN | 0.270881 | 0.674301 | 0.798988 | 0.001098 | 0.001298 | 0.001329 |
| define_struct_inline_move.test | RUN | 0.171366 | 0.393390 | 0.479889 | 0.001253 | 0.001295 | 0.001182 |
| define_assoc_struct.test | RUN | 0.296878 | 0.782678 | 0.937163 | 0.001286 | 0.001208 | 0.001354 |
| define_assoc_struct_empty.test | RUN | 0.269444 | 0.686714 | 0.812424 | 0.001178 | 0.001136 | 0.001222 |
| define_assoc_struct_move.test | RUN | 0.168057 | 0.377811 | 0.479533 | 0.001341 | 0.001233 | 0.001342 |
| define_tpl_struct.test | RUN | 0.299131 | 0.808142 | 0.952238 | 0.001309 | 0.001201 | 0.001244 |
| define_tpl_struct_empty.test | RUN | 0.267699 | 0.683013 | 0.828151 | 0.001280 | 0.001180 | 0.001376 |
| define_tpl_struct_move.test | RUN | 0.174484 | 0.390633 | 0.488418 | 0.001196 | 0.000992 | 0.001396 |
| define_tpl_struct_inline.test | RUN | 0.319443 | 0.930305 | 1.038633 | 0.001242 | 0.001123 | 0.001152 |
| define_tpl_struct_inline_empty.test | RUN | 0.267949 | 0.705164 | 0.834477 | 0.001285 | 0.001246 | 0.001530 |
| define_tpl_struct_inline_move.test | RUN | 0.176441 | 0.374706 | 0.490275 | 0.001011 | 0.001163 | 0.001276 |
| define_assoc_tpl_struct.test | RUN | 0.304401 | 0.793074 | 0.958489 | 0.001302 | 0.001234 | 0.001282 |
| define_assoc_tpl_struct_empty.test | RUN | 0.267091 | 0.689942 | 0.822006 | 0.001204 | 0.001092 | 0.001358 |
| define_assoc_tpl_struct_move.test | RUN | 0.169765 | 0.389642 | 0.493371 | 0.001084 | 0.001442 | 0.001267 |
| std_tuple.test | RUN | 0.179340 | 0.429449 | 0.545376 | 0.001088 | 0.001159 | 0.001281 |
| std_tuple_iterator.test | RUN | 0.182158 | 0.450827 | 0.547697 | 0.001219 | 0.001296 | 0.001235 |
| ref_vector.test | RUN | 0.199068 | 0.458323 | 0.578341 | 0.001272 | 0.001302 | 0.001297 |
| flatten_view.test | RUN | 0.212088 | 0.570197 | 0.698043 | 0.001084 | 0.001230 | 0.001183 |
| github-159.test | COMPILE | 0.097713 | 0.207772 | 0.251198 | — | — | — |
| github-176.test | RUN | 0.194885 | 0.510377 | 0.600425 | 0.001180 | 0.001202 | 0.001258 |
| size.test | COMPILE | 0.162854 | 0.358262 | 0.455745 | — | — | — |
| fused.test | RUN | 0.234207 | 0.544593 | 0.692983 | 0.001210 | 0.001299 | 0.001488 |
| fused_function_object.test | RUN | 0.174949 | 0.414426 | 0.516491 | 0.001187 | 0.001061 | 0.001372 |
| fused_procedure.test | RUN | 0.218349 | 0.517382 | 0.645112 | 0.001120 | 0.001121 | 0.001351 |
| unfused.test | RUN | 0.183548 | 0.416459 | 0.539274 | 0.001066 | 0.001097 | 0.001310 |
| unfused_typed.test | RUN | 0.179192 | 0.452031 | 0.560224 | 0.001069 | 0.001112 | 0.001310 |
| make_fused.test | RUN | 0.242234 | 0.531779 | 0.690189 | 0.001088 | 0.001173 | 0.001487 |
| make_fused_function_object.test | RUN | 0.171507 | 0.421791 | 0.523849 | 0.001091 | 0.001208 | 0.001202 |
| make_fused_procedure.test | RUN | 0.229739 | 0.517342 | 0.655201 | 0.001251 | 0.001288 | 0.001369 |
| make_unfused.test | RUN | 0.180355 | 0.414364 | 0.534702 | 0.001185 | 0.001325 | 0.001384 |
| invoke.test | RUN | 0.459707 | 1.497311 | 1.886822 | 0.001301 | 0.001124 | 0.001346 |
| invoke_function_object.test | RUN | 0.208861 | 0.547392 | 0.687595 | 0.001232 | 0.001201 | 0.001470 |
| invoke_procedure.test | RUN | 0.370207 | 1.113612 | 1.380235 | 0.001230 | 0.001320 | 0.001592 |
| swap.test | RUN | 0.275445 | 0.668724 | 0.872056 | 0.001054 | 0.001134 | 0.001552 |
| is_sequence.test | COMPILE | 0.045620 | 0.093986 | 0.106713 | — | — | — |
| is_view.test | COMPILE | 0.045609 | 0.094742 | 0.109860 | — | — | — |
| pair_deque.test | COMPILE | 0.096869 | 0.213532 | 0.263363 | — | — | — |
| pair_list.test | COMPILE | 0.093291 | 0.212572 | 0.258180 | — | — | — |
| pair_map.test | COMPILE | 0.098811 | 0.222357 | 0.286862 | — | — | — |
| pair_set.test | COMPILE | 0.099527 | 0.219068 | 0.269027 | — | — | — |
| pair_vector.test | COMPILE | 0.097101 | 0.215372 | 0.262667 | — | — | — |
| pair_nest.test | COMPILE | 0.038870 | 0.084750 | 0.105490 | — | — | — |
| index_sequence.test | COMPILE | 0.032866 | 0.059879 | 0.078290 | — | — | — |
| and.test | COMPILE | 0.024211 | 0.045265 | 0.055580 | — | — | — |
| tag_of.test | COMPILE | 0.041655 | 0.089728 | 0.105124 | — | — | — |
| unused.test | COMPILE | 0.022518 | 0.057502 | 0.063344 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.039286 | 0.066682 | 0.108930 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.031054 | 0.084496 | 0.099756 | — | — | — |
| quick.test | RUN | 0.132711 | 0.308252 | 0.342106 | 0.000391 | 0.000855 | 0.000988 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.005× | 1.009×, 1.000× | 1.002× |
| clang | 0.998× | 0.995×, 1.002× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000826s | 0.000796s | 0.000849s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000827s | 0.000796s | 0.000806s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000817s | 0.000823s | 0.000809s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000794s | 0.000811s | 0.000806s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000842s | 0.000852s | 0.000817s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000833s | 0.000826s | 0.000820s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000842s | 0.000851s | 0.000829s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000805s | 0.000835s | 0.000826s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000848s | 0.000836s | 0.000782s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000841s | 0.000818s | 0.000817s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000832s | 0.000811s | 0.000828s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000830s | 0.000802s | 0.000851s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000832s | 0.000846s | 0.000808s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000828s | 0.000806s | 0.000815s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000840s | 0.000820s | 0.000847s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000806s | 0.000821s | 0.000853s | byte equality observed |
| adapt_struct.test | 10 | 0.000823s | 0.000823s | 0.000793s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000831s | 0.000833s | 0.000839s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000823s | 0.000844s | 0.000822s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000804s | 0.000842s | 0.000795s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000823s | 0.000828s | 0.000821s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000823s | 0.000838s | 0.000803s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000845s | 0.000834s | 0.000832s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000816s | 0.000848s | 0.000846s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000802s | 0.000798s | 0.000849s | byte equality observed |
| all.test | 10 | 0.000806s | 0.000789s | 0.000807s | byte equality observed |
| any.test | 10 | 0.000822s | 0.000812s | 0.000830s | byte equality observed |
| array.test | 10 | 0.000798s | 0.000797s | 0.000811s | byte equality observed |
| as_deque.test | 10 | 0.000808s | 0.000815s | 0.000817s | byte equality observed |
| as_list.test | 10 | 0.000836s | 0.000824s | 0.000836s | byte equality observed |
| as_map.test | 10 | 0.000844s | 0.000820s | 0.000826s | byte equality observed |
| as_map_assoc.test | 10 | 0.000821s | 0.000796s | 0.000800s | byte equality observed |
| as_set.test | 10 | 0.000840s | 0.000852s | 0.000814s | byte equality observed |
| as_vector.test | 10 | 0.000831s | 0.000838s | 0.000811s | byte equality observed |
| back_extended_deque.test | 10 | 0.000819s | 0.000814s | 0.000825s | byte equality observed |
| boost_array.test | 10 | 0.000806s | 0.000822s | 0.000810s | byte equality observed |
| boost_tuple.test | 10 | 0.000824s | 0.000815s | 0.000851s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000820s | 0.000807s | 0.000811s | byte equality observed |
| clear.test | 10 | 0.000824s | 0.000802s | 0.000854s | byte equality observed |
| cons.test | 10 | 0.000837s | 0.000834s | 0.000846s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000853s | 0.000806s | 0.000821s | byte equality observed |
| convert_deque.test | 10 | 0.000818s | 0.000824s | 0.000832s | byte equality observed |
| convert_list.test | 10 | 0.000842s | 0.000816s | 0.000796s | byte equality observed |
| convert_std_pair.test | 10 | 0.000811s | 0.000801s | 0.000821s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000821s | 0.000792s | 0.000795s | byte equality observed |
| convert_vector.test | 10 | 0.000800s | 0.000833s | 0.000819s | byte equality observed |
| copy.test | 10 | 0.000800s | 0.000828s | 0.000801s | byte equality observed |
| count.test | 10 | 0.000809s | 0.000831s | 0.000820s | byte equality observed |
| count_if.test | 10 | 0.000817s | 0.000812s | 0.000811s | byte equality observed |
| deduce_sequence.test | 10 | 0.000798s | 0.000815s | 0.000794s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000831s | 0.000860s | 0.000843s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000820s | 0.000824s | 0.000855s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000807s | 0.000801s | 0.000833s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000831s | 0.000805s | 0.000843s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000820s | 0.000813s | 0.000855s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000790s | 0.000800s | 0.000811s | byte equality observed |
| define_struct.test | 10 | 0.000804s | 0.000797s | 0.000801s | byte equality observed |
| define_struct_empty.test | 10 | 0.000819s | 0.000799s | 0.000825s | byte equality observed |
| define_struct_inline.test | 10 | 0.000837s | 0.000816s | 0.000830s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000823s | 0.000822s | 0.000788s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000823s | 0.000803s | 0.000832s | byte equality observed |
| define_struct_move.test | 10 | 0.000815s | 0.000799s | 0.000828s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000827s | 0.000823s | 0.000864s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000807s | 0.000820s | 0.000809s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.000830s | 0.000859s | 0.000836s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000838s | 0.000823s | 0.000866s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000807s | 0.000810s | 0.000834s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000813s | 0.000829s | 0.000801s | byte equality observed |
| deque_comparison.test | 10 | 0.000811s | 0.000801s | 0.000786s | byte equality observed |
| deque_construction.test | 10 | 0.000836s | 0.000822s | 0.000815s | byte equality observed |
| deque_copy.test | 10 | 0.000803s | 0.000786s | 0.000815s | byte equality observed |
| deque_hash.test | 10 | 0.000808s | 0.000813s | 0.000832s | byte equality observed |
| deque_iterator.test | 10 | 0.000804s | 0.000810s | 0.000810s | byte equality observed |
| deque_make.test | 10 | 0.000808s | 0.000835s | 0.000814s | byte equality observed |
| deque_misc.test | 10 | 0.000850s | 0.000810s | 0.000801s | byte equality observed |
| deque_move.test | 10 | 0.000840s | 0.000804s | 0.000795s | byte equality observed |
| deque_mutate.test | 10 | 0.000780s | 0.000817s | 0.000793s | byte equality observed |
| deque_nest.test | 10 | 0.000805s | 0.000800s | 0.000785s | byte equality observed |
| deque_tie.test | 10 | 0.000804s | 0.000806s | 0.000795s | byte equality observed |
| deque_value_at.test | 10 | 0.000815s | 0.000790s | 0.000829s | byte equality observed |
| erase.test | 10 | 0.000841s | 0.000819s | 0.000843s | byte equality observed |
| erase_key.test | 10 | 0.000834s | 0.000866s | 0.000875s | byte equality observed |
| filter.test | 10 | 0.000825s | 0.000848s | 0.000835s | byte equality observed |
| filter_if.test | 10 | 0.000852s | 0.000821s | 0.000813s | byte equality observed |
| filter_view.test | 10 | 0.000815s | 0.000803s | 0.000832s | byte equality observed |
| find.test | 10 | 0.000830s | 0.000808s | 0.000859s | byte equality observed |
| find_if.test | 10 | 0.000811s | 0.000797s | 0.000831s | byte equality observed |
| flatten.test | 10 | 0.000805s | 0.000806s | 0.000834s | byte equality observed |
| flatten_view.test | 10 | 0.000830s | 0.000819s | 0.000824s | byte equality observed |
| fold.test | 10 | 0.000817s | 0.000862s | 0.000844s | byte equality observed |
| for_each.test | 10 | 0.000844s | 0.000826s | 0.000824s | byte equality observed |
| front_extended_deque.test | 10 | 0.000813s | 0.000820s | 0.000842s | byte equality observed |
| fused.test | 10 | 0.000811s | 0.000809s | 0.000795s | byte equality observed |
| fused_function_object.test | 10 | 0.000784s | 0.000785s | 0.000800s | byte equality observed |
| fused_procedure.test | 10 | 0.000819s | 0.000800s | 0.000817s | byte equality observed |
| github-176.test | 10 | 0.000792s | 0.000832s | 0.000804s | byte equality observed |
| hash.test | 10 | 0.000835s | 0.000807s | 0.000807s | byte equality observed |
| identity_view.test | 10 | 0.000832s | 0.000790s | 0.000811s | byte equality observed |
| insert.test | 10 | 0.000841s | 0.000852s | 0.000842s | byte equality observed |
| insert_range.test | 10 | 0.000836s | 0.000860s | 0.000835s | byte equality observed |
| invoke.test | 10 | 0.000824s | 0.000826s | 0.000784s | byte equality observed |
| invoke_function_object.test | 10 | 0.000803s | 0.000797s | 0.000774s | byte equality observed |
| invoke_procedure.test | 10 | 0.000811s | 0.000828s | 0.000820s | byte equality observed |
| io.test | 10 | 0.000901s | 0.000912s | 0.000911s | byte equality observed |
| iter_fold.test | 10 | 0.000819s | 0.000813s | 0.000819s | byte equality observed |
| iterator_range.test | 10 | 0.000832s | 0.000801s | 0.000811s | byte equality observed |
| join.test | 10 | 0.000808s | 0.000800s | 0.000782s | byte equality observed |
| joint_view.test | 10 | 0.000842s | 0.000807s | 0.000825s | byte equality observed |
| list_comparison.test | 10 | 0.000784s | 0.000802s | 0.000819s | byte equality observed |
| list_construction.test | 10 | 0.000789s | 0.000778s | 0.000811s | byte equality observed |
| list_copy.test | 10 | 0.000806s | 0.000799s | 0.000814s | byte equality observed |
| list_hash.test | 10 | 0.000801s | 0.000810s | 0.000807s | byte equality observed |
| list_iterator.test | 10 | 0.000799s | 0.000827s | 0.000816s | byte equality observed |
| list_make.test | 10 | 0.000808s | 0.000823s | 0.000817s | byte equality observed |
| list_misc.test | 10 | 0.000821s | 0.000806s | 0.000831s | byte equality observed |
| list_mutate.test | 10 | 0.000812s | 0.000820s | 0.000804s | byte equality observed |
| list_nest.test | 10 | 0.000811s | 0.000812s | 0.000815s | byte equality observed |
| list_tie.test | 10 | 0.000804s | 0.000789s | 0.000799s | byte equality observed |
| list_value_at.test | 10 | 0.000799s | 0.000833s | 0.000816s | byte equality observed |
| make_fused.test | 10 | 0.000805s | 0.000787s | 0.000795s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000834s | 0.000818s | 0.000802s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000836s | 0.000818s | 0.000840s | byte equality observed |
| make_list.test | 10 | 0.000814s | 0.000815s | 0.000810s | byte equality observed |
| make_unfused.test | 10 | 0.000807s | 0.000857s | 0.000820s | byte equality observed |
| make_vector.test | 10 | 0.000812s | 0.000821s | 0.000805s | byte equality observed |
| map.test | 10 | 0.000824s | 0.000827s | 0.000822s | byte equality observed |
| map_comparison.test | 10 | 0.000820s | 0.000830s | 0.000791s | byte equality observed |
| map_construction.test | 10 | 0.000807s | 0.000810s | 0.000786s | byte equality observed |
| map_copy.test | 10 | 0.000840s | 0.000818s | 0.000820s | byte equality observed |
| map_misc.test | 10 | 0.000817s | 0.000826s | 0.000809s | byte equality observed |
| map_move.test | 10 | 0.000835s | 0.000812s | 0.000803s | byte equality observed |
| map_mutate.test | 10 | 0.000798s | 0.000799s | 0.000779s | byte equality observed |
| map_tie.test | 10 | 0.000811s | 0.000803s | 0.000808s | byte equality observed |
| move.test | 10 | 0.000797s | 0.000774s | 0.000802s | byte equality observed |
| nil.test | 10 | 0.000813s | 0.000797s | 0.000804s | byte equality observed |
| none.test | 10 | 0.000815s | 0.000810s | 0.000802s | byte equality observed |
| nview.test | 10 | 0.000797s | 0.000816s | 0.000816s | byte equality observed |
| pop_back.test | 10 | 0.000833s | 0.000862s | 0.000857s | byte equality observed |
| pop_front.test | 10 | 0.000846s | 0.000834s | 0.000828s | byte equality observed |
| push_back.test | 10 | 0.000849s | 0.000833s | 0.000850s | byte equality observed |
| push_front.test | 10 | 0.000845s | 0.000821s | 0.000847s | byte equality observed |
| quick.test | 10 | 0.000367s | 0.000792s | 0.000802s | byte equality observed |
| ref_vector.test | 10 | 0.000822s | 0.000830s | 0.000788s | byte equality observed |
| remove.test | 10 | 0.000833s | 0.000857s | 0.000820s | byte equality observed |
| remove_if.test | 10 | 0.000827s | 0.000832s | 0.000826s | byte equality observed |
| repetitive_view.test | 10 | 0.000807s | 0.000822s | 0.000790s | byte equality observed |
| replace.test | 10 | 0.000806s | 0.000803s | 0.000795s | byte equality observed |
| replace_if.test | 10 | 0.000840s | 0.000857s | 0.000832s | byte equality observed |
| reverse.test | 10 | 0.000839s | 0.000870s | 0.000823s | byte equality observed |
| reverse_fold.test | 10 | 0.000830s | 0.000835s | 0.000837s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000813s | 0.000812s | 0.000831s | byte equality observed |
| reverse_view.test | 10 | 0.000820s | 0.000821s | 0.000819s | byte equality observed |
| segmented_find.test | 10 | 0.000806s | 0.000835s | 0.000781s | byte equality observed |
| segmented_find_if.test | 10 | 0.000813s | 0.000832s | 0.000785s | byte equality observed |
| segmented_fold.test | 10 | 0.000812s | 0.000854s | 0.000827s | byte equality observed |
| segmented_for_each.test | 10 | 0.000819s | 0.000836s | 0.000811s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.000848s | 0.000879s | 0.000850s | byte equality observed |
| set.test | 10 | 0.000843s | 0.000854s | 0.000833s | byte equality observed |
| single_view.test | 10 | 0.000828s | 0.000841s | 0.000838s | byte equality observed |
| std_array.test | 10 | 0.000812s | 0.000826s | 0.000827s | byte equality observed |
| std_pair.test | 10 | 0.000825s | 0.000834s | 0.000815s | byte equality observed |
| std_tuple.test | 10 | 0.000840s | 0.000817s | 0.000791s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000805s | 0.000798s | 0.000810s | byte equality observed |
| swap.test | 10 | 0.000808s | 0.000811s | 0.000795s | byte equality observed |
| transform.test | 10 | 0.000812s | 0.000840s | 0.000839s | byte equality observed |
| transform_view.test | 10 | 0.000824s | 0.000834s | 0.000831s | byte equality observed |
| tuple_comparison.test | 10 | 0.000800s | 0.000821s | 0.000801s | byte equality observed |
| tuple_construction.test | 10 | 0.000781s | 0.000804s | 0.000826s | byte equality observed |
| tuple_conversion.test | 10 | 0.000805s | 0.000810s | 0.000805s | byte equality observed |
| tuple_copy.test | 10 | 0.000820s | 0.000799s | 0.000802s | byte equality observed |
| tuple_element.test | 10 | 0.000807s | 0.000821s | 0.000818s | byte equality observed |
| tuple_hash.test | 10 | 0.000799s | 0.000815s | 0.000815s | byte equality observed |
| tuple_make.test | 10 | 0.000833s | 0.000834s | 0.000825s | byte equality observed |
| tuple_misc.test | 10 | 0.000805s | 0.000847s | 0.000820s | byte equality observed |
| tuple_mutate.test | 10 | 0.000819s | 0.000803s | 0.000825s | byte equality observed |
| tuple_nest.test | 10 | 0.000797s | 0.000814s | 0.000783s | byte equality observed |
| tuple_tie.test | 10 | 0.000813s | 0.000807s | 0.000815s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000809s | 0.000815s | 0.000797s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000804s | 0.000817s | 0.000815s | byte equality observed |
| unfused.test | 10 | 0.000799s | 0.000802s | 0.000796s | byte equality observed |
| unfused_typed.test | 10 | 0.000829s | 0.000791s | 0.000816s | byte equality observed |
| vector_comparison.test | 10 | 0.000791s | 0.000816s | 0.000805s | byte equality observed |
| vector_construction.test | 10 | 0.000824s | 0.000780s | 0.000809s | byte equality observed |
| vector_conversion.test | 10 | 0.000809s | 0.000809s | 0.000791s | byte equality observed |
| vector_copy.test | 10 | 0.000818s | 0.000785s | 0.000813s | byte equality observed |
| vector_hash.test | 10 | 0.000804s | 0.000788s | 0.000773s | byte equality observed |
| vector_iterator.test | 10 | 0.000829s | 0.000789s | 0.000807s | byte equality observed |
| vector_make.test | 10 | 0.000843s | 0.000792s | 0.000800s | byte equality observed |
| vector_misc.test | 10 | 0.000802s | 0.000796s | 0.000803s | byte equality observed |
| vector_move.test | 10 | 0.000831s | 0.000836s | 0.000849s | byte equality observed |
| vector_mutate.test | 10 | 0.000807s | 0.000782s | 0.000843s | byte equality observed |
| vector_n.test | 10 | 0.000818s | 0.000824s | 0.000850s | byte equality observed |
| vector_nest.test | 10 | 0.000790s | 0.000823s | 0.000818s | byte equality observed |
| vector_tie.test | 10 | 0.000802s | 0.000808s | 0.000776s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000820s | 0.000798s | 0.000801s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000823s | 0.000827s | 0.000797s | byte equality observed |
| vector_value_at.test | 10 | 0.000825s | 0.000833s | 0.000790s | byte equality observed |
| zip.test | 10 | 0.000806s | 0.000841s | 0.000811s | byte equality observed |
| zip2.test | 10 | 0.000809s | 0.000839s | 0.000798s | byte equality observed |
| zip_ignore.test | 10 | 0.000824s | 0.000831s | 0.000822s | byte equality observed |
| zip_view.test | 10 | 0.000802s | 0.000791s | 0.000834s | byte equality observed |
| zip_view2.test | 10 | 0.000800s | 0.000789s | 0.000829s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000794s | 0.000791s | 0.000792s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
