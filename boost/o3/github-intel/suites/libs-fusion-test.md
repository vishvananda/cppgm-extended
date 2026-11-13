# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.333803 | 0.742680 | 0.960758 | 0.001225 | 0.001296 | 0.001279 |
| any.test | RUN | 0.324318 | 0.709854 | 0.905399 | 0.001265 | 0.001317 | 0.001272 |
| clear.test | RUN | 0.238438 | 0.656807 | 0.763313 | 0.001322 | 0.001440 | 0.001328 |
| copy.test | RUN | 0.202371 | 0.488486 | 0.599384 | 0.001387 | 0.001391 | 0.001286 |
| count.test | RUN | 0.223237 | 0.540950 | 0.656355 | 0.001542 | 0.001409 | 0.001295 |
| count_if.test | RUN | 0.210045 | 0.498404 | 0.593214 | 0.001143 | 0.001370 | 0.001253 |
| erase.test | RUN | 0.303332 | 0.881765 | 0.980892 | 0.001211 | 0.001382 | 0.001234 |
| erase_key.test | RUN | 0.289836 | 0.881013 | 0.922248 | 0.001286 | 0.001455 | 0.001323 |
| filter.test | RUN | 0.267123 | 0.741365 | 0.837737 | 0.001453 | 0.001439 | 0.001331 |
| filter_if.test | RUN | 0.296862 | 0.871474 | 0.984163 | 0.001490 | 0.001342 | 0.001321 |
| find.test | RUN | 0.257354 | 0.610977 | 0.750138 | 0.001308 | 0.001253 | 0.001247 |
| find_if.test | RUN | 0.240166 | 0.545315 | 0.684370 | 0.001347 | 0.001431 | 0.001264 |
| fold.test | RUN | 0.269334 | 0.686049 | 0.838557 | 0.001474 | 0.001476 | 0.001264 |
| for_each.test | RUN | 0.235961 | 0.636512 | 0.729232 | 0.001402 | 0.001537 | 0.001293 |
| insert.test | RUN | 0.349732 | 1.080274 | 1.191418 | 0.001397 | 0.001676 | 0.001386 |
| insert_range.test | RUN | 0.352259 | 1.097516 | 1.206819 | 0.001298 | 0.001381 | 0.001321 |
| iter_fold.test | RUN | 0.401482 | 1.040016 | 1.411331 | 0.001575 | 0.001479 | 0.001402 |
| move.test | RUN | 0.207172 | 0.500418 | 0.633108 | 0.001354 | 0.001350 | 0.001651 |
| none.test | RUN | 0.339942 | 0.723446 | 0.904070 | 0.001238 | 0.001410 | 0.001278 |
| pop_back.test | RUN | 0.321093 | 0.971967 | 1.075779 | 0.001396 | 0.001395 | 0.001354 |
| pop_front.test | RUN | 0.271824 | 0.790424 | 0.855112 | 0.001415 | 0.001359 | 0.001164 |
| push_back.test | RUN | 0.346639 | 1.111818 | 1.178504 | 0.001431 | 0.001384 | 0.001298 |
| push_front.test | RUN | 0.303090 | 0.921332 | 0.974923 | 0.001477 | 0.001339 | 0.001312 |
| remove.test | RUN | 0.331890 | 1.029822 | 1.167726 | 0.001221 | 0.001271 | 0.001329 |
| remove_if.test | RUN | 0.298373 | 0.863480 | 0.985846 | 0.001283 | 0.001449 | 0.001327 |
| replace.test | RUN | 0.291742 | 0.851809 | 0.930545 | 0.001476 | 0.001339 | 0.001273 |
| replace_if.test | RUN | 0.289647 | 0.854765 | 0.915309 | 0.001420 | 0.001332 | 0.001374 |
| reverse_fold.test | RUN | 0.438972 | 1.160670 | 1.550401 | 0.001393 | 0.001336 | 0.001271 |
| reverse_iter_fold.test | RUN | 0.448954 | 1.203522 | 1.635793 | 0.001327 | 0.001458 | 0.001244 |
| reverse.test | RUN | 0.284063 | 0.871754 | 0.931033 | 0.001531 | 0.001449 | 0.001286 |
| segmented_for_each.test | RUN | 0.507202 | 1.621392 | 1.913989 | 0.001454 | 0.001296 | 0.001333 |
| segmented_find.test | RUN | 0.324851 | 0.858763 | 1.055732 | 0.001328 | 0.001344 | 0.001304 |
| segmented_find_if.test | RUN | 0.323441 | 0.870147 | 1.077063 | 0.001185 | 0.001374 | 0.001266 |
| segmented_fold.test | RUN | 0.413240 | 1.124077 | 1.358530 | 0.001412 | 0.001470 | 0.001382 |
| transform.test | RUN | 0.297500 | 0.924002 | 1.004563 | 0.001259 | 0.001374 | 0.001328 |
| join.test | RUN | 0.220679 | 0.557949 | 0.685844 | 0.001430 | 0.001359 | 0.001395 |
| zip.test | RUN | 0.392160 | 1.063979 | 1.299801 | 0.001387 | 0.001610 | 0.001411 |
| zip2.test | RUN | 0.374133 | 1.006673 | 1.244504 | 0.001286 | 0.001357 | 0.001292 |
| zip_ignore.test | RUN | 0.282596 | 0.700986 | 0.835083 | 0.001274 | 0.001448 | 0.001181 |
| flatten.test | RUN | 0.273320 | 0.779245 | 0.857666 | 0.001144 | 0.001474 | 0.001353 |
| ticket-5490.test | COMPILE | 0.144140 | 0.328878 | 0.381530 | — | — | — |
| as_deque.test | RUN | 0.271985 | 0.726247 | 0.896908 | 0.001264 | 0.001447 | 0.001231 |
| as_list.test | RUN | 0.305568 | 0.901437 | 0.988017 | 0.001251 | 0.001432 | 0.001233 |
| as_map.test | RUN | 0.296666 | 0.886100 | 0.989625 | 0.001241 | 0.001343 | 0.001235 |
| as_map_assoc.test | RUN | 0.297997 | 0.823089 | 0.918254 | 0.001232 | 0.001377 | 0.001307 |
| as_set.test | RUN | 0.356637 | 1.026533 | 1.135227 | 0.001309 | 0.001484 | 0.001304 |
| as_vector.test | RUN | 0.328876 | 0.915850 | 1.021855 | 0.001298 | 0.001376 | 0.001185 |
| boost_tuple.test | RUN | 0.332963 | 0.922309 | 1.005631 | 0.001213 | 0.001299 | 0.001177 |
| boost_tuple_iterator.test | RUN | 0.229778 | 0.537856 | 0.608352 | 0.001466 | 0.001476 | 0.001342 |
| cons.test | RUN | 0.382912 | 1.020077 | 1.170343 | 0.001252 | 0.001287 | 0.001196 |
| convert_boost_tuple.test | RUN | 0.239892 | 0.778585 | 0.780825 | 0.001477 | 0.001338 | 0.001286 |
| convert_deque.test | RUN | 0.243634 | 0.781916 | 0.777790 | 0.001308 | 0.001469 | 0.001210 |
| convert_list.test | RUN | 0.238359 | 0.798851 | 0.794662 | 0.001185 | 0.001263 | 0.001228 |
| convert_std_pair.test | RUN | 0.256322 | 0.796074 | 0.798722 | 0.001331 | 0.001269 | 0.001273 |
| convert_std_tuple.test | RUN | 0.243693 | 0.774185 | 0.771401 | 0.001357 | 0.001290 | 0.001297 |
| convert_vector.test | RUN | 0.241976 | 0.774116 | 0.787260 | 0.001348 | 0.001326 | 0.001228 |
| filter_view.test | RUN | 0.283701 | 0.817709 | 0.914716 | 0.001294 | 0.001305 | 0.001351 |
| hash.test | RUN | 0.293748 | 0.740689 | 0.875321 | 0.001358 | 0.001262 | 0.001296 |
| io.test | RUN | 0.303364 | 1.066435 | 1.069929 | 0.001384 | 0.001548 | 0.001448 |
| iterator_range.test | RUN | 0.278155 | 0.782738 | 0.889386 | 0.001283 | 0.001391 | 0.001419 |
| joint_view.test | RUN | 0.384441 | 1.316025 | 1.330032 | 0.001392 | 0.001410 | 0.001371 |
| list_comparison.test | RUN | 0.217037 | 0.548019 | 0.648857 | 0.001331 | 0.001313 | 0.001217 |
| list_construction.test | RUN | 0.213287 | 0.529158 | 0.616274 | 0.001296 | 0.001311 | 0.001279 |
| list_copy.test | RUN | 0.325111 | 0.977468 | 1.059026 | 0.001300 | 0.001252 | 0.001203 |
| list_iterator.test | RUN | 0.209438 | 0.542643 | 0.597823 | 0.001324 | 0.001327 | 0.001245 |
| list_hash.test | RUN | 0.290508 | 1.107339 | 0.883142 | 0.001284 | 0.001340 | 0.001206 |
| list_make.test | RUN | 0.220706 | 0.591441 | 0.672634 | 0.001359 | 0.001358 | 0.001272 |
| list_misc.test | RUN | 0.291034 | 0.693380 | 0.822155 | 0.001180 | 0.001406 | 0.001262 |
| list_mutate.test | RUN | 0.200095 | 0.446028 | 0.546807 | 0.001254 | 0.001450 | 0.001248 |
| list_nest.test | RUN | 0.394246 | 1.333141 | 1.534507 | 0.001242 | 0.001385 | 0.001199 |
| list_tie.test | RUN | 0.211339 | 0.524816 | 0.614465 | 0.001281 | 0.001546 | 0.001250 |
| list_value_at.test | RUN | 0.198953 | 0.482170 | 0.587335 | 0.001242 | 0.001323 | 0.001016 |
| deque_comparison.test | RUN | 0.208090 | 0.564199 | 0.643125 | 0.001055 | 0.001315 | 0.001283 |
| deque_construction.test | RUN | 0.214693 | 0.516189 | 0.674208 | 0.001419 | 0.001433 | 0.001206 |
| deque_copy.test | RUN | 0.312419 | 0.895850 | 0.997432 | 0.001175 | 0.001288 | 0.001229 |
| deque_iterator.test | RUN | 0.212288 | 0.575019 | 0.626538 | 0.001266 | 0.001143 | 0.001295 |
| deque_hash.test | RUN | 0.291736 | 1.029683 | 0.884429 | 0.001201 | 0.001428 | 0.001318 |
| deque_is_constructible.test | COMPILE | 0.116131 | 0.247323 | 0.278778 | — | — | — |
| deque_make.test | RUN | 0.218738 | 0.575955 | 0.649591 | 0.001407 | 0.001230 | 0.001257 |
| deque_misc.test | RUN | 0.282496 | 0.673586 | 0.800540 | 0.001256 | 0.001370 | 0.001119 |
| deque_move.test | RUN | 0.216255 | 0.585040 | 0.634786 | 0.001179 | 0.001357 | 0.001246 |
| deque_mutate.test | RUN | 0.200003 | 0.461655 | 0.544405 | 0.001317 | 0.001486 | 0.001236 |
| deque_nest.test | RUN | 0.595244 | 2.094979 | 2.430502 | 0.001221 | 0.001361 | 0.001148 |
| deque_tie.test | RUN | 0.200321 | 0.499046 | 0.577989 | 0.001300 | 0.001413 | 0.001300 |
| deque_value_at.test | RUN | 0.200959 | 0.472163 | 0.565180 | 0.001361 | 0.001367 | 0.000924 |
| front_extended_deque.test | RUN | 0.287789 | 0.707024 | 0.889988 | 0.001296 | 0.001415 | 0.001464 |
| back_extended_deque.test | RUN | 0.290368 | 0.721622 | 0.876081 | 0.001251 | 0.001359 | 0.001297 |
| make_list.test | RUN | 0.226334 | 0.587019 | 0.699291 | 0.001289 | 0.001361 | 0.001347 |
| make_vector.test | RUN | 0.219771 | 0.561606 | 0.693948 | 0.001253 | 0.001398 | 0.001194 |
| map.test | RUN | 0.294087 | 0.888979 | 0.938106 | 0.001610 | 0.001440 | 0.001257 |
| map_comparison.test | RUN | 0.217499 | 0.557453 | 0.645497 | 0.001575 | 0.001384 | 0.001228 |
| map_construction.test | RUN | 0.220637 | 0.535266 | 0.628181 | 0.001287 | 0.001325 | 0.001223 |
| map_copy.test | RUN | 0.265973 | 0.692058 | 0.771156 | 0.001240 | 0.001412 | 0.001214 |
| map_misc.test | RUN | 0.277932 | 0.707495 | 0.803745 | 0.001222 | 0.001314 | 0.001119 |
| map_move.test | RUN | 0.219482 | 0.584390 | 0.647322 | 0.001455 | 0.001416 | 0.001293 |
| map_mutate.test | RUN | 0.197242 | 0.477355 | 0.547833 | 0.001202 | 0.001279 | 0.001214 |
| map_tie.test | RUN | 0.204812 | 0.462578 | 0.567828 | 0.001196 | 0.001305 | 0.001220 |
| nil.test | RUN | 0.137904 | 0.328399 | 0.379538 | 0.001305 | 0.001423 | 0.001270 |
| nview.test | RUN | 0.277951 | 0.677479 | 0.825436 | 0.001296 | 0.001492 | 0.001288 |
| reverse_view.test | RUN | 0.354542 | 1.136910 | 1.220498 | 0.001346 | 0.001459 | 0.001265 |
| segmented_iterator_range.test | RUN | 0.413946 | 1.340165 | 1.496579 | 0.001346 | 0.001428 | 0.001022 |
| set.test | RUN | 0.267809 | 0.756053 | 0.847278 | 0.001292 | 0.001456 | 0.001316 |
| single_view.test | RUN | 0.248607 | 0.698660 | 0.773308 | 0.001327 | 0.001407 | 0.001228 |
| std_pair.test | RUN | 0.316712 | 0.896240 | 0.991982 | 0.001341 | 0.001282 | 0.001313 |
| boost_array.test | RUN | 0.267012 | 0.626830 | 0.732490 | 0.001266 | 0.001537 | 0.001255 |
| array.test | RUN | 0.257373 | 0.616699 | 0.731811 | 0.001305 | 0.001404 | 0.001151 |
| std_array.test | RUN | 0.261588 | 0.631313 | 0.709069 | 0.001321 | 0.001332 | 0.001315 |
| tuple_comparison.test | RUN | 0.230928 | 0.580848 | 0.700655 | 0.001323 | 0.001448 | 0.001300 |
| tuple_construction.test | RUN | 0.238258 | 0.584909 | 0.707613 | 0.001335 | 0.001352 | 0.001275 |
| tuple_conversion.test | RUN | 0.515847 | 1.782602 | 2.216163 | 0.001240 | 0.001322 | 0.001289 |
| tuple_copy.test | RUN | 0.344563 | 1.031408 | 1.191519 | 0.001481 | 0.001446 | 0.001359 |
| tuple_element.test | RUN | 0.215534 | 0.505851 | 0.626390 | 0.001269 | 0.001773 | 0.001228 |
| tuple_make.test | RUN | 0.234833 | 0.596626 | 0.692959 | 0.001267 | 0.001588 | 0.001317 |
| tuple_misc.test | RUN | 0.298975 | 0.731012 | 0.851434 | 0.001380 | 0.001279 | 0.001255 |
| tuple_mutate.test | RUN | 0.204975 | 0.497410 | 0.624119 | 0.001471 | 0.001156 | 0.001314 |
| tuple_nest.test | RUN | 0.866599 | 3.611564 | 4.150231 | 0.001230 | 0.001447 | 0.001278 |
| tuple_hash.test | RUN | 0.299515 | 1.056536 | 0.877758 | 0.001304 | 0.001466 | 0.001234 |
| tuple_tie.test | RUN | 0.223248 | 0.555254 | 0.642160 | 0.001360 | 0.001405 | 0.001297 |
| tuple_traits__maybe_variadic.test | RUN | 0.238580 | 0.658771 | 0.695217 | 0.001228 | 0.001265 | 0.001180 |
| tuple_traits__no_variadic.test | RUN | 0.265677 | 0.679374 | 0.743105 | 0.001284 | 0.001427 | 0.001331 |
| transform_view.test | RUN | 0.353367 | 1.130773 | 1.215837 | 0.001339 | 0.001540 | 0.001282 |
| identity_view.test | RUN | 0.340584 | 1.051559 | 1.150869 | 0.001354 | 0.001312 | 0.001284 |
| vector_comparison.test | RUN | 0.214780 | 0.568053 | 0.642436 | 0.001196 | 0.001518 | 0.001197 |
| vector_construction.test | RUN | 0.215844 | 0.525048 | 0.665045 | 0.001113 | 0.001404 | 0.001245 |
| vector_conversion.test | RUN | 0.623325 | 2.341162 | 2.815122 | 0.001370 | 0.001346 | 0.001242 |
| vector_copy.test | RUN | 0.328809 | 1.005716 | 1.155779 | 0.001300 | 0.001477 | 0.001277 |
| vector_iterator.test | RUN | 0.207199 | 0.562042 | 0.589716 | 0.001452 | 0.001478 | 0.001245 |
| vector_make.test | RUN | 0.218383 | 0.574099 | 0.639178 | 0.001358 | 0.001400 | 0.001325 |
| vector_misc.test | RUN | 0.280081 | 0.730589 | 0.831347 | 0.001252 | 0.001399 | 0.001274 |
| vector_move.test | RUN | 0.223344 | 0.570935 | 0.685878 | 0.001328 | 0.001370 | 0.001279 |
| vector_mutate.test | RUN | 0.198741 | 0.472480 | 0.572149 | 0.001246 | 0.001345 | 0.001247 |
| vector_n.test | RUN | 0.241451 | 0.603923 | 0.723341 | 0.001412 | 0.001438 | 0.001357 |
| vector_nest.test | RUN | 0.881832 | 3.833548 | 4.199581 | 0.001319 | 0.001405 | 0.001206 |
| vector_hash.test | RUN | 0.286449 | 1.050377 | 0.877233 | 0.001324 | 0.001564 | 0.001325 |
| vector_tie.test | RUN | 0.210155 | 0.518899 | 0.594168 | 0.001336 | 0.001318 | 0.001369 |
| vector_traits__maybe_variadic.test | RUN | 0.224925 | 0.640585 | 0.624461 | 0.001435 | 0.001424 | 0.001355 |
| vector_traits__no_variadic.test | RUN | 0.249635 | 0.642849 | 0.650736 | 0.001225 | 0.001332 | 0.001256 |
| vector_value_at.test | RUN | 0.197481 | 0.493498 | 0.590512 | 0.001250 | 0.001338 | 0.001248 |
| zip_view.test | RUN | 0.416841 | 1.158273 | 1.432803 | 0.001441 | 0.001307 | 0.001169 |
| zip_view2.test | RUN | 0.333404 | 0.898059 | 1.102321 | 0.001245 | 0.001458 | 0.001219 |
| zip_view_ignore.test | RUN | 0.316982 | 0.821384 | 1.016786 | 0.001375 | 0.001417 | 0.001258 |
| repetitive_view.test | RUN | 0.544028 | 1.485450 | 1.958975 | 0.001368 | 0.001402 | 0.001217 |
| deduce_sequence.test | RUN | 0.254130 | 0.632618 | 0.755977 | 0.001374 | 0.001486 | 0.001246 |
| adapt_adt_named.test | RUN | 0.301134 | 0.901628 | 0.958599 | 0.001250 | 0.001342 | 0.001371 |
| adapt_adt_named_empty.test | RUN | 0.267992 | 0.722415 | 0.793859 | 0.001262 | 0.001438 | 0.001355 |
| adapt_adt.test | RUN | 0.344142 | 1.191661 | 1.191235 | 0.001282 | 0.001485 | 0.001288 |
| adapt_adt_empty.test | RUN | 0.266418 | 0.734709 | 0.796379 | 0.001265 | 0.001385 | 0.001252 |
| adapt_assoc_adt_named.test | RUN | 0.339337 | 0.941235 | 1.079062 | 0.001250 | 0.001487 | 0.001219 |
| adapt_assoc_adt_named_empty.test | RUN | 0.271270 | 0.721603 | 0.834249 | 0.001324 | 0.001457 | 0.001267 |
| adapt_assoc_adt.test | RUN | 0.358365 | 0.972380 | 1.125374 | 0.001433 | 0.001455 | 0.001288 |
| adapt_assoc_adt_empty.test | RUN | 0.264378 | 0.718093 | 0.828819 | 0.001270 | 0.001421 | 0.001246 |
| adapt_assoc_struct_named.test | RUN | 0.342449 | 0.925592 | 1.035871 | 0.001266 | 0.001381 | 0.001312 |
| adapt_assoc_struct_named_empty.test | RUN | 0.261044 | 0.719062 | 0.792000 | 0.001064 | 0.001474 | 0.001411 |
| adapt_assoc_struct.test | RUN | 0.304680 | 0.874339 | 0.973911 | 0.001273 | 0.001439 | 0.001336 |
| adapt_assoc_struct_empty.test | RUN | 0.260448 | 0.712096 | 0.791264 | 0.001281 | 0.001446 | 0.001256 |
| adapt_assoc_tpl_adt.test | RUN | 0.362042 | 1.009707 | 1.150070 | 0.001244 | 0.001459 | 0.001248 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.268244 | 0.730322 | 0.803155 | 0.001272 | 0.001458 | 0.001341 |
| adapt_assoc_tpl_struct.test | RUN | 0.294988 | 0.860427 | 0.939678 | 0.001343 | 0.001526 | 0.001223 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.265677 | 0.722519 | 0.793107 | 0.001207 | 0.001435 | 0.001271 |
| adapt_struct_named.test | RUN | 0.300059 | 0.889916 | 0.961867 | 0.001285 | 0.001479 | 0.001324 |
| adapt_struct_named_empty.test | RUN | 0.266362 | 0.719108 | 0.822579 | 0.001287 | 0.001382 | 0.001157 |
| adapt_struct.test | RUN | 0.328016 | 1.006287 | 1.083314 | 0.001454 | 0.001413 | 0.001366 |
| adapt_struct_empty.test | RUN | 0.264348 | 0.727104 | 0.809497 | 0.001555 | 0.001458 | 0.001195 |
| adapt_tpl_adt.test | RUN | 0.343963 | 1.171797 | 1.120884 | 0.001361 | 0.001439 | 0.001290 |
| adapt_tpl_adt_empty.test | RUN | 0.268452 | 0.739242 | 0.801418 | 0.001316 | 0.001394 | 0.001405 |
| adapt_tpl_struct.test | RUN | 0.285477 | 0.809058 | 0.883435 | 0.001404 | 0.001500 | 0.001202 |
| adapt_tpl_struct_empty.test | RUN | 0.265171 | 0.738817 | 0.797852 | 0.001284 | 0.001379 | 0.001249 |
| adt_attribute_proxy.test | RUN | 0.296863 | 0.710749 | 0.852172 | 0.001369 | 0.001560 | 0.001190 |
| define_struct.test | RUN | 0.369379 | 1.005734 | 1.130075 | 0.001331 | 0.001487 | 0.001279 |
| define_struct_empty.test | RUN | 0.317499 | 0.823749 | 0.930697 | 0.001299 | 0.001351 | 0.001334 |
| define_struct_move.test | RUN | 0.208184 | 0.462687 | 0.543343 | 0.001360 | 0.001384 | 0.001251 |
| define_struct_inline.test | RUN | 0.387489 | 1.202513 | 1.269695 | 0.001335 | 0.001402 | 0.001315 |
| define_struct_inline_empty.test | RUN | 0.317870 | 0.823507 | 0.930676 | 0.001320 | 0.001286 | 0.001160 |
| define_struct_inline_move.test | RUN | 0.205788 | 0.467140 | 0.562299 | 0.001252 | 0.001235 | 0.001286 |
| define_assoc_struct.test | RUN | 0.355015 | 0.951254 | 1.070801 | 0.001426 | 0.001449 | 0.001442 |
| define_assoc_struct_empty.test | RUN | 0.316704 | 0.820728 | 0.928591 | 0.001217 | 0.001410 | 0.001372 |
| define_assoc_struct_move.test | RUN | 0.207614 | 0.463711 | 0.549659 | 0.001213 | 0.001431 | 0.001007 |
| define_tpl_struct.test | RUN | 0.355231 | 0.965063 | 1.064138 | 0.001421 | 0.001380 | 0.001266 |
| define_tpl_struct_empty.test | RUN | 0.323431 | 0.839266 | 0.931564 | 0.001299 | 0.001453 | 0.001326 |
| define_tpl_struct_move.test | RUN | 0.202038 | 0.460171 | 0.553874 | 0.001479 | 0.001396 | 0.001351 |
| define_tpl_struct_inline.test | RUN | 0.373390 | 1.132949 | 1.175396 | 0.001302 | 0.001382 | 0.001303 |
| define_tpl_struct_inline_empty.test | RUN | 0.316510 | 0.815599 | 0.935739 | 0.001411 | 0.001279 | 0.001315 |
| define_tpl_struct_inline_move.test | RUN | 0.203604 | 0.479528 | 0.564775 | 0.001365 | 0.001494 | 0.001268 |
| define_assoc_tpl_struct.test | RUN | 0.356515 | 0.970885 | 1.081384 | 0.001368 | 0.001502 | 0.001287 |
| define_assoc_tpl_struct_empty.test | RUN | 0.311115 | 0.826359 | 0.941211 | 0.001219 | 0.001376 | 0.001265 |
| define_assoc_tpl_struct_move.test | RUN | 0.202166 | 0.466630 | 0.566192 | 0.001236 | 0.001461 | 0.001430 |
| std_tuple.test | RUN | 0.213835 | 0.518759 | 0.620292 | 0.001283 | 0.001389 | 0.001449 |
| std_tuple_iterator.test | RUN | 0.213087 | 0.547848 | 0.619501 | 0.001445 | 0.001418 | 0.001225 |
| ref_vector.test | RUN | 0.241736 | 0.576459 | 0.663389 | 0.001347 | 0.001370 | 0.001368 |
| flatten_view.test | RUN | 0.252588 | 0.687101 | 0.785789 | 0.001312 | 0.001441 | 0.001292 |
| github-159.test | COMPILE | 0.113616 | 0.257133 | 0.282912 | — | — | — |
| github-176.test | RUN | 0.235880 | 0.618531 | 0.693335 | 0.001349 | 0.001422 | 0.001414 |
| size.test | COMPILE | 0.194954 | 0.426759 | 0.538039 | — | — | — |
| fused.test | RUN | 0.278568 | 0.664425 | 0.803959 | 0.001173 | 0.001503 | 0.001328 |
| fused_function_object.test | RUN | 0.208743 | 0.506247 | 0.589701 | 0.001357 | 0.001277 | 0.001344 |
| fused_procedure.test | RUN | 0.272548 | 0.624078 | 0.751380 | 0.001321 | 0.001385 | 0.001299 |
| unfused.test | RUN | 0.216843 | 0.523597 | 0.604687 | 0.001280 | 0.001429 | 0.001244 |
| unfused_typed.test | RUN | 0.221633 | 0.555886 | 0.628025 | 0.001372 | 0.001377 | 0.001342 |
| make_fused.test | RUN | 0.285833 | 0.667127 | 0.771015 | 0.001364 | 0.001357 | 0.001251 |
| make_fused_function_object.test | RUN | 0.213058 | 0.493335 | 0.594373 | 0.001261 | 0.001368 | 0.001401 |
| make_fused_procedure.test | RUN | 0.271509 | 0.635344 | 0.742841 | 0.001315 | 0.001490 | 0.001384 |
| make_unfused.test | RUN | 0.212396 | 0.496705 | 0.600276 | 0.001410 | 0.001414 | 0.001260 |
| invoke.test | RUN | 0.548302 | 1.826913 | 2.077510 | 0.001242 | 0.001372 | 0.001294 |
| invoke_function_object.test | RUN | 0.260867 | 0.631574 | 0.770645 | 0.001333 | 0.001510 | 0.001361 |
| invoke_procedure.test | RUN | 0.444872 | 1.341518 | 1.542779 | 0.001156 | 0.001430 | 0.001374 |
| swap.test | RUN | 0.322995 | 0.842305 | 0.996029 | 0.001181 | 0.001402 | 0.001332 |
| is_sequence.test | COMPILE | 0.052963 | 0.117180 | 0.123920 | — | — | — |
| is_view.test | COMPILE | 0.052723 | 0.117581 | 0.127590 | — | — | — |
| pair_deque.test | COMPILE | 0.116656 | 0.267698 | 0.320619 | — | — | — |
| pair_list.test | COMPILE | 0.115133 | 0.259957 | 0.288876 | — | — | — |
| pair_map.test | COMPILE | 0.117913 | 0.269307 | 0.309900 | — | — | — |
| pair_set.test | COMPILE | 0.121154 | 0.267396 | 0.303986 | — | — | — |
| pair_vector.test | COMPILE | 0.116852 | 0.259991 | 0.293354 | — | — | — |
| pair_nest.test | COMPILE | 0.046422 | 0.105457 | 0.114326 | — | — | — |
| index_sequence.test | COMPILE | 0.039960 | 0.076412 | 0.084955 | — | — | — |
| and.test | COMPILE | 0.029654 | 0.054608 | 0.060216 | — | — | — |
| tag_of.test | COMPILE | 0.052135 | 0.112379 | 0.114401 | — | — | — |
| unused.test | COMPILE | 0.028852 | 0.073623 | 0.069771 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.045619 | 0.114772 | 0.105926 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.036433 | 0.106228 | 0.086405 | — | — | — |
| quick.test | RUN | 0.158626 | 0.344031 | 0.379425 | 0.000416 | 0.000989 | 0.000944 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 1.001×, 0.992× | 1.006× |
| clang | 1.002× | 1.000×, 1.003× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000934s | 0.000904s | 0.000916s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000906s | 0.000912s | 0.000901s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000910s | 0.000894s | 0.000913s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000920s | 0.000873s | 0.000896s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000895s | 0.000909s | 0.000896s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000903s | 0.000893s | 0.000896s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000898s | 0.000884s | 0.000922s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000890s | 0.000899s | 0.000909s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000909s | 0.000885s | 0.000907s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000911s | 0.000883s | 0.000899s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000914s | 0.000876s | 0.000919s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000898s | 0.000904s | 0.000908s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000891s | 0.000910s | 0.000921s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000894s | 0.000898s | 0.000909s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000899s | 0.000903s | 0.000941s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000902s | 0.000910s | 0.000915s | byte equality observed |
| adapt_struct.test | 10 | 0.000916s | 0.000912s | 0.000930s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000889s | 0.000910s | 0.000912s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000891s | 0.000895s | 0.000930s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000898s | 0.000908s | 0.000921s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000914s | 0.000911s | 0.000927s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000900s | 0.000905s | 0.000917s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000906s | 0.000908s | 0.000938s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000907s | 0.000903s | 0.000924s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000891s | 0.000904s | 0.000904s | byte equality observed |
| all.test | 10 | 0.000879s | 0.000885s | 0.000894s | byte equality observed |
| any.test | 10 | 0.000881s | 0.000893s | 0.000911s | byte equality observed |
| array.test | 10 | 0.000890s | 0.000898s | 0.000901s | byte equality observed |
| as_deque.test | 10 | 0.000900s | 0.000912s | 0.000878s | byte equality observed |
| as_list.test | 10 | 0.000929s | 0.000942s | 0.000926s | byte equality observed |
| as_map.test | 10 | 0.000913s | 0.000940s | 0.000918s | byte equality observed |
| as_map_assoc.test | 10 | 0.000892s | 0.000927s | 0.000918s | byte equality observed |
| as_set.test | 10 | 0.000913s | 0.000933s | 0.000926s | byte equality observed |
| as_vector.test | 10 | 0.000908s | 0.000914s | 0.000943s | byte equality observed |
| back_extended_deque.test | 10 | 0.000883s | 0.000906s | 0.000906s | byte equality observed |
| boost_array.test | 10 | 0.000899s | 0.000899s | 0.000911s | byte equality observed |
| boost_tuple.test | 10 | 0.000910s | 0.000891s | 0.000920s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000881s | 0.000889s | 0.000883s | byte equality observed |
| clear.test | 10 | 0.000883s | 0.000912s | 0.000909s | byte equality observed |
| cons.test | 10 | 0.000937s | 0.000901s | 0.000935s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000889s | 0.000890s | 0.000891s | byte equality observed |
| convert_deque.test | 10 | 0.000907s | 0.000894s | 0.000917s | byte equality observed |
| convert_list.test | 10 | 0.000898s | 0.000892s | 0.000905s | byte equality observed |
| convert_std_pair.test | 10 | 0.000883s | 0.000886s | 0.000924s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000874s | 0.000896s | 0.000921s | byte equality observed |
| convert_vector.test | 10 | 0.000891s | 0.000917s | 0.000889s | byte equality observed |
| copy.test | 10 | 0.000875s | 0.000877s | 0.000884s | byte equality observed |
| count.test | 10 | 0.000902s | 0.000892s | 0.000896s | byte equality observed |
| count_if.test | 10 | 0.000888s | 0.000889s | 0.000896s | byte equality observed |
| deduce_sequence.test | 10 | 0.000919s | 0.000876s | 0.000894s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000900s | 0.000917s | 0.000906s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000922s | 0.000897s | 0.000917s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000909s | 0.000879s | 0.000893s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000888s | 0.000905s | 0.000896s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000897s | 0.000912s | 0.000935s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000870s | 0.000857s | 0.000895s | byte equality observed |
| define_struct.test | 10 | 0.000938s | 0.000927s | 0.000922s | byte equality observed |
| define_struct_empty.test | 10 | 0.000892s | 0.000901s | 0.000914s | byte equality observed |
| define_struct_inline.test | 10 | 0.000905s | 0.000928s | 0.000933s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000923s | 0.000907s | 0.000901s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000888s | 0.000873s | 0.000884s | byte equality observed |
| define_struct_move.test | 10 | 0.000878s | 0.000877s | 0.000883s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000891s | 0.000924s | 0.000884s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000907s | 0.000921s | 0.000900s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.000906s | 0.000905s | 0.000905s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000916s | 0.000910s | 0.000891s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000878s | 0.000906s | 0.000874s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000897s | 0.000892s | 0.000908s | byte equality observed |
| deque_comparison.test | 10 | 0.000898s | 0.000887s | 0.000884s | byte equality observed |
| deque_construction.test | 10 | 0.000910s | 0.000856s | 0.000906s | byte equality observed |
| deque_copy.test | 10 | 0.000892s | 0.000910s | 0.000907s | byte equality observed |
| deque_hash.test | 10 | 0.000895s | 0.000909s | 0.000912s | byte equality observed |
| deque_iterator.test | 10 | 0.000890s | 0.000909s | 0.000881s | byte equality observed |
| deque_make.test | 10 | 0.000892s | 0.000898s | 0.000880s | byte equality observed |
| deque_misc.test | 10 | 0.000897s | 0.000897s | 0.000885s | byte equality observed |
| deque_move.test | 10 | 0.000904s | 0.000925s | 0.000905s | byte equality observed |
| deque_mutate.test | 10 | 0.000891s | 0.000869s | 0.000897s | byte equality observed |
| deque_nest.test | 10 | 0.000914s | 0.000875s | 0.000913s | byte equality observed |
| deque_tie.test | 10 | 0.000910s | 0.000873s | 0.000896s | byte equality observed |
| deque_value_at.test | 10 | 0.000887s | 0.000878s | 0.000876s | byte equality observed |
| erase.test | 10 | 0.000902s | 0.000914s | 0.000921s | byte equality observed |
| erase_key.test | 10 | 0.000945s | 0.000938s | 0.000906s | byte equality observed |
| filter.test | 10 | 0.000894s | 0.000915s | 0.000905s | byte equality observed |
| filter_if.test | 10 | 0.000908s | 0.000908s | 0.000913s | byte equality observed |
| filter_view.test | 10 | 0.000925s | 0.000892s | 0.000900s | byte equality observed |
| find.test | 10 | 0.000914s | 0.000917s | 0.000897s | byte equality observed |
| find_if.test | 10 | 0.000909s | 0.000918s | 0.000880s | byte equality observed |
| flatten.test | 10 | 0.000910s | 0.000870s | 0.000899s | byte equality observed |
| flatten_view.test | 10 | 0.000914s | 0.000890s | 0.000867s | byte equality observed |
| fold.test | 10 | 0.000916s | 0.000885s | 0.000926s | byte equality observed |
| for_each.test | 10 | 0.000919s | 0.000926s | 0.000910s | byte equality observed |
| front_extended_deque.test | 10 | 0.000879s | 0.000866s | 0.000895s | byte equality observed |
| fused.test | 10 | 0.000906s | 0.000903s | 0.000895s | byte equality observed |
| fused_function_object.test | 10 | 0.000900s | 0.000880s | 0.000898s | byte equality observed |
| fused_procedure.test | 10 | 0.000899s | 0.000878s | 0.000898s | byte equality observed |
| github-176.test | 10 | 0.000890s | 0.000883s | 0.000909s | byte equality observed |
| hash.test | 10 | 0.000896s | 0.000886s | 0.000924s | byte equality observed |
| identity_view.test | 10 | 0.000928s | 0.000921s | 0.000894s | byte equality observed |
| insert.test | 10 | 0.000949s | 0.000936s | 0.000900s | byte equality observed |
| insert_range.test | 10 | 0.000919s | 0.000931s | 0.000915s | byte equality observed |
| invoke.test | 10 | 0.000881s | 0.000898s | 0.000898s | byte equality observed |
| invoke_function_object.test | 10 | 0.000882s | 0.000909s | 0.000898s | byte equality observed |
| invoke_procedure.test | 10 | 0.000908s | 0.000892s | 0.000893s | byte equality observed |
| io.test | 10 | 0.001035s | 0.001009s | 0.000998s | byte equality observed |
| iter_fold.test | 10 | 0.000917s | 0.000895s | 0.000893s | byte equality observed |
| iterator_range.test | 10 | 0.000932s | 0.000906s | 0.000898s | byte equality observed |
| join.test | 10 | 0.000889s | 0.000897s | 0.000898s | byte equality observed |
| joint_view.test | 10 | 0.000922s | 0.000948s | 0.000948s | byte equality observed |
| list_comparison.test | 10 | 0.000887s | 0.000867s | 0.000872s | byte equality observed |
| list_construction.test | 10 | 0.000908s | 0.000863s | 0.000880s | byte equality observed |
| list_copy.test | 10 | 0.000895s | 0.000883s | 0.000892s | byte equality observed |
| list_hash.test | 10 | 0.000891s | 0.000915s | 0.000899s | byte equality observed |
| list_iterator.test | 10 | 0.000873s | 0.000865s | 0.000877s | byte equality observed |
| list_make.test | 10 | 0.000912s | 0.000884s | 0.000889s | byte equality observed |
| list_misc.test | 10 | 0.000871s | 0.000921s | 0.000863s | byte equality observed |
| list_mutate.test | 10 | 0.000874s | 0.000886s | 0.000915s | byte equality observed |
| list_nest.test | 10 | 0.000883s | 0.000899s | 0.000886s | byte equality observed |
| list_tie.test | 10 | 0.000902s | 0.000877s | 0.000930s | byte equality observed |
| list_value_at.test | 10 | 0.000913s | 0.000893s | 0.000912s | byte equality observed |
| make_fused.test | 10 | 0.000905s | 0.000867s | 0.000894s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000888s | 0.000876s | 0.000920s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000911s | 0.000859s | 0.000915s | byte equality observed |
| make_list.test | 10 | 0.000884s | 0.000878s | 0.000896s | byte equality observed |
| make_unfused.test | 10 | 0.000888s | 0.000896s | 0.000905s | byte equality observed |
| make_vector.test | 10 | 0.000907s | 0.000889s | 0.000899s | byte equality observed |
| map.test | 10 | 0.000921s | 0.000924s | 0.000903s | byte equality observed |
| map_comparison.test | 10 | 0.000876s | 0.000862s | 0.000904s | byte equality observed |
| map_construction.test | 10 | 0.000925s | 0.000864s | 0.000884s | byte equality observed |
| map_copy.test | 10 | 0.000886s | 0.000912s | 0.000884s | byte equality observed |
| map_misc.test | 10 | 0.000906s | 0.000940s | 0.000901s | byte equality observed |
| map_move.test | 10 | 0.000907s | 0.000902s | 0.000912s | byte equality observed |
| map_mutate.test | 10 | 0.000889s | 0.000909s | 0.000878s | byte equality observed |
| map_tie.test | 10 | 0.000893s | 0.000887s | 0.000895s | byte equality observed |
| move.test | 10 | 0.000903s | 0.000871s | 0.000863s | byte equality observed |
| nil.test | 10 | 0.000908s | 0.000858s | 0.000893s | byte equality observed |
| none.test | 10 | 0.000897s | 0.000915s | 0.000909s | byte equality observed |
| nview.test | 10 | 0.000897s | 0.000877s | 0.000866s | byte equality observed |
| pop_back.test | 10 | 0.000927s | 0.000917s | 0.000941s | byte equality observed |
| pop_front.test | 10 | 0.000915s | 0.000928s | 0.000923s | byte equality observed |
| push_back.test | 10 | 0.000930s | 0.000916s | 0.000921s | byte equality observed |
| push_front.test | 10 | 0.000919s | 0.000910s | 0.000933s | byte equality observed |
| quick.test | 10 | 0.000387s | 0.000885s | 0.000848s | byte equality observed |
| ref_vector.test | 10 | 0.000910s | 0.000898s | 0.000924s | byte equality observed |
| remove.test | 10 | 0.000933s | 0.000919s | 0.000945s | byte equality observed |
| remove_if.test | 10 | 0.000920s | 0.000921s | 0.000948s | byte equality observed |
| repetitive_view.test | 10 | 0.000882s | 0.000876s | 0.000895s | byte equality observed |
| replace.test | 10 | 0.000897s | 0.000897s | 0.000925s | byte equality observed |
| replace_if.test | 10 | 0.000895s | 0.000920s | 0.000919s | byte equality observed |
| reverse.test | 10 | 0.000910s | 0.000907s | 0.000928s | byte equality observed |
| reverse_fold.test | 10 | 0.000904s | 0.000881s | 0.000897s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000904s | 0.000891s | 0.000899s | byte equality observed |
| reverse_view.test | 10 | 0.000912s | 0.000897s | 0.000906s | byte equality observed |
| segmented_find.test | 10 | 0.000877s | 0.000910s | 0.000899s | byte equality observed |
| segmented_find_if.test | 10 | 0.000884s | 0.000888s | 0.000901s | byte equality observed |
| segmented_fold.test | 10 | 0.000917s | 0.000923s | 0.000942s | byte equality observed |
| segmented_for_each.test | 10 | 0.000924s | 0.000920s | 0.000913s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.000932s | 0.000954s | 0.000971s | byte equality observed |
| set.test | 10 | 0.000892s | 0.000888s | 0.000897s | byte equality observed |
| single_view.test | 10 | 0.000926s | 0.000898s | 0.000893s | byte equality observed |
| std_array.test | 10 | 0.000881s | 0.000886s | 0.000883s | byte equality observed |
| std_pair.test | 10 | 0.000917s | 0.000911s | 0.000935s | byte equality observed |
| std_tuple.test | 10 | 0.000910s | 0.000881s | 0.000867s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000900s | 0.000915s | 0.000859s | byte equality observed |
| swap.test | 10 | 0.000897s | 0.000912s | 0.000895s | byte equality observed |
| transform.test | 10 | 0.000906s | 0.000932s | 0.000916s | byte equality observed |
| transform_view.test | 10 | 0.000924s | 0.000935s | 0.000905s | byte equality observed |
| tuple_comparison.test | 10 | 0.000880s | 0.000885s | 0.000891s | byte equality observed |
| tuple_construction.test | 10 | 0.000866s | 0.000901s | 0.000908s | byte equality observed |
| tuple_conversion.test | 10 | 0.000883s | 0.000883s | 0.000911s | byte equality observed |
| tuple_copy.test | 10 | 0.000901s | 0.000887s | 0.000869s | byte equality observed |
| tuple_element.test | 10 | 0.000879s | 0.000884s | 0.000902s | byte equality observed |
| tuple_hash.test | 10 | 0.000890s | 0.000884s | 0.000932s | byte equality observed |
| tuple_make.test | 10 | 0.000896s | 0.000900s | 0.000919s | byte equality observed |
| tuple_misc.test | 10 | 0.000892s | 0.000909s | 0.000881s | byte equality observed |
| tuple_mutate.test | 10 | 0.000897s | 0.000900s | 0.000885s | byte equality observed |
| tuple_nest.test | 10 | 0.000876s | 0.000869s | 0.000892s | byte equality observed |
| tuple_tie.test | 10 | 0.000887s | 0.000889s | 0.000898s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000918s | 0.000890s | 0.000908s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000893s | 0.000896s | 0.000912s | byte equality observed |
| unfused.test | 10 | 0.000892s | 0.000878s | 0.000880s | byte equality observed |
| unfused_typed.test | 10 | 0.000876s | 0.000907s | 0.000915s | byte equality observed |
| vector_comparison.test | 10 | 0.000891s | 0.000904s | 0.000917s | byte equality observed |
| vector_construction.test | 10 | 0.000870s | 0.000882s | 0.000913s | byte equality observed |
| vector_conversion.test | 10 | 0.000884s | 0.000912s | 0.000915s | byte equality observed |
| vector_copy.test | 10 | 0.000905s | 0.000898s | 0.000877s | byte equality observed |
| vector_hash.test | 10 | 0.000906s | 0.000888s | 0.000888s | byte equality observed |
| vector_iterator.test | 10 | 0.000908s | 0.000902s | 0.000907s | byte equality observed |
| vector_make.test | 10 | 0.000918s | 0.000888s | 0.000895s | byte equality observed |
| vector_misc.test | 10 | 0.000900s | 0.000916s | 0.000879s | byte equality observed |
| vector_move.test | 10 | 0.000904s | 0.000886s | 0.000905s | byte equality observed |
| vector_mutate.test | 10 | 0.000880s | 0.000890s | 0.000886s | byte equality observed |
| vector_n.test | 10 | 0.000908s | 0.000914s | 0.000903s | byte equality observed |
| vector_nest.test | 10 | 0.000882s | 0.000870s | 0.000887s | byte equality observed |
| vector_tie.test | 10 | 0.000913s | 0.000882s | 0.000895s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000868s | 0.000883s | 0.000886s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000890s | 0.000910s | 0.000897s | byte equality observed |
| vector_value_at.test | 10 | 0.000894s | 0.000901s | 0.000923s | byte equality observed |
| zip.test | 10 | 0.000876s | 0.000866s | 0.000919s | byte equality observed |
| zip2.test | 10 | 0.000896s | 0.000889s | 0.000909s | byte equality observed |
| zip_ignore.test | 10 | 0.000871s | 0.000901s | 0.000907s | byte equality observed |
| zip_view.test | 10 | 0.000875s | 0.000910s | 0.000890s | byte equality observed |
| zip_view2.test | 10 | 0.000872s | 0.000868s | 0.000875s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000901s | 0.000873s | 0.000869s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
