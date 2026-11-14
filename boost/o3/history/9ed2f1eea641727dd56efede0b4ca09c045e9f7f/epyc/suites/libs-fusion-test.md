# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.350060 | 0.716810 | 1.016958 | 0.001465 | 0.001532 | 0.001499 |
| any.test | RUN | 0.340012 | 0.682862 | 0.973125 | 0.001558 | 0.001504 | 0.001658 |
| clear.test | RUN | 0.243954 | 0.631421 | 0.789692 | 0.001624 | 0.001751 | 0.001790 |
| copy.test | RUN | 0.208357 | 0.479113 | 0.650906 | 0.001517 | 0.001598 | 0.001718 |
| count.test | RUN | 0.232315 | 0.525749 | 0.709110 | 0.001692 | 0.001818 | 0.001724 |
| count_if.test | RUN | 0.221750 | 0.496471 | 0.653440 | 0.001623 | 0.001900 | 0.001576 |
| erase.test | RUN | 0.312354 | 0.867883 | 1.053679 | 0.001809 | 0.001923 | 0.001517 |
| erase_key.test | RUN | 0.303987 | 0.862000 | 0.996344 | 0.001620 | 0.001693 | 0.001666 |
| filter.test | RUN | 0.280113 | 0.727029 | 0.908947 | 0.001970 | 0.002142 | 0.001732 |
| filter_if.test | RUN | 0.320091 | 0.877060 | 1.056664 | 0.001660 | 0.001558 | 0.001758 |
| find.test | RUN | 0.277611 | 0.606283 | 0.845957 | 0.001732 | 0.001682 | 0.001636 |
| find_if.test | RUN | 0.263830 | 0.543274 | 0.743323 | 0.001592 | 0.001686 | 0.001640 |
| fold.test | RUN | 0.281625 | 0.677173 | 0.924315 | 0.001813 | 0.001822 | 0.001563 |
| for_each.test | RUN | 0.251864 | 0.610767 | 0.790651 | 0.001629 | 0.001940 | 0.001685 |
| insert.test | RUN | 0.364615 | 1.088231 | 1.275524 | 0.001865 | 0.001656 | 0.001640 |
| insert_range.test | RUN | 0.373103 | 1.079157 | 1.284119 | 0.001690 | 0.001820 | 0.001701 |
| iter_fold.test | RUN | 0.443648 | 1.049784 | 1.541161 | 0.001841 | 0.001900 | 0.001780 |
| move.test | RUN | 0.224286 | 0.504626 | 0.671618 | 0.001826 | 0.001728 | 0.001638 |
| none.test | RUN | 0.350684 | 0.705742 | 0.978310 | 0.001559 | 0.001936 | 0.001632 |
| pop_back.test | RUN | 0.335300 | 0.957278 | 1.169012 | 0.001645 | 0.001733 | 0.001828 |
| pop_front.test | RUN | 0.288084 | 0.783501 | 0.954297 | 0.001890 | 0.001672 | 0.001587 |
| push_back.test | RUN | 0.356747 | 1.100282 | 1.222169 | 0.001903 | 0.001998 | 0.001512 |
| push_front.test | RUN | 0.317339 | 0.912616 | 1.055912 | 0.001756 | 0.001867 | 0.001695 |
| remove.test | RUN | 0.350571 | 1.037348 | 1.255552 | 0.001618 | 0.001690 | 0.001434 |
| remove_if.test | RUN | 0.311013 | 0.893412 | 1.092245 | 0.001656 | 0.001836 | 0.001554 |
| replace.test | RUN | 0.312696 | 0.833652 | 0.995379 | 0.001798 | 0.001975 | 0.001524 |
| replace_if.test | RUN | 0.298312 | 0.815532 | 0.996789 | 0.001875 | 0.001906 | 0.001387 |
| reverse_fold.test | RUN | 0.485386 | 1.181325 | 1.704377 | 0.001594 | 0.001870 | 0.001553 |
| reverse_iter_fold.test | RUN | 0.497488 | 1.234856 | 1.772504 | 0.001687 | 0.001688 | 0.001624 |
| reverse.test | RUN | 0.306304 | 0.870854 | 1.013136 | 0.001571 | 0.001821 | 0.001724 |
| segmented_for_each.test | RUN | 0.518347 | 1.638012 | 2.042503 | 0.001589 | 0.001578 | 0.001733 |
| segmented_find.test | RUN | 0.314862 | 0.899720 | 1.155876 | 0.001640 | 0.001786 | 0.001470 |
| segmented_find_if.test | RUN | 0.324311 | 0.878565 | 1.167410 | 0.001605 | 0.001756 | 0.001512 |
| segmented_fold.test | RUN | 0.411709 | 1.152365 | 1.451232 | 0.001774 | 0.002010 | 0.001816 |
| transform.test | RUN | 0.310335 | 0.901494 | 1.056232 | 0.001695 | 0.001870 | 0.001690 |
| join.test | RUN | 0.241292 | 0.556037 | 0.761559 | 0.001760 | 0.001832 | 0.001747 |
| zip.test | RUN | 0.430888 | 1.078024 | 1.427061 | 0.001623 | 0.001828 | 0.001688 |
| zip2.test | RUN | 0.414771 | 1.031760 | 1.362422 | 0.001664 | 0.001766 | 0.001572 |
| zip_ignore.test | RUN | 0.323117 | 0.682826 | 0.921401 | 0.001600 | 0.001479 | 0.001479 |
| flatten.test | RUN | 0.280524 | 0.787453 | 0.939668 | 0.001541 | 0.001773 | 0.001731 |
| ticket-5490.test | COMPILE | 0.151875 | 0.345687 | 0.407319 | — | — | — |
| as_deque.test | RUN | 0.286163 | 0.731854 | 0.979437 | 0.001844 | 0.001852 | 0.001726 |
| as_list.test | RUN | 0.319874 | 0.889368 | 1.063139 | 0.001660 | 0.001740 | 0.001767 |
| as_map.test | RUN | 0.316586 | 0.865422 | 1.075030 | 0.001523 | 0.001687 | 0.001522 |
| as_map_assoc.test | RUN | 0.311711 | 0.827188 | 0.991334 | 0.001480 | 0.001918 | 0.001428 |
| as_set.test | RUN | 0.363423 | 1.029655 | 1.251491 | 0.001556 | 0.001850 | 0.001660 |
| as_vector.test | RUN | 0.322696 | 0.940789 | 1.084066 | 0.001677 | 0.001734 | 0.001431 |
| boost_tuple.test | RUN | 0.325479 | 0.904278 | 1.061658 | 0.001452 | 0.001654 | 0.001581 |
| boost_tuple_iterator.test | RUN | 0.230608 | 0.554830 | 0.666792 | 0.001537 | 0.001581 | 0.001752 |
| cons.test | RUN | 0.409198 | 1.052353 | 1.287198 | 0.001574 | 0.001833 | 0.001587 |
| convert_boost_tuple.test | RUN | 0.257397 | 0.789977 | 0.844874 | 0.001803 | 0.001732 | 0.001488 |
| convert_deque.test | RUN | 0.261093 | 0.777918 | 0.840870 | 0.001490 | 0.001718 | 0.001254 |
| convert_list.test | RUN | 0.257725 | 0.798315 | 0.857355 | 0.001520 | 0.001692 | 0.001580 |
| convert_std_pair.test | RUN | 0.267682 | 0.789379 | 0.860544 | 0.001756 | 0.001561 | 0.001660 |
| convert_std_tuple.test | RUN | 0.252610 | 0.773390 | 0.830148 | 0.001279 | 0.001749 | 0.001504 |
| convert_vector.test | RUN | 0.256432 | 0.783070 | 0.845099 | 0.001782 | 0.001645 | 0.001590 |
| filter_view.test | RUN | 0.302183 | 0.829002 | 0.998260 | 0.001656 | 0.001785 | 0.001765 |
| hash.test | RUN | 0.304701 | 0.734358 | 0.984400 | 0.001746 | 0.001704 | 0.001617 |
| io.test | RUN | 0.313506 | 1.012534 | 1.132972 | 0.001518 | 0.002084 | 0.001965 |
| iterator_range.test | RUN | 0.285295 | 0.777600 | 0.936040 | 0.001659 | 0.001927 | 0.001370 |
| joint_view.test | RUN | 0.380792 | 1.286000 | 1.416831 | 0.001778 | 0.001729 | 0.001797 |
| list_comparison.test | RUN | 0.226758 | 0.543716 | 0.687970 | 0.001762 | 0.001463 | 0.001611 |
| list_construction.test | RUN | 0.224409 | 0.538634 | 0.675120 | 0.001568 | 0.001688 | 0.001706 |
| list_copy.test | RUN | 0.335582 | 0.942809 | 1.164780 | 0.001619 | 0.001577 | 0.001623 |
| list_iterator.test | RUN | 0.226606 | 0.537554 | 0.652370 | 0.001564 | 0.001667 | 0.001440 |
| list_hash.test | RUN | 0.312032 | 0.990453 | 0.958414 | 0.001600 | 0.001830 | 0.001744 |
| list_make.test | RUN | 0.236601 | 0.585520 | 0.721190 | 0.001587 | 0.001584 | 0.001684 |
| list_misc.test | RUN | 0.316565 | 0.693210 | 0.900376 | 0.001417 | 0.001708 | 0.001732 |
| list_mutate.test | RUN | 0.217712 | 0.470183 | 0.593981 | 0.001583 | 0.001544 | 0.001752 |
| list_nest.test | RUN | 0.417291 | 1.295043 | 1.645663 | 0.001672 | 0.001909 | 0.001701 |
| list_tie.test | RUN | 0.231798 | 0.533133 | 0.658006 | 0.001869 | 0.001854 | 0.001748 |
| list_value_at.test | RUN | 0.221424 | 0.488628 | 0.658576 | 0.001425 | 0.001748 | 0.001707 |
| deque_comparison.test | RUN | 0.235156 | 0.534763 | 0.682594 | 0.001538 | 0.001545 | 0.001738 |
| deque_construction.test | RUN | 0.235699 | 0.524058 | 0.679784 | 0.001748 | 0.001760 | 0.001684 |
| deque_copy.test | RUN | 0.321778 | 0.879639 | 1.106234 | 0.001583 | 0.001695 | 0.001734 |
| deque_iterator.test | RUN | 0.225139 | 0.566204 | 0.674665 | 0.001592 | 0.001579 | 0.001600 |
| deque_hash.test | RUN | 0.309259 | 0.967112 | 0.991212 | 0.001535 | 0.001702 | 0.001712 |
| deque_is_constructible.test | COMPILE | 0.121808 | 0.253466 | 0.321789 | — | — | — |
| deque_make.test | RUN | 0.228862 | 0.534917 | 0.680237 | 0.001796 | 0.001732 | 0.001726 |
| deque_misc.test | RUN | 0.296750 | 0.682476 | 0.851504 | 0.001508 | 0.001636 | 0.001501 |
| deque_move.test | RUN | 0.229523 | 0.566625 | 0.677529 | 0.001531 | 0.001541 | 0.001465 |
| deque_mutate.test | RUN | 0.212369 | 0.453571 | 0.613462 | 0.001308 | 0.001802 | 0.001716 |
| deque_nest.test | RUN | 0.623581 | 2.058412 | 2.614430 | 0.001765 | 0.001598 | 0.001600 |
| deque_tie.test | RUN | 0.212995 | 0.488944 | 0.646435 | 0.001556 | 0.001617 | 0.001539 |
| deque_value_at.test | RUN | 0.211942 | 0.479045 | 0.629100 | 0.001600 | 0.001442 | 0.001419 |
| front_extended_deque.test | RUN | 0.317959 | 0.719062 | 0.970964 | 0.001674 | 0.001898 | 0.001662 |
| back_extended_deque.test | RUN | 0.313435 | 0.708188 | 0.922440 | 0.001553 | 0.001686 | 0.001673 |
| make_list.test | RUN | 0.231919 | 0.572975 | 0.738287 | 0.001688 | 0.001728 | 0.001724 |
| make_vector.test | RUN | 0.232067 | 0.593024 | 0.702900 | 0.001659 | 0.001505 | 0.001452 |
| map.test | RUN | 0.305514 | 0.859895 | 1.046695 | 0.001846 | 0.001758 | 0.001773 |
| map_comparison.test | RUN | 0.229912 | 0.573220 | 0.709962 | 0.001700 | 0.001672 | 0.001608 |
| map_construction.test | RUN | 0.237383 | 0.543941 | 0.697244 | 0.001524 | 0.001806 | 0.001746 |
| map_copy.test | RUN | 0.274794 | 0.701158 | 0.848423 | 0.001515 | 0.001711 | 0.001627 |
| map_misc.test | RUN | 0.293982 | 0.698162 | 0.870389 | 0.001353 | 0.001736 | 0.001409 |
| map_move.test | RUN | 0.230944 | 0.580474 | 0.705500 | 0.001370 | 0.001520 | 0.001557 |
| map_mutate.test | RUN | 0.212007 | 0.458621 | 0.596451 | 0.001528 | 0.001821 | 0.001604 |
| map_tie.test | RUN | 0.215358 | 0.469508 | 0.621883 | 0.001777 | 0.001744 | 0.001598 |
| nil.test | RUN | 0.149338 | 0.322747 | 0.437853 | 0.001758 | 0.001790 | 0.001657 |
| nview.test | RUN | 0.287710 | 0.671946 | 0.890486 | 0.001820 | 0.001940 | 0.001662 |
| reverse_view.test | RUN | 0.360813 | 1.105825 | 1.294565 | 0.001790 | 0.001809 | 0.001633 |
| segmented_iterator_range.test | RUN | 0.445710 | 1.307305 | 1.650263 | 0.001718 | 0.001764 | 0.001503 |
| set.test | RUN | 0.282464 | 0.748013 | 0.894089 | 0.001526 | 0.001849 | 0.001800 |
| single_view.test | RUN | 0.255865 | 0.685978 | 0.858044 | 0.001790 | 0.001920 | 0.001474 |
| std_pair.test | RUN | 0.329831 | 0.891822 | 1.047244 | 0.001717 | 0.001820 | 0.001580 |
| boost_array.test | RUN | 0.277536 | 0.634734 | 0.819564 | 0.001648 | 0.001850 | 0.001649 |
| array.test | RUN | 0.281560 | 0.609203 | 0.800329 | 0.001479 | 0.001589 | 0.001614 |
| std_array.test | RUN | 0.273276 | 0.605017 | 0.794290 | 0.001810 | 0.001850 | 0.001603 |
| tuple_comparison.test | RUN | 0.242008 | 0.569159 | 0.753212 | 0.001715 | 0.001853 | 0.001497 |
| tuple_construction.test | RUN | 0.249782 | 0.581490 | 0.762967 | 0.001854 | 0.001817 | 0.001506 |
| tuple_conversion.test | RUN | 0.546618 | 1.723073 | 2.403865 | 0.001413 | 0.001725 | 0.001301 |
| tuple_copy.test | RUN | 0.351524 | 1.004900 | 1.286742 | 0.001608 | 0.001506 | 0.001796 |
| tuple_element.test | RUN | 0.236970 | 0.539459 | 0.688453 | 0.001681 | 0.001625 | 0.001699 |
| tuple_make.test | RUN | 0.254400 | 0.615896 | 0.783537 | 0.001680 | 0.001744 | 0.001520 |
| tuple_misc.test | RUN | 0.313752 | 0.723277 | 0.929749 | 0.001728 | 0.001843 | 0.001602 |
| tuple_mutate.test | RUN | 0.220937 | 0.486842 | 0.673139 | 0.001861 | 0.001749 | 0.001624 |
| tuple_nest.test | RUN | 0.889184 | 3.548087 | 4.566194 | 0.001666 | 0.001837 | 0.001654 |
| tuple_hash.test | RUN | 0.324665 | 0.996697 | 0.993240 | 0.001787 | 0.001628 | 0.001624 |
| tuple_tie.test | RUN | 0.235905 | 0.542899 | 0.723893 | 0.001666 | 0.001556 | 0.001756 |
| tuple_traits__maybe_variadic.test | RUN | 0.259398 | 0.671936 | 0.788831 | 0.001540 | 0.001538 | 0.001362 |
| tuple_traits__no_variadic.test | RUN | 0.288497 | 0.678988 | 0.811164 | 0.001573 | 0.001830 | 0.001569 |
| transform_view.test | RUN | 0.388942 | 1.085997 | 1.322605 | 0.001728 | 0.001806 | 0.001671 |
| identity_view.test | RUN | 0.358610 | 0.997684 | 1.216985 | 0.001604 | 0.001600 | 0.001723 |
| vector_comparison.test | RUN | 0.235035 | 0.558314 | 0.693776 | 0.001551 | 0.001556 | 0.001642 |
| vector_construction.test | RUN | 0.236066 | 0.512412 | 0.659610 | 0.001540 | 0.001910 | 0.001654 |
| vector_conversion.test | RUN | 0.633990 | 2.306217 | 3.101038 | 0.001750 | 0.001640 | 0.001785 |
| vector_copy.test | RUN | 0.343166 | 0.992332 | 1.251908 | 0.001622 | 0.001794 | 0.001588 |
| vector_iterator.test | RUN | 0.221164 | 0.549839 | 0.644158 | 0.001715 | 0.001415 | 0.001782 |
| vector_make.test | RUN | 0.242780 | 0.589695 | 0.699749 | 0.001607 | 0.001940 | 0.001732 |
| vector_misc.test | RUN | 0.284504 | 0.692705 | 0.879711 | 0.001792 | 0.001523 | 0.001812 |
| vector_move.test | RUN | 0.226176 | 0.569501 | 0.713229 | 0.001607 | 0.001522 | 0.001735 |
| vector_mutate.test | RUN | 0.212227 | 0.507373 | 0.608999 | 0.001759 | 0.001479 | 0.001717 |
| vector_n.test | RUN | 0.255505 | 0.600443 | 0.806754 | 0.001824 | 0.001874 | 0.001677 |
| vector_nest.test | RUN | 0.925450 | 3.715206 | 4.511190 | 0.001784 | 0.001705 | 0.001464 |
| vector_hash.test | RUN | 0.314367 | 0.978478 | 0.933823 | 0.001783 | 0.001812 | 0.001613 |
| vector_tie.test | RUN | 0.221898 | 0.515578 | 0.691149 | 0.001711 | 0.001812 | 0.001686 |
| vector_traits__maybe_variadic.test | RUN | 0.243505 | 0.582756 | 0.680515 | 0.001848 | 0.001700 | 0.001763 |
| vector_traits__no_variadic.test | RUN | 0.264588 | 0.638836 | 0.763142 | 0.001858 | 0.001781 | 0.001520 |
| vector_value_at.test | RUN | 0.222800 | 0.470260 | 0.620561 | 0.001640 | 0.001761 | 0.001729 |
| zip_view.test | RUN | 0.504549 | 1.127705 | 1.616022 | 0.001701 | 0.001836 | 0.001747 |
| zip_view2.test | RUN | 0.364187 | 0.948032 | 1.238204 | 0.001658 | 0.001636 | 0.001585 |
| zip_view_ignore.test | RUN | 0.337079 | 0.820953 | 1.101003 | 0.001502 | 0.001687 | 0.001321 |
| repetitive_view.test | RUN | 0.584069 | 1.514090 | 2.070565 | 0.001804 | 0.001912 | 0.001615 |
| deduce_sequence.test | RUN | 0.284225 | 0.642797 | 0.827585 | 0.001514 | 0.001818 | 0.001581 |
| adapt_adt_named.test | RUN | 0.326605 | 0.909880 | 1.077782 | 0.001552 | 0.001903 | 0.001728 |
| adapt_adt_named_empty.test | RUN | 0.285921 | 0.716955 | 0.853203 | 0.001749 | 0.001897 | 0.001715 |
| adapt_adt.test | RUN | 0.368109 | 1.161117 | 1.261620 | 0.001824 | 0.001782 | 0.001788 |
| adapt_adt_empty.test | RUN | 0.284976 | 0.694884 | 0.848078 | 0.001792 | 0.001860 | 0.001809 |
| adapt_assoc_adt_named.test | RUN | 0.370551 | 0.914617 | 1.148418 | 0.001794 | 0.001694 | 0.001701 |
| adapt_assoc_adt_named_empty.test | RUN | 0.284092 | 0.713020 | 0.842233 | 0.001756 | 0.001873 | 0.001757 |
| adapt_assoc_adt.test | RUN | 0.365511 | 0.938657 | 1.173220 | 0.001888 | 0.001832 | 0.001711 |
| adapt_assoc_adt_empty.test | RUN | 0.287244 | 0.704507 | 0.859498 | 0.001837 | 0.001817 | 0.001599 |
| adapt_assoc_struct_named.test | RUN | 0.369674 | 0.881253 | 1.106325 | 0.001607 | 0.001829 | 0.001600 |
| adapt_assoc_struct_named_empty.test | RUN | 0.285082 | 0.694989 | 0.848604 | 0.001745 | 0.001761 | 0.001632 |
| adapt_assoc_struct.test | RUN | 0.326740 | 0.844870 | 1.025797 | 0.001559 | 0.001882 | 0.001504 |
| adapt_assoc_struct_empty.test | RUN | 0.278611 | 0.689422 | 0.845956 | 0.001852 | 0.001844 | 0.001731 |
| adapt_assoc_tpl_adt.test | RUN | 0.386250 | 0.973532 | 1.210176 | 0.001778 | 0.001817 | 0.001514 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.287218 | 0.705168 | 0.846931 | 0.001666 | 0.001743 | 0.001790 |
| adapt_assoc_tpl_struct.test | RUN | 0.317737 | 0.818387 | 1.023686 | 0.001993 | 0.001762 | 0.001676 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.280410 | 0.706128 | 0.852945 | 0.001694 | 0.001715 | 0.001408 |
| adapt_struct_named.test | RUN | 0.323552 | 0.862025 | 1.000265 | 0.001785 | 0.001687 | 0.001783 |
| adapt_struct_named_empty.test | RUN | 0.293728 | 0.707276 | 0.863759 | 0.001587 | 0.001841 | 0.001467 |
| adapt_struct.test | RUN | 0.356014 | 0.979046 | 1.154326 | 0.001674 | 0.001802 | 0.001652 |
| adapt_struct_empty.test | RUN | 0.288718 | 0.710659 | 0.858883 | 0.001934 | 0.001962 | 0.001591 |
| adapt_tpl_adt.test | RUN | 0.383704 | 1.135755 | 1.211550 | 0.001523 | 0.001679 | 0.001619 |
| adapt_tpl_adt_empty.test | RUN | 0.312246 | 0.710497 | 0.869864 | 0.001745 | 0.001777 | 0.001487 |
| adapt_tpl_struct.test | RUN | 0.300502 | 0.796065 | 0.934399 | 0.001594 | 0.001703 | 0.001781 |
| adapt_tpl_struct_empty.test | RUN | 0.284206 | 0.751157 | 0.875447 | 0.001562 | 0.001891 | 0.001733 |
| adt_attribute_proxy.test | RUN | 0.314913 | 0.717962 | 0.937661 | 0.001720 | 0.001788 | 0.001584 |
| define_struct.test | RUN | 0.374426 | 0.981148 | 1.214521 | 0.001907 | 0.001627 | 0.001789 |
| define_struct_empty.test | RUN | 0.335287 | 0.816331 | 0.982895 | 0.001595 | 0.001808 | 0.001873 |
| define_struct_move.test | RUN | 0.213402 | 0.445881 | 0.611683 | 0.001614 | 0.001613 | 0.001622 |
| define_struct_inline.test | RUN | 0.404865 | 1.166409 | 1.386513 | 0.001757 | 0.001568 | 0.001794 |
| define_struct_inline_empty.test | RUN | 0.339693 | 0.823025 | 0.987511 | 0.001584 | 0.001756 | 0.001575 |
| define_struct_inline_move.test | RUN | 0.219743 | 0.444316 | 0.617538 | 0.001603 | 0.001651 | 0.001806 |
| define_assoc_struct.test | RUN | 0.383789 | 0.912648 | 1.170264 | 0.001839 | 0.001866 | 0.001736 |
| define_assoc_struct_empty.test | RUN | 0.336843 | 0.796993 | 1.006049 | 0.001930 | 0.001865 | 0.001752 |
| define_assoc_struct_move.test | RUN | 0.218330 | 0.452000 | 0.642439 | 0.001745 | 0.001972 | 0.001753 |
| define_tpl_struct.test | RUN | 0.370433 | 0.944038 | 1.148150 | 0.001833 | 0.001634 | 0.001464 |
| define_tpl_struct_empty.test | RUN | 0.338891 | 0.794030 | 1.019205 | 0.001852 | 0.001790 | 0.001666 |
| define_tpl_struct_move.test | RUN | 0.220910 | 0.475081 | 0.620462 | 0.001620 | 0.001680 | 0.001426 |
| define_tpl_struct_inline.test | RUN | 0.395303 | 1.109304 | 1.268972 | 0.001726 | 0.001760 | 0.001507 |
| define_tpl_struct_inline_empty.test | RUN | 0.337019 | 0.809008 | 1.001787 | 0.001768 | 0.001682 | 0.001900 |
| define_tpl_struct_inline_move.test | RUN | 0.220490 | 0.498606 | 0.628646 | 0.001790 | 0.001636 | 0.001644 |
| define_assoc_tpl_struct.test | RUN | 0.385254 | 0.970013 | 1.192005 | 0.001774 | 0.001840 | 0.001835 |
| define_assoc_tpl_struct_empty.test | RUN | 0.345940 | 0.798021 | 0.992366 | 0.001602 | 0.001740 | 0.001683 |
| define_assoc_tpl_struct_move.test | RUN | 0.220533 | 0.470018 | 0.609511 | 0.001570 | 0.001709 | 0.001631 |
| std_tuple.test | RUN | 0.232862 | 0.506841 | 0.690926 | 0.001720 | 0.001648 | 0.001664 |
| std_tuple_iterator.test | RUN | 0.228695 | 0.545678 | 0.680852 | 0.001608 | 0.001685 | 0.001657 |
| ref_vector.test | RUN | 0.261080 | 0.549927 | 0.733268 | 0.001794 | 0.001662 | 0.001697 |
| flatten_view.test | RUN | 0.271797 | 0.683075 | 0.846455 | 0.001823 | 0.001749 | 0.001572 |
| github-159.test | COMPILE | 0.121686 | 0.256016 | 0.306968 | — | — | — |
| github-176.test | RUN | 0.248531 | 0.593564 | 0.740920 | 0.001724 | 0.001798 | 0.001772 |
| size.test | COMPILE | 0.211861 | 0.434938 | 0.567696 | — | — | — |
| fused.test | RUN | 0.300931 | 0.657981 | 0.872305 | 0.001582 | 0.001847 | 0.001780 |
| fused_function_object.test | RUN | 0.227285 | 0.495929 | 0.631253 | 0.001715 | 0.001678 | 0.001744 |
| fused_procedure.test | RUN | 0.285548 | 0.627684 | 0.811558 | 0.001695 | 0.001830 | 0.001486 |
| unfused.test | RUN | 0.236718 | 0.504122 | 0.655465 | 0.001737 | 0.001744 | 0.001529 |
| unfused_typed.test | RUN | 0.235645 | 0.549676 | 0.695708 | 0.001814 | 0.001660 | 0.001707 |
| make_fused.test | RUN | 0.302614 | 0.637487 | 0.855762 | 0.001509 | 0.001664 | 0.001513 |
| make_fused_function_object.test | RUN | 0.226808 | 0.481520 | 0.648688 | 0.001844 | 0.001632 | 0.001600 |
| make_fused_procedure.test | RUN | 0.301325 | 0.634123 | 0.824817 | 0.001822 | 0.001701 | 0.001774 |
| make_unfused.test | RUN | 0.235721 | 0.492978 | 0.656008 | 0.001646 | 0.001726 | 0.001774 |
| invoke.test | RUN | 0.594318 | 1.767107 | 2.250923 | 0.001540 | 0.001536 | 0.001796 |
| invoke_function_object.test | RUN | 0.275201 | 0.638697 | 0.843740 | 0.001918 | 0.001743 | 0.001674 |
| invoke_procedure.test | RUN | 0.463596 | 1.343697 | 1.723311 | 0.001745 | 0.001561 | 0.001658 |
| swap.test | RUN | 0.351337 | 0.845543 | 1.035286 | 0.001810 | 0.001564 | 0.001832 |
| is_sequence.test | COMPILE | 0.061450 | 0.122032 | 0.137603 | — | — | — |
| is_view.test | COMPILE | 0.058154 | 0.116197 | 0.139385 | — | — | — |
| pair_deque.test | COMPILE | 0.126582 | 0.274769 | 0.308402 | — | — | — |
| pair_list.test | COMPILE | 0.124086 | 0.257827 | 0.315257 | — | — | — |
| pair_map.test | COMPILE | 0.127088 | 0.262525 | 0.330762 | — | — | — |
| pair_set.test | COMPILE | 0.129688 | 0.265584 | 0.315580 | — | — | — |
| pair_vector.test | COMPILE | 0.128435 | 0.263273 | 0.323464 | — | — | — |
| pair_nest.test | COMPILE | 0.052257 | 0.106923 | 0.137730 | — | — | — |
| index_sequence.test | COMPILE | 0.044464 | 0.078939 | 0.092400 | — | — | — |
| and.test | COMPILE | 0.035421 | 0.063743 | 0.066009 | — | — | — |
| tag_of.test | COMPILE | 0.057404 | 0.111976 | 0.128541 | — | — | — |
| unused.test | COMPILE | 0.029729 | 0.075483 | 0.076538 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.056039 | 0.117521 | 0.122054 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.046636 | 0.106148 | 0.095582 | — | — | — |
| quick.test | RUN | 0.167063 | 0.359236 | 0.405940 | 0.000580 | 0.001266 | 0.001211 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.002× | 1.002×, 1.003× | 1.004× |
| clang | 1.000× | 0.997×, 1.004× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001041s | 0.001043s | 0.001043s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001018s | 0.001029s | 0.001002s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001028s | 0.001040s | 0.001005s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001029s | 0.001017s | 0.001007s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001026s | 0.001029s | 0.001025s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001016s | 0.001015s | 0.001012s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001030s | 0.000998s | 0.001041s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001018s | 0.001024s | 0.000996s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001014s | 0.001033s | 0.001007s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001002s | 0.001047s | 0.001008s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001026s | 0.001037s | 0.001010s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001014s | 0.001022s | 0.001010s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001031s | 0.001019s | 0.001002s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001024s | 0.001025s | 0.000985s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001057s | 0.001050s | 0.001049s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001037s | 0.001025s | 0.001004s | byte equality observed |
| adapt_struct.test | 10 | 0.001066s | 0.001034s | 0.001050s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001051s | 0.001027s | 0.001021s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001050s | 0.001034s | 0.001026s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001050s | 0.001055s | 0.001012s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001071s | 0.001049s | 0.001044s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001036s | 0.001070s | 0.001033s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001025s | 0.001048s | 0.001034s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001041s | 0.001016s | 0.001081s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001027s | 0.001016s | 0.001012s | byte equality observed |
| all.test | 10 | 0.001032s | 0.001007s | 0.001009s | byte equality observed |
| any.test | 10 | 0.001023s | 0.000978s | 0.001012s | byte equality observed |
| array.test | 10 | 0.001031s | 0.001014s | 0.001033s | byte equality observed |
| as_deque.test | 10 | 0.001027s | 0.001007s | 0.001017s | byte equality observed |
| as_list.test | 10 | 0.001050s | 0.001054s | 0.001039s | byte equality observed |
| as_map.test | 10 | 0.001038s | 0.001064s | 0.001017s | byte equality observed |
| as_map_assoc.test | 10 | 0.001041s | 0.001019s | 0.001031s | byte equality observed |
| as_set.test | 10 | 0.001061s | 0.001042s | 0.001046s | byte equality observed |
| as_vector.test | 10 | 0.001037s | 0.001035s | 0.001043s | byte equality observed |
| back_extended_deque.test | 10 | 0.001035s | 0.001013s | 0.000999s | byte equality observed |
| boost_array.test | 10 | 0.001012s | 0.001017s | 0.000986s | byte equality observed |
| boost_tuple.test | 10 | 0.001035s | 0.001026s | 0.001030s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001016s | 0.001016s | 0.000997s | byte equality observed |
| clear.test | 10 | 0.001041s | 0.001011s | 0.001014s | byte equality observed |
| cons.test | 10 | 0.001047s | 0.001052s | 0.001064s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001013s | 0.001025s | 0.001009s | byte equality observed |
| convert_deque.test | 10 | 0.001021s | 0.001035s | 0.000998s | byte equality observed |
| convert_list.test | 10 | 0.001022s | 0.001023s | 0.000991s | byte equality observed |
| convert_std_pair.test | 10 | 0.001005s | 0.000998s | 0.001007s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001018s | 0.001007s | 0.001046s | byte equality observed |
| convert_vector.test | 10 | 0.001004s | 0.001018s | 0.001007s | byte equality observed |
| copy.test | 10 | 0.001019s | 0.001012s | 0.001006s | byte equality observed |
| count.test | 10 | 0.001028s | 0.001004s | 0.001008s | byte equality observed |
| count_if.test | 10 | 0.001008s | 0.001003s | 0.001006s | byte equality observed |
| deduce_sequence.test | 10 | 0.001014s | 0.001003s | 0.001018s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001034s | 0.001010s | 0.001035s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001034s | 0.001022s | 0.001037s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000999s | 0.000990s | 0.001020s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001038s | 0.001029s | 0.001019s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001033s | 0.001036s | 0.001021s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000993s | 0.001008s | 0.001012s | byte equality observed |
| define_struct.test | 10 | 0.001032s | 0.001030s | 0.001041s | byte equality observed |
| define_struct_empty.test | 10 | 0.001020s | 0.001055s | 0.001005s | byte equality observed |
| define_struct_inline.test | 10 | 0.001029s | 0.001046s | 0.001050s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001029s | 0.001043s | 0.001025s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001037s | 0.000989s | 0.001008s | byte equality observed |
| define_struct_move.test | 10 | 0.000990s | 0.001011s | 0.001002s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001013s | 0.001037s | 0.001051s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001005s | 0.001052s | 0.001002s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001022s | 0.001029s | 0.001022s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001020s | 0.001020s | 0.000990s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.001001s | 0.000988s | 0.000986s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.001033s | 0.000978s | 0.001042s | byte equality observed |
| deque_comparison.test | 10 | 0.001017s | 0.001033s | 0.001017s | byte equality observed |
| deque_construction.test | 10 | 0.001019s | 0.001017s | 0.001024s | byte equality observed |
| deque_copy.test | 10 | 0.001010s | 0.001018s | 0.001042s | byte equality observed |
| deque_hash.test | 10 | 0.001003s | 0.001045s | 0.001006s | byte equality observed |
| deque_iterator.test | 10 | 0.001013s | 0.000999s | 0.001024s | byte equality observed |
| deque_make.test | 10 | 0.001013s | 0.001048s | 0.001037s | byte equality observed |
| deque_misc.test | 10 | 0.001008s | 0.001028s | 0.001000s | byte equality observed |
| deque_move.test | 10 | 0.001008s | 0.001022s | 0.001019s | byte equality observed |
| deque_mutate.test | 10 | 0.001011s | 0.001005s | 0.000996s | byte equality observed |
| deque_nest.test | 10 | 0.001028s | 0.001042s | 0.000974s | byte equality observed |
| deque_tie.test | 10 | 0.000985s | 0.001028s | 0.000985s | byte equality observed |
| deque_value_at.test | 10 | 0.000998s | 0.001012s | 0.001035s | byte equality observed |
| erase.test | 10 | 0.001030s | 0.001048s | 0.001025s | byte equality observed |
| erase_key.test | 10 | 0.001049s | 0.001017s | 0.001063s | byte equality observed |
| filter.test | 10 | 0.001024s | 0.001024s | 0.001017s | byte equality observed |
| filter_if.test | 10 | 0.001056s | 0.001044s | 0.001028s | byte equality observed |
| filter_view.test | 10 | 0.001036s | 0.001037s | 0.001018s | byte equality observed |
| find.test | 10 | 0.001047s | 0.001038s | 0.001008s | byte equality observed |
| find_if.test | 10 | 0.001035s | 0.001026s | 0.001012s | byte equality observed |
| flatten.test | 10 | 0.001028s | 0.001004s | 0.001008s | byte equality observed |
| flatten_view.test | 10 | 0.001009s | 0.001002s | 0.001035s | byte equality observed |
| fold.test | 10 | 0.001027s | 0.001030s | 0.001069s | byte equality observed |
| for_each.test | 10 | 0.001044s | 0.001033s | 0.001047s | byte equality observed |
| front_extended_deque.test | 10 | 0.001020s | 0.001012s | 0.000992s | byte equality observed |
| fused.test | 10 | 0.001012s | 0.001001s | 0.000990s | byte equality observed |
| fused_function_object.test | 10 | 0.000993s | 0.001031s | 0.000996s | byte equality observed |
| fused_procedure.test | 10 | 0.001023s | 0.001014s | 0.000990s | byte equality observed |
| github-176.test | 10 | 0.001010s | 0.000983s | 0.000995s | byte equality observed |
| hash.test | 10 | 0.001014s | 0.001013s | 0.001005s | byte equality observed |
| identity_view.test | 10 | 0.001038s | 0.001031s | 0.001034s | byte equality observed |
| insert.test | 10 | 0.001072s | 0.001017s | 0.001056s | byte equality observed |
| insert_range.test | 10 | 0.001042s | 0.001053s | 0.001047s | byte equality observed |
| invoke.test | 10 | 0.001010s | 0.001004s | 0.001002s | byte equality observed |
| invoke_function_object.test | 10 | 0.001004s | 0.000983s | 0.001024s | byte equality observed |
| invoke_procedure.test | 10 | 0.000999s | 0.000995s | 0.000993s | byte equality observed |
| io.test | 10 | 0.001151s | 0.001125s | 0.001162s | byte equality observed |
| iter_fold.test | 10 | 0.001028s | 0.001072s | 0.001036s | byte equality observed |
| iterator_range.test | 10 | 0.001050s | 0.001060s | 0.001051s | byte equality observed |
| join.test | 10 | 0.001004s | 0.001005s | 0.000998s | byte equality observed |
| joint_view.test | 10 | 0.001070s | 0.001063s | 0.001079s | byte equality observed |
| list_comparison.test | 10 | 0.000994s | 0.000997s | 0.000989s | byte equality observed |
| list_construction.test | 10 | 0.001002s | 0.001020s | 0.001001s | byte equality observed |
| list_copy.test | 10 | 0.001007s | 0.001018s | 0.001013s | byte equality observed |
| list_hash.test | 10 | 0.001018s | 0.001009s | 0.001022s | byte equality observed |
| list_iterator.test | 10 | 0.001023s | 0.001012s | 0.001027s | byte equality observed |
| list_make.test | 10 | 0.001049s | 0.001023s | 0.000993s | byte equality observed |
| list_misc.test | 10 | 0.001007s | 0.001019s | 0.001027s | byte equality observed |
| list_mutate.test | 10 | 0.001006s | 0.001023s | 0.001009s | byte equality observed |
| list_nest.test | 10 | 0.000996s | 0.001004s | 0.001035s | byte equality observed |
| list_tie.test | 10 | 0.001022s | 0.001010s | 0.001008s | byte equality observed |
| list_value_at.test | 10 | 0.001019s | 0.000980s | 0.001004s | byte equality observed |
| make_fused.test | 10 | 0.001003s | 0.001016s | 0.001006s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001029s | 0.001014s | 0.001066s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000987s | 0.000972s | 0.001042s | byte equality observed |
| make_list.test | 10 | 0.001024s | 0.001002s | 0.001005s | byte equality observed |
| make_unfused.test | 10 | 0.001001s | 0.001005s | 0.000998s | byte equality observed |
| make_vector.test | 10 | 0.001016s | 0.001025s | 0.001008s | byte equality observed |
| map.test | 10 | 0.001035s | 0.001031s | 0.001032s | byte equality observed |
| map_comparison.test | 10 | 0.001018s | 0.001023s | 0.000992s | byte equality observed |
| map_construction.test | 10 | 0.001004s | 0.001015s | 0.000978s | byte equality observed |
| map_copy.test | 10 | 0.001015s | 0.001012s | 0.000993s | byte equality observed |
| map_misc.test | 10 | 0.001026s | 0.001000s | 0.000998s | byte equality observed |
| map_move.test | 10 | 0.001032s | 0.001043s | 0.001029s | byte equality observed |
| map_mutate.test | 10 | 0.001009s | 0.001000s | 0.001030s | byte equality observed |
| map_tie.test | 10 | 0.001001s | 0.001009s | 0.001009s | byte equality observed |
| move.test | 10 | 0.000997s | 0.000991s | 0.001022s | byte equality observed |
| nil.test | 10 | 0.001012s | 0.001022s | 0.000997s | byte equality observed |
| none.test | 10 | 0.001030s | 0.001009s | 0.001033s | byte equality observed |
| nview.test | 10 | 0.001016s | 0.000979s | 0.001029s | byte equality observed |
| pop_back.test | 10 | 0.001064s | 0.001037s | 0.001064s | byte equality observed |
| pop_front.test | 10 | 0.001058s | 0.001010s | 0.001030s | byte equality observed |
| push_back.test | 10 | 0.001047s | 0.001038s | 0.001071s | byte equality observed |
| push_front.test | 10 | 0.001050s | 0.001028s | 0.001079s | byte equality observed |
| quick.test | 10 | 0.000460s | 0.000982s | 0.000974s | byte equality observed |
| ref_vector.test | 10 | 0.001046s | 0.001035s | 0.001025s | byte equality observed |
| remove.test | 10 | 0.001051s | 0.001049s | 0.001046s | byte equality observed |
| remove_if.test | 10 | 0.001037s | 0.001020s | 0.001027s | byte equality observed |
| repetitive_view.test | 10 | 0.000990s | 0.001012s | 0.001021s | byte equality observed |
| replace.test | 10 | 0.001049s | 0.001048s | 0.001045s | byte equality observed |
| replace_if.test | 10 | 0.001033s | 0.001069s | 0.001045s | byte equality observed |
| reverse.test | 10 | 0.001050s | 0.001046s | 0.001045s | byte equality observed |
| reverse_fold.test | 10 | 0.001043s | 0.001065s | 0.001031s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001029s | 0.001013s | 0.001055s | byte equality observed |
| reverse_view.test | 10 | 0.001041s | 0.001041s | 0.001023s | byte equality observed |
| segmented_find.test | 10 | 0.000999s | 0.001006s | 0.001010s | byte equality observed |
| segmented_find_if.test | 10 | 0.000995s | 0.001010s | 0.000998s | byte equality observed |
| segmented_fold.test | 10 | 0.001035s | 0.001066s | 0.001023s | byte equality observed |
| segmented_for_each.test | 10 | 0.001042s | 0.001043s | 0.001010s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001023s | 0.001095s | 0.001058s | byte equality observed |
| set.test | 10 | 0.001033s | 0.001048s | 0.001027s | byte equality observed |
| single_view.test | 10 | 0.001033s | 0.001025s | 0.001039s | byte equality observed |
| std_array.test | 10 | 0.001008s | 0.001000s | 0.000989s | byte equality observed |
| std_pair.test | 10 | 0.001035s | 0.001037s | 0.001023s | byte equality observed |
| std_tuple.test | 10 | 0.001020s | 0.000990s | 0.000991s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.001009s | 0.001010s | 0.001051s | byte equality observed |
| swap.test | 10 | 0.001009s | 0.001014s | 0.001000s | byte equality observed |
| transform.test | 10 | 0.001037s | 0.001037s | 0.001022s | byte equality observed |
| transform_view.test | 10 | 0.001047s | 0.001044s | 0.001040s | byte equality observed |
| tuple_comparison.test | 10 | 0.000991s | 0.001003s | 0.001008s | byte equality observed |
| tuple_construction.test | 10 | 0.000998s | 0.000996s | 0.001019s | byte equality observed |
| tuple_conversion.test | 10 | 0.000999s | 0.001005s | 0.001010s | byte equality observed |
| tuple_copy.test | 10 | 0.001019s | 0.001006s | 0.001061s | byte equality observed |
| tuple_element.test | 10 | 0.001022s | 0.001027s | 0.000996s | byte equality observed |
| tuple_hash.test | 10 | 0.000999s | 0.001003s | 0.000992s | byte equality observed |
| tuple_make.test | 10 | 0.001010s | 0.001037s | 0.000995s | byte equality observed |
| tuple_misc.test | 10 | 0.001007s | 0.001008s | 0.001021s | byte equality observed |
| tuple_mutate.test | 10 | 0.000999s | 0.001000s | 0.001025s | byte equality observed |
| tuple_nest.test | 10 | 0.001021s | 0.001025s | 0.000993s | byte equality observed |
| tuple_tie.test | 10 | 0.001001s | 0.000989s | 0.000999s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001009s | 0.001003s | 0.001055s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001009s | 0.001013s | 0.001033s | byte equality observed |
| unfused.test | 10 | 0.001024s | 0.001001s | 0.000995s | byte equality observed |
| unfused_typed.test | 10 | 0.000993s | 0.001026s | 0.001000s | byte equality observed |
| vector_comparison.test | 10 | 0.001039s | 0.001032s | 0.001014s | byte equality observed |
| vector_construction.test | 10 | 0.000982s | 0.001037s | 0.001000s | byte equality observed |
| vector_conversion.test | 10 | 0.000987s | 0.001058s | 0.000983s | byte equality observed |
| vector_copy.test | 10 | 0.001002s | 0.001057s | 0.001021s | byte equality observed |
| vector_hash.test | 10 | 0.001027s | 0.001015s | 0.000996s | byte equality observed |
| vector_iterator.test | 10 | 0.000992s | 0.001017s | 0.000984s | byte equality observed |
| vector_make.test | 10 | 0.000986s | 0.001016s | 0.001019s | byte equality observed |
| vector_misc.test | 10 | 0.001000s | 0.001012s | 0.001010s | byte equality observed |
| vector_move.test | 10 | 0.001012s | 0.001023s | 0.001020s | byte equality observed |
| vector_mutate.test | 10 | 0.001002s | 0.001017s | 0.001004s | byte equality observed |
| vector_n.test | 10 | 0.001051s | 0.001034s | 0.001028s | byte equality observed |
| vector_nest.test | 10 | 0.001003s | 0.000998s | 0.001000s | byte equality observed |
| vector_tie.test | 10 | 0.000998s | 0.000978s | 0.000995s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001044s | 0.001016s | 0.001016s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001001s | 0.000970s | 0.001049s | byte equality observed |
| vector_value_at.test | 10 | 0.001006s | 0.000997s | 0.001014s | byte equality observed |
| zip.test | 10 | 0.001007s | 0.001002s | 0.001036s | byte equality observed |
| zip2.test | 10 | 0.001028s | 0.001056s | 0.001018s | byte equality observed |
| zip_ignore.test | 10 | 0.001025s | 0.001030s | 0.001005s | byte equality observed |
| zip_view.test | 10 | 0.001037s | 0.000995s | 0.001004s | byte equality observed |
| zip_view2.test | 10 | 0.001011s | 0.001013s | 0.000998s | byte equality observed |
| zip_view_ignore.test | 10 | 0.001005s | 0.000997s | 0.000989s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
