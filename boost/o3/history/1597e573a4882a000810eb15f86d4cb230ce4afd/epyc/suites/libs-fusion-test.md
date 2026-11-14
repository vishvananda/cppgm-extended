# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.299331 | 0.569363 | 0.645200 | 0.001414 | 0.001492 | 0.001280 |
| any.test | RUN | 0.282086 | 0.531961 | 0.628148 | 0.001474 | 0.001400 | 0.001348 |
| clear.test | RUN | 0.203238 | 0.497241 | 0.524911 | 0.001464 | 0.001435 | 0.001342 |
| copy.test | RUN | 0.173559 | 0.370013 | 0.417732 | 0.001547 | 0.001700 | 0.001441 |
| count.test | RUN | 0.190931 | 0.391100 | 0.464518 | 0.001855 | 0.001322 | 0.001556 |
| count_if.test | RUN | 0.184100 | 0.360868 | 0.432447 | 0.001450 | 0.001464 | 0.001400 |
| erase.test | RUN | 0.257775 | 0.662603 | 0.697577 | 0.001691 | 0.001636 | 0.001300 |
| erase_key.test | RUN | 0.264487 | 0.654632 | 0.665965 | 0.001565 | 0.001427 | 0.001399 |
| filter.test | RUN | 0.236445 | 0.546589 | 0.603146 | 0.002112 | 0.001719 | 0.001479 |
| filter_if.test | RUN | 0.264092 | 0.650852 | 0.710721 | 0.001584 | 0.001531 | 0.001587 |
| find.test | RUN | 0.229469 | 0.439739 | 0.521907 | 0.001677 | 0.001702 | 0.001243 |
| find_if.test | RUN | 0.211795 | 0.404067 | 0.505763 | 0.001533 | 0.001342 | 0.001434 |
| fold.test | RUN | 0.238126 | 0.498523 | 0.598253 | 0.001677 | 0.001648 | 0.001489 |
| for_each.test | RUN | 0.207565 | 0.450833 | 0.531507 | 0.001479 | 0.001757 | 0.001434 |
| insert.test | RUN | 0.303491 | 0.800459 | 0.846433 | 0.001752 | 0.001320 | 0.001612 |
| insert_range.test | RUN | 0.312352 | 0.812304 | 0.857837 | 0.001789 | 0.001570 | 0.001326 |
| iter_fold.test | RUN | 0.367367 | 0.775299 | 0.997218 | 0.001750 | 0.001714 | 0.001479 |
| move.test | RUN | 0.186964 | 0.370746 | 0.424873 | 0.001718 | 0.001565 | 0.001495 |
| none.test | RUN | 0.300367 | 0.529297 | 0.657846 | 0.001637 | 0.001623 | 0.001398 |
| pop_back.test | RUN | 0.289836 | 0.702658 | 0.764681 | 0.001511 | 0.001557 | 0.001301 |
| pop_front.test | RUN | 0.241823 | 0.571899 | 0.606812 | 0.001831 | 0.001696 | 0.001467 |
| push_back.test | RUN | 0.300644 | 0.846306 | 0.853222 | 0.001892 | 0.001690 | 0.001331 |
| push_front.test | RUN | 0.272166 | 0.725859 | 0.709590 | 0.001870 | 0.001689 | 0.001308 |
| remove.test | RUN | 0.302916 | 0.787914 | 0.834441 | 0.001356 | 0.001639 | 0.001419 |
| remove_if.test | RUN | 0.278675 | 0.676522 | 0.704025 | 0.001655 | 0.001792 | 0.001364 |
| replace.test | RUN | 0.261976 | 0.632557 | 0.665103 | 0.001842 | 0.001730 | 0.001438 |
| replace_if.test | RUN | 0.257935 | 0.623939 | 0.660740 | 0.001850 | 0.001746 | 0.001435 |
| reverse_fold.test | RUN | 0.415555 | 0.887482 | 1.109513 | 0.001454 | 0.001726 | 0.001494 |
| reverse_iter_fold.test | RUN | 0.411581 | 0.924415 | 1.140919 | 0.001707 | 0.001772 | 0.001281 |
| reverse.test | RUN | 0.253476 | 0.662546 | 0.671559 | 0.001771 | 0.001781 | 0.001318 |
| segmented_for_each.test | RUN | 0.439871 | 1.225742 | 1.353885 | 0.001725 | 0.001653 | 0.001243 |
| segmented_find.test | RUN | 0.266176 | 0.634780 | 0.758864 | 0.001530 | 0.001198 | 0.001349 |
| segmented_find_if.test | RUN | 0.271283 | 0.640834 | 0.735530 | 0.001677 | 0.001551 | 0.001539 |
| segmented_fold.test | RUN | 0.346659 | 0.852448 | 0.961194 | 0.001870 | 0.001756 | 0.001471 |
| transform.test | RUN | 0.265909 | 0.691984 | 0.729750 | 0.001656 | 0.001597 | 0.001444 |
| join.test | RUN | 0.201177 | 0.422073 | 0.481537 | 0.001774 | 0.001581 | 0.001485 |
| zip.test | RUN | 0.360427 | 0.789437 | 0.927095 | 0.001727 | 0.001712 | 0.001457 |
| zip2.test | RUN | 0.353363 | 0.720230 | 0.899828 | 0.001659 | 0.001566 | 0.001242 |
| zip_ignore.test | RUN | 0.261388 | 0.527910 | 0.591518 | 0.001608 | 0.001550 | 0.001495 |
| flatten.test | RUN | 0.249682 | 0.552192 | 0.617396 | 0.001508 | 0.001635 | 0.001467 |
| ticket-5490.test | COMPILE | 0.133164 | 0.237150 | 0.274939 | — | — | — |
| as_deque.test | RUN | 0.235680 | 0.538007 | 0.653963 | 0.001401 | 0.001609 | 0.001298 |
| as_list.test | RUN | 0.264150 | 0.653931 | 0.693025 | 0.001710 | 0.001618 | 0.001381 |
| as_map.test | RUN | 0.263865 | 0.672339 | 0.726142 | 0.001544 | 0.001528 | 0.001429 |
| as_map_assoc.test | RUN | 0.256298 | 0.626690 | 0.687952 | 0.001439 | 0.001595 | 0.001457 |
| as_set.test | RUN | 0.296819 | 0.758798 | 0.834608 | 0.001644 | 0.001347 | 0.001421 |
| as_vector.test | RUN | 0.270812 | 0.672249 | 0.713904 | 0.001376 | 0.001497 | 0.001454 |
| boost_tuple.test | RUN | 0.266299 | 0.692731 | 0.722918 | 0.001526 | 0.001677 | 0.001444 |
| boost_tuple_iterator.test | RUN | 0.183649 | 0.399025 | 0.439792 | 0.001567 | 0.001438 | 0.001191 |
| cons.test | RUN | 0.339577 | 0.759952 | 0.838848 | 0.001419 | 0.001595 | 0.001355 |
| convert_boost_tuple.test | RUN | 0.210150 | 0.581097 | 0.557568 | 0.001818 | 0.001637 | 0.001550 |
| convert_deque.test | RUN | 0.217575 | 0.567658 | 0.555563 | 0.001572 | 0.001317 | 0.001384 |
| convert_list.test | RUN | 0.210833 | 0.585706 | 0.569378 | 0.001455 | 0.001354 | 0.001311 |
| convert_std_pair.test | RUN | 0.223693 | 0.574078 | 0.585491 | 0.001795 | 0.001625 | 0.001404 |
| convert_std_tuple.test | RUN | 0.217855 | 0.562301 | 0.571221 | 0.001507 | 0.001321 | 0.001398 |
| convert_vector.test | RUN | 0.215759 | 0.576371 | 0.549836 | 0.001398 | 0.001473 | 0.001459 |
| filter_view.test | RUN | 0.268392 | 0.600600 | 0.658015 | 0.001664 | 0.001458 | 0.001508 |
| hash.test | RUN | 0.258942 | 0.530211 | 0.611148 | 0.001661 | 0.001622 | 0.001496 |
| io.test | RUN | 0.271164 | 0.777117 | 0.780289 | 0.001610 | 0.001768 | 0.001576 |
| iterator_range.test | RUN | 0.246485 | 0.579384 | 0.652288 | 0.001433 | 0.001599 | 0.001447 |
| joint_view.test | RUN | 0.331164 | 0.971314 | 0.937891 | 0.001906 | 0.001471 | 0.001508 |
| list_comparison.test | RUN | 0.190442 | 0.390173 | 0.446883 | 0.001574 | 0.001657 | 0.001462 |
| list_construction.test | RUN | 0.188365 | 0.392176 | 0.453673 | 0.001525 | 0.001621 | 0.001471 |
| list_copy.test | RUN | 0.279753 | 0.717878 | 0.769708 | 0.001790 | 0.001388 | 0.001336 |
| list_iterator.test | RUN | 0.186923 | 0.405236 | 0.439016 | 0.001470 | 0.001498 | 0.001396 |
| list_hash.test | RUN | 0.250648 | 0.757884 | 0.627654 | 0.001488 | 0.001661 | 0.001443 |
| list_make.test | RUN | 0.196598 | 0.447128 | 0.492144 | 0.001519 | 0.001636 | 0.001297 |
| list_misc.test | RUN | 0.254149 | 0.518697 | 0.590006 | 0.001542 | 0.001673 | 0.001544 |
| list_mutate.test | RUN | 0.180126 | 0.353267 | 0.396206 | 0.001650 | 0.001635 | 0.001471 |
| list_nest.test | RUN | 0.340124 | 0.951229 | 1.059979 | 0.001531 | 0.001381 | 0.001442 |
| list_tie.test | RUN | 0.186600 | 0.378322 | 0.437490 | 0.001344 | 0.001662 | 0.001434 |
| list_value_at.test | RUN | 0.180369 | 0.363131 | 0.435808 | 0.001574 | 0.001551 | 0.001532 |
| deque_comparison.test | RUN | 0.187853 | 0.419423 | 0.451207 | 0.001288 | 0.001604 | 0.001379 |
| deque_construction.test | RUN | 0.186899 | 0.379245 | 0.452503 | 0.001667 | 0.001638 | 0.001306 |
| deque_copy.test | RUN | 0.268152 | 0.655258 | 0.708465 | 0.001275 | 0.001374 | 0.001452 |
| deque_iterator.test | RUN | 0.183022 | 0.402123 | 0.428020 | 0.001768 | 0.001513 | 0.001522 |
| deque_hash.test | RUN | 0.254682 | 0.728301 | 0.643970 | 0.001698 | 0.001456 | 0.001488 |
| deque_is_constructible.test | COMPILE | 0.100615 | 0.182177 | 0.209884 | — | — | — |
| deque_make.test | RUN | 0.187962 | 0.404344 | 0.462931 | 0.001792 | 0.001577 | 0.001411 |
| deque_misc.test | RUN | 0.241983 | 0.508890 | 0.587642 | 0.001482 | 0.001457 | 0.001287 |
| deque_move.test | RUN | 0.189215 | 0.419506 | 0.447100 | 0.001545 | 0.001664 | 0.001362 |
| deque_mutate.test | RUN | 0.170335 | 0.344087 | 0.399008 | 0.001550 | 0.001604 | 0.001439 |
| deque_nest.test | RUN | 0.502430 | 1.525485 | 1.751585 | 0.001720 | 0.001803 | 0.001388 |
| deque_tie.test | RUN | 0.175485 | 0.369163 | 0.425500 | 0.001619 | 0.001735 | 0.001252 |
| deque_value_at.test | RUN | 0.172776 | 0.348030 | 0.404571 | 0.001446 | 0.001482 | 0.001238 |
| front_extended_deque.test | RUN | 0.255710 | 0.506508 | 0.605376 | 0.001738 | 0.001670 | 0.001502 |
| back_extended_deque.test | RUN | 0.249083 | 0.516995 | 0.599330 | 0.001497 | 0.001466 | 0.001467 |
| make_list.test | RUN | 0.199818 | 0.452330 | 0.475616 | 0.001734 | 0.001683 | 0.001338 |
| make_vector.test | RUN | 0.197436 | 0.421661 | 0.508473 | 0.001603 | 0.001549 | 0.001206 |
| map.test | RUN | 0.259411 | 0.694595 | 0.670555 | 0.001704 | 0.001562 | 0.001526 |
| map_comparison.test | RUN | 0.199422 | 0.455010 | 0.447963 | 0.001573 | 0.001656 | 0.001394 |
| map_construction.test | RUN | 0.201430 | 0.413803 | 0.452974 | 0.001579 | 0.001324 | 0.001334 |
| map_copy.test | RUN | 0.238907 | 0.495567 | 0.564026 | 0.001598 | 0.001649 | 0.001476 |
| map_misc.test | RUN | 0.248961 | 0.509763 | 0.579963 | 0.001672 | 0.001529 | 0.001201 |
| map_move.test | RUN | 0.197336 | 0.427432 | 0.460188 | 0.001568 | 0.001352 | 0.001192 |
| map_mutate.test | RUN | 0.177736 | 0.342508 | 0.398447 | 0.001612 | 0.001522 | 0.001224 |
| map_tie.test | RUN | 0.178437 | 0.343079 | 0.406131 | 0.001646 | 0.001564 | 0.001485 |
| nil.test | RUN | 0.126551 | 0.245661 | 0.284037 | 0.001696 | 0.001658 | 0.001446 |
| nview.test | RUN | 0.242697 | 0.502161 | 0.590559 | 0.001820 | 0.001701 | 0.001450 |
| reverse_view.test | RUN | 0.310744 | 0.819744 | 0.851179 | 0.001868 | 0.001528 | 0.001487 |
| segmented_iterator_range.test | RUN | 0.376121 | 0.971065 | 1.074440 | 0.001756 | 0.001449 | 0.001343 |
| set.test | RUN | 0.238970 | 0.574284 | 0.606610 | 0.001521 | 0.001317 | 0.001540 |
| single_view.test | RUN | 0.215581 | 0.521732 | 0.565124 | 0.001833 | 0.001559 | 0.001408 |
| std_pair.test | RUN | 0.268025 | 0.666406 | 0.716278 | 0.001673 | 0.001449 | 0.001360 |
| boost_array.test | RUN | 0.233486 | 0.466974 | 0.537813 | 0.001810 | 0.001670 | 0.001418 |
| array.test | RUN | 0.225076 | 0.458206 | 0.525259 | 0.001504 | 0.001298 | 0.001320 |
| std_array.test | RUN | 0.229412 | 0.463456 | 0.514595 | 0.001614 | 0.001704 | 0.001457 |
| tuple_comparison.test | RUN | 0.197752 | 0.415502 | 0.462340 | 0.001538 | 0.001704 | 0.001494 |
| tuple_construction.test | RUN | 0.204389 | 0.421143 | 0.482230 | 0.001756 | 0.001680 | 0.001300 |
| tuple_conversion.test | RUN | 0.444438 | 1.288000 | 1.603612 | 0.001556 | 0.001516 | 0.001248 |
| tuple_copy.test | RUN | 0.288366 | 0.735925 | 0.829981 | 0.001677 | 0.001454 | 0.001290 |
| tuple_element.test | RUN | 0.191366 | 0.385741 | 0.458605 | 0.001410 | 0.001624 | 0.001244 |
| tuple_make.test | RUN | 0.211401 | 0.437962 | 0.494810 | 0.001735 | 0.002462 | 0.001253 |
| tuple_misc.test | RUN | 0.265485 | 0.535180 | 0.645600 | 0.002124 | 0.001680 | 0.001463 |
| tuple_mutate.test | RUN | 0.185729 | 0.352071 | 0.420112 | 0.001746 | 0.001664 | 0.001602 |
| tuple_nest.test | RUN | 0.744797 | 2.756227 | 3.016073 | 0.001496 | 0.001675 | 0.001475 |
| tuple_hash.test | RUN | 0.272817 | 0.750071 | 0.655885 | 0.001801 | 0.001691 | 0.001183 |
| tuple_tie.test | RUN | 0.191686 | 0.402249 | 0.462997 | 0.001621 | 0.001600 | 0.001454 |
| tuple_traits__maybe_variadic.test | RUN | 0.216491 | 0.484286 | 0.477824 | 0.001825 | 0.001451 | 0.001457 |
| tuple_traits__no_variadic.test | RUN | 0.245508 | 0.540039 | 0.513844 | 0.001756 | 0.001693 | 0.001472 |
| transform_view.test | RUN | 0.316373 | 0.838321 | 0.861187 | 0.001814 | 0.001356 | 0.001312 |
| identity_view.test | RUN | 0.300824 | 0.735791 | 0.830239 | 0.001516 | 0.001361 | 0.001252 |
| vector_comparison.test | RUN | 0.191925 | 0.403418 | 0.499399 | 0.001486 | 0.001401 | 0.001511 |
| vector_construction.test | RUN | 0.192305 | 0.381070 | 0.449459 | 0.001441 | 0.001673 | 0.001524 |
| vector_conversion.test | RUN | 0.526346 | 1.722935 | 2.010512 | 0.001743 | 0.001340 | 0.001434 |
| vector_copy.test | RUN | 0.275952 | 0.736995 | 0.821462 | 0.001456 | 0.001487 | 0.001591 |
| vector_iterator.test | RUN | 0.182698 | 0.404191 | 0.452254 | 0.001663 | 0.001535 | 0.001406 |
| vector_make.test | RUN | 0.196179 | 0.434743 | 0.468791 | 0.001252 | 0.001719 | 0.001546 |
| vector_misc.test | RUN | 0.251485 | 0.538190 | 0.612450 | 0.001549 | 0.001466 | 0.001332 |
| vector_move.test | RUN | 0.194121 | 0.416985 | 0.447545 | 0.001550 | 0.001457 | 0.001327 |
| vector_mutate.test | RUN | 0.176027 | 0.341433 | 0.393564 | 0.001182 | 0.001609 | 0.001412 |
| vector_n.test | RUN | 0.219337 | 0.438656 | 0.534973 | 0.001840 | 0.001724 | 0.001501 |
| vector_nest.test | RUN | 0.743806 | 2.847763 | 3.024595 | 0.001750 | 0.001576 | 0.001196 |
| vector_hash.test | RUN | 0.253278 | 0.739728 | 0.599334 | 0.001756 | 0.001645 | 0.001336 |
| vector_tie.test | RUN | 0.183460 | 0.387781 | 0.449884 | 0.001750 | 0.001646 | 0.001202 |
| vector_traits__maybe_variadic.test | RUN | 0.198643 | 0.436187 | 0.465309 | 0.001738 | 0.001636 | 0.001367 |
| vector_traits__no_variadic.test | RUN | 0.208672 | 0.460197 | 0.481020 | 0.001692 | 0.001579 | 0.001419 |
| vector_value_at.test | RUN | 0.181870 | 0.340239 | 0.412210 | 0.001507 | 0.001764 | 0.001528 |
| zip_view.test | RUN | 0.375039 | 0.837349 | 1.018709 | 0.001553 | 0.001234 | 0.001459 |
| zip_view2.test | RUN | 0.294571 | 0.676293 | 0.802492 | 0.001505 | 0.001630 | 0.001366 |
| zip_view_ignore.test | RUN | 0.276689 | 0.652046 | 0.707381 | 0.001492 | 0.001666 | 0.001642 |
| repetitive_view.test | RUN | 0.477489 | 1.102809 | 1.349198 | 0.001642 | 0.001351 | 0.001512 |
| deduce_sequence.test | RUN | 0.225102 | 0.464461 | 0.582020 | 0.001442 | 0.001500 | 0.001383 |
| adapt_adt_named.test | RUN | 0.265743 | 0.668206 | 0.687667 | 0.001389 | 0.001692 | 0.001402 |
| adapt_adt_named_empty.test | RUN | 0.234100 | 0.554539 | 0.577354 | 0.001456 | 0.001607 | 0.001374 |
| adapt_adt.test | RUN | 0.311780 | 0.872786 | 0.834920 | 0.001569 | 0.001275 | 0.001402 |
| adapt_adt_empty.test | RUN | 0.234448 | 0.560516 | 0.573370 | 0.001723 | 0.001304 | 0.001626 |
| adapt_assoc_adt_named.test | RUN | 0.311166 | 0.681228 | 0.744726 | 0.001709 | 0.001358 | 0.001378 |
| adapt_assoc_adt_named_empty.test | RUN | 0.246402 | 0.542449 | 0.575092 | 0.001800 | 0.001660 | 0.001510 |
| adapt_assoc_adt.test | RUN | 0.305485 | 0.727405 | 0.800550 | 0.001586 | 0.001612 | 0.001433 |
| adapt_assoc_adt_empty.test | RUN | 0.232330 | 0.541784 | 0.565447 | 0.001784 | 0.001685 | 0.001459 |
| adapt_assoc_struct_named.test | RUN | 0.298300 | 0.692855 | 0.733187 | 0.001386 | 0.001619 | 0.001323 |
| adapt_assoc_struct_named_empty.test | RUN | 0.236325 | 0.540841 | 0.574169 | 0.001738 | 0.001384 | 0.001488 |
| adapt_assoc_struct.test | RUN | 0.271477 | 0.651004 | 0.691965 | 0.001715 | 0.001554 | 0.001491 |
| adapt_assoc_struct_empty.test | RUN | 0.233285 | 0.546659 | 0.569354 | 0.001462 | 0.001589 | 0.001399 |
| adapt_assoc_tpl_adt.test | RUN | 0.314670 | 0.738553 | 0.775121 | 0.001888 | 0.001564 | 0.001528 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.230364 | 0.540551 | 0.573268 | 0.001394 | 0.001590 | 0.001555 |
| adapt_assoc_tpl_struct.test | RUN | 0.266128 | 0.631683 | 0.670649 | 0.001571 | 0.001362 | 0.001281 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.235982 | 0.545721 | 0.587131 | 0.001573 | 0.001396 | 0.001410 |
| adapt_struct_named.test | RUN | 0.269235 | 0.657177 | 0.665996 | 0.001917 | 0.001347 | 0.001615 |
| adapt_struct_named_empty.test | RUN | 0.238902 | 0.541120 | 0.569162 | 0.001711 | 0.001395 | 0.001392 |
| adapt_struct.test | RUN | 0.293510 | 0.745964 | 0.778554 | 0.001598 | 0.001640 | 0.001449 |
| adapt_struct_empty.test | RUN | 0.236424 | 0.544031 | 0.566150 | 0.001847 | 0.001723 | 0.001464 |
| adapt_tpl_adt.test | RUN | 0.301333 | 0.840537 | 0.809315 | 0.001801 | 0.001617 | 0.001476 |
| adapt_tpl_adt_empty.test | RUN | 0.238637 | 0.537321 | 0.576436 | 0.001843 | 0.001592 | 0.001436 |
| adapt_tpl_struct.test | RUN | 0.251470 | 0.602430 | 0.626916 | 0.001637 | 0.001442 | 0.001482 |
| adapt_tpl_struct_empty.test | RUN | 0.235119 | 0.552518 | 0.578516 | 0.001571 | 0.001604 | 0.001448 |
| adt_attribute_proxy.test | RUN | 0.266880 | 0.518109 | 0.600814 | 0.001747 | 0.001678 | 0.001493 |
| define_struct.test | RUN | 0.327339 | 0.726079 | 0.801828 | 0.001714 | 0.001590 | 0.001455 |
| define_struct_empty.test | RUN | 0.290536 | 0.600443 | 0.643619 | 0.001540 | 0.001186 | 0.001485 |
| define_struct_move.test | RUN | 0.187765 | 0.349534 | 0.390139 | 0.001625 | 0.001442 | 0.001407 |
| define_struct_inline.test | RUN | 0.341684 | 0.871322 | 0.914929 | 0.001822 | 0.001466 | 0.001567 |
| define_struct_inline_empty.test | RUN | 0.280326 | 0.614780 | 0.650318 | 0.001327 | 0.001669 | 0.001486 |
| define_struct_inline_move.test | RUN | 0.181652 | 0.334650 | 0.407722 | 0.001812 | 0.001666 | 0.001397 |
| define_assoc_struct.test | RUN | 0.307803 | 0.695680 | 0.762420 | 0.001665 | 0.001640 | 0.001432 |
| define_assoc_struct_empty.test | RUN | 0.282693 | 0.597740 | 0.659112 | 0.001584 | 0.001584 | 0.001458 |
| define_assoc_struct_move.test | RUN | 0.180754 | 0.340370 | 0.394402 | 0.001749 | 0.001652 | 0.001541 |
| define_tpl_struct.test | RUN | 0.307750 | 0.704930 | 0.764203 | 0.001489 | 0.001446 | 0.001557 |
| define_tpl_struct_empty.test | RUN | 0.289151 | 0.624390 | 0.652825 | 0.001549 | 0.001604 | 0.001336 |
| define_tpl_struct_move.test | RUN | 0.186928 | 0.334110 | 0.395213 | 0.001422 | 0.001295 | 0.001464 |
| define_tpl_struct_inline.test | RUN | 0.325473 | 0.798255 | 0.834941 | 0.001706 | 0.001565 | 0.001376 |
| define_tpl_struct_inline_empty.test | RUN | 0.280769 | 0.605838 | 0.665414 | 0.001519 | 0.001469 | 0.001353 |
| define_tpl_struct_inline_move.test | RUN | 0.183270 | 0.339320 | 0.421227 | 0.001776 | 0.001635 | 0.001232 |
| define_assoc_tpl_struct.test | RUN | 0.313029 | 0.713013 | 0.765932 | 0.002044 | 0.001652 | 0.001491 |
| define_assoc_tpl_struct_empty.test | RUN | 0.293502 | 0.597427 | 0.666112 | 0.001679 | 0.001577 | 0.001367 |
| define_assoc_tpl_struct_move.test | RUN | 0.188180 | 0.340768 | 0.400321 | 0.001680 | 0.001572 | 0.001430 |
| std_tuple.test | RUN | 0.191630 | 0.392973 | 0.434954 | 0.001575 | 0.001276 | 0.001485 |
| std_tuple_iterator.test | RUN | 0.185574 | 0.398176 | 0.439302 | 0.001758 | 0.001627 | 0.001433 |
| ref_vector.test | RUN | 0.213031 | 0.403120 | 0.470915 | 0.001618 | 0.001644 | 0.001511 |
| flatten_view.test | RUN | 0.226693 | 0.499523 | 0.566014 | 0.001515 | 0.001319 | 0.001278 |
| github-159.test | COMPILE | 0.102983 | 0.188390 | 0.212953 | — | — | — |
| github-176.test | RUN | 0.209674 | 0.441525 | 0.473561 | 0.001774 | 0.001644 | 0.001421 |
| size.test | COMPILE | 0.176583 | 0.325547 | 0.370548 | — | — | — |
| fused.test | RUN | 0.252931 | 0.482131 | 0.555290 | 0.001649 | 0.001603 | 0.001463 |
| fused_function_object.test | RUN | 0.183740 | 0.361284 | 0.414538 | 0.001541 | 0.001473 | 0.001469 |
| fused_procedure.test | RUN | 0.239979 | 0.459659 | 0.521307 | 0.001405 | 0.001516 | 0.001506 |
| unfused.test | RUN | 0.191657 | 0.369588 | 0.439298 | 0.001613 | 0.001659 | 0.001390 |
| unfused_typed.test | RUN | 0.193086 | 0.404888 | 0.448267 | 0.001674 | 0.001663 | 0.001412 |
| make_fused.test | RUN | 0.240757 | 0.476089 | 0.556008 | 0.001536 | 0.001437 | 0.001381 |
| make_fused_function_object.test | RUN | 0.188515 | 0.367068 | 0.418121 | 0.001461 | 0.001299 | 0.001341 |
| make_fused_procedure.test | RUN | 0.236705 | 0.452806 | 0.528869 | 0.001803 | 0.001668 | 0.001391 |
| make_unfused.test | RUN | 0.185825 | 0.372024 | 0.432069 | 0.001818 | 0.001590 | 0.001442 |
| invoke.test | RUN | 0.477812 | 1.300518 | 1.459998 | 0.001894 | 0.001438 | 0.001243 |
| invoke_function_object.test | RUN | 0.227408 | 0.473428 | 0.535256 | 0.001827 | 0.001692 | 0.001460 |
| invoke_procedure.test | RUN | 0.383209 | 0.971280 | 1.072853 | 0.001534 | 0.001447 | 0.001479 |
| swap.test | RUN | 0.293974 | 0.602309 | 0.719497 | 0.001756 | 0.001669 | 0.001574 |
| is_sequence.test | COMPILE | 0.050395 | 0.089528 | 0.092788 | — | — | — |
| is_view.test | COMPILE | 0.049331 | 0.090452 | 0.093530 | — | — | — |
| pair_deque.test | COMPILE | 0.101248 | 0.191155 | 0.229968 | — | — | — |
| pair_list.test | COMPILE | 0.099707 | 0.189453 | 0.210316 | — | — | — |
| pair_map.test | COMPILE | 0.105466 | 0.198889 | 0.215192 | — | — | — |
| pair_set.test | COMPILE | 0.109631 | 0.192294 | 0.221376 | — | — | — |
| pair_vector.test | COMPILE | 0.104692 | 0.193223 | 0.210462 | — | — | — |
| pair_nest.test | COMPILE | 0.045373 | 0.079575 | 0.083455 | — | — | — |
| index_sequence.test | COMPILE | 0.037245 | 0.058548 | 0.068752 | — | — | — |
| and.test | COMPILE | 0.029739 | 0.043936 | 0.049681 | — | — | — |
| tag_of.test | COMPILE | 0.048190 | 0.084956 | 0.088636 | — | — | — |
| unused.test | COMPILE | 0.027216 | 0.054354 | 0.053909 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.049155 | 0.082464 | 0.072783 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.033363 | 0.057909 | 0.072162 | — | — | — |
| quick.test | RUN | 0.136642 | 0.279163 | 0.266193 | 0.000596 | 0.001095 | 0.001011 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.004× | 1.003×, 1.006× | 0.991× |
| clang | 1.002× | 1.005×, 0.999× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.000901s | 0.000909s | 0.000879s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000886s | 0.000890s | 0.000882s | byte equality observed |
| adapt_adt_named.test | 10 | 0.000882s | 0.000868s | 0.000870s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.000869s | 0.000858s | 0.000868s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000876s | 0.000873s | 0.000868s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.000879s | 0.000856s | 0.000871s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000874s | 0.000870s | 0.000866s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.000867s | 0.000881s | 0.000874s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.000877s | 0.000876s | 0.000867s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000862s | 0.000868s | 0.000874s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.000862s | 0.000874s | 0.000882s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.000856s | 0.000865s | 0.000874s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.000864s | 0.000880s | 0.000865s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000859s | 0.000867s | 0.000869s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.000878s | 0.000893s | 0.000887s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.000867s | 0.000867s | 0.000884s | byte equality observed |
| adapt_struct.test | 10 | 0.000879s | 0.000895s | 0.000889s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.000866s | 0.000870s | 0.000864s | byte equality observed |
| adapt_struct_named.test | 10 | 0.000872s | 0.000885s | 0.000895s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.000857s | 0.000864s | 0.000885s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.000869s | 0.000905s | 0.000877s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.000870s | 0.000883s | 0.000885s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.000873s | 0.000867s | 0.000869s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.000868s | 0.000870s | 0.000869s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000882s | 0.000858s | 0.000868s | byte equality observed |
| all.test | 10 | 0.000851s | 0.000852s | 0.000868s | byte equality observed |
| any.test | 10 | 0.000854s | 0.000850s | 0.000863s | byte equality observed |
| array.test | 10 | 0.000861s | 0.000853s | 0.000876s | byte equality observed |
| as_deque.test | 10 | 0.000869s | 0.000848s | 0.000858s | byte equality observed |
| as_list.test | 10 | 0.000904s | 0.000897s | 0.000891s | byte equality observed |
| as_map.test | 10 | 0.000889s | 0.000879s | 0.000876s | byte equality observed |
| as_map_assoc.test | 10 | 0.000875s | 0.000867s | 0.000880s | byte equality observed |
| as_set.test | 10 | 0.000892s | 0.000878s | 0.000868s | byte equality observed |
| as_vector.test | 10 | 0.000875s | 0.000885s | 0.000878s | byte equality observed |
| back_extended_deque.test | 10 | 0.000856s | 0.000860s | 0.000858s | byte equality observed |
| boost_array.test | 10 | 0.000853s | 0.000854s | 0.000848s | byte equality observed |
| boost_tuple.test | 10 | 0.000883s | 0.000889s | 0.000870s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000849s | 0.000844s | 0.000863s | byte equality observed |
| clear.test | 10 | 0.000875s | 0.000875s | 0.000878s | byte equality observed |
| cons.test | 10 | 0.000892s | 0.000883s | 0.000896s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000870s | 0.000889s | 0.000856s | byte equality observed |
| convert_deque.test | 10 | 0.000862s | 0.000858s | 0.000857s | byte equality observed |
| convert_list.test | 10 | 0.000862s | 0.000860s | 0.000866s | byte equality observed |
| convert_std_pair.test | 10 | 0.000882s | 0.000876s | 0.000873s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000856s | 0.000862s | 0.000865s | byte equality observed |
| convert_vector.test | 10 | 0.000861s | 0.000870s | 0.000868s | byte equality observed |
| copy.test | 10 | 0.000852s | 0.000855s | 0.000858s | byte equality observed |
| count.test | 10 | 0.000854s | 0.000873s | 0.000851s | byte equality observed |
| count_if.test | 10 | 0.000848s | 0.000873s | 0.000859s | byte equality observed |
| deduce_sequence.test | 10 | 0.000852s | 0.000843s | 0.000866s | byte equality observed |
| define_assoc_struct.test | 10 | 0.000878s | 0.000889s | 0.000883s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.000873s | 0.000883s | 0.000873s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000860s | 0.000872s | 0.000876s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.000887s | 0.000879s | 0.000895s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.000881s | 0.000878s | 0.000885s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000854s | 0.000858s | 0.000851s | byte equality observed |
| define_struct.test | 10 | 0.000884s | 0.000898s | 0.000879s | byte equality observed |
| define_struct_empty.test | 10 | 0.000873s | 0.000883s | 0.000863s | byte equality observed |
| define_struct_inline.test | 10 | 0.000881s | 0.000879s | 0.000873s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.000862s | 0.000897s | 0.000861s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000863s | 0.000880s | 0.000855s | byte equality observed |
| define_struct_move.test | 10 | 0.000870s | 0.000854s | 0.000852s | byte equality observed |
| define_tpl_struct.test | 10 | 0.000886s | 0.000880s | 0.000871s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000881s | 0.000872s | 0.000873s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.000884s | 0.000883s | 0.000888s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.000865s | 0.000865s | 0.000885s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000846s | 0.000847s | 0.000887s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000860s | 0.000870s | 0.000861s | byte equality observed |
| deque_comparison.test | 10 | 0.000846s | 0.000863s | 0.000874s | byte equality observed |
| deque_construction.test | 10 | 0.000862s | 0.000846s | 0.000899s | byte equality observed |
| deque_copy.test | 10 | 0.000861s | 0.000877s | 0.000883s | byte equality observed |
| deque_hash.test | 10 | 0.000861s | 0.000857s | 0.000870s | byte equality observed |
| deque_iterator.test | 10 | 0.000854s | 0.000889s | 0.000872s | byte equality observed |
| deque_make.test | 10 | 0.000870s | 0.000862s | 0.000864s | byte equality observed |
| deque_misc.test | 10 | 0.000880s | 0.000873s | 0.000868s | byte equality observed |
| deque_move.test | 10 | 0.000865s | 0.000879s | 0.000877s | byte equality observed |
| deque_mutate.test | 10 | 0.000871s | 0.000858s | 0.000867s | byte equality observed |
| deque_nest.test | 10 | 0.000851s | 0.000875s | 0.000854s | byte equality observed |
| deque_tie.test | 10 | 0.000856s | 0.000861s | 0.000874s | byte equality observed |
| deque_value_at.test | 10 | 0.000844s | 0.000865s | 0.000895s | byte equality observed |
| erase.test | 10 | 0.000882s | 0.000883s | 0.000901s | byte equality observed |
| erase_key.test | 10 | 0.000879s | 0.000886s | 0.000918s | byte equality observed |
| filter.test | 10 | 0.000873s | 0.000875s | 0.000888s | byte equality observed |
| filter_if.test | 10 | 0.000889s | 0.000893s | 0.000888s | byte equality observed |
| filter_view.test | 10 | 0.000871s | 0.000873s | 0.000880s | byte equality observed |
| find.test | 10 | 0.000879s | 0.000876s | 0.000880s | byte equality observed |
| find_if.test | 10 | 0.000867s | 0.000871s | 0.000865s | byte equality observed |
| flatten.test | 10 | 0.000863s | 0.000862s | 0.000863s | byte equality observed |
| flatten_view.test | 10 | 0.000865s | 0.000865s | 0.000863s | byte equality observed |
| fold.test | 10 | 0.000867s | 0.000866s | 0.000888s | byte equality observed |
| for_each.test | 10 | 0.000875s | 0.000881s | 0.000895s | byte equality observed |
| front_extended_deque.test | 10 | 0.000863s | 0.000852s | 0.000868s | byte equality observed |
| fused.test | 10 | 0.000867s | 0.000850s | 0.000859s | byte equality observed |
| fused_function_object.test | 10 | 0.000866s | 0.000855s | 0.000859s | byte equality observed |
| fused_procedure.test | 10 | 0.000856s | 0.000864s | 0.000870s | byte equality observed |
| github-176.test | 10 | 0.000855s | 0.000879s | 0.000901s | byte equality observed |
| hash.test | 10 | 0.000880s | 0.000860s | 0.000870s | byte equality observed |
| identity_view.test | 10 | 0.000882s | 0.000894s | 0.000887s | byte equality observed |
| insert.test | 10 | 0.000892s | 0.000883s | 0.000895s | byte equality observed |
| insert_range.test | 10 | 0.000901s | 0.000901s | 0.000911s | byte equality observed |
| invoke.test | 10 | 0.000864s | 0.000874s | 0.000863s | byte equality observed |
| invoke_function_object.test | 10 | 0.000852s | 0.000851s | 0.000873s | byte equality observed |
| invoke_procedure.test | 10 | 0.000863s | 0.000867s | 0.000874s | byte equality observed |
| io.test | 10 | 0.000992s | 0.000974s | 0.000982s | byte equality observed |
| iter_fold.test | 10 | 0.000882s | 0.000893s | 0.000873s | byte equality observed |
| iterator_range.test | 10 | 0.000891s | 0.000888s | 0.000889s | byte equality observed |
| join.test | 10 | 0.000861s | 0.000862s | 0.000843s | byte equality observed |
| joint_view.test | 10 | 0.000911s | 0.000929s | 0.000921s | byte equality observed |
| list_comparison.test | 10 | 0.000856s | 0.000856s | 0.000848s | byte equality observed |
| list_construction.test | 10 | 0.000859s | 0.000851s | 0.000854s | byte equality observed |
| list_copy.test | 10 | 0.000856s | 0.000873s | 0.000837s | byte equality observed |
| list_hash.test | 10 | 0.000859s | 0.000880s | 0.000858s | byte equality observed |
| list_iterator.test | 10 | 0.000871s | 0.000859s | 0.000854s | byte equality observed |
| list_make.test | 10 | 0.000866s | 0.000876s | 0.000851s | byte equality observed |
| list_misc.test | 10 | 0.000869s | 0.000867s | 0.000871s | byte equality observed |
| list_mutate.test | 10 | 0.000857s | 0.000854s | 0.000850s | byte equality observed |
| list_nest.test | 10 | 0.000864s | 0.000848s | 0.000844s | byte equality observed |
| list_tie.test | 10 | 0.000854s | 0.000898s | 0.000846s | byte equality observed |
| list_value_at.test | 10 | 0.000868s | 0.000850s | 0.000871s | byte equality observed |
| make_fused.test | 10 | 0.000854s | 0.000876s | 0.000861s | byte equality observed |
| make_fused_function_object.test | 10 | 0.000868s | 0.000864s | 0.000868s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000852s | 0.000852s | 0.000874s | byte equality observed |
| make_list.test | 10 | 0.000900s | 0.000859s | 0.000862s | byte equality observed |
| make_unfused.test | 10 | 0.000861s | 0.000849s | 0.000858s | byte equality observed |
| make_vector.test | 10 | 0.000874s | 0.000868s | 0.000852s | byte equality observed |
| map.test | 10 | 0.000896s | 0.000888s | 0.000893s | byte equality observed |
| map_comparison.test | 10 | 0.000859s | 0.000866s | 0.000853s | byte equality observed |
| map_construction.test | 10 | 0.000867s | 0.000856s | 0.000868s | byte equality observed |
| map_copy.test | 10 | 0.000869s | 0.000862s | 0.000872s | byte equality observed |
| map_misc.test | 10 | 0.000861s | 0.000872s | 0.000869s | byte equality observed |
| map_move.test | 10 | 0.000882s | 0.000878s | 0.000882s | byte equality observed |
| map_mutate.test | 10 | 0.000862s | 0.000870s | 0.000844s | byte equality observed |
| map_tie.test | 10 | 0.000847s | 0.000848s | 0.000849s | byte equality observed |
| move.test | 10 | 0.000858s | 0.000850s | 0.000878s | byte equality observed |
| nil.test | 10 | 0.000867s | 0.000851s | 0.000872s | byte equality observed |
| none.test | 10 | 0.000852s | 0.000857s | 0.000857s | byte equality observed |
| nview.test | 10 | 0.000860s | 0.000859s | 0.000861s | byte equality observed |
| pop_back.test | 10 | 0.000897s | 0.000881s | 0.000893s | byte equality observed |
| pop_front.test | 10 | 0.000895s | 0.000901s | 0.000888s | byte equality observed |
| push_back.test | 10 | 0.000898s | 0.000901s | 0.000890s | byte equality observed |
| push_front.test | 10 | 0.000893s | 0.000905s | 0.000892s | byte equality observed |
| quick.test | 10 | 0.000393s | 0.000833s | 0.000838s | byte equality observed |
| ref_vector.test | 10 | 0.000887s | 0.000859s | 0.000861s | byte equality observed |
| remove.test | 10 | 0.000895s | 0.000908s | 0.000901s | byte equality observed |
| remove_if.test | 10 | 0.000880s | 0.000883s | 0.000884s | byte equality observed |
| repetitive_view.test | 10 | 0.000868s | 0.000853s | 0.000865s | byte equality observed |
| replace.test | 10 | 0.000878s | 0.000877s | 0.000880s | byte equality observed |
| replace_if.test | 10 | 0.000904s | 0.000874s | 0.000907s | byte equality observed |
| reverse.test | 10 | 0.000902s | 0.000886s | 0.000884s | byte equality observed |
| reverse_fold.test | 10 | 0.000880s | 0.000874s | 0.000881s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.000882s | 0.000873s | 0.000873s | byte equality observed |
| reverse_view.test | 10 | 0.000886s | 0.000887s | 0.000863s | byte equality observed |
| segmented_find.test | 10 | 0.000859s | 0.000861s | 0.000851s | byte equality observed |
| segmented_find_if.test | 10 | 0.000863s | 0.000877s | 0.000856s | byte equality observed |
| segmented_fold.test | 10 | 0.000884s | 0.000888s | 0.000883s | byte equality observed |
| segmented_for_each.test | 10 | 0.000893s | 0.000883s | 0.000880s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.000907s | 0.000926s | 0.000899s | byte equality observed |
| set.test | 10 | 0.000876s | 0.000886s | 0.000868s | byte equality observed |
| single_view.test | 10 | 0.000882s | 0.000892s | 0.000882s | byte equality observed |
| std_array.test | 10 | 0.000877s | 0.000854s | 0.000863s | byte equality observed |
| std_pair.test | 10 | 0.000889s | 0.000882s | 0.000897s | byte equality observed |
| std_tuple.test | 10 | 0.000871s | 0.000856s | 0.000845s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000849s | 0.000869s | 0.000886s | byte equality observed |
| swap.test | 10 | 0.000863s | 0.000851s | 0.000848s | byte equality observed |
| transform.test | 10 | 0.000890s | 0.000870s | 0.000873s | byte equality observed |
| transform_view.test | 10 | 0.000896s | 0.000879s | 0.000882s | byte equality observed |
| tuple_comparison.test | 10 | 0.000847s | 0.000846s | 0.000850s | byte equality observed |
| tuple_construction.test | 10 | 0.000871s | 0.000873s | 0.000850s | byte equality observed |
| tuple_conversion.test | 10 | 0.000853s | 0.000864s | 0.000853s | byte equality observed |
| tuple_copy.test | 10 | 0.000868s | 0.000850s | 0.000855s | byte equality observed |
| tuple_element.test | 10 | 0.000868s | 0.000869s | 0.000867s | byte equality observed |
| tuple_hash.test | 10 | 0.000858s | 0.000866s | 0.000854s | byte equality observed |
| tuple_make.test | 10 | 0.000862s | 0.000877s | 0.000870s | byte equality observed |
| tuple_misc.test | 10 | 0.000849s | 0.000873s | 0.000844s | byte equality observed |
| tuple_mutate.test | 10 | 0.000865s | 0.000880s | 0.000850s | byte equality observed |
| tuple_nest.test | 10 | 0.000864s | 0.000849s | 0.000861s | byte equality observed |
| tuple_tie.test | 10 | 0.000869s | 0.000831s | 0.000842s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000843s | 0.000851s | 0.000882s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.000872s | 0.000846s | 0.000876s | byte equality observed |
| unfused.test | 10 | 0.000861s | 0.000869s | 0.000850s | byte equality observed |
| unfused_typed.test | 10 | 0.000848s | 0.000860s | 0.000849s | byte equality observed |
| vector_comparison.test | 10 | 0.000883s | 0.000857s | 0.000865s | byte equality observed |
| vector_construction.test | 10 | 0.000857s | 0.000870s | 0.000857s | byte equality observed |
| vector_conversion.test | 10 | 0.000874s | 0.000854s | 0.000872s | byte equality observed |
| vector_copy.test | 10 | 0.000861s | 0.000860s | 0.000860s | byte equality observed |
| vector_hash.test | 10 | 0.000857s | 0.000868s | 0.000860s | byte equality observed |
| vector_iterator.test | 10 | 0.000860s | 0.000848s | 0.000857s | byte equality observed |
| vector_make.test | 10 | 0.000863s | 0.000862s | 0.000854s | byte equality observed |
| vector_misc.test | 10 | 0.000857s | 0.000853s | 0.000865s | byte equality observed |
| vector_move.test | 10 | 0.000869s | 0.000884s | 0.000864s | byte equality observed |
| vector_mutate.test | 10 | 0.000856s | 0.000868s | 0.000875s | byte equality observed |
| vector_n.test | 10 | 0.000880s | 0.000887s | 0.000897s | byte equality observed |
| vector_nest.test | 10 | 0.000850s | 0.000857s | 0.000870s | byte equality observed |
| vector_tie.test | 10 | 0.000863s | 0.000874s | 0.000846s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000846s | 0.000856s | 0.000863s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000872s | 0.000869s | 0.000852s | byte equality observed |
| vector_value_at.test | 10 | 0.000860s | 0.000852s | 0.000854s | byte equality observed |
| zip.test | 10 | 0.000868s | 0.000858s | 0.000853s | byte equality observed |
| zip2.test | 10 | 0.000865s | 0.000865s | 0.000867s | byte equality observed |
| zip_ignore.test | 10 | 0.000846s | 0.000843s | 0.000864s | byte equality observed |
| zip_view.test | 10 | 0.000873s | 0.000870s | 0.000873s | byte equality observed |
| zip_view2.test | 10 | 0.000881s | 0.000882s | 0.000851s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000854s | 0.000850s | 0.000864s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
