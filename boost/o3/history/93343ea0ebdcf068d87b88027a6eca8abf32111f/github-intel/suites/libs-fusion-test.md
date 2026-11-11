# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.483938 | 0.958142 | 1.277580 | 0.001514 | 0.001536 | 0.001585 |
| any.test | RUN | 0.472722 | 0.896859 | 1.203468 | 0.001666 | 0.001641 | 0.001657 |
| clear.test | RUN | 0.345433 | 0.818224 | 0.963921 | 0.001565 | 0.001588 | 0.001708 |
| copy.test | RUN | 0.299529 | 0.625777 | 0.794059 | 0.001602 | 0.001852 | 0.001487 |
| count.test | RUN | 0.326728 | 0.684448 | 0.876210 | 0.001654 | 0.001736 | 0.001703 |
| count_if.test | RUN | 0.307646 | 0.624258 | 0.789601 | 0.001484 | 0.001616 | 0.001528 |
| erase.test | RUN | 0.420808 | 1.120490 | 1.294916 | 0.001446 | 0.001452 | 0.001490 |
| erase_key.test | RUN | 0.410407 | 1.100802 | 1.218057 | 0.001630 | 0.001488 | 0.001681 |
| filter.test | RUN | 0.366377 | 0.949153 | 1.107074 | 0.001756 | 0.001685 | 0.001704 |
| filter_if.test | RUN | 0.420948 | 1.119125 | 1.305914 | 0.001623 | 0.001666 | 0.001736 |
| find.test | RUN | 0.359968 | 0.756687 | 1.009369 | 0.001600 | 0.001637 | 0.001647 |
| find_if.test | RUN | 0.341006 | 0.701046 | 0.919716 | 0.001676 | 0.001528 | 0.001706 |
| fold.test | RUN | 0.379344 | 0.860337 | 1.120212 | 0.001712 | 0.001712 | 0.001676 |
| for_each.test | RUN | 0.349017 | 0.784725 | 0.976476 | 0.001520 | 0.001801 | 0.001722 |
| insert.test | RUN | 0.498073 | 1.386533 | 1.561988 | 0.001709 | 0.001660 | 0.001574 |
| insert_range.test | RUN | 0.497734 | 1.396775 | 1.580117 | 0.001537 | 0.001643 | 0.001548 |
| iter_fold.test | RUN | 0.579700 | 1.339961 | 1.871376 | 0.001794 | 0.001794 | 0.001698 |
| move.test | RUN | 0.300920 | 0.631686 | 0.818611 | 0.001681 | 0.001542 | 0.001509 |
| none.test | RUN | 0.465928 | 0.905017 | 1.220387 | 0.001637 | 0.001706 | 0.001683 |
| pop_back.test | RUN | 0.465790 | 1.193379 | 1.393208 | 0.001640 | 0.001825 | 0.001734 |
| pop_front.test | RUN | 0.380850 | 0.977405 | 1.137519 | 0.001950 | 0.001590 | 0.001654 |
| push_back.test | RUN | 0.489506 | 1.397537 | 1.525152 | 0.001732 | 0.001809 | 0.001742 |
| push_front.test | RUN | 0.429080 | 1.147111 | 1.309877 | 0.001797 | 0.001782 | 0.001758 |
| remove.test | RUN | 0.468650 | 1.326017 | 1.539848 | 0.001484 | 0.001687 | 0.001660 |
| remove_if.test | RUN | 0.425416 | 1.091310 | 1.322328 | 0.001754 | 0.001778 | 0.001712 |
| replace.test | RUN | 0.415909 | 1.087732 | 1.231273 | 0.001630 | 0.001767 | 0.001676 |
| replace_if.test | RUN | 0.404769 | 1.053777 | 1.213853 | 0.002098 | 0.001783 | 0.001707 |
| reverse_fold.test | RUN | 0.612427 | 1.526085 | 2.037650 | 0.001645 | 0.001685 | 0.001593 |
| reverse_iter_fold.test | RUN | 0.641136 | 1.562839 | 2.132109 | 0.001721 | 0.001718 | 0.001658 |
| reverse.test | RUN | 0.395716 | 1.110730 | 1.224304 | 0.001598 | 0.001746 | 0.001726 |
| segmented_for_each.test | RUN | 0.676414 | 2.056365 | 2.500758 | 0.001734 | 0.001486 | 0.001694 |
| segmented_find.test | RUN | 0.429548 | 1.123850 | 1.383823 | 0.001369 | 0.001628 | 0.001431 |
| segmented_find_if.test | RUN | 0.431281 | 1.114430 | 1.393969 | 0.001580 | 0.001497 | 0.001620 |
| segmented_fold.test | RUN | 0.527019 | 1.465155 | 1.797852 | 0.001717 | 0.001877 | 0.001689 |
| transform.test | RUN | 0.420348 | 1.164543 | 1.299048 | 0.001741 | 0.001804 | 0.001644 |
| join.test | RUN | 0.328986 | 0.714568 | 0.913213 | 0.001650 | 0.001775 | 0.001694 |
| zip.test | RUN | 0.562700 | 1.326194 | 1.729769 | 0.001743 | 0.001700 | 0.001684 |
| zip2.test | RUN | 0.539346 | 1.300401 | 1.643959 | 0.001473 | 0.001529 | 0.001533 |
| zip_ignore.test | RUN | 0.407200 | 0.870915 | 1.102315 | 0.001758 | 0.001776 | 0.001532 |
| flatten.test | RUN | 0.383831 | 1.001841 | 1.125081 | 0.001497 | 0.001778 | 0.001634 |
| ticket-5490.test | COMPILE | 0.214647 | 0.416232 | 0.503919 | — | — | — |
| as_deque.test | RUN | 0.382625 | 0.924411 | 1.200723 | 0.001599 | 0.001744 | 0.001641 |
| as_list.test | RUN | 0.433940 | 1.140137 | 1.282502 | 0.001610 | 0.001587 | 0.001666 |
| as_map.test | RUN | 0.426252 | 1.120121 | 1.285399 | 0.001500 | 0.001622 | 0.001519 |
| as_map_assoc.test | RUN | 0.423009 | 1.053809 | 1.206668 | 0.001509 | 0.001760 | 0.001728 |
| as_set.test | RUN | 0.472812 | 1.293153 | 1.495148 | 0.001580 | 0.001552 | 0.001454 |
| as_vector.test | RUN | 0.442401 | 1.163876 | 1.313296 | 0.001714 | 0.001616 | 0.001724 |
| boost_tuple.test | RUN | 0.428519 | 1.147797 | 1.282034 | 0.001548 | 0.001561 | 0.001430 |
| boost_tuple_iterator.test | RUN | 0.301321 | 0.684567 | 0.789184 | 0.001520 | 0.001596 | 0.001686 |
| cons.test | RUN | 0.533359 | 1.293724 | 1.566468 | 0.001430 | 0.001695 | 0.001712 |
| convert_boost_tuple.test | RUN | 0.353393 | 0.989188 | 1.017792 | 0.001646 | 0.001740 | 0.001503 |
| convert_deque.test | RUN | 0.354860 | 0.986207 | 1.015527 | 0.001512 | 0.001667 | 0.001517 |
| convert_list.test | RUN | 0.351638 | 0.991422 | 1.020815 | 0.001581 | 0.001435 | 0.001415 |
| convert_std_pair.test | RUN | 0.372127 | 1.007568 | 1.040881 | 0.001813 | 0.001742 | 0.001682 |
| convert_std_tuple.test | RUN | 0.357282 | 0.984049 | 1.002053 | 0.001449 | 0.001533 | 0.001567 |
| convert_vector.test | RUN | 0.352167 | 0.965722 | 1.013012 | 0.001697 | 0.001417 | 0.001496 |
| filter_view.test | RUN | 0.410754 | 1.020618 | 1.212679 | 0.001645 | 0.001679 | 0.001666 |
| hash.test | RUN | 0.418068 | 0.933318 | 1.172910 | 0.001768 | 0.001772 | 0.001712 |
| io.test | RUN | 0.438233 | 1.302259 | 1.399093 | 0.001732 | 0.001954 | 0.001865 |
| iterator_range.test | RUN | 0.385225 | 0.966458 | 1.149706 | 0.001552 | 0.001772 | 0.001343 |
| joint_view.test | RUN | 0.520983 | 1.644067 | 1.730390 | 0.001651 | 0.001682 | 0.001679 |
| list_comparison.test | RUN | 0.315031 | 0.703941 | 0.838784 | 0.001642 | 0.001437 | 0.001476 |
| list_construction.test | RUN | 0.302092 | 0.648606 | 0.817900 | 0.001594 | 0.001408 | 0.001403 |
| list_copy.test | RUN | 0.467405 | 1.202819 | 1.439853 | 0.001676 | 0.001534 | 0.001714 |
| list_iterator.test | RUN | 0.299845 | 0.684522 | 0.815560 | 0.001578 | 0.001461 | 0.001539 |
| list_hash.test | RUN | 0.409939 | 1.338182 | 1.179345 | 0.001551 | 0.001584 | 0.001672 |
| list_make.test | RUN | 0.327026 | 0.739702 | 0.893599 | 0.001731 | 0.001676 | 0.001687 |
| list_misc.test | RUN | 0.414927 | 0.873665 | 1.087497 | 0.001607 | 0.001705 | 0.001658 |
| list_mutate.test | RUN | 0.293541 | 0.592673 | 0.741742 | 0.001647 | 0.001733 | 0.001609 |
| list_nest.test | RUN | 0.580442 | 1.636121 | 1.991165 | 0.001639 | 0.001611 | 0.001643 |
| list_tie.test | RUN | 0.301931 | 0.650766 | 0.827832 | 0.001684 | 0.001571 | 0.001679 |
| list_value_at.test | RUN | 0.302484 | 0.642016 | 0.772564 | 0.001592 | 0.001783 | 0.001616 |
| deque_comparison.test | RUN | 0.308668 | 0.687970 | 0.836443 | 0.001613 | 0.001586 | 0.001636 |
| deque_construction.test | RUN | 0.304703 | 0.656525 | 0.860475 | 0.001671 | 0.001702 | 0.001713 |
| deque_copy.test | RUN | 0.432883 | 1.135552 | 1.326399 | 0.001435 | 0.001660 | 0.001659 |
| deque_iterator.test | RUN | 0.304044 | 0.723931 | 0.827615 | 0.001663 | 0.001511 | 0.001701 |
| deque_hash.test | RUN | 0.418999 | 1.255736 | 1.185522 | 0.001687 | 0.001691 | 0.001743 |
| deque_is_constructible.test | COMPILE | 0.172359 | 0.310977 | 0.376945 | — | — | — |
| deque_make.test | RUN | 0.309340 | 0.713023 | 0.870784 | 0.001630 | 0.001694 | 0.001709 |
| deque_misc.test | RUN | 0.405424 | 0.849815 | 1.056757 | 0.001576 | 0.001663 | 0.001683 |
| deque_move.test | RUN | 0.320338 | 0.739266 | 0.837166 | 0.001486 | 0.001624 | 0.001613 |
| deque_mutate.test | RUN | 0.293265 | 0.581560 | 0.726009 | 0.001219 | 0.001687 | 0.001596 |
| deque_nest.test | RUN | 0.851419 | 2.594320 | 3.129325 | 0.001635 | 0.001651 | 0.001677 |
| deque_tie.test | RUN | 0.302815 | 0.632872 | 0.772857 | 0.001806 | 0.001766 | 0.001761 |
| deque_value_at.test | RUN | 0.302512 | 0.619862 | 0.751545 | 0.001656 | 0.001009 | 0.001464 |
| front_extended_deque.test | RUN | 0.422919 | 0.907636 | 1.131097 | 0.001576 | 0.001694 | 0.001619 |
| back_extended_deque.test | RUN | 0.431912 | 0.907776 | 1.115835 | 0.001573 | 0.001699 | 0.001655 |
| make_list.test | RUN | 0.332605 | 0.743205 | 0.879544 | 0.001683 | 0.001566 | 0.001564 |
| make_vector.test | RUN | 0.326556 | 0.738413 | 0.892722 | 0.001599 | 0.001690 | 0.001137 |
| map.test | RUN | 0.410441 | 1.126877 | 1.246294 | 0.001768 | 0.001667 | 0.001596 |
| map_comparison.test | RUN | 0.325117 | 0.724021 | 0.858655 | 0.001413 | 0.001708 | 0.001667 |
| map_construction.test | RUN | 0.323567 | 0.677483 | 0.839246 | 0.001643 | 0.001751 | 0.001630 |
| map_copy.test | RUN | 0.382206 | 0.874517 | 1.048627 | 0.001830 | 0.001669 | 0.001670 |
| map_misc.test | RUN | 0.410241 | 0.879368 | 1.046449 | 0.001749 | 0.001486 | 0.001358 |
| map_move.test | RUN | 0.323167 | 0.752014 | 0.841075 | 0.001442 | 0.001764 | 0.001453 |
| map_mutate.test | RUN | 0.290863 | 0.579741 | 0.731852 | 0.001738 | 0.001713 | 0.001392 |
| map_tie.test | RUN | 0.298915 | 0.592195 | 0.754025 | 0.001639 | 0.001699 | 0.001642 |
| nil.test | RUN | 0.209554 | 0.419890 | 0.518461 | 0.001711 | 0.001744 | 0.001649 |
| nview.test | RUN | 0.401869 | 0.878003 | 1.087919 | 0.001626 | 0.001690 | 0.001678 |
| reverse_view.test | RUN | 0.484533 | 1.393672 | 1.584887 | 0.001713 | 0.001744 | 0.001634 |
| segmented_iterator_range.test | RUN | 0.581225 | 1.681003 | 1.991406 | 0.001704 | 0.001611 | 0.001435 |
| set.test | RUN | 0.384203 | 0.962584 | 1.105981 | 0.001581 | 0.001794 | 0.001704 |
| single_view.test | RUN | 0.344001 | 0.879217 | 1.012490 | 0.001726 | 0.001886 | 0.001481 |
| std_pair.test | RUN | 0.440661 | 1.115544 | 1.289203 | 0.001596 | 0.001808 | 0.001647 |
| boost_array.test | RUN | 0.383993 | 0.778747 | 0.985537 | 0.001568 | 0.001705 | 0.001431 |
| array.test | RUN | 0.371225 | 0.791647 | 0.965691 | 0.001536 | 0.001551 | 0.001529 |
| std_array.test | RUN | 0.376587 | 0.774463 | 0.966436 | 0.001795 | 0.001547 | 0.001457 |
| tuple_comparison.test | RUN | 0.343046 | 0.733546 | 0.881361 | 0.001487 | 0.001668 | 0.001440 |
| tuple_construction.test | RUN | 0.344731 | 0.742042 | 0.892018 | 0.001750 | 0.001740 | 0.001394 |
| tuple_conversion.test | RUN | 0.762287 | 2.250381 | 2.897915 | 0.001706 | 0.001509 | 0.001487 |
| tuple_copy.test | RUN | 0.492219 | 1.287767 | 1.530763 | 0.001499 | 0.001621 | 0.001602 |
| tuple_element.test | RUN | 0.314508 | 0.667449 | 0.822700 | 0.001593 | 0.001646 | 0.001507 |
| tuple_make.test | RUN | 0.343773 | 0.771414 | 0.899295 | 0.001534 | 0.001848 | 0.001401 |
| tuple_misc.test | RUN | 0.429694 | 0.928491 | 1.131204 | 0.001713 | 0.001652 | 0.001472 |
| tuple_mutate.test | RUN | 0.309597 | 0.624857 | 0.790446 | 0.001584 | 0.001511 | 0.001648 |
| tuple_nest.test | RUN | 1.247764 | 4.457970 | 5.416533 | 0.001698 | 0.001737 | 0.001650 |
| tuple_hash.test | RUN | 0.435437 | 1.312893 | 1.181756 | 0.001740 | 0.001628 | 0.001420 |
| tuple_tie.test | RUN | 0.319761 | 0.710053 | 0.865307 | 0.001704 | 0.001729 | 0.001712 |
| tuple_traits__maybe_variadic.test | RUN | 0.340954 | 0.843109 | 0.915485 | 0.001740 | 0.001680 | 0.001478 |
| tuple_traits__no_variadic.test | RUN | 0.390206 | 0.883247 | 0.992353 | 0.001625 | 0.001745 | 0.001710 |
| transform_view.test | RUN | 0.499377 | 1.401268 | 1.587755 | 0.001769 | 0.001750 | 0.001583 |
| identity_view.test | RUN | 0.473460 | 1.322104 | 1.503639 | 0.001776 | 0.001446 | 0.001747 |
| vector_comparison.test | RUN | 0.316257 | 0.700579 | 0.854468 | 0.001779 | 0.001645 | 0.001683 |
| vector_construction.test | RUN | 0.313879 | 0.660317 | 0.872662 | 0.001683 | 0.001734 | 0.001612 |
| vector_conversion.test | RUN | 0.885851 | 2.897877 | 3.697256 | 0.001648 | 0.001694 | 0.001660 |
| vector_copy.test | RUN | 0.472201 | 1.271300 | 1.504214 | 0.001444 | 0.001772 | 0.001548 |
| vector_iterator.test | RUN | 0.304994 | 0.707148 | 0.794140 | 0.001807 | 0.001648 | 0.001556 |
| vector_make.test | RUN | 0.323050 | 0.725881 | 0.853582 | 0.001566 | 0.001652 | 0.001660 |
| vector_misc.test | RUN | 0.407223 | 0.884620 | 1.067488 | 0.001494 | 0.001497 | 0.001314 |
| vector_move.test | RUN | 0.329245 | 0.725128 | 0.857465 | 0.001479 | 0.001708 | 0.001573 |
| vector_mutate.test | RUN | 0.283575 | 0.601340 | 0.731766 | 0.001591 | 0.001561 | 0.001469 |
| vector_n.test | RUN | 0.347580 | 0.758747 | 0.995657 | 0.001701 | 0.001675 | 0.001700 |
| vector_nest.test | RUN | 1.273209 | 4.603267 | 5.445496 | 0.001662 | 0.001684 | 0.001691 |
| vector_hash.test | RUN | 0.420374 | 1.291040 | 1.144312 | 0.001716 | 0.001690 | 0.001463 |
| vector_tie.test | RUN | 0.305852 | 0.660857 | 0.795944 | 0.001658 | 0.001716 | 0.001626 |
| vector_traits__maybe_variadic.test | RUN | 0.321701 | 0.765785 | 0.844142 | 0.001707 | 0.001714 | 0.001642 |
| vector_traits__no_variadic.test | RUN | 0.360643 | 0.817840 | 0.889476 | 0.001576 | 0.001681 | 0.001509 |
| vector_value_at.test | RUN | 0.304070 | 0.607743 | 0.757571 | 0.001631 | 0.001670 | 0.001396 |
| zip_view.test | RUN | 0.592550 | 1.478954 | 1.958069 | 0.001794 | 0.001756 | 0.001644 |
| zip_view2.test | RUN | 0.493614 | 1.160928 | 1.459233 | 0.001621 | 0.001432 | 0.001647 |
| zip_view_ignore.test | RUN | 0.453106 | 1.050863 | 1.308238 | 0.001493 | 0.001634 | 0.001687 |
| repetitive_view.test | RUN | 0.751717 | 1.952569 | 2.492578 | 0.001606 | 0.001586 | 0.001483 |
| deduce_sequence.test | RUN | 0.376066 | 0.818093 | 1.007718 | 0.001747 | 0.001508 | 0.001701 |
| adapt_adt_named.test | RUN | 0.448314 | 1.123388 | 1.310567 | 0.001572 | 0.001653 | 0.001665 |
| adapt_adt_named_empty.test | RUN | 0.384836 | 0.927658 | 1.054401 | 0.001589 | 0.001620 | 0.001600 |
| adapt_adt.test | RUN | 0.487083 | 1.474905 | 1.534150 | 0.001512 | 0.001664 | 0.001666 |
| adapt_adt_empty.test | RUN | 0.393152 | 0.916210 | 1.049856 | 0.001706 | 0.001753 | 0.001629 |
| adapt_assoc_adt_named.test | RUN | 0.491035 | 1.195213 | 1.380201 | 0.001562 | 0.001720 | 0.001754 |
| adapt_assoc_adt_named_empty.test | RUN | 0.389658 | 0.926026 | 1.062504 | 0.001681 | 0.001506 | 0.001687 |
| adapt_assoc_adt.test | RUN | 0.505386 | 1.232115 | 1.463094 | 0.001534 | 0.001512 | 0.001701 |
| adapt_assoc_adt_empty.test | RUN | 0.389474 | 0.929885 | 1.038057 | 0.001781 | 0.001678 | 0.001728 |
| adapt_assoc_struct_named.test | RUN | 0.482759 | 1.177469 | 1.373128 | 0.001547 | 0.001687 | 0.001415 |
| adapt_assoc_struct_named_empty.test | RUN | 0.386277 | 0.917523 | 1.045956 | 0.001647 | 0.001662 | 0.001586 |
| adapt_assoc_struct.test | RUN | 0.443898 | 1.091222 | 1.263520 | 0.001569 | 0.001570 | 0.001611 |
| adapt_assoc_struct_empty.test | RUN | 0.383341 | 0.906305 | 1.050818 | 0.001554 | 0.001539 | 0.001622 |
| adapt_assoc_tpl_adt.test | RUN | 0.522490 | 1.281859 | 1.477718 | 0.001755 | 0.001702 | 0.001488 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.396426 | 0.934109 | 1.051162 | 0.001656 | 0.001707 | 0.001580 |
| adapt_assoc_tpl_struct.test | RUN | 0.419683 | 1.063503 | 1.232349 | 0.001406 | 0.001538 | 0.001586 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.386287 | 0.915433 | 1.043832 | 0.001682 | 0.001702 | 0.001607 |
| adapt_struct_named.test | RUN | 0.435484 | 1.109593 | 1.246898 | 0.001843 | 0.001744 | 0.001691 |
| adapt_struct_named_empty.test | RUN | 0.384178 | 0.906286 | 1.043815 | 0.001503 | 0.001656 | 0.001671 |
| adapt_struct.test | RUN | 0.478092 | 1.260290 | 1.409739 | 0.001715 | 0.001764 | 0.001626 |
| adapt_struct_empty.test | RUN | 0.383618 | 0.928195 | 1.046941 | 0.001725 | 0.001706 | 0.001604 |
| adapt_tpl_adt.test | RUN | 0.497586 | 1.459027 | 1.471825 | 0.001768 | 0.001634 | 0.001669 |
| adapt_tpl_adt_empty.test | RUN | 0.392157 | 0.904193 | 1.057670 | 0.001704 | 0.001723 | 0.001722 |
| adapt_tpl_struct.test | RUN | 0.403375 | 1.041429 | 1.164541 | 0.001750 | 0.001636 | 0.001638 |
| adapt_tpl_struct_empty.test | RUN | 0.383481 | 0.908452 | 1.041149 | 0.001586 | 0.001516 | 0.001687 |
| adt_attribute_proxy.test | RUN | 0.439081 | 0.901174 | 1.121197 | 0.001690 | 0.001639 | 0.001526 |
| define_struct.test | RUN | 0.518044 | 1.264041 | 1.452474 | 0.001748 | 0.001667 | 0.001575 |
| define_struct_empty.test | RUN | 0.454816 | 1.038714 | 1.205761 | 0.001716 | 0.001576 | 0.001632 |
| define_struct_move.test | RUN | 0.305401 | 0.585857 | 0.716454 | 0.001803 | 0.001629 | 0.001529 |
| define_struct_inline.test | RUN | 0.547857 | 1.485312 | 1.652453 | 0.001844 | 0.001657 | 0.001656 |
| define_struct_inline_empty.test | RUN | 0.455596 | 1.028004 | 1.208284 | 0.001770 | 0.001540 | 0.001616 |
| define_struct_inline_move.test | RUN | 0.309383 | 0.577449 | 0.732756 | 0.001514 | 0.001728 | 0.001622 |
| define_assoc_struct.test | RUN | 0.497905 | 1.198082 | 1.396355 | 0.001762 | 0.001636 | 0.001687 |
| define_assoc_struct_empty.test | RUN | 0.455145 | 1.020187 | 1.211338 | 0.001490 | 0.001745 | 0.001647 |
| define_assoc_struct_move.test | RUN | 0.305042 | 0.591068 | 0.736862 | 0.001786 | 0.001701 | 0.001630 |
| define_tpl_struct.test | RUN | 0.497645 | 1.209899 | 1.401240 | 0.001884 | 0.001607 | 0.001478 |
| define_tpl_struct_empty.test | RUN | 0.461480 | 1.047146 | 1.212611 | 0.001708 | 0.001713 | 0.001618 |
| define_tpl_struct_move.test | RUN | 0.309504 | 0.607207 | 0.735879 | 0.001540 | 0.001618 | 0.001564 |
| define_tpl_struct_inline.test | RUN | 0.525230 | 1.393432 | 1.543137 | 0.001667 | 0.001586 | 0.001712 |
| define_tpl_struct_inline_empty.test | RUN | 0.460628 | 1.032917 | 1.217098 | 0.001914 | 0.001790 | 0.001649 |
| define_tpl_struct_inline_move.test | RUN | 0.310294 | 0.599242 | 0.746924 | 0.001607 | 0.001708 | 0.001580 |
| define_assoc_tpl_struct.test | RUN | 0.502864 | 1.224229 | 1.429512 | 0.001715 | 0.001794 | 0.001667 |
| define_assoc_tpl_struct_empty.test | RUN | 0.454736 | 1.029755 | 1.217192 | 0.001670 | 0.001744 | 0.001487 |
| define_assoc_tpl_struct_move.test | RUN | 0.307586 | 0.600628 | 0.741693 | 0.001467 | 0.001718 | 0.001663 |
| std_tuple.test | RUN | 0.304690 | 0.657218 | 0.811468 | 0.001590 | 0.001490 | 0.001387 |
| std_tuple_iterator.test | RUN | 0.306338 | 0.690206 | 0.815937 | 0.001742 | 0.001670 | 0.001649 |
| ref_vector.test | RUN | 0.353878 | 0.721139 | 0.876660 | 0.001767 | 0.001773 | 0.001633 |
| flatten_view.test | RUN | 0.371124 | 0.898860 | 1.054623 | 0.001509 | 0.001708 | 0.001416 |
| github-159.test | COMPILE | 0.173690 | 0.308611 | 0.389578 | — | — | — |
| github-176.test | RUN | 0.326282 | 0.777507 | 0.884068 | 0.001773 | 0.001621 | 0.001611 |
| size.test | COMPILE | 0.291512 | 0.551837 | 0.680275 | — | — | — |
| fused.test | RUN | 0.412004 | 0.830326 | 1.039062 | 0.001434 | 0.001717 | 0.001663 |
| fused_function_object.test | RUN | 0.309977 | 0.642604 | 0.781729 | 0.001747 | 0.001735 | 0.002909 |
| fused_procedure.test | RUN | 0.398111 | 0.777174 | 1.027436 | 0.001538 | 0.001611 | 0.001456 |
| unfused.test | RUN | 0.315735 | 0.653631 | 0.839195 | 0.001648 | 0.001733 | 0.001503 |
| unfused_typed.test | RUN | 0.324286 | 0.699827 | 0.857399 | 0.001489 | 0.001762 | 0.001650 |
| make_fused.test | RUN | 0.410479 | 0.824120 | 1.038897 | 0.001688 | 0.001518 | 0.001408 |
| make_fused_function_object.test | RUN | 0.308586 | 0.631998 | 0.791227 | 0.001422 | 0.001678 | 0.001432 |
| make_fused_procedure.test | RUN | 0.391052 | 0.797800 | 0.995395 | 0.001660 | 0.001689 | 0.001622 |
| make_unfused.test | RUN | 0.308783 | 0.648697 | 0.802536 | 0.001698 | 0.001677 | 0.001632 |
| invoke.test | RUN | 0.765884 | 2.292137 | 2.722785 | 0.001644 | 0.001667 | 0.001714 |
| invoke_function_object.test | RUN | 0.394055 | 0.834113 | 1.025183 | 0.001741 | 0.001746 | 0.001719 |
| invoke_procedure.test | RUN | 0.613581 | 1.717860 | 2.017018 | 0.001554 | 0.001755 | 0.001652 |
| swap.test | RUN | 0.458579 | 1.061659 | 1.285178 | 0.001617 | 0.001620 | 0.001711 |
| is_sequence.test | COMPILE | 0.087588 | 0.145993 | 0.154681 | — | — | — |
| is_view.test | COMPILE | 0.086809 | 0.144952 | 0.156261 | — | — | — |
| pair_deque.test | COMPILE | 0.175721 | 0.341374 | 0.397677 | — | — | — |
| pair_list.test | COMPILE | 0.170634 | 0.324998 | 0.375596 | — | — | — |
| pair_map.test | COMPILE | 0.179960 | 0.342620 | 0.422772 | — | — | — |
| pair_set.test | COMPILE | 0.183522 | 0.343738 | 0.398744 | — | — | — |
| pair_vector.test | COMPILE | 0.179174 | 0.342419 | 0.390368 | — | — | — |
| pair_nest.test | COMPILE | 0.069867 | 0.136770 | 0.150707 | — | — | — |
| index_sequence.test | COMPILE | 0.065504 | 0.095257 | 0.110119 | — | — | — |
| and.test | COMPILE | 0.050649 | 0.072665 | 0.075845 | — | — | — |
| tag_of.test | COMPILE | 0.083938 | 0.141095 | 0.152672 | — | — | — |
| unused.test | COMPILE | 0.040818 | 0.093462 | 0.089349 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.068054 | 0.142829 | 0.137657 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.057511 | 0.109371 | 0.110915 | — | — | — |
| quick.test | RUN | 0.202579 | 0.425219 | 0.484879 | 0.000554 | 0.001105 | 0.001115 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.997× | 1.007×, 0.988× | 1.007× |
| clang | 1.011× | 1.011×, 1.012× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001014s | 0.001029s | 0.001037s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.000994s | 0.001032s | 0.001019s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001034s | 0.001036s | 0.001061s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001021s | 0.001022s | 0.000997s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.001046s | 0.001010s | 0.000987s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001002s | 0.001034s | 0.001024s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.000997s | 0.001015s | 0.001010s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001024s | 0.001041s | 0.001035s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001026s | 0.001031s | 0.001052s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.001021s | 0.001051s | 0.001014s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001028s | 0.001051s | 0.001026s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001032s | 0.001031s | 0.001011s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001017s | 0.001058s | 0.000995s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.001019s | 0.000997s | 0.001034s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001044s | 0.001033s | 0.001052s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001018s | 0.001016s | 0.001056s | byte equality observed |
| adapt_struct.test | 10 | 0.001042s | 0.001055s | 0.001039s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001025s | 0.001063s | 0.001034s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001023s | 0.000996s | 0.001024s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001035s | 0.001053s | 0.001068s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001019s | 0.001044s | 0.001058s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001015s | 0.001030s | 0.001036s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001030s | 0.001032s | 0.001049s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001034s | 0.001048s | 0.001009s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.000999s | 0.001029s | 0.001023s | byte equality observed |
| all.test | 10 | 0.000990s | 0.000987s | 0.000985s | byte equality observed |
| any.test | 10 | 0.001007s | 0.000969s | 0.000996s | byte equality observed |
| array.test | 10 | 0.000987s | 0.001022s | 0.001017s | byte equality observed |
| as_deque.test | 10 | 0.000989s | 0.000993s | 0.001000s | byte equality observed |
| as_list.test | 10 | 0.001022s | 0.001005s | 0.001009s | byte equality observed |
| as_map.test | 10 | 0.001035s | 0.001095s | 0.001056s | byte equality observed |
| as_map_assoc.test | 10 | 0.001031s | 0.001023s | 0.001007s | byte equality observed |
| as_set.test | 10 | 0.001022s | 0.001070s | 0.001001s | byte equality observed |
| as_vector.test | 10 | 0.001088s | 0.001060s | 0.001083s | byte equality observed |
| back_extended_deque.test | 10 | 0.000983s | 0.000952s | 0.001020s | byte equality observed |
| boost_array.test | 10 | 0.001021s | 0.000967s | 0.000988s | byte equality observed |
| boost_tuple.test | 10 | 0.001014s | 0.001013s | 0.001018s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.001057s | 0.000950s | 0.001039s | byte equality observed |
| clear.test | 10 | 0.000992s | 0.001029s | 0.001006s | byte equality observed |
| cons.test | 10 | 0.001046s | 0.001030s | 0.001017s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.000996s | 0.000995s | 0.000994s | byte equality observed |
| convert_deque.test | 10 | 0.000996s | 0.001015s | 0.000996s | byte equality observed |
| convert_list.test | 10 | 0.001011s | 0.000984s | 0.000986s | byte equality observed |
| convert_std_pair.test | 10 | 0.000991s | 0.001011s | 0.001015s | byte equality observed |
| convert_std_tuple.test | 10 | 0.001012s | 0.000980s | 0.000983s | byte equality observed |
| convert_vector.test | 10 | 0.001002s | 0.000994s | 0.001025s | byte equality observed |
| copy.test | 10 | 0.000990s | 0.000956s | 0.001017s | byte equality observed |
| count.test | 10 | 0.000986s | 0.000997s | 0.001034s | byte equality observed |
| count_if.test | 10 | 0.001005s | 0.001032s | 0.001051s | byte equality observed |
| deduce_sequence.test | 10 | 0.001018s | 0.000965s | 0.000999s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001007s | 0.001006s | 0.001040s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001002s | 0.001035s | 0.001053s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.001004s | 0.000982s | 0.000976s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001004s | 0.001014s | 0.000999s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001006s | 0.001024s | 0.000986s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.000997s | 0.000969s | 0.001047s | byte equality observed |
| define_struct.test | 10 | 0.001036s | 0.001031s | 0.001010s | byte equality observed |
| define_struct_empty.test | 10 | 0.001007s | 0.001026s | 0.001037s | byte equality observed |
| define_struct_inline.test | 10 | 0.001031s | 0.001022s | 0.000991s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001040s | 0.001013s | 0.001020s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.001035s | 0.000965s | 0.000983s | byte equality observed |
| define_struct_move.test | 10 | 0.000991s | 0.001038s | 0.001032s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001006s | 0.001041s | 0.000993s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.001028s | 0.000979s | 0.001036s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001036s | 0.001018s | 0.001038s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001025s | 0.001025s | 0.001032s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000976s | 0.000977s | 0.000980s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000981s | 0.000990s | 0.001022s | byte equality observed |
| deque_comparison.test | 10 | 0.000970s | 0.001019s | 0.000958s | byte equality observed |
| deque_construction.test | 10 | 0.001001s | 0.000979s | 0.001009s | byte equality observed |
| deque_copy.test | 10 | 0.001024s | 0.000995s | 0.000996s | byte equality observed |
| deque_hash.test | 10 | 0.000979s | 0.001026s | 0.001019s | byte equality observed |
| deque_iterator.test | 10 | 0.000986s | 0.000970s | 0.000963s | byte equality observed |
| deque_make.test | 10 | 0.001017s | 0.000996s | 0.000969s | byte equality observed |
| deque_misc.test | 10 | 0.001007s | 0.001024s | 0.000973s | byte equality observed |
| deque_move.test | 10 | 0.000989s | 0.001018s | 0.001068s | byte equality observed |
| deque_mutate.test | 10 | 0.000995s | 0.001027s | 0.001012s | byte equality observed |
| deque_nest.test | 10 | 0.000968s | 0.001017s | 0.000999s | byte equality observed |
| deque_tie.test | 10 | 0.000998s | 0.001010s | 0.001003s | byte equality observed |
| deque_value_at.test | 10 | 0.000998s | 0.001017s | 0.000998s | byte equality observed |
| erase.test | 10 | 0.001012s | 0.000999s | 0.001023s | byte equality observed |
| erase_key.test | 10 | 0.001035s | 0.001070s | 0.001036s | byte equality observed |
| filter.test | 10 | 0.001014s | 0.001039s | 0.001030s | byte equality observed |
| filter_if.test | 10 | 0.001002s | 0.001024s | 0.001037s | byte equality observed |
| filter_view.test | 10 | 0.001031s | 0.001017s | 0.001028s | byte equality observed |
| find.test | 10 | 0.001020s | 0.001084s | 0.001008s | byte equality observed |
| find_if.test | 10 | 0.001022s | 0.001011s | 0.001064s | byte equality observed |
| flatten.test | 10 | 0.001042s | 0.000993s | 0.001018s | byte equality observed |
| flatten_view.test | 10 | 0.001049s | 0.001007s | 0.001024s | byte equality observed |
| fold.test | 10 | 0.001013s | 0.001028s | 0.000998s | byte equality observed |
| for_each.test | 10 | 0.001009s | 0.001017s | 0.001017s | byte equality observed |
| front_extended_deque.test | 10 | 0.000986s | 0.000964s | 0.000983s | byte equality observed |
| fused.test | 10 | 0.000985s | 0.000999s | 0.001042s | byte equality observed |
| fused_function_object.test | 10 | 0.001027s | 0.001005s | 0.001016s | byte equality observed |
| fused_procedure.test | 10 | 0.000975s | 0.000980s | 0.001017s | byte equality observed |
| github-176.test | 10 | 0.000966s | 0.001060s | 0.001036s | byte equality observed |
| hash.test | 10 | 0.001004s | 0.000984s | 0.000972s | byte equality observed |
| identity_view.test | 10 | 0.001026s | 0.001010s | 0.001043s | byte equality observed |
| insert.test | 10 | 0.001040s | 0.001026s | 0.001107s | byte equality observed |
| insert_range.test | 10 | 0.001063s | 0.001063s | 0.001031s | byte equality observed |
| invoke.test | 10 | 0.001008s | 0.000984s | 0.001033s | byte equality observed |
| invoke_function_object.test | 10 | 0.000976s | 0.000984s | 0.000987s | byte equality observed |
| invoke_procedure.test | 10 | 0.001008s | 0.001001s | 0.001028s | byte equality observed |
| io.test | 10 | 0.001124s | 0.001122s | 0.001138s | byte equality observed |
| iter_fold.test | 10 | 0.001019s | 0.001018s | 0.001043s | byte equality observed |
| iterator_range.test | 10 | 0.001060s | 0.001056s | 0.001014s | byte equality observed |
| join.test | 10 | 0.000959s | 0.000975s | 0.000996s | byte equality observed |
| joint_view.test | 10 | 0.001045s | 0.001074s | 0.001034s | byte equality observed |
| list_comparison.test | 10 | 0.001030s | 0.000975s | 0.001045s | byte equality observed |
| list_construction.test | 10 | 0.000965s | 0.000985s | 0.001006s | byte equality observed |
| list_copy.test | 10 | 0.001000s | 0.000977s | 0.000994s | byte equality observed |
| list_hash.test | 10 | 0.001047s | 0.000992s | 0.001002s | byte equality observed |
| list_iterator.test | 10 | 0.000976s | 0.001058s | 0.000965s | byte equality observed |
| list_make.test | 10 | 0.000986s | 0.000974s | 0.000973s | byte equality observed |
| list_misc.test | 10 | 0.001031s | 0.001000s | 0.001010s | byte equality observed |
| list_mutate.test | 10 | 0.001013s | 0.000966s | 0.001030s | byte equality observed |
| list_nest.test | 10 | 0.000998s | 0.001010s | 0.001027s | byte equality observed |
| list_tie.test | 10 | 0.001009s | 0.001006s | 0.001017s | byte equality observed |
| list_value_at.test | 10 | 0.000962s | 0.001009s | 0.001025s | byte equality observed |
| make_fused.test | 10 | 0.001015s | 0.000968s | 0.000975s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001015s | 0.000979s | 0.001016s | byte equality observed |
| make_fused_procedure.test | 10 | 0.000971s | 0.000971s | 0.001025s | byte equality observed |
| make_list.test | 10 | 0.001007s | 0.001002s | 0.000972s | byte equality observed |
| make_unfused.test | 10 | 0.000995s | 0.001008s | 0.001011s | byte equality observed |
| make_vector.test | 10 | 0.000965s | 0.000988s | 0.000999s | byte equality observed |
| map.test | 10 | 0.001007s | 0.000995s | 0.001040s | byte equality observed |
| map_comparison.test | 10 | 0.001004s | 0.000998s | 0.000980s | byte equality observed |
| map_construction.test | 10 | 0.001022s | 0.000964s | 0.000983s | byte equality observed |
| map_copy.test | 10 | 0.000989s | 0.001006s | 0.001015s | byte equality observed |
| map_misc.test | 10 | 0.001044s | 0.000973s | 0.001002s | byte equality observed |
| map_move.test | 10 | 0.000996s | 0.001006s | 0.001006s | byte equality observed |
| map_mutate.test | 10 | 0.000973s | 0.000995s | 0.001020s | byte equality observed |
| map_tie.test | 10 | 0.001052s | 0.000974s | 0.001001s | byte equality observed |
| move.test | 10 | 0.000978s | 0.000993s | 0.000985s | byte equality observed |
| nil.test | 10 | 0.000986s | 0.001020s | 0.000997s | byte equality observed |
| none.test | 10 | 0.000989s | 0.001022s | 0.000984s | byte equality observed |
| nview.test | 10 | 0.001010s | 0.000986s | 0.001007s | byte equality observed |
| pop_back.test | 10 | 0.001044s | 0.001028s | 0.001037s | byte equality observed |
| pop_front.test | 10 | 0.001022s | 0.001043s | 0.001054s | byte equality observed |
| push_back.test | 10 | 0.001060s | 0.001064s | 0.001039s | byte equality observed |
| push_front.test | 10 | 0.001023s | 0.001029s | 0.001039s | byte equality observed |
| quick.test | 10 | 0.000448s | 0.000961s | 0.000986s | byte equality observed |
| ref_vector.test | 10 | 0.001012s | 0.001010s | 0.000986s | byte equality observed |
| remove.test | 10 | 0.001048s | 0.001045s | 0.001071s | byte equality observed |
| remove_if.test | 10 | 0.001010s | 0.001038s | 0.001010s | byte equality observed |
| repetitive_view.test | 10 | 0.001030s | 0.001039s | 0.000995s | byte equality observed |
| replace.test | 10 | 0.001023s | 0.000998s | 0.001031s | byte equality observed |
| replace_if.test | 10 | 0.001071s | 0.001045s | 0.001048s | byte equality observed |
| reverse.test | 10 | 0.001042s | 0.001041s | 0.001050s | byte equality observed |
| reverse_fold.test | 10 | 0.001027s | 0.001006s | 0.001027s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001022s | 0.000988s | 0.001048s | byte equality observed |
| reverse_view.test | 10 | 0.001065s | 0.000993s | 0.001043s | byte equality observed |
| segmented_find.test | 10 | 0.000965s | 0.001053s | 0.000999s | byte equality observed |
| segmented_find_if.test | 10 | 0.000996s | 0.000986s | 0.000984s | byte equality observed |
| segmented_fold.test | 10 | 0.001017s | 0.001050s | 0.001048s | byte equality observed |
| segmented_for_each.test | 10 | 0.001041s | 0.001034s | 0.001075s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001060s | 0.001034s | 0.001032s | byte equality observed |
| set.test | 10 | 0.001022s | 0.001047s | 0.000995s | byte equality observed |
| single_view.test | 10 | 0.001022s | 0.001014s | 0.001010s | byte equality observed |
| std_array.test | 10 | 0.001006s | 0.001069s | 0.001014s | byte equality observed |
| std_pair.test | 10 | 0.001004s | 0.001004s | 0.001018s | byte equality observed |
| std_tuple.test | 10 | 0.001011s | 0.001000s | 0.000990s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000997s | 0.000993s | 0.000983s | byte equality observed |
| swap.test | 10 | 0.001024s | 0.001023s | 0.001066s | byte equality observed |
| transform.test | 10 | 0.001035s | 0.001017s | 0.001013s | byte equality observed |
| transform_view.test | 10 | 0.001035s | 0.001034s | 0.001007s | byte equality observed |
| tuple_comparison.test | 10 | 0.000992s | 0.000990s | 0.001017s | byte equality observed |
| tuple_construction.test | 10 | 0.000996s | 0.000970s | 0.001014s | byte equality observed |
| tuple_conversion.test | 10 | 0.000985s | 0.000980s | 0.000975s | byte equality observed |
| tuple_copy.test | 10 | 0.000974s | 0.000995s | 0.000993s | byte equality observed |
| tuple_element.test | 10 | 0.000954s | 0.000993s | 0.001029s | byte equality observed |
| tuple_hash.test | 10 | 0.001005s | 0.000981s | 0.001003s | byte equality observed |
| tuple_make.test | 10 | 0.001000s | 0.000997s | 0.000991s | byte equality observed |
| tuple_misc.test | 10 | 0.000996s | 0.001002s | 0.000975s | byte equality observed |
| tuple_mutate.test | 10 | 0.000987s | 0.000980s | 0.000992s | byte equality observed |
| tuple_nest.test | 10 | 0.001009s | 0.001020s | 0.000997s | byte equality observed |
| tuple_tie.test | 10 | 0.001020s | 0.001013s | 0.000996s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.000986s | 0.000994s | 0.000995s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001007s | 0.001006s | 0.000995s | byte equality observed |
| unfused.test | 10 | 0.001008s | 0.001003s | 0.000988s | byte equality observed |
| unfused_typed.test | 10 | 0.001011s | 0.000982s | 0.000971s | byte equality observed |
| vector_comparison.test | 10 | 0.000999s | 0.000976s | 0.001016s | byte equality observed |
| vector_construction.test | 10 | 0.000976s | 0.000989s | 0.001035s | byte equality observed |
| vector_conversion.test | 10 | 0.001029s | 0.001015s | 0.000971s | byte equality observed |
| vector_copy.test | 10 | 0.000961s | 0.000984s | 0.001009s | byte equality observed |
| vector_hash.test | 10 | 0.000989s | 0.001002s | 0.001009s | byte equality observed |
| vector_iterator.test | 10 | 0.000978s | 0.000972s | 0.000972s | byte equality observed |
| vector_make.test | 10 | 0.000998s | 0.001038s | 0.001045s | byte equality observed |
| vector_misc.test | 10 | 0.000981s | 0.000981s | 0.001021s | byte equality observed |
| vector_move.test | 10 | 0.001000s | 0.001007s | 0.000985s | byte equality observed |
| vector_mutate.test | 10 | 0.000997s | 0.001041s | 0.001072s | byte equality observed |
| vector_n.test | 10 | 0.000991s | 0.000989s | 0.001009s | byte equality observed |
| vector_nest.test | 10 | 0.001010s | 0.000976s | 0.001065s | byte equality observed |
| vector_tie.test | 10 | 0.000996s | 0.001019s | 0.000987s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.000989s | 0.000989s | 0.001026s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.001006s | 0.001002s | 0.000989s | byte equality observed |
| vector_value_at.test | 10 | 0.000958s | 0.000990s | 0.001023s | byte equality observed |
| zip.test | 10 | 0.001000s | 0.001024s | 0.000994s | byte equality observed |
| zip2.test | 10 | 0.001006s | 0.000991s | 0.000985s | byte equality observed |
| zip_ignore.test | 10 | 0.000987s | 0.000978s | 0.001017s | byte equality observed |
| zip_view.test | 10 | 0.000977s | 0.001000s | 0.000993s | byte equality observed |
| zip_view2.test | 10 | 0.001008s | 0.000988s | 0.000974s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000999s | 0.000990s | 0.001020s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
