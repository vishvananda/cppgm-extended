# libs/fusion/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| all.test | RUN | 0.344103 | 0.729144 | 1.063864 | 0.001476 | 0.001557 | 0.001561 |
| any.test | RUN | 0.331679 | 0.703905 | 1.003030 | 0.001501 | 0.001621 | 0.001649 |
| clear.test | RUN | 0.240516 | 0.634447 | 0.782120 | 0.001631 | 0.001652 | 0.001877 |
| copy.test | RUN | 0.204726 | 0.471000 | 0.647817 | 0.001501 | 0.001840 | 0.001561 |
| count.test | RUN | 0.231089 | 0.527101 | 0.725810 | 0.001776 | 0.001709 | 0.001725 |
| count_if.test | RUN | 0.216997 | 0.487741 | 0.664898 | 0.001595 | 0.001710 | 0.001573 |
| erase.test | RUN | 0.306693 | 0.866762 | 1.068496 | 0.001453 | 0.001846 | 0.001568 |
| erase_key.test | RUN | 0.300396 | 0.853536 | 1.002313 | 0.001701 | 0.001585 | 0.001543 |
| filter.test | RUN | 0.275086 | 0.714778 | 0.939226 | 0.001825 | 0.001854 | 0.001736 |
| filter_if.test | RUN | 0.306746 | 0.845458 | 1.063972 | 0.001360 | 0.001810 | 0.001867 |
| find.test | RUN | 0.263104 | 0.597895 | 0.829831 | 0.001111 | 0.001643 | 0.001599 |
| find_if.test | RUN | 0.256381 | 0.532225 | 0.769584 | 0.001788 | 0.001784 | 0.001742 |
| fold.test | RUN | 0.271958 | 0.660146 | 0.926496 | 0.001698 | 0.001896 | 0.001679 |
| for_each.test | RUN | 0.248461 | 0.608163 | 0.820003 | 0.002016 | 0.001855 | 0.001479 |
| insert.test | RUN | 0.363054 | 1.059585 | 1.302976 | 0.001779 | 0.001796 | 0.001746 |
| insert_range.test | RUN | 0.360804 | 1.072736 | 1.308789 | 0.001414 | 0.001739 | 0.001689 |
| iter_fold.test | RUN | 0.431313 | 1.015384 | 1.522345 | 0.001699 | 0.001873 | 0.001764 |
| move.test | RUN | 0.217009 | 0.500196 | 0.671987 | 0.001681 | 0.001521 | 0.001587 |
| none.test | RUN | 0.337908 | 0.710467 | 1.024205 | 0.001704 | 0.001724 | 0.001636 |
| pop_back.test | RUN | 0.343144 | 0.936129 | 1.156205 | 0.001471 | 0.001713 | 0.001790 |
| pop_front.test | RUN | 0.277159 | 0.754161 | 0.925694 | 0.001760 | 0.001676 | 0.001595 |
| push_back.test | RUN | 0.350778 | 1.098811 | 1.252471 | 0.001843 | 0.001824 | 0.001816 |
| push_front.test | RUN | 0.318527 | 0.899601 | 1.059004 | 0.001688 | 0.001753 | 0.001802 |
| remove.test | RUN | 0.340789 | 1.008567 | 1.256926 | 0.001611 | 0.001604 | 0.001771 |
| remove_if.test | RUN | 0.312634 | 0.856296 | 1.071733 | 0.001770 | 0.001744 | 0.001573 |
| replace.test | RUN | 0.305863 | 0.819485 | 0.981906 | 0.001707 | 0.001822 | 0.001802 |
| replace_if.test | RUN | 0.291269 | 0.826599 | 0.972514 | 0.001866 | 0.001857 | 0.001844 |
| reverse_fold.test | RUN | 0.468323 | 1.148430 | 1.695489 | 0.001708 | 0.001782 | 0.001707 |
| reverse_iter_fold.test | RUN | 0.483174 | 1.193754 | 1.797065 | 0.001530 | 0.001694 | 0.001673 |
| reverse.test | RUN | 0.292380 | 0.842378 | 1.021373 | 0.001507 | 0.001766 | 0.001728 |
| segmented_for_each.test | RUN | 0.507178 | 1.603934 | 2.046920 | 0.001646 | 0.001753 | 0.001528 |
| segmented_find.test | RUN | 0.313147 | 0.867177 | 1.178351 | 0.001434 | 0.001749 | 0.001317 |
| segmented_find_if.test | RUN | 0.317041 | 0.845095 | 1.170308 | 0.001459 | 0.001433 | 0.001542 |
| segmented_fold.test | RUN | 0.396387 | 1.105236 | 1.480862 | 0.001739 | 0.001865 | 0.001765 |
| transform.test | RUN | 0.307264 | 0.900403 | 1.058481 | 0.001707 | 0.001743 | 0.001800 |
| join.test | RUN | 0.231358 | 0.538592 | 0.748963 | 0.001724 | 0.001666 | 0.001772 |
| zip.test | RUN | 0.410094 | 1.049044 | 1.455814 | 0.001728 | 0.001881 | 0.001776 |
| zip2.test | RUN | 0.405471 | 0.968529 | 1.380855 | 0.001580 | 0.001755 | 0.001784 |
| zip_ignore.test | RUN | 0.297525 | 0.698544 | 0.909312 | 0.001344 | 0.001618 | 0.001639 |
| flatten.test | RUN | 0.277101 | 0.748588 | 0.892821 | 0.001662 | 0.001895 | 0.001668 |
| ticket-5490.test | COMPILE | 0.152130 | 0.324426 | 0.415892 | — | — | — |
| as_deque.test | RUN | 0.275644 | 0.702550 | 0.987796 | 0.001315 | 0.001855 | 0.001648 |
| as_list.test | RUN | 0.312648 | 0.879371 | 1.040653 | 0.001605 | 0.001660 | 0.001714 |
| as_map.test | RUN | 0.306030 | 0.876294 | 1.074412 | 0.001616 | 0.001732 | 0.001574 |
| as_map_assoc.test | RUN | 0.305171 | 0.789341 | 1.029408 | 0.001625 | 0.001876 | 0.001417 |
| as_set.test | RUN | 0.347637 | 1.004149 | 1.260453 | 0.001710 | 0.001711 | 0.001662 |
| as_vector.test | RUN | 0.316293 | 0.901802 | 1.112905 | 0.001573 | 0.001667 | 0.001550 |
| boost_tuple.test | RUN | 0.314909 | 0.901846 | 1.042269 | 0.001530 | 0.001642 | 0.001600 |
| boost_tuple_iterator.test | RUN | 0.223506 | 0.527422 | 0.670804 | 0.001568 | 0.001720 | 0.001736 |
| cons.test | RUN | 0.432690 | 0.992992 | 1.288525 | 0.001539 | 0.001605 | 0.001852 |
| convert_boost_tuple.test | RUN | 0.281873 | 0.771486 | 0.845276 | 0.001807 | 0.001786 | 0.001704 |
| convert_deque.test | RUN | 0.284143 | 0.750907 | 0.859118 | 0.001453 | 0.001624 | 0.001409 |
| convert_list.test | RUN | 0.272655 | 0.775366 | 0.848777 | 0.001585 | 0.001589 | 0.001424 |
| convert_std_pair.test | RUN | 0.261362 | 0.763792 | 0.870243 | 0.001643 | 0.001634 | 0.001410 |
| convert_std_tuple.test | RUN | 0.249581 | 0.747481 | 0.821930 | 0.001581 | 0.001412 | 0.001617 |
| convert_vector.test | RUN | 0.249501 | 0.759126 | 0.847049 | 0.001656 | 0.001690 | 0.001444 |
| filter_view.test | RUN | 0.304273 | 0.808461 | 1.012361 | 0.001507 | 0.001690 | 0.001684 |
| hash.test | RUN | 0.295433 | 0.723708 | 0.981111 | 0.001812 | 0.001775 | 0.001677 |
| io.test | RUN | 0.310434 | 1.005925 | 1.135291 | 0.001620 | 0.002096 | 0.002019 |
| iterator_range.test | RUN | 0.281289 | 0.773277 | 0.968900 | 0.001614 | 0.001702 | 0.001506 |
| joint_view.test | RUN | 0.379434 | 1.265712 | 1.435270 | 0.001766 | 0.001818 | 0.001860 |
| list_comparison.test | RUN | 0.221989 | 0.533614 | 0.699551 | 0.001726 | 0.001420 | 0.001330 |
| list_construction.test | RUN | 0.219159 | 0.528372 | 0.687475 | 0.001517 | 0.001776 | 0.001458 |
| list_copy.test | RUN | 0.335171 | 0.916562 | 1.158646 | 0.001737 | 0.001680 | 0.001702 |
| list_iterator.test | RUN | 0.226111 | 0.515153 | 0.660453 | 0.001621 | 0.001529 | 0.001801 |
| list_hash.test | RUN | 0.292951 | 0.992860 | 0.943280 | 0.001531 | 0.001898 | 0.001702 |
| list_make.test | RUN | 0.232004 | 0.569150 | 0.729082 | 0.001739 | 0.001647 | 0.001671 |
| list_misc.test | RUN | 0.301931 | 0.647342 | 0.891212 | 0.001545 | 0.001846 | 0.001478 |
| list_mutate.test | RUN | 0.209619 | 0.449975 | 0.587643 | 0.001614 | 0.001788 | 0.001752 |
| list_nest.test | RUN | 0.408658 | 1.279659 | 1.589992 | 0.001506 | 0.001714 | 0.001769 |
| list_tie.test | RUN | 0.222126 | 0.504808 | 0.669259 | 0.001838 | 0.001840 | 0.001743 |
| list_value_at.test | RUN | 0.209774 | 0.505636 | 0.634778 | 0.001580 | 0.001820 | 0.001706 |
| deque_comparison.test | RUN | 0.225904 | 0.531688 | 0.729526 | 0.001490 | 0.001793 | 0.001654 |
| deque_construction.test | RUN | 0.224954 | 0.510706 | 0.711374 | 0.001831 | 0.001813 | 0.001722 |
| deque_copy.test | RUN | 0.319366 | 0.870763 | 1.129693 | 0.001542 | 0.001726 | 0.001516 |
| deque_iterator.test | RUN | 0.219444 | 0.538464 | 0.651492 | 0.001730 | 0.001703 | 0.001737 |
| deque_hash.test | RUN | 0.307341 | 0.987461 | 1.003136 | 0.001413 | 0.001679 | 0.001594 |
| deque_is_constructible.test | COMPILE | 0.125253 | 0.239742 | 0.324608 | — | — | — |
| deque_make.test | RUN | 0.223007 | 0.541562 | 0.705287 | 0.001786 | 0.001798 | 0.001764 |
| deque_misc.test | RUN | 0.284485 | 0.655550 | 0.878947 | 0.001314 | 0.001790 | 0.001861 |
| deque_move.test | RUN | 0.223929 | 0.573995 | 0.701718 | 0.001539 | 0.001576 | 0.001565 |
| deque_mutate.test | RUN | 0.204362 | 0.446064 | 0.623409 | 0.001576 | 0.001825 | 0.001463 |
| deque_nest.test | RUN | 0.609206 | 2.019113 | 2.622260 | 0.001724 | 0.001636 | 0.001757 |
| deque_tie.test | RUN | 0.205604 | 0.487588 | 0.670330 | 0.001672 | 0.001714 | 0.001487 |
| deque_value_at.test | RUN | 0.206388 | 0.469950 | 0.627829 | 0.001506 | 0.001593 | 0.001539 |
| front_extended_deque.test | RUN | 0.299793 | 0.699285 | 0.920764 | 0.001514 | 0.001753 | 0.001747 |
| back_extended_deque.test | RUN | 0.295816 | 0.734112 | 0.959738 | 0.001640 | 0.001761 | 0.001773 |
| make_list.test | RUN | 0.229912 | 0.566311 | 0.712494 | 0.001635 | 0.001665 | 0.001538 |
| make_vector.test | RUN | 0.235016 | 0.545676 | 0.752160 | 0.001523 | 0.001438 | 0.001530 |
| map.test | RUN | 0.294046 | 0.877276 | 1.014570 | 0.001616 | 0.001723 | 0.001839 |
| map_comparison.test | RUN | 0.226626 | 0.569141 | 0.724089 | 0.001932 | 0.001587 | 0.001577 |
| map_construction.test | RUN | 0.231203 | 0.532254 | 0.688359 | 0.001545 | 0.001684 | 0.001524 |
| map_copy.test | RUN | 0.279498 | 0.670729 | 0.866061 | 0.001591 | 0.001837 | 0.001750 |
| map_misc.test | RUN | 0.288920 | 0.685420 | 0.881053 | 0.001578 | 0.001699 | 0.001457 |
| map_move.test | RUN | 0.228118 | 0.559289 | 0.694489 | 0.001739 | 0.001684 | 0.001523 |
| map_mutate.test | RUN | 0.210389 | 0.467762 | 0.620922 | 0.001489 | 0.001521 | 0.001466 |
| map_tie.test | RUN | 0.212489 | 0.470818 | 0.615499 | 0.001522 | 0.001758 | 0.001775 |
| nil.test | RUN | 0.144201 | 0.317088 | 0.404607 | 0.001808 | 0.001834 | 0.001694 |
| nview.test | RUN | 0.280816 | 0.671092 | 0.926362 | 0.001610 | 0.001856 | 0.001734 |
| reverse_view.test | RUN | 0.359752 | 1.099097 | 1.288194 | 0.001699 | 0.001833 | 0.001888 |
| segmented_iterator_range.test | RUN | 0.434571 | 1.309471 | 1.628837 | 0.001761 | 0.001849 | 0.001754 |
| set.test | RUN | 0.277195 | 0.731410 | 0.933362 | 0.001674 | 0.001763 | 0.001804 |
| single_view.test | RUN | 0.254239 | 0.673859 | 0.807990 | 0.001826 | 0.001882 | 0.001497 |
| std_pair.test | RUN | 0.321501 | 0.865055 | 1.065770 | 0.001667 | 0.001762 | 0.001743 |
| boost_array.test | RUN | 0.281037 | 0.590661 | 0.799908 | 0.001724 | 0.001837 | 0.001544 |
| array.test | RUN | 0.266491 | 0.608808 | 0.772436 | 0.001565 | 0.001614 | 0.001659 |
| std_array.test | RUN | 0.270616 | 0.622591 | 0.801448 | 0.001761 | 0.001769 | 0.001756 |
| tuple_comparison.test | RUN | 0.244974 | 0.562812 | 0.735736 | 0.001601 | 0.001852 | 0.001674 |
| tuple_construction.test | RUN | 0.246135 | 0.572557 | 0.735703 | 0.001673 | 0.001993 | 0.001632 |
| tuple_conversion.test | RUN | 0.527273 | 1.719644 | 2.375030 | 0.001472 | 0.001874 | 0.001556 |
| tuple_copy.test | RUN | 0.346169 | 1.007369 | 1.284241 | 0.001702 | 0.001522 | 0.001361 |
| tuple_element.test | RUN | 0.228010 | 0.538906 | 0.711269 | 0.001614 | 0.001785 | 0.001772 |
| tuple_make.test | RUN | 0.242139 | 0.596567 | 0.774089 | 0.001607 | 0.001732 | 0.001544 |
| tuple_misc.test | RUN | 0.307084 | 0.719804 | 0.963341 | 0.001696 | 0.001736 | 0.001718 |
| tuple_mutate.test | RUN | 0.214335 | 0.474078 | 0.696793 | 0.001824 | 0.001831 | 0.001653 |
| tuple_nest.test | RUN | 0.883734 | 3.473088 | 4.473853 | 0.001628 | 0.001710 | 0.001730 |
| tuple_hash.test | RUN | 0.309529 | 0.989568 | 0.982229 | 0.001808 | 0.001572 | 0.001459 |
| tuple_tie.test | RUN | 0.232342 | 0.545125 | 0.748059 | 0.001673 | 0.001801 | 0.001781 |
| tuple_traits__maybe_variadic.test | RUN | 0.257888 | 0.633251 | 0.779803 | 0.001631 | 0.001865 | 0.001372 |
| tuple_traits__no_variadic.test | RUN | 0.275215 | 0.676786 | 0.872016 | 0.001385 | 0.001903 | 0.001772 |
| transform_view.test | RUN | 0.361382 | 1.067293 | 1.288475 | 0.001714 | 0.001592 | 0.001616 |
| identity_view.test | RUN | 0.353600 | 0.963532 | 1.212491 | 0.001641 | 0.001891 | 0.001636 |
| vector_comparison.test | RUN | 0.221401 | 0.569379 | 0.694678 | 0.001363 | 0.001755 | 0.001887 |
| vector_construction.test | RUN | 0.219925 | 0.514429 | 0.713628 | 0.001338 | 0.001676 | 0.001562 |
| vector_conversion.test | RUN | 0.632402 | 2.281225 | 3.098870 | 0.001629 | 0.001647 | 0.001812 |
| vector_copy.test | RUN | 0.345147 | 0.998822 | 1.325889 | 0.001798 | 0.001681 | 0.001640 |
| vector_iterator.test | RUN | 0.212991 | 0.537562 | 0.652260 | 0.001595 | 0.001537 | 0.001499 |
| vector_make.test | RUN | 0.220991 | 0.562925 | 0.714769 | 0.001415 | 0.001720 | 0.001807 |
| vector_misc.test | RUN | 0.290410 | 0.734282 | 0.895658 | 0.001642 | 0.001447 | 0.001446 |
| vector_move.test | RUN | 0.222449 | 0.560451 | 0.701662 | 0.001785 | 0.001535 | 0.001552 |
| vector_mutate.test | RUN | 0.204609 | 0.463817 | 0.626996 | 0.001382 | 0.001536 | 0.001363 |
| vector_n.test | RUN | 0.252947 | 0.586152 | 0.852927 | 0.001849 | 0.001737 | 0.001756 |
| vector_nest.test | RUN | 0.893206 | 3.696547 | 4.592917 | 0.001472 | 0.001709 | 0.001647 |
| vector_hash.test | RUN | 0.301358 | 0.970102 | 0.954404 | 0.001513 | 0.001750 | 0.001534 |
| vector_tie.test | RUN | 0.215199 | 0.524861 | 0.675218 | 0.001717 | 0.001794 | 0.001602 |
| vector_traits__maybe_variadic.test | RUN | 0.232297 | 0.620075 | 0.727055 | 0.001762 | 0.001844 | 0.001698 |
| vector_traits__no_variadic.test | RUN | 0.252191 | 0.619832 | 0.738003 | 0.001662 | 0.001655 | 0.001551 |
| vector_value_at.test | RUN | 0.215748 | 0.487845 | 0.636831 | 0.001299 | 0.001580 | 0.001402 |
| zip_view.test | RUN | 0.428954 | 1.174667 | 1.653130 | 0.001632 | 0.001625 | 0.001768 |
| zip_view2.test | RUN | 0.350543 | 0.874023 | 1.219710 | 0.001544 | 0.001690 | 0.001438 |
| zip_view_ignore.test | RUN | 0.321529 | 0.799346 | 1.105279 | 0.001503 | 0.001406 | 0.001375 |
| repetitive_view.test | RUN | 0.568305 | 1.483885 | 2.082099 | 0.001689 | 0.001818 | 0.001545 |
| deduce_sequence.test | RUN | 0.278858 | 0.653273 | 0.842657 | 0.001642 | 0.001657 | 0.001797 |
| adapt_adt_named.test | RUN | 0.312840 | 0.886034 | 1.069330 | 0.001609 | 0.001726 | 0.001773 |
| adapt_adt_named_empty.test | RUN | 0.271795 | 0.699293 | 0.869418 | 0.001497 | 0.001874 | 0.001832 |
| adapt_adt.test | RUN | 0.349343 | 1.147447 | 1.262450 | 0.001484 | 0.001896 | 0.001910 |
| adapt_adt_empty.test | RUN | 0.270667 | 0.699959 | 0.861782 | 0.001572 | 0.001846 | 0.001621 |
| adapt_assoc_adt_named.test | RUN | 0.350285 | 0.906884 | 1.150461 | 0.001603 | 0.001876 | 0.001839 |
| adapt_assoc_adt_named_empty.test | RUN | 0.280669 | 0.705553 | 0.860836 | 0.001754 | 0.001698 | 0.001836 |
| adapt_assoc_adt.test | RUN | 0.358318 | 0.955853 | 1.198487 | 0.001530 | 0.001687 | 0.001691 |
| adapt_assoc_adt_empty.test | RUN | 0.269532 | 0.690535 | 0.861472 | 0.001822 | 0.001859 | 0.001800 |
| adapt_assoc_struct_named.test | RUN | 0.343824 | 0.896489 | 1.130636 | 0.001548 | 0.001686 | 0.001574 |
| adapt_assoc_struct_named_empty.test | RUN | 0.275215 | 0.696059 | 0.853073 | 0.001624 | 0.001918 | 0.001435 |
| adapt_assoc_struct.test | RUN | 0.311196 | 0.844044 | 1.042570 | 0.001794 | 0.001830 | 0.001618 |
| adapt_assoc_struct_empty.test | RUN | 0.266220 | 0.691937 | 0.862876 | 0.001656 | 0.001732 | 0.001788 |
| adapt_assoc_tpl_adt.test | RUN | 0.371077 | 0.975530 | 1.221122 | 0.001597 | 0.001956 | 0.001768 |
| adapt_assoc_tpl_adt_empty.test | RUN | 0.280804 | 0.701895 | 0.872225 | 0.001732 | 0.001820 | 0.001780 |
| adapt_assoc_tpl_struct.test | RUN | 0.298505 | 0.823965 | 0.997892 | 0.001814 | 0.001626 | 0.001560 |
| adapt_assoc_tpl_struct_empty.test | RUN | 0.270429 | 0.710284 | 0.881092 | 0.001486 | 0.001666 | 0.001367 |
| adapt_struct_named.test | RUN | 0.311289 | 0.856360 | 1.040305 | 0.001611 | 0.001792 | 0.001677 |
| adapt_struct_named_empty.test | RUN | 0.273314 | 0.696248 | 0.859937 | 0.001472 | 0.001524 | 0.001726 |
| adapt_struct.test | RUN | 0.337244 | 0.980813 | 1.153788 | 0.001479 | 0.001912 | 0.001717 |
| adapt_struct_empty.test | RUN | 0.272424 | 0.722121 | 0.890507 | 0.002078 | 0.001738 | 0.001805 |
| adapt_tpl_adt.test | RUN | 0.355263 | 1.111528 | 1.212279 | 0.001727 | 0.001792 | 0.001683 |
| adapt_tpl_adt_empty.test | RUN | 0.284056 | 0.692018 | 0.872573 | 0.001794 | 0.001827 | 0.001724 |
| adapt_tpl_struct.test | RUN | 0.292493 | 0.787044 | 0.963140 | 0.001786 | 0.001582 | 0.001656 |
| adapt_tpl_struct_empty.test | RUN | 0.270665 | 0.714427 | 0.891506 | 0.001602 | 0.001765 | 0.001810 |
| adt_attribute_proxy.test | RUN | 0.318315 | 0.695129 | 0.901367 | 0.001650 | 0.001831 | 0.001684 |
| define_struct.test | RUN | 0.381324 | 0.961406 | 1.176432 | 0.001746 | 0.001884 | 0.001460 |
| define_struct_empty.test | RUN | 0.330006 | 0.785563 | 1.012752 | 0.001716 | 0.001874 | 0.001656 |
| define_struct_move.test | RUN | 0.220067 | 0.460165 | 0.601183 | 0.001671 | 0.001719 | 0.001776 |
| define_struct_inline.test | RUN | 0.389799 | 1.143717 | 1.383565 | 0.001816 | 0.001544 | 0.001772 |
| define_struct_inline_empty.test | RUN | 0.320588 | 0.791901 | 1.018317 | 0.001718 | 0.001550 | 0.001637 |
| define_struct_inline_move.test | RUN | 0.210890 | 0.458847 | 0.626531 | 0.001520 | 0.001740 | 0.001673 |
| define_assoc_struct.test | RUN | 0.360246 | 0.917517 | 1.136186 | 0.001694 | 0.001898 | 0.001732 |
| define_assoc_struct_empty.test | RUN | 0.324511 | 0.792628 | 0.993292 | 0.001550 | 0.001914 | 0.001810 |
| define_assoc_struct_move.test | RUN | 0.208169 | 0.442459 | 0.595527 | 0.001816 | 0.001791 | 0.001368 |
| define_tpl_struct.test | RUN | 0.362134 | 0.921003 | 1.143561 | 0.001752 | 0.001747 | 0.001624 |
| define_tpl_struct_empty.test | RUN | 0.326128 | 0.801753 | 0.993412 | 0.001616 | 0.001581 | 0.001814 |
| define_tpl_struct_move.test | RUN | 0.214793 | 0.473018 | 0.617739 | 0.001542 | 0.001593 | 0.001653 |
| define_tpl_struct_inline.test | RUN | 0.378865 | 1.077005 | 1.247605 | 0.001743 | 0.001820 | 0.001600 |
| define_tpl_struct_inline_empty.test | RUN | 0.333409 | 0.824916 | 0.989694 | 0.001685 | 0.001712 | 0.001800 |
| define_tpl_struct_inline_move.test | RUN | 0.214517 | 0.437551 | 0.630179 | 0.001737 | 0.001763 | 0.001622 |
| define_assoc_tpl_struct.test | RUN | 0.370427 | 0.923213 | 1.193043 | 0.001732 | 0.001859 | 0.001800 |
| define_assoc_tpl_struct_empty.test | RUN | 0.329567 | 0.794398 | 0.993099 | 0.001446 | 0.001879 | 0.001471 |
| define_assoc_tpl_struct_move.test | RUN | 0.212224 | 0.461650 | 0.607738 | 0.001650 | 0.001732 | 0.001704 |
| std_tuple.test | RUN | 0.218742 | 0.516975 | 0.682635 | 0.001532 | 0.001567 | 0.001584 |
| std_tuple_iterator.test | RUN | 0.216611 | 0.544860 | 0.679122 | 0.001748 | 0.001858 | 0.001627 |
| ref_vector.test | RUN | 0.248816 | 0.553243 | 0.725467 | 0.001701 | 0.001740 | 0.001754 |
| flatten_view.test | RUN | 0.268989 | 0.676270 | 0.869048 | 0.001705 | 0.001862 | 0.001592 |
| github-159.test | COMPILE | 0.119436 | 0.247728 | 0.337670 | — | — | — |
| github-176.test | RUN | 0.245545 | 0.592797 | 0.714628 | 0.001662 | 0.001834 | 0.001671 |
| size.test | COMPILE | 0.207041 | 0.423826 | 0.574686 | — | — | — |
| fused.test | RUN | 0.286810 | 0.659453 | 0.889147 | 0.001355 | 0.001841 | 0.001597 |
| fused_function_object.test | RUN | 0.214532 | 0.484418 | 0.611285 | 0.001660 | 0.001646 | 0.001708 |
| fused_procedure.test | RUN | 0.285920 | 0.623054 | 0.847525 | 0.001746 | 0.001721 | 0.001556 |
| unfused.test | RUN | 0.224024 | 0.493198 | 0.683276 | 0.001739 | 0.001849 | 0.001744 |
| unfused_typed.test | RUN | 0.251733 | 0.543029 | 0.686400 | 0.001625 | 0.001733 | 0.001774 |
| make_fused.test | RUN | 0.309163 | 0.648779 | 0.868766 | 0.001667 | 0.001854 | 0.001289 |
| make_fused_function_object.test | RUN | 0.235185 | 0.506761 | 0.633423 | 0.001500 | 0.001501 | 0.001410 |
| make_fused_procedure.test | RUN | 0.279982 | 0.610069 | 0.847599 | 0.001716 | 0.001840 | 0.001720 |
| make_unfused.test | RUN | 0.219345 | 0.506279 | 0.684826 | 0.001767 | 0.001696 | 0.001712 |
| invoke.test | RUN | 0.568293 | 1.769880 | 2.198552 | 0.001721 | 0.001587 | 0.001412 |
| invoke_function_object.test | RUN | 0.270841 | 0.647430 | 0.846600 | 0.001824 | 0.001826 | 0.001634 |
| invoke_procedure.test | RUN | 0.450996 | 1.307827 | 1.717050 | 0.001533 | 0.001706 | 0.001580 |
| swap.test | RUN | 0.339949 | 0.820138 | 1.108322 | 0.001436 | 0.001766 | 0.001623 |
| is_sequence.test | COMPILE | 0.058470 | 0.119374 | 0.139356 | — | — | — |
| is_view.test | COMPILE | 0.055512 | 0.119861 | 0.139547 | — | — | — |
| pair_deque.test | COMPILE | 0.122238 | 0.267832 | 0.330539 | — | — | — |
| pair_list.test | COMPILE | 0.120795 | 0.253387 | 0.320588 | — | — | — |
| pair_map.test | COMPILE | 0.124260 | 0.263063 | 0.322211 | — | — | — |
| pair_set.test | COMPILE | 0.128265 | 0.267819 | 0.321525 | — | — | — |
| pair_vector.test | COMPILE | 0.122946 | 0.264657 | 0.319587 | — | — | — |
| pair_nest.test | COMPILE | 0.049516 | 0.104937 | 0.134011 | — | — | — |
| index_sequence.test | COMPILE | 0.042375 | 0.078335 | 0.097064 | — | — | — |
| and.test | COMPILE | 0.033801 | 0.059077 | 0.069520 | — | — | — |
| tag_of.test | COMPILE | 0.055711 | 0.112082 | 0.126441 | — | — | — |
| unused.test | COMPILE | 0.030997 | 0.074662 | 0.082363 | — | — | — |
| tag_of_fallback.test | COMPILE | 0.055862 | 0.090474 | 0.115202 | — | — | — |
| make_pair_r-value.test | COMPILE | 0.038666 | 0.103904 | 0.122823 | — | — | — |
| quick.test | RUN | 0.164246 | 0.368911 | 0.419484 | 0.000499 | 0.001259 | 0.001153 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.998× | 1.000×, 0.997× | 1.010× |
| clang | 1.001× | 1.003×, 0.999× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| adapt_adt.test | 10 | 0.001027s | 0.001019s | 0.001035s | byte equality observed |
| adapt_adt_empty.test | 10 | 0.001006s | 0.001014s | 0.001001s | byte equality observed |
| adapt_adt_named.test | 10 | 0.001012s | 0.001050s | 0.001014s | byte equality observed |
| adapt_adt_named_empty.test | 10 | 0.001006s | 0.001018s | 0.001013s | byte equality observed |
| adapt_assoc_adt.test | 10 | 0.000995s | 0.001039s | 0.001043s | byte equality observed |
| adapt_assoc_adt_empty.test | 10 | 0.001019s | 0.001008s | 0.001003s | byte equality observed |
| adapt_assoc_adt_named.test | 10 | 0.001003s | 0.001040s | 0.001038s | byte equality observed |
| adapt_assoc_adt_named_empty.test | 10 | 0.001007s | 0.001010s | 0.001046s | byte equality observed |
| adapt_assoc_struct.test | 10 | 0.001011s | 0.001061s | 0.001036s | byte equality observed |
| adapt_assoc_struct_empty.test | 10 | 0.000997s | 0.001003s | 0.001025s | byte equality observed |
| adapt_assoc_struct_named.test | 10 | 0.001008s | 0.001010s | 0.001033s | byte equality observed |
| adapt_assoc_struct_named_empty.test | 10 | 0.001005s | 0.001009s | 0.001030s | byte equality observed |
| adapt_assoc_tpl_adt.test | 10 | 0.001005s | 0.001045s | 0.001026s | byte equality observed |
| adapt_assoc_tpl_adt_empty.test | 10 | 0.000989s | 0.001030s | 0.001011s | byte equality observed |
| adapt_assoc_tpl_struct.test | 10 | 0.001039s | 0.001043s | 0.001040s | byte equality observed |
| adapt_assoc_tpl_struct_empty.test | 10 | 0.001010s | 0.001025s | 0.001012s | byte equality observed |
| adapt_struct.test | 10 | 0.001025s | 0.001018s | 0.001050s | byte equality observed |
| adapt_struct_empty.test | 10 | 0.001007s | 0.000988s | 0.001007s | byte equality observed |
| adapt_struct_named.test | 10 | 0.001010s | 0.001027s | 0.001025s | byte equality observed |
| adapt_struct_named_empty.test | 10 | 0.001001s | 0.000995s | 0.001039s | byte equality observed |
| adapt_tpl_adt.test | 10 | 0.001023s | 0.001020s | 0.001035s | byte equality observed |
| adapt_tpl_adt_empty.test | 10 | 0.001002s | 0.001019s | 0.000992s | byte equality observed |
| adapt_tpl_struct.test | 10 | 0.001025s | 0.001018s | 0.001016s | byte equality observed |
| adapt_tpl_struct_empty.test | 10 | 0.001018s | 0.001029s | 0.001008s | byte equality observed |
| adt_attribute_proxy.test | 10 | 0.001016s | 0.000994s | 0.001007s | byte equality observed |
| all.test | 10 | 0.001013s | 0.000993s | 0.001001s | byte equality observed |
| any.test | 10 | 0.000983s | 0.000995s | 0.000990s | byte equality observed |
| array.test | 10 | 0.001012s | 0.000995s | 0.000992s | byte equality observed |
| as_deque.test | 10 | 0.000998s | 0.001026s | 0.000969s | byte equality observed |
| as_list.test | 10 | 0.001041s | 0.001029s | 0.001001s | byte equality observed |
| as_map.test | 10 | 0.001013s | 0.000992s | 0.001038s | byte equality observed |
| as_map_assoc.test | 10 | 0.001001s | 0.001000s | 0.001046s | byte equality observed |
| as_set.test | 10 | 0.001027s | 0.001014s | 0.001028s | byte equality observed |
| as_vector.test | 10 | 0.001028s | 0.001040s | 0.001031s | byte equality observed |
| back_extended_deque.test | 10 | 0.000978s | 0.000975s | 0.000986s | byte equality observed |
| boost_array.test | 10 | 0.000995s | 0.000992s | 0.001020s | byte equality observed |
| boost_tuple.test | 10 | 0.001031s | 0.001025s | 0.001006s | byte equality observed |
| boost_tuple_iterator.test | 10 | 0.000986s | 0.000987s | 0.000977s | byte equality observed |
| clear.test | 10 | 0.001037s | 0.001006s | 0.001009s | byte equality observed |
| cons.test | 10 | 0.001033s | 0.001041s | 0.001028s | byte equality observed |
| convert_boost_tuple.test | 10 | 0.001001s | 0.001001s | 0.001016s | byte equality observed |
| convert_deque.test | 10 | 0.000990s | 0.001029s | 0.000988s | byte equality observed |
| convert_list.test | 10 | 0.001006s | 0.000994s | 0.001008s | byte equality observed |
| convert_std_pair.test | 10 | 0.001007s | 0.000992s | 0.000994s | byte equality observed |
| convert_std_tuple.test | 10 | 0.000985s | 0.001033s | 0.000992s | byte equality observed |
| convert_vector.test | 10 | 0.000992s | 0.001007s | 0.000981s | byte equality observed |
| copy.test | 10 | 0.000985s | 0.001003s | 0.000992s | byte equality observed |
| count.test | 10 | 0.001002s | 0.000982s | 0.001004s | byte equality observed |
| count_if.test | 10 | 0.000990s | 0.001025s | 0.001018s | byte equality observed |
| deduce_sequence.test | 10 | 0.000987s | 0.001016s | 0.000976s | byte equality observed |
| define_assoc_struct.test | 10 | 0.001025s | 0.001022s | 0.001014s | byte equality observed |
| define_assoc_struct_empty.test | 10 | 0.001024s | 0.001022s | 0.000995s | byte equality observed |
| define_assoc_struct_move.test | 10 | 0.000992s | 0.000976s | 0.000983s | byte equality observed |
| define_assoc_tpl_struct.test | 10 | 0.001028s | 0.001055s | 0.001023s | byte equality observed |
| define_assoc_tpl_struct_empty.test | 10 | 0.001019s | 0.001010s | 0.001000s | byte equality observed |
| define_assoc_tpl_struct_move.test | 10 | 0.001007s | 0.000977s | 0.000976s | byte equality observed |
| define_struct.test | 10 | 0.001010s | 0.001036s | 0.001016s | byte equality observed |
| define_struct_empty.test | 10 | 0.001002s | 0.001020s | 0.001013s | byte equality observed |
| define_struct_inline.test | 10 | 0.001025s | 0.001014s | 0.001050s | byte equality observed |
| define_struct_inline_empty.test | 10 | 0.001013s | 0.001006s | 0.001016s | byte equality observed |
| define_struct_inline_move.test | 10 | 0.000979s | 0.000987s | 0.000990s | byte equality observed |
| define_struct_move.test | 10 | 0.000988s | 0.000996s | 0.001002s | byte equality observed |
| define_tpl_struct.test | 10 | 0.001026s | 0.001007s | 0.001033s | byte equality observed |
| define_tpl_struct_empty.test | 10 | 0.000989s | 0.001029s | 0.001009s | byte equality observed |
| define_tpl_struct_inline.test | 10 | 0.001015s | 0.001034s | 0.000998s | byte equality observed |
| define_tpl_struct_inline_empty.test | 10 | 0.001024s | 0.001013s | 0.001031s | byte equality observed |
| define_tpl_struct_inline_move.test | 10 | 0.000978s | 0.000985s | 0.000985s | byte equality observed |
| define_tpl_struct_move.test | 10 | 0.000990s | 0.001002s | 0.000974s | byte equality observed |
| deque_comparison.test | 10 | 0.000991s | 0.000971s | 0.000970s | byte equality observed |
| deque_construction.test | 10 | 0.001016s | 0.001007s | 0.001015s | byte equality observed |
| deque_copy.test | 10 | 0.001007s | 0.000974s | 0.000992s | byte equality observed |
| deque_hash.test | 10 | 0.001021s | 0.000985s | 0.001007s | byte equality observed |
| deque_iterator.test | 10 | 0.001024s | 0.000987s | 0.001004s | byte equality observed |
| deque_make.test | 10 | 0.001012s | 0.000965s | 0.000984s | byte equality observed |
| deque_misc.test | 10 | 0.001001s | 0.000976s | 0.000991s | byte equality observed |
| deque_move.test | 10 | 0.001011s | 0.001025s | 0.001017s | byte equality observed |
| deque_mutate.test | 10 | 0.000988s | 0.000984s | 0.000985s | byte equality observed |
| deque_nest.test | 10 | 0.000999s | 0.000993s | 0.000996s | byte equality observed |
| deque_tie.test | 10 | 0.000994s | 0.001016s | 0.001012s | byte equality observed |
| deque_value_at.test | 10 | 0.000996s | 0.001010s | 0.000986s | byte equality observed |
| erase.test | 10 | 0.001011s | 0.001032s | 0.001011s | byte equality observed |
| erase_key.test | 10 | 0.001040s | 0.001031s | 0.001028s | byte equality observed |
| filter.test | 10 | 0.001008s | 0.001018s | 0.001033s | byte equality observed |
| filter_if.test | 10 | 0.001035s | 0.001037s | 0.001033s | byte equality observed |
| filter_view.test | 10 | 0.001044s | 0.001022s | 0.001009s | byte equality observed |
| find.test | 10 | 0.001033s | 0.001015s | 0.001019s | byte equality observed |
| find_if.test | 10 | 0.001024s | 0.001021s | 0.001000s | byte equality observed |
| flatten.test | 10 | 0.001010s | 0.001020s | 0.001002s | byte equality observed |
| flatten_view.test | 10 | 0.001030s | 0.000984s | 0.000999s | byte equality observed |
| fold.test | 10 | 0.001017s | 0.001021s | 0.001034s | byte equality observed |
| for_each.test | 10 | 0.001032s | 0.001000s | 0.001028s | byte equality observed |
| front_extended_deque.test | 10 | 0.000986s | 0.000966s | 0.000984s | byte equality observed |
| fused.test | 10 | 0.001008s | 0.000974s | 0.000967s | byte equality observed |
| fused_function_object.test | 10 | 0.001000s | 0.000997s | 0.001013s | byte equality observed |
| fused_procedure.test | 10 | 0.000995s | 0.000985s | 0.001029s | byte equality observed |
| github-176.test | 10 | 0.001004s | 0.000997s | 0.001037s | byte equality observed |
| hash.test | 10 | 0.000995s | 0.000991s | 0.001042s | byte equality observed |
| identity_view.test | 10 | 0.001014s | 0.001021s | 0.001036s | byte equality observed |
| insert.test | 10 | 0.001047s | 0.001032s | 0.001018s | byte equality observed |
| insert_range.test | 10 | 0.001035s | 0.001030s | 0.001031s | byte equality observed |
| invoke.test | 10 | 0.000992s | 0.001007s | 0.001004s | byte equality observed |
| invoke_function_object.test | 10 | 0.000979s | 0.001020s | 0.001000s | byte equality observed |
| invoke_procedure.test | 10 | 0.001015s | 0.000984s | 0.001001s | byte equality observed |
| io.test | 10 | 0.001130s | 0.001121s | 0.001116s | byte equality observed |
| iter_fold.test | 10 | 0.001002s | 0.001021s | 0.000984s | byte equality observed |
| iterator_range.test | 10 | 0.001045s | 0.001035s | 0.001052s | byte equality observed |
| join.test | 10 | 0.000974s | 0.000996s | 0.000992s | byte equality observed |
| joint_view.test | 10 | 0.001057s | 0.001067s | 0.001050s | byte equality observed |
| list_comparison.test | 10 | 0.000986s | 0.000962s | 0.000985s | byte equality observed |
| list_construction.test | 10 | 0.000972s | 0.000968s | 0.000977s | byte equality observed |
| list_copy.test | 10 | 0.000985s | 0.001008s | 0.001004s | byte equality observed |
| list_hash.test | 10 | 0.000990s | 0.001009s | 0.000990s | byte equality observed |
| list_iterator.test | 10 | 0.000987s | 0.000985s | 0.000983s | byte equality observed |
| list_make.test | 10 | 0.000999s | 0.001018s | 0.000991s | byte equality observed |
| list_misc.test | 10 | 0.000985s | 0.000999s | 0.001011s | byte equality observed |
| list_mutate.test | 10 | 0.000994s | 0.000995s | 0.000978s | byte equality observed |
| list_nest.test | 10 | 0.001005s | 0.000983s | 0.000985s | byte equality observed |
| list_tie.test | 10 | 0.001005s | 0.001003s | 0.000995s | byte equality observed |
| list_value_at.test | 10 | 0.000998s | 0.001034s | 0.001020s | byte equality observed |
| make_fused.test | 10 | 0.001009s | 0.000990s | 0.001012s | byte equality observed |
| make_fused_function_object.test | 10 | 0.001012s | 0.000979s | 0.000973s | byte equality observed |
| make_fused_procedure.test | 10 | 0.001017s | 0.000987s | 0.000977s | byte equality observed |
| make_list.test | 10 | 0.001034s | 0.000982s | 0.000996s | byte equality observed |
| make_unfused.test | 10 | 0.001017s | 0.001009s | 0.001002s | byte equality observed |
| make_vector.test | 10 | 0.000986s | 0.001005s | 0.000997s | byte equality observed |
| map.test | 10 | 0.001034s | 0.001022s | 0.001027s | byte equality observed |
| map_comparison.test | 10 | 0.000989s | 0.000980s | 0.000988s | byte equality observed |
| map_construction.test | 10 | 0.001011s | 0.001015s | 0.000991s | byte equality observed |
| map_copy.test | 10 | 0.001002s | 0.001005s | 0.001012s | byte equality observed |
| map_misc.test | 10 | 0.000999s | 0.001026s | 0.000982s | byte equality observed |
| map_move.test | 10 | 0.001008s | 0.001010s | 0.001022s | byte equality observed |
| map_mutate.test | 10 | 0.000998s | 0.001028s | 0.000998s | byte equality observed |
| map_tie.test | 10 | 0.000984s | 0.000986s | 0.000981s | byte equality observed |
| move.test | 10 | 0.000985s | 0.000983s | 0.000993s | byte equality observed |
| nil.test | 10 | 0.000986s | 0.000996s | 0.001027s | byte equality observed |
| none.test | 10 | 0.000998s | 0.000979s | 0.000996s | byte equality observed |
| nview.test | 10 | 0.001001s | 0.000991s | 0.000992s | byte equality observed |
| pop_back.test | 10 | 0.001046s | 0.001017s | 0.001048s | byte equality observed |
| pop_front.test | 10 | 0.001037s | 0.001034s | 0.001040s | byte equality observed |
| push_back.test | 10 | 0.001027s | 0.001024s | 0.001027s | byte equality observed |
| push_front.test | 10 | 0.001033s | 0.001034s | 0.001021s | byte equality observed |
| quick.test | 10 | 0.000455s | 0.000965s | 0.000991s | byte equality observed |
| ref_vector.test | 10 | 0.001019s | 0.000993s | 0.001026s | byte equality observed |
| remove.test | 10 | 0.001042s | 0.001022s | 0.001033s | byte equality observed |
| remove_if.test | 10 | 0.001033s | 0.001005s | 0.001043s | byte equality observed |
| repetitive_view.test | 10 | 0.000997s | 0.000986s | 0.000983s | byte equality observed |
| replace.test | 10 | 0.001030s | 0.001022s | 0.001024s | byte equality observed |
| replace_if.test | 10 | 0.001046s | 0.001017s | 0.001067s | byte equality observed |
| reverse.test | 10 | 0.001045s | 0.001031s | 0.001024s | byte equality observed |
| reverse_fold.test | 10 | 0.001025s | 0.001012s | 0.001016s | byte equality observed |
| reverse_iter_fold.test | 10 | 0.001004s | 0.001017s | 0.001005s | byte equality observed |
| reverse_view.test | 10 | 0.001024s | 0.001016s | 0.001031s | byte equality observed |
| segmented_find.test | 10 | 0.001015s | 0.000980s | 0.000983s | byte equality observed |
| segmented_find_if.test | 10 | 0.001002s | 0.000987s | 0.001010s | byte equality observed |
| segmented_fold.test | 10 | 0.001041s | 0.001018s | 0.001025s | byte equality observed |
| segmented_for_each.test | 10 | 0.001025s | 0.001003s | 0.001034s | byte equality observed |
| segmented_iterator_range.test | 10 | 0.001038s | 0.001036s | 0.001023s | byte equality observed |
| set.test | 10 | 0.001009s | 0.001001s | 0.001026s | byte equality observed |
| single_view.test | 10 | 0.001030s | 0.000995s | 0.000993s | byte equality observed |
| std_array.test | 10 | 0.000983s | 0.000991s | 0.000986s | byte equality observed |
| std_pair.test | 10 | 0.001021s | 0.001012s | 0.001033s | byte equality observed |
| std_tuple.test | 10 | 0.000995s | 0.000982s | 0.001002s | byte equality observed |
| std_tuple_iterator.test | 10 | 0.000995s | 0.000970s | 0.001006s | byte equality observed |
| swap.test | 10 | 0.001003s | 0.001028s | 0.000974s | byte equality observed |
| transform.test | 10 | 0.001007s | 0.001037s | 0.001021s | byte equality observed |
| transform_view.test | 10 | 0.001036s | 0.001030s | 0.001007s | byte equality observed |
| tuple_comparison.test | 10 | 0.000988s | 0.001005s | 0.001013s | byte equality observed |
| tuple_construction.test | 10 | 0.001007s | 0.000990s | 0.001026s | byte equality observed |
| tuple_conversion.test | 10 | 0.001007s | 0.000976s | 0.001003s | byte equality observed |
| tuple_copy.test | 10 | 0.000995s | 0.001011s | 0.000993s | byte equality observed |
| tuple_element.test | 10 | 0.000993s | 0.001008s | 0.000981s | byte equality observed |
| tuple_hash.test | 10 | 0.001047s | 0.001007s | 0.000968s | byte equality observed |
| tuple_make.test | 10 | 0.001005s | 0.001018s | 0.001009s | byte equality observed |
| tuple_misc.test | 10 | 0.000979s | 0.001012s | 0.000997s | byte equality observed |
| tuple_mutate.test | 10 | 0.000991s | 0.001003s | 0.000998s | byte equality observed |
| tuple_nest.test | 10 | 0.001007s | 0.001005s | 0.000983s | byte equality observed |
| tuple_tie.test | 10 | 0.001021s | 0.000983s | 0.000990s | byte equality observed |
| tuple_traits__maybe_variadic.test | 10 | 0.001009s | 0.000987s | 0.000991s | byte equality observed |
| tuple_traits__no_variadic.test | 10 | 0.001011s | 0.000975s | 0.000994s | byte equality observed |
| unfused.test | 10 | 0.000997s | 0.000982s | 0.000985s | byte equality observed |
| unfused_typed.test | 10 | 0.001030s | 0.000993s | 0.000992s | byte equality observed |
| vector_comparison.test | 10 | 0.000987s | 0.001010s | 0.000971s | byte equality observed |
| vector_construction.test | 10 | 0.001015s | 0.000985s | 0.000972s | byte equality observed |
| vector_conversion.test | 10 | 0.000999s | 0.000962s | 0.000985s | byte equality observed |
| vector_copy.test | 10 | 0.001005s | 0.000986s | 0.000983s | byte equality observed |
| vector_hash.test | 10 | 0.001002s | 0.000968s | 0.001007s | byte equality observed |
| vector_iterator.test | 10 | 0.001011s | 0.000974s | 0.001005s | byte equality observed |
| vector_make.test | 10 | 0.001008s | 0.000998s | 0.001024s | byte equality observed |
| vector_misc.test | 10 | 0.001004s | 0.000991s | 0.000986s | byte equality observed |
| vector_move.test | 10 | 0.001007s | 0.001008s | 0.001026s | byte equality observed |
| vector_mutate.test | 10 | 0.000998s | 0.000994s | 0.000996s | byte equality observed |
| vector_n.test | 10 | 0.001025s | 0.001031s | 0.001034s | byte equality observed |
| vector_nest.test | 10 | 0.000993s | 0.000981s | 0.000987s | byte equality observed |
| vector_tie.test | 10 | 0.001026s | 0.000998s | 0.000994s | byte equality observed |
| vector_traits__maybe_variadic.test | 10 | 0.001002s | 0.001008s | 0.000994s | byte equality observed |
| vector_traits__no_variadic.test | 10 | 0.000998s | 0.000991s | 0.001020s | byte equality observed |
| vector_value_at.test | 10 | 0.000997s | 0.000999s | 0.001008s | byte equality observed |
| zip.test | 10 | 0.000995s | 0.001018s | 0.000998s | byte equality observed |
| zip2.test | 10 | 0.000999s | 0.001012s | 0.000988s | byte equality observed |
| zip_ignore.test | 10 | 0.001004s | 0.001005s | 0.001005s | byte equality observed |
| zip_view.test | 10 | 0.000992s | 0.000992s | 0.001002s | byte equality observed |
| zip_view2.test | 10 | 0.000981s | 0.000997s | 0.000978s | byte equality observed |
| zip_view_ignore.test | 10 | 0.000988s | 0.001013s | 0.000974s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
