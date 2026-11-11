# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.294180 | 0.517204 | 0.646120 | 0.001312 | 0.001355 | 0.001217 |
| any.test | RUN | 0.279594 | 0.488353 | 0.620952 | 0.001354 | 0.001245 | 0.001408 |
| clear.test | RUN | 0.197040 | 0.464331 | 0.518178 | 0.001200 | 0.001372 | 0.001470 |
| copy.test | RUN | 0.173669 | 0.345835 | 0.413421 | 0.001391 | 0.001454 | 0.001397 |
| count.test | RUN | 0.192553 | 0.384165 | 0.476579 | 0.001295 | 0.001540 | 0.001432 |
| count_if.test | RUN | 0.182206 | 0.360074 | 0.435081 | 0.001647 | 0.001464 | 0.001411 |
| erase.test | RUN | 0.257259 | 0.624794 | 0.710036 | 0.001425 | 0.001579 | 0.001488 |
| erase_key.test | RUN | 0.256866 | 0.623673 | 0.683326 | 0.001324 | 0.001428 | 0.001230 |
| filter.test | RUN | 0.221830 | 0.529003 | 0.617017 | 0.001524 | 0.001637 | 0.001543 |
| filter_if.test | RUN | 0.245469 | 0.633885 | 0.698932 | 0.001358 | 0.001454 | 0.001632 |
| find.test | RUN | 0.215150 | 0.423170 | 0.532911 | 0.001451 | 0.001441 | 0.001369 |
| find_if.test | RUN | 0.205823 | 0.394171 | 0.483792 | 0.001287 | 0.001251 | 0.001622 |
| fold.test | RUN | 0.228593 | 0.478250 | 0.594732 | 0.001610 | 0.001633 | 0.001165 |
| for_each.test | RUN | 0.204336 | 0.438637 | 0.524626 | 0.001389 | 0.001665 | 0.001585 |
| insert.test | RUN | 0.295022 | 0.773070 | 0.841104 | 0.001587 | 0.001489 | 0.001447 |
| insert_range.test | RUN | 0.296814 | 0.777556 | 0.874654 | 0.001475 | 0.001287 | 0.001544 |
| iter_fold.test | RUN | 0.354718 | 0.733784 | 1.012990 | 0.001648 | 0.001654 | 0.001588 |
| move.test | RUN | 0.184474 | 0.355079 | 0.435871 | 0.001587 | 0.001523 | 0.001347 |
| none.test | RUN | 0.295266 | 0.511395 | 0.636361 | 0.001428 | 0.001622 | 0.001556 |
| pop_back.test | RUN | 0.292938 | 0.689998 | 0.764832 | 0.001380 | 0.001378 | 0.001470 |
| pop_front.test | RUN | 0.232923 | 0.573833 | 0.613435 | 0.001374 | 0.001287 | 0.001376 |
| push_back.test | RUN | 0.296078 | 0.788771 | 0.824272 | 0.001814 | 0.001708 | 0.001495 |
| push_front.test | RUN | 0.276442 | 0.676864 | 0.701969 | 0.001427 | 0.001470 | 0.001423 |
| remove.test | RUN | 0.291011 | 0.747118 | 0.837701 | 0.001474 | 0.001598 | 0.001436 |
| remove_if.test | RUN | 0.262449 | 0.613406 | 0.713504 | 0.001372 | 0.001473 | 0.001521 |
| replace.test | RUN | 0.245598 | 0.614265 | 0.660610 | 0.001608 | 0.001648 | 0.001602 |
| replace_if.test | RUN | 0.242321 | 0.615670 | 0.652814 | 0.001588 | 0.001666 | 0.001430 |
| reverse_fold.test | RUN | 0.365638 | 0.824652 | 1.117302 | 0.001609 | 0.001571 | 0.001501 |
| reverse_iter_fold.test | RUN | 0.378150 | 0.862677 | 1.170167 | 0.001396 | 0.001560 | 0.001535 |
| reverse.test | RUN | 0.236480 | 0.621726 | 0.677303 | 0.001557 | 0.001708 | 0.001459 |
| segmented_for_each.test | RUN | 0.389035 | 1.164672 | 1.351358 | 0.001564 | 0.001684 | 0.001542 |
| segmented_find.test | RUN | 0.254768 | 0.603853 | 0.753452 | 0.001302 | 0.001635 | 0.001570 |
| segmented_find_if.test | RUN | 0.250804 | 0.599160 | 0.746984 | 0.001428 | 0.001469 | 0.001566 |
| segmented_fold.test | RUN | 0.318679 | 0.809899 | 0.958046 | 0.001531 | 0.001679 | 0.001501 |
| transform.test | RUN | 0.247739 | 0.654219 | 0.706716 | 0.001353 | 0.001530 | 0.001473 |
| join.test | RUN | 0.194579 | 0.406120 | 0.491406 | 0.001478 | 0.001362 | 0.001455 |
| zip.test | RUN | 0.330555 | 0.725264 | 0.937375 | 0.001575 | 0.001615 | 0.001372 |
| zip2.test | RUN | 0.317230 | 0.719808 | 0.884812 | 0.001455 | 0.001367 | 0.001605 |
| zip_ignore.test | RUN | 0.255140 | 0.489115 | 0.597758 | 0.001189 | 0.001507 | 0.001176 |
| flatten.test | RUN | 0.231162 | 0.542467 | 0.602352 | 0.001415 | 0.001453 | 0.001409 |
| ticket-5490.test | COMPILE | 0.133621 | 0.234546 | 0.265061 | — | — | — |
| as_deque.test | RUN | 0.223503 | 0.509930 | 0.636180 | 0.001495 | 0.001545 | 0.001413 |
| as_list.test | RUN | 0.253837 | 0.641144 | 0.699119 | 0.001610 | 0.001516 | 0.001432 |
| as_map.test | RUN | 0.251620 | 0.623725 | 0.702174 | 0.001064 | 0.001363 | 0.001529 |
| as_map_assoc.test | RUN | 0.251217 | 0.596528 | 0.653433 | 0.001432 | 0.001434 | 0.001444 |
| as_set.test | RUN | 0.278736 | 0.729794 | 0.834239 | 0.001499 | 0.001466 | 0.001479 |
| as_vector.test | RUN | 0.261402 | 0.640062 | 0.761260 | 0.001282 | 0.001375 | 0.001410 |
| boost_tuple.test | RUN | 0.262806 | 0.667934 | 0.744507 | 0.001318 | 0.001489 | 0.001272 |
| boost_tuple_iterator.test | RUN | 0.179674 | 0.375805 | 0.453321 | 0.001484 | 0.001569 | 0.001502 |
| cons.test | RUN | 0.320669 | 0.731114 | 0.839426 | 0.001350 | 0.001366 | 0.001268 |
| convert_boost_tuple.test | RUN | 0.207482 | 0.553619 | 0.559892 | 0.001465 | 0.001398 | 0.001335 |
| convert_deque.test | RUN | 0.209951 | 0.548606 | 0.551707 | 0.001359 | 0.001346 | 0.001429 |
| convert_list.test | RUN | 0.213946 | 0.574506 | 0.560125 | 0.001179 | 0.001493 | 0.001266 |
| convert_std_pair.test | RUN | 0.221613 | 0.572239 | 0.571526 | 0.001438 | 0.001293 | 0.001321 |
| convert_std_tuple.test | RUN | 0.210157 | 0.556284 | 0.554735 | 0.001418 | 0.001588 | 0.001546 |
| convert_vector.test | RUN | 0.211276 | 0.565058 | 0.567273 | 0.001430 | 0.001253 | 0.001317 |
| filter_view.test | RUN | 0.247650 | 0.595428 | 0.654480 | 0.001326 | 0.001498 | 0.001520 |
| hash.test | RUN | 0.249323 | 0.526070 | 0.635424 | 0.001444 | 0.001501 | 0.001536 |
| io.test | RUN | 0.254203 | 0.753441 | 0.757867 | 0.001540 | 0.001851 | 0.001617 |
| iterator_range.test | RUN | 0.232902 | 0.554171 | 0.628933 | 0.001362 | 0.001538 | 0.001275 |
| joint_view.test | RUN | 0.305157 | 0.940624 | 0.945231 | 0.001688 | 0.001451 | 0.001542 |
| list_comparison.test | RUN | 0.253551 | 0.377664 | 0.471059 | 0.001576 | 0.001472 | 0.001551 |
| list_construction.test | RUN | 0.241704 | 0.379914 | 0.452870 | 0.001294 | 0.001467 | 0.001145 |
| list_copy.test | RUN | 0.331880 | 0.682573 | 0.769441 | 0.001402 | 0.001404 | 0.001372 |
| list_iterator.test | RUN | 0.199659 | 0.390278 | 0.417582 | 0.001344 | 0.001310 | 0.001498 |
| list_hash.test | RUN | 0.245582 | 0.718566 | 0.639022 | 0.001434 | 0.001646 | 0.001516 |
| list_make.test | RUN | 0.190104 | 0.412220 | 0.484229 | 0.001303 | 0.001403 | 0.001600 |
| list_misc.test | RUN | 0.245862 | 0.493483 | 0.590441 | 0.001391 | 0.001559 | 0.001464 |
| list_mutate.test | RUN | 0.174602 | 0.335826 | 0.403992 | 0.001395 | 0.001563 | 0.001510 |
| list_nest.test | RUN | 0.341245 | 0.916270 | 1.080071 | 0.001441 | 0.001542 | 0.001448 |
| list_tie.test | RUN | 0.186051 | 0.389299 | 0.427991 | 0.001502 | 0.001509 | 0.001527 |
| list_value_at.test | RUN | 0.185480 | 0.342975 | 0.437127 | 0.001399 | 0.001565 | 0.001582 |
| deque_comparison.test | RUN | 0.196640 | 0.392879 | 0.453222 | 0.001273 | 0.001421 | 0.001549 |
| deque_construction.test | RUN | 0.182214 | 0.378045 | 0.457696 | 0.001571 | 0.001610 | 0.001544 |
| deque_copy.test | RUN | 0.252207 | 0.631088 | 0.747717 | 0.001373 | 0.001454 | 0.001504 |
| deque_iterator.test | RUN | 0.183779 | 0.401914 | 0.434849 | 0.001372 | 0.001422 | 0.001253 |
| deque_hash.test | RUN | 0.253804 | 0.699497 | 0.660010 | 0.001360 | 0.001670 | 0.001526 |
| deque_is_constructible.test | COMPILE | 0.103775 | 0.179130 | 0.205023 | — | — | — |
| deque_make.test | RUN | 0.198312 | 0.402384 | 0.472158 | 0.001449 | 0.001454 | 0.001436 |
| deque_misc.test | RUN | 0.250166 | 0.487045 | 0.578487 | 0.001166 | 0.001486 | 0.001536 |
| deque_move.test | RUN | 0.200344 | 0.423146 | 0.459081 | 0.001150 | 0.001375 | 0.001259 |
| deque_mutate.test | RUN | 0.180679 | 0.335575 | 0.389404 | 0.001441 | 0.001521 | 0.001480 |
| deque_nest.test | RUN | 0.487756 | 1.486682 | 1.766451 | 0.001576 | 0.001522 | 0.001494 |
| deque_tie.test | RUN | 0.178646 | 0.355407 | 0.418111 | 0.001569 | 0.001368 | 0.001274 |
| deque_value_at.test | RUN | 0.173195 | 0.337411 | 0.401556 | 0.001242 | 0.001482 | 0.001416 |
| front_extended_deque.test | RUN | 0.248376 | 0.497252 | 0.639601 | 0.001225 | 0.001589 | 0.001673 |
| back_extended_deque.test | RUN | 0.249748 | 0.492263 | 0.641241 | 0.001400 | 0.001541 | 0.001557 |
| make_list.test | RUN | 0.190567 | 0.423603 | 0.492921 | 0.001434 | 0.001636 | 0.001500 |
| make_vector.test | RUN | 0.191365 | 0.401127 | 0.459963 | 0.001262 | 0.001463 | 0.001402 |
| map.test | RUN | 0.243479 | 0.624724 | 0.709186 | 0.001698 | 0.001484 | 0.001447 |
| map_comparison.test | RUN | 0.184535 | 0.403472 | 0.455449 | 0.001468 | 0.001586 | 0.001432 |
| map_construction.test | RUN | 0.191797 | 0.390345 | 0.462724 | 0.001345 | 0.001628 | 0.001509 |
| map_copy.test | RUN | 0.227196 | 0.483963 | 0.565253 | 0.001342 | 0.001624 | 0.001524 |
| map_misc.test | RUN | 0.236713 | 0.492605 | 0.568032 | 0.001406 | 0.001520 | 0.001393 |
| map_move.test | RUN | 0.190590 | 0.417519 | 0.457384 | 0.001307 | 0.001545 | 0.001320 |
| map_mutate.test | RUN | 0.174628 | 0.335661 | 0.404115 | 0.001264 | 0.001192 | 0.001472 |
| map_tie.test | RUN | 0.177858 | 0.339465 | 0.406221 | 0.001361 | 0.001504 | 0.001557 |
| nil.test | RUN | 0.123787 | 0.234359 | 0.284708 | 0.001781 | 0.001636 | 0.001609 |
| nview.test | RUN | 0.239753 | 0.481080 | 0.585349 | 0.001602 | 0.001596 | 0.001567 |
| reverse_view.test | RUN | 0.285614 | 0.799471 | 0.864859 | 0.001428 | 0.001526 | 0.001647 |
| segmented_iterator_range.test | RUN | 0.340026 | 0.943692 | 1.079842 | 0.001478 | 0.001592 | 0.002155 |
| set.test | RUN | 0.226384 | 0.552824 | 0.627170 | 0.001282 | 0.001493 | 0.001583 |
| single_view.test | RUN | 0.206002 | 0.511657 | 0.554722 | 0.001576 | 0.001426 | 0.002400 |
| std_pair.test | RUN | 0.258234 | 0.636718 | 0.770789 | 0.001431 | 0.001600 | 0.001453 |
| boost_array.test | RUN | 0.225361 | 0.454964 | 0.575932 | 0.001275 | 0.001311 | 0.001309 |
| array.test | RUN | 0.220350 | 0.440876 | 0.544715 | 0.001097 | 0.001634 | 0.001282 |
| std_array.test | RUN | 0.224558 | 0.447417 | 0.527070 | 0.001524 | 0.001397 | 0.001503 |
| tuple_comparison.test | RUN | 0.200792 | 0.412893 | 0.554170 | 0.001370 | 0.001469 | 0.001482 |
| tuple_construction.test | RUN | 0.208117 | 0.412347 | 0.573803 | 0.001328 | 0.001660 | 0.001420 |
| tuple_conversion.test | RUN | 0.431365 | 1.267055 | 1.755940 | 0.001214 | 0.001680 | 0.001241 |
| tuple_copy.test | RUN | 0.284723 | 0.736970 | 0.990371 | 0.001576 | 0.001324 | 0.001340 |
| tuple_element.test | RUN | 0.185890 | 0.387770 | 0.471083 | 0.001421 | 0.001466 | 0.001418 |
| tuple_make.test | RUN | 0.207992 | 0.442853 | 0.500983 | 0.001332 | 0.001607 | 0.001322 |
| tuple_misc.test | RUN | 0.247119 | 0.528203 | 0.614754 | 0.001427 | 0.001574 | 0.001446 |
| tuple_mutate.test | RUN | 0.182214 | 0.350943 | 0.470471 | 0.001577 | 0.001437 | 0.001478 |
| tuple_nest.test | RUN | 0.719139 | 2.650726 | 3.106871 | 0.001550 | 0.001657 | 0.001536 |
| tuple_hash.test | RUN | 0.262452 | 0.755299 | 0.664702 | 0.001491 | 0.001487 | 0.001467 |
| tuple_tie.test | RUN | 0.193482 | 0.384500 | 0.468592 | 0.001485 | 0.001623 | 0.001410 |
| tuple_traits__maybe_variadic.test | RUN | 0.200582 | 0.467276 | 0.492534 | 0.001403 | 0.001503 | 0.001264 |
| tuple_traits__no_variadic.test | RUN | 0.231054 | 0.491847 | 0.572967 | 0.001399 | 0.001642 | 0.001518 |
| transform_view.test | RUN | 0.299308 | 0.807559 | 0.917877 | 0.001580 | 0.001399 | 0.001672 |
| identity_view.test | RUN | 0.280126 | 0.751712 | 0.872560 | 0.001340 | 0.001193 | 0.001503 |
| vector_comparison.test | RUN | 0.192640 | 0.403479 | 0.504247 | 0.001429 | 0.001314 | 0.001531 |
| vector_construction.test | RUN | 0.190342 | 0.376450 | 0.499139 | 0.001198 | 0.001544 | 0.001343 |
| vector_conversion.test | RUN | 0.510645 | 1.667945 | 2.069986 | 0.001557 | 0.001528 | 0.001459 |
| vector_copy.test | RUN | 0.269216 | 0.723774 | 0.829501 | 0.001063 | 0.001672 | 0.001501 |
| vector_iterator.test | RUN | 0.179701 | 0.399210 | 0.485105 | 0.001486 | 0.001423 | 0.001451 |
| vector_make.test | RUN | 0.193032 | 0.411935 | 0.494806 | 0.001402 | 0.001508 | 0.001492 |
| vector_misc.test | RUN | 0.242539 | 0.490522 | 0.612989 | 0.001216 | 0.001638 | 0.001364 |
| vector_move.test | RUN | 0.188681 | 0.436808 | 0.465707 | 0.001359 | 0.001406 | 0.001553 |
| vector_mutate.test | RUN | 0.167696 | 0.333619 | 0.405852 | 0.001330 | 0.001618 | 0.001244 |
| vector_n.test | RUN | 0.201942 | 0.418068 | 0.538518 | 0.001695 | 0.001707 | 0.001556 |
| vector_nest.test | RUN | 0.736301 | 2.776053 | 3.027493 | 0.001520 | 0.001448 | 0.003267 |
| vector_hash.test | RUN | 0.246466 | 0.726725 | 0.592781 | 0.001411 | 0.001587 | 0.001476 |
| vector_tie.test | RUN | 0.181188 | 0.376096 | 0.442176 | 0.001570 | 0.001600 | 0.001508 |
| vector_traits__maybe_variadic.test | RUN | 0.193096 | 0.423830 | 0.460483 | 0.001604 | 0.001555 | 0.001415 |
| vector_traits__no_variadic.test | RUN | 0.206251 | 0.459491 | 0.476674 | 0.001441 | 0.001516 | 0.001740 |
| vector_value_at.test | RUN | 0.174477 | 0.338571 | 0.432708 | 0.001269 | 0.001641 | 0.001395 |
| zip_view.test | RUN | 0.341111 | 0.797008 | 1.049444 | 0.001441 | 0.001496 | 0.001580 |
| zip_view2.test | RUN | 0.275092 | 0.654968 | 0.808965 | 0.001518 | 0.001541 | 0.001404 |
| zip_view_ignore.test | RUN | 0.273052 | 0.586170 | 0.715177 | 0.001251 | 0.001576 | 0.001611 |
| repetitive_view.test | RUN | 0.433190 | 1.042371 | 1.447325 | 0.001557 | 0.001621 | 0.001384 |
| deduce_sequence.test | RUN | 0.217392 | 0.454327 | 0.535491 | 0.001301 | 0.001669 | 0.003105 |
| adapt_adt_named.test | RUN | 0.265481 | 0.659583 | 0.740781 | 0.001358 | 0.001550 | 0.001458 |
| adapt_adt_named_empty.test | RUN | 0.232555 | 0.529840 | 0.613276 | 0.001378 | 0.001604 | 0.001350 |
| adapt_adt.test | RUN | 0.285999 | 0.842801 | 0.848442 | 0.001303 | 0.001560 | 0.001530 |
| adapt_adt_empty.test | RUN | 0.228103 | 0.527317 | 0.611317 | 0.001444 | 0.001614 | 0.001332 |
| adapt_assoc_adt_named.test | RUN | 0.295321 | 0.672915 | 0.769704 | 0.001376 | 0.001611 | 0.001563 |
| adapt_assoc_adt_named_empty.test | RUN | 0.241931 | 0.538532 | 0.593101 | 0.001418 | 0.001396 | 0.001610 |
| adapt_assoc_adt.test | RUN | 0.289432 | 0.693928 | 0.806014 | 0.001433 | 0.001579 | 0.001521 |
| adapt_assoc_adt_empty.test | RUN | 0.226890 | 0.533551 | 0.595348 | 0.001476 | 0.001334 | 0.001592 |
| adapt_assoc_struct_named.test | RUN | 0.281858 | 0.666952 | 0.774704 | 0.001328 | 0.001459 | 0.001526 |
| adapt_assoc_struct_named_empty.test | RUN | 0.229264 | 0.522037 | 0.606358 | 0.001549 | 0.001421 | 0.001330 |
| adapt_assoc_struct.test | RUN | 0.250673 | 0.620826 | 0.730233 | 0.001767 | 0.001454 | 0.001573 |
| adapt_assoc_struct_empty.test | RUN | 0.222183 | 0.522942 | 0.612669 | 0.001437 | 0.001556 | 0.001298 |
| adapt_assoc_tpl_adt.test | RUN | 0.303614 | 0.727162 | 0.822627 | 0.001395 | 0.001553 | 0.001651 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.233858 | 0.532038 | 0.600210 | 0.001536 | 0.001461 | 0.001559 |
| adapt_assoc_tpl_struct.test | RUN | 0.244645 | 0.612314 | 0.697031 | 0.003069 | 0.001385 | 0.001331 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.225675 | 0.534488 | 0.591119 | 0.003079 | 0.001552 | 0.001320 |
| adapt_struct_named.test | RUN | 0.344870 | 0.631837 | 0.722703 | 0.001588 | 0.001492 | 0.001526 |
| adapt_struct_named_empty.test | RUN | 0.329438 | 0.532412 | 0.601166 | 0.001339 | 0.001319 | 0.001553 |
| adapt_struct.test | RUN | 0.279391 | 0.712175 | 0.802892 | 0.001331 | 0.001614 | 0.001486 |
| adapt_struct_empty.test | RUN | 0.231617 | 0.524692 | 0.594485 | 0.001572 | 0.001606 | 0.001508 |
| adapt_tpl_adt.test | RUN | 0.282068 | 0.828860 | 0.823939 | 0.001308 | 0.001431 | 0.001537 |
| adapt_tpl_adt_empty.test | RUN | 0.232502 | 0.528294 | 0.574989 | 0.001536 | 0.001602 | 0.001585 |
| adapt_tpl_struct.test | RUN | 0.237884 | 0.571541 | 0.651793 | 0.001547 | 0.001452 | 0.001300 |
| adapt_tpl_struct_empty.test | RUN | 0.229140 | 0.513142 | 0.573189 | 0.001541 | 0.001496 | 0.001591 |
| adt_attribute_proxy.test | RUN | 0.263112 | 0.514174 | 0.620899 | 0.001410 | 0.001621 | 0.001482 |
| define_struct.test | RUN | 0.306578 | 0.707200 | 0.853776 | 0.001445 | 0.001343 | 0.001579 |
| define_struct_empty.test | RUN | 0.275466 | 0.577546 | 0.689260 | 0.001683 | 0.001496 | 0.001526 |
| define_struct_move.test | RUN | 0.181540 | 0.323101 | 0.403690 | 0.001427 | 0.001516 | 0.001292 |
| define_struct_inline.test | RUN | 0.321910 | 0.840324 | 0.934521 | 0.001533 | 0.001394 | 0.001699 |
| define_struct_inline_empty.test | RUN | 0.272663 | 0.574666 | 0.699768 | 0.001386 | 0.001638 | 0.001529 |
| define_struct_inline_move.test | RUN | 0.191732 | 0.342666 | 0.413917 | 0.001313 | 0.001498 | 0.001477 |
| define_assoc_struct.test | RUN | 0.301516 | 0.689224 | 0.803811 | 0.001640 | 0.001636 | 0.001568 |
| define_assoc_struct_empty.test | RUN | 0.272317 | 0.589043 | 0.706370 | 0.001457 | 0.001604 | 0.001540 |
| define_assoc_struct_move.test | RUN | 0.181243 | 0.333236 | 0.409835 | 0.001532 | 0.001552 | 0.001584 |
| define_tpl_struct.test | RUN | 0.299068 | 0.688342 | 0.803569 | 0.001570 | 0.001556 | 0.001520 |
| define_tpl_struct_empty.test | RUN | 0.273744 | 0.583847 | 0.706920 | 0.001532 | 0.001345 | 0.001511 |
| define_tpl_struct_move.test | RUN | 0.183283 | 0.346328 | 0.421653 | 0.001382 | 0.001523 | 0.001208 |
| define_tpl_struct_inline.test | RUN | 0.305588 | 0.780387 | 0.884157 | 0.001489 | 0.001485 | 0.001494 |
| define_tpl_struct_inline_empty.test | RUN | 0.276454 | 0.603145 | 0.684925 | 0.001383 | 0.001431 | 0.001562 |
| define_tpl_struct_inline_move.test | RUN | 0.187114 | 0.326354 | 0.411751 | 0.001404 | 0.001606 | 0.001466 |
| define_assoc_tpl_struct.test | RUN | 0.305917 | 0.695192 | 0.836481 | 0.001926 | 0.001657 | 0.001576 |
| define_assoc_tpl_struct_empty.test | RUN | 0.279743 | 0.583903 | 0.700633 | 0.001156 | 0.001425 | 0.001487 |
| define_assoc_tpl_struct_move.test | RUN | 0.187770 | 0.334445 | 0.419058 | 0.001415 | 0.001570 | 0.001540 |
| std_tuple.test | RUN | 0.186685 | 0.368709 | 0.446660 | 0.001264 | 0.001588 | 0.001070 |
| std_tuple_iterator.test | RUN | 0.183839 | 0.392278 | 0.447683 | 0.001784 | 0.001664 | 0.001450 |
| ref_vector.test | RUN | 0.209377 | 0.400999 | 0.488739 | 0.001450 | 0.001644 | 0.001593 |
| flatten_view.test | RUN | 0.217649 | 0.488294 | 0.570149 | 0.001309 | 0.001535 | 0.001470 |
| github-159.test | COMPILE | 0.104220 | 0.178848 | 0.217898 | — | — | — |
| github-176.test | RUN | 0.205272 | 0.434956 | 0.496868 | 0.001497 | 0.001487 | 0.001506 |
| size.test | COMPILE | 0.173490 | 0.313812 | 0.396316 | — | — | — |
| fused.test | RUN | 0.251128 | 0.462612 | 0.553337 | 0.001479 | 0.001565 | 0.001526 |
| fused_function_object.test | RUN | 0.183699 | 0.346091 | 0.442127 | 0.001605 | 0.001429 | 0.001195 |
| fused_procedure.test | RUN | 0.240968 | 0.455672 | 0.526995 | 0.001301 | 0.001557 | 0.001358 |
| unfused.test | RUN | 0.190614 | 0.365163 | 0.434167 | 0.001298 | 0.001491 | 0.001497 |
| unfused_typed.test | RUN | 0.189800 | 0.391714 | 0.449765 | 0.001419 | 0.001335 | 0.001463 |
| make_fused.test | RUN | 0.243834 | 0.456568 | 0.549975 | 0.001425 | 0.001273 | 0.001272 |
| make_fused_function_object.test | RUN | 0.186331 | 0.362122 | 0.429719 | 0.001364 | 0.001525 | 0.001306 |
| make_fused_procedure.test | RUN | 0.239692 | 0.453508 | 0.525229 | 0.001494 | 0.001533 | 0.001488 |
| make_unfused.test | RUN | 0.185555 | 0.357776 | 0.431660 | 0.001513 | 0.001622 | 0.001469 |
| invoke.test | RUN | 0.452909 | 1.251670 | 1.456103 | 0.001518 | 0.001461 | 0.001373 |
| invoke_function_object.test | RUN | 0.230792 | 0.463960 | 0.562152 | 0.001620 | 0.001552 | 0.001540 |
| invoke_procedure.test | RUN | 0.358728 | 0.944332 | 1.083855 | 0.001317 | 0.001463 | 0.001442 |
| swap.test | RUN | 0.279094 | 0.563702 | 0.705809 | 0.001324 | 0.001575 | 0.001527 |
| is_sequence.test | COMPILE | 0.055331 | 0.086152 | 0.093539 | — | — | — |
| is_view.test | COMPILE | 0.052527 | 0.087535 | 0.094342 | — | — | — |
| pair_deque.test | COMPILE | 0.104921 | 0.187325 | 0.232847 | — | — | — |
| pair_list.test | COMPILE | 0.103834 | 0.187683 | 0.207204 | — | — | — |
| pair_map.test | COMPILE | 0.105483 | 0.197266 | 0.221287 | — | — | — |
| pair_set.test | COMPILE | 0.112376 | 0.191864 | 0.214553 | — | — | — |
| pair_vector.test | COMPILE | 0.106949 | 0.189830 | 0.214471 | — | — | — |
| pair_nest.test | COMPILE | 0.043542 | 0.077902 | 0.084358 | — | — | — |
| index_sequence.test | COMPILE | 0.041783 | 0.057888 | 0.067223 | — | — | — |
| and.test | COMPILE | 0.033517 | 0.044258 | 0.051238 | — | — | — |
| tag_of.test | COMPILE | 0.055753 | 0.082740 | 0.090887 | — | — | — |
| unused.test | COMPILE | 0.028062 | 0.053551 | 0.054978 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.053562 | 0.060165 | 0.082287 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.036840 | 0.076580 | 0.069074 | — | — | — |
| quick.test | RUN | 0.138629 | 0.278593 | 0.267928 | 0.000401 | 0.001088 | 0.001069 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.986× | 0.994×, 0.978× | 0.960× |
| clang | 0.911× | 0.891×, 0.932× | 0.981× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000974s | 0.000980s | 0.001023s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000962s | 0.000999s | 0.001007s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000954s | 0.000981s | 0.001027s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000955s | 0.000965s | 0.001010s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000964s | 0.000988s | 0.001031s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000957s | 0.000946s | 0.001036s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000979s | 0.000953s | 0.001059s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000970s | 0.000951s | 0.001011s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000979s | 0.000957s | 0.001015s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000948s | 0.000969s | 0.000988s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000957s | 0.000978s | 0.001016s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000937s | 0.000950s | 0.001030s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000956s | 0.000964s | 0.001047s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000939s | 0.000963s | 0.001020s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000974s | 0.000987s | 0.001045s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000950s | 0.000948s | 0.001057s | byte equality observed |
| adapt_struct.test | 10 | 0.001012s | 0.000976s | 0.001032s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000989s | 0.000956s | 0.001023s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000995s | 0.000970s | 0.001054s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000979s | 0.000957s | 0.001027s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000982s | 0.000979s | 0.001031s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001001s | 0.000949s | 0.001023s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000993s | 0.000975s | 0.001046s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000953s | 0.000966s | 0.001023s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000964s | 0.000959s | 0.001017s | byte equality observed |
| all.test | 10 | 0.000946s | 0.000969s | 0.001007s | byte equality observed |
| any.test | 10 | 0.000960s | 0.000941s | 0.000998s | byte equality observed |
| array.test | 10 | 0.000953s | 0.000949s | 0.001005s | byte equality observed |
| as_deque.test | 10 | 0.000950s | 0.000946s | 0.001002s | byte equality observed |
| as_list.test | 10 | 0.000992s | 0.000987s | 0.001051s | byte equality observed |
| as_map.test | 10 | 0.000990s | 0.000977s | 0.001035s | byte equality observed |
| as_map_assoc.test | 10 | 0.000963s | 0.000975s | 0.001040s | byte equality observed |
| as_set.test | 10 | 0.001011s | 0.000963s | 0.001071s | byte equality observed |
| as_vector.test | 10 | 0.001004s | 0.000977s | 0.001026s | byte equality observed |
| back_extended_deque.test | 10 | 0.000932s | 0.000918s | 0.000994s | byte equality observed |
| boost_array.test | 10 | 0.000961s | 0.000940s | 0.000997s | byte equality observed |
| boost_tuple.test | 10 | 0.000978s | 0.000957s | 0.001031s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000968s | 0.000949s | 0.000989s | byte equality observed |
| clear.test | 10 | 0.000961s | 0.000973s | 0.001027s | byte equality observed |
| cons.test | 10 | 0.001010s | 0.000978s | 0.001047s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000958s | 0.000957s | 0.001026s | byte equality observed |
| convert_deque.test | 10 | 0.000967s | 0.000940s | 0.001019s | byte equality observed |
| convert_list.test | 10 | 0.000944s | 0.000950s | 0.000998s | byte equality observed |
| convert_std_pair.test | 10 | 0.000963s | 0.000947s | 0.001021s | byte equality observed |
| convert_std_tuple.test | 8 | 0.000968s | 0.000971s | 0.001014s | byte equality observed |
| convert_vector.test | 10 | 0.000960s | 0.000953s | 0.001021s | byte equality observed |
| copy.test | 10 | 0.000941s | 0.000964s | 0.001012s | byte equality observed |
| count.test | 10 | 0.000961s | 0.000920s | 0.001015s | byte equality observed |
| count_if.test | 10 | 0.000964s | 0.000892s | 0.000985s | byte equality observed |
| deduce_sequence.test | 10 | 0.000960s | 0.000924s | 0.001017s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000990s | 0.000933s | 0.001038s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000984s | 0.000939s | 0.001018s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000959s | 0.000918s | 0.001016s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001001s | 0.000926s | 0.001019s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000975s | 0.000920s | 0.001067s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000987s | 0.000909s | 0.000999s | byte equality observed |
| define_struct.test | 10 | 0.000993s | 0.000945s | 0.001009s | byte equality observed |
| define_struct_empty.test | 10 | 0.000989s | 0.000935s | 0.001021s | byte equality observed |
| define_struct_inline.test | 10 | 0.000994s | 0.000950s | 0.001023s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000965s | 0.000923s | 0.001012s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000952s | 0.000905s | 0.000994s | byte equality observed |
| define_struct_move.test | 10 | 0.000958s | 0.000950s | 0.000996s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000990s | 0.000933s | 0.001031s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000972s | 0.000938s | 0.001018s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001004s | 0.000932s | 0.001049s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000978s | 0.000942s | 0.001032s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000953s | 0.000901s | 0.001017s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001000s | 0.000906s | 0.001019s | byte equality observed |
| deque_comparison.test | 10 | 0.000951s | 0.000910s | 0.001050s | byte equality observed |
| deque_construction.test | 10 | 0.001000s | 0.000922s | 0.001019s | byte equality observed |
| deque_copy.test | 10 | 0.000984s | 0.000948s | 0.001004s | byte equality observed |
| deque_hash.test | 10 | 0.001008s | 0.000912s | 0.001034s | byte equality observed |
| deque_iterator.test | 10 | 0.001000s | 0.000908s | 0.001015s | byte equality observed |
| deque_make.test | 10 | 0.000976s | 0.000908s | 0.001027s | byte equality observed |
| deque_misc.test | 10 | 0.000972s | 0.000903s | 0.001008s | byte equality observed |
| deque_move.test | 10 | 0.000983s | 0.000922s | 0.001043s | byte equality observed |
| deque_mutate.test | 10 | 0.000957s | 0.000916s | 0.001008s | byte equality observed |
| deque_nest.test | 10 | 0.000947s | 0.000908s | 0.000999s | byte equality observed |
| deque_tie.test | 10 | 0.000973s | 0.000912s | 0.000999s | byte equality observed |
| deque_value_at.test | 10 | 0.000997s | 0.000921s | 0.000994s | byte equality observed |
| erase.test | 10 | 0.001010s | 0.000932s | 0.001037s | byte equality observed |
| erase_key.test | 10 | 0.001023s | 0.000934s | 0.001043s | byte equality observed |
| filter.test | 10 | 0.000995s | 0.000915s | 0.001028s | byte equality observed |
| filter_if.test | 10 | 0.001003s | 0.000966s | 0.001077s | byte equality observed |
| filter_view.test | 10 | 0.001023s | 0.000955s | 0.001043s | byte equality observed |
| find.test | 10 | 0.001014s | 0.000953s | 0.001053s | byte equality observed |
| find_if.test | 10 | 0.001012s | 0.000950s | 0.001030s | byte equality observed |
| flatten.test | 10 | 0.001022s | 0.000943s | 0.001026s | byte equality observed |
| flatten_view.test | 10 | 0.000972s | 0.000949s | 0.001019s | byte equality observed |
| fold.test | 10 | 0.000979s | 0.000932s | 0.001012s | byte equality observed |
| for_each.test | 10 | 0.000965s | 0.000955s | 0.001052s | byte equality observed |
| front_extended_deque.test | 10 | 0.000962s | 0.000920s | 0.001010s | byte equality observed |
| fused.test | 10 | 0.000943s | 0.000926s | 0.001025s | byte equality observed |
| fused_function_object.test | 10 | 0.000938s | 0.000931s | 0.001004s | byte equality observed |
| fused_procedure.test | 10 | 0.000969s | 0.000932s | 0.001004s | byte equality observed |
| github-176.test | 10 | 0.001010s | 0.000936s | 0.001003s | byte equality observed |
| hash.test | 10 | 0.000987s | 0.000937s | 0.000991s | byte equality observed |
| identity_view.test | 10 | 0.001016s | 0.000962s | 0.001027s | byte equality observed |
| insert.test | 10 | 0.001022s | 0.000969s | 0.001053s | byte equality observed |
| insert_range.test | 10 | 0.001020s | 0.000968s | 0.001034s | byte equality observed |
| invoke.test | 10 | 0.001010s | 0.000914s | 0.001003s | byte equality observed |
| invoke_function_object.test | 10 | 0.000992s | 0.000924s | 0.001031s | byte equality observed |
| invoke_procedure.test | 10 | 0.000993s | 0.000916s | 0.001012s | byte equality observed |
| io.test | 4 | 0.001163s | 0.001066s | 0.001163s | byte equality observed |
| iter_fold.test | 10 | 0.000975s | 0.000936s | 0.001023s | byte equality observed |
| iterator_range.test | 10 | 0.000994s | 0.000960s | 0.001053s | byte equality observed |
| join.test | 10 | 0.000967s | 0.000895s | 0.000995s | byte equality observed |
| joint_view.test | 10 | 0.001039s | 0.000991s | 0.001064s | byte equality observed |
| list_comparison.test | 8 | 0.000961s | 0.000929s | 0.001014s | byte equality observed |
| list_construction.test | 9 | 0.000968s | 0.000920s | 0.001022s | byte equality observed |
| list_copy.test | 10 | 0.000963s | 0.000923s | 0.001019s | byte equality observed |
| list_hash.test | 10 | 0.000949s | 0.000938s | 0.001008s | byte equality observed |
| list_iterator.test | 10 | 0.000959s | 0.000912s | 0.000987s | byte equality observed |
| list_make.test | 10 | 0.000972s | 0.000931s | 0.001008s | byte equality observed |
| list_misc.test | 10 | 0.000954s | 0.000938s | 0.001032s | byte equality observed |
| list_mutate.test | 10 | 0.000972s | 0.000914s | 0.001003s | byte equality observed |
| list_nest.test | 10 | 0.000958s | 0.000922s | 0.001017s | byte equality observed |
| list_tie.test | 10 | 0.000972s | 0.000912s | 0.000999s | byte equality observed |
| list_value_at.test | 10 | 0.000985s | 0.000903s | 0.001000s | byte equality observed |
| make_fused.test | 10 | 0.000999s | 0.000912s | 0.001031s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000963s | 0.000897s | 0.001015s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000983s | 0.000904s | 0.000988s | byte equality observed |
| make_list.test | 10 | 0.000973s | 0.000925s | 0.001033s | byte equality observed |
| make_unfused.test | 10 | 0.000966s | 0.000911s | 0.001006s | byte equality observed |
| make_vector.test | 10 | 0.001021s | 0.000920s | 0.001001s | byte equality observed |
| map.test | 10 | 0.001018s | 0.000936s | 0.001059s | byte equality observed |
| map_comparison.test | 10 | 0.000980s | 0.000916s | 0.001015s | byte equality observed |
| map_construction.test | 10 | 0.000991s | 0.000921s | 0.001009s | byte equality observed |
| map_copy.test | 8 | 0.000982s | 0.000901s | 0.000994s | byte equality observed |
| map_misc.test | 10 | 0.000991s | 0.000907s | 0.001032s | byte equality observed |
| map_move.test | 10 | 0.000994s | 0.000911s | 0.001031s | byte equality observed |
| map_mutate.test | 10 | 0.000986s | 0.000911s | 0.001034s | byte equality observed |
| map_tie.test | 10 | 0.000974s | 0.000893s | 0.000988s | byte equality observed |
| move.test | 10 | 0.000978s | 0.000889s | 0.001016s | byte equality observed |
| nil.test | 10 | 0.000964s | 0.000910s | 0.001053s | byte equality observed |
| none.test | 10 | 0.000994s | 0.000896s | 0.001028s | byte equality observed |
| nview.test | 10 | 0.000981s | 0.000923s | 0.001013s | byte equality observed |
| pop_back.test | 10 | 0.001032s | 0.000943s | 0.001078s | byte equality observed |
| pop_front.test | 10 | 0.001002s | 0.000946s | 0.001057s | byte equality observed |
| push_back.test | 9 | 0.000996s | 0.000940s | 0.001054s | byte equality observed |
| push_front.test | 10 | 0.001002s | 0.000934s | 0.001058s | byte equality observed |
| quick.test | 10 | 0.000451s | 0.000872s | 0.000989s | byte equality observed |
| ref_vector.test | 10 | 0.000985s | 0.000933s | 0.001003s | byte equality observed |
| remove.test | 10 | 0.001023s | 0.000970s | 0.001074s | byte equality observed |
| remove_if.test | 10 | 0.001010s | 0.000962s | 0.001045s | byte equality observed |
| repetitive_view.test | 10 | 0.000987s | 0.000925s | 0.000998s | byte equality observed |
| replace.test | 10 | 0.001018s | 0.000918s | 0.001031s | byte equality observed |
| replace_if.test | 10 | 0.001013s | 0.000956s | 0.001036s | byte equality observed |
| reverse.test | 10 | 0.001010s | 0.000934s | 0.001034s | byte equality observed |
| reverse_fold.test | 10 | 0.000984s | 0.000932s | 0.001017s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000992s | 0.000934s | 0.001023s | byte equality observed |
| reverse_view.test | 10 | 0.001010s | 0.000936s | 0.001050s | byte equality observed |
| segmented_find.test | 10 | 0.000973s | 0.000909s | 0.000989s | byte equality observed |
| segmented_find_if.test | 10 | 0.000972s | 0.000928s | 0.001028s | byte equality observed |
| segmented_fold.test | 10 | 0.000992s | 0.000938s | 0.001089s | byte equality observed |
| segmented_for_each.test | 10 | 0.001003s | 0.000940s | 0.001026s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001008s | 0.000958s | 0.001085s | byte equality observed |
| set.test | 10 | 0.000982s | 0.000972s | 0.001047s | byte equality observed |
| single_view.test | 10 | 0.000982s | 0.000960s | 0.001023s | byte equality observed |
| std_array.test | 10 | 0.000961s | 0.000909s | 0.001009s | byte equality observed |
| std_pair.test | 10 | 0.001002s | 0.000947s | 0.001048s | byte equality observed |
| std_tuple.test | 10 | 0.000988s | 0.000911s | 0.001027s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000947s | 0.000905s | 0.001002s | byte equality observed |
| swap.test | 10 | 0.000977s | 0.000925s | 0.001011s | byte equality observed |
| transform.test | 10 | 0.000998s | 0.000925s | 0.001029s | byte equality observed |
| transform_view.test | 10 | 0.001022s | 0.000981s | 0.001058s | byte equality observed |
| tuple_comparison.test | 10 | 0.000985s | 0.000928s | 0.000987s | byte equality observed |
| tuple_construction.test | 10 | 0.001002s | 0.000903s | 0.001013s | byte equality observed |
| tuple_conversion.test | 10 | 0.000969s | 0.000895s | 0.001020s | byte equality observed |
| tuple_copy.test | 10 | 0.000996s | 0.000927s | 0.001023s | byte equality observed |
| tuple_element.test | 10 | 0.000991s | 0.000897s | 0.001005s | byte equality observed |
| tuple_hash.test | 10 | 0.000967s | 0.000909s | 0.001011s | byte equality observed |
| tuple_make.test | 10 | 0.000988s | 0.000926s | 0.000998s | byte equality observed |
| tuple_misc.test | 10 | 0.000968s | 0.000916s | 0.001021s | byte equality observed |
| tuple_mutate.test | 10 | 0.000973s | 0.000912s | 0.001007s | byte equality observed |
| tuple_nest.test | 10 | 0.001011s | 0.000927s | 0.001008s | byte equality observed |
| tuple_tie.test | 10 | 0.000976s | 0.000902s | 0.001010s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001000s | 0.000931s | 0.000994s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000987s | 0.000941s | 0.001008s | byte equality observed |
| unfused.test | 10 | 0.000983s | 0.000899s | 0.001001s | byte equality observed |
| unfused_typed.test | 10 | 0.000981s | 0.000902s | 0.001036s | byte equality observed |
| vector_comparison.test | 10 | 0.001038s | 0.000917s | 0.001007s | byte equality observed |
| vector_construction.test | 10 | 0.000991s | 0.000923s | 0.001009s | byte equality observed |
| vector_conversion.test | 10 | 0.001008s | 0.000903s | 0.001013s | byte equality observed |
| vector_copy.test | 10 | 0.000982s | 0.000946s | 0.001016s | byte equality observed |
| vector_hash.test | 10 | 0.000982s | 0.000927s | 0.001025s | byte equality observed |
| vector_iterator.test | 10 | 0.000994s | 0.000916s | 0.001023s | byte equality observed |
| vector_make.test | 10 | 0.000997s | 0.000934s | 0.001021s | byte equality observed |
| vector_misc.test | 10 | 0.000978s | 0.000915s | 0.001022s | byte equality observed |
| vector_move.test | 10 | 0.001001s | 0.000919s | 0.001027s | byte equality observed |
| vector_mutate.test | 10 | 0.000957s | 0.000915s | 0.001023s | byte equality observed |
| vector_n.test | 10 | 0.000999s | 0.000941s | 0.001031s | byte equality observed |
| vector_nest.test | 10 | 0.000979s | 0.000924s | 0.001039s | byte equality observed |
| vector_tie.test | 10 | 0.000971s | 0.000910s | 0.000988s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000993s | 0.000911s | 0.000992s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000986s | 0.000927s | 0.001024s | byte equality observed |
| vector_value_at.test | 10 | 0.000971s | 0.000919s | 0.001023s | byte equality observed |
| zip.test | 10 | 0.000978s | 0.000930s | 0.000996s | byte equality observed |
| zip2.test | 10 | 0.000990s | 0.000913s | 0.000990s | byte equality observed |
| zip_ignore.test | 10 | 0.000957s | 0.000951s | 0.001047s | byte equality observed |
| zip_view.test | 10 | 0.000981s | 0.000901s | 0.001010s | byte equality observed |
| zip_view2.test | 10 | 0.000953s | 0.000922s | 0.001000s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000971s | 0.000938s | 0.001009s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
